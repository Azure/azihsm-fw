// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_pcie_controller::TdispIntInfo;
use test_log::test;

use crate::event::AdminFsmEvent;
use crate::fsm::tests::helper::{make_tdisp_interrupt_fsm, make_tdisp_interrupt_fsm_test};
use crate::fsm::AdminErr;
use crate::fsm::AdminFsm;
use crate::fsm::ResId;
use crate::fsm::ResourceTestFsm;
use crate::fsm::TdispIntFsm;
use crate::mock::MockAdminEnvTrait;
use crate::resource::AdminFsmResourceId;
use crate::AdminFsmContext;
use crate::{cmd_scheduler::*, AdminFsmEventRecorder};

// TDISP Interrupt test state machine configuration
#[derive(Clone, Copy, Default)]
pub(crate) struct AdminFsmTdispIntTestConfigs {
    pub receive_tdisp_interrupt: bool,
    pub receive_ide_interrupt: bool,
    pub receive_flr_interrupt: bool,
    pub receive_perst_up_interrupt: bool,
    pub receive_perst_down_interrupt: bool,
    pub receive_tdisp_interrupt_info: TdispIntInfo,
    pub receive_response: bool,
    pub another_interrupt_during_response: bool,
    pub err_when_sending_ipc: bool,
    pub err_when_receiving_response: bool,
    pub err_when_invalid_header_response: bool,
    pub err_when_invalid_payload_response: bool,
}

#[test]
fn tdisp_interrupt_tests_acquire_resource_err() {
    let config = AdminFsmTdispIntTestConfigs::default();
    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    // Use incorrect resource
    let event = fsm.acquire_resource(0xFF, AdminFsmResourceId::HsmIpcChannel);
    assert!(event == AdminFsmEvent::Unknown);
}

#[test]
fn tdisp_interrupt_tests_acquire_resource() {
    let config = AdminFsmTdispIntTestConfigs::default();
    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    let event = fsm.acquire_resource(0xFF, AdminFsmResourceId::HspIpcChannel);

    assert!(event == AdminFsmEvent::ResourceReady(ResId::HspIpcChannel));
}

#[test]
fn tdisp_interrupt_tests_acquire_itself() {
    let config = AdminFsmTdispIntTestConfigs::default();
    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    let event = fsm.acquire_resource(0xFF, AdminFsmResourceId::TdispIdle);

    assert!(event == AdminFsmEvent::ResourceReady(ResId::TdispIdle));
}

#[test]
fn tdisp_interrupt_tests_unknown_event() {
    let config = AdminFsmTdispIntTestConfigs::default();
    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::DoeFsmInit, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn tdisp_interrupt_tests_tdisp_interrupt_event() {
    let tdisp_interrupt_info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmTdispIntTestConfigs {
        receive_tdisp_interrupt: true,
        receive_tdisp_interrupt_info: tdisp_interrupt_info,
        ..Default::default()
    };

    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(
            AdminFsmEvent::TdispInt(config.receive_tdisp_interrupt_info),
            0xFF
        ),
        Err(AdminErr::Pending)
    );
}

#[test]
fn tdisp_interrupt_tests_ide_interrupt_event() {
    let config = AdminFsmTdispIntTestConfigs {
        receive_ide_interrupt: true,
        ..Default::default()
    };

    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Ide(None), 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn tdisp_interrupt_tests_flr_interrupt_event() {
    let config = AdminFsmTdispIntTestConfigs {
        receive_flr_interrupt: true,
        ..Default::default()
    };

    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn tdisp_interrupt_tests_perst_up_interrupt_event() {
    let config = AdminFsmTdispIntTestConfigs {
        receive_perst_up_interrupt: true,
        ..Default::default()
    };

    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn tdisp_interrupt_tests_perst_down_interrupt_event() {
    let config = AdminFsmTdispIntTestConfigs {
        receive_perst_down_interrupt: true,
        ..Default::default()
    };

    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstDown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn tdisp_interrupt_tests_ipc_response() {
    let tdisp_interrupt_info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmTdispIntTestConfigs {
        receive_tdisp_interrupt: true,
        receive_tdisp_interrupt_info: tdisp_interrupt_info,
        receive_response: true,
        ..Default::default()
    };

    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TdispInt(tdisp_interrupt_info), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn tdisp_interrupt_tests_pending_bits_available() {
    let tdisp_interrupt_info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmTdispIntTestConfigs {
        receive_tdisp_interrupt: true,
        receive_tdisp_interrupt_info: tdisp_interrupt_info,
        receive_response: true,
        another_interrupt_during_response: true,
        ..Default::default()
    };

    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TdispInt(tdisp_interrupt_info), 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::TdispInt(tdisp_interrupt_info), 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn tdisp_interrupt_tests_wait_ipc_channel() {
    let tdisp_interrupt_info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmTdispIntTestConfigs {
        receive_tdisp_interrupt: true,
        receive_tdisp_interrupt_info: tdisp_interrupt_info,
        receive_response: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_tdisp_interrupt_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let tdisp_fsm_tag = scheduler.alloc(AdminFsm::TdispInt(TdispIntFsm::new(ctx)));
    assert!(tdisp_fsm_tag.is_some());

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(tdisp_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::TdispInt(tdisp_interrupt_info),
            tdisp_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::HspToAdminIpcRequest, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(tdisp_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::HspToAdminIpcResponse,
            tdisp_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn tdisp_interrupt_tests_wait_tdisp_idle() {
    let tdisp_interrupt_info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmTdispIntTestConfigs {
        receive_tdisp_interrupt: true,
        receive_tdisp_interrupt_info: tdisp_interrupt_info,
        receive_response: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_tdisp_interrupt_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let tdisp_fsm_tag = scheduler.alloc(AdminFsm::TdispInt(TdispIntFsm::new(ctx)));
    assert!(tdisp_fsm_tag.is_some());

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::Ide(None), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(tdisp_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::TdispInt(tdisp_interrupt_info),
            tdisp_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::Ide(None), rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(tdisp_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::HspToAdminIpcResponse,
            tdisp_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn tdisp_interrupt_tests_lock_tdisp_idle() {
    let tdisp_interrupt_info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmTdispIntTestConfigs {
        receive_tdisp_interrupt: true,
        receive_tdisp_interrupt_info: tdisp_interrupt_info,
        receive_response: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_tdisp_interrupt_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let tdisp_fsm_tag = scheduler.alloc(AdminFsm::TdispInt(TdispIntFsm::new(ctx)));
    assert!(tdisp_fsm_tag.is_some());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(tdisp_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::TdispInt(tdisp_interrupt_info),
            tdisp_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::Ide(None), rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(tdisp_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::HspToAdminIpcResponse,
            tdisp_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
#[should_panic]
fn tdisp_interrupt_tests_err_when_sending_ipc() {
    let tdisp_interrupt_info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmTdispIntTestConfigs {
        receive_tdisp_interrupt: true,
        receive_tdisp_interrupt_info: tdisp_interrupt_info,
        err_when_sending_ipc: true,
        ..Default::default()
    };

    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TdispInt(tdisp_interrupt_info), 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
#[should_panic]
fn tdisp_interrupt_tests_err_when_receiving_response() {
    let tdisp_interrupt_info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmTdispIntTestConfigs {
        receive_tdisp_interrupt: true,
        receive_tdisp_interrupt_info: tdisp_interrupt_info,
        receive_response: true,
        err_when_receiving_response: true,
        ..Default::default()
    };

    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TdispInt(tdisp_interrupt_info), 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
#[should_panic]
fn tdisp_interrupt_tests_err_when_receiving_invalid_header() {
    let tdisp_interrupt_info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmTdispIntTestConfigs {
        receive_tdisp_interrupt: true,
        receive_tdisp_interrupt_info: tdisp_interrupt_info,
        receive_response: true,
        err_when_invalid_header_response: true,
        ..Default::default()
    };

    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TdispInt(tdisp_interrupt_info), 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
#[should_panic]
fn tdisp_interrupt_tests_err_when_receiving_invalid_payload() {
    let tdisp_interrupt_info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmTdispIntTestConfigs {
        receive_tdisp_interrupt: true,
        receive_tdisp_interrupt_info: tdisp_interrupt_info,
        receive_response: true,
        err_when_invalid_payload_response: true,
        ..Default::default()
    };

    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TdispInt(tdisp_interrupt_info), 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::HspToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
#[should_panic]
fn tdisp_interrupt_tests_tdisp_idle_timeout() {
    let config = AdminFsmTdispIntTestConfigs {
        receive_ide_interrupt: true,
        ..Default::default()
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_tdisp_interrupt_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let tdisp_fsm_tag = scheduler.alloc(AdminFsm::TdispInt(TdispIntFsm::new(ctx)));
    assert!(tdisp_fsm_tag.is_some());

    // Let the resource get acquired by some other FSM.
    scheduler.on_event(AdminFsmEvent::Ide(None), rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(tdisp_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::Ide(None), tdisp_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(tdisp_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::TimerElapsed, tdisp_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
#[should_panic]
fn tdisp_interrupt_tests_ipc_response_timeout() {
    let tdisp_interrupt_info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmTdispIntTestConfigs {
        receive_tdisp_interrupt: true,
        receive_tdisp_interrupt_info: tdisp_interrupt_info,
        ..Default::default()
    };

    let ctx = make_tdisp_interrupt_fsm(config);
    let mut fsm = TdispIntFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TdispInt(tdisp_interrupt_info), 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );
}
