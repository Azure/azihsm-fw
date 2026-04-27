// Copyright (c) Microsoft Corporation. All rights reserved.

use bitfield::BitMut;
use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_logging::*;
use mcr_msix_controller::*;
use mcr_queue_controller::QueueCntrlId;
use mcr_simplex::SimplexPipeTrait;

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

/// Controller FSM states
#[derive(Copy, Clone, Debug)]
enum CntrlFsmState {
    /// Controller FSM is idle
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

/// Controller FSM
pub(crate) struct CntrlFsm<E: AdminEnvTrait + 'static> {
    /// FSM state
    state: CntrlFsmState,

    /// Fastpath IPC channel resource
    admin_to_fp_ipc_channel:
        Option<CmdResourceRef<AdminToFpIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    // HSM IPC channel resource
    hsm_ipc_channel: Option<CmdResourceRef<HsmIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// Pending controller enable bit map
    pending_cntrl_list: u128,

    /// NVMe Sub-System Reset pending list
    nssr_pending_list: u128,

    /// Active Controller ID that is going through state change
    active_cntrl_id: Option<QueueCntrlId>,

    /// Based on the last HSM response, if this action needs to be deferred
    pending_deferred_action: bool,

    /// Flag to indicate if response from hsm is valid
    valid_hsm_response: bool,

    /// Flag to indicate if response from fp is valid
    valid_fp_response: bool,

    /// Controller enable status
    enable_status: CntrlStateChangeAction,

    /// Context
    ctx: AdminFsmContext<E>,
}

impl<E: AdminEnvTrait> CmdFsm for CntrlFsm<E> {
    type Error = AdminErr;
    type ResourceId = ResId;
    type Event = AdminFsmEvent;
    type Recorder = AdminFsmEventRecorder;

    /// Received an event for the FSM
    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match (self.state, event) {
            // Receive state change while the FSM is in Idle state
            (CntrlFsmState::Idle, AdminFsmEvent::CntrlStateChange(pending_list)) => {
                self.on_cntrl_state_change(tag, pending_list)
            }

            // Receive NVMe sub-system reset while the FSM is in Idle state
            (CntrlFsmState::Idle, AdminFsmEvent::Nssr(pending_list)) => {
                self.on_nssr(tag, pending_list)
            }

            // Receive state change while the FSM is in WaitIpc(*)Channel / WaitIpc(*)Response
            (CntrlFsmState::WaitIpcChannel, AdminFsmEvent::CntrlStateChange(pending_list))
            | (CntrlFsmState::WaitFpIpcChannel, AdminFsmEvent::CntrlStateChange(pending_list))
            | (CntrlFsmState::WaitHsmIpcChannel, AdminFsmEvent::CntrlStateChange(pending_list))
            | (CntrlFsmState::WaitIpcResponse, AdminFsmEvent::CntrlStateChange(pending_list))
            | (CntrlFsmState::WaitFpIpcResponse, AdminFsmEvent::CntrlStateChange(pending_list))
            | (CntrlFsmState::WaitHsmIpcResponse, AdminFsmEvent::CntrlStateChange(pending_list)) => {
                self.on_pend_cntrl(pending_list)
            }

            // Receive NVMe sub-system reset while the FSM is in
            // WaitIpc(*)Channel / WaitIpc(*)Response
            (CntrlFsmState::WaitIpcChannel, AdminFsmEvent::Nssr(pending_list))
            | (CntrlFsmState::WaitFpIpcChannel, AdminFsmEvent::Nssr(pending_list))
            | (CntrlFsmState::WaitHsmIpcChannel, AdminFsmEvent::Nssr(pending_list))
            | (CntrlFsmState::WaitIpcResponse, AdminFsmEvent::Nssr(pending_list))
            | (CntrlFsmState::WaitFpIpcResponse, AdminFsmEvent::Nssr(pending_list))
            | (CntrlFsmState::WaitHsmIpcResponse, AdminFsmEvent::Nssr(pending_list)) => {
                self.on_pend_nssr(pending_list)
            }

            // On first IPC channel ready
            (CntrlFsmState::WaitIpcChannel, AdminFsmEvent::ResourceReady(resource)) => {
                self.on_first_ipc_channel_ready(resource)
            }

            // On second IPC channel ready
            (
                CntrlFsmState::WaitFpIpcChannel,
                AdminFsmEvent::ResourceReady(ResId::AdminToFpIpcChannel),
            )
            | (
                CntrlFsmState::WaitHsmIpcChannel,
                AdminFsmEvent::ResourceReady(ResId::HsmIpcChannel),
            ) => self.on_second_ipc_channel_ready(tag),

            // Received response from either FP or HSM  IPC Channel while waiting for
            // either FP or HSM IPC Channel response
            (CntrlFsmState::WaitIpcResponse, AdminFsmEvent::FpToAdminIpcResponse)
            | (CntrlFsmState::WaitIpcResponse, AdminFsmEvent::HsmIpcResponse) => {
                self.on_ipc_response_message(event)
            }

            (CntrlFsmState::WaitFpIpcResponse, AdminFsmEvent::FpToAdminIpcResponse) => {
                self.on_fp_ipc_response_message(tag)
            }

            (CntrlFsmState::WaitHsmIpcResponse, AdminFsmEvent::HsmIpcResponse) => {
                self.on_hsm_ipc_response_message(tag)
            }

            (_, AdminFsmEvent::IoCancellationComplete) => self.on_handle_deferred_action(tag),
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

impl<E: AdminEnvTrait> CntrlFsm<E> {
    pub fn new(ctx: AdminFsmContext<E>) -> Self {
        Self {
            state: CntrlFsmState::Idle,
            admin_to_fp_ipc_channel: None,
            hsm_ipc_channel: None,
            pending_cntrl_list: Default::default(),
            nssr_pending_list: Default::default(),
            active_cntrl_id: None,
            enable_status: CntrlStateChangeAction::Invalid,
            pending_deferred_action: false,
            valid_fp_response: false,
            valid_hsm_response: false,
            ctx,
        }
    }

    /// On controller state change
    fn on_cntrl_state_change(&mut self, tag: TagId, pending_list: u128) -> Result<(), AdminErr> {
        self.pending_cntrl_list |= pending_list;

        self.acquire_ipc_channel_and_take_action(tag)
    }

    /// On NVMe sub-system reset
    fn on_nssr(&mut self, tag: TagId, pending_list: u128) -> Result<(), AdminErr> {
        self.nssr_pending_list |= pending_list;

        self.acquire_ipc_channel_and_take_action(tag)
    }

    /// On new event while in Idle state
    fn acquire_ipc_channel_and_take_action(&mut self, tag: u16) -> Result<(), AdminErr> {
        self.admin_to_fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
        self.hsm_ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());

        self.state = match (
            self.admin_to_fp_ipc_channel.as_ref(),
            self.hsm_ipc_channel.as_ref(),
        ) {
            (Some(_), Some(_)) => self.update_cntrl_state(tag),
            (Some(_), None) => CntrlFsmState::WaitHsmIpcChannel,
            (None, Some(_)) => CntrlFsmState::WaitFpIpcChannel,
            (None, None) => CntrlFsmState::WaitIpcChannel,
        };

        Err(AdminErr::Pending)
    }

    /// New controller state change pending
    fn on_pend_cntrl(&mut self, pending_list: u128) -> Result<(), AdminErr> {
        self.pending_cntrl_list |= pending_list;

        Err(AdminErr::Pending)
    }

    /// New NVMe sub-system reset pending
    fn on_pend_nssr(&mut self, pending_list: u128) -> Result<(), AdminErr> {
        self.nssr_pending_list |= pending_list;

        Err(AdminErr::Pending)
    }

    /// On first IPC channel ready
    fn on_first_ipc_channel_ready(&mut self, resource: ResId) -> Result<(), AdminErr> {
        match resource {
            ResId::AdminToFpIpcChannel => {
                self.state = CntrlFsmState::WaitHsmIpcChannel;
            }
            ResId::HsmIpcChannel => {
                self.state = CntrlFsmState::WaitFpIpcChannel;
            }
            _ => unreachable!(),
        }

        Err(AdminErr::Pending)
    }

    /// On second IPC channel ready
    fn on_second_ipc_channel_ready(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.state = self.update_cntrl_state(tag);

        Err(AdminErr::Pending)
    }

    /// Check the hsm response and update current_deferred_action flag
    fn check_hsm_reponse(&mut self) -> Result<(), AdminErr> {
        let msg = self.receive_hsm_ipc_message().ok_or_else(|| {
            error!("Failed to receive IPC message from HSM");

            AdminErr::Pending
        })?;

        let response: IpcMessagePfnEnableDisable =
            IpcMessageDecoder::decode(msg).map_err(|_| {
                error!("Failed to decode IPC message from HSM");

                AdminErr::Pending
            })?;

        self.pending_deferred_action =
            response.header.status() == IpcMessageStatusCode::Pending.into();

        Ok(())
    }

    /// Check if fp response is valid
    fn check_fp_response(&mut self) -> Result<(), AdminErr> {
        let msg = self.receive_fp_ipc_message().ok_or_else(|| {
            error!("Failed to receive IPC message from FP");
            AdminErr::Pending
        })?;

        let _: IpcMessagePfnEnableDisable = IpcMessageDecoder::decode(msg).map_err(|_| {
            error!("Failed to decode IPC message from FP");

            AdminErr::Pending
        })?;

        Ok(())
    }

    /// On IPC channel response message
    fn on_ipc_response_message(&mut self, event: AdminFsmEvent) -> Result<(), AdminErr> {
        match event {
            AdminFsmEvent::FpToAdminIpcResponse => {
                self.valid_fp_response = self.check_fp_response().is_ok();
                self.state = CntrlFsmState::WaitHsmIpcResponse;
            }
            AdminFsmEvent::HsmIpcResponse => {
                self.valid_hsm_response = self.check_hsm_reponse().is_ok();
                self.state = CntrlFsmState::WaitFpIpcResponse;
            }
            _ => {}
        }

        Err(AdminErr::Pending)
    }

    /// On FP IPC channel response message
    fn on_fp_ipc_response_message(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.valid_fp_response = self.check_fp_response().is_ok();

        if let Some(cntrl_id) = self.active_cntrl_id {
            self.complete(tag, cntrl_id)
        }

        Err(AdminErr::Pending)
    }

    /// On HSM IPC channel response message
    fn on_hsm_ipc_response_message(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.valid_hsm_response = self.check_hsm_reponse().is_ok();

        if let Some(cntrl_id) = self.active_cntrl_id {
            self.complete(tag, cntrl_id);
        }

        Err(AdminErr::Pending)
    }

    /// On deferred action
    fn on_handle_deferred_action(&mut self, tag: u16) -> Result<(), AdminErr> {
        let message = self.ctx.queue_delete_notification().recv().ok_or_else(|| {
            error!("Fail to receive queue_delete_notification");
            AdminErr::Pending
        })?;

        if message.tag != tag {
            error!(
                "Mismatch tags: received ({:?}) vs expected ({:?})",
                message.tag as u32, tag as u32
            );

            Err(AdminErr::Pending)?;
        }

        let cntrl_id = QueueCntrlId::try_from(message.pfn).map_err(|_| AdminErr::Pending)?;

        if self.active_cntrl_id != Some(cntrl_id) {
            error!(
                "Received deferred action for a different controller: {:?}",
                cntrl_id as u32
            );

            Err(AdminErr::Pending)?;
        }

        self.pending_deferred_action = false;

        self.complete(tag, cntrl_id);

        Err(AdminErr::Pending)
    }

    /// Complete the controller state change
    fn complete(&mut self, tag: TagId, cntrl_id: QueueCntrlId) {
        if self.valid_fp_response && self.valid_hsm_response {
            match self.enable_status {
                CntrlStateChangeAction::Enable => {
                    if self
                        .ctx
                        .function_mgr()
                        .function(cntrl_id.into())
                        .enable()
                        .is_err()
                    {
                        error!("Error in enabling controller {:?}", cntrl_id as u32);
                    }
                    self.ctx.msix_cntrl().enable_pcie_fn(cntrl_id.into());
                }
                CntrlStateChangeAction::Disable | CntrlStateChangeAction::Migrate => {
                    if !self.pending_deferred_action {
                        self.ctx.function_mgr().function(cntrl_id.into()).disable();
                    }
                }
                CntrlStateChangeAction::Invalid => unreachable!(),
            }
        }

        if !self.pending_deferred_action {
            self.valid_fp_response = false;
            self.valid_hsm_response = false;

            if self.pending_cntrl_list != 0 || self.nssr_pending_list != 0 {
                self.state = self.update_cntrl_state(tag);
            } else {
                self.reset();
                self.state = CntrlFsmState::Idle;
            }
        }
    }

    /// Update controller state
    fn update_cntrl_state(&mut self, tag: TagId) -> CntrlFsmState {
        loop {
            self.active_cntrl_id = self.next_pending_cntrl_id();

            if let Some(cntrl_id) = self.active_cntrl_id {
                let state_change = self
                    .ctx
                    .function_mgr()
                    .function(cntrl_id.into())
                    .query_state_change();
                match state_change {
                    CntrlStateChangeAction::Invalid => {
                        // This controller state change is invalid, try the next controller
                        self.pending_cntrl_list.set_bit(cntrl_id as usize, false);

                        continue;
                    }
                    _ => {
                        self.pending_cntrl_list.set_bit(cntrl_id as usize, false);
                        if self.send_ipc(tag, state_change, cntrl_id).is_err() {
                            continue;
                        }
                        break CntrlFsmState::WaitIpcResponse;
                    }
                }
            } else if self.nssr_pending_list != 0 {
                self.active_cntrl_id = self.next_pending_cntrl_id_to_reset();

                if let Some(cntrl_id) = self.active_cntrl_id {
                    self.nssr_pending_list.set_bit(cntrl_id as usize, false);
                    if self
                        .send_ipc(tag, CntrlStateChangeAction::Migrate, cntrl_id)
                        .is_err()
                    {
                        continue;
                    }
                    break CntrlFsmState::WaitIpcResponse;
                }
            } else {
                self.reset();
                break CntrlFsmState::Idle;
            }
        }
    }

    /// Send IPC message to FP and HSM
    fn send_ipc(
        &mut self,
        tag: TagId,
        status: CntrlStateChangeAction,
        cntrl_id: QueueCntrlId,
    ) -> Result<(), AdminErr> {
        // Clear the bit for the controller that is going through state change
        self.enable_status = status;

        let message = self.prepare_fp_ipc_message(cntrl_id, tag);
        self.admin_to_fp_ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, message.encode()))
            .map_err(|err| {
                error!("[cntrl] Failed to send IPC message to FP: {:?}", err);
                AdminErr::IpcSendRequestError
            })?;

        let message = self.prepare_hsm_ipc_message(cntrl_id, tag);
        self.hsm_ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, message.encode()))
            .map_err(|err| {
                error!("[cntrl] Failed to send IPC message to HSM: {:?}", err);
                AdminErr::IpcSendRequestError
            })
    }

    /// Prepare IPC message for Pcie Function enable disable action to FP and HSM
    fn prepare_fp_ipc_message(
        &self,
        cntrl_id: QueueCntrlId,
        tag: TagId,
    ) -> IpcMessagePfnEnableDisable {
        let mut message = IpcMessagePfnEnableDisable {
            info: PfnEnableDisableInfo {
                pfn: PcieFunction::from(cntrl_id),
                action: if self.enable_status == CntrlStateChangeAction::Enable {
                    PfnEnableDisableAction::Enable
                } else {
                    PfnEnableDisableAction::Disable
                },
            },
            ..Default::default()
        };

        message.header.set_tag(tag.into());

        message
    }

    /// Prepare HSM IPC message for Pcie Function enable/disable/reset action to FP and HSM
    fn prepare_hsm_ipc_message(
        &self,
        cntrl_id: QueueCntrlId,
        tag: TagId,
    ) -> IpcMessagePfnEnableDisable {
        let mut message = IpcMessagePfnEnableDisable {
            info: PfnEnableDisableInfo {
                pfn: PcieFunction::from(cntrl_id),
                action: if self.enable_status == CntrlStateChangeAction::Enable {
                    PfnEnableDisableAction::Enable
                } else if self.enable_status == CntrlStateChangeAction::Migrate {
                    PfnEnableDisableAction::Migrate
                } else {
                    PfnEnableDisableAction::Disable
                },
            },
            ..Default::default()
        };

        message.header.set_tag(tag.into());

        message
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
        self.pending_cntrl_list = 0;
        self.nssr_pending_list = 0;
        self.enable_status = CntrlStateChangeAction::Invalid;
        self.admin_to_fp_ipc_channel.take();
        self.hsm_ipc_channel.take();
        self.active_cntrl_id.take();
    }

    /// Get the next pending controller ID
    fn next_pending_cntrl_id(&self) -> Option<QueueCntrlId> {
        let id = self.pending_cntrl_list.trailing_zeros();

        id.try_into().ok()
    }

    /// Get the next pending controller ID to issue reset
    fn next_pending_cntrl_id_to_reset(&self) -> Option<QueueCntrlId> {
        let id = self.nssr_pending_list.trailing_zeros();

        id.try_into().ok()
    }
}
