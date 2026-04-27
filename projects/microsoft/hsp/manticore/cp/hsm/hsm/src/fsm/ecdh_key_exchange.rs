// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

/// FSM states
#[derive(Clone, Copy, PartialEq, Eq)]
enum State {
    /// Initial state
    Init,

    /// Waiting for PKA Engine to compute ECDH
    WaitForEngine,

    /// Wait for PKA operation
    WaitForCmd,

    /// Final state
    Final,
}

/// ECDH Key Exchange command
pub(crate) struct EcdhKeyExchangeCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// App Session
    session: E::UserSession,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Decoded request
    decoded_req: Option<DdiEcdhKeyExchangeReq>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,

    /// ECDH Command data
    op: Option<EcdhComputeCmd<E>>,

    /// Key ID in case of rollback
    key_id: Option<KeyId>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for EcdhKeyExchangeCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (State::WaitForEngine, HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)) => {
                self.on_engine_ready(tag)
            }
            (State::WaitForCmd, HsmFsmEvent::PkaDone(_)) | (_, HsmFsmEvent::PkaError(_)) => {
                self.on_cmd_complete()
            }
            (State::Final, _) => {
                error!(
                    "[ecdh_key_exchange] Invalid State, state:{:?} event: {:?}",
                    self.state as u32,
                    u32::from(event)
                );
                Err(HsmErr::InvalidState)
            }
            (_, _) => {
                error!(
                    "[ecdh_key_exchange] Invalid Event, state:{:?} event: {:?}",
                    self.state as u32,
                    u32::from(event)
                );
                Err(HsmErr::InvalidEvent)
            }
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

impl<E: HsmEnvTrait> EcdhKeyExchangeCmd<E> {
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
            decoded_req: None,
            resp: None,
            op: None,
            key_id: None,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> HsmResult<()> {
        // Decode the request
        let req = decode_buf::<DdiEcdhKeyExchangeCmdReq, E>(&self.req)?;
        self.decoded_req = Some(req.data);

        // Validate the key metadata to have a valid usage
        let _key_usage: DdiKeyUsage = self
            .decoded_req
            .as_ref()
            .ok_or(HsmErr::InvalidState)?
            .key_properties
            .key_metadata
            .try_into()
            .map_err(|_| HsmErr::InvalidPermissions)?;

        self.begin_ecdh_compute(tag)
    }

    ///  Handle the engine ready event
    fn on_engine_ready(&mut self, tag: TagId) -> HsmResult<()> {
        self.begin_ecdh_compute(tag)
    }

    /// Handle the command complete event
    fn on_cmd_complete(&mut self) -> Result<(), HsmErr> {
        self.on_ecdh_cmd_complete()
    }

    /// Begin ECDH Compute (begin Montgomery Constant Calculation)
    fn begin_ecdh_compute(&mut self, tag: TagId) -> HsmResult<()> {
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::EcdhComputeFailed)?;
        let pub_key = &(&decoded_req.pub_key_der).into();

        match self.session.begin_ecdh_compute_with_pub_key_validation(
            tag,
            decoded_req.priv_key_id,
            decoded_req.key_type,
            pub_key,
        ) {
            Ok(op) => {
                self.op = Some(op);
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }
            Err(err) if err.pending() && self.state == State::WaitForEngine => {
                self.state = State::Final;
                Err(HsmErr::InvalidState)
            }
            Err(err) if err.pending() => {
                self.state = State::WaitForEngine;
                Err(err)
            }
            Err(err) => {
                self.state = State::Final;
                Err(err)
            }
        }
    }

    /// Handle the ECDH command done event
    fn on_ecdh_cmd_complete(&mut self) -> HsmResult<()> {
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::EcdhComputeFailed)?;
        let op = self.op.take().ok_or(HsmErr::InvalidState)?;

        match op.state {
            EcdhComputeCmdState::MontgomeryConstCal | EcdhComputeCmdState::PointValidation => {
                self.handle_continue_ecdh_compute(op, &(&decoded_req.pub_key_der).into())
            }
            EcdhComputeCmdState::EcdhCompute => {
                let key_usage: DdiKeyUsage = decoded_req
                    .key_properties
                    .key_metadata
                    .try_into()
                    .map_err(|_| HsmErr::InvalidPermissions)?;

                let key_availability = if decoded_req.key_properties.key_metadata.session() {
                    KeyAvailability::Session
                } else {
                    KeyAvailability::App
                };

                self.handle_end_ecdh_compute(op, key_usage, decoded_req.key_tag, key_availability)
            }
        }
    }

    /// Handle the continue ECDH compute command
    fn handle_continue_ecdh_compute(
        &mut self,
        op: EcdhComputeCmd<E>,
        pub_key: &IoMemRange,
    ) -> HsmResult<()> {
        match self.session.continue_ecdh_compute_zc(op, pub_key) {
            Ok(op) => {
                self.op = Some(op);
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }
            Err(mut err) => {
                if err.pending() {
                    // Pending error unexpected here
                    err = HsmErr::InvalidState;
                }
                self.state = State::Final;
                Err(err)
            }
        }
    }

    /// Handle the end ECDH compute command
    fn handle_end_ecdh_compute(
        &mut self,
        op: EcdhComputeCmd<E>,
        key_usage: DdiKeyUsage,
        key_tag: Option<u16>,
        key_availability: KeyAvailability,
    ) -> Result<(), HsmErr> {
        let key_id = self
            .session
            .end_ecdh_compute(op, key_usage, key_tag, key_availability)?;

        // Save key_id for rollback
        self.key_id = Some(key_id);

        // Encode and save the buffer
        self.resp =
            self.generate_response_with_mk(self.session.api_rev(), self.session.id(), key_id)?;

        self.state = State::Final;

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(
        &self,
        rev: DdiApiRev,
        sess_id: u16,
        key_id: u16,
        masked_key_len: usize,
    ) -> DdiEcdhKeyExchangeCmdResp {
        DdiEcdhKeyExchangeCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::EcdhKeyExchange,
                sess_id: Some(sess_id),
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiEcdhKeyExchangeResp {
                key_id,
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
    ) -> Result<Option<DmaBuffer<E>>, HsmErr> {
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::EcdhComputeFailed)?;

        let masked_key_len = self.session.get_masked_key_len_from_vault(
            decoded_req.key_properties.key_label.as_slice(),
            key_id,
            None,
        )?;

        let mut resp = self.cmd_resp(rev, sess_id, key_id, masked_key_len);

        let buf = Some(encode_buf(&resp, &self.heap)?);

        self.session.mask_key_from_vault(
            decoded_req.key_properties.key_label.as_slice(),
            key_id,
            None,
            resp.data.masked_key.as_mut_slice(),
        )?;

        Ok(buf)
    }
}
