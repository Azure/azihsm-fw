// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_queue_controller::QueueMem;
use mcr_types::PcieFunction;

use super::types::AdminSqe;
use super::*;
use crate::context::AdminFsmContext;
use crate::error::HostStatusCode;
use crate::function::*;

/// Create Submission Queue Command FSM states
#[derive(Copy, Clone, Debug)]
enum CreateSqCmdFsmState {
    /// FSM is idle
    Init,

    /// Waiting for IPC channel resource to be ready
    WaitIpcChannel,

    /// Waiting for IPC channel response from HSM or FP core
    WaitIpcResponse,
}

/// Create Submission Queue command
pub(crate) struct AdminCreateSqCmd<E: AdminEnvTrait + 'static> {
    /// Create Submission Queue command FSM state
    state: CreateSqCmdFsmState,

    /// Device submission queue identifier
    dev_sq: DevSqId,

    /// Device completion queue identifier corresponding to this submission queue
    dev_cq: DevCqId,

    /// Fastpath IPC channel resource
    admin_to_fp_ipc_channel:
        Option<CmdResourceRef<AdminToFpIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// HSM IPC channel resource
    hsm_ipc_channel: Option<CmdResourceRef<HsmIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// PcieFunction
    pfn: PcieFunction,

    /// Admin SQE
    sqe: AdminSqe,

    /// Admin CQE
    cqe: Option<AdminCqe>,

    /// DMA buffer
    dma_buf: Option<DmaBuffer<E>>,

    /// Context
    ctx: AdminFsmContext<E>,
}

impl<E: AdminEnvTrait> AdminCmdTrait<E> for AdminCreateSqCmd<E> {
    /// Get the response buffer
    fn response(&mut self) -> (Option<AdminCqe>, Option<DmaBuffer<E>>) {
        (self.cqe.take(), self.dma_buf.take())
    }

    /// Handle an event
    fn on_event(&mut self, event: AdminFsmEvent, tag: TagId) -> Result<(), AdminErr> {
        match (self.state, event) {
            (CreateSqCmdFsmState::Init, AdminFsmEvent::StartCmd) => self.on_start(tag),
            (CreateSqCmdFsmState::WaitIpcChannel, AdminFsmEvent::ResourceReady(resource)) => {
                self.on_ipc_channel_ready(tag, resource)
            }
            (CreateSqCmdFsmState::WaitIpcResponse, AdminFsmEvent::HsmIpcResponse) => {
                self.on_hsm_ipc_response()
            }
            (CreateSqCmdFsmState::WaitIpcResponse, AdminFsmEvent::FpToAdminIpcResponse) => {
                self.on_fp_ipc_response()
            }
            _ => Err(AdminErr::Pending),
        }
    }

    /// Acquire a resource
    fn acquire_resource(&mut self, tag: TagId, res_id: ResId) -> AdminFsmEvent {
        match res_id {
            AdminFsmResourceId::AdminToFpIpcChannel => {
                self.admin_to_fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::AdminToFpIpcChannel)
            }
            AdminFsmResourceId::HsmIpcChannel => {
                self.hsm_ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::HsmIpcChannel)
            }
            _ => AdminFsmEvent::Unknown,
        }
    }
}

impl<E: AdminEnvTrait> AdminCreateSqCmd<E> {
    /// Create a new Admin Create Submission Queue command FSM
    pub fn new(
        ctx: AdminFsmContext<E>,
        pfn: PcieFunction,
        sqe: AdminSqe,
        request: Option<DmaBuffer<E>>,
    ) -> Self {
        Self {
            state: CreateSqCmdFsmState::Init,
            dev_sq: Default::default(),
            dev_cq: Default::default(),
            admin_to_fp_ipc_channel: None,
            hsm_ipc_channel: None,
            pfn,
            sqe,
            cqe: None,
            dma_buf: request,
            ctx,
        }
    }

    /// On start event of this command FSM
    fn on_start(&mut self, tag: TagId) -> Result<(), AdminErr> {
        // Validate and decode the SQE
        let create_sq_sqe = self
            .validate_and_decode_sqe()
            .or_else(|err| self.prepare_err_cqe(err))?;

        self.create_device_queue(create_sq_sqe)
            .or_else(|err| self.prepare_err_cqe(err))?;

        if let HostQueueType::Hsm = HostQueueType::from(create_sq_sqe.queue_id) {
            self.try_send_hsm_ipc_request(tag)
        } else {
            self.try_send_fp_ipc_request(tag)
        }
    }

    /// On IPC channel ready event
    fn on_ipc_channel_ready(&mut self, tag: TagId, resource: ResId) -> Result<(), AdminErr> {
        match resource {
            ResId::HsmIpcChannel => self.send_hsm_ipc_request(tag),
            ResId::AdminToFpIpcChannel => self.send_fp_ipc_request(tag),
            _ => unreachable!(),
        }
    }

    /// On IPC channel response event from HSM core
    fn on_hsm_ipc_response(&mut self) -> Result<(), AdminErr> {
        let message = self
            .hsm_ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()));

        self.process_ipc_response(message).or_else(|err| {
            let create_sq_sqe: CreateSqSqe = self.sqe.into();
            let _ = self
                .ctx
                .function_mgr()
                .function(self.pfn)
                .delete_sq(create_sq_sqe.queue_id);

            self.prepare_err_cqe(err)
        })
    }

    /// On IPC channel response event from FP core
    fn on_fp_ipc_response(&mut self) -> Result<(), AdminErr> {
        let message = self
            .admin_to_fp_ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()));

        self.process_ipc_response(message).or_else(|err| {
            let create_sq_sqe: CreateSqSqe = self.sqe.into();
            let _ = self
                .ctx
                .function_mgr()
                .function(self.pfn)
                .delete_sq(create_sq_sqe.queue_id);

            self.prepare_err_cqe(err)
        })
    }

    /// Try acquiring the IPC channel resoruce for HSM and send an IPC message
    fn try_send_hsm_ipc_request(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.hsm_ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());
        match self.hsm_ipc_channel.as_ref() {
            Some(_) => self.send_hsm_ipc_request(tag),
            None => {
                self.state = CreateSqCmdFsmState::WaitIpcChannel;

                Err(AdminErr::Pending)
            }
        }
    }

    /// Send an IPC message to HSM core
    fn send_hsm_ipc_request(&mut self, tag: u16) -> Result<(), AdminErr> {
        let message = self.prepare_ipc_message();

        self.hsm_ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, message.encode()))
            .or_else(|_| self.prepare_err_cqe(AdminErr::IpcSendRequestError))?;

        self.state = CreateSqCmdFsmState::WaitIpcResponse;

        Err(AdminErr::Pending)
    }

    /// Try acquiring the IPC channel resoruce for FP and send an IPC message
    fn try_send_fp_ipc_request(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.admin_to_fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
        if self.admin_to_fp_ipc_channel.is_none() {
            self.state = CreateSqCmdFsmState::WaitIpcChannel;
            Err(AdminErr::Pending)?;
        }
        self.send_fp_ipc_request(tag)
    }

    /// Send an IPC message to FP core
    fn send_fp_ipc_request(&mut self, tag: u16) -> Result<(), AdminErr> {
        let message = self.prepare_ipc_message();

        self.admin_to_fp_ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, message.encode()))
            .or_else(|_| self.prepare_err_cqe(AdminErr::IpcSendRequestError))?;

        self.state = CreateSqCmdFsmState::WaitIpcResponse;

        Err(AdminErr::Pending)
    }

    fn process_ipc_response(&mut self, message: Option<IpcMessage>) -> Result<(), AdminErr> {
        if let Some(response) = message {
            let header = IpcMessageDecoder::decode_header(&response)
                .map_err(|_| AdminErr::InvalidIpcHeader)?;

            if header.status() != 0 {
                Err(AdminErr::IpcResponseError)?;
            }
        } else {
            Err(AdminErr::SpuriousIpcMessage)?;
        }

        self.prepare_cqe();

        Ok(())
    }

    /// Create device submission queue
    fn create_device_queue(&mut self, create_sq_sqe: CreateSqSqe) -> Result<(), AdminErr> {
        let mem = QueueMem {
            addr: create_sq_sqe.prp1,
            len: create_sq_sqe.queue_len.into(),
        };

        (self.dev_sq, self.dev_cq) = self.ctx.function_mgr().function(self.pfn).create_sq(
            create_sq_sqe.queue_id,
            create_sq_sqe.host_cq_id,
            mem,
        )?;

        Ok(())
    }

    /// Prepare an IPC message
    fn prepare_ipc_message(&mut self) -> IpcMessageCreateDeleteSq {
        IpcMessageCreateDeleteSq {
            info: SqCreateDeleteInfo {
                pfn: self.pfn,
                device_sq_id: self.dev_sq,
                device_cq_id: self.dev_cq,
                action: SqAction::Create,
            },
            ..Default::default()
        }
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
    fn prepare_cqe(&mut self) {
        let cqe = AdminCqe {
            command_specific: 0,
            _rsvd: 0,
            sq_head: 0,
            sq_id: 0,
            cmd_id: self.sqe.cmd.id,
            psf: StatusField::new().with_status(HostStatusCode::Success),
        };

        self.cqe = Some(cqe);
    }

    /// Validate and decode the SQE
    fn validate_and_decode_sqe(&mut self) -> Result<CreateSqSqe, AdminErr> {
        let create_sq_sqe: CreateSqSqe = self.sqe.into();

        if !create_sq_sqe.attr.pc() {
            Err(AdminErr::InvalidFieldInCreateSqCmd)?
        }

        if create_sq_sqe.queue_len == 0 {
            Err(AdminErr::InvalidQueueSize)?
        }

        if create_sq_sqe.host_cq_id == HostCqId(0) {
            Err(AdminErr::InvalidHostCq)?
        }

        if let HostQueueType::Admin = HostQueueType::from(create_sq_sqe.queue_id) {
            Err(AdminErr::InvalidQueueId)?;
        }

        Ok(create_sq_sqe)
    }
}
