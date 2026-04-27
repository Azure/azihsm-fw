// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_queue_controller::QueueCntrlId;
use mcr_types::*;
use test_log::test;

use super::helper::*;
use crate::error::AdminErr;
use crate::fsm::{AdminFsm, CntrlFsm, ResourceTestFsm};
use crate::mock::MockAdminEnvTrait;
use crate::AdminFsmEvent;
use crate::{cmd_scheduler::*, AdminFsmContext, AdminFsmEventRecorder};

#[derive(Default, Copy, Clone, PartialEq, Eq)]
pub(crate) enum ControllerAction {
    #[default]
    Enable,

    Disable,

    Migrate,
}

/// Create SQ test state machine configuration
#[derive(Clone, Copy)]
pub(crate) struct ControllerFsmTestConfigs {
    /// Controller Action
    pub action: ControllerAction,
    /// Controller query call count
    pub call_count: usize,
    /// Error states
    pub error: bool,
    /// HSM send request fail
    pub hsm_send_request_fail: bool,
    /// FP send request fail
    pub fp_send_request_fail: bool,
    /// Unknown Event
    pub unknown_event: bool,
    /// Deferred HSM delete response
    pub defer_action: bool,
    /// Deferred action error
    pub defer_action_error: bool,
    /// Deferred queue delete response
    pub deferred_action_response: Option<QueueDeleteResponse>,
    /// IOCancellationComplete event is received
    pub io_cancellation_complete: bool,
    /// Invalid response from HSM
    pub hsm_invalid_response: bool,
    /// Invalid response from FP
    pub fp_invalid_response: bool,
}

impl Default for ControllerFsmTestConfigs {
    fn default() -> Self {
        Self {
            action: ControllerAction::default(),
            call_count: 1,
            error: false,
            hsm_send_request_fail: false,
            fp_send_request_fail: false,
            unknown_event: false,
            defer_action: false,
            defer_action_error: false,
            deferred_action_response: None,
            io_cancellation_complete: true,
            hsm_invalid_response: false,
            fp_invalid_response: false,
        }
    }
}

#[test]
fn cntrl_state_change_unknown_event() {
    let config = ControllerFsmTestConfigs {
        unknown_event: true,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn cntrl_state_change_for_enable_in_idle() {
    let config = ControllerFsmTestConfigs::default();

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
fn cntrl_nssr_in_idle() {
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Migrate,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Nssr(pending_list), 0xFF),
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
fn additional_cntrl_nssr_while_wait_for_hsm_response() {
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Migrate,
        call_count: 2,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Nssr(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    let pending_list: u128 = 1 << QueueCntrlId::Vf0 as u128;
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Nssr(pending_list), 0xFF),
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
fn additional_cntrl_nssr_while_wait_for_ipc_response() {
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Migrate,
        call_count: 2,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Nssr(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    let pending_list: u128 = 1 << QueueCntrlId::Vf0 as u128;
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Nssr(pending_list), 0xFF),
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
fn additional_cntrl_nssr_while_wait_for_fp_response() {
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Migrate,
        call_count: 2,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Nssr(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    let pending_list: u128 = 1 << QueueCntrlId::Vf0 as u128;
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Nssr(pending_list), 0xFF),
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
fn cntrl_state_change_for_enable_in_idle_send_fp_ipc_failed() {
    let config = ControllerFsmTestConfigs {
        fp_send_request_fail: true,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn cntrl_state_change_for_enable_in_idle_send_hsm_ipc_failed() {
    let config = ControllerFsmTestConfigs {
        hsm_send_request_fail: true,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn cntrl_state_change_for_disable_in_idle_send_fp_ipc_failed() {
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Disable,
        fp_send_request_fail: true,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn cntrl_state_change_for_disable_in_idle_send_hsm_ipc_failed() {
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Disable,
        hsm_send_request_fail: true,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn cntrl_state_change_for_enable_in_idle_hsm_respond_first() {
    let config = ControllerFsmTestConfigs::default();

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
fn cntrl_state_change_for_disable_in_idle() {
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Disable,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
fn cntrl_state_change_for_disable_in_idle_hsm_respond_first() {
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Disable,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
fn cntrl_state_change_for_enable_while_waiting_for_both_ipc_response() {
    let config = ControllerFsmTestConfigs {
        call_count: 2,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
fn cntrl_state_change_for_enable_while_waiting_for_hsm_ipc_response() {
    let config = ControllerFsmTestConfigs {
        call_count: 2,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
fn cntrl_state_change_for_enable_while_waiting_for_fp_ipc_response() {
    let config = ControllerFsmTestConfigs {
        call_count: 2,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
fn cntrl_state_change_for_disable_while_waiting_for_both_ipc_response() {
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Disable,
        call_count: 2,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
fn cntrl_state_change_for_disable_while_waiting_for_hsm_ipc_response() {
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Disable,
        call_count: 2,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
fn cntrl_state_change_for_disable_while_waiting_for_fp_ipc_response() {
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Disable,
        call_count: 2,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
fn cntrl_state_change_for_enable_wait_for_fp_ipc_channel_resource() {
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let configs = ControllerFsmTestConfigs::default();
    let mut fsm_test = make_cntrl_fsm_test(configs);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let cntrl_fsm_tag = scheduler.alloc(AdminFsm::Cntrl(CntrlFsm::new(ctx)));
    assert!(cntrl_fsm_tag.is_some());

    // Acquire a FP IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(cntrl_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::CntrlStateChange(pending_list),
            cntrl_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(cntrl_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::FpToAdminIpcResponse, cntrl_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(cntrl_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HsmIpcResponse, cntrl_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn cntrl_state_change_for_enable_wait_for_hsm_ipc_channel_resource() {
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let configs = ControllerFsmTestConfigs::default();
    let mut fsm_test = make_cntrl_fsm_test(configs);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let cntrl_fsm_tag = scheduler.alloc(AdminFsm::Cntrl(CntrlFsm::new(ctx)));
    assert!(cntrl_fsm_tag.is_some());

    // Acquire a HSM IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(cntrl_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::CntrlStateChange(pending_list),
            cntrl_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(cntrl_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::FpToAdminIpcResponse, cntrl_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(cntrl_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HsmIpcResponse, cntrl_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn cntrl_state_change_for_enable_wait_for_both_ipc_channel_resource() {
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let configs = ControllerFsmTestConfigs::default();
    let mut fsm_test = make_cntrl_fsm_test(configs);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let cntrl_fsm_tag = scheduler.alloc(AdminFsm::Cntrl(CntrlFsm::new(ctx)));
    assert!(cntrl_fsm_tag.is_some());

    // Acquire a FP IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    // Acquire a HSM IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(cntrl_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::CntrlStateChange(pending_list),
            cntrl_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(cntrl_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::FpToAdminIpcResponse, cntrl_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(cntrl_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HsmIpcResponse, cntrl_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn cntrl_state_change_for_enable_wait_for_both_ipc_channel_resource_release_hsm_first() {
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let configs = ControllerFsmTestConfigs::default();
    let mut fsm_test = make_cntrl_fsm_test(configs);
    let mut env = fsm_test.env();
    env.expect_clone().once().returning(MockAdminEnvTrait::new);
    let ctx = AdminFsmContext::new(env, scheduler.clone());

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let cntrl_fsm_tag = scheduler.alloc(AdminFsm::Cntrl(CntrlFsm::new(ctx)));
    assert!(cntrl_fsm_tag.is_some());

    // Acquire a FP IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    // Acquire a HSM IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        scheduler.map(cntrl_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::CntrlStateChange(pending_list),
            cntrl_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(cntrl_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::FpToAdminIpcResponse, cntrl_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(cntrl_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HsmIpcResponse, cntrl_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn cntrl_state_change_for_enable_in_idle_with_error() {
    let config = ControllerFsmTestConfigs {
        error: true,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn cntrl_state_change_for_vf0_enable_in_idle() {
    let config = ControllerFsmTestConfigs::default();

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Vf0 as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
fn cntrl_state_change_for_disable_with_deferred_hsm_response() {
    let deferred_action_response = Some(QueueDeleteResponse {
        tag: 0xFF,
        pfn: PcieFunction::Pf,
        _rsvd: Default::default(),
    });
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Disable,
        defer_action: true,
        deferred_action_response,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
        fsm.on_event(AdminFsmEvent::IoCancellationComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn cntrl_state_change_for_disable_with_deferred_hsm_response_with_invalid_tag() {
    let deferred_action_response = Some(QueueDeleteResponse {
        tag: 0,
        pfn: PcieFunction::Pf,
        _rsvd: Default::default(),
    });
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Disable,
        defer_action: true,
        defer_action_error: true,
        deferred_action_response,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
        fsm.on_event(AdminFsmEvent::IoCancellationComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn cntrl_state_change_for_disable_with_spurious_deferred_hsm_response() {
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Disable,
        defer_action: true,
        defer_action_error: true,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
        fsm.on_event(AdminFsmEvent::IoCancellationComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn cntrl_state_change_for_disable_with_deferred_hsm_response_io_cancenllation_incomplete_1() {
    let deferred_action_response = Some(QueueDeleteResponse {
        tag: 0xFF,
        pfn: PcieFunction::Pf,
        _rsvd: Default::default(),
    });
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Disable,
        defer_action: true,
        deferred_action_response,
        io_cancellation_complete: false,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
fn cntrl_state_change_for_disable_with_deferred_hsm_response_io_cancenllation_incomplete_2() {
    let deferred_action_response = Some(QueueDeleteResponse {
        tag: 0xFF,
        pfn: PcieFunction::Pf,
        _rsvd: Default::default(),
    });
    let config = ControllerFsmTestConfigs {
        action: ControllerAction::Disable,
        defer_action: true,
        deferred_action_response,
        io_cancellation_complete: false,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
fn cntrl_state_change_for_enable_invalid_hsm_response() {
    let config = ControllerFsmTestConfigs {
        hsm_invalid_response: true,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
fn cntrl_state_change_for_enable_invalid_fp_response() {
    let config = ControllerFsmTestConfigs {
        fp_invalid_response: true,
        ..Default::default()
    };

    let ctx = make_cntrl_fsm(config);

    // Controller Enable Disable command FSM
    let mut fsm = CntrlFsm::new(ctx);

    let pending_list: u128 = 1 << QueueCntrlId::Pf as u128;

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::CntrlStateChange(pending_list), 0xFF),
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
