// Copyright (c) Microsoft Corporation. All rights reserved.

use super::helper::*;
use crate::error::{AdminErr, HostStatusCode};
use crate::fsm::vf_prep::AdminVfPrepCmd;
use crate::fsm::AdminCmdTrait;
use crate::mock::MockAdminEnvTrait;
use crate::AdminFsmEvent;
use mcr_types::*;

/// VF prepare command test configurations
#[derive(Default)]
pub(crate) struct VfPrepareCmdTestConfigs {
    /// Opcode for VF Prepare command
    pub opcode: u8,

    /// Controller Identified as populated by the device driver in the SQE
    pub cntrl_id: u32,
}

#[test]
fn test_vf_prep_unknown_event() {
    let test_configs = VfPrepareCmdTestConfigs {
        opcode: 0xC5,
        cntrl_id: 1,
    };
    let (_context, sqe) = make_fsm_for_vf_prepare(test_configs);

    // VF Prepare command FSM
    let mut fsm: AdminVfPrepCmd<MockAdminEnvTrait> = AdminVfPrepCmd::new(PcieFunction::Pf, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_vf_prep_normal_completion() {
    let test_configs = VfPrepareCmdTestConfigs {
        opcode: 0xC5,
        cntrl_id: 1,
    };
    let (_context, sqe) = make_fsm_for_vf_prepare(test_configs);

    // VF Prepare command FSM
    let mut fsm: AdminVfPrepCmd<MockAdminEnvTrait> = AdminVfPrepCmd::new(PcieFunction::Pf, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(fsm.on_event(AdminFsmEvent::StartCmd, 0xFF), Ok(()));

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_none());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::Success);
}

#[test]
fn test_vf_prep_invalid_src_pfn() {
    let test_configs = VfPrepareCmdTestConfigs {
        opcode: 0xC5,
        cntrl_id: 200,
    };
    let (_context, sqe) = make_fsm_for_vf_prepare(test_configs);

    // VF Prepare command FSM
    let mut fsm: AdminVfPrepCmd<MockAdminEnvTrait> = AdminVfPrepCmd::new(PcieFunction::Vf0, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidSourcePfn)
    );

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_none());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::InvalidFieldInCommand);
}

#[test]
fn test_vf_prep_invalid_controller_id() {
    let test_configs = VfPrepareCmdTestConfigs {
        opcode: 0xC5,
        cntrl_id: 200,
    };
    let (_context, sqe) = make_fsm_for_vf_prepare(test_configs);

    // VF Prepare command FSM
    let mut fsm: AdminVfPrepCmd<MockAdminEnvTrait> = AdminVfPrepCmd::new(PcieFunction::Pf, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidCntrlIdFieldInSqe)
    );

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_none());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::InvalidFieldInCommand);
}

#[test]
fn test_vf_prep_invalid_request_on_pf() {
    let test_configs = VfPrepareCmdTestConfigs {
        opcode: 0xC5,
        cntrl_id: 0,
    };
    let (_context, sqe) = make_fsm_for_vf_prepare(test_configs);

    // VF Prepare command FSM
    let mut fsm: AdminVfPrepCmd<MockAdminEnvTrait> = AdminVfPrepCmd::new(PcieFunction::Pf, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidPcieFn)
    );

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_none());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::InternalError);
}
