// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

// Important: Keep in-sync with docs/idfu/dfu_cp_admin.puml

use mcr_doe::PcieDoeTrait;
use mcr_error::McrResult;
use mcr_io_controller::IoControllerTrait;
use mcr_ipc_controller::IpcMessageChannelTrait;
use mcr_ipc_message::IpcMessageDecoder;
use mcr_ipc_message::IpcMessageEncoderTrait;
use mcr_ipc_message::IpcMessageShutdown;
use mcr_ipc_message::IpcMessageStatusCode;
use mcr_ipc_message::ShutdownInfo;
use mcr_logging::*;
use mcr_tcon::Tcon;

use super::*;
use crate::context::AdminFsmContext;
use crate::env::AdminEnvTrait;
use crate::error::AdminErr;
use crate::fsm::ResId;
use crate::recorder::AdminFsmEventRecorder;
use crate::resource::AdminFsmResourceId;
use crate::resource::AdminToFpIpcChannel;
use crate::resource::HsmIpcChannel;
use crate::AdminFsmEvent;

const IDFU_DEFAULT_DRAIN_TIME_MS: u32 = 1500;

/// Impactless Device Firmware Update state
#[derive(Copy, Clone, Debug, PartialEq)]
enum IdfuState {
    /// Device is in the Idle state
    Idle,

    /// Device is in the Started/Drain state
    DfuDrain,

    /// Device is in the Started/Wait state
    DfuWait,

    /// Device is in the Started/Ready state
    DfuReady,
}

/// Impactless Device Firmware Update faulted system
#[repr(u32)]
#[derive(Copy, Clone, Debug, PartialEq)]
enum IdfuFaultedSystem {
    /// CP Admin
    Admin = 0b0000_0001,

    /// CP HSM
    Hsm = 0b0000_0010,

    /// FP System
    Fp = 0b0000_0100,

    /// PCIe DOE
    Doe = 0b0000_1000,

    /// CAST FIPS testing
    Cast = 0b0001_0000,

    /// PCIe TDISP
    Tdisp = 0b0010_0000,
}

/// Impactless Device Firmware Update Admin FSM
pub(crate) struct IdfuFsm<E: AdminEnvTrait + 'static> {
    /// Current state of the FSM
    state: IdfuState,

    /// Context
    ctx: AdminFsmContext<E>,

    /// Timer
    timer: CmdTimer,

    /// Maximum Drain Time Milliseconds
    maximum_drain_time_ms: u32,

    /// Drain time start as obtained from TCON/Timestamp Counter
    drain_time_start_tsc: u64,

    /// Fastpath IPC channel resource
    admin_to_fp_ipc_channel:
        Option<CmdResourceRef<AdminToFpIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// HSM IPC channel resource
    hsm_ipc_channel: Option<CmdResourceRef<HsmIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// SP IPC tag transaction ID
    sp_ipc_tag: u32,

    /// Cast idle resource
    cast_idle: Option<CmdResourceRef<CastIdle, AdminFsm<E>>>,

    /// DOE idle resource
    doe_idle: Option<CmdResourceRef<DoeIdle, AdminFsm<E>>>,

    /// TDISP idle resource
    tdisp_idle: Option<CmdResourceRef<TdispIdle, AdminFsm<E>>>,

    /// Scheduled empty
    scheduler_queue_empty: bool,
}

impl<E: AdminEnvTrait> CmdFsm for IdfuFsm<E> {
    type Error = AdminErr;
    type ResourceId = AdminFsmResourceId;
    type Event = AdminFsmEvent;
    type Recorder = AdminFsmEventRecorder;

    /// Get the timer for the hierarchical FSM
    fn get_timer(&mut self) -> Option<&mut CmdTimer> {
        Some(&mut self.timer)
    }

    /// Handle an event for the hierarchical FSM
    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match self.state {
            // Idle state
            IdfuState::Idle => self.handle_dfu_idle(tag, event),

            // DFU Started sub states
            IdfuState::DfuWait => self.handle_dfu_wait(tag, event),
            IdfuState::DfuDrain => self.handle_dfu_drain(tag, event),
            IdfuState::DfuReady => self.handle_dfu_ready(event),
        }
    }

    /// On resource acquisition
    fn acquire_resource(&mut self, tag: TagId, id: Self::ResourceId) -> Self::Event {
        match id {
            // debug_assert instead of panic.
            AdminFsmResourceId::AdminToFpIpcChannel => {
                debug_assert!(self.admin_to_fp_ipc_channel.is_none());
                self.admin_to_fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::AdminToFpIpcChannel)
            }
            AdminFsmResourceId::HsmIpcChannel => {
                debug_assert!(self.hsm_ipc_channel.is_none());
                self.hsm_ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::HsmIpcChannel)
            }
            AdminFsmResourceId::CastIdle => {
                debug_assert!(self.cast_idle.is_none());
                self.cast_idle = self.ctx.cast_idle().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::CastIdle)
            }
            AdminFsmResourceId::DoeIdle => {
                debug_assert!(self.doe_idle.is_none());
                self.doe_idle = self.ctx.doe_idle().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::DoeIdle)
            }
            AdminFsmResourceId::TdispIdle => {
                debug_assert!(self.tdisp_idle.is_none());
                self.tdisp_idle = self.ctx.tdisp_idle().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::TdispIdle)
            }
            _ => AdminFsmEvent::Unknown,
        }
    }
}

impl<E: AdminEnvTrait> IdfuFsm<E> {
    /// Create a new iDFU FSM
    pub fn new(ctx: AdminFsmContext<E>) -> Self {
        let timer = CmdTimer::new();
        Self {
            state: IdfuState::Idle,
            ctx,
            timer,
            hsm_ipc_channel: None,
            admin_to_fp_ipc_channel: None,
            maximum_drain_time_ms: 0,
            sp_ipc_tag: 0,
            drain_time_start_tsc: 0,
            cast_idle: None,
            doe_idle: None,
            tdisp_idle: None,
            scheduler_queue_empty: false,
        }
    }

    /// Get the approximate number of ticks from milliseconds
    fn get_override_ticks(drain_time_override_ms: u32) -> u8 {
        match drain_time_override_ms {
            500..=25000 => Tcon::get_approximate_ticks_from_ms(drain_time_override_ms) as u8,
            _ => Tcon::get_approximate_ticks_from_ms(IDFU_DEFAULT_DRAIN_TIME_MS) as u8,
        }
    }

    /// CP_Admin_DFU_Normal root state handler
    fn handle_normal(&mut self, event: AdminFsmEvent) -> Result<(), AdminErr> {
        let mut res_id: Option<u32> = None;
        match event {
            AdminFsmEvent::ResourceReady(ResId::HsmIpcChannel) => {
                debug_assert!(self.hsm_ipc_channel.is_some());

                res_id = Some(ResId::HsmIpcChannel as u32);

                self.hsm_ipc_channel.take();
            }
            AdminFsmEvent::ResourceReady(ResId::AdminToFpIpcChannel) => {
                debug_assert!(self.admin_to_fp_ipc_channel.is_some());

                res_id = Some(ResId::AdminToFpIpcChannel as u32);

                self.admin_to_fp_ipc_channel.take();
            }
            AdminFsmEvent::ResourceReady(ResId::CastIdle) => {
                debug_assert!(self.cast_idle.is_some());

                res_id = Some(ResId::CastIdle as u32);

                self.cast_idle.take();
            }
            AdminFsmEvent::ResourceReady(ResId::DoeIdle) => {
                debug_assert!(self.doe_idle.is_some());

                res_id = Some(ResId::DoeIdle as u32);

                self.doe_idle.take();
            }
            AdminFsmEvent::ResourceReady(ResId::TdispIdle) => {
                debug_assert!(self.tdisp_idle.is_some());

                res_id = Some(ResId::TdispIdle as u32);

                self.tdisp_idle.take();
            }
            AdminFsmEvent::HsmIpcResponse => {
                warn!(
                    "Late IPC_HSM_SHUTDOWN_RSP after {}us",
                    self.get_elapsed_time_us()
                );

                _ = self.get_hsm_ipc_response_status();
            }
            AdminFsmEvent::FpToAdminIpcResponse => {
                warn!(
                    "Late IPC_FP_SHUTDOWN_RSP after {}us",
                    self.get_elapsed_time_us()
                );

                _ = self.get_fp_ipc_response_status();
            }
            AdminFsmEvent::SchedulerQueueEmptyEvent => {
                // no-op
            }
            _ => {
                // no-op
            }
        }
        if res_id.is_some() {
            warn!(
                "Late resource {} acquire {}us",
                res_id.unwrap(),
                self.get_elapsed_time_us()
            );
        }

        Err(AdminErr::Pending)
    }

    /// CP_Admin_DFU_Idle state handler
    fn handle_dfu_idle(&mut self, tag: TagId, event: AdminFsmEvent) -> Result<(), AdminErr> {
        match event {
            AdminFsmEvent::ShutdownRequest((info, ipc_message_tag)) => {
                self.maximum_drain_time_ms = info.drain_time_ms;
                self.sp_ipc_tag = ipc_message_tag;

                self.state = IdfuState::DfuDrain;
                self.enter_dfu_started(tag);
            }
            _ => {
                return self.handle_normal(event);
            }
        }

        Err(AdminErr::Pending)
    }

    /// CP_Admin_DFU_Started enter handler
    fn enter_dfu_started(&mut self, tag: TagId) {
        info!("Starting");

        self.scheduler_queue_empty = false;

        // UCD pause
        self.ctx.io_controller().pause_inbound();
        self.ctx.fp_io_controller().pause_inbound();
        self.ctx.pause_queue_controller();

        debug_assert!(self.cast_idle.is_none());
        self.cast_idle = self.ctx.cast_idle().acquire(tag, ());
        debug_assert!(self.doe_idle.is_none());
        self.doe_idle = self.ctx.doe_idle().acquire(tag, ());
        if self.doe_idle.is_some() {
            self.ctx.pcie_doe().set_busy(true);
        }

        debug_assert!(self.tdisp_idle.is_none());
        self.tdisp_idle = self.ctx.tdisp_idle().acquire(tag, ());

        self.drain_time_start_tsc = self.ctx.tcon_tsc();
        self.timer
            .start(Self::get_override_ticks(self.maximum_drain_time_ms));
    }

    //// CP_Admin_DFU_Started exit handler
    fn exit_dfu_started(&mut self) {
        // Handle substate exit_ states (no-op).

        // UCD_resume
        self.ctx.io_controller().resume_inbound();
        self.ctx.fp_io_controller().resume_inbound();
        self.ctx.resume_queue_controller();

        if self.doe_idle.is_some() {
            self.ctx.pcie_doe().set_busy(false);
        }

        self.doe_idle.take();
        self.cast_idle.take();
        self.tdisp_idle.take();

        self.timer.stop();

        info!("Stopped after {}us", self.get_elapsed_time_us());
    }

    /// CP_Admin_DFU_Started error handler
    fn error_dfu(&mut self, faulted_core: IdfuFaultedSystem, status: IpcMessageStatusCode) {
        match faulted_core {
            IdfuFaultedSystem::Admin => {
                warn!(
                    "Admin abort with status: {:?}. State: ({:?})",
                    <mcr_ipc_message::IpcMessageStatusCode as Into<u32>>::into(status),
                    self.state as u32
                );
            }
            IdfuFaultedSystem::Hsm => {
                warn!(
                    "HSM abort with status: {:?}. State: ({:?})",
                    <mcr_ipc_message::IpcMessageStatusCode as Into<u32>>::into(status),
                    self.state as u32
                );
            }
            IdfuFaultedSystem::Fp => {
                warn!(
                    "FP abort with status: {:?}. State: ({:?})",
                    <mcr_ipc_message::IpcMessageStatusCode as Into<u32>>::into(status),
                    self.state as u32
                );
            }
            _ => {
                unreachable!()
            }
        }

        _ = self
            .send_ipc_sp_shutdown_response(IpcMessageStatusCode::OperationFailed)
            .map_err(|err| {
                error!("error_dfu: IPC_SP_SHUTDOWN_RSP {:?}", err);
            });

        self.exit_dfu_started();
        self.state = IdfuState::Idle;
    }

    /// CP_Admin_DFU_Started state handler
    fn handle_dfu_started(&mut self, event: AdminFsmEvent) -> Result<(), AdminErr> {
        match event {
            AdminFsmEvent::TimerElapsed => {
                let mut timeout_resources = 0;
                if self.admin_to_fp_ipc_channel.is_some() {
                    timeout_resources |= IdfuFaultedSystem::Fp as u32;
                }
                if self.hsm_ipc_channel.is_some() {
                    timeout_resources |= IdfuFaultedSystem::Hsm as u32;
                }
                if self.cast_idle.is_none() {
                    timeout_resources |= IdfuFaultedSystem::Cast as u32;
                }
                if self.doe_idle.is_none() {
                    timeout_resources |= IdfuFaultedSystem::Doe as u32;
                }
                if self.tdisp_idle.is_none() {
                    timeout_resources |= IdfuFaultedSystem::Tdisp as u32;
                }

                warn!(
                    "Timeout in state {:?} resources: {:#x}",
                    self.state as u32, timeout_resources
                );

                if self.state != IdfuState::DfuReady {
                    _ = self
                        .send_ipc_sp_shutdown_response(IpcMessageStatusCode::OperationTimeout)
                        .map_err(|err| {
                            error!("handle_dfu_started: IPC_SP_SHUTDOWN_RSP {:?}", err);
                        });
                }

                self.exit_dfu_started();
                self.state = IdfuState::Idle;
            }
            AdminFsmEvent::HsmIpcResponse => {
                let status = self.get_hsm_ipc_response_status();
                if status != IpcMessageStatusCode::Success {
                    self.error_dfu(IdfuFaultedSystem::Hsm, status);
                }
            }
            AdminFsmEvent::FpToAdminIpcResponse => {
                let status = self.get_fp_ipc_response_status();
                if status != IpcMessageStatusCode::Success {
                    self.error_dfu(IdfuFaultedSystem::Fp, status);
                }
            }
            AdminFsmEvent::SchedulerQueueEmptyEvent => {
                // no-op.
            }
            _ => {
                return self.handle_normal(event);
            }
        }

        Err(AdminErr::Pending)
    }

    /// CP_Admin_DFU_Drain check if all resources are idle.
    fn dfu_check_drain_ready(&mut self, tag: TagId) {
        if self.cast_idle.is_some()
            && self.doe_idle.is_some()
            && self.tdisp_idle.is_some()
            && self.scheduler_queue_empty
        {
            // CP HSM core does not implement a timeout timer when UCD is resumed. It is
            // guaranteed that CP HSM will timeout I/Os within 1 tick.
            let remaining_ticks =
                Tcon::get_approximate_ticks_from_ms(self.get_remaining_drain_time_ms());
            if remaining_ticks >= 2 {
                self.state = IdfuState::DfuWait;
                self.enter_dfu_wait(tag);
            } else {
                warn!("Insufficient time to drain CP HSM/FP.");

                self.error_dfu(
                    IdfuFaultedSystem::Admin,
                    IpcMessageStatusCode::OperationTimeout,
                );
            }
        }
    }

    /// CP_Admin_DFU_Drain state handler
    fn handle_dfu_drain(&mut self, tag: TagId, event: AdminFsmEvent) -> Result<(), AdminErr> {
        match event {
            AdminFsmEvent::SchedulerQueueEmptyEvent => {
                self.scheduler_queue_empty = true;
                self.dfu_check_drain_ready(tag);
            }
            AdminFsmEvent::ResourceReady(ResId::CastIdle) => {
                debug_assert!(self.cast_idle.is_some());
                self.dfu_check_drain_ready(tag);
            }
            AdminFsmEvent::ResourceReady(ResId::DoeIdle) => {
                debug_assert!(self.doe_idle.is_some());
                self.ctx.pcie_doe().set_busy(true);
                self.dfu_check_drain_ready(tag);
            }
            AdminFsmEvent::ResourceReady(ResId::TdispIdle) => {
                debug_assert!(self.tdisp_idle.is_some());
                self.dfu_check_drain_ready(tag);
            }
            _ => {
                return self.handle_dfu_started(event);
            }
        }

        Err(AdminErr::Pending)
    }

    /// CP_Admin_DFU_Wait enter handler
    fn enter_dfu_wait(&mut self, tag: TagId) {
        if self.admin_to_fp_ipc_channel.is_none() {
            self.admin_to_fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
            self.dfu_wait_send_shutdown_fp(tag);
        }

        if self.hsm_ipc_channel.is_none() {
            self.hsm_ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());
            self.dfu_wait_send_shutdown_hsm(tag);
        }
    }

    /// CP_Admin_DFU_Wait send IPC_HSM shutdown
    fn dfu_wait_send_shutdown_hsm(&mut self, tag: TagId) {
        if self.hsm_ipc_channel.is_some() {
            if let Err(err) = self.send_ipc_cp_hsm_shutdown_request(tag) {
                error!("IPC_CP_HSM_SHUTDOWN_REQ {:#x}", err);
                self.error_dfu(IdfuFaultedSystem::Hsm, IpcMessageStatusCode::UnknownStatus);
            }
        }
    }

    /// CP_Admin_DFU_Wait send IPC_FP shutdown
    fn dfu_wait_send_shutdown_fp(&mut self, tag: TagId) {
        if self.admin_to_fp_ipc_channel.is_some() {
            if let Err(err) = self.send_ipc_fp_shutdown_request(tag) {
                error!("IPC_FP_SHUTDOWN_REQ {:#x}", err);
                self.error_dfu(IdfuFaultedSystem::Fp, IpcMessageStatusCode::UnknownStatus);
            }
        }
    }

    /// CP_Admin_DFU_Wait verify all cores reported drain
    fn check_all_cores_reported_drain(&mut self) {
        if let (None, None) = (
            self.admin_to_fp_ipc_channel.as_ref(),
            self.hsm_ipc_channel.as_ref(),
        ) {
            self.state = IdfuState::DfuReady;
            self.enter_dfu_ready();
        }
    }

    /// CP_Admin_DFU_Wait handler
    fn handle_dfu_wait(&mut self, tag: TagId, event: AdminFsmEvent) -> Result<(), AdminErr> {
        match event {
            AdminFsmEvent::ResourceReady(ResId::HsmIpcChannel) => {
                debug_assert!(self.hsm_ipc_channel.is_some());
                self.dfu_wait_send_shutdown_hsm(tag);
            }
            AdminFsmEvent::ResourceReady(ResId::AdminToFpIpcChannel) => {
                debug_assert!(self.admin_to_fp_ipc_channel.is_some());
                self.dfu_wait_send_shutdown_fp(tag);
            }
            AdminFsmEvent::HsmIpcResponse => {
                let status = self.get_hsm_ipc_response_status();

                match status {
                    IpcMessageStatusCode::Success => {
                        self.check_all_cores_reported_drain();
                    }
                    _ => {
                        self.error_dfu(IdfuFaultedSystem::Hsm, status);
                    }
                }
            }
            AdminFsmEvent::FpToAdminIpcResponse => {
                let status = self.get_fp_ipc_response_status();

                match status {
                    IpcMessageStatusCode::Success => {
                        self.check_all_cores_reported_drain();
                    }
                    _ => {
                        self.error_dfu(IdfuFaultedSystem::Fp, status);
                    }
                }
            }
            _ => {
                return self.handle_dfu_started(event);
            }
        }

        Err(AdminErr::Pending)
    }

    /// CP_Admin_DFU_Ready enter handler
    fn enter_dfu_ready(&mut self) {
        info!("Ready in {} us", self.get_elapsed_time_us());

        #[cfg(feature = "idfu_fault_pre_reset_admin_shutdown_ipc_no_resp_err")]
        {
            log::debug!("iDFU Fault Injected: idfu_fault_pre_reset_admin_shutdown_ipc_no_resp_err");
        }

        #[cfg(feature = "idfu_fault_pre_reset_admin_drain_timeout")]
        {
            log::debug!("iDFU Fault Injected: idfu_fault_pre_reset_admin_drain_timeout");
            if let Err(err) =
                self.send_ipc_sp_shutdown_response(IpcMessageStatusCode::OperationTimeout)
            {
                error!(
                    "iDFU Abort with OperationTimeout: Failure sending IPC_SP_SHUTDOWN_RSP {:#x}",
                    err
                );
                self.exit_dfu_started();
                self.state = IdfuState::Idle;
            }
        }

        #[cfg(feature = "idfu_fault_pre_reset_admin_shutdown_ipc_resp_err")]
        {
            log::debug!("iDFU Fault Injected: idfu_fault_pre_reset_admin_shutdown_ipc_resp_err");
            if let Err(err) =
                self.send_ipc_sp_shutdown_response(IpcMessageStatusCode::OperationFailed)
            {
                error!(
                    "iDFU Abort with OperationFailed: Failure sending IPC_SP_SHUTDOWN_RSP {:#x}",
                    err
                );
                self.exit_dfu_started();
                self.state = IdfuState::Idle;
            }
        }

        #[cfg(not(any(
            feature = "idfu_fault_pre_reset_admin_shutdown_ipc_no_resp_err",
            feature = "idfu_fault_pre_reset_admin_shutdown_ipc_resp_err",
            feature = "idfu_fault_pre_reset_admin_drain_timeout"
        )))]
        if let Err(err) = self.send_ipc_sp_shutdown_response(IpcMessageStatusCode::Success) {
            error!("IPC_SP_SHUTDOWN_RSP {:#x}", err);
            self.exit_dfu_started();
            self.state = IdfuState::Idle;
        }

        let mut remaining_ms = self.get_remaining_drain_time_ms() as u64;
        if remaining_ms < 1000 {
            remaining_ms = 1000;
        }

        // Busy-wait for the remaining time to pause NVIC interrupt processing before SP reset.
        let stop = self.ctx.tcon_tsc() + (Tcon::tsc_freq_hz() as u64) * remaining_ms / 1000u64;
        while self.ctx.tcon_tsc() < stop {
            self.ctx.update_core_liveliness();
        }

        error!("No reset detected.");
    }

    /// CP_Admin_DFU_Ready state handler
    fn handle_dfu_ready(&mut self, event: AdminFsmEvent) -> Result<(), AdminErr> {
        // Send events to super-state.
        self.handle_dfu_started(event)
    }

    /// Prepare the shutdown response
    fn prepare_shutdown_response(&self, status: IpcMessageStatusCode) -> IpcMessageShutdown {
        let mut response = IpcMessageShutdown {
            ..Default::default()
        };

        response.header.set_tag(self.sp_ipc_tag);
        response.header.set_response(true);
        response.header.set_status(status.into());

        response
    }

    /// Send the shutdown response
    fn send_ipc_sp_shutdown_response(&self, status: IpcMessageStatusCode) -> McrResult<()> {
        self.ctx
            .hsp_to_admin_ipc_channel()
            .send_response(self.prepare_shutdown_response(status).encode())
    }

    /// Prepare the shutdown request
    fn prepare_shutdown_request(&self) -> IpcMessageShutdown {
        IpcMessageShutdown {
            info: ShutdownInfo {
                drain_time_ms: self.get_remaining_drain_time_ms(),
            },
            ..Default::default()
        }
    }

    /// Send the shutdown request
    fn send_ipc_cp_hsm_shutdown_request(&self, tag: TagId) -> McrResult<()> {
        if let Some(hsm_ipc) = self.hsm_ipc_channel.as_ref() {
            return hsm_ipc
                .deref()
                .send_request(tag, self.prepare_shutdown_request().encode());
        }

        Err(AdminErr::SendResetEventToHsmFailed.into())
    }

    /// Send the shutdown request
    fn send_ipc_fp_shutdown_request(&self, tag: TagId) -> McrResult<()> {
        if let Some(fp_ipc) = self.admin_to_fp_ipc_channel.as_ref() {
            return fp_ipc
                .deref()
                .send_request(tag, self.prepare_shutdown_request().encode());
        }

        Err(AdminErr::SendResetEventToFpFailed.into())
    }

    /// Receive the IPC shutdown response
    fn recv_ipc_shutdown_response(&self, channel: &E::IpcChannel) -> IpcMessageStatusCode {
        let message = channel.receive_message().unwrap();
        let message: IpcMessageShutdown = match IpcMessageDecoder::decode(message) {
            Ok(msg) => msg,
            Err(_) => {
                return IpcMessageStatusCode::InvalidField;
            }
        };

        message.header.status().into()
    }

    /// Receive the IPC shutdown response from FP
    fn recv_ipc_fp_shutdown_response(&mut self) -> IpcMessageStatusCode {
        let fp_ipc = self.admin_to_fp_ipc_channel.as_ref().unwrap();

        self.recv_ipc_shutdown_response(&fp_ipc.deref())
    }

    /// Receive the IPC shutdown response from HSM
    fn recv_ipc_hsm_shutdown_response(&mut self) -> IpcMessageStatusCode {
        let hsm_ipc = self.hsm_ipc_channel.as_ref().unwrap();

        self.recv_ipc_shutdown_response(&hsm_ipc.deref())
    }

    // Get elapsed shutdown time.
    fn get_elapsed_time_us(&self) -> u32 {
        ((self.ctx.tcon_tsc() - self.drain_time_start_tsc)
            / (Tcon::tsc_freq_hz() as u64 / 1000000u64)) as u32
    }

    // Get remaining drain time.
    fn get_remaining_drain_time_ms(&self) -> u32 {
        self.maximum_drain_time_ms - self.get_elapsed_time_us() / 1000
    }

    // Handle a late or spurious HSM IPC.
    fn get_hsm_ipc_response_status(&mut self) -> IpcMessageStatusCode {
        if self.hsm_ipc_channel.is_some() {
            let status = self.recv_ipc_hsm_shutdown_response();
            self.hsm_ipc_channel.take();

            status
        } else {
            warn!(
                "Spurious IPC_CP_HSM_SHUTDOWN_RSP after {}us",
                self.get_elapsed_time_us()
            );

            IpcMessageStatusCode::UnknownStatus
        }
    }

    // Handle a late or spurious FP IPC.
    fn get_fp_ipc_response_status(&mut self) -> IpcMessageStatusCode {
        if self.admin_to_fp_ipc_channel.is_some() {
            let status = self.recv_ipc_fp_shutdown_response();
            self.admin_to_fp_ipc_channel.take();

            status
        } else {
            warn!(
                "Spurious IPC_FP_SHUTDOWN_RSP after {}us",
                self.get_elapsed_time_us()
            );

            IpcMessageStatusCode::UnknownStatus
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_drain_time() {
        let default_ticks: u8 = (IDFU_DEFAULT_DRAIN_TIME_MS * Tcon::wakeup_timer0_freq_hz() / 1000)
            .try_into()
            .unwrap();

        assert_eq!(IdfuFsm::<AdminEnv>::get_override_ticks(0), default_ticks);
        assert_eq!(IdfuFsm::<AdminEnv>::get_override_ticks(499), default_ticks);
        assert_eq!(IdfuFsm::<AdminEnv>::get_override_ticks(500), 2);
        assert_eq!(IdfuFsm::<AdminEnv>::get_override_ticks(25000), 100);
        assert_eq!(
            IdfuFsm::<AdminEnv>::get_override_ticks(25001),
            default_ticks
        );
    }
}
