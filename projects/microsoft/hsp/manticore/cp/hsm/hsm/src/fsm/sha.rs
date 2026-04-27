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

/// Change manager credential command
pub(crate) struct ShaDigestCmd<E: HsmEnvTrait + 'static> {
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

impl<E: HsmEnvTrait> HsmCmdTrait<E> for ShaDigestCmd<E> {
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

impl<E: HsmEnvTrait> ShaDigestCmd<E> {
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

        let req = decode_buf::<DdiShaDigestGenerateCmdReq, E>(&self.req_buf)?;
        let sha_type: ShaType = req.data.sha_mode.try_into()?;

        // Length of digest buffer that sha hardware requires
        let hw_digest_len = sha_type.get_digest_size_hw();

        // Actual length of the digest by the Hash algorithm
        let actual_digest_len = sha_type.get_digest_size();

        // Allocate DMA buffer that SHA hardware engine can use to produce the digest data
        let digest_buffer = self
            .heap
            .allocate(hw_digest_len)
            .ok_or(HsmErr::DmaAllocFailure)?;

        let digest_buffer_mbor: MborByteArray<64> =
            MborByteArray::new_with_len(digest_buffer.as_ref().as_ptr(), hw_digest_len);

        self.session.sha_single_block_zc(
            sha_type,
            &(&req.data.msg).into(),
            &mut (&digest_buffer_mbor).into(),
        )?;

        let mut resp = self.cmd_resp(self.session.api_rev(), self.session.id(), actual_digest_len);
        let resp_buf = encode_buf(&resp, &self.heap)?;

        resp.data
            .digest
            .as_mut_slice()
            .copy_from_slice(&digest_buffer_mbor.as_slice()[..actual_digest_len]);

        self.resp_buf = Some(resp_buf);

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(
        &self,
        rev: DdiApiRev,
        sess_id: u16,
        actual_digest_len: usize,
    ) -> DdiShaDigestGenerateCmdResp {
        DdiShaDigestGenerateCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::ShaDigest,
                sess_id: Some(sess_id),
                status: DdiStatus::Success,
                fips_approved: false,
            },
            data: DdiShaDigestGenerateResp {
                digest: MborByteArray::new_with_len(core::ptr::null(), actual_digest_len),
            },
        }
    }
}
