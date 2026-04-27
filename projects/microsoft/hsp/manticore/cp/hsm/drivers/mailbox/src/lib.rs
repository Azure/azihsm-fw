// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod mailbox;

pub use mailbox::MailboxController;

#[derive(Copy, Clone)]
pub enum MailboxId {
    /// Mailbox 0
    Mailbox0 = 0,

    /// Mailbox 1
    Mailbox1 = 1,
}

/// This trait describes the way to communicate from CP to HSP
pub trait MailboxControllerTrait {
    /// Initialize the controller
    fn init(&self);

    /// Send a notification to HSP
    fn trigger_mbx_err(&self);
}
