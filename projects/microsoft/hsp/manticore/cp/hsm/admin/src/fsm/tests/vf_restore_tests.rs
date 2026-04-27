// Copyright (c) Microsoft Corporation. All rights reserved.

use super::helper::*;
use crate::error::{AdminErr, HostStatusCode};
use crate::fsm::vf_restore::AdminVfRestoreCmd;
use crate::fsm::AdminCmdTrait;
use crate::mock::MockAdminEnvTrait;
use crate::AdminFsmEvent;
use mcr_alloc::DmaHeapTrait;
use mcr_types::*;

/// VF restore command test configurations
#[derive(Default)]
pub(crate) struct VfRestoreCmdTestConfigs {
    /// Unknown event
    pub unknown_event: bool,

    /// Opcode for VF Restore command
    pub opcode: u8,

    /// Controller Identified as populated by the device driver in the SQE
    pub cntrl_id: u32,

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

    /// Persistent store address
    pub persistent_store_addr: Option<usize>,
}

#[test]
fn test_vf_restore_unknown_event() {
    let test_configs = VfRestoreCmdTestConfigs {
        opcode: 0xC9,
        cntrl_id: 1,
        unknown_event: true,
        ..Default::default()
    };
    let (ctx, sqe) = make_fsm_for_vf_restore(test_configs);

    let dma_buffer = ctx
        .dma_heap()
        .allocate(core::mem::size_of::<VmLiveMigrationInfo>());

    assert!(dma_buffer.is_some());
    let dma_buffer = dma_buffer.unwrap();

    // VF Restore command FSM
    let mut fsm: AdminVfRestoreCmd<MockAdminEnvTrait> =
        AdminVfRestoreCmd::new(PcieFunction::Pf, ctx, sqe, dma_buffer);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_vf_restore_normal_flow() {
    let persistent_store = [0u8; 2048];
    let test_configs = VfRestoreCmdTestConfigs {
        opcode: 0xC9,
        cntrl_id: 1,
        persistent_store_addr: Some(persistent_store.as_ptr() as usize),
        ..Default::default()
    };
    let (ctx, sqe) = make_fsm_for_vf_restore(test_configs);

    let dma_buffer = ctx
        .dma_heap()
        .allocate(core::mem::size_of::<VmLiveMigrationInfo>());

    assert!(dma_buffer.is_some());
    let dma_buffer = dma_buffer.unwrap();

    // VF Restore command FSM
    let mut fsm: AdminVfRestoreCmd<MockAdminEnvTrait> =
        AdminVfRestoreCmd::new(PcieFunction::Pf, ctx, sqe, dma_buffer);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
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
fn test_vf_restore_invalid_src_pfn() {
    let test_configs = VfRestoreCmdTestConfigs {
        opcode: 0xC9,
        cntrl_id: 200,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_restore(test_configs);

    let dma_buffer = ctx
        .dma_heap()
        .allocate(core::mem::size_of::<VmLiveMigrationInfo>());

    assert!(dma_buffer.is_some());
    let dma_buffer = dma_buffer.unwrap();

    // VF restore command FSM
    let mut fsm: AdminVfRestoreCmd<MockAdminEnvTrait> =
        AdminVfRestoreCmd::new(PcieFunction::Vf10, ctx, sqe, dma_buffer);

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
fn test_vf_restore_invalid_controller_id() {
    let test_configs = VfRestoreCmdTestConfigs {
        opcode: 0xC9,
        cntrl_id: 200,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_restore(test_configs);

    let dma_buffer = ctx
        .dma_heap()
        .allocate(core::mem::size_of::<VmLiveMigrationInfo>());

    assert!(dma_buffer.is_some());
    let dma_buffer = dma_buffer.unwrap();

    // VF restore command FSM
    let mut fsm: AdminVfRestoreCmd<MockAdminEnvTrait> =
        AdminVfRestoreCmd::new(PcieFunction::Pf, ctx, sqe, dma_buffer);

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
fn test_vf_restore_invalid_request_on_pf() {
    let test_configs = VfRestoreCmdTestConfigs {
        opcode: 0xC9,
        cntrl_id: 0,
        ..Default::default()
    };

    let (ctx, sqe) = make_fsm_for_vf_restore(test_configs);

    let dma_buffer = ctx
        .dma_heap()
        .allocate(core::mem::size_of::<VmLiveMigrationInfo>());

    assert!(dma_buffer.is_some());
    let dma_buffer = dma_buffer.unwrap();

    // VF restore command FSM
    let mut fsm: AdminVfRestoreCmd<MockAdminEnvTrait> =
        AdminVfRestoreCmd::new(PcieFunction::Pf, ctx, sqe, dma_buffer);

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
fn test_vf_restore_send_ipc_request_to_fp_failed() {
    let test_configs = VfRestoreCmdTestConfigs {
        opcode: 0xC9,
        cntrl_id: 1,
        fp_ipc_send_req_failure: true,
        ..Default::default()
    };
    let (ctx, sqe) = make_fsm_for_vf_restore(test_configs);

    let dma_buffer = ctx
        .dma_heap()
        .allocate(core::mem::size_of::<VmLiveMigrationInfo>());

    assert!(dma_buffer.is_some());
    let dma_buffer = dma_buffer.unwrap();

    // VF Restore command FSM
    let mut fsm: AdminVfRestoreCmd<MockAdminEnvTrait> =
        AdminVfRestoreCmd::new(PcieFunction::Pf, ctx, sqe, dma_buffer);

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
fn test_vf_restore_send_ipc_request_to_hsm_failed() {
    let test_configs = VfRestoreCmdTestConfigs {
        opcode: 0xC9,
        cntrl_id: 1,
        hsm_ipc_send_req_failure: true,
        ..Default::default()
    };
    let (ctx, sqe) = make_fsm_for_vf_restore(test_configs);

    let dma_buffer = ctx
        .dma_heap()
        .allocate(core::mem::size_of::<VmLiveMigrationInfo>());

    assert!(dma_buffer.is_some());
    let dma_buffer = dma_buffer.unwrap();

    // VF Restore command FSM
    let mut fsm: AdminVfRestoreCmd<MockAdminEnvTrait> =
        AdminVfRestoreCmd::new(PcieFunction::Pf, ctx, sqe, dma_buffer);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
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
fn test_vf_restore_none_fp_ipc_resp() {
    let test_configs = VfRestoreCmdTestConfigs {
        opcode: 0xC9,
        cntrl_id: 1,
        none_fp_ipc_resp: true,
        ..Default::default()
    };
    let (ctx, sqe) = make_fsm_for_vf_restore(test_configs);

    let dma_buffer = ctx
        .dma_heap()
        .allocate(core::mem::size_of::<VmLiveMigrationInfo>());

    assert!(dma_buffer.is_some());
    let dma_buffer = dma_buffer.unwrap();

    // VF Restore command FSM
    let mut fsm: AdminVfRestoreCmd<MockAdminEnvTrait> =
        AdminVfRestoreCmd::new(PcieFunction::Pf, ctx, sqe, dma_buffer);

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
fn test_vf_restore_none_hsm_ipc_resp() {
    let test_configs = VfRestoreCmdTestConfigs {
        opcode: 0xC9,
        cntrl_id: 1,
        none_hsm_ipc_resp: true,
        ..Default::default()
    };
    let (ctx, sqe) = make_fsm_for_vf_restore(test_configs);

    let dma_buffer = ctx
        .dma_heap()
        .allocate(core::mem::size_of::<VmLiveMigrationInfo>());

    assert!(dma_buffer.is_some());
    let dma_buffer = dma_buffer.unwrap();

    // VF Restore command FSM
    let mut fsm: AdminVfRestoreCmd<MockAdminEnvTrait> =
        AdminVfRestoreCmd::new(PcieFunction::Pf, ctx, sqe, dma_buffer);

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
fn test_vf_restore_invalid_fp_ipc_resp() {
    let test_configs = VfRestoreCmdTestConfigs {
        opcode: 0xC9,
        cntrl_id: 1,
        invalid_fp_ipc_resp: true,
        ..Default::default()
    };
    let (ctx, sqe) = make_fsm_for_vf_restore(test_configs);

    let dma_buffer = ctx
        .dma_heap()
        .allocate(core::mem::size_of::<VmLiveMigrationInfo>());

    assert!(dma_buffer.is_some());
    let dma_buffer = dma_buffer.unwrap();

    // VF Restore command FSM
    let mut fsm: AdminVfRestoreCmd<MockAdminEnvTrait> =
        AdminVfRestoreCmd::new(PcieFunction::Pf, ctx, sqe, dma_buffer);

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
fn test_vf_restore_invalid_hsm_ipc_resp() {
    let test_configs = VfRestoreCmdTestConfigs {
        opcode: 0xC9,
        cntrl_id: 1,
        invalid_hsm_ipc_resp: true,
        ..Default::default()
    };
    let (ctx, sqe) = make_fsm_for_vf_restore(test_configs);

    let dma_buffer = ctx
        .dma_heap()
        .allocate(core::mem::size_of::<VmLiveMigrationInfo>());

    assert!(dma_buffer.is_some());
    let dma_buffer = dma_buffer.unwrap();

    // VF Restore command FSM
    let mut fsm: AdminVfRestoreCmd<MockAdminEnvTrait> =
        AdminVfRestoreCmd::new(PcieFunction::Pf, ctx, sqe, dma_buffer);

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
        fsm.on_event(AdminFsmEvent::HsmIpcResponse, 0xFF),
        Err(AdminErr::IpcResponseError)
    );

    let (cqe, dma_buff) = fsm.response();

    assert!(dma_buff.is_none());
    assert!(cqe.is_some());
    let cqe = cqe.unwrap();

    assert_eq!(cqe.psf.status(), HostStatusCode::InternalError);
}
