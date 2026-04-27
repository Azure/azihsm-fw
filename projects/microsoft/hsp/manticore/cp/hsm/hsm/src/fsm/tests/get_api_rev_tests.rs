// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::cmd_scheduler::TagId;

const MIN_API_REV: DdiApiRev = DdiApiRev { major: 1, minor: 0 };
const MAX_API_REV: DdiApiRev = DdiApiRev { major: 1, minor: 2 };

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let req = encode_buf::<DdiGetApiRevCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = GetApiRevCmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let part = MockPartition::new();
    let req = MockDmaAlloc::new(10);

    let mut cmd = GetApiRevCmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
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
    part.expect_min_api_rev().times(1).return_const(MIN_API_REV);
    part.expect_max_api_rev().times(1).return_const(MAX_API_REV);

    let req = encode_buf::<DdiGetApiRevCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetApiRevCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
}

#[test]
fn test_get_api_rev() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_min_api_rev().times(1).return_const(MIN_API_REV);
    part.expect_max_api_rev().times(1).return_const(MAX_API_REV);

    let req = encode_buf::<DdiGetApiRevCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetApiRevCmd::<MockEnv>::new(req, heap, part);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());

    let resp = decode_buf::<DdiGetApiRevCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert!(resp.hdr.rev.is_none());
    assert_eq!(resp.hdr.op, DdiOp::GetApiRev);
    assert!(resp.hdr.sess_id.is_none());
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.min, MIN_API_REV);
    assert_eq!(resp.data.max, MAX_API_REV);
}

#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_min_api_rev().times(1).return_const(MIN_API_REV);
    part.expect_max_api_rev().times(1).return_const(MAX_API_REV);
    let req = encode_buf::<DdiGetApiRevCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = GetApiRevCmd::<MockEnv>::new(req, heap, part);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.session_id().is_none());

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
}

fn cmd_req() -> DdiGetApiRevCmdReq {
    DdiGetApiRevCmdReq {
        hdr: DdiReqHdr {
            rev: None,
            op: DdiOp::GetApiRev,
            sess_id: None,
        },
        data: DdiGetApiRevReq {},
    }
}
