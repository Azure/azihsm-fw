// Copyright (c) Microsoft Corporation. All rights reserved.

mod admin_fsm;
mod aes_gcm_ext;
mod cast;
mod cntrl;
mod create_cq;
mod create_sq;
mod delete_cq;
mod delete_sq;
mod doe;
mod get_features;
mod get_res;
mod identify;
mod idfu;
mod pcie;
mod set_features;
mod set_res;
mod stop_interface;
mod tdisp_int;
mod telemetry;
mod types;
mod unsupported;
mod vf_prep;
mod vf_restore;
mod vf_save;
mod vf_start;
mod vf_stop;
mod vflr;

#[cfg(test)]
mod tests;

pub(crate) use admin_fsm::AdminCmdFsm;
pub(crate) use aes_gcm_ext::AesGcmExtFsm;
pub(crate) use cast::CastFsm;
pub(crate) use cntrl::CntrlFsm;
pub(crate) use doe::DoeFsm;
pub(crate) use idfu::IdfuFsm;
pub(crate) use pcie::PcieFsm;
pub(crate) use stop_interface::StopInterfaceFsm;
pub(crate) use tdisp_int::TdispIntFsm;
pub(crate) use telemetry::TelemetryFsm;
pub(crate) use vflr::VflrFsm;

use mcr_alloc::*;
use mcr_io_controller::*;
use mcr_types::*;

cfg_if::cfg_if! {
    if #[cfg(test)] {
        use mcr_self_test::SelfTest;
        pub(crate) use tests::AdminFsmTest;
        pub(crate) use tests::ResourceTestFsm;
    }
}

use self::create_cq::AdminCreateCqCmd;
use self::create_sq::AdminCreateSqCmd;
use self::delete_cq::AdminDeleteCqCmd;
use self::delete_sq::AdminDeleteSqCmd;
use self::get_features::AdminGetFeaturesCmd;
use self::get_res::AdminGetResCmd;
use self::identify::AdminIdentifyCmd;
use self::set_features::AdminSetFeaturesCmd;
use self::set_res::AdminSetResCmd;
use self::types::*;
use self::unsupported::AdminUnsupportedCmd;
use self::vf_prep::AdminVfPrepCmd;
use self::vf_restore::AdminVfRestoreCmd;
use self::vf_save::AdminVfSaveCmd;
use self::vf_start::AdminVfStartCmd;
use self::vf_stop::AdminVfStopCmd;

use crate::env::*;
use crate::error::*;
use crate::recorder::*;
use crate::resource::*;
use crate::*;

pub(crate) type DmaBuffer<E> = <<E as AdminEnvTrait>::DmaHeap as DmaHeapTrait>::Alloc;
pub(crate) type ResId = AdminFsmResourceId;

pub(crate) enum AdminFsm<E: AdminEnvTrait + 'static> {
    Pcie(PcieFsm<E>),
    Vflr(VflrFsm<E>),
    Cntrl(CntrlFsm<E>),
    Doe(DoeFsm<E>),
    Idfu(IdfuFsm<E>),
    AdminCmd(AdminCmdFsm<E>),
    Cast(CastFsm<E>),
    Telemetry(TelemetryFsm<E>),
    #[cfg(test)]
    ResourceTest(ResourceTestFsm<E>),
    TdispInt(TdispIntFsm<E>),
    StopInterface(StopInterfaceFsm<E>),
    AesGcmExt(AesGcmExtFsm<E>),
}

impl<E: AdminEnvTrait> CmdFsm for AdminFsm<E> {
    type Error = AdminErr;
    type ResourceId = AdminFsmResourceId;
    type Event = AdminFsmEvent;
    type Recorder = AdminFsmEventRecorder;

    fn get_timer(&mut self) -> Option<&mut CmdTimer> {
        match self {
            AdminFsm::Pcie(fsm) => fsm.get_timer(),
            AdminFsm::Vflr(fsm) => fsm.get_timer(),
            AdminFsm::Cntrl(fsm) => fsm.get_timer(),
            AdminFsm::Doe(fsm) => fsm.get_timer(),
            AdminFsm::Idfu(fsm) => fsm.get_timer(),
            AdminFsm::AdminCmd(fsm) => fsm.get_timer(),
            AdminFsm::Cast(fsm) => fsm.get_timer(),
            AdminFsm::Telemetry(fsm) => fsm.get_timer(),
            #[cfg(test)]
            AdminFsm::ResourceTest(fsm) => fsm.get_timer(),
            AdminFsm::TdispInt(fsm) => fsm.get_timer(),
            AdminFsm::StopInterface(fsm) => fsm.get_timer(),
            AdminFsm::AesGcmExt(fsm) => fsm.get_timer(),
        }
    }

    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match self {
            AdminFsm::Pcie(fsm) => fsm.on_event(event, tag),
            AdminFsm::Vflr(fsm) => fsm.on_event(event, tag),
            AdminFsm::Cntrl(fsm) => fsm.on_event(event, tag),
            AdminFsm::Doe(fsm) => fsm.on_event(event, tag),
            AdminFsm::Idfu(fsm) => fsm.on_event(event, tag),
            AdminFsm::AdminCmd(fsm) => fsm.on_event(event, tag),
            AdminFsm::Cast(fsm) => fsm.on_event(event, tag),
            AdminFsm::Telemetry(fsm) => fsm.on_event(event, tag),
            #[cfg(test)]
            AdminFsm::ResourceTest(fsm) => fsm.on_event(event, tag),
            AdminFsm::TdispInt(fsm) => fsm.on_event(event, tag),
            AdminFsm::StopInterface(fsm) => fsm.on_event(event, tag),
            AdminFsm::AesGcmExt(fsm) => fsm.on_event(event, tag),
        }
    }

    fn acquire_resource(&mut self, tag: TagId, res_id: Self::ResourceId) -> Self::Event {
        match self {
            AdminFsm::Pcie(fsm) => fsm.acquire_resource(tag, res_id),
            AdminFsm::Vflr(fsm) => fsm.acquire_resource(tag, res_id),
            AdminFsm::Cntrl(fsm) => fsm.acquire_resource(tag, res_id),
            AdminFsm::Doe(fsm) => fsm.acquire_resource(tag, res_id),
            AdminFsm::Idfu(fsm) => fsm.acquire_resource(tag, res_id),
            AdminFsm::AdminCmd(fsm) => fsm.acquire_resource(tag, res_id),
            AdminFsm::Cast(fsm) => fsm.acquire_resource(tag, res_id),
            AdminFsm::Telemetry(fsm) => fsm.acquire_resource(tag, res_id),
            #[cfg(test)]
            AdminFsm::ResourceTest(fsm) => fsm.acquire_resource(tag, res_id),
            AdminFsm::TdispInt(fsm) => fsm.acquire_resource(tag, res_id),
            AdminFsm::StopInterface(fsm) => fsm.acquire_resource(tag, res_id),
            AdminFsm::AesGcmExt(fsm) => fsm.acquire_resource(tag, res_id),
        }
    }
}

#[cfg(test)]
impl<E: AdminEnvTrait> AdminFsm<E> {
    pub fn cast_fsm_set_next_test(&mut self, next_test: SelfTest) {
        match self {
            AdminFsm::Cast(fsm) => fsm.set_next_test(next_test),
            _ => unreachable!(),
        }
    }
}

pub(crate) trait AdminCmdTrait<E: AdminEnvTrait> {
    /// Get the response buffer
    ///
    /// # Returns
    ///
    /// * `Option<DmaBuffer<E>>` - Output DMA buffer
    fn response(&mut self) -> (Option<AdminCqe>, Option<DmaBuffer<E>>);

    /// Handle an event
    ///
    /// # Arguments
    ///
    /// * `event` - Event to handle
    /// * `tag` - Tag ID
    ///
    /// # Returns
    ///
    /// * `Result<(), HsmErr>` - Result of handling the event
    fn on_event(&mut self, event: AdminFsmEvent, tag: TagId) -> Result<(), AdminErr>;

    /// Acquire a resource
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `res_id` - Resource ID
    ///
    /// # Returns
    ///
    /// * `AdminFsmEvent` - Event to wake the state machine with
    #[allow(unused_variables)]
    fn acquire_resource(&mut self, tag: TagId, res_id: ResId) -> AdminFsmEvent {
        unimplemented!()
    }
}
