// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_logging::*;

use super::types::AdminSqe;
use super::*;
use crate::context::AdminFsmContext;
use crate::error::HostStatusCode;
use crate::function::*;

/// Get Features command
pub(crate) struct AdminGetFeaturesCmd<E: AdminEnvTrait + 'static> {
    /// Pcie function
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

impl<E: AdminEnvTrait> AdminCmdTrait<E> for AdminGetFeaturesCmd<E> {
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

impl<E: AdminEnvTrait> AdminGetFeaturesCmd<E> {
    /// Create a new Admin Get Features command FSM
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
        let num_res = self.ctx.function_mgr().function(self.pfn).res_cnt();
        if num_res == 0 {
            self.prepare_err_cqe(AdminErr::InvalidFieldInGetSetFeaturesCmd)?;
        }

        let get_features_sqe: GetFeaturesSqe = self.sqe.into();
        let cmd_specific_data = self
            .get_cmd_specific_data(get_features_sqe.id, num_res)
            .or_else(|err| {
                error!("[get_features] Invalid SQE: {:?}", err as u32);
                self.prepare_err_cqe(err)
            })?;

        self.prepare_cqe(cmd_specific_data);

        Ok(())
    }

    /// Get the command specific parameters for the given feature
    fn get_cmd_specific_data(
        &mut self,
        feature_id: AdminFeatureId,
        num_res: u32,
    ) -> Result<u32, AdminErr> {
        const NUM_FAST_PATH_QUEUES: u32 = 2;

        // Queues reported to host are 0 based so subtract by 1
        match feature_id {
            AdminFeatureId::NumberOfQueues => {
                let num_queues = num_res - 1;

                Ok((num_queues << 16) | num_queues)
            }
            AdminFeatureId::FpNumberOfQueues => {
                let num_queues = num_res * NUM_FAST_PATH_QUEUES - 1;

                Ok((num_queues << 16) | num_queues)
            }
            AdminFeatureId::RtFwCapabilities => {
                // Bit 0: Supports AES GCM WA
                let fw_cap = RtFwCapabilities::default().into();

                Ok(fw_cap)
            }
            _ => self.prepare_err_cqe(AdminErr::InvalidFeatureId),
        }
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
    fn prepare_cqe(&mut self, cmd_specific_data: u32) {
        let cqe = AdminCqe {
            command_specific: cmd_specific_data,
            _rsvd: 0,
            sq_head: 0,
            sq_id: 0,
            cmd_id: self.sqe.cmd.id,
            psf: StatusField::new().with_status(HostStatusCode::Success),
        };

        self.cqe = Some(cqe);
    }
}
