// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaTrait;
use mcr_error::McrResult;
use mcr_gdma_controller::GdmaChannelTrait;
use mcr_io_controller::IoChannelTrait;
use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_logging::*;
use mcr_mem_map::*;
use mcr_simplex::SimplexPipeTrait;
use mcr_types::DebugLogComponent;
use mcr_types::DebugLogEntryParameters;
use mcr_types::DebugLogSeverity;
use mcr_types::PcieFunction;

use crate::cmd_scheduler::*;
use crate::env::HsmHalTrait;
use crate::error;
use crate::error::HsmErr;
use crate::event::HsmFsmEvent;
use crate::fsm::ComboFsm;
use crate::fsm::HsmFsm;
use crate::fsm::HsmPartInitFsm;
use crate::fsm::HsmResCleanupFsm;
use crate::partition::HsmPartition;
use crate::partition::IoQueueDeleteContext;
use crate::warn;
use crate::HsmEnvTrait;

use alloc::rc::Rc;
use core::cell::RefCell;

/// Hsm Core Event handler
pub(crate) struct HsmEventHandler<E: HsmEnvTrait + 'static> {
    env: Rc<RefCell<E>>,
    scheduler: CmdScheduler<ComboFsm<E>>,
}

impl<E: HsmEnvTrait> Drop for HsmEventHandler<E> {
    fn drop(&mut self) {}
}

impl<E: HsmEnvTrait> HsmEventHandler<E> {
    /// Create an instance of `HsmEventHandler`
    ///
    /// # Arguments
    ///
    /// * `env`      - Environment
    /// * `recorder` - FSM event recorder
    ///
    /// # Returns
    ///
    /// * `HsmEventHandler` - Hsm Event Handler Instance
    pub fn new(env: E, scheduler: CmdScheduler<ComboFsm<E>>) -> Self {
        let env = Rc::new(RefCell::new(env));

        // Setup the Cleanup FSM
        let cleanup_fsm = ComboFsm::new_hsm_res_cleanup_fsm(HsmResCleanupFsm::new(env.clone()));
        let tag = scheduler.alloc_static(cleanup_fsm).unwrap();

        // Wire up the cleanup FSM to PKA engine
        env.borrow_mut().pka_engine().set_cleanup_fsm(tag);

        Self { env, scheduler }
    }

    pub fn env(&self) -> &Rc<RefCell<E>> {
        &self.env
    }

    pub fn scheduler(&self) -> &CmdScheduler<ComboFsm<E>> {
        &self.scheduler
    }

    /// Handle event
    ///
    /// # Arguments
    ///
    /// * `event` - Event to handle
    pub fn on_event(&mut self, event: HsmFsmEvent) {
        match event {
            HsmFsmEvent::RxReady => self.on_request_ready(),
            HsmFsmEvent::TxComplete => self.on_response_complete(),
            HsmFsmEvent::DmaComplete => self.on_dma_complete(),
            HsmFsmEvent::StartCmd => unreachable!(),
            HsmFsmEvent::Flr => self.on_flr(),
            HsmFsmEvent::AdminToHsmIpcRequest => self.on_admin_ipc(),
            HsmFsmEvent::PkaDone(index) => self.on_pka(event, index),
            HsmFsmEvent::PkaError(index) => self.on_pka(event, index),
            HsmFsmEvent::FpToHsmIpcRequest => self.on_fp_to_hsm_ipc_request(),
            HsmFsmEvent::FpToHsmIpcResponse => self.on_fp_to_hsm_ipc_response(event),
            HsmFsmEvent::HspToHsmIpcResponse => self.on_hsp_ipc_response(event),
            HsmFsmEvent::AdminToHsmIpcResponse => self.on_admin_ipc_response(event),
            HsmFsmEvent::ResourceReady(_) => unreachable!(),
            HsmFsmEvent::TimerElapsed => self.on_timer_elapsed(),
            HsmFsmEvent::CheckAlive => {}
            HsmFsmEvent::SoftAesResp => self.on_soft_aes_resp(),
            HsmFsmEvent::SelfTestRequest => self.on_self_test_request(),
            HsmFsmEvent::ResourceCleanup(_, _) => {} // Delivered directly to the scheduler
            HsmFsmEvent::InitPartition(_) => {}      // Delivered directly to the scheduler
            HsmFsmEvent::Unknown => {}
        }
    }

    /// Request ready handler
    fn on_request_ready(&mut self) {
        // Allocate new fsm in scheduler
        if let Some(tag) = self
            .scheduler
            .alloc(ComboFsm::new_hsm_fsm(HsmFsm::new(self.env.clone())))
        {
            self.scheduler.on_event(HsmFsmEvent::RxReady, tag)
        } else {
            error!("on_request_ready: spurious event");
        }
    }

    /// Response complete handler
    fn on_response_complete(&mut self) {
        if let Some(tag) = self.env.borrow().hal().io_channel().peek_tag() {
            self.scheduler.on_event(HsmFsmEvent::TxComplete, tag)
        } else {
            error!("on_response_complete: spurious event");
        }
    }

    /// DMA complete handler
    fn on_dma_complete(&mut self) {
        if let Some(tag) = self.env.borrow().hal().dma_channel().peek_tag() {
            self.scheduler.on_event(HsmFsmEvent::DmaComplete, tag)
        } else {
            error!("on_dma_complete: spurious event");
        }
    }

    /// On FLR request
    fn on_flr(&mut self) {
        if let Some(event) = self
            .env
            .borrow()
            .hal()
            .ipc_event_channel()
            .receive_event(IpcDescriptor::Descriptor28)
        {
            for pfn in PcieFunction::iter() {
                self.env.borrow().partition(pfn).reset();
            }

            if let Err(_e) = self
                .env
                .borrow()
                .hal()
                .ipc_event_channel()
                .end_event(IpcDescriptor::Descriptor29, event)
            {
                error!("Failed to send FLR complete event");
            }
        } else {
            error!("on_flr: spurious event");
        }
    }

    // On receiving IPC from Admin core
    fn on_admin_ipc(&mut self) {
        let _ = self.handle_admin_ipc_message();
    }

    /// Handle IPC message from Admin core
    fn handle_admin_ipc_message(&mut self) -> McrResult<()> {
        let message = self
            .env
            .borrow()
            .hal()
            .admin_ipc_channel()
            .receive_message()
            .ok_or(HsmErr::SpuriousIpcMessageEvent)
            .map_err(|_: HsmErr| HsmErr::SpuriousIpcMessageEvent)?;

        let header = IpcMessageDecoder::decode_header(&message).map_err(|_| {
            error!("Invalid Header found in IPC message");
            HsmErr::IpcMessageDecodeErr
        })?;

        let op_code = IpcMessageOpCode::try_from(header.msg_op() as u8).map_err(|_| {
            let _ = self.handle_invalid_message_opcode(message);
            HsmErr::InvalidMessageOpcode
        })?;

        match op_code {
            IpcMessageOpCode::PfnEnableDisable => self.handle_pfn_enable(header, message),
            IpcMessageOpCode::SetResource => self.handle_set_resource(header, message),
            IpcMessageOpCode::CreateDeleteSq => self.handle_create_delete_sq(header, message),
            IpcMessageOpCode::Shutdown => self.handle_shutdown_for_reset_req(header, message),
            _ => self.handle_invalid_message_opcode(message),
        }
    }

    /// Handle Pcie function enable/disable message
    fn handle_pfn_enable(
        &mut self,
        header: IpcMessageHeader,
        message: IpcMessage,
    ) -> McrResult<()> {
        let message =
            IpcMessageDecoder::decode::<IpcMessagePfnEnableDisable>(message).map_err(|_| {
                error!("Invalid PcieFunction enable/disable message");
                self.send_admin_ipc_response(
                    self.prepare_pfn_en_response(header, IpcMessageStatusCode::InvalidField)
                        .encode(),
                );

                HsmErr::InvalidIpcMessage
            })?;
        let pfn = message.info.pfn;
        let mut status = IpcMessageStatusCode::Success;

        match message.info.action {
            PfnEnableDisableAction::Enable => self.env.borrow().partition(pfn).enable(),
            PfnEnableDisableAction::Disable => {
                if self
                    .env
                    .borrow()
                    .partition(pfn)
                    .disable(Some(IoQueueDeleteContext::new(header.tag() as u16, false)))
                {
                    status = IpcMessageStatusCode::Pending;
                }
            }
            PfnEnableDisableAction::Migrate => {
                let part = self.env.borrow().partition(pfn);
                if part.begin_migrate(Some(IoQueueDeleteContext::new(header.tag() as u16, true))) {
                    status = IpcMessageStatusCode::Pending;
                } else {
                    part.end_migrate();
                }
            }
            _ => status = IpcMessageStatusCode::InvalidField,
        }

        let mut response = self.prepare_pfn_en_response(header, status);
        response.info = message.info;
        self.send_admin_ipc_response(response.encode());

        Ok(())
    }

    /// Handle Set Resource message
    fn handle_set_resource(
        &mut self,
        header: IpcMessageHeader,
        message: IpcMessage,
    ) -> McrResult<()> {
        let set_res_message =
            IpcMessageDecoder::decode::<IpcMessageSetRes>(message).inspect_err(|err| {
                error!("Invalid Set Resource message {:?}", *err);
                let response =
                    self.prepare_set_res_response(header, IpcMessageStatusCode::InvalidField);
                self.send_admin_ipc_response(response.encode());
            })?;

        let mask = u128::from_le_bytes(set_res_message.info.mask);
        let pfn = set_res_message.info.pfn;

        if mask.count_ones() > 0 {
            let part_init_fsm = ComboFsm::new_hsm_part_init_fsm(HsmPartInitFsm::new(
                self.env.clone(),
                self.env.borrow().partition(pfn),
            ));
            match self.scheduler.alloc(part_init_fsm) {
                Some(part_init_fsm_tag) => {
                    // Partition init state machine is going to respond the set resource message when it is done
                    // creating partition identifiers.
                    self.scheduler.on_event(
                        HsmFsmEvent::InitPartition(set_res_message),
                        part_init_fsm_tag,
                    );
                }
                None => {
                    error!("Failed to allocate partition init FSM");
                    let response =
                        self.prepare_set_res_response(header, IpcMessageStatusCode::InvalidField);
                    self.send_admin_ipc_response(response.encode());

                    // Handle the error gracefully without panicking
                    return Err(HsmErr::FsmAllocationFailed.into());
                }
            }
        } else {
            let part = self.env.borrow().partition(pfn);
            part.reset();

            let response = self.prepare_set_res_response(header, IpcMessageStatusCode::Success);
            self.send_admin_ipc_response(response.encode());
        }
        Ok(())
    }

    /// Handle Create/Delete SQ message
    fn handle_create_delete_sq(
        &mut self,
        header: IpcMessageHeader,
        message: IpcMessage,
    ) -> McrResult<()> {
        let message =
            IpcMessageDecoder::decode::<IpcMessageCreateDeleteSq>(message).inspect_err(|err| {
                error!(
                    "Invalid IPC Create Delete Submission Queue Message {:?}",
                    *err
                );
                let response = self
                    .prepare_create_delete_sq_response(header, IpcMessageStatusCode::InvalidField);
                self.send_admin_ipc_response(response.encode());
            })?;
        let dev_sq = message.info.device_sq_id;
        let dev_cq = message.info.device_cq_id;
        let pfn = message.info.pfn;
        let action = message.info.action;
        let mut response =
            self.prepare_create_delete_sq_response(header, IpcMessageStatusCode::Success);

        let part = self.env.borrow().partition(pfn);
        if !part.enabled() {
            error!("{:?} not Enabled", u32::from(pfn));
            let response = self.prepare_create_delete_sq_response(
                header,
                IpcMessageStatusCode::FunctionNotEnabled,
            );
            self.send_admin_ipc_response(response.encode());
            Err(HsmErr::InvalidIpcMessage)?
        }

        #[allow(unused_mut)]
        let mut delete_ctx = Some(IoQueueDeleteContext::new(
            response.header.tag() as u16,
            false,
        ));

        #[cfg(feature = "mcr_test_hooks")]
        if action == SqAction::Delete && part.test_hook_to_trigger_level2_abort() {
            delete_ctx = None;
        }

        if action == SqAction::Create {
            part.enable_io_queue(dev_sq, dev_cq);
        } else if part.disable_io_queue(dev_sq, delete_ctx) {
            response
                .header
                .set_status(IpcMessageStatusCode::Pending.into());
        }

        #[cfg(feature = "mcr_test_hooks")]
        if action == SqAction::Delete && part.test_hook_to_trigger_level2_abort() {
            response
                .header
                .set_status(IpcMessageStatusCode::OperationFailed.into());
        }

        response.info = message.info;
        self.send_admin_ipc_response(response.encode());

        Ok(())
    }

    /// Handle Shutdown for reset request message
    fn handle_shutdown_for_reset_req(
        &mut self,
        header: IpcMessageHeader,
        message: IpcMessage,
    ) -> McrResult<()> {
        let _message = IpcMessageDecoder::decode::<IpcMessageShutdown>(message).map_err(|_| {
            error!("Invalid Shutdown for reset request message");
            self.send_admin_ipc_response(
                Self::prepare_shutdown_response(header, IpcMessageStatusCode::InvalidField)
                    .encode(),
            );

            HsmErr::InvalidIpcShutdownRequest
        })?;

        let env = self.env.borrow().clone();

        let idfu_drain_closure = move || {
            env.prepare_for_shutdown();

            #[cfg(not(feature = "idfu_fault_pre_reset_hsm_shutdown_ipc_no_resp_err"))]
            let message =
                Self::prepare_shutdown_response(header, IpcMessageStatusCode::Success).encode();

            #[cfg(feature = "idfu_fault_pre_reset_hsm_shutdown_ipc_no_resp_err")]
            {
                warn!("iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_no_resp_err");
            }
            #[cfg(not(feature = "idfu_fault_pre_reset_hsm_shutdown_ipc_no_resp_err"))]
            let _ = env
                .hal()
                .admin_ipc_channel()
                .send_response(message)
                .map_err(|err| {
                    error!(
                        "Failed to send prepare shutdown response to Admin core {:?}",
                        err
                    );
                });
        };

        if self.scheduler.drain(idfu_drain_closure).is_err() {
            let message =
                Self::prepare_shutdown_response(header, IpcMessageStatusCode::OperationFailed)
                    .encode();

            self.send_admin_ipc_response(message);
            error!("Schedule is busy in draining IOs");
        }

        Ok(())
    }

    /// Prepare response for Shutdown request
    fn prepare_shutdown_response(
        header: IpcMessageHeader,
        _status: IpcMessageStatusCode,
    ) -> IpcMessageShutdown {
        let mut response = IpcMessageShutdown {
            header,
            ..Default::default()
        };
        response.header.set_response(true);

        #[cfg(feature = "idfu_fault_pre_reset_hsm_shutdown_ipc_resp_err")]
        {
            warn!("iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_resp_err");
            response
                .header
                .set_status(IpcMessageStatusCode::OperationFailed.into());
        }

        #[cfg(feature = "idfu_fault_pre_reset_hsm_drain_timeout")]
        {
            warn!("iDFU Fault Injected: idfu_fault_pre_reset_hsm_drain_timeout");
            response
                .header
                .set_status(IpcMessageStatusCode::OperationTimeout.into());
        }

        #[cfg(not(any(
            feature = "idfu_fault_pre_reset_hsm_shutdown_ipc_resp_err",
            feature = "idfu_fault_pre_reset_hsm_drain_timeout"
        )))]
        response.header.set_status(_status.into());

        response
    }

    /// Send invalid message response to Admin
    fn handle_invalid_message_opcode(&mut self, message: IpcMessage) -> McrResult<()> {
        let mut header = IpcMessageDecoder::decode_header(&message).inspect_err(|err| {
            error!("Invalid IPC message header {:?}", *err);
        })?;

        warn!("Unhandled message with opcode {:#X}", header.msg_op());
        header.set_status(IpcMessageStatusCode::MessageNotSupported.into());
        header.set_response(true);
        let mut message = message;
        message.data[0] = header.into();
        self.send_admin_ipc_response(message);

        Ok(())
    }

    /// Send an encoded IPC message to Admin core
    fn send_admin_ipc_response(&self, message: IpcMessage) {
        let _ = self
            .env
            .borrow()
            .hal()
            .admin_ipc_channel()
            .send_response(message)
            .map_err(|err| {
                error!("Failed to send response to Admin core {:?}", err);
            });
    }

    fn prepare_create_delete_sq_response(
        &self,
        header: IpcMessageHeader,
        status: IpcMessageStatusCode,
    ) -> IpcMessageCreateDeleteSq {
        let mut response = IpcMessageCreateDeleteSq {
            header,
            ..Default::default()
        };
        response.header.set_response(true);
        response.header.set_status(status.into());

        response
    }

    fn prepare_set_res_response(
        &self,
        header: IpcMessageHeader,
        status: IpcMessageStatusCode,
    ) -> IpcMessageSetRes {
        let mut response = IpcMessageSetRes {
            header,
            ..Default::default()
        };
        response.header.set_response(true);
        response.header.set_status(status.into());

        response
    }

    fn prepare_pfn_en_response(
        &self,
        header: IpcMessageHeader,
        status: IpcMessageStatusCode,
    ) -> IpcMessagePfnEnableDisable {
        let mut response = IpcMessagePfnEnableDisable {
            header,
            ..Default::default()
        };
        response.header.set_response(true);
        response.header.set_status(status.into());

        response
    }

    /// PKA event handler
    fn on_pka(&mut self, event: HsmFsmEvent, idx: usize) {
        if let Some(tag) = self.env.borrow().hal().pka()[idx].peek_tag() {
            self.scheduler.on_event(event, tag)
        } else {
            error!("on_pka: spurious event. Index: {}", idx as u32);
        }
    }

    // On receiving IPC from FP core
    fn on_fp_to_hsm_ipc_request(&mut self) {
        let _ = self.handle_fp_to_hsm_ipc_message();
    }

    /// Handle IPC message from FP core
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Result
    fn handle_fp_to_hsm_ipc_message(&mut self) -> McrResult<()> {
        let message = self
            .env
            .borrow()
            .hal()
            .fp_to_hsm_ipc_channel()
            .receive_message()
            .ok_or(HsmErr::SpuriousIpcMessageEvent)
            .map_err(|_: HsmErr| HsmErr::SpuriousIpcMessageEvent)?;

        let header = IpcMessageDecoder::decode_header(&message).map_err(|_| {
            error!("Invalid Header found for fp to hsm IPC message");
            HsmErr::IpcMessageDecodeErr
        })?;

        let op_code = IpcMessageOpCode::try_from(header.msg_op() as u8).map_err(|_| {
            let _ = self.handle_fp_to_hsm_invalid_opcode(message);
            HsmErr::InvalidMessageOpcode
        })?;

        match op_code {
            IpcMessageOpCode::CdmaEccErr => self.handle_cdma_vault_ecc_error(header, message),
            _ => self.handle_fp_to_hsm_invalid_opcode(message),
        }
    }

    /// Handle CDMA Vault ECC error
    ///
    /// # Arguments
    ///
    /// * `header`  - IPC message header
    /// * `message` - IPC message
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Result
    fn handle_cdma_vault_ecc_error(
        &self,
        header: IpcMessageHeader,
        message: IpcMessage,
    ) -> McrResult<()> {
        let mut message =
            IpcMessageDecoder::decode::<IpcMessageCdmaErr>(message).inspect_err(|_| {
                let mut response = IpcMessageCdmaErr {
                    header,
                    ..Default::default()
                };
                response.header.set_response(true);
                response
                    .header
                    .set_status(IpcMessageStatusCode::InvalidField.into());

                self.send_fp_to_hsm_ipc_response(response.encode());
            })?;

        // Reload key vault memory to reset any existing ECC errors
        let kv_len = CdmaMemMap::key_vault().len();
        let cdma_key_vault_addr = self.env.borrow().hal().cdma_vault_addr();

        let dst: &mut [u32] = mem_addr_to_slice(cdma_key_vault_addr, kv_len);

        for v in dst.iter_mut() {
            let val = *v;
            unsafe {
                core::ptr::write_volatile(v, val);
            }
        }

        error!("Correctable ECC errors have exceeded threshold, Key vault memory reloaded");

        // send response to fp: modify the message to be a response and send it back to FP
        message
            .header
            .set_status(IpcMessageStatusCode::Success.into());
        message.header.set_response(true);
        self.send_fp_to_hsm_ipc_response(message.encode());

        #[cfg(feature = "mcr_test_hooks")]
        {
            // Increment the corrected ECC error interrupt count
            self.env.borrow().hal().set_corr_ecc_err_intr_count(
                self.env
                    .borrow()
                    .hal()
                    .get_corr_ecc_err_intr_count()
                    .unwrap_or(0)
                    + 1,
            )?;
        }

        Ok(())
    }

    /// Send an encoded IPC message to fp core from HSM
    ///
    /// # Arguments
    ///
    /// * `message` - IPC message
    fn send_fp_to_hsm_ipc_response(&self, message: IpcMessage) {
        let _ = self
            .env
            .borrow()
            .hal()
            .fp_to_hsm_ipc_channel()
            .send_response(message)
            .map_err(|err| {
                error!("Failed to send response to FP core {:?}", err);
            });
    }

    fn handle_fp_to_hsm_invalid_opcode(&mut self, message: IpcMessage) -> McrResult<()> {
        #[allow(clippy::manual_inspect)]
        let mut header = IpcMessageDecoder::decode_header(&message).map_err(|err| {
            error!("Invalid op code found in IPC message");
            err
        })?;

        header.set_status(IpcMessageStatusCode::MessageNotSupported.into());
        header.set_response(true);
        let mut message = message;
        message.data[0] = header.into();
        self.send_fp_to_hsm_ipc_response(message);

        Ok(())
    }

    /// On FP IPC response
    fn on_fp_to_hsm_ipc_response(&mut self, event: HsmFsmEvent) {
        if let Some(tag) = self.env.borrow().hal().hsm_to_fp_ipc_channel().peek_tag() {
            self.scheduler.on_event(event, tag)
        } else {
            error!("on_fp_ipc_response: spurious event");
        }
    }

    /// On HSP IPC response
    fn on_hsp_ipc_response(&mut self, event: HsmFsmEvent) {
        if let Some(tag) = self.env.borrow().hal().hsp_ipc_channel().peek_tag() {
            self.scheduler.on_event(event, tag)
        } else {
            error!("on_hsp_ipc_response: spurious event");
        }
    }

    /// On Admin IPC response
    fn on_admin_ipc_response(&mut self, event: HsmFsmEvent) {
        if let Some(tag) = self
            .env
            .borrow()
            .hal()
            .hsm_to_admin_ipc_channel()
            .peek_tag()
        {
            self.scheduler.on_event(event, tag)
        } else {
            error!("on_admin_ipc_response: spurious event");
        }
    }

    /// Timer elapsed handler
    fn on_timer_elapsed(&mut self) {
        self.env.borrow().hal().update_core_liveliness();

        self.scheduler.on_tick(HsmFsmEvent::CheckAlive)
    }

    /// SoftAes response handler
    fn on_soft_aes_resp(&mut self) {
        if let Some(resp) = self.env.borrow().hal().soft_aes_resp().peek() {
            self.scheduler.on_event(HsmFsmEvent::SoftAesResp, resp.tag)
        } else {
            error!("on_aes_unwrap_resp: spurious event");
        }
    }

    /// Self test request handler
    fn on_self_test_request(&mut self) {
        let self_test_req_packet = match self.env.borrow().hal().self_test_req().recv() {
            Some(packet) => packet,
            None => {
                error!("Failed to receive self test request");
                return;
            }
        };

        self.execute_self_test(&self_test_req_packet);
    }
}

#[cfg(test)]
mod tests {
    use mcr_ipc_message::IpcMessageCdmaErr;
    use mcr_types::DevCqId;
    use mcr_types::DevSqId;

    use super::*;
    use crate::mock::*;
    use crate::recorder::HsmFsmEventRecorder;
    use crate::resource::PkaResource;

    #[derive(Default, Copy, Clone, PartialEq, Eq)]
    enum TestPfnAction {
        #[default]
        Enable,

        Disable,

        Migrate,
    }

    impl From<TestPfnAction> for PfnEnableDisableAction {
        fn from(value: TestPfnAction) -> Self {
            match value {
                TestPfnAction::Enable => PfnEnableDisableAction::Enable,
                TestPfnAction::Disable => PfnEnableDisableAction::Disable,
                TestPfnAction::Migrate => PfnEnableDisableAction::Migrate,
            }
        }
    }

    /// Test configurations for admin IPC event handler
    #[derive(Default)]
    struct AdminIpcTestConfigs {
        /// IPC message opcode
        ipc_message_opcode: IpcMessageOpCode,

        /// Function Action
        pfn_action: TestPfnAction,

        /// PCIe Function
        func: u8,

        /// Inject invalid opcode
        invalid_opcode_infusion: bool,

        /// Inject spurious message
        spurious_message: bool,

        /// Queue Action
        action: SqAction,

        /// Resource mask
        resource_mask: u128,

        /// Queue delete pending
        queue_delete_pending: bool,
    }

    /// Test configurations for FLR event handler
    struct FlrTestConfigs {
        /// Inject spurious message
        spurious_message: bool,

        /// Flag indicating whether event ended successfully
        event_ended: bool,
    }

    /// Test configurations for FP->HSM IPC request event handler
    #[derive(Default)]
    struct FpIpcReqTestConfigs {
        /// Inject spurious message
        spurious_message: bool,

        /// Inject invalid opcode
        invalid_opcode_infusion: bool,

        /// Optional IPC message to return from channel
        ipc_message: Option<IpcMessage>,

        /// Optional CDMA vault address to expose via HAL
        cdma_vault_addr: Option<usize>,

        /// Optional expected times `env.hal()` will be called
        env_hal_times: Option<usize>,
        /// Optional configured result for channel send_response (None => Ok(()))
        send_response_result: Option<Result<(), u32>>,
    }

    /// Create a mock env with expectations to test handler module admin IPC event handling.
    fn handler_test_env_admin_ipc(config: AdminIpcTestConfigs) -> MockEnv {
        let mut env = MockEnv::new();

        let ipc_message = if config.invalid_opcode_infusion {
            IpcMessage { data: [0; 16] }
        } else {
            prepare_ipc_message(&config)
        };

        let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
        mock_ipc_message_channel
            .expect_receive_message()
            .once()
            .returning(move || {
                if config.spurious_message {
                    None
                } else {
                    Some(ipc_message)
                }
            });

        let mut hal = MockHal::new();
        hal.expect_admin_ipc_channel()
            .once()
            .return_const(mock_ipc_message_channel);
        env.expect_hal().once().return_const(hal);

        match config.ipc_message_opcode {
            IpcMessageOpCode::Shutdown => {
                env.expect_clone().once().returning(|| {
                    let mut env = MockEnv::new();

                    let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
                    mock_ipc_message_channel
                        .expect_send_response()
                        .once()
                        .returning(|_| Ok(()));

                    let mut hal = MockHal::new();
                    hal.expect_admin_ipc_channel()
                        .once()
                        .return_const(mock_ipc_message_channel);
                    env.expect_hal().once().return_const(hal);

                    env.expect_prepare_for_shutdown().once().returning(|| ());

                    env
                });
            }
            _ => {
                if !config.spurious_message
                    && config.func <= PcieFunction::Pf.into()
                    && !config.invalid_opcode_infusion
                {
                    let mut partition = MockPartition::new();
                    partition.expect_clone().once().returning(move || {
                        let mut part = MockPartition::new();
                        match config.ipc_message_opcode {
                            IpcMessageOpCode::PfnEnableDisable => {
                                if config.pfn_action == TestPfnAction::Enable {
                                    part.expect_enable().once().returning(|| ());
                                } else if config.pfn_action == TestPfnAction::Disable {
                                    part.expect_disable()
                                        .once()
                                        .returning(move |_| config.queue_delete_pending);
                                } else if config.pfn_action == TestPfnAction::Migrate {
                                    part.expect_begin_migrate()
                                        .once()
                                        .returning(move |_| config.queue_delete_pending);
                                    if !config.queue_delete_pending {
                                        part.expect_end_migrate().once().returning(|| ());
                                    }
                                }
                            }
                            IpcMessageOpCode::SetResource => {
                                if config.resource_mask > 0 {
                                    part.expect_set_resource_mask().once().returning(|_| ());
                                    part.expect_set_unwrapping_key_required()
                                        .once()
                                        .returning(|_| ());
                                    part.expect_set_vm_launch_guid().once().returning(|_| ());
                                    part.expect_begin_generate_partition_identifiers()
                                        .once()
                                        .returning(|_| Err(HsmErr::Pending));
                                } else {
                                    part.expect_reset().once().returning(|| ());
                                }
                            }
                            IpcMessageOpCode::CreateDeleteSq => {
                                if config.pfn_action == TestPfnAction::Enable {
                                    part.expect_enabled().once().returning(|| true);
                                    if config.action == SqAction::Create {
                                        part.expect_enable_io_queue().once().returning(|_, _| ());
                                    } else {
                                        part.expect_disable_io_queue()
                                            .once()
                                            .returning(|_, _| false);
                                        #[cfg(feature = "mcr_test_hooks")]
                                        part.expect_test_hook_to_trigger_level2_abort()
                                            .times(2)
                                            .returning(|| false);
                                    }
                                } else {
                                    part.expect_enabled().once().returning(|| false);
                                }
                            }
                            IpcMessageOpCode::Shutdown => {}
                            _ => unreachable!(),
                        }
                        part
                    });
                    env.expect_partition().once().return_const(partition);
                }

                if !config.spurious_message {
                    let mut mock_ipc_message_rsp = MockIpcMessageChannel::new();
                    mock_ipc_message_rsp
                        .expect_send_response()
                        .once()
                        .returning(|_| Ok(()));

                    let mut hal = MockHal::new();
                    hal.expect_admin_ipc_channel()
                        .once()
                        .return_const(mock_ipc_message_rsp);
                    env.expect_hal().once().return_const(hal);
                }
            }
        }

        env
    }

    /// Create a mock env with expectations to test handler module FLR event handling.
    fn handler_test_env_flr(config: FlrTestConfigs) -> MockEnv {
        let mut env = MockEnv::new();

        let mut mock_ipc_event_channel = MockIpcEventChannel::new();
        mock_ipc_event_channel
            .expect_receive_event()
            .once()
            .returning(move |_| {
                if config.spurious_message {
                    None
                } else {
                    Some(0)
                }
            });

        let mut hal = MockHal::new();
        hal.expect_ipc_event_channel()
            .once()
            .return_const(mock_ipc_event_channel);
        env.expect_hal().once().return_const(hal);

        if !config.spurious_message {
            for _ in PcieFunction::iter() {
                let mut partition = MockPartition::new();
                partition.expect_clone().once().returning(move || {
                    let mut part = MockPartition::new();
                    part.expect_reset().once().returning(|| ());
                    part
                });
                env.expect_partition().once().return_const(partition);
            }

            let mut mock_ipc_event_rsp = MockIpcEventChannel::new();
            mock_ipc_event_rsp
                .expect_end_event()
                .once()
                .returning(move |_, _| if config.event_ended { Ok(()) } else { Err(1) });

            let mut hal = MockHal::new();
            hal.expect_ipc_event_channel()
                .once()
                .return_const(mock_ipc_event_rsp);
            env.expect_hal().once().return_const(hal);
        }

        env
    }

    /// Create a mock env with expectations to test handler module FP->HSM IPC request event handling
    fn handler_test_env_fp_ipc_cfg(config: FpIpcReqTestConfigs) -> MockEnv {
        let mut env = MockEnv::new();

        // make local copies so closures won't move the whole `config`
        let spurious = config.spurious_message;
        let invalid_opcode = config.invalid_opcode_infusion;
        let ipc_message = if invalid_opcode {
            IpcMessage { data: [0; 16] }
        } else if let Some(m) = config.ipc_message {
            m
        } else {
            IpcMessage { data: [0; 16] }
        };

        let send_response_result = config.send_response_result;

        let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
        mock_ipc_message_channel
            .expect_receive_message()
            .once()
            .returning(move || if spurious { None } else { Some(ipc_message) });

        if !spurious {
            let expect = mock_ipc_message_channel.expect_send_response().once();
            match send_response_result {
                Some(res) => {
                    expect.returning(move |_| res);
                }
                None => {
                    expect.returning(|_| Ok(()));
                }
            }
        }

        let mut hal = MockHal::new();

        if let Some(addr) = config.cdma_vault_addr {
            hal.expect_cdma_vault_addr().once().return_const(addr);
        }

        hal.expect_fp_to_hsm_ipc_channel()
            .times(2)
            .return_const(mock_ipc_message_channel);

        #[cfg(feature = "mcr_test_hooks")]
        {
            hal.expect_get_corr_ecc_err_intr_count()
                .times(1..)
                .return_const(Some(0));
            hal.expect_set_corr_ecc_err_intr_count()
                .times(1..)
                .returning(|_| Ok(()));
        }

        match config.env_hal_times {
            Some(n) => env.expect_hal().times(n).return_const(hal),
            None => env.expect_hal().times(1..).return_const(hal),
        };

        env
    }

    /// Prepare IPC Message based on the test configuration
    fn prepare_ipc_message(config: &AdminIpcTestConfigs) -> IpcMessage {
        match config.ipc_message_opcode {
            IpcMessageOpCode::PfnEnableDisable => {
                let pfn_enable_disable_message = IpcMessagePfnEnableDisable {
                    info: PfnEnableDisableInfo {
                        pfn: PcieFunction(config.func),
                        action: config.pfn_action.into(),
                    },
                    ..Default::default()
                };
                pfn_enable_disable_message.encode()
            }
            IpcMessageOpCode::SetResource => {
                let resource_mask_bytes = config.resource_mask.to_le_bytes();
                let set_res_message = IpcMessageSetRes {
                    info: SetResInfo {
                        mask: resource_mask_bytes,
                        pfn: PcieFunction(config.func),
                        vm_launch_guid: [0; 16],
                    },
                    ..Default::default()
                };
                set_res_message.encode()
            }
            IpcMessageOpCode::CreateDeleteSq => {
                let create_delete_sq_message = IpcMessageCreateDeleteSq {
                    info: SqCreateDeleteInfo {
                        device_sq_id: DevSqId::Id1,
                        device_cq_id: DevCqId::Id1,
                        action: config.action,
                        pfn: PcieFunction(config.func),
                    },
                    ..Default::default()
                };
                create_delete_sq_message.encode()
            }
            IpcMessageOpCode::Shutdown => {
                let shutdown_req_message = IpcMessageShutdown {
                    info: ShutdownInfo { drain_time_ms: 0 },
                    ..Default::default()
                };
                shutdown_req_message.encode()
            }
            _ => unreachable!(),
        }
    }

    #[test]
    fn test_hsm_handler_admin_ipc_pfn_enable_function() {
        const PCIE_FUNCTION_PF: u8 = 64;
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::PfnEnableDisable,
            pfn_action: TestPfnAction::Enable,
            func: PCIE_FUNCTION_PF,
            invalid_opcode_infusion: false,
            spurious_message: false,
            action: SqAction::Create,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_pfn_migrate_function() {
        const PCIE_FUNCTION_PF: u8 = 64;
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::PfnEnableDisable,
            pfn_action: TestPfnAction::Migrate,
            func: PCIE_FUNCTION_PF,
            invalid_opcode_infusion: false,
            spurious_message: false,
            action: SqAction::Create,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_pfn_migrate_function_delete_pending() {
        const PCIE_FUNCTION_PF: u8 = 64;
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::PfnEnableDisable,
            pfn_action: TestPfnAction::Migrate,
            func: PCIE_FUNCTION_PF,
            invalid_opcode_infusion: false,
            spurious_message: false,
            action: SqAction::Create,
            queue_delete_pending: true,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_pfn_disable_function() {
        const PCIE_FUNCTION_PF: u8 = 64;
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::PfnEnableDisable,
            pfn_action: TestPfnAction::Disable,
            func: PCIE_FUNCTION_PF,
            invalid_opcode_infusion: false,
            spurious_message: false,
            action: SqAction::Create,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_pfn_disable_function_delete_pending() {
        const PCIE_FUNCTION_PF: u8 = 64;
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::PfnEnableDisable,
            pfn_action: TestPfnAction::Disable,
            func: PCIE_FUNCTION_PF,
            invalid_opcode_infusion: false,
            spurious_message: false,
            action: SqAction::Create,
            queue_delete_pending: true,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_pfn_enable_disable_message_decode_fails() {
        const INVALID_PCIE_FUNCTION: u8 = 120;
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::PfnEnableDisable,
            pfn_action: TestPfnAction::Enable,
            func: INVALID_PCIE_FUNCTION,
            invalid_opcode_infusion: false,
            spurious_message: false,
            action: SqAction::Create,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_invalid_opcode() {
        const PCIE_FUNCTION_PF: u8 = 64;
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::PfnEnableDisable,
            pfn_action: TestPfnAction::Enable,
            func: PCIE_FUNCTION_PF,
            invalid_opcode_infusion: true,
            spurious_message: false,
            action: SqAction::Create,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_spurious_message() {
        const INVALID_PCIE_FUNCTION: u8 = 64;
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::PfnEnableDisable,
            pfn_action: TestPfnAction::Enable,
            func: INVALID_PCIE_FUNCTION,
            invalid_opcode_infusion: false,
            spurious_message: true,
            action: SqAction::Create,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_set_res_invalid_pcie_function() {
        const INVALID_PFN: u8 = 200;
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::SetResource,
            pfn_action: TestPfnAction::Enable,
            func: INVALID_PFN,
            invalid_opcode_infusion: false,
            spurious_message: false,
            action: SqAction::Create,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_set_res_n_count() {
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::SetResource,
            resource_mask: 0x3u128,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_set_res_zero_count() {
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::SetResource,
            resource_mask: 0,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_create_delete_function_disabled() {
        const PCIE_FUNCTION_PF: u8 = 64;
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::CreateDeleteSq,
            pfn_action: TestPfnAction::Disable,
            func: PCIE_FUNCTION_PF,
            invalid_opcode_infusion: false,
            spurious_message: false,
            action: SqAction::Delete,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);

        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());

        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_create_delete_sq_disable() {
        const PCIE_FUNCTION_PF: u8 = 64;
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::CreateDeleteSq,
            pfn_action: TestPfnAction::Enable,
            func: PCIE_FUNCTION_PF,
            invalid_opcode_infusion: false,
            spurious_message: false,
            action: SqAction::Delete,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());

        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_create_delete_sq_enable() {
        const PCIE_FUNCTION_PF: u8 = 64;
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::CreateDeleteSq,
            pfn_action: TestPfnAction::Enable,
            func: PCIE_FUNCTION_PF,
            invalid_opcode_infusion: false,
            spurious_message: false,
            action: SqAction::Create,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_create_delete_sq_invalid_pcie_fn() {
        const INVALID_PCIE_FN: u8 = 120;
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::CreateDeleteSq,
            pfn_action: TestPfnAction::Enable,
            func: INVALID_PCIE_FN,
            invalid_opcode_infusion: false,
            spurious_message: false,
            action: SqAction::Create,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_create_delete_sq_invalid_function() {
        const PCIE_FUNCTION_PF: u8 = 64;
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::CreateDeleteSq,
            pfn_action: TestPfnAction::Enable,
            func: PCIE_FUNCTION_PF,
            invalid_opcode_infusion: false,
            spurious_message: false,
            action: SqAction::Create,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_flr() {
        let config = FlrTestConfigs {
            spurious_message: false,
            event_ended: true,
        };

        let mut env = handler_test_env_flr(config);

        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::Flr);
    }

    #[test]
    fn test_hsm_handler_flr_spurious_message() {
        let config = FlrTestConfigs {
            spurious_message: true,
            event_ended: true,
        };

        let mut env = handler_test_env_flr(config);

        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::Flr);
    }

    #[test]
    fn test_hsm_handler_end_event_err() {
        let config = FlrTestConfigs {
            spurious_message: false,
            event_ended: false,
        };

        let mut env = handler_test_env_flr(config);

        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::Flr);
    }

    #[test]
    fn test_hsm_handler_admin_ipc_shutdown_for_reset_req() {
        let config = AdminIpcTestConfigs {
            ipc_message_opcode: IpcMessageOpCode::Shutdown,
            ..Default::default()
        };

        let mut env = handler_test_env_admin_ipc(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::AdminToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_fp_ipc_req_spurious_message() {
        let config = FpIpcReqTestConfigs {
            spurious_message: true,
            ..Default::default()
        };

        let mut env = handler_test_env_fp_ipc_cfg(config);

        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::FpToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_fp_ipc_req_invalid_opcode() {
        let config = FpIpcReqTestConfigs {
            invalid_opcode_infusion: true,
            ..Default::default()
        };

        let mut env = handler_test_env_fp_ipc_cfg(config);

        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::FpToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_fp_ipc_req_cdma_err_decode_fails_response() {
        let mut header = IpcMessageCdmaErr::default().header;
        header.set_length(0);

        let mut msg = IpcMessage { data: [0; 16] };
        msg.data[0] = header.into();

        let config = FpIpcReqTestConfigs {
            spurious_message: false,
            invalid_opcode_infusion: false,
            ipc_message: Some(msg),
            cdma_vault_addr: None,
            env_hal_times: Some(2),
            send_response_result: None,
        };

        let mut env = handler_test_env_fp_ipc_cfg(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::FpToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_fp_ipc_req_send_response_error_is_handled() {
        let mut header = IpcMessageCdmaErr::default().header;
        header.set_length(0);

        let mut msg = IpcMessage { data: [0; 16] };
        msg.data[0] = header.into();

        let config = FpIpcReqTestConfigs {
            spurious_message: false,
            invalid_opcode_infusion: false,
            ipc_message: Some(msg),
            cdma_vault_addr: None,
            env_hal_times: Some(2),
            send_response_result: Some(Err(1)),
        };

        let mut env = handler_test_env_fp_ipc_cfg(config);

        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::FpToHsmIpcRequest);
    }

    #[test]
    fn test_hsm_handler_fp_ipc_req_cdma_ecc_error_reload_kv() {
        let cdma_err = IpcMessageCdmaErr::default();
        let msg = cdma_err.encode();
        let cdma_vault_memory: [u8; 16384] = [0; 16384];

        let env_hal_times = if cfg!(feature = "mcr_test_hooks") {
            Some(5)
        } else {
            Some(3)
        };

        let config = FpIpcReqTestConfigs {
            spurious_message: false,
            invalid_opcode_infusion: false,
            ipc_message: Some(msg),
            cdma_vault_addr: Some(cdma_vault_memory.as_ptr() as usize),
            env_hal_times,
            send_response_result: None,
        };

        let mut env = handler_test_env_fp_ipc_cfg(config);
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();

        for _ in 0..16 {
            pka.push(MockPka::new());
        }
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        handler.on_event(HsmFsmEvent::FpToHsmIpcRequest);
    }
}
