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

/// RSA Modular Exponentiation command
pub(crate) struct RsaModExpCmd<E: HsmEnvTrait + 'static> {
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
    req: Option<DdiRsaModExpCmdReq>,

    /// Response DMA buffer
    resp_buf: Option<DmaBuffer<E>>,

    /// Response
    resp: Option<DdiRsaModExpCmdResp>,

    /// Rsa Modular Exponentiation Operation
    op: Option<RsaModExp<E>>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for RsaModExpCmd<E> {
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
                    "[rsa_mod_exp] Invalid State, state:{:?} event: {:?}",
                    self.state as u32,
                    u32::from(event)
                );
                Err(HsmErr::InvalidState)
            }
            (_, _) => {
                error!(
                    "[rsa_mod_exp] Invalid Event, state:{:?} event: {:?}",
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

impl<E: HsmEnvTrait> RsaModExpCmd<E> {
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
            req: None,
            req_buf,
            resp: None,
            resp_buf: None,
            op: None,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let req = decode_buf::<DdiRsaModExpCmdReq, E>(&self.req_buf)?;
        let resp = self.cmd_resp(self.session.api_rev(), self.session.id(), req.data.y.len());
        self.req = Some(req);
        self.resp_buf = Some(encode_buf(&resp, &self.heap)?);
        self.resp = Some(resp);

        match self.begin_rsa_mod_exp(tag) {
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
        match self.begin_rsa_mod_exp(tag) {
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

    /// Handle the RSA modular exponentiation command done event
    fn on_cmd_complete(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let op = self.op.take().ok_or(HsmErr::InvalidState)?;

        // Complete the command and retrieve the output
        self.end_rsa_mod_exp(tag, op).inspect_err(|_err| {
            self.resp_buf = None;
        })
    }

    /// Begin RSA Modular Exponentiation.
    fn begin_rsa_mod_exp(&mut self, tag: TagId) -> HsmResult<RsaModExp<E>> {
        let req = self.req.as_ref().ok_or(HsmErr::InvalidState)?;
        let resp = self.resp.as_ref().ok_or(HsmErr::InvalidState)?;

        self.session
            .begin_rsa_mod_exp_zc(
                tag,
                req.data.key_id,
                Some(req.data.op_type.try_into()?),
                &(&req.data.y).into(),
                &(&resp.data.x).into(),
            )
            .inspect_err(|err| {
                if !err.pending() {
                    self.resp_buf = None;
                }
            })
    }

    /// End RSA Modular Exponentiation operation.
    fn end_rsa_mod_exp(&mut self, tag: TagId, op: RsaModExp<E>) -> HsmResult<()> {
        self.session.end_rsa_mod_exp_zc(tag, op)
    }

    /// Create a command response
    fn cmd_resp(&self, rev: DdiApiRev, sess_id: u16, len: usize) -> DdiRsaModExpCmdResp {
        DdiRsaModExpCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::RsaModExp,
                sess_id: Some(sess_id),
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiRsaModExpResp {
                x: MborByteArray::new_with_len(core::ptr::null(), len),
            },
        }
    }
}
