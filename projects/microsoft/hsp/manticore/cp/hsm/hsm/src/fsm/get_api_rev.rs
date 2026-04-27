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

/// Get API revision command
pub(crate) struct GetApiRevCmd<E: HsmEnvTrait> {
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
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for GetApiRevCmd<E> {
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
}

impl<E: HsmEnvTrait> GetApiRevCmd<E> {
    /// Create a new command FSM
    pub fn new(req: DmaBuffer<E>, heap: DmaHeap<E>, part: E::Partition) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            req,
            resp: None,
        }
    }

    /// Handle the start event
    fn on_start(&mut self) -> Result<(), HsmErr> {
        // FSM can be called only once
        self.state = State::Final;

        // Force decode the header and body to ensure body is empty
        let _ = decode_buf::<DdiGetApiRevCmdReq, E>(&self.req)?;

        // Encode and save the buffer
        self.resp = Some(encode_buf(&self.cmd_resp(), &self.heap)?);

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(&self) -> DdiGetApiRevCmdResp {
        DdiGetApiRevCmdResp {
            hdr: DdiRespHdr {
                rev: None,
                op: DdiOp::GetApiRev,
                sess_id: None,
                status: DdiStatus::Success,
                fips_approved: false,
            },
            data: DdiGetApiRevResp {
                min: self.part.min_api_rev(),
                max: self.part.max_api_rev(),
            },
        }
    }
}
