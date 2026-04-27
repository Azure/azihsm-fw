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

pub(crate) struct SetSealedBk3Cmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// Request DMA buffer
    req_buf: DmaBuffer<E>,

    /// Response DMA buffer
    resp_buf: Option<DmaBuffer<E>>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for SetSealedBk3Cmd<E> {
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp_buf.take()
    }

    fn on_event(&mut self, event: HsmFsmEvent, _tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(),
            _ => Err(HsmErr::InvalidEvent),
        }
    }
}

impl<E: HsmEnvTrait> SetSealedBk3Cmd<E> {
    /// Creates a new `SetSealedBK3` instance.
    pub fn new(req_buf: DmaBuffer<E>, heap: DmaHeap<E>, part: E::Partition) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            req_buf,
            resp_buf: None,
        }
    }

    /// Handles the start event for the command.
    fn on_start(&mut self) -> Result<(), HsmErr> {
        // Initialize the command state
        self.state = State::Final;

        // Check if SetSealedBk3 has already been called.
        if self.part.get_sealed_bk3_len() != 0 {
            return Err(HsmErr::SealedBk3AlreadySet);
        }

        let decoded_req = decode_buf::<DdiSetSealedBk3CmdReq, E>(&self.req_buf)?;
        let rev = decoded_req.hdr.rev.ok_or(HsmErr::UnsupportedRevision)?;
        let input_sealed_bk3 = decoded_req.data.sealed_bk3.as_slice();

        let input_len = input_sealed_bk3.len();
        if input_len > SEALED_BK3_SIZE {
            return Err(HsmErr::SealedBk3TooLarge);
        }

        let mut sealed_bk3_addr = self.part.sealed_bk3();
        sealed_bk3_addr.slice_mut()[..input_len].copy_from_slice(input_sealed_bk3);
        self.part.set_sealed_bk3_len(input_len as u32);

        self.resp_buf = Some(encode_buf(&self.cmd_resp(Some(rev)), &self.heap)?);

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(&self, api_rev: Option<DdiApiRev>) -> DdiSetSealedBk3CmdResp {
        DdiSetSealedBk3CmdResp {
            hdr: DdiRespHdr {
                rev: api_rev,
                op: DdiOp::SetSealedBk3,
                sess_id: None,
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiSetSealedBk3Resp {},
        }
    }
}
