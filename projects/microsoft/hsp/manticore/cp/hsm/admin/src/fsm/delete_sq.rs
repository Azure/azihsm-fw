// Copyright (c) Microsoft Corporation. All rights reserved.

use log::trace;
use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_logging::*;
use mcr_simplex::SimplexPipeTrait;
use mcr_types::*;

use super::types::AdminSqe;
use super::*;
use crate::context::AdminFsmContext;
use crate::error::HostStatusCode;
use crate::function::*;

/// Delete Submission Queue Command FSM states
#[derive(Copy, Clone, Debug)]
enum DeleteSqCmdFsmState {
    /// FSM is idle
    Init,

    /// Waiting for IPC channel resource to be ready
    WaitIpcChannel,

    /// Waiting for IPC channel response from HSM or FP core
    WaitIpcResponse,

    /// Waiting for pending queue delete from IO core
    WaitForPendingDelete,
}

/// Delete Submission Queue command
pub(crate) struct AdminDeleteSqCmd<E: AdminEnvTrait + 'static> {
    /// Delete Submission Queue command FSM state
    state: DeleteSqCmdFsmState,

    /// Host submission queue identifier
    host_sq: HostSqId,

    /// Device submission queue identifier
    dev_sq: DevSqId,

    /// Device completion queue identifier corresponding to this submission queue
    dev_cq: DevCqId,

    /// Fastpath IPC channel resource
    admin_to_fp_ipc_channel:
        Option<CmdResourceRef<AdminToFpIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    // HSM IPC channel resource
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

impl<E: AdminEnvTrait> AdminCmdTrait<E> for AdminDeleteSqCmd<E> {
    /// Get the response buffer
    fn response(&mut self) -> (Option<AdminCqe>, Option<DmaBuffer<E>>) {
        (self.cqe.take(), self.dma_buf.take())
    }

    /// Handle an event
    fn on_event(&mut self, event: AdminFsmEvent, tag: TagId) -> Result<(), AdminErr> {
        match (self.state, event) {
            (DeleteSqCmdFsmState::Init, AdminFsmEvent::StartCmd) => self.on_start(tag),
            (DeleteSqCmdFsmState::WaitIpcChannel, AdminFsmEvent::ResourceReady(resource)) => {
                self.on_ipc_channel_ready(tag, resource)
            }
            (DeleteSqCmdFsmState::WaitIpcResponse, AdminFsmEvent::HsmIpcResponse) => {
                self.on_hsm_ipc_response(tag)
            }
            (DeleteSqCmdFsmState::WaitIpcResponse, AdminFsmEvent::FpToAdminIpcResponse) => {
                self.on_fp_ipc_response()
            }
            (DeleteSqCmdFsmState::WaitForPendingDelete, AdminFsmEvent::IoCancellationComplete) => {
                self.on_deferred_queue_deletion(tag)
            }
            _ => Err(AdminErr::Pending),
        }
    }

    /// Acquire a resource
    fn acquire_resource(&mut self, tag: TagId, res_id: ResId) -> AdminFsmEvent {
        match res_id {
            AdminFsmResourceId::AdminToFpIpcChannel => {
                trace!("[tag: {}] FP IPC channel ready", tag);
                self.admin_to_fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::AdminToFpIpcChannel)
            }
            AdminFsmResourceId::HsmIpcChannel => {
                trace!("[tag: {}] HSM IPC channel ready", tag);
                self.hsm_ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());
                AdminFsmEvent::ResourceReady(ResId::HsmIpcChannel)
            }
            _ => AdminFsmEvent::Unknown,
        }
    }
}

impl<E: AdminEnvTrait> AdminDeleteSqCmd<E> {
    /// Create a new Admin Delete Submission Queue command FSM
    pub fn new(
        ctx: AdminFsmContext<E>,
        pfn: PcieFunction,
        sqe: AdminSqe,
        request: Option<DmaBuffer<E>>,
    ) -> Self {
        Self {
            state: DeleteSqCmdFsmState::Init,
            host_sq: Default::default(),
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
        self.validate_sqe().or_else(|err| {
            error!("[delete_sq] Invalid SQE: {:?}", err as u32);
            self.prepare_err_cqe(err)
        })?;

        // Get device submission and completion queue Id and store it in the command FSM
        self.device_queues(self.host_sq).or_else(|err| {
            error!("Get device Sq Failed {:?}", err as u32);
            self.prepare_err_cqe(err)
        })?;

        if let HostQueueType::Hsm = HostQueueType::from(self.host_sq) {
            self.try_send_hsm_ipc_request(tag)
        } else {
            self.try_send_fp_ipc_request(tag)
        }
    }

    /// On IPC channel ready event
    fn on_ipc_channel_ready(&mut self, tag: TagId, resource: ResId) -> Result<(), AdminErr> {
        trace!("[tag: {}] on_ipc_channel_ready", tag);

        match resource {
            ResId::HsmIpcChannel => self.send_hsm_ipc_request(tag),
            ResId::AdminToFpIpcChannel => self.send_fp_ipc_request(tag),
            _ => unreachable!(),
        }
    }

    /// On IPC channel response event from HSM core
    fn on_hsm_ipc_response(&mut self, tag: TagId) -> Result<(), AdminErr> {
        trace!("[tag: {}] on_hsm_ipc_response", tag);

        let message = self
            .hsm_ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()));

        self.process_ipc_response(message).or_else(|err| {
            if err == AdminErr::Pending {
                self.state = DeleteSqCmdFsmState::WaitForPendingDelete;

                Err(err)?
            } else {
                error!("Process HSM IPC failed {:?}", err as u32);
                self.prepare_err_cqe(err)
            }
        })?;

        self.delete_device_queue(self.host_sq).or_else(|err| {
            error!("[delete_sq] Delete device queue on_hsm_ipc_response: Delete submission queue failed {:?}", err as u32);
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
            error!("Process FP IPC failed {:?}", err as u32);
            self.prepare_err_cqe(err)
        })?;

        self.delete_device_queue(self.host_sq).or_else(|err| {
            error!("[delete_sq] Delete device queue on_fp_ipc_response: Delete submission queue failed {:?}", err as u32);
            self.prepare_err_cqe(err)
        })
    }

    /// On Deferred IO submission queue delete
    fn on_deferred_queue_deletion(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.validate_queue_delete_response(tag).or_else(|err| {
            error!(
                "[delete_sq] on_deferred_queue_deletion: Delete submission queue failed {:?}",
                err as u32
            );
            self.prepare_err_cqe(err)
        })?;

        self.prepare_cqe();

        Ok(())
    }

    /// Try acquiring the IPC channel resoruce for FP and send an IPC message
    fn try_send_fp_ipc_request(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.admin_to_fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
        if self.admin_to_fp_ipc_channel.is_none() {
            self.state = DeleteSqCmdFsmState::WaitIpcChannel;
            Err(AdminErr::Pending)?;
        }
        self.send_fp_ipc_request(tag)
    }

    /// Send an IPC message to FP core
    fn send_fp_ipc_request(&mut self, tag: TagId) -> Result<(), AdminErr> {
        let message = self.prepare_ipc_message(tag);

        self.admin_to_fp_ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, message.encode()))
            .or_else(|err| {
                error!("Failed to send FP IPC request {:?}", err);
                self.prepare_err_cqe(AdminErr::IpcSendRequestError)
            })?;

        self.state = DeleteSqCmdFsmState::WaitIpcResponse;

        Err(AdminErr::Pending)
    }

    /// Try acquiring the IPC channel resoruce for HSM and send an IPC message
    fn try_send_hsm_ipc_request(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.hsm_ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());
        if self.hsm_ipc_channel.is_none() {
            self.state = DeleteSqCmdFsmState::WaitIpcChannel;
            Err(AdminErr::Pending)?;
        }

        self.send_hsm_ipc_request(tag)
    }

    /// Send an IPC message to HSM core
    fn send_hsm_ipc_request(&mut self, tag: TagId) -> Result<(), AdminErr> {
        let message = self.prepare_ipc_message(tag);

        self.hsm_ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, message.encode()))
            .or_else(|err| {
                error!("Failed to send HSM IPC request {:?}", err);
                self.prepare_err_cqe(AdminErr::IpcSendRequestError)
            })?;

        self.state = DeleteSqCmdFsmState::WaitIpcResponse;

        Err(AdminErr::Pending)
    }

    fn process_ipc_response(&mut self, message: Option<IpcMessage>) -> Result<(), AdminErr> {
        if let Some(response) = message {
            let header = IpcMessageDecoder::decode_header(&response)
                .map_err(|_| AdminErr::InvalidIpcHeader)?;

            match header.status().into() {
                IpcMessageStatusCode::Success => (),
                IpcMessageStatusCode::Pending => Err(AdminErr::Pending)?,
                _ => Err(AdminErr::IpcResponseError)?,
            }
        } else {
            Err(AdminErr::SpuriousIpcMessage)?;
        }
        self.prepare_cqe();

        Ok(())
    }

    /// Get device submission queue info
    fn device_queues(&mut self, host_sq: HostSqId) -> Result<(), AdminErr> {
        self.dev_sq = self.ctx.function_mgr().function(self.pfn).dev_sq(host_sq)?;
        self.dev_cq = self.dev_sq.into();

        Ok(())
    }

    /// Delete device submission queue
    fn delete_device_queue(&mut self, host_sq: HostSqId) -> Result<(), AdminErr> {
        (self.dev_sq, self.dev_cq) = self
            .ctx
            .function_mgr()
            .function(self.pfn)
            .delete_sq(host_sq)?;

        Ok(())
    }

    /// Prepare an IPC message
    fn prepare_ipc_message(&mut self, tag: TagId) -> IpcMessageCreateDeleteSq {
        let mut message = IpcMessageCreateDeleteSq {
            info: SqCreateDeleteInfo {
                pfn: self.pfn,
                device_sq_id: self.dev_sq,
                device_cq_id: self.dev_cq,
                action: SqAction::Delete,
            },
            ..Default::default()
        };

        message.header.set_tag(tag.into());

        message
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
    fn validate_sqe(&mut self) -> Result<(), AdminErr> {
        let delete_sq_sqe: DeleteSqSqe = self.sqe.into();

        if let HostQueueType::Admin = HostQueueType::from(delete_sq_sqe.queue_id) {
            Err(AdminErr::InvalidQueueId)?;
        }

        self.host_sq = delete_sq_sqe.queue_id;

        Ok(())
    }

    /// Validate deferred queue delete response from HSM IO core
    fn validate_queue_delete_response(&mut self, tag: TagId) -> Result<(), AdminErr> {
        let message = self
            .ctx
            .queue_delete_notification()
            .recv()
            .ok_or(AdminErr::SpuriousIoQueueDeleteResp)?;

        if message.tag != tag {
            Err(AdminErr::IoTagMismatch)?;
        }

        if message.pfn != self.pfn {
            Err(AdminErr::InvalidPcieFn)?;
        }

        self.delete_device_queue(self.host_sq)
    }
}
