// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::helper::*;
use crate::error::AdminErr;
use crate::fsm::delete_sq::AdminDeleteSqCmd;
use crate::fsm::{AdminCmdFsm, AdminCmdTrait, AdminFsm, ResourceTestFsm};
use crate::{AdminFsmContext, AdminFsmEvent, AdminFsmEventRecorder, CmdFsm, CmdScheduler};

/// Delete SQ test state machine configuration
pub(crate) struct AdminFsmDeleteSqTestConfigs {
    /// The host sq id
    pub host_sq: HostSqId,
    /// The device sq or device cq error code
    pub dev_sq_dev_cq_err_code: Option<AdminErr>,
    /// The delete sq error code
    pub delete_sq_err_code: Option<AdminErr>,
    /// The ipc send failure
    pub ipc_send_failure: bool,
    /// The ipc response error status
    pub ipc_response_err_status: bool,
    /// The ipc response none
    pub ipc_response_none: bool,
    /// Unknown Event
    pub unknown_event: bool,
    /// Deferred HSM delete response
    pub deferred_delete_response: bool,
    /// Deferred queue delete response error
    pub deferred_delete_response_error: bool,
    /// Deferred queue delete response
    pub deferred_queue_delete_response: Option<QueueDeleteResponse>,
}

#[test]
fn test_delete_sq_unknwon_event() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(0),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: true,
        deferred_delete_response: false,
        deferred_delete_response_error: false,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_delete_sq_cmd_hsm_queue() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(1),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: false,
        deferred_delete_response_error: false,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF), Ok(()));
}

#[test]
fn test_delete_sq_cmd_fp_queue() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(256),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: false,
        deferred_delete_response_error: false,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Ok(())
    );
}

#[test]
fn test_delete_sq_cmd_fp_queue_with_invalid_host_sq() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(0),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: false,
        deferred_delete_response_error: false,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidQueueId)
    );
}

#[test]
fn test_delete_sq_cmd_fp_queue_delete_device_queue_invalid_host_sq() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(256),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: Some(AdminErr::InvalidQueueId),
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: false,
        deferred_delete_response_error: false,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::InvalidQueueId)
    );
}

#[test]
fn test_delete_sq_cmd_hsm_queue_delete_device_queue_invalid_host_sq() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(12),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: Some(AdminErr::InvalidQueueId),
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: false,
        deferred_delete_response_error: false,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::InvalidQueueId)
    );
}

#[test]
fn test_delete_sq_cmd_fp_queue_dev_sq_dev_cq_fails() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(256),
        dev_sq_dev_cq_err_code: Some(AdminErr::InvalidQueueId),
        delete_sq_err_code: None,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: false,
        deferred_delete_response_error: false,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidQueueId)
    );
}

#[test]
fn test_delete_sq_cmd_fp_queue_send_ipc_fails() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(256),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: true,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: false,
        deferred_delete_response_error: false,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::IpcSendRequestError)
    );
}

#[test]
fn test_delete_sq_cmd_hsm_queue_send_ipc_fails() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(2),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: true,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: false,
        deferred_delete_response_error: false,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::IpcSendRequestError)
    );
}

#[test]
fn test_delete_sq_cmd_fp_queue_ipc_receive_failure_status_in_msg() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(256),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: false,
        ipc_response_err_status: true,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: false,
        deferred_delete_response_error: false,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::IpcResponseError)
    );
}

#[test]
fn test_delete_sq_cmd_hsm_queue_ipc_receive_failure_status_in_msg() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(20),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: false,
        ipc_response_err_status: true,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: false,
        deferred_delete_response_error: false,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

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
fn test_delete_sq_cmd_hsm_queue_ipc_receive_none_msg() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(20),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: true,
        unknown_event: false,
        deferred_delete_response: false,
        deferred_delete_response_error: false,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

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
fn test_delete_sq_cmd_hsm_wait_for_ipc_resource() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(1),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: false,
        deferred_delete_response_error: false,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let ctx = AdminFsmContext::new(
        make_env_for_admin_fsm_to_perform_delete_sq(config),
        scheduler.clone(),
    );

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let delete_sq_fsm_tag = scheduler.alloc(AdminFsm::AdminCmd(AdminCmdFsm::new(ctx)));
    assert!(delete_sq_fsm_tag.is_some());

    // Acquire a HSM IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM under test
    assert_eq!(
        scheduler.map(delete_sq_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::RxReady, delete_sq_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(delete_sq_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HsmIpcResponse, delete_sq_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn test_delete_sq_cmd_fp_wait_for_ipc_resource() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(256),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: false,
        deferred_delete_response_error: false,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let ctx = AdminFsmContext::new(
        make_env_for_admin_fsm_to_perform_delete_sq(config),
        scheduler.clone(),
    );

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let delete_sq_fsm_tag = scheduler.alloc(AdminFsm::AdminCmd(AdminCmdFsm::new(ctx)));
    assert!(delete_sq_fsm_tag.is_some());

    // Acquire a FP IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM under test
    assert_eq!(
        scheduler.map(delete_sq_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::RxReady, delete_sq_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(delete_sq_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::FpToAdminIpcResponse,
            delete_sq_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );
}

#[test]
fn test_delete_sq_cmd_hsm_queue_with_deferred_response() {
    let deferred_queue_delete_response = Some(QueueDeleteResponse {
        tag: 0xFF,
        pfn: PcieFunction::Pf,
        _rsvd: Default::default(),
    });
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(1),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: true,
        deferred_delete_response_error: false,
        deferred_queue_delete_response,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::IoCancellationComplete, 0xFF),
        Ok(())
    );
}

#[test]
fn test_delete_sq_cmd_hsm_queue_with_deferred_response_with_invalid_tag() {
    let deferred_queue_delete_response = Some(QueueDeleteResponse {
        tag: 0,
        pfn: PcieFunction::Pf,
        _rsvd: Default::default(),
    });
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(1),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: true,
        deferred_delete_response_error: true,
        deferred_queue_delete_response,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::IoCancellationComplete, 0xFF),
        Err(AdminErr::IoTagMismatch)
    );
}

#[test]
fn test_delete_sq_cmd_hsm_queue_with_deferred_response_with_invalid_pcie_function() {
    let deferred_queue_delete_response = Some(QueueDeleteResponse {
        tag: 0xFF,
        pfn: PcieFunction::Vf0,
        _rsvd: Default::default(),
    });
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(1),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: true,
        deferred_delete_response_error: true,
        deferred_queue_delete_response,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::IoCancellationComplete, 0xFF),
        Err(AdminErr::InvalidPcieFn)
    );
}

#[test]
fn test_delete_sq_cmd_hsm_queue_with_spurious_deferred_queue_delete_response() {
    let config = AdminFsmDeleteSqTestConfigs {
        host_sq: HostSqId(1),
        dev_sq_dev_cq_err_code: None,
        delete_sq_err_code: None,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
        deferred_delete_response: true,
        deferred_delete_response_error: true,
        deferred_queue_delete_response: None,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_sq(config);

    // Create Delete Submission Queue command FSM
    let mut fsm = AdminDeleteSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::IoCancellationComplete, 0xFF),
        Err(AdminErr::SpuriousIoQueueDeleteResp)
    );
}
