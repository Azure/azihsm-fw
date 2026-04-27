// Copyright (c) Microsoft Corporation. All rights reserved.

use core::marker::PhantomData;

use super::types::AdminSqe;
use super::*;
use crate::error::HostStatusCode;

/// Unsupported command
pub(crate) struct AdminUnsupportedCmd<E: AdminEnvTrait + 'static> {
    /// Admin SQE
    sqe: AdminSqe,

    /// Admin CQE
    cqe: Option<AdminCqe>,

    /// Phantom data
    marker: PhantomData<E>,
}

impl<E: AdminEnvTrait> AdminCmdTrait<E> for AdminUnsupportedCmd<E> {
    /// Get the response buffer
    fn response(&mut self) -> (Option<AdminCqe>, Option<DmaBuffer<E>>) {
        (self.cqe.take(), None)
    }

    /// Handle an event
    fn on_event(&mut self, event: AdminFsmEvent, _tag: TagId) -> Result<(), AdminErr> {
        match event {
            AdminFsmEvent::StartCmd => self.on_start(),
            _ => Err(AdminErr::Pending),
        }
    }
}

impl<E: AdminEnvTrait> AdminUnsupportedCmd<E> {
    /// Create a new Admin Unsupported command FSM
    pub fn new(sqe: AdminSqe) -> Self {
        Self {
            sqe,
            cqe: None,
            marker: Default::default(),
        }
    }

    /// Handle the start event
    fn on_start(&mut self) -> Result<(), AdminErr> {
        self.prepare_cqe();

        Ok(())
    }

    /// Prepare the CQE
    fn prepare_cqe(&mut self) {
        let cqe = AdminCqe {
            command_specific: 0,
            _rsvd: 0,
            sq_head: 0,
            sq_id: 0,
            cmd_id: self.sqe.cmd.id,
            psf: StatusField::new().with_status(HostStatusCode::InvalidCommandOpCode),
        };
        self.cqe = Some(cqe);
    }
}
