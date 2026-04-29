// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_logging::*;
use mcr_queue_controller::QueueCntrlId;
use mcr_types::PcieFunction;

use super::types::AdminSqe;
use super::*;
use crate::context::AdminFsmContext;
use crate::error::HostStatusCode;
use crate::function::*;

/// Get resource command
pub(crate) struct AdminGetResCmd<E: AdminEnvTrait + 'static> {
    /// Admin SQE
    sqe: AdminSqe,

    /// Admin CQE
    cqe: Option<AdminCqe>,

    /// DMA buffer
    dma_buf: Option<DmaBuffer<E>>,

    /// Context
    ctx: AdminFsmContext<E>,
}

impl<E: AdminEnvTrait> AdminCmdTrait<E> for AdminGetResCmd<E> {
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

impl<E: AdminEnvTrait> AdminGetResCmd<E> {
    /// Create a new Admin Get Resource command FSM
    pub fn new(
        ctx: AdminFsmContext<E>,
        _pfn: PcieFunction,
        sqe: AdminSqe,
        request: Option<DmaBuffer<E>>,
    ) -> Self {
        Self {
            ctx,
            sqe,
            cqe: None,
            dma_buf: request,
        }
    }
    /// On start event of this command FSM
    fn on_start(&mut self) -> Result<(), AdminErr> {
        // Validate and decode the SQE
        let func = self.validate_and_decode_sqe().or_else(|err| {
            error!("[get_res] Invalid SQE: {:?}", err as u32);
            self.prepare_err_cqe(err)
        })?;

        self.prepare_cqe(self.ctx.function_mgr().function(func).res_cnt());

        Ok(())
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
    fn validate_and_decode_sqe(&mut self) -> Result<PcieFunction, AdminErr> {
        let get_res_sqe: GetSetResourceSqe = self.sqe.into();

        let cntrl_id: QueueCntrlId = get_res_sqe
            .cntrl_id
            .try_into()
            .map_err(|_| AdminErr::InvalidCntrlIdFieldInSqe)?;
        let func: PcieFunction = cntrl_id.into();

        Ok(func)
    }
}
