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

pub(crate) struct GetSealedBk3Cmd<E: HsmEnvTrait + 'static> {
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

impl<E: HsmEnvTrait> HsmCmdTrait<E> for GetSealedBk3Cmd<E> {
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

impl<E: HsmEnvTrait> GetSealedBk3Cmd<E> {
    /// Creates a new `GetSealedBK3` instance.
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

        let decoded_req = decode_buf::<DdiGetSealedBk3CmdReq, E>(&self.req_buf)?;
        let rev = decoded_req.hdr.rev.ok_or(HsmErr::UnsupportedRevision)?;

        let sealed_len = self.part.get_sealed_bk3_len();
        if sealed_len == 0 {
            return Err(HsmErr::SealedBk3NotPresent);
        }

        let sealed_len_usize = sealed_len.try_into().map_err(|_| HsmErr::InvalidArgument)?;

        let sealed_bk3_blob = self.part.sealed_bk3();
        self.resp_buf = Some(encode_buf(
            &self.cmd_resp(Some(rev), sealed_bk3_blob.slice(), sealed_len_usize),
            &self.heap,
        )?);

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(
        &self,
        api_rev: Option<DdiApiRev>,
        sealed_buf: &[u8],
        len: usize,
    ) -> DdiGetSealedBk3CmdResp {
        DdiGetSealedBk3CmdResp {
            hdr: DdiRespHdr {
                rev: api_rev,
                op: DdiOp::GetSealedBk3,
                sess_id: None,
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiGetSealedBk3Resp {
                sealed_bk3: MborByteArray::new_with_len(sealed_buf.as_ptr(), len),
            },
        }
    }
}
