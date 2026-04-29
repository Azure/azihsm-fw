// Copyright (c) Microsoft Corporation. All rights reserved.

use super::helper::*;
use crate::error::AdminErr;
use crate::fsm::unsupported::AdminUnsupportedCmd;
use crate::fsm::AdminCmdTrait;
use crate::mock::MockAdminEnvTrait;
use crate::AdminFsmEvent;

#[test]
fn test_unsupported() {
    let sqe = make_sqe_with_opcode(0x88);
    // Create Delete Submission Queue command FSM
    let mut fsm: AdminUnsupportedCmd<MockAdminEnvTrait> = AdminUnsupportedCmd::new(sqe);

    // Execute test by passing events to the FSM
    assert_eq!(fsm.on_event(AdminFsmEvent::StartCmd, 0xFF), Ok(()));
}

#[test]
fn test_unsupported_unexpected_event() {
    let sqe = make_sqe_with_opcode(0x88);
    // Create Delete Submission Queue command FSM
    let mut fsm: AdminUnsupportedCmd<MockAdminEnvTrait> = AdminUnsupportedCmd::new(sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}
