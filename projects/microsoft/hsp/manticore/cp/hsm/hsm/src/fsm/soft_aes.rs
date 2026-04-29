// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

/// FSM states
#[derive(Clone, Copy)]
enum State {
    /// Initial state
    Init,

    /// Wait for SoftAes operation
    WaitForCmd,

    /// Final state
    Final,
}

/// Change manager credential command
pub(crate) struct SoftAesCmd<E: HsmEnvTrait + 'static> {
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

impl<E: HsmEnvTrait> HsmCmdTrait<E> for SoftAesCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp_buf.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            #[cfg(feature = "fips_validation_hooks")]
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            #[cfg(feature = "fips_validation_hooks")]
            (State::WaitForCmd, HsmFsmEvent::SoftAesResp) => self.on_cmd_complete(tag),
            #[cfg(feature = "fips_validation_hooks")]
            (State::Final, _) => Err(HsmErr::InvalidState),
            (_, _) => Err(HsmErr::InvalidEvent),
        }
    }

    /// Get the session ID
    fn session_id(&self) -> Option<u16> {
        Some(self.session.id())
    }
}

impl<E: HsmEnvTrait> SoftAesCmd<E> {
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
    fn on_start(&mut self, tag: TagId) -> HsmResult<()> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;

        let mut req = decode_buf::<DdiSoftAesCmdReq, E>(&self.req_buf)?;
        let key = req.data.key.as_slice();
        let inout = req.data.inout.as_mut_slice();
        let op = SoftAesOp::from(req.data.op);

        self.state = State::WaitForCmd;

        self.session.begin_soft_aes(tag, key, inout, op)
    }

    fn on_cmd_complete(&mut self, tag: TagId) -> Result<(), HsmErr> {
        self.state = State::Final;

        let req = decode_buf::<DdiSoftAesCmdReq, E>(&self.req_buf)?;

        let range = self.session.end_soft_aes(tag)?;

        let inout = req.data.inout.as_slice();

        if inout.get(range.clone()).is_none() {
            Err(HsmErr::SoftAesInvalidResp)?;
        }

        let plaintext = &req.data.inout.as_slice()[range];
        let plaintext_mbor = MborByteArray::new_with_len(plaintext.as_ptr(), plaintext.len());

        // Response
        let resp = self.cmd_resp(req.hdr.rev, req.hdr.sess_id, plaintext_mbor);
        self.resp_buf = Some(encode_buf(&resp, &self.heap)?);

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(
        &self,
        rev: Option<DdiApiRev>,
        sess_id: Option<u16>,
        plaintext: MborByteArray<1024>,
    ) -> DdiSoftAesCmdResp {
        DdiSoftAesCmdResp {
            hdr: DdiRespHdr {
                rev: rev,
                op: DdiOp::SoftAes,
                sess_id: sess_id,
                status: DdiStatus::Success,
                fips_approved: false,
            },
            data: DdiSoftAesResp { plaintext },
        }
    }
}
