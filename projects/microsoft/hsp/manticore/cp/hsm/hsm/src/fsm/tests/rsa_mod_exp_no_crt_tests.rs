// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaRsaCmd;
use mcr_crypto_pka::PkaRsaSize;
use mcr_types::*;

use super::*;
use crate::cmd_scheduler::*;
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
    let y = [1u8; 512];
    let req = encode_buf::<DdiRsaModExpCmdReq, _>(&cmd(&y, 256), &heap).unwrap();

    let mut cmd = RsaModExpCmd::<MockEnv>::new(req, heap, session, part);
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

    let mut cmd = RsaModExpCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_rsa_mod_exp() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let is_crt = false;

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
        .expect_begin_rsa_mod_exp_zc()
        .once()
        .returning(move |_tag, _key_id, _key_usage, _in, _out| begin_rsa_mod_exp(rsa_type, is_crt));
    session
        .expect_end_rsa_mod_exp_zc()
        .once()
        .returning(move |_tag, _op| Ok(()));
    let y = [1u8; 512];
    let req = encode_buf::<DdiRsaModExpCmdReq, _>(&cmd(&y, 256), &heap).unwrap();
    let mut cmd = RsaModExpCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());
}

#[test]
fn test_rsa_mod_exp_on_engine_ready() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let is_crt = false;

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
        .expect_begin_rsa_mod_exp_zc()
        .once()
        .returning(|_tag, _key_id, _key_usage, _in, _out| Err(HsmErr::Pending));
    session
        .expect_begin_rsa_mod_exp_zc()
        .once()
        .returning(move |_tag, _key_id, _key_usage, _in, _out| begin_rsa_mod_exp(rsa_type, is_crt));
    session
        .expect_end_rsa_mod_exp_zc()
        .once()
        .returning(move |_tag, _op| Ok(()));

    let y = [1u8; 512];
    let req = encode_buf::<DdiRsaModExpCmdReq, _>(&cmd(&y, 256), &heap).unwrap();
    let mut cmd = RsaModExpCmd::<MockEnv>::new(req, heap, session, part);

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
fn test_rsa_mod_exp_on_engine_ready_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().times(1).returning(SessionId::default);
    session.expect_api_rev().once().returning(api_rev);
    session
        .expect_begin_rsa_mod_exp_zc()
        .once()
        .returning(|_tag, _key_id, _key_usage, _in, _out| Err(HsmErr::Pending));
    session
        .expect_begin_rsa_mod_exp_zc()
        .once()
        .returning(move |_tag, _key_id, _key_usage, _in, _out| Err(HsmErr::Pending));

    let y = [1u8; 512];
    let req = encode_buf::<DdiRsaModExpCmdReq, _>(&cmd(&y, 256), &heap).unwrap();
    let mut cmd = RsaModExpCmd::<MockEnv>::new(req, heap, session, part);

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
    let y = [1u8; 512];
    let cmd: DdiRsaModExpCmdReq = cmd(&y, 256);

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
        .expect_begin_rsa_mod_exp_zc()
        .once()
        .returning(|_tag, _key_id, _key_usage, _in, _out| Err(HsmErr::InvalidArgument));

    let req = encode_buf::<DdiRsaModExpCmdReq, _>(&cmd, &heap).unwrap();
    let mut cmd = RsaModExpCmd::<MockEnv>::new(req, heap, session, part);
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
fn test_rsa_mod_exp_requires_and_acquire_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let session = MockUserSession::new();

    let y = [1u8; 512];
    let req = encode_buf::<DdiRsaModExpCmdReq, _>(&cmd(&y, 256), &heap).unwrap();
    let mut cmd = RsaModExpCmd::<MockEnv>::new(req, heap, session, part);
    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::Pka));
    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::Pka)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    );
}

fn cmd(y: &[u8; 512], len: usize) -> DdiRsaModExpCmdReq {
    DdiRsaModExpCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::RsaModExp,
            sess_id: Some(SessionId::default()),
        },
        data: DdiRsaModExpReq {
            key_id: 1,
            y: MborByteArray::new_with_len(y.as_ptr(), len),
            op_type: DdiRsaOpType::Sign,
        },
    }
}

fn begin_rsa_mod_exp(rsa_type: PkaRsaSize, is_crt: bool) -> HsmResult<RsaModExp<MockEnv>> {
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
    let engine = resource.acquire(TagId::default(), Some(key_id()));

    Ok(RsaModExp {
        tag: TagId::default(),
        engine_ref: engine.unwrap(),
        is_crt,
        cmd_info: PkaRsaCmd { rsa_type },
    })
}

fn validate_response(resp: Option<MockDmaAlloc>) {
    let y = [1u8; 512];
    let resp = decode_buf::<DdiRsaModExpCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd(&y, 256).hdr.rev);
    assert_eq!(resp.hdr.op, cmd(&y, 256).hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd(&y, 256).hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

fn key_id() -> u16 {
    1
}

fn api_rev() -> DdiApiRev {
    DdiApiRev { major: 1, minor: 0 }
}
