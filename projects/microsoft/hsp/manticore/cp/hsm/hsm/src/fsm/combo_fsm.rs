// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::fsm::*;
use crate::*;

#[allow(clippy::enum_variant_names)]
#[allow(clippy::large_enum_variant)]
enum Fsm<E: HsmEnvTrait + 'static> {
    HsmFsm(HsmFsm<E>),
    ResourceCleanup(HsmResCleanupFsm<E>),
    PartInitFsm(HsmPartInitFsm<E>),
}

pub(crate) struct ComboFsm<E: HsmEnvTrait + 'static> {
    fsm: Fsm<E>,
}

impl<E: HsmEnvTrait + 'static> CmdFsm for ComboFsm<E> {
    type Error = HsmErr;
    type ResourceId = HsmFsmResourceId;
    type Event = HsmFsmEvent;
    type Recorder = HsmFsmEventRecorder;

    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match &mut self.fsm {
            Fsm::HsmFsm(fsm) => fsm.on_event(event, tag),
            Fsm::ResourceCleanup(fsm) => fsm.on_event(event, tag),
            Fsm::PartInitFsm(fsm) => fsm.on_event(event, tag),
        }
    }

    fn acquire_resource(&mut self, tag: TagId, res_id: Self::ResourceId) -> Self::Event {
        match &mut self.fsm {
            Fsm::HsmFsm(fsm) => fsm.acquire_resource(tag, res_id),
            Fsm::ResourceCleanup(fsm) => fsm.acquire_resource(tag, res_id),
            Fsm::PartInitFsm(fsm) => fsm.acquire_resource(tag, res_id),
        }
    }
}

impl<E: HsmEnvTrait + 'static> ComboFsm<E> {
    pub fn new_hsm_fsm(fsm: HsmFsm<E>) -> Self {
        Self {
            fsm: Fsm::HsmFsm(fsm),
        }
    }

    pub fn new_hsm_res_cleanup_fsm(fsm: HsmResCleanupFsm<E>) -> Self {
        Self {
            fsm: Fsm::ResourceCleanup(fsm),
        }
    }

    pub fn new_hsm_part_init_fsm(fsm: HsmPartInitFsm<E>) -> Self {
        Self {
            fsm: Fsm::PartInitFsm(fsm),
        }
    }
}
