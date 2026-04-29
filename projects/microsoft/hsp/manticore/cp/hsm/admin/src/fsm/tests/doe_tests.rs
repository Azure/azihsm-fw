// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_doe::DoeEvents;
use test_log::test;

use super::helper::*;
use crate::cmd_scheduler::*;
use crate::context::AdminFsmContext;
use crate::error::AdminErr;
use crate::event::AdminFsmEvent;
use crate::fsm::{AdminFsm, DoeFsm, ResId, ResourceTestFsm};
use crate::mock::MockAdminEnvTrait;
use crate::recorder::AdminFsmEventRecorder;
use crate::resource::AdminFsmResourceId;

// DOE test state machine configuration
#[derive(Clone, Copy, Default)]
pub(crate) struct AdminFsmDoeTestConfigs {
    pub perst_up: bool,
    pub link_up: bool,
    pub wait_doe_idle: bool,
    pub doe_abort: bool,
    pub doe_error: bool,
    pub rx_ready: bool,
    pub doe_go: bool,
    pub hsp_response: bool,
    pub tx_ready: bool,
    pub tx_done: bool,
    pub rx_ready_err: bool,
    pub doe_go_recv_err: bool,
    pub doe_go_end_recv_err: bool,
    pub hsp_response_send_err: bool,
    pub hsp_response_spurious_msg: bool,
    pub hsp_response_header_err: bool,
    pub hsp_response_after_abort: bool,
    pub tx_ready_err: bool,
    pub send_request_err: bool,
    pub perst_down: bool,
    pub rx_ready_timeout: bool,
}

const TAG: TagId = 0xFF;

#[test]
fn doe_tests_acquire_resource_err() {
    let config = AdminFsmDoeTestConfigs::default();
    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    // Use incorrect resource
    let event = fsm.acquire_resource(TAG, AdminFsmResourceId::HsmIpcChannel);
    assert!(event == AdminFsmEvent::Unknown);
}

#[test]
fn doe_tests_acquire_resource() {
    let config = AdminFsmDoeTestConfigs::default();
    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    let event = fsm.acquire_resource(TAG, AdminFsmResourceId::HspIpcChannel);
    assert!(event == AdminFsmEvent::ResourceReady(ResId::HspIpcChannel));

    let event = fsm.acquire_resource(TAG, AdminFsmResourceId::DoeIdle);
    assert!(event == AdminFsmEvent::ResourceReady(ResId::DoeIdle));
}

#[test]
fn doe_tests_unknown_event() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_link_up_already() {
    let config = AdminFsmDoeTestConfigs {
        link_up: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);
}

#[test]
fn doe_tests_wait_doe_idle() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        wait_doe_idle: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), rsc_fsm_tag.unwrap());
}

#[test]
fn doe_tests_wait_ipc_channel() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::RxReady),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HspToAdminIpcResponse, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn doe_tests_release_doe_idle_by_perst_down() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // At this time, FSM is not in IDLE state, the outside shall fail to acquire it
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstDown, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn doe_tests_release_doe_idle_by_doe_abort() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        doe_abort: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // At this time, FSM is not in IDLE state, the outside shall fail to acquire it
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::DoeAbort),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn doe_tests_release_doe_idle_by_flr() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        doe_abort: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // At this time, FSM is not in IDLE state, the outside shall fail to acquire it
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PcieFlr, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn doe_tests_release_doe_idle_by_poisoned_config_write() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        doe_error: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // At this time, FSM is not in IDLE state, the outside shall fail to acquire it
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::PoisonedConfigWrite),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn doe_tests_release_doe_idle_by_tx_overflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        doe_error: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // At this time, FSM is not in IDLE state, the outside shall fail to acquire it
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::TxOverflow),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn doe_tests_release_doe_idle_by_rx_underflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        doe_error: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // At this time, FSM is not in IDLE state, the outside shall fail to acquire it
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::RxUnderflow),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn doe_tests_release_doe_idle_normal_flow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        tx_ready: true,
        tx_done: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // At this time, FSM is not in IDLE state, the outside shall fail to acquire it
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::RxReady),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HspToAdminIpcResponse, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::TxReady),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::Doe(DoeEvents::TxDone), doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // At this point, the doe_idle will be released and acquired by another FSM
}

#[test]
fn doe_tests_rx_ready() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_rx_ready_err() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        rx_ready_err: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_doe_go() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_doe_go_2() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        doe_go: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_doe_go_recv_err() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        doe_go_recv_err: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_doe_go_end_recv_err() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        doe_go_end_recv_err: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_hsp_response() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_hsp_response_send_err() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        hsp_response_send_err: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
#[should_panic]
fn doe_tests_hsp_response_spurious_msg() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        hsp_response_spurious_msg: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_hsp_response_header_err() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        hsp_response_header_err: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_tx_ready() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        tx_ready: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxReady), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_tx_ready_err() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        tx_ready: true,
        tx_ready_err: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxReady), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_tx_done() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        tx_ready: true,
        tx_done: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxDone), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
#[should_panic]
fn doe_tests_hsp_send_request_err() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        send_request_err: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_idle_abort() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        doe_abort: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeAbort), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_idle_flr() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        doe_abort: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_rx_abort() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_abort: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeAbort), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_rx_flr() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_abort: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_wait_ipc_response_abort() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response_after_abort: true,
        doe_abort: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeAbort), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_wait_ipc_response_flr() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response_after_abort: true,
        doe_abort: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_after_ipc_response_abort() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        doe_abort: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeAbort), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_after_ipc_response_flr() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        doe_abort: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_tx_ready_abort() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        tx_ready: true,
        doe_abort: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeAbort), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_tx_ready_flr() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        tx_ready: true,
        doe_abort: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_wait_doe_idle_abort() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        wait_doe_idle: true,
        doe_abort: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::DoeAbort),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());
}

#[test]
fn doe_tests_wait_doe_idle_flr() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        wait_doe_idle: true,
        doe_abort: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PcieFlr, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());
}

#[test]
fn doe_tests_wait_ipc_channel_abort() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        doe_abort: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::RxReady),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::DoeAbort),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());
}

#[test]
fn doe_tests_wait_ipc_channel_flr() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        doe_abort: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::RxReady),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PcieFlr, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());
}

#[test]
fn doe_tests_idle_perst_down() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        perst_down: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstDown, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_rx_perst_down() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        perst_down: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstDown, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_wait_ipc_response_perst_down() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response_after_abort: true,
        perst_down: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstDown, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_after_ipc_response_perst_down() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        perst_down: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstDown, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_tx_ready_perst_down() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        tx_ready: true,
        perst_down: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstDown, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_wait_doe_idle_perst_down() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        wait_doe_idle: true,
        perst_down: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstDown, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());
}

#[test]
fn doe_tests_wait_ipc_channel_perst_down() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        perst_down: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::RxReady),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstDown, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());
}

#[test]
fn doe_tests_rx_ready_timeout() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready_timeout: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_ipc_response_timeout() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response_after_abort: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_idle_tx_overflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxOverflow), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_rx_tx_overflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxOverflow), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_wait_ipc_response_tx_overflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response_after_abort: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxOverflow), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_after_ipc_response_tx_overflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxOverflow), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_tx_ready_tx_overflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        tx_ready: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxOverflow), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_wait_doe_idle_tx_overflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        wait_doe_idle: true,
        doe_error: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::TxOverflow),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());
}

#[test]
fn doe_tests_wait_ipc_channel_tx_overflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        doe_error: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::RxReady),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::TxOverflow),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());
}

#[test]
fn doe_tests_idle_rx_underflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxUnderflow), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_rx_rx_underflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxUnderflow), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_wait_ipc_response_rx_underflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response_after_abort: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxUnderflow), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_after_ipc_response_rx_underflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxUnderflow), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_tx_ready_rx_underflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        tx_ready: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxUnderflow), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_wait_doe_idle_rx_underflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        wait_doe_idle: true,
        doe_error: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::RxUnderflow),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());
}

#[test]
fn doe_tests_wait_ipc_channel_rx_underflow() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        doe_error: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::RxReady),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::RxUnderflow),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());
}

#[test]
fn doe_tests_idle_poisoned_config_write() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::PoisonedConfigWrite), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_rx_poisoned_config_write() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::PoisonedConfigWrite), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_wait_ipc_response_poisoned_config_write() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response_after_abort: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::PoisonedConfigWrite), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_after_ipc_response_poisoned_config_write() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::PoisonedConfigWrite), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_tx_ready_poisoned_config_write() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        hsp_response: true,
        tx_ready: true,
        doe_error: true,
        ..Default::default()
    };

    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::PoisonedConfigWrite), TAG),
        Err(AdminErr::Pending)
    );
}

#[test]
fn doe_tests_wait_doe_idle_poisoned_config_write() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        wait_doe_idle: true,
        doe_error: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::PoisonedConfigWrite),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), rsc_fsm_tag.unwrap());
}

#[test]
fn doe_tests_wait_ipc_channel_poisoned_config_write() {
    let config = AdminFsmDoeTestConfigs {
        perst_up: true,
        rx_ready: true,
        doe_go: true,
        doe_error: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_doe_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let doe_fsm_tag = scheduler.alloc(AdminFsm::Doe(DoeFsm::new(ctx)));
    assert!(doe_fsm_tag.is_some());

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DoeFsmInit, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::PciePerstUp, doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::RxReady),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), doe_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(doe_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::Doe(DoeEvents::PoisonedConfigWrite),
            doe_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // The resource gets dropped by the other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());
}

#[test]
fn doe_tests_standby_no_op_events() {
    let config = AdminFsmDoeTestConfigs::default();
    let ctx = make_doe_fsm(config);
    let mut fsm = DoeFsm::new(ctx);
    let _ = fsm.on_event(AdminFsmEvent::DoeFsmInit, TAG);

    // Now the FSM is in standby state
    // All the following events should not trigger any state transition
    assert_eq!(
        fsm.on_event(AdminFsmEvent::ResourceReady(ResId::DoeIdle), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::ResourceReady(ResId::HspIpcChannel), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::DoeGo), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxReady), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxDone), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::TxOverflow), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::RxUnderflow), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::Doe(DoeEvents::PoisonedConfigWrite), TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, TAG),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstDown, TAG),
        Err(AdminErr::Pending)
    );
}
