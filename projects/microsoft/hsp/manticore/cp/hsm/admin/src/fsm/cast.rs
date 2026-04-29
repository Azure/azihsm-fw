// Copyright (c) Microsoft Corporation. All rights reserved.

//! CAST: Cryptographic Algorithm Self Test
//! This FSM schedules self-tests periodically and handles the self-test response.
//! Any error during self-test execution (e.g. timeout or test failure) will be handled here.
//! Self-test failures are handled by, logging a detailed error log on which self-test failed and crash the system.
//! The onus of handling a self-test lies on this FSM alone.

use super::AdminEnvTrait;
use super::CmdTimer;
use super::ResId;

use crate::error;
use crate::fsm::AdminErr;
use crate::fsm::AdminFsm;
use crate::preop_cdma_io::CdmaIoTestName;
use crate::preop_cdma_io::CdmaKeyIndex;
use crate::preop_cdma_io::SELF_TEST_VAULT_ID;
use crate::resource::AdminFsmResourceId;
use crate::resource::AdminToFpIpcChannel;
use crate::resource::CastIdle;
use crate::AdminFsmContext;
use crate::AdminFsmEvent;
use crate::AdminFsmEventRecorder;
use crate::CmdFsm;
use crate::CmdResourceRef;
use crate::TagId;

use alloc::vec;
use core::fmt::Debug;
use core::panic;
use mcr_crypto_cdma_io::aes_fp_self_test_constants::*;
use mcr_crypto_cdma_io::cdma_io::AesGcm256SelfTestVectors;
use mcr_crypto_cdma_io::cdma_io::AesXts256SelfTestVectors;
use mcr_crypto_cdma_io::*;
use mcr_crypto_softaes::SoftAesTrait;
use mcr_io_controller::IoControllerTrait;
use mcr_ipc_controller::IpcMessageChannelTrait;
use mcr_ipc_message::*;
use mcr_logging::*;
use mcr_self_test::SelfTest;
use mcr_self_test::SelfTestReqPacket;
use mcr_self_test::SelfTestTracker;
use mcr_simplex::SimplexPipeTrait;
#[cfg(feature = "fips_validation_hooks")]
use mcr_soc::*;
use mcr_tcon::Tcon;
use mcr_types::DebugLogComponent;
use mcr_types::DebugLogEntryParameters;
use mcr_types::DebugLogSeverity;
use mcr_types::MemoryLocation;
use mcr_types::SecureByteArray;

/// Self test execution periodicity in ms.
const SELF_TEST_PERIODICITY_IN_MS: u32 = 60_000;

/// Reduced self test execution scehdule timer on negative self test request.
const SELF_TEST_TIMER_ON_NEGATIVE_TEST: u32 = 2_000;

/// CAST FSM state
#[derive(Debug, Clone, Copy, PartialEq)]
enum CastFsmState {
    /// Waiting for the timer event for a new test.
    Idle,

    /// Waiting for cast resource.
    WaitingForCastResource,

    /// Waiting for the self test to complete or timeout.
    WaitingForCompletion,

    /// Waiting for FP IPC resource
    WaitingForFpIpcResource,

    /// Waiting for FP IPC response
    WaitingForFpIpcResponse,
}

/// CAST FSM
pub(crate) struct CastFsm<E: AdminEnvTrait + 'static> {
    /// CAST state
    state: CastFsmState,

    /// Admin FSM Context
    ctx: AdminFsmContext<E>,

    /// Timer
    timer: CmdTimer,

    /// self test tracker.
    self_test_tracker: SelfTestTracker,

    /// CAST resource
    cast_resource: Option<CmdResourceRef<CastIdle, AdminFsm<E>>>,

    /// The next negative self-test id.
    neg_self_test_id: Option<SelfTest>,

    /// time for scheduling the next self test.
    next_self_test_time: u32,

    /// current test
    curr_self_test: SelfTest,

    /// AES FP self test object
    aes_fp_self_test: CdmaIoSelfTest<E>,
}

impl<E: AdminEnvTrait> CmdFsm for CastFsm<E> {
    type Error = AdminErr;
    type ResourceId = AdminFsmResourceId;
    type Event = AdminFsmEvent;
    type Recorder = AdminFsmEventRecorder;

    fn get_timer(&mut self) -> Option<&mut CmdTimer> {
        Some(&mut self.timer)
    }

    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match (self.state, event) {
            (CastFsmState::Idle, AdminFsmEvent::TimerElapsed) => {
                self.on_timer_event(tag);
                Err(AdminErr::Pending)
            }
            (CastFsmState::WaitingForCompletion, AdminFsmEvent::TimerElapsed) => {
                self.handle_test_failure(AdminErr::SelfTestTimeout);
                Err(AdminErr::Pending)
            }
            (CastFsmState::WaitingForCastResource, AdminFsmEvent::TimerElapsed) => {
                error!("Received timer event while waiting for cast resource");

                Err(AdminErr::Pending)
            }
            (CastFsmState::WaitingForCompletion, AdminFsmEvent::SelfTestResponse) => {
                self.on_self_test_complete();
                Err(AdminErr::Pending)
            }
            (
                CastFsmState::WaitingForCastResource,
                AdminFsmEvent::ResourceReady(ResId::CastIdle),
            ) => {
                self.schedule_next_test(tag);
                Err(AdminErr::Pending)
            }
            (
                CastFsmState::Idle | CastFsmState::WaitingForFpIpcResource,
                AdminFsmEvent::ResourceReady(ResId::AdminToFpIpcChannel),
            ) => self.on_fp_to_admin_channel_ready(tag),
            (CastFsmState::WaitingForFpIpcResponse, AdminFsmEvent::FpToAdminIpcResponse) => {
                self.on_fp_to_admin_ipc_response(tag)
            }
            (_, AdminFsmEvent::NegativeSelfTest(test)) => self.on_negative_self_test_request(test),
            (_, _) => {
                error!("Invalid event. state: {:?}", self.state as u32);

                Err(AdminErr::Pending)
            }
        }
    }

    /// Acquire cast resource
    fn acquire_resource(&mut self, tag: TagId, res_id: Self::ResourceId) -> Self::Event {
        match res_id {
            AdminFsmResourceId::CastIdle => {
                self.cast_resource = self.ctx.cast_idle().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::CastIdle)
            }
            AdminFsmResourceId::AdminToFpIpcChannel => {
                if self.aes_fp_self_test.admin_to_fp_ipc_channel.is_some() {
                    panic!("AdminToFpIpcChannel already acquired");
                }

                self.aes_fp_self_test.admin_to_fp_ipc_channel =
                    self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::AdminToFpIpcChannel)
            }
            _ => AdminFsmEvent::Unknown,
        }
    }
}

impl<E: AdminEnvTrait> CastFsm<E> {
    /// Create a new CAST FSM
    pub fn new(ctx: AdminFsmContext<E>) -> Self {
        let mut self_test = Self {
            ctx,
            state: CastFsmState::Idle,
            timer: CmdTimer::new(),
            self_test_tracker: SelfTestTracker::new(),
            cast_resource: None,
            neg_self_test_id: None,
            next_self_test_time: SELF_TEST_PERIODICITY_IN_MS,
            curr_self_test: SelfTest::SelfTestCompleted,
            aes_fp_self_test: CdmaIoSelfTest::new(),
        };
        self_test.start_timer_event_for_next_test();

        self_test
    }

    #[cfg(test)]
    pub fn set_next_test(&mut self, next_test: SelfTest) {
        self.self_test_tracker.set_next_test(next_test);
    }

    /// Handle timer event
    fn on_timer_event(&mut self, tag: TagId) {
        self.cast_resource = self.ctx.cast_idle().acquire(tag, ());
        match self.cast_resource {
            None => self.state = CastFsmState::WaitingForCastResource,
            Some(_) => self.schedule_next_test(tag),
        }
    }

    /// Schedule the next self test
    fn schedule_next_test(&mut self, tag: TagId) {
        // Pick up the next test from the list or a negative self test.
        let (curr_self_test, _induce_failure) = match self.neg_self_test_id {
            Some(neg_self_test) => {
                // Done with the negative self test handling, reset the negative self test id
                self.neg_self_test_id.take();

                (neg_self_test, true)
            }
            None => (self.next_self_test(), false),
        };

        self.curr_self_test = curr_self_test;
        #[cfg(feature = "fips_validation_hooks")]
        if _induce_failure {
            self.ctx
                .soc_info()
                .set_negative_cast_hooks(self.curr_self_test);
        }

        match self.curr_self_test {
            SelfTest::AesGcmAlignedAndUnalignedData
            | SelfTest::AesGcmAadNoAlignedData
            | SelfTest::AesGcmAlignedData
            | SelfTest::AesXtsNegEnc
            | SelfTest::AesXtsNegDec => {
                // Admin executes AES GCM and AES XTS self tests
                self.aes_fp_self_test.admin_to_fp_ipc_channel =
                    self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());

                if self.aes_fp_self_test.admin_to_fp_ipc_channel.is_some() {
                    if let Err(err) = self.aes_fp_self_test_start(tag) {
                        self.handle_test_failure(err);
                    };
                } else {
                    self.state = CastFsmState::WaitingForFpIpcResource;
                }
            }
            SelfTest::AesEcb => {
                // Admin executes AES ECB self test
                self.start_completion_timer();

                let result = self
                    .ctx
                    .soft_aes()
                    .aes_ecb_256_decrypt_self_test()
                    .map_err(|_| AdminErr::AesEcbSelfTestFailed);

                self.inspect_result(result);
            }
            SelfTest::AesUnwrapWithPadding => {
                // Admin executes AES Unwrap with padding self test
                self.start_completion_timer();

                let result = self
                    .ctx
                    .soft_aes()
                    .aes_256_key_unwrap_self_test()
                    .map_err(|_| AdminErr::AesUnwrapSelfTestFailed);

                self.inspect_result(result);
            }
            _ => self.hsm_executed_self_test(),
        }
    }

    /// Execute HSM executed self test
    fn hsm_executed_self_test(&mut self) {
        // Send the self test request message to the HSM.
        match self.ctx.self_test_req().send(SelfTestReqPacket {
            test_id: self.curr_self_test,
        }) {
            Err(_err) => {
                self.handle_test_failure(AdminErr::FailedToSendSelfTestReq);
            }
            Ok(()) => {
                // For RNG self test all the IOs need to be stopped, so just pause the slow path
                // inbound operations
                if self.curr_self_test == SelfTest::Rng {
                    self.ctx.io_controller().pause_inbound();
                }

                self.state = CastFsmState::WaitingForCompletion;
                self.start_completion_timer();
            }
        }
    }

    /// AES GCM/XTS self test start
    fn aes_fp_self_test_start(&mut self, tag_id: TagId) -> Result<(), AdminErr> {
        self.start_completion_timer();

        if self.aes_fp_self_test.state != CdmaIoTestState::Begin {
            return Err(AdminErr::CdmaIoInvalidCastState);
        }

        match self.curr_self_test {
            SelfTest::AesGcmAlignedAndUnalignedData => {
                self.aes_fp_self_test.set_gcm_test_vector(unsafe {
                    #[allow(static_mut_refs)]
                    &AES_GCM_256_TEST_VECTORS
                });
                let result = self.aes_fp_self_test.begin_gcm_test(
                    tag_id,
                    &self.ctx,
                    CdmaIoTestName::GcmAlignedAndUnalignedData,
                );
                self.check_for_err(result)?;
            }
            SelfTest::AesGcmAlignedData => {
                self.aes_fp_self_test.set_gcm_test_vector(unsafe {
                    #[allow(static_mut_refs)]
                    &AES_GCM_256_AAD_ALIGNED_DATA_ONLY_TEST_VECTORS
                });
                let result = self.aes_fp_self_test.begin_gcm_test(
                    tag_id,
                    &self.ctx,
                    CdmaIoTestName::GcmAlignedDataOnly,
                );
                self.check_for_err(result)?;
            }
            SelfTest::AesGcmAadNoAlignedData => {
                self.aes_fp_self_test.set_gcm_test_vector(unsafe {
                    #[allow(static_mut_refs)]
                    &AES_GCM_256_AAD_NO_ALIGNED_DATA_TEST_VECTORS
                });
                let result = self.aes_fp_self_test.begin_gcm_test(
                    tag_id,
                    &self.ctx,
                    CdmaIoTestName::GcmAadNoAlignedData,
                );
                self.check_for_err(result)?;
            }
            SelfTest::AesXtsNegEnc => {
                let result = self.aes_fp_self_test.begin_xts_test(
                    tag_id,
                    &self.ctx,
                    CdmaIoTestName::XtsNegEnc,
                );
                self.check_for_err(result)?;
            }
            SelfTest::AesXtsNegDec => {
                let result = self.aes_fp_self_test.begin_xts_test(
                    tag_id,
                    &self.ctx,
                    CdmaIoTestName::XtsNegDec,
                );
                self.check_for_err(result)?;
            }
            _ => return Err(AdminErr::CdmaIoInvalidCastState),
        }

        Ok(())
    }

    /// AES GCM/XTS self test continue
    fn aes_fp_self_test_continue(&mut self, tag: TagId) -> Result<(), AdminErr> {
        match self.curr_self_test {
            SelfTest::AesGcmAlignedAndUnalignedData
            | SelfTest::AesGcmAlignedData
            | SelfTest::AesGcmAadNoAlignedData => self.gcm_self_test_continue(tag)?,
            SelfTest::AesXtsNegEnc | SelfTest::AesXtsNegDec => self.xts_self_test_continue(tag)?,
            _ => return Err(AdminErr::CdmaIoInvalidCastState),
        }

        Ok(())
    }

    /// Helper function for GCM self test states
    fn gcm_self_test_continue(&mut self, tag: TagId) -> Result<(), AdminErr> {
        match self.aes_fp_self_test.state {
            CdmaIoTestState::Continue => {
                let result = self.aes_fp_self_test.continue_gcm_test(tag, &self.ctx);
                self.check_for_err(result)?;
            }
            CdmaIoTestState::End => {
                let result = self.aes_fp_self_test.end_gcm_test(tag, &self.ctx);
                self.inspect_result(result);
            }
            _ => return Err(AdminErr::CdmaIoInvalidCastState),
        }

        Ok(())
    }

    /// Helper function for XTS self test states
    fn xts_self_test_continue(&mut self, tag: TagId) -> Result<(), AdminErr> {
        match self.aes_fp_self_test.state {
            CdmaIoTestState::Continue => {
                let result = self.aes_fp_self_test.continue_xts_test(tag, &self.ctx);
                self.check_for_err(result)?;
            }
            CdmaIoTestState::End => {
                let result = self.aes_fp_self_test.end_xts_test(tag, &self.ctx);
                self.inspect_result(result);
            }
            _ => return Err(AdminErr::CdmaIoInvalidCastState),
        }

        Ok(())
    }

    /// Handle FP to Admin channel resource ready
    fn on_fp_to_admin_channel_ready(&mut self, tag: TagId) -> Result<(), AdminErr> {
        if self.aes_fp_self_test.state != CdmaIoTestState::Begin {
            error!(
                "on_fp_to_admin_channel_ready: Invalid state {:?}",
                self.aes_fp_self_test.state as u32
            );

            self.handle_test_failure(AdminErr::CdmaIoInvalidCastState);
            return Err(AdminErr::Pending);
        }

        if let Err(err) = self.aes_fp_self_test_start(tag) {
            self.handle_test_failure(err);
        };

        Err(AdminErr::Pending)
    }

    /// Handle FP to Admin IPC channel response event
    fn on_fp_to_admin_ipc_response(&mut self, tag: TagId) -> Result<(), AdminErr> {
        if self.aes_fp_self_test.state == CdmaIoTestState::Begin {
            error!(
                "on_fp_to_admin_ipc_response: Invalid state {:?}",
                self.aes_fp_self_test.state as u32
            );
            self.handle_test_failure(AdminErr::CdmaIoInvalidCastState);

            return Err(AdminErr::Pending);
        }

        if let Err(err) = self.aes_fp_self_test_continue(tag) {
            self.handle_test_failure(err);
        };

        Err(AdminErr::Pending)
    }

    /// Get the next self-test to execute.
    fn next_self_test(&mut self) -> SelfTest {
        loop {
            let test = self.self_test_tracker.next_test();
            if test != SelfTest::SelfTestCompleted {
                break test;
            }
        }
    }

    /// Handle self test complete event
    fn on_self_test_complete(&mut self) {
        self.handle_self_test_response();
    }

    /// Handle self test response message
    fn handle_self_test_response(&mut self) {
        if let Some(test_result) = self.ctx.self_test_resp().recv() {
            self.inspect_result(test_result.result);
        } else {
            self.handle_test_failure(AdminErr::FailedToRecvSelfTestResp);
        }
    }

    /// Inspect the result of the self test
    fn inspect_result<T: Debug + Into<u32> + Copy>(&mut self, result: Result<(), T>) {
        match result {
            Err(_err) => {
                self.handle_test_failure(AdminErr::SelfTestFailed);
            }
            Ok(()) => {
                // For RNG self test all the IOs were stopped before running the test, since the
                // test is completed, resume the slow path inbound operations
                if self.curr_self_test == SelfTest::Rng {
                    self.ctx.io_controller().resume_inbound();
                }

                self.drop_cast_resource();
                self.start_timer_event_for_next_test();
            }
        }
    }

    /// Check for CDMA IO test sub FSM state errors
    fn check_for_err(&mut self, result: Result<(), AdminErr>) -> Result<(), AdminErr> {
        if let Err(err) = result {
            self.ctx.cdma_io().zeroize_buffers();
            self.handle_test_failure(err);
        }

        self.state = CastFsmState::WaitingForFpIpcResponse;

        result
    }

    // Release the CAST resource.
    fn drop_cast_resource(&mut self) {
        let _ = self.cast_resource.take();
        self.state = CastFsmState::Idle;
    }

    /// Handle self test execution failure
    fn handle_test_failure(&mut self, _err: AdminErr) {
        // Clear the negative self test id if it is the current test.
        if self.neg_self_test_id.is_some() {
            self.neg_self_test_id = None;
        }

        // Send Notification to the HSP about the error using the mailbox
        self.ctx.notify_self_test_failure(self.curr_self_test);
    }

    /// Handle negative self test request
    fn on_negative_self_test_request(&mut self, test: SelfTest) -> Result<(), AdminErr> {
        if self.neg_self_test_id.is_none() {
            self.neg_self_test_id = Some(test);
            self.next_self_test_time = SELF_TEST_TIMER_ON_NEGATIVE_TEST;

            // If we are not waiting for command completion, start the timer again with new wait time.
            if self.state != CastFsmState::WaitingForCompletion {
                self.start_timer_event_for_next_test();
            }
        }

        Err(AdminErr::Pending)
    }

    /// Schedule the timer event for the next test
    fn start_timer_event_for_next_test(&mut self) {
        self.timer
            .start(Tcon::get_approximate_ticks_from_ms(self.next_self_test_time) as u8);
    }

    /// Start the completion timer
    fn start_completion_timer(&mut self) {
        /// Self Test timeout in ms
        const SELF_TEST_TIMEOUT_IN_MS: u32 = 1_000;

        self.timer
            .start(Tcon::get_approximate_ticks_from_ms(SELF_TEST_TIMEOUT_IN_MS) as u8);
    }
}

/// CDMA IO FSM state
#[derive(Debug, Default, Clone, Copy, PartialEq)]
enum CdmaIoTestState {
    /// State to begin AES GCM/XTS Encryption/Decryption
    #[default]
    Begin,

    /// State to continue AES GCM/XTS Encryption/Decryption
    Continue,

    /// State to end AES GCM/XTS Encryption/Decryption
    End,
}

/// CDMA IO internal state machine
pub(crate) struct CdmaIoSelfTest<E: AdminEnvTrait + 'static> {
    /// CDMA IO Test state
    state: CdmaIoTestState,

    /// Fastpath IPC channel resource
    admin_to_fp_ipc_channel:
        Option<CmdResourceRef<AdminToFpIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// CDMA IO Config Data
    cdma_io_config: CdmaIoConfig,

    /// AES GCM Self Test Vector
    gcm_test_vector: &'static AesGcm256SelfTestVectors,

    /// AES XTS Self Test Vector
    xts_test_vector: &'static AesXts256SelfTestVectors,
}

impl<E: AdminEnvTrait> CdmaIoSelfTest<E> {
    pub(crate) fn new() -> Self {
        Self {
            state: CdmaIoTestState::default(),
            admin_to_fp_ipc_channel: None,
            cdma_io_config: CdmaIoConfig::default(),
            gcm_test_vector: unsafe {
                #[allow(static_mut_refs)]
                &AES_GCM_256_TEST_VECTORS
            },
            xts_test_vector: unsafe {
                #[allow(static_mut_refs)]
                &AES_XTS_256_TEST_VECTORS
            },
        }
    }

    /// Set GCM test vector
    fn set_gcm_test_vector(&mut self, vector: &'static AesGcm256SelfTestVectors) {
        self.gcm_test_vector = vector;
    }

    /// Setup SQE for GCM test and send CDMA IO IPC message to FP
    fn begin_gcm_test(
        &mut self,
        tag_id: TagId,
        ctx: &AdminFsmContext<E>,
        test_name: CdmaIoTestName,
    ) -> Result<(), AdminErr> {
        let aligned_data_len = self.gcm_test_vector.aligned_data_len;

        let padded_aad = self.gcm_test_vector.padded_aad;
        let padded_aad = padded_aad.ok_or(AdminErr::CdmaIoAesGcmSelfTestFailed)?;
        let source_data_len = padded_aad.len() as u32 + aligned_data_len;

        // Store the encryption key in cdma io config
        match test_name {
            CdmaIoTestName::GcmAlignedAndUnalignedData => {
                self.cdma_io_config.key1_id = ctx.self_test_key_table()
                    [CdmaKeyIndex::GcmAlignedAndUnalignedData as usize]
                    .ok_or(AdminErr::CdmaIoAesGcmSelfTestFailed)?;
            }
            CdmaIoTestName::GcmAlignedDataOnly => {
                self.cdma_io_config.key1_id = ctx.self_test_key_table()
                    [CdmaKeyIndex::GcmAlignedDataOnly as usize]
                    .ok_or(AdminErr::CdmaIoAesGcmSelfTestFailed)?;
            }
            CdmaIoTestName::GcmAadNoAlignedData => {
                self.cdma_io_config.key1_id = ctx.self_test_key_table()
                    [CdmaKeyIndex::GcmAadNoAlignedData as usize]
                    .ok_or(AdminErr::CdmaIoAesGcmSelfTestFailed)?;
            }
            _ => return Err(AdminErr::CdmaIoAesGcmSelfTestFailed),
        }

        #[cfg(not(feature = "fips_validation_hooks"))]
        let plaintext = self.gcm_test_vector.plaintext;

        #[cfg(feature = "fips_validation_hooks")]
        let plaintext = {
            let mut plaintext = self.gcm_test_vector.plaintext;
            if SocInfo::default().induce_cast_failure(SelfTest::AesGcmAlignedData, None)
                || SocInfo::default().induce_cast_failure(SelfTest::AesGcmAadNoAlignedData, None)
            {
                plaintext[0] = plaintext[0].wrapping_add(1);
                plaintext[plaintext.len() - 1] = plaintext[plaintext.len() - 1].wrapping_add(1);
            }
            plaintext
        };

        // Set cdma io config
        self.cdma_io_config.mode = AesFpCipher::Gcm;
        self.cdma_io_config.op = AesFpOp::Encrypt;
        self.cdma_io_config.iv = Some(self.gcm_test_vector.iv);
        self.cdma_io_config.iv_bytes = Some(self.gcm_test_vector.iv_bytes);
        self.cdma_io_config.tag = Some(self.gcm_test_vector.tag);
        self.cdma_io_config.unpadded_aad_len = self.gcm_test_vector.unpadded_aad_len;
        self.cdma_io_config.padded_aad = Some(padded_aad);
        self.cdma_io_config.src_len = source_data_len;
        self.cdma_io_config.dst_len = source_data_len;
        self.cdma_io_config.frm_id = 1;
        self.cdma_io_config.input_text = plaintext;

        // Load SQE for GCM test into PSRAM
        ctx.cdma_io()
            .begin_enc_dec(
                tag_id,
                &self.cdma_io_config,
                &self.cdma_io_config.input_text[0..aligned_data_len as usize],
            )
            .map_err(|_| {
                ctx.cdma_io().zeroize_buffers();
                AdminErr::CdmaIoAesGcmSelfTestFailed
            })?;

        // Send CDMA IO IPC message to FP
        self.send_cdma_io_ipc(tag_id).map_err(|_| {
            ctx.cdma_io().zeroize_buffers();
            AdminErr::CdmaIoAesGcmSelfTestFailed
        })?;

        // Set cdma io test state to continue encryption/decryption
        self.state = CdmaIoTestState::Continue;

        Ok(())
    }

    /// Setup SQE for XTS test and send CDMA IO IPC message to FP
    fn begin_xts_test(
        &mut self,
        tag_id: TagId,
        ctx: &AdminFsmContext<E>,
        test_name: CdmaIoTestName,
    ) -> Result<(), AdminErr> {
        match test_name {
            CdmaIoTestName::XtsNegEnc => {
                self.cdma_io_config.key1_id = ctx.self_test_key_table()
                    [CdmaKeyIndex::XtsNegEncEnc as usize]
                    .ok_or(AdminErr::CdmaIoAesXtsSelfTestFailed)?;
                self.cdma_io_config.key2_id = Some(
                    ctx.self_test_key_table()[CdmaKeyIndex::XtsNegEncTweak as usize]
                        .ok_or(AdminErr::CdmaIoAesXtsSelfTestFailed)?,
                );
            }
            CdmaIoTestName::XtsNegDec => {
                self.cdma_io_config.key1_id = ctx.self_test_key_table()
                    [CdmaKeyIndex::XtsNegDecEnc as usize]
                    .ok_or(AdminErr::CdmaIoAesXtsSelfTestFailed)?;
                self.cdma_io_config.key2_id = Some(
                    ctx.self_test_key_table()[CdmaKeyIndex::XtsNegDecTweak as usize]
                        .ok_or(AdminErr::CdmaIoAesXtsSelfTestFailed)?,
                );
            }
            _ => return Err(AdminErr::CdmaIoAesXtsSelfTestFailed),
        }

        // Set cdma io config
        self.cdma_io_config.mode = AesFpCipher::Xts;
        self.cdma_io_config.op = AesFpOp::Encrypt;
        self.cdma_io_config.tweak = Some(self.xts_test_vector.tweak);

        #[cfg(not(feature = "fips_validation_hooks"))]
        let plaintext = self.xts_test_vector.plaintext;

        #[cfg(feature = "fips_validation_hooks")]
        let plaintext = {
            let mut plaintext = self.xts_test_vector.plaintext;
            if SocInfo::default().induce_cast_failure(SelfTest::AesXtsNegEnc, None) {
                plaintext[plaintext.len() - 1] = plaintext[plaintext.len() - 1].wrapping_add(1);
            }
            plaintext
        };

        // Load SQE for XTS test into PSRAM
        ctx.cdma_io()
            .begin_enc_dec(tag_id, &self.cdma_io_config, &plaintext)
            .map_err(|_| {
                ctx.cdma_io().zeroize_buffers();
                AdminErr::CdmaIoAesXtsSelfTestFailed
            })?;

        // Send CDMA IO IPC message to FP
        self.send_cdma_io_ipc(tag_id).map_err(|_| {
            ctx.cdma_io().zeroize_buffers();
            AdminErr::CdmaIoAesXtsSelfTestFailed
        })?;

        // Set cdma io test state to continue encryption/decryption
        self.state = CdmaIoTestState::Continue;

        Ok(())
    }

    /// Check CQE status for GCM encrypt test and send CDMA IO IPC message to FP for GCM decrypt test
    fn continue_gcm_test(
        &mut self,
        tag_id: TagId,
        ctx: &AdminFsmContext<E>,
    ) -> Result<(), AdminErr> {
        let aligned_data_len = self.gcm_test_vector.aligned_data_len as usize;

        // Check IPC response for GCM CDMA IO ipc message
        self.check_ipc_resp(AesFpCipher::Gcm).map_err(|_| {
            ctx.cdma_io().zeroize_buffers();
            AdminErr::CdmaIoAesGcmSelfTestFailed
        })?;

        let correct_tag =
            self.gcm_test_vector.aligned_data_len as usize != self.gcm_test_vector.text_len;

        // Check CQE status for GCM test
        self.end_and_validate_gcm(
            tag_id,
            ctx,
            &self.cdma_io_config.input_text[0..self.gcm_test_vector.text_len],
            &self.gcm_test_vector.ciphertext[0..self.gcm_test_vector.text_len],
            correct_tag,
        )
        .map_err(|_| {
            ctx.cdma_io().zeroize_buffers();
            AdminErr::CdmaIoAesGcmSelfTestFailed
        })?;

        // Set cdma io config
        self.cdma_io_config.op = AesFpOp::Decrypt;

        #[cfg(not(feature = "fips_validation_hooks"))]
        let ciphertext = self.gcm_test_vector.ciphertext;

        #[cfg(feature = "fips_validation_hooks")]
        let ciphertext = {
            let mut ciphertext = self.gcm_test_vector.ciphertext;
            if SocInfo::default().induce_cast_failure(SelfTest::AesGcmAlignedAndUnalignedData, None)
            {
                ciphertext[0] = ciphertext[0].wrapping_add(1);
                ciphertext[ciphertext.len() - 1] = ciphertext[ciphertext.len() - 1].wrapping_add(1);
            }
            ciphertext
        };

        self.cdma_io_config.input_text = ciphertext;

        // Load SQE for GCM test into PSRAM
        ctx.cdma_io()
            .begin_enc_dec(
                tag_id,
                &self.cdma_io_config,
                &self.cdma_io_config.input_text[0..aligned_data_len],
            )
            .map_err(|_| {
                ctx.cdma_io().zeroize_buffers();
                AdminErr::CdmaIoAesGcmSelfTestFailed
            })?;

        // Send CDMA IO IPC message to FP
        self.send_cdma_io_ipc(tag_id).map_err(|_| {
            ctx.cdma_io().zeroize_buffers();
            AdminErr::CdmaIoAesGcmSelfTestFailed
        })?;

        // Set cdma io test state to end encryption/decryption
        self.state = CdmaIoTestState::End;

        Ok(())
    }

    /// Check CQE status for GCM test and send delete keys IPC message to FP
    fn end_gcm_test(&mut self, tag_id: TagId, ctx: &AdminFsmContext<E>) -> Result<(), AdminErr> {
        let correct_tag =
            self.gcm_test_vector.aligned_data_len as usize != self.gcm_test_vector.text_len;

        if correct_tag {
            // Check IPC response for failure due to incomplete tag calculation by CDMA engine
            // An unsuccessful status is expected in this case since tag needs to be corrected
            self.check_ipc_resp_for_incomplete_tag_calc().map_err(|_| {
                ctx.cdma_io().zeroize_buffers();
                AdminErr::CdmaIoAesGcmSelfTestFailed
            })?;
        } else {
            // Check IPC response for GCM CDMA IO ipc message
            self.check_ipc_resp(AesFpCipher::Gcm).map_err(|_| {
                ctx.cdma_io().zeroize_buffers();
                AdminErr::CdmaIoAesGcmSelfTestFailed
            })?;
        }

        // Check CQE status for GCM test
        self.end_and_validate_gcm(
            tag_id,
            ctx,
            &self.cdma_io_config.input_text[0..self.gcm_test_vector.text_len],
            &self.gcm_test_vector.plaintext[0..self.gcm_test_vector.text_len],
            correct_tag,
        )
        .map_err(|_| {
            ctx.cdma_io().zeroize_buffers();
            AdminErr::CdmaIoAesGcmSelfTestFailed
        })?;

        // Clear test buffers
        ctx.cdma_io().zeroize_buffers();

        self.reset_test();

        // Set cdma io test state to Begin for next test
        self.state = CdmaIoTestState::Begin;

        Ok(())
    }

    /// Check IPC response for the given AesFpCipher
    /// Production flow differences:
    /// - This self test AES GCM implementation tries to mimic the production flow as closely
    ///   as possible, but this is the key difference to note:
    /// - In the production flow, the inputs (AAD, input text, etc.) are first preprocessed
    ///   into its aligned, unaligned, and padded AAD components. Then the data is processed
    ///   by the FP directly to perform the AES GCM operation on the aligned data. The FP
    ///   determines whether software tag correction is necessary based on the presence of
    ///   unaligned data, and if so, the unaligned data is sent to CP0 for software tag
    ///   correction.
    ///
    /// However, in this self test implementation, the entire processing of the input text and the
    /// decision flow of whether the tag correction is necessary is handled inside
    /// CP0/Admin directly. Thus although the procedure and the same hardware and software
    /// algorithms are used for the processing, the branching decision is not identical
    /// to the production flow.
    fn end_and_validate_gcm(
        &self,
        tag_id: TagId,
        ctx: &AdminFsmContext<E>,
        input_text: &[u8],
        expected_output: &[u8],
        correct_tag: bool,
    ) -> Result<(), AdminErr> {
        let aligned_data_len = self.gcm_test_vector.aligned_data_len as usize;
        let unaligned_data_len = input_text.len() - aligned_data_len;

        let aad = self
            .cdma_io_config
            .padded_aad
            .ok_or(AdminErr::CdmaIoAesGcmSelfTestFailed)?;

        let source_data_len = aad.len() + aligned_data_len;
        let mut output_buf = vec![0u8; source_data_len];

        let gcm_tag = ctx
            .cdma_io()
            .end_enc_dec(tag_id, &self.cdma_io_config, output_buf.as_mut_slice())
            .map_err(|_| AdminErr::CdmaIoAesGcmSelfTestFailed)?;

        // Note: This logic is to mimic the logic from the FP where it decides whether to send
        // the unaligned data to CP0 for tag correction processing based on
        // the unaligned data length. Here the correct_tag flag is calculated based on whether
        // there is unaligned data or not.
        let final_tag = if correct_tag {
            let mut key_blob = SecureByteArray::<KEY_SIZE>::new([0u8; KEY_SIZE]);
            let key_bytes = key_blob.as_mut_slice();
            for (&key_word, dst_bytes) in self
                .gcm_test_vector
                .key
                .iter()
                .zip(key_bytes.chunks_mut(size_of::<u32>()))
            {
                dst_bytes.copy_from_slice(&key_word.to_le_bytes());
            }

            let mut tag_ext_out_buf = vec![0u8; unaligned_data_len];

            let intermediate_tag = gcm_tag.ok_or(AdminErr::CdmaIoAesGcmSelfTestFailed)?;

            let iv = self
                .cdma_io_config
                .iv_bytes
                .ok_or(AdminErr::CdmaIoAesGcmSelfTestFailed)?;

            let unpadded_aad_len =
                self.cdma_io_config
                    .unpadded_aad_len
                    .ok_or(AdminErr::CdmaIoAesGcmSelfTestFailed)? as u64;

            // run gcm tag correction for the remaining unaligned data
            let updated_tag = ctx
                .soft_aes()
                .aes_gcm_tag_correction(
                    self.cdma_io_config.op == AesFpOp::Encrypt,
                    key_blob.as_slice(),
                    &iv,
                    unpadded_aad_len,
                    input_text.len() as u64,
                    None,
                    Some(&intermediate_tag),
                    &input_text[aligned_data_len..],
                    aligned_data_len,
                    tag_ext_out_buf.as_mut_slice(),
                )
                .map_err(|_| AdminErr::CdmaIoAesGcmSelfTestFailed)?;

            output_buf.extend_from_slice(&tag_ext_out_buf);

            updated_tag
        } else {
            gcm_tag.ok_or(AdminErr::CdmaIoAesGcmSelfTestFailed)?
        };

        // In the production flow, this step gets processed by the Host software
        output_buf.drain(0..aad.len());

        if final_tag != self.gcm_test_vector.tag {
            // Clear test buffers
            Err(AdminErr::CdmaIoAesGcmSelfTestFailed)?
        }

        if output_buf.as_slice() != expected_output {
            // Clear test buffers
            Err(AdminErr::CdmaIoAesGcmSelfTestFailed)?
        }

        Ok(())
    }

    /// Check CQE status for XTS encrypt test and send CDMA IO IPC message to FP for XTS decrypt test
    fn continue_xts_test(
        &mut self,
        tag_id: TagId,
        ctx: &AdminFsmContext<E>,
    ) -> Result<(), AdminErr> {
        // Check IPC response for XTS CDMA IO ipc message
        self.check_ipc_resp(AesFpCipher::Xts).map_err(|_| {
            ctx.cdma_io().zeroize_buffers();
            AdminErr::CdmaIoAesXtsSelfTestFailed
        })?;

        // Check CQE status for XTS test
        self.end_and_validate_xts(tag_id, ctx, &self.xts_test_vector.ciphertext)
            .map_err(|_| {
                ctx.cdma_io().zeroize_buffers();
                AdminErr::CdmaIoAesXtsSelfTestFailed
            })?;

        // Set cdma io config
        self.cdma_io_config.op = AesFpOp::Decrypt;

        #[cfg(not(feature = "fips_validation_hooks"))]
        let ciphertext = self.xts_test_vector.ciphertext;

        #[cfg(feature = "fips_validation_hooks")]
        let ciphertext = {
            let mut ciphertext = self.xts_test_vector.ciphertext;
            if SocInfo::default().induce_cast_failure(SelfTest::AesXtsNegDec, None) {
                ciphertext[ciphertext.len() - 1] = ciphertext[ciphertext.len() - 1].wrapping_add(1);
            }
            ciphertext
        };

        // Load SQE for XTS test into PSRAM
        ctx.cdma_io()
            .begin_enc_dec(tag_id, &self.cdma_io_config, &ciphertext)
            .map_err(|_| {
                ctx.cdma_io().zeroize_buffers();
                AdminErr::CdmaIoAesXtsSelfTestFailed
            })?;

        // Send CDMA IO IPC message to FP
        self.send_cdma_io_ipc(tag_id).map_err(|_| {
            ctx.cdma_io().zeroize_buffers();
            AdminErr::CdmaIoAesXtsSelfTestFailed
        })?;

        // Set cdma io test state to delete keys
        self.state = CdmaIoTestState::End;

        Ok(())
    }

    /// Check CQE status for XTS test and send delete keys IPC message to FP
    fn end_xts_test(&mut self, tag_id: TagId, ctx: &AdminFsmContext<E>) -> Result<(), AdminErr> {
        // Check IPC response for XTS CDMA IO ipc message
        self.check_ipc_resp(AesFpCipher::Xts).map_err(|_| {
            ctx.cdma_io().zeroize_buffers();
            AdminErr::CdmaIoAesXtsSelfTestFailed
        })?;

        // Check CQE status for XTS test
        self.end_and_validate_xts(tag_id, ctx, &self.xts_test_vector.plaintext)
            .map_err(|_| {
                ctx.cdma_io().zeroize_buffers();
                AdminErr::CdmaIoAesXtsSelfTestFailed
            })?;

        // Clear test buffers
        ctx.cdma_io().zeroize_buffers();

        self.reset_test();

        // Set cdma io test state to Begin for next test
        self.state = CdmaIoTestState::Begin;

        Ok(())
    }

    /// End XTS test and validate the output
    fn end_and_validate_xts(
        &mut self,
        tag_id: TagId,
        ctx: &AdminFsmContext<E>,
        expected_output: &[u8],
    ) -> Result<(), AdminErr> {
        let result_len = expected_output.len();
        let mut res_buf = vec![0u8; result_len];

        ctx.cdma_io()
            .end_enc_dec(tag_id, &self.cdma_io_config, res_buf.as_mut_slice())
            .map_err(|_| AdminErr::CdmaIoAesXtsSelfTestFailed)?;

        if res_buf != expected_output {
            return Err(AdminErr::CdmaIoAesXtsSelfTestFailed);
        }

        Ok(())
    }

    /// Reset test config and ipc channel
    fn reset_test(&mut self) {
        self.cdma_io_config = CdmaIoConfig::default();
        self.admin_to_fp_ipc_channel.take();
    }

    /// Send CDMA IO ipc message to FP
    fn send_cdma_io_ipc(&self, tag_id: TagId) -> Result<(), AdminErr> {
        let message = IpcMessageCdmaIoReq {
            info: CdmaIoMsgDataReq {
                dw0: CdmaIoMsgDataReqDw0::new().with_vfid(SELF_TEST_VAULT_ID as u8),
                dw1: CdmaIoMsgDataReqDw1::new()
                    .with_src_desc_inter_sel(MemoryLocation::Soc.into())
                    .with_src_data_inter_sel(MemoryLocation::Soc.into())
                    .with_dst_desc_inter_sel(MemoryLocation::Soc.into())
                    .with_dst_data_inter_sel(MemoryLocation::Soc.into()),
            },
            ..Default::default()
        };

        self.admin_to_fp_ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag_id, message.encode()))
            .map_err(|_| AdminErr::IpcSendRequestError)
    }

    /// Check if IPC message response is successful
    fn check_ipc_resp(&self, aes_mode: AesFpCipher) -> Result<(), AdminErr> {
        let err = match self
            .admin_to_fp_ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()))
        {
            Some(ipc_message) => {
                let resp_header = IpcMessageDecoder::decode_header(&ipc_message)
                    .map_err(|_| AdminErr::InvalidIpcHeader)?;

                resp_header.status() != 0
            }
            None => true,
        };

        if err {
            match aes_mode {
                AesFpCipher::Xts => Err(AdminErr::CdmaIoAesXtsSelfTestFailed)?,
                AesFpCipher::Gcm => Err(AdminErr::CdmaIoAesGcmSelfTestFailed)?,
            }
        }

        Ok(())
    }

    /// Check IPC message response is unsuccessful for incomplete tag calculation
    /// Only used for AES GCM decrypt operation
    fn check_ipc_resp_for_incomplete_tag_calc(&self) -> Result<(), AdminErr> {
        match self
            .admin_to_fp_ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()))
        {
            Some(ipc_message) => {
                let resp_header = IpcMessageDecoder::decode_header(&ipc_message)
                    .map_err(|_| AdminErr::InvalidIpcHeader)?;
                if resp_header.status() == 0 {
                    Err(AdminErr::CdmaIoAesGcmSelfTestFailed)?
                }
            }
            None => Err(AdminErr::CdmaIoAesGcmSelfTestFailed)?,
        }

        Ok(())
    }
}
