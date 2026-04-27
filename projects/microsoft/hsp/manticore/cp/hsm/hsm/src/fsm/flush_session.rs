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
pub(crate) struct FlushSessionCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// Partition
    part: E::Partition,

    /// Flag indicating whether command logic has been executed
    committed: bool,

    /// AES Bulk 256 Command data
    aes_bulk256_cmd_data:
        Option<AesBulk256Cmd<<<E as env::HsmEnvTrait>::Partition as partition::HsmPartition>::Env>>,

    /// Pfn required to send AES Bulk 256 IPC to FP
    pfn: PcieFunction,

    /// Session ID
    sess_id: u16,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for FlushSessionCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        None
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
                    "[flush_session] Invalid Event, state:{:?}, event: {:?}",
                    self.state as u32,
                    u32::from(event)
                );
                Err(HsmErr::InvalidEvent)
            }
        }
    }

    /// Get the session ID
    fn session_id(&self) -> Option<u16> {
        Some(self.sess_id)
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
    fn acquire_resource(&mut self, _tag: TagId, res_id: ResId) -> HsmFsmEvent {
        match res_id {
            HsmFsmResourceId::FpIpcChannel => {
                HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
            }
            _ => unreachable!(),
        }
    }
}

impl<E: HsmEnvTrait> FlushSessionCmd<E> {
    /// Create a new command FSM
    pub fn new(sess_id: u16, part: E::Partition, pfn: PcieFunction) -> Self {
        Self {
            state: State::Init,
            part,
            committed: false,
            sess_id,
            aes_bulk256_cmd_data: None,
            pfn,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        match self
            .part
            .begin_close_user_session(tag, self.pfn, self.sess_id)
        {
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
        match self
            .part
            .begin_close_user_session(tag, self.pfn, self.sess_id)
        {
            Ok(op) => {
                self.aes_bulk256_cmd_data = Some(op);
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }
            Err(mut err) => {
                warn!(
                    "[flush_session] begin_close_user_session returned err: {:?}",
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
        let aes_bulk256_cmd_data = self
            .aes_bulk256_cmd_data
            .as_ref()
            .ok_or(HsmErr::InvalidState)?;
        self.part.end_close_user_session(aes_bulk256_cmd_data)?;

        // Command operation has succeeded; set committed flag to avoid retry
        self.committed = true;

        self.state = State::Final;

        Ok(())
    }
}
