// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::*;
use crate::cmd_scheduler::TagId;
use crate::fsm::SoftAesCmd;
use core::ops::Range;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);

    let key = [1_u8; 24];
    let msg = [0_u8; 16];
    let op = SoftAesOp::Kwp;

    let req = encode_buf::<DdiSoftAesCmdReq, _>(&cmd_req(&key, &msg, op.into()), &heap).unwrap();

    let mut cmd = SoftAesCmd::<MockEnv>::new(req, heap, app_session);
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
    let req = MockDmaAlloc::new(10);

    let mut cmd = SoftAesCmd::<MockEnv>::new(req, heap, app_session);
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

    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_begin_soft_aes()
        .once()
        .returning(|_, _, _, _| Err(HsmErr::Pending));
    app_session.expect_end_soft_aes().once().returning(|_| {
        Ok(Range {
            start: 8,
            end: 8 + 16,
        })
    });

    let key = [1_u8; 32];
    let msg = [0_u8; 32];
    let op = SoftAesOp::Kwp;

    let req = encode_buf::<DdiSoftAesCmdReq, _>(&cmd_req(&key, &msg, op.into()), &heap).unwrap();

    let mut cmd = SoftAesCmd::<MockEnv>::new(req, heap, app_session);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::SoftAesResp, TagId::default()),
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
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_begin_soft_aes()
        .once()
        .returning(|_, _, _, _| Err(HsmErr::Pending));
    app_session
        .expect_end_soft_aes()
        .once()
        .returning(|_| Err(HsmErr::Pending));

    let key = [1_u8; 32];
    let msg = [0_u8; 16];
    let op = SoftAesOp::Kwp;

    let req = encode_buf::<DdiSoftAesCmdReq, _>(&cmd_req(&key, &msg, op.into()), &heap).unwrap();

    let mut cmd = SoftAesCmd::<MockEnv>::new(req, heap, app_session);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::SoftAesResp, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::SoftAesResp, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
}

#[test]
fn test_soft_aes_kwp() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_begin_soft_aes()
        .once()
        .returning(|_, _, _, _| Err(HsmErr::Pending));
    app_session.expect_end_soft_aes().once().returning(|_| {
        Ok(Range {
            start: 8,
            end: 8 + 16,
        })
    });

    let key = [1_u8; 32];
    let msg = [0_u8; 32];
    let op = SoftAesOp::Kwp;

    let req = encode_buf::<DdiSoftAesCmdReq, _>(&cmd_req(&key, &msg, op.into()), &heap).unwrap();

    let mut cmd = SoftAesCmd::<MockEnv>::new(req, heap, app_session);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::SoftAesResp, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiSoftAesCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req(&key, &msg, op.into()).hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req(&key, &msg, op.into()).hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req(&key, &msg, op.into()).hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_soft_aes_kwp_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_begin_soft_aes()
        .once()
        .returning(|_, _, _, _| Err(HsmErr::InvalidArgument));

    let key = [1_u8; 24];
    let msg = [0_u8; 16];
    let op = SoftAesOp::Kwp;

    let req = encode_buf::<DdiSoftAesCmdReq, _>(&cmd_req(&key, &msg, op.into()), &heap).unwrap();

    let mut cmd = SoftAesCmd::<MockEnv>::new(req, heap, app_session);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_soft_aes_ecb_decrypt() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_begin_soft_aes()
        .once()
        .returning(|_, _, _, _| Err(HsmErr::Pending));
    app_session
        .expect_end_soft_aes()
        .once()
        .returning(|_| Ok(Range { start: 0, end: 16 }));

    let key = [1_u8; 24];
    let msg = [0_u8; 16];
    let op = SoftAesOp::EcbDecrypt;

    let req = encode_buf::<DdiSoftAesCmdReq, _>(&cmd_req(&key, &msg, op.into()), &heap).unwrap();

    let mut cmd = SoftAesCmd::<MockEnv>::new(req, heap, app_session);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::SoftAesResp, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiSoftAesCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req(&key, &msg, op.into()).hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req(&key, &msg, op.into()).hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req(&key, &msg, op.into()).hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_soft_aes_ecb_decrypt_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_begin_soft_aes()
        .once()
        .returning(|_, _, _, _| Err(HsmErr::InvalidArgument));

    let key = [1_u8; 24];
    let msg = [0_u8; 16];
    let op = SoftAesOp::EcbDecrypt;

    let req = encode_buf::<DdiSoftAesCmdReq, _>(&cmd_req(&key, &msg, op.into()), &heap).unwrap();

    let mut cmd = SoftAesCmd::<MockEnv>::new(req, heap, app_session);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

fn cmd_req(key: &[u8], msg: &[u8], req_op: DdiSoftAesOp) -> DdiSoftAesCmdReq {
    DdiSoftAesCmdReq {
        hdr: DdiReqHdr {
            op: DdiOp::SoftAes,
            sess_id: Some(SessionId::default()),
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
        },
        data: DdiSoftAesReq {
            key: MborByteArray::new_with_len(key.as_ptr(), key.len()),
            inout: MborByteArray::new_with_len(msg.as_ptr(), msg.len()),
            op: req_op,
        },
    }
}
