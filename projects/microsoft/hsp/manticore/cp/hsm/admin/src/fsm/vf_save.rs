// Copyright (c) Microsoft Corporation. All rights reserved.

use function::{FunctionMgrTrait, FunctionTrait};
use mcr_queue_controller::QueueCntrlId;

use super::types::AdminSqe;
use super::*;
use crate::error::HostStatusCode;

/// VF Save command FSM
pub(crate) struct AdminVfSaveCmd<E: AdminEnvTrait + 'static> {
    /// Source PCIe Function
    src_pfn: PcieFunction,

    /// PCIe Function to be used for VF Save
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

impl<E: AdminEnvTrait> AdminCmdTrait<E> for AdminVfSaveCmd<E> {
    /// Get the response buffer
    fn response(&mut self) -> (Option<AdminCqe>, Option<DmaBuffer<E>>) {
        (self.cqe.take(), self.dma_buf.take())
    }

    /// Handle an event
    fn on_event(&mut self, event: AdminFsmEvent, _tag: TagId) -> Result<(), AdminErr> {
        match event {
            AdminFsmEvent::StartCmd => self.on_start(),
            _ => self.on_unexpected_event(),
        }
    }
}

impl<E: AdminEnvTrait> AdminVfSaveCmd<E> {
    /// Create a new Admin Unsupported command FSM
    pub fn new(src_pfn: PcieFunction, ctx: AdminFsmContext<E>, sqe: AdminSqe) -> Self {
        Self {
            src_pfn,
            pfn: PcieFunction::Pf, // Default to Physical Function
            sqe,
            cqe: None,
            dma_buf: None,
            ctx,
        }
    }

    /// Handle the start event
    fn on_start(&mut self) -> Result<(), AdminErr> {
        // Replace the PcieFunction with the one from SQE for targeted VF
        self.pfn = self
            .validate_and_decode_sqe()
            .or_else(|err| self.prepare_err_cqe(err))?;

        info!("VF Save for PCIe function: {:?}", self.pfn.0 as u32);

        self.save_lm_context()
    }

    /// Save the Live Migration context
    fn save_lm_context(&mut self) -> Result<(), AdminErr> {
        let buf = self
            .ctx
            .dma_heap()
            .allocate(core::mem::size_of::<VmLiveMigrationInfo>())
            .ok_or(AdminErr::NoMemory)
            .or_else(|err| self.prepare_err_cqe(err))?;

        let lm_info: &'static mut [VmLiveMigrationInfo] =
            mcr_mem_map::mem_addr_to_slice(buf.as_ref().as_ptr() as usize, 1);

        let part_persistent_store = self.ctx.hsm_part_persistent_store_addr(self.pfn.into());

        self.ctx.function_mgr().function(self.pfn).save_lm_context(
            &mut lm_info[0],
            part_persistent_store.session_table[0],
            &part_persistent_store.masked_bk_boot,
            &part_persistent_store.sealed_bk3,
        );

        self.dma_buf = Some(buf);

        self.prepare_cqe();

        // This is a good time to disable the controller and this will trigger the controller
        // disable sequence though cntrl FSM after the completion of this FSM.
        self.ctx.function_mgr().function(self.pfn).clear_enable();

        Ok(())
    }

    /// On unexpected event
    fn on_unexpected_event(&self) -> Result<(), AdminErr> {
        error!("[Vf Save] Unsupported event");

        Err(AdminErr::Pending)
    }

    /// Prepare the CQE
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

    /// Validate and decode the SQE
    fn validate_and_decode_sqe(&mut self) -> Result<PcieFunction, AdminErr> {
        if self.src_pfn != PcieFunction::Pf {
            return self.prepare_err_cqe(AdminErr::InvalidSourcePfn);
        }

        let sqe: VfSaveSqe = self.sqe.into();

        // Controller ID to be used for VF Save
        let cntrl_id: QueueCntrlId = sqe
            .cntrl_id
            .try_into()
            .map_err(|_| AdminErr::InvalidCntrlIdFieldInSqe)?;

        // PCIe function to be used for VF save
        let func: PcieFunction = cntrl_id.into();

        // VF save is not supported on Physical function
        if func == PcieFunction::Pf {
            Err(AdminErr::InvalidPcieFn)?
        }

        Ok(func)
    }
}
