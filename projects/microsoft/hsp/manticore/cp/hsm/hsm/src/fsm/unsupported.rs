// Copyright (c) Microsoft Corporation. All rights reserved.

use core::marker::PhantomData;

use super::*;

/// Unsupported command
pub(crate) struct UnsupportedCmd<E: HsmEnvTrait> {
    /// Phantom data
    marker: PhantomData<*mut E>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for UnsupportedCmd<E> {
    /// Get the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        None
    }

    /// Handle an event
    fn on_event(&mut self, _event: HsmFsmEvent, _tag: TagId) -> Result<(), HsmErr> {
        Err(HsmErr::UnsupportedCmd)
    }

    /// Get the session ID this command FSM operates on-behalf of
    fn session_id(&self) -> Option<u16> {
        None
    }
}

impl<E: HsmEnvTrait> UnsupportedCmd<E> {
    /// Create a new command FSM
    pub fn new() -> Self {
        Self {
            marker: Default::default(),
        }
    }
}
