// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod simplex;
#[cfg(test)]
mod tests;

use mcr_error::*;
use mcr_types::VolatileCell;
pub use simplex::SimplexPipe;

/// Simplex Queue configuration
pub struct SimplexPipeConfig<T: 'static + Copy> {
    /// Queue
    pub queue: &'static mut [T],

    /// Queue consumer index
    pub ci: &'static VolatileCell<u32>,

    /// Queue producer index
    pub pi: &'static VolatileCell<u32>,
}

/// Simplex Pipe Trait to send message from one CPU to another without waiting for an ack
pub trait SimplexPipeTrait<Message>: Clone {
    /// Send a message using the simplex pipe
    ///
    /// # Arguments
    ///
    /// * `msg` - Message to be sent over the pipe
    ///
    /// # Returns
    ///
    /// * `McrResult<())>` - Ok if the message is sent successfully or an error code if the message
    ///   transmission failed
    fn send(&self, msg: Message) -> McrResult<()>;

    /// Receive a message from the simplex pipe
    ///
    /// # Returns
    ///
    /// * `Option<Self::Message>` - Some(Message) if a new message is available in the queue else
    ///   None
    fn recv(&self) -> Option<Message>;

    /// Peek the message at front of the pipe
    ///
    /// # Returns
    ///
    /// * `Option<Self::Message>` - Some(Message) if a new message is available in the queue else
    ///   None
    ///
    /// # Notes
    ///
    /// This call will not drain the message from the queue and will only peek the message. recv()
    /// function is required to be called to drain the message from the pipe
    fn peek(&self) -> Option<Message>;

    /// Check if the simplex pipe is empty
    ///
    /// # Returns
    ///
    /// * `bool` - true if the simple pipe is empty, false otherwise
    fn is_empty(&self) -> bool;

    /// Check if the simplex pipe is full
    ///
    /// # Returns
    ///
    /// * `bool` - true if the simple pipe is full, false otherwise
    fn is_full(&self) -> bool;

    /// Get count of empty slots in the pipe
    ///
    /// # Returns
    ///
    /// * `usize` - number of empty slots in the pipe
    fn empty_slot_count(&self) -> usize;
}

mcr_err_decl! {
    SimplexPipe,
    SimplexPipeErr
    {
        // Cannot send new message over the pipe since the Pipe is full
        PipeFull = 1,
    }
}
