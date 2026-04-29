// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;
use mcr_types::*;

use super::*;
use crate::cmd_scheduler::*;
use crate::fsm::ecc_sign::EccSignCmd;
use crate::partition::*;
use crate::recorder::HsmFsmEventRecorder;
use crate::resource::HsmFsmResourceId;
use crate::resource::PkaResource;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);

    let digest = [0u8; 96];
    let req = encode_buf::<DdiEccSignCmdReq, _>(&cmd(&digest, 32), &heap).unwrap();

    let mut cmd = EccSignCmd::<MockEnv>::new(req, heap, session, part);
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
    let part = MockPartition::new();
    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);

    let req = MockDmaAlloc::new(10);

    let mut cmd = EccSignCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_ecc_sign() {
    let curve = EccCurve::P256;
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    session.expect_api_rev().once().returning(api_rev);
    session
        .expect_get_key_kind()
        .once()
        .returning(|_k| Ok(EntryKind::Ecc256Private));

    session
        .expect_begin_ecc_sign_zc()
        .once()
        .returning(move |_tag, _key_id, _digest, _digest_hash_algo, _sig| begin_ecc_sign(curve));
    session
        .expect_end_ecc_sign_zc()
        .once()
        .returning(move |_tag, _op| Ok(()));
    let digest = [0u8; 96];
    let req = encode_buf::<DdiEccSignCmdReq, _>(&cmd(&digest, 32), &heap).unwrap();
    let mut cmd = EccSignCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());
}

#[test]
fn test_ecc_sign_on_engine_ready() {
    let curve = EccCurve::P256;
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().times(2).returning(SessionId::default);
    session.expect_api_rev().once().returning(api_rev);
    session
        .expect_get_key_kind()
        .once()
        .returning(|_k| Ok(EntryKind::Ecc256Private));
    session
        .expect_begin_ecc_sign_zc()
        .once()
        .returning(|_tag, _key_id, _digest, _digest_hash_algo, _sig| Err(HsmErr::Pending));
    session
        .expect_begin_ecc_sign_zc()
        .once()
        .returning(move |_tag, _key_id, _digest, _digest_hash_algo, _sig| begin_ecc_sign(curve));
    session
        .expect_end_ecc_sign_zc()
        .once()
        .returning(move |_tag, _op| Ok(()));

    let digest = [0u8; 96];
    let req = encode_buf::<DdiEccSignCmdReq, _>(&cmd(&digest, 32), &heap).unwrap();
    let mut cmd = EccSignCmd::<MockEnv>::new(req, heap, session, part);

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
        Ok(())
    );
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    validate_response(resp);
}

#[test]
fn test_ecc_sign_on_engine_ready_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_api_rev().once().returning(api_rev);
    session.expect_id().times(1).returning(SessionId::default);
    session
        .expect_get_key_kind()
        .once()
        .returning(|_k| Ok(EntryKind::Ecc256Private));
    session
        .expect_begin_ecc_sign_zc()
        .once()
        .returning(|_tag, _key_id, _digest, _digest_hash_algo, _sig| Err(HsmErr::Pending));
    session
        .expect_begin_ecc_sign_zc()
        .once()
        .returning(move |_tag, _key_id, _digest, _digest_hash_algo, _sig| Err(HsmErr::Pending));

    let digest = [0u8; 96];
    let req = encode_buf::<DdiEccSignCmdReq, _>(&cmd(&digest, 32), &heap).unwrap();
    let mut cmd = EccSignCmd::<MockEnv>::new(req, heap, session, part);

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
fn test_invalid_state() {
    let digest = [0u8; 96];
    let cmd: DdiEccSignCmdReq = cmd(&digest, 32);

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_api_rev().once().returning(api_rev);
    session.expect_id().times(2).returning(SessionId::default);
    session
        .expect_get_key_kind()
        .once()
        .returning(|_k| Ok(EntryKind::Ecc256Private));
    session
        .expect_begin_ecc_sign_zc()
        .once()
        .returning(|_tag, _key_id, _digest, _digest_hash_algo, _sig| Err(HsmErr::InvalidArgument));

    let req = encode_buf::<DdiEccSignCmdReq, _>(&cmd, &heap).unwrap();
    let mut cmd = EccSignCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
}

#[test]
fn test_ecc_sign_requires_and_acquire_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let session = MockUserSession::new();

    let digest = [0u8; 96];
    let req = encode_buf::<DdiEccSignCmdReq, _>(&cmd(&digest, 32), &heap).unwrap();
    let mut cmd = EccSignCmd::<MockEnv>::new(req, heap, session, part);
    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::Pka));
    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::Pka)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    );
}

fn cmd(digest: &[u8; 96], len: usize) -> DdiEccSignCmdReq {
    DdiEccSignCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::EccSign,
            sess_id: Some(SessionId::default()),
        },
        data: DdiEccSignReq {
            key_id: 1,
            digest: MborByteArray::new_with_len(digest.as_ptr(), len),
            digest_algo: DdiHashAlgorithm::Sha384,
        },
    }
}

fn begin_ecc_sign(curve: EccCurve) -> HsmResult<EccSign<MockEnv>> {
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
    let engine = resource.acquire(TagId::default(), Some(key_id()));
    let pka_curve: PkaEccCurve = curve.into();
    Ok(EccSign {
        tag: TagId::default(),
        engine_ref: engine.unwrap(),
        curve,
        cmd_info: PkaEccCmd { curve: pka_curve },
    })
}

fn validate_response(resp: Option<MockDmaAlloc>) {
    let digest = [0u8; 96];
    let resp = decode_buf::<DdiEccSignCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd(&digest, 32).hdr.rev);
    assert_eq!(resp.hdr.op, cmd(&digest, 32).hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd(&digest, 32).hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

fn key_id() -> u16 {
    1
}

fn api_rev() -> DdiApiRev {
    DdiApiRev { major: 1, minor: 0 }
}
