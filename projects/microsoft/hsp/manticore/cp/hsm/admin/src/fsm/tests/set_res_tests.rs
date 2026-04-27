// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::PcieFunction;

use super::helper::*;
use crate::error::AdminErr;
use crate::fsm::set_res::AdminSetResCmd;
use crate::fsm::{AdminCmdFsm, AdminCmdTrait, AdminFsm, ResourceTestFsm};
use crate::{AdminFsmContext, AdminFsmEvent, AdminFsmEventRecorder, CmdFsm, CmdScheduler};

/// Set resource test state machine configuration
#[derive(Default)]
pub(crate) struct AdminFsmSetResTestConfigs {
    /// Previously assigned resource count to this function
    pub prev_resource: u32,
    /// Current requested resource count to this function
    pub num_resource: u32,
    /// The control id
    pub cntrl_id: u32,
    /// Request from PF
    pub request_from_pf: bool,
    /// IPC response error status
    pub ipc_resp_err_status: bool,
    /// IPC send request error status
    pub ipc_send_req_err_status: bool,
    /// Receive none IPC message
    pub receive_none_ipc_message: bool,
    /// Unknown Event
    pub unknown_event: bool,
}

#[test]
fn test_set_res_cmd_unknown_event() {
    let config = AdminFsmSetResTestConfigs {
        num_resource: 1,
        cntrl_id: 0,
        request_from_pf: true,
        ipc_resp_err_status: false,
        ipc_send_req_err_status: false,
        receive_none_ipc_message: false,
        unknown_event: true,
        ..Default::default()
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_set_res(config);

    // Set Resource command FSM
    let mut fsm = AdminSetResCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_set_res_cmd() {
    let config = AdminFsmSetResTestConfigs {
        num_resource: 1,
        cntrl_id: 0,
        request_from_pf: true,
        ipc_resp_err_status: false,
        ipc_send_req_err_status: false,
        receive_none_ipc_message: false,
        unknown_event: false,
        ..Default::default()
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_set_res(config);

    // Set Resource command FSM
    let mut fsm = AdminSetResCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF), Ok(()));
}

#[test]
fn test_set_res_cmd_with_invalid_num_resource() {
    let config = AdminFsmSetResTestConfigs {
        num_resource: 66,
        cntrl_id: 0,
        request_from_pf: true,
        ipc_resp_err_status: false,
        ipc_send_req_err_status: false,
        receive_none_ipc_message: false,
        unknown_event: false,
        ..Default::default()
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_set_res(config);

    // Set Resource command FSM
    let mut fsm = AdminSetResCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidResCountFieldInSqe)
    );
}

#[test]
fn test_set_res_cmd_with_invalid_cntrl_id_in_field() {
    let config = AdminFsmSetResTestConfigs {
        num_resource: 65,
        cntrl_id: 80,
        request_from_pf: true,
        ipc_resp_err_status: false,
        ipc_send_req_err_status: false,
        receive_none_ipc_message: false,
        unknown_event: false,
        ..Default::default()
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_set_res(config);

    // Set Resource command FSM
    let mut fsm = AdminSetResCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidCntrlIdFieldInSqe)
    );
}

#[test]
fn test_set_res_cmd_coming_from_a_vf() {
    let config = AdminFsmSetResTestConfigs {
        num_resource: 65,
        cntrl_id: 0,
        request_from_pf: false,
        ipc_resp_err_status: false,
        ipc_send_req_err_status: false,
        receive_none_ipc_message: false,
        unknown_event: false,
        ..Default::default()
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_set_res(config);

    // Set Resource command FSM
    let mut fsm = AdminSetResCmd::new(ctx, PcieFunction::Vf0, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidSetResCmd)
    );
}

#[test]
fn test_set_res_cmd_ipc_response_error() {
    let config = AdminFsmSetResTestConfigs {
        num_resource: 1,
        cntrl_id: 0,
        request_from_pf: true,
        ipc_resp_err_status: true,
        ipc_send_req_err_status: false,
        receive_none_ipc_message: false,
        unknown_event: false,
        ..Default::default()
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_set_res(config);

    // Set Resource command FSM
    let mut fsm = AdminSetResCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::IpcResponseError)
    );
}

#[test]
fn test_set_res_cmd_ipc_send_request_error() {
    let config = AdminFsmSetResTestConfigs {
        num_resource: 1,
        cntrl_id: 0,
        request_from_pf: true,
        ipc_resp_err_status: true,
        ipc_send_req_err_status: true,
        receive_none_ipc_message: false,
        unknown_event: false,
        ..Default::default()
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_set_res(config);

    // Set Resource command FSM
    let mut fsm = AdminSetResCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::IpcSendRequestError)
    );
}

#[test]
fn test_set_res_cmd_set_res_cnt_fails() {
    let config = AdminFsmSetResTestConfigs {
        num_resource: 0,
        cntrl_id: 0,
        request_from_pf: true,
        ipc_resp_err_status: false,
        ipc_send_req_err_status: false,
        receive_none_ipc_message: false,
        unknown_event: false,
        ..Default::default()
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_set_res(config);

    // Set Resource command FSM
    let mut fsm = AdminSetResCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::SetResCountLimitExceeded)
    );
}

#[test]
fn test_set_res_cmd_receive_none_ipc_message() {
    let config = AdminFsmSetResTestConfigs {
        num_resource: 1,
        cntrl_id: 0,
        request_from_pf: true,
        ipc_resp_err_status: false,
        ipc_send_req_err_status: false,
        receive_none_ipc_message: true,
        unknown_event: false,
        ..Default::default()
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_set_res(config);

    // Set Resource command FSM
    let mut fsm = AdminSetResCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::SpuriousIpcMessage)
    );
}

#[test]
fn test_set_res_cmd_request_to_allocate_same_res_cnt() {
    let config = AdminFsmSetResTestConfigs {
        prev_resource: 1,
        num_resource: 1,
        cntrl_id: 0,
        request_from_pf: true,
        ipc_resp_err_status: false,
        ipc_send_req_err_status: false,
        receive_none_ipc_message: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_set_res(config);

    // Set Resource command FSM
    let mut fsm = AdminSetResCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(fsm.on_event(AdminFsmEvent::StartCmd, 0xFF), Ok(()));
}

#[test]
fn test_set_res_ipc_resource_ready() {
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let ctx = AdminFsmContext::new(
        make_env_for_admin_fsm_to_perform_set_res(),
        scheduler.clone(),
    );

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let set_res_admin_fsm_tag = scheduler.alloc(AdminFsm::AdminCmd(AdminCmdFsm::new(ctx)));
    assert!(set_res_admin_fsm_tag.is_some());

    // Acquire a HSM IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM under test
    assert_eq!(
        scheduler.map(set_res_admin_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::RxReady, set_res_admin_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(set_res_admin_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::HsmIpcResponse,
            set_res_admin_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(set_res_admin_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DmaComplete, set_res_admin_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(set_res_admin_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::TxComplete, set_res_admin_fsm_tag.unwrap())),
        Some(Ok(()))
    );
}
