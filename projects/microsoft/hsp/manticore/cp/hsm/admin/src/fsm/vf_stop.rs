// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::vec::Vec;

use mcr_ipc_controller::IpcMessage;
use mcr_ipc_controller::IpcMessageChannelTrait;
use mcr_ipc_message::IpcMessageCreateDeleteSq;
use mcr_ipc_message::IpcMessageDecoder;
use mcr_ipc_message::IpcMessageEncoderTrait;
use mcr_ipc_message::IpcMessageStatusCode;
use mcr_ipc_message::SqAction;
use mcr_ipc_message::SqCreateDeleteInfo;
use mcr_queue_controller::QueueCntrlId;
use mcr_simplex::SimplexPipeTrait;

use crate::function::FunctionMgrTrait;
use crate::function::FunctionTrait;

use super::*;

/// VF Stop States
#[derive(Debug, Clone, Copy)]
enum VfStopFsmState {
    /// Initial state
    Init,

    /// Waiting for IPC channel
    WaitForIpcChannel,

    /// Waiting for IPC response
    WaitForIpcResponse,

    /// Wait for deferred HSM IPC response
    WaitIpcDeferredResponse,

    /// Final
    Final,
}

/// VF Stop command FSM
pub(crate) struct AdminVfStopCmd<E: AdminEnvTrait + 'static> {
    /// Current state of the FSM
    state: VfStopFsmState,

    /// Source PCIe Function
    src_pfn: PcieFunction,

    /// PCIe Function to be used for VF Stop
    pfn: PcieFunction,

    /// IPC channel to the Function Process
    fp_ipc_channel: Option<CmdResourceRef<AdminToFpIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// IPC channel to the HSM
    hsm_ipc_channel: Option<CmdResourceRef<HsmIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// Admin SQE
    sqe: AdminSqe,

    /// Admin CQE
    cqe: Option<AdminCqe>,

    /// List of submission queues to restore
    sq_list: Vec<(HostSqId, DevSqId, DevCqId)>,

    /// Context
    ctx: AdminFsmContext<E>,
}

impl<E: AdminEnvTrait> AdminCmdTrait<E> for AdminVfStopCmd<E> {
    /// Get the response buffer
    fn response(&mut self) -> (Option<AdminCqe>, Option<DmaBuffer<E>>) {
        (self.cqe.take(), None)
    }

    /// Handle an event
    fn on_event(&mut self, event: AdminFsmEvent, tag: TagId) -> Result<(), AdminErr> {
        match (self.state, event) {
            (VfStopFsmState::Init, AdminFsmEvent::StartCmd) => self.on_start(tag),
            (VfStopFsmState::WaitForIpcChannel, AdminFsmEvent::ResourceReady(resource)) => {
                self.on_ipc_channel_ready(tag, resource)
            }
            (VfStopFsmState::WaitForIpcResponse, AdminFsmEvent::FpToAdminIpcResponse) => {
                self.on_fp_ipc_response(tag)
            }
            (VfStopFsmState::WaitForIpcResponse, AdminFsmEvent::HsmIpcResponse) => {
                self.on_hsm_ipc_response(tag)
            }
            (VfStopFsmState::WaitIpcDeferredResponse, AdminFsmEvent::IoCancellationComplete) => {
                self.on_deferred_queue_deletion(tag)
            }
            (_, _) => self.on_unexpected_event(),
        }
    }

    /// Acquire a resource
    fn acquire_resource(&mut self, tag: TagId, res_id: ResId) -> AdminFsmEvent {
        match res_id {
            AdminFsmResourceId::AdminToFpIpcChannel => {
                self.fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
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

impl<E: AdminEnvTrait> AdminVfStopCmd<E> {
    /// Create a new Admin Create Submission Queue command FSM
    pub fn new(src_pfn: PcieFunction, ctx: AdminFsmContext<E>, sqe: AdminSqe) -> Self {
        Self {
            state: VfStopFsmState::Init,
            src_pfn,
            pfn: PcieFunction::Pf, // Default to PF, will be updated later
            fp_ipc_channel: None,
            hsm_ipc_channel: None,
            sqe,
            cqe: None,
            sq_list: Vec::new(),
            ctx,
        }
    }

    // On start event handler
    fn on_start(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.pfn = self
            .validate_and_decode_sqe()
            .or_else(|err| self.prepare_err_cqe(err))?;

        info!("VF Stop for PCIe function: {:?}", self.pfn.0 as u32);

        self.sq_list.clear();

        self.sq_list = self
            .ctx
            .function_mgr()
            .function(self.pfn)
            .get_enabled_sq_info();

        // There are submission queues to restore, so we need to acquire IPC channels
        // for both FP and HSM cores as we need both to recreate the submission queues
        self.fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
        self.hsm_ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());

        match (self.fp_ipc_channel.as_ref(), self.hsm_ipc_channel.as_ref()) {
            (Some(_), Some(_)) => self.disable_submission_queues(tag),
            (_, _) => {
                self.state = VfStopFsmState::WaitForIpcChannel;

                Err(AdminErr::Pending)
            }
        }
    }

    /// On IPC channel ready event
    fn on_ipc_channel_ready(
        &mut self,
        tag: TagId,
        res_id: AdminFsmResourceId,
    ) -> Result<(), AdminErr> {
        // Validate if the resource acquisition is successful
        match res_id {
            AdminFsmResourceId::AdminToFpIpcChannel => {
                if self.fp_ipc_channel.is_none() {
                    self.prepare_err_cqe(AdminErr::InvalidEvent)?
                }
            }
            AdminFsmResourceId::HsmIpcChannel => {
                if self.hsm_ipc_channel.is_none() {
                    self.prepare_err_cqe(AdminErr::InvalidEvent)?
                }
            }
            _ => self.prepare_err_cqe(AdminErr::InvalidResourceId)?,
        }

        match (self.fp_ipc_channel.as_ref(), self.hsm_ipc_channel.as_ref()) {
            (Some(_), Some(_)) => self.disable_submission_queues(tag),
            (_, _) => {
                self.state = VfStopFsmState::WaitForIpcChannel;

                Err(AdminErr::Pending)
            }
        }
    }

    /// On IPC channel response event from HSM core
    fn on_hsm_ipc_response(&mut self, tag: TagId) -> Result<(), AdminErr> {
        let message = self
            .hsm_ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()));

        self.process_ipc_response(message).or_else(|err| {
            if err == AdminErr::Pending {
                self.state = VfStopFsmState::WaitIpcDeferredResponse;

                Err(err)?
            } else {
                self.prepare_err_cqe(err)
            }
        })?;

        self.disable_submission_queues(tag)
    }

    /// On IPC channel response event from FP core
    fn on_fp_ipc_response(&mut self, tag: TagId) -> Result<(), AdminErr> {
        let message = self
            .fp_ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()));

        self.process_ipc_response(message)
            .or_else(|err| self.prepare_err_cqe(err))?;

        self.disable_submission_queues(tag)
    }

    /// On Deferred IO submission queue delete
    fn on_deferred_queue_deletion(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.validate_deferred_completion(tag)
            .or_else(|err| self.prepare_err_cqe(err))?;

        self.disable_submission_queues(tag)
    }

    /// Send submission queues disable notification to FP and HSM cores
    fn disable_submission_queues(&mut self, tag: TagId) -> Result<(), AdminErr> {
        if let Some((host_sq, dev_sq, dev_cq)) = self.sq_list.pop() {
            let mut message = IpcMessageCreateDeleteSq {
                info: SqCreateDeleteInfo {
                    pfn: self.pfn,
                    device_sq_id: dev_sq,
                    device_cq_id: dev_cq,
                    action: SqAction::Delete,
                },
                ..Default::default()
            };

            message.header.set_tag(tag.into());

            self.ctx
                .function_mgr()
                .function(self.pfn)
                .disable_sq(dev_sq, host_sq);

            if let HostQueueType::Hsm = HostQueueType::from(host_sq) {
                // If the queue is HSM, we send the IPC message to HSM core
                self.hsm_ipc_channel
                    .as_ref()
                    .unwrap()
                    .map(|c| c.send_request(tag, message.encode()))
                    .or_else(|_| {
                        self.ctx
                            .function_mgr()
                            .function(self.pfn)
                            .enable_sq(dev_sq, host_sq);

                        self.prepare_err_cqe(AdminErr::IpcSendRequestError)
                    })?;
            } else {
                // Otherwise, we send it to FP core
                self.fp_ipc_channel
                    .as_ref()
                    .unwrap()
                    .map(|c| c.send_request(tag, message.encode()))
                    .or_else(|_| {
                        self.ctx
                            .function_mgr()
                            .function(self.pfn)
                            .enable_sq(dev_sq, host_sq);

                        self.prepare_err_cqe(AdminErr::IpcSendRequestError)
                    })?;
            }

            self.state = VfStopFsmState::WaitForIpcResponse;

            Err(AdminErr::Pending)
        } else {
            // All the submission queues have been disabled
            self.prepare_cqe();

            self.state = VfStopFsmState::Final;

            Ok(())
        }
    }

    /// Process the IPC response from HSM core or FP core
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

        Ok(())
    }

    /// On unexpected event
    fn on_unexpected_event(&mut self) -> Result<(), AdminErr> {
        error!("[Vf_Stop] Unsupported event, expected StartCmd");

        Err(AdminErr::Pending)
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

        self.state = VfStopFsmState::Final;

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
    fn validate_and_decode_sqe(&mut self) -> Result<PcieFunction, AdminErr> {
        if self.src_pfn != PcieFunction::Pf {
            return self.prepare_err_cqe(AdminErr::InvalidSourcePfn);
        }

        let sqe: VfStopSqe = self.sqe.into();

        // Controller ID to be used for VF Save
        let cntrl_id: QueueCntrlId = sqe
            .cntrl_id
            .try_into()
            .map_err(|_| AdminErr::InvalidCntrlIdFieldInSqe)?;

        // PCIe function to be used for VF Restore
        let func: PcieFunction = cntrl_id.into();

        if func == PcieFunction::Pf {
            Err(AdminErr::InvalidPcieFn)?
        }

        Ok(func)
    }

    /// Validate deferred queue delete response from HSM IO core
    fn validate_deferred_completion(&mut self, tag: TagId) -> Result<(), AdminErr> {
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

        Ok(())
    }
}
