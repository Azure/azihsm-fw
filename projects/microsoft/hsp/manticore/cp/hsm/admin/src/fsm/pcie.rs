// Copyright (c) Microsoft Corporation. All rights reserved.

use log::error;
use mcr_ipc_controller::IpcDescriptor;
use mcr_ipc_controller::IpcEventChannelTrait;
use mcr_pcie_controller::*;
use mcr_types::PcieFunction;

use super::*;
use crate::context::AdminFsmContext;
use crate::env::AdminEnvTrait;
use crate::error::AdminErr;
use crate::fsm::PcieResetReason;
use crate::function::FunctionMgrTrait;
use crate::recorder::AdminFsmEventRecorder;
use crate::resource::AdminFsmResourceId;
use crate::AdminFsmEvent;

/// PCIe Link state
#[derive(Copy, Clone, Debug)]
enum PcieLinkState {
    /// PCIe link is down
    Down,

    /// PCIe link is up
    Up,
}

/// PCIe PERST FSM
pub(crate) struct PcieFsm<E: AdminEnvTrait + 'static> {
    /// PCIe Link state
    state: PcieLinkState,

    /// Admin FSM Context
    ctx: AdminFsmContext<E>,

    /// Flr in progress
    flr_in_progress: bool,

    /// Response pending from FP CPU
    fp_response_pending: bool,

    /// Response pending from HSM CPU
    hsm_response_pending: bool,
}

impl<E: AdminEnvTrait> CmdFsm for PcieFsm<E> {
    type Error = AdminErr;
    type ResourceId = AdminFsmResourceId;
    type Event = AdminFsmEvent;
    type Recorder = AdminFsmEventRecorder;

    /// Handle an event
    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match (self.state, event) {
            (PcieLinkState::Down, AdminFsmEvent::PciePerstUp) => self.on_pcie_perst_up(),
            (PcieLinkState::Up, AdminFsmEvent::PciePerstDown) => self.on_pcie_perst_down(tag),
            (PcieLinkState::Up, AdminFsmEvent::PcieFlr) => self.on_pcie_flr(tag),
            (_, AdminFsmEvent::FpResetComplete) => self.process_fp_ipc_event(),
            (_, AdminFsmEvent::HsmResetComplete) => self.process_hsm_ipc_event(),
            _ => Err(AdminErr::Pending),
        }
    }
}

impl<E: AdminEnvTrait> PcieFsm<E> {
    /// Create a new PCIe FSM
    pub fn new(ctx: AdminFsmContext<E>) -> Self {
        let state = if ctx.pcie_cntrl().link_status().is_ok() {
            PcieLinkState::Up
        } else {
            PcieLinkState::Down
        };

        Self {
            ctx,
            state,
            flr_in_progress: false,
            fp_response_pending: false,
            hsm_response_pending: false,
        }
    }

    /// Handle PCIe PERST up event
    fn on_pcie_perst_up(&mut self) -> Result<(), AdminErr> {
        if let Err(err) = self.ctx.pcie_cntrl().perst_up() {
            error!("PCIE_PERST_UP Error: 0x{:08x}", err);
        } else {
            self.state = PcieLinkState::Up;
        }

        // Since PERST FSM is always running, return in progress
        Err(AdminErr::Pending)
    }

    /// Handle PCIe PERST down event
    fn on_pcie_perst_down(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.state = PcieLinkState::Down;

        // If FLR is in progress, do not send PERST DOWN event to HSM and FP as the cleanup is
        // already taken care of
        if !self.flr_in_progress {
            self.send_reset(tag, PcieResetReason::Perst)
        }

        Err(AdminErr::Pending)
    }

    /// Handle FLR event
    fn on_pcie_flr(&mut self, tag: TagId) -> Result<(), AdminErr> {
        if !self.flr_in_progress {
            self.flr_in_progress = true;

            self.send_reset(tag, PcieResetReason::Flr)
        }

        Err(AdminErr::Pending)
    }

    /// Send PF reset request to IO cores
    fn send_reset(&mut self, tag: TagId, event: PcieResetReason) {
        let hsm_desc = IpcDescriptor::Descriptor28;
        let fp_desc = IpcDescriptor::Descriptor17;

        let channel = self.ctx.hsm_ipc_event_channel();
        if channel.begin_event(tag, hsm_desc, event as u32).is_err() {
            // This can only fail with a firmware bug where, we try to send event twice without
            // processing the response
            panic!("Sending Reset Event to HSM failed");
        }
        self.hsm_response_pending = true;

        let channel = self.ctx.fp_ipc_event_channel();
        if channel.begin_event(tag, fp_desc, event as u32).is_err() {
            // This can only fail with a firmware bug where, we try to send event twice without
            // processing the response
            panic!("Sending Reset Event to FP failed");
        }
        self.fp_response_pending = true;
    }

    /// Receive FP IPC event
    fn process_fp_ipc_event(&mut self) -> Result<(), AdminErr> {
        if self.fp_response_pending {
            let _ = self
                .ctx
                .fp_ipc_event_channel()
                .receive_event(IpcDescriptor::Descriptor18);
            self.fp_response_pending = false;

            if !self.hsm_response_pending {
                match self.state {
                    PcieLinkState::Down => self.complete_perst_down(),
                    PcieLinkState::Up => self.complete_flr(),
                }
            }
        }

        Err(AdminErr::Pending)
    }

    /// Receive HSM IPC event
    fn process_hsm_ipc_event(&mut self) -> Result<(), AdminErr> {
        if self.hsm_response_pending {
            let _ = self
                .ctx
                .hsm_ipc_event_channel()
                .receive_event(IpcDescriptor::Descriptor29);
            self.hsm_response_pending = false;

            if !self.fp_response_pending {
                match self.state {
                    PcieLinkState::Down => self.complete_perst_down(),
                    PcieLinkState::Up => self.complete_flr(),
                }
            }
        }

        Err(AdminErr::Pending)
    }

    /// Complete FLR
    fn complete_flr(&mut self) {
        if self.flr_in_progress {
            self.ctx.function_mgr().reset();
            self.ctx.pcie_cntrl().complete_flr(PcieFunction::Pf);
            self.flr_in_progress = false;
            info!("PF FLR complete");
        }
    }

    /// Complete Perst Down
    fn complete_perst_down(&mut self) {
        self.ctx.function_mgr().reset();
        self.flr_in_progress = false;
        self.ctx.pcie_cntrl().perst_down();
        info!("Perst Down complete");
    }
}
