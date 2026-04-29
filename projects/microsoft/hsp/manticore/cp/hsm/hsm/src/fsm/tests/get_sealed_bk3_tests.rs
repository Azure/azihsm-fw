// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::fsm::get_sealed_bk3::*;

use crate::TagId;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();
    let req = encode_buf::<DdiGetSealedBk3CmdReq, _>(&cmd(), &heap).unwrap();

    let mut cmd = GetSealedBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_decode_request() {
    let heap = MockDmaHeap::new();
    let part = MockPartition::new();
    let req = MockDmaAlloc::new(10);

    let mut cmd = GetSealedBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_rev_error() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let cmd = DdiGetSealedBk3CmdReq {
        hdr: DdiReqHdr {
            rev: None,
            op: DdiOp::GetSealedBk3,
            sess_id: Some(SessionId::default()),
        },
        data: DdiGetSealedBk3Req {},
    };

    let part = MockPartition::new();
    let req = encode_buf::<DdiGetSealedBk3CmdReq, _>(&cmd, &heap).unwrap();

    let mut cmd = GetSealedBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::UnsupportedRevision)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_invalid_sealed_bk3() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_get_sealed_bk3_len().times(1).returning(|| 0u32);

    let req = encode_buf::<DdiGetSealedBk3CmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSealedBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::SealedBk3NotPresent)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_encode_failure() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);

    let mut part = MockPartition::new();
    part.expect_get_sealed_bk3_len()
        .times(1)
        .returning(|| 64u32);
    part.expect_is_fips_approved().times(1).returning(|| true);

    let sealed_bk3 = [0u8; 48];
    part.expect_sealed_bk3()
        .times(1)
        .returning(move || sealed_bk3.as_ref().into());

    let req = encode_buf::<DdiGetSealedBk3CmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSealedBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_encode_success() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_get_sealed_bk3_len()
        .times(1)
        .returning(|| 64u32);
    part.expect_is_fips_approved().times(1).returning(|| true);

    let sealed_bk3 = [0u8; 48];
    part.expect_sealed_bk3()
        .times(1)
        .returning(move || sealed_bk3.as_ref().into());

    let req = encode_buf::<DdiGetSealedBk3CmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSealedBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Ok(())
    );
    assert!(cmd.take_response().is_some());
}

fn cmd() -> DdiGetSealedBk3CmdReq {
    DdiGetSealedBk3CmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::GetSealedBk3,
            sess_id: None,
        },
        data: DdiGetSealedBk3Req {},
    }
}
