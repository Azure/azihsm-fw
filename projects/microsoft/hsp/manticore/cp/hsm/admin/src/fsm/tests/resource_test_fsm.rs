// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::{
    error::AdminErr,
    fsm::{AdminFsm, ResId},
    resource::{
        AdminFsmResourceId, AdminToFpIpcChannel, CastIdle, DoeIdle, HsmIpcChannel, HspIpcChannel,
        TdispIdle,
    },
    AdminEnvTrait, AdminFsmContext, AdminFsmEvent, AdminFsmEventRecorder, CmdFsm, CmdResourceRef,
    CmdTimer,
};

pub(crate) struct ResourceTestFsm<E: AdminEnvTrait + 'static> {
    /// Admin to HSP IPC channel resource
    hsp_ipc_channel: Option<CmdResourceRef<HspIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// Admin to FP IPC channel resource
    fp_ipc_channel: Option<CmdResourceRef<AdminToFpIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// Admin to HSM IPC channel resource
    hsm_ipc_channel: Option<CmdResourceRef<HsmIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// cast_idle resource
    cast_idle: Option<CmdResourceRef<CastIdle, AdminFsm<E>>>,

    /// doe_idle resource
    doe_idle: Option<CmdResourceRef<DoeIdle, AdminFsm<E>>>,

    /// tdisp_idle resource
    tdisp_idle: Option<CmdResourceRef<TdispIdle, AdminFsm<E>>>,

    /// Context
    ctx: AdminFsmContext<E>,

    /// Timer
    timer: CmdTimer,

    /// The expected ticks, after which timeout will happen and resource will be released
    expected_ticks: Option<u8>,
}

/// This FSM is used to acquire resouces and release resources
///
/// It makes use of the existing AdminFsmEvent to acquire different resources
/// AdminFsmEvent::Doe -> DoeIdle
/// AdminFsmEvent::Ide -> TdispIdle
/// AdminFsmEvent::HspToAdminIpcRequest -> HspIpcChannel
/// AdminFsmEvent::SelfTestResponse -> CastIdle
/// AdminFsmEvent::FpToAdminIpcResponse -> AdminToFpIpcChannel
/// AdminFsmEvent::HsmIpcResponse -> HsmIpcChannel
///
/// Once the resource is acquired, if FSM receives the same event again, it will release the resource
///
impl<E: AdminEnvTrait> CmdFsm for ResourceTestFsm<E> {
    type Error = AdminErr;
    type ResourceId = AdminFsmResourceId;
    type Event = AdminFsmEvent;
    type Recorder = AdminFsmEventRecorder;

    /// Get the timer for the hierarchical FSM
    fn get_timer(&mut self) -> Option<&mut CmdTimer> {
        Some(&mut self.timer)
    }

    fn acquire_resource(&mut self, tag: crate::TagId, res_id: Self::ResourceId) -> Self::Event {
        match res_id {
            AdminFsmResourceId::HspIpcChannel => {
                assert!(self.hsp_ipc_channel.is_none());
                self.hsp_ipc_channel = self.ctx.admin_to_hsp_ipc_channel().acquire(tag, ());
                assert!(self.hsp_ipc_channel.is_some());
                AdminFsmEvent::ResourceReady(ResId::HspIpcChannel)
            }
            AdminFsmResourceId::CastIdle => {
                assert!(self.cast_idle.is_none());
                self.cast_idle = self.ctx.cast_idle().acquire(tag, ());
                assert!(self.cast_idle.is_some());
                AdminFsmEvent::ResourceReady(ResId::CastIdle)
            }
            AdminFsmResourceId::DoeIdle => {
                assert!(self.doe_idle.is_none());
                self.doe_idle = self.ctx.doe_idle().acquire(tag, ());
                assert!(self.doe_idle.is_some());
                AdminFsmEvent::ResourceReady(ResId::DoeIdle)
            }
            AdminFsmResourceId::TdispIdle => {
                assert!(self.tdisp_idle.is_none());
                self.tdisp_idle = self.ctx.tdisp_idle().acquire(tag, ());
                assert!(self.tdisp_idle.is_some());
                AdminFsmEvent::ResourceReady(ResId::TdispIdle)
            }
            AdminFsmResourceId::AdminToFpIpcChannel => {
                assert!(self.fp_ipc_channel.is_none());
                self.fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
                assert!(self.fp_ipc_channel.is_some());
                AdminFsmEvent::ResourceReady(ResId::AdminToFpIpcChannel)
            }
            AdminFsmResourceId::HsmIpcChannel => {
                assert!(self.hsm_ipc_channel.is_none());
                self.hsm_ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());
                assert!(self.hsm_ipc_channel.is_some());
                AdminFsmEvent::ResourceReady(ResId::HsmIpcChannel)
            }
        }
    }

    fn on_event(&mut self, event: Self::Event, tag: crate::TagId) -> Result<(), Self::Error> {
        match event {
            AdminFsmEvent::SelfTestResponse => {
                if self.cast_idle.is_none() {
                    self.cast_idle = self.ctx.cast_idle().acquire(tag, ());
                    if let Some(ticks) = self.expected_ticks {
                        self.timer.start(ticks);
                    }
                } else {
                    self.cast_idle.take();
                }

                Err(AdminErr::Pending)
            }
            AdminFsmEvent::Ide(_) => {
                if self.tdisp_idle.is_none() {
                    self.tdisp_idle = self.ctx.tdisp_idle().acquire(tag, ());
                    if let Some(ticks) = self.expected_ticks {
                        self.timer.start(ticks);
                    }
                } else {
                    self.tdisp_idle.take();
                }

                Err(AdminErr::Pending)
            }
            AdminFsmEvent::Doe(_) => {
                if self.doe_idle.is_none() {
                    self.doe_idle = self.ctx.doe_idle().acquire(tag, ());
                    if let Some(ticks) = self.expected_ticks {
                        self.timer.start(ticks);
                    }
                } else {
                    self.doe_idle.take();
                }

                Err(AdminErr::Pending)
            }
            AdminFsmEvent::HspToAdminIpcRequest => {
                if self.hsp_ipc_channel.is_none() {
                    self.hsp_ipc_channel = self.ctx.admin_to_hsp_ipc_channel().acquire(tag, ());
                    if let Some(ticks) = self.expected_ticks {
                        self.timer.start(ticks);
                    }
                } else {
                    self.hsp_ipc_channel.take();
                }

                Err(AdminErr::Pending)
            }
            AdminFsmEvent::FpToAdminIpcResponse => {
                if self.fp_ipc_channel.is_none() {
                    self.fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
                    if let Some(ticks) = self.expected_ticks {
                        self.timer.start(ticks);
                    }
                } else {
                    self.fp_ipc_channel.take();
                }

                Err(AdminErr::Pending)
            }
            AdminFsmEvent::HsmIpcResponse => {
                if self.hsm_ipc_channel.is_none() {
                    self.hsm_ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());
                    if let Some(ticks) = self.expected_ticks {
                        self.timer.start(ticks);
                    }
                } else {
                    self.hsm_ipc_channel.take();
                }

                Err(AdminErr::Pending)
            }
            AdminFsmEvent::ResourceReady(res_id) => {
                match res_id {
                    AdminFsmResourceId::HspIpcChannel => {
                        assert!(self.hsp_ipc_channel.is_some());
                    }
                    AdminFsmResourceId::CastIdle => {
                        assert!(self.cast_idle.is_some());
                    }
                    AdminFsmResourceId::DoeIdle => {
                        assert!(self.doe_idle.is_some());
                    }
                    AdminFsmResourceId::TdispIdle => {
                        assert!(self.tdisp_idle.is_some());
                    }
                    AdminFsmResourceId::AdminToFpIpcChannel => {
                        assert!(self.fp_ipc_channel.is_some());
                    }
                    AdminFsmResourceId::HsmIpcChannel => {
                        assert!(self.hsm_ipc_channel.is_some());
                    }
                }

                Err(AdminErr::Pending)
            }
            AdminFsmEvent::TimerElapsed => {
                self.cast_idle.take();
                self.doe_idle.take();
                self.fp_ipc_channel.take();
                self.hsm_ipc_channel.take();
                self.hsp_ipc_channel.take();
                self.tdisp_idle.take();

                Err(AdminErr::Pending)
            }
            _ => unreachable!(),
        }
    }
}

impl<E: AdminEnvTrait> ResourceTestFsm<E> {
    pub fn new(ctx: AdminFsmContext<E>, expected_ticks: Option<u8>) -> Self {
        Self {
            hsp_ipc_channel: None,
            fp_ipc_channel: None,
            hsm_ipc_channel: None,
            cast_idle: None,
            doe_idle: None,
            tdisp_idle: None,
            ctx,
            expected_ticks,
            timer: CmdTimer::new(),
        }
    }
}
