// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::helper::*;
use crate::error::AdminErr;
use crate::fsm::create_sq::AdminCreateSqCmd;
use crate::fsm::{AdminCmdFsm, AdminCmdTrait, AdminFsm, ResourceTestFsm};
use crate::{AdminFsmContext, AdminFsmEvent, AdminFsmEventRecorder, CmdFsm, CmdScheduler};

/// Create SQ test state machine configuration
pub(crate) struct AdminFsmCreateSqTestConfigs {
    /// The host sq id
    pub host_sq: HostSqId,
    /// The host cq id
    pub host_cq: HostCqId,
    /// The physically contiguous
    pub pc: bool,
    /// The queue priority
    pub queue_priority: HostQueuePriority,
    /// The create sq error code
    pub create_sq_err_code: Option<AdminErr>,
    /// The host queue length
    pub queue_len: u16,
    /// The IPC send failure
    pub ipc_send_failure: bool,
    /// The IPC response error status
    pub ipc_response_err_status: bool,
    /// The IPC response none
    pub ipc_response_none: bool,
    /// Unknown event
    pub unknown_event: bool,
}

#[test]
fn test_create_cq_unknown_event() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(1),
        host_cq: HostCqId(2),
        pc: true,
        queue_priority: HostQueuePriority::High,
        create_sq_err_code: None,
        queue_len: 0x1000,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: true,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_create_sq_cmd_hsm_queue() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(1),
        host_cq: HostCqId(2),
        pc: true,
        queue_priority: HostQueuePriority::High,
        create_sq_err_code: None,
        queue_len: 0x1000,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF), Ok(()));
}

#[test]
fn test_create_sq_cmd_fp_high_priority_queue() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(256),
        host_cq: HostCqId(300),
        pc: true,
        queue_priority: HostQueuePriority::High,
        create_sq_err_code: None,
        queue_len: 0x1000,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

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
fn test_create_sq_cmd_fp_low_priority_queue() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(256),
        host_cq: HostCqId(300),
        pc: true,
        queue_priority: HostQueuePriority::Low,
        create_sq_err_code: None,
        queue_len: 0x1000,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

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
fn test_create_sq_cmd_hsm_queue_with_no_pc_addr() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(256),
        host_cq: HostCqId(300),
        pc: false,
        queue_priority: HostQueuePriority::Low,
        create_sq_err_code: None,
        queue_len: 0x1000,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidFieldInCreateSqCmd)
    );
}

#[test]
fn test_create_sq_cmd_fp_queue_with_invalid_host_cq() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(256),
        host_cq: HostCqId(0),
        pc: true,
        queue_priority: HostQueuePriority::Low,
        create_sq_err_code: None,
        queue_len: 0x1000,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidHostCq)
    );
}

#[test]
fn test_create_sq_cmd_fp_queue_with_invalid_host_sq() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(0),
        host_cq: HostCqId(280),
        pc: true,
        queue_priority: HostQueuePriority::Low,
        create_sq_err_code: None,
        queue_len: 0x1000,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidQueueId)
    );
}

#[test]
fn test_create_sq_cmd_fp_queue_with_invalid_queue_len() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(256),
        host_cq: HostCqId(280),
        pc: true,
        queue_priority: HostQueuePriority::Low,
        create_sq_err_code: None,
        queue_len: 0,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidQueueSize)
    );
}

#[test]
fn test_create_sq_cmd_fp_queue_create_device_queue_invalid_host_cq() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(256),
        host_cq: HostCqId(280),
        pc: true,
        queue_priority: HostQueuePriority::Low,
        create_sq_err_code: Some(AdminErr::InvalidHostCq),
        queue_len: 0x1000,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidHostCq)
    );
}

#[test]
fn test_create_sq_cmd_fp_queue_create_device_queue_fails() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(256),
        host_cq: HostCqId(280),
        pc: true,
        queue_priority: HostQueuePriority::Low,
        create_sq_err_code: Some(AdminErr::CreateSqFailedByQueueController),
        queue_len: 0x1000,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::CreateSqFailedByQueueController)
    );
}

#[test]
fn test_create_sq_cmd_fp_queue_send_ipc_fails() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(256),
        host_cq: HostCqId(280),
        pc: true,
        queue_priority: HostQueuePriority::Low,
        create_sq_err_code: None,
        queue_len: 0x1000,
        ipc_send_failure: true,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::IpcSendRequestError)
    );
}

#[test]
fn test_create_sq_cmd_hsm_queue_send_ipc_fails() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(2),
        host_cq: HostCqId(2),
        pc: true,
        queue_priority: HostQueuePriority::Low,
        create_sq_err_code: None,
        queue_len: 0x1000,
        ipc_send_failure: true,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::IpcSendRequestError)
    );
}

#[test]
fn test_create_sq_cmd_fp_queue_ipc_receive_failure_status_in_msg() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(256),
        host_cq: HostCqId(280),
        pc: true,
        queue_priority: HostQueuePriority::Low,
        create_sq_err_code: None,
        queue_len: 0x1000,
        ipc_send_failure: false,
        ipc_response_err_status: true,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

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
fn test_create_sq_cmd_hsm_queue_ipc_receive_failure_status_in_msg() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(20),
        host_cq: HostCqId(2),
        pc: true,
        queue_priority: HostQueuePriority::Low,
        create_sq_err_code: None,
        queue_len: 0x1000,
        ipc_send_failure: false,
        ipc_response_err_status: true,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

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
fn test_create_sq_cmd_hsm_queue_ipc_receive_none_msg() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(20),
        host_cq: HostCqId(2),
        pc: true,
        queue_priority: HostQueuePriority::Low,
        create_sq_err_code: None,
        queue_len: 0x1000,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: true,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_sq(config);

    // Create Create Submission Queue command FSM
    let mut fsm = AdminCreateSqCmd::new(ctx, PcieFunction::Pf, sqe, None);

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
fn test_create_sq_cmd_hsm_wait_for_ipc_resource() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(1),
        host_cq: HostCqId(2),
        pc: true,
        queue_priority: HostQueuePriority::High,
        create_sq_err_code: None,
        queue_len: 0x1000,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let ctx = AdminFsmContext::new(
        make_env_for_admin_fsm_to_perform_create_sq(config),
        scheduler.clone(),
    );

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let create_sq_fsm_tag = scheduler.alloc(AdminFsm::AdminCmd(AdminCmdFsm::new(ctx)));
    assert!(create_sq_fsm_tag.is_some());

    // Acquire a HSM IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM under test
    assert_eq!(
        scheduler.map(create_sq_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::RxReady, create_sq_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::HsmIpcResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(create_sq_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::HsmIpcResponse, create_sq_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(create_sq_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DmaComplete, create_sq_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(create_sq_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::TxComplete, create_sq_fsm_tag.unwrap())),
        Some(Ok(()))
    );
}

#[test]
fn test_create_sq_cmd_fp_wait_for_ipc_resource() {
    let config = AdminFsmCreateSqTestConfigs {
        host_sq: HostSqId(256),
        host_cq: HostCqId(300),
        pc: true,
        queue_priority: HostQueuePriority::High,
        create_sq_err_code: None,
        queue_len: 0x1000,
        ipc_send_failure: false,
        ipc_response_err_status: false,
        ipc_response_none: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());
    let ctx = AdminFsmContext::new(
        make_env_for_admin_fsm_to_perform_create_sq(config),
        scheduler.clone(),
    );

    let rsc_fsm_tag = scheduler.alloc(AdminFsm::ResourceTest(ResourceTestFsm::new(
        ctx.clone(),
        None,
    )));
    assert!(rsc_fsm_tag.is_some());

    let create_sq_fsm_tag = scheduler.alloc(AdminFsm::AdminCmd(AdminCmdFsm::new(ctx)));
    assert!(create_sq_fsm_tag.is_some());

    // Acquire a FP IPC resource to create resource contention scenario
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    // Execute test by passing events to the FSM under test
    assert_eq!(
        scheduler.map(create_sq_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::RxReady, create_sq_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    // Drop the resource to simulate the resource is released for the IPC channel
    scheduler.on_event(AdminFsmEvent::FpToAdminIpcResponse, rsc_fsm_tag.unwrap());

    assert_eq!(
        scheduler.map(create_sq_fsm_tag.unwrap(), |fsm| fsm.on_event(
            AdminFsmEvent::FpToAdminIpcResponse,
            create_sq_fsm_tag.unwrap()
        )),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(create_sq_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::DmaComplete, create_sq_fsm_tag.unwrap())),
        Some(Err(AdminErr::Pending))
    );

    assert_eq!(
        scheduler.map(create_sq_fsm_tag.unwrap(), |fsm| fsm
            .on_event(AdminFsmEvent::TxComplete, create_sq_fsm_tag.unwrap())),
        Some(Ok(()))
    );
}
