// Copyright (c) Microsoft Corporation. All rights reserved.

#![warn(missing_docs)]

extern crate alloc;

use alloc::rc::Rc;
use core::cell::RefCell;

use mcr_error::*;

use crate::*;

/// Simplex pipe to be used to send message of type T from one CPU to another
#[derive(Clone)]
pub struct SimplexPipe<T: 'static + Clone + Copy> {
    /// Simplex Pipe implementation
    rimpl: Rc<RefCell<SimplexPipeImpl<T>>>,
}

impl<T: 'static + Clone + Copy> SimplexPipe<T> {
    /// Create new Simplex Pipe
    ///
    /// # Arguments
    ///
    /// * `config` - Simple Pipe configuration to send and receivce message over a queue
    pub fn new(config: SimplexPipeConfig<T>) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(SimplexPipeImpl::new(config))),
        }
    }
}

impl<T: Clone + Copy> SimplexPipeTrait<T> for SimplexPipe<T> {
    /// Send a message using the simplex pipe
    fn send(&self, msg: T) -> mcr_error::McrResult<()> {
        self.rimpl.borrow_mut().send(msg)
    }

    /// Receive a message from the simplex pipe
    fn recv(&self) -> Option<T> {
        self.rimpl.borrow_mut().recv()
    }

    /// Peek the message at front of the pipe
    fn peek(&self) -> Option<T> {
        self.rimpl.borrow().peek()
    }

    /// Check if the simplex pipe is empty
    fn is_empty(&self) -> bool {
        self.rimpl.borrow().is_empty()
    }

    /// Check if the simplex pipe is full
    fn is_full(&self) -> bool {
        self.rimpl.borrow().is_full()
    }

    /// Get count of empty slots in the pipe
    fn empty_slot_count(&self) -> usize {
        self.rimpl.borrow().empty_slot_count()
    }
}

struct SimplexPipeImpl<T: 'static + Copy> {
    config: SimplexPipeConfig<T>,
}

impl<T: 'static + Copy> SimplexPipeImpl<T> {
    /// Create new SimplexPipeImpl
    fn new(config: SimplexPipeConfig<T>) -> Self {
        config.ci.set(0);
        config.pi.set(0);

        Self { config }
    }

    /// Compute the next index with wraparound for the queue
    fn next_index(&self, idx: u32) -> u32 {
        let mut next = idx + 1;

        if next as usize == self.config.queue.len() {
            next = 0;
        }

        next
    }

    /// Send a new message using simplex pipe
    fn send(&mut self, msg: T) -> McrResult<()> {
        let ci = self.config.ci.get();
        let pi = self.config.pi.get();

        let new_pi = self.next_index(pi);
        if new_pi == ci {
            Err(SimplexPipeErr::PipeFull)?
        }

        self.config.queue[pi as usize] = msg;

        cortex_m::asm::dmb();

        self.config.pi.set(new_pi);

        Ok(())
    }

    /// Receive a new message from simplex pipe
    fn recv(&mut self) -> Option<T> {
        let mut ci = self.config.ci.get();

        if self.is_empty() {
            return None;
        }

        let message = self.config.queue[ci as usize];

        // Increment the consumer index with rollover condition
        ci = self.next_index(ci);

        self.config.ci.set(ci);

        Some(message)
    }

    /// Peek the message at front of the pipe
    fn peek(&self) -> Option<T> {
        if self.is_empty() {
            return None;
        }

        Some(self.config.queue[self.config.ci.get() as usize])
    }

    /// Check if the simplex pipe is empty
    fn is_empty(&self) -> bool {
        self.config.pi.get() == self.config.ci.get()
    }

    /// Check if the simplex pipe is full
    fn is_full(&self) -> bool {
        let new_pi = self.next_index(self.config.pi.get());

        new_pi == self.config.ci.get()
    }

    /// Get count of empty slots in the pipe
    fn empty_slot_count(&self) -> usize {
        let ci = self.config.ci.get();
        let pi = self.config.pi.get();
        if pi >= ci {
            self.config.queue.len() - (pi - ci) as usize - 1
        } else {
            (ci - pi) as usize - 1
        }
    }
}
