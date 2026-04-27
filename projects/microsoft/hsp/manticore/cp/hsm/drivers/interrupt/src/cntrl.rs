// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::rc::Rc;
use core::cell::RefCell;

use cortex_m::interrupt::InterruptNumber;
use mcr_registers::cortexm7::systemcontrol;

use crate::Interrupt;

/// Interrupt controller trait
pub trait InterruptControllerTrait: Default {
    /// Check if a specified interrupt is pending
    ///
    /// # Arguments
    ///
    /// * `interrupt` - Interrupt to check
    ///
    /// # Returns
    ///
    /// * `true` if interrupt is pending
    /// * `false` if interrupt is not pending
    fn pending(&self, interrupt: Interrupt) -> bool;

    /// Clear pending interrupt
    ///
    /// # Arguments
    ///
    /// * `interrupt` - Interrupt to clear
    fn clear(&self, interrupt: Interrupt);

    /// Enable interrupt
    ///
    /// # Arguments
    ///
    /// * `interrupt` - Interrupt to be enabled
    fn enable(&self, interrupt: Interrupt);

    /// Disable interrupt
    ///
    /// # Arguments
    ///
    /// * `interrupt` - Interrupt to be disabled
    fn disable(&self, interrupt: Interrupt);
}

/// Interrupt controller
#[derive(Clone)]
pub struct InterruptController {
    rimpl: Rc<RefCell<InterruptControllerImpl>>,
}

impl InterruptControllerTrait for InterruptController {
    /// Check if an interrupt is pending
    fn pending(&self, interrupt: Interrupt) -> bool {
        self.rimpl.borrow().pending(interrupt)
    }

    /// Clear pending interrupt
    fn clear(&self, interrupt: Interrupt) {
        self.rimpl.borrow().clear(interrupt);
    }

    /// Enable interrupt
    fn enable(&self, interrupt: Interrupt) {
        self.rimpl.borrow().enable(interrupt)
    }

    /// Disable interrupt
    fn disable(&self, interrupt: Interrupt) {
        self.rimpl.borrow().disable(interrupt)
    }
}

impl Default for InterruptController {
    /// Returns the "default value" for a type.
    fn default() -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(InterruptControllerImpl::default())),
        }
    }
}

/// Interrupt controller implementation
struct InterruptControllerImpl {
    reg: systemcontrol::RegisterBlock,
}

impl InterruptControllerImpl {
    /// Check if an interrupt is pending
    fn pending(&self, interrupt: Interrupt) -> bool {
        let nr = interrupt.number();
        self.reg
            .nvic_ispr_at(usize::from(nr >> 5))
            .nvic_ispr()
            .read()
            & (1 << (nr & 0x1F))
            != 0
    }

    /// Clear pending interrupt
    fn clear(&self, interrupt: Interrupt) {
        let nr = interrupt.number();
        self.reg
            .nvic_icpr_at(usize::from(nr >> 5))
            .nvic_icpr()
            .write(|_| 1 << (nr & 0x1F));
    }

    /// Enable interrupt
    fn enable(&self, interrupt: Interrupt) {
        let nr = interrupt.number();
        self.reg
            .nvic_iser_at(usize::from(nr >> 5))
            .nvic_iser()
            .write(|_| 1 << (nr & 0x1F));
    }

    /// Disable interrupt
    fn disable(&self, interrupt: Interrupt) {
        let nr = interrupt.number();
        self.reg
            .nvic_icer_at(usize::from(nr >> 5))
            .nvic_icer()
            .write(|_| 1 << (nr & 0x1F));
    }
}

impl Default for InterruptControllerImpl {
    /// Returns the "default value" for a type.
    fn default() -> Self {
        Self {
            reg: systemcontrol::RegisterBlock::block(),
        }
    }
}
