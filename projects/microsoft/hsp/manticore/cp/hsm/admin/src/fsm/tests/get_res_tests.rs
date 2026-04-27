// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::PcieFunction;

use super::helper::*;
use crate::error::AdminErr;
use crate::fsm::get_res::AdminGetResCmd;
use crate::fsm::AdminCmdTrait;
use crate::AdminFsmEvent;

#[test]
fn test_get_res_cmd_unknown_event() {
    const CNTRL_ID: u32 = 0;
    const UNKNOWN_EVENT: bool = true;

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_get_res(CNTRL_ID, UNKNOWN_EVENT);

    // Create Get Resource command FSM
    let mut fsm = AdminGetResCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_get_res_cmd() {
    const CNTRL_ID: u32 = 0;
    const UNKNOWN_EVENT: bool = false;

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_get_res(CNTRL_ID, UNKNOWN_EVENT);

    // Create Get Resource command FSM
    let mut fsm = AdminGetResCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(fsm.on_event(AdminFsmEvent::StartCmd, 0xFF), Ok(()));
}

#[test]
fn test_get_res_cmd_invalid_cntrl_id_in_sqe() {
    const CNTRL_ID: u32 = 120;
    const UNKNOWN_EVENT: bool = false;

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_get_res(CNTRL_ID, UNKNOWN_EVENT);

    // Create Get Resource command FSM
    let mut fsm = AdminGetResCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidCntrlIdFieldInSqe)
    );
}
