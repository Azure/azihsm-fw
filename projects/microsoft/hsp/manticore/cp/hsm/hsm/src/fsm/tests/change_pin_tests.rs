// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;
use mcr_types::*;

use super::*;
use crate::cmd_scheduler::TagId;
use crate::fsm::ChangePinCmd;
use crate::partition::ChangePinCmdCtx;
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

    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);
    let req = encode_buf::<DdiChangePinCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = ChangePinCmd::<MockEnv>::new(req, heap, app_session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(cmd.retry());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let part = MockPartition::new();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);
    let req = MockDmaAlloc::new(10);

    let mut cmd = ChangePinCmd::<MockEnv>::new(req, heap, app_session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(cmd.retry());
}

#[test]
fn test_encode_buf_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool().once().returning(|_| None);

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_change_pin()
        .times(1)
        .returning(|_| Ok(ctx()));
    app_session
        .expect_continue_change_pin()
        .times(1)
        .returning(|_, _| Ok(ctx()));
    app_session
        .expect_end_change_pin()
        .times(1)
        .returning(|_, _| Ok(()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });

    let req = encode_buf::<DdiChangePinCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = ChangePinCmd::<MockEnv>::new(req, heap, app_session, part);

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
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_change_pin() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_change_pin()
        .times(1)
        .returning(|_| Ok(ctx()));
    app_session
        .expect_continue_change_pin()
        .times(1)
        .returning(|_, _| Ok(ctx()));
    app_session
        .expect_end_change_pin()
        .times(1)
        .returning(|_, _| Ok(()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });

    let req = encode_buf::<DdiChangePinCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = ChangePinCmd::<MockEnv>::new(req, heap, app_session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());
    assert!(cmd.take_response().is_some());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_change_pin_on_engine_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_change_pin()
        .times(1)
        .returning(|_| Err(HsmErr::Pending));
    app_session
        .expect_begin_change_pin()
        .times(1)
        .returning(|_| Ok(ctx()));
    app_session
        .expect_continue_change_pin()
        .times(1)
        .returning(|_, _| Ok(ctx()));
    app_session
        .expect_end_change_pin()
        .times(1)
        .returning(|_, _| Ok(()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });

    let req = encode_buf::<DdiChangePinCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = ChangePinCmd::<MockEnv>::new(req, heap, app_session, part);

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
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());
    assert!(cmd.take_response().is_some());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_change_pin()
        .times(1)
        .returning(|_| Err(HsmErr::InvalidArgument));
    app_session.expect_id().times(1).returning(Default::default);

    let req = encode_buf::<DdiChangePinCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = ChangePinCmd::<MockEnv>::new(req, heap, app_session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(cmd.retry());
}

#[test]
fn test_resource_pending_twice_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_change_pin()
        .times(1)
        .returning(|_| Err(HsmErr::Pending));
    app_session
        .expect_begin_change_pin()
        .times(1)
        .returning(|_| Err(HsmErr::Pending));
    app_session.expect_id().times(1).returning(Default::default);

    let req = encode_buf::<DdiChangePinCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = ChangePinCmd::<MockEnv>::new(req, heap, app_session, part);

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
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(cmd.retry());
}

#[test]
fn test_on_continue_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_change_pin()
        .times(1)
        .returning(|_| Ok(ctx()));
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_continue_change_pin()
        .times(1)
        .returning(|_, _| Err(HsmErr::Pending));

    let req = encode_buf::<DdiChangePinCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = ChangePinCmd::<MockEnv>::new(req, heap, app_session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(cmd.retry());
}

#[test]
fn test_on_end_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_change_pin()
        .times(1)
        .returning(|_| Ok(ctx()));
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_continue_change_pin()
        .times(1)
        .returning(|_, _| Ok(ctx()));
    app_session
        .expect_end_change_pin()
        .times(1)
        .returning(|_, _| Err(HsmErr::Pending));

    let req = encode_buf::<DdiChangePinCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = ChangePinCmd::<MockEnv>::new(req, heap, app_session, part);

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
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(cmd.retry());
}

#[test]
fn test_requires_and_acquire_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let app_session = MockUserSession::new();

    let req = encode_buf::<DdiChangePinCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = ChangePinCmd::<MockEnv>::new(req, heap, app_session, part);
    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::Pka));
    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::Pka)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    );
}

fn cmd_req() -> DdiChangePinCmdReq {
    DdiChangePinCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::ChangePin,
            sess_id: Some(SessionId::default()),
        },
        data: DdiChangePinReq {
            new_pin: DdiEncryptedPin {
                encrypted_pin: MborByteArray::new_with_len(core::ptr::null(), 16),
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

fn ctx() -> ChangePinCmdCtx<MockEnv> {
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
    let engine = resource.acquire(TagId::default(), Some(key_id()));
    let pka_curve = PkaEccCurve::Ecc384;
    ChangePinCmdCtx {
        tag: TagId::default(),
        engine_ref: engine.unwrap(),
        cmd_info: PkaEccCmd { curve: pka_curve },
    }
}

fn key_id() -> u16 {
    1
}
