// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

/// Max Random number length in Bytes
const MAX_RNG_NUMBER_SIZE: usize = 64;

/// FSM states
#[derive(Clone, Copy)]
enum State {
    /// Initial state
    Init,

    /// Final state
    Final,
}

/// Change manager credential command
pub(crate) struct GetRngCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    session: E::UserSession,

    /// Request DMA buffer
    req_buf: DmaBuffer<E>,

    /// Response DMA buffer
    resp_buf: Option<DmaBuffer<E>>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for GetRngCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp_buf.take()
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

impl<E: HsmEnvTrait> GetRngCmd<E> {
    /// Create a new command FSM
    pub fn new(req_buf: DmaBuffer<E>, heap: DmaHeap<E>, session: E::UserSession) -> Self {
        Self {
            state: State::Init,
            heap,
            session,
            req_buf,
            resp_buf: None,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, _tag: TagId) -> Result<(), HsmErr> {
        // FSM can be called only once
        self.state = State::Final;

        let req = decode_buf::<DdiGetRngGenerateCmdReq, E>(&self.req_buf)?;

        // Verify if requested length is within max limit
        if req.data.rng_len > MAX_RNG_NUMBER_SIZE as u8 {
            Err(HsmErr::InvalidArgument)?;
        }

        let resp = self.cmd_resp(self.session.api_rev(), self.session.id(), req.data.rng_len);
        let resp_buf = encode_buf(&resp, &self.heap)?;

        self.session
            .get_random_number(&mut (&(resp.data.rng_number)).into())?;

        self.resp_buf = Some(resp_buf);

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(&self, rev: DdiApiRev, sess_id: u16, rng_len: u8) -> DdiGetRngGenerateCmdResp {
        DdiGetRngGenerateCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::GetRandomNumber,
                sess_id: Some(sess_id),
                status: DdiStatus::Success,
                fips_approved: false,
            },
            data: DdiGetRngGenerateResp {
                rng_number: MborByteArray::new_with_len(core::ptr::null(), rng_len as usize),
            },
        }
    }
}
