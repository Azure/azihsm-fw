// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;
use mcr_types::*;

use super::*;
use crate::cmd_scheduler::*;
use crate::fsm::ecdh_key_exchange::EcdhKeyExchangeCmd;
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
    let req = encode_buf::<DdiEcdhKeyExchangeCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.rollback(TagId::default()).is_ok());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let part = MockPartition::new();
    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);

    let req = MockDmaAlloc::new(10);

    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.rollback(TagId::default()).is_ok());
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

    let mut session = MockUserSession::new();
    session.expect_id().times(2).returning(SessionId::default);
    session.expect_api_rev().once().returning(api_rev);

    session
        .expect_begin_ecdh_compute_with_pub_key_validation()
        .once()
        .returning(|_tag, _key_id, _key_type, _| begin_ecdh_compute(EccCurve::P256));
    session
        .expect_continue_ecdh_compute_zc()
        .once()
        .returning(|op, _pub_key_der| {
            Ok(EcdhComputeCmd {
                tag: op.tag,
                engine_ref: op.engine_ref,
                key_id: op.key_id,
                curve: op.curve,
                cmd_info: PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                },
                state: EcdhComputeCmdState::EcdhCompute,
            })
        });
    session
        .expect_end_ecdh_compute()
        .once()
        .returning(|_op, _key_usage, _key_tag, _key_availability| Ok(key_id()));
    session.expect_delete_key().once().returning(|_| Ok(()));
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));

    let req = encode_buf::<DdiEcdhKeyExchangeCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);

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
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_rollback_err() {
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
        .expect_begin_ecdh_compute_with_pub_key_validation()
        .once()
        .returning(|_tag, _key_id, _key_type, _| begin_ecdh_compute(EccCurve::P256));
    session
        .expect_continue_ecdh_compute_zc()
        .once()
        .returning(|op, _pub_key_der| {
            Ok(EcdhComputeCmd {
                tag: op.tag,
                engine_ref: op.engine_ref,
                key_id: op.key_id,
                curve: op.curve,
                cmd_info: PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                },
                state: EcdhComputeCmdState::EcdhCompute,
            })
        });
    session
        .expect_end_ecdh_compute()
        .once()
        .returning(|_op, _key_usage, _key_tag, _key_availability| Ok(key_id()));
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

    let req = encode_buf::<DdiEcdhKeyExchangeCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);

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
        Ok(())
    );
    assert!(cmd.take_response().is_some());
    assert_eq!(cmd.rollback(TagId::default()), Err(HsmErr::InvalidKeyIndex));
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_ecdh_compute_ecp256() {
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
        .expect_begin_ecdh_compute_with_pub_key_validation()
        .once()
        .returning(|_tag, _key_id, _key_type, _| begin_ecdh_compute(EccCurve::P256));
    session
        .expect_continue_ecdh_compute_zc()
        .once()
        .returning(|op, _pub_key_der| {
            Ok(EcdhComputeCmd {
                tag: op.tag,
                engine_ref: op.engine_ref,
                key_id: op.key_id,
                curve: op.curve,
                cmd_info: PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                },
                state: EcdhComputeCmdState::EcdhCompute,
            })
        });
    session
        .expect_end_ecdh_compute()
        .once()
        .returning(|_op, _key_usage, _key_tag, _key_availability| Ok(key_id()));
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiEcdhKeyExchangeCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    validate_response(resp);
}

#[test]
fn test_ecdh_compute_ecp384() {
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
        .expect_begin_ecdh_compute_with_pub_key_validation()
        .once()
        .returning(|_, _, _, _| begin_ecdh_compute(EccCurve::P384));
    session
        .expect_continue_ecdh_compute_zc()
        .once()
        .returning(|op, _pub_key_der| {
            Ok(EcdhComputeCmd {
                tag: op.tag,
                engine_ref: op.engine_ref,
                key_id: op.key_id,
                curve: op.curve,
                cmd_info: PkaEccCmd {
                    curve: PkaEccCurve::Ecc384,
                },
                state: EcdhComputeCmdState::EcdhCompute,
            })
        });
    session
        .expect_end_ecdh_compute()
        .once()
        .returning(|_op, _key_usage, _key_tag, _key_availability| Ok(key_id()));
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiEcdhKeyExchangeCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    validate_response(resp);
}

#[test]
fn test_ecdh_compute_ecp521() {
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
        .expect_begin_ecdh_compute_with_pub_key_validation()
        .once()
        .returning(|_tag, _key_id, _key_type, _| begin_ecdh_compute(EccCurve::P521));
    session
        .expect_continue_ecdh_compute_zc()
        .once()
        .returning(|op, _pub_key_der| {
            Ok(EcdhComputeCmd {
                tag: op.tag,
                engine_ref: op.engine_ref,
                key_id: op.key_id,
                curve: op.curve,
                cmd_info: PkaEccCmd {
                    curve: PkaEccCurve::Ecc521,
                },
                state: EcdhComputeCmdState::EcdhCompute,
            })
        });
    session
        .expect_end_ecdh_compute()
        .once()
        .returning(|_op, _key_usage, _key_tag, _key_availability| Ok(key_id()));
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiEcdhKeyExchangeCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    validate_response(resp);
}

#[test]
fn test_ecdh_compute_begin_error() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session
        .expect_begin_ecdh_compute_with_pub_key_validation()
        .once()
        .returning(|_tag, _key_id, _key_type, _| Err(HsmErr::InvalidArgument));

    let req = encode_buf::<DdiEcdhKeyExchangeCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );

    let resp = cmd.take_response();
    assert!(resp.is_none());
}

#[test]
fn test_ecdh_compute_on_engine_ready() {
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
        .expect_begin_ecdh_compute_with_pub_key_validation()
        .once()
        .returning(|_tag, _key_id, _key_type, _| Err(HsmErr::Pending));
    session
        .expect_begin_ecdh_compute_with_pub_key_validation()
        .once()
        .returning(|_tag, _key_id, _key_type, _| begin_ecdh_compute(EccCurve::P256));
    session
        .expect_continue_ecdh_compute_zc()
        .once()
        .returning(|op, _pub_key_der| {
            Ok(EcdhComputeCmd {
                tag: op.tag,
                engine_ref: op.engine_ref,
                key_id: op.key_id,
                curve: op.curve,
                cmd_info: PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                },
                state: EcdhComputeCmdState::EcdhCompute,
            })
        });
    session
        .expect_end_ecdh_compute()
        .once()
        .returning(|_op, _key_usage, _key_tag, _key_availability| Ok(key_id()));
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiEcdhKeyExchangeCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);
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
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    validate_response(resp);
}

#[test]
fn test_ecdh_compute_on_engine_ready_during_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let session = MockUserSession::new();

    let req = encode_buf::<DdiEcdhKeyExchangeCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);

    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::InvalidEvent)
    );
}

#[test]
fn test_ecdh_compute_on_cmd_complete_during_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let session = MockUserSession::new();

    let req = encode_buf::<DdiEcdhKeyExchangeCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
}

#[test]
fn test_ecdh_compute_on_engine_ready_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();

    session
        .expect_begin_ecdh_compute_with_pub_key_validation()
        .once()
        .returning(|_tag, _key_id, _key_type, _| Err(HsmErr::Pending));
    session
        .expect_begin_ecdh_compute_with_pub_key_validation()
        .once()
        .returning(move |_tag, _key_id, _key_type, _| Err(HsmErr::Pending));

    let req = encode_buf::<DdiEcdhKeyExchangeCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);

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
fn test_ecdh_compute_continue_ecdh_compute_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();

    session
        .expect_begin_ecdh_compute_with_pub_key_validation()
        .once()
        .returning(|_tag, _key_id, _key_type, _| begin_ecdh_compute(EccCurve::P256));
    session
        .expect_continue_ecdh_compute_zc()
        .once()
        .returning(|_op, _pub_key_der| Err(HsmErr::Pending));
    let req = encode_buf::<DdiEcdhKeyExchangeCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::InvalidState)
    );
}

#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);

    session
        .expect_begin_ecdh_compute_with_pub_key_validation()
        .once()
        .returning(|_, _, _, _| Err(HsmErr::InvalidArgument));

    let req = encode_buf::<DdiEcdhKeyExchangeCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);
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

#[test]
fn test_requires_and_acquire_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let session = MockUserSession::new();

    let req = encode_buf::<DdiEcdhKeyExchangeCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = EcdhKeyExchangeCmd::<MockEnv>::new(req, heap, session, part);
    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::Pka));
    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::Pka)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    );
}

fn cmd_req() -> DdiEcdhKeyExchangeCmdReq {
    DdiEcdhKeyExchangeCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::EcdhKeyExchange,
            sess_id: Some(SessionId::default()),
        },
        data: DdiEcdhKeyExchangeReq {
            priv_key_id: key_id(),
            pub_key_der: MborByteArray::new_with_len(core::ptr::null(), 91),
            key_type: DdiKeyType::Secret256,
            key_tag: Some(1),
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_derive(true)
                    .with_session(false),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
        },
    }
}

fn begin_ecdh_compute(curve: EccCurve) -> HsmResult<EcdhComputeCmd<MockEnv>> {
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
    let engine = resource.acquire(TagId::default(), Some(key_id()));
    let pka_curve: PkaEccCurve = curve.into();
    Ok(EcdhComputeCmd {
        tag: TagId::default(),
        engine_ref: engine.unwrap(),
        key_id: key_id(),
        curve,
        cmd_info: PkaEccCmd { curve: pka_curve },
        state: EcdhComputeCmdState::MontgomeryConstCal,
    })
}

fn validate_response(resp: Option<MockDmaAlloc>) {
    let resp = decode_buf::<DdiEcdhKeyExchangeCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.key_id, key_id())
}

fn key_id() -> u16 {
    1
}

fn api_rev() -> DdiApiRev {
    DdiApiRev { major: 1, minor: 0 }
}
