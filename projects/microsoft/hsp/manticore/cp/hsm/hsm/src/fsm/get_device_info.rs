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
pub(crate) struct GetDeviceInfoCmd<E: HsmEnvTrait> {
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

impl<E: HsmEnvTrait> HsmCmdTrait<E> for GetDeviceInfoCmd<E> {
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

impl<E: HsmEnvTrait> GetDeviceInfoCmd<E> {
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
        let req = decode_buf::<DdiGetDeviceInfoCmdReq, E>(&self.req)?;

        // Encode and save the buffer
        self.resp = Some(encode_buf(&self.cmd_resp(req.hdr.rev), &self.heap)?);

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(&self, rev: Option<DdiApiRev>) -> DdiGetDeviceInfoCmdResp {
        DdiGetDeviceInfoCmdResp {
            hdr: DdiRespHdr {
                rev,
                op: DdiOp::GetDeviceInfo,
                sess_id: None,
                status: DdiStatus::Success,
                fips_approved: false,
            },
            data: DdiGetDeviceInfoResp {
                kind: DdiDeviceKind::Physical,
                tables: self.part.resource_mask().count_ones() as u8,
                fips_approved: self.part.is_fips_approved(),
            },
        }
    }
}
