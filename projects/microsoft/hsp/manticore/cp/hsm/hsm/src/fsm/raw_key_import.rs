// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

/// FSM states
#[derive(Clone, Copy, PartialEq)]
enum State {
    /// Initial state
    Init,

    /// Final state
    Final,
}

/// Raw Key Import command
pub(crate) struct RawKeyImportCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Session
    session: E::UserSession,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Decoded request data
    decoded_req: Option<DdiRawKeyImportCmdReq>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,
}

/// Implement the HSM command trait for the RawKeyImportCmd
impl<E: HsmEnvTrait> HsmCmdTrait<E> for RawKeyImportCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (State::Init, _) => Err(HsmErr::InvalidEvent),
            (State::Final, _) => Err(HsmErr::InvalidState),
        }
    }

    /// Get the session ID
    fn session_id(&self) -> Option<u16> {
        Some(self.session.id())
    }
}

impl<E: HsmEnvTrait> RawKeyImportCmd<E> {
    /// Create a new command FSM
    pub fn new(req: DmaBuffer<E>, heap: DmaHeap<E>, session: E::UserSession) -> Self {
        Self {
            state: State::Init,
            heap,
            session,
            req,
            decoded_req: None,
            resp: None,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, _tag: TagId) -> Result<(), HsmErr> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;

        self.decoded_req = Some(decode_buf::<DdiRawKeyImportCmdReq, E>(&self.req)?);
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;

        let key_id = self.session.import_raw_key(
            decoded_req.data.key_kind,
            decoded_req
                .data
                .key_properties
                .clone()
                .try_into()
                .map_err(|_| HsmErr::InvalidPermissions)?,
            decoded_req.data.key_tag,
            decoded_req.data.raw.as_slice(),
        )?;

        // Encode and save the response buffer
        self.resp = self.generate_response_with_mk(
            decoded_req.hdr.rev,
            decoded_req.hdr.sess_id,
            key_id,
            decoded_req.data.key_properties.key_label.as_slice(),
        )?;

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(
        &self,
        rev: Option<DdiApiRev>,
        sess_id: Option<u16>,
        key_id: u16,
        masked_key_len: usize,
    ) -> DdiRawKeyImportCmdResp {
        DdiRawKeyImportCmdResp {
            hdr: DdiRespHdr {
                rev,
                op: DdiOp::RawKeyImport,
                sess_id,
                status: DdiStatus::Success,
                fips_approved: false,
            },
            data: DdiRawKeyImportResp {
                key_id,
                bulk_key_id: None,
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
        rev: Option<DdiApiRev>,
        sess_id: Option<u16>,
        key_id: u16,
        key_label: &[u8],
    ) -> Result<Option<DmaBuffer<E>>, HsmErr> {
        let masked_key_len = self
            .session
            .get_masked_key_len_from_vault(key_label, key_id, None)?;

        let mut resp = self.cmd_resp(rev, sess_id, key_id, masked_key_len);

        let buf = Some(encode_buf(&resp, &self.heap)?);

        self.session.mask_key_from_vault(
            key_label,
            key_id,
            None,
            resp.data.masked_key.as_mut_slice(),
        )?;

        Ok(buf)
    }
}
