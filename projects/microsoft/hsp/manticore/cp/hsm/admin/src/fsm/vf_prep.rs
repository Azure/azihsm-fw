// Copyright (c) Microsoft Corporation. All rights reserved.

use core::marker::PhantomData;

use mcr_queue_controller::QueueCntrlId;

use super::types::AdminSqe;
use super::*;
use crate::error::HostStatusCode;

/// VF Prepare
pub(crate) struct AdminVfPrepCmd<E: AdminEnvTrait + 'static> {
    /// Source PCIe Function
    src_pfn: PcieFunction,

    /// Pcie Function
    pfn: PcieFunction,

    /// Admin SQE
    sqe: AdminSqe,

    /// Admin CQE
    cqe: Option<AdminCqe>,

    /// Phantom data
    marker: PhantomData<E>,
}

impl<E: AdminEnvTrait> AdminCmdTrait<E> for AdminVfPrepCmd<E> {
    /// Get the response buffer
    fn response(&mut self) -> (Option<AdminCqe>, Option<DmaBuffer<E>>) {
        (self.cqe.take(), None)
    }

    /// Handle an event
    fn on_event(&mut self, event: AdminFsmEvent, _tag: TagId) -> Result<(), AdminErr> {
        match event {
            AdminFsmEvent::StartCmd => self.on_start(),
            _ => self.on_unexpected_event(),
        }
    }
}

impl<E: AdminEnvTrait> AdminVfPrepCmd<E> {
    /// Create a new Admin Unsupported command FSM
    pub fn new(src_pfn: PcieFunction, sqe: AdminSqe) -> Self {
        Self {
            src_pfn,
            pfn: PcieFunction::Pf, // Default to PF, will be updated after decoding the SQE
            sqe,
            cqe: None,
            marker: Default::default(),
        }
    }

    /// Handle the start event
    fn on_start(&mut self) -> Result<(), AdminErr> {
        self.pfn = self
            .validate_and_decode_sqe()
            .or_else(|err| self.prepare_err_cqe(err))?;

        info!("VF Prepare for PCIe function: {:?}", self.pfn.0 as u32);

        self.prepare_cqe();

        Ok(())
    }

    /// On unexpected event
    fn on_unexpected_event(&self) -> Result<(), AdminErr> {
        error!("[Vf Prepare] Unsupported event");

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

        let sqe: VfPrepSqe = self.sqe.into();

        // Controller ID to be used for VF Save
        let cntrl_id: QueueCntrlId = sqe
            .cntrl_id
            .try_into()
            .map_err(|_| AdminErr::InvalidCntrlIdFieldInSqe)?;

        // PCIe function to be used for VF Prepare
        let func: PcieFunction = cntrl_id.into();

        if func == PcieFunction::Pf {
            self.prepare_err_cqe(AdminErr::InvalidPcieFn)?
        }

        Ok(func)
    }
}
