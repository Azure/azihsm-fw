// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;
use mcr_types::*;

use super::*;
use crate::cmd_scheduler::TagId;
use crate::partition::*;
use crate::recorder::HsmFsmEventRecorder;
use crate::resource::HsmFsmResourceId;
use crate::resource::PkaResource;
use crate::CmdResource;
use crate::CmdScheduler;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
    assert!(cmd.rollback(TagId::default()).is_ok());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let mut part = MockPartition::new();
    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    let req = MockDmaAlloc::new(10);

    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_encode_buf_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool().once().returning(|_| None);
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::MontgomeryConstCalc)));

    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::PublicKeyValidation)));
    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::EcdhCompute)));

    part.expect_end_open_user_session()
        .times(1)
        .returning(|_, _, _, _, _, _, _| {
            let mut app_session = MockUserSession::new();
            app_session.expect_id().times(2).returning(Default::default);
            app_session
                .expect_app_vault_id()
                .times(2)
                .returning(Default::default);
            Ok(app_session)
        });

    part.expect_generate_bmk_session()
        .times(1)
        .returning(|_, _, _, _| Err(HsmErr::InsufficientBuffer));

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_fips_approved().times(1).returning(|| false);

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_open_app_session() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::MontgomeryConstCalc)));

    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::PublicKeyValidation)));
    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::EcdhCompute)));

    part.expect_end_open_user_session()
        .times(1)
        .returning(|_, _, _, _, _, _, _| {
            let mut app_session = MockUserSession::new();
            app_session.expect_id().times(2).returning(Default::default);
            app_session
                .expect_app_vault_id()
                .times(2)
                .returning(Default::default);
            Ok(app_session)
        });

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_generate_bmk_session()
        .times(1)
        .returning(|_, _, _, _| Err(HsmErr::InsufficientBuffer));
    part.expect_generate_bmk_session()
        .times(1)
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
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

    let resp = decode_buf::<DdiOpenSessionCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, Some(SessionId::default()));
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_open_app_session_on_engine_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Err(HsmErr::Pending));
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::MontgomeryConstCalc)));
    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::PublicKeyValidation)));
    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::EcdhCompute)));

    part.expect_end_open_user_session()
        .times(1)
        .returning(|_, _, _, _, _, _, _| {
            let mut app_session = MockUserSession::new();
            app_session.expect_id().times(2).returning(Default::default);
            app_session
                .expect_app_vault_id()
                .times(2)
                .returning(Default::default);
            Ok(app_session)
        });

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_generate_bmk_session()
        .times(1)
        .returning(|_, _, _, _| Err(HsmErr::InsufficientBuffer));
    part.expect_generate_bmk_session()
        .times(1)
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

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
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
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

    let resp = decode_buf::<DdiOpenSessionCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, Some(SessionId::default()));
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_open_app_session_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::MontgomeryConstCalc)));

    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::PublicKeyValidation)));
    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::EcdhCompute)));
    part.expect_end_open_user_session()
        .times(1)
        .returning(|_, _, _, _, _, _, _| Err(HsmErr::InvalidUserCredential));

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::InvalidUserCredential)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_open_app_session_rollback() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::MontgomeryConstCalc)));

    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::PublicKeyValidation)));
    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::EcdhCompute)));

    part.expect_end_open_user_session()
        .times(1)
        .returning(|_, _, _, _, _, _, _| {
            let mut app_session = MockUserSession::new();
            app_session.expect_id().times(2).returning(Default::default);
            app_session
                .expect_app_vault_id()
                .times(2)
                .returning(Default::default);
            Ok(app_session)
        });

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_rollback_open_session()
        .times(1)
        .returning(|_, _| Ok(()));

    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_generate_bmk_session()
        .times(1)
        .returning(|_, _, _, _| Err(HsmErr::InsufficientBuffer));
    part.expect_generate_bmk_session()
        .times(1)
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
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

    assert!(cmd.rollback(TagId::default()).is_ok());
}

#[test]
fn test_open_app_session_rollback_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::MontgomeryConstCalc)));

    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::PublicKeyValidation)));
    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::EcdhCompute)));

    part.expect_end_open_user_session()
        .times(1)
        .returning(|_, _, _, _, _, _, _| {
            let mut app_session = MockUserSession::new();
            app_session.expect_id().times(2).returning(Default::default);
            app_session
                .expect_app_vault_id()
                .times(2)
                .returning(Default::default);
            Ok(app_session)
        });

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_rollback_open_session()
        .times(1)
        .returning(|_, _| Err(HsmErr::InvalidArgument));

    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_generate_bmk_session()
        .times(1)
        .returning(|_, _, _, _| Err(HsmErr::InsufficientBuffer));
    part.expect_generate_bmk_session()
        .times(1)
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
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

    assert!(cmd.rollback(TagId::default()).is_err());
}

#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Err(HsmErr::InvalidArgument));

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_resource_pending_twice_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Err(HsmErr::Pending));
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Err(HsmErr::Pending));

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

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
fn test_requires_and_acquire_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);
    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::Pka));
    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::Pka)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    );
}

#[test]
fn test_app_vault_id() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::MontgomeryConstCalc)));

    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::PublicKeyValidation)));
    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::EcdhCompute)));

    part.expect_end_open_user_session()
        .times(1)
        .returning(|_, _, _, _, _, _, _| {
            let mut app_session = MockUserSession::new();
            app_session.expect_id().times(2).returning(Default::default);
            app_session.expect_app_vault_id().times(2).returning(|| 9);
            Ok(app_session)
        });

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_generate_bmk_session()
        .times(1)
        .returning(|_, _, _, _| Err(HsmErr::InsufficientBuffer));
    part.expect_generate_bmk_session()
        .times(1)
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

    assert_eq!(cmd.app_vault_id(), None);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    assert_eq!(cmd.app_vault_id(), Some(9));

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiOpenSessionCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, Some(SessionId::default()));
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_open_app_session_on_continue_err_non_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::MontgomeryConstCalc)));

    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Err(HsmErr::InvalidArgument));

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::InvalidArgument)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_open_session_on_start_fails_with_null_pin() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));
    let req = DdiOpenSessionCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::OpenSession,
            sess_id: Some(SessionId::default()),
        },
        data: DdiOpenSessionReq {
            encrypted_credential: DdiEncryptedSessionCredential {
                encrypted_id: MborByteArray::new_with_len(core::ptr::null(), 16),
                encrypted_pin: MborByteArray::new_with_len(core::ptr::null(), 0),
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

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&req, &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_open_app_session_on_continue_err_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::MontgomeryConstCalc)));

    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Err(HsmErr::Pending));

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::InvalidState)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_open_app_session_on_err_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::MontgomeryConstCalc)));

    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::PublicKeyValidation)));
    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::EcdhCompute)));

    part.expect_end_open_user_session()
        .times(1)
        .returning(|_, _, _, _, _, _, _| Err(HsmErr::Pending));

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::InvalidState)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_dma_alloc_failure_1() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate().times(1).returning(|_| None);

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::MontgomeryConstCalc)));

    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::PublicKeyValidation)));
    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::EcdhCompute)));

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_dma_alloc_failure_2() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate().times(1).returning(|_| None);

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::MontgomeryConstCalc)));

    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::PublicKeyValidation)));
    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::EcdhCompute)));

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_generate_bmk_session_failure_1() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::MontgomeryConstCalc)));

    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::PublicKeyValidation)));
    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::EcdhCompute)));
    part.expect_end_open_user_session()
        .times(1)
        .returning(|_, _, _, _, _, _, _| {
            let mut app_session = MockUserSession::new();
            app_session.expect_id().times(2).returning(Default::default);
            app_session.expect_app_vault_id().times(2).returning(|| 9);
            Ok(app_session)
        });

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_generate_bmk_session()
        .times(1)
        .returning(|_, _, _, _| Err(HsmErr::MetadataEncodeFailed));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::MetadataEncodeFailed)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_generate_bmk_session_failure_2() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::MontgomeryConstCalc)));

    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::PublicKeyValidation)));
    part.expect_continue_open_user_session()
        .times(1)
        .returning(|_, _| Ok(open_session_ctx(OpenSessionCmdState::EcdhCompute)));
    part.expect_end_open_user_session()
        .times(1)
        .returning(|_, _, _, _, _, _, _| {
            let mut app_session = MockUserSession::new();
            app_session.expect_id().times(2).returning(Default::default);
            app_session.expect_app_vault_id().times(2).returning(|| 9);
            Ok(app_session)
        });

    part.expect_verify_cred_is_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_generate_bmk_session()
        .times(1)
        .returning(|_, _, _, _| Err(HsmErr::InsufficientBuffer));
    part.expect_generate_bmk_session()
        .times(1)
        .returning(|_, _, _, _| Err(HsmErr::MetadataEncodeFailed));

    let req = encode_buf::<DdiOpenSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenSessionCmd::<MockEnv>::new(req, heap, part, false);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::MetadataEncodeFailed)
    );

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

fn cmd_req() -> DdiOpenSessionCmdReq {
    DdiOpenSessionCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::OpenSession,
            sess_id: Some(SessionId::default()),
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
    }
}

fn open_session_ctx(state: OpenSessionCmdState) -> OpenSessionCtx<MockEnv> {
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
    let engine = resource.acquire(TagId::default(), Some(key_id()));
    let pka_curve = PkaEccCurve::Ecc384;
    OpenSessionCtx {
        tag: TagId::default(),
        engine_ref: engine.unwrap(),
        cmd_info: PkaEccCmd { curve: pka_curve },
        state,
    }
}

fn key_id() -> u16 {
    1
}
