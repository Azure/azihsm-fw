// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;
use mcr_crypto_pka::PkaEccPublicKey;
use mcr_types::*;

use super::*;
use crate::cmd_scheduler::*;
use crate::fsm::ecc_gen_key::EccGenKeyCmd;
use crate::partition::pct_engine::PctEngine;
use crate::partition::pct_engine_impl::PctEngineImpl;
use crate::partition::*;
use crate::recorder::HsmFsmEventRecorder;
use crate::resource::HsmFsmResourceId;
use crate::resource::PkaResource;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    let req = encode_buf::<DdiEccGenerateKeyPairCmdReq, _>(&cmd(), &heap).unwrap();

    let mut cmd = EccGenKeyCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);

    let req = MockDmaAlloc::new(10);

    let mut cmd = EccGenKeyCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_encode_buf_err() {
    let curve = EccCurve::P256;
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().times(2).returning(SessionId::default);
    session.expect_api_rev().once().returning(api_rev);
    session
        .expect_begin_ecc_gen_key()
        .once()
        .returning(|_tag, _key_tag, _curve, _key_usage, _avail| begin_ecc_gen_key(EccCurve::P256));
    session
        .expect_end_ecc_gen_key()
        .once()
        .returning(move |_tag, _op| {
            Ok(EccGenKeyOut {
                ecc_key: EccKey::new(VaultKey::new(KeyStore::new(0, 1u128), 0)),
                pub_key: PkaEccPublicKey {
                    curve: curve.into(),
                    data: [0; PkaEccCurve::MAX_LEN * 2],
                },
            })
        });
    session.expect_delete_key().once().returning(|_| Ok(()));

    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));

    set_ecc_pct_expectations(&mut session, true);

    session
        .expect_is_pct_final_state()
        .once()
        .returning(|_| false);
    session
        .expect_is_pct_final_state()
        .once()
        .returning(|_| true);

    let req = encode_buf::<DdiEccGenerateKeyPairCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = EccGenKeyCmd::<MockEnv>::new(req, heap, session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_requires_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let session = MockUserSession::new();

    let req = encode_buf::<DdiEccGenerateKeyPairCmdReq, _>(&cmd(), &heap).unwrap();
    let cmd = EccGenKeyCmd::<MockEnv>::new(req, heap, session, part);

    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::Pka));
}

#[test]
fn test_acquire_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let session = MockUserSession::new();
    let req = encode_buf::<DdiEccGenerateKeyPairCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = EccGenKeyCmd::<MockEnv>::new(req, heap, session, part);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::Pka)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    );
}

#[test]
fn test_rollback_err() {
    let curve = EccCurve::P256;
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().times(2).returning(SessionId::default);
    session.expect_api_rev().once().returning(api_rev);

    session
        .expect_begin_ecc_gen_key()
        .once()
        .returning(|_tag, _key_tag, _curve, _key_usage, _avail| begin_ecc_gen_key(EccCurve::P256));
    session
        .expect_end_ecc_gen_key()
        .once()
        .returning(move |_tag, _op| {
            Ok(EccGenKeyOut {
                ecc_key: EccKey::new(VaultKey::new(KeyStore::new(0, 1u128), 0)),
                pub_key: PkaEccPublicKey {
                    curve: curve.into(),
                    data: [0; PkaEccCurve::MAX_LEN * 2],
                },
            })
        });
    session
        .expect_delete_key()
        .once()
        .returning(|_| Err(HsmErr::InvalidKeyIndex));

    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    set_ecc_pct_expectations(&mut session, true);

    session
        .expect_is_pct_final_state()
        .once()
        .returning(|_| false);
    session
        .expect_is_pct_final_state()
        .once()
        .returning(|_| true);

    let req = encode_buf::<DdiEccGenerateKeyPairCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = EccGenKeyCmd::<MockEnv>::new(req, heap, session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Ok(())
    );
    assert!(cmd.take_response().is_some());
    assert_eq!(cmd.rollback(TagId::default()), Err(HsmErr::InvalidKeyIndex));
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_ecc_gen_key() {
    let curve = EccCurve::P256;
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    session.expect_api_rev().once().returning(api_rev);

    session
        .expect_begin_ecc_gen_key()
        .once()
        .returning(|_tag, _key_tag, _curve, _key_usage, _avail| begin_ecc_gen_key(EccCurve::P256));
    session
        .expect_end_ecc_gen_key()
        .once()
        .returning(move |_tag, _op| {
            Ok(EccGenKeyOut {
                ecc_key: EccKey::new(VaultKey::new(KeyStore::new(0, 1u128), 0)),
                pub_key: PkaEccPublicKey {
                    curve: curve.into(),
                    data: [0; PkaEccCurve::MAX_LEN * 2],
                },
            })
        });

    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    set_ecc_pct_expectations(&mut session, true);

    session
        .expect_is_pct_final_state()
        .once()
        .returning(|_| false);
    session
        .expect_is_pct_final_state()
        .once()
        .returning(|_| true);

    let req = encode_buf::<DdiEccGenerateKeyPairCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = EccGenKeyCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    // Key Generation completes and PCT Validation starts
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (Sign → Verify)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // PCT Validation completes successfully
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Ok(())
    );
}

#[test]
fn test_ecc_gen_key_on_engine_ready() {
    let curve = EccCurve::P256;
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().times(2).returning(SessionId::default);
    session.expect_api_rev().once().returning(api_rev);
    session
        .expect_begin_ecc_gen_key()
        .once()
        .returning(|_tag, _key_tag, _curve, _key_usage, _avail| Err(HsmErr::Pending));
    session
        .expect_begin_ecc_gen_key()
        .once()
        .returning(|_tag, _key_tag, _curve, _key_usage, _avail| begin_ecc_gen_key(EccCurve::P256));
    session
        .expect_end_ecc_gen_key()
        .once()
        .returning(move |_tag, _op| {
            Ok(EccGenKeyOut {
                ecc_key: EccKey::new(VaultKey::new(KeyStore::new(0, 1u128), 0)),
                pub_key: PkaEccPublicKey {
                    curve: curve.into(),
                    data: [0; PkaEccCurve::MAX_LEN * 2],
                },
            })
        });

    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    session
        .expect_begin_ecc_pct_validation()
        .once()
        .returning(|_tag, _key_id, _usage, _pub_key| Err(HsmErr::Pending));
    set_ecc_pct_expectations(&mut session, true);

    session
        .expect_is_pct_final_state()
        .once()
        .returning(|_| false);
    session
        .expect_is_pct_final_state()
        .once()
        .returning(|_| true);

    let req = encode_buf::<DdiEccGenerateKeyPairCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = EccGenKeyCmd::<MockEnv>::new(req, heap, session, part);

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
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Ok(())
    );
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    validate_response(resp);
}

#[test]
fn test_ecc_gen_key_failure_in_verify() {
    let curve = EccCurve::P256;
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();

    session
        .expect_notify_pct_validation_failure()
        .once()
        .returning(|err| println!("Simulating PCT Validation Failure: {:?}", err));

    session.expect_api_rev().once().returning(api_rev);
    session.expect_id().times(1).returning(SessionId::default);

    session
        .expect_begin_ecc_gen_key()
        .once()
        .returning(|_tag, _key_tag, _curve, _key_usage, _avail| begin_ecc_gen_key(EccCurve::P256));
    session
        .expect_end_ecc_gen_key()
        .once()
        .returning(move |_tag, _op| {
            Ok(EccGenKeyOut {
                ecc_key: EccKey::new(VaultKey::new(KeyStore::new(0, 1u128), 0)),
                pub_key: PkaEccPublicKey {
                    curve: curve.into(),
                    data: [0; PkaEccCurve::MAX_LEN * 2],
                },
            })
        });

    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    set_ecc_pct_expectations(&mut session, false);

    session
        .expect_is_pct_final_state()
        .once()
        .returning(|_| false);
    session
        .expect_is_pct_final_state()
        .once()
        .returning(|_| true);

    let req = encode_buf::<DdiEccGenerateKeyPairCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = EccGenKeyCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    // Key Generation completes and PCT Validation starts
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (Sign → Verify)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // PCT Validation completes failure
    let _ = cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default());
}

#[test]
fn test_ecc_gen_key_on_engine_ready_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session
        .expect_begin_ecc_gen_key()
        .once()
        .returning(|_tag, _key_tag, _curve, _key_usage, _avail| Err(HsmErr::Pending));
    session
        .expect_begin_ecc_gen_key()
        .once()
        .returning(|_tag, _key_tag, _curve, _key_usage, _avail| Err(HsmErr::Pending));

    let req = encode_buf::<DdiEccGenerateKeyPairCmdReq, _>(&cmd(), &heap).unwrap();
    let mut cmd = EccGenKeyCmd::<MockEnv>::new(req, heap, session, part);

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
}

#[test]
fn test_invalid_state() {
    let cmd: DdiEccGenerateKeyPairCmdReq = cmd();

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().times(1).returning(SessionId::default);
    session
        .expect_begin_ecc_gen_key()
        .once()
        .returning(|_tag, _key_tag, _curve, _key_usage, _avail| Err(HsmErr::InvalidArgument));

    let req = encode_buf::<DdiEccGenerateKeyPairCmdReq, _>(&cmd, &heap).unwrap();
    let mut cmd = EccGenKeyCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
}

fn cmd() -> DdiEccGenerateKeyPairCmdReq {
    DdiEccGenerateKeyPairCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::EccGenerateKeyPair,
            sess_id: Some(SessionId::default()),
        },
        data: DdiEccGenerateKeyPairReq {
            curve: DdiEccCurve::P256,
            key_tag: Some(1),
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_sign(true)
                    .with_verify(true)
                    .with_session(false),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
        },
    }
}

fn begin_ecc_gen_key(curve: EccCurve) -> HsmResult<EccGenKey<MockEnv>> {
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
    let engine = resource.acquire(TagId::default(), Some(key_id()));
    let pka_curve: PkaEccCurve = curve.into();
    Ok(EccGenKey {
        tag: TagId::default(),
        engine_ref: engine.unwrap(),
        curve,
        key_tag: Some(1),
        usage: EccKeyUsage::SignVerify,
        availability: KeyAvailability::App,
        cmd_info: PkaEccCmd { curve: pka_curve },
    })
}

fn validate_response(resp: Option<MockDmaAlloc>) {
    let resp = decode_buf::<DdiEccGenerateKeyPairCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd().hdr.rev);
    assert_eq!(resp.hdr.op, cmd().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

fn key_id() -> u16 {
    1
}

fn api_rev() -> DdiApiRev {
    DdiApiRev { major: 1, minor: 0 }
}

fn set_ecc_pct_expectations(session: &mut MockUserSession, success: bool) {
    session.expect_begin_ecc_pct_validation().once().returning(
        move |_tag, _key_id, _usage, _pub_key| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
            let engine = resource.acquire(TagId::default(), None);

            let sign_verify_buf_size = PkaEccCurve::MAX_LEN + ECC_SIGNATURE_MAX_LEN;
            let op_dma_buf = MockDmaAlloc::new(sign_verify_buf_size);

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
    session
        .expect_continue_ecc_pct_validation()
        .times(1)
        .returning(|_tag, _op| Ok(()));
    session
        .expect_end_ecc_pct_validation()
        .once()
        .returning(move |_tag, _op| Ok(success));
}
