// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;
use test_log::test;

use super::helper::*;
use crate::cmd_scheduler::*;
use crate::context::AdminFsmContext;
use crate::error::AdminErr;
use crate::fsm::tests::harness::AdminFsmTest;
use crate::fsm::tests::identify_tests::AdminFsmIdentifyTestConfigs;
use crate::fsm::types::AdminSqe;
use crate::fsm::AdminCmdFsm;
use crate::recorder::AdminFsmEventRecorder;
use crate::AdminFsmEvent;

#[test]
fn test_admin_fsm_unknown_event() {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);

    let env = test.env();
    let scheduler = CmdScheduler::new(65 + 4, 1, AdminFsmEventRecorder::default());

    let ctx = AdminFsmContext::new(env, scheduler);

    // Admin FSM
    let mut fsm = AdminCmdFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_io_channel_recv_none() {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);

    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| None);

    let env = test.env();
    let scheduler = CmdScheduler::new(65 + 4, 1, AdminFsmEventRecorder::default());

    let ctx = AdminFsmContext::new(env, scheduler);

    // Admin FSM
    let mut fsm = AdminCmdFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::RxReady, 0xFF),
        Err(AdminErr::IoChannelRecvNone)
    );
}

#[test]
fn test_rx_desc_error() {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);

    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(PcieFunction::Pf, AdminSqe::default(), false));

    let env = test.env();
    let scheduler = CmdScheduler::new(65 + 4, 1, AdminFsmEventRecorder::default());

    let ctx = AdminFsmContext::new(env, scheduler);

    // Admin FSM
    let mut fsm = AdminCmdFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::RxReady, 0xFF),
        Err(AdminErr::IoChannelRecvErr)
    );
}

#[test]
fn test_rx_desc_invalid_opcode() {
    // Admin FSM
    let mut fsm = AdminCmdFsm::new(make_fsm_for_invalid_opcode());

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::RxReady, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(fsm.on_event(AdminFsmEvent::TxComplete, 0xFF), Ok(()));
}

#[test]
fn test_dma_out_fails() {
    // Admin FSM
    let mut fsm = AdminCmdFsm::new(make_fsm_for_dma_out_fail());

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::RxReady, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(fsm.on_event(AdminFsmEvent::TxComplete, 0xFF), Ok(()));
}

#[test]
fn test_dma_out_send_err_cqe_fails() {
    // Admin FSM
    let mut fsm = AdminCmdFsm::new(make_fsm_for_dma_out_and_send_err_cqe_fail());

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::RxReady, 0xFF),
        Err(AdminErr::IoChannelSendError)
    );
}

#[test]
fn test_send_cqe_fails_after_dma_complete() {
    let mut fsm = AdminCmdFsm::new(make_fsm_for_send_cqe_fails_after_dma_complete());

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::RxReady, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, 0xFF),
        Err(AdminErr::IoChannelSendError)
    );
}

#[test]
fn test_send_cqe_fails_after_cmd_complete() {
    let mut fsm = AdminCmdFsm::new(make_fsm_for_send_cqe_fails_after_cmd_completes());

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::RxReady, 0xFF),
        Err(AdminErr::IoChannelSendError)
    );
}

#[test]
fn test_identify_cmd_end_send_returns_none() {
    let config = AdminFsmIdentifyTestConfigs {
        dma_success_status: true,
        invalid_dma_tag: false,
        optional_dma_desc: true,
        invalid_io_tag: false,
        io_success_status: true,
        dma_alloc_succeeds: true,
        invalid_admin_queue: false,
        none_admin_queue: false,
        io_end_send_returns_none: true,
    };

    let mut fsm = AdminCmdFsm::new(make_fsm_for_identify(config));

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::RxReady, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TxComplete, 0xFF),
        Err(AdminErr::IoChannelSendCompleteNone)
    );
}

#[test]
fn test_end_dma_fails_with_failure_status_in_desc() {
    let config = AdminFsmIdentifyTestConfigs {
        dma_success_status: false,
        invalid_dma_tag: false,
        optional_dma_desc: true,
        invalid_io_tag: false,
        io_success_status: true,
        dma_alloc_succeeds: true,
        invalid_admin_queue: false,
        none_admin_queue: false,
        io_end_send_returns_none: false,
    };

    let mut fsm = AdminCmdFsm::new(make_fsm_for_identify(config));

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::RxReady, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(fsm.on_event(AdminFsmEvent::TxComplete, 0xFF), Ok(()));
}

#[test]
fn test_end_dma_fails_with_invalid_tag_in_desc() {
    let config = AdminFsmIdentifyTestConfigs {
        dma_success_status: true,
        invalid_dma_tag: true,
        optional_dma_desc: true,
        invalid_io_tag: false,
        io_success_status: true,
        dma_alloc_succeeds: true,
        invalid_admin_queue: false,
        none_admin_queue: false,
        io_end_send_returns_none: false,
    };

    let mut fsm = AdminCmdFsm::new(make_fsm_for_identify(config));

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::RxReady, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(fsm.on_event(AdminFsmEvent::TxComplete, 0xFF), Ok(()));
}

#[test]
fn test_end_dma_fails_with_none_dma_desc() {
    let config = AdminFsmIdentifyTestConfigs {
        dma_success_status: true,
        invalid_dma_tag: false,
        optional_dma_desc: false,
        invalid_io_tag: false,
        io_success_status: true,
        dma_alloc_succeeds: true,
        invalid_admin_queue: false,
        none_admin_queue: false,
        io_end_send_returns_none: false,
    };

    let mut fsm = AdminCmdFsm::new(make_fsm_for_identify(config));

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::RxReady, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(fsm.on_event(AdminFsmEvent::TxComplete, 0xFF), Ok(()));
}

#[test]
fn test_end_recv_sqe_fails_with_failure_io_desc_status() {
    let config = AdminFsmIdentifyTestConfigs {
        dma_success_status: true,
        invalid_dma_tag: false,
        optional_dma_desc: true,
        invalid_io_tag: false,
        io_success_status: false,
        dma_alloc_succeeds: true,
        invalid_admin_queue: false,
        none_admin_queue: false,
        io_end_send_returns_none: false,
    };

    let mut fsm = AdminCmdFsm::new(make_fsm_for_identify(config));

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::RxReady, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TxComplete, 0xFF),
        Err(AdminErr::IoChannelSendCompleteError)
    );
}

#[test]
fn test_identify_with_invalid_admin_queue() {
    let config = AdminFsmIdentifyTestConfigs {
        dma_success_status: true,
        invalid_dma_tag: false,
        optional_dma_desc: true,
        invalid_io_tag: false,
        io_success_status: true,
        dma_alloc_succeeds: true,
        invalid_admin_queue: true,
        none_admin_queue: false,
        io_end_send_returns_none: false,
    };

    let mut fsm = AdminCmdFsm::new(make_fsm_for_identify(config));

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::RxReady, 0xFF),
        Err(AdminErr::InvalidAdminQueue)
    );
}

#[test]
fn test_identify_with_none_admin_queue() {
    let config = AdminFsmIdentifyTestConfigs {
        dma_success_status: true,
        invalid_dma_tag: false,
        optional_dma_desc: true,
        invalid_io_tag: false,
        io_success_status: true,
        dma_alloc_succeeds: true,
        invalid_admin_queue: true,
        none_admin_queue: true,
        io_end_send_returns_none: false,
    };

    let mut fsm = AdminCmdFsm::new(make_fsm_for_identify(config));

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::RxReady, 0xFF),
        Err(AdminErr::ExpectedAdminQueue)
    );
}
