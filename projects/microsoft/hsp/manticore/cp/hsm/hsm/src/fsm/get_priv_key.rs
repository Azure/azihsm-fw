// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

/// FSM states
#[derive(Clone, Copy)]
#[allow(dead_code)]
enum State {
    /// Initial state
    Init,

    /// Final state
    Final,
}

/// Get Private key command
pub(crate) struct GetPrivKeyCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Session
    session: E::UserSession,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Response DMA buffer
    resp_buf: Option<DmaBuffer<E>>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for GetPrivKeyCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp_buf.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        let result = match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_trigger(tag),
            (State::Final, _) => Err(HsmErr::InvalidState),
            (_, _) => Err(HsmErr::InvalidEvent),
        };

        result
    }

    /// Get the session ID this command FSM operates on-behalf of
    fn session_id(&self) -> Option<u16> {
        Some(self.session.id())
    }

    /// Check if the command requires resource
    fn requires_resource(&self, _tag: TagId, _res_id: ResId) -> bool {
        false
    }
}

impl<E: HsmEnvTrait> GetPrivKeyCmd<E> {
    /// Create a new command FSM
    pub fn new(req: DmaBuffer<E>, heap: DmaHeap<E>, session: E::UserSession) -> Self {
        Self {
            state: State::Init,
            heap,
            session,
            req,
            resp_buf: None,
        }
    }

    /// Handle the event
    fn on_trigger(&mut self, _tag: TagId) -> Result<(), HsmErr> {
        // Set the state to Final early to avoid repeated calls on failure
        self.state = State::Final;

        let req = decode_buf::<DdiGetPrivKeyCmdReq, E>(&self.req)?;
        let data = &req.data;
        let key_kind = self.session.get_key_kind(data.key_id)?;
        let key_length = self.key_length(data.key_id, key_kind)?;

        let resp = self.cmd_resp(
            Some(self.session.api_rev()),
            Some(self.session.id()),
            key_kind.try_into()?,
            key_length,
        );

        self.resp_buf = Some(encode_buf(&resp, &self.heap)?);

        self.session
            .get_priv_key(data.key_id, &mut (&resp.data.key_data).into())?;

        Ok(())
    }

    /// Get the key length based on the key kind
    fn key_length(&self, key_id: KeyId, key_kind: EntryKind) -> HsmResult<usize> {
        if key_kind.is_bulk_key() {
            // Adjust length for bulk keys, as raw_key_blob_size() gives the key ID length (2 bytes)
            Ok(AesKeyKind::max_len())
        } else {
            Ok(self.session.get_key_length(key_id)? as usize)
        }
    }

    /// Create a command response
    fn cmd_resp(
        &self,
        rev: Option<DdiApiRev>,
        sess_id: Option<u16>,
        ddi_key_type: DdiKeyType,
        length: usize,
    ) -> DdiGetPrivKeyCmdResp {
        DdiGetPrivKeyCmdResp {
            hdr: DdiRespHdr {
                rev,
                op: DdiOp::GetPrivKey,
                sess_id,
                status: DdiStatus::Success,
                fips_approved: false,
            },
            data: DdiGetPrivKeyResp {
                key_kind: ddi_key_type,
                key_data: MborByteArray::new_with_len(core::ptr::null(), length),
            },
        }
    }
}
