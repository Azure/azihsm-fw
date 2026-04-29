// Copyright (c) Microsoft Corporation. All rights reserved.

use test_log::test;

use super::helper::*;
use crate::cmd_scheduler::CmdFsm;
use crate::error::AdminErr;
use crate::event::AdminFsmEvent;
use crate::fsm::PcieFsm;

/// PCIe reset state machine test configuration
#[derive(Default)]
pub(crate) struct PcieFsmTestConfigs {
    /// Can this test case expect an unknown event?
    pub unknown_event: bool,

    /// Can this test case expect a PERST_DOWN or FLR reset event?
    pub reset: bool,

    /// is current outstanding reset event an FLR?
    pub flr: bool,

    /// Does sending FP CPU a reset event succeeded?
    pub send_fp_event_succeed: bool,

    /// Does sending HSM  CPU a reset event succeeded?
    pub send_hsm_event_succeed: bool,

    /// How many times do we expect send reset event to FP and HSM?
    pub ipc_call_count: usize,

    /// SoC Reset or Warm Reset
    pub warm_reset: bool,

    /// PERST# Down while FLR is pending
    pub perst_down_while_flr_pending: bool,
}

#[test]
fn unknown_event() {
    let config = PcieFsmTestConfigs {
        unknown_event: true,
        ..Default::default()
    };

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn perst_up() {
    let config = PcieFsmTestConfigs::default();

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn warm_reset_link_up() {
    let config = PcieFsmTestConfigs {
        unknown_event: true,
        warm_reset: true,
        ..Default::default()
    };

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn perst_up_back_to_back() {
    let config = PcieFsmTestConfigs::default();

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn perst_down_after_perst_up_fp_responds_first() {
    let config = PcieFsmTestConfigs {
        reset: true,
        send_fp_event_succeed: true,
        send_hsm_event_succeed: true,
        ipc_call_count: 1,
        ..Default::default()
    };

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstDown, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpResetComplete, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmResetComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn perst_down_after_perst_up_hsm_responds_first() {
    let config = PcieFsmTestConfigs {
        reset: true,
        send_fp_event_succeed: true,
        send_hsm_event_succeed: true,
        ipc_call_count: 1,
        ..Default::default()
    };

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstDown, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmResetComplete, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpResetComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn flr_fp_respond_first() {
    let config = PcieFsmTestConfigs {
        reset: true,
        flr: true,
        send_fp_event_succeed: true,
        send_hsm_event_succeed: true,
        ipc_call_count: 1,
        ..Default::default()
    };

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpResetComplete, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmResetComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn flr_hsm_respond_first() {
    let config = PcieFsmTestConfigs {
        reset: true,
        flr: true,
        send_fp_event_succeed: true,
        send_hsm_event_succeed: true,
        ipc_call_count: 1,
        ..Default::default()
    };

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmResetComplete, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpResetComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn flr_in_perst_down() {
    let config = PcieFsmTestConfigs {
        unknown_event: true,
        ..Default::default()
    };

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
#[should_panic]
fn flr_tests_fp_event_channel_begin_event_failure() {
    let config = PcieFsmTestConfigs {
        reset: true,
        flr: true,
        send_hsm_event_succeed: true,
        ipc_call_count: 1,
        ..Default::default()
    };

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmResetComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
#[should_panic]
fn perst_down_tests_fp_event_channel_begin_event_failure() {
    let config = PcieFsmTestConfigs {
        reset: true,
        send_hsm_event_succeed: true,
        ipc_call_count: 1,
        ..Default::default()
    };

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstDown, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmResetComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
#[should_panic]
fn flr_tests_hsm_event_channel_begin_event_failure() {
    let config = PcieFsmTestConfigs {
        reset: true,
        flr: true,
        send_fp_event_succeed: true,
        ipc_call_count: 1,
        ..Default::default()
    };

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
#[should_panic]
fn perst_down_tests_hsm_event_channel_begin_event_failure() {
    let config = PcieFsmTestConfigs {
        reset: true,
        send_fp_event_succeed: true,
        ipc_call_count: 1,
        ..Default::default()
    };

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstDown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn perst_down_after_flr_before_hsm_responds() {
    let config = PcieFsmTestConfigs {
        reset: true,
        flr: true,
        send_fp_event_succeed: true,
        send_hsm_event_succeed: true,
        ipc_call_count: 1,
        perst_down_while_flr_pending: true,
        ..Default::default()
    };

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstDown, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmResetComplete, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpResetComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn flr_repeated() {
    let config = PcieFsmTestConfigs {
        reset: true,
        flr: true,
        send_fp_event_succeed: true,
        send_hsm_event_succeed: true,
        ipc_call_count: 1,
        ..Default::default()
    };

    let ctx = make_pcie_fsm(config);

    // PCIe FSM
    let mut fsm = PcieFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PcieFlr, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmResetComplete, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpResetComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}
