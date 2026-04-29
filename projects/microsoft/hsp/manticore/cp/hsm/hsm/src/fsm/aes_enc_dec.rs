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
pub(crate) struct AesEncDecCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// User Session
    session: E::UserSession,

    /// Request DMA buffer
    req_buf: DmaBuffer<E>,

    /// Response DMA buffer
    resp_buf: Option<DmaBuffer<E>>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for AesEncDecCmd<E> {
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

impl<E: HsmEnvTrait> AesEncDecCmd<E> {
    /// Create a new command FSM
    pub fn new(
        req_buf: DmaBuffer<E>,
        heap: DmaHeap<E>,
        session: E::UserSession,
        part: E::Partition,
    ) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            session,
            req_buf,
            resp_buf: None,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // FSM can be called only once
        self.state = State::Final;

        let req = decode_buf::<DdiAesEncryptDecryptCmdReq, E>(&self.req_buf)?;
        let mut resp = self.cmd_resp(
            self.session.api_rev(),
            self.session.id(),
            req.data.msg.len(),
        );
        let resp_buf = encode_buf(&resp, &self.heap)?;

        let iv = &(&req.data.iv).into();
        let msg_in = &(&req.data.msg).into();
        let msg_out = &(&resp.data.msg).into();

        let input = AesEncDecIn::new(
            AesEncDecMode::Cbc,
            req.data.op.try_into()?,
            Some(iv),
            msg_in,
            msg_out,
        );

        self.session
            .aes_enc_dec(tag, AesKeyIn::KeyId(req.data.key_id), &input)?;

        // Copy updated IV to resp since AES engine updates the input IV
        resp.data
            .iv
            .as_mut_slice()
            .copy_from_slice(req.data.iv.as_slice());

        self.resp_buf = Some(resp_buf);
        Ok(())
    }

    /// Create a command response
    fn cmd_resp(
        &self,
        rev: DdiApiRev,
        sess_id: u16,
        msg_len: usize,
    ) -> DdiAesEncryptDecryptCmdResp {
        DdiAesEncryptDecryptCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::AesEncryptDecrypt,
                sess_id: Some(sess_id),
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiAesEncryptDecryptResp {
                msg: MborByteArray::new_with_len(core::ptr::null(), msg_len),
                iv: MborByteArray::new_with_len(core::ptr::null(), 16), // IV is always 16 bytes
            },
        }
    }
}
