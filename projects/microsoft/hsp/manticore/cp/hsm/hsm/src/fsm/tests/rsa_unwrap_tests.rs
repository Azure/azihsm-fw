// Copyright (c) Microsoft Corporation. All rights reserved.

use core::ops::Range;

use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;
use mcr_crypto_pka::PkaRsaCmd;
use mcr_crypto_pka::PkaRsaSize;
use mcr_types::*;

use super::*;
use crate::cmd_scheduler::*;
use crate::partition::pct_engine::PctEngine;
use crate::partition::pct_engine_impl::PctEngineImpl;
use crate::partition::*;
use crate::recorder::HsmFsmEventRecorder;
use crate::resource::FpIpcChannelResource;
use crate::resource::HsmFsmResourceId;
use crate::resource::PkaResource;

const IMPORTED_KEY_ID: u16 = 23;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Aes), &heap).unwrap();

    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);
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

    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_begin_rsa_unwrap_on_start() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Aes), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_begin_rsa_unwrap_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, _, _| Err(HsmErr::InvalidArgument));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Aes), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_on_pka_complete() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().times(2).returning(SessionId::default);
    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| {
            let kek = [0xAAu8; 32];
            Ok(SecureByteVec::from(kek.to_vec()))
        });

    // AES unwrap
    session
        .expect_begin_aes_key_unwrap()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    session.expect_end_aes_key_unwrap().once().returning(|_| {
        Ok(Range {
            start: 8,
            end: 8 + 16,
        })
    });
    // Import key
    session.expect_api_rev().once().returning(api_rev);
    session
        .expect_import_der_key()
        .once()
        .returning(|_, _, _, _, _| {
            Ok(ImportDerKeyResult {
                priv_key_id: IMPORTED_KEY_ID,
                pub_key_data: Some(vec![8u8; 600]),
                key_type: DdiKeyType::Rsa2kPrivate,
            })
        });
    // Roll back
    session.expect_delete_key().once().returning(|_| Ok(()));
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Aes), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::SoftAesResp, TagId::default()),
        Ok(())
    );

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    validate_response(resp);
}

#[test]
fn test_on_pka_complete_structural_validation_engine_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().times(2).returning(SessionId::default);
    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| {
            let kek = [0xAAu8; 32];
            Ok(SecureByteVec::from(kek.to_vec()))
        });
    // AES unwrap
    session
        .expect_begin_aes_key_unwrap()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    session.expect_end_aes_key_unwrap().once().returning(|_| {
        Ok(Range {
            start: 8,
            end: 8 + 16,
        })
    });
    // Import key
    session.expect_api_rev().once().returning(api_rev);
    session
        .expect_import_der_key()
        .once()
        .returning(|_, _, _, _, _| {
            Ok(ImportDerKeyResult {
                priv_key_id: IMPORTED_KEY_ID,
                pub_key_data: Some(vec![8u8; 64]),
                key_type: DdiKeyType::Ecc256Private,
            })
        });
    session
        .expect_begin_ecc_structural_validation()
        .once()
        .returning(|_, _, _, _| Err(HsmErr::Pending));
    session
        .expect_begin_ecc_structural_validation()
        .once()
        .returning(move |tag, key_id, _usage, pub_key_blob| {
            Ok(begin_structural_validation_mock(
                tag,
                key_id,
                EccCurve::P256,
                pub_key_blob,
            ))
        });
    session
        .expect_continue_ecc_structural_validation()
        .times(1)
        .returning(Ok);
    session
        .expect_end_ecc_structural_validation()
        .once()
        .returning(|_op| Ok(()));
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
    // Roll back
    session.expect_delete_key().once().returning(|_| Ok(()));
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Ecc), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::SoftAesResp, TagId::default()),
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
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
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
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Ok(())
    );

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    validate_response(resp);
}

#[test]
fn test_on_pka_complete_structural_validation() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().times(2).returning(SessionId::default);
    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| {
            let kek = [0xAAu8; 32];
            Ok(SecureByteVec::from(kek.to_vec()))
        });
    // AES unwrap
    session
        .expect_begin_aes_key_unwrap()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    session.expect_end_aes_key_unwrap().once().returning(|_| {
        Ok(Range {
            start: 8,
            end: 8 + 16,
        })
    });
    // Import key
    session.expect_api_rev().once().returning(api_rev);
    session
        .expect_import_der_key()
        .once()
        .returning(|_, _, _, _, _| {
            Ok(ImportDerKeyResult {
                priv_key_id: IMPORTED_KEY_ID,
                pub_key_data: Some(vec![8u8; 64]),
                key_type: DdiKeyType::Ecc256Private,
            })
        });
    session
        .expect_begin_ecc_structural_validation()
        .once()
        .returning(move |tag, key_id, _usage, pub_key_blob| {
            Ok(begin_structural_validation_mock(
                tag,
                key_id,
                EccCurve::P256,
                pub_key_blob,
            ))
        });
    session
        .expect_continue_ecc_structural_validation()
        .times(1)
        .returning(Ok);
    session
        .expect_end_ecc_structural_validation()
        .once()
        .returning(|_op| Ok(()));
    set_ecc_pct_expectations(&mut session, true);
    session
        .expect_is_pct_final_state()
        .once()
        .returning(|_| false);
    session
        .expect_is_pct_final_state()
        .once()
        .returning(|_| true);
    // Roll back
    session.expect_delete_key().once().returning(|_| Ok(()));
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Ecc), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::SoftAesResp, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Ok(())
    );

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    validate_response(resp);
}

#[test]
fn test_on_pka_complete_structural_validation_engine_ready_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .once()
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    // part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();

    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| {
            let kek = [0xAAu8; 32];
            Ok(SecureByteVec::from(kek.to_vec()))
        });
    // AES unwrap
    session
        .expect_begin_aes_key_unwrap()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    session.expect_end_aes_key_unwrap().once().returning(|_| {
        Ok(Range {
            start: 8,
            end: 8 + 16,
        })
    });
    // Import key
    session
        .expect_import_der_key()
        .once()
        .returning(|_, _, _, _, _| {
            Ok(ImportDerKeyResult {
                priv_key_id: IMPORTED_KEY_ID,
                pub_key_data: Some(vec![8u8; 600]),
                key_type: DdiKeyType::Ecc256Private,
            })
        });
    session
        .expect_begin_ecc_structural_validation()
        .once()
        .returning(|_, _, _, _| Err(HsmErr::Pending));
    session
        .expect_begin_ecc_structural_validation()
        .once()
        .returning(|_, _, _, _| Err(HsmErr::Pending));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Ecc), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::SoftAesResp, TagId::default()),
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
fn test_check_alive_and_timeout() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().times(1).returning(SessionId::default);
    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| {
            let kek = [0xAAu8; 32];
            Ok(SecureByteVec::from(kek.to_vec()))
        });
    // AES unwrap
    session
        .expect_begin_aes_key_unwrap()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Aes), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );

    const TIMEOUT_CNT_MAX: usize = 4;
    for _i in 0..TIMEOUT_CNT_MAX {
        assert_eq!(
            cmd.on_event(HsmFsmEvent::CheckAlive, TagId::default()),
            Err(HsmErr::Pending)
        );
    }

    assert_eq!(
        cmd.on_event(HsmFsmEvent::CheckAlive, TagId::default()),
        Err(HsmErr::IoTimeOut)
    );

    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_on_pka_complete_sha_input_allocate_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate().once().returning(|_| None);
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| {
            let kek = [0xAAu8; 32];
            Ok(SecureByteVec::from(kek.to_vec()))
        });

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Aes), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_on_pka_complete_sha_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| Err(HsmErr::RsaUnwrapOaepDecodeFailed));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Aes), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::RsaUnwrapOaepDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_on_pka_complete_aes_key_allocate_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate().once().returning(|_| None);
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| {
            let kek = [0xAAu8; 32];
            Ok(SecureByteVec::from(kek.to_vec()))
        });

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Aes), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_import_rsa() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().times(2).returning(SessionId::default);
    session.expect_api_rev().once().returning(api_rev);
    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| {
            let kek = [0xAAu8; 32];
            Ok(SecureByteVec::from(kek.to_vec()))
        });
    // AES unwrap
    session
        .expect_begin_aes_key_unwrap()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    session.expect_end_aes_key_unwrap().once().returning(|_| {
        Ok(Range {
            start: 8,
            end: 8 + 16,
        })
    });
    // Import key
    session
        .expect_import_der_key()
        .once()
        .returning(|_, _, _, _, _| {
            Ok(ImportDerKeyResult {
                priv_key_id: IMPORTED_KEY_ID,
                pub_key_data: Some(vec![8u8; 260]),
                key_type: DdiKeyType::Rsa2kPrivate,
            })
        });
    session
        .expect_begin_rsa_pct_validation()
        .once()
        .returning(|_tag, _key_id, _usage, _rsa_type, _, _| Err(HsmErr::Pending));
    session.expect_begin_rsa_pct_validation().once().returning(
        move |tag, key_id, usage, rsa_type, _, _| {
            begin_rsa_pct_validation(tag, key_id, usage, rsa_type)
        },
    );

    session
        .expect_continue_rsa_pct_validation()
        .times(1)
        .returning(move |op| {
            let mut op = op;
            op.state = RsaPctValidationState::WaitForDecrypt;
            Ok(op)
        });
    session
        .expect_end_rsa_pct_validation()
        .once()
        .returning(|_op| Ok(true));
    session
        .expect_is_rsa_pct_final_state()
        .once()
        .returning(|_| false);
    session
        .expect_is_rsa_pct_final_state()
        .once()
        .returning(|_| true);
    // Roll back
    session.expect_delete_key().once().returning(|_| Ok(()));
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Rsa), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::SoftAesResp, TagId::default()),
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
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    validate_response(resp);
}

#[test]
fn test_import_crt() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().times(2).returning(SessionId::default);
    session.expect_api_rev().once().returning(api_rev);
    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| {
            let kek = [0xAAu8; 32];
            Ok(SecureByteVec::from(kek.to_vec()))
        });
    // AES unwrap
    session
        .expect_begin_aes_key_unwrap()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    session.expect_end_aes_key_unwrap().once().returning(|_| {
        Ok(Range {
            start: 8,
            end: 8 + 16,
        })
    });
    // Import key
    session
        .expect_begin_import_der_crt_key()
        .once()
        .returning(|_tag, _der| Ok((import_der_crt_key().unwrap(), vec![1; 260])));
    session
        .expect_continue_import_der_crt_key()
        .once()
        .returning(|_der| Ok(import_der_crt_key().unwrap()));
    session.expect_end_import_der_crt_key().once().returning(
        |_op, _key_usage, _key_tag, _key_availability| Ok((0, DdiKeyType::Rsa2kPrivateCrt)),
    );
    session.expect_begin_rsa_pct_validation().once().returning(
        move |tag, key_id, usage, rsa_type, _, _| {
            begin_rsa_pct_validation(tag, key_id, usage, rsa_type)
        },
    );
    session
        .expect_continue_rsa_pct_validation()
        .times(1)
        .returning(move |op| {
            let mut op = op;
            op.state = RsaPctValidationState::WaitForDecrypt;
            Ok(op)
        });
    session
        .expect_end_rsa_pct_validation()
        .once()
        .returning(|_op| Ok(true));
    session
        .expect_is_rsa_pct_final_state()
        .once()
        .returning(|_| false);
    session
        .expect_is_rsa_pct_final_state()
        .once()
        .returning(|_| true);
    // Roll back
    session.expect_delete_key().once().returning(|_| Ok(()));
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::RsaCrt), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::SoftAesResp, TagId::default()),
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
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    validate_response(resp);
}

#[test]
fn test_import_crt_on_engine_ready_err_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().times(1).returning(SessionId::default);
    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| {
            let kek = [0xAAu8; 32];
            Ok(SecureByteVec::from(kek.to_vec()))
        });
    // AES unwrap
    session
        .expect_begin_aes_key_unwrap()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    session.expect_end_aes_key_unwrap().once().returning(|_| {
        Ok(Range {
            start: 8,
            end: 8 + 16,
        })
    });
    // Import key
    session
        .expect_begin_import_der_crt_key()
        .once()
        .returning(|_tag, _der| Err(HsmErr::Pending));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::RsaCrt), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::SoftAesResp, TagId::default()),
        Err(HsmErr::Pending)
    );

    let resp = cmd.take_response();
    assert!(resp.is_none());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_import_aesbulk256() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().times(2).returning(SessionId::default);
    session.expect_api_rev().once().returning(api_rev);
    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| {
            let kek = [0xAAu8; 32];
            Ok(SecureByteVec::from(kek.to_vec()))
        });
    // AES unwrap
    session
        .expect_begin_aes_key_unwrap()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    session.expect_end_aes_key_unwrap().once().returning(|_| {
        Ok(Range {
            start: 8,
            end: 8 + 16,
        })
    });
    // Import key
    session
        .expect_begin_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Ok(import_der_aesbulk256_key().unwrap()));
    session
        .expect_end_import_der_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    #[cfg(feature = "mcr_test_hooks")]
    session
        .expect_cmd_fsm_test_action()
        .times(1)
        .returning(|_| None);

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::AesGcmBulk), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        cmd.on_event(HsmFsmEvent::SoftAesResp, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    validate_response(resp);
}

#[test]
fn test_import_aesbulk256_rollback_on_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().times(2).returning(SessionId::default);
    session.expect_api_rev().once().returning(api_rev);
    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| {
            let kek = [0xAAu8; 32];
            Ok(SecureByteVec::from(kek.to_vec()))
        });
    // AES unwrap
    session
        .expect_begin_aes_key_unwrap()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    session.expect_end_aes_key_unwrap().once().returning(|_| {
        Ok(Range {
            start: 8,
            end: 8 + 16,
        })
    });
    // Import key
    session
        .expect_begin_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Ok(import_der_aesbulk256_key().unwrap()));
    session
        .expect_end_import_der_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    session
        .expect_begin_rollback_aesbulk256_key()
        .once()
        .returning(|_, _, _| Ok(()));
    session
        .expect_end_rollback_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    #[cfg(feature = "mcr_test_hooks")]
    session
        .expect_cmd_fsm_test_action()
        .times(1)
        .returning(|_| None);

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::AesGcmBulk), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        cmd.on_event(HsmFsmEvent::SoftAesResp, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());

    let _ = cmd.rollback(TagId::default());

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    validate_response(resp);
}

#[test]
fn test_import_aesbulk256_on_engine_ready_err_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().times(1).returning(SessionId::default);
    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| {
            let kek = [0xAAu8; 32];
            Ok(SecureByteVec::from(kek.to_vec()))
        });
    // AES unwrap
    session
        .expect_begin_aes_key_unwrap()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    session.expect_end_aes_key_unwrap().once().returning(|_| {
        Ok(Range {
            start: 8,
            end: 8 + 16,
        })
    });
    // Import key
    session
        .expect_begin_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Err(HsmErr::Pending));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::AesGcmBulk), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        cmd.on_event(HsmFsmEvent::SoftAesResp, TagId::default()),
        Err(HsmErr::Pending)
    );

    let resp = cmd.take_response();
    assert!(resp.is_none());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_acquire_fp_ipc_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let session = MockUserSession::new();
    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::AesGcmBulk), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel));
    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
    );
}

#[test]
fn test_encode_buf_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().times(2).returning(SessionId::default);
    session.expect_api_rev().once().returning(api_rev);
    // RSA decrypt
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));
    session
        .expect_end_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _| end_rsa_mod_exp());
    // OAEP decode
    session
        .expect_decode_oaep_kek()
        .once()
        .returning(|_key_id, _wrapped, _| {
            let kek = [0xAAu8; 32];
            Ok(SecureByteVec::from(kek.to_vec()))
        });
    // AES unwrap
    session
        .expect_begin_aes_key_unwrap()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    session.expect_end_aes_key_unwrap().once().returning(|_| {
        Ok(Range {
            start: 8,
            end: 8 + 16,
        })
    });
    // Import key
    session
        .expect_import_der_key()
        .once()
        .returning(|_, _, _, _, _| {
            Ok(ImportDerKeyResult {
                priv_key_id: IMPORTED_KEY_ID,
                pub_key_data: Some(vec![8u8; 600]),
                key_type: DdiKeyType::Rsa2kPrivate,
            })
        });
    // Roll back
    session.expect_delete_key().once().returning(|_| Ok(()));
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Aes), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(1), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::SoftAesResp, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, _, _| Err(HsmErr::InvalidArgument));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Aes), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_rsa_unwrap_on_pka_engine_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, _, _| Err(HsmErr::Pending));
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .once()
        .returning(|_, _, _, output, _| begin_rsa_mod_exp(output));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Aes), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

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
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_rsa_unwrap_on_engine_ready_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().times(1).returning(SessionId::default);
    session
        .expect_begin_rsa_unwrap_mod_exp_zc()
        .times(2)
        .returning(|_, _, _, _, _| Err(HsmErr::Pending));

    let req = encode_buf::<DdiRsaUnwrapCmdReq, _>(&cmd(DdiKeyClass::Aes), &heap).unwrap();
    let mut cmd = RsaUnwrapCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

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
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

fn begin_structural_validation_mock(
    tag: TagId,
    key_id: KeyId,
    curve: EccCurve,
    pub_key_blob: Vec<u8>,
) -> EccStructuralValidationCmd<MockEnv> {
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());

    let mock_pka = MockPka::new();
    let resource = CmdResource::new(PkaResource::new(vec![mock_pka]), scheduler, 1);

    let engine = resource.acquire(tag, Some(key_id)).unwrap();
    let pka_curve: PkaEccCurve = curve.into();

    let ecc_op = EccGenPubKeyCmd {
        tag,
        engine_ref: engine,
        key_id,
        curve,
        cmd_info: PkaEccCmd { curve: pka_curve },
        state: EccPtMultiplicationState::WaitForPointMultiplication,
    };

    EccStructuralValidationCmd {
        tag,
        pub_key_blob,
        dma_buf: MockDmaAlloc::new(400),
        ecc_op,
    }
}

fn validate_response(resp: Option<MockDmaAlloc>) {
    let resp = decode_buf::<DdiRsaUnwrapCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    let cmd = cmd(DdiKeyClass::Aes);
    assert_eq!(resp.hdr.rev, cmd.hdr.rev);
    assert_eq!(resp.hdr.op, cmd.hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd.hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

fn api_rev() -> DdiApiRev {
    DdiApiRev { major: 1, minor: 0 }
}

fn cmd(wrapped_blob_key_class: DdiKeyClass) -> DdiRsaUnwrapCmdReq {
    // RSA2k-wrapped AES + AES-wrapped AES
    const WRAPPED_BLOB: [u8; 280] = [
        0x66, 0xc9, 0xf4, 0xad, 0x17, 0x2c, 0x10, 0x5d, 0xaf, 0x98, 0xd8, 0x6a, 0x45, 0x86, 0x00,
        0x06, 0x10, 0x8f, 0x01, 0x8c, 0xa2, 0x2d, 0x52, 0xeb, 0x9a, 0xfe, 0xb8, 0xea, 0x5e, 0xd5,
        0x48, 0x30, 0x72, 0x43, 0xe0, 0x2c, 0xa2, 0xe0, 0x30, 0x13, 0xcc, 0xb1, 0x9a, 0xca, 0xe1,
        0x13, 0x01, 0xbd, 0x42, 0xd0, 0xa5, 0x4a, 0x89, 0xa0, 0xa2, 0xa0, 0x0a, 0xe6, 0x85, 0x7a,
        0xcc, 0xd3, 0x9b, 0x54, 0xcb, 0x45, 0x7c, 0x63, 0x41, 0xb2, 0xdc, 0xe0, 0x14, 0x63, 0xf3,
        0x38, 0x6d, 0x4b, 0x1c, 0x43, 0xf6, 0xb4, 0x35, 0x11, 0xd9, 0x4e, 0x9b, 0x4f, 0x6e, 0x82,
        0x89, 0xe8, 0x07, 0xf3, 0xa1, 0xe9, 0x10, 0x41, 0x4b, 0x8d, 0xd9, 0xfd, 0x3c, 0x38, 0x53,
        0x60, 0x61, 0xac, 0x69, 0x3c, 0xf7, 0xfa, 0x82, 0xed, 0x55, 0x60, 0xe3, 0xa9, 0xdc, 0xf9,
        0xb0, 0x81, 0xee, 0x68, 0x0c, 0x8d, 0x3d, 0xd8, 0x33, 0x38, 0x76, 0xb9, 0x2a, 0x43, 0x00,
        0xef, 0x91, 0x67, 0x2b, 0xc0, 0xff, 0x18, 0x8b, 0x87, 0xfe, 0x56, 0x81, 0x79, 0x25, 0x32,
        0xbe, 0x37, 0x40, 0x5c, 0xb4, 0xb2, 0xcd, 0x18, 0xb2, 0x50, 0x11, 0xd3, 0x5c, 0x31, 0x0f,
        0xe2, 0xf1, 0x4d, 0x6c, 0xf6, 0x91, 0x07, 0xa5, 0xb4, 0x36, 0x15, 0x94, 0x9c, 0xbb, 0x0f,
        0x27, 0x47, 0xaf, 0x44, 0x99, 0x28, 0x4e, 0x6d, 0x4f, 0xca, 0x5a, 0x6d, 0x0f, 0xa0, 0x0b,
        0x98, 0x4a, 0xc3, 0x3e, 0x6e, 0xee, 0xcb, 0xff, 0xe7, 0x26, 0x14, 0x70, 0x5f, 0xf2, 0xdc,
        0xf5, 0x4c, 0x9b, 0x79, 0x25, 0x82, 0x9a, 0xe5, 0x4f, 0x9f, 0x05, 0x7d, 0x57, 0xe3, 0x74,
        0x9e, 0x39, 0x0e, 0x96, 0x66, 0xea, 0xba, 0xea, 0x1a, 0xec, 0xe9, 0x90, 0xef, 0x32, 0x58,
        0x8a, 0x66, 0x09, 0x36, 0xe7, 0xd6, 0x76, 0x6f, 0x17, 0xf8, 0x9b, 0xdf, 0xda, 0x99, 0xda,
        0x55, 0x76, 0x3a, 0xb5, 0x1a, 0xb5, 0x62, 0xe4, 0xc7, 0x7c, 0x3a, 0xcd, 0xd7, 0x6b, 0x1b,
        0xc4, 0x91, 0x51, 0xa6, 0xcd, 0x6f, 0x88, 0xdc, 0xb0, 0x63,
    ];
    let key_usage = match wrapped_blob_key_class {
        DdiKeyClass::Rsa => Ok(DdiKeyUsage::EncryptDecrypt),
        DdiKeyClass::RsaCrt => Ok(DdiKeyUsage::EncryptDecrypt),
        DdiKeyClass::Aes | DdiKeyClass::AesGcmBulk => Ok(DdiKeyUsage::EncryptDecrypt),
        DdiKeyClass::Ecc => Ok(DdiKeyUsage::SignVerify),
        _ => Err(HsmErr::InvalidArgument),
    };

    let mut key_metadata = DdiTargetKeyMetadata::default().with_session(true);
    match key_usage.expect("InvalidArgument") {
        DdiKeyUsage::EncryptDecrypt => {
            key_metadata.set_encrypt(true);
            key_metadata.set_decrypt(true);
        }
        DdiKeyUsage::SignVerify => {
            key_metadata.set_sign(true);
            key_metadata.set_verify(true);
        }
        DdiKeyUsage::Unwrap => {
            key_metadata.set_unwrap(true);
        }
        DdiKeyUsage::Derive => {
            key_metadata.set_derive(true);
        }
        _ => {
            panic!("Invalid key usage")
        }
    }

    DdiRsaUnwrapCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::RsaUnwrap,
            sess_id: Some(SessionId::default()),
        },
        data: DdiRsaUnwrapReq {
            key_id: key_id(),
            wrapped_blob: MborByteArray::new_with_len(WRAPPED_BLOB.as_ptr(), WRAPPED_BLOB.len()),
            wrapped_blob_key_class,
            wrapped_blob_padding: DdiRsaCryptoPadding::Oaep,
            wrapped_blob_hash_algorithm: DdiHashAlgorithm::Sha1,
            key_tag: None,
            key_properties: DdiTargetKeyProperties {
                key_metadata,
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
        },
    }
}

fn key_id() -> u16 {
    1
}

fn begin_rsa_mod_exp(output: &IoMemRange) -> HsmResult<RsaModExp<MockEnv>> {
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
    let engine = resource.acquire(TagId::default(), Some(key_id()));

    // This unsafe operation is normally only done by the PKA operation
    let mut_output = unsafe {
        let const_ptr = output as *const IoMemRange;
        #[allow(invalid_reference_casting)]
        let mut_ptr = const_ptr as *mut IoMemRange;
        #[allow(invalid_reference_casting)]
        &mut *mut_ptr
    };

    const RSA_2K_MOD_EXP_OUTPUT: [u8; 256] = [
        0x00, 0xcc, 0x5a, 0x42, 0xaa, 0xd0, 0x37, 0x18, 0xb7, 0x84, 0x58, 0xa7, 0x8f, 0x05, 0xc3,
        0x7f, 0xaf, 0xec, 0x42, 0x45, 0x93, 0x23, 0xf5, 0x4b, 0xe6, 0x45, 0xca, 0x95, 0x85, 0x14,
        0x4a, 0x1c, 0xe8, 0x7f, 0x82, 0xfa, 0xdd, 0xf8, 0x32, 0x40, 0xed, 0x5e, 0x7e, 0xf4, 0x23,
        0x03, 0x9f, 0x88, 0xb9, 0x41, 0x61, 0x5d, 0x01, 0x78, 0x9c, 0x36, 0x07, 0x9c, 0x0f, 0x7a,
        0x5d, 0x4c, 0x76, 0x5e, 0xb8, 0x24, 0x7b, 0xbe, 0xbe, 0xf1, 0x58, 0xa3, 0xbb, 0x8d, 0xc0,
        0x37, 0x88, 0xcb, 0xbc, 0x2b, 0x32, 0x43, 0x4c, 0x70, 0xb6, 0xa5, 0x35, 0x96, 0xe7, 0x2c,
        0x3e, 0x4a, 0x15, 0x96, 0xd7, 0xd3, 0x2b, 0x9d, 0x24, 0x04, 0x7f, 0x6c, 0xb1, 0x43, 0xf9,
        0x56, 0x24, 0x70, 0xc1, 0xe5, 0x0f, 0x35, 0x7d, 0x14, 0xcf, 0x34, 0x4f, 0x25, 0xbd, 0x5a,
        0xb1, 0xc6, 0x77, 0x5b, 0x44, 0xda, 0x67, 0x0f, 0xd3, 0xee, 0xab, 0x7d, 0x00, 0x17, 0x9b,
        0x98, 0xa1, 0xa5, 0xb3, 0x31, 0xb6, 0x38, 0xc6, 0x50, 0x35, 0x64, 0x30, 0x93, 0xca, 0x18,
        0xa2, 0x06, 0x81, 0xbb, 0x81, 0xb8, 0x9a, 0x35, 0x3d, 0x9c, 0xb2, 0xbe, 0x09, 0x40, 0x71,
        0xf7, 0x3f, 0x2c, 0xc9, 0x6a, 0xe8, 0x39, 0xaf, 0xf9, 0xfd, 0x93, 0x52, 0x6a, 0x80, 0xd3,
        0x2d, 0x6a, 0xeb, 0x82, 0x21, 0xd7, 0xab, 0xd7, 0x2c, 0xe1, 0xad, 0xa0, 0xf7, 0xcd, 0xed,
        0x99, 0x4d, 0x89, 0x14, 0xd7, 0x84, 0x84, 0xf3, 0x3a, 0x9c, 0xd5, 0x3b, 0x82, 0x4b, 0xab,
        0x63, 0x60, 0x33, 0xc8, 0x5f, 0xe5, 0x26, 0x8f, 0x4a, 0x2c, 0x71, 0xf6, 0x3e, 0xfb, 0x58,
        0xf0, 0x74, 0x38, 0xb5, 0xbc, 0xd6, 0x65, 0xfe, 0x73, 0xd2, 0xfb, 0xe2, 0x8c, 0x8c, 0x57,
        0xde, 0xc0, 0x04, 0x0a, 0x95, 0xfd, 0x94, 0xd0, 0x27, 0x2f, 0xf3, 0xb1, 0x0d, 0x13, 0xe0,
        0x91,
    ];
    crate::partition::reverse_copy(
        &mut mut_output.slice_mut()[..RSA_2K_MOD_EXP_OUTPUT.len()],
        &RSA_2K_MOD_EXP_OUTPUT,
    );

    Ok(RsaModExp {
        tag: TagId::default(),
        engine_ref: engine.unwrap(),
        is_crt: false,
        cmd_info: PkaRsaCmd {
            rsa_type: PkaRsaSize::Rsa2k,
        },
    })
}

fn end_rsa_mod_exp() -> HsmResult<()> {
    Ok(())
}

fn import_der_aesbulk256_key() -> HsmResult<AesBulk256Cmd<MockEnv>> {
    let mock_ipc_message_channel = MockIpcMessageChannel::new();
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(
        FpIpcChannelResource::new(mock_ipc_message_channel),
        scheduler,
        1,
    );
    let channel = resource.acquire(TagId::default(), ());

    Ok(AesBulk256Cmd::DerKeyImport(
        Default::default(),
        0,
        channel.unwrap(),
    ))
}

fn begin_rsa_pct_validation(
    tag: TagId,
    key_id: u16,
    usage: RsaKeyUsage,
    rsa_type: PkaRsaSize,
) -> Result<RsaPctValidationCmd<MockEnv>, HsmErr> {
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
    let engine = resource.acquire(TagId::default(), Some(key_id));
    let cmd_info = PkaRsaCmd { rsa_type };
    let rsa_op = RsaModExp {
        tag,
        engine_ref: engine.unwrap(),
        is_crt: false,
        cmd_info,
    };
    Ok(RsaPctValidationCmd {
        tag,
        key_id,
        rsa_type,
        usage,
        dma_buf: MockDmaAlloc::new(400),
        state: RsaPctValidationState::WaitForEncrypt,
        rsa_op,
    })
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
