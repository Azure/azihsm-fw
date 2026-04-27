// Copyright (c) Microsoft Corporation. All rights reserved.

use bitfield::Bit;
use bitfield::BitMut;
use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_simplex::SimplexPipeTrait;
use mcr_tcon::Tcon;
use mcr_types::*;

use super::*;
use crate::context::AdminFsmContext;
use crate::env::AdminEnvTrait;
use crate::error::AdminErr;
use crate::event::AdminFsmEvent;
use crate::resource::AdminFsmResourceId;
use crate::resource::AdminToFpIpcChannel;
use crate::resource::HsmIpcChannel;
use crate::AdminFsmEventRecorder;

const WAIT_IPC_RESPONSE_TIMEOUT: u32 = 1000; // ms

/// Stop Interface FSM states
#[repr(u32)]
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
enum StopInterfaceFsmState {
    /// Stop Interface FSM is idle
    Idle,

    /// Waiting for IPC channel resource to be ready
    WaitIpcChannels,

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

    /// Waiting for IoCancellationComplete to clean up deferred bits
    WaitIoCancellationComplete,

    /// A fatal error state that can't be recovered
    FatalError,
}

/// Stop Interface FSM
pub(crate) struct StopInterfaceFsm<E: AdminEnvTrait + 'static> {
    /// FSM state
    state: StopInterfaceFsmState,

    /// Timer
    timer: CmdTimer,

    /// Fastpath IPC channel resource
    admin_to_fp_ipc_channel:
        Option<CmdResourceRef<AdminToFpIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    // HSM IPC channel resource
    hsm_ipc_channel: Option<CmdResourceRef<HsmIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// Pending active interrupt bit map
    pending_bits: u128,

    /// List of controller for which disable action is deferred until future notification
    deferred_bits: u128,

    /// Active Pcie function that is being stopped
    current_pfn: Option<PcieFunction>,

    /// Based on the last HSM response, if this action needs to be deferred
    current_deferred_action: bool,

    /// Flag to indicate if response from hsm is valid
    valid_hsm_response: bool,

    /// Flag to indicate if response from fp is valid
    valid_fp_response: bool,

    /// Context
    ctx: AdminFsmContext<E>,

    /// SP IPC tag transaction ID
    sp_ipc_tag: u32,
}

impl<E: AdminEnvTrait> CmdFsm for StopInterfaceFsm<E> {
    type Error = AdminErr;
    type ResourceId = ResId;
    type Event = AdminFsmEvent;
    type Recorder = AdminFsmEventRecorder;

    /// Get the timer for the hierarchical FSM
    fn get_timer(&mut self) -> Option<&mut CmdTimer> {
        Some(&mut self.timer)
    }

    /// Received an event for the FSM
    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match (self.state, event) {
            // Receive HSP stop interface request while the FSM is in Idle state
            (StopInterfaceFsmState::Idle, AdminFsmEvent::StopInterfaceRequest((bits, sp_tag))) => {
                self.sp_ipc_tag = sp_tag;
                self.on_stop_interface_request(tag, bits)
            }

            // On first IPC channel ready
            (StopInterfaceFsmState::WaitIpcChannels, AdminFsmEvent::ResourceReady(resource)) => {
                self.on_first_ipc_channel_ready(resource)
            }

            // On second IPC channel ready
            (
                StopInterfaceFsmState::WaitFpIpcChannel,
                AdminFsmEvent::ResourceReady(ResId::AdminToFpIpcChannel),
            )
            | (
                StopInterfaceFsmState::WaitHsmIpcChannel,
                AdminFsmEvent::ResourceReady(ResId::HsmIpcChannel),
            ) => self.on_second_ipc_channel_ready(tag),

            // Received response from either FP or HSM  IPC Channel while waiting for
            // either FP or HSM IPC Channel response
            (StopInterfaceFsmState::WaitIpcResponse, AdminFsmEvent::FpToAdminIpcResponse)
            | (StopInterfaceFsmState::WaitIpcResponse, AdminFsmEvent::HsmIpcResponse) => {
                self.on_ipc_response_message(event)
            }

            (StopInterfaceFsmState::WaitFpIpcResponse, AdminFsmEvent::FpToAdminIpcResponse) => {
                self.on_fp_ipc_response_message(tag)
            }

            (StopInterfaceFsmState::WaitHsmIpcResponse, AdminFsmEvent::HsmIpcResponse) => {
                self.on_hsm_ipc_response_message(tag)
            }

            (StopInterfaceFsmState::WaitIpcResponse, AdminFsmEvent::TimerElapsed)
            | (StopInterfaceFsmState::WaitFpIpcResponse, AdminFsmEvent::TimerElapsed)
            | (StopInterfaceFsmState::WaitHsmIpcResponse, AdminFsmEvent::TimerElapsed) => {
                // Timer elapsed while waiting for IPC response
                error!(
                    "[stop_interface] Timer elapsed, state: {:?}",
                    self.state as u32
                );
                self.handle_err();
                Err(AdminErr::Pending)
            }

            (StopInterfaceFsmState::WaitIpcResponse, AdminFsmEvent::IoCancellationComplete)
            | (StopInterfaceFsmState::WaitHsmIpcResponse, AdminFsmEvent::IoCancellationComplete)
            | (StopInterfaceFsmState::WaitFpIpcResponse, AdminFsmEvent::IoCancellationComplete)
            | (
                StopInterfaceFsmState::WaitIoCancellationComplete,
                AdminFsmEvent::IoCancellationComplete,
            ) => self.on_handle_deferred_action(tag),
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

impl<E: AdminEnvTrait> StopInterfaceFsm<E> {
    pub fn new(ctx: AdminFsmContext<E>) -> Self {
        Self {
            state: StopInterfaceFsmState::Idle,
            timer: CmdTimer::new(),
            admin_to_fp_ipc_channel: None,
            hsm_ipc_channel: None,
            pending_bits: Default::default(),
            deferred_bits: Default::default(),
            current_pfn: None,
            current_deferred_action: false,
            valid_hsm_response: false,
            valid_fp_response: false,
            ctx,
            sp_ipc_tag: Default::default(),
        }
    }

    /// Handle the stop interface request
    fn on_stop_interface_request(&mut self, tag: TagId, bits: u128) -> Result<(), AdminErr> {
        self.pending_bits = bits;

        self.admin_to_fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
        self.hsm_ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());

        self.state = match (
            self.admin_to_fp_ipc_channel.as_ref(),
            self.hsm_ipc_channel.as_ref(),
        ) {
            (Some(_), Some(_)) => self.stop_interface_loop(tag)?,
            (Some(_), None) => StopInterfaceFsmState::WaitHsmIpcChannel,
            (None, Some(_)) => StopInterfaceFsmState::WaitFpIpcChannel,
            (None, None) => StopInterfaceFsmState::WaitIpcChannels,
        };

        Err(AdminErr::Pending)
    }

    /// On first IPC channel ready
    fn on_first_ipc_channel_ready(&mut self, resource: ResId) -> Result<(), AdminErr> {
        match resource {
            ResId::AdminToFpIpcChannel => {
                self.state = StopInterfaceFsmState::WaitHsmIpcChannel;
            }
            ResId::HsmIpcChannel => {
                self.state = StopInterfaceFsmState::WaitFpIpcChannel;
            }
            _ => unreachable!(),
        }

        Err(AdminErr::Pending)
    }

    /// On second IPC channel ready
    fn on_second_ipc_channel_ready(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.state = self.stop_interface_loop(tag)?;

        Err(AdminErr::Pending)
    }

    /// Check the hsm response and update current_deferred_action flag
    fn check_hsm_reponse(&mut self) -> Result<(), AdminErr> {
        // Reset the flag
        self.current_deferred_action = false;

        let msg = self.receive_hsm_ipc_message().ok_or(AdminErr::Pending)?;

        let response: IpcMessagePfnEnableDisable =
            IpcMessageDecoder::decode(msg).map_err(|_| AdminErr::Pending)?;

        if let Some(pfn) = self.current_pfn {
            self.current_deferred_action =
                response.header.status() == IpcMessageStatusCode::Pending.into();

            if self.current_deferred_action {
                self.deferred_bits.set_bit(u8::from(pfn) as usize, true);
            }
        }

        Ok(())
    }

    /// Check if fp response is valid
    fn check_fp_response(&mut self) -> Result<(), AdminErr> {
        let msg = self.receive_fp_ipc_message().ok_or(AdminErr::Pending)?;

        let _: IpcMessagePfnEnableDisable =
            IpcMessageDecoder::decode(msg).map_err(|_| AdminErr::Pending)?;

        Ok(())
    }

    /// On IPC channel response message (first response)
    fn on_ipc_response_message(&mut self, event: AdminFsmEvent) -> Result<(), AdminErr> {
        match event {
            AdminFsmEvent::FpToAdminIpcResponse => {
                self.valid_fp_response = self.check_fp_response().is_ok();
                self.state = StopInterfaceFsmState::WaitHsmIpcResponse;
            }
            AdminFsmEvent::HsmIpcResponse => {
                self.valid_hsm_response = self.check_hsm_reponse().is_ok();
                self.state = StopInterfaceFsmState::WaitFpIpcResponse;
            }
            _ => {}
        }

        Err(AdminErr::Pending)
    }

    /// On FP IPC channel response message
    fn on_fp_ipc_response_message(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.timer.stop();

        self.valid_fp_response = self.check_fp_response().is_ok();

        if let Some(pfn) = self.current_pfn {
            self.state = self.complete(tag, pfn)?;
        }

        Err(AdminErr::Pending)
    }

    /// On HSM IPC channel response message
    fn on_hsm_ipc_response_message(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.timer.stop();

        self.valid_hsm_response = self.check_hsm_reponse().is_ok();

        if let Some(pfn) = self.current_pfn {
            self.state = self.complete(tag, pfn)?;
        }

        Err(AdminErr::Pending)
    }

    /// On deferred action
    fn on_handle_deferred_action(&mut self, tag: u16) -> Result<(), AdminErr> {
        let message = self.ctx.queue_delete_notification().recv().ok_or_else(|| {
            error!("[stop_interface] Failed to receive notification from queue");

            self.handle_err();
            AdminErr::Pending
        })?;

        if message.tag != tag {
            error!(
                "[stop_interface] Message tag {} is not expected {}",
                message.tag as u32, tag as u32
            );

            self.handle_err();
            Err(AdminErr::Pending)?;
        }

        let pfn = message.pfn;
        if self.deferred_bits.bit(u32::from(pfn) as usize) {
            self.deferred_bits.set_bit(u32::from(pfn) as usize, false);
        }

        if self.state == StopInterfaceFsmState::WaitIoCancellationComplete {
            // Only enter the loop if it is already in WaitIoCancellationComplete state
            self.state = self.stop_interface_loop(tag)?;
        }

        Err(AdminErr::Pending)
    }

    /// Complete the controller state change
    fn complete(
        &mut self,
        tag: TagId,
        pfn: PcieFunction,
    ) -> Result<StopInterfaceFsmState, AdminErr> {
        if !self.valid_fp_response || !self.valid_hsm_response {
            // Fail to get the valid response from FP or HSM
            // Add this bit back to the pending bits
            self.pending_bits.set_bit(u32::from(pfn) as usize, true);
        }

        self.valid_fp_response = false;
        self.valid_hsm_response = false;

        self.stop_interface_loop(tag)
    }

    /// Stop interface based on the cntrl id
    fn stop_interface_loop(&mut self, tag: TagId) -> Result<StopInterfaceFsmState, AdminErr> {
        self.current_pfn = self.next_pending_pfn();

        if let Some(pfn) = self.current_pfn {
            if self.send_ipc(tag, pfn).is_err() {
                self.handle_err();

                Err(AdminErr::Pending)?
            }

            Ok(StopInterfaceFsmState::WaitIpcResponse)
        } else if self.deferred_bits > 0 {
            Ok(StopInterfaceFsmState::WaitIoCancellationComplete)
        } else {
            self.send_hsp_response()
        }
    }

    /// Send IPC message to FP and HSM
    fn send_ipc(&mut self, tag: TagId, pfn: PcieFunction) -> Result<(), AdminErr> {
        // Clear the bit for the controller that is going through state change
        self.pending_bits.set_bit(u32::from(pfn) as usize, false);

        let message = self.prepare_ipc_message(pfn, tag);
        self.admin_to_fp_ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, message.encode()))
            .map_err(|err| {
                error!(
                    "[stop_interface] Failed to send IPC message to FP: {:?}",
                    err
                );
                self.handle_err();
                AdminErr::IpcSendRequestError
            })?;

        let message = self.prepare_ipc_message(pfn, tag);
        self.hsm_ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, message.encode()))
            .map_err(|err| {
                error!(
                    "[stop_interface] Failed to send IPC message to HSM: {:?}",
                    err
                );
                self.handle_err();
                AdminErr::IpcSendRequestError
            })?;

        self.timer
            .start(Tcon::get_approximate_ticks_from_ms(WAIT_IPC_RESPONSE_TIMEOUT) as u8);

        Ok(())
    }

    /// Prepare IPC message for Pcie Function disable action to FP and HSM
    fn prepare_ipc_message(&self, pfn: PcieFunction, tag: TagId) -> IpcMessagePfnEnableDisable {
        let mut message = IpcMessagePfnEnableDisable {
            info: PfnEnableDisableInfo {
                pfn,
                action: PfnEnableDisableAction::Disable,
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

    /// Request is complete. Send response to HSP
    fn send_hsp_response(&mut self) -> Result<StopInterfaceFsmState, AdminErr> {
        assert_eq!(self.pending_bits, 0);
        assert_eq!(self.deferred_bits, 0);

        let mut message = IpcMessageStopInterface::default();
        message.header.set_response(true);
        message
            .header
            .set_status(IpcMessageStatusCode::Success.into());
        message.header.set_tag(self.sp_ipc_tag);

        self.ctx
            .hsp_to_admin_stop_interface_ipc_channel()
            .send_response(message.encode())
            .map_err(|err| {
                error!("[stop_interface] Failed to send response to HSP: {:?}", err);
                self.handle_err();
                AdminErr::Pending
            })?;

        self.admin_to_fp_ipc_channel.take();
        self.hsm_ipc_channel.take();

        Ok(StopInterfaceFsmState::Idle)
    }

    /// Get the next pending controller ID
    fn next_pending_pfn(&self) -> Option<PcieFunction> {
        let id = self.pending_bits.trailing_zeros();

        (id as u8).try_into().ok()
    }

    /// Handle the fatal error
    fn handle_err(&mut self) {
        self.state = StopInterfaceFsmState::FatalError;

        panic!();
    }
}
