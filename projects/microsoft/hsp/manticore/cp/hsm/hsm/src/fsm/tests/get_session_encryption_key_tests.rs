// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;
use mcr_crypto_pka::PkaEccPublicKey;
use mcr_crypto_sha::ShaMode;
use mcr_types::*;

use super::*;
use crate::fsm::get_session_encryption_key::GetSessionEncryptionKeyCmd;
use crate::fsm::CredentialEncryptionKeyData;
use crate::fsm::GetSessionEncryptionKeyCtx;
use crate::fsm::HsmFsmEventRecorder;
use crate::fsm::HsmFsmResourceId;
use crate::partition::pct_engine::PctEngine;
use crate::partition::pct_engine_impl::PctEngineImpl;
use crate::partition::KeySignContext;
use crate::resource::PkaResource;
use crate::CmdResource;
use crate::CmdScheduler;
use crate::TagId;

use crate::partition::GetSessionEncryptionKeyOut;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();
    let req = encode_buf::<DdiGetSessionEncryptionKeyCmdReq, _>(&cmd(), &heap).unwrap();

    let mut cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();

    let part = MockPartition::new();
    let req = MockDmaAlloc::new(10);

    let mut cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
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

    part.expect_begin_get_session_encryption_key()
        .once()
        .returning(|_| Ok(pending_ctx()));
    part.expect_end_get_session_encryption_key()
        .once()
        .returning(|_, _| {
            Ok(GetSessionEncryptionKeyOut {
                pub_key: PkaEccPublicKey {
                    data: [1u8; 136],
                    curve: PkaEccCurve::Ecc384,
                },
                nonce: (1..33u8).collect::<Vec<_>>().try_into().unwrap(),
                new_key_id: Some(key_id()),
            })
        });
    part.expect_delete_internal_key()
        .once()
        .returning(|_| Ok(()));
    part.expect_unset_session_encryption_key_id()
        .once()
        .returning(|| ());

    let req = encode_buf::<DdiGetSessionEncryptionKeyCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    // Key Generation complete
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
}

#[test]
fn test_get_session_encryption_key_new_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_begin_get_session_encryption_key()
        .once()
        .returning(|_| Ok(pending_ctx()));
    part.expect_end_get_session_encryption_key()
        .once()
        .returning(|_, _| {
            Ok(GetSessionEncryptionKeyOut {
                pub_key: PkaEccPublicKey {
                    data: [1u8; 136],
                    curve: PkaEccCurve::Ecc384,
                },
                nonce: (1..33u8).collect::<Vec<_>>().try_into().unwrap(),
                new_key_id: Some(key_id()),
            })
        });

    part.expect_begin_signature_with_part_priv_key()
        .once()
        .returning(|tag, _, _| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let mock_pka = MockPka::new();
            let resource = CmdResource::new(PkaResource::new(vec![mock_pka]), scheduler, 1);

            let engine = resource.acquire(tag, None).unwrap();
            let digest_buf = MockDmaAlloc::new(ShaMode::Sha384.get_digest_size_hw());

            Ok(KeySignContext {
                engine_ref: engine,
                _digest_buf: digest_buf,
            })
        });

    part.expect_end_signature_with_key_blob()
        .once()
        .returning(|_, _| Ok(()));

    set_ecc_pct_key_agreement_expectations(&mut part);

    let req = encode_buf::<DdiGetSessionEncryptionKeyCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    // Key Generation complete and PCT Validation starts
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (Init -> EcdhMontgomeryConstCalculationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhMontgomeryConstCalculationFirst -> EcdhPointMultiplicationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhPointMultiplicationFirst -> EcdhMontgomeryConstCalculationSecond)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Finish PCT validation
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Finish signature generation
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());

    let resp = decode_buf::<DdiGetSessionEncryptionKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::GetSessionEncryptionKey);
    assert_eq!(resp.hdr.sess_id, None);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.pub_key.der.len(), 96);
    assert_eq!(
        resp.data.pub_key.der.as_slice(),
        vec![1; resp.data.pub_key.der.len()]
    );
    let expected_nonce: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
    assert_eq!(resp.data.nonce, expected_nonce);
}

#[test]
fn test_get_session_encryption_key_on_resource_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_begin_get_session_encryption_key()
        .once()
        .returning(|_| Err(HsmErr::Pending));
    part.expect_begin_get_session_encryption_key()
        .once()
        .returning(|_| Ok(pending_ctx()));

    part.expect_end_get_session_encryption_key()
        .once()
        .returning(|_, _| {
            Ok(GetSessionEncryptionKeyOut {
                pub_key: PkaEccPublicKey {
                    data: [1u8; 136],
                    curve: PkaEccCurve::Ecc384,
                },
                nonce: (1..33u8).collect::<Vec<_>>().try_into().unwrap(),
                new_key_id: Some(key_id()),
            })
        });

    part.expect_begin_ecc_pct_validation()
        .once()
        .returning(|_tag, _key_id, _usage, _pub_key| Err(HsmErr::Pending));

    part.expect_begin_signature_with_part_priv_key()
        .once()
        .returning(|tag, _, _| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let mock_pka = MockPka::new();
            let resource = CmdResource::new(PkaResource::new(vec![mock_pka]), scheduler, 1);

            let engine = resource.acquire(tag, None).unwrap();
            let digest_buf = MockDmaAlloc::new(ShaMode::Sha384.get_digest_size_hw());

            Ok(KeySignContext {
                engine_ref: engine,
                _digest_buf: digest_buf,
            })
        });

    part.expect_end_signature_with_key_blob()
        .once()
        .returning(|_, _| Ok(()));

    set_ecc_pct_key_agreement_expectations(&mut part);

    let req = encode_buf::<DdiGetSessionEncryptionKeyCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
    // Key Generation complete and PCT Validation starts
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Got resource for PCT Validation
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (Init -> EcdhMontgomeryConstCalculationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhMontgomeryConstCalculationFirst -> EcdhPointMultiplicationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhPointMultiplicationFirst -> EcdhMontgomeryConstCalculationSecond)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Finish PCT validation
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Finish signature generation
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());

    let resp = decode_buf::<DdiGetSessionEncryptionKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::GetSessionEncryptionKey);
    assert_eq!(resp.hdr.sess_id, None);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.pub_key.der.len(), 96);
    assert_eq!(
        resp.data.pub_key.der.as_slice(),
        vec![1; resp.data.pub_key.der.len()]
    );
    let expected_nonce: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
    assert_eq!(resp.data.nonce, expected_nonce);
}

#[test]
fn test_get_session_encryption_key_on_resource_ready_error() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_get_session_encryption_key()
        .once()
        .returning(|_| Err(HsmErr::Pending));
    part.expect_begin_get_session_encryption_key()
        .once()
        .returning(|_| Err(HsmErr::Pending));

    let req = encode_buf::<DdiGetSessionEncryptionKeyCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::InvalidState)
    );

    assert!(cmd.take_response().is_none());
}

#[test]
fn test_get_session_encryption_key_existing_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_begin_get_session_encryption_key()
        .once()
        .returning(|_| Ok(final_ctx()));

    part.expect_begin_signature_with_part_priv_key()
        .once()
        .returning(|tag, _, _| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let mock_pka = MockPka::new();
            let resource = CmdResource::new(PkaResource::new(vec![mock_pka]), scheduler, 1);

            let engine = resource.acquire(tag, None).unwrap();
            let digest_buf = MockDmaAlloc::new(ShaMode::Sha384.get_digest_size_hw());

            Ok(KeySignContext {
                engine_ref: engine,
                _digest_buf: digest_buf,
            })
        });

    part.expect_end_signature_with_key_blob()
        .once()
        .returning(|_, _| Ok(()));

    let req = encode_buf::<DdiGetSessionEncryptionKeyCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    // Finish signature generation
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());

    let resp = decode_buf::<DdiGetSessionEncryptionKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::GetSessionEncryptionKey);
    assert_eq!(resp.hdr.sess_id, None);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.pub_key.der.len(), 96);
}

#[test]
fn test_requires_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();

    let req = encode_buf::<DdiGetSessionEncryptionKeyCmdReq, _>(&cmd(), &heap).unwrap();
    let cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);

    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::Pka));
}

#[test]
fn test_acquire_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();

    let req = encode_buf::<DdiGetSessionEncryptionKeyCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::Pka)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    );
}

#[test]
fn test_rollback_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_begin_get_session_encryption_key()
        .once()
        .returning(|_| Ok(pending_ctx()));
    part.expect_end_get_session_encryption_key()
        .once()
        .returning(|_, _| {
            Ok(GetSessionEncryptionKeyOut {
                pub_key: PkaEccPublicKey {
                    data: [1u8; 136],
                    curve: PkaEccCurve::Ecc384,
                },
                nonce: (1..33u8).collect::<Vec<_>>().try_into().unwrap(),
                new_key_id: Some(key_id()),
            })
        });

    part.expect_begin_signature_with_part_priv_key()
        .once()
        .returning(|tag, _, _| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let mock_pka = MockPka::new();
            let resource = CmdResource::new(PkaResource::new(vec![mock_pka]), scheduler, 1);

            let engine = resource.acquire(tag, None).unwrap();
            let digest_buf = MockDmaAlloc::new(ShaMode::Sha384.get_digest_size_hw());

            Ok(KeySignContext {
                engine_ref: engine,
                _digest_buf: digest_buf,
            })
        });

    part.expect_end_signature_with_key_blob()
        .once()
        .returning(|_, _| Ok(()));

    part.expect_delete_internal_key()
        .once()
        .returning(|_| Err(HsmErr::InvalidKeyIndex));

    set_ecc_pct_key_agreement_expectations(&mut part);

    let req = encode_buf::<DdiGetSessionEncryptionKeyCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    // Key Generation complete and PCT Validation starts
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (Init -> EcdhMontgomeryConstCalculationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhMontgomeryConstCalculationFirst -> EcdhPointMultiplicationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhPointMultiplicationFirst -> EcdhMontgomeryConstCalculationSecond)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Finish PCT validation
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Finish signature generation
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    assert!(cmd.take_response().is_some());
    assert_eq!(cmd.rollback(TagId::default()), Err(HsmErr::InvalidKeyIndex));
}

#[test]
fn test_get_session_encryption_key_new_key_bad_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_begin_get_session_encryption_key()
        .once()
        .returning(|_| Ok(pending_ctx()));
    part.expect_end_get_session_encryption_key()
        .once()
        .returning(|_, _| {
            Ok(GetSessionEncryptionKeyOut {
                pub_key: PkaEccPublicKey {
                    data: [1u8; 136],
                    curve: PkaEccCurve::Ecc384,
                },
                nonce: (1..33u8).collect::<Vec<_>>().try_into().unwrap(),
                new_key_id: Some(key_id()),
            })
        });

    part.expect_begin_signature_with_part_priv_key()
        .once()
        .returning(|tag, _, _| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let mock_pka = MockPka::new();
            let resource = CmdResource::new(PkaResource::new(vec![mock_pka]), scheduler, 1);

            let engine = resource.acquire(tag, None).unwrap();
            let digest_buf = MockDmaAlloc::new(ShaMode::Sha384.get_digest_size_hw());

            Ok(KeySignContext {
                engine_ref: engine,
                _digest_buf: digest_buf,
            })
        });

    part.expect_end_signature_with_key_blob()
        .once()
        .returning(|_, _| Ok(()));

    set_ecc_pct_key_agreement_expectations(&mut part);

    let req = encode_buf::<DdiGetSessionEncryptionKeyCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    // Key Generation complete and PCT Validation starts
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (Init -> EcdhMontgomeryConstCalculationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhMontgomeryConstCalculationFirst -> EcdhPointMultiplicationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhPointMultiplicationFirst -> EcdhMontgomeryConstCalculationSecond)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Finish PCT validation
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Finish signature generation
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::InvalidEvent)
    );

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());

    let resp = decode_buf::<DdiGetSessionEncryptionKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::GetSessionEncryptionKey);
    assert_eq!(resp.hdr.sess_id, None);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.pub_key.der.len(), 96);
    assert_eq!(
        resp.data.pub_key.der.as_slice(),
        vec![1; resp.data.pub_key.der.len()]
    );
    let expected_nonce: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
    assert_eq!(resp.data.nonce, expected_nonce);
}

#[test]
fn test_get_session_encryption_begin_error() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_get_session_encryption_key()
        .once()
        .returning(|_| Err(HsmErr::InvalidArgument));

    let req = encode_buf::<DdiGetSessionEncryptionKeyCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );

    assert!(cmd.take_response().is_none());
}

#[test]
fn test_incomplete_pct_failure() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_begin_get_session_encryption_key()
        .once()
        .returning(|_| Ok(pending_ctx()));
    part.expect_end_get_session_encryption_key()
        .once()
        .returning(|_, _| {
            Ok(GetSessionEncryptionKeyOut {
                pub_key: PkaEccPublicKey {
                    data: [1u8; 136],
                    curve: PkaEccCurve::Ecc384,
                },
                nonce: (1..33u8).collect::<Vec<_>>().try_into().unwrap(),
                new_key_id: Some(key_id()),
            })
        });

    part.expect_begin_ecc_pct_validation().once().returning(
        move |_tag, _key_id, _usage, _pub_key| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
            let engine = resource.acquire(TagId::default(), None);

            let key_agreement_buffer_size =
                (PkaEccCurve::MAX_LEN * 2) + PkaEccCurve::MAX_LEN + (PkaEccCurve::MAX_LEN * 2);
            let op_dma_buf = MockDmaAlloc::new(key_agreement_buffer_size);

            let priv_key_blob = &[0u8; 96];
            let pub_key_blob = &[0u8; 136];

            let priv_key_size = PkaEccPrivateKey::data_len(PkaEccCurve::Ecc384);
            let pub_key_data_len = PkaEccCurve::MAX_LEN * 2;

            let mut key_blob_dma_buf = MockDmaAlloc::new(pub_key_data_len + priv_key_size);

            key_blob_dma_buf.as_ref_mut()[pub_key_data_len..pub_key_data_len + priv_key_size]
                .copy_from_slice(&priv_key_blob[..priv_key_size]);
            let priv_key_blob = IoMemRange::from(
                &key_blob_dma_buf.as_ref()[pub_key_data_len..pub_key_data_len + priv_key_size],
            );

            key_blob_dma_buf.as_ref_mut()[..pub_key_data_len].copy_from_slice(pub_key_blob);
            let pub_key_blob = IoMemRange::from(&key_blob_dma_buf.as_ref()[..pub_key_data_len]);

            let sha = MockSha::new();
            let engine: Box<dyn PctEngine> =
                Box::new(PctEngineImpl::<MockEnv>::new(engine.unwrap(), sha));

            let ecc_key_pct = EccKeyPct::new(
                priv_key_blob,
                pub_key_blob,
                key_blob_dma_buf,
                PkaEccCurve::Ecc384,
                op_dma_buf,
                engine,
            );

            Ok(ecc_key_pct)
        },
    );
    part.expect_continue_ecc_pct_validation()
        .times(1)
        .returning(move |_tag, _op| Ok(()));
    part.expect_continue_ecc_pct_validation()
        .times(1)
        .returning(move |_tag, _op| Err(HsmErr::EcdhComputeFailed));
    part.expect_is_pct_final_state()
        .times(2)
        .returning(|_| false);

    let req = encode_buf::<DdiGetSessionEncryptionKeyCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    // Key Generation complete and PCT Validation starts
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (Init -> EcdhMontgomeryConstCalculationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhMontgomeryConstCalculationFirst -> EcdhPointMultiplicationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::EcdhComputeFailed)
    );
}

#[test]
fn test_signature_generation_wait_for_engine() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_begin_get_session_encryption_key()
        .once()
        .returning(|_| Ok(pending_ctx()));
    part.expect_end_get_session_encryption_key()
        .once()
        .returning(|_, _| {
            Ok(GetSessionEncryptionKeyOut {
                pub_key: PkaEccPublicKey {
                    data: [1u8; 136],
                    curve: PkaEccCurve::Ecc384,
                },
                nonce: (1..33u8).collect::<Vec<_>>().try_into().unwrap(),
                new_key_id: Some(key_id()),
            })
        });

    part.expect_begin_signature_with_part_priv_key()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));

    part.expect_begin_signature_with_part_priv_key()
        .once()
        .returning(|tag, _, _| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let mock_pka = MockPka::new();
            let resource = CmdResource::new(PkaResource::new(vec![mock_pka]), scheduler, 1);

            let engine = resource.acquire(tag, None).unwrap();
            let digest_buf = MockDmaAlloc::new(ShaMode::Sha384.get_digest_size_hw());

            Ok(KeySignContext {
                engine_ref: engine,
                _digest_buf: digest_buf,
            })
        });

    part.expect_end_signature_with_key_blob()
        .once()
        .returning(|_, _| Ok(()));

    set_ecc_pct_key_agreement_expectations(&mut part);

    let req = encode_buf::<DdiGetSessionEncryptionKeyCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    // Key Generation complete and PCT Validation starts
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (Init -> EcdhMontgomeryConstCalculationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhMontgomeryConstCalculationFirst -> EcdhPointMultiplicationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhPointMultiplicationFirst -> EcdhMontgomeryConstCalculationSecond)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Finish PCT validation
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Got resource for signature generation
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
    // Finish signature generation
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());

    let resp = decode_buf::<DdiGetSessionEncryptionKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::GetSessionEncryptionKey);
    assert_eq!(resp.hdr.sess_id, None);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.pub_key.der.len(), 96);
    assert_eq!(
        resp.data.pub_key.der.as_slice(),
        vec![1; resp.data.pub_key.der.len()]
    );
    let expected_nonce: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
    assert_eq!(resp.data.nonce, expected_nonce);
}

#[test]
fn test_signature_generation_fail() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_begin_get_session_encryption_key()
        .once()
        .returning(|_| Ok(pending_ctx()));
    part.expect_end_get_session_encryption_key()
        .once()
        .returning(|_, _| {
            Ok(GetSessionEncryptionKeyOut {
                pub_key: PkaEccPublicKey {
                    data: [1u8; 136],
                    curve: PkaEccCurve::Ecc384,
                },
                nonce: (1..33u8).collect::<Vec<_>>().try_into().unwrap(),
                new_key_id: Some(key_id()),
            })
        });

    part.expect_begin_signature_with_part_priv_key()
        .once()
        .returning(|_, _, _| Err(HsmErr::EccSignFailed));

    set_ecc_pct_key_agreement_expectations(&mut part);

    let req = encode_buf::<DdiGetSessionEncryptionKeyCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = GetSessionEncryptionKeyCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    // Key Generation complete and PCT Validation starts
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (Init -> EcdhMontgomeryConstCalculationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhMontgomeryConstCalculationFirst -> EcdhPointMultiplicationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhPointMultiplicationFirst -> EcdhMontgomeryConstCalculationSecond)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Finish PCT validation
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::EccSignFailed)
    );
}

fn pending_ctx() -> GetSessionEncryptionKeyCtx<MockEnv> {
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
    let engine = resource.acquire(TagId::default(), Some(key_id()));
    let pka_curve = PkaEccCurve::Ecc384;
    GetSessionEncryptionKeyCtx {
        tag: TagId::default(),
        engine_ref: Some(engine.unwrap()),
        cmd_info: Some(PkaEccCmd { curve: pka_curve }),
        key_data: None,
    }
}

fn final_ctx() -> GetSessionEncryptionKeyCtx<MockEnv> {
    GetSessionEncryptionKeyCtx {
        tag: TagId::default(),
        engine_ref: None,
        cmd_info: None,
        key_data: Some(CredentialEncryptionKeyData {
            pub_key_data: PkaEccPublicKey {
                data: [1u8; PkaEccCurve::MAX_LEN * 2],
                curve: PkaEccCurve::Ecc384,
            },
            nonce: (1..33u8).collect::<Vec<_>>().try_into().unwrap(),
        }),
    }
}

fn cmd() -> DdiGetSessionEncryptionKeyCmdReq {
    DdiGetSessionEncryptionKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::GetSessionEncryptionKey,
            sess_id: Some(SessionId::default()),
        },
        data: DdiGetSessionEncryptionKeyReq {},
    }
}

fn key_id() -> u16 {
    1
}
