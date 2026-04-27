// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_logging::*;
use mcr_types::PcieFunction;

use super::types::AdminSqe;
use super::*;
use crate::context::AdminFsmContext;
use crate::error::HostStatusCode;
use crate::function::*;

/// Delete Completion Queue command
pub(crate) struct AdminDeleteCqCmd<E: AdminEnvTrait + 'static> {
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

impl<E: AdminEnvTrait> AdminCmdTrait<E> for AdminDeleteCqCmd<E> {
    /// Get the response buffer
    fn response(&mut self) -> (Option<AdminCqe>, Option<DmaBuffer<E>>) {
        (self.cqe.take(), self.dma_buf.take())
    }

    /// Handle an event
    fn on_event(&mut self, event: AdminFsmEvent, _tag: TagId) -> Result<(), AdminErr> {
        match event {
            AdminFsmEvent::StartCmd => self.on_start(),
            _ => Err(AdminErr::Pending),
        }
    }
}

impl<E: AdminEnvTrait> AdminDeleteCqCmd<E> {
    /// Create a new Admin Delete Completion Queue command FSM
    pub fn new(
        ctx: AdminFsmContext<E>,
        pfn: PcieFunction,
        sqe: AdminSqe,
        request: Option<DmaBuffer<E>>,
    ) -> Self {
        Self {
            ctx,
            pfn,
            sqe,
            cqe: None,
            dma_buf: request,
        }
    }
    /// On start event of this command FSM
    fn on_start(&mut self) -> Result<(), AdminErr> {
        // Validate and decode the SQE
        let delete_cq_sqe = self.validate_and_decode_sqe().or_else(|err| {
            error!("[delete_cq] Invalid SQE: {:?}", err as u32);
            self.prepare_err_cqe(err)
        })?;

        self.delete_device_queue(delete_cq_sqe).or_else(|err| {
            error!("Delete Cq Failed {:?}", err as u32);
            self.prepare_err_cqe(err)
        })?;

        self.prepare_cqe();

        Ok(())
    }

    /// Create device completion queue
    fn delete_device_queue(&mut self, delete_cq_sqe: DeleteCqSqe) -> Result<(), AdminErr> {
        self.ctx
            .function_mgr()
            .function(self.pfn)
            .delete_cq(delete_cq_sqe.queue_id)
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
    fn validate_and_decode_sqe(&mut self) -> Result<DeleteCqSqe, AdminErr> {
        let delete_cq_sqe: DeleteCqSqe = self.sqe.into();

        if let HostQueueType::Admin = HostQueueType::from(delete_cq_sqe.queue_id) {
            return Err(AdminErr::InvalidQueueId);
        }

        Ok(delete_cq_sqe)
    }
}
