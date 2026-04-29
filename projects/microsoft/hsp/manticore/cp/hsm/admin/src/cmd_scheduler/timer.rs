// Copyright (c) Microsoft Corporation. All rights reserved.

/// `CmdTimer` is a simple countdown timer.
///
/// It counts down from a given number of ticks to zero.
pub struct CmdTimer {
    /// The number of ticks remaining until the timer times out.
    ticks_to_timeout: u8,
}

impl CmdTimer {
    /// Creates a new `CmdTimer` with no ticks (i.e., in a stopped state).
    pub fn new() -> Self {
        Self {
            ticks_to_timeout: 0,
        }
    }

    /// Starts the timer with the given number of ticks.
    ///
    /// # Panics
    ///
    /// Panics in debug mode if `ticks` is zero.
    pub fn start(&mut self, ticks: u8) {
        if ticks == 0 {
            panic!("Timer started with zero ticks");
        }
        self.ticks_to_timeout = ticks;
    }

    /// Stops the timer, setting the remaining ticks to zero.
    pub fn stop(&mut self) {
        self.ticks_to_timeout = 0;
    }

    /// Decrements the timer by one tick.
    ///
    /// # Returns
    ///
    /// * `true` if the programmed time out expired
    /// * `false` otherwise.
    pub fn tick(&mut self) -> bool {
        if self.ticks_to_timeout > 0 {
            self.ticks_to_timeout -= 1;
            return self.ticks_to_timeout == 0;
        }

        false
    }
}

impl Default for CmdTimer {
    /// Creates a default `CmdTimer`.
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_timer_elapses() {
        let mut timer = CmdTimer::new();
        timer.start(3);

        assert!(!timer.tick());
        assert!(!timer.tick());
        assert!(timer.tick());
        assert!(!timer.tick());
    }

    #[test]
    fn test_timer_stop() {
        let mut timer = CmdTimer::new();
        timer.start(3);

        assert!(!timer.tick());
        timer.stop();
        assert!(!timer.tick());
    }

    #[test]
    fn test_timer_restart() {
        let mut timer = CmdTimer::new();
        timer.start(3);

        assert!(!timer.tick());
        timer.start(3);
        assert!(!timer.tick());
        assert!(!timer.tick());
    }

    #[test]
    #[should_panic(expected = "Timer started with zero ticks")]
    fn test_timer_start_zero() {
        let mut timer = CmdTimer::default();
        timer.start(0);
    }
}
