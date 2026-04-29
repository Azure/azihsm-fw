// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_pcie_controller::*;
use mcr_tcon::Tcon;
use mcr_types::DebugLogEntryParameters;
use mcr_types::DebugLogSeverity;

use super::*;
use crate::context::AdminFsmContext;
use crate::env::AdminEnvTrait;
use crate::error::AdminErr;
use crate::recorder::AdminFsmEventRecorder;
use crate::resource::AdminFsmResourceId;
use crate::AdminFsmEvent;

/// PCIe Timer Interval in milliseconds
const PCIE_TIMER_INTERVAL_MS: u32 = 1000;

/// PCIe Timer Ticks
const PCIE_TIMER_TICKS: u8 = Tcon::get_approximate_ticks_from_ms(PCIE_TIMER_INTERVAL_MS) as u8;

/// Expected PCIe Link Speed and Width
const PCIE_EXPECTED_LINK: PcieLinkStatus = PcieLinkStatus { speed: 5, width: 4 };

/// Telemetry FSM States
#[derive(Eq, PartialEq, Debug)]
enum TelemetryState {
    /// PCIe Telemetry FSM is initialized
    Initialized,

    /// PCIe Telemetry FSM is running normally with expected link values
    RunningNormally,

    /// PCIe Telemetry FSM is running but with unexpected link values
    RunningAbnormally,

    /// PCIe Telemetry FSM is faulted
    Faulted,
}

/// PCIe Telemetry FSM
pub(crate) struct TelemetryFsm<E: AdminEnvTrait + 'static> {
    /// Admin FSM Context
    ctx: AdminFsmContext<E>,

    /// Timer
    timer: CmdTimer,

    /// PCIe Telemetry State
    state: TelemetryState,

    /// Previous PCIe link status
    previous_pcie_link_status: PcieLinkStatus,
}

impl<E: AdminEnvTrait> CmdFsm for TelemetryFsm<E> {
    type Error = AdminErr;
    type ResourceId = AdminFsmResourceId;
    type Event = AdminFsmEvent;
    type Recorder = AdminFsmEventRecorder;

    /// Get the timer for the hierarchical FSM
    fn get_timer(&mut self) -> Option<&mut CmdTimer> {
        Some(&mut self.timer)
    }

    /// Handle an event
    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match event {
            AdminFsmEvent::PciePerstUp => self.on_pcie_perst_up(tag),
            AdminFsmEvent::PciePerstDown => self.on_pcie_perst_down(tag),
            AdminFsmEvent::TimerElapsed => self.handle_timer_event(tag, event),
            _ => {
                warn!(
                    "Telemetry FSM received unexpected event. Event: {:?}",
                    u32::from(event)
                );
                Err(AdminErr::Pending)
            }
        }
    }
}

impl<E: AdminEnvTrait> TelemetryFsm<E> {
    /// Create a new Telemetry FSM
    pub fn new(ctx: AdminFsmContext<E>) -> Self {
        let result = ctx.pcie_cntrl().link_status();

        let timer = CmdTimer::new();

        let mut telemetry_fsm = Self {
            ctx,
            timer,
            state: TelemetryState::Initialized,
            previous_pcie_link_status: PcieLinkStatus { speed: 0, width: 0 },
        };

        match result {
            Ok(result) => {
                telemetry_fsm.start_timer();
                telemetry_fsm.previous_pcie_link_status = PcieLinkStatus {
                    speed: result.speed,
                    width: result.width,
                };
            }
            Err(_e) => {
                warn!("Error running telemetry PCIe monitor, code {:#x}", _e);
            }
        };

        telemetry_fsm
    }

    /// Start the PCIe telemetry timer
    fn start_timer(&mut self) {
        self.timer.start(PCIE_TIMER_TICKS);
    }

    /// Check if the current PCIe link values are expected
    ///
    /// # Arguments
    ///
    /// * `current_link` - Current PCIe link status
    ///
    /// # Returns
    ///
    /// * `bool` - True if the current link values are expected, false otherwise
    fn link_values_expected(&mut self, current_link: PcieLinkStatus) -> bool {
        let mut result = false;
        if (current_link.speed == PCIE_EXPECTED_LINK.speed)
            && (current_link.width == PCIE_EXPECTED_LINK.width)
        {
            result = true;
            match self.state {
                TelemetryState::Initialized
                | TelemetryState::RunningAbnormally
                | TelemetryState::Faulted => {
                    info!("Telemetry PCIe monitor is running normally. Link Speed: {}, Link Width: x{}",
                        current_link.speed, current_link.width);
                }
                _ => {}
            }
        } else if self.state != TelemetryState::RunningAbnormally
            || (self.previous_pcie_link_status.speed != current_link.speed)
            || (self.previous_pcie_link_status.width != current_link.width)
        {
            warn!(
                "Unexpected PCIe Link. Link Speed: {}, Link Width: {}",
                current_link.speed, current_link.width
            );
        }

        result
    }

    /// Handle the PCIe timer event
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `event` - Admin FSM Event
    ///
    /// # Returns
    ///
    /// * `Result<(), AdminErr>` - Result of the operation
    fn handle_timer_event(&mut self, _tag: TagId, _event: AdminFsmEvent) -> Result<(), AdminErr> {
        let result = self.ctx.pcie_cntrl().link_status();

        match result {
            Ok(current_link) => {
                match self.state {
                    TelemetryState::Initialized => {
                        if self.link_values_expected(current_link) {
                            self.state = TelemetryState::RunningNormally;
                        } else {
                            self.state = TelemetryState::RunningAbnormally;
                        }
                    }
                    TelemetryState::RunningNormally => {
                        if !self.link_values_expected(current_link) {
                            self.state = TelemetryState::RunningAbnormally;
                        }
                    }
                    TelemetryState::RunningAbnormally => {
                        if self.link_values_expected(current_link) {
                            self.state = TelemetryState::RunningNormally;
                        }
                    }
                    TelemetryState::Faulted => {
                        if self.link_values_expected(current_link) {
                            self.state = TelemetryState::RunningNormally;
                        } else {
                            self.state = TelemetryState::RunningAbnormally;
                        }
                    }
                }
                self.previous_pcie_link_status = current_link;
            }
            Err(_e) => {
                if self.state != TelemetryState::Faulted {
                    warn!("Telemetry PCIe monitor not able to start, code {:#x}", _e);
                    self.state = TelemetryState::Faulted;
                    self.previous_pcie_link_status = PcieLinkStatus { speed: 0, width: 0 };
                }
            }
        }

        // Restart timer
        self.timer.start(PCIE_TIMER_TICKS);

        Err(AdminErr::Pending)
    }

    /// Handle PCIe PERST up event
    fn on_pcie_perst_up(&mut self, _tag: TagId) -> Result<(), AdminErr> {
        self.start_timer();

        Err(AdminErr::Pending)
    }

    /// Handle PCIe PERST down event
    fn on_pcie_perst_down(&mut self, _tag: TagId) -> Result<(), AdminErr> {
        self.timer.stop();

        Err(AdminErr::Pending)
    }
}
