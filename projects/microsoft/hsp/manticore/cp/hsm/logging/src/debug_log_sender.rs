// Copyright (c) Microsoft Corporation. All rights reserved.

#![warn(missing_docs)]

use mcr_simplex::SimplexPipe;
use mcr_simplex::SimplexPipeConfig;
use mcr_simplex::SimplexPipeTrait;
use mcr_types::DebugLogEntryParameters;
use mcr_types::VolatileCell;

/// Global object for the debug log sender
///
/// Each core(CP0/CP1) will initialize this object with its respective queues
pub static mut DEBUG_LOG_SENDER: Option<DebugLogSender> = None;

/// Sends debug logs from cores to the SP cores.
#[derive(Clone)]
pub struct DebugLogSender {
    /// The FIFO that logs are sent to.
    pipe: SimplexPipe<DebugLogEntryParameters>,

    /// The number of elements in the queue array. Must be filled in by the sender on
    /// initialization.
    ring_buffer_size: &'static VolatileCell<u32>,

    /// The number of times a send was attempted on a full pipe. This is an increasing count that
    /// is set by the sender and triggers an appropriate diagnostic action on the receiver when
    /// incremented.
    sender_overflows: &'static VolatileCell<u32>,
}

impl DebugLogSender {
    /// Creates and initializes a new instance with the provided reference parameters. References
    /// are to data structure fields that are common across cores and languages.
    pub fn new(
        pipe_config: SimplexPipeConfig<DebugLogEntryParameters>,
        ring_buffer_size: &'static VolatileCell<u32>,
        sender_overflows: &'static VolatileCell<u32>,
    ) -> Self {
        let queue_len = pipe_config.queue.len() as u32;
        let buffer = DebugLogSender {
            pipe: SimplexPipe::new(pipe_config),
            ring_buffer_size,
            sender_overflows,
        };
        buffer.ring_buffer_size.set(queue_len);
        buffer.sender_overflows.set(0);
        buffer
    }
}

impl DebugLogSenderTrait for DebugLogSender {
    /// Sends the provided debug log entry to the destination
    fn send(&self, debug_log_entry: DebugLogEntryParameters) {
        if self.pipe.send(debug_log_entry).is_err() {
            self.sender_overflows.set(self.sender_overflows.get() + 1);
        }
    }
}

/// Debug Log Sender Trait
pub trait DebugLogSenderTrait {
    /// Send the provided debug log entry to the ring buffer.
    ///
    /// # Arguments
    /// * `self` - DebugLogSender object
    /// * `debug_log_entry` - Structure entry formatted with the necessary information to send.
    ///
    fn send(&self, debug_log_entry: DebugLogEntryParameters);
}

/// Get the global debug log sender instance.
///
pub fn get_debug_log_sender() -> Option<&'static DebugLogSender> {
    #[allow(static_mut_refs)]
    unsafe {
        DEBUG_LOG_SENDER.as_ref()
    }
}
