// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

/// FSM states
#[derive(Clone, Copy)]
enum State {
    /// Initial state
    Init,

    /// Waiting for PKA Engine
    WaitForEngine,

    /// Wait for PKA operation
    WaitForCmd,

    /// Final state
    Final,
}

/// ECC Sign command
pub(crate) struct EccSignCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// App Session
    session: E::UserSession,

    /// Request DMA buffer
    req_buf: DmaBuffer<E>,

    /// Request
    req: Option<DdiEccSignCmdReq>,

    /// Response DMA buffer
    resp_buf: Option<DmaBuffer<E>>,

    /// Response
    resp: Option<DdiEccSignCmdResp>,

    /// ECC Sign Operation
    op: Option<EccSign<E>>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for EccSignCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp_buf.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (State::WaitForEngine, HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)) => {
                self.on_engine_ready(tag)
            }
            (State::WaitForCmd, HsmFsmEvent::PkaDone(_))
            | (State::WaitForCmd, HsmFsmEvent::PkaError(_)) => self.on_cmd_complete(tag),
            (State::Final, _) => {
                error!(
                    "[ecc_sign] Invalid State, state:{:?} event: {:?}",
                    self.state as u32,
                    u32::from(event)
                );
                Err(HsmErr::InvalidState)
            }
            (_, _) => {
                error!(
                    "[ecc_sign] Invalid Event, state:{:?} event: {:?}",
                    self.state as u32,
                    u32::from(event)
                );
                Err(HsmErr::InvalidEvent)
            }
        }
    }

    /// Get the session ID
    fn session_id(&self) -> Option<u16> {
        Some(self.session.id())
    }

    /// Check if the command requires resource
    fn requires_resource(&self, _tag: TagId, _res_id: ResId) -> bool {
        true
    }

    /// Acquire a resource
    fn acquire_resource(&mut self, _tag: TagId, _res_id: ResId) -> HsmFsmEvent {
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    }
}

impl<E: HsmEnvTrait> EccSignCmd<E> {
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
            req: None,
            resp_buf: None,
            resp: None,
            op: None,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let req = decode_buf::<DdiEccSignCmdReq, E>(&self.req_buf)?;
        let resp = self.cmd_resp(
            self.session.api_rev(),
            self.session.id(),
            self.sig_len(req.data.key_id)?,
        );
        self.req = Some(req);
        self.resp_buf = Some(encode_buf(&resp, &self.heap)?);
        self.resp = Some(resp);

        match self.begin_ecc_sign(tag) {
            Ok(op) => {
                self.op = Some(op);
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }

            Err(err) => {
                if err.pending() {
                    self.state = State::WaitForEngine;
                } else {
                    self.resp_buf = None;
                    self.state = State::Final;
                }
                Err(err)
            }
        }
    }

    /// Handle the PKA Engine ready event
    fn on_engine_ready(&mut self, tag: TagId) -> Result<(), HsmErr> {
        match self.begin_ecc_sign(tag) {
            Ok(op) => {
                self.op = Some(op);
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }
            Err(mut err) => {
                if err.pending() {
                    err = HsmErr::InvalidState;
                }
                self.resp_buf = None;
                self.state = State::Final;
                Err(err)
            }
        }
    }

    /// Handle the ECC command done event
    fn on_cmd_complete(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let op = self.op.take().ok_or(HsmErr::InvalidState)?;

        // Complete the command and retrieve the output
        self.end_ecc_sign(tag, op).inspect_err(|_err| {
            self.resp_buf = None;
        })
    }

    /// Begin ECC sign.
    fn begin_ecc_sign(&mut self, tag: TagId) -> HsmResult<EccSign<E>> {
        // Decode the request

        let req = self.req.as_ref().ok_or(HsmErr::InvalidState)?;
        let resp = self.resp.as_ref().ok_or(HsmErr::InvalidState)?;

        self.session
            .begin_ecc_sign_zc(
                tag,
                EccKeyIn::KeyId(req.data.key_id),
                &(&req.data.digest).into(),
                req.data.digest_algo,
                &(&resp.data.signature).into(),
            )
            .inspect_err(|err| {
                if !err.pending() {
                    self.resp_buf = None;
                }
            })
    }

    /// End ECC sign operation.
    fn end_ecc_sign(&mut self, tag: TagId, op: EccSign<E>) -> HsmResult<()> {
        self.session.end_ecc_sign_zc(tag, op)
    }

    /// Create a command response
    fn cmd_resp(&self, rev: DdiApiRev, sess_id: u16, sig_len: usize) -> DdiEccSignCmdResp {
        DdiEccSignCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::EccSign,
                sess_id: Some(sess_id),
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiEccSignResp {
                signature: MborByteArray::new_with_len(core::ptr::null(), sig_len),
            },
        }
    }

    /// Calculate signature len
    fn sig_len(&self, key_id: KeyId) -> HsmResult<usize> {
        match self.session.get_key_kind(key_id)? {
            EntryKind::Ecc256Private => Ok(32 * 2),
            EntryKind::Ecc384Private => Ok(48 * 2),
            // for ECC-521 Hardware produces components with 68 bytes instead of 66 bytes
            EntryKind::Ecc521Private => Ok(68 * 2),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}
