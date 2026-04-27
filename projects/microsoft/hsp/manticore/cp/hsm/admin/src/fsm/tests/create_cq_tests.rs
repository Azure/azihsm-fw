// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::helper::*;
use crate::error::AdminErr;
use crate::fsm::create_cq::AdminCreateCqCmd;
use crate::fsm::AdminCmdTrait;
use crate::AdminFsmEvent;

/// Create CQ test state machine configuration
pub(crate) struct AdminFsmCreateCqTestConfigs {
    /// The host cq id
    pub host_cq: HostCqId,
    /// The host queue length
    pub queue_len: u16,
    /// The interrupt vector
    pub iv: u16,
    /// The interrupt enable
    pub ien: bool,
    /// The physically contiguous
    pub pc: bool,
    /// The create cq fails
    pub create_cq_fails: bool,
    /// Unknown event
    pub unknown_event: bool,
}

#[test]
fn test_create_cq_unknown_event() {
    let config = AdminFsmCreateCqTestConfigs {
        host_cq: HostCqId(1),
        queue_len: 0x100,
        iv: 1,
        ien: true,
        pc: true,
        create_cq_fails: false,
        unknown_event: true,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_cq(config);

    // Create Create Completion Queue command FSM
    let mut fsm = AdminCreateCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_create_cq_cmd_hsm_queue() {
    let config = AdminFsmCreateCqTestConfigs {
        host_cq: HostCqId(1),
        queue_len: 0x100,
        iv: 1,
        ien: true,
        pc: true,
        create_cq_fails: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_cq(config);

    // Create Create Completion Queue command FSM
    let mut fsm = AdminCreateCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(fsm.on_event(AdminFsmEvent::StartCmd, 0xFF), Ok(()));
}

#[test]
fn test_create_cq_cmd_fp_queue() {
    let config = AdminFsmCreateCqTestConfigs {
        host_cq: HostCqId(257),
        queue_len: 0x100,
        iv: 0x10,
        ien: true,
        pc: true,
        create_cq_fails: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_cq(config);

    // Create Create Completion Queue command FSM
    let mut fsm = AdminCreateCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(fsm.on_event(AdminFsmEvent::StartCmd, 0xFF), Ok(()));
}

#[test]
fn test_create_cq_cmd_hsm_queue_invalid_iv() {
    let config = AdminFsmCreateCqTestConfigs {
        host_cq: HostCqId(2),
        queue_len: 0x100,
        iv: 0x20,
        ien: true,
        pc: true,
        create_cq_fails: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_cq(config);

    // Create Create Completion Queue command FSM
    let mut fsm = AdminCreateCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidInterruptVector)
    );
}

#[test]
fn test_create_cq_cmd_fp_queue_invalid_iv() {
    let config = AdminFsmCreateCqTestConfigs {
        host_cq: HostCqId(300),
        queue_len: 0x100,
        iv: 0x0,
        ien: true,
        pc: true,
        create_cq_fails: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_cq(config);

    // Create Create Completion Queue command FSM
    let mut fsm = AdminCreateCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidInterruptVector)
    );
}

#[test]
fn test_create_cq_cmd_physically_discontiguous() {
    let config = AdminFsmCreateCqTestConfigs {
        host_cq: HostCqId(20),
        queue_len: 0x100,
        iv: 0x1,
        ien: true,
        pc: false,
        create_cq_fails: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_cq(config);

    // Create Create Completion Queue command FSM
    let mut fsm = AdminCreateCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidFieldInCreateCqCmd)
    );
}

#[test]
fn test_create_cq_cmd_zero_queue_len() {
    let config = AdminFsmCreateCqTestConfigs {
        host_cq: HostCqId(20),
        queue_len: 0,
        iv: 0x1,
        ien: true,
        pc: true,
        create_cq_fails: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_cq(config);

    // Create Create Completion Queue command FSM
    let mut fsm = AdminCreateCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidQueueSize)
    );
}

#[test]
fn test_create_cq_cmd_admin_queue_id() {
    let config = AdminFsmCreateCqTestConfigs {
        host_cq: HostCqId(0),
        queue_len: 0x100,
        iv: 0x1,
        ien: true,
        pc: true,
        create_cq_fails: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_cq(config);

    // Create Create Completion Queue command FSM
    let mut fsm = AdminCreateCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidQueueId)
    );
}

#[test]
fn test_create_cq_cmd_create_cq_fails() {
    let config = AdminFsmCreateCqTestConfigs {
        host_cq: HostCqId(1),
        queue_len: 0x1000,
        iv: 0x1,
        ien: true,
        pc: true,
        create_cq_fails: true,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_cq(config);

    // Create Create Completion Queue command FSM
    let mut fsm = AdminCreateCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidQueueId)
    );
}

#[test]
fn test_create_cq_cmd_interrupt_disabled() {
    let config = AdminFsmCreateCqTestConfigs {
        host_cq: HostCqId(1),
        queue_len: 0x1000,
        iv: 0x0,
        ien: false,
        pc: true,
        create_cq_fails: false,
        unknown_event: false,
    };

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_create_cq(config);

    // Create Create Completion Queue command FSM
    let mut fsm = AdminCreateCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(fsm.on_event(AdminFsmEvent::StartCmd, 0xFF), Ok(()));
}
