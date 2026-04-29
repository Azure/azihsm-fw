// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::cmd_scheduler::TagId;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let req = encode_buf::<DdiGetDeviceInfoCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = GetDeviceInfoCmd::<MockEnv>::new(req, heap, part);
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

    let mut cmd = GetDeviceInfoCmd::<MockEnv>::new(req, heap, part);
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
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_resource_mask()
        .times(1)
        .return_const(0b101010100011u128);

    let req = encode_buf::<DdiGetDeviceInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetDeviceInfoCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
}

#[test]
fn test_get_device_info() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_resource_mask()
        .times(1)
        .return_const(0b101010100011u128);

    let req = encode_buf::<DdiGetDeviceInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetDeviceInfoCmd::<MockEnv>::new(req, heap, part);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());

    let resp = decode_buf::<DdiGetDeviceInfoCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert!(resp.hdr.rev.is_none());
    assert_eq!(resp.hdr.op, DdiOp::GetDeviceInfo);
    assert!(resp.hdr.sess_id.is_none());
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.kind, DdiDeviceKind::Physical);
    assert_eq!(resp.data.tables, 6);
}

#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_resource_mask()
        .times(1)
        .return_const(0b101010100011u128);

    let req = encode_buf::<DdiGetDeviceInfoCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = GetDeviceInfoCmd::<MockEnv>::new(req, heap, part);
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

fn cmd_req() -> DdiGetDeviceInfoCmdReq {
    DdiGetDeviceInfoCmdReq {
        hdr: DdiReqHdr {
            rev: None,
            op: DdiOp::GetDeviceInfo,
            sess_id: None,
        },
        data: DdiGetDeviceInfoReq {},
    }
}
