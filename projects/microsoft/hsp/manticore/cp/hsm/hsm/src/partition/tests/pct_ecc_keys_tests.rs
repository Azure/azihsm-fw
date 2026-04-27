// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use mcr_crypto_pka::PkaEccCurve;
use mcr_types::*;

use crate::mock::*;
use crate::partition::pct_engine::PctEngine;
use crate::partition::pct_engine_impl::PctEngineImpl;
use crate::partition::EccKeyUsage;
use crate::resource::PkaResource;
use crate::CmdResource;
use crate::CmdScheduler;
use crate::TagId;

const ECC_SIGNATURE_MAX_LEN: usize = 192;

struct EccKeyPctParams {
    usage: EccKeyUsage,
    priv_key_data: [u8; 96],
    pub_key_data: [u8; 136],
    curve: PkaEccCurve,
    num_begin_mont_const_calc: usize,
    num_end_mont_const_calc: usize,
    num_begin_ecdh_compute_zc: usize,
    num_end_ecdh_compute: usize,
    num_peek_tag: usize,
    num_begin_ecc_sign_zc: usize,
    num_end_ecc_sign_zc: usize,
    num_begin_ecc_verify_zc: usize,
    num_end_ecc_verify_zc: usize,
    num_failed_shared_secret: usize,
    successful_ecc_verify: Option<bool>,
}

#[test]
fn test_ecc_key_agreement_pct() {
    let tag = TagId::default();
    let params = EccKeyPctParams {
        usage: EccKeyUsage::KeyAgreement,
        priv_key_data: [0u8; 96],
        pub_key_data: [0u8; 136],
        curve: PkaEccCurve::Ecc384,
        num_begin_mont_const_calc: 2,
        num_end_mont_const_calc: 2,
        num_begin_ecdh_compute_zc: 2,
        num_end_ecdh_compute: 2,
        num_peek_tag: 4,
        num_begin_ecc_sign_zc: 0,
        num_end_ecc_sign_zc: 0,
        num_begin_ecc_verify_zc: 0,
        num_end_ecc_verify_zc: 0,
        num_failed_shared_secret: 0,
        successful_ecc_verify: None,
    };

    let mut ecc_key_pct = set_ecc_pct_expectations(&params);

    let resp = ecc_key_pct.begin_ecc_pct_validation_inner(tag, params.usage);
    assert!(resp.is_ok());

    let resp = ecc_key_pct.continue_ecc_pct_validation_inner(tag);
    assert!(resp.is_ok());

    let resp = ecc_key_pct.continue_ecc_pct_validation_inner(tag);
    assert!(resp.is_ok());

    let resp = ecc_key_pct.continue_ecc_pct_validation_inner(tag);
    assert!(resp.is_ok());

    let resp = ecc_key_pct.end_ecc_pct_validation_inner(tag);
    assert!(resp.is_ok());
    assert!(resp.unwrap());
}

#[test]
fn test_ecc_key_agreement_pct_shared_secret_mismatch() {
    let tag = TagId::default();
    let params = EccKeyPctParams {
        usage: EccKeyUsage::KeyAgreement,
        priv_key_data: [0u8; 96],
        pub_key_data: [0u8; 136],
        curve: PkaEccCurve::Ecc384,
        num_begin_mont_const_calc: 2,
        num_end_mont_const_calc: 2,
        num_begin_ecdh_compute_zc: 2,
        num_end_ecdh_compute: 1,
        num_peek_tag: 4,
        num_begin_ecc_sign_zc: 0,
        num_end_ecc_sign_zc: 0,
        num_begin_ecc_verify_zc: 0,
        num_end_ecc_verify_zc: 0,
        num_failed_shared_secret: 1,
        successful_ecc_verify: None,
    };

    let mut ecc_key_pct = set_ecc_pct_expectations(&params);

    let resp = ecc_key_pct.begin_ecc_pct_validation_inner(tag, params.usage);
    assert!(resp.is_ok());

    let resp = ecc_key_pct.continue_ecc_pct_validation_inner(tag);
    assert!(resp.is_ok());

    let resp = ecc_key_pct.continue_ecc_pct_validation_inner(tag);
    assert!(resp.is_ok());

    let resp = ecc_key_pct.continue_ecc_pct_validation_inner(tag);
    assert!(resp.is_ok());

    let resp = ecc_key_pct.end_ecc_pct_validation_inner(tag);
    assert!(resp.is_ok());
    assert!(!resp.unwrap());
}

#[test]
fn test_ecc_sign_verify_pct() {
    let tag = TagId::default();
    let params = EccKeyPctParams {
        usage: EccKeyUsage::SignVerify,
        priv_key_data: [0u8; 96],
        pub_key_data: [0u8; 136],
        curve: PkaEccCurve::Ecc256,
        num_begin_mont_const_calc: 0,
        num_end_mont_const_calc: 0,
        num_begin_ecdh_compute_zc: 0,
        num_end_ecdh_compute: 0,
        num_peek_tag: 3,
        num_begin_ecc_sign_zc: 1,
        num_end_ecc_sign_zc: 1,
        num_begin_ecc_verify_zc: 1,
        num_end_ecc_verify_zc: 1,
        num_failed_shared_secret: 0,
        successful_ecc_verify: Some(true),
    };

    let mut ecc_key_pct = set_ecc_pct_expectations(&params);

    let resp = ecc_key_pct.begin_ecc_pct_validation_inner(tag, params.usage);
    assert!(resp.is_ok());

    let resp = ecc_key_pct.continue_ecc_pct_validation_inner(tag);
    assert!(resp.is_ok());

    let resp = ecc_key_pct.end_ecc_pct_validation_inner(tag);
    assert!(resp.is_ok());
    assert!(resp.unwrap());
}

#[test]
fn test_ecc_sign_verify_pct_failed_verify() {
    let tag = TagId::default();
    let params = EccKeyPctParams {
        usage: EccKeyUsage::SignVerify,
        priv_key_data: [0u8; 96],
        pub_key_data: [0u8; 136],
        curve: PkaEccCurve::Ecc256,
        num_begin_mont_const_calc: 0,
        num_end_mont_const_calc: 0,
        num_begin_ecdh_compute_zc: 0,
        num_end_ecdh_compute: 0,
        num_peek_tag: 3,
        num_begin_ecc_sign_zc: 1,
        num_end_ecc_sign_zc: 1,
        num_begin_ecc_verify_zc: 1,
        num_end_ecc_verify_zc: 1,
        num_failed_shared_secret: 0,
        successful_ecc_verify: Some(false),
    };

    let mut ecc_key_pct = set_ecc_pct_expectations(&params);

    let resp = ecc_key_pct.begin_ecc_pct_validation_inner(tag, params.usage);
    assert!(resp.is_ok());

    let resp = ecc_key_pct.continue_ecc_pct_validation_inner(tag);
    assert!(resp.is_ok());

    let resp = ecc_key_pct.end_ecc_pct_validation_inner(tag);
    assert!(resp.is_ok());
    assert!(!resp.unwrap());
}

fn set_ecc_pct_expectations(params: &EccKeyPctParams) -> EccKeyPct<MockEnv> {
    let mut engine_pka = MockPka::new();
    let mut sha = MockSha::new();

    let ecc_data_buffer_size = match params.usage {
        EccKeyUsage::SignVerify => {
            set_pka_expectations_for_sign_verify(&mut engine_pka, params, &mut sha);
            PkaEccCurve::MAX_LEN + ECC_SIGNATURE_MAX_LEN + PkaEccCurve::MAX_LEN * 2
        }
        EccKeyUsage::KeyAgreement => {
            set_pka_expectations_for_key_agreement(&mut engine_pka, params);
            (PkaEccCurve::MAX_LEN * 2) + PkaEccCurve::MAX_LEN + (PkaEccCurve::MAX_LEN * 2)
        }
    };

    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());

    let resource = CmdResource::new(PkaResource::new(vec![engine_pka]), scheduler, 1);
    let engine = resource.acquire(TagId::default(), None);

    let op_dma_buf = MockDmaAlloc::new(ecc_data_buffer_size);

    let priv_key_size = PkaEccPrivateKey::data_len(params.curve);
    let pub_key_data_len = PkaEccCurve::MAX_LEN * 2;

    let mut key_blob_dma_buf = MockDmaAlloc::new(pub_key_data_len + priv_key_size);

    key_blob_dma_buf.as_ref_mut()[pub_key_data_len..pub_key_data_len + priv_key_size]
        .copy_from_slice(&params.priv_key_data[..priv_key_size]);
    let priv_key_blob = IoMemRange::from(
        &key_blob_dma_buf.as_ref()[pub_key_data_len..pub_key_data_len + priv_key_size],
    );

    key_blob_dma_buf.as_ref_mut()[..pub_key_data_len].copy_from_slice(&params.pub_key_data);
    let pub_key_blob = IoMemRange::from(&key_blob_dma_buf.as_ref()[..pub_key_data_len]);

    let engine: Box<dyn PctEngine> = Box::new(PctEngineImpl::<MockEnv>::new(engine.unwrap(), sha));
    EccKeyPct::new(
        priv_key_blob,
        pub_key_blob,
        key_blob_dma_buf,
        params.curve,
        op_dma_buf,
        engine,
    )
}

fn set_pka_expectations_for_key_agreement(pka: &mut MockPka, params: &EccKeyPctParams) {
    pka.expect_begin_montgomery_constant_calculation()
        .times(params.num_begin_mont_const_calc)
        .returning(|_tag, _curve| Ok(()));
    pka.expect_peek_tag()
        .times(params.num_peek_tag)
        .returning(|| Some(TagId::default()));
    pka.expect_end_montgomery_constant_calculation()
        .times(params.num_end_mont_const_calc)
        .returning(|_tag| Ok(()));

    pka.expect_begin_ecdh_compute_zc()
        .times(params.num_begin_ecdh_compute_zc)
        .returning(move |_tag, _curve, _privkey, _pubkey| {
            Ok(PkaEccCmd {
                curve: PkaEccCurve::Ecc384,
            })
        });
    pka.expect_end_ecdh_compute()
        .times(params.num_end_ecdh_compute)
        .returning(move |_tag, _op| {
            Ok(PkaEccSecretValue {
                curve: PkaEccCurve::Ecc384,
                secret: [0; PkaEccCurve::MAX_LEN],
            })
        });
    pka.expect_end_ecdh_compute()
        .times(params.num_failed_shared_secret)
        .returning(move |_tag, _op| {
            Ok(PkaEccSecretValue {
                curve: PkaEccCurve::Ecc384,
                secret: [1; PkaEccCurve::MAX_LEN],
            })
        });
}

fn set_pka_expectations_for_sign_verify(
    pka: &mut MockPka,
    params: &EccKeyPctParams,
    sha: &mut MockSha,
) {
    let curve = params.curve;
    let success = params.successful_ecc_verify.unwrap();

    sha.expect_digest_zc().times(1).returning(move |cmd_info| {
        let output_slice = cmd_info.output_buffer.slice();

        let mut modifiable_mem_range = IoMemRange::from(output_slice);
        let hw_digest_len = cmd_info.mode.get_digest_size_hw();
        // fill the output buffer with 1s to ensure the validity check 1 < m < n - 1 passes
        modifiable_mem_range
            .slice_mut()
            .copy_from_slice(&[1u8; 512][..hw_digest_len]);

        Ok(())
    });

    pka.expect_begin_ecc_sign_zc()
        .times(params.num_begin_ecc_sign_zc)
        .returning(move |_tag, _curve, _digest, _privkey, _output| Ok(PkaEccCmd { curve }));
    pka.expect_peek_tag()
        .times(params.num_peek_tag)
        .returning(|| Some(TagId::default()));
    pka.expect_end_ecc_sign_zc()
        .times(params.num_end_ecc_sign_zc)
        .returning(|_tag| Ok(()));

    pka.expect_begin_ecc_verify_zc()
        .times(params.num_begin_ecc_verify_zc)
        .returning(move |_tag, _curve, _pubkey, _digest, _sig| Ok(()));
    pka.expect_end_ecc_verify_zc()
        .times(params.num_end_ecc_verify_zc)
        .returning(move |_tag| Ok(success));
}
