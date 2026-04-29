// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::PcieFunction;
use test_log::test;

use super::helper::*;
use crate::cmd_scheduler::CmdFsm;
use crate::error::AdminErr;
use crate::fsm::{AdminFsm, ResourceTestFsm, VflrFsm};
use crate::mock::MockAdminEnvTrait;
use crate::{AdminFsmContext, AdminFsmEvent, AdminFsmEventRecorder, CmdScheduler};

/// FLR test state machine configuration
pub(crate) struct AdminFsmVFlrTestConfigs {
    /// Controller ready state during vFLR
    pub ready: bool,
    /// The number of IPC calls
    pub call_count: usize,
    /// The number of functions
    pub function_count: usize,
    /// The invalid ipc call
    pub error: bool,
    /// The hsm send request fail
    pub hsm_send_request_fail: bool,
    /// The fp send request fail
    pub fp_send_request_fail: bool,
    /// The hsm receive message status
    pub hsm_receive_message_status: u32,
    /// The fp receive message status
    pub fp_receive_message_status: u32,
    /// Unknown event
    pub unknown_event: bool,
}

#[test]
fn cntrl_state_change_unknown_event() {
    let config = AdminFsmVFlrTestConfigs {
        ready: true,
        call_count: 1,
        function_count: 1,
        error: false,
        hsm_send_request_fail: false,
        fp_send_request_fail: false,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: true,
    };

    // Prepare the test environment
    let ctx = make_vflr_fsm(config);

    // PCIE vFLR command FSM
    let mut fsm = VflrFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn vflr_in_idle_with_function_in_ready_state() {
    let config = AdminFsmVFlrTestConfigs {
        ready: true,
        call_count: 1,
        function_count: 1,
        error: false,
        hsm_send_request_fail: false,
        fp_send_request_fail: false,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: false,
    };

    // Prepare the test environment
    let ctx = make_vflr_fsm(config);

    // PCIE vFLR command FSM
    let mut fsm = VflrFsm::new(ctx);
    let pending_list: u64 = 1 << PcieFunction::Vf0.0 as u64;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieVflr(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn vflr_in_idle_with_function_in_ready_state_hsm_respond_first() {
    let config = AdminFsmVFlrTestConfigs {
        ready: true,
        call_count: 1,
        function_count: 1,
        error: false,
        hsm_send_request_fail: false,
        fp_send_request_fail: false,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: false,
    };

    // Prepare the test environment
    let ctx = make_vflr_fsm(config);

    // PCIE vFLR command FSM
    let mut fsm = VflrFsm::new(ctx);
    let pending_list: u64 = 1 << PcieFunction::Vf0.0 as u64;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieVflr(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn vflr_in_idle_with_function_not_in_ready_state() {
    let config = AdminFsmVFlrTestConfigs {
        ready: false,
        call_count: 1,
        function_count: 1,
        error: false,
        hsm_send_request_fail: false,
        fp_send_request_fail: false,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: false,
    };

    // Prepare the test environment
    let ctx = make_vflr_fsm(config);

    // PCIE vFLR command FSM
    let mut fsm = VflrFsm::new(ctx);
    let pending_list: u64 = 1 << PcieFunction::Vf0.0 as u64;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieVflr(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn vflr_in_idle_with_function_in_ready_state_multiple_functions() {
    let config = AdminFsmVFlrTestConfigs {
        ready: true,
        call_count: 4,
        function_count: 4,
        error: false,
        hsm_send_request_fail: false,
        fp_send_request_fail: false,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: false,
    };

    // Prepare the test environment
    let ctx = make_vflr_fsm(config);

    // PCIE vFLR command FSM
    let mut fsm = VflrFsm::new(ctx);
    let pending_list: u64 = 0xF << PcieFunction::Vf0.0 as u64;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieVflr(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn vflr_in_idle_with_function_not_in_ready_state_multiple_functions() {
    let config = AdminFsmVFlrTestConfigs {
        ready: false,
        call_count: 1,
        function_count: 4,
        error: false,
        hsm_send_request_fail: false,
        fp_send_request_fail: false,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: false,
    };

    // Prepare the test environment
    let ctx = make_vflr_fsm(config);

    // PCIE vFLR command FSM
    let mut fsm = VflrFsm::new(ctx);
    let pending_list: u64 = 0xF << PcieFunction::Vf0.0 as u64;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieVflr(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn vflr_in_idle_send_fp_ipc_failed() {
    let config = AdminFsmVFlrTestConfigs {
        ready: true,
        call_count: 1,
        function_count: 1,
        error: false,
        hsm_send_request_fail: false,
        fp_send_request_fail: true,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: false,
    };

    // Prepare the test environment
    let ctx = make_vflr_fsm(config);

    // PCIE vFLR command FSM
    let mut fsm = VflrFsm::new(ctx);
    let pending_list: u64 = 1 << PcieFunction::Vf0.0 as u64;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieVflr(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn vflr_in_idle_send_hsm_ipc_failed() {
    let config = AdminFsmVFlrTestConfigs {
        ready: true,
        call_count: 1,
        function_count: 1,
        error: false,
        hsm_send_request_fail: true,
        fp_send_request_fail: false,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: false,
    };

    // Prepare the test environment
    let ctx = make_vflr_fsm(config);

    // PCIE vFLR command FSM
    let mut fsm = VflrFsm::new(ctx);
    let pending_list: u64 = 1 << PcieFunction::Vf0.0 as u64;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieVflr(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn vflr_repeated_while_waiting_for_both_ipc_response() {
    let config = AdminFsmVFlrTestConfigs {
        ready: true,
        call_count: 2,
        function_count: 2,
        error: false,
        hsm_send_request_fail: false,
        fp_send_request_fail: false,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: false,
    };

    // Prepare the test environment
    let ctx = make_vflr_fsm(config);

    // PCIE vFLR command FSM
    let mut fsm = VflrFsm::new(ctx);
    let pending_list: u64 = 1 << PcieFunction::Vf0.0 as u64;
    let pending_list_1: u64 = 1 << PcieFunction::Vf1.0 as u64;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieVflr(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieVflr(pending_list_1), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn vflr_repeated_while_waiting_for_hsm_ipc_response() {
    let config = AdminFsmVFlrTestConfigs {
        ready: true,
        call_count: 2,
        function_count: 2,
        error: false,
        hsm_send_request_fail: false,
        fp_send_request_fail: false,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: false,
    };

    // Prepare the test environment
    let ctx = make_vflr_fsm(config);

    // PCIE vFLR command FSM
    let mut fsm = VflrFsm::new(ctx);
    let pending_list: u64 = 1 << PcieFunction::Vf0.0 as u64;
    let pending_list_1: u64 = 1 << PcieFunction::Vf1.0 as u64;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieVflr(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieVflr(pending_list_1), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn vflr_repeated_while_waiting_for_fp_ipc_response() {
    let config = AdminFsmVFlrTestConfigs {
        ready: true,
        call_count: 2,
        function_count: 2,
        error: false,
        hsm_send_request_fail: false,
        fp_send_request_fail: false,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: false,
    };

    // Prepare the test environment
    let ctx = make_vflr_fsm(config);

    // PCIE vFLR command FSM
    let mut fsm = VflrFsm::new(ctx);
    let pending_list: u64 = 1 << PcieFunction::Vf0.0 as u64;
    let pending_list_1: u64 = 1 << PcieFunction::Vf1.0 as u64;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieVflr(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieVflr(pending_list_1), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn vflr_wait_for_fp_ipc_channel_resource() {
    let config = AdminFsmVFlrTestConfigs {
        ready: true,
        call_count: 1,
        function_count: 1,
        error: false,
        hsm_send_request_fail: false,
        fp_send_request_fail: false,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: false,
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_vflr_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let vflr_fsm_tag = scheduler.alloc(AdminFsm::Vflr(VflrFsm::new(ctx)));
    assert!(vflr_fsm_tag.is_some());

    // Acquire a FP IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    let pending_list: u64 = 1 << PcieFunction::Vf0.0 as u64;

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(vflr_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::PcieVflr(pending_list),
            vflr_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(vflr_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::FpToAdminIpcResponse, vflr_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(vflr_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HsmIpcResponse, vflr_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn vflr_wait_for_hsm_ipc_channel_resource() {
    let config = AdminFsmVFlrTestConfigs {
        ready: true,
        call_count: 1,
        function_count: 1,
        error: false,
        hsm_send_request_fail: false,
        fp_send_request_fail: false,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: false,
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_vflr_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());
    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let vflr_fsm_tag = scheduler.alloc(AdminFsm::Vflr(VflrFsm::new(ctx)));
    assert!(vflr_fsm_tag.is_some());

    // Acquire a HSM IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());

    // PCIE vFLR command FSM
    let pending_list: u64 = 1 << PcieFunction::Vf0.0 as u64;

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(vflr_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::PcieVflr(pending_list),
            vflr_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(vflr_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::FpToAdminIpcResponse, vflr_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(vflr_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HsmIpcResponse, vflr_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn vflr_wait_for_both_ipc_channel_resource_release_fp_first() {
    let config = AdminFsmVFlrTestConfigs {
        ready: true,
        call_count: 1,
        function_count: 1,
        error: false,
        hsm_send_request_fail: false,
        fp_send_request_fail: false,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: false,
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_vflr_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let vflr_fsm_tag = scheduler.alloc(AdminFsm::Vflr(VflrFsm::new(ctx)));
    assert!(vflr_fsm_tag.is_some());

    // Acquire a HSM/FP IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    // PCIE vFLR command FSM
    let pending_list: u64 = 1 << PcieFunction::Vf0.0 as u64;

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(vflr_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::PcieVflr(pending_list),
            vflr_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resources to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(vflr_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::FpToAdminIpcResponse, vflr_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(vflr_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HsmIpcResponse, vflr_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn vflr_wait_for_both_ipc_channel_resource_release_hsm_first() {
    let config = AdminFsmVFlrTestConfigs {
        ready: true,
        call_count: 1,
        function_count: 1,
        error: false,
        hsm_send_request_fail: false,
        fp_send_request_fail: false,
        hsm_receive_message_status: 0,
        fp_receive_message_status: 0,
        unknown_event: false,
    };

    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let mut fsm_test = make_vflr_fsm_test(config);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let vflr_fsm_tag = scheduler.alloc(AdminFsm::Vflr(VflrFsm::new(ctx)));
    assert!(vflr_fsm_tag.is_some());

    // Acquire a HSM/FP IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    // PCIE vFLR command FSM
    let pending_list: u64 = 1 << PcieFunction::Vf0.0 as u64;

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(vflr_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::PcieVflr(pending_list),
            vflr_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resources to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(vflr_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::FpToAdminIpcResponse, vflr_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(vflr_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HsmIpcResponse, vflr_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}
