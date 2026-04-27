// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::helper::*;
use crate::error::AdminErr;
use crate::fsm::delete_cq::AdminDeleteCqCmd;
use crate::fsm::AdminCmdTrait;
use crate::AdminFsmEvent;

#[test]
fn test_delete_cq_unknown_event() {
    const HOST_CQ_ID: HostCqId = HostCqId(1);
    const DELETE_CQ_ERR_CODE: Option<AdminErr> = None;
    const UNKNOWN_EVENT: bool = true;

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_cq(HOST_CQ_ID, DELETE_CQ_ERR_CODE, UNKNOWN_EVENT);

    // CreateDelete Completion Queue command FSM
    let mut fsm = AdminDeleteCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_delete_cq_cmd_hsm_queue() {
    const HOST_CQ_ID: HostCqId = HostCqId(1);
    const DELETE_CQ_ERR_CODE: Option<AdminErr> = None;
    const UNKNOWN_EVENT: bool = false;

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_cq(HOST_CQ_ID, DELETE_CQ_ERR_CODE, UNKNOWN_EVENT);

    // CreateDelete Completion Queue command FSM
    let mut fsm = AdminDeleteCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(fsm.on_event(AdminFsmEvent::StartCmd, 0xFF), Ok(()));
}

#[test]
fn test_delete_cq_cmd_fp_queue() {
    const HOST_CQ_ID: HostCqId = HostCqId(300);
    const DELETE_CQ_ERR_CODE: Option<AdminErr> = None;
    const UNKNOWN_EVENT: bool = false;

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_cq(HOST_CQ_ID, DELETE_CQ_ERR_CODE, UNKNOWN_EVENT);

    // CreateDelete Completion Queue command FSM
    let mut fsm = AdminDeleteCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(fsm.on_event(AdminFsmEvent::StartCmd, 0xFF), Ok(()));
}

#[test]
fn test_delete_cq_cmd_admin_queue() {
    const HOST_CQ_ID: HostCqId = HostCqId(0);
    const DELETE_CQ_ERR_CODE: Option<AdminErr> = None;
    const UNKNOWN_EVENT: bool = false;

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_cq(HOST_CQ_ID, DELETE_CQ_ERR_CODE, UNKNOWN_EVENT);

    // CreateDelete Completion Queue command FSM
    let mut fsm = AdminDeleteCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidQueueId)
    );
}

#[test]
fn test_delete_cq_cmd_delete_cq_fails_with_invalid_qid() {
    let err: Option<AdminErr> = Some(AdminErr::InvalidQueueId);
    const HOST_CQ_ID: HostCqId = HostCqId(1);
    const UNKNOWN_EVENT: bool = false;

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_delete_cq(HOST_CQ_ID, err, UNKNOWN_EVENT);

    // CreateDelete Completion Queue command FSM
    let mut fsm = AdminDeleteCqCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidQueueId)
    );
}
