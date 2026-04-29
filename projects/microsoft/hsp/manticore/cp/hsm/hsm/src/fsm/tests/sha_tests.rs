// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::*;
use crate::cmd_scheduler::TagId;
use crate::fsm::sha::ShaDigestCmd;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);

    let msg = [1; 1024];
    let req =
        encode_buf::<DdiShaDigestGenerateCmdReq, _>(&cmd_req(&msg, DdiHashAlgorithm::Sha1), &heap)
            .unwrap();

    let mut cmd = ShaDigestCmd::<MockEnv>::new(req, heap, app_session);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);
    let req = MockDmaAlloc::new(1024);

    let mut cmd = ShaDigestCmd::<MockEnv>::new(req, heap, app_session);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
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
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_sha_single_block_zc()
        .once()
        .returning(|_, _, _| Ok(()));

    let msg = [1; 1024];
    let req = encode_buf::<DdiShaDigestGenerateCmdReq, _>(
        &cmd_req(&msg, DdiHashAlgorithm::Sha256),
        &heap,
    )
    .unwrap();

    let mut cmd = ShaDigestCmd::<MockEnv>::new(req, heap, app_session);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session.expect_id().times(3).returning(Default::default);
    app_session
        .expect_sha_single_block_zc()
        .once()
        .returning(|_, _, _| Ok(()));

    let msg = [1; 1024];
    let req =
        encode_buf::<DdiShaDigestGenerateCmdReq, _>(&cmd_req(&msg, DdiHashAlgorithm::Sha1), &heap)
            .unwrap();

    let mut cmd = ShaDigestCmd::<MockEnv>::new(req, heap, app_session);

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
fn test_sha_digest_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_sha_single_block_zc()
        .once()
        .returning(|_, _, _| Err(HsmErr::ShaCmdFailed));

    let msg = [1; 1024];
    let req = encode_buf::<DdiShaDigestGenerateCmdReq, _>(
        &cmd_req(&msg, DdiHashAlgorithm::Sha256),
        &heap,
    )
    .unwrap();

    let mut cmd = ShaDigestCmd::<MockEnv>::new(req, heap, app_session);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::ShaCmdFailed)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_sha_digest_sha384() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_sha_single_block_zc()
        .once()
        .returning(|_, _, _| Ok(()));

    let msg = [1; 1024];
    let req = encode_buf::<DdiShaDigestGenerateCmdReq, _>(
        &cmd_req(&msg, DdiHashAlgorithm::Sha384),
        &heap,
    )
    .unwrap();

    let mut cmd = ShaDigestCmd::<MockEnv>::new(req, heap, app_session);

    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiShaDigestGenerateCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(
        resp.hdr.rev,
        cmd_req(&msg, DdiHashAlgorithm::Sha384).hdr.rev
    );
    assert_eq!(resp.hdr.op, cmd_req(&msg, DdiHashAlgorithm::Sha384).hdr.op);
    assert_eq!(
        resp.hdr.sess_id,
        cmd_req(&msg, DdiHashAlgorithm::Sha384).hdr.sess_id
    );
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_sha_digest_sha512() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_sha_single_block_zc()
        .once()
        .returning(|_, _, _| Ok(()));

    let msg = [1; 1024];
    let req = encode_buf::<DdiShaDigestGenerateCmdReq, _>(
        &cmd_req(&msg, DdiHashAlgorithm::Sha512),
        &heap,
    )
    .unwrap();

    let mut cmd = ShaDigestCmd::<MockEnv>::new(req, heap, app_session);

    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiShaDigestGenerateCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(
        resp.hdr.rev,
        cmd_req(&msg, DdiHashAlgorithm::Sha512).hdr.rev
    );
    assert_eq!(resp.hdr.op, cmd_req(&msg, DdiHashAlgorithm::Sha512).hdr.op);
    assert_eq!(
        resp.hdr.sess_id,
        cmd_req(&msg, DdiHashAlgorithm::Sha512).hdr.sess_id
    );
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

fn cmd_req(msg: &[u8], sha_mode: DdiHashAlgorithm) -> DdiShaDigestGenerateCmdReq {
    DdiShaDigestGenerateCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::ShaDigest,
            sess_id: Some(SessionId::default()),
        },
        data: DdiShaDigestGenerateReq {
            sha_mode,
            msg: MborByteArray::new_with_len(msg.as_ptr(), msg.len()),
        },
    }
}
