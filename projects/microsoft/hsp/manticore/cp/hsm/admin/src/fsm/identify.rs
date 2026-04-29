// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_soc::SocInfoTrait;
use mcr_types::PcieFunction;
use zerocopy::IntoBytes;

use super::types::AdminSqe;
use super::*;
use crate::error::HostStatusCode;
use crate::function::FunctionMgrTrait;
use crate::function::FunctionTrait;

/// Identify command
pub(crate) struct AdminIdentifyCmd<E: AdminEnvTrait + 'static> {
    /// Pcie Function
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

impl<E: AdminEnvTrait> AdminCmdTrait<E> for AdminIdentifyCmd<E> {
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

impl<E: AdminEnvTrait> AdminIdentifyCmd<E> {
    /// Create a new Admin Identify command FSM
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
        let cntrl_id = self.ctx.function_mgr().function(self.pfn).cntrl_id();

        let mut buf = self
            .ctx
            .dma_heap()
            .allocate(core::mem::size_of::<McrCntrlIdentify>())
            .ok_or(AdminErr::NoMemory)
            .or_else(|err| self.prepare_err_cqe(err))?;

        let soc_info = self.ctx.soc_info();

        buf.as_ref_mut().copy_from_slice(
            McrCntrlIdentify::new(
                cntrl_id.into(),
                soc_info.fw_version(),
                soc_info.id(),
                soc_info.svn(),
            )
            .as_bytes(),
        );

        self.dma_buf = Some(buf);
        self.prepare_cqe();

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
}
