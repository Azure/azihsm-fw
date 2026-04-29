// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::vec::Vec;

use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_msix_controller::MsixControllerTrait;
use mcr_queue_controller::QueueCntrlId;

use crate::function::*;

use super::*;

/// VF Start FSM states
#[derive(Copy, Clone, Debug)]
enum VfStartFsmState {
    /// Controller FSM is idle
    Init,

    /// Waiting for IPC channel resource to be ready
    WaitForIpcChannel,

    /// Waiting for IPC channel response from either FP or HSM core
    WaitForIpcResponse,

    /// Final state
    Final,
}

/// VF Start command FSM to support Live Migration
pub(crate) struct AdminVfStartCmd<E: AdminEnvTrait + 'static> {
    /// FSM state
    state: VfStartFsmState,

    /// Admin SQE
    sqe: AdminSqe,

    /// Admin CQE
    cqe: Option<AdminCqe>,

    /// Source PCIe Function
    src_pfn: PcieFunction,

    /// PCIe Function to be used for VF Save
    pfn: PcieFunction,

    /// Fastpath IPC channel resource
    fp_ipc_channel: Option<CmdResourceRef<AdminToFpIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    // HSM IPC channel resource
    hsm_ipc_channel: Option<CmdResourceRef<HsmIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// List of submission queues to restore
    sq_list: Vec<(HostSqId, DevSqId, DevCqId)>,

    /// Context
    ctx: AdminFsmContext<E>,
}

impl<E: AdminEnvTrait> AdminCmdTrait<E> for AdminVfStartCmd<E> {
    /// Get the response buffer
    fn response(&mut self) -> (Option<AdminCqe>, Option<DmaBuffer<E>>) {
        (self.cqe.take(), None)
    }

    /// Handle an event
    fn on_event(&mut self, event: AdminFsmEvent, tag: TagId) -> Result<(), AdminErr> {
        match (self.state, event) {
            (VfStartFsmState::Init, AdminFsmEvent::StartCmd) => self.on_start(tag),
            (VfStartFsmState::WaitForIpcChannel, AdminFsmEvent::ResourceReady(res_id)) => {
                self.on_ipc_channel_ready(tag, res_id)
            }
            (VfStartFsmState::WaitForIpcResponse, AdminFsmEvent::FpToAdminIpcResponse)
            | (VfStartFsmState::WaitForIpcResponse, AdminFsmEvent::HsmIpcResponse) => {
                self.op_ipc_channel_response(tag, event)
            }
            _ => self.on_unexpected_event(),
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

impl<E: AdminEnvTrait> AdminVfStartCmd<E> {
    /// Create a new Admin Create Submission Queue command FSM
    pub fn new(src_pfn: PcieFunction, ctx: AdminFsmContext<E>, sqe: AdminSqe) -> Self {
        Self {
            state: VfStartFsmState::Init,
            sqe,
            cqe: None,
            src_pfn,
            pfn: PcieFunction::Pf, // Placeholder, will be set in validate_and_decode_sqe
            fp_ipc_channel: None,
            hsm_ipc_channel: None,
            sq_list: Vec::new(),
            ctx,
        }
    }

    // On start event handler
    fn on_start(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.pfn = self
            .validate_and_decode_sqe()
            .or_else(|err| self.prepare_err_cqe(err))?;

        info!("VF Start for PCIe function: {:?}", self.pfn.0 as u32);

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
            (Some(_), Some(_)) => self.recreate_submission_queues(tag),
            (_, _) => {
                self.state = VfStartFsmState::WaitForIpcChannel;

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
            (Some(_), Some(_)) => self.recreate_submission_queues(tag),
            (_, _) => {
                self.state = VfStartFsmState::WaitForIpcChannel;

                Err(AdminErr::Pending)
            }
        }
    }

    /// On IPC channel response event
    fn op_ipc_channel_response(
        &mut self,
        tag: TagId,
        event: AdminFsmEvent,
    ) -> Result<(), AdminErr> {
        match event {
            AdminFsmEvent::FpToAdminIpcResponse => self.on_sq_recreation_ipc_response_from_fp(tag),
            AdminFsmEvent::HsmIpcResponse => self.on_sq_recreation_ipc_response_from_hsm(tag),
            _ => self.prepare_err_cqe(AdminErr::InvalidEvent),
        }
    }

    /// On SQ recreation IPC response event from FP core
    fn on_sq_recreation_ipc_response_from_fp(&mut self, tag: TagId) -> Result<(), AdminErr> {
        let message = self
            .fp_ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()));

        self.process_ipc_response(message)
            .or_else(|err| self.prepare_err_cqe(err))?;

        self.recreate_submission_queues(tag)
    }

    /// On SQ recreation IPC response event from HSM core
    fn on_sq_recreation_ipc_response_from_hsm(&mut self, tag: TagId) -> Result<(), AdminErr> {
        let message = self
            .hsm_ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()));

        self.process_ipc_response(message)
            .or_else(|err| self.prepare_err_cqe(err))?;

        self.recreate_submission_queues(tag)
    }

    /// Handle the recreation of submission queues
    fn recreate_submission_queues(&mut self, tag: TagId) -> Result<(), AdminErr> {
        if let Some((host_sq, dev_sq, dev_cq)) = self.sq_list.pop() {
            let message = IpcMessageCreateDeleteSq {
                info: SqCreateDeleteInfo {
                    pfn: self.pfn,
                    device_sq_id: dev_sq,
                    device_cq_id: dev_cq,
                    action: SqAction::Create,
                },
                ..Default::default()
            };

            self.ctx
                .function_mgr()
                .function(self.pfn)
                .enable_sq(dev_sq, host_sq);

            if let HostQueueType::Hsm = HostQueueType::from(host_sq) {
                // If the queue is HSM, we send the IPC message to HSM core
                self.hsm_ipc_channel
                    .as_ref()
                    .unwrap()
                    .map(|c| c.send_request(tag, message.encode()))
                    .or_else(|_| self.prepare_err_cqe(AdminErr::IpcSendRequestError))?;
            } else {
                // Otherwise, we send it to FP core
                self.fp_ipc_channel
                    .as_ref()
                    .unwrap()
                    .map(|c| c.send_request(tag, message.encode()))
                    .or_else(|_| self.prepare_err_cqe(AdminErr::IpcSendRequestError))?;
            }

            self.state = VfStartFsmState::WaitForIpcResponse;

            Err(AdminErr::Pending)
        } else {
            // All the submission queues have been recreated, so we can complete the live migration
            self.prepare_cqe();

            self.ctx.msix_cntrl().enable_pcie_fn(self.pfn);

            self.state = VfStartFsmState::Final;

            Ok(())
        }
    }

    /// Process the IPC response from HSM core or FP core
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

        Ok(())
    }

    /// On unexpectd event
    fn on_unexpected_event(&mut self) -> Result<(), AdminErr> {
        error!("[Vf_Start] Unsupported event, expected StartCmd");

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

        let sqe: VfStartSqe = self.sqe.into();

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
}
