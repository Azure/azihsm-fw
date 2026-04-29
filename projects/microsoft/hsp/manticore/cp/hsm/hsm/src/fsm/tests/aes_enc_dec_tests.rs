// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::*;
use crate::cmd_scheduler::TagId;
use crate::fsm::aes_enc_dec::AesEncDecCmd;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);

    let part = MockPartition::new();

    let msg = [1; 512];
    let iv = [0; 16];
    let req = encode_buf::<DdiAesEncryptDecryptCmdReq, _>(&cmd_req(&msg, &iv), &heap).unwrap();

    let mut cmd = AesEncDecCmd::<MockEnv>::new(req, heap, app_session, part);
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
    let part = MockPartition::new();

    let mut cmd = AesEncDecCmd::<MockEnv>::new(req, heap, app_session, part);
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
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session.expect_id().times(2).returning(Default::default);

    let msg = [1; 512];
    let iv = [0; 16];
    let req = encode_buf::<DdiAesEncryptDecryptCmdReq, _>(&cmd_req(&msg, &iv), &heap).unwrap();

    let mut cmd = AesEncDecCmd::<MockEnv>::new(req, heap, app_session, part);

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
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session.expect_id().times(3).returning(Default::default);
    app_session
        .expect_aes_enc_dec()
        .once()
        .returning(|_, _, _| Ok(()));

    let msg = [1; 512];
    let iv = [0; 16];
    let req = encode_buf::<DdiAesEncryptDecryptCmdReq, _>(&cmd_req(&msg, &iv), &heap).unwrap();

    let mut cmd = AesEncDecCmd::<MockEnv>::new(req, heap, app_session, part);

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
fn test_aes_enc_dec() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_aes_enc_dec()
        .once()
        .returning(|_, _, _| Ok(()));

    let msg = [1; 512];
    let iv = [0; 16];
    let req = encode_buf::<DdiAesEncryptDecryptCmdReq, _>(&cmd_req(&msg, &iv), &heap).unwrap();

    let mut cmd = AesEncDecCmd::<MockEnv>::new(req, heap, app_session, part);

    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiAesEncryptDecryptCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req(&msg, &iv).hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req(&msg, &iv).hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req(&msg, &iv).hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_aes_enc_dec_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_aes_enc_dec()
        .once()
        .returning(|_, _, _| Err(HsmErr::InvalidArgument));

    let msg = [1; 512];
    let iv = [0; 16];
    let req = encode_buf::<DdiAesEncryptDecryptCmdReq, _>(&cmd_req(&msg, &iv), &heap).unwrap();

    let mut cmd = AesEncDecCmd::<MockEnv>::new(req, heap, app_session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

fn cmd_req(msg: &[u8], iv: &[u8; 16]) -> DdiAesEncryptDecryptCmdReq {
    DdiAesEncryptDecryptCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::AesEncryptDecrypt,
            sess_id: Some(SessionId::default()),
        },
        data: DdiAesEncryptDecryptReq {
            key_id: 1,
            op: DdiAesOp::Encrypt,
            msg: MborByteArray::new_with_len(msg.as_ptr(), msg.len()),
            iv: MborByteArray::new_with_len(iv.as_ptr(), 16),
        },
    }
}
