// Copyright (c) Microsoft Corporation. All rights reserved.

use cfg_if::cfg_if;
use mcr_types::*;

cfg_if! {
    if #[cfg(not(feature = "mcr_test_hooks"))] {
        use mcr_crypto_pka::PkaRsaCmd;
        use mcr_crypto_pka::PkaRsaSize;
        use crate::fsm::HsmFsmEventRecorder;
        use crate::resource::FpIpcChannelResource;
        use crate::resource::PkaResource;
    }
}

use super::*;
use crate::cmd_scheduler::*;
use crate::fsm::der_key_import::DerKeyImportCmd;
use crate::fsm::HsmFsmResourceId;
use crate::partition::*;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(SessionId::default);
    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(SessionId::default);
    app_session
        .expect_delete_key()
        .times(0)
        .returning(|_| Ok(()));

    let req = MockDmaAlloc::new(10);

    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_encode_buf_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool().once().returning(|_| None);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(1)
        .returning(SessionId::default);
    app_session.expect_import_der_key().once().returning(
        |_entry_class, _key_usage, _entry_name, _entry_availability, _der| {
            Ok(ImportDerKeyResult {
                priv_key_id: 4,
                pub_key_data: None,
                key_type: DdiKeyType::Rsa2kPrivate,
            })
        },
    );
    app_session.expect_delete_key().once().returning(|_| Ok(()));
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));

    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_requires_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let session = MockUserSession::new();

    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req(), &heap).unwrap();
    let cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, session, PcieFunction::Vf0);

    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::Pka));
}

#[test]
fn test_acquire_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let session = MockUserSession::new();
    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, session, PcieFunction::Vf0);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::Pka)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    );
}

#[test]
fn test_der_key_import_no_crt() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session.expect_import_der_key().once().returning(
        |_entry_class, _key_usage, _entry_name, _entry_availability, _der| {
            Ok(ImportDerKeyResult {
                priv_key_id: 4,
                pub_key_data: Some(vec![1; 16]),
                key_type: DdiKeyType::Rsa2kPrivate,
            })
        },
    );
    app_session.expect_id().once().returning(Default::default);
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiDerKeyImportCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_der_key_import_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session.expect_import_der_key().once().returning(
        |_entry_class, _key_usage, _entry_name, _entry_availability, _der| {
            Err(HsmErr::DerDecodeFailed)
        },
    );
    app_session.expect_id().once().returning(Default::default);

    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DerDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_der_key_import_crt() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_import_der_crt_key()
        .once()
        .returning(|_tag, _der| Ok((import_der_crt_key().unwrap(), vec![1; 16])));
    app_session
        .expect_continue_import_der_crt_key()
        .once()
        .returning(|_der| Ok(import_der_crt_key().unwrap()));
    app_session
        .expect_end_import_der_crt_key()
        .once()
        .returning(|_op, _key_usage, _key_tag, _key_availability| {
            Ok((0, DdiKeyType::Rsa2kPrivateCrt))
        });
    app_session.expect_id().times(2).returning(Default::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req_crt(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiDerKeyImportCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req_crt().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req_crt().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req_crt().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_der_key_import_crt_on_engine_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_import_der_crt_key()
        .once()
        .returning(|_tag, _der| Err(HsmErr::Pending));
    app_session
        .expect_begin_import_der_crt_key()
        .once()
        .returning(|_tag, _der| Ok((import_der_crt_key().unwrap(), vec![1; 16])));
    app_session
        .expect_continue_import_der_crt_key()
        .once()
        .returning(|_der| Ok(import_der_crt_key().unwrap()));
    app_session
        .expect_end_import_der_crt_key()
        .once()
        .returning(|_op, _key_usage, _key_tag, _key_availability| {
            Ok((0, DdiKeyType::Rsa2kPrivateCrt))
        });
    app_session.expect_id().times(2).returning(Default::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req_crt(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiDerKeyImportCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req_crt().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req_crt().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req_crt().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_der_key_import_crt_on_engine_ready_err_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_import_der_crt_key()
        .once()
        .returning(|_tag, _der| Err(HsmErr::Pending));
    app_session
        .expect_begin_import_der_crt_key()
        .once()
        .returning(|_tag, _der| Err(HsmErr::Pending));

    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req_crt(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::InvalidState)
    );
}

#[test]
fn test_der_key_import_crt_on_engine_ready_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_import_der_crt_key()
        .once()
        .returning(|_tag, _der| Err(HsmErr::InvalidArgument));
    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req_crt(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
}

#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session.expect_import_der_key().once().returning(
        |_entry_class, _key_usage, _entry_name, _entry_availability, _der| {
            Ok(ImportDerKeyResult {
                priv_key_id: 4,
                pub_key_data: None,
                key_type: DdiKeyType::Rsa2kPrivate,
            })
        },
    );
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
}

#[test]
fn test_der_key_import_aesbulk256() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Ok(import_der_aesbulk256_key().unwrap()));
    app_session
        .expect_end_import_der_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_get_masked_bulk_key_len()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_bulk_key()
        .once()
        .returning(|_, _, _, _| Ok(()));

    #[cfg(feature = "mcr_test_hooks")]
    app_session
        .expect_cmd_fsm_test_action()
        .times(1)
        .returning(|_| None);

    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req_aesbulk256(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiDerKeyImportCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req_aesbulk256().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req_aesbulk256().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req_aesbulk256().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_der_key_import_aesbulk256_on_engine_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Err(HsmErr::Pending));
    app_session
        .expect_begin_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Ok(import_der_aesbulk256_key().unwrap()));
    app_session
        .expect_end_import_der_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_get_masked_bulk_key_len()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_bulk_key()
        .once()
        .returning(|_, _, _, _| Ok(()));

    #[cfg(feature = "mcr_test_hooks")]
    app_session
        .expect_cmd_fsm_test_action()
        .times(1)
        .returning(|_| None);

    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req_aesbulk256(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiDerKeyImportCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req_aesbulk256().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req_aesbulk256().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req_aesbulk256().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_acquire_fp_ipc_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let session = MockUserSession::new();
    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req_aesbulk256(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, session, PcieFunction::Vf0);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
    );
}

#[test]
fn test_der_key_import_aesbulk256_on_engine_ready_err_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Err(HsmErr::Pending));
    app_session
        .expect_begin_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Err(HsmErr::Pending));

    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req_aesbulk256(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel),
            TagId::default()
        ),
        Err(HsmErr::InvalidState)
    );
}

#[test]
fn test_der_key_import_aesbulk256_on_engine_ready_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Err(HsmErr::InvalidArgument));

    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req_aesbulk256(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
}

#[test]
fn test_der_key_import_aesbulk256_rollback_on_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Ok(import_der_aesbulk256_key().unwrap()));
    app_session
        .expect_end_import_der_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_get_masked_bulk_key_len()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_bulk_key()
        .once()
        .returning(|_, _, _, _| Ok(()));
    app_session
        .expect_begin_rollback_aesbulk256_key()
        .once()
        .returning(|_, _, _| Ok(()));
    app_session
        .expect_end_rollback_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));

    #[cfg(feature = "mcr_test_hooks")]
    app_session
        .expect_cmd_fsm_test_action()
        .times(1)
        .returning(|_| None);

    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req_aesbulk256(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiDerKeyImportCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req_aesbulk256().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req_aesbulk256().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req_aesbulk256().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);

    assert_eq!(cmd.rollback(TagId::default()), Err(HsmErr::Pending));

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());
}

#[test]
fn test_der_key_import_aesbulk256_rollback_during_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Ok(import_der_aesbulk256_key().unwrap()));
    app_session
        .expect_end_import_der_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_get_masked_bulk_key_len()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_bulk_key()
        .once()
        .returning(|_, _, _, _| Ok(()));
    app_session
        .expect_begin_rollback_aesbulk256_key()
        .once()
        .returning(|_, _, _| Ok(()));
    app_session
        .expect_end_rollback_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));

    #[cfg(feature = "mcr_test_hooks")]
    app_session
        .expect_cmd_fsm_test_action()
        .times(1)
        .returning(|_| None);

    let req = encode_buf::<DdiDerKeyImportCmdReq, _>(&cmd_req_aesbulk256(), &heap).unwrap();
    let mut cmd = DerKeyImportCmd::<MockEnv>::new(req, heap, app_session, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(cmd.rollback(TagId::default()), Ok(()));

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiDerKeyImportCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req_aesbulk256().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req_aesbulk256().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req_aesbulk256().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);

    assert_eq!(cmd.rollback(TagId::default()), Err(HsmErr::Pending));

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());
}

fn cmd_req() -> DdiDerKeyImportCmdReq {
    let der = MborByteArray::new_with_len(
        TEST_RSA_2K_PRIVATE_KEY.as_ptr(),
        TEST_RSA_2K_PRIVATE_KEY.len(),
    );

    DdiDerKeyImportCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::DerKeyImport,
            sess_id: Some(SessionId::default()),
        },
        data: DdiDerKeyImportReq {
            der,
            key_class: DdiKeyClass::Rsa,
            key_tag: Some(1),
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_sign(true)
                    .with_verify(true)
                    .with_session(false),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
        },
    }
}

fn cmd_req_aesbulk256() -> DdiDerKeyImportCmdReq {
    let der = MborByteArray::new_with_len(
        TEST_RSA_2K_PRIVATE_KEY.as_ptr(),
        TEST_RSA_2K_PRIVATE_KEY.len(),
    );

    DdiDerKeyImportCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::DerKeyImport,
            sess_id: Some(SessionId::default()),
        },
        data: DdiDerKeyImportReq {
            der,
            key_class: DdiKeyClass::AesGcmBulk,
            key_tag: Some(1),
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_sign(true)
                    .with_verify(true)
                    .with_session(false),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
        },
    }
}

fn cmd_req_crt() -> DdiDerKeyImportCmdReq {
    let der = MborByteArray::new_with_len(
        TEST_RSA_2K_PRIVATE_KEY.as_ptr(),
        TEST_RSA_2K_PRIVATE_KEY.len(),
    );

    DdiDerKeyImportCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::DerKeyImport,
            sess_id: Some(SessionId::default()),
        },
        data: DdiDerKeyImportReq {
            der,
            key_class: DdiKeyClass::RsaCrt,
            key_tag: Some(1),
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_sign(true)
                    .with_verify(true)
                    .with_session(false),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
        },
    }
}

fn api_rev() -> DdiApiRev {
    DdiApiRev { major: 1, minor: 0 }
}

const TEST_RSA_2K_PRIVATE_KEY: [u8; 1214] = [
    0x30, 0x82, 0x04, 0xba, 0x02, 0x01, 0x00, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7,
    0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x04, 0x82, 0x04, 0xa4, 0x30, 0x82, 0x04, 0xa0, 0x02, 0x01,
    0x00, 0x02, 0x82, 0x01, 0x01, 0x00, 0xe1, 0x60, 0x77, 0xe2, 0x62, 0x3f, 0x84, 0x56, 0xc9, 0x2a,
    0xc1, 0xf2, 0x09, 0x9e, 0x97, 0x22, 0x88, 0x68, 0x47, 0xa4, 0x94, 0x35, 0x6e, 0x81, 0x85, 0xc6,
    0xe6, 0x1e, 0xa8, 0x59, 0xb8, 0x69, 0x6f, 0xfe, 0x29, 0x31, 0x96, 0xac, 0x68, 0x8a, 0x09, 0x39,
    0x3b, 0x89, 0x9b, 0x96, 0xbf, 0x8f, 0x23, 0x12, 0x61, 0xbf, 0x46, 0x69, 0x6d, 0x67, 0x28, 0x56,
    0xab, 0xdf, 0x41, 0xc9, 0x5e, 0x80, 0x0b, 0x73, 0xac, 0xbe, 0x50, 0x08, 0xe0, 0x29, 0x12, 0x71,
    0xce, 0xd0, 0x8e, 0xff, 0x3e, 0x90, 0x3d, 0x5a, 0xcc, 0x14, 0x7f, 0xa9, 0xf0, 0x68, 0xdc, 0x1c,
    0xd8, 0xaf, 0x64, 0xcc, 0x0b, 0x43, 0xb1, 0xa9, 0x3d, 0xfb, 0xe8, 0xbc, 0x90, 0x1a, 0x45, 0xd2,
    0xdb, 0x17, 0xf5, 0x7a, 0xb5, 0xb3, 0x9e, 0x64, 0x31, 0xa5, 0x43, 0xb7, 0x94, 0xa7, 0x31, 0x29,
    0x79, 0x41, 0x69, 0x14, 0xdd, 0x6d, 0x67, 0x68, 0x0a, 0x36, 0x38, 0x0e, 0x35, 0xc6, 0x62, 0xcf,
    0x38, 0xcc, 0x52, 0x64, 0x8d, 0xa6, 0x7e, 0x7e, 0x70, 0x60, 0x46, 0x29, 0x68, 0x3c, 0x42, 0x2e,
    0xe2, 0xd8, 0x21, 0x6d, 0x01, 0x65, 0xc5, 0x86, 0x36, 0xeb, 0x0f, 0x1e, 0x6d, 0xf1, 0xd8, 0x7b,
    0xe0, 0x4d, 0xce, 0x71, 0xc8, 0x35, 0x5c, 0x6f, 0x0c, 0x4a, 0x8b, 0xf8, 0x07, 0x23, 0x6b, 0xfe,
    0x47, 0xdc, 0xbd, 0x02, 0xf2, 0xff, 0xb0, 0xdf, 0xcf, 0x02, 0xf6, 0xa1, 0x4b, 0x6b, 0x99, 0xcc,
    0xc6, 0x76, 0x30, 0xc5, 0xe4, 0x02, 0xf4, 0xa2, 0x02, 0xbf, 0x71, 0x31, 0x3d, 0x80, 0x70, 0x60,
    0x23, 0x12, 0xad, 0x2f, 0x02, 0x20, 0x42, 0x67, 0x15, 0x7a, 0x6d, 0xf4, 0x58, 0x2a, 0x8a, 0x1d,
    0x25, 0x1d, 0xfd, 0x01, 0x3f, 0x83, 0x5f, 0x5a, 0xfb, 0x11, 0x98, 0xda, 0x55, 0x96, 0x8f, 0x26,
    0x61, 0x25, 0x8b, 0xdb, 0xfc, 0xc9, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02, 0x81, 0xff, 0x7b, 0x8b,
    0x66, 0x2c, 0x5d, 0xaf, 0x1e, 0x87, 0x1f, 0x14, 0xa6, 0x91, 0xb2, 0x09, 0x92, 0xcf, 0xb0, 0xa1,
    0x79, 0x4f, 0x13, 0xef, 0x8b, 0xa4, 0x1f, 0x5b, 0xe8, 0xc9, 0x90, 0x2a, 0x49, 0x42, 0x2d, 0xcc,
    0xd0, 0x1d, 0x5e, 0xd0, 0x79, 0x28, 0x87, 0x3b, 0x2d, 0xbd, 0x41, 0x37, 0xb7, 0x1f, 0xbf, 0xc4,
    0xa9, 0x25, 0xdb, 0xc8, 0x99, 0xda, 0xf2, 0x97, 0x3a, 0xf5, 0x7c, 0xc5, 0x3b, 0x5d, 0xa0, 0x3e,
    0xc8, 0xc8, 0x35, 0x17, 0x53, 0x1f, 0x30, 0xa7, 0xdd, 0x0c, 0x76, 0xac, 0x1f, 0x4a, 0x47, 0xad,
    0x28, 0xdc, 0xbe, 0x74, 0x14, 0x55, 0x66, 0xfe, 0x69, 0x1f, 0x11, 0xcc, 0xc8, 0x5f, 0xfe, 0x03,
    0xc8, 0x4b, 0xf9, 0x9e, 0x0e, 0xb5, 0xad, 0x90, 0xe8, 0x89, 0x39, 0xb2, 0x5f, 0xe8, 0x6b, 0xeb,
    0x2b, 0x4b, 0xc2, 0x28, 0x8a, 0xff, 0x1b, 0x9e, 0xa0, 0x84, 0x3a, 0xc0, 0xdf, 0xf5, 0x11, 0x6e,
    0xa5, 0x93, 0xd3, 0x05, 0x13, 0x6b, 0x98, 0x70, 0x1d, 0xa8, 0x8d, 0xda, 0x2d, 0xcd, 0xcb, 0x11,
    0x48, 0x59, 0xf4, 0xaa, 0xa9, 0x8a, 0xb0, 0x8a, 0xf4, 0x8d, 0xb4, 0x00, 0x35, 0x9f, 0x2b, 0x44,
    0x99, 0x06, 0x41, 0x99, 0xcb, 0xe4, 0x24, 0xc1, 0xfa, 0xb4, 0x2b, 0x42, 0xc7, 0xbe, 0x50, 0xd1,
    0xca, 0x16, 0xed, 0x69, 0x68, 0xfe, 0xcc, 0x13, 0xd0, 0x6b, 0x8c, 0xa2, 0xfd, 0x97, 0x37, 0xf4,
    0xdc, 0x2b, 0x59, 0x63, 0xf0, 0x4f, 0x15, 0x1e, 0x6d, 0x4a, 0xed, 0x16, 0x4f, 0xff, 0xc8, 0x73,
    0x79, 0x8f, 0x4c, 0x3f, 0x29, 0xfa, 0x00, 0x8d, 0xc1, 0xf2, 0xe9, 0x32, 0x46, 0xae, 0x68, 0x9d,
    0x64, 0xe1, 0xbe, 0x0e, 0x3d, 0xad, 0x31, 0xa4, 0x1f, 0x16, 0x58, 0x73, 0x5d, 0x89, 0x70, 0xd9,
    0xdb, 0xf4, 0x91, 0xab, 0x6d, 0x71, 0xf0, 0x2b, 0x8d, 0xf9, 0x59, 0xd7, 0x1d, 0x02, 0x81, 0x81,
    0x00, 0xfb, 0x9c, 0x2b, 0xde, 0xb2, 0xda, 0x0d, 0xc7, 0x16, 0xd4, 0x69, 0xb7, 0xca, 0xb1, 0x2d,
    0x3d, 0x4b, 0xd0, 0x4a, 0x9c, 0xbb, 0xc6, 0x99, 0x7d, 0xfe, 0x50, 0xb2, 0xde, 0x84, 0x64, 0xef,
    0xbd, 0xf8, 0x72, 0x9e, 0x55, 0x4e, 0xb6, 0xa6, 0xa1, 0x68, 0x47, 0xb0, 0x69, 0x77, 0x1a, 0x7b,
    0x63, 0x4a, 0x05, 0xc1, 0xfa, 0xce, 0x10, 0x9c, 0x1f, 0x18, 0x1a, 0x46, 0xb9, 0xb5, 0x66, 0xef,
    0xfc, 0x04, 0x29, 0x27, 0xb9, 0x3d, 0xa4, 0x85, 0x78, 0x99, 0xe4, 0x6e, 0x2e, 0xb0, 0x42, 0xbb,
    0x95, 0x93, 0x46, 0xf9, 0x70, 0x91, 0x1b, 0xf3, 0x9a, 0xfb, 0xaa, 0x96, 0x7c, 0x82, 0x77, 0x33,
    0xb9, 0x98, 0x2f, 0xa3, 0xd4, 0x24, 0x82, 0xca, 0x57, 0x74, 0xde, 0x19, 0x38, 0x3e, 0xc2, 0xda,
    0x31, 0x03, 0xf9, 0x6d, 0xcb, 0x7f, 0x5c, 0x90, 0xc8, 0x0d, 0x62, 0x31, 0xab, 0xb3, 0xde, 0x84,
    0x9b, 0x02, 0x81, 0x81, 0x00, 0xe5, 0x4f, 0x1f, 0xff, 0x19, 0x17, 0xc2, 0x40, 0xf5, 0xe4, 0x3a,
    0x88, 0xaa, 0xa5, 0x45, 0x11, 0x13, 0xe7, 0x2b, 0x14, 0x96, 0x83, 0xbb, 0x83, 0x3f, 0x75, 0x07,
    0xaa, 0x94, 0x1d, 0x82, 0xd6, 0x8a, 0x63, 0xad, 0xf1, 0x3a, 0xb4, 0x4c, 0xcd, 0x34, 0x09, 0x65,
    0xc6, 0xe2, 0x0b, 0xfd, 0xc3, 0x4d, 0x95, 0x7c, 0xf2, 0x9d, 0x5e, 0x97, 0xe1, 0x4a, 0x07, 0x5f,
    0xb2, 0x7b, 0x0a, 0x4e, 0x00, 0x40, 0xb1, 0xa5, 0x3b, 0xf6, 0x99, 0x21, 0xb5, 0x8b, 0x97, 0xc1,
    0xf1, 0x1e, 0x86, 0xfa, 0xaa, 0x03, 0x7f, 0xa8, 0x9a, 0xb1, 0x01, 0x98, 0x89, 0xf1, 0x01, 0x63,
    0x89, 0x64, 0xd4, 0x0b, 0x6e, 0x89, 0x72, 0xc9, 0x85, 0xcf, 0x55, 0x9c, 0xe8, 0x2e, 0x34, 0xbd,
    0xf3, 0x7c, 0x32, 0xc2, 0xfc, 0xf8, 0xc5, 0xf1, 0xf5, 0xa9, 0x12, 0xc0, 0xf2, 0xee, 0xb4, 0xb1,
    0x57, 0x5b, 0x10, 0xb0, 0x6b, 0x02, 0x81, 0x80, 0x74, 0xd2, 0xbd, 0x37, 0xc8, 0x79, 0x20, 0x1e,
    0x89, 0x46, 0x14, 0xd3, 0xe6, 0x43, 0xbf, 0x8a, 0x8f, 0x51, 0xe5, 0xe2, 0xc1, 0xf8, 0xe3, 0x39,
    0xb1, 0xc4, 0x0c, 0x58, 0xee, 0xc5, 0xe2, 0xde, 0xa4, 0xa5, 0xab, 0x48, 0x56, 0xa4, 0xcd, 0xd7,
    0x71, 0x90, 0x9f, 0xa3, 0x48, 0x4e, 0xbe, 0x6d, 0x8a, 0x68, 0x03, 0xfa, 0x0c, 0x85, 0x7f, 0xc7,
    0x9c, 0x2c, 0x4f, 0x1c, 0x58, 0xd2, 0xb3, 0xa8, 0xa2, 0xd1, 0xed, 0x04, 0xc0, 0x4f, 0x4c, 0x3d,
    0x83, 0xce, 0xa1, 0x2e, 0x02, 0x5e, 0xe9, 0xb3, 0xf8, 0x4e, 0xe2, 0xf0, 0x56, 0x1f, 0xd1, 0x4a,
    0xeb, 0x80, 0xf8, 0x20, 0x55, 0x7f, 0x3d, 0x3f, 0xf6, 0x1e, 0x60, 0x85, 0xd6, 0x71, 0xf7, 0xbb,
    0x05, 0xa3, 0x3d, 0xb8, 0x74, 0xc3, 0x8a, 0x05, 0x6a, 0x1f, 0xfc, 0xcf, 0x98, 0x92, 0x05, 0x13,
    0x2d, 0xcb, 0xa2, 0xde, 0x63, 0x44, 0x74, 0xf3, 0x02, 0x81, 0x80, 0x6c, 0x0b, 0xfc, 0x67, 0x96,
    0xcb, 0x3b, 0x1c, 0xa0, 0xc0, 0x09, 0x54, 0x9c, 0x13, 0x83, 0x97, 0xa8, 0x69, 0x24, 0x43, 0x6f,
    0x28, 0x63, 0x12, 0x54, 0xb4, 0x30, 0x08, 0x90, 0x01, 0xd7, 0xc4, 0x7f, 0x30, 0xb8, 0xa5, 0x11,
    0xa4, 0x23, 0x0c, 0x0d, 0x98, 0xdf, 0xfb, 0xf6, 0x46, 0xf0, 0x2b, 0x36, 0x43, 0x59, 0xbc, 0x77,
    0xaa, 0x3a, 0xa6, 0x4c, 0xdb, 0x6c, 0x9c, 0x0c, 0x9d, 0xae, 0x63, 0x30, 0x18, 0x84, 0x62, 0xdc,
    0xaf, 0x0a, 0xd3, 0x20, 0x13, 0x41, 0xae, 0xfb, 0x53, 0x5e, 0x88, 0xfd, 0x5d, 0x09, 0x74, 0xda,
    0x32, 0x86, 0x4d, 0x78, 0xe1, 0xce, 0xa4, 0xce, 0x7d, 0x9b, 0x65, 0x5a, 0x1e, 0x5c, 0x16, 0x50,
    0xbb, 0x66, 0x53, 0x80, 0x72, 0x19, 0x8e, 0xc0, 0xd6, 0xaa, 0x49, 0xc8, 0x6e, 0x7c, 0xb3, 0xe4,
    0x16, 0x92, 0x13, 0xe5, 0xa5, 0xfe, 0x69, 0xca, 0xde, 0xf2, 0x41, 0x02, 0x81, 0x80, 0x24, 0xf0,
    0x01, 0x0f, 0x34, 0xc7, 0x27, 0x2b, 0x7a, 0xf1, 0x4e, 0x04, 0x50, 0xd3, 0x18, 0x2b, 0x9c, 0x75,
    0x10, 0x22, 0x1c, 0xaa, 0x63, 0xb9, 0x7a, 0x52, 0xd3, 0x15, 0x91, 0xa6, 0xf4, 0xd4, 0xa4, 0x27,
    0x51, 0x17, 0x10, 0x01, 0xac, 0x83, 0xf7, 0x95, 0x58, 0xf4, 0x70, 0x29, 0x37, 0x65, 0x63, 0x0f,
    0x5e, 0x77, 0x75, 0xd3, 0x44, 0x83, 0xa1, 0xf0, 0x4e, 0x9b, 0x66, 0xfd, 0x4b, 0x38, 0x65, 0x2b,
    0x5d, 0x9f, 0x4d, 0x3c, 0x2e, 0xb6, 0xc9, 0xe3, 0x10, 0x3f, 0xd4, 0x14, 0x2e, 0x6f, 0x20, 0xe5,
    0x77, 0x1f, 0x92, 0x41, 0xc2, 0x60, 0x23, 0x4a, 0x98, 0xbd, 0x2b, 0x24, 0x3c, 0x23, 0x02, 0xa9,
    0x32, 0x5e, 0x21, 0xe7, 0xbe, 0x2e, 0x56, 0x90, 0xab, 0x49, 0x73, 0x49, 0x7c, 0xf9, 0xd9, 0x8c,
    0x6f, 0x46, 0xb1, 0x13, 0x32, 0x5c, 0x5e, 0x07, 0xea, 0x74, 0x02, 0x45, 0xce, 0x87,
];
