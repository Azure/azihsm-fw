// Copyright (c) Microsoft Corporation. All rights reserved.

use log::debug;
use mcr_crashdump::*;
use mcr_crypto_rng::RngTrait;
use mcr_crypto_softaes::SoftAesTrait;
use mcr_ddi_types::DdiTestActionCrashType;
use mcr_ddi_types::DdiTestStackErrorType;
use mcr_error::McrResult;
use mcr_gdma_controller::*;
use mcr_ide::*;
use mcr_io_controller::*;
use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_logging::*;
#[cfg(feature = "mcr_manual_test_hooks")]
use mcr_pcie_controller::TdispIntInfo;
use mcr_self_test::SelfTest;
use mcr_simplex::*;
use mcr_types::AesGcmIV;
use mcr_types::DebugLogComponent;
use mcr_types::DebugLogEntryParameters;
use mcr_types::DebugLogSeverity;
use mcr_types::IoMemRange;
use mcr_types::SoftAesOffloadResp;
use mcr_types::SoftAesOp;

use crate::context::AdminFsmContext;
use crate::error;
use crate::error::AdminErr;
use crate::event::AdminFsmEvent;
use crate::fsm::AdminCmdFsm;
use crate::fsm::AdminFsm;
use crate::fsm::AesGcmExtFsm;
use crate::fsm::CastFsm;
use crate::fsm::CntrlFsm;
use crate::fsm::DoeFsm;
use crate::fsm::IdfuFsm;
use crate::fsm::PcieFsm;
use crate::fsm::StopInterfaceFsm;
use crate::fsm::TdispIntFsm;
use crate::fsm::TelemetryFsm;
use crate::fsm::VflrFsm;
use crate::recorder::AdminFsmEventRecorder;
use crate::warn;
use crate::AdminEnvTrait;
use crate::CmdScheduler;
use crate::TagId;

/// Admin Core Event handler
pub(crate) struct AdminEventHandler<E: AdminEnvTrait + 'static> {
    ctx: AdminFsmContext<E>,
    scheduler: CmdScheduler<AdminFsm<E>>,
    pcie_tag: TagId,
    vflr_tag: TagId,
    cntrl_tag: TagId,
    doe_tag: TagId,
    idfu_tag: TagId,
    cast_tag: TagId,
    telemetry_tag: TagId,
    tdisp_int_tag: TagId,
    stop_interface_tag: TagId,
    aes_gcm_ext_tag: TagId,
    admin_to_fp_ipc_channel: E::IpcChannel,
    hsm_ipc_channel: E::IpcChannel,
    admin_to_hsp_ipc_channel: E::IpcChannel,
    soft_event_round_robin: u8,
}

impl<E: AdminEnvTrait> AdminEventHandler<E> {
    /// Create an instance of `AdminEventHandler`
    ///
    /// # Arguments
    ///
    /// * `env` - Environment
    /// * `recorder` - Event Recorder
    ///
    /// # Retruns
    ///
    /// * `AdminEventHandler` - Admin Event Handler Instance
    pub fn new(env: E, recorder: AdminFsmEventRecorder) -> Self {
        const VF: usize = 64;
        const PF: usize = 1;
        const STATIC: usize = 10;

        let scheduler = CmdScheduler::new(VF + PF + STATIC, 1, recorder);
        let ctx = AdminFsmContext::new(env.clone(), scheduler.clone());

        // Create Pcie Reset FSM
        let pcie_fsm = AdminFsm::Pcie(PcieFsm::new(ctx.clone()));
        let pcie_tag = scheduler.alloc_static(pcie_fsm);

        // Create VFLR FSM
        let vflr_fsm = AdminFsm::Vflr(VflrFsm::new(ctx.clone()));
        let vflr_tag = scheduler.alloc_static(vflr_fsm);

        // Create Controller FSM
        let cntrl_fsm = AdminFsm::Cntrl(CntrlFsm::new(ctx.clone()));
        let cntrl_tag = scheduler.alloc_static(cntrl_fsm);

        // Create DOE FSM
        let doe_fsm = AdminFsm::Doe(DoeFsm::new(ctx.clone()));
        let doe_tag = scheduler.alloc_static(doe_fsm);
        // This event is to explicitly initialize DOE FSM so that it can acquire resources with the proper tag
        scheduler.on_event(AdminFsmEvent::DoeFsmInit, doe_tag.unwrap());

        // Impactless Device Firmware Update FSM
        let idfu_fsm = AdminFsm::Idfu(IdfuFsm::new(ctx.clone()));
        let idfu_tag = scheduler.alloc_static(idfu_fsm);
        scheduler.set_scheduler_empty_notification(
            idfu_tag.unwrap(),
            AdminFsmEvent::SchedulerQueueEmptyEvent,
        );

        // Create CAST FSM
        let self_test_fsm = AdminFsm::Cast(CastFsm::new(ctx.clone()));
        let self_test_fsm_tag = scheduler.alloc_static(self_test_fsm);

        // Create Telemetry FSM
        let telemetry_fsm = AdminFsm::Telemetry(TelemetryFsm::new(ctx.clone()));
        let telemetry_tag = scheduler.alloc_static(telemetry_fsm);

        // Create TdispInt FSM
        let tdisp_int_fsm = AdminFsm::TdispInt(TdispIntFsm::new(ctx.clone()));
        let tdisp_int_tag = scheduler.alloc_static(tdisp_int_fsm);

        // Create Stop Interface FSM
        let stop_interface_fsm = AdminFsm::StopInterface(StopInterfaceFsm::new(ctx.clone()));
        let stop_interface_tag = scheduler.alloc_static(stop_interface_fsm);

        // Create AES GCM Extension FSM
        let aes_gcm_ext_fsm = AdminFsm::AesGcmExt(AesGcmExtFsm::new(ctx.clone()));
        let aes_gcm_ext_tag = scheduler.alloc_static(aes_gcm_ext_fsm);

        // IMPORTANT: ensure that STATIC matches the number of alloc_static calls.

        Self {
            admin_to_fp_ipc_channel: env.admin_to_fp_ipc_channel().clone(),
            hsm_ipc_channel: env.hsm_ipc_channel().clone(),
            admin_to_hsp_ipc_channel: env.admin_to_hsp_ipc_channel().clone(),
            ctx,
            scheduler,
            pcie_tag: pcie_tag.unwrap(),
            vflr_tag: vflr_tag.unwrap(),
            cntrl_tag: cntrl_tag.unwrap(),
            doe_tag: doe_tag.unwrap(),
            idfu_tag: idfu_tag.unwrap(),
            cast_tag: self_test_fsm_tag.unwrap(),
            telemetry_tag: telemetry_tag.unwrap(),
            tdisp_int_tag: tdisp_int_tag.unwrap(),
            stop_interface_tag: stop_interface_tag.unwrap(),
            aes_gcm_ext_tag: aes_gcm_ext_tag.unwrap(),
            soft_event_round_robin: 0,
        }
    }

    /// Handle event
    ///
    /// # Arguments
    ///
    /// * `event` - Event to handle
    pub fn on_event(&mut self, event: AdminFsmEvent) {
        match event {
            AdminFsmEvent::PciePerstUp | AdminFsmEvent::PciePerstDown | AdminFsmEvent::PcieFlr => {
                self.on_pcie_event(event)
            }
            AdminFsmEvent::PcieVflr(_) => self.on_pcie_vflr(event),
            AdminFsmEvent::DoeFsmInit | AdminFsmEvent::Doe(_) => self.on_doe_event(event),
            AdminFsmEvent::Ide(_) => self.on_ide_event(event),
            AdminFsmEvent::FpToAdminIpcResponse => self.on_fp_to_admin_ipc_response(),
            AdminFsmEvent::FpResetComplete => self.on_fp_ipc_reset_event(),
            AdminFsmEvent::HsmIpcResponse => self.on_hsm_ipc(),
            AdminFsmEvent::HsmResetComplete => self.on_hsm_ipc_reset_event(),
            AdminFsmEvent::HspToAdminIpcResponse => self.on_hsp_to_admin_ipc_response(),
            AdminFsmEvent::HspToAdminIpcRequest => self.on_hsp_to_admin_ipc_request(),
            AdminFsmEvent::CntrlStateChange(_) | AdminFsmEvent::Nssr(_) => {
                self.on_cntrl_state_change(event)
            }
            AdminFsmEvent::RxReady => self.on_request_ready(),
            AdminFsmEvent::TxComplete => self.on_response_complete(),
            AdminFsmEvent::DmaComplete => self.on_dma_complete(),
            AdminFsmEvent::ResourceReady(_) => unreachable!(),
            AdminFsmEvent::StartCmd => unreachable!(),
            AdminFsmEvent::TimerElapsed => self.on_timer_elapsed(),
            AdminFsmEvent::ShutdownRequest(_) => self.on_idfu_event(event),
            AdminFsmEvent::SchedulerQueueEmptyEvent => self.on_idfu_event(event),
            AdminFsmEvent::IoCancellationComplete => self.on_io_cancellation_complete(event),
            AdminFsmEvent::HsmToAdminIpcRequest => self.on_hsm_to_admin_ipc(),
            AdminFsmEvent::SoftAesRequest => self.on_soft_aes_request(),
            AdminFsmEvent::SelfTestResponse => self.on_self_test_response(),
            AdminFsmEvent::NegativeSelfTest(_) => unreachable!(),
            AdminFsmEvent::TdispInt(_) => self.on_tdisp_interrupt_event(event),
            AdminFsmEvent::AesGcmExtRequest => self.on_aes_gcm_ext_request(event),
            AdminFsmEvent::HspToAdminStopInterfaceIpcRequest => {
                self.on_hsp_to_admin_stop_interface_ipc_request()
            }
            AdminFsmEvent::StopInterfaceRequest(_) => self.on_stop_interface_request(event),

            AdminFsmEvent::Unknown => {}
        }
    }

    /// Check for pending soft events with priority and round-robin fairness.
    /// IoCancellationComplete and SelfTestResponse are highest priority.
    /// Lower-priority events use round-robin to prevent starvation.
    pub fn check_soft_events(&mut self) -> Option<AdminFsmEvent> {
        if !self.ctx.queue_delete_notification().is_empty() {
            Some(AdminFsmEvent::IoCancellationComplete)
        } else if !self.ctx.self_test_resp().is_empty() {
            Some(AdminFsmEvent::SelfTestResponse)
        } else {
            // Round-robin for lower-priority soft events.
            //
            // Policy: when both queues have work, dispatch GCM 5× per 1 SoftAes.
            // Rationale: GCM tag-correction is cheap per event; SoftAes is expensive
            // per event. A 5:1 dispatch ratio drains GCM queue depth fast while
            // SoftAes still drains at >= producer rate because each dispatch covers
            // substantial work. Both paths share the same 6s host timeout.
            const GCM_WEIGHT: u8 = 5;
            const KWP_WEIGHT: u8 = 1;
            const CYCLE: u8 = GCM_WEIGHT + KWP_WEIGHT;

            let gcm_req_queue_empty = self.ctx.aes_gcm_req_queue().is_empty();
            let soft_aes_req_empty = self.ctx.soft_aes_req().is_empty();

            match (gcm_req_queue_empty, soft_aes_req_empty) {
                (false, false) => {
                    // Both have work — apply weighted RR.
                    // Slots [0..GCM_WEIGHT) in the cycle dispatch GCM; the last
                    // slot dispatches SoftAes.
                    let event = if self.soft_event_round_robin < GCM_WEIGHT {
                        AdminFsmEvent::AesGcmExtRequest
                    } else {
                        AdminFsmEvent::SoftAesRequest
                    };
                    // Advance and wrap at the cycle boundary. Counter is always in
                    // [0, CYCLE), so the comparison is direct (no mod at read),
                    // there is no u8 overflow risk, and no parity drift across wraps.
                    // Tick the counter ONLY when we actually arbitrate, so parity
                    // tracks dispatch history, not poll-loop calls into this function.
                    self.soft_event_round_robin = (self.soft_event_round_robin + 1) % CYCLE;
                    Some(event)
                }
                (true, false) => Some(AdminFsmEvent::SoftAesRequest),
                (false, true) => Some(AdminFsmEvent::AesGcmExtRequest),
                (true, true) => None,
            }
        }
    }

    /// Handle GCM request for tag correction
    fn on_aes_gcm_ext_request(&mut self, event: AdminFsmEvent) {
        self.scheduler.on_event(event, self.aes_gcm_ext_tag)
    }

    /// Populate AES GCM IVs
    pub fn fill_aes_gcm_iv_queue(&mut self) {
        // Check if the queue is already full
        if !self.ctx.aes_gcm_iv_queue().is_full() {
            let empty_slot_count = self.ctx.aes_gcm_iv_queue().empty_slot_count();

            for _ in 0..empty_slot_count {
                let mut gcm_iv: AesGcmIV = AesGcmIV::default();
                self.ctx.rng().bytes(&mut gcm_iv.iv);
                if self.ctx.aes_gcm_iv_queue().send(gcm_iv).is_err() {
                    break;
                }
            }
        }
    }

    /// Handle PCIe reset and link down
    fn on_pcie_event(&mut self, event: AdminFsmEvent) {
        self.scheduler.on_event(event, self.pcie_tag);
        self.scheduler.on_event(event, self.doe_tag);
        self.scheduler.on_event(event, self.telemetry_tag);
        self.scheduler.on_event(event, self.tdisp_int_tag);
    }

    /// Handle PCIe vFLR
    fn on_pcie_vflr(&mut self, event: AdminFsmEvent) {
        self.scheduler.on_event(event, self.vflr_tag);
        self.scheduler.on_event(event, self.tdisp_int_tag);
    }

    /// Handle DOE event
    fn on_doe_event(&mut self, event: AdminFsmEvent) {
        self.scheduler.on_event(event, self.doe_tag)
    }

    /// Handle IDE event
    fn on_ide_event(&mut self, event: AdminFsmEvent) {
        let report_status = PcieIde::event_handler();
        if report_status.is_some() {
            self.scheduler.on_event(event, self.tdisp_int_tag);
        }
    }

    /// Handle TDISP Interrupt event
    fn on_tdisp_interrupt_event(&mut self, event: AdminFsmEvent) {
        self.scheduler.on_event(event, self.tdisp_int_tag)
    }

    // Handle Stop Interface event
    fn on_stop_interface_request(&mut self, event: AdminFsmEvent) {
        self.scheduler.on_event(event, self.stop_interface_tag)
    }

    // Handle IDFU event
    fn on_idfu_event(&mut self, event: AdminFsmEvent) {
        self.scheduler.on_event(event, self.idfu_tag)
    }

    /// Handle control state change
    fn on_cntrl_state_change(&mut self, event: AdminFsmEvent) {
        self.scheduler.on_event(event, self.cntrl_tag)
    }

    /// Handle FP to Admin IPC Message Response
    fn on_fp_to_admin_ipc_response(&mut self) {
        if let Some(tag) = self.admin_to_fp_ipc_channel.peek_tag() {
            self.scheduler
                .on_event(AdminFsmEvent::FpToAdminIpcResponse, tag)
        }
    }

    /// Handle FP IPC reset event for FLR
    fn on_fp_ipc_reset_event(&mut self) {
        if let Some(tag) = self.ctx.fp_ipc_event_channel().peek_tag() {
            self.scheduler.on_event(AdminFsmEvent::FpResetComplete, tag)
        }
    }

    /// Handle HSM IPC Message
    fn on_hsm_ipc(&mut self) {
        if let Some(tag) = self.hsm_ipc_channel.peek_tag() {
            self.scheduler.on_event(AdminFsmEvent::HsmIpcResponse, tag)
        }
    }

    /// Handle HSM IPC reset event for FLR
    fn on_hsm_ipc_reset_event(&mut self) {
        if let Some(tag) = self.ctx.hsm_ipc_event_channel().peek_tag() {
            self.scheduler
                .on_event(AdminFsmEvent::HsmResetComplete, tag)
        }
    }

    /// Handle CP Admin to HSP IPC Message
    fn on_hsp_to_admin_ipc_response(&mut self) {
        if let Some(tag) = self.admin_to_hsp_ipc_channel.peek_tag() {
            self.scheduler
                .on_event(AdminFsmEvent::HspToAdminIpcResponse, tag)
        }
    }

    /// Handle HSP IPC Request Message
    fn on_hsp_to_admin_ipc_request(&mut self) {
        let _ = self.handle_hsp_ipc_request_message();
    }

    fn on_hsp_to_admin_stop_interface_ipc_request(&mut self) {
        let _ = self.handle_hsp_stop_interface_ipc_request_message();
    }

    /// Handle HSM to Admin IPC Request Message
    fn on_hsm_to_admin_ipc(&mut self) {
        let _ = self.handle_hsm_to_admin_ipc_message();
    }

    /// Handle IPC request message from HSM
    fn handle_hsm_to_admin_ipc_message(&mut self) -> McrResult<()> {
        let message = self
            .ctx
            .hsm_to_admin_ipc_channel()
            .receive_message()
            .ok_or(AdminErr::SpuriousIpcMessage)
            .map_err(|_: AdminErr| {
                debug!("Spurious Admin IPC message");
                AdminErr::SpuriousIpcMessage
            })?;

        let header = IpcMessageDecoder::decode_header(&message).inspect_err(|_err| {
            error!("Invalid IPC message header");
        })?;

        let op_code = IpcMessageOpCode::try_from(header.msg_op() as u8).map_err(|_| {
            error!("Cannot decode opcode. Invalid IPC message opcode");
            let _ = self.send_hsm_ipc_response(message, IpcMessageStatusCode::MessageNotSupported);
            AdminErr::InvalidIpcMessageOpcode
        })?;

        match op_code {
            IpcMessageOpCode::TriggerCrash => self.handle_trigger_crash(message),
            IpcMessageOpCode::TriggerStackValidation => {
                self.handle_trigger_stack_validation(message)
            }
            IpcMessageOpCode::NegativeSelfTest => self.handle_negative_self_test(message),
            #[cfg(feature = "mcr_manual_test_hooks")]
            IpcMessageOpCode::TdispInterrupt => self.handle_tdisp_interrupt_request(message),
            _ => {
                error!("Opcode does not match. Invalid IPC message opcode");
                self.send_hsm_ipc_response(message, IpcMessageStatusCode::MessageNotSupported)?;
                Err(AdminErr::InvalidIpcMessageOpcode.into())
            }
        }
    }

    fn handle_trigger_crash(&mut self, message: IpcMessage) -> McrResult<()> {
        let message_payload = IpcMessageDecoder::decode::<IpcMessageTriggerCrash>(message)
            .inspect_err(|_err| {
                error!("Invalid IPC message");
            })?;

        self.send_hsm_ipc_response(message, IpcMessageStatusCode::Success)?;

        match message_payload.crash_type {
            CrashType::ExplicitCrash => crashdump_trigger(DdiTestActionCrashType::ExplicitCrash),
            CrashType::HardFault => crashdump_trigger(DdiTestActionCrashType::HardFault),
            CrashType::Hang => crashdump_trigger(DdiTestActionCrashType::Hang),
            _ => crashdump_trigger(DdiTestActionCrashType::Panic),
        }

        Ok(())
    }

    fn handle_trigger_stack_validation(&mut self, message: IpcMessage) -> McrResult<()> {
        let message_payload = IpcMessageDecoder::decode::<IpcMessageTriggerStackValidation>(
            message,
        )
        .inspect_err(|_err| {
            error!("Invalid IPC message for TriggerStackValidation");
        })?;

        let error_type = match message_payload.error_type {
            StackErrorType::StackOverflow => DdiTestStackErrorType::StackOverflow,
            StackErrorType::StackGuardViolation => DdiTestStackErrorType::StackGuardViolation,
            _ => {
                error!("Invalid stack error type");
                self.send_hsm_ipc_response(message, IpcMessageStatusCode::MessageNotSupported)?;
                return Ok(());
            }
        };

        self.send_hsm_ipc_response(message, IpcMessageStatusCode::Success)?;

        trigger_stack_validation(error_type);

        Ok(())
    }

    fn handle_negative_self_test(&mut self, message: IpcMessage) -> McrResult<()> {
        let message_payload = IpcMessageDecoder::decode::<IpcMessageNegSelfTestReq>(message)
            .inspect_err(|_err| {
                error!("Invalid IPC message for Negative self test.");
            })?;

        let status = match message_payload.test_id.try_into() {
            Ok(test) => {
                if test != SelfTest::SelfTestCompleted {
                    self.scheduler
                        .on_event(AdminFsmEvent::NegativeSelfTest(test), self.cast_tag);
                }
                IpcMessageStatusCode::Success
            }
            Err(_) => {
                error!("Invalid test ID in negative self test IPC payload.");

                IpcMessageStatusCode::MessageNotSupported
            }
        };

        self.send_hsm_ipc_response(message, status)
    }

    #[cfg(feature = "mcr_manual_test_hooks")]
    fn handle_tdisp_interrupt_request(&mut self, message: IpcMessage) -> McrResult<()> {
        let message_payload = IpcMessageDecoder::decode::<IpcMessageTdispInterrupt>(message)?;

        let res = self.send_hsm_ipc_response(message, IpcMessageStatusCode::Success);

        match message_payload.info.source {
            InterruptSource::Tdisp => {
                self.scheduler.on_event(
                    AdminFsmEvent::TdispInt(TdispIntInfo {
                        vf_mask: message_payload.info.vf_mask[0] as u64
                            | (message_payload.info.vf_mask[1] as u64) << 32,
                        pf_mask: message_payload.info.pf_mask == 0x1,
                        info_regs: message_payload.info.reg_values,
                    }),
                    self.tdisp_int_tag,
                );
            }
            InterruptSource::Ide => {
                self.scheduler
                    .on_event(AdminFsmEvent::Ide(Some(0)), self.tdisp_int_tag);
            }
            InterruptSource::Flr => {
                if message_payload.info.pf_mask == 0x1 {
                    self.scheduler
                        .on_event(AdminFsmEvent::PcieFlr, self.tdisp_int_tag);
                }

                if message_payload.info.vf_mask[0] > 0 || message_payload.info.vf_mask[1] > 0 {
                    self.scheduler.on_event(
                        AdminFsmEvent::PcieVflr(
                            message_payload.info.vf_mask[0] as u64
                                | (message_payload.info.vf_mask[1] as u64) << 32,
                        ),
                        self.tdisp_int_tag,
                    );
                }
            }
            InterruptSource::PerstUp => {
                self.scheduler
                    .on_event(AdminFsmEvent::PciePerstUp, self.tdisp_int_tag);
            }
            InterruptSource::PerstDown => {
                self.scheduler
                    .on_event(AdminFsmEvent::PciePerstDown, self.tdisp_int_tag);
            }
            _ => unreachable!(),
        };

        res
    }

    fn on_io_cancellation_complete(&self, event: AdminFsmEvent) {
        let message = self.ctx.queue_delete_notification().peek();
        if let Some(queue_delete_req) = message {
            self.scheduler.on_event(event, queue_delete_req.tag)
        }
    }

    /// Handle IPC request message from HSP
    fn handle_hsp_ipc_request_message(&mut self) -> McrResult<()> {
        let message = self
            .ctx
            .hsp_to_admin_ipc_channel()
            .receive_message()
            .ok_or(AdminErr::SpuriousIpcMessage)
            .map_err(|_: AdminErr| {
                debug!("Spurious Admin IPC message");
                AdminErr::SpuriousIpcMessage
            })?;

        let header = IpcMessageDecoder::decode_header(&message).map_err(|_| {
            error!("Invalid Header found in IPC message");
            AdminErr::InvalidIpcHeader
        })?;

        let op_code = IpcMessageOpCode::try_from(header.msg_op() as u8).map_err(|_| {
            let _ =
                Self::handle_invalid_message_opcode(self.ctx.hsp_to_admin_ipc_channel(), message);
            AdminErr::InvalidIpcMessageOpcode
        })?;

        match op_code {
            IpcMessageOpCode::Shutdown => self.handle_shutdown_for_reset_req(header, message),
            _ => Self::handle_invalid_message_opcode(self.ctx.hsp_to_admin_ipc_channel(), message),
        }
    }

    fn handle_hsp_stop_interface_ipc_request_message(&mut self) -> McrResult<()> {
        let message = self
            .ctx
            .hsp_to_admin_stop_interface_ipc_channel()
            .receive_message()
            .ok_or(AdminErr::SpuriousIpcMessage)
            .map_err(|_| {
                debug!("Spurious Admin IPC message");
                AdminErr::SpuriousIpcMessage
            })?;

        let header =
            IpcMessageDecoder::decode_header(&message).map_err(|_| AdminErr::InvalidIpcHeader)?;

        let op_code = IpcMessageOpCode::try_from(header.msg_op() as u8).map_err(|_| {
            let _ = Self::handle_invalid_message_opcode(
                self.ctx.hsp_to_admin_stop_interface_ipc_channel(),
                message,
            );
            AdminErr::InvalidIpcMessageOpcode
        })?;

        match op_code {
            IpcMessageOpCode::StopInterface => self.handle_stop_interface_req(header, message),
            _ => Self::handle_invalid_message_opcode(
                self.ctx.hsp_to_admin_stop_interface_ipc_channel(),
                message,
            ),
        }
    }

    /// Handle Shutdown for reset request message
    fn handle_shutdown_for_reset_req(
        &mut self,
        header: IpcMessageHeader,
        message: IpcMessage,
    ) -> McrResult<()> {
        let message = IpcMessageDecoder::decode::<IpcMessageShutdown>(message).map_err(|_| {
            error!("Invalid Shutdown for reset request message");
            Self::send_hsp_ipc_response(
                self.ctx.hsp_to_admin_ipc_channel(),
                AdminEventHandler::<E>::prepare_shutdown_response(
                    header,
                    IpcMessageStatusCode::InvalidField,
                )
                .encode(),
            );

            AdminErr::InvalidIpcShutdownRequest
        })?;

        self.on_event(AdminFsmEvent::ShutdownRequest((
            message.info,
            message.header.tag(),
        )));
        Ok(())
    }

    /// Prepare response for StopInterface request
    fn prepare_stop_interface_response(
        header: IpcMessageHeader,
        status: IpcMessageStatusCode,
    ) -> IpcMessageStopInterface {
        let mut response = IpcMessageStopInterface {
            header,
            ..Default::default()
        };
        response.header.set_response(true);
        response.header.set_status(status.into());

        response
    }

    fn handle_stop_interface_req(
        &mut self,
        header: IpcMessageHeader,
        message: IpcMessage,
    ) -> McrResult<()> {
        let stop_interface_request = IpcMessageDecoder::decode::<IpcMessageStopInterface>(message)
            .map_err(|_| {
                Self::send_hsp_ipc_response(
                    self.ctx.hsp_to_admin_stop_interface_ipc_channel(),
                    Self::prepare_stop_interface_response(
                        header,
                        IpcMessageStatusCode::InvalidField,
                    )
                    .encode(),
                );

                AdminErr::InvalidStopInterfaceRequest
            })?;

        let bits = stop_interface_request.info.vf_mask[0] as u128
            | (stop_interface_request.info.vf_mask[1] as u128) << 32
            | (stop_interface_request.info.pf_mask as u128) << 64;

        self.on_event(AdminFsmEvent::StopInterfaceRequest((
            bits,
            stop_interface_request.header.tag(),
        )));
        Ok(())
    }

    /// Send invalid message response to HSP
    fn handle_invalid_message_opcode(
        channel: &E::IpcChannel,
        message: IpcMessage,
    ) -> McrResult<()> {
        let mut header = IpcMessageDecoder::decode_header(&message).inspect_err(|err| {
            error!(
                "handle_invalid_message_opcode: Invalid IPC message header {:?}",
                *err
            );
        })?;

        warn!("Unhandled message with opcode {:#X}", header.msg_op());
        header.set_status(IpcMessageStatusCode::MessageNotSupported.into());
        header.set_response(true);
        let mut message = message;
        message.data[0] = header.into();
        Self::send_hsp_ipc_response(channel, message);

        Ok(())
    }

    /// Send an encoded IPC message to HSP
    fn send_hsp_ipc_response(channel: &E::IpcChannel, message: IpcMessage) {
        let _ = channel.send_response(message).map_err(|err| {
            error!("Failed to send response to HSP core {:?}", err);
        });
    }

    /// Send an encoded IPC message to HSM
    fn send_hsm_ipc_response(
        &mut self,
        message: IpcMessage,
        status: IpcMessageStatusCode,
    ) -> McrResult<()> {
        let mut header = IpcMessageDecoder::decode_header(&message).inspect_err(|err| {
            error!(
                "send_hsm_ipc_response: Invalid IPC message header {:?}",
                *err
            );
        })?;

        header.set_status(status.into());
        header.set_response(true);
        let mut message = message;
        message.data[0] = header.into();
        let _ = self
            .ctx
            .hsm_to_admin_ipc_channel()
            .send_response(message)
            .map_err(|err| {
                error!("Failed to send response to hsm core {:?}", err);
            });

        Ok(())
    }

    /// Prepare response for Shutdown request
    fn prepare_shutdown_response(
        header: IpcMessageHeader,
        status: IpcMessageStatusCode,
    ) -> IpcMessageShutdown {
        let mut response = IpcMessageShutdown {
            header,
            ..Default::default()
        };
        response.header.set_response(true);
        response.header.set_status(status.into());

        response
    }

    /// Request ready handler
    fn on_request_ready(&mut self) {
        // Allocate new fsm in scheduler
        if let Some(tag) = self
            .scheduler
            .alloc(AdminFsm::AdminCmd(AdminCmdFsm::new(self.ctx.clone())))
        {
            self.scheduler.on_event(AdminFsmEvent::RxReady, tag)
        }
    }

    /// Response complete handler
    fn on_response_complete(&mut self) {
        if let Some(tag) = self.ctx.io_channel().peek_tag() {
            self.scheduler.on_event(AdminFsmEvent::TxComplete, tag)
        }
    }

    /// DMA complete handler
    fn on_dma_complete(&mut self) {
        if let Some(tag) = self.ctx.dma_channel().peek_tag() {
            self.scheduler.on_event(AdminFsmEvent::DmaComplete, tag)
        }
    }

    /// Timer elapsed handler
    fn on_timer_elapsed(&mut self) {
        self.ctx.update_core_liveliness();

        self.scheduler.on_tick(AdminFsmEvent::TimerElapsed);
    }

    /// On new SoftAes request from IO core
    fn on_soft_aes_request(&mut self) {
        if let Some(req) = self.ctx.soft_aes_req().recv() {
            let key_range: IoMemRange = req.key;

            let inout_range: IoMemRange = req.inout;

            let key = mcr_mem_map::mem_addr_to_slice(key_range.addr() as usize, key_range.len());
            let inout =
                mcr_mem_map::mem_addr_to_slice(inout_range.addr() as usize, inout_range.len());

            let result = match req.op {
                SoftAesOp::Kwp => self.ctx.soft_aes().key_unwrap_inplace(key, inout),
                SoftAesOp::EcbDecrypt => self.ctx.soft_aes().ecb_decrypt(key, inout),
            };

            let range = match result {
                Ok(r) => Ok((r.start, r.end)),
                Err(e) => Err(e),
            };

            if let Err(e) = self.ctx.soft_aes_resp().send(SoftAesOffloadResp {
                range,
                tag: req.tag,
            }) {
                // Enter crash handler
                panic!("Failure: Send SoftAes offloading response {:?}", e);
            }
        }
    }

    /// Handle self test response
    fn on_self_test_response(&mut self) {
        self.scheduler
            .on_event(AdminFsmEvent::SelfTestResponse, self.cast_tag);
    }
}

#[cfg(test)]
mod tests {
    use hex_literal::hex;
    use mcr_crypto_softaes::SoftAes;
    use mcr_pcie_controller::TdispIntInfo;
    use mcr_self_test::SelfTestRespPacket;
    use mcr_types::AesGcmReqEntry;
    use mcr_types::AesGcmRespEntry;
    use mcr_types::PcieFunction;
    use mcr_types::QueueDeleteResponse;
    use mcr_types::SoftAesOffloadReq;

    use super::*;
    use crate::fsm::AdminFsmTest;
    use crate::mock::*;
    use crate::recorder::AdminFsmEventRecorder;

    #[derive(Default, Copy, Clone)]
    struct SoftAesReqParam {
        key_addr: usize,
        key_len: usize,
        inout_addr: usize,
        inout_len: usize,
        op: SoftAesOp,
    }

    /// Admin Handler unit testing configuations
    #[derive(Default)]
    struct AdminHandlerTestConfigs {
        /// Test case to handle Perst up event
        perst_up: bool,

        /// Test case to handle FP IPC response event
        fp_ipc_response: bool,

        /// Test case to handle spurious FP IPC response
        spurious_fp_ipc_response: bool,

        /// Test case to handle FP IPC event response event
        fp_ipc_event_reset_response: bool,

        /// Test case to handle spurious FP IPC event response event
        spurious_fp_ipc_event_reset_response: bool,

        /// Test case to handle HSM IPC response event
        hsm_ipc_response: bool,

        /// Test case to handle spurious HSM IPC response event
        spurious_hsm_ipc_response: bool,

        /// Test case to handle HSM IPC event response event
        hsm_ipc_event_reset_response: bool,

        /// Test case to handle spurious HSM IPC event response event
        spurious_hsm_ipc_event_reset_response: bool,

        /// Test case to trigger TDISP interrupt
        tdisp_interrupt_triggered: bool,

        /// Test case to handle HSP TDISP Interrupt IPC response event
        hsp_tdisp_interrupt_ipc_response: bool,

        /// Test case to handle spurious HSP TDISP Interrupt IPC response event
        spurious_hsp_tdisp_interrupt_ipc_response: bool,

        /// Test case to handle HSP IPC response event
        hsp_ipc_response: bool,

        /// Test case to handle spurious HSP IPC response event
        spurious_hsp_ipc_response: bool,

        /// Test case to handle HSP IPC request message
        hsp_ipc_request: bool,

        /// Test case to handle HSP Stop Interface IPC request message
        hsp_stop_interface_ipc_request: bool,

        /// IPC message opcode
        ipc_message_opcode: IpcMessageOpCode,

        /// Inject invalid opcode
        invalid_opcode_infusion: bool,

        /// Inject spurious message
        spurious_hsp_ipc_request_message: bool,

        /// Inject spurious message
        spurious_hsp_stop_interface_ipc_request_message: bool,

        /// Set invalid shutdown request message
        invalid_shutdown_request_message: bool,

        /// Set invalid ipc request message header
        invalid_ipc_request_header: bool,

        /// Set invalid stop interface request message
        invalid_stop_interface_request_message: bool,

        /// Test case to handle Rx Ready event
        rx_ready: bool,

        /// Test case to handle Tx Complete event
        tx_complete: bool,

        /// Test case to handle spurious Tx Complete event
        spurious_tx_complete: bool,

        /// Test case to handle DMA Complete event
        dma_complete: bool,

        /// Test case to handle spurious DMA Complete event
        spurious_dma_complete: bool,

        /// Test case to include IDFU normal state IPC requests
        idfu_handle_normal_ipc_requests: bool,

        /// IoCancellationComplete
        io_cancellation_complete: bool,

        /// Spurious IoCancellationComplete
        spurious_io_cancellation: bool,

        /// Soft AES Request
        soft_aes_request: bool,

        /// Soft AES Request parameter
        soft_aes_req_param: SoftAesReqParam,

        /// AES Key Unwrap Spurious Request
        aes_unwrap_spurious_request: bool,

        /// Handler to check for soft events
        check_soft_events: bool,

        /// Non-empty Queue delete notification event
        non_empty_queue_delete_notification: bool,

        /// Non-empty AES unwrap request event
        non_empty_aes_unwrap_request: bool,

        /// AES unwrap send failure
        aes_unwrap_send_failure: bool,

        /// timer elapsed event
        timer_elapsed: bool,

        /// Hsm to Admin IPC Request
        hsm_to_admin_ipc_request: bool,

        /// Self test response
        no_empty_self_test_response: bool,

        /// Populate GCM IV queue
        gcm_iv_queue_populate: bool,

        /// Number of empty slots to report from the GCM IV queue
        gcm_iv_empty_slots: usize,

        /// Make the first send() call return an error
        gcm_iv_send_failure: bool,

        /// Non-empty AES GCM extension request event
        non_empty_aes_gcm_request: bool,
    }

    /// Configure GCM IV queue and RNG expectations
    fn configure_gcm_iv_queue_on(
        env: &mut MockAdminEnvTrait,
        empty_slots: usize,
        send_failure: bool,
    ) {
        let mut aes_gcm_resp_queue = MockSimplexPipe::new();

        aes_gcm_resp_queue
            .expect_is_full()
            .times(0..=1)
            .return_const(empty_slots == 0);

        if empty_slots != 0 {
            aes_gcm_resp_queue
                .expect_empty_slot_count()
                .times(0..=1)
                .return_const(empty_slots);

            if send_failure {
                aes_gcm_resp_queue
                    .expect_send()
                    .times(0..=1)
                    .returning(|_msg| Err(u32::MAX));
            } else {
                aes_gcm_resp_queue
                    .expect_send()
                    .times(0..=empty_slots)
                    .return_const(Ok(()));
            }
        }
        env.expect_aes_gcm_iv_queue()
            .return_const(aes_gcm_resp_queue);

        let mut rng = MockRng::new();
        let rng_calls = if send_failure { 1 } else { empty_slots };
        rng.expect_bytes().times(0..=rng_calls).return_const(());
        env.expect_rng().return_const(rng);
    }

    /// Create a mock env with expectations to test handler module.
    fn handler_test_env_hsp_req_ipc(config: AdminHandlerTestConfigs) -> MockAdminEnvTrait {
        let mut env = MockAdminEnvTrait::new();
        let ipc_message = prepare_ipc_message(&config);

        env.expect_clone().times(1).returning(move || {
            let mut test = AdminFsmTest::default();

            // Create AdminFsmContext
            test.admin_to_fp_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(move || {
                    let mut channel = MockIpcMessageChannel::new();
                    if config.idfu_handle_normal_ipc_requests {
                        channel.expect_send_request().once().return_const(Ok(()));
                    }

                    if config.hsp_stop_interface_ipc_request
                        && !config.spurious_hsp_stop_interface_ipc_request_message
                        && !config.invalid_stop_interface_request_message
                        && !config.invalid_ipc_request_header
                        && !config.invalid_opcode_infusion
                    {
                        channel.expect_send_request().once().return_const(Ok(()));
                    }

                    channel
                });

            test.hsm_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(move || {
                    let mut channel = MockIpcMessageChannel::new();
                    if config.idfu_handle_normal_ipc_requests {
                        channel.expect_send_request().once().return_const(Ok(()));
                    }

                    if config.hsp_stop_interface_ipc_request
                        && !config.spurious_hsp_stop_interface_ipc_request_message
                        && !config.invalid_stop_interface_request_message
                        && !config.invalid_ipc_request_header
                        && !config.invalid_opcode_infusion
                    {
                        channel.expect_send_request().once().return_const(Ok(()));
                    }

                    channel
                });

            test.admin_to_hsp_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(move || {
                    let mut channel = MockIpcMessageChannel::new();

                    if config.tdisp_interrupt_triggered {
                        channel.expect_send_request().once().return_const(Ok(()));
                    }

                    channel
                });
            if config.fp_ipc_event_reset_response {
                test.fp_ipc_event_channel()
                    .expect_peek_tag()
                    .times(1)
                    .returning(move || {
                        if config.spurious_fp_ipc_event_reset_response {
                            None
                        } else {
                            Some(3)
                        }
                    });
            }
            if config.hsm_ipc_event_reset_response {
                test.hsm_ipc_event_channel()
                    .expect_peek_tag()
                    .times(1)
                    .returning(move || {
                        if config.spurious_hsm_ipc_event_reset_response {
                            None
                        } else {
                            Some(3)
                        }
                    });
            }
            if config.tx_complete {
                test.io_channel()
                    .expect_peek_tag()
                    .times(1)
                    .returning(move || {
                        if config.spurious_tx_complete {
                            None
                        } else {
                            Some(3)
                        }
                    });
            }
            if config.dma_complete {
                test.dma_channel()
                    .expect_peek_tag()
                    .times(1)
                    .returning(move || {
                        if config.spurious_dma_complete {
                            None
                        } else {
                            Some(3)
                        }
                    });
            }

            // Self:hsp_to_admin_ipc_channel
            if config.hsp_ipc_request {
                test.hsp_to_admin_ipc_channel()
                    .expect_receive_message()
                    .times(1)
                    .returning(move || {
                        if config.spurious_hsp_ipc_request_message {
                            None
                        } else {
                            Some(ipc_message)
                        }
                    });
                if !config.spurious_hsp_ipc_request_message
                    && !config.idfu_handle_normal_ipc_requests
                {
                    test.hsp_to_admin_ipc_channel()
                        .expect_send_response()
                        .times(1)
                        .returning(|_| Ok(()));
                }
            }

            // Self:hsp_to_admin_stop_interface_ipc_channel
            if config.hsp_stop_interface_ipc_request {
                test.hsp_to_admin_stop_interface_ipc_channel()
                    .expect_receive_message()
                    .once()
                    .returning(move || {
                        if config.spurious_hsp_stop_interface_ipc_request_message {
                            None
                        } else {
                            Some(ipc_message)
                        }
                    });

                if config.invalid_ipc_request_header
                    || config.invalid_opcode_infusion
                    || config.invalid_stop_interface_request_message
                {
                    test.hsp_to_admin_stop_interface_ipc_channel()
                        .expect_send_response()
                        .once()
                        .returning(|_| Ok(()));
                }
            }

            let mut env = test.env();

            if config.gcm_iv_queue_populate {
                configure_gcm_iv_queue_on(
                    &mut env,
                    config.gcm_iv_empty_slots,
                    config.gcm_iv_send_failure,
                );
            }

            // Create AdminFsm::Pcie
            env.expect_clone().times(1).returning(move || {
                let mut test = AdminFsmTest::default();
                test.pcie_cntrl()
                    .expect_link_status()
                    .times(1)
                    .returning(|| Err(u32::MAX));

                if config.perst_up {
                    test.pcie_cntrl()
                        .expect_perst_up()
                        .times(1)
                        .returning(|| Err(u32::MAX));
                }

                test.env()
            });

            // Create VFLR FSM
            env.expect_clone()
                .times(1)
                .returning(MockAdminEnvTrait::new);

            // Create Controller FSM
            env.expect_clone()
                .times(1)
                .returning(MockAdminEnvTrait::new);

            // Create DOE FSM
            env.expect_clone().times(1).returning(move || {
                let mut test = AdminFsmTest::default();
                test.pcie_cntrl()
                    .expect_link_status()
                    .times(1)
                    .returning(|| Err(u32::MAX));

                if config.perst_up {
                    test.pcie_doe().expect_set_busy().once().return_const(());
                }

                test.env()
            });

            // Create IDFU FSM
            env.expect_clone().times(1).returning(move || {
                let mut test = AdminFsmTest::default();
                test.io_controller()
                    .expect_pause_inbound()
                    .once()
                    .return_const(());
                test.fp_io_controller()
                    .expect_pause_inbound()
                    .once()
                    .return_const(());
                test.pcie_doe().expect_set_busy().once().return_const(());

                let mut env = test.env();
                env.expect_tcon_tsc().return_const(0u64);
                env.expect_pause_queue_controller().return_const(());
                env.expect_resume_queue_controller().return_const(());

                if config.gcm_iv_queue_populate {
                    configure_gcm_iv_queue_on(
                        &mut env,
                        config.gcm_iv_empty_slots,
                        config.gcm_iv_send_failure,
                    );
                }

                env
            });

            // Create CAST FSM
            env.expect_clone().times(1).returning(|| {
                let mut test = AdminFsmTest::default();
                test.env()
            });

            // Create Telemetry FSM
            env.expect_clone().times(1).returning(move || {
                let mut test = AdminFsmTest::default();
                test.pcie_cntrl()
                    .expect_link_status()
                    .times(1)
                    .returning(|| Err(u32::MAX));

                if config.perst_up {
                    test.pcie_cntrl()
                        .expect_perst_up()
                        .times(1)
                        .returning(|| Err(u32::MAX));
                }

                test.env()
            });

            // Create TDISP Interrupt FSM
            env.expect_clone().times(1).returning(|| {
                let mut test = AdminFsmTest::default();
                test.env()
            });

            // Create Stop Interface FSM
            env.expect_clone().times(1).returning(move || {
                let mut test = AdminFsmTest::default();
                test.env()
            });

            // Create AesGcmExt FSM
            env.expect_clone().times(1).returning(move || {
                let mut test = AdminFsmTest::default();
                let mut env = test.env();
                if config.non_empty_aes_gcm_request {
                    let mut aes_gcm_req_queue: MockSimplexPipe<AesGcmReqEntry> =
                        MockSimplexPipe::new();
                    aes_gcm_req_queue
                        .expect_recv()
                        .once()
                        .returning(|| Some(AesGcmReqEntry::default()));
                    aes_gcm_req_queue
                        .expect_is_empty()
                        .once()
                        .return_const(true);
                    let mut aes_gcm_resp_queue: MockSimplexPipe<AesGcmRespEntry> =
                        MockSimplexPipe::new();
                    aes_gcm_resp_queue.expect_send().once().return_const(Ok(()));
                    env.expect_aes_gcm_resp_queue()
                        .once()
                        .return_const(aes_gcm_resp_queue);
                    env.expect_aes_gcm_req_queue()
                        .once()
                        .return_const(aes_gcm_req_queue);
                }
                env
            });

            if config.rx_ready {
                env.expect_clone().times(1).returning(|| {
                    let mut env = MockAdminEnvTrait::new();
                    let mut io_channel = MockIoChannel::new();
                    io_channel.expect_begin_recv().times(1).returning(|| None);

                    env.expect_io_channel().times(1).return_const(io_channel);

                    env
                });
            }

            // Self::hsm_to_admin_ipc_channel
            if config.hsm_to_admin_ipc_request {
                let mut hsm_to_admin_ipc_channel = MockIpcMessageChannel::new();
                hsm_to_admin_ipc_channel
                    .expect_receive_message()
                    .times(1)
                    .returning(move || {
                        if config.spurious_io_cancellation {
                            None
                        } else {
                            Some(ipc_message)
                        }
                    });

                hsm_to_admin_ipc_channel
                    .expect_send_response()
                    .times(1)
                    .returning(|_| Ok(()));

                env.expect_hsm_to_admin_ipc_channel()
                    .times(2)
                    .return_const(hsm_to_admin_ipc_channel);
            }

            if config.io_cancellation_complete {
                let mut queue_delete_notification = MockSimplexPipe::new();
                queue_delete_notification
                    .expect_peek()
                    .once()
                    .returning(move || {
                        if config.spurious_io_cancellation {
                            None
                        } else {
                            Some(QueueDeleteResponse {
                                tag: 3,
                                pfn: PcieFunction::Pf,
                                _rsvd: 0,
                            })
                        }
                    });
                env.expect_deferred_queue_delete_pipe()
                    .once()
                    .return_const(queue_delete_notification);
            }

            if config.soft_aes_request {
                if !config.aes_unwrap_spurious_request {
                    let mut soft_aes_resp = MockSimplexPipe::new();
                    soft_aes_resp.expect_send().once().returning(move |_| {
                        if config.aes_unwrap_send_failure {
                            Err(u32::MAX)
                        } else {
                            Ok(())
                        }
                    });
                    env.expect_soft_aes_resp()
                        .once()
                        .return_const(soft_aes_resp);

                    let mut soft_aes = MockSoftAes::new();

                    match config.soft_aes_req_param.op {
                        SoftAesOp::Kwp => {
                            soft_aes.expect_key_unwrap_inplace().once().returning(
                                move |kek: &[u8], input: &mut [u8]| {
                                    let soft_aes = SoftAes::new();
                                    soft_aes.key_unwrap_inplace(kek, input)
                                },
                            );
                        }
                        SoftAesOp::EcbDecrypt => {
                            soft_aes.expect_ecb_decrypt().once().returning(
                                move |kek: &[u8], input: &mut [u8]| {
                                    let soft_aes = SoftAes::new();
                                    soft_aes.ecb_decrypt(kek, input)
                                },
                            );
                        }
                    }
                    env.expect_soft_aes().once().return_const(soft_aes);
                }

                let mut aes_unwrap_request = MockSimplexPipe::new();
                aes_unwrap_request.expect_recv().once().returning(move || {
                    if config.aes_unwrap_spurious_request {
                        None
                    } else {
                        let kek: &[u8] = mcr_mem_map::mem_addr_to_slice(
                            config.soft_aes_req_param.key_addr,
                            config.soft_aes_req_param.key_len,
                        );
                        let inout: &[u8] = mcr_mem_map::mem_addr_to_slice(
                            config.soft_aes_req_param.inout_addr,
                            config.soft_aes_req_param.inout_len,
                        );
                        Some(SoftAesOffloadReq {
                            key: kek.into(),
                            inout: inout.into(),
                            op: config.soft_aes_req_param.op,
                            tag: 0,
                        })
                    }
                });

                env.expect_soft_aes_req()
                    .once()
                    .return_const(aes_unwrap_request);
            }

            if config.check_soft_events {
                let mut queue_delete_notification: MockSimplexPipe<QueueDeleteResponse> =
                    MockSimplexPipe::new();
                queue_delete_notification
                    .expect_is_empty()
                    .once()
                    .return_const(!config.non_empty_queue_delete_notification);
                env.expect_deferred_queue_delete_pipe()
                    .once()
                    .return_const(queue_delete_notification);

                if !config.non_empty_queue_delete_notification {
                    let mut self_test_resp: MockSimplexPipe<SelfTestRespPacket> =
                        MockSimplexPipe::new();
                    self_test_resp
                        .expect_is_empty()
                        .once()
                        .return_const(!config.no_empty_self_test_response);

                    env.expect_self_test_resp()
                        .once()
                        .return_const(self_test_resp);

                    if !config.no_empty_self_test_response {
                        let mut aes_gcm_req_queue: MockSimplexPipe<AesGcmReqEntry> =
                            MockSimplexPipe::new();
                        aes_gcm_req_queue
                            .expect_is_empty()
                            .once()
                            .return_const(!config.non_empty_aes_gcm_request);
                        env.expect_aes_gcm_req_queue()
                            .once()
                            .return_const(aes_gcm_req_queue);

                        let mut aes_unwrap_request: MockSimplexPipe<SoftAesOffloadReq> =
                            MockSimplexPipe::new();
                        aes_unwrap_request
                            .expect_is_empty()
                            .once()
                            .return_const(!config.non_empty_aes_unwrap_request);
                        env.expect_soft_aes_req()
                            .once()
                            .return_const(aes_unwrap_request);
                    }
                }
            }

            if config.timer_elapsed {
                env.expect_update_core_liveliness()
                    .times(1)
                    .return_const(());
            }

            env
        });

        // Self::admin_to_fp_ipc_channel
        let mut admin_to_fp_ipc_channel = MockIpcMessageChannel::new();
        admin_to_fp_ipc_channel
            .expect_clone()
            .times(1)
            .returning(move || {
                let mut ipc_channel = MockIpcMessageChannel::new();
                if config.fp_ipc_response {
                    ipc_channel.expect_peek_tag().times(1).returning(move || {
                        if config.spurious_fp_ipc_response {
                            None
                        } else {
                            Some(2)
                        }
                    });
                }

                ipc_channel
            });
        env.expect_admin_to_fp_ipc_channel()
            .times(1)
            .return_const(admin_to_fp_ipc_channel);

        // Self::hsm_ipc_channel
        let mut hsm_ipc_channel = MockIpcMessageChannel::new();
        hsm_ipc_channel.expect_clone().times(1).returning(move || {
            let mut ipc_channel = MockIpcMessageChannel::new();
            if config.hsm_ipc_response {
                ipc_channel.expect_peek_tag().times(1).returning(move || {
                    if config.spurious_hsm_ipc_response {
                        None
                    } else {
                        Some(2)
                    }
                });
            }

            ipc_channel
        });
        env.expect_hsm_ipc_channel()
            .times(1)
            .return_const(hsm_ipc_channel);

        // Self:admin_to_hsp_ipc_channel
        let mut admin_to_hsp_ipc_channel = MockIpcMessageChannel::new();
        admin_to_hsp_ipc_channel
            .expect_clone()
            .times(1)
            .returning(move || {
                let mut ipc_channel = MockIpcMessageChannel::new();
                if config.hsp_ipc_response || config.hsp_tdisp_interrupt_ipc_response {
                    ipc_channel.expect_peek_tag().times(1).returning(move || {
                        if config.spurious_hsp_ipc_response
                            || config.spurious_hsp_tdisp_interrupt_ipc_response
                        {
                            None
                        } else {
                            Some(2)
                        }
                    });
                }

                ipc_channel
            });
        env.expect_admin_to_hsp_ipc_channel()
            .times(1)
            .return_const(admin_to_hsp_ipc_channel);

        if config.gcm_iv_queue_populate {
            configure_gcm_iv_queue_on(
                &mut env,
                config.gcm_iv_empty_slots,
                config.gcm_iv_send_failure,
            );
        }

        env
    }

    /// Prepare IPC Message based on the test configuration
    fn prepare_ipc_message(config: &AdminHandlerTestConfigs) -> IpcMessage {
        if (!config.hsp_ipc_request && !config.hsp_stop_interface_ipc_request)
            || config.invalid_ipc_request_header
            || config.invalid_opcode_infusion
        {
            IpcMessage { data: [0; 16] }
        } else {
            match config.ipc_message_opcode {
                IpcMessageOpCode::Shutdown => {
                    let mut shutdown_for_reset_req = IpcMessageShutdown {
                        info: ShutdownInfo {
                            drain_time_ms: 0x7D0,
                        },
                        ..Default::default()
                    };
                    if config.invalid_shutdown_request_message {
                        shutdown_for_reset_req.info.drain_time_ms = 0xffff;
                    }

                    shutdown_for_reset_req.encode()
                }
                IpcMessageOpCode::StopInterface => {
                    let stop_interface_req = IpcMessageStopInterface {
                        info: StopInterfaceInfo {
                            pf_mask: if config.invalid_stop_interface_request_message {
                                0x1234
                            } else {
                                0x1
                            },
                            ..Default::default()
                        },
                        ..Default::default()
                    };

                    stop_interface_req.encode()
                }
                _ => unreachable!(),
            }
        }
    }

    #[test]
    fn test_handler_on_unknown_event() {
        let config = AdminHandlerTestConfigs::default();
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::Unknown)
    }

    #[test]
    fn test_handler_on_pcie_perst_up_event() {
        let config = AdminHandlerTestConfigs {
            perst_up: true,
            tdisp_interrupt_triggered: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::PciePerstUp)
    }

    #[test]
    fn test_handler_on_pcie_perst_down_event() {
        let config = AdminHandlerTestConfigs {
            tdisp_interrupt_triggered: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::PciePerstDown)
    }

    #[test]
    fn test_handler_on_pcie_flr_event() {
        let config = AdminHandlerTestConfigs {
            tdisp_interrupt_triggered: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::PcieFlr)
    }

    #[test]
    fn test_handler_on_pcie_vflr_event() {
        let config = AdminHandlerTestConfigs {
            tdisp_interrupt_triggered: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::PcieVflr(0))
    }

    #[test]
    fn test_handler_on_tdisp_int_event() {
        let config = AdminHandlerTestConfigs {
            tdisp_interrupt_triggered: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::TdispInt(TdispIntInfo::default()))
    }

    #[test]
    fn test_handler_on_doe_event() {
        let config = AdminHandlerTestConfigs::default();
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::Doe(mcr_doe::DoeEvents::DoeGo))
    }

    #[test]
    fn test_handler_on_cntrl_state_change() {
        let config = AdminHandlerTestConfigs::default();
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::CntrlStateChange(0))
    }

    #[test]
    fn test_handler_on_fp_to_admin_ipc_response() {
        let config = AdminHandlerTestConfigs {
            fp_ipc_response: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::FpToAdminIpcResponse)
    }

    #[test]
    fn test_handler_on_spurious_fp_to_admin_ipc_response() {
        let config = AdminHandlerTestConfigs {
            fp_ipc_response: true,
            spurious_fp_ipc_response: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::FpToAdminIpcResponse)
    }

    #[test]
    fn test_handler_on_fp_ipc_reset_event() {
        let config = AdminHandlerTestConfigs {
            fp_ipc_event_reset_response: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::FpResetComplete)
    }

    #[test]
    fn test_handler_on_spurious_fp_ipc_reset_event() {
        let config = AdminHandlerTestConfigs {
            fp_ipc_event_reset_response: true,
            spurious_fp_ipc_event_reset_response: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::FpResetComplete)
    }

    #[test]
    fn test_handler_on_hsm_ipc_response() {
        let config = AdminHandlerTestConfigs {
            hsm_ipc_response: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HsmIpcResponse)
    }

    #[test]
    fn test_handler_on_spurious_hsm_ipc_response() {
        let config = AdminHandlerTestConfigs {
            hsm_ipc_response: true,
            spurious_hsm_ipc_response: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HsmIpcResponse)
    }

    #[test]
    fn test_handler_on_hsm_ipc_reset_event() {
        let config = AdminHandlerTestConfigs {
            hsm_ipc_event_reset_response: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HsmResetComplete)
    }

    #[test]
    fn test_handler_on_spurious_hsm_ipc_reset_event() {
        let config = AdminHandlerTestConfigs {
            hsm_ipc_event_reset_response: true,
            spurious_hsm_ipc_event_reset_response: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HsmResetComplete)
    }

    #[test]
    fn test_handler_on_hsp_ipc_response() {
        let config = AdminHandlerTestConfigs {
            hsp_ipc_response: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HspToAdminIpcResponse)
    }

    #[test]
    fn test_handler_on_spurious_hsp_ipc_response() {
        let config = AdminHandlerTestConfigs {
            hsp_ipc_response: true,
            spurious_hsp_ipc_response: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HspToAdminIpcResponse)
    }

    #[test]
    fn test_handler_on_hsp_stop_interface_ipc_request() {
        let config = AdminHandlerTestConfigs {
            hsp_stop_interface_ipc_request: true,
            ipc_message_opcode: IpcMessageOpCode::StopInterface,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HspToAdminStopInterfaceIpcRequest)
    }

    #[test]
    fn test_handler_on_hsp_stop_interface_ipc_invalid_opcode_request() {
        let config = AdminHandlerTestConfigs {
            hsp_stop_interface_ipc_request: true,
            ipc_message_opcode: IpcMessageOpCode::StopInterface,
            invalid_opcode_infusion: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HspToAdminStopInterfaceIpcRequest)
    }

    #[test]
    fn test_handler_on_hsp_stop_interface_ipc_request_invalid_payload() {
        let config = AdminHandlerTestConfigs {
            hsp_stop_interface_ipc_request: true,
            ipc_message_opcode: IpcMessageOpCode::StopInterface,
            invalid_stop_interface_request_message: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HspToAdminStopInterfaceIpcRequest)
    }

    #[test]
    fn test_handler_on_hsp_stop_interface_ipc_spurious_request() {
        let config = AdminHandlerTestConfigs {
            hsp_stop_interface_ipc_request: true,
            ipc_message_opcode: IpcMessageOpCode::StopInterface,
            spurious_hsp_stop_interface_ipc_request_message: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HspToAdminStopInterfaceIpcRequest)
    }

    #[test]
    fn test_handler_on_hsp_tdisp_interrupt_ipc_response() {
        let config = AdminHandlerTestConfigs {
            hsp_tdisp_interrupt_ipc_response: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HspToAdminIpcResponse)
    }

    #[test]
    fn test_handler_on_spurious_hsp_tdisp_interrupt_ipc_response() {
        let config = AdminHandlerTestConfigs {
            hsp_tdisp_interrupt_ipc_response: true,
            spurious_hsp_tdisp_interrupt_ipc_response: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HspToAdminIpcResponse)
    }

    #[test]
    fn test_handler_on_hsp_ipc_request_shutdown() {
        let config = AdminHandlerTestConfigs {
            hsp_ipc_request: true,
            ipc_message_opcode: IpcMessageOpCode::Shutdown,
            spurious_hsp_ipc_request_message: false,
            idfu_handle_normal_ipc_requests: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HspToAdminIpcRequest)
    }

    #[test]
    fn test_handler_on_hsp_ipc_invalid_opcode_request() {
        let config = AdminHandlerTestConfigs {
            hsp_ipc_request: true,
            ipc_message_opcode: IpcMessageOpCode::Shutdown,
            invalid_opcode_infusion: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HspToAdminIpcRequest)
    }

    #[test]
    fn test_handler_on_hsp_ipc_request_shutdown_invalid_payload() {
        let config = AdminHandlerTestConfigs {
            hsp_ipc_request: true,
            ipc_message_opcode: IpcMessageOpCode::Shutdown,
            invalid_shutdown_request_message: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HspToAdminIpcRequest)
    }

    #[test]
    fn test_handler_on_hsp_ipc_spurious_request() {
        let config = AdminHandlerTestConfigs {
            hsp_ipc_request: true,
            ipc_message_opcode: IpcMessageOpCode::Shutdown,
            spurious_hsp_ipc_request_message: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HspToAdminIpcRequest)
    }

    #[test]
    fn test_handler_on_request_ready() {
        let config = AdminHandlerTestConfigs {
            rx_ready: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::RxReady)
    }

    #[test]
    fn test_handler_on_tx_complete() {
        let config = AdminHandlerTestConfigs {
            tx_complete: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::TxComplete)
    }

    #[test]
    fn test_handler_on_spurious_tx_complete() {
        let config = AdminHandlerTestConfigs {
            tx_complete: true,
            spurious_tx_complete: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::TxComplete)
    }

    #[test]
    fn test_handler_on_dma_complete() {
        let config = AdminHandlerTestConfigs {
            dma_complete: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::DmaComplete)
    }

    #[test]
    fn test_handler_on_spurious_dma_complete() {
        let config = AdminHandlerTestConfigs {
            dma_complete: true,
            spurious_dma_complete: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::DmaComplete)
    }

    #[test]
    fn test_handler_on_timer_elapsed() {
        let config = AdminHandlerTestConfigs {
            timer_elapsed: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);
        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::TimerElapsed);
    }

    #[test]
    fn test_handler_on_io_cancellation_complete() {
        let config = AdminHandlerTestConfigs {
            io_cancellation_complete: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::IoCancellationComplete)
    }

    #[test]
    fn test_handler_on_spurious_io_cancellation_complete() {
        let config = AdminHandlerTestConfigs {
            io_cancellation_complete: true,
            spurious_io_cancellation: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::IoCancellationComplete)
    }

    #[test]
    fn test_handler_on_aes_ecb_decrypt_request() {
        let inout = &hex!("f3eed1bdb5d2a03c 064b5a7e3db181f8 591ccb10d410ed26 dc5ba74a31362870 b6ed21b99ca6f4f9 f153e7b1beafed1d 23304b7a39f9f3ff 067d8d8f9e24ecc7");
        let key = &hex!("603deb1015ca71be 2b73aef0857d7781 1f352c073b6108d7 2d9810a30914dff4");

        let mut inout_vec = vec![0u8; inout.len()];
        inout_vec.copy_from_slice(inout);

        let mut key_vec = vec![0u8; key.len()];
        key_vec.copy_from_slice(key);

        let soft_aes_req_param = SoftAesReqParam {
            key_addr: key_vec.as_ptr() as usize,
            key_len: key_vec.len(),
            inout_addr: inout_vec.as_ptr() as usize,
            inout_len: inout_vec.len(),
            op: SoftAesOp::EcbDecrypt,
        };
        let config = AdminHandlerTestConfigs {
            soft_aes_request: true,
            soft_aes_req_param,
            ..Default::default()
        };

        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::SoftAesRequest)
    }

    #[test]
    fn test_handler_on_aes_unwrap_request() {
        let inout = &hex!("138bdeaa9b8fa7fc 61f97742e72248ee 5ae6ae5360d1ae6a 5f54f373fa543b6a");
        let kek = &hex!("5840df6e29b02af1 ab493b705bf16ea1 ae8338f4dcc176a8");

        let mut inout_vec = vec![0u8; inout.len()];
        inout_vec.copy_from_slice(inout);

        let mut kek_vec = vec![0u8; kek.len()];
        kek_vec.copy_from_slice(kek);

        let soft_aes_req_param = SoftAesReqParam {
            key_addr: kek_vec.as_ptr() as usize,
            key_len: kek_vec.len(),
            inout_addr: inout_vec.as_ptr() as usize,
            inout_len: inout_vec.len(),
            op: SoftAesOp::Kwp,
        };
        let config = AdminHandlerTestConfigs {
            soft_aes_request: true,
            soft_aes_req_param,
            ..Default::default()
        };

        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::SoftAesRequest)
    }

    #[test]
    fn test_handler_on_aes_unwrap_spurious_request() {
        let inout = &hex!("138bdeaa9b8fa7fc 61f97742e72248ee 5ae6ae5360d1ae6a 5f54f373fa543b6a");
        let kek = &hex!("5840df6e29b02af1 ab493b705bf16ea1 ae8338f4dcc176a8");

        let mut inout_vec = vec![0u8; inout.len()];
        inout_vec.copy_from_slice(inout);

        let mut kek_vec = vec![0u8; kek.len()];
        kek_vec.copy_from_slice(kek);

        let soft_aes_req_param = SoftAesReqParam {
            key_addr: kek_vec.as_ptr() as usize,
            key_len: kek_vec.len(),
            inout_addr: inout_vec.as_ptr() as usize,
            inout_len: inout_vec.len(),
            op: SoftAesOp::Kwp,
        };
        let config = AdminHandlerTestConfigs {
            soft_aes_request: true,
            soft_aes_req_param,
            aes_unwrap_spurious_request: true,
            ..Default::default()
        };

        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::SoftAesRequest)
    }

    #[test]
    fn test_handler_on_aes_unwrap_request_invalid_kek_len() {
        let inout = &hex!("138bdeaa9b8fa7fc 61f97742e72248ee 5ae6ae5360d1ae6a 5f54f373fa543b6a");
        let kek = &hex!("5840df6e29b02af1");

        let mut inout_vec = vec![0u8; inout.len()];
        inout_vec.copy_from_slice(inout);

        let mut kek_vec = vec![0u8; kek.len()];
        kek_vec.copy_from_slice(kek);

        let soft_aes_req_param = SoftAesReqParam {
            key_addr: kek_vec.as_ptr() as usize,
            key_len: kek_vec.len(),
            inout_addr: inout_vec.as_ptr() as usize,
            inout_len: inout_vec.len(),
            op: SoftAesOp::Kwp,
        };
        let config = AdminHandlerTestConfigs {
            soft_aes_request: true,
            soft_aes_req_param,
            ..Default::default()
        };

        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::SoftAesRequest)
    }

    #[test]
    #[should_panic]
    fn test_handler_on_aes_unwrap_request_send_response_failure() {
        let inout = &hex!("138bdeaa9b8fa7fc 61f97742e72248ee 5ae6ae5360d1ae6a 5f54f373fa543b6a");
        let kek = &hex!("5840df6e29b02af1 ab493b705bf16ea1 ae8338f4dcc176a8");

        let mut inout_vec = vec![0u8; inout.len()];
        inout_vec.copy_from_slice(inout);

        let mut kek_vec = vec![0u8; kek.len()];
        kek_vec.copy_from_slice(kek);

        let soft_aes_req_param = SoftAesReqParam {
            key_addr: kek_vec.as_ptr() as usize,
            key_len: kek_vec.len(),
            inout_addr: inout_vec.as_ptr() as usize,
            inout_len: inout_vec.len(),
            op: SoftAesOp::Kwp,
        };
        let config = AdminHandlerTestConfigs {
            soft_aes_request: true,
            soft_aes_req_param,
            aes_unwrap_send_failure: true,
            ..Default::default()
        };

        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::SoftAesRequest)
    }

    #[test]
    fn test_handler_check_soft_event_none() {
        let config = AdminHandlerTestConfigs {
            check_soft_events: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        assert!(handler.check_soft_events().is_none());
    }

    #[test]
    fn test_handler_check_queue_delete_soft_event() {
        let config = AdminHandlerTestConfigs {
            check_soft_events: true,
            non_empty_queue_delete_notification: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        assert!(handler.check_soft_events() == Some(AdminFsmEvent::IoCancellationComplete));
    }

    #[test]
    fn test_handler_check_aes_unwrap_soft_event() {
        let config = AdminHandlerTestConfigs {
            check_soft_events: true,
            non_empty_aes_unwrap_request: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        assert!(handler.check_soft_events() == Some(AdminFsmEvent::SoftAesRequest));
    }

    #[test]
    fn test_handler_on_hsm_to_admin_ipc_request_crashdump() {
        let config = AdminHandlerTestConfigs {
            hsm_to_admin_ipc_request: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::HsmToAdminIpcRequest)
    }

    #[test]
    fn test_handler_self_test_response() {
        let config = AdminHandlerTestConfigs {
            check_soft_events: true,
            no_empty_self_test_response: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());
        assert!(handler.check_soft_events() == Some(AdminFsmEvent::SelfTestResponse));

        handler.on_event(AdminFsmEvent::SelfTestResponse);
    }

    #[test]
    fn test_fill_aes_gcm_iv_queue_populate_success() {
        let config = AdminHandlerTestConfigs {
            gcm_iv_queue_populate: true,
            gcm_iv_empty_slots: 3,
            gcm_iv_send_failure: false,
            ..Default::default()
        };

        let env = handler_test_env_hsp_req_ipc(config);
        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.fill_aes_gcm_iv_queue();
    }

    #[test]
    fn test_fill_aes_gcm_iv_queue_already_full() {
        let config = AdminHandlerTestConfigs {
            gcm_iv_queue_populate: true,
            gcm_iv_empty_slots: 0,
            gcm_iv_send_failure: false,
            ..Default::default()
        };

        let env = handler_test_env_hsp_req_ipc(config);
        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.fill_aes_gcm_iv_queue();
    }

    #[test]
    fn test_fill_aes_gcm_iv_queue_send_failure_breaks() {
        let config = AdminHandlerTestConfigs {
            gcm_iv_queue_populate: true,
            gcm_iv_empty_slots: 5,
            gcm_iv_send_failure: true,
            ..Default::default()
        };

        let env = handler_test_env_hsp_req_ipc(config);
        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.fill_aes_gcm_iv_queue();
    }

    #[test]
    fn test_handler_check_aes_gcm_ext_soft_event() {
        let config = AdminHandlerTestConfigs {
            check_soft_events: true,
            non_empty_aes_gcm_request: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        assert!(handler.check_soft_events() == Some(AdminFsmEvent::AesGcmExtRequest));
    }

    #[test]
    fn test_handler_on_aes_gcm_ext_request_event() {
        let config = AdminHandlerTestConfigs {
            non_empty_aes_gcm_request: true,
            ..Default::default()
        };
        let env = handler_test_env_hsp_req_ipc(config);

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());

        handler.on_event(AdminFsmEvent::AesGcmExtRequest)
    }
}
