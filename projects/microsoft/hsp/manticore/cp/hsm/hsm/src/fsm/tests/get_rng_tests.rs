// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::*;
use crate::cmd_scheduler::TagId;
use crate::fsm::get_rng::GetRngCmd;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);

    let req = encode_buf::<DdiGetRngGenerateCmdReq, _>(&cmd_req(32u8), &heap).unwrap();

    let mut cmd = GetRngCmd::<MockEnv>::new(req, heap, app_session);

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

    let mut cmd = GetRngCmd::<MockEnv>::new(req, heap, app_session);

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
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session.expect_id().times(2).returning(Default::default);

    let req = encode_buf::<DdiGetRngGenerateCmdReq, _>(&cmd_req(32u8), &heap).unwrap();

    let mut cmd = GetRngCmd::<MockEnv>::new(req, heap, app_session);

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

    let mut app_session = MockUserSession::new();
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session.expect_id().times(3).returning(Default::default);
    app_session
        .expect_get_random_number()
        .once()
        .returning(|_| Ok(()));

    let req = encode_buf::<DdiGetRngGenerateCmdReq, _>(&cmd_req(32u8), &heap).unwrap();

    let mut cmd = GetRngCmd::<MockEnv>::new(req, heap, app_session);

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
fn test_get_rng_32_bytes() {
    run_get_rng_success(32u8);
}

#[test]
fn test_get_rng_64_bytes() {
    run_get_rng_success(64u8);
}

#[test]
fn test_get_rng_64_bytes_limit_exceed_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);
    let req = encode_buf::<DdiGetRngGenerateCmdReq, _>(&cmd_req(65u8), &heap).unwrap();
    let mut cmd = GetRngCmd::<MockEnv>::new(req, heap, app_session);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

fn run_get_rng_success(expected_size: u8) {
    // let (heap, app_session) = setup_mock_environment();
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_get_random_number()
        .once()
        .returning(|_| Ok(()));

    let req = encode_buf::<DdiGetRngGenerateCmdReq, _>(&cmd_req(expected_size), &heap).unwrap();
    let mut cmd = GetRngCmd::<MockEnv>::new(req, heap, app_session);

    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiGetRngGenerateCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req(expected_size).hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req(expected_size).hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req(expected_size).hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

fn cmd_req(rng_len: u8) -> DdiGetRngGenerateCmdReq {
    DdiGetRngGenerateCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::GetRandomNumber,
            sess_id: Some(SessionId::default()),
        },
        data: DdiGetRngGenerateReq { rng_len },
    }
}
