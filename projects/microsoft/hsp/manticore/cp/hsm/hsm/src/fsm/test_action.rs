// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use cfg_if::cfg_if;

#[cfg(any(feature = "mcr_test_hooks", feature = "mcr_manual_test_hooks"))]
use log::trace;

cfg_if! {
    if #[cfg(feature = "mcr_test_hooks")] {
        cfg_if! {
            if #[cfg(feature = "mcr_test_hooks_cdma_ecc_err")] {
                use mcr_cdma::*;
            }
        }
        use mcr_dtcm_controller::DtcmController;
        use mcr_itcm_controller::ItcmController;
        use mcr_gdma_controller::GdmaController;
    use mcr_self_test::SelfTest;
    }
}

cfg_if! {
    if #[cfg(feature = "mcr_manual_test_hooks")] {
        use mcr_ipc_message::InterruptSource;
        use mcr_ipc_message::TdispInterruptInfo;
    }
}

/// FSM states
#[derive(Clone, Copy)]
#[allow(dead_code)]
enum State {
    /// Initial state
    Init,

    /// Waiting for Admin IPC channel
    WaitForResource,

    /// Waiting for Admin IPC response
    WaitForAdminIpcResponse,

    /// Final state
    Final,
}

/// Test action command
#[allow(dead_code)]
pub(crate) struct TestActionCmd<E: HsmEnvTrait> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,

    /// Partition
    session: E::UserSession,

    /// Pfn used in TDISP interrupt test
    pfn: PcieFunction,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for TestActionCmd<E> {
    /// Get the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            #[cfg(any(feature = "mcr_test_hooks", feature = "mcr_manual_test_hooks"))]
            (State::WaitForResource, HsmFsmEvent::ResourceReady(_)) => self.on_resource_ready(tag),
            #[cfg(any(feature = "mcr_test_hooks", feature = "mcr_manual_test_hooks"))]
            (State::WaitForAdminIpcResponse, HsmFsmEvent::AdminToHsmIpcResponse) => {
                self.handle_neg_self_test_resp(tag)
            }
            (State::Init, _) => Err(HsmErr::InvalidEvent),
            (State::Final, _) => Err(HsmErr::InvalidState),
            _ => Err(HsmErr::InvalidState),
        }
    }

    /// Get the session ID this command FSM operates on-behalf of
    fn session_id(&self) -> Option<u16> {
        Some(self.session.id())
    }

    /// Check if the command requires resource
    fn requires_resource(&self, _tag: TagId, _res_id: ResId) -> bool {
        true
    }

    /// Acquire a resource
    fn acquire_resource(&mut self, _tag: TagId, res_id: ResId) -> HsmFsmEvent {
        match res_id {
            HsmFsmResourceId::HsmToAdminIpcChannel => {
                HsmFsmEvent::ResourceReady(HsmFsmResourceId::HsmToAdminIpcChannel)
            }
            HsmFsmResourceId::FpIpcChannel => {
                HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
            }
            _ => unreachable!(),
        }
    }
}

#[allow(dead_code)]
impl<E: HsmEnvTrait> TestActionCmd<E> {
    /// Create a new command FSM
    pub fn new(
        req: DmaBuffer<E>,
        heap: DmaHeap<E>,
        session: E::UserSession,
        part: E::Partition,
        pfn: PcieFunction,
    ) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            req,
            resp: None,
            session,
            pfn,
        }
    }

    /// On start
    /// Note that, this class will only be instantiated if at least one of the features ("mcr_test_hooks", "mcr_manual_test_hooks", "fips_validation_hooks") is enabled.
    /// Therefore, all the code paths inside this function should follow the pattern of conditional compilation.
    fn on_start(&mut self, _tag: TagId) -> HsmResult<()> {
        // FSM can be called only once
        self.state = State::Final;

        #[allow(unused_mut)]
        let mut res = None;
        let decoded_req = decode_buf::<DdiTestActionCmdReq, E>(&self.req)?;

        match decoded_req.data.action {
            DdiTestAction::Level1SkipIo => {
                #[cfg(feature = "mcr_test_hooks")]
                {
                    trace!("[tag: {}] Skipping IO", _tag);
                    Err(HsmErr::Pending)?
                }

                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::SetLevel2SkipIo => {
                #[cfg(feature = "mcr_test_hooks")]
                {
                    self.part.set_test_hook_to_trigger_level2_abort(true);
                    Err(HsmErr::Pending)?
                }

                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::ClearLevel2SkipIo => {
                #[cfg(feature = "mcr_test_hooks")]
                self.part.set_test_hook_to_trigger_level2_abort(false);

                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::InvalidateCertSizeCache => {
                #[cfg(feature = "mcr_test_hooks")]
                {
                    trace!("[tag: {}] Invalidating cert size cache", _tag);
                    self.part.set_cert_chain_lengths_info(None);
                }

                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::TriggerCrash => {
                #[cfg(feature = "mcr_test_hooks")]
                if decoded_req.data.crash_info.is_some() {
                    let cpu_id = decoded_req.data.crash_info.unwrap().cpu_id;
                    trace!("[tag: {}] Triggering crash dump for CPU {:?}", _tag, cpu_id);

                    if cpu_id == DdiTestActionSocCpuId::Hsm {
                        #[cfg(feature = "mcr_test_hooks")]
                        self.trigger_crashdump_local(_tag)?;
                    } else {
                        #[cfg(feature = "mcr_test_hooks")]
                        self.send_crashdump_request(_tag)?;
                    }
                }

                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::ExecuteNegativeSelfTest => {
                #[cfg(feature = "mcr_test_hooks")]
                match decoded_req.data.neg_test_id {
                    Some(neg_test_id) => {
                        trace!("[tag: {}] Executing negative self test", _tag);
                        {
                            self.handle_negative_self_test(neg_test_id, _tag)?;
                            self.state = State::WaitForAdminIpcResponse;
                            Err(HsmErr::Pending)?
                        }
                    }
                    None => {
                        error!("[test_action] Missing negative self test ID");
                        Err(HsmErr::InvalidArgument)?
                    }
                }

                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::PinPolicyOverride => {
                #[cfg(feature = "mcr_test_hooks")]
                match decoded_req.data.pin_policy_config {
                    Some(pin_policy_config) => {
                        trace!(
                            "[tag: {}] Overriding pin policy context with: {:?}",
                            _tag,
                            pin_policy_config
                        );
                        self.part.override_pin_policy_context(pin_policy_config);
                    }
                    None => {
                        error!("[test_action] Missing pin policy config");
                        Err(HsmErr::InvalidArgument)?
                    }
                };

                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::PinPolicyClear => {
                #[cfg(feature = "mcr_test_hooks")]
                self.part.clear_pin_policy();

                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::ForcePkaInstance => {
                #[cfg(feature = "fips_validation_hooks")]
                {
                    let instance = decoded_req.data.force_pka_instance.map(|x| x as usize);
                    self.session.force_pka_instance(instance);
                }

                #[cfg(not(feature = "fips_validation_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::TriggerRngHwFailure => {
                #[cfg(feature = "fips_validation_hooks")]
                match decoded_req.data.neg_test_id {
                    Some(rng_hw_self_test_id) => {
                        self.part.inject_rng_hw_failure(rng_hw_self_test_id);
                    }
                    None => {
                        error!("Invalid rng failure test id received");
                        Err(HsmErr::InvalidArgument)?
                    }
                }

                #[cfg(not(feature = "fips_validation_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::TriggerTdispInterrupt => {
                #[cfg(feature = "mcr_manual_test_hooks")]
                match decoded_req.data.tdisp_interrupt_type {
                    Some(interrupt_type) => {
                        self.send_tdisp_interrupt_request(_tag, interrupt_type)?;
                    }
                    None => Err(HsmErr::InvalidArgument)?,
                }

                #[cfg(not(feature = "mcr_manual_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::ToggleFipsApprovedState => {
                #[cfg(feature = "fips_validation_hooks")]
                self.part.toggle_fips_approved_state();

                #[cfg(not(feature = "fips_validation_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::TriggerNegativePctFailure => {
                #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
                self.part
                    .neg_pct_skip_cnt(decoded_req.data.neg_pct_skip_cnt);

                #[cfg(not(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks")))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::TriggerEccError => {
                #[cfg(all(
                    feature = "mcr_test_hooks",
                    not(feature = "mcr_test_hooks_cdma_ecc_err")
                ))]
                if decoded_req.data.ecc_error_info.is_some() {
                    let error_info = decoded_req.data.ecc_error_info.unwrap();
                    let error_type = error_info.ecc_error_type;
                    let cpu_id = error_info.cpu_id;

                    match error_type {
                        DdiTestActionEccErrorType::DtcmDoubleBit => {
                            DtcmController::inject_ecc_error(cpu_id);
                        }
                        DdiTestActionEccErrorType::CdmaSingleBit => Err(HsmErr::UnsupportedCmd)?,
                        DdiTestActionEccErrorType::CdmaEccErrIntrCount => {
                            Err(HsmErr::UnsupportedCmd)?
                        }
                        DdiTestActionEccErrorType::ItcmDoubleBit => {
                            ItcmController::inject_ecc_error();
                        }
                        _ => Err(HsmErr::InvalidArgument)?,
                    }
                }

                #[cfg(all(feature = "mcr_test_hooks", feature = "mcr_test_hooks_cdma_ecc_err"))]
                if decoded_req.data.ecc_error_info.is_some() {
                    let error_info = decoded_req.data.ecc_error_info.unwrap();
                    let error_type = error_info.ecc_error_type;
                    let cpu_id = error_info.cpu_id;

                    match error_type {
                        DdiTestActionEccErrorType::DtcmDoubleBit => {
                            DtcmController::inject_ecc_error(cpu_id);
                        }
                        DdiTestActionEccErrorType::ItcmDoubleBit => {
                            ItcmController::inject_ecc_error();
                        }
                        DdiTestActionEccErrorType::CdmaSingleBit => {
                            CdmaErr::inject_correctable_ecc_error();
                        }
                        DdiTestActionEccErrorType::CdmaEccErrIntrCount => {
                            res = self.part.get_corr_ecc_err_intr_count();
                        }
                        _ => Err(HsmErr::InvalidArgument)?,
                    }
                }

                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::TriggerGdmaError => {
                #[cfg(feature = "mcr_test_hooks")]
                if let Some(error_type) = decoded_req.data.gdma_error_type {
                    GdmaController::inject_gdma_error(error_type);
                }

                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }

            DdiTestAction::ClearUserCredentials => {
                #[cfg(feature = "mcr_test_hooks")]
                self.part.clear_credentials()?;

                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::ClearProvisioningState => {
                #[cfg(feature = "mcr_test_hooks")]
                self.part.clear_provisioning_state()?;

                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::UpdateSvn => {
                #[cfg(feature = "mcr_test_hooks")]
                if decoded_req.data.updated_svn.is_some() {
                    self.part
                        .set_current_svn(decoded_req.data.updated_svn.unwrap())?;
                }

                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::ClearBk3 => {
                #[cfg(feature = "fips_validation_hooks")]
                self.part.clear_bk3_info();

                #[cfg(not(feature = "fips_validation_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            DdiTestAction::TriggerStackValidation => {
                #[cfg(feature = "mcr_test_hooks")]
                {
                    let stack_info = decoded_req
                        .data
                        .stack_validation_info
                        .ok_or(HsmErr::InvalidArgument)?;
                    let cpu_id = stack_info.cpu_id;
                    let error_type = stack_info.error_type;
                    trace!(
                        "[tag: {}] Testing stack validation ({:?}) for CPU {:?}",
                        _tag,
                        error_type,
                        cpu_id
                    );
                    if cpu_id == DdiTestActionSocCpuId::Admin {
                        self.send_stack_validation_request(_tag, cpu_id, error_type)?;
                    } else {
                        mcr_crashdump::trigger_stack_validation(error_type);
                    }
                }

                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
            _ => {
                #[cfg(feature = "mcr_test_hooks")]
                {
                    let _ = self.part.cmd_fsm_test_action(Some(decoded_req.data.action));
                    let _ = self.part.hsm_fsm_test_action(None);
                }
                #[cfg(not(feature = "mcr_test_hooks"))]
                Err(HsmErr::UnsupportedCmd)?
            }
        }

        // Encode and save the buffer
        self.resp = Some(encode_buf(
            &self.cmd_resp(decoded_req.hdr.rev, decoded_req.hdr.sess_id, res),
            &self.heap,
        )?);

        Ok(())
    }

    /// Handle the IPC resource ready
    #[cfg(any(feature = "mcr_test_hooks", feature = "mcr_manual_test_hooks"))]
    fn on_resource_ready(&mut self, tag: TagId) -> Result<(), HsmErr> {
        trace!("[tag: {}] On IPC ready", tag);
        #[cfg(not(any(feature = "mcr_test_hooks", feature = "mcr_manual_test_hooks")))]
        {
            Ok(())
        }

        #[cfg(feature = "mcr_manual_test_hooks")]
        {
            let decoded_req = decode_buf::<DdiTestActionCmdReq, E>(&self.req)?;
            match decoded_req.data.tdisp_interrupt_type {
                Some(interrupt_type) => self.send_tdisp_interrupt_request(tag, interrupt_type),
                None => Err(HsmErr::InvalidArgument)?,
            }
        }

        #[cfg(feature = "mcr_test_hooks")]
        {
            let decoded_req = decode_buf::<DdiTestActionCmdReq, E>(&self.req)?;
            match decoded_req.data.action {
                DdiTestAction::TriggerStackValidation => {
                    let stack_info = decoded_req
                        .data
                        .stack_validation_info
                        .ok_or(HsmErr::InvalidArgument)?;
                    self.send_stack_validation_request(
                        tag,
                        stack_info.cpu_id,
                        stack_info.error_type,
                    )
                }
                DdiTestAction::TriggerCrash => self.send_crashdump_request(tag),
                _ => Err(HsmErr::InvalidArgument)?,
            }
        }
    }

    /// Trigger crash dump
    #[cfg(feature = "mcr_test_hooks")]
    fn trigger_crashdump_local(&self, _tag: TagId) -> Result<(), HsmErr> {
        let decoded_req = decode_buf::<DdiTestActionCmdReq, E>(&self.req)?;
        let crash_info = match decoded_req.data.crash_info {
            Some(crash_info) => crash_info,
            None => {
                error!("[test_action] trigger_crashdump_local: Missing crash info");
                Err(HsmErr::InvalidArgument)?
            }
        };
        mcr_crashdump::crashdump_trigger(crash_info.crash_type);
        Ok(())
    }

    /// Send stack validation request to another core.
    #[cfg(feature = "mcr_test_hooks")]
    fn send_stack_validation_request(
        &mut self,
        tag: TagId,
        cpu_id: DdiTestActionSocCpuId,
        error_type: DdiTestStackErrorType,
    ) -> Result<(), HsmErr> {
        match self
            .session
            .send_stack_validation_request(tag, cpu_id.into(), error_type.into())
        {
            Ok(_) => {
                trace!("Stack validation request has been sent.");
                self.state = State::Final;
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

    /// Send crash dump request to another core.
    #[cfg(feature = "mcr_test_hooks")]
    fn send_crashdump_request(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let decoded_req = decode_buf::<DdiTestActionCmdReq, E>(&self.req)?;
        let crash_info = match decoded_req.data.crash_info {
            Some(crash_info) => crash_info,
            None => {
                error!("[test_action] send_crashdump_request: Missing crash info");
                Err(HsmErr::InvalidArgument)?
            }
        };

        // Acquire the HSM to admin IPC channel and send the crashdump request.
        match self.session.send_crashdump_request(
            tag,
            crash_info.cpu_id.into(),
            crash_info.crash_type.into(),
        ) {
            Ok(_) => {
                trace!("Crashdump request has been sent.");
                self.state = State::Final;
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

    /// Send tdisp interrupt info to Admin core
    #[cfg(feature = "mcr_manual_test_hooks")]
    fn send_tdisp_interrupt_request(
        &mut self,
        tag: TagId,
        interrupt_type: DdiTestActionInterruptSimulationType,
    ) -> Result<(), HsmErr> {
        // Acquire the HSM to admin IPC channel and send the crashdump request.
        let int_info = match interrupt_type {
            DdiTestActionInterruptSimulationType::Tdisp => {
                let vf_mask_0 = if matches!(self.pfn.0, 0..32) {
                    1 << self.pfn.0
                } else {
                    0
                };
                let vf_mask_1 = if matches!(self.pfn.0, 32..64) {
                    1 << (self.pfn.0 - 32)
                } else {
                    0
                };
                let pf_mask = if self.pfn.0 == 64 { 1 } else { 0 };

                TdispInterruptInfo {
                    source: InterruptSource::Tdisp,
                    vf_mask: [vf_mask_0, vf_mask_1],
                    pf_mask,
                    ..Default::default()
                }
            }
            DdiTestActionInterruptSimulationType::Ide => TdispInterruptInfo {
                source: InterruptSource::Ide,
                ..Default::default()
            },
            DdiTestActionInterruptSimulationType::Flr => TdispInterruptInfo {
                source: InterruptSource::Flr,
                ..Default::default()
            },
            DdiTestActionInterruptSimulationType::PerstUp => TdispInterruptInfo {
                source: InterruptSource::PerstUp,
                ..Default::default()
            },
            DdiTestActionInterruptSimulationType::PerstDown => TdispInterruptInfo {
                source: InterruptSource::PerstDown,
                ..Default::default()
            },
            _ => Err(HsmErr::InvalidArgument)?,
        };

        match self.session.send_tdisp_interrupt_request(tag, int_info) {
            Ok(_) => {
                trace!("TDISP interrupt request has been sent.");
                self.state = State::WaitForAdminIpcResponse;
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

    /// Handle negative self test
    #[cfg(feature = "mcr_test_hooks")]
    fn handle_negative_self_test(&mut self, neg_self_test: u32, tag: TagId) -> Result<(), HsmErr> {
        let neg_test: SelfTest = match SelfTest::try_from(neg_self_test) {
            Ok(neg_test) => neg_test,
            Err(_) => {
                error!("[test_action] Invalid negative self test ID");
                Err(HsmErr::InvalidArgument)?
            }
        };

        self.session.begin_neg_self_test_req(neg_test, tag)
    }

    /// Handle negative self test request response
    #[cfg(any(feature = "mcr_test_hooks", feature = "mcr_manual_test_hooks"))]
    fn handle_neg_self_test_resp(&mut self, tag: TagId) -> Result<(), HsmErr> {
        self.session.end_neg_self_test_resp(tag)?;

        let decoded_req = decode_buf::<DdiTestActionCmdReq, E>(&self.req)?;
        self.resp = Some(encode_buf(
            &self.cmd_resp(decoded_req.hdr.rev, decoded_req.hdr.sess_id, None),
            &self.heap,
        )?);
        self.state = State::Final;

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(
        &self,
        rev: Option<DdiApiRev>,
        sess_id: Option<u16>,
        res: Option<u32>,
    ) -> DdiTestActionCmdResp {
        DdiTestActionCmdResp {
            hdr: DdiRespHdr {
                rev,
                op: DdiOp::TestAction,
                sess_id,
                status: DdiStatus::Success,
                fips_approved: false,
            },
            data: DdiTestActionResp { result: res },
        }
    }
}
