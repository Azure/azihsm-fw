// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::rc::Rc;
use core::cell::RefCell;

use crate::*;

use mcr_registers::sys_mbx::mbx::RegisterBlock as MbxRegBlock;

/// Mailbox Controller
#[derive(Clone)]
pub struct MailboxController {
    rimpl: Rc<RefCell<MailboxControllerImpl>>,
}

impl MailboxController {
    /// Create an instance of Mailbox Controller
    ///
    /// # Arguments
    ///
    /// * `id` - Mailbox ID to be created of type MailboxId
    ///
    /// # Returns
    ///
    /// * `MailboxController` - Mailbox Controller instance
    pub fn create(id: MailboxId) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(MailboxControllerImpl::new(id))),
        }
    }
}

impl MailboxControllerTrait for MailboxController {
    fn init(&self) {
        self.rimpl.borrow().disable_mbx_err();
    }

    fn trigger_mbx_err(&self) {
        self.rimpl.borrow().trigger_mbx_err();
    }
}

struct MailboxControllerImpl {
    /// Mailbox register block
    mbx_reg: MbxRegBlock,

    /// Mailbox ID
    id: usize,
}

impl MailboxControllerImpl {
    pub fn new(id: MailboxId) -> Self {
        Self {
            mbx_reg: MbxRegBlock::block(),
            id: id as usize,
        }
    }
}

impl MailboxControllerImpl {
    fn disable_mbx_err(&self) {
        self.mbx_reg
            .at(self.id)
            .s2_h_mbx_insts()
            .read_and_modify(|_, w| w.err_bit(false));
    }

    fn trigger_mbx_err(&self) {
        self.mbx_reg
            .at(self.id)
            .s2_h_mbx_insts()
            .read_and_modify(|_, w| w.err_bit(true));
    }
}
