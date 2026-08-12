// Copyright (c) Microsoft Corporation. All rights reserved.

use super::helper::*;

use mcr_types::*;

use crate::cmd_scheduler::*;
use crate::context::AdminFsmContext;
use crate::error::AdminErr;
use crate::fsm::tests::harness::AdminFsmTest;
use crate::fsm::AesGcmExtFsm;
use crate::mock::MockAdminEnvTrait;
use crate::recorder::AdminFsmEventRecorder;
use crate::{AdminFsmEvent, CmdFsm};

/// AES GCM EXT test configuration
#[derive(Clone, Copy)]
pub(crate) struct AesGcmExtTestConfigs {
    pub missing_request: bool,
    pub response_error: bool,
    pub pfn: u8,
    pub sqe_addr: usize,
    pub sqe_idx: u32,
    pub unaligned_src_data_ptr_null: bool,
    pub dma_begin_txn_count: usize,
    pub dma_end_txn_count: usize,
    pub dma_in_begin_txn_fail: bool,
    pub dma_out_begin_txn_fail: bool,
    pub dma_in_end_txn_fail: bool,
    pub dma_out_end_txn_fail: bool,
    pub dma_in_end_txn_empty: bool,
    pub dma_out_end_txn_empty: bool,
    pub tag_correction_fail: bool,
    pub tag_mismatch_error: bool,
    pub tag: u16,
}

impl Default for AesGcmExtTestConfigs {
    fn default() -> Self {
        Self {
            missing_request: false,
            response_error: false,
            pfn: 0x1,
            sqe_addr: 0,
            sqe_idx: 1,
            unaligned_src_data_ptr_null: false,
            dma_begin_txn_count: 0,
            dma_end_txn_count: 0,
            dma_in_begin_txn_fail: false,
            dma_out_begin_txn_fail: false,
            dma_in_end_txn_fail: false,
            dma_out_end_txn_fail: false,
            dma_in_end_txn_empty: false,
            dma_out_end_txn_empty: false,
            tag_correction_fail: false,
            tag_mismatch_error: false,
            tag: 0xAB,
        }
    }
}

#[test]
fn test_aes_gcm_ext_fsm_unknown_event() {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);

    let env = test.env();
    let scheduler = CmdScheduler::new(65 + 4, 1, AdminFsmEventRecorder::default());

    let ctx = AdminFsmContext::new(env, scheduler);

    let mut fsm = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_invalid_request_ptr() {
    let cfg = AesGcmExtTestConfigs {
        missing_request: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_invalid_request_config(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, 0xAB),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_response_send_error() {
    let cfg = AesGcmExtTestConfigs {
        response_error: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_invalid_request_config(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, 0xAB),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_invalid_pfn() {
    let cfg = AesGcmExtTestConfigs {
        pfn: 100,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_invalid_request_config(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, 0xAB),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_invalid_sqe_addr_zero() {
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: 0,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_invalid_request_config(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_invalid_unaligned_data_length() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_length = 20;
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_invalid_unaligned_data_length(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    // First event triggers IPC request, FSM waits for IPC response
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_tag_correction_failed() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_length = 0;
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        tag_correction_fail: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_unaligned_data_len_zero(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_decrypt_op_tag_mismatch_error() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_length = 0;
    // Original tag in SQE
    sqe.cmd.tag = [1u8; 16];
    let cqe = (&mut sqe as *mut CdmaIoGcmSqe) as *mut CdmaIoCqe;
    // Set decrypt operation and intermediate tag in CQE
    unsafe {
        (*cqe).attr.set_op(true);
        (*cqe).output_data_length = 16;
        (*cqe).tag = [1u8; 16];
    }
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        tag_mismatch_error: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_unaligned_data_len_zero(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_unaligned_data_length_zero_success() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_length = 0;
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_unaligned_data_len_zero(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_unaligned_src_data_ptr_null() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_ptr = MemoryAddr { lo: 0x0, hi: 0x0 };
    sqe.cmd.unaligned_src_data_length = 10;
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        unaligned_src_data_ptr_null: true,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_unaligned_data_len_nonzero(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );

    // DMA complete event
    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_unaligned_dst_data_ptr_null() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_ptr = MemoryAddr {
        lo: 0x11111111,
        hi: 0x11111111,
    };
    sqe.cmd.unaligned_src_data_length = 10;
    sqe.cmd.unaligned_dst_data_ptr = MemoryAddr { lo: 0x0, hi: 0x0 };
    sqe.cmd.unaligned_dst_data_length = 10;
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        dma_begin_txn_count: 1,
        dma_end_txn_count: 1,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_unaligned_data_len_nonzero(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_dma_buf_and_unaligned_dst_data_len_mismatch() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_ptr = MemoryAddr {
        lo: 0x11111111,
        hi: 0x11111111,
    };
    sqe.cmd.unaligned_src_data_length = 10;
    sqe.cmd.unaligned_dst_data_ptr = MemoryAddr {
        lo: 0x22222222,
        hi: 0x22222222,
    };
    sqe.cmd.unaligned_dst_data_length = 12;
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        dma_begin_txn_count: 1,
        dma_end_txn_count: 1,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_unaligned_data_len_nonzero(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_dma_in_begin_txn_failed() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_ptr = MemoryAddr {
        lo: 0x11111111,
        hi: 0x11111111,
    };
    sqe.cmd.unaligned_src_data_length = 10;
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        dma_in_begin_txn_fail: true,
        dma_begin_txn_count: 1,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_unaligned_data_len_nonzero(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_dma_in_end_txn_empty() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_ptr = MemoryAddr {
        lo: 0x11111111,
        hi: 0x11111111,
    };
    sqe.cmd.unaligned_src_data_length = 10;
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        dma_in_end_txn_empty: true,
        dma_begin_txn_count: 1,
        dma_end_txn_count: 1,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_unaligned_data_len_nonzero(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_dma_in_end_txn_failed() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_ptr = MemoryAddr {
        lo: 0x11111111,
        hi: 0x11111111,
    };
    sqe.cmd.unaligned_src_data_length = 10;
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        dma_in_end_txn_fail: true,
        dma_begin_txn_count: 1,
        dma_end_txn_count: 1,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_unaligned_data_len_nonzero(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_dma_in_tag_correction_failed() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_ptr = MemoryAddr {
        lo: 0x11111111,
        hi: 0x11111111,
    };
    sqe.cmd.unaligned_src_data_length = 10;
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        tag_correction_fail: true,
        dma_begin_txn_count: 1,
        dma_end_txn_count: 1,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_unaligned_data_len_nonzero(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_dma_out_begin_txn_failed() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_ptr = MemoryAddr {
        lo: 0x11111111,
        hi: 0x11111111,
    };
    sqe.cmd.unaligned_src_data_length = 10;
    sqe.cmd.unaligned_dst_data_ptr = MemoryAddr {
        lo: 0x22222222,
        hi: 0x22222222,
    };
    sqe.cmd.unaligned_dst_data_length = 10;
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        dma_out_begin_txn_fail: true,
        dma_begin_txn_count: 2,
        dma_end_txn_count: 1,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_unaligned_data_len_nonzero(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_dma_out_end_txn_empty() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_ptr = MemoryAddr {
        lo: 0x11111111,
        hi: 0x11111111,
    };
    sqe.cmd.unaligned_src_data_length = 10;
    sqe.cmd.unaligned_dst_data_ptr = MemoryAddr {
        lo: 0x22222222,
        hi: 0x22222222,
    };
    sqe.cmd.unaligned_dst_data_length = 10;
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        dma_out_end_txn_empty: true,
        dma_begin_txn_count: 2,
        dma_end_txn_count: 2,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_unaligned_data_len_nonzero(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, cfg.tag),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_dma_out_end_txn_failed() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_ptr = MemoryAddr {
        lo: 0x11111111,
        hi: 0x11111111,
    };
    sqe.cmd.unaligned_src_data_length = 10;
    sqe.cmd.unaligned_dst_data_ptr = MemoryAddr {
        lo: 0x22222222,
        hi: 0x22222222,
    };
    sqe.cmd.unaligned_dst_data_length = 10;
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        dma_out_end_txn_fail: true,
        dma_begin_txn_count: 2,
        dma_end_txn_count: 2,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_unaligned_data_len_nonzero(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, cfg.tag),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, cfg.tag),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_aes_gcm_ext_unaligned_data_length_nonzero_success() {
    let mut sqe = CdmaIoGcmSqe::default();
    sqe.cmd.unaligned_src_data_ptr = MemoryAddr {
        lo: 0x11111111,
        hi: 0x11111111,
    };
    sqe.cmd.unaligned_src_data_length = 10;
    sqe.cmd.unaligned_dst_data_ptr = MemoryAddr {
        lo: 0x22222222,
        hi: 0x22222222,
    };
    sqe.cmd.unaligned_dst_data_length = 10;
    let cfg = AesGcmExtTestConfigs {
        sqe_addr: (&mut sqe as *mut CdmaIoGcmSqe) as usize,
        dma_begin_txn_count: 2,
        dma_end_txn_count: 2,
        ..Default::default()
    };

    let ctx = make_fsm_for_aes_gcm_ext_unaligned_data_len_nonzero(cfg);
    let mut fsm: AesGcmExtFsm<MockAdminEnvTrait> = AesGcmExtFsm::new(ctx);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::AesGcmExtRequest, cfg.tag),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, cfg.tag),
        Err(AdminErr::Pending)
    );

    assert_eq!(
        fsm.on_event(AdminFsmEvent::DmaComplete, cfg.tag),
        Err(AdminErr::Pending)
    );
}
