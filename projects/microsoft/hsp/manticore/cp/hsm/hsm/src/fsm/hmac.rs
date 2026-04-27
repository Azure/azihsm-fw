// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

/// FSM states
#[derive(Clone, Copy)]
enum State {
    /// Initial state
    Init,

    /// Final state
    Final,
}

/// Hash based message authntication code (HMAC) command
pub(crate) struct HmacCmd<E: HsmEnvTrait> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// App session
    session: E::UserSession,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for HmacCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, _tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(),
            (State::Init, _) => Err(HsmErr::InvalidEvent),
            (State::Final, _) => Err(HsmErr::InvalidState),
        }
    }

    /// Get the session ID
    fn session_id(&self) -> Option<u16> {
        Some(self.session.id())
    }
}

impl<E: HsmEnvTrait> HmacCmd<E> {
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
        }
    }

    /// Handle the start event
    fn on_start(&mut self) -> Result<(), HsmErr> {
        // FSM can be called only once
        self.state = State::Final;

        // Decode the request from DMA buffer
        let req = decode_buf::<DdiHmacCmdReq, E>(&self.req)?;
        let msg = &req.data.msg;
        let key_id = req.data.key_id;
        let key_kind = self.session.get_key_kind(key_id)?;
        let output_len = match key_kind {
            EntryKind::HmacSha256 | EntryKind::VarLenHmacSha256 => ShaType::Sha256 as usize,
            EntryKind::HmacSha384 | EntryKind::VarLenHmacSha384 => ShaType::Sha384 as usize,
            EntryKind::HmacSha512 | EntryKind::VarLenHmacSha512 => ShaType::Sha512 as usize,
            _ => Err(HsmErr::InvalidKeyType)?,
        };

        let resp = self.cmd_resp(self.session.api_rev(), self.session.id(), output_len);

        // Encode and save the buffer
        let resp_buf = encode_buf(&resp, &self.heap)?;

        if key_kind.is_var_hmac_key() {
            // Call Hmac
            self.session
                .var_hmac(key_id, msg.as_slice(), &mut (&resp.data.tag).into())?;
        } else {
            // Call Hmac
            self.session
                .hmac(key_id, msg.as_slice(), &mut (&resp.data.tag).into())?;
        }

        // Encode and save the buffer
        self.resp = Some(resp_buf);

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(&self, rev: DdiApiRev, session_id: u16, size: usize) -> DdiHmacCmdResp {
        DdiHmacCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::Hmac,
                sess_id: Some(session_id),
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiHmacResp {
                tag: MborByteArray::new_with_len(core::ptr::null(), size),
            },
        }
    }
}
