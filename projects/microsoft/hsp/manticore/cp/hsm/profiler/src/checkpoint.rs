// Copyright (c) Microsoft Corporation. All rights reserved.

//! This module contains the checkpoint profiler.
//!
//! The checkpoint profiler is suitable for super high performance sensitive code paths.
//! It measures time from point A to point B to ... to point N.
//!
//! You should fetch the log using the `get_log` method. You should call this method against
//! the last checkpoint to get the full log preferably outside of the performance sensitive
//! code path.
//!
//! # Example Log
//!     5:foo:10:barbar:20
//! This above log means:
//!     - 5:foo: The time from the start to the checkpoint foo. The label of the start checkpoint
//!              is skipped for brevity.
//!     - foo:10:barbar: The time from the checkpoint foo to the checkpoint barbar.
//!     - barbar:20: The time from the checkpoint barbar to the time `get_log` was called.

extern crate alloc;

use alloc::format;
use alloc::rc::Rc;
use alloc::string::String;
use core::cell::RefCell;

/// Represents a checkpoint.
#[derive(Debug, Clone)]
pub struct Checkpoint {
    rimpl: Rc<RefCell<CheckpointImpl>>,
}

#[derive(Debug, Clone)]
struct CheckpointImpl {
    time: u64,
    label: &'static str,
    prev: Option<Checkpoint>,
}

impl Checkpoint {
    /// Create a new start checkpoint.
    /// You will need to call this method only for the start checkpoint.
    ///
    /// # Arguments
    /// * `label` - The label of the checkpoint.
    /// * `time` - The current time at the checkpoint.
    ///
    /// # Returns
    /// The checkpoint object.
    pub fn start(label: &'static str, time: u64) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(CheckpointImpl::new(label, time, None))),
        }
    }

    fn new_with_prev(label: &'static str, time: u64, prev: Checkpoint) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(CheckpointImpl::new(label, time, Some(prev)))),
        }
    }

    /// Record a new checkpoint.
    /// This method should be called on the most recent checkpoint object.
    ///
    /// # Arguments
    /// * `label` - The label of the checkpoint.
    /// * `time` - The current time at the checkpoint.
    ///
    /// # Returns
    /// The new checkpoint object.
    pub fn record(&self, label: &'static str, time: u64) -> Checkpoint {
        self.rimpl.borrow_mut().record(label, time, self.clone())
    }

    /// Get the log from the start to the current checkpoint.
    ///
    /// # Arguments
    /// * `end_time` - The current time.
    ///
    /// # Returns
    /// The log string.
    ///
    /// # Example Log
    ///     5:foo:10:barbar:20
    /// This above log means:
    ///     - 5:foo: The time taken from the start to the checkpoint foo. The label of the start checkpoint
    ///              is skipped for brevity.
    ///     - foo:10:barbar: The time taken from the checkpoint foo to the checkpoint barbar.
    ///     - barbar:20: The time taken from the checkpoint barbar to the time `get_log` was called.
    pub fn get_log(&self, end_time: u64) -> String {
        self.rimpl.borrow().get_log(end_time)
    }
}

impl CheckpointImpl {
    fn new(label: &'static str, time: u64, prev: Option<Checkpoint>) -> Self {
        Self { time, label, prev }
    }

    fn record(&mut self, label: &'static str, time: u64, prev: Checkpoint) -> Checkpoint {
        Checkpoint::new_with_prev(label, time, prev)
    }

    fn get_log(&self, end_time: u64) -> String {
        let mut log = String::new();
        let time_diff = end_time - self.time;

        if let Some(prev_cp) = &self.prev {
            log.push_str(&prev_cp.get_log(self.time));
            log.push_str(&format!(":{}:{}", self.label, time_diff));
        } else {
            log.push_str(&format!("{}", time_diff));
        }

        log
    }
}
