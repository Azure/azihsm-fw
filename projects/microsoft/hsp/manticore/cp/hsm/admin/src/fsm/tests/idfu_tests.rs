// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_doe::DoeEvents;
use mcr_ipc_message::IpcMessageEncoderTrait;
use mcr_ipc_message::IpcMessageShutdown;
use mcr_ipc_message::IpcMessageStatusCode;
use mcr_ipc_message::ShutdownInfo;
use mcr_tcon::Tcon;
use test_log::test;

use super::helper::*;
use crate::cmd_scheduler::CmdFsm;
use crate::event::AdminFsmEvent;
use crate::fsm::tests::harness::AdminFsmTest;
use crate::fsm::AdminErr;
use crate::fsm::AdminFsm;
use crate::fsm::IdfuFsm;
use crate::fsm::ResId;
use crate::fsm::ResourceTestFsm;
use crate::mock::MockAdminEnvTrait;
use crate::mock::MockIpcMessageChannel;
use crate::AdminFsmContext;
use crate::AdminFsmEventRecorder;
use crate::CmdScheduler;

pub(crate) const IDFU_IPC_TAG: u32 = 0xDF;
pub(crate) const IDFU_FSM_TAG: u16 = 0x1DF;
pub(crate) const IDFU_DEFAULT_DRAIN_TIME_MS: u32 = 1500;
pub(crate) const IDFU_SIMULATED_TSC_INC_MS: u32 = 50;

// IDFU test state machine configuration
#[derive(Clone, Copy)]
pub(crate) struct IdfuTestConfig {
    pub(crate) ipc_cp_hsm_shutdown_req_send_ok: bool,
    pub(crate) ipc_fp_shutdown_req_send_ok: bool,

    pub(crate) ipc_sp_shutdown_rsp_send_ok: bool,

    pub(crate) ipc_cp_hsm_shutdown_rsp_reports_ok: bool,
    pub(crate) ipc_fp_shutdown_rsp_reports_ok: bool,

    pub(crate) drain_abort: bool,
    pub(crate) wait_timeout: bool,
    pub(crate) expect_rollback: bool,
    pub(crate) expect_doe_acquired: bool,

    pub(crate) simulated_tsc: u64,

    pub(crate) pause_queue_controller: bool,
    pub(crate) resume_queue_controller: bool,
}

impl Default for IdfuTestConfig {
    fn default() -> Self {
        Self {
            ipc_cp_hsm_shutdown_req_send_ok: true,
            ipc_fp_shutdown_req_send_ok: true,

            ipc_sp_shutdown_rsp_send_ok: true,

            ipc_cp_hsm_shutdown_rsp_reports_ok: true,
            ipc_fp_shutdown_rsp_reports_ok: true,

            drain_abort: false,
            wait_timeout: false,
            expect_rollback: false,
            expect_doe_acquired: true,

            simulated_tsc: 0,

            pause_queue_controller: false,
            resume_queue_controller: false,
        }
    }
}

fn prepare_ipc_message() -> IpcMessageShutdown {
    let mut msg = IpcMessageShutdown {
        info: ShutdownInfo {
            drain_time_ms: 0x7D0,
        },
        ..Default::default()
    };

    msg.header.set_tag(IDFU_IPC_TAG);

    msg
}

/// Set default expectations on the resources held in handler
pub(crate) fn idfu_resource_expectations(test: &mut AdminFsmTest, config: IdfuTestConfig) {
    // Create AdminFsmContext
    test.admin_to_fp_ipc_channel()
        .expect_clone()
        .once()
        .returning(move || {
            let mut channel = MockIpcMessageChannel::new();

            // CP Admin Drain Timeout will prevent CP HSM/FP IPC Shutdown Request
            if !config.drain_abort {
                if config.ipc_fp_shutdown_req_send_ok {
                    channel.expect_send_request().once().return_const(Ok(()));
                } else {
                    channel
                        .expect_send_request()
                        .once()
                        .return_const(Err(0xBAD));
                }

                if !config.wait_timeout {
                    channel.expect_receive_message().once().returning(move || {
                        let mut response = prepare_ipc_message();
                        response.header.set_response(true);

                        match config.ipc_fp_shutdown_rsp_reports_ok {
                            true => {
                                response
                                    .header
                                    .set_status(IpcMessageStatusCode::Success.into());
                            }
                            false => {
                                response
                                    .header
                                    .set_status(IpcMessageStatusCode::OperationFailed.into());
                            }
                        };

                        Some(response.encode())
                    });
                }
            }

            channel
        });

    test.hsm_ipc_channel()
        .expect_clone()
        .once()
        .returning(move || {
            let mut channel = MockIpcMessageChannel::new();

            // CP Admin Drain Timeout will prevent CP HSM/FP IPC Shutdown Request
            if !config.drain_abort {
                if config.ipc_cp_hsm_shutdown_req_send_ok {
                    channel.expect_send_request().once().return_const(Ok(()));
                } else {
                    channel
                        .expect_send_request()
                        .once()
                        .return_const(Err(0xBAD));
                }

                if !config.wait_timeout {
                    channel.expect_receive_message().once().returning(move || {
                        let mut response = prepare_ipc_message();
                        response.header.set_response(true);

                        match config.ipc_cp_hsm_shutdown_rsp_reports_ok {
                            true => {
                                response
                                    .header
                                    .set_status(IpcMessageStatusCode::Success.into());
                            }
                            false => {
                                response
                                    .header
                                    .set_status(IpcMessageStatusCode::OperationFailed.into());
                            }
                        };

                        Some(response.encode())
                    });
                }
            }

            channel
        });

    test.admin_to_hsp_ipc_channel()
        .expect_clone()
        .once()
        .returning(MockIpcMessageChannel::new);

    if config.ipc_sp_shutdown_rsp_send_ok {
        test.hsp_to_admin_ipc_channel()
            .expect_send_response()
            .once()
            .return_const(Ok(()));
    } else {
        test.hsp_to_admin_ipc_channel()
            .expect_send_response()
            .once()
            .return_const(Err(0xBAD));
    }

    test.io_controller()
        .expect_pause_inbound()
        .once()
        .return_const(());
    test.fp_io_controller()
        .expect_pause_inbound()
        .once()
        .return_const(());

    if config.expect_doe_acquired {
        test.pcie_doe()
            .expect_set_busy()
            .withf(|val: &bool| *val)
            .once()
            .return_const(());
    }

    if config.expect_rollback {
        test.io_controller()
            .expect_resume_inbound()
            .once()
            .return_const(());
        test.fp_io_controller()
            .expect_resume_inbound()
            .once()
            .return_const(());

        if config.expect_doe_acquired {
            test.pcie_doe()
                .expect_set_busy()
                .withf(|val: &bool| !(*val))
                .once()
                .return_const(());
        }
    }
}

fn get_idfu_test_context(mut config: IdfuTestConfig) -> AdminFsmContext<MockAdminEnvTrait> {
    let mut test = AdminFsmTest::default();
    idfu_resource_expectations(&mut test, config);
    let mut env = test.env();

    env.expect_tcon_tsc().returning(move || {
        config.simulated_tsc += (IDFU_SIMULATED_TSC_INC_MS * Tcon::tsc_freq_hz() / 1000) as u64;
        config.simulated_tsc
    });

    env.expect_update_core_liveliness().return_const(());

    if config.pause_queue_controller {
        env.expect_pause_queue_controller().once().return_const(());
    }
    if config.resume_queue_controller {
        env.expect_resume_queue_controller().once().return_const(());
    }

    get_context_from_environment(env)
}

fn get_idfu_test_fsm(config: IdfuTestConfig) -> IdfuFsm<MockAdminEnvTrait> {
    IdfuFsm::new(get_idfu_test_context(config))
}

#[test]
fn idfu_tests_acquire_resource_err() {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);
    let mut fsm = IdfuFsm::new(get_context(test));

    // Use incorrect resource
    let event = fsm.acquire_resource(IDFU_FSM_TAG, ResId::HspIpcChannel);
    assert!(event == AdminFsmEvent::Unknown);
}

#[test]
fn idfu_tests_acquire_resource() {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);
    let mut fsm = IdfuFsm::new(get_context(test));

    let event = fsm.acquire_resource(IDFU_FSM_TAG, ResId::HsmIpcChannel);
    assert!(event == AdminFsmEvent::ResourceReady(ResId::HsmIpcChannel));

    let event = fsm.acquire_resource(IDFU_FSM_TAG, ResId::AdminToFpIpcChannel);
    assert!(event == AdminFsmEvent::ResourceReady(ResId::AdminToFpIpcChannel));

    let event = fsm.acquire_resource(IDFU_FSM_TAG, ResId::CastIdle);
    assert!(event == AdminFsmEvent::ResourceReady(ResId::CastIdle));

    let event = fsm.acquire_resource(IDFU_FSM_TAG, ResId::DoeIdle);
    assert!(event == AdminFsmEvent::ResourceReady(ResId::DoeIdle));

    let event = fsm.acquire_resource(IDFU_FSM_TAG, ResId::TdispIdle);
    assert!(event == AdminFsmEvent::ResourceReady(ResId::TdispIdle));
}

#[test]
fn idfu_tests_state_normal_unknown_event() {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);
    let mut fsm = IdfuFsm::new(get_context(test));

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_state_normal_late_hsm_ipc_response_event() {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);
    let mut env = test.env();
    env.expect_tcon_tsc().return_const(0u64);
    let mut fsm = IdfuFsm::new(get_context_from_environment(env));

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_state_normal_late_fp_ipc_response_event() {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);
    let mut env = test.env();
    env.expect_tcon_tsc().return_const(0u64);
    let mut fsm = IdfuFsm::new(get_context_from_environment(env));

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_get_timer() {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);
    let mut fsm = IdfuFsm::new(get_context(test));

    assert!(fsm.get_timer().is_some());
}

#[test]
fn idfu_tests_happy_path() {
    let config = IdfuTestConfig {
        pause_queue_controller: true,
        ..Default::default()
    };
    let mut fsm = get_idfu_test_fsm(config);

    // SP IPC Shutdown Request
    assert_eq!(
        fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS
                },
                IDFU_IPC_TAG
            )),
            IDFU_FSM_TAG
        ),
        Err(AdminErr::Pending)
    );

    // Scheduler Queues Empty
    assert_eq!(
        fsm.on_event(AdminFsmEvent::SchedulerQueueEmptyEvent, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // HSM IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // FP IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_happy_path_res_busy_sequence1() {
    let mut config = IdfuTestConfig::default();
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = AdminFsmTest::default();
    idfu_resource_expectations(&mut fsm_test, config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    env.expect_tcon_tsc().returning(move || {
        config.simulated_tsc += (IDFU_SIMULATED_TSC_INC_MS * Tcon::tsc_freq_hz() / 1000) as u64;
        config.simulated_tsc
    });
    env.expect_update_core_liveliness().return_const(());
    env.expect_pause_queue_controller().once().return_const(());
    env.expect_resume_queue_controller().once().return_const(());

    let ctx = AdminFsmContext::new(env, scheduler.clone());

    // Let the resource get acquired by some other FSM.
    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let idfu_fsm_tag = scheduler.alloc(AdminFsm::Idfu(IdfuFsm::new(ctx)));
    assert!(idfu_fsm_tag.is_some());

    scheduler.on_event(AdminFsmEvent::SelfTestResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::Ide(None), rsc_fsm_tag.unwrap());

    // SP IPC Shutdown Request
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS,
                },
                idfu_fsm_tag.unwrap().into(),
            )),
            idfu_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    scheduler.on_event(AdminFsmEvent::SelfTestResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::Ide(None), rsc_fsm_tag.unwrap());

    // Scheduler Queues Empty
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::SchedulerQueueEmptyEvent,
            idfu_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // HSM IPC Response
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HsmIpcResponse, idfu_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // FP IPC Response
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::FpToAdminIpcResponse, idfu_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn idfu_tests_happy_path_res_busy_sequence2() {
    let mut config = IdfuTestConfig::default();
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = AdminFsmTest::default();
    idfu_resource_expectations(&mut fsm_test, config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    env.expect_tcon_tsc().returning(move || {
        config.simulated_tsc += (IDFU_SIMULATED_TSC_INC_MS * Tcon::tsc_freq_hz() / 1000) as u64;
        config.simulated_tsc
    });
    env.expect_update_core_liveliness().return_const(());
    env.expect_pause_queue_controller().once().return_const(());
    env.expect_resume_queue_controller().once().return_const(());

    let ctx = AdminFsmContext::new(env, scheduler.clone());

    // Let the resource get acquired by some other FSM.
    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let idfu_fsm_tag = scheduler.alloc(AdminFsm::Idfu(IdfuFsm::new(ctx)));
    assert!(idfu_fsm_tag.is_some());

    scheduler.on_event(AdminFsmEvent::SelfTestResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::Ide(None), rsc_fsm_tag.unwrap());

    // SP IPC Shutdown Request
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS,
                },
                idfu_fsm_tag.unwrap().into(),
            )),
            idfu_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    scheduler.on_event(AdminFsmEvent::Ide(None), rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());

    // Scheduler Queues Empty
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::SchedulerQueueEmptyEvent,
            idfu_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    scheduler.on_event(AdminFsmEvent::SelfTestResponse, rsc_fsm_tag.unwrap());

    // HSM IPC Response
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HsmIpcResponse, idfu_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // FP IPC Response
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::FpToAdminIpcResponse, idfu_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn idfu_tests_happy_path_ipc_busy() {
    let mut config = IdfuTestConfig::default();
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = AdminFsmTest::default();
    idfu_resource_expectations(&mut fsm_test, config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    env.expect_tcon_tsc().returning(move || {
        config.simulated_tsc += (IDFU_SIMULATED_TSC_INC_MS * Tcon::tsc_freq_hz() / 1000) as u64;
        config.simulated_tsc
    });
    env.expect_update_core_liveliness().return_const(());
    env.expect_pause_queue_controller().once().return_const(());
    env.expect_resume_queue_controller().once().return_const(());

    let ctx = AdminFsmContext::new(env, scheduler.clone());

    // Let the resource get acquired by some other FSM.
    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let idfu_fsm_tag = scheduler.alloc(AdminFsm::Idfu(IdfuFsm::new(ctx)));
    assert!(idfu_fsm_tag.is_some());

    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    // SP IPC Shutdown Request
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS,
                },
                idfu_fsm_tag.unwrap().into(),
            )),
            idfu_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // Scheduler Queues Empty
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::SchedulerQueueEmptyEvent,
            idfu_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    // HSM IPC Response
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HsmIpcResponse, idfu_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // FP IPC Response
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::FpToAdminIpcResponse, idfu_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn idfu_tests_state_drain_timeout() {
    let config = IdfuTestConfig {
        drain_abort: true,
        expect_rollback: true,
        pause_queue_controller: true,
        resume_queue_controller: true,
        ..IdfuTestConfig::default()
    };

    let mut fsm = get_idfu_test_fsm(config);

    // SP IPC Shutdown Request
    assert_eq!(
        fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS
                },
                IDFU_IPC_TAG
            )),
            IDFU_FSM_TAG
        ),
        Err(AdminErr::Pending)
    );

    // Timeout waiting for Scheduler Queues Empty
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // HSM IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // FP IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_state_drain_timeout_late_resource_acquisition() {
    let mut config = IdfuTestConfig {
        drain_abort: true,
        expect_rollback: true,
        expect_doe_acquired: false,
        pause_queue_controller: true,
        resume_queue_controller: true,
        ..IdfuTestConfig::default()
    };
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = AdminFsmTest::default();
    idfu_resource_expectations(&mut fsm_test, config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    env.expect_tcon_tsc().returning(move || {
        config.simulated_tsc += (IDFU_SIMULATED_TSC_INC_MS * Tcon::tsc_freq_hz() / 1000) as u64;
        config.simulated_tsc
    });
    env.expect_update_core_liveliness().return_const(());
    env.expect_pause_queue_controller().once().return_const(());
    env.expect_resume_queue_controller().once().return_const(());

    let ctx = AdminFsmContext::new(env, scheduler.clone());

    // Let the resource get acquired by some other FSM.
    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let idfu_fsm_tag = scheduler.alloc(AdminFsm::Idfu(IdfuFsm::new(ctx)));
    assert!(idfu_fsm_tag.is_some());

    scheduler.on_event(AdminFsmEvent::SelfTestResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::Ide(None), rsc_fsm_tag.unwrap());

    // SP IPC Shutdown Request
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS,
                },
                idfu_fsm_tag.unwrap().into(),
            )),
            idfu_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::TimerElapsed, idfu_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    scheduler.on_event(AdminFsmEvent::SelfTestResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::Ide(None), rsc_fsm_tag.unwrap());
}

#[test]
fn idfu_tests_state_wait_late_ipc_acquisition() {
    let mut config = IdfuTestConfig {
        drain_abort: true,
        ipc_cp_hsm_shutdown_req_send_ok: false,
        ipc_fp_shutdown_req_send_ok: false,
        ipc_cp_hsm_shutdown_rsp_reports_ok: false,
        ipc_fp_shutdown_rsp_reports_ok: false,
        expect_rollback: true,
        pause_queue_controller: true,
        resume_queue_controller: true,
        ..IdfuTestConfig::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = AdminFsmTest::default();
    idfu_resource_expectations(&mut fsm_test, config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    env.expect_tcon_tsc().returning(move || {
        config.simulated_tsc += (IDFU_SIMULATED_TSC_INC_MS * Tcon::tsc_freq_hz() / 1000) as u64;
        config.simulated_tsc
    });
    env.expect_update_core_liveliness().return_const(());
    env.expect_pause_queue_controller().once().return_const(());
    env.expect_resume_queue_controller().once().return_const(());

    let ctx = AdminFsmContext::new(env, scheduler.clone());

    // Let the resource get acquired by some other FSM.
    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let idfu_fsm_tag = scheduler.alloc(AdminFsm::Idfu(IdfuFsm::new(ctx)));
    assert!(idfu_fsm_tag.is_some());

    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    // SP IPC Shutdown Request
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS,
                },
                idfu_fsm_tag.unwrap().into(),
            )),
            idfu_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::SchedulerQueueEmptyEvent,
            idfu_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::TimerElapsed, idfu_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());
}

#[test]
fn idfu_tests_doe_released_after_timeout() {
    let mut config = IdfuTestConfig::default();
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = AdminFsmTest::default();
    idfu_resource_expectations(&mut fsm_test, config);
    let mut env = fsm_test.env();
    env.expect_clone()
        .times(1..)
        .returning(MockAdminEnvTrait::new);
    env.expect_tcon_tsc().returning(move || {
        config.simulated_tsc += (IDFU_SIMULATED_TSC_INC_MS * Tcon::tsc_freq_hz() / 1000) as u64;
        config.simulated_tsc
    });
    env.expect_update_core_liveliness().return_const(());
    env.expect_pause_queue_controller().once().return_const(());
    env.expect_resume_queue_controller().once().return_const(());

    let ctx = AdminFsmContext::new(env, scheduler.clone());

    // Let the resource get acquired by some other FSM.
    // Use None for expected_ticks to avoid timer-based release
    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let idfu_fsm_tag = scheduler.alloc(AdminFsm::Idfu(IdfuFsm::new(ctx)));
    assert!(idfu_fsm_tag.is_some());

    // ResourceTestFsm acquires DOE resource
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());

    // SP IPC Shutdown Request
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS,
                },
                idfu_fsm_tag.unwrap().into(),
            )),
            idfu_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // Scheduler Queues Empty
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::SchedulerQueueEmptyEvent,
            idfu_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // Manually release DOE resource by sending the same event again
    // This simulates the resource being released after a timeout
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());

    // HSM IPC Response
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HsmIpcResponse, idfu_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // FP IPC Response
    assert_eq!(
        scheduler.map(idfu_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::FpToAdminIpcResponse, idfu_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn idfu_tests_state_wait_hsm_drain_failure() {
    let config = IdfuTestConfig {
        ipc_cp_hsm_shutdown_rsp_reports_ok: false,
        expect_rollback: true,
        pause_queue_controller: true,
        resume_queue_controller: true,
        ..IdfuTestConfig::default()
    };

    let mut fsm = get_idfu_test_fsm(config);

    // SP IPC Shutdown Request
    assert_eq!(
        fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS
                },
                IDFU_IPC_TAG
            )),
            IDFU_FSM_TAG
        ),
        Err(AdminErr::Pending)
    );

    // Scheduler Queues Empty
    assert_eq!(
        fsm.on_event(AdminFsmEvent::SchedulerQueueEmptyEvent, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // HSM IPC Response Fail
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // FP IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_state_wait_fp_drain_failure() {
    let config = IdfuTestConfig {
        ipc_fp_shutdown_rsp_reports_ok: false,
        expect_rollback: true,
        pause_queue_controller: true,
        resume_queue_controller: true,
        ..IdfuTestConfig::default()
    };

    let mut fsm = get_idfu_test_fsm(config);

    // SP IPC Shutdown Request
    assert_eq!(
        fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS
                },
                IDFU_IPC_TAG
            )),
            IDFU_FSM_TAG
        ),
        Err(AdminErr::Pending)
    );

    // Scheduler Queues Empty
    assert_eq!(
        fsm.on_event(AdminFsmEvent::SchedulerQueueEmptyEvent, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // FP IPC Response Fail
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // HSM IPC Response Fail
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_state_drain_spurious_hsm_ipc() {
    let config = IdfuTestConfig {
        drain_abort: true,
        ipc_fp_shutdown_rsp_reports_ok: false,
        expect_rollback: true,
        pause_queue_controller: true,
        resume_queue_controller: true,
        ..IdfuTestConfig::default()
    };

    let mut fsm = get_idfu_test_fsm(config);

    // SP IPC Shutdown Request
    assert_eq!(
        fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS
                },
                IDFU_IPC_TAG
            )),
            IDFU_FSM_TAG
        ),
        Err(AdminErr::Pending)
    );

    // Spurious/late CP HSM IPC
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // Scheduler Queues Empty
    assert_eq!(
        fsm.on_event(AdminFsmEvent::SchedulerQueueEmptyEvent, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_state_drain_spurious_fp_ipc() {
    let config = IdfuTestConfig {
        drain_abort: true,
        ipc_fp_shutdown_rsp_reports_ok: false,
        expect_rollback: true,
        pause_queue_controller: true,
        resume_queue_controller: true,
        ..IdfuTestConfig::default()
    };

    let mut fsm = get_idfu_test_fsm(config);

    // SP IPC Shutdown Request
    assert_eq!(
        fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS
                },
                IDFU_IPC_TAG
            )),
            IDFU_FSM_TAG
        ),
        Err(AdminErr::Pending)
    );

    // Spurious/late FPS IPC
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // Scheduler Queues Empty
    assert_eq!(
        fsm.on_event(AdminFsmEvent::SchedulerQueueEmptyEvent, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_state_drain_not_enough_time_for_hsm_drain() {
    let config = IdfuTestConfig {
        drain_abort: true,
        expect_rollback: true,
        pause_queue_controller: true,
        resume_queue_controller: true,
        ..IdfuTestConfig::default()
    };

    let mut fsm = get_idfu_test_fsm(config);

    // SP IPC Shutdown Request
    assert_eq!(
        fsm.on_event(
            AdminFsmEvent::ShutdownRequest((ShutdownInfo { drain_time_ms: 500 }, IDFU_IPC_TAG)),
            IDFU_FSM_TAG
        ),
        Err(AdminErr::Pending)
    );

    // Scheduler Queues Empty
    assert_eq!(
        fsm.on_event(AdminFsmEvent::SchedulerQueueEmptyEvent, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_state_wait_timeout() {
    let config = IdfuTestConfig {
        wait_timeout: true,
        expect_rollback: true,
        pause_queue_controller: true,
        resume_queue_controller: true,
        ..IdfuTestConfig::default()
    };

    let mut fsm = get_idfu_test_fsm(config);

    // SP IPC Shutdown Request
    assert_eq!(
        fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS
                },
                IDFU_IPC_TAG
            )),
            IDFU_FSM_TAG
        ),
        Err(AdminErr::Pending)
    );

    // Scheduler Queues Empty
    assert_eq!(
        fsm.on_event(AdminFsmEvent::SchedulerQueueEmptyEvent, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // Timeout
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_state_ready_timeout() {
    let config = IdfuTestConfig {
        expect_rollback: true,
        pause_queue_controller: true,
        resume_queue_controller: true,
        ..IdfuTestConfig::default()
    };

    let mut fsm = get_idfu_test_fsm(config);

    // SP IPC Shutdown Request
    assert_eq!(
        fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS
                },
                IDFU_IPC_TAG
            )),
            IDFU_FSM_TAG
        ),
        Err(AdminErr::Pending)
    );

    // Scheduler Queues Empty
    assert_eq!(
        fsm.on_event(AdminFsmEvent::SchedulerQueueEmptyEvent, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // HSM IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // FP IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // Timeout
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_state_ready_sp_response_fail() {
    let config = IdfuTestConfig {
        ipc_sp_shutdown_rsp_send_ok: false,
        expect_rollback: true,
        pause_queue_controller: true,
        resume_queue_controller: true,
        ..IdfuTestConfig::default()
    };

    let mut fsm = get_idfu_test_fsm(config);

    // SP IPC Shutdown Request
    assert_eq!(
        fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS
                },
                IDFU_IPC_TAG
            )),
            IDFU_FSM_TAG
        ),
        Err(AdminErr::Pending)
    );

    // Scheduler Queues Empty
    assert_eq!(
        fsm.on_event(AdminFsmEvent::SchedulerQueueEmptyEvent, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // HSM IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // FP IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_state_ready_spurious_empty_event() {
    let config = IdfuTestConfig {
        pause_queue_controller: true,
        ..IdfuTestConfig::default()
    };

    let mut fsm = get_idfu_test_fsm(config);

    // SP IPC Shutdown Request
    assert_eq!(
        fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS
                },
                IDFU_IPC_TAG
            )),
            IDFU_FSM_TAG
        ),
        Err(AdminErr::Pending)
    );

    // Scheduler Queues Empty
    assert_eq!(
        fsm.on_event(AdminFsmEvent::SchedulerQueueEmptyEvent, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // HSM IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // FP IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // Unknown event is ignored.
    assert_eq!(
        fsm.on_event(AdminFsmEvent::SchedulerQueueEmptyEvent, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_state_ready_unknown_event() {
    let config = IdfuTestConfig {
        pause_queue_controller: true,
        ..IdfuTestConfig::default()
    };
    let mut fsm = get_idfu_test_fsm(config);

    // SP IPC Shutdown Request
    assert_eq!(
        fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    drain_time_ms: IDFU_DEFAULT_DRAIN_TIME_MS
                },
                IDFU_IPC_TAG
            )),
            IDFU_FSM_TAG
        ),
        Err(AdminErr::Pending)
    );

    // Scheduler Queues Empty
    assert_eq!(
        fsm.on_event(AdminFsmEvent::SchedulerQueueEmptyEvent, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // HSM IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // FP IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // Unknown event is ignored.
    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn idfu_tests_state_ready_override_low_drain_time() {
    let config = IdfuTestConfig {
        pause_queue_controller: true,
        ..Default::default()
    };
    let mut fsm = get_idfu_test_fsm(config);

    // SP IPC Shutdown Request
    assert_eq!(
        fsm.on_event(
            AdminFsmEvent::ShutdownRequest((
                ShutdownInfo {
                    // Determined experimentally by placing a breakpoint in enter_dfu_ready such
                    // that remaining_ms < 1000.
                    drain_time_ms: IDFU_SIMULATED_TSC_INC_MS * 11
                },
                IDFU_IPC_TAG
            )),
            IDFU_FSM_TAG
        ),
        Err(AdminErr::Pending)
    );

    // Scheduler Queues Empty
    assert_eq!(
        fsm.on_event(AdminFsmEvent::SchedulerQueueEmptyEvent, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // HSM IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );

    // FP IPC Response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, IDFU_FSM_TAG),
        Err(AdminErr::Pending)
    );
}
