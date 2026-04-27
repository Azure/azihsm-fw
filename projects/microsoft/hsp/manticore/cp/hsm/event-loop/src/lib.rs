// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

extern crate alloc;

use alloc::string::String;
use alloc::vec::Vec;

use mcr_interrupt_controller::Interrupt;

/// Interrupt sensitivity
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum IrqSensitivity {
    /// Level sensitive
    Level,

    /// Edge sensitive
    Edge,
}

/// Interrupt information
#[derive(Clone, Copy)]
pub struct IrqInfo {
    /// Interrupt
    pub num: Interrupt,

    /// Interrupt sensitivity
    pub sensitivity: IrqSensitivity,

    /// Event number associated with the interrupt
    pub event_num: usize,
}

/// Interrupt group
pub struct IrqGroup {
    /// Interrupt group id
    pub id: Option<String>,

    /// Interrupts information
    pub interrupts: Vec<IrqInfo>,

    /// Next interrupt to be processed
    pub next: usize,
}

impl IrqGroup {
    /// Move to next interrupt
    pub fn update_next(&mut self) {
        self.next = (self.next + 1) % self.interrupts.len();
    }
}
pub trait Event: From<usize> + Into<usize> + Copy + Clone + Eq + PartialEq {
    /// Get Interrupts in priority order
    fn interrupts() -> Vec<IrqGroup>;
}

/// Event loop interface
pub trait EventLoopInterface {
    /// Event Type
    type Event: Event;
    type EventIterator: Iterator<Item = Self::Event>;

    /// Run the event loop
    ///
    /// # Arguments
    ///
    /// * `f` - Closure to execute
    fn run<F>(&self, f: F)
    where
        F: FnOnce(Self::EventIterator),
        F: Send + 'static;
}

/// Event Loop
#[derive(Default)]
pub struct EventLoop<I>
where
    I: EventLoopInterface,
{
    event_loop: I,
}

impl<I> EventLoop<I>
where
    I: EventLoopInterface,
{
    /// Create an instance of `EventLoop`
    ///
    /// # Arguments
    ///
    /// * `event_loop` - Event loop
    pub fn new(event_loop: I) -> Self {
        Self { event_loop }
    }

    /// Run the event loop
    ///
    /// # Arguments
    ///
    /// * `f` - Closure to execute
    pub fn run<F>(&self, f: F)
    where
        F: FnOnce(I::EventIterator),
        F: Send + 'static,
    {
        self.event_loop.run(f)
    }
}
