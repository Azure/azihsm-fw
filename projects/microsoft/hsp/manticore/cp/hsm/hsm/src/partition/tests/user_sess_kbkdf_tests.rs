// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_sha::KbkdfInfo;
use mcr_crypto_sha::KbkdfInputData;
use mcr_crypto_sha::ShaMode;
use mcr_crypto_sha::KDF_MAX_LENGTH_MULTIPLIER;

use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;
use mcr_crypto_pka::PkaEccKeyPair;
use mcr_crypto_pka::PkaEccPrivateKey;
use mcr_crypto_pka::PkaEccPublicKey;
use mcr_crypto_pka::PkaEccSecretValue;
use mcr_ddi_mbor::MborByteArray;
use mcr_ddi_types::DdiHashAlgorithm;
use mcr_ddi_types::DdiKeyAvailability;
use mcr_ddi_types::DdiKeyProperties;
use mcr_ddi_types::DdiKeyType;
use mcr_ddi_types::DdiKeyUsage;
use mcr_types::*;

use super::HsmResult;
use crate::cmd_scheduler::TagId;
use crate::mock::*;
use crate::partition::tests::cmd_scheduler;
use crate::partition::tests::rev;
use crate::partition::tests::set_ipc_expectations;
use crate::partition::tests::TEST_RAW_ECC_256_PUBLIC_KEY;
use crate::partition::tests::TEST_RAW_ECC_384_PUBLIC_KEY;
use crate::partition::tests::TEST_RAW_ECC_521_PUBLIC_KEY;
use crate::partition::EccCurve;
use crate::partition::EccKeyUsage;
use crate::partition::HsmErr;
use crate::partition::HsmUserSession;
use crate::partition::KeyAvailability;
use crate::partition::PartEnv;
use crate::partition::PartState;
use crate::partition::ShaType;
use crate::partition::UserSession;

fn test_kbkdf(
    ecc_curve_type: PkaEccCurve,
    label: Option<&[u8]>,
    context: Option<&[u8]>,
    hash_algo: DdiHashAlgorithm,
    key_type: DdiKeyType,
    key_properties: DdiKeyProperties,
    key_tag: Option<u16>,
) -> HsmResult<u16> {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let label = match label {
        None => Vec::new(),
        Some(label_inner) => label_inner.to_vec(),
    };
    let context = match context {
        None => Vec::new(),
        Some(context_inner) => context_inner.to_vec(),
    };

    // Clone for use in mock
    let label_input = label.clone();
    let context_input = context.clone();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(move || {
        let mut pka = MockPka::new();

        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(move |_tag, _curve| {
                Ok(PkaEccCmd {
                    curve: ecc_curve_type,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [1; PkaEccCurve::MAX_LEN],
                        curve: ecc_curve_type,
                    },
                    pub_key: PkaEccPublicKey {
                        data: [2; PkaEccCurve::MAX_LEN * 2],
                        curve: ecc_curve_type,
                    },
                })
            });

        pka.expect_begin_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag, _curve| Ok(()));
        pka.expect_end_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Ok(()));
        pka.expect_begin_ecc_point_validation_zc()
            .times(1)
            .returning(|_, _, _| Ok(()));
        pka.expect_end_ecc_point_validation_zc()
            .times(1)
            .returning(|_tag| Ok(true));
        pka.expect_begin_ecdh_compute_zc().times(1).returning(
            move |_tag, _curve, _privkey, _pubkey| {
                Ok(PkaEccCmd {
                    curve: ecc_curve_type,
                })
            },
        );
        pka.expect_end_ecdh_compute()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaEccSecretValue {
                    curve: ecc_curve_type,
                    secret: [0; PkaEccCurve::MAX_LEN],
                })
            });

        pka
    });

    let mut sha = MockSha::new();
    sha.expect_kbkdf_counter_hmac().times(0..).returning(
        move |kbkdf_info: KbkdfInfo, sha_mode: ShaMode, _, output: &mut [u8]| {
            // As we have removed openssl kdf crate due to S360 issues, we no longer can compare
            // using the openssl_kdf::kdf::derive function.
            // Instead, we will compare the inputs which should not be modified when the flow reaches
            // here.

            let (label, context) = match kbkdf_info.input_data {
                KbkdfInputData::SelfTestData { .. } => Err(0xe_u32)?,
                KbkdfInputData::ConcatData { label, context } => (label, context),
            };

            let hash_len = ShaType::from(sha_mode) as usize;
            let hash_buffer_len = ShaType::from(sha_mode).get_digest_size_hw();

            // Sanity checks
            if hash_len > SHA_DIGEST_MAX_SIZE_BYTES || hash_len == 0 {
                // KbkdfSanityCheckFailed = 0xe,
                Err(0xe_u32)?
            }

            if kbkdf_info.out_len as usize > (KDF_MAX_LENGTH_MULTIPLIER * hash_len) {
                // KbkdfSanityCheckFailed = 0xe,
                Err(0xe_u32)?
            }

            if output.len() < hash_buffer_len + kbkdf_info.out_len as usize {
                // KbkdfSanityCheckFailed = 0xe,
                Err(0xe_u32)?
            }

            assert_eq!(label_input, label);
            assert_eq!(context_input, context);

            let out_vec = vec![0u8; kbkdf_info.out_len as usize];

            let expected_out_len = kbkdf_info.out_len as usize;
            output[..expected_out_len].copy_from_slice(&out_vec[..expected_out_len]);

            Ok(())
        },
    );

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);

    hal.expect_sha().times(0..).return_const(sha);

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(0..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    hal.expect_dma_heap().times(0..).return_const(heap);

    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let result = app_session.begin_ecc_gen_key(
        tag,
        None,
        ecc_curve_type.into(),
        EccKeyUsage::KeyAgreement,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 2);

    let (ecc_key_type, raw_pub_key) = match ecc_curve_type {
        PkaEccCurve::Ecc256 => (
            DdiKeyType::Secret256,
            IoMemRange::from(TEST_RAW_ECC_256_PUBLIC_KEY.as_slice()),
        ),
        PkaEccCurve::Ecc384 => (
            DdiKeyType::Secret384,
            IoMemRange::from(TEST_RAW_ECC_384_PUBLIC_KEY.as_slice()),
        ),
        PkaEccCurve::Ecc521 => (
            DdiKeyType::Secret521,
            IoMemRange::from(TEST_RAW_ECC_521_PUBLIC_KEY.as_slice()),
        ),
    };
    let result =
        app_session.begin_ecdh_compute_with_pub_key_validation(tag, 0, ecc_key_type, &raw_pub_key);
    assert!(result.is_ok());
    let op = result.unwrap();

    let continue_ecdh_result = app_session.continue_ecdh_compute_zc(op, &raw_pub_key);
    assert!(continue_ecdh_result.is_ok());
    let op = continue_ecdh_result.unwrap();

    let continue_ecdh_result = app_session.continue_ecdh_compute_zc(op, &raw_pub_key);
    assert!(continue_ecdh_result.is_ok());
    let op = continue_ecdh_result.unwrap();

    let end_ecdh_compute_result =
        app_session.end_ecdh_compute(op, DdiKeyUsage::Derive, None, KeyAvailability::App);
    assert!(end_ecdh_compute_result.is_ok());
    let key_id_result = end_ecdh_compute_result.unwrap();
    assert_eq!(key_id_result, 1);

    app_session.kbkdf_derive(
        key_id_result,
        &label,
        &context,
        hash_algo,
        key_type,
        key_properties,
        key_tag,
        None,
    )
}

#[test]
fn test_kbkdf_aes128() {
    assert!(test_kbkdf(
        PkaEccCurve::Ecc256,
        None,
        None,
        DdiHashAlgorithm::Sha256,
        DdiKeyType::Aes128,
        DdiKeyProperties {
            key_usage: DdiKeyUsage::EncryptDecrypt,
            key_availability: DdiKeyAvailability::Session,
            key_label: MborByteArray::new_with_len([].as_ptr(), 0),
        },
        None
    )
    .is_ok());
}

#[test]
fn test_kbkdf_large_label() {
    let label = [0u8; 2048];
    assert_eq!(
        test_kbkdf(
            PkaEccCurve::Ecc521,
            Some(&label),
            None,
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Aes256,
            DdiKeyProperties {
                key_usage: DdiKeyUsage::EncryptDecrypt,
                key_availability: DdiKeyAvailability::Session,
                key_label: MborByteArray::new_with_len([].as_ptr(), 0),
            },
            None
        ),
        Err(HsmErr::KbkdfInvalidInputParam)
    );
}

#[test]
fn test_kbkdf_large_context() {
    let context = [0u8; 2048];
    assert_eq!(
        test_kbkdf(
            PkaEccCurve::Ecc521,
            None,
            Some(&context),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Aes256,
            DdiKeyProperties {
                key_usage: DdiKeyUsage::EncryptDecrypt,
                key_availability: DdiKeyAvailability::Session,
                key_label: MborByteArray::new_with_len([].as_ptr(), 0),
            },
            None
        ),
        Err(HsmErr::KbkdfInvalidInputParam)
    );
}

#[test]
fn test_kbkdf_aesbulk256_invalid() {
    assert_eq!(
        test_kbkdf(
            PkaEccCurve::Ecc256,
            None,
            None,
            DdiHashAlgorithm::Sha256,
            DdiKeyType::AesGcmBulk256Unapproved,
            DdiKeyProperties {
                key_usage: DdiKeyUsage::EncryptDecrypt,
                key_availability: DdiKeyAvailability::Session,
                key_label: MborByteArray::new_with_len([].as_ptr(), 0),
            },
            None
        ),
        Err(HsmErr::InvalidKeyType)
    );
}

#[test]
fn test_kbkdf_secret_invalid() {
    assert_eq!(
        test_kbkdf(
            PkaEccCurve::Ecc256,
            None,
            None,
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Secret384,
            DdiKeyProperties {
                key_usage: DdiKeyUsage::Derive,
                key_availability: DdiKeyAvailability::Session,
                key_label: MborByteArray::new_with_len([].as_ptr(), 0),
            },
            None
        ),
        Err(HsmErr::InvalidKeyType)
    );
}

#[test]
fn test_kbkdf_hmac256() {
    assert!(test_kbkdf(
        PkaEccCurve::Ecc256,
        None,
        None,
        DdiHashAlgorithm::Sha256,
        DdiKeyType::HmacSha256,
        DdiKeyProperties {
            key_usage: DdiKeyUsage::SignVerify,
            key_availability: DdiKeyAvailability::Session,
            key_label: MborByteArray::new_with_len([].as_ptr(), 0),
        },
        None
    )
    .is_ok());
}

#[test]
fn test_kbkdf_hmac384() {
    assert!(test_kbkdf(
        PkaEccCurve::Ecc256,
        None,
        None,
        DdiHashAlgorithm::Sha384,
        DdiKeyType::HmacSha384,
        DdiKeyProperties {
            key_usage: DdiKeyUsage::SignVerify,
            key_availability: DdiKeyAvailability::Session,
            key_label: MborByteArray::new_with_len([].as_ptr(), 0),
        },
        None
    )
    .is_ok());
}

#[test]
fn test_kbkdf_hmac512() {
    assert!(test_kbkdf(
        PkaEccCurve::Ecc256,
        None,
        None,
        DdiHashAlgorithm::Sha512,
        DdiKeyType::HmacSha512,
        DdiKeyProperties {
            key_usage: DdiKeyUsage::SignVerify,
            key_availability: DdiKeyAvailability::Session,
            key_label: MborByteArray::new_with_len([].as_ptr(), 0),
        },
        None
    )
    .is_ok());
}
