// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

/// FSM states
#[derive(Clone, Copy)]
enum State {
    /// Initial state
    Init,

    /// Waiting for CP/FP IPC Channel
    WaitForResource,

    /// Wait for CP/FP IPC operation
    WaitForCmd,

    /// Final state
    Final,
}

/// Close manager session command
pub(crate) struct CloseSessionCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Cmd struct to avoid multiple decode operations on request DMA buffer
    decoded_req: Option<DdiCloseSessionCmdReq>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,

    /// Flag indicating whether command logic has been executed
    committed: bool,

    /// AES Bulk 256 Command data
    aes_bulk256_cmd_data:
        Option<AesBulk256Cmd<<<E as env::HsmEnvTrait>::Partition as partition::HsmPartition>::Env>>,

    /// Pfn required to send AES Bulk 256 IPC to FP
    pfn: PcieFunction,

    /// Session ID
    sess_id: Option<u16>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for CloseSessionCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (State::WaitForResource, HsmFsmEvent::ResourceReady(_res)) => self.on_engine_ready(tag),
            (State::WaitForCmd, HsmFsmEvent::FpToHsmIpcResponse) => self.on_cmd_complete(),
            (State::Final, _) => Err(HsmErr::InvalidState),
            (_, _) => {
                error!(
                    "[close_session] Invalid Event, state:{:?}, event: {:?}",
                    self.state as u32,
                    u32::from(event)
                );
                Err(HsmErr::InvalidEvent)
            }
        }
    }

    /// Get the session ID
    fn session_id(&self) -> Option<u16> {
        self.sess_id
    }

    /// Check if the command needs to be retried
    fn retry(&self) -> bool {
        !self.committed
    }

    /// Check if the command requires resource
    fn requires_resource(&self, _tag: TagId, _res_id: ResId) -> bool {
        true
    }

    /// Acquire a resource
    fn acquire_resource(&mut self, _tag: TagId, _res_id: ResId) -> HsmFsmEvent {
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
    }
}

impl<E: HsmEnvTrait> CloseSessionCmd<E> {
    /// Create a new command FSM
    pub fn new(req: DmaBuffer<E>, heap: DmaHeap<E>, part: E::Partition, pfn: PcieFunction) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            req,
            resp: None,
            committed: false,
            sess_id: None,
            decoded_req: None,
            aes_bulk256_cmd_data: None,
            pfn,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // Decode the request
        self.decoded_req = Some(decode_buf::<DdiCloseSessionCmdReq, E>(&self.req)?);

        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;

        let Some(sess_id) = decoded_req.hdr.sess_id else {
            return Err(HsmErr::SessionExpected);
        };

        self.sess_id = Some(sess_id);

        if self.part.needs_renegotiation(sess_id) {
            // No need to renegotiate, delete the session directly
            self.part.delete_user_session(sess_id);

            // Command operation has succeeded; set committed flag to avoid retry
            self.committed = true;

            // Encode and save the buffer
            self.resp = Some(encode_buf(
                &self.cmd_resp(decoded_req.hdr.rev, decoded_req.hdr.sess_id),
                &self.heap,
            )?);

            self.state = State::Final;

            return Ok(());
        }

        match self.part.begin_close_user_session(tag, self.pfn, sess_id) {
            Ok(op) => {
                self.aes_bulk256_cmd_data = Some(op);
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }
            Err(err) => {
                if err.pending() {
                    self.state = State::WaitForResource;
                } else {
                    self.state = State::Final;
                }
                Err(err)
            }
        }
    }

    /// Handle the FP IPC ready event
    fn on_engine_ready(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let sess_id = self.sess_id.ok_or(HsmErr::InvalidState)?;

        match self.part.begin_close_user_session(tag, self.pfn, sess_id) {
            Ok(op) => {
                self.aes_bulk256_cmd_data = Some(op);
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }
            Err(mut err) => {
                warn!(
                    "[close_session] begin_close_user_session err: {:?}",
                    u32::from(err)
                );
                if err.pending() {
                    err = HsmErr::InvalidState;
                }
                self.state = State::Final;
                Err(err)
            }
        }
    }

    /// Handle the FP IPC response event
    fn on_cmd_complete(&mut self) -> Result<(), HsmErr> {
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;
        let aes_bulk256_cmd_data = self
            .aes_bulk256_cmd_data
            .as_ref()
            .ok_or(HsmErr::InvalidState)?;
        self.part.end_close_user_session(aes_bulk256_cmd_data)?;

        // Command operation has succeeded; set committed flag to avoid retry
        self.committed = true;

        // Encode and save the buffer
        self.resp = Some(encode_buf(
            &self.cmd_resp(decoded_req.hdr.rev, decoded_req.hdr.sess_id),
            &self.heap,
        )?);

        self.state = State::Final;

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(&self, rev: Option<DdiApiRev>, sess_id: Option<u16>) -> DdiCloseSessionCmdResp {
        DdiCloseSessionCmdResp {
            hdr: DdiRespHdr {
                rev,
                op: DdiOp::CloseSession,
                sess_id,
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiCloseSessionResp {},
        }
    }
}
