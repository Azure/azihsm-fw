// Copyright (c) Microsoft Corporation. All rights reserved.

#![cfg_attr(not(test), no_std)]

cfg_if::cfg_if! {
    if #[cfg(test)] {
        mod mock;
    }
}

extern crate alloc;

use alloc::vec::Vec;
use core::marker::PhantomData;

use mcr_event_loop::*;
use mcr_interrupt_controller::Interrupt;
use mcr_interrupt_controller::InterruptControllerTrait;

/// NVIC Event Loop
pub struct NvicEventLoop<E: Event, I: InterruptControllerTrait> {
    marker1: PhantomData<E>,
    marker2: PhantomData<I>,
}

impl<E: Event, I: InterruptControllerTrait> Default for NvicEventLoop<E, I> {
    fn default() -> Self {
        Self {
            marker1: Default::default(),
            marker2: Default::default(),
        }
    }
}

impl<E: Event, I: InterruptControllerTrait> EventLoopInterface for NvicEventLoop<E, I> {
    type Event = E;
    type EventIterator = NvicEventIterator<E, I>;

    /// Run the event loop
    fn run<F>(&self, closure: F)
    where
        F: FnOnce(Self::EventIterator),
        F: Send + 'static,
    {
        closure(NvicEventIterator::<Self::Event, I>::new());
    }
}

impl<E: Event, I: InterruptControllerTrait> NvicEventLoop<E, I> {
    /// Run the event loop with a custom interrupt controller
    #[cfg(test)]
    fn run_with_controller<F>(&self, intc_cntrl: I, closure: F)
    where
        F: FnOnce(NvicEventIterator<E, I>),
        F: Send + 'static,
    {
        closure(NvicEventIterator::<E, I>::new_with_controller(intc_cntrl));
    }
}

/// NVIC Event Iterator
pub struct NvicEventIterator<E: Event, I: InterruptControllerTrait> {
    intc_cntrl: I,
    prev_irq: Option<Interrupt>,
    marker: PhantomData<E>,
    irqs: Vec<IrqGroup>,
}

impl<E: Event, I: InterruptControllerTrait> NvicEventIterator<E, I> {
    /// Create an instance of `NvicEventIterator`
    fn new() -> Self {
        Self {
            intc_cntrl: I::default(),
            prev_irq: None,
            marker: PhantomData,
            irqs: E::interrupts(),
        }
    }

    /// Create an instance of `NvicEventIterator` with a custom interrupt controller
    #[cfg(test)]
    fn new_with_controller(intc_cntrl: I) -> Self {
        Self {
            intc_cntrl,
            prev_irq: None,
            marker: PhantomData,
            irqs: E::interrupts(),
        }
    }
}

impl<E: Event, I: InterruptControllerTrait> Iterator for NvicEventIterator<E, I> {
    type Item = E;

    /// Advances the iterator and returns the next value.
    fn next(&mut self) -> Option<Self::Item> {
        // Mark previous NVIC interrupt as handled only after it is handled at the source.
        if let Some(irq) = self.prev_irq {
            self.intc_cntrl.clear(irq);
        }

        self.prev_irq = None;

        for irq_group in self.irqs.iter_mut() {
            let mut count = 0;
            let next = irq_group.next;
            irq_group.update_next();
            let iter = irq_group
                .interrupts
                .iter()
                .cycle()
                .skip(next)
                .take(irq_group.interrupts.len());

            for irq_info in iter {
                if self.intc_cntrl.pending(irq_info.num) {
                    if irq_info.sensitivity == IrqSensitivity::Edge {
                        self.intc_cntrl.clear(irq_info.num);
                    } else {
                        self.prev_irq = Some(irq_info.num);
                    }

                    return Some(E::from(irq_info.event_num));
                }

                count += 1;
                if count == irq_group.interrupts.len() {
                    break;
                }
            }
        }

        None
    }
}

#[cfg(test)]
mod tests {
    use mcr_interrupt_controller::Interrupt;

    use super::*;
    use crate::mock::MockInterruptController;

    #[derive(mcr_event_loop_derive::Event, Copy, Clone, Eq, PartialEq)]
    pub enum AdminEvent {
        #[event(interrupt = Interrupt::pcie_perst_up_irq)]
        PerstUp,

        #[event(interrupt = Interrupt::pcie_perst_down_irq)]
        PerstDown,

        #[event(interrupt = Interrupt::ucd_ibcq_rr1_irq)]
        UcdIbcq,

        #[event(interrupt = Interrupt::upka0_done_irq, sensitivity = IrqSensitivity::Edge, group = "upka_done")]
        Upka0Done,

        #[event(interrupt = Interrupt::upka1_done_irq, sensitivity = IrqSensitivity::Edge, group = "upka_done")]
        Upka1Done,

        #[event(sensitivity = IrqSensitivity::Edge, interrupt = Interrupt::upka0_error_irq, group = "upka_err")]
        Upka0Error,

        #[event(group = "upka_err", sensitivity = IrqSensitivity::Edge, interrupt = Interrupt::upka1_error_irq)]
        Upka1Error,
    }

    #[test]
    fn test_pri0_event() {
        let mut mock = MockInterruptController::new();
        let times: usize = usize::from(AdminEvent::PerstUp) + 1usize;
        mock.expect_pending()
            .returning(|i| i == Interrupt::pcie_perst_up_irq)
            .times(times);

        let event_loop = NvicEventLoop::<AdminEvent, MockInterruptController>::default();
        event_loop.run_with_controller(mock, |mut iter| {
            assert!(matches!(iter.next(), Some(AdminEvent::PerstUp)));
        });
    }

    #[test]
    fn test_pri1_event() {
        let mut intc = MockInterruptController::new();
        let times: usize = usize::from(AdminEvent::PerstDown) + 1;
        intc.expect_pending()
            .returning(|i| i == Interrupt::pcie_perst_down_irq)
            .times(times);

        let event_loop = NvicEventLoop::<AdminEvent, MockInterruptController>::default();
        event_loop.run_with_controller(intc, |mut iter| {
            assert!(matches!(iter.next(), Some(AdminEvent::PerstDown)));
        });
    }

    #[test]
    fn test_edge_event_hi_priority() {
        let mut intc = MockInterruptController::new();
        let times: usize = usize::from(AdminEvent::Upka0Done) + 1;
        intc.expect_pending()
            .returning(|i| i == Interrupt::upka0_done_irq)
            .times(times);
        intc.expect_clear().returning(|_| ()).times(1);

        let event_loop = NvicEventLoop::<AdminEvent, MockInterruptController>::default();
        event_loop.run_with_controller(intc, |mut iter| {
            assert!(matches!(iter.next(), Some(AdminEvent::Upka0Done)));
        });
    }

    #[test]
    fn test_edge_event_lo_priority() {
        let mut intc = MockInterruptController::new();
        let times: usize = usize::from(AdminEvent::Upka1Done) + 1;
        intc.expect_pending()
            .returning(|i| i == Interrupt::upka1_done_irq)
            .times(times);
        intc.expect_clear().returning(|_| ()).times(1);

        let event_loop = NvicEventLoop::<AdminEvent, MockInterruptController>::default();
        event_loop.run_with_controller(intc, |mut iter| {
            assert!(matches!(iter.next(), Some(AdminEvent::Upka1Done)));
        });
    }

    #[test]
    fn test_edge_event_hi_after_lo_priority() {
        let mut intc = MockInterruptController::new();
        let times: usize = usize::from(AdminEvent::Upka0Done) + 1;
        intc.expect_pending()
            .returning(|i| i == Interrupt::upka0_done_irq)
            .times(times);
        intc.expect_clear().returning(|_| ()).times(1);
        intc.expect_pending()
            .returning(|i| i == Interrupt::upka1_done_irq)
            .times(times);
        intc.expect_clear().returning(|_| ()).times(1);

        let event_loop = NvicEventLoop::<AdminEvent, MockInterruptController>::default();
        event_loop.run_with_controller(intc, |mut iter| {
            assert!(matches!(iter.next(), Some(AdminEvent::Upka0Done)));
            assert!(matches!(iter.next(), Some(AdminEvent::Upka1Done)));
        });
    }

    #[test]
    fn test_error_edge_event_hi_priority() {
        let mut intc = MockInterruptController::new();
        let times: usize = usize::from(AdminEvent::Upka0Error) + 1;
        intc.expect_pending()
            .returning(|i| i == Interrupt::upka0_error_irq)
            .times(times);
        intc.expect_clear().returning(|_| ()).times(1);

        let event_loop = NvicEventLoop::<AdminEvent, MockInterruptController>::default();
        event_loop.run_with_controller(intc, |mut iter| {
            assert!(matches!(iter.next(), Some(AdminEvent::Upka0Error)));
        });
    }

    #[test]
    fn test_error_edge_event_lo_priority() {
        let mut intc = MockInterruptController::new();
        let times: usize = usize::from(AdminEvent::Upka1Error) + 1;
        intc.expect_pending()
            .returning(|i| i == Interrupt::upka1_error_irq)
            .times(times);
        intc.expect_clear().returning(|_| ()).times(1);

        let event_loop = NvicEventLoop::<AdminEvent, MockInterruptController>::default();
        event_loop.run_with_controller(intc, |mut iter| {
            assert!(matches!(iter.next(), Some(AdminEvent::Upka1Error)));
        });
    }

    #[test]
    fn test_error_edge_event_hi_after_lo_priority() {
        let mut intc = MockInterruptController::new();
        let times: usize = usize::from(AdminEvent::Upka0Error) + 1;
        intc.expect_pending()
            .returning(|i| i == Interrupt::upka0_error_irq)
            .times(times);
        intc.expect_clear().returning(|_| ()).times(1);
        intc.expect_pending()
            .returning(|i| i == Interrupt::upka1_error_irq)
            .times(times);
        intc.expect_clear().returning(|_| ()).times(1);

        let event_loop = NvicEventLoop::<AdminEvent, MockInterruptController>::default();
        event_loop.run_with_controller(intc, |mut iter| {
            assert!(matches!(iter.next(), Some(AdminEvent::Upka0Error)));
            assert!(matches!(iter.next(), Some(AdminEvent::Upka1Error)));
        });
    }

    #[test]
    fn test_clear_interrupt() {
        let mut mock = MockInterruptController::new();
        let times: usize = usize::from(AdminEvent::PerstUp) + 2;
        mock.expect_pending()
            .returning(|i| i == Interrupt::pcie_perst_up_irq)
            .times(times);
        mock.expect_clear().returning(|_| ()).times(1);

        let event_loop = NvicEventLoop::<AdminEvent, MockInterruptController>::default();
        event_loop.run_with_controller(mock, |mut iter| {
            assert!(matches!(iter.next(), Some(AdminEvent::PerstUp)));
            assert!(matches!(iter.next(), Some(AdminEvent::PerstUp)));
        });
    }
}
