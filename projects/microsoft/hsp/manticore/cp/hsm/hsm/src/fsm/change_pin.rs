// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

/// FSM states
#[derive(Clone, Copy)]
enum State {
    /// Initial state
    Init,

    /// Waiting for PKA Engine
    WaitForEngine,

    /// Wait for first operation
    WaitForMontgomeryConstCalc,

    /// Wait for second operation
    WaitForPointMultiplication,

    /// Final state
    Final,
}

/// Change Pin command
pub(crate) struct ChangePinCmd<E: HsmEnvTrait + 'static> {
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
    req: Option<DdiChangePinCmdReq>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,

    /// Command context
    cmd_ctx: Option<ChangePinCmdCtx<E>>,

    /// Flag indicating whether command logic has been executed
    committed: bool,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for ChangePinCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (State::WaitForEngine, HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)) => {
                self.on_engine_ready(tag)
            }
            (State::WaitForMontgomeryConstCalc, HsmFsmEvent::PkaDone(_))
            | (State::WaitForMontgomeryConstCalc, HsmFsmEvent::PkaError(_)) => {
                self.on_continue(tag)
            }
            (State::WaitForPointMultiplication, HsmFsmEvent::PkaDone(_))
            | (State::WaitForPointMultiplication, HsmFsmEvent::PkaError(_)) => self.on_end(tag),
            (State::Final, _) => Err(HsmErr::InvalidState),
            (_, _) => Err(HsmErr::InvalidEvent),
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

    /// Check if the command needs to be retried
    fn retry(&self) -> bool {
        !self.committed
    }
}

impl<E: HsmEnvTrait> ChangePinCmd<E> {
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
            cmd_ctx: None,
            committed: false,
        }
    }

    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        self.req = Some(decode_buf::<DdiChangePinCmdReq, E>(&self.req_buf)?);

        match self.session.begin_change_pin(tag) {
            Ok(ctx) => {
                self.cmd_ctx = Some(ctx);
                self.state = State::WaitForMontgomeryConstCalc;
                Err(HsmErr::Pending)
            }
            Err(err) => {
                if err.pending() {
                    self.state = State::WaitForEngine;
                } else {
                    self.state = State::Final;
                }
                Err(err)
            }
        }
    }

    fn on_engine_ready(&mut self, tag: TagId) -> Result<(), HsmErr> {
        match self.session.begin_change_pin(tag) {
            Ok(ctx) => {
                self.cmd_ctx = Some(ctx);
                self.state = State::WaitForMontgomeryConstCalc;
                Err(HsmErr::Pending)
            }
            Err(mut err) => {
                if err.pending() {
                    // Pending error unexpected here
                    err = HsmErr::InvalidState;
                }
                self.state = State::Final;
                Err(err)
            }
        }
    }

    fn on_continue(&mut self, _tag: TagId) -> Result<(), HsmErr> {
        let req = self.req.as_ref().ok_or(HsmErr::InvalidState)?;
        let pub_key_der = &req.data.pub_key.der;
        let ctx = self.cmd_ctx.take().ok_or(HsmErr::InvalidState)?;

        match self.session.continue_change_pin(ctx, &(pub_key_der).into()) {
            Ok(ctx) => {
                self.cmd_ctx = Some(ctx);
                self.state = State::WaitForPointMultiplication;
                Err(HsmErr::Pending)
            }
            Err(mut err) => {
                if err.pending() {
                    // Pending error unexpected here
                    err = HsmErr::InvalidState;
                }
                self.state = State::Final;
                Err(err)
            }
        }
    }

    fn on_end(&mut self, _tag: TagId) -> Result<(), HsmErr> {
        let req = self.req.as_ref().ok_or(HsmErr::InvalidState)?;
        let ctx = self.cmd_ctx.take().ok_or(HsmErr::InvalidState)?;

        match self.session.end_change_pin(ctx, &req.data.new_pin) {
            Ok(_) => {
                // Command operation has succeeded; set committed flag to avoid retry
                self.committed = true;

                // Encode and save the buffer
                self.resp = Some(encode_buf(
                    &self.cmd_resp(self.session.api_rev(), self.session.id()),
                    &self.heap,
                )?);

                Ok(())
            }
            Err(mut err) => {
                if err.pending() {
                    // Pending error unexpected here
                    err = HsmErr::InvalidState;
                }
                self.state = State::Final;
                Err(err)
            }
        }
    }

    /// Create a command response
    fn cmd_resp(&self, rev: DdiApiRev, sess_id: u16) -> DdiChangePinCmdResp {
        DdiChangePinCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::ChangePin,
                sess_id: Some(sess_id),
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiChangePinResp {},
        }
    }
}
