// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaRsaSize;

use super::*;

/// Max time tick count to wait for IPC resource
const MAX_RESOURCE_WAIT_TIME: u8 = 16;

/// FSM states
#[derive(Clone, Copy, PartialEq)]
enum State {
    /// Initial state
    Init,

    /// Waiting for PKA command completion (PCT in progress).
    WaitForCmd,

    /// Wait for begin PCT validation
    WaitForEngineToBeginPct,

    /// Final state
    Final,
}

/// Get unwrapping key id command
pub(crate) struct GetUnwrappingKeyCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Session
    session: E::UserSession,

    /// Partition
    part: E::Partition,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,

    /// PCIe Function this get unwrapping key belongs to
    pfn: PcieFunction,

    /// Unwrapping key id
    key_id: Option<KeyId>,

    // GetUnwrappingKeyOut
    output: Option<GetUnwrappingKeyOut>,

    /// PCT Validation Operation
    pct_op: Option<RsaPctValidationCmd<E>>,

    /// Check Alive Counter
    check_alive_cnt: u8,

    /// Flag indicating if PCT validation is in progress
    in_middle_of_pct_validation: bool,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for GetUnwrappingKeyCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (_, HsmFsmEvent::CheckAlive) => self.check_alive(),
            (State::WaitForEngineToBeginPct, HsmFsmEvent::ResourceReady(_)) => {
                self.handle_pct_validation_on_engine_ready(tag)
            }
            (State::WaitForCmd, HsmFsmEvent::PkaError(_))
            | (State::WaitForCmd, HsmFsmEvent::PkaDone(_)) => self.handle_continue_pct_validation(),
            (State::Final, _) => Err(HsmErr::InvalidState),
            (_, _) => Err(HsmErr::InvalidEvent),
        }
    }

    /// Get the session ID
    fn session_id(&self) -> Option<u16> {
        Some(self.session.id())
    }

    /// Check if the command requires resource
    fn requires_resource(&self, _tag: TagId, _res_id: ResId) -> bool {
        true
    }

    /// Acquire a resource
    fn acquire_resource(&mut self, _tag: TagId, _res_id: ResId) -> HsmFsmEvent {
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    }

    /// Perform any rollback in case of error
    fn rollback(&mut self, _tag: TagId) -> HsmResult<()> {
        if self.in_middle_of_pct_validation {
            self.part.clear_unwrapping_key()?;
            self.in_middle_of_pct_validation = false;
        }

        Ok(())
    }
}

impl<E: HsmEnvTrait> GetUnwrappingKeyCmd<E> {
    /// Create a new command FSM
    pub fn new(
        req: DmaBuffer<E>,
        heap: DmaHeap<E>,
        session: E::UserSession,
        part: E::Partition,
        pfn: PcieFunction,
    ) -> Self {
        Self {
            state: State::Init,
            heap,
            session,
            part,
            req,
            resp: None,
            pfn,
            key_id: None,
            output: None,
            pct_op: None,
            check_alive_cnt: 0,
            in_middle_of_pct_validation: false,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // Force decode the header and body to ensure body is empty
        let _ = decode_buf::<DdiGetUnwrappingKeyCmdReq, E>(&self.req)?;

        self.get_unwrapping_key(tag)
    }

    /// Dispatch to vault-hit / BK-import / PendingKeyGeneration paths.
    fn get_unwrapping_key(&mut self, tag: u16) -> Result<(), HsmErr> {
        self.key_id = self.part.unwrapping_key_id();

        match self.session.get_unwrapping_key(tag, self.key_id, self.pfn) {
            Ok(ctx) => {
                let output = ctx.output.ok_or(HsmErr::InvalidState)?;

                // Gate the PCT on the validity flag, not on `key_id`.  A key imported via
                // `unmask_unwrapping_key_and_import` lands in the vault (`key_id.is_some()`) in the
                // `PendingPct` state, so it must still run the deferred PCT before it is returned to
                // the host.  Only a `PctPassed` key may skip the PCT.
                if self.part.is_unwrapping_key_pct_verified() {
                    self.state = State::Final;
                    self.prepare_response(output)
                } else {
                    self.output = Some(output);
                    self.handle_begin_pct_validation(tag)
                }
            }
            Err(err) => {
                self.state = State::Final;
                Err(err)
            }
        }
    }

    /// Prepare the response for a successful get unwrapping key command
    fn prepare_response(&mut self, output: GetUnwrappingKeyOut) -> Result<(), HsmErr> {
        self.resp = self.generate_response_with_mk(
            self.session.api_rev(),
            self.session.id(),
            output.id,
            output.data.pka_as_slice()?,
        )?;

        Ok(())
    }

    /// Handle the RSA pct validation command
    fn handle_begin_pct_validation(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let output = self.output.as_mut().ok_or(HsmErr::InvalidState)?;
        let key_id = output.id;
        let usage = RsaKeyUsage::Unwrap;
        let rsa_type = PkaRsaSize::Rsa2k;
        let (n, e) = output.data.data.split_at_mut(output.data.n_len);
        let e = &e[..4];
        self.in_middle_of_pct_validation = true;

        match self
            .session
            .begin_rsa_pct_validation(tag, key_id, usage, rsa_type, n, e)
        {
            Ok(pct_op) => {
                self.pct_op = Some(pct_op);
                self.state = State::WaitForCmd;

                Err(HsmErr::Pending)
            }
            Err(err) => {
                if err.pending() {
                    self.state = State::WaitForEngineToBeginPct;

                    Err(HsmErr::Pending)
                } else {
                    self.on_error(err)
                }
            }
        }
    }

    /// Handle the RSA pct validation command on engine ready
    fn handle_pct_validation_on_engine_ready(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let output = self.output.as_mut().ok_or(HsmErr::InvalidState)?;
        let key_id = output.id;
        let usage = RsaKeyUsage::Unwrap;
        let rsa_type = PkaRsaSize::Rsa2k;
        let (n, e) = output.data.data.split_at_mut(output.data.n_len);
        let e = &e[..4];

        match self
            .session
            .begin_rsa_pct_validation(tag, key_id, usage, rsa_type, n, e)
        {
            Ok(pct_op) => {
                self.pct_op = Some(pct_op);
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }
            Err(err) => self.on_error(err),
        }
    }

    /// Handle the RSA PCT continue validation
    fn handle_continue_pct_validation(&mut self) -> Result<(), HsmErr> {
        let pct_op = self.pct_op.take().ok_or(HsmErr::InvalidState)?;

        if self.session.is_rsa_pct_final_state(&pct_op) {
            match self.session.end_rsa_pct_validation(pct_op) {
                Ok(success) => {
                    if !success {
                        // The next step will be a crash and recovery
                        // so we ignore the return result here
                        let _ = self.part.clear_unwrapping_key();

                        self.session.notify_pct_validation_failure(
                            HsmErr::PctValidationUnwrappingKeyFailed as u32,
                        );
                    } else {
                        // Promote flag to `PctPassed` so a post-vault-clear re-import can skip PCT.
                        self.part.mark_unwrapping_key_pct_verified();
                    }
                }
                Err(err) => {
                    return self.on_error(err);
                }
            }

            // Create final response buffer after successful validation
            let output = self.output.take().ok_or(HsmErr::InvalidState)?;

            self.in_middle_of_pct_validation = false;

            self.state = State::Final;
            self.prepare_response(output)
        } else {
            match self.session.continue_rsa_pct_validation(pct_op) {
                Ok(continue_op) => {
                    // Continue PCT validation
                    self.pct_op = Some(continue_op);
                    self.state = State::WaitForCmd;

                    Err(HsmErr::Pending)
                }
                Err(err) => self.on_error(err),
            }
        }
    }

    /// Handle operation errors
    fn on_error(&mut self, mut err: HsmErr) -> Result<(), HsmErr> {
        if err.pending() {
            err = HsmErr::InvalidState;
        }

        // Move FSM to Final state
        self.state = State::Final;

        Err(err)
    }

    /// On timer event response
    fn check_alive(&mut self) -> Result<(), HsmErr> {
        if self.state != State::WaitForCmd && self.state != State::WaitForEngineToBeginPct {
            return Err(HsmErr::Pending);
        }

        if self.check_alive_cnt < MAX_RESOURCE_WAIT_TIME {
            self.check_alive_cnt += 1;

            Err(HsmErr::Pending)
        } else {
            error!(
                "GetUnwrappingKey command timed out. {:?}",
                self.state as u32
            );
            self.check_alive_cnt = 0;
            self.state = State::Final;

            Err(HsmErr::IoTimeOut)
        }
    }

    /// Create a command response
    fn cmd_resp(
        &self,
        rev: DdiApiRev,
        sess_id: u16,
        key_id: KeyId,
        pub_key: &[u8],
        masked_key_len: usize,
    ) -> DdiGetUnwrappingKeyCmdResp {
        DdiGetUnwrappingKeyCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::GetUnwrappingKey,
                sess_id: Some(sess_id),
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiGetUnwrappingKeyResp {
                key_id,
                pub_key: DdiDerPublicKey {
                    der: MborByteArray::new_with_len(pub_key.as_ptr(), pub_key.len()),
                    key_kind: DdiKeyType::Rsa2kPublic,
                },
                masked_key: MborByteArray::new_with_len(core::ptr::null(), masked_key_len),
            },
        }
    }

    /// Generate the masked key and encode the response
    /// Step:
    /// 1. Get the encoded length
    /// 2. Pre encode the response
    /// 3. Generate the masked key in the pre-encoded field `masked_key`
    fn generate_response_with_mk(
        &self,
        rev: DdiApiRev,
        sess_id: u16,
        key_id: KeyId,
        pub_key: &[u8],
    ) -> Result<Option<DmaBuffer<E>>, HsmErr> {
        let masked_key_len =
            self.session
                .get_masked_key_len_from_vault(&[], key_id, Some(pub_key))?;

        let mut resp = self.cmd_resp(rev, sess_id, key_id, pub_key, masked_key_len);

        let buf = Some(encode_buf(&resp, &self.heap)?);

        self.session.mask_key_from_vault(
            &[],
            key_id,
            Some(pub_key),
            resp.data.masked_key.as_mut_slice(),
        )?;

        Ok(buf)
    }
}
