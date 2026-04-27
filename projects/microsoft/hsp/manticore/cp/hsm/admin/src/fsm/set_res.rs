// Copyright (c) Microsoft Corporation. All rights reserved.

use log::trace;
use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_logging::*;
use mcr_queue_controller::QueueCntrlId;
use mcr_types::PcieFunction;

use super::types::AdminSqe;
use super::*;
use crate::context::AdminFsmContext;
use crate::error::HostStatusCode;
use crate::function::*;
use crate::resource::HsmIpcChannel;

/// Set Resource Command FSM states
#[derive(Copy, Clone, Debug)]
enum SetResCmdFsmState {
    /// FSM is idle
    Init,

    /// Waiting for IPC channel resource to be ready
    WaitIpcChannel,

    /// Waiting for IPC channel response from HSM core
    WaitIpcResponse,
}

/// Set resource command
pub(crate) struct AdminSetResCmd<E: AdminEnvTrait + 'static> {
    /// Set Resource command FSM state
    state: SetResCmdFsmState,

    /// Number of resources to be assigned to the owner function
    res_cnt: u32,

    /// Function owning the resources
    res_owner: PcieFunction,

    /// IPC channel resource
    ipc_channel: Option<CmdResourceRef<HsmIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// PCIe function number this command belongs to
    pfn: PcieFunction,

    /// Admin SQE
    sqe: AdminSqe,

    /// Admin CQE
    cqe: Option<AdminCqe>,

    /// DMA buffer
    dma_buf: Option<DmaBuffer<E>>,

    /// Context
    ctx: AdminFsmContext<E>,

    /// GUID for the resource
    vm_launch_guid: VmLaunchGuid,
}

impl<E: AdminEnvTrait> AdminCmdTrait<E> for AdminSetResCmd<E> {
    /// Get the response buffer
    fn response(&mut self) -> (Option<AdminCqe>, Option<DmaBuffer<E>>) {
        (self.cqe.take(), self.dma_buf.take())
    }

    /// Handle an event
    fn on_event(&mut self, event: AdminFsmEvent, tag: TagId) -> Result<(), AdminErr> {
        match (self.state, event) {
            (SetResCmdFsmState::Init, AdminFsmEvent::StartCmd) => self.on_start(tag),
            (SetResCmdFsmState::WaitIpcChannel, AdminFsmEvent::ResourceReady(_)) => {
                self.on_ipc_channel_ready(tag)
            }
            (SetResCmdFsmState::WaitIpcResponse, AdminFsmEvent::HsmIpcResponse) => {
                self.on_ipc_channel_response(tag)
            }
            _ => Err(AdminErr::Pending),
        }
    }

    /// Acquire a resource
    fn acquire_resource(&mut self, tag: TagId, _res_id: ResId) -> AdminFsmEvent {
        trace!("[tag: {}] HSM IPC channel ready", tag);
        self.ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());
        AdminFsmEvent::ResourceReady(ResId::HsmIpcChannel)
    }
}

impl<E: AdminEnvTrait> AdminSetResCmd<E> {
    /// Create a new Admin Set Resource command FSM
    pub fn new(
        ctx: AdminFsmContext<E>,
        pfn: PcieFunction,
        sqe: AdminSqe,
        request: Option<DmaBuffer<E>>,
    ) -> Self {
        Self {
            ctx,
            state: SetResCmdFsmState::Init,
            res_cnt: Default::default(),
            res_owner: PcieFunction::Pf,
            ipc_channel: None,
            pfn,
            sqe,
            cqe: None,
            dma_buf: request,
            vm_launch_guid: [0u8; 16],
        }
    }

    /// On start event of this command FSM
    fn on_start(&mut self, tag: TagId) -> Result<(), AdminErr> {
        trace!("[tag: {}] on_start", tag);

        // Only PF can issue this command
        if self.pfn != PcieFunction::Pf {
            self.prepare_err_cqe(AdminErr::InvalidSetResCmd)?;
        }

        // Validate and decode the SQE
        let (func, res_cnt) = self.validate_and_decode_sqe().or_else(|err| {
            error!("[set_res] Invalid SQE: {:?}", err as u32);
            self.prepare_err_cqe(err)
        })?;

        // set_res_cnt returns the current resource count that this function owns before new
        // allocation
        let previous_res_cnt =
            self.ctx
                .function_mgr()
                .set_res_cnt(func, res_cnt)
                .or_else(|err| {
                    error!("Failed to set resource count: {:?}", err as u32);
                    self.prepare_err_cqe(AdminErr::SetResCountLimitExceeded)
                })?;

        if previous_res_cnt == res_cnt {
            self.res_cnt = res_cnt;
            self.prepare_cqe(self.res_cnt);

            Ok(())
        } else {
            self.res_owner = func;
            self.try_send_ipc_request(tag)
        }
    }

    /// On IPC channel ready event
    fn on_ipc_channel_ready(&mut self, tag: TagId) -> Result<(), AdminErr> {
        trace!("[tag: {}] on_ipc_channel_ready", tag);

        self.send_ipc_request(tag)
    }

    /// On IPC channel response event
    fn on_ipc_channel_response(&mut self, tag: TagId) -> Result<(), AdminErr> {
        trace!("[tag: {}] on_ipc_channel_response", tag);

        if let Some(message) = self.receive_ipc_message() {
            let header = IpcMessageDecoder::decode_header(&message)
                .or_else(|_| self.prepare_err_cqe(AdminErr::InvalidIpcHeader))?;

            if header.status() != 0 {
                error!(
                    "[set_res] Invalid IPC response with status {}",
                    header.status()
                );
                self.prepare_err_cqe(AdminErr::IpcResponseError)?;
            }
        } else {
            self.prepare_err_cqe(AdminErr::SpuriousIpcMessage)?;
        }

        self.prepare_cqe(self.res_cnt);

        Ok(())
    }

    /// Try acquiring the IPC channel resoruce and send an IPC message
    fn try_send_ipc_request(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());
        if self.ipc_channel.is_none() {
            self.state = SetResCmdFsmState::WaitIpcChannel;
            Err(AdminErr::Pending)?;
        }

        self.send_ipc_request(tag)
    }

    /// Send an IPC message to HSM core
    fn send_ipc_request(&mut self, tag: u16) -> Result<(), AdminErr> {
        let message = IpcMessageSetRes {
            info: SetResInfo {
                mask: self.ctx.function_mgr().function(self.res_owner).res_mask(),
                pfn: self.res_owner,
                vm_launch_guid: self.vm_launch_guid,
            },
            ..Default::default()
        };

        self.ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, message.encode()))
            .or_else(|_| self.prepare_err_cqe(AdminErr::IpcSendRequestError))?;

        self.state = SetResCmdFsmState::WaitIpcResponse;

        Err(AdminErr::Pending)
    }

    /// Receive an IPC message from HSM core
    fn receive_ipc_message(&mut self) -> Option<IpcMessage> {
        self.ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()))
    }

    /// Prepare an error CQE
    fn prepare_err_cqe<T>(&mut self, err: AdminErr) -> Result<T, AdminErr> {
        let cqe = AdminCqe {
            command_specific: 0,
            _rsvd: 0,
            sq_head: 0,
            sq_id: 0,
            cmd_id: self.sqe.cmd.id,
            psf: StatusField::new().with_status(err.into()),
        };
        self.cqe = Some(cqe);

        Err(err)
    }

    /// Prepare a success CQE
    fn prepare_cqe(&mut self, res_cnt: u32) {
        let cqe = AdminCqe {
            command_specific: res_cnt,
            _rsvd: 0,
            sq_head: 0,
            sq_id: 0,
            cmd_id: self.sqe.cmd.id,
            psf: StatusField::new().with_status(HostStatusCode::Success),
        };

        self.cqe = Some(cqe);
    }

    /// Validate and decode the SQE
    fn validate_and_decode_sqe(&mut self) -> Result<(PcieFunction, u32), AdminErr> {
        let set_res_sqe: GetSetResourceSqe = self.sqe.into();
        let res_cnt = set_res_sqe.num_resource;

        if res_cnt > MAX_FUNCTION_RESOURCES.into() {
            self.prepare_err_cqe(AdminErr::InvalidResCountFieldInSqe)?
        }
        let cntrl_id: QueueCntrlId = set_res_sqe
            .cntrl_id
            .try_into()
            .map_err(|_| AdminErr::InvalidCntrlIdFieldInSqe)?;
        let func: PcieFunction = cntrl_id.into();

        self.vm_launch_guid = set_res_sqe.vm_launch_guid;

        Ok((func, res_cnt))
    }
}
