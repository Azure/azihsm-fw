// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

/// Controller State
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum ControllerState {
    /// Enabled State
    Enabled,

    /// Disabled State
    Disabled,
}

/// Controller
pub trait Controller {
    /// Controller state
    fn state(&self) -> ControllerState;

    /// Enable the controller
    fn enable(&self);

    /// Disable the controller
    fn disable(&self);

    /// Reset the controller
    fn reset(&self);
}
