// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaEccCurve;
use mcr_crypto_pka::PkaEccPublicKey;

use super::*;

type GetSessionEncryptionKeyCmdCtx<E> = GetSessionEncryptionKeyCtx<HsmPartitionEnv<E>>;

/// Max time tick count to wait for IPC resource
const MAX_RESOURCE_WAIT_TIME: u8 = 16;

/// FSM states
#[derive(Clone, Copy, PartialEq)]
enum State {
    /// Initial state
    Init,

    /// Waiting for UPKA engine available
    WaitForEngineToBeginSessionEncKey,

    /// Wait for completion of PKA operation for getting session key
    WaitForEndGetSessionEncKey,

    /// Wait for begin PCT validation
    WaitForEngineToBeginPct,

    /// Wait for completion of PKA operation for ECC PCT validation
    WaitForPctValidaiton,

    /// Wait for resource for ECC sign
    WaitForEngineToBeginEccSign,

    /// Wait for completion of PKA operation for ECC sign
    WaitForEndEccSign,

    /// Final state
    Final,
}

/// Get session encryption key id command
pub(crate) struct GetSessionEncryptionKeyCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,

    /// API rev
    api_rev: Option<DdiApiRev>,

    /// Command response
    cmd_resp: Option<DdiGetSessionEncryptionKeyCmdResp>,

    /// Get Session Encryption Key command context
    cmd_ctx: Option<GetSessionEncryptionKeyCmdCtx<E>>,

    /// Key ID
    key_id: Option<KeyId>,

    /// PCT Validation Operation (Sign, Verify, ECDH)
    pct_op: Option<EccKeyPct<HsmPartitionEnv<E>>>,

    /// ECC Key Usage
    usage: EccKeyUsage,

    /// Separate public key for easy access
    public_key: Option<PkaEccPublicKey>,

    /// Key sign context
    key_sign_ctx: Option<
        KeySignContext<<<E as env::HsmEnvTrait>::Partition as partition::HsmPartition>::Env>,
    >,

    /// Check Alive Counter
    check_alive_cnt: u8,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for GetSessionEncryptionKeyCmd<E> {
    /// Take the response buffer if it is available
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (State::WaitForEngineToBeginSessionEncKey, HsmFsmEvent::ResourceReady(_)) => {
                self.begin_get_session_encryption_key(tag)
            }
            (State::WaitForEngineToBeginPct, HsmFsmEvent::ResourceReady(_)) => {
                self.begin_pct_validation(tag)
            }
            (State::WaitForEngineToBeginEccSign, HsmFsmEvent::ResourceReady(_)) => {
                self.begin_ecc_sign_with_partition_id_priv_key(tag)
            }
            (State::WaitForEndGetSessionEncKey, HsmFsmEvent::PkaDone(_))
            | (State::WaitForEndGetSessionEncKey, HsmFsmEvent::PkaError(_)) => {
                self.end_get_session_encryption_key(tag)
            }
            (State::WaitForPctValidaiton, HsmFsmEvent::PkaDone(_))
            | (State::WaitForPctValidaiton, HsmFsmEvent::PkaError(_)) => {
                self.continue_pct_validation(tag)
            }
            (State::WaitForEndEccSign, HsmFsmEvent::PkaDone(_))
            | (State::WaitForEndEccSign, HsmFsmEvent::PkaError(_)) => {
                self.end_ecc_sign_with_partition_id_priv_key(tag)
            }
            (_, HsmFsmEvent::CheckAlive) => self.check_alive(),
            (_, _) => Err(HsmErr::InvalidEvent),
        }
    }

    /// Perform any rollback in case of error
    fn rollback(&mut self, _tag: TagId) -> HsmResult<()> {
        if let Some(key_id) = self.key_id {
            self.part.delete_internal_key(key_id)?;
            self.part.unset_session_encryption_key_id();
        }

        Ok(())
    }

    /// Check if the command requires resource
    fn requires_resource(&self, _tag: TagId, _res_id: ResId) -> bool {
        true
    }

    /// Acquire a resource
    fn acquire_resource(&mut self, _tag: TagId, _res_id: ResId) -> HsmFsmEvent {
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    }
}

impl<E: HsmEnvTrait> GetSessionEncryptionKeyCmd<E> {
    /// Create a new command FSM
    pub fn new(req: DmaBuffer<E>, heap: DmaHeap<E>, part: E::Partition) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            req,
            resp: None,
            api_rev: None,
            cmd_resp: None,
            cmd_ctx: None,
            key_id: None,
            pct_op: None,
            usage: EccKeyUsage::KeyAgreement,
            public_key: None,
            key_sign_ctx: None,
            check_alive_cnt: 0,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let req = decode_buf::<DdiGetSessionEncryptionKeyCmdReq, E>(&self.req)?;

        let rev = req.hdr.rev.ok_or(HsmErr::UnsupportedRevision)?;
        self.api_rev = Some(rev);

        self.begin_get_session_encryption_key(tag)
    }

    /// Helper to begin the process of get session encryption key
    fn begin_get_session_encryption_key(&mut self, tag: TagId) -> Result<(), HsmErr> {
        match self.part.begin_get_session_encryption_key(tag) {
            Ok(ctx) => {
                if let Some(key_data) = ctx.key_data {
                    self.state = State::Final;

                    self.public_key = Some(key_data.pub_key_data);

                    self.prepare_response(key_data.nonce)?;

                    // Generate the signature using the partition ID private key
                    self.begin_ecc_sign_with_partition_id_priv_key(tag)
                } else {
                    self.state = State::WaitForEndGetSessionEncKey;
                    self.cmd_ctx = Some(ctx);

                    Err(HsmErr::Pending)
                }
            }
            Err(mut err) => {
                if err.pending() && self.state == State::Init {
                    self.state = State::WaitForEngineToBeginSessionEncKey
                } else if err.pending() && self.state == State::WaitForEngineToBeginSessionEncKey {
                    self.state = State::Final;
                    err = HsmErr::InvalidState
                } else {
                    self.state = State::Final
                }

                Err(err)
            }
        }
    }

    /// On PKA cmd complete
    fn end_get_session_encryption_key(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let ctx = self.cmd_ctx.take().ok_or(HsmErr::InvalidState)?;
        let out = self.part.end_get_session_encryption_key(ctx.tag, ctx)?;

        self.key_id = out.new_key_id;
        self.public_key = Some(out.pub_key);

        self.prepare_response(out.nonce)?;

        if out.new_key_id.is_none() {
            // The establish cred encryption key is already established
            // Generate the signature using the partition ID private key
            // Note: It is possible for the key to be established but the
            // the PCT for that key has yet to be validated, in the condition
            // that two of these FSMs are running concurrently. In that case,
            // since a PCT failure will cause a soft reset in the firmware it
            // should be okay.
            self.begin_ecc_sign_with_partition_id_priv_key(tag)
        } else {
            // Start PCT validation
            self.begin_pct_validation(tag)
        }
    }

    /// Handle the ECC pct validation command
    fn begin_pct_validation(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let key_id = self.key_id.ok_or(HsmErr::InvalidState)?;
        let public_key = self.public_key.as_ref().ok_or(HsmErr::InvalidState)?;
        let public_key_copy = PkaEccPublicKey {
            data: public_key.data,
            curve: public_key.curve,
        };

        match self
            .part
            .begin_ecc_pct_validation(tag, key_id, self.usage, public_key_copy)
        {
            Ok(pct_op) => {
                self.pct_op = Some(pct_op);
                self.state = State::WaitForPctValidaiton;

                Err(HsmErr::Pending)
            }
            Err(err) if err.pending() && self.state == State::WaitForEngineToBeginPct => {
                self.on_error(self.state as u32, EccPctValidationState::Init as u32, err)
            }
            Err(err) if err.pending() => {
                self.state = State::WaitForEngineToBeginPct;

                Err(HsmErr::Pending)
            }
            Err(err) => self.on_error(self.state as u32, EccPctValidationState::Init as u32, err),
        }
    }

    /// Handle the ECC PCT continue validation
    fn continue_pct_validation(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let mut pct_op = self.pct_op.take().ok_or(HsmErr::InvalidState)?;

        if self.part.is_pct_final_state(&pct_op) {
            match self.part.end_ecc_pct_validation(tag, &mut pct_op) {
                Ok(success) => {
                    if !success {
                        self.part.notify_pct_validation_failure(
                            HsmErr::PctValidationSessionEncKeyFailed as u32,
                        );
                    }
                }
                Err(err) => self.on_error(self.state as u32, pct_op.pct_state as u32, err)?,
            }

            // Drop the EccKeyPct instance since PCT validation is complete.
            // This frees up resources held by the instance.
            drop(pct_op);

            self.begin_ecc_sign_with_partition_id_priv_key(tag)
        } else {
            match self.part.continue_ecc_pct_validation(tag, &mut pct_op) {
                Ok(_) => {
                    // Continue PCT validation
                    self.pct_op = Some(pct_op);
                    self.state = State::WaitForPctValidaiton;

                    Err(HsmErr::Pending)
                }
                Err(err) => self.on_error(self.state as u32, pct_op.pct_state as u32, err),
            }
        }
    }

    /// Begin ECC sign of public key with partition ID private key
    fn begin_ecc_sign_with_partition_id_priv_key(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let resp = self.cmd_resp.as_mut().ok_or(HsmErr::InvalidState)?;

        match self.part.begin_signature_with_part_priv_key(
            tag,
            &(&resp.data.pub_key.der).into(),
            &(&resp.data.pub_key_signature).into(),
        ) {
            Err(HsmErr::Pending) => {
                if self.state == State::WaitForEngineToBeginEccSign {
                    // We don't expect to wait for the PKA engine here, we should have already acquired it.
                    self.state = State::Final;
                    return Err(HsmErr::InvalidState);
                }

                self.state = State::WaitForEngineToBeginEccSign;

                Err(HsmErr::Pending)
            }
            Ok(key_sign_ctx) => {
                self.state = State::WaitForEndEccSign;

                self.key_sign_ctx = Some(key_sign_ctx);

                Err(HsmErr::Pending)
            }
            Err(err) => {
                self.state = State::Final;
                Err(err)
            }
        }
    }

    /// End ECC sign of public key with partition ID private key
    fn end_ecc_sign_with_partition_id_priv_key(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let key_sign_ctx = self.key_sign_ctx.take().ok_or(HsmErr::InvalidState)?;

        match self.part.end_signature_with_key_blob(tag, key_sign_ctx) {
            Ok(()) => {
                self.state = State::Final;
                Ok(())
            }
            Err(err) => self.on_error(self.state as u32, 0, err),
        }
    }

    /// Handle errors
    fn on_error(&mut self, fsm_state: u32, pct_state: u32, mut err: HsmErr) -> Result<(), HsmErr> {
        let state: u32 = fsm_state << 16 | pct_state;

        error!(
            "[get_session_encryption_key] [op state: {:?}] PCT Validation Error: {:?}.",
            state,
            u32::from(err)
        );

        if err.pending() {
            err = HsmErr::InvalidState;
        }

        // Move FSM to Final state
        self.state = State::Final;

        Err(err)
    }

    /// On timer event response
    fn check_alive(&mut self) -> Result<(), HsmErr> {
        if (self.state == State::WaitForEngineToBeginSessionEncKey
            || self.state == State::WaitForEndGetSessionEncKey
            || self.state == State::WaitForEngineToBeginPct
            || self.state == State::WaitForPctValidaiton
            || self.state == State::WaitForEngineToBeginEccSign
            || self.state == State::WaitForEndEccSign)
            && self.check_alive_cnt < MAX_RESOURCE_WAIT_TIME
        {
            self.check_alive_cnt += 1;

            Err(HsmErr::Pending)
        } else {
            self.check_alive_cnt = 0;

            self.state = State::Final;

            Err(HsmErr::IoTimeOut)
        }
    }

    /// Prepare the response for a successful get param encryption key command
    fn prepare_response(&mut self, nonce: [u8; 32]) -> Result<(), HsmErr> {
        let pub_key = self.public_key.as_ref().ok_or(HsmErr::InvalidState)?;
        let pub_key_data = &pub_key.data[..pub_key.curve.len() * 2];

        let cmd_resp = self.cmd_resp(self.api_rev, pub_key_data, nonce);
        self.resp = Some(encode_buf(&cmd_resp, &self.heap)?);
        self.cmd_resp = Some(cmd_resp);

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(
        &self,
        rev: Option<DdiApiRev>,
        pub_key: &[u8],
        nonce: [u8; 32],
    ) -> DdiGetSessionEncryptionKeyCmdResp {
        DdiGetSessionEncryptionKeyCmdResp {
            hdr: DdiRespHdr {
                rev,
                op: DdiOp::GetSessionEncryptionKey,
                sess_id: None,
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiGetSessionEncryptionKeyResp {
                pub_key: DdiDerPublicKey {
                    der: MborByteArray::new_with_len(pub_key.as_ptr(), pub_key.len()),
                    key_kind: DdiKeyType::Ecc384Public,
                },
                nonce,
                pub_key_signature: MborByteArray::new_with_len(
                    core::ptr::null(),
                    PkaEccCurve::Ecc384.len() * 2,
                ),
            },
        }
    }
}
