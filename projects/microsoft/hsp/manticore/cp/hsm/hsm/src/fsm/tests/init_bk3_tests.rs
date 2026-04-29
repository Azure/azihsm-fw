// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::fsm::init_bk3::InitBk3Cmd;

use crate::TagId;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();
    let req = encode_buf::<DdiInitBk3CmdReq, _>(&cmd(), &heap).unwrap();

    let mut cmd = InitBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let mut part = MockPartition::new();
    part.expect_get_masked_bk_boot_len()
        .times(1)
        .returning(|| 0);

    let req = MockDmaAlloc::new(10);

    let mut cmd = InitBk3Cmd::<MockEnv>::new(req, heap, part);
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

    let cmd = DdiInitBk3CmdReq {
        hdr: DdiReqHdr {
            rev: None,
            op: DdiOp::InitBk3,
            sess_id: Some(SessionId::default()),
        },
        data: DdiInitBk3Req {
            bk3: MborByteArray::new_with_len(core::ptr::null(), 48),
        },
    };

    let mut part = MockPartition::new();
    part.expect_get_masked_bk_boot_len()
        .times(1)
        .returning(|| 0);

    let req = encode_buf::<DdiInitBk3CmdReq, _>(&cmd, &heap).unwrap();

    let mut cmd = InitBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::UnsupportedRevision)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_init_bk3_already_done() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();

    part.expect_get_masked_bk_boot_len()
        .times(1)
        .returning(|| 100);

    let req = encode_buf::<DdiInitBk3CmdReq, _>(&cmd(), &heap).unwrap();

    let mut cmd = InitBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Bk3AlreadyInitialized)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_bk3_mask_fail() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_generate_bk_boot().times(1).returning(|buf| {
        for b in buf.iter_mut() {
            *b = 0xA5;
        }
        Ok(())
    });

    part.expect_get_masked_bk_boot_len()
        .times(1)
        .returning(|| 0);

    part.expect_mask_bk3()
        .times(1)
        .return_const(Err(HsmErr::MaskingBk3Failed));

    let req = encode_buf::<DdiInitBk3CmdReq, _>(&cmd(), &heap).unwrap();

    let mut cmd = InitBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::MaskingBk3Failed)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_bk3_mask_fail_2() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_generate_bk_boot().times(1).returning(|buf| {
        for b in buf.iter_mut() {
            *b = 0xA5;
        }
        Ok(())
    });

    part.expect_get_masked_bk_boot_len()
        .times(1)
        .returning(|| 0);

    part.expect_mask_bk3()
        .times(1)
        .return_const(Err(HsmErr::InsufficientBuffer));

    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_vm_launch_guid().times(1).returning(|| [1; 16]);

    part.expect_mask_bk3()
        .times(1)
        .return_const(Err(HsmErr::MaskingBk3Failed));

    let req = encode_buf::<DdiInitBk3CmdReq, _>(&cmd(), &heap).unwrap();

    let mut cmd = InitBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::MaskingBk3Failed)
    );
    assert!(cmd.take_response().is_some());
}

#[test]
fn test_mask_bk_boot_fail() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_generate_bk_boot().times(1).returning(|buf| {
        for b in buf.iter_mut() {
            *b = 0xA5;
        }
        Ok(())
    });

    part.expect_get_masked_bk_boot_len()
        .times(1)
        .returning(|| 0);

    part.expect_mask_bk3()
        .times(1)
        .return_const(Err(HsmErr::InsufficientBuffer));

    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_vm_launch_guid().times(1).returning(|| [1; 16]);

    part.expect_mask_bk3().times(1).returning(|_, _, len, buf| {
        let to_copy = core::cmp::min(*len, buf.len());
        for item in buf.iter_mut().take(to_copy) {
            *item = 0x5A;
        }
        *len = to_copy;
        Ok(())
    });

    part.expect_mask_bk_boot()
        .times(1)
        .return_const(Err(HsmErr::MaskingBkBootFailed));

    let req = encode_buf::<DdiInitBk3CmdReq, _>(&cmd(), &heap).unwrap();

    let mut cmd = InitBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::MaskingBkBootFailed)
    );
    assert!(cmd.take_response().is_some());
}

#[test]
fn test_bk3_mask_success() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_generate_bk_boot().times(1).returning(|buf| {
        for b in buf.iter_mut() {
            *b = 0xA5;
        }
        Ok(())
    });

    part.expect_get_masked_bk_boot_len()
        .times(1)
        .returning(|| 0);

    part.expect_mask_bk3()
        .times(1)
        .return_const(Err(HsmErr::InsufficientBuffer));

    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_vm_launch_guid().times(1).returning(|| [1; 16]);

    part.expect_mask_bk3().times(1).returning(|_, _, len, buf| {
        let to_copy = core::cmp::min(*len, buf.len());
        for item in buf.iter_mut().take(to_copy) {
            *item = 0x5A;
        }
        *len = to_copy;
        Ok(())
    });

    part.expect_mask_bk_boot()
        .times(1)
        .returning(|_, len, buf| {
            let to_copy = core::cmp::min(*len, buf.len());
            for item in buf.iter_mut().take(to_copy) {
                *item = 0x5A;
            }
            *len = to_copy;
            Ok(())
        });

    part.expect_masked_bk_boot().times(1).returning(|| {
        let mut masked_bk_boot_addr = [0u8; MASKED_BK_BOOT_SIZE];
        let masked_bk_boot_mem_range: IoMemRange = (&mut masked_bk_boot_addr[..]).into();
        masked_bk_boot_mem_range
    });

    part.expect_set_masked_bk_boot_len()
        .times(1)
        .returning(|_| ());

    let req = encode_buf::<DdiInitBk3CmdReq, _>(&cmd(), &heap).unwrap();

    let mut cmd = InitBk3Cmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Ok(())
    );
    assert!(cmd.take_response().is_some());
}

fn cmd() -> DdiInitBk3CmdReq {
    let bk3 = [0u8; 48];
    DdiInitBk3CmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::InitBk3,
            sess_id: None,
        },
        data: DdiInitBk3Req {
            bk3: MborByteArray::new_with_len(&bk3 as *const _ as *const u8, bk3.len()),
        },
    }
}
