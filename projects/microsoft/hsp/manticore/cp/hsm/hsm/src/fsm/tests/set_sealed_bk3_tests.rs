// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::fsm::set_sealed_bk3::*;
use crate::TagId;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();
    let req = encode_buf::<DdiSetSealedBk3CmdReq, _>(&cmd(), &heap).unwrap();

    let mut cmd = SetSealedBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_decode_request() {
    let heap = MockDmaHeap::new();
    let mut part = MockPartition::new();
    part.expect_get_sealed_bk3_len().times(1).returning(|| 0);

    let req = MockDmaAlloc::new(10);

    let mut cmd = SetSealedBk3Cmd::<MockEnv>::new(req, heap, part);
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

    let cmd = DdiSetSealedBk3CmdReq {
        hdr: DdiReqHdr {
            rev: None,
            op: DdiOp::SetSealedBk3,
            sess_id: Some(SessionId::default()),
        },
        data: DdiSetSealedBk3Req {
            sealed_bk3: MborByteArray::new_with_len(core::ptr::null(), 48),
        },
    };

    let mut part = MockPartition::new();
    part.expect_get_sealed_bk3_len().times(1).returning(|| 0);

    let req = encode_buf::<DdiSetSealedBk3CmdReq, _>(&cmd, &heap).unwrap();

    let mut cmd = SetSealedBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::UnsupportedRevision)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_sealed_bk3_already_set() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_get_sealed_bk3_len().times(1).returning(|| 48);

    let req = encode_buf::<DdiSetSealedBk3CmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = SetSealedBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::SealedBk3AlreadySet)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_sealed_bk3_size_tool_large() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let sealed_bk3 = [0u8; 513];
    let cmd = DdiSetSealedBk3CmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::SetSealedBk3,
            sess_id: Some(SessionId::default()),
        },
        data: DdiSetSealedBk3Req {
            sealed_bk3: MborByteArray::new_with_len(sealed_bk3.as_ptr(), sealed_bk3.len()),
        },
    };

    let mut part = MockPartition::new();
    part.expect_get_sealed_bk3_len().times(1).returning(|| 0);

    let req = encode_buf::<DdiSetSealedBk3CmdReq, _>(&cmd, &heap).unwrap();

    let mut cmd = SetSealedBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::SealedBk3TooLarge)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_heap_alloc_failure() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);

    let mut part = MockPartition::new();
    part.expect_get_sealed_bk3_len().times(1).returning(|| 0);
    let sealed_bk3 = [0u8; 48];
    part.expect_sealed_bk3()
        .times(1)
        .returning(move || sealed_bk3.as_ref().into());

    part.expect_set_sealed_bk3_len().times(1).returning(|_| ());

    part.expect_is_fips_approved().times(1).returning(|| false);

    let req = encode_buf::<DdiSetSealedBk3CmdReq, _>(&cmd(), &heap).unwrap();

    let mut cmd = SetSealedBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_cmd_success() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_get_sealed_bk3_len().times(1).returning(|| 0);
    let sealed_bk3 = [0u8; 48];
    part.expect_sealed_bk3()
        .times(1)
        .returning(move || sealed_bk3.as_ref().into());

    part.expect_set_sealed_bk3_len().times(1).returning(|_| ());

    part.expect_is_fips_approved().times(1).returning(|| false);

    let req = encode_buf::<DdiSetSealedBk3CmdReq, _>(&cmd(), &heap).unwrap();

    let mut cmd = SetSealedBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Ok(())
    );
    assert!(cmd.take_response().is_some());
}

fn cmd() -> DdiSetSealedBk3CmdReq {
    let sealed_bk3 = [0u8; 48];
    DdiSetSealedBk3CmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::SetSealedBk3,
            sess_id: None,
        },
        data: DdiSetSealedBk3Req {
            sealed_bk3: MborByteArray::new_with_len(sealed_bk3.as_ptr(), sealed_bk3.len()),
        },
    }
}
