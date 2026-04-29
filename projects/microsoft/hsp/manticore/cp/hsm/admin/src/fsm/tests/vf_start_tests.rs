// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::PcieFunction;

use super::helper::*;
use crate::error::{AdminErr, HostStatusCode};
use crate::fsm::vf_start::AdminVfStartCmd;
use crate::fsm::AdminCmdTrait;
use crate::mock::MockAdminEnvTrait;
use crate::AdminFsmEvent;

/// VF start command test configurations
#[derive(Default)]
pub(crate) struct VfStartCmdTestConfigs {
    /// Unknown event
    pub unknown_event: bool,

    /// Opcode for VF Restore command
    pub opcode: u8,

    /// Controller Identified as populated by the device driver in the SQE
    pub cntrl_id: u32,

    /// Number of enabled resource groups
    pub num_res_grps: usize,

    /// Send FP IPC request failed
    pub fp_ipc_send_req_failure: bool,

    /// Send HSM IPC request failed
    pub hsm_ipc_send_req_failure: bool,

    /// None FD IPC response
    pub none_fp_ipc_resp: bool,

    /// None HSM IPC response
    pub none_hsm_ipc_resp: bool,

    /// Invalid FP IPC response error
    pub invalid_fp_ipc_resp: bool,

    /// Invalid HSM IPC response error
    pub invalid_hsm_ipc_resp: bool,
}

#[test]
fn test_vf_start_unknown_event() {
    let test_configs = VfStartCmdTestConfigs {
        opcode: 0xC7,
        cntrl_id: 1,
        unknown_event: true,
        ..Default::default()
    };
    let (ctx, sqe) = make_fsm_for_vf_start(test_configs);

    // VF stop command FSM
    let mut fsm: AdminVfStartCmd<MockAdminEnvTrait> =
        AdminVfStartCmd::new(PcieFunction::Pf, ctx, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_vf_start_invalid_src_pfn() {
    let test_configs = VfStartCmdTestConfigs {
        opcode: 0xC7,
        cntrl_id: 200,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_start(test_configs);

    // VF stop command FSM
    let mut fsm: AdminVfStartCmd<MockAdminEnvTrait> =
        AdminVfStartCmd::new(PcieFunction::Vf11, ctx, sqe);

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
fn test_vf_start_invalid_controller_id() {
    let test_configs = VfStartCmdTestConfigs {
        opcode: 0xC7,
        cntrl_id: 200,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_start(test_configs);

    // VF stop command FSM
    let mut fsm: AdminVfStartCmd<MockAdminEnvTrait> =
        AdminVfStartCmd::new(PcieFunction::Pf, ctx, sqe);

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
fn test_vf_start_invalid_request_on_pf() {
    let test_configs = VfStartCmdTestConfigs {
        opcode: 0xC7,
        cntrl_id: 0,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_start(test_configs);

    // VF save command FSM
    let mut fsm: AdminVfStartCmd<MockAdminEnvTrait> =
        AdminVfStartCmd::new(PcieFunction::Pf, ctx, sqe);

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
fn test_vf_start_normal_flow() {
    let test_configs = VfStartCmdTestConfigs {
        opcode: 0xC7,
        cntrl_id: 1,
        num_res_grps: 1,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_start(test_configs);

    // VF start command FSM
    let mut fsm: AdminVfStartCmd<MockAdminEnvTrait> =
        AdminVfStartCmd::new(PcieFunction::Pf, ctx, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF), Ok(()));

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_none());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::Success);
}

#[test]
fn test_vf_start_with_two_resource_groups_normal_flow() {
    let test_configs = VfStartCmdTestConfigs {
        opcode: 0xC7,
        cntrl_id: 1,
        num_res_grps: 2,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_start(test_configs);

    // VF start command FSM
    let mut fsm: AdminVfStartCmd<MockAdminEnvTrait> =
        AdminVfStartCmd::new(PcieFunction::Pf, ctx, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF), Ok(()));

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_none());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::Success);
}

#[test]
fn test_vf_start_fp_send_request_failure() {
    let test_configs = VfStartCmdTestConfigs {
        opcode: 0xC7,
        cntrl_id: 1,
        num_res_grps: 1,
        fp_ipc_send_req_failure: true,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_start(test_configs);

    // VF start command FSM
    let mut fsm: AdminVfStartCmd<MockAdminEnvTrait> =
        AdminVfStartCmd::new(PcieFunction::Pf, ctx, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::IpcSendRequestError)
    );

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_none());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::InternalError);
}

#[test]
fn test_vf_start_hsm_send_request_failure() {
    let test_configs = VfStartCmdTestConfigs {
        opcode: 0xC7,
        cntrl_id: 1,
        num_res_grps: 1,
        hsm_ipc_send_req_failure: true,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_start(test_configs);

    // VF start command FSM
    let mut fsm: AdminVfStartCmd<MockAdminEnvTrait> =
        AdminVfStartCmd::new(PcieFunction::Pf, ctx, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::IpcSendRequestError)
    );

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_none());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::InternalError);
}

#[test]
fn test_vf_start_fp_ipc_none_response() {
    let test_configs = VfStartCmdTestConfigs {
        opcode: 0xC7,
        cntrl_id: 1,
        num_res_grps: 1,
        none_fp_ipc_resp: true,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_start(test_configs);

    // VF start command FSM
    let mut fsm: AdminVfStartCmd<MockAdminEnvTrait> =
        AdminVfStartCmd::new(PcieFunction::Pf, ctx, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::SpuriousIpcMessage)
    );

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_none());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::InternalError);
}

#[test]
fn test_vf_start_hsm_ipc_none_response() {
    let test_configs = VfStartCmdTestConfigs {
        opcode: 0xC7,
        cntrl_id: 1,
        num_res_grps: 1,
        none_hsm_ipc_resp: true,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_start(test_configs);

    // VF start command FSM
    let mut fsm: AdminVfStartCmd<MockAdminEnvTrait> =
        AdminVfStartCmd::new(PcieFunction::Pf, ctx, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::SpuriousIpcMessage)
    );

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_none());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::InternalError);
}

#[test]
fn test_vf_start_fp_ipc_invalid_response() {
    let test_configs = VfStartCmdTestConfigs {
        opcode: 0xC7,
        cntrl_id: 1,
        num_res_grps: 1,
        invalid_fp_ipc_resp: true,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_start(test_configs);

    // VF start command FSM
    let mut fsm: AdminVfStartCmd<MockAdminEnvTrait> =
        AdminVfStartCmd::new(PcieFunction::Pf, ctx, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::IpcResponseError)
    );

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_none());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::InternalError);
}

#[test]
fn test_vf_start_hsm_ipc_invalid_response() {
    let test_configs = VfStartCmdTestConfigs {
        opcode: 0xC7,
        cntrl_id: 1,
        num_res_grps: 1,
        invalid_hsm_ipc_resp: true,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_start(test_configs);

    // VF start command FSM
    let mut fsm: AdminVfStartCmd<MockAdminEnvTrait> =
        AdminVfStartCmd::new(PcieFunction::Pf, ctx, sqe);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::FpToAdminIpcResponse, 0xFF),
        Err(AdminErr::Pending)
    );
    assert_eq!(
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::IpcResponseError)
    );

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_none());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::InternalError);
}
