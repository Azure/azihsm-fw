// Copyright (c) Microsoft Corporation. All rights reserved.

use hmac::Hmac;
use hmac::Mac;
use mcr_crypto_sha::ShaMode;
use mcr_crypto_sha::KDF_MAX_LENGTH_MULTIPLIER;
use sha2::Sha256;
use sha2::Sha384;
use sha2::Sha512;

use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;
use mcr_crypto_pka::PkaEccKeyPair;
use mcr_crypto_pka::PkaEccPrivateKey;
use mcr_crypto_pka::PkaEccPublicKey;
use mcr_crypto_pka::PkaEccSecretValue;
use mcr_ddi_mbor::MborByteArray;
use mcr_ddi_types::DdiKeyType;
use mcr_ddi_types::DdiKeyUsage;
use mcr_types::*;

use openssl::md::Md;
use openssl::pkey::Id;
use openssl::pkey_ctx::HkdfMode;
use openssl::pkey_ctx::PkeyCtx;

use super::HsmResult;
use crate::cmd_scheduler::TagId;
use crate::mock::*;
use crate::partition::tests::cmd_scheduler;
use crate::partition::tests::rev;
use crate::partition::tests::set_ipc_expectations;
use crate::partition::tests::TEST_RAW_ECC_256_PUBLIC_KEY;
use crate::partition::tests::TEST_RAW_ECC_384_PUBLIC_KEY;
use crate::partition::tests::TEST_RAW_ECC_521_PUBLIC_KEY;
use crate::partition::DdiHashAlgorithm;
use crate::partition::DdiKeyAvailability;
use crate::partition::DdiKeyProperties;
use crate::partition::EccCurve;
use crate::partition::EccKeyUsage;
use crate::partition::HsmErr;
use crate::partition::HsmUserSession;
use crate::partition::KeyAvailability;
use crate::partition::PartEnv;
use crate::partition::PartState;
use crate::partition::ShaType;
use crate::partition::UserSession;
use mcr_crypto_sha::HkdfInfo;

#[test]
fn test_hmac_small_msg() {
    assert!(test_hmac(&[0; 31], PkaEccCurve::Ecc256).is_ok());
}

#[test]
fn test_hmac_large_msg() {
    assert!(test_hmac(&[0; 131], PkaEccCurve::Ecc256).is_ok());
}

#[test]
fn test_hmac_large_key() {
    assert!(test_hmac(&[0; 131], PkaEccCurve::Ecc521).is_ok());
}

#[test]
fn test_hmac_invalid_key() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type = PkaEccCurve::Ecc256;
    let tag = TagId::default();

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

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
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

    let raw_pub_key = IoMemRange::from(TEST_RAW_ECC_256_PUBLIC_KEY.as_slice());

    let result = app_session.begin_ecdh_compute_with_pub_key_validation(
        tag,
        0,
        DdiKeyType::Secret256,
        &raw_pub_key,
    );
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

    let hmac_out = [0u8; 64];
    let hmac_mborbytearray = MborByteArray::<64>::new_with_len(hmac_out.as_ptr(), 64);
    let result = app_session.hmac(12, &[0; 31], &mut (&hmac_mborbytearray).into());
    if let Err(err) = result {
        assert_eq!(err, HsmErr::InvalidKeyIndex);
    }
}

fn test_hmac(msg: &[u8], ecc_curve_type: PkaEccCurve) -> HsmResult<Vec<u8>> {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let mode = ShaType::Sha256;
    let sha_out_size: usize = mode as usize;
    let tag = TagId::default();

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
    sha.expect_hmac().once().returning(
        move |key: &[u8], data: &[u8], sha_mode: ShaMode, _, out_buf: &mut IoMemRange| {
            // Use the rust crypto hmac implementation here. This is to unit test the flow
            let output = match sha_mode {
                ShaMode::Sha1 => panic!("sha2 doesn't implement Sha1"),
                ShaMode::Sha256 => {
                    type HmacSha256 = Hmac<Sha256>;
                    let mut mac = HmacSha256::new_from_slice(key).expect("Unexpected error");
                    mac.update(data);
                    mac.finalize().into_bytes().to_vec()
                }
                ShaMode::Sha384 => {
                    type HmacSha384 = Hmac<Sha384>;
                    let mut mac = HmacSha384::new_from_slice(key).expect("Unexpected error");
                    mac.update(data);
                    mac.finalize().into_bytes().to_vec()
                }
                ShaMode::Sha512 => {
                    type HmacSha512 = Hmac<Sha512>;
                    let mut mac = HmacSha512::new_from_slice(key).expect("Unexpected error");
                    mac.update(data);
                    mac.finalize().into_bytes().to_vec()
                }
            };

            let sha_digest_size = ShaType::from(sha_mode) as usize;
            out_buf.slice_mut()[..sha_digest_size].copy_from_slice(&output[..sha_digest_size]);

            Ok(())
        },
    );

    let mut sha_for_kdf = MockSha::new();
    sha_for_kdf.expect_hkdf().times(0..).returning(
        move |hkdf_info: HkdfInfo, sha_mode: ShaMode, _, _, output: &mut [u8]| {
            // Call openssl hkdf with the given function parameters
            let sha_algo = match sha_mode {
                ShaMode::Sha1 => Md::sha1(),
                ShaMode::Sha256 => Md::sha256(),
                ShaMode::Sha384 => Md::sha384(),
                ShaMode::Sha512 => Md::sha512(),
            };

            let hash_len = ShaType::from(sha_mode) as usize;
            let hash_buffer_len = ShaType::from(sha_mode).get_digest_size_hw();

            if hash_len > SHA_DIGEST_MAX_SIZE_BYTES {
                // HkdfSanityCheckFailed = 0xb,
                Err(0xb_u32)?
            }

            if hkdf_info.out_len > (KDF_MAX_LENGTH_MULTIPLIER * hash_len) as u16 {
                // HkdfSanityCheckFailed = 0xb,
                Err(0xb_u32)?
            }

            if output.len() < hash_buffer_len + hkdf_info.out_len as usize {
                // HkdfSanityCheckFailed = 0xb,
                Err(0xb_u32)?
            }

            let mut ctx = PkeyCtx::new_id(Id::HKDF).unwrap();
            assert!(ctx.derive_init().is_ok());
            assert!(ctx.set_hkdf_key(hkdf_info.key).is_ok());
            assert!(ctx.set_hkdf_md(sha_algo).is_ok());
            assert!(ctx.set_hkdf_salt(hkdf_info.salt).is_ok());
            assert!(ctx.add_hkdf_info(hkdf_info.info).is_ok());
            assert!(ctx.set_hkdf_mode(HkdfMode::EXTRACT_THEN_EXPAND).is_ok());

            let mut openssl_out_vec = vec![0u8; hkdf_info.out_len as usize];
            assert!(ctx.derive(Some(&mut openssl_out_vec)).is_ok());

            let expected_out_len = hkdf_info.out_len as usize;
            output[..expected_out_len].copy_from_slice(&openssl_out_vec[..expected_out_len]);
            Ok(())
        },
    );

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);

    hal.expect_sha().once().return_const(sha_for_kdf);
    hal.expect_sha().once().return_const(sha);
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

    let (key_type, raw_pub_key) = match ecc_curve_type {
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
        app_session.begin_ecdh_compute_with_pub_key_validation(tag, 0, key_type, &raw_pub_key);
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

    let salt = [0; 32];
    let info = [0; 16];

    let hkdf_result = app_session.hkdf_derive(
        key_id_result,
        &salt,
        &info,
        DdiHashAlgorithm::Sha256,
        DdiKeyType::HmacSha256,
        DdiKeyProperties {
            key_usage: DdiKeyUsage::SignVerify,
            key_availability: DdiKeyAvailability::Session,
            key_label: MborByteArray::new_with_len([].as_ptr(), 0),
        },
        None,
        None,
    );
    assert!(hkdf_result.is_ok(), "{:?}", hkdf_result);
    let hmac_key_id = hkdf_result.unwrap();

    let hmac_out = [0u8; 32];
    let hmac_mborbytearray = MborByteArray::<64>::new_with_len(hmac_out.as_ptr(), 32);
    let hmac_result = app_session.hmac(hmac_key_id, msg, &mut (&hmac_mborbytearray).into());
    assert!(hmac_result.is_ok(), "{:?}", hmac_result);

    Ok(hmac_out[..sha_out_size].to_vec())
}
