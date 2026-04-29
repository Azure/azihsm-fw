// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::PcieFunction;

use super::helper::*;
use crate::error::{AdminErr, HostStatusCode};
use crate::fsm::vf_save::AdminVfSaveCmd;
use crate::fsm::AdminCmdTrait;
use crate::mock::MockAdminEnvTrait;
use crate::AdminFsmEvent;

/// VF prepare command test configurations
#[derive(Default)]
pub(crate) struct VfSaveCmdTestConfigs {
    /// Unknown event
    pub unknown_event: bool,

    /// Opcode for VF Save command
    pub opcode: u8,

    /// Controller Identified as populated by the device driver in the SQE
    pub cntrl_id: u32,

    /// DMA allocation failure
    pub dma_alloc_fail: bool,
}

#[test]
fn test_vf_save_unknown_event() {
    let test_configs = VfSaveCmdTestConfigs {
        opcode: 0xC8,
        cntrl_id: 1,
        unknown_event: true,
        ..Default::default()
    };
    let (ctx, sqe) = make_fsm_for_vf_save(test_configs);

    // VF save command FSM
    let mut fsm: AdminVfSaveCmd<MockAdminEnvTrait> =
        AdminVfSaveCmd::new(PcieFunction::Pf, ctx, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_vf_save_invalid_controller_id() {
    let test_configs = VfSaveCmdTestConfigs {
        opcode: 0xC8,
        cntrl_id: 200,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_save(test_configs);

    // VF save command FSM
    let mut fsm: AdminVfSaveCmd<MockAdminEnvTrait> =
        AdminVfSaveCmd::new(PcieFunction::Pf, ctx, sqe);

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
fn test_vf_save_invalid_src_pfn() {
    let test_configs = VfSaveCmdTestConfigs {
        opcode: 0xC8,
        cntrl_id: 200,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_save(test_configs);

    // VF save command FSM
    let mut fsm: AdminVfSaveCmd<MockAdminEnvTrait> =
        AdminVfSaveCmd::new(PcieFunction::Vf10, ctx, sqe);

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
fn test_vf_save_invalid_request_on_pf() {
    let test_configs = VfSaveCmdTestConfigs {
        opcode: 0xC8,
        cntrl_id: 0,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_save(test_configs);

    // VF save command FSM
    let mut fsm: AdminVfSaveCmd<MockAdminEnvTrait> =
        AdminVfSaveCmd::new(PcieFunction::Pf, ctx, sqe);

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

#[test]
fn test_vf_save_dma_alloc_fail() {
    let test_configs = VfSaveCmdTestConfigs {
        opcode: 0xC8,
        cntrl_id: 1,
        dma_alloc_fail: true,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_save(test_configs);

    // VF save command FSM
    let mut fsm: AdminVfSaveCmd<MockAdminEnvTrait> =
        AdminVfSaveCmd::new(PcieFunction::Pf, ctx, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::NoMemory)
    );

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_none());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::InternalError);
}

#[test]
fn test_vf_save_normal_flow() {
    let test_configs = VfSaveCmdTestConfigs {
        opcode: 0xC8,
        cntrl_id: 1,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_save(test_configs);

    // VF save command FSM
    let mut fsm: AdminVfSaveCmd<MockAdminEnvTrait> =
        AdminVfSaveCmd::new(PcieFunction::Pf, ctx, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(fsm.on_event(AdminFsmEvent::StartCmd, 0xFF), Ok(()));

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_some());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::Success);
}
