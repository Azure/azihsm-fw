// Copyright (c) Microsoft Corporation. All rights reserved.

use core::cell::RefCell;
use std::rc::Rc;

use mcr_gdma_controller::DmaTxnCompletionDesc;
use test_log::test;

use self::helper::*;
use crate::error::*;
use crate::event::HsmFsmEvent;
use crate::fsm::tests::close_session_tests::close_app_session_cmd;
use crate::fsm::tests::ddi_decode_page;
use crate::fsm::tests::ddi_encode_page;
use crate::fsm::tests::harness::HsmFsmTest;
use crate::fsm::tests::page_alloc::Page;
use crate::fsm::*;
use crate::mock::*;

#[test]
fn test_invalid_event() {
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(MockEnv::new())));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
}

#[test]
fn test_io_channel_recv_none() {
    let config = HsmFsmTestConfigs {
        io_recv_none: true,
        partition_disabled: true,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env(HsmSqe::default(), config).env(1),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::IoChannelRecvNone)
    );
}

#[test]
fn test_rx_desc_error() {
    let config = HsmFsmTestConfigs {
        io_desc_err: true,
        partition_disabled: true,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env(HsmSqe::default(), config).env(1),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::IoChannelRecvErr)
    );
}

#[test]
fn test_pfn_not_enabled() {
    let config = HsmFsmTestConfigs {
        partition_disabled: true,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env(HsmSqe::default(), config).env(1),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::PartitionNotEnabled)
    );
}

#[test]
fn test_queue_not_enabled() {
    let config = HsmFsmTestConfigs {
        io_queue_disabled: true,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env(HsmSqe::default(), config).env(1),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::QueueNotEnabled)
    );
}

#[test]
fn test_send_err_cqe_failure() {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd().with_op(HsmSqeCmdOpcode::Unknown),
        ..make_sqe()
    };
    let config = HsmFsmTestConfigs {
        io_send_err: true,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(make_mock_env(sqe, config).env(1))));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::IoChannelSendError)
    );
}

#[test]
fn test_sqe_with_invalid_psdt() {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd().with_psdt(2),
        ..make_sqe()
    };
    let config = HsmFsmTestConfigs {
        expected_cqe_status: HostStatusCode::InvalidPsdtFieldInCommand,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(make_mock_env(sqe, config).env(1))));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_sqe_with_unknown_op() {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd().with_op(HsmSqeCmdOpcode::Unknown),
        ..make_sqe()
    };
    let config = HsmFsmTestConfigs {
        expected_cqe_status: HostStatusCode::InvalidCommandOpCode,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(make_mock_env(sqe, config).env(1))));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_sqe_with_invalid_src_len() {
    let sqe = HsmSqe {
        src: HsmSqeDmaDesc {
            len: 4097,
            ..Default::default()
        },
        ..make_sqe()
    };
    let config = HsmFsmTestConfigs {
        expected_cqe_status: HostStatusCode::InvalidSrcLenFieldInCommand,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(make_mock_env(sqe, config).env(1))));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_sqe_with_invalid_dst_len() {
    let sqe = HsmSqe {
        dst: HsmSqeDmaDesc {
            len: 8193,
            ..Default::default()
        },
        ..make_sqe()
    };
    let config = HsmFsmTestConfigs {
        expected_cqe_status: HostStatusCode::InvalidDstLenFieldInCommand,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(make_mock_env(sqe, config).env(1))));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_sqe_with_invalid_src_prp1_align() {
    let sqe = HsmSqe {
        src: HsmSqeDmaDesc {
            len: 4096,
            prp1: MemoryAddr {
                lo: 1,
                ..Default::default()
            },
            ..Default::default()
        },
        ..make_sqe()
    };
    let config = HsmFsmTestConfigs {
        expected_cqe_status: HostStatusCode::InvalidSrcPrpFieldInCommand,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(make_mock_env(sqe, config).env(1))));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_sqe_with_invalid_src_prp2_align() {
    let sqe = HsmSqe {
        src: HsmSqeDmaDesc {
            len: 4096,
            prp2: MemoryAddr {
                lo: 1,
                ..Default::default()
            },
            ..Default::default()
        },
        ..make_sqe()
    };
    let config = HsmFsmTestConfigs {
        expected_cqe_status: HostStatusCode::InvalidSrcPrpFieldInCommand,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(make_mock_env(sqe, config).env(1))));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_sqe_with_invalid_dst_prp1_align() {
    let sqe = HsmSqe {
        dst: HsmSqeDmaDesc {
            len: 4096,
            prp1: MemoryAddr {
                lo: 1,
                ..Default::default()
            },
            ..Default::default()
        },
        ..make_sqe()
    };
    let config = HsmFsmTestConfigs {
        expected_cqe_status: HostStatusCode::InvalidDstPrpFieldInCommand,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(make_mock_env(sqe, config).env(1))));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_sqe_with_invalid_dst_prp2_align() {
    let sqe = HsmSqe {
        dst: HsmSqeDmaDesc {
            len: 4096,
            prp2: MemoryAddr {
                lo: 1,
                ..Default::default()
            },
            ..Default::default()
        },
        ..make_sqe()
    };
    let config = HsmFsmTestConfigs {
        expected_cqe_status: HostStatusCode::InvalidDstPrpFieldInCommand,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(make_mock_env(sqe, config).env(1))));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_in_dma_alloc_failure() {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        ..make_sqe()
    };
    let config = HsmFsmTestConfigs {
        expected_cqe_status: HostStatusCode::InternalError,
        ..Default::default()
    };

    let mut test = make_mock_env(sqe, config);
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .return_once(|_| None);

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(test.env(1))));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_begin_in_dma_failure() {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        ..make_sqe()
    };
    let config = HsmFsmTestConfigs {
        expected_cqe_status: HostStatusCode::DmaStartError,
        ..Default::default()
    };

    let mut test = make_mock_env(sqe, config);
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .return_once(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(1)
        .return_const(Err(u32::MAX));

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(test.env(1))));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_end_in_dma_with_empty_entry() {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        ..make_sqe()
    };
    let config = HsmFsmTestConfigs {
        expected_cqe_status: HostStatusCode::DmaCompletionEmpty,
        ..Default::default()
    };

    let mut test = make_mock_env(sqe, config);
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .return_once(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(1)
        .return_const(Ok(()));
    test.dma_channel()
        .expect_end_txn()
        .times(1)
        .return_once(|| None);

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(test.env(1))));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_end_in_dma_with_error() {
    let desc = DmaTxnCompletionDesc {
        success: false,
        tag: TagId::default(),
    };
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        ..make_sqe()
    };
    let config = HsmFsmTestConfigs {
        expected_cqe_status: HostStatusCode::DmaTxnError,
        ..Default::default()
    };

    let mut test = make_mock_env(sqe, config);
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .return_once(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(1)
        .return_const(Ok(()));
    test.dma_channel()
        .expect_end_txn()
        .times(1)
        .return_once(|| Some(desc));

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(test.env(1))));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_end_in_dma_with_tag_mismatch() {
    let desc = DmaTxnCompletionDesc {
        success: true,
        tag: 1,
    };
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        ..make_sqe()
    };
    let config = HsmFsmTestConfigs {
        expected_cqe_status: HostStatusCode::DmaTagMismatch,
        ..Default::default()
    };

    let mut test = make_mock_env(sqe, config);
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .return_once(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(1)
        .return_const(Ok(()));
    test.dma_channel()
        .expect_end_txn()
        .times(1)
        .return_once(|| Some(desc));

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(test.env(1))));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_req_hdr_decode_failure() {
    let req = DdiGetApiRevCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            sess_id: None,
            op: DdiOp::GetApiRev,
        },
        data: DdiGetApiRevReq {},
    };

    let mut req_page = ddi_encode_page(&req);
    req_page.slice_mut()[0] = 0xFF;
    let mut resp_page = Page::new().unwrap();

    let config = HsmFsmTestConfigsWithDma {
        req_hdr: Some(&req.hdr),
        req: Some(&req_page),
        resp: Some(&mut resp_page),
        req_decode_error: true,
        cmd_fsm_compl_failure: true,
        ..Default::default()
    };

    let partition_call = if cfg!(feature = "mcr_test_hooks") {
        4
    } else {
        1
    };
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_with_dma(config).env(partition_call),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );

    let _resp: DdiErrCmdResp = ddi_decode_page(&resp_page);
}

#[test]
fn test_get_api_rev_cmd() {
    let req = DdiGetApiRevCmdReq {
        hdr: DdiReqHdr {
            rev: None,
            sess_id: None,
            op: DdiOp::GetApiRev,
        },
        data: DdiGetApiRevReq {},
    };

    let req_page = super::ddi_encode_page(&req);
    let mut resp_page = Page::new().unwrap();

    let config = HsmFsmTestConfigsWithDma {
        req_hdr: Some(&req.hdr),
        req: Some(&req_page),
        resp: Some(&mut resp_page),
        ..Default::default()
    };
    let partition_call = if cfg!(feature = "mcr_test_hooks") {
        6
    } else {
        3
    };
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_with_dma(config).env(partition_call),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );

    let _resp: DdiGetApiRevCmdResp = ddi_decode_page(&resp_page);
}

#[test]
fn test_aes_gen_key_cmd() {
    let req = DdiAesGenerateKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            sess_id: Some(0),
            op: DdiOp::AesGenerateKey,
        },
        data: DdiAesGenerateKeyReq {
            key_size: DdiAesKeySize::AesGcmBulk256,
            key_tag: None,
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_encrypt(true)
                    .with_decrypt(true)
                    .with_session(false),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
        },
    };

    let req_page = super::ddi_encode_page(&req);
    let mut resp_page = Page::new().unwrap();

    let config = HsmFsmTestConfigsWithDma {
        req_hdr: Some(&req.hdr),
        req: Some(&req_page),
        resp: Some(&mut resp_page),
        ..Default::default()
    };

    let partition_call = if cfg!(feature = "mcr_test_hooks") {
        6
    } else {
        3
    };
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_with_dma(config).env(partition_call),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_begin_send_failure() {
    let req = DdiGetApiRevCmdReq {
        hdr: DdiReqHdr {
            rev: None,
            sess_id: None,
            op: DdiOp::GetApiRev,
        },
        data: DdiGetApiRevReq {},
    };

    let req_page = super::ddi_encode_page(&req);
    let mut resp_page = Page::new().unwrap();

    let config = HsmFsmTestConfigsWithDma {
        req_hdr: Some(&req.hdr),
        req: Some(&req_page),
        resp: Some(&mut resp_page),
        io_send_err: true,
        ..Default::default()
    };

    let partition_call = if cfg!(feature = "mcr_test_hooks") {
        6
    } else {
        3
    };
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_with_dma(config).env(partition_call),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::IoChannelSendError)
    );
}

#[test]
fn test_dma_in_begin_txn_failure() {
    let req = DdiGetApiRevCmdReq {
        hdr: DdiReqHdr {
            rev: None,
            sess_id: None,
            op: DdiOp::GetApiRev,
        },
        data: DdiGetApiRevReq {},
    };

    let req_page = super::ddi_encode_page(&req);
    let mut resp_page = Page::new().unwrap();

    let config = HsmFsmTestConfigsWithDma {
        req_hdr: Some(&req.hdr),
        req: Some(&req_page),
        resp: Some(&mut resp_page),
        dma_out_begin_txn_error: true,
        io_send_err: true,
        ..Default::default()
    };

    let partition_call = if cfg!(feature = "mcr_test_hooks") {
        5
    } else {
        3
    };
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_with_dma(config).env(partition_call),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::IoChannelSendError)
    );
}

#[test]
fn test_flush_cmd_through_app_session() {
    let config = HsmFsmFlushCmdTestConfigs {
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_for_flush_session(config).env(2),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        fsm.on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_flush_cmd_invalid_manager_session() {
    let config = HsmFsmFlushCmdTestConfigs {
        invalid_session: true,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_for_flush_session(config).env(2),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_flush_cmd_invalid_app_session() {
    let config = HsmFsmFlushCmdTestConfigs {
        invalid_session: true,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_for_flush_session(config).env(2),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_flush_cmd_io_send_error() {
    let config = HsmFsmFlushCmdTestConfigs {
        io_send_error: true,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_for_flush_session(config).env(2),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        fsm.on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default()),
        Err(HsmErr::IoChannelSendError)
    );
}

#[test]
fn test_flush_cmd_with_invalid_id_in_sqe() {
    let config = HsmFsmFlushCmdTestConfigs {
        invalid_session_id: true,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_for_flush_session(config).env(1),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_flush_cmd_with_invalid_session_control_kind() {
    let config = HsmFsmFlushCmdTestConfigs {
        invalid_session_control_kind: true,
        ..Default::default()
    };

    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_for_flush_session(config).env(1),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );
}

#[test]
fn test_validate_req_hdr_session_not_expected() {
    let req = DdiOpenSessionCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            sess_id: Some(1024),
            op: DdiOp::OpenSession,
        },
        data: DdiOpenSessionReq {
            encrypted_credential: DdiEncryptedSessionCredential {
                encrypted_id: MborByteArray::new_with_len(core::ptr::null(), 16),
                encrypted_pin: MborByteArray::new_with_len(core::ptr::null(), 16),
                encrypted_seed: MborByteArray::new_with_len(core::ptr::null(), 48),
                iv: MborByteArray::new_with_len(core::ptr::null(), 16),
                nonce: [0; 32],
                tag: [0u8; 48],
            },
            pub_key: DdiDerPublicKey {
                der: MborByteArray::new_with_len(core::ptr::null(), 96),
                key_kind: DdiKeyType::Ecc384Public,
            },
        },
    };

    let req_page = super::ddi_encode_page(&req);
    let mut resp_page = Page::new().unwrap();

    let config = HsmFsmTestConfigsWithDma {
        req_hdr: Some(&req.hdr),
        req: Some(&req_page),
        resp: Some(&mut resp_page),
        cmd_fsm_compl_failure: true,
        invalid_session: true,
        ..Default::default()
    };

    let partition_call = if cfg!(feature = "mcr_test_hooks") {
        5
    } else {
        2
    };
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_with_dma(config).env(partition_call),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );

    let resp = ddi_decode_page::<<mcr_ddi_types::DdiErrCmdReq as mcr_ddi_types::DdiOpReq>::OpResp>(
        &resp_page,
    );
    assert_eq!(resp.hdr.status, DdiStatus::SessionNotExpected);
}

#[test]
fn test_validate_req_hdr_manager_session_not_found() {
    let req = DdiCloseSessionCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            sess_id: Some(1024),
            op: DdiOp::CloseSession,
        },
        data: DdiCloseSessionReq {},
    };

    let req_page = super::ddi_encode_page(&req);
    let mut resp_page = Page::new().unwrap();

    let config = HsmFsmTestConfigsWithDma {
        req_hdr: Some(&req.hdr),
        req: Some(&req_page),
        resp: Some(&mut resp_page),
        cmd_fsm_compl_failure: true,
        invalid_session: true,
        ..Default::default()
    };

    let partition_call = if cfg!(feature = "mcr_test_hooks") {
        5
    } else {
        2
    };
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_with_dma(config).env(partition_call),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );

    let resp = ddi_decode_page::<<mcr_ddi_types::DdiErrCmdReq as mcr_ddi_types::DdiOpReq>::OpResp>(
        &resp_page,
    );
    assert_eq!(resp.hdr.status, DdiStatus::SessionNotFound);
}

#[test]
fn test_validate_req_hdr_app_session_not_found() {
    let req = DdiAesGenerateKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            sess_id: Some(1024),
            op: DdiOp::AesGenerateKey,
        },
        data: DdiAesGenerateKeyReq {
            key_size: DdiAesKeySize::Aes128,
            key_tag: None,
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_encrypt(true)
                    .with_decrypt(true)
                    .with_session(false),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
        },
    };

    let req_page = super::ddi_encode_page(&req);
    let mut resp_page = Page::new().unwrap();

    let config = HsmFsmTestConfigsWithDma {
        req_hdr: Some(&req.hdr),
        req: Some(&req_page),
        resp: Some(&mut resp_page),
        cmd_fsm_compl_failure: true,
        invalid_session: true,
        ..Default::default()
    };

    let partition_call = if cfg!(feature = "mcr_test_hooks") {
        5
    } else {
        2
    };
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_with_dma(config).env(partition_call),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );

    let resp = ddi_decode_page::<<mcr_ddi_types::DdiErrCmdReq as mcr_ddi_types::DdiOpReq>::OpResp>(
        &resp_page,
    );
    assert_eq!(resp.hdr.status, DdiStatus::SessionNotFound);
}

#[test]
fn test_validate_req_hdr_unsupported_major_revision() {
    let req = DdiAesGenerateKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 2, minor: 0 }),
            sess_id: Some(1024),
            op: DdiOp::AesGenerateKey,
        },
        data: DdiAesGenerateKeyReq {
            key_size: DdiAesKeySize::Aes128,
            key_tag: None,
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_encrypt(true)
                    .with_decrypt(true)
                    .with_session(false),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
        },
    };

    let req_page = super::ddi_encode_page(&req);
    let mut resp_page = Page::new().unwrap();

    let config = HsmFsmTestConfigsWithDma {
        req_hdr: Some(&req.hdr),
        req: Some(&req_page),
        resp: Some(&mut resp_page),
        cmd_fsm_compl_failure: true,
        ..Default::default()
    };

    let partition_call = if cfg!(feature = "mcr_test_hooks") {
        5
    } else {
        2
    };
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_with_dma(config).env(partition_call),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );

    let resp = ddi_decode_page::<<mcr_ddi_types::DdiErrCmdReq as mcr_ddi_types::DdiOpReq>::OpResp>(
        &resp_page,
    );
    assert_eq!(resp.hdr.status, DdiStatus::UnsupportedRevision);
}

#[test]
fn test_validate_req_hdr_nosession_with_session_id() {
    let req = DdiGetApiRevCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            sess_id: Some(1024),
            op: DdiOp::GetApiRev,
        },
        data: DdiGetApiRevReq {},
    };

    let req_page = super::ddi_encode_page(&req);
    let mut resp_page = Page::new().unwrap();

    let config = HsmFsmTestConfigsWithDma {
        req_hdr: Some(&req.hdr),
        req: Some(&req_page),
        resp: Some(&mut resp_page),
        cmd_fsm_compl_failure: true,
        ..Default::default()
    };

    let partition_call = if cfg!(feature = "mcr_test_hooks") {
        5
    } else {
        2
    };
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_with_dma(config).env(partition_call),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );

    let resp = ddi_decode_page::<<mcr_ddi_types::DdiErrCmdReq as mcr_ddi_types::DdiOpReq>::OpResp>(
        &resp_page,
    );
    assert_eq!(resp.hdr.status, DdiStatus::InvalidArg);
}

#[test]
fn test_validate_req_hdr_sqe_session_id_mismatch() {
    let req = DdiAesGenerateKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            sess_id: Some(1024),
            op: DdiOp::AesGenerateKey,
        },
        data: DdiAesGenerateKeyReq {
            key_size: DdiAesKeySize::Aes128,
            key_tag: None,
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_encrypt(true)
                    .with_decrypt(true)
                    .with_session(false),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
        },
    };

    let req_page = super::ddi_encode_page(&req);
    let mut resp_page = Page::new().unwrap();

    let config = HsmFsmTestConfigsWithDma {
        req_hdr: Some(&req.hdr),
        req: Some(&req_page),
        resp: Some(&mut resp_page),
        cmd_fsm_compl_failure: true,
        sqe_session_id_deviation: 1,
        ..Default::default()
    };

    let partition_call = if cfg!(feature = "mcr_test_hooks") {
        5
    } else {
        2
    };
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_with_dma(config).env(partition_call),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );

    let resp = ddi_decode_page::<<mcr_ddi_types::DdiErrCmdReq as mcr_ddi_types::DdiOpReq>::OpResp>(
        &resp_page,
    );
    assert_eq!(resp.hdr.status, DdiStatus::InvalidArg);
}

#[test]
fn test_validate_req_hdr_sqe_session_ctrl_flags_mismatch() {
    let req = DdiAesGenerateKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            sess_id: Some(1024),
            op: DdiOp::AesGenerateKey,
        },
        data: DdiAesGenerateKeyReq {
            key_size: DdiAesKeySize::Aes128,
            key_tag: None,
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_encrypt(true)
                    .with_decrypt(true)
                    .with_session(false),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
        },
    };

    let req_page = super::ddi_encode_page(&req);
    let mut resp_page = Page::new().unwrap();

    let config = HsmFsmTestConfigsWithDma {
        req_hdr: Some(&req.hdr),
        req: Some(&req_page),
        resp: Some(&mut resp_page),
        cmd_fsm_compl_failure: true,
        invalid_session_flags: true,
        ..Default::default()
    };

    let partition_call = if cfg!(feature = "mcr_test_hooks") {
        5
    } else {
        2
    };
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_with_dma(config).env(partition_call),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );

    let resp = ddi_decode_page::<<mcr_ddi_types::DdiErrCmdReq as mcr_ddi_types::DdiOpReq>::OpResp>(
        &resp_page,
    );
    assert_eq!(resp.hdr.status, DdiStatus::InvalidArg);
}

#[test]
fn test_validate_req_hdr_cbor_session_id_mismatch() {
    let req = DdiAesGenerateKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            sess_id: None,
            op: DdiOp::AesGenerateKey,
        },
        data: DdiAesGenerateKeyReq {
            key_size: DdiAesKeySize::Aes128,
            key_tag: None,
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_encrypt(true)
                    .with_decrypt(true)
                    .with_session(false),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
        },
    };

    let req_page = super::ddi_encode_page(&req);
    let mut resp_page = Page::new().unwrap();

    let config = HsmFsmTestConfigsWithDma {
        req_hdr: Some(&req.hdr),
        req: Some(&req_page),
        resp: Some(&mut resp_page),
        cmd_fsm_compl_failure: true,
        ..Default::default()
    };

    let partition_call = if cfg!(feature = "mcr_test_hooks") {
        5
    } else {
        2
    };
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_with_dma(config).env(partition_call),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );

    let resp = ddi_decode_page::<<mcr_ddi_types::DdiErrCmdReq as mcr_ddi_types::DdiOpReq>::OpResp>(
        &resp_page,
    );
    assert_eq!(resp.hdr.status, DdiStatus::InvalidArg);
}

/// Helper module for hsm_fsm_tests
mod helper {
    use mcr_crypto_pka::PkaEccCmd;
    use mcr_crypto_pka::PkaEccCurve;
    use resource::PkaResource;

    use super::*;

    /// HSM FSM Test environment configurations
    #[derive(Default)]
    pub(crate) struct HsmFsmTestConfigs {
        /// Io Channel retuns None during recv call
        pub io_recv_none: bool,
        /// Io receive descriptor reports error status
        pub io_desc_err: bool,
        /// Parition Enabled?
        pub partition_disabled: bool,
        /// IoQueue Enabled?
        pub io_queue_disabled: bool,
        /// Io Send Error
        pub io_send_err: bool,
        /// Host CQE Status
        pub expected_cqe_status: HostStatusCode,
    }

    /// HSM FSM Test environment configurations with DMA layer mocking
    #[derive(Default)]
    pub(crate) struct HsmFsmTestConfigsWithDma<'a> {
        /// Request header
        pub req_hdr: Option<&'a DdiReqHdr>,
        /// Request page
        pub req: Option<&'a Page>,
        /// Response page
        pub resp: Option<&'a mut Page>,
        /// Host CQE Status
        pub expected_cqe_status: HostStatusCode,
        /// Request decode error
        pub req_decode_error: bool,
        /// GetApiRev success response
        pub cmd_fsm_compl_failure: bool,
        /// DMA In begin txn error
        pub dma_in_begin_txn_error: bool,
        /// DMA Out begin txn error
        pub dma_out_begin_txn_error: bool,
        /// Io Send Error
        pub io_send_err: bool,
        /// INvalid Session
        pub invalid_session: bool,
        /// Session Id mismatch
        pub sqe_session_id_deviation: u16,
        /// Invalid SQE session flags
        pub invalid_session_flags: bool,
        /// Expected app vault id
        pub expected_app_vault_id: u8,
        /// Expected session flags
        pub expected_session_flags: HsmSessionFlags,
    }

    /// Flush command environment configurations
    #[derive(Default)]
    pub struct HsmFsmFlushCmdTestConfigs {
        /// Valid session control kind?
        pub invalid_session_control_kind: bool,
        /// Valid Session?
        pub invalid_session: bool,
        /// Io Channel send error?
        pub io_send_error: bool,
        /// Valid Session Id?
        pub invalid_session_id: bool,
    }

    /// Make command field of the SQE
    ///
    /// # Returns
    ///
    /// HSM SQE command field
    pub(crate) fn make_sqe_cmd() -> HsmSqeCmd {
        HsmSqeCmd::default().with_op(HsmSqeCmdOpcode::Generic)
    }

    /// Make a default SQE
    ///
    /// # Returns
    ///
    /// Default SQE
    pub(crate) fn make_sqe() -> HsmSqe {
        HsmSqe {
            cmd: make_sqe_cmd(),
            src: HsmSqeDmaDesc {
                len: 1,
                ..Default::default()
            },
            dst: HsmSqeDmaDesc {
                len: 1,
                ..Default::default()
            },
            ..Default::default()
        }
    }

    /// Make IO receive descriptor
    ///
    /// # Arguments
    ///
    /// * `sqe` - SQE
    /// * `status` - true: Success, false: Failure
    ///
    /// # Returns
    ///
    /// IO receive descriptor
    pub(crate) fn make_rx_desc(sqe: HsmSqe, status: bool) -> Option<IoRxDesc> {
        let mut entry = [0u8; 64];
        entry.copy_from_slice(sqe.as_bytes());

        Some(IoRxDesc {
            sq_id: DevSqId(0),
            addr: 0,
            entry,
            pfn: PcieFunction::Pf,
            status,
        })
    }

    /// Make a mock environment for validation
    ///
    /// # Arguments
    ///
    /// * `sqe` - Hsm SQE
    /// * `config` - Test configurations
    ///
    /// # Returns
    ///
    /// HSM FSM Mock test environment
    pub(crate) fn make_mock_env(sqe: HsmSqe, config: HsmFsmTestConfigs) -> HsmFsmTest {
        let mut test = HsmFsmTest::default();
        let io_tx_complete_desc = IoTxCompleteDesc {
            queue_id: DevCqId::Id65.into(),
            queue_index: 0,
            tag: 0,
            status: IoTxCompleteStatus::Success,
        };

        test.io_channel()
            .expect_begin_recv()
            .times(1)
            .return_once(move || {
                if config.io_recv_none {
                    None
                } else {
                    make_rx_desc(sqe, !config.io_desc_err)
                }
            });

        if !config.io_desc_err && !config.io_recv_none {
            test.partition().expect_clone().times(1).returning(move || {
                let mut part = MockPartition::new();

                part.expect_enabled()
                    .times(1)
                    .return_once(move || !config.partition_disabled);

                if !config.partition_disabled {
                    part.expect_io_queue().times(1).returning(move |_| {
                        if config.io_queue_disabled {
                            None
                        } else {
                            Some(IoQueue::new(DevSqId::Id0, DevCqId::Id0))
                        }
                    });
                }

                part
            });

            if !config.io_queue_disabled && !config.partition_disabled {
                test.io_channel()
                    .expect_begin_send()
                    .times(1)
                    .return_once(move |desc| {
                        if config.io_send_err {
                            Err(u32::MAX)
                        } else {
                            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
                            assert_eq!(cqe.psf.status(), config.expected_cqe_status);
                            Ok(())
                        }
                    });
                if !config.io_send_err {
                    test.io_channel()
                        .expect_end_send()
                        .times(1)
                        .return_once(|| Some(io_tx_complete_desc));
                }
            }

            test.io_channel()
                .expect_end_recv()
                .times(1)
                .returning(|_, _| ());
        }

        test
    }

    /// Make a mock environment with DMA mocking complete HSM FSM command execution
    ///
    /// # Arguments
    ///
    /// * `config` - Test configurations
    ///
    /// # Returns
    ///
    /// HSM FSM Mock test environment
    pub(crate) fn make_mock_env_with_dma(config: HsmFsmTestConfigsWithDma) -> HsmFsmTest {
        let req = config.req.unwrap();
        let req_hdr = config.req_hdr.unwrap();
        let req_op = req_hdr.op;
        let resp = config.resp.unwrap();

        let sqe = HsmSqe {
            cmd: make_sqe_cmd(),
            src: HsmSqeDmaDesc {
                len: req.len() as u32,
                prp1: req.addr(),
                ..Default::default()
            },
            dst: HsmSqeDmaDesc {
                len: resp.len() as u32,
                prp1: resp.addr(),
                ..Default::default()
            },
            session_id: req_hdr.sess_id.unwrap_or(0) + config.sqe_session_id_deviation,
            session_flags: if config.invalid_session_flags {
                HsmSessionFlags::default()
            } else {
                HsmSessionFlags::default()
                    .with_ctrl(HsmSessionControlKind::from(req_hdr.op))
                    .with_id_valid(req_hdr.sess_id.is_some())
            },
            ..Default::default()
        };
        let io_tx_complete_desc = IoTxCompleteDesc {
            queue_id: DevCqId::Id65.into(),
            queue_index: 0,
            tag: 0,
            status: IoTxCompleteStatus::Success,
        };

        let mut test = HsmFsmTest::default();

        test.io_channel()
            .expect_begin_recv()
            .times(1)
            .return_once(move || make_rx_desc(sqe, true));

        // Below first mock partition object is cloned at hsm_fsm >
        // on_rx_ready() > ioq() > self.env.partition(desc.pfn) and dropped at
        // the end of the ioq() call. All following expectations on the mock
        // object should be met by the end of ioq() call
        test.partition().expect_clone().times(1).returning(|| {
            let mut part = MockPartition::new();

            part.expect_enabled().times(1).return_once(move || true);

            part.expect_io_queue()
                .times(1)
                .returning(move |_| Some(IoQueue::new(DevSqId::Id0, DevCqId::Id0)));

            part
        });

        // Below conditional mock partition object is cloned at hsm_fsm >
        // end_dma() > self.env.partition(self.pfn) under mcr_test_hooks
        #[cfg(feature = "mcr_test_hooks")]
        test.partition().expect_clone().times(1).returning(|| {
            let mut part = MockPartition::new();

            part.expect_hsm_fsm_test_action()
                .times(1)
                .returning(|_| None);

            part
        });

        let op = req_hdr.op;
        let session_flags_valid = config.invalid_session_flags;
        let session_id_valid = req_hdr.sess_id.is_some();
        if !config.req_decode_error {
            // Below second mock partition object is cloned at hsm_fsm >
            // init_cmd() > validate_req_hdr() > self.env.partition(pfn) and
            // dropped at the end of the validate_req_hdr() call. All following
            // expectations on the mock object should be met by the end of
            // validate_req_hdr() call
            test.partition().expect_clone().times(1).returning(move || {
                let mut part = MockPartition::new();

                part.expect_enabled().times(1).return_once(|| true);

                match op.into() {
                    DdiSessionKind::User => {
                        if config.sqe_session_id_deviation == 0
                            && !session_flags_valid
                            && session_id_valid
                        {
                            part.expect_needs_renegotiation().once().return_const(false);

                            part.expect_user_session().times(1).returning(move |_, _| {
                                if config.invalid_session {
                                    Err(HsmErr::SessionNotFound)
                                } else {
                                    let mut session = MockUserSession::new();
                                    session
                                        .expect_api_rev()
                                        .times(1)
                                        .return_const(DdiApiRev { major: 1, minor: 0 });

                                    Ok(session)
                                }
                            });
                        }
                    }
                    DdiSessionKind::None => {
                        if op == DdiOp::OpenSession && !session_id_valid {
                            part.expect_max_api_rev()
                                .times(1)
                                .return_once(move || DdiApiRev { major: 1, minor: 0 });

                            part.expect_min_api_rev()
                                .times(1)
                                .return_once(move || DdiApiRev { major: 1, minor: 0 });
                        }
                    }
                }

                part
            });
        }

        if !config.cmd_fsm_compl_failure {
            match req_op {
                // Below third mock partition object is cloned at hsm_fsm >
                // init_cmd() > self.env.partition(pfn) and dropped at the end
                // of the init_cmd() call. All following expectations on the
                // mock object should be met by the end of init_cmd() call
                DdiOp::OpenSession => {
                    let expected_app_vault_id = config.expected_app_vault_id;
                    test.partition().expect_clone().times(1).returning(move || {
                        let mut part = MockPartition::new();

                        part.expect_begin_open_user_session()
                            .times(1)
                            .returning(|_, _| {
                                let scheduler =
                                    CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
                                let resource = CmdResource::new(
                                    PkaResource::new(vec![MockPka::new()]),
                                    scheduler,
                                    1,
                                );
                                let engine = resource.acquire(TagId::default(), Some(0));
                                let pka_curve = PkaEccCurve::Ecc384;
                                Ok(OpenSessionCtx {
                                    tag: TagId::default(),
                                    engine_ref: engine.unwrap(),
                                    cmd_info: PkaEccCmd { curve: pka_curve },
                                    state: OpenSessionCmdState::MontgomeryConstCalc,
                                })
                            });

                        part.expect_continue_open_user_session()
                            .times(1)
                            .returning(|_, _| {
                                let scheduler =
                                    CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
                                let resource = CmdResource::new(
                                    PkaResource::new(vec![MockPka::new()]),
                                    scheduler,
                                    1,
                                );
                                let engine = resource.acquire(TagId::default(), Some(0));
                                let pka_curve = PkaEccCurve::Ecc384;
                                Ok(OpenSessionCtx {
                                    tag: TagId::default(),
                                    engine_ref: engine.unwrap(),
                                    cmd_info: PkaEccCmd { curve: pka_curve },
                                    state: OpenSessionCmdState::EcdhCompute,
                                })
                            });

                        part.expect_end_open_user_session().times(1).returning(
                            move |_, _, _, _, _, _, _| {
                                let mut app_session = MockUserSession::new();

                                app_session
                                    .expect_id()
                                    .times(2)
                                    .return_const(0x1 as SessionId);

                                app_session
                                    .expect_app_vault_id()
                                    .times(2)
                                    .returning(move || expected_app_vault_id as AppVaultId);

                                Ok(app_session)
                            },
                        );

                        part.expect_generate_bmk_session()
                            .times(1)
                            .returning(|_, _, _, _| Err(HsmErr::InsufficientBuffer));
                        part.expect_generate_bmk_session()
                            .times(1)
                            .returning(|_, _, _, _| Ok(()));

                        part.expect_verify_cred_is_set()
                            .times(1)
                            .returning(|| Ok(()));

                        part.expect_is_fips_approved().times(1).returning(|| false);

                        part
                    });
                }
                DdiOp::GetApiRev => {
                    test.partition().expect_clone().times(1).returning(|| {
                        let mut part = MockPartition::new();

                        part.expect_max_api_rev()
                            .times(1)
                            .return_once(move || DdiApiRev { major: 1, minor: 0 });

                        part.expect_min_api_rev()
                            .times(1)
                            .return_once(move || DdiApiRev { major: 1, minor: 0 });

                        part
                    });
                }
                DdiOp::AesGenerateKey => {
                    test.partition().expect_clone().times(1).returning(move || {
                        let mut part = MockPartition::new();

                        part.expect_is_partition_provisioned()
                            .times(1)
                            .returning(move || true);

                        part.expect_user_session().times(1).returning(move |_, _| {
                            let mut app_session = MockUserSession::new();

                            app_session
                                .expect_begin_aesbulk256_gen_key()
                                .times(1)
                                .return_once(|_, _, _, _, _| {
                                    Err(HsmErr::AesBulk256InvalidParameter)
                                });

                            Ok(app_session)
                        });

                        part
                    });
                }
                _ => unreachable!(),
            }

            test.dma_heap().expect_clone().times(1).returning(move || {
                let mut heap = MockDmaHeap::new();

                if req_op == DdiOp::OpenSession || req_op == DdiOp::ReopenSession {
                    heap.expect_allocate()
                        .times(2)
                        .returning(|s| Some(MockDmaAlloc::new(s)));
                }

                if req_op != DdiOp::AesGenerateKey {
                    heap.expect_allocate_from_pool()
                        .times(1)
                        .returning(|s| Some(MockDmaAlloc::new(s)));
                }
                heap
            });
        }

        test.dma_heap()
            .expect_allocate_from_pool()
            .times(
                if config.cmd_fsm_compl_failure || req_op == DdiOp::AesGenerateKey {
                    2
                } else {
                    1
                },
            )
            .returning(|s| Some(MockDmaAlloc::new(s)));

        // Begin inbound DMA operation
        test.dma_channel()
            .expect_begin_txn()
            .times(1)
            .returning(move |d| {
                if config.dma_in_begin_txn_error {
                    Err(u32::MAX)
                } else {
                    d.dst_fst
                        .addr
                        .slice_mut(d.len as usize)
                        .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
                    Ok(())
                }
            });
        // Complete inbound DMA operation
        if !config.dma_in_begin_txn_error {
            test.dma_channel().expect_end_txn().times(1).returning(|| {
                Some(DmaTxnCompletionDesc {
                    success: true,
                    tag: TagId::default(),
                })
            });
        }

        if !config.dma_in_begin_txn_error {
            // Begin outbound DMA operation
            test.dma_channel()
                .expect_begin_txn()
                .times(1)
                .returning(move |d| {
                    if config.dma_out_begin_txn_error {
                        Err(u32::MAX)
                    } else {
                        d.dst_fst
                            .addr
                            .slice_mut(d.len as usize)
                            .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
                        Ok(())
                    }
                });
            // Complete outbound DMA operation
            if !config.dma_out_begin_txn_error {
                test.dma_channel().expect_end_txn().times(1).returning(|| {
                    Some(DmaTxnCompletionDesc {
                        success: true,
                        tag: TagId::default(),
                    })
                });
            }
        }

        // Below conditional mock partition object is cloned at hsm_fsm >
        // begin_out_dma() > self.env.partition(self.pfn) under mcr_test_hooks
        #[cfg(feature = "mcr_test_hooks")]
        // if !config.dma_out_begin_txn_error {
        test.partition().expect_clone().times(1).returning(|| {
            let mut part = MockPartition::new();

            part.expect_hsm_fsm_test_action()
                .times(1)
                .returning(|_| None);

            part
        });
        // }

        // Below conditional mock partition object is cloned at hsm_fsm >
        // end_dma() > self.env.partition(self.pfn) under mcr_test_hooks
        #[cfg(feature = "mcr_test_hooks")]
        if !config.dma_out_begin_txn_error {
            test.partition().expect_clone().times(1).returning(|| {
                let mut part = MockPartition::new();

                part.expect_hsm_fsm_test_action()
                    .times(1)
                    .returning(|_| None);

                part
            });
        }

        // Send completion queue entry
        let expected_session_flags = config.expected_session_flags;
        test.io_channel()
            .expect_begin_send()
            .times(1)
            .return_once(move |desc| {
                if config.io_send_err {
                    Err(u32::MAX)
                } else {
                    let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
                    assert_eq!(cqe.psf.status(), config.expected_cqe_status);
                    if expected_session_flags.app_vault_id_is_valid() {
                        assert!(cqe.session_flags.app_vault_id_is_valid());
                        assert_eq!(cqe.app_vault_id, config.expected_app_vault_id);
                    }
                    Ok(())
                }
            });

        if !config.io_send_err {
            // Receive submission queue entry
            test.io_channel()
                .expect_end_send()
                .times(1)
                .return_once(|| Some(io_tx_complete_desc));
        }

        // End the receive operation
        test.io_channel()
            .expect_end_recv()
            .times(1)
            .return_const(());

        test
    }

    /// Make a mock environment for flush command
    ///
    /// # Arguments
    ///
    /// * `config` - Test configurations
    ///
    /// # Returns
    ///
    /// HSM FSM Mock test environment
    pub(crate) fn make_mock_env_for_flush_session(config: HsmFsmFlushCmdTestConfigs) -> HsmFsmTest {
        let sqe = HsmSqe {
            cmd: HsmSqeCmd::default().with_op(HsmSqeCmdOpcode::Flush),
            src: HsmSqeDmaDesc::default(),
            dst: HsmSqeDmaDesc::default(),
            session_flags: HsmSessionFlags::new()
                .with_id_valid(!config.invalid_session_id)
                .with_ctrl(if config.invalid_session_control_kind {
                    HsmSessionControlKind::InSession
                } else {
                    HsmSessionControlKind::Close
                }),
            session_id: 0,
            ..Default::default()
        };
        let io_tx_complete_desc = IoTxCompleteDesc {
            queue_id: DevCqId::Id65.into(),
            queue_index: 0,
            tag: 0,
            status: IoTxCompleteStatus::Success,
        };

        let mut test = HsmFsmTest::default();
        test.io_channel()
            .expect_begin_recv()
            .times(1)
            .return_once(|| make_rx_desc(sqe, true));

        test.partition().expect_clone().times(1).returning(move || {
            let mut part = MockPartition::new();
            part.expect_enabled().times(1).return_once(move || true);
            part.expect_io_queue()
                .times(1)
                .returning(move |_| Some(IoQueue::new(DevSqId::Id0, DevCqId::Id0)));

            part
        });

        if !config.invalid_session_id && !config.invalid_session_control_kind {
            test.partition().expect_clone().times(1).returning(move || {
                let mut part = MockPartition::new();

                part.expect_user_session().times(1).returning(move |_, _| {
                    if config.invalid_session {
                        Err(HsmErr::SessionNotFound)
                    } else {
                        Ok(MockUserSession::new())
                    }
                });
                if config.invalid_session {
                    part.expect_flush_session().once().returning(|_| ());
                } else {
                    part.expect_begin_close_user_session()
                        .once()
                        .returning(|_, _, _| Ok(close_app_session_cmd().unwrap()));
                    part.expect_end_close_user_session()
                        .once()
                        .returning(|_| Ok(()));
                }

                part
            });
        }
        test.io_channel()
            .expect_begin_send()
            .times(1)
            .return_once(move |_| {
                if config.io_send_error {
                    Err(HsmErr::IoChannelSendError)?
                } else {
                    Ok(())
                }
            });
        if !config.io_send_error {
            test.io_channel()
                .expect_end_send()
                .times(1)
                .return_once(|| Some(io_tx_complete_desc));
        }

        test.io_channel()
            .expect_end_recv()
            .times(1)
            .return_const(());

        test
    }
}

#[test]
fn test_short_app_id_in_cqe_with_open_app_session_cmd() {
    let req = DdiOpenSessionCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            sess_id: None,
            op: DdiOp::OpenSession,
        },
        data: DdiOpenSessionReq {
            encrypted_credential: DdiEncryptedSessionCredential {
                encrypted_id: MborByteArray::new_with_len(core::ptr::null(), 16),
                encrypted_pin: MborByteArray::new_with_len(core::ptr::null(), 16),
                encrypted_seed: MborByteArray::new_with_len(core::ptr::null(), 48),
                iv: MborByteArray::new_with_len(core::ptr::null(), 16),
                nonce: [0; 32],
                tag: [0u8; 48],
            },
            pub_key: DdiDerPublicKey {
                der: MborByteArray::new_with_len(core::ptr::null(), 96),
                key_kind: DdiKeyType::Ecc384Public,
            },
        },
    };

    let req_page = super::ddi_encode_page(&req);
    let mut resp_page = Page::new().unwrap();

    let expected_app_vault_id: u8 = 0x29;
    let config = HsmFsmTestConfigsWithDma {
        req_hdr: Some(&req.hdr),
        req: Some(&req_page),
        resp: Some(&mut resp_page),
        expected_app_vault_id,
        expected_session_flags: HsmSessionFlags::default().with_app_vault_id_is_valid(true),
        ..Default::default()
    };

    let partition_call = if cfg!(feature = "mcr_test_hooks") {
        6
    } else {
        3
    };
    let mut fsm = HsmFsm::new(Rc::new(RefCell::new(
        make_mock_env_with_dma(config).env(partition_call),
    )));

    assert_eq!(
        fsm.on_event(HsmFsmEvent::RxReady, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::DmaComplete, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        fsm.on_event(HsmFsmEvent::TxComplete, TagId::default()),
        Ok(())
    );

    let resp: DdiOpenSessionCmdResp = ddi_decode_page(&resp_page);
    assert_eq!(resp.data.short_app_id, expected_app_vault_id);
}
