// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::*;
use alloc::rc::Rc;
use bitfield::{Bit, BitMut};
use core::cell::RefCell;
use mcr_crypto_pka::PkaTrait;

/// FSM for cleaning up resources that needs interaction with the hardware. For now, only PKA
/// is supported. The FSM is used to clean up the PKA resources when a PKA engine instance is
/// released by another command FSM.
pub(crate) struct HsmResCleanupFsm<E: HsmEnvTrait + 'static> {
    pub(crate) env: Rc<RefCell<E>>,
    pub(crate) pka_state: u16,
}

impl<E: HsmEnvTrait + 'static> CmdFsm for HsmResCleanupFsm<E> {
    type Error = HsmErr;
    type ResourceId = HsmFsmResourceId;
    type Event = HsmFsmEvent;
    type Recorder = HsmFsmEventRecorder;

    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match event {
            HsmFsmEvent::ResourceCleanup(res, id) => {
                if res == HsmFsmResourceId::Pka {
                    if !self.pka_state.bit(id) {
                        self.on_pka_cleanup(id, tag)?;
                    } else {
                        error!(
                            "Unexpected: Resource cleanup for an already cleaned up resource {}.",
                            id as u32
                        );
                    }
                }
            }
            HsmFsmEvent::PkaDone(id) => {
                if self.pka_state.bit(id) {
                    self.on_pka_cleanup_done(id, tag)?;
                } else {
                    error!(
                        "Unexpected: Resource cleanup completed for resource {}.",
                        id as u32
                    );
                }
            }
            HsmFsmEvent::PkaError(id) => {
                if self.pka_state.bit(id) {
                    self.on_pka_cleanup_error(id)?;
                }
            }
            _ => {}
        }

        // Always return pending as it is always running FSM
        Err(HsmErr::Pending)
    }
}

impl<E: HsmEnvTrait + 'static> HsmResCleanupFsm<E> {
    pub fn new(env: Rc<RefCell<E>>) -> Self {
        Self { env, pka_state: 0 }
    }

    fn on_pka_cleanup(&mut self, id: usize, tag: TagId) -> Result<(), HsmErr> {
        self.env.borrow().hal().pka()[id]
            .begin_memory_wipe(tag)
            .map_err(|_| HsmErr::PkaMemoryWipeFailed)?;
        self.pka_state.set_bit(id, true);

        Err(HsmErr::Pending)
    }

    fn on_pka_cleanup_done(&mut self, id: usize, tag: TagId) -> Result<(), HsmErr> {
        self.pka_state.set_bit(id, false);
        self.env.borrow().hal().pka()[id]
            .end_memory_wipe(tag)
            .map_err(|_| HsmErr::PkaMemoryWipeFailed)?;

        // Continue waking up the next fsm in the queue
        self.env.borrow().pka_engine().wakeup(id);

        Err(HsmErr::DrainReady)
    }

    fn on_pka_cleanup_error(&mut self, id: usize) -> Result<(), HsmErr> {
        error!("PKA instance {} cleanup error", id as u32);
        panic!("PKA ({}) cleanup error", id);
    }
}
