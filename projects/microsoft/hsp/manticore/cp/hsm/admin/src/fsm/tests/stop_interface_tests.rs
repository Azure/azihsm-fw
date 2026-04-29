// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_pcie_controller::TdispIntInfo;
use test_log::test;

use crate::cmd_scheduler::*;
use crate::event::AdminFsmEvent;
use crate::fsm::tests::helper::make_stop_interface_fsm;
use crate::fsm::AdminErr;
use crate::fsm::ResId;
use crate::fsm::StopInterfaceFsm;
use crate::resource::AdminFsmResourceId;

// Stop Interface test state machine configuration
#[derive(Clone, Copy, Default)]
pub(crate) struct AdminFsmStopInterfaceTestConfigs {
    pub receive_stop_interface_request: bool,
    pub stop_interface_info: TdispIntInfo,
    pub fp_response_count: Option<u32>,
    pub hsm_response_count: Option<u32>,
    pub deferred_io: bool,
    pub deferred_io_completed: bool,
    pub err_hsm_request: bool,
    pub err_fp_request: bool,
    pub err_hsm_response: bool,
    pub err_fp_response: bool,
}

#[test]
fn stop_interface_tests_acquire_resource_err() {
    let config = AdminFsmStopInterfaceTestConfigs::default();
    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Use incorrect resource
    let event = fsm.acquire_resource(0xFF, AdminFsmResourceId::CastIdle);
    assert!(event == AdminFsmEvent::Unknown);
}

#[test]
fn stop_interface_tests_acquire_resource_fp() {
    let config = AdminFsmStopInterfaceTestConfigs::default();
    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Use AdminToFpIpcChannel resource
    let event = fsm.acquire_resource(0xFF, AdminFsmResourceId::AdminToFpIpcChannel);
    assert!(event == AdminFsmEvent::ResourceReady(ResId::AdminToFpIpcChannel));
}

#[test]
fn stop_interface_tests_acquire_resource_hsm() {
    let config = AdminFsmStopInterfaceTestConfigs::default();
    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Use HsmIpcChannel resource
    let event = fsm.acquire_resource(0xFF, AdminFsmResourceId::HsmIpcChannel);
    assert!(event == AdminFsmEvent::ResourceReady(ResId::HsmIpcChannel));
}

#[test]
fn stop_interface_tests_unknown_event() {
    let config = AdminFsmStopInterfaceTestConfigs::default();
    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF) == Err(AdminErr::Pending));
}

#[test]
fn stop_interface_tests_receive_stop_interface_request() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );
}

#[test]
fn stop_interface_tests_with_hsm_response() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        hsm_response_count: Some(1),
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );

    assert!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF) == Err(AdminErr::Pending));
}

#[test]
fn stop_interface_tests_with_fp_response() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        fp_response_count: Some(1),
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );

    assert!(fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF) == Err(AdminErr::Pending));
}

#[test]
fn stop_interface_tests_with_fp_and_hsm_response() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        ..Default::default()
    };

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        fp_response_count: Some(1),
        hsm_response_count: Some(1),
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );

    assert!(fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF) == Err(AdminErr::Pending));

    assert!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF) == Err(AdminErr::Pending));
}

#[test]
fn stop_interface_tests_with_more_fp_than_hsm_response() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        pf_mask: true,
        ..Default::default()
    };

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        fp_response_count: Some(2),
        hsm_response_count: Some(1),
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );

    assert!(fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF) == Err(AdminErr::Pending));

    assert!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF) == Err(AdminErr::Pending));

    assert!(fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF) == Err(AdminErr::Pending));
}

#[test]
fn stop_interface_tests_with_less_fp_than_hsm_response() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        pf_mask: true,
        ..Default::default()
    };

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        fp_response_count: Some(1),
        hsm_response_count: Some(2),
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );

    assert!(fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF) == Err(AdminErr::Pending));

    assert!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF) == Err(AdminErr::Pending));

    assert!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF) == Err(AdminErr::Pending));
}

#[test]
fn stop_interface_tests_with_not_full_fp_hsm_response() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        pf_mask: true,
        ..Default::default()
    };

    let iter = 2;

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        fp_response_count: Some(iter),
        hsm_response_count: Some(iter),
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );

    for _ in 1..=iter {
        assert!(fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF) == Err(AdminErr::Pending));

        assert!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF) == Err(AdminErr::Pending));
    }
}

#[test]
fn stop_interface_tests_with_full_fp_hsm_response() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        pf_mask: true,
        ..Default::default()
    };

    let iter = 3;

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        fp_response_count: Some(iter),
        hsm_response_count: Some(iter),
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );

    for _ in 1..=iter {
        assert!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF) == Err(AdminErr::Pending));

        assert!(fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF) == Err(AdminErr::Pending));
    }
}

#[test]
fn stop_interface_tests_with_deferred_io_not_completed() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        pf_mask: true,
        ..Default::default()
    };

    let iter = 3;

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        fp_response_count: Some(iter),
        hsm_response_count: Some(iter),
        deferred_io: true,
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );

    for _ in 1..=iter {
        assert!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF) == Err(AdminErr::Pending));

        assert!(fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF) == Err(AdminErr::Pending));
    }
}

fn stop_interface_tests_with_deferred_io_completed(event_arr: &[AdminFsmEvent]) {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        pf_mask: true,
        ..Default::default()
    };

    let iter = 3;

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        fp_response_count: Some(iter),
        hsm_response_count: Some(iter),
        deferred_io: true,
        deferred_io_completed: true,
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );

    assert_eq!(event_arr.len(), (iter * 3) as usize);

    for &event in event_arr.iter() {
        assert_eq!(fsm.on_event(event, 0xFF), Err(AdminErr::Pending));
    }
}

#[test]
fn stop_interface_tests_with_deferred_io_completed_at_end_1() {
    let event_arr = [
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
    ];

    stop_interface_tests_with_deferred_io_completed(&event_arr);
}

#[test]
fn stop_interface_tests_with_deferred_io_completed_at_end_2() {
    let event_arr = [
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
    ];

    stop_interface_tests_with_deferred_io_completed(&event_arr);
}

#[test]
fn stop_interface_tests_with_deferred_io_completed_in_the_middle() {
    let event_arr = [
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::FpToAdminIpcResponse,
    ];

    stop_interface_tests_with_deferred_io_completed(&event_arr);
}

#[test]
fn stop_interface_tests_with_deferred_io_completed_in_random_order_1() {
    let event_arr = [
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::IoCancellationComplete,
    ];

    stop_interface_tests_with_deferred_io_completed(&event_arr);
}

#[test]
fn stop_interface_tests_with_deferred_io_completed_in_random_order_2() {
    let event_arr = [
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::IoCancellationComplete,
    ];

    stop_interface_tests_with_deferred_io_completed(&event_arr);
}

#[test]
fn stop_interface_tests_with_deferred_io_completed_in_random_order_3() {
    let event_arr = [
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
    ];

    stop_interface_tests_with_deferred_io_completed(&event_arr);
}

#[test]
fn stop_interface_tests_with_deferred_io_completed_in_random_order_4() {
    let event_arr = [
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
        AdminFsmEvent::FpToAdminIpcResponse,
        AdminFsmEvent::HsmIpcResponse,
        AdminFsmEvent::IoCancellationComplete,
    ];

    stop_interface_tests_with_deferred_io_completed(&event_arr);
}

#[test]
#[should_panic]
fn stop_interface_tests_err_hsm_request() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        pf_mask: true,
        ..Default::default()
    };

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        err_hsm_request: true,
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );
}

#[test]
#[should_panic]
fn stop_interface_tests_err_fp_request() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        pf_mask: true,
        ..Default::default()
    };

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        err_fp_request: true,
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );
}

#[test]
fn stop_interface_tests_err_fp_response() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        pf_mask: true,
        ..Default::default()
    };

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        fp_response_count: Some(1),
        err_fp_response: true,
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );

    assert!(fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF) == Err(AdminErr::Pending));
}

#[test]
fn stop_interface_tests_err_hsm_response() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        pf_mask: true,
        ..Default::default()
    };

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        hsm_response_count: Some(1),
        err_hsm_response: true,
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );

    assert!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF) == Err(AdminErr::Pending));
}

#[test]
#[should_panic]
fn stop_interface_tests_ipc_response_timeout() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        pf_mask: true,
        ..Default::default()
    };

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );

    assert!(fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF) == Err(AdminErr::Pending));
}

#[test]
#[should_panic]
fn stop_interface_tests_hsm_response_timeout() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        pf_mask: true,
        ..Default::default()
    };

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        fp_response_count: Some(1),
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );

    assert!(fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF) == Err(AdminErr::Pending));

    assert!(fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF) == Err(AdminErr::Pending));
}

#[test]
#[should_panic]
fn stop_interface_tests_fp_response_timeout() {
    let info = TdispIntInfo {
        vf_mask: 0x3,
        pf_mask: true,
        ..Default::default()
    };

    let config = AdminFsmStopInterfaceTestConfigs {
        receive_stop_interface_request: true,
        stop_interface_info: info,
        hsm_response_count: Some(1),
        ..Default::default()
    };

    let ctx = make_stop_interface_fsm(config);
    let mut fsm = StopInterfaceFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert!(
        fsm.on_event(
            AdminFsmEvent::StopInterfaceRequest((
                info.vf_mask as u128 | (info.pf_mask as u128) << 64,
                0xff
            )),
            0xFF
        ) == Err(AdminErr::Pending)
    );

    assert!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF) == Err(AdminErr::Pending));

    assert!(fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF) == Err(AdminErr::Pending));
}
