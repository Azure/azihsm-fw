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

pub(crate) struct InitBk3Cmd<E: HsmEnvTrait + 'static> {
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

    /// Response
    resp: Option<DdiInitBk3CmdResp>,

    /// API rev
    api_rev: Option<DdiApiRev>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for InitBk3Cmd<E> {
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

impl<E: HsmEnvTrait> InitBk3Cmd<E> {
    /// Creates a new `InitBk3Cmd` instance.
    pub fn new(req_buf: DmaBuffer<E>, heap: DmaHeap<E>, part: E::Partition) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            req_buf,
            resp_buf: None,
            resp: None,
            api_rev: None,
        }
    }

    /// Handles the start event for the command.
    fn on_start(&mut self) -> Result<(), HsmErr> {
        self.state = State::Final;

        // If InitBk3 has already been called, return failure immediately.
        if self.part.get_masked_bk_boot_len() != 0 {
            return Err(HsmErr::Bk3AlreadyInitialized);
        }

        // 0. Get the request buffer.
        let decoded_req = decode_buf::<DdiInitBk3CmdReq, E>(&self.req_buf)?;
        let rev = decoded_req.hdr.rev.ok_or(HsmErr::UnsupportedRevision)?;
        self.api_rev = Some(rev);

        let mut bk_boot = self
            .heap
            .allocate(BK_AES_CBC_256_HMAC384_SIZE_BYTES)
            .ok_or(HsmErr::DmaAllocFailure)?;

        // 1. Generate BK boot key.
        self.part.generate_bk_boot(bk_boot.as_ref_mut())?;

        let mut masked_bk3_len = 0;
        let err = self.part.mask_bk3(
            decoded_req.data.bk3.as_slice(),
            bk_boot.as_ref(),
            &mut masked_bk3_len,
            &mut [],
        );
        if err != Err(HsmErr::InsufficientBuffer) {
            err?;
        }

        // Prepare the response with the known length first for zero-copy.
        self.prepare_response(masked_bk3_len)?;

        let resp = self.resp.as_mut().ok_or(HsmErr::InvalidState)?;
        let mut masked_bk3: IoMemRange = (&resp.data.masked_bk3).into();

        self.part.mask_bk3(
            decoded_req.data.bk3.as_slice(),
            bk_boot.as_ref(),
            &mut masked_bk3_len,
            masked_bk3.slice_mut(),
        )?;

        // 3.(a) Mask the BK boot with FW key.
        let mut masked_bk_boot_len = MASKED_BK_BOOT_SIZE;
        let mut masked_bk_boot = self
            .heap
            .allocate(MASKED_BK_BOOT_SIZE)
            .ok_or(HsmErr::DmaAllocFailure)?;

        self.part.mask_bk_boot(
            bk_boot.as_ref(),
            &mut masked_bk_boot_len,
            masked_bk_boot.as_ref_mut(),
        )?;

        // 3.(b) Store Masked BK Boot in the persistent store
        self.part.masked_bk_boot().slice_mut()[..masked_bk_boot_len]
            .copy_from_slice(&masked_bk_boot.as_ref()[..masked_bk_boot_len]);

        self.part.set_masked_bk_boot_len(
            masked_bk_boot_len
                .try_into()
                .map_err(|_| HsmErr::InvalidArgument)?,
        );

        Ok(())
    }

    /// Prepare the response buffer.
    fn prepare_response(&mut self, masked_bk3_len: usize) -> Result<(), HsmErr> {
        let resp = self.cmd_resp(masked_bk3_len);
        self.resp_buf = Some(encode_buf(&resp, &self.heap)?);
        self.resp = Some(resp);

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(&self, masked_key_len: usize) -> DdiInitBk3CmdResp {
        DdiInitBk3CmdResp {
            hdr: DdiRespHdr {
                rev: self.api_rev,
                op: DdiOp::InitBk3,
                sess_id: None,
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiInitBk3Resp {
                masked_bk3: MborByteArray::new_with_len(core::ptr::null(), masked_key_len),
                vm_launch_guid: self.part.vm_launch_guid(),
            },
        }
    }
}
