// Copyright (c) Microsoft Corporation. All rights reserved.

use bitfield::BitMut;
use log::trace;
use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_logging::*;
use mcr_pcie_controller::PcieControllerTrait;

use super::*;
use crate::context::AdminFsmContext;
use crate::env::AdminEnvTrait;
use crate::error::AdminErr;
use crate::event::AdminFsmEvent;
use crate::function::*;
use crate::resource::AdminFsmResourceId;
use crate::resource::AdminToFpIpcChannel;
use crate::resource::HsmIpcChannel;
use crate::AdminFsmEventRecorder;

/// Virtual Function Level Reset (vFLR) FSM states
#[derive(Copy, Clone, Debug)]
enum VflrFsmState {
    /// vFLR FSM is idle
    Idle,

    /// Waiting for IPC channel resource to be ready
    WaitIpcChannel,

    /// Waiting for FP IPC channel resource to be ready
    WaitFpIpcChannel,

    /// Waiting for HSM IPC channel resource to be ready
    WaitHsmIpcChannel,

    /// Waiting for IPC channel response from either FP or HSM
    WaitIpcResponse,

    /// Waiting for IPC channel response from FP
    WaitFpIpcResponse,

    /// Waiting for IPC channel response from HSM
    WaitHsmIpcResponse,
}

/// Virtual Function Level Reset (vFLR) FSM
pub(crate) struct VflrFsm<E: AdminEnvTrait + 'static> {
    /// FSM state
    state: VflrFsmState,

    /// Fastpath IPC channel resource
    admin_to_fp_ipc_channel:
        Option<CmdResourceRef<AdminToFpIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    // HSM IPC channel resource
    hsm_ipc_channel: Option<CmdResourceRef<HsmIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// PCIe virtual function list that are pending a FLR
    pending_fn_list: u64,

    /// Active PCIE function that is going through VFLR
    pending_pfn: Option<PcieFunction>,

    /// Context
    ctx: AdminFsmContext<E>,
}

impl<E: AdminEnvTrait> CmdFsm for VflrFsm<E> {
    type Error = AdminErr;
    type ResourceId = ResId;
    type Event = AdminFsmEvent;
    type Recorder = AdminFsmEventRecorder;

    /// Received an event for the FSM
    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match (self.state, event) {
            // Receive new virtual function level reset request while FSM is in Idle state
            (VflrFsmState::Idle, AdminFsmEvent::PcieVflr(pending_list)) => {
                self.on_new_vflr_request(tag, pending_list)
            }

            // Receive state change while the FSM is in WaitIpc(*)Channel / WaitIpc(*)Response
            (VflrFsmState::WaitIpcChannel, AdminFsmEvent::PcieVflr(pending_list))
            | (VflrFsmState::WaitFpIpcChannel, AdminFsmEvent::PcieVflr(pending_list))
            | (VflrFsmState::WaitHsmIpcChannel, AdminFsmEvent::PcieVflr(pending_list))
            | (VflrFsmState::WaitIpcResponse, AdminFsmEvent::PcieVflr(pending_list))
            | (VflrFsmState::WaitFpIpcResponse, AdminFsmEvent::PcieVflr(pending_list))
            | (VflrFsmState::WaitHsmIpcResponse, AdminFsmEvent::PcieVflr(pending_list)) => {
                self.on_pend_vflr(tag, pending_list)
            }

            // On first IPC channel ready
            (VflrFsmState::WaitIpcChannel, AdminFsmEvent::ResourceReady(resource)) => {
                self.on_first_ipc_channel_ready(tag, resource)
            }

            // On second IPC channel ready
            (
                VflrFsmState::WaitFpIpcChannel,
                AdminFsmEvent::ResourceReady(ResId::AdminToFpIpcChannel),
            )
            | (
                VflrFsmState::WaitHsmIpcChannel,
                AdminFsmEvent::ResourceReady(ResId::HsmIpcChannel),
            ) => self.on_second_ipc_channel_ready(tag),

            // Received response from either FP or HSM  IPC Channel while waiting for
            // either FP or HSM IPC Channel response
            (VflrFsmState::WaitIpcResponse, AdminFsmEvent::FpToAdminIpcResponse)
            | (VflrFsmState::WaitIpcResponse, AdminFsmEvent::HsmIpcResponse) => {
                self.on_ipc_response_message(event)
            }

            (VflrFsmState::WaitFpIpcResponse, AdminFsmEvent::FpToAdminIpcResponse) => {
                self.on_fp_ipc_response_message(tag)
            }

            (VflrFsmState::WaitHsmIpcResponse, AdminFsmEvent::HsmIpcResponse) => {
                self.on_hsm_ipc_response_message(tag)
            }

            // Ignore other events
            _ => Err(AdminErr::Pending),
        }
    }

    /// Acquire a resource for the FSM
    fn acquire_resource(&mut self, tag: TagId, id: Self::ResourceId) -> Self::Event {
        match id {
            AdminFsmResourceId::AdminToFpIpcChannel => {
                self.admin_to_fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::AdminToFpIpcChannel)
            }
            AdminFsmResourceId::HsmIpcChannel => {
                self.hsm_ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::HsmIpcChannel)
            }
            _ => AdminFsmEvent::Unknown,
        }
    }
}

impl<E: AdminEnvTrait> VflrFsm<E> {
    pub fn new(ctx: AdminFsmContext<E>) -> Self {
        Self {
            ctx,
            state: VflrFsmState::Idle,
            admin_to_fp_ipc_channel: None,
            hsm_ipc_channel: None,
            pending_fn_list: Default::default(),
            pending_pfn: None,
        }
    }
    /// On new virtual function level reset request
    fn on_new_vflr_request(&mut self, tag: TagId, pending_list: u64) -> Result<(), AdminErr> {
        trace!("[tag: {}] on_new_vflr_request", tag);

        self.pending_fn_list |= pending_list;

        self.admin_to_fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
        self.hsm_ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());

        self.state = match (
            self.admin_to_fp_ipc_channel.as_ref(),
            self.hsm_ipc_channel.as_ref(),
        ) {
            (Some(_), Some(_)) => self.process_vflr(tag),
            (Some(_), None) => VflrFsmState::WaitHsmIpcChannel,
            (None, Some(_)) => VflrFsmState::WaitFpIpcChannel,
            (None, None) => VflrFsmState::WaitIpcChannel,
        };

        Err(AdminErr::Pending)
    }

    /// New FLR request for virtual function is available
    fn on_pend_vflr(&mut self, tag: TagId, pending_list: u64) -> Result<(), AdminErr> {
        trace!(
            "[tag: {}] on_pend_cntrl, Current state = {:?}",
            tag,
            self.state
        );

        self.pending_fn_list |= pending_list;

        Err(AdminErr::Pending)
    }

    /// On first IPC channel ready
    fn on_first_ipc_channel_ready(&mut self, tag: TagId, resource: ResId) -> Result<(), AdminErr> {
        trace!(
            "[tag: {}] on_first_ipc_channel_ready for {:?}",
            tag,
            resource
        );

        match resource {
            ResId::AdminToFpIpcChannel => {
                self.state = VflrFsmState::WaitHsmIpcChannel;
            }
            ResId::HsmIpcChannel => {
                self.state = VflrFsmState::WaitFpIpcChannel;
            }
            _ => unreachable!(),
        }

        Err(AdminErr::Pending)
    }

    /// On second IPC channel ready
    fn on_second_ipc_channel_ready(&mut self, tag: TagId) -> Result<(), AdminErr> {
        trace!("[tag: {}] on_second_ipc_channel_ready", tag);
        self.state = self.process_vflr(tag);

        Err(AdminErr::Pending)
    }

    /// On IPC channel response message
    fn on_ipc_response_message(&mut self, event: AdminFsmEvent) -> Result<(), AdminErr> {
        match event {
            AdminFsmEvent::FpToAdminIpcResponse => {
                let _ = self.receive_fp_ipc_message();
                self.state = VflrFsmState::WaitHsmIpcResponse;
            }
            AdminFsmEvent::HsmIpcResponse => {
                let _ = self.receive_hsm_ipc_message();
                self.state = VflrFsmState::WaitFpIpcResponse;
            }
            _ => {}
        }

        Err(AdminErr::Pending)
    }

    /// On FP IPC channel response message
    fn on_fp_ipc_response_message(&mut self, tag: TagId) -> Result<(), AdminErr> {
        trace!("[tag: {}] on_fp_ipc_response_message", tag);

        let _ = self.receive_fp_ipc_message();

        if let Some(pfn) = self.pending_pfn {
            self.complete(tag, pfn)
        }

        Err(AdminErr::Pending)
    }

    /// On HSM IPC channel response message
    fn on_hsm_ipc_response_message(&mut self, tag: TagId) -> Result<(), AdminErr> {
        trace!("[tag: {}] on_hsm_ipc_response_message", tag);

        let _ = self.receive_hsm_ipc_message();

        if let Some(pfn) = self.pending_pfn {
            self.complete(tag, pfn)
        }

        Err(AdminErr::Pending)
    }

    /// Complete the VFLR
    fn complete(&mut self, tag: TagId, pfn: PcieFunction) {
        info!("Complete VFLR for {:?}", pfn.0 as u32);
        self.ctx.function_mgr().function(pfn).disable();
        self.ctx.pcie_cntrl().complete_flr(pfn);

        if self.pending_fn_list != 0 {
            self.state = self.process_vflr(tag);
        } else {
            self.reset();
            self.state = VflrFsmState::Idle;
        }
    }

    /// Process VFLR request
    fn process_vflr(&mut self, tag: TagId) -> VflrFsmState {
        loop {
            self.pending_pfn = self.next_pending_pfn();

            if let Some(pfn) = self.pending_pfn {
                if self.ctx.function_mgr().function(pfn).ready() {
                    if self.send_ipc(tag, pfn).is_err() {
                        continue;
                    }
                    break VflrFsmState::WaitIpcResponse;
                } else {
                    // If the function is not ready, then skip sending IPC and
                    // respond to the PCIe host that the FLR can be completed
                    // Clear this function from the list of pending functions to handle vFLR
                    self.ctx.pcie_cntrl().complete_flr(pfn);
                    self.pending_fn_list.set_bit(pfn.into(), false);
                    continue;
                }
            } else {
                self.reset();
                break VflrFsmState::Idle;
            }
        }
    }

    /// Send IPC message to FP and HSM
    fn send_ipc(&mut self, tag: TagId, pfn: PcieFunction) -> Result<(), AdminErr> {
        // Clear the bit for the Pcie function that is going through FLR
        self.pending_fn_list.set_bit(pfn.into(), false);

        self.admin_to_fp_ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, self.prepare_ipc_message(pfn).encode()))
            .map_err(|err| {
                error!("[vflr] Failed to send IPC message to FP: {:?}", err);
                AdminErr::IpcSendRequestError
            })?;

        self.hsm_ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, self.prepare_ipc_message(pfn).encode()))
            .map_err(|err| {
                error!("[vflr] Failed to send IPC message to HSM: {:?}", err);
                AdminErr::IpcSendRequestError
            })
    }

    /// Prepare IPC message
    fn prepare_ipc_message(&self, pfn: PcieFunction) -> IpcMessagePfnEnableDisable {
        IpcMessagePfnEnableDisable {
            info: PfnEnableDisableInfo {
                pfn,
                action: PfnEnableDisableAction::Disable,
            },
            ..Default::default()
        }
    }

    /// Receive IPC message from FP
    fn receive_fp_ipc_message(&mut self) -> Option<IpcMessage> {
        self.admin_to_fp_ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()))
    }

    /// Receive IPC message from HSM
    fn receive_hsm_ipc_message(&mut self) -> Option<IpcMessage> {
        self.hsm_ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()))
    }

    /// Reset the FSM
    fn reset(&mut self) {
        self.pending_fn_list = 0;
        self.admin_to_fp_ipc_channel.take();
        self.hsm_ipc_channel.take();
        self.pending_pfn.take();
    }

    /// Get the next pending PCIe function ID
    fn next_pending_pfn(&self) -> Option<PcieFunction> {
        let function = self.pending_fn_list.trailing_zeros() as u8;

        // This state machine only deals with Virtual function, so we need to filter out
        // PF function ID and beyond
        if function >= PCIE_PF_FUNCTION_ID {
            return None;
        }

        function.try_into().ok()
    }
}
