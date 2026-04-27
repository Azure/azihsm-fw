// Copyright (c) Microsoft Corporation. All rights reserved.

use test_log::test;

use super::helper::*;
use crate::cmd_scheduler::CmdFsm;
use crate::error::AdminErr;
use crate::event::AdminFsmEvent;
use crate::fsm::TelemetryFsm;

/// PCIe reset state machine test configuration
#[derive(Default)]
pub(crate) struct TelemetryFsmTestConfigs {
    /// Can this test case expect an unknown event?
    pub unknown_event: bool,

    /// SoC Reset or Warm Reset
    pub warm_reset: bool,

    /// Simulate Timer Configuration
    pub timer_event: bool,

    /// Simulate unexpected link status output
    pub link_status_unexpected: bool,

    /// Simulate service fsm related test cases
    pub test_service_fsm: bool,

    /// Simulate service fsm which test normal to abnormal pcie link
    pub normal_to_abnormal: bool,

    /// Simulate service fsm which test abnormal to normal pcie link
    pub abnormal_to_normal: bool,

    /// Simulate service fsm which test abnormal to abnormal pcie link
    pub abnormal_to_abnormal: bool,

    /// Simulate service fsm which test faulted to normal pcie link
    pub faulted_to_normal: bool,

    /// Simulate service fsm which test faulted to normal pcie link
    pub faulted_to_abnormal: bool,
}

#[test]
fn unknown_event() {
    let config = TelemetryFsmTestConfigs {
        unknown_event: true,
        ..Default::default()
    };

    let ctx = make_telemetry_fsm(config);

    // Telemetry FSM
    let mut fsm = TelemetryFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn perst_down() {
    let config = TelemetryFsmTestConfigs {
        ..Default::default()
    };

    let ctx = make_telemetry_fsm(config);

    // Telemetry FSM
    let mut fsm = TelemetryFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstDown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn perst_up() {
    let config = TelemetryFsmTestConfigs {
        warm_reset: true,
        ..Default::default()
    };

    let ctx = make_telemetry_fsm(config);

    // Telemetry FSM
    let mut fsm = TelemetryFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::PciePerstUp, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn get_timer() {
    let config = TelemetryFsmTestConfigs {
        ..Default::default()
    };

    let ctx = make_telemetry_fsm(config);

    // Telemetry FSM
    let mut fsm = TelemetryFsm::new(ctx);

    assert!(fsm.get_timer().is_some());
}

#[test]
fn timer_elapsed_event_link_status_err() {
    let config = TelemetryFsmTestConfigs {
        timer_event: true,
        ..Default::default()
    };

    let ctx = make_telemetry_fsm(config);

    // Telemetry FSM
    let mut fsm = TelemetryFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn timer_elapsed_event_normal_to_abnormal_link() {
    let config = TelemetryFsmTestConfigs {
        timer_event: true,
        warm_reset: true,
        test_service_fsm: true,
        normal_to_abnormal: true,
        ..Default::default()
    };

    let ctx = make_telemetry_fsm(config);

    // Telemetry FSM
    let mut fsm = TelemetryFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn timer_elapsed_event_abnormal_to_normal_link() {
    let config = TelemetryFsmTestConfigs {
        timer_event: true,
        link_status_unexpected: true,
        test_service_fsm: true,
        abnormal_to_normal: true,
        ..Default::default()
    };

    let ctx = make_telemetry_fsm(config);

    // Telemetry FSM
    let mut fsm = TelemetryFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn timer_elapsed_event_abnormal_to_abnormal_link() {
    let config = TelemetryFsmTestConfigs {
        timer_event: true,
        link_status_unexpected: true,
        test_service_fsm: true,
        abnormal_to_abnormal: true,
        ..Default::default()
    };

    let ctx = make_telemetry_fsm(config);

    // Telemetry FSM
    let mut fsm = TelemetryFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn timer_elapsed_event_abnormal_to_faulted_link() {
    let config = TelemetryFsmTestConfigs {
        timer_event: true,
        link_status_unexpected: true,
        test_service_fsm: true,
        ..Default::default()
    };

    let ctx = make_telemetry_fsm(config);

    // Telemetry FSM
    let mut fsm = TelemetryFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn timer_elapsed_event_normal_to_faulted_link() {
    let config = TelemetryFsmTestConfigs {
        timer_event: true,
        warm_reset: true,
        test_service_fsm: true,
        ..Default::default()
    };

    let ctx = make_telemetry_fsm(config);

    // Telemetry FSM
    let mut fsm = TelemetryFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn timer_elapsed_event_faulted_to_normal_link() {
    let config = TelemetryFsmTestConfigs {
        timer_event: true,
        warm_reset: true,
        test_service_fsm: true,
        faulted_to_normal: true,
        ..Default::default()
    };

    let ctx = make_telemetry_fsm(config);

    // Telemetry FSM
    let mut fsm = TelemetryFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn timer_elapsed_event_faulted_to_abnormal_link() {
    let config = TelemetryFsmTestConfigs {
        timer_event: true,
        warm_reset: true,
        test_service_fsm: true,
        faulted_to_abnormal: true,
        ..Default::default()
    };

    let ctx = make_telemetry_fsm(config);

    // Telemetry FSM
    let mut fsm = TelemetryFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::TimerElapsed, 0xFF),
        Err(AdminErr::Pending)
    );
}
