// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaEccCurve;
use mcr_crypto_pka::PkaEccPublicKey;
use mcr_types::*;

use super::*;
use crate::cmd_scheduler::TagId;
use crate::fsm::HsmFsmResourceId;
use crate::fsm::OpenKeyPhase;
use crate::fsm::PkaConvertible;
use crate::fsm::PublicKey;
use crate::fsm::RsaPubKey;
use crate::fsm::RsaSize;
use crate::partition::store::EntryAttributeFlags;
use crate::partition::EntryKind;
use crate::partition::OpenKeyData;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);
    let req = encode_buf::<DdiOpenKeyCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = OpenKeyCmd::<MockEnv>::new(req, heap, app_session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(!cmd.retry());
    assert!(cmd.rollback(TagId::default()).is_ok());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let part = MockPartition::new();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);
    let req = MockDmaAlloc::new(10);

    let mut cmd = OpenKeyCmd::<MockEnv>::new(req, heap, app_session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(!cmd.retry());
}

#[test]
fn test_encode_buf_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(2).returning(Default::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_open_key_zc()
        .once()
        .returning(|_, _, _, _, _, _, _, _| {
            Ok(OpenKeyData {
                phase: OpenKeyPhase::Done,
                id: 12,
                kind: EntryKind::Aes128,
                flags: EntryAttributeFlags::default(),
                pub_key: None,
                bulk_key_id: None,
            })
        });

    let req = encode_buf::<DdiOpenKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenKeyCmd::<MockEnv>::new(req, heap, app_session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_requires_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();
    let session = MockUserSession::new();

    let req = encode_buf::<DdiOpenKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let cmd = OpenKeyCmd::<MockEnv>::new(req, heap, session, part);

    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::Pka));
}

#[test]
fn test_acquire_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();
    let session = MockUserSession::new();

    let req = encode_buf::<DdiOpenKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenKeyCmd::<MockEnv>::new(req, heap, session, part);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::Pka)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    );
}

#[test]
fn test_open_key_async() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(5).returning(Default::default);
    app_session.expect_api_rev().once().returning(api_rev);

    for phase in [
        OpenKeyPhase::PendingUpkaEngine,
        OpenKeyPhase::PendingMontgomeryConstCalc,
    ] {
        app_session
            .expect_open_key_zc()
            .once()
            .returning(move |_, _, _, _, _, _, _, _| {
                Ok(OpenKeyData {
                    phase,
                    id: 12,
                    kind: EntryKind::Ecc256Private,
                    flags: EntryAttributeFlags::default(),
                    pub_key: None,
                    bulk_key_id: None,
                })
            });
    }
    for phase in [OpenKeyPhase::PendingPointMultiplication, OpenKeyPhase::Done] {
        app_session
            .expect_open_key_zc()
            .once()
            .returning(move |_, _, _, _, _, _, _, _| {
                Ok(OpenKeyData {
                    phase,
                    id: 12,
                    kind: EntryKind::Aes128,
                    flags: EntryAttributeFlags::default(),
                    pub_key: None,
                    bulk_key_id: None,
                })
            });
    }

    let req = encode_buf::<DdiOpenKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenKeyCmd::<MockEnv>::new(req, heap, app_session, part);

    for event in [
        HsmFsmEvent::StartCmd,
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
        HsmFsmEvent::PkaDone(0),
    ] {
        assert_eq!(cmd.on_event(event, TagId::default()), Err(HsmErr::Pending));
        assert!(cmd.session_id().is_some());
    }

    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());

    let resp = decode_buf::<DdiOpenKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, Some(SessionId::default()));
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.key_id, 12);
    assert_eq!(resp.data.key_kind, DdiKeyType::Ecc256Private);

    assert!(resp.data.pub_key.is_some());
    let pub_key_ddi = resp.data.pub_key.unwrap();
    assert_eq!(pub_key_ddi.key_kind, DdiKeyType::Ecc256Public);
    assert_eq!(
        pub_key_ddi.der.as_slice(),
        pub_key().to_pka_bytes().unwrap()
    );
}

#[test]
fn test_open_key_done_no_pub_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(2).returning(Default::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_open_key_zc()
        .once()
        .returning(|_, _, _, _, _, _, _, _| {
            Ok(OpenKeyData {
                phase: OpenKeyPhase::Done,
                id: 12,
                kind: EntryKind::Aes128,
                flags: EntryAttributeFlags::default(),
                pub_key: None,
                bulk_key_id: None,
            })
        });

    let req = encode_buf::<DdiOpenKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenKeyCmd::<MockEnv>::new(req, heap, app_session, part);

    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiOpenKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, Some(SessionId::default()));
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.key_id, 12);
    assert_eq!(resp.data.key_kind, DdiKeyType::Aes128);
    assert_eq!(resp.data.pub_key, None);
}

#[test]
fn test_open_key_done_rsa_pub_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(2).returning(Default::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_open_key_zc()
        .once()
        .returning(|_, _, _, _, _, _, _, _| {
            Ok(OpenKeyData {
                phase: OpenKeyPhase::Done,
                id: 12,
                kind: EntryKind::Rsa2kPrivate,
                flags: EntryAttributeFlags::default(),
                pub_key: Some(PublicKey::RsaPubKey(
                    RsaPubKey::from_priv_pka_slice(&[1u8; 516], RsaSize::Rsa2k).unwrap(),
                )),
                bulk_key_id: None,
            })
        });

    let req = encode_buf::<DdiOpenKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenKeyCmd::<MockEnv>::new(req, heap, app_session, part);

    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiOpenKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, Some(SessionId::default()));
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.key_id, 12);
    assert_eq!(resp.data.key_kind, DdiKeyType::Rsa2kPrivate);
    assert!(resp.data.pub_key.is_some());
    assert_eq!(resp.data.pub_key.unwrap().key_kind, DdiKeyType::Rsa2kPublic);
}

#[test]
fn test_open_key_key_not_found() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);
    app_session
        .expect_open_key_zc()
        .once()
        .returning(|_, _, _, _, _, _, _, _| Err(HsmErr::KeyNotFound));

    let req = encode_buf::<DdiOpenKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenKeyCmd::<MockEnv>::new(req, heap, app_session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::KeyNotFound)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_invalid_state_multiple_calls() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(3).returning(Default::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_open_key_zc()
        .once()
        .returning(|_, _, _, _, _, _, _, _| {
            Ok(OpenKeyData {
                phase: OpenKeyPhase::Done,
                id: 12,
                kind: EntryKind::Aes128,
                flags: EntryAttributeFlags::default(),
                pub_key: None,
                bulk_key_id: None,
            })
        });

    let req = encode_buf::<DdiOpenKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenKeyCmd::<MockEnv>::new(req, heap, app_session, part);

    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiOpenKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, Some(SessionId::default()));
    assert_eq!(resp.hdr.status, DdiStatus::Success);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
}

#[test]
fn test_invalid_state_open_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .once()
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);
    app_session
        .expect_open_key_zc()
        .once()
        .returning(|_, _, _, _, _, _, _, _| {
            Ok(OpenKeyData {
                phase: OpenKeyPhase::Init,
                id: 12,
                kind: EntryKind::Aes128,
                flags: EntryAttributeFlags::default(),
                pub_key: None,
                bulk_key_id: None,
            })
        });

    let req = encode_buf::<DdiOpenKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenKeyCmd::<MockEnv>::new(req, heap, app_session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
}

#[test]
fn test_open_key_invalid_key_kind() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(2).returning(Default::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_open_key_zc()
        .once()
        .returning(|_, _, _, _, _, _, _, _| {
            Ok(OpenKeyData {
                phase: OpenKeyPhase::Done,
                id: 12,
                kind: EntryKind::Free,
                flags: EntryAttributeFlags::default(),
                pub_key: None,
                bulk_key_id: None,
            })
        });

    let req = encode_buf::<DdiOpenKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = OpenKeyCmd::<MockEnv>::new(req, heap, app_session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidKeyType)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

fn pub_key() -> PublicKey {
    PublicKey::EccPubKey(PkaEccPublicKey {
        curve: PkaEccCurve::Ecc256,
        data: [0u8; PkaEccCurve::MAX_LEN * 2],
    })
}

fn api_rev() -> DdiApiRev {
    DdiApiRev { major: 1, minor: 0 }
}

fn cmd_req() -> DdiOpenKeyCmdReq {
    DdiOpenKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::OpenKey,
            sess_id: Some(SessionId::default()),
        },
        data: DdiOpenKeyReq { key_tag: 0 },
    }
}
