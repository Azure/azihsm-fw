// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaEccPublicKey;

use super::*;
use log::error;

/// FSM states
#[derive(Clone, Copy, PartialEq)]
enum State {
    /// Initial state
    Init,

    /// Waiting for PKA Engine
    WaitForEngine,

    /// Wait for PKA operation
    WaitForCmd,

    /// Wait for begin PCT validation
    WaitForEngineToBeginPct,

    /// Final state
    Final,
}

/// ECC Gen Key command
pub(crate) struct EccGenKeyCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// Session
    session: E::UserSession,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,

    /// ECC Operation
    op: Option<EccGenKey<E>>,

    /// Key ID
    key_id: Option<u16>,

    /// PCT Validation Operation (Sign, Verify, ECDH)
    pct_op: Option<EccKeyPct<E>>,

    /// ECC Key Usage
    usage: Option<EccKeyUsage>,

    /// Separate public key for easy access
    public_key: Option<PkaEccPublicKey>,

    /// Cmd struct to avoid multiple decode operations on request DMA buffer
    decoded_req: Option<DdiEccGenerateKeyPairCmdReq>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for EccGenKeyCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (State::WaitForEngine, HsmFsmEvent::ResourceReady(_res)) => self.on_engine_ready(tag),
            (State::WaitForCmd, HsmFsmEvent::PkaDone(_))
            | (State::WaitForCmd, HsmFsmEvent::PkaError(_)) => self.on_cmd_complete(tag),
            (State::WaitForEngineToBeginPct, HsmFsmEvent::ResourceReady(_)) => {
                self.handle_begin_pct_validation(tag)
            }
            (State::Final, _) => Err(HsmErr::InvalidState),
            (_, _) => Err(HsmErr::InvalidEvent),
        }
    }

    /// Get the session ID
    fn session_id(&self) -> Option<u16> {
        Some(self.session.id())
    }

    /// Perform any rollback in case of error
    fn rollback(&mut self, _tag: TagId) -> HsmResult<()> {
        if let Some(key_id) = self.key_id {
            self.session.delete_key(key_id)?;
        }

        Ok(())
    }

    fn requires_resource(&self, _tag: TagId, _res_id: ResId) -> bool {
        true
    }

    fn acquire_resource(&mut self, _tag: TagId, _res_id: ResId) -> HsmFsmEvent {
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    }
}

impl<E: HsmEnvTrait> EccGenKeyCmd<E> {
    /// Create a new command FSM
    pub fn new(
        req: DmaBuffer<E>,
        heap: DmaHeap<E>,
        session: E::UserSession,
        part: E::Partition,
    ) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            session,
            req,
            resp: None,
            op: None,
            key_id: None,
            pct_op: None,
            usage: None,
            public_key: None,
            decoded_req: None,
        }
    }
    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        self.begin_ecc_gen_key(tag, true)
    }

    /// Handle the PKA Engine ready event
    fn on_engine_ready(&mut self, tag: TagId) -> Result<(), HsmErr> {
        self.begin_ecc_gen_key(tag, false)
    }

    /// Handle the ECC command done event
    fn on_cmd_complete(&mut self, tag: TagId) -> Result<(), HsmErr> {
        if let Some(op) = self.op.take() {
            self.usage = Some(op.usage);

            // Complete the ECC Key Generation
            let out = self._end_ecc_gen_key(tag, op)?;
            self.key_id = Some(out.ecc_key.id());
            self.public_key = Some(out.pub_key);

            // Start PCT validation
            self.handle_begin_pct_validation(tag)
        } else if self.pct_op.is_some() {
            // Continue PCT validation
            self.handle_continue_pct_validation(tag)
        } else {
            self.on_error(self.state as u32, 0xFF, HsmErr::InvalidState)
        }
    }

    /// Handle the ECC begin keygen command
    fn _begin_ecc_gen_key(&mut self, tag: TagId) -> HsmResult<EccGenKey<E>> {
        // Decode the request
        self.decoded_req = Some(decode_buf::<DdiEccGenerateKeyPairCmdReq, E>(&self.req)?);
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;
        let data = &decoded_req.data;

        let key_usage: DdiKeyUsage = data
            .key_properties
            .key_metadata
            .try_into()
            .map_err(|_| HsmErr::InvalidPermissions)?;

        let key_availability = if data.key_properties.key_metadata.session() {
            KeyAvailability::Session
        } else {
            KeyAvailability::App
        };

        self.session.begin_ecc_gen_key(
            tag,
            data.key_tag,
            data.curve.try_into()?,
            key_usage.try_into()?,
            key_availability,
        )
    }

    /// Handle the ECC end keygen command
    fn _end_ecc_gen_key(&mut self, tag: TagId, op: EccGenKey<E>) -> HsmResult<EccGenKeyOut> {
        self.session.end_ecc_gen_key(tag, op)
    }

    /// Handle the ECC begin keygen command
    fn begin_ecc_gen_key(&mut self, tag: TagId, transition_to_engine: bool) -> Result<(), HsmErr> {
        match self._begin_ecc_gen_key(tag) {
            Ok(op) => {
                self.op = Some(op);
                self.state = State::WaitForCmd;

                Err(HsmErr::Pending)
            }
            Err(err) => {
                if transition_to_engine && err.pending() {
                    self.state = State::WaitForEngine;

                    Err(HsmErr::Pending)
                } else {
                    self.on_error(self.state as u32, 0xFF, err)
                }
            }
        }
    }

    /// Handle the ECC pct validation command
    fn handle_begin_pct_validation(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let key_id = self.key_id.ok_or(HsmErr::InvalidState)?;
        let usage = self.usage.ok_or(HsmErr::InvalidState)?;
        let public_key = self.public_key.as_ref().ok_or(HsmErr::InvalidState)?;
        let public_key_copy = PkaEccPublicKey {
            data: public_key.data,
            curve: public_key.curve,
        };

        match self
            .session
            .begin_ecc_pct_validation(tag, key_id, usage, public_key_copy)
        {
            Ok(pct_op) => {
                self.pct_op = Some(pct_op);
                self.state = State::WaitForCmd;

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
    fn handle_continue_pct_validation(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let mut pct_op = self.pct_op.take().ok_or(HsmErr::InvalidState)?;

        if self.session.is_pct_final_state(&pct_op) {
            match self.session.end_ecc_pct_validation(tag, &mut pct_op) {
                Ok(success) => {
                    if !success {
                        self.session.notify_pct_validation_failure(
                            HsmErr::PctValidationEccGenKeyFailed as u32,
                        );
                    }
                }
                Err(err) => self.on_error(self.state as u32, pct_op.pct_state as u32, err)?,
            }

            // Create final response buffer after successful validation
            let key_id = self.key_id.ok_or(HsmErr::InvalidState)?;
            let public_key = self.public_key.as_ref().ok_or(HsmErr::InvalidState)?;

            self.resp = self.generate_response_with_mk(
                self.session.api_rev(),
                self.session.id(),
                key_id,
                public_key.pka_as_slice()?,
                public_key.curve.into(),
            )?;

            self.state = State::Final;

            Ok(())
        } else {
            match self.session.continue_ecc_pct_validation(tag, &mut pct_op) {
                Ok(_) => {
                    // Continue PCT validation
                    self.pct_op = Some(pct_op);
                    self.state = State::WaitForCmd;

                    Err(HsmErr::Pending)
                }
                Err(err) => self.on_error(self.state as u32, pct_op.pct_state as u32, err),
            }
        }
    }

    /// Handle errors
    fn on_error(&mut self, fsm_state: u32, pct_state: u32, mut err: HsmErr) -> Result<(), HsmErr> {
        // Concatinate the FSM state and PCT state since the error log can only accept 2 dwords
        // If the pct_state is 0xFF, it means that the PCT validation was not started
        let state: u32 = fsm_state << 16 | pct_state;

        error!(
            "[ecc_gen_key] [op state: {:?}] PCT Validation Error: {:?}.",
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

    /// Create a command response
    fn cmd_resp(
        &self,
        rev: DdiApiRev,
        sess_id: u16,
        key_id: u16,
        pubkey_data: &[u8],
        curve: EccCurve,
        masked_key_len: usize,
    ) -> DdiEccGenerateKeyPairCmdResp {
        let pub_key_kind = match curve {
            EccCurve::P256 => DdiKeyType::Ecc256Public,
            EccCurve::P384 => DdiKeyType::Ecc384Public,
            EccCurve::P521 => DdiKeyType::Ecc521Public,
        };

        DdiEccGenerateKeyPairCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::EccGenerateKeyPair,
                sess_id: Some(sess_id),
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiEccGenerateKeyPairResp {
                private_key_id: key_id,
                pub_key: Some(DdiDerPublicKey {
                    der: MborByteArray::new_with_len(pubkey_data.as_ptr(), pubkey_data.len()),
                    key_kind: pub_key_kind,
                }),
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
        key_id: u16,
        pubkey_data: &[u8],
        curve: EccCurve,
    ) -> Result<Option<DmaBuffer<E>>, HsmErr> {
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;
        let masked_key_len = self.session.get_masked_key_len_from_vault(
            decoded_req.data.key_properties.key_label.as_slice(),
            key_id,
            Some(pubkey_data),
        )?;

        let mut resp = self.cmd_resp(rev, sess_id, key_id, pubkey_data, curve, masked_key_len);

        let buf = Some(encode_buf(&resp, &self.heap)?);

        self.session.mask_key_from_vault(
            decoded_req.data.key_properties.key_label.as_slice(),
            key_id,
            Some(pubkey_data),
            resp.data.masked_key.as_mut_slice(),
        )?;

        Ok(buf)
    }
}
