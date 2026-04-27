// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::PcieFunction;
use test_log::test;

use super::helper::*;
use crate::error::AdminErr;
use crate::fsm::identify::AdminIdentifyCmd;
use crate::fsm::AdminCmdTrait;
use crate::AdminFsmEvent;

/// Identify test state machine configuration
pub(crate) struct AdminFsmIdentifyTestConfigs {
    /// The dma success status
    pub dma_success_status: bool,
    /// The invalid dma tag flag
    pub invalid_dma_tag: bool,
    /// The optional dma desc flag
    pub optional_dma_desc: bool,
    /// The invalid io tag flag
    pub invalid_io_tag: bool,
    /// The io success status
    pub io_success_status: bool,
    /// The dma alloc succeeds
    pub dma_alloc_succeeds: bool,
    /// Invalid Admin queue
    pub invalid_admin_queue: bool,
    /// No Admin queue created
    pub none_admin_queue: bool,
    /// Io end send returns None
    pub io_end_send_returns_none: bool,
}

#[test]
fn test_identify_cmd() {
    const UNKNOWN_EVENT: bool = false;
    const DMA_ALLOC_SUCCEEDS: bool = true;

    // Prepare the test environment
    let (ctx, sqe) = make_stand_alone_fsm_for_identify(UNKNOWN_EVENT, DMA_ALLOC_SUCCEEDS);

    // Create Identify Controller command FSM
    let mut fsm = AdminIdentifyCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(fsm.on_event(AdminFsmEvent::StartCmd, 0xFF), Ok(()));
}

#[test]
fn test_dentify_cmd_unknown_event() {
    const UNKNOWN_EVENT: bool = true;
    const DMA_ALLOC_SUCCEEDS: bool = true;

    // Prepare the test environment
    let (ctx, sqe) = make_stand_alone_fsm_for_identify(UNKNOWN_EVENT, DMA_ALLOC_SUCCEEDS);

    // Create Identify Controller command FSM
    let mut fsm = AdminIdentifyCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_identify_cmd_dma_alloc_fails() {
    const UNKNOWN_EVENT: bool = false;
    const DMA_ALLOC_SUCCEEDS: bool = false;

    // Prepare the test environment
    let (ctx, sqe) = make_stand_alone_fsm_for_identify(UNKNOWN_EVENT, DMA_ALLOC_SUCCEEDS);

    // Create Identify Controller command FSM
    let mut fsm = AdminIdentifyCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::NoMemory)
    );
}
