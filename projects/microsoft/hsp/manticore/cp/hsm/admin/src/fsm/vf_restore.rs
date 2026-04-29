// Copyright (c) Microsoft Corporation. All rights reserved.

use function::*;
use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_msix_controller::MsixControllerTrait;
use mcr_queue_controller::QueueCntrlId;

use super::types::AdminSqe;
use super::*;
use crate::error::HostStatusCode;

/// VF Restore FSM states
#[derive(Copy, Clone, Debug)]
enum VfRestoreFsmState {
    /// Controller FSM is idle
    Init,

    /// Waiting for FP IPC channel resource to be ready
    WaitForIpcChannel,

    /// Waiting for IPC channel response from either FP or HSM core to process VF start
    WaitForIpcResponse,

    /// Final state
    Final,
}

/// VF Restore command FSM
pub(crate) struct AdminVfRestoreCmd<E: AdminEnvTrait + 'static> {
    /// FSM state
    state: VfRestoreFsmState,

    /// Source PCIe Function
    src_pfn: PcieFunction,

    /// PCIe Function to be used for VF Save
    pfn: PcieFunction,

    /// Fastpath IPC channel resource
    fp_ipc_channel: Option<CmdResourceRef<AdminToFpIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    // HSM IPC channel resource
    hsm_ipc_channel: Option<CmdResourceRef<HsmIpcChannel<E::IpcChannel>, AdminFsm<E>>>,

    /// Admin SQE
    sqe: AdminSqe,

    /// Admin CQE
    cqe: Option<AdminCqe>,

    /// DMA buffer
    dma_buf: DmaBuffer<E>,

    /// Context
    ctx: AdminFsmContext<E>,
}

impl<E: AdminEnvTrait> AdminCmdTrait<E> for AdminVfRestoreCmd<E> {
    /// Get the response buffer
    fn response(&mut self) -> (Option<AdminCqe>, Option<DmaBuffer<E>>) {
        (self.cqe.take(), None)
    }

    /// Handle an event
    fn on_event(&mut self, event: AdminFsmEvent, tag: TagId) -> Result<(), AdminErr> {
        match (self.state, event) {
            (VfRestoreFsmState::Init, AdminFsmEvent::StartCmd) => self.on_start(tag),
            (VfRestoreFsmState::WaitForIpcChannel, AdminFsmEvent::ResourceReady(res_id)) => {
                self.on_ipc_channel_ready(tag, res_id)
            }
            (VfRestoreFsmState::WaitForIpcResponse, AdminFsmEvent::HsmIpcResponse)
            | (VfRestoreFsmState::WaitForIpcResponse, AdminFsmEvent::FpToAdminIpcResponse) => {
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

impl<E: AdminEnvTrait> AdminVfRestoreCmd<E> {
    /// Create a new Admin Unsupported command FSM
    pub fn new(
        src_pfn: PcieFunction,
        ctx: AdminFsmContext<E>,
        sqe: AdminSqe,
        dma_buf: DmaBuffer<E>,
    ) -> Self {
        Self {
            state: VfRestoreFsmState::Init,
            src_pfn,
            pfn: PcieFunction::Pf, // Placeholder, will be set in validate_and_decode_sqe
            fp_ipc_channel: None,
            hsm_ipc_channel: None,
            sqe,
            cqe: None,
            dma_buf,
            ctx,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.pfn = self
            .validate_and_decode_sqe()
            .or_else(|err| self.prepare_err_cqe(err))?;

        info!("VF Restore for PCIe function: {:?}", self.pfn.0 as u32);

        self.try_send_fp_ipc_request(tag)
    }

    /// On IPC channel ready event
    fn on_ipc_channel_ready(&mut self, tag: TagId, resource: ResId) -> Result<(), AdminErr> {
        match resource {
            ResId::AdminToFpIpcChannel => self.send_vf_start_ipc_request_to_fp(tag),
            ResId::HsmIpcChannel => self.send_vf_start_ipc_request_to_hsm(tag),
            _ => self.prepare_err_cqe(AdminErr::InvalidResourceId),
        }
    }

    /// On IPC channel response event
    fn op_ipc_channel_response(
        &mut self,
        tag: TagId,
        event: AdminFsmEvent,
    ) -> Result<(), AdminErr> {
        match event {
            AdminFsmEvent::FpToAdminIpcResponse => self.on_vf_start_ipc_response_from_fp(tag),
            AdminFsmEvent::HsmIpcResponse => self.on_vf_start_ipc_response_from_hsm(tag),
            _ => self.prepare_err_cqe(AdminErr::InvalidEvent),
        }
    }

    /// Restore the live migration context
    fn restore_lm_context(&mut self) -> Result<(), AdminErr> {
        let lm_info: &'static mut [VmLiveMigrationInfo] =
            mcr_mem_map::mem_addr_to_slice(self.dma_buf.as_ref().as_ptr() as usize, 1);

        self.ctx
            .function_mgr()
            .function(self.pfn)
            .restore_lm_context(&mut lm_info[0])
            .or_else(|err| self.prepare_err_cqe(err))?;

        // Restore the session allocation and re-negotiation mask
        let part_persistent_store = self.ctx.hsm_part_persistent_store_addr(self.pfn.into());
        let session_mask = lm_info[0].session_allocation_mask;
        part_persistent_store.session_table[0] = session_mask;
        part_persistent_store.session_table[1] = session_mask;
        let session_table = [0u8; 16];
        part_persistent_store.session_table[2..].copy_from_slice(&session_table[..]);
        part_persistent_store.masked_bk_boot = lm_info[0].masked_bk_boot;
        part_persistent_store.sealed_bk3 = lm_info[0].sealed_bk3;

        Ok(())
    }

    /// Try acquiring the IPC channel resoruce for FP and send an IPC message
    fn try_send_fp_ipc_request(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.fp_ipc_channel = self.ctx.admin_to_fp_ipc_channel().acquire(tag, ());
        if self.fp_ipc_channel.is_none() {
            self.state = VfRestoreFsmState::WaitForIpcChannel;
            Err(AdminErr::Pending)?;
        }

        self.send_vf_start_ipc_request_to_fp(tag)
    }

    /// Send an IPC message to FP core to perform start VF operation
    fn send_vf_start_ipc_request_to_fp(&mut self, tag: TagId) -> Result<(), AdminErr> {
        let message = self.prepare_vf_start_ipc_message(tag);

        self.fp_ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, message.encode()))
            .or_else(|_| self.prepare_err_cqe(AdminErr::IpcSendRequestError))?;

        self.state = VfRestoreFsmState::WaitForIpcResponse;

        Err(AdminErr::Pending)
    }

    /// On IPC channel response event from FP core
    fn on_vf_start_ipc_response_from_fp(&mut self, tag: TagId) -> Result<(), AdminErr> {
        let message = self
            .fp_ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()));

        self.process_ipc_response(message)
            .or_else(|err| self.prepare_err_cqe(err))?;

        self.try_send_hsm_ipc_request(tag)
    }

    /// Try acquiring the IPC channel resoruce for HSM and send an IPC message
    fn try_send_hsm_ipc_request(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.hsm_ipc_channel = self.ctx.hsm_ipc_channel().acquire(tag, ());
        if self.hsm_ipc_channel.is_none() {
            self.state = VfRestoreFsmState::WaitForIpcChannel;
            Err(AdminErr::Pending)?;
        }

        self.send_vf_start_ipc_request_to_hsm(tag)
    }

    /// Send an IPC message to HSM core to perform start VF operation
    fn send_vf_start_ipc_request_to_hsm(&mut self, tag: u16) -> Result<(), AdminErr> {
        let message = self.prepare_vf_start_ipc_message(tag);

        self.hsm_ipc_channel
            .as_ref()
            .unwrap()
            .map(|c| c.send_request(tag, message.encode()))
            .or_else(|_| self.prepare_err_cqe(AdminErr::IpcSendRequestError))?;

        self.state = VfRestoreFsmState::WaitForIpcResponse;

        Err(AdminErr::Pending)
    }

    /// On IPC channel response event from HSM core
    fn on_vf_start_ipc_response_from_hsm(&mut self, _tag: TagId) -> Result<(), AdminErr> {
        let message = self
            .hsm_ipc_channel
            .as_ref()
            .and_then(|c| c.map(|c| c.receive_message()));

        self.process_ipc_response(message)
            .or_else(|err| self.prepare_err_cqe(err))?;

        // Set the controller state as both enabled and ready to avoid executing the controller
        // enable sequence again.
        self.ctx
            .function_mgr()
            .function(self.pfn)
            .complete_live_migration();

        self.ctx.msix_cntrl().enable_pcie_fn(self.pfn);

        self.restore_lm_context()
            .or_else(|err| self.prepare_err_cqe(err))?;

        self.prepare_cqe();
        self.state = VfRestoreFsmState::Final;

        Ok(())
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

    /// Prepare IPC message for Pcie Function enable action to FP and HSM
    fn prepare_vf_start_ipc_message(&self, tag: TagId) -> IpcMessagePfnEnableDisable {
        let mut message = IpcMessagePfnEnableDisable {
            info: PfnEnableDisableInfo {
                pfn: self.pfn,
                action: PfnEnableDisableAction::Enable,
            },
            ..Default::default()
        };

        message.header.set_tag(tag.into());

        message
    }

    /// On unexpected event
    fn on_unexpected_event(&self) -> Result<(), AdminErr> {
        error!("[Vf Restore] Unsupported event");

        Err(AdminErr::Pending)
    }

    /// Prepare the CQE
    fn prepare_cqe(&mut self) {
        self.cqe = Some(AdminCqe {
            command_specific: 0,
            _rsvd: 0,
            sq_head: 0,
            sq_id: 0,
            cmd_id: self.sqe.cmd.id,
            psf: StatusField::new().with_status(HostStatusCode::Success),
        })
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

        self.state = VfRestoreFsmState::Final;

        Err(err)
    }

    /// Validate and decode the SQE
    fn validate_and_decode_sqe(&mut self) -> Result<PcieFunction, AdminErr> {
        if self.src_pfn != PcieFunction::Pf {
            return self.prepare_err_cqe(AdminErr::InvalidSourcePfn);
        }

        let sqe: VfRestoreSqe = self.sqe.into();

        // Controller ID to be used for VF Save
        let cntrl_id: QueueCntrlId = sqe
            .cntrl_id
            .try_into()
            .map_err(|_| AdminErr::InvalidCntrlIdFieldInSqe)?;

        // PCIe function to be used for VF Restore
        let func: PcieFunction = cntrl_id.into();

        // VF restore is not supported on Physical function
        if func == PcieFunction::Pf {
            Err(AdminErr::InvalidPcieFn)?
        }

        Ok(func)
    }
}
