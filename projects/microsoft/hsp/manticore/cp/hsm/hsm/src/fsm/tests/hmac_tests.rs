// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::*;
use crate::cmd_scheduler::TagId;
use crate::fsm::hmac::HmacCmd;
use crate::partition::EntryKind;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);

    let msg = [1; 1024];
    let req = encode_buf::<DdiHmacCmdReq, _>(&cmd_req(&msg), &heap).unwrap();

    let mut cmd = HmacCmd::<MockEnv>::new(req, heap, app_session, part);
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
    let part = MockPartition::new();
    app_session.expect_id().once().returning(Default::default);
    let req = MockDmaAlloc::new(1024);

    let mut cmd = HmacCmd::<MockEnv>::new(req, heap, app_session, part);
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
    app_session
        .expect_get_key_kind()
        .once()
        .returning(|_| Ok(EntryKind::HmacSha256));

    let msg = [1; 1024];
    let req = encode_buf::<DdiHmacCmdReq, _>(&cmd_req(&msg), &heap).unwrap();

    let mut cmd = HmacCmd::<MockEnv>::new(req, heap, app_session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_invalid_key_entry_kind() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_get_key_kind()
        .once()
        .returning(|_| Err(HsmErr::InvalidKeyType));

    let msg = [1; 1024];
    let req = encode_buf::<DdiHmacCmdReq, _>(&cmd_req(&msg), &heap).unwrap();

    let mut cmd = HmacCmd::<MockEnv>::new(req, heap, app_session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidKeyType)
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
        .expect_get_key_kind()
        .once()
        .returning(|_| Ok(EntryKind::HmacSha256));
    app_session.expect_hmac().once().returning(|_, _, _| Ok(()));

    let msg = [1; 1024];
    let req = encode_buf::<DdiHmacCmdReq, _>(&cmd_req(&msg), &heap).unwrap();

    let mut cmd = HmacCmd::<MockEnv>::new(req, heap, app_session, part);

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
fn test_hmac_err() {
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
        .expect_get_key_kind()
        .once()
        .returning(|_| Ok(EntryKind::HmacSha256));
    app_session
        .expect_hmac()
        .once()
        .returning(|_, _, _| Err(HsmErr::HmacComputeFailed));

    let msg = [1; 1024];
    let req = encode_buf::<DdiHmacCmdReq, _>(&cmd_req(&msg), &heap).unwrap();

    let mut cmd = HmacCmd::<MockEnv>::new(req, heap, app_session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::HmacComputeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_var_length_hmac() {
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
        .expect_get_key_kind()
        .once()
        .returning(|_| Ok(EntryKind::VarLenHmacSha256));
    app_session
        .expect_var_hmac()
        .once()
        .returning(|_, _, _| Err(HsmErr::HmacComputeFailed));

    let msg = [1; 1024];
    let req = encode_buf::<DdiHmacCmdReq, _>(&cmd_req(&msg), &heap).unwrap();

    let mut cmd = HmacCmd::<MockEnv>::new(req, heap, app_session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::HmacComputeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

fn cmd_req(msg: &[u8]) -> DdiHmacCmdReq {
    DdiHmacCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::Hmac,
            sess_id: Some(SessionId::default()),
        },
        data: DdiHmacReq {
            key_id: 1,
            msg: MborByteArray::new_with_len(msg.as_ptr(), msg.len()),
        },
    }
}
