// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_queue_controller::QueueMem;
use mcr_types::PcieFunction;

use super::types::AdminSqe;
use super::*;
use crate::context::AdminFsmContext;
use crate::error::HostStatusCode;
use crate::function::*;

/// Create Completion Queue command
pub(crate) struct AdminCreateCqCmd<E: AdminEnvTrait + 'static> {
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

impl<E: AdminEnvTrait> AdminCmdTrait<E> for AdminCreateCqCmd<E> {
    /// Get the response buffer
    fn response(&mut self) -> (Option<AdminCqe>, Option<DmaBuffer<E>>) {
        (self.cqe.take(), self.dma_buf.take())
    }

    /// Handle an event
    fn on_event(&mut self, event: AdminFsmEvent, tag: TagId) -> Result<(), AdminErr> {
        match event {
            AdminFsmEvent::StartCmd => self.on_start(tag),
            _ => Err(AdminErr::Pending),
        }
    }
}

impl<E: AdminEnvTrait> AdminCreateCqCmd<E> {
    /// Create a new Admin Create Completion Queue command FSM
    pub fn new(
        ctx: AdminFsmContext<E>,
        pfn: PcieFunction,
        sqe: AdminSqe,
        request: Option<DmaBuffer<E>>,
    ) -> Self {
        Self {
            pfn,
            sqe,
            cqe: None,
            dma_buf: request,
            ctx,
        }
    }

    /// On start event of this command FSM
    fn on_start(&mut self, _tag: TagId) -> Result<(), AdminErr> {
        // Validate and decode the SQE
        let create_cq_sqe = self
            .validate_and_decode_sqe()
            .or_else(|err| self.prepare_err_cqe(err))?;

        self.create_device_queue(create_cq_sqe)
            .or_else(|err| self.prepare_err_cqe(err))?;

        self.prepare_cqe();

        Ok(())
    }

    /// Create device completion queue
    fn create_device_queue(&mut self, create_cq_sqe: CreateCqSqe) -> Result<(), AdminErr> {
        let mem = QueueMem {
            addr: create_cq_sqe.prp1,
            len: create_cq_sqe.queue_len.into(),
        };
        let irq: Option<u16> = if create_cq_sqe.attr.ien() {
            Some(create_cq_sqe.attr.iv())
        } else {
            None
        };

        self.ctx
            .function_mgr()
            .function(self.pfn)
            .create_cq(create_cq_sqe.queue_id, mem, irq)
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
    fn validate_and_decode_sqe(&mut self) -> Result<CreateCqSqe, AdminErr> {
        let create_cq_sqe: CreateCqSqe = self.sqe.into();
        const MIN_HSM_IVEC: u16 = 0x1;
        const MAX_HSM_IVEC: u16 = 0xF;
        const MAX_FP_IVEC: u16 = 0x1F;

        if !create_cq_sqe.attr.pc() {
            Err(AdminErr::InvalidFieldInCreateCqCmd)?
        }

        if create_cq_sqe.queue_len == 0 {
            Err(AdminErr::InvalidQueueSize)?
        }

        match create_cq_sqe.queue_id.into() {
            HostQueueType::Admin => return Err(AdminErr::InvalidQueueId),
            HostQueueType::Hsm => {
                if create_cq_sqe.attr.ien()
                    && (create_cq_sqe.attr.iv() < MIN_HSM_IVEC
                        || create_cq_sqe.attr.iv() > MAX_HSM_IVEC)
                {
                    return Err(AdminErr::InvalidInterruptVector);
                }
            }
            HostQueueType::Fp => {
                if create_cq_sqe.attr.ien()
                    && (create_cq_sqe.attr.iv() <= MAX_HSM_IVEC
                        || create_cq_sqe.attr.iv() > MAX_FP_IVEC)
                {
                    return Err(AdminErr::InvalidInterruptVector);
                }
            }
        }

        Ok(create_cq_sqe)
    }
}
