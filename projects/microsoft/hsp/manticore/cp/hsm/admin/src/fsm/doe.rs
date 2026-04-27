// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_doe::DoeEvents;
use mcr_doe::PcieDoeTrait;
use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_logging::*;
use mcr_pcie_controller::PcieControllerTrait;
use mcr_tcon::Tcon;
use mcr_types::DebugLogComponent;
use mcr_types::DebugLogEntryParameters;
use mcr_types::DebugLogSeverity;

use super::*;
use crate::context::AdminFsmContext;
use crate::env::AdminEnvTrait;
use crate::error::AdminErr;
use crate::recorder::AdminFsmEventRecorder;
use crate::resource::AdminFsmResourceId;
use crate::resource::DoeIdle;
use crate::resource::HspIpcChannel;
use crate::AdminFsmEvent;

const DOE_WAIT_RX_EVENT_TIME_MS: u32 = 250;
const DOE_WAIT_IPC_RESPONSE_TIME_MS: u32 = 1000;

/// DOE FSM states
#[derive(Copy, Clone, Debug, PartialEq)]
enum DoeFsmState {
    /// The very first state when FSM is created. Should never go into this state after initialization
    UnInitialized,

    /// DOE message handling FSM is unavailable to receive new request
    Standby,

    /// DOE message handling FSM is ready to receive new request
    Ready,

    /// Waiting for DoeIdle resource to be ready
    WaitDoeIdle,

    /// Wait for Rx Done indicating request has been received
    WaitRxDone,

    /// Waiting for IPC channel resource to be ready
    WaitIpcChannel,

    /// Waiting for IPC channel response *from HSP*
    WaitIpcResponse,

    /// Wait for Tx Done indicating response has been sent
    WaitTxDone,

    /// DOE-related Error state
    DoeError,

    /// A fatal error state that can't be recovered
    FatalError,
}

/// PCIe DOE FSM
pub(crate) struct DoeFsm<E: AdminEnvTrait + 'static> {
    /// Current state of the FSM
    state: DoeFsmState,

    /// Timer
    timer: CmdTimer,

    /// HSP IPC channel resource
    ipc_channel: Option<CmdResourceRef<HspIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// DoeIdle resource
    doe_idle: Option<CmdResourceRef<DoeIdle, AdminFsm<E>>>,

    /// Context
    ctx: AdminFsmContext<E>,

    /// Flag to interrupt a normal flow, indicating to move to standby state
    move_to_standby: bool,

    /// Flag to interrupt a normal flow, indicating to move to doe_error state
    move_to_doe_error: bool,

    /// Flag to interrupt a normal flow, indicating to move to idle state
    move_to_idle: bool,

    /// Flag that indicates doe process is ongoing, need to abort/reset in case DoeAbort/FLR happens
    doe_msg_active: bool,
}

impl<E: AdminEnvTrait> CmdFsm for DoeFsm<E> {
    type Error = AdminErr;
    type ResourceId = AdminFsmResourceId;
    type Event = AdminFsmEvent;
    type Recorder = AdminFsmEventRecorder;

    /// Get the timer for the hierarchical FSM
    fn get_timer(&mut self) -> Option<&mut CmdTimer> {
        Some(&mut self.timer)
    }

    /// Handle an event
    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match (self.state, event) {
            (DoeFsmState::UnInitialized, AdminFsmEvent::DoeFsmInit) => self.on_init_event(tag),
            (DoeFsmState::Standby, AdminFsmEvent::PciePerstUp) => self.on_perst_up(tag),
            (DoeFsmState::Standby, _) => Err(AdminErr::Pending),
            (
                DoeFsmState::WaitDoeIdle,
                AdminFsmEvent::ResourceReady(AdminFsmResourceId::DoeIdle),
            ) => self.on_doe_idle(tag),
            (DoeFsmState::Ready, AdminFsmEvent::Doe(DoeEvents::RxReady))
            | (DoeFsmState::WaitRxDone, AdminFsmEvent::Doe(DoeEvents::RxReady)) => {
                self.on_rx_ready()
            }
            (DoeFsmState::Ready, AdminFsmEvent::TimerElapsed) => self.on_rx_ready_timeout(tag),
            (DoeFsmState::Ready, AdminFsmEvent::Doe(DoeEvents::DoeGo))
            | (DoeFsmState::WaitRxDone, AdminFsmEvent::Doe(DoeEvents::DoeGo)) => {
                self.on_doe_go(tag)
            }
            (
                DoeFsmState::WaitIpcChannel,
                AdminFsmEvent::ResourceReady(AdminFsmResourceId::HspIpcChannel),
            ) => self.send_ipc_request(tag),
            (DoeFsmState::WaitIpcResponse, AdminFsmEvent::HspToAdminIpcResponse) => {
                self.on_ipc_response(tag)
            }
            (DoeFsmState::WaitIpcResponse, AdminFsmEvent::TimerElapsed) => {
                self.on_ipc_response_timeout()
            }
            (DoeFsmState::WaitTxDone, AdminFsmEvent::Doe(DoeEvents::TxReady)) => self.on_tx_ready(),
            (DoeFsmState::WaitTxDone, AdminFsmEvent::Doe(DoeEvents::TxDone)) => {
                self.on_tx_done(tag)
            }
            (_, AdminFsmEvent::Doe(DoeEvents::DoeAbort)) | (_, AdminFsmEvent::PcieFlr) => {
                self.on_doe_abort(tag)
            }
            (_, AdminFsmEvent::PciePerstDown) => self.on_perst_down(),
            (_, AdminFsmEvent::Doe(DoeEvents::PoisonedConfigWrite))
            | (_, AdminFsmEvent::Doe(DoeEvents::TxOverflow))
            | (_, AdminFsmEvent::Doe(DoeEvents::RxUnderflow)) => self.on_doe_err(),
            _ => Err(AdminErr::Pending),
        }
    }

    /// Acquire a resource for the FSM
    fn acquire_resource(&mut self, tag: TagId, id: Self::ResourceId) -> Self::Event {
        match id {
            AdminFsmResourceId::HspIpcChannel => {
                debug_assert!(self.ipc_channel.is_none());
                self.ipc_channel = self.ctx.admin_to_hsp_ipc_channel().acquire(tag, ());
                debug_assert!(self.ipc_channel.is_some());
                AdminFsmEvent::ResourceReady(ResId::HspIpcChannel)
            }
            AdminFsmResourceId::DoeIdle => {
                debug_assert!(self.doe_idle.is_none());
                self.doe_idle = self.ctx.doe_idle().acquire(tag, ());
                debug_assert!(self.doe_idle.is_some());
                AdminFsmEvent::ResourceReady(ResId::DoeIdle)
            }
            _ => AdminFsmEvent::Unknown,
        }
    }
}

impl<E: AdminEnvTrait> DoeFsm<E> {
    pub fn new(ctx: AdminFsmContext<E>) -> Self {
        let timer = CmdTimer::new();

        Self {
            state: DoeFsmState::UnInitialized,
            timer,
            ipc_channel: None,
            doe_idle: None,
            ctx,
            move_to_standby: false,
            move_to_doe_error: false,
            move_to_idle: false,
            doe_msg_active: false,
        }
    }

    /// Reset the FSM internal variables, release the resources
    fn reset(&mut self) {
        self.move_to_standby = false;
        self.move_to_doe_error = false;
        self.move_to_idle = false;
        self.doe_msg_active = false;

        self.doe_idle.take();
        self.ipc_channel.take();
    }

    /// Check if FSM is waiting for some resources or ipc response to come
    fn waiting_for_resource_or_ipc_response(&self) -> bool {
        self.state == DoeFsmState::WaitDoeIdle
            || self.state == DoeFsmState::WaitIpcChannel
            || self.state == DoeFsmState::WaitIpcResponse
    }

    /// Check local flags to move to correct state when resource is available
    /// Return false if it does not move to any state
    fn general_check_on_resource(&mut self, tag: TagId) -> bool {
        if self.move_to_standby {
            self.move_to_standby();
        } else if self.move_to_idle {
            self.move_to_idle(tag);
        } else if self.move_to_doe_error {
            self.move_to_doe_error();
        } else {
            return false;
        }

        true
    }

    /// Move to standby state, doe_soft_reset and reset FSM
    fn move_to_standby(&mut self) {
        if self.doe_msg_active {
            self.ctx.pcie_doe().abort();
        }

        self.reset();
        self.state = DoeFsmState::Standby;
    }

    /// Move to Idle state, doe_soft_reset + doe_hard_reset, and reset FSM
    /// Try to acquire DoeIdle
    fn move_to_idle(&mut self, tag: TagId) {
        if self.doe_msg_active {
            self.ctx.pcie_doe().abort();
            self.ctx.pcie_doe().reset();
        }

        self.reset();
        self.doe_idle = self.ctx.doe_idle().acquire(tag, ());
        if self.doe_idle.is_some() {
            self.move_to_ready();
        } else {
            self.move_to_wait_doe_idle();
        }
    }

    /// Move to WaitDoeIdle state and set busy bit
    fn move_to_wait_doe_idle(&mut self) {
        self.ctx.pcie_doe().set_busy(true);
        self.state = DoeFsmState::WaitDoeIdle;
    }

    /// Move to Ready state and clear the busy bit
    /// Start the timer for RxReady Event
    fn move_to_ready(&mut self) {
        self.ctx.pcie_doe().set_busy(false);
        self.timer
            .start(Tcon::get_approximate_ticks_from_ms(DOE_WAIT_RX_EVENT_TIME_MS) as u8);
        self.state = DoeFsmState::Ready;
    }

    /// Move to DoeError state and set error bit
    /// Reset FSM but keep track of the doe_msg_active bit
    fn move_to_doe_error(&mut self) {
        self.ctx.pcie_doe().set_err();
        let cache = self.doe_msg_active;
        self.reset();
        self.doe_msg_active = cache;
        self.state = DoeFsmState::DoeError;
    }

    /// Move to FatalError state and set error bit
    /// Reset FSM
    /// Panic
    fn move_to_fatal_error(&mut self) {
        self.ctx.pcie_doe().set_err();
        self.reset();
        self.state = DoeFsmState::FatalError;
        panic!();
    }

    /// PERST is up, move FSM to Idle state
    fn on_perst_up(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.move_to_idle(tag);

        Err(AdminErr::Pending)
    }

    /// Move to Idle state if link is already up
    fn on_init_event(&mut self, tag: TagId) -> Result<(), AdminErr> {
        if self.ctx.pcie_cntrl().link_status().is_ok() {
            self.move_to_idle(tag);
        } else {
            self.move_to_standby();
        }

        Err(AdminErr::Pending)
    }

    /// DoeIdle is available, if move_to_* flag is not set, move to Ready state
    fn on_doe_idle(&mut self, tag: TagId) -> Result<(), AdminErr> {
        if !self.general_check_on_resource(tag) {
            self.move_to_ready();
        }

        Err(AdminErr::Pending)
    }

    /// Host is sending new message to CP
    fn on_rx_ready(&mut self) -> Result<(), AdminErr> {
        if self.state == DoeFsmState::Ready {
            self.doe_msg_active = true;
            self.timer.stop();
        }

        self.ctx.pcie_doe().recv().map_err(|_err| {
            error!("[doe] on_rx_ready Failed to recv DOE message: {:?}", _err);

            self.move_to_doe_error();

            AdminErr::Pending
        })?;

        self.state = DoeFsmState::WaitRxDone;

        Err(AdminErr::Pending)
    }

    /// No RxReady event received, move back to Idle
    fn on_rx_ready_timeout(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.move_to_idle(tag);

        Err(AdminErr::Pending)
    }

    /// Host has sent DOE Go, indicating CP can read request from FIFO
    fn on_doe_go(&mut self, tag: TagId) -> Result<(), AdminErr> {
        if self.state == DoeFsmState::Ready {
            self.doe_msg_active = true;
            self.timer.stop();
        }

        self.ctx.pcie_doe().recv().map_err(|_err| {
            error!("[doe] on_doe_go: Failed to recv DOE message: {:?}", _err);
            self.move_to_doe_error();

            AdminErr::Pending
        })?;

        self.ctx.pcie_doe().end_recv().map_err(|_err| {
            error!("Failed to end recv DOE message: {:?}", _err);
            self.move_to_doe_error();

            AdminErr::Pending
        })?;

        self.try_send_ipc_request(tag)
    }

    /// No IPC response is received, mark next move to DoeError
    fn on_ipc_response_timeout(&mut self) -> Result<(), AdminErr> {
        self.move_to_doe_error = true;
        Err(AdminErr::Pending)
    }

    /// On IPC response event
    /// SP has sent DOE response to CP
    fn on_ipc_response(&mut self, tag: TagId) -> Result<(), AdminErr> {
        if let Some(message) = self.receive_ipc_message() {
            if self.general_check_on_resource(tag) {
                Err(AdminErr::Pending)?;
            }

            let response: IpcMessageDoe = IpcMessageDecoder::decode(message).map_err(|_err| {
                error!("Failed to decode IPC message: {:?}", _err);

                self.move_to_fatal_error();

                AdminErr::Pending
            })?;

            if response.header.status() != 0 {
                error!(
                    "[doe] Invalid IPC response with status {}",
                    response.header.status()
                );

                self.move_to_doe_error();

                Err(AdminErr::Pending)?
            }
        } else {
            error!("[doe] Spurious Message");
            self.move_to_fatal_error();

            Err(AdminErr::Pending)?
        }

        self.ctx.pcie_doe().send().map_err(|_err| {
            error!(
                "[doe] on_ipc_response: Failed to send DOE message: {:?}",
                _err
            );

            self.move_to_doe_error();

            AdminErr::Pending
        })?;

        self.state = DoeFsmState::WaitTxDone;
        self.ipc_channel.take();

        Err(AdminErr::Pending)
    }

    /// DOE message data is ready to be sent
    fn on_tx_ready(&mut self) -> Result<(), AdminErr> {
        self.ctx.pcie_doe().send().map_err(|_err| {
            error!("[doe] on_tx_ready: Failed to send DOE message: {:?}", _err);

            self.move_to_doe_error();

            AdminErr::Pending
        })?;

        Err(AdminErr::Pending)
    }

    /// All DOE message data has been sent
    fn on_tx_done(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.ctx.pcie_doe().end_send();
        self.doe_msg_active = false;
        self.move_to_idle(tag);

        Err(AdminErr::Pending)
    }

    /// Receive DoeAbort event or FLR event
    fn on_doe_abort(&mut self, tag: TagId) -> Result<(), AdminErr> {
        if self.waiting_for_resource_or_ipc_response() {
            self.move_to_idle = true;
        } else {
            self.move_to_idle(tag);
        }

        Err(AdminErr::Pending)
    }

    /// Receive PerstDown event
    fn on_perst_down(&mut self) -> Result<(), AdminErr> {
        if self.waiting_for_resource_or_ipc_response() {
            self.move_to_standby = true;
        } else {
            self.move_to_standby();
        }

        Err(AdminErr::Pending)
    }

    /// On DOE error event
    fn on_doe_err(&mut self) -> Result<(), AdminErr> {
        if self.waiting_for_resource_or_ipc_response() {
            self.move_to_doe_error = true;
        } else {
            self.move_to_doe_error();
        }

        Err(AdminErr::Pending)
    }

    /// Try acquiring the IPC channel resource and send an IPC message
    fn try_send_ipc_request(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.ipc_channel = self.ctx.admin_to_hsp_ipc_channel().acquire(tag, ());
        if self.ipc_channel.is_none() {
            self.state = DoeFsmState::WaitIpcChannel;
            Err(AdminErr::Pending)?;
        }

        self.send_ipc_request(tag)
    }

    /// Send an IPC message to HSP core
    fn send_ipc_request(&mut self, tag: u16) -> Result<(), AdminErr> {
        if self.general_check_on_resource(tag) {
            Err(AdminErr::Pending)?;
        }

        let message = IpcMessageDoe {
            info: DoeInfo {
                addr: self.ctx.pcie_doe().buffer_addr().lo,
            },
            ..Default::default()
        };

        self.ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, message.encode()))
            .map_err(|_err| {
                error!("Failed to send IPC message to HSP: {:?}", _err);

                self.move_to_fatal_error();

                AdminErr::Pending
            })?;

        self.state = DoeFsmState::WaitIpcResponse;
        self.timer
            .start(Tcon::get_approximate_ticks_from_ms(DOE_WAIT_IPC_RESPONSE_TIME_MS) as u8);

        Err(AdminErr::Pending)
    }

    /// Receive an IPC message from HSP core
    fn receive_ipc_message(&mut self) -> Option<IpcMessage> {
        self.ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()))
    }
}
