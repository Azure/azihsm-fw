// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

/// FSM states
#[derive(Clone, Copy, PartialEq)]
#[allow(dead_code)]
enum State {
    /// Initial state
    Init,

    /// Waiting for PKA Engine
    WaitForResource,

    /// Wait for PKA operation
    WaitForCmd,

    /// Final state
    Final,
}

/// RSA Unwrap KEK Test command (FIPS validation only)
pub(crate) struct RsaUnwrapKekTestCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Session
    session: E::UserSession,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Rsa Modular Exponentiation Operation
    rsa_op: Option<RsaModExp<E>>,

    /// Response DMA buffer
    resp_buf: Option<DmaBuffer<E>>,

    /// Unwrapped data
    unwrapped_data: Option<DmaBuffer<E>>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for RsaUnwrapKekTestCmd<E> {
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp_buf.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.handle_begin_rsa_unwrap(tag),
            (State::WaitForResource, HsmFsmEvent::ResourceReady(_res)) => {
                self.handle_begin_rsa_unwrap(tag)
            }
            (State::WaitForCmd, HsmFsmEvent::PkaDone(_))
            | (State::WaitForCmd, HsmFsmEvent::PkaError(_)) => self.on_cmd_complete(tag),
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
    fn acquire_resource(&mut self, _tag: TagId, res_id: ResId) -> HsmFsmEvent {
        match res_id {
            HsmFsmResourceId::Pka => HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            _ => unreachable!(),
        }
    }
}

impl<E: HsmEnvTrait> RsaUnwrapKekTestCmd<E> {
    /// Create a new command FSM
    pub fn new(req: DmaBuffer<E>, heap: DmaHeap<E>, session: E::UserSession) -> Self {
        Self {
            state: State::Init,
            heap,
            session,
            req,
            rsa_op: None,
            resp_buf: None,
            unwrapped_data: None,
        }
    }

    /// Handle the PKA command done event
    fn on_cmd_complete(&mut self, tag: TagId) -> HsmResult<()> {
        self.handle_end_rsa_unwrap(tag)
    }

    fn handle_begin_rsa_unwrap(&mut self, tag: u16) -> Result<(), HsmErr> {
        match self.begin_rsa_unwrap(tag) {
            Ok(op) => {
                self.rsa_op = Some(op);
                self.state = State::WaitForCmd;

                Err(HsmErr::Pending)
            }
            Err(mut err) => {
                if err.pending() && self.state == State::Init {
                    self.state = State::WaitForResource
                } else if err.pending() && self.state == State::WaitForResource {
                    self.state = State::Final;
                    err = HsmErr::InvalidState
                } else {
                    self.state = State::Final
                }

                Err(err)
            }
        }
    }

    fn handle_end_rsa_unwrap(&mut self, tag: u16) -> Result<(), HsmErr> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;

        // Extract the KEK by OAEP decoding and copy it to the GSRAM
        // so that it can be shared across cores.
        let decoded_req = decode_buf::<DdiRsaUnwrapKekCmdReq, E>(&self.req)?;
        let kek = {
            let op = self.rsa_op.take().ok_or(HsmErr::InvalidState)?;

            self.session.end_rsa_unwrap_mod_exp_zc(tag, op)?;

            let unwrapped_data = self
                .unwrapped_data
                .take()
                .ok_or(HsmErr::RsaUnwrapInternalErr)?;
            let kek = self.session.decode_oaep_kek(
                unwrapped_data.as_ref(),
                decoded_req.data.wrapped_blob_padding,
                decoded_req.data.wrapped_blob_hash_algorithm,
            )?;

            // The size of Key-encryption key is bounded by AES 256
            if kek.len() > AesKeyKind::Aes256.into() {
                Err(HsmErr::RsaUnwrapInvalidKek)?
            }

            kek
        };

        let resp = self.cmd_resp(self.session.api_rev(), self.session.id(), kek.as_ref());

        self.resp_buf = Some(encode_buf(&resp, &self.heap)?);
        Ok(())
    }

    /// Start the CKM_RSA_AES_KEY_WRAP unwrap operation.
    fn begin_rsa_unwrap(&mut self, tag: TagId) -> HsmResult<RsaModExp<E>> {
        // Decode the request
        let decoded_req = decode_buf::<DdiRsaUnwrapKekCmdReq, E>(&self.req)?;

        let wrapped_blob: MborByteArray<256> =
            MborByteArray::new_with_len(decoded_req.data.wrapped_blob.ptr(), 256);

        // Create a buffer for rsa_unwrap_mod_exp_zc to output to
        self.unwrapped_data = Some(self.heap.allocate(256).ok_or(HsmErr::DmaAllocFailure)?);
        let unwrap_output_buffer = self
            .unwrapped_data
            .as_ref()
            .ok_or(HsmErr::RsaUnwrapInternalErr)?
            .as_ref();
        let unwrap_output_buffer_mbor: MborByteArray<256> =
            MborByteArray::new_with_len(unwrap_output_buffer.as_ptr(), 256);

        self.session.begin_rsa_unwrap_mod_exp_zc(
            tag,
            decoded_req.data.key_id,
            &(&wrapped_blob).into(),
            &(&unwrap_output_buffer_mbor).into(),
            Some(RsaKeyUsage::Unwrap),
        )
    }

    fn cmd_resp(&self, rev: DdiApiRev, sess_id: u16, kek: &[u8]) -> DdiRsaUnwrapKekCmdResp {
        DdiRsaUnwrapKekCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::RsaUnwrapKekTest,
                sess_id: Some(sess_id),
                status: DdiStatus::Success,
                fips_approved: false,
            },
            data: DdiRsaUnwrapKekResp {
                kek: mcr_ddi_mbor::MborByteArray::<256>::new_with_len(kek.as_ptr(), kek.len()),
            },
        }
    }
}
