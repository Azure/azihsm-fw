// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_tcon::Tcon;

use super::*;
use crate::context::AdminFsmContext;
use crate::env::AdminEnvTrait;
use crate::error::AdminErr;
use crate::recorder::AdminFsmEventRecorder;
use crate::resource::AdminFsmResourceId;
use crate::resource::HspIpcChannel;
use crate::AdminFsmEvent;

const WAIT_TDISP_IDLE_TIMEOUT: u32 = 1000; // ms
const WAIT_IPC_RESPONSE_TIMEOUT: u32 = 1000; // ms

/// TDISP Interrupt FSM states
#[repr(u32)]
#[derive(Copy, Clone, Debug, PartialEq)]
enum TdispIntFsmState {
    /// TDISP Interrupt FSM is waiting for new interrupts
    Idle,

    /// Wait for TDISP Interrupt FSM as a resource to be available
    WaitTdispIdle,

    /// Waiting for IPC channel resource to be ready
    WaitIpcChannel,

    /// Waiting for IPC channel response *from HSP*
    WaitIpcResponse,

    /// A fatal error state that can't be recovered
    FatalError,
}

/// TDISP Interrupt FSM
pub(crate) struct TdispIntFsm<E: AdminEnvTrait + 'static> {
    /// Current state of the FSM
    state: TdispIntFsmState,

    /// Timer
    timer: CmdTimer,

    /// HSP IPC channel resource
    ipc_channel: Option<CmdResourceRef<HspIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// Resource of itself
    self_resource: Option<CmdResourceRef<TdispIdle, AdminFsm<E>>>,

    /// Context
    ctx: AdminFsmContext<E>,

    /// Cache the incoming source of interrupts
    source_mask: u32,

    /// Cache the vf/pf masks from tdisp interrupts
    tdisp_interrupt_mask: u128,

    /// Cache the last interrupts information
    last_tdisp_interrupts_info_regs: [u32; 5],

    /// Cache the latest IDE irq status
    last_ide_irq_status: u32,
}

impl<E: AdminEnvTrait> CmdFsm for TdispIntFsm<E> {
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
            (_, AdminFsmEvent::TdispInt(info)) => {
                self.source_mask |= 1 << u32::from(InterruptSource::Tdisp);
                self.tdisp_interrupt_mask |= info.vf_mask as u128;
                self.tdisp_interrupt_mask |= (info.pf_mask as u128) << 64;
                self.last_tdisp_interrupts_info_regs = info.info_regs; // NOTE: We are overwriting the last info regs

                self.on_interrupt_event(tag)
            }
            (_, AdminFsmEvent::Ide(status)) => {
                self.source_mask |= 1 << u32::from(InterruptSource::Ide);
                self.last_ide_irq_status = status.unwrap_or_default();

                self.on_interrupt_event(tag)
            }
            (_, AdminFsmEvent::PcieFlr) => {
                self.source_mask |= 1 << u32::from(InterruptSource::Flr);

                self.on_interrupt_event(tag)
            }
            (_, AdminFsmEvent::PcieVflr(_)) => {
                self.source_mask |= 1 << u32::from(InterruptSource::Flr);

                self.on_interrupt_event(tag)
            }
            (_, AdminFsmEvent::PciePerstUp) => {
                self.source_mask |= 1 << u32::from(InterruptSource::PerstUp);

                self.on_interrupt_event(tag)
            }
            (_, AdminFsmEvent::PciePerstDown) => {
                self.source_mask |= 1 << u32::from(InterruptSource::PerstDown);

                self.on_interrupt_event(tag)
            }
            (TdispIntFsmState::WaitTdispIdle, AdminFsmEvent::TimerElapsed)
            | (TdispIntFsmState::WaitIpcResponse, AdminFsmEvent::TimerElapsed) => {
                error!(
                    "[tdisp_int] TdispIntFsm timer elapsed, state: {}",
                    self.state as u32
                );
                self.handle_err();
                Err(AdminErr::Pending)
            }
            (
                TdispIntFsmState::WaitTdispIdle,
                AdminFsmEvent::ResourceReady(AdminFsmResourceId::TdispIdle),
            ) => self.try_send_ipc_request(tag),
            (
                TdispIntFsmState::WaitIpcChannel,
                AdminFsmEvent::ResourceReady(AdminFsmResourceId::HspIpcChannel),
            ) => self.send_ipc_request(tag),
            (TdispIntFsmState::WaitIpcResponse, AdminFsmEvent::HspToAdminIpcResponse) => {
                self.on_ipc_response(tag)
            }
            _ => Err(AdminErr::Pending),
        }
    }

    /// Acquire a resource for the FSM
    fn acquire_resource(&mut self, tag: TagId, id: Self::ResourceId) -> Self::Event {
        match id {
            AdminFsmResourceId::HspIpcChannel => {
                self.ipc_channel = self.ctx.admin_to_hsp_ipc_channel().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::HspIpcChannel)
            }
            AdminFsmResourceId::TdispIdle => {
                self.self_resource = self.ctx.tdisp_idle().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::TdispIdle)
            }
            _ => AdminFsmEvent::Unknown,
        }
    }
}

impl<E: AdminEnvTrait> TdispIntFsm<E> {
    pub fn new(ctx: AdminFsmContext<E>) -> Self {
        let timer = CmdTimer::new();

        Self {
            state: TdispIntFsmState::Idle,
            ipc_channel: None,
            self_resource: None,
            ctx,
            timer,
            source_mask: 0,
            tdisp_interrupt_mask: 0,
            last_tdisp_interrupts_info_regs: Default::default(),
            last_ide_irq_status: Default::default(),
        }
    }

    /// Handle the incoming interrupt event
    fn on_interrupt_event(&mut self, tag: TagId) -> Result<(), AdminErr> {
        if self.state == TdispIntFsmState::Idle {
            self.self_resource = self.ctx.tdisp_idle().acquire(tag, ());
            if self.self_resource.is_some() {
                self.try_send_ipc_request(tag)
            } else {
                self.state = TdispIntFsmState::WaitTdispIdle;
                self.timer
                    .start(Tcon::get_approximate_ticks_from_ms(WAIT_TDISP_IDLE_TIMEOUT) as u8);
                Err(AdminErr::Pending)
            }
        } else {
            Err(AdminErr::Pending)
        }
    }

    /// Try acquiring the IPC channel resource and send an IPC message
    fn try_send_ipc_request(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.timer.stop();

        self.ipc_channel = self.ctx.admin_to_hsp_ipc_channel().acquire(tag, ());
        if self.ipc_channel.is_none() {
            self.state = TdispIntFsmState::WaitIpcChannel;
            Err(AdminErr::Pending)?;
        }

        self.send_ipc_request(tag)
    }

    /// Send an IPC message to HSP core
    fn send_ipc_request(&mut self, tag: u16) -> Result<(), AdminErr> {
        let src = self.get_current_source();

        let info = match src {
            Some(InterruptSource::Tdisp) => TdispInterruptInfo {
                source: src.unwrap(),
                vf_mask: [
                    self.tdisp_interrupt_mask as u32,
                    (self.tdisp_interrupt_mask >> 32) as u32,
                ],
                pf_mask: (self.tdisp_interrupt_mask >> 64) as u32,
                reg_values: self.last_tdisp_interrupts_info_regs,
            },

            Some(InterruptSource::Ide) => TdispInterruptInfo {
                source: src.unwrap(),
                reg_values: [self.last_ide_irq_status, 0, 0, 0, 0],
                ..Default::default()
            },
            Some(InterruptSource::Flr)
            | Some(InterruptSource::PerstUp)
            | Some(InterruptSource::PerstDown) => TdispInterruptInfo {
                source: src.unwrap(),
                ..Default::default()
            },
            _ => {
                error!(
                    "[tdisp_int] Invalid interrupt source, source_mask: {}",
                    self.source_mask
                );
                self.handle_err();
                Err(AdminErr::Pending)?
            }
        };

        let message = IpcMessageTdispInterrupt {
            info,
            ..Default::default()
        };

        self.ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, message.encode()))
            .map_err(|err| {
                error!("[tdisp_int] Failed to send IPC message to HSP: {:?}", err);
                self.handle_err();

                AdminErr::Pending
            })?;

        let src = self.get_current_source();
        if let Some(src) = src {
            self.source_mask ^= 1 << u32::from(src);
            match src {
                InterruptSource::Tdisp => {
                    self.tdisp_interrupt_mask = 0;
                    self.last_tdisp_interrupts_info_regs = Default::default();
                }
                InterruptSource::Ide => {
                    self.last_ide_irq_status = Default::default();
                }
                _ => {}
            }
        }

        self.state = TdispIntFsmState::WaitIpcResponse;
        self.timer
            .start(Tcon::get_approximate_ticks_from_ms(WAIT_IPC_RESPONSE_TIMEOUT) as u8);

        Err(AdminErr::Pending)
    }

    /// On IPC response event: SP has sent response to CP
    fn on_ipc_response(&mut self, tag: u16) -> Result<(), AdminErr> {
        self.timer.stop();

        if let Some(message) = self.receive_ipc_message() {
            let ipc_response: IpcMessageTdispInterrupt = IpcMessageDecoder::decode(message)
                .map_err(|err| {
                    error!("[tdisp_int] Failed to decode IPC message: {:?}", err);
                    self.handle_err();

                    AdminErr::Pending
                })?;

            if ipc_response.header.status() != 0 {
                error!(
                    "[tdisp_int] Invalid IPC response with status {}",
                    ipc_response.header.status()
                );
                self.handle_err();

                Err(AdminErr::Pending)?
            }

            let src = self.get_current_source();
            if src.is_some() {
                return self.send_ipc_request(tag);
            } else {
                self.state = TdispIntFsmState::Idle;
                self.ipc_channel.take();
                self.self_resource.take();
            }
        } else {
            error!("[tdisp_int] Spurious Message");
            self.handle_err();
        }

        Err(AdminErr::Pending)
    }

    /// Receive an IPC message from HSP core
    fn receive_ipc_message(&mut self) -> Option<IpcMessage> {
        self.ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()))
    }

    /// Handle fatal error
    fn handle_err(&mut self) {
        self.state = TdispIntFsmState::FatalError;

        panic!();
    }

    /// Get the current source of the interrupt from source mask
    fn get_current_source(&mut self) -> Option<InterruptSource> {
        let idx = self.source_mask.trailing_zeros();
        let src = InterruptSource::from(idx);
        if src == InterruptSource::Unknown {
            None
        } else {
            Some(src)
        }
    }
}
