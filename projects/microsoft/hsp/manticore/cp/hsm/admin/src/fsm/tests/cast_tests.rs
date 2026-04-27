// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_self_test::SelfTest;
use mcr_self_test::SelfTestReqPacket;
use mcr_self_test::SelfTestRespPacket;

use crate::fsm::tests::helper::default_resource_expectations;
use crate::fsm::tests::helper::get_context;
use crate::fsm::tests::helper::make_fsm_for_self_test;
use crate::fsm::tests::helper::make_fsm_test_for_self_test;
use crate::fsm::AdminErr;
use crate::fsm::AdminFsm;
use crate::fsm::AdminFsmTest;
use crate::fsm::CastFsm;
use crate::fsm::ResourceTestFsm;
use crate::mock::MockAdminEnvTrait;
use crate::mock::MockSimplexPipe;
use crate::resource::AdminFsmResourceId;
use crate::AdminFsmContext;
use crate::AdminFsmEvent;
use crate::AdminFsmEventRecorder;
use crate::CmdFsm;
use crate::CmdScheduler;

#[derive(Clone)]
pub(crate) struct AdminFsmSelfTestTestConfigs {
    /// Failure to send the test request over the simplex pipe
    pub send_failure: bool,

    /// Failure to receive the test response over the simplex pipe
    pub recv_failure: bool,

    /// Response to the test request was received but status was not success
    pub test_success: bool,

    /// Timeout while waiting for the test response
    pub test_timeout: bool,

    // number of self test called
    pub num_self_tests: usize,

    /// Run HSM executed self tests;
    pub hsm_executed_self_test: bool,

    /// Run Admin executed AES Unwrap with padding self test
    pub execute_aes_unwrap: bool,

    /// Run Admin executed AES Ecb self test
    pub execute_aes_ecb: bool,

    /// Run Admin executed Aes Xts self tests
    pub execute_aes_xts_neg_enc: bool,

    /// Run Admin executed Aes Xts self tests
    pub execute_aes_xts_neg_dec: bool,

    /// Run Admin executed Aes Gcm self tests with Aligned and Unaligned Data
    pub execute_aes_gcm_aligned_and_unaligned_data: bool,

    /// Run Admin executed Aes Gcm self tests with Aligned Data
    pub execute_aes_gcm_aligned_data: bool,

    /// Run Admin executed Aes Gcm self tests with no Aligned Data
    pub execute_aes_gcm_no_aligned_data: bool,

    /// Failure to begin encryption/decryption
    pub begin_enc_dec_failure: bool,

    /// Failure to end encryption/decryption
    pub end_enc_dec_failure: bool,

    /// Failure to send IPC request
    pub ipc_send_failure: bool,

    /// Receive message status
    pub fp_receive_message_status: u32,

    /// Number of times begin enc dec is called
    pub num_begin_enc_dec: usize,

    /// Number of times end enc dec is called
    pub num_end_enc_dec: usize,

    /// Number of times aes gcm tag correction is called
    pub num_tag_correction: usize,

    /// Number of times zeroize buffers is called
    pub num_zeroize_buffers: usize,

    /// Number of times send requests is called
    pub num_xts_send_requests: usize,

    /// Number of times receive responses is called
    pub num_xts_receive_responses: usize,

    /// Number of times gcm send requests is called
    pub num_gcm_send_requests: usize,

    /// Mock configurations required for RNG self test
    pub rng_self_test: bool,

    /// Notify self test failure
    pub notify_self_test_failure: bool,
}

impl Default for AdminFsmSelfTestTestConfigs {
    fn default() -> Self {
        AdminFsmSelfTestTestConfigs {
            send_failure: false,
            recv_failure: false,
            test_success: true,
            test_timeout: false,
            num_self_tests: 1,
            hsm_executed_self_test: true,
            execute_aes_unwrap: false,
            execute_aes_ecb: false,
            execute_aes_xts_neg_enc: false,
            execute_aes_xts_neg_dec: false,
            execute_aes_gcm_aligned_and_unaligned_data: false,
            execute_aes_gcm_aligned_data: false,
            execute_aes_gcm_no_aligned_data: false,
            begin_enc_dec_failure: false,
            end_enc_dec_failure: false,
            ipc_send_failure: false,
            fp_receive_message_status: 0,
            num_begin_enc_dec: 0,
            num_end_enc_dec: 0,
            num_tag_correction: 0,
            num_zeroize_buffers: 0,
            num_xts_send_requests: 0,
            num_xts_receive_responses: 0,
            num_gcm_send_requests: 0,
            rng_self_test: false,
            notify_self_test_failure: false,
        }
    }
}

#[test]
fn test_unknown_event() {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);
    let ctx = get_context(test);
    let mut self_test_fsm = CastFsm::new(ctx);

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_hsm_executed_self_test_success() {
    let ctx = make_fsm_for_self_test(AdminFsmSelfTestTestConfigs::default());
    let mut self_test_fsm = CastFsm::new(ctx);
    self_test_fsm.set_next_test(SelfTest::EcdhEngineInstance0);

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::SelfTestResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_self_test_invalid_event() {
    let configs = AdminFsmSelfTestTestConfigs {
        num_self_tests: 0,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::HsmToAdminIpcRequest, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[cfg(not(feature = "fips_validation_hooks"))]
#[test]
fn loop_through_all_tests() {
    // Admin executed tests include: AES GCM (aligned data), AES GCM (no aligned data), AES XTS,
    // AES ECB, AES Key Unwrap with Padding
    const NUM_ADMIN_EXECUTED_SELF_TESTS: usize = 7;

    let configs = AdminFsmSelfTestTestConfigs {
        num_self_tests: mcr_self_test::SelfTest::all().len() - NUM_ADMIN_EXECUTED_SELF_TESTS - 1,
        hsm_executed_self_test: true,
        execute_aes_unwrap: true,
        execute_aes_ecb: true,
        execute_aes_xts_neg_enc: true,
        execute_aes_xts_neg_dec: true,
        execute_aes_gcm_aligned_and_unaligned_data: true,
        execute_aes_gcm_aligned_data: true,
        execute_aes_gcm_no_aligned_data: true,
        num_begin_enc_dec: 2,
        num_end_enc_dec: 2,
        num_zeroize_buffers: 1,
        num_xts_send_requests: 4,
        num_xts_receive_responses: 4,
        num_gcm_send_requests: 2,
        num_tag_correction: 2,
        rng_self_test: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);

    for test in mcr_self_test::SelfTest::all() {
        match test {
            SelfTest::AesXtsNegEnc
            | SelfTest::AesXtsNegDec
            | SelfTest::AesGcmAlignedAndUnalignedData
            | SelfTest::AesGcmAlignedData
            | SelfTest::AesGcmAadNoAlignedData => {
                aes_fp_self_test(&mut self_test_fsm);
            }
            SelfTest::AesEcb | SelfTest::AesUnwrapWithPadding => {
                assert_eq!(
                    self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
                    Err(AdminErr::Pending)
                );
            }
            SelfTest::SelfTestCompleted => (),
            _ => {
                assert_eq!(
                    self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
                    Err(AdminErr::Pending)
                );

                assert_eq!(
                    self_test_fsm.on_event(AdminFsmEvent::SelfTestResponse, 0xFF),
                    Err(AdminErr::Pending)
                );
            }
        }
    }
}

#[test]
fn test_self_test_other_resource() {
    let configs = AdminFsmSelfTestTestConfigs {
        num_self_tests: 0,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);

    let event = self_test_fsm.acquire_resource(0xFF, AdminFsmResourceId::HspIpcChannel);
    assert_eq!(self_test_fsm.on_event(event, 0xFF), Err(AdminErr::Pending));
}

#[test]
#[should_panic]
fn test_self_test_fsm_send_failure() {
    let configs = AdminFsmSelfTestTestConfigs {
        send_failure: true,
        hsm_executed_self_test: true,
        notify_self_test_failure: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);
    self_test_fsm.set_next_test(SelfTest::EcdhEngineInstance0);

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::FailedToSendSelfTestReq)
    );
}

#[test]
#[should_panic]
fn test_self_test_timeout() {
    let configs = AdminFsmSelfTestTestConfigs {
        test_timeout: true,
        hsm_executed_self_test: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);
    self_test_fsm.set_next_test(SelfTest::EcdhEngineInstance0);

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::SelfTestTimeout)
    );
}

#[test]
#[should_panic]
fn test_self_test_failure() {
    let configs = AdminFsmSelfTestTestConfigs {
        test_success: false,
        hsm_executed_self_test: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);
    self_test_fsm.set_next_test(SelfTest::EcdhEngineInstance0);

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::SelfTestResponse, 0xFF),
        Err(AdminErr::SelfTestFailed)
    );
}

#[test]
#[should_panic]
fn test_self_test_recv_failure() {
    let configs = AdminFsmSelfTestTestConfigs {
        recv_failure: true,
        hsm_executed_self_test: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);
    self_test_fsm.set_next_test(SelfTest::EcdhEngineInstance0);

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::SelfTestResponse, 0xFF),
        Err(AdminErr::FailedToRecvSelfTestResp)
    );
}

#[test]
fn test_self_test_invalid_state_change() {
    let ctx = make_fsm_for_self_test(AdminFsmSelfTestTestConfigs::default());
    let mut self_test_fsm = CastFsm::new(ctx);
    self_test_fsm.set_next_test(SelfTest::EcdhEngineInstance0);

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::SelfTestResponse, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::SelfTestResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_self_test_waiting_for_resource() {
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let configs = AdminFsmSelfTestTestConfigs::default();
    let mut fsm_test = make_fsm_test_for_self_test(configs);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    // Let the resource get acquired by some other FSM.
    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let self_test_fsm_tag = scheduler.alloc(AdminFsm::Cast(CastFsm::new(ctx)));
    assert!(self_test_fsm_tag.is_some());

    scheduler.on_event(AdminFsmEvent::SelfTestResponse, rsc_fsm_tag.unwrap());

    scheduler.map(self_test_fsm_tag.unwrap(), |fsm| {
        fsm.cast_fsm_set_next_test(SelfTest::EcdhEngineInstance0)
    });

    assert_eq!(
        scheduler.map(self_test_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::TimerElapsed, self_test_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::SelfTestResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(self_test_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::SelfTestResponse, self_test_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
#[should_panic]
fn test_negative_self_test() {
    let configs = AdminFsmSelfTestTestConfigs {
        test_success: false,
        hsm_executed_self_test: true,
        notify_self_test_failure: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);

    assert_eq!(
        self_test_fsm.on_event(
            AdminFsmEvent::NegativeSelfTest(mcr_self_test::SelfTest::EcdhEngineInstance0),
            0xFF
        ),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::SelfTestResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_self_test_timer_event_on_waiting_for_resources() {
    let configs = AdminFsmSelfTestTestConfigs {
        num_self_tests: 0,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let resource = ctx.cast_idle().acquire(1, ());
    assert!(resource.is_some());

    let mut self_test_fsm = CastFsm::new(ctx);

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_self_test_drop_resource_after_self_test() {
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = AdminFsmTest::default();
    default_resource_expectations(&mut fsm_test);
    let mut env = fsm_test.env();
    env.expect_clone().times(2).returning(move || {
        let mut env = MockAdminEnvTrait::new();
        let mut self_test_req = MockSimplexPipe::<SelfTestReqPacket>::new();
        self_test_req.expect_send().once().return_const(Ok(()));
        env.expect_self_test_req()
            .once()
            .return_const(self_test_req);

        let mut self_test_resp = MockSimplexPipe::<SelfTestRespPacket>::new();
        self_test_resp
            .expect_recv()
            .once()
            .return_const(Some(SelfTestRespPacket { result: Ok(()) }));
        env.expect_self_test_resp()
            .once()
            .return_const(self_test_resp);
        env
    });
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    // Let the resource get acquired by some other FSM.
    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let self_test_fsm_tag = scheduler.alloc(AdminFsm::Cast(CastFsm::new(ctx.clone())));
    assert!(self_test_fsm_tag.is_some());

    scheduler.on_event(AdminFsmEvent::SelfTestResponse, rsc_fsm_tag.unwrap());

    scheduler.map(self_test_fsm_tag.unwrap(), |fsm| {
        fsm.cast_fsm_set_next_test(SelfTest::EcdhEngineInstance0)
    });

    assert_eq!(
        scheduler.map(self_test_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::TimerElapsed, self_test_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::SelfTestResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(self_test_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::SelfTestResponse, self_test_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // The resource should be available.
    let resource = ctx.cast_idle().acquire(1, ());
    assert!(resource.is_some());
}

// Helper function for AES XTS and GCM self tests
fn aes_fp_self_test(self_test_fsm: &mut CastFsm<MockAdminEnvTrait>) {
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    // begin enc dec
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );

    // continue enc dec
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );

    // end enc dec
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[cfg(not(feature = "fips_validation_hooks"))]
#[test]
fn test_aes_xts_self_test_success() {
    let configs = AdminFsmSelfTestTestConfigs {
        test_success: true,
        hsm_executed_self_test: false,
        execute_aes_xts_neg_enc: true,
        num_begin_enc_dec: 2,
        num_end_enc_dec: 2,
        num_zeroize_buffers: 1,
        num_xts_send_requests: 2,
        num_xts_receive_responses: 2,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);

    let mut self_test_fsm = CastFsm::new(ctx);

    aes_fp_self_test(&mut self_test_fsm);
}

#[cfg(not(feature = "fips_validation_hooks"))]
#[test]
fn test_aes_gcm_self_test_success() {
    let configs = AdminFsmSelfTestTestConfigs {
        test_success: true,
        hsm_executed_self_test: false,
        execute_aes_gcm_aligned_and_unaligned_data: true,
        num_begin_enc_dec: 2,
        num_end_enc_dec: 2,
        num_zeroize_buffers: 1,
        num_gcm_send_requests: 2,
        num_tag_correction: 2,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);
    self_test_fsm.set_next_test(SelfTest::AesGcmAlignedAndUnalignedData);

    aes_fp_self_test(&mut self_test_fsm);
}

#[cfg(not(feature = "fips_validation_hooks"))]
#[test]
#[should_panic]
fn test_aes_xts_fail_import_key1() {
    let configs = AdminFsmSelfTestTestConfigs {
        test_success: true,
        hsm_executed_self_test: false,
        execute_aes_xts_neg_enc: true,
        num_zeroize_buffers: 1,
        notify_self_test_failure: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);

    // import key 1
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::CdmaIoAesXtsSelfTestFailed)
    );
}

#[cfg(not(feature = "fips_validation_hooks"))]
#[test]
#[should_panic]
fn test_aes_xts_fail_import_key2() {
    let configs = AdminFsmSelfTestTestConfigs {
        test_success: true,
        hsm_executed_self_test: false,
        execute_aes_xts_neg_enc: true,
        num_xts_send_requests: 1,
        num_xts_receive_responses: 1,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);

    // import key 1
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    // import key 2 if XTS
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::CdmaIoAesXtsSelfTestFailed)
    );
}

#[cfg(not(feature = "fips_validation_hooks"))]
#[test]
#[should_panic]
fn test_aes_xts_fail_begin_enc_dec() {
    let configs = AdminFsmSelfTestTestConfigs {
        test_success: true,
        hsm_executed_self_test: false,
        begin_enc_dec_failure: true,
        execute_aes_xts_neg_enc: true,
        num_begin_enc_dec: 1,
        num_xts_send_requests: 2,
        num_xts_receive_responses: 2,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);

    // import key 1
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    // import key 2 if XTS
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );

    // begin enc dec
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::CdmaIoAesXtsSelfTestFailed)
    );
}

#[cfg(not(feature = "fips_validation_hooks"))]
#[test]
#[should_panic]
fn test_aes_xts_fail_end_enc_dec() {
    let configs = AdminFsmSelfTestTestConfigs {
        test_success: true,
        hsm_executed_self_test: false,
        end_enc_dec_failure: true,
        execute_aes_xts_neg_enc: true,
        num_begin_enc_dec: 1,
        num_end_enc_dec: 1,
        num_zeroize_buffers: 1,
        num_xts_send_requests: 1,
        num_xts_receive_responses: 1,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);

    // import key 1
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    // import key 2 if XTS
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );

    // begin enc dec
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );

    // end enc dec
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::CdmaIoAesXtsSelfTestFailed)
    );
}

#[cfg(not(feature = "fips_validation_hooks"))]
#[test]
#[should_panic]
fn test_aes_gcm_fail_begin_enc_dec() {
    let configs = AdminFsmSelfTestTestConfigs {
        test_success: true,
        hsm_executed_self_test: false,
        begin_enc_dec_failure: true,
        execute_aes_gcm_aligned_and_unaligned_data: true,
        num_begin_enc_dec: 1,
        num_gcm_send_requests: 1,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);
    self_test_fsm.set_next_test(SelfTest::AesGcmAlignedAndUnalignedData);

    // import key 1
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    // begin enc dec
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::CdmaIoAesGcmSelfTestFailed)
    );
}

#[cfg(not(feature = "fips_validation_hooks"))]
#[test]
#[should_panic]
fn test_aes_gcm_fail_end_enc_dec() {
    let configs = AdminFsmSelfTestTestConfigs {
        test_success: true,
        hsm_executed_self_test: false,
        execute_aes_gcm_aligned_and_unaligned_data: true,
        num_begin_enc_dec: 1,
        num_end_enc_dec: 1,
        num_zeroize_buffers: 1,
        num_gcm_send_requests: 2,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);
    self_test_fsm.set_next_test(SelfTest::AesGcmAlignedAndUnalignedData);

    // import key 1
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    // begin enc dec
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );

    // end enc dec
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::CdmaIoAesGcmSelfTestFailed)
    );
}

#[cfg(not(feature = "fips_validation_hooks"))]
#[test]
#[should_panic]
fn test_aes_fp_self_test_fail_ipc_send() {
    let configs = AdminFsmSelfTestTestConfigs {
        test_success: true,
        hsm_executed_self_test: false,
        ipc_send_failure: true,
        execute_aes_gcm_aligned_and_unaligned_data: true,
        num_gcm_send_requests: 1,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);
    self_test_fsm.set_next_test(SelfTest::AesGcmAlignedAndUnalignedData);

    // import key 1
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::CdmaIoKeyUpdateFailed)
    );
}

#[cfg(not(feature = "fips_validation_hooks"))]
#[test]
#[should_panic]
fn test_aes_fp_self_test_fail_ipc_resp() {
    let configs = AdminFsmSelfTestTestConfigs {
        test_success: true,
        hsm_executed_self_test: false,
        fp_receive_message_status: 1,
        execute_aes_gcm_aligned_and_unaligned_data: true,
        num_gcm_send_requests: 1,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);
    self_test_fsm.set_next_test(SelfTest::AesGcmAlignedAndUnalignedData);

    // import key 1
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    // begin enc dec
    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::CdmaIoAesGcmSelfTestFailed)
    );
}

#[cfg(not(feature = "fips_validation_hooks"))]
#[test]
fn test_aes_fp_self_test_wait_for_ipc_resource() {
    let configs = AdminFsmSelfTestTestConfigs {
        test_success: true,
        hsm_executed_self_test: false,
        execute_aes_gcm_aligned_and_unaligned_data: true,
        num_begin_enc_dec: 2,
        num_end_enc_dec: 2,
        num_zeroize_buffers: 1,
        num_gcm_send_requests: 4,
        num_tag_correction: 2,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_fsm_test_for_self_test(configs);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    // Let the resource get acquired by some other FSM.
    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let self_test_fsm_tag = scheduler.alloc(AdminFsm::Cast(CastFsm::new(ctx)));
    assert!(self_test_fsm_tag.is_some());

    // Acquire admin to fp ipc channel resource to create contention scenario
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    scheduler.map(self_test_fsm_tag.unwrap(), |fsm| {
        fsm.cast_fsm_set_next_test(SelfTest::AesGcmAlignedAndUnalignedData)
    });

    assert_eq!(
        scheduler.map(self_test_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::TimerElapsed, self_test_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    // begin enc dec
    assert_eq!(
        scheduler.map(self_test_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::FpToAdminIpcResponse,
            self_test_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // continue enc dec
    assert_eq!(
        scheduler.map(self_test_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::FpToAdminIpcResponse,
            self_test_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // end enc dec
    assert_eq!(
        scheduler.map(self_test_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::FpToAdminIpcResponse,
            self_test_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // delete key
    assert_eq!(
        scheduler.map(self_test_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::FpToAdminIpcResponse,
            self_test_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
#[should_panic]
fn test_aes_gcm_negative_self_test() {
    let configs = AdminFsmSelfTestTestConfigs {
        hsm_executed_self_test: false,
        execute_aes_gcm_aligned_and_unaligned_data: true,
        num_begin_enc_dec: 1,
        num_end_enc_dec: 1,
        num_zeroize_buffers: 2,
        num_gcm_send_requests: 2,
        end_enc_dec_failure: true,
        notify_self_test_failure: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);

    assert_eq!(
        self_test_fsm.on_event(
            AdminFsmEvent::NegativeSelfTest(mcr_self_test::SelfTest::AesGcmAlignedAndUnalignedData),
            0xFF
        ),
        Err(AdminErr::Pending)
    );

    aes_fp_self_test(&mut self_test_fsm);
}

#[test]
#[should_panic]
fn test_aes_xts_negative_self_test() {
    let configs = AdminFsmSelfTestTestConfigs {
        hsm_executed_self_test: false,
        execute_aes_xts_neg_enc: true,
        num_begin_enc_dec: 1,
        num_end_enc_dec: 1,
        num_zeroize_buffers: 2,
        num_xts_send_requests: 2,
        num_xts_receive_responses: 2,
        end_enc_dec_failure: true,
        notify_self_test_failure: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);

    assert_eq!(
        self_test_fsm.on_event(
            AdminFsmEvent::NegativeSelfTest(mcr_self_test::SelfTest::AesXtsNegEnc),
            0xFF
        ),
        Err(AdminErr::Pending)
    );

    aes_fp_self_test(&mut self_test_fsm);
}

#[test]
#[should_panic]
fn test_aes_ecb_negative_self_test() {
    let configs = AdminFsmSelfTestTestConfigs {
        execute_aes_ecb: true,
        notify_self_test_failure: true,
        hsm_executed_self_test: false,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);

    assert_eq!(
        self_test_fsm.on_event(
            AdminFsmEvent::NegativeSelfTest(mcr_self_test::SelfTest::AesEcb),
            0xFF
        ),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[cfg(not(feature = "fips_validation_hooks"))]
#[test]
fn test_aes_unwrap_with_padding_self_test() {
    let configs = AdminFsmSelfTestTestConfigs {
        hsm_executed_self_test: false,
        execute_aes_unwrap: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_self_test(configs);
    let mut self_test_fsm = CastFsm::new(ctx);
    self_test_fsm.set_next_test(SelfTest::AesUnwrapWithPadding);

    assert_eq!(
        self_test_fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );
}
