// Copyright (c) Microsoft Corporation. All rights reserved.

use std::sync::Arc;

use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;
use mcr_crypto_pka::PkaEccKeyPair;
use mcr_crypto_pka::PkaEccPrivateKey;
use mcr_crypto_pka::PkaEccPublicKey;
use mcr_crypto_pka::PkaEccSecretValue;
use mcr_ddi_mbor::MborByteArray;
use mcr_ddi_types::DdiHashAlgorithm;
use mcr_ddi_types::DdiKeyType;
use mcr_ddi_types::DdiKeyUsage;
use mcr_types::*;

use super::TEST_RAW_ECC_256_PUBLIC_KEY;
use super::TEST_RAW_ECC_384_PUBLIC_KEY;
use super::TEST_RAW_ECC_521_PUBLIC_KEY;
use crate::cmd_scheduler::TagId;
use crate::error::HsmErr;
use crate::mock::*;
use crate::partition::tests::cmd_scheduler;
use crate::partition::tests::rev;
use crate::partition::tests::set_ipc_expectations;
use crate::partition::EccCurve;
use crate::partition::EccKeyIn;
use crate::partition::EccKeyUsage;
use crate::partition::EcdhComputeCmdState;
use crate::partition::HsmUserSession;
use crate::partition::KeyAvailability;
use crate::partition::KeyId;
use crate::partition::PartEnv;
use crate::partition::PartState;
use crate::partition::UserSession;

#[test]
fn test_ecc_gen_key_256() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(|_tag, _curve| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(|_tag, _op| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [1; PkaEccCurve::MAX_LEN],
                        curve: PkaEccCurve::Ecc256,
                    },
                    pub_key: PkaEccPublicKey {
                        data: [1; PkaEccCurve::MAX_LEN * 2],
                        curve: PkaEccCurve::Ecc256,
                    },
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
        EccCurve::P256,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let gen_key_result = end_ecc_gen_key_result.unwrap();
    assert_eq!(gen_key_result.ecc_key.id(), 0);
    assert_eq!(gen_key_result.pub_key.data[EccCurve::MAX_LEN - 1], 1);
    assert_eq!(gen_key_result.pub_key.data[EccCurve::MAX_LEN * 2 - 1], 1);
}

#[test]
fn test_ecc_gen_key_384() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(|_tag, _curve| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc384,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(|_tag, _op| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [2; PkaEccCurve::MAX_LEN],
                        curve: PkaEccCurve::Ecc384,
                    },
                    pub_key: PkaEccPublicKey {
                        data: [2; PkaEccCurve::MAX_LEN * 2],
                        curve: PkaEccCurve::Ecc384,
                    },
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
        EccCurve::P384,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 2);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN * 2 - 1], 2);
}

#[test]
fn test_ecc_gen_key_521() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(|_tag, _curve| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc521,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(|_tag, _op| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [3; PkaEccCurve::MAX_LEN],
                        curve: PkaEccCurve::Ecc521,
                    },
                    pub_key: PkaEccPublicKey {
                        data: [3; PkaEccCurve::MAX_LEN * 2],
                        curve: PkaEccCurve::Ecc521,
                    },
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
        EccCurve::P521,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 3);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN * 2 - 1], 3);
}

#[test]
fn test_ecc_gen_key_pka_begin_fail() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(|_tag, _curve| Err(u32::MAX));

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
        EccCurve::P256,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_err());
}

#[test]
fn test_ecc_gen_key_pka_end_fail() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(|_tag, _curve| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(|_tag, _op| Err(u32::MAX));

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
        EccCurve::P256,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_err());
}

#[test]
fn test_ecc_gen_key_tag_err() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(|_tag, _curve| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

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
        EccCurve::P256,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag + 1, cmd_info);
    if let Err(err) = end_ecc_gen_key_result {
        assert_eq!(err, HsmErr::PkaTagMismatch);
    }
}

#[test]
fn test_ecc_gen_key_op_tag_err() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(|_tag, _curve| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

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
        EccCurve::P256,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let mut cmd_info = result.unwrap();
    cmd_info.tag = tag + 1;

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    if let Err(err) = end_ecc_gen_key_result {
        assert_eq!(err, HsmErr::PkaTagMismatch);
    }
}

#[test]
fn test_ecc_gen_key_with_keytag() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(|_tag, _curve| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(|_tag, _op| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [1; PkaEccCurve::MAX_LEN],
                        curve: PkaEccCurve::Ecc256,
                    },
                    pub_key: PkaEccPublicKey {
                        data: [1; PkaEccCurve::MAX_LEN * 2],
                        curve: PkaEccCurve::Ecc256,
                    },
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
        Some(12),
        EccCurve::P256,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 1);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN * 2 - 1], 1);
}

fn test_ecc_sign_helper(curve: EccCurve) {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();

    pka.expect_clone().times(1).returning(move || {
        let mut pka = MockPka::new();
        let ecc_curve = curve;
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(move |_, _| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::from(ecc_curve),
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(move |_, _| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [1; PkaEccCurve::MAX_LEN],
                        curve: PkaEccCurve::from(ecc_curve),
                    },
                    pub_key: PkaEccPublicKey {
                        data: [1; PkaEccCurve::MAX_LEN * 2],
                        curve: PkaEccCurve::from(ecc_curve),
                    },
                })
            });
        pka.expect_begin_ecc_sign_zc()
            .times(1)
            .returning(move |_, _, _, _, _| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::from(ecc_curve),
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_sign_zc()
            .times(1)
            .returning(move |_| Ok(()));

        pka
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_is_fips_approved().return_const(false);
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
        curve,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 1);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN * 2 - 1], 1);

    // PKA device requires digest array padded to maximum length (68 bytes)
    let digest_data = vec![0; PkaEccCurve::MAX_LEN];
    let digest_range = IoMemRange::from(digest_data.as_slice());

    let signature_len = 2 * curve.len(); // r + s components
    let signature_data = vec![0; signature_len];
    let signature_range = IoMemRange::from(signature_data.as_slice());

    let hash_algorithm = match curve {
        EccCurve::P256 => DdiHashAlgorithm::Sha256,
        EccCurve::P384 => DdiHashAlgorithm::Sha384,
        EccCurve::P521 => DdiHashAlgorithm::Sha512,
    };
    let begin_ecc_sign_zc_result = app_session.begin_ecc_sign_zc(
        tag,
        EccKeyIn::KeyId(0),
        &digest_range,
        hash_algorithm,
        &signature_range,
    );
    assert!(begin_ecc_sign_zc_result.is_ok());
    let end_ecc_sign_zc_result =
        app_session.end_ecc_sign_zc(tag, begin_ecc_sign_zc_result.unwrap());
    assert!(end_ecc_sign_zc_result.is_ok());
}

#[test]
fn test_ecc_sign_256() {
    test_ecc_sign_helper(EccCurve::P256);
}

#[test]
fn test_ecc_sign_384() {
    test_ecc_sign_helper(EccCurve::P384);
}

#[test]
fn test_ecc_sign_521() {
    test_ecc_sign_helper(EccCurve::P521);
}

#[test]
fn test_ecc_sign_zc_256() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(|_tag, _curve| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(|_tag, _op| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [1; PkaEccCurve::MAX_LEN],
                        curve: PkaEccCurve::Ecc256,
                    },
                    pub_key: PkaEccPublicKey {
                        data: [1; PkaEccCurve::MAX_LEN * 2],
                        curve: PkaEccCurve::Ecc256,
                    },
                })
            });
        pka.expect_begin_ecc_sign_zc().times(1).returning(
            |_tag, _curve, _digest, _privkey, _output| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                })
            },
        );
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_sign_zc()
            .times(1)
            .returning(|_tag| Ok(()));

        pka
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_is_fips_approved().return_const(false);
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
        EccCurve::P256,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 1);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN * 2 - 1], 1);

    let output_buffer = [0u8; PkaEccCurve::MAX_LEN];
    let mbor_byte_array: MborByteArray<{ PkaEccCurve::MAX_LEN }> =
        MborByteArray::new_with_len(output_buffer.as_ptr(), PkaEccCurve::MAX_LEN);
    let begin_ecc_sign_zc_result = app_session.begin_ecc_sign_zc(
        tag,
        EccKeyIn::KeyId(0),
        &(&mbor_byte_array).into(),
        DdiHashAlgorithm::Sha256,
        &(&mbor_byte_array).into(),
    );
    assert!(begin_ecc_sign_zc_result.is_ok());
    let end_ecc_sign_zc_result =
        app_session.end_ecc_sign_zc(tag, begin_ecc_sign_zc_result.unwrap());
    assert!(end_ecc_sign_zc_result.is_ok());
}

#[test]
fn test_begin_ecc_sign_zc_fail() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(|_tag, _curve| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc521,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(|_tag, _op| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [1; PkaEccCurve::MAX_LEN],
                        curve: PkaEccCurve::Ecc521,
                    },
                    pub_key: PkaEccPublicKey {
                        data: [1; PkaEccCurve::MAX_LEN * 2],
                        curve: PkaEccCurve::Ecc521,
                    },
                })
            });
        pka.expect_begin_ecc_sign_zc()
            .times(1)
            .returning(|_tag, _curve, _key, _digest, _sig| Err(u32::MAX));

        pka
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_is_fips_approved().return_const(false);
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
        EccCurve::P521,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 1);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN * 2 - 1], 1);

    // PKA device requires digest array padded to maximum length (68 bytes)
    let digest_data = [0; PkaEccCurve::MAX_LEN];
    let digest_range = IoMemRange::from(digest_data.as_slice());
    let signature_data = vec![0; 132]; // 2 * 66 for P521
    let signature_range = IoMemRange::from(signature_data.as_slice());
    let begin_ecc_sign_zc_result = app_session.begin_ecc_sign_zc(
        tag,
        EccKeyIn::KeyId(0),
        &digest_range,
        DdiHashAlgorithm::Sha512,
        &signature_range,
    );
    assert!(begin_ecc_sign_zc_result.is_err());
    if let Err(err) = begin_ecc_sign_zc_result {
        assert_eq!(err, HsmErr::EccSignFailed);
    }
}

#[test]
fn test_begin_ecc_sign_zc_fail_with_sha1_on_fips_approved() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(|_tag, _curve| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc521,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(|_tag, _op| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [1; PkaEccCurve::MAX_LEN],
                        curve: PkaEccCurve::Ecc521,
                    },
                    pub_key: PkaEccPublicKey {
                        data: [1; PkaEccCurve::MAX_LEN * 2],
                        curve: PkaEccCurve::Ecc521,
                    },
                })
            });
        pka
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_is_fips_approved().return_const(true);
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
        EccCurve::P521,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 1);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN * 2 - 1], 1);

    // PKA device requires digest array padded to maximum length (68 bytes)
    let digest_data = [0; PkaEccCurve::MAX_LEN];
    let digest_range = IoMemRange::from(digest_data.as_slice());
    let signature_data = vec![0; 64]; // 2 * 32 for P256
    let signature_range = IoMemRange::from(signature_data.as_slice());
    let begin_ecc_sign_zc_result = app_session.begin_ecc_sign_zc(
        tag,
        EccKeyIn::KeyId(0),
        &digest_range,
        DdiHashAlgorithm::Sha1,
        &signature_range,
    );
    assert!(begin_ecc_sign_zc_result.is_err());
    if let Err(err) = begin_ecc_sign_zc_result {
        assert_eq!(err, HsmErr::NonFipsApprovedMessageDigest);
    }
}

#[test]
fn test_begin_ecc_sign_zc_fail_with_mismatched_sha_on_fips_approved() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(|_tag, _curve| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc384,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(|_tag, _op| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [1; PkaEccCurve::MAX_LEN],
                        curve: PkaEccCurve::Ecc384,
                    },
                    pub_key: PkaEccPublicKey {
                        data: [1; PkaEccCurve::MAX_LEN * 2],
                        curve: PkaEccCurve::Ecc384,
                    },
                })
            });
        pka
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_is_fips_approved().return_const(true);
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
        EccCurve::P384,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 1);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN * 2 - 1], 1);

    // PKA device requires digest array padded to maximum length (68 bytes)
    let digest_data = [0; PkaEccCurve::MAX_LEN];
    let digest_range = IoMemRange::from(digest_data.as_slice());
    let signature_data = vec![0; 96]; // 2 * 48 for P384
    let signature_range = IoMemRange::from(signature_data.as_slice());
    let begin_ecc_sign_zc_result = app_session.begin_ecc_sign_zc(
        tag,
        EccKeyIn::KeyId(0),
        &digest_range,
        DdiHashAlgorithm::Sha512,
        &signature_range,
    );
    assert!(begin_ecc_sign_zc_result.is_err());
    if let Err(err) = begin_ecc_sign_zc_result {
        assert_eq!(err, HsmErr::DigestHashMismatchWithEccCurve);
    }
}

#[test]
fn test_end_ecc_sign_zc_fail() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(|_tag, _curve| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc521,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(|_tag, _op| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [1; PkaEccCurve::MAX_LEN],
                        curve: PkaEccCurve::Ecc521,
                    },
                    pub_key: PkaEccPublicKey {
                        data: [1; PkaEccCurve::MAX_LEN * 2],
                        curve: PkaEccCurve::Ecc521,
                    },
                })
            });
        pka.expect_begin_ecc_sign_zc()
            .times(1)
            .returning(|_tag, _curve, _key, _digest, _sig| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc521,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_sign_zc()
            .times(1)
            .returning(|_tag| Err(u32::MAX));

        pka
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_is_fips_approved().return_const(false);
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
        EccCurve::P521,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 1);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN * 2 - 1], 1);

    // PKA device requires digest array padded to maximum length (68 bytes)
    let digest_data = [0; PkaEccCurve::MAX_LEN];
    let digest_range = IoMemRange::from(digest_data.as_slice());
    let signature_data = vec![0; 132]; // 2 * 66 for P521
    let signature_range = IoMemRange::from(signature_data.as_slice());
    let begin_ecc_sign_zc_result = app_session.begin_ecc_sign_zc(
        tag,
        EccKeyIn::KeyId(0),
        &digest_range,
        DdiHashAlgorithm::Sha512,
        &signature_range,
    );
    assert!(begin_ecc_sign_zc_result.is_ok());
    let end_ecc_sign_zc_result =
        app_session.end_ecc_sign_zc(tag, begin_ecc_sign_zc_result.unwrap());
    assert!(end_ecc_sign_zc_result.is_err());
    if let Err(err) = end_ecc_sign_zc_result {
        assert_eq!(err, HsmErr::EccSignFailed);
    }
}

#[test]
fn test_ecc_sign_invalid_key() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(|_tag, _curve| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                })
            });
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(|_tag, _op| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [1; PkaEccCurve::MAX_LEN],
                        curve: PkaEccCurve::Ecc256,
                    },
                    pub_key: PkaEccPublicKey {
                        data: [1; PkaEccCurve::MAX_LEN * 2],
                        curve: PkaEccCurve::Ecc256,
                    },
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
        EccCurve::P256,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 1);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN * 2 - 1], 1);

    let digest_data = [0; 32];
    let digest_range = IoMemRange::from(digest_data.as_slice());
    let signature_data = vec![0; 64]; // 2 * 32 for P256
    let signature_range = IoMemRange::from(signature_data.as_slice());
    let begin_ecc_sign_zc_result = app_session.begin_ecc_sign_zc(
        tag,
        EccKeyIn::KeyId(1),
        &digest_range,
        DdiHashAlgorithm::Sha256,
        &signature_range,
    );
    assert!(begin_ecc_sign_zc_result.is_err());
    if let Err(err) = begin_ecc_sign_zc_result {
        assert_eq!(err, HsmErr::InvalidKeyIndex);
    }
}

fn test_ecc_pct_validation_sign_verify(ecc_curve_type: PkaEccCurve) {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 18 * 1024;
    let tag = TagId::default();
    let key_id = KeyId::from(0_u16);

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut pka = MockPka::new();

    pka.expect_clone().times(1).returning({
        let curve = ecc_curve_type;
        move || {
            let mut pka = MockPka::new();

            pka.expect_begin_ecc_gen_key()
                .times(1)
                .returning(move |_tag, _curve| Ok(PkaEccCmd { curve }));
            pka.expect_peek_tag()
                .times(4)
                .returning(|| Some(TagId::default()));
            pka.expect_end_ecc_gen_key()
                .times(1)
                .returning(move |_tag, _op| {
                    Ok(PkaEccKeyPair {
                        priv_key: PkaEccPrivateKey {
                            k: [1; PkaEccCurve::MAX_LEN],
                            curve,
                        },
                        pub_key: PkaEccPublicKey {
                            data: [1; PkaEccCurve::MAX_LEN * 2],
                            curve,
                        },
                    })
                });
            pka.expect_begin_ecc_sign_zc().times(1).returning(
                move |_tag, _curve, _key, _digest, _sig| {
                    Ok(PkaEccCmd {
                        curve: ecc_curve_type,
                    })
                },
            );
            pka.expect_end_ecc_sign_zc()
                .times(1)
                .returning(move |_tag| Ok(()));
            pka.expect_begin_ecc_verify_zc()
                .times(1)
                .returning(move |_tag, _curve, _pubkey, _digest, _sig| Ok(()));
            pka.expect_end_ecc_verify_zc()
                .times(1)
                .returning(move |_tag| Ok(true));

            pka
        }
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();
    let mut sha = MockSha::new();

    sha.expect_clone().times(1).returning(|| {
        let mut sha = MockSha::new();
        sha.expect_digest_zc().once().returning(move |_| Ok(()));

        sha
    });
    hal.expect_sha().return_const(sha);
    hal.expect_dma_heap().times(2).return_const(heap);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_is_fips_approved().return_const(false);
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
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());

    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);

    let public_key = key_pair.pub_key;
    let result =
        app_session.begin_ecc_pct_validation(tag, key_id, EccKeyUsage::SignVerify, public_key);
    assert!(result.is_ok());
    let mut pct_op = result.unwrap();

    let continue_result = app_session.continue_ecc_pct_validation(tag, &mut pct_op);
    assert!(continue_result.is_ok());

    let end_result = app_session.end_ecc_pct_validation(tag, &mut pct_op);
    assert!(end_result.is_ok());
    assert!(end_result.unwrap());
}

#[test]
fn test_ecc_pct_validation_sign_verify_256() {
    test_ecc_pct_validation_sign_verify(PkaEccCurve::Ecc256);
}

#[test]
fn test_ecc_pct_validation_sign_verify_384() {
    test_ecc_pct_validation_sign_verify(PkaEccCurve::Ecc384);
}

#[test]
fn test_ecc_pct_validation_sign_verify_521() {
    test_ecc_pct_validation_sign_verify(PkaEccCurve::Ecc521);
}

fn test_ecc_pct_validation_key_agreement(ecc_curve_type: PkaEccCurve) {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 18 * 1024;
    let tag = TagId::default();
    let key_id = KeyId::from(0_u16);

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut pka = MockPka::new();

    pka.expect_clone().times(1).returning({
        let curve = ecc_curve_type;
        move || {
            let mut pka = MockPka::new();
            pka.expect_begin_ecc_gen_key()
                .times(1)
                .returning(move |_tag, _curve| Ok(PkaEccCmd { curve }));
            pka.expect_peek_tag()
                .times(1)
                .returning(|| Some(TagId::default()));
            pka.expect_end_ecc_gen_key()
                .times(1)
                .returning(move |_tag, _op| {
                    Ok(PkaEccKeyPair {
                        priv_key: PkaEccPrivateKey {
                            k: [1; PkaEccCurve::MAX_LEN],
                            curve,
                        },
                        pub_key: PkaEccPublicKey {
                            data: [1; PkaEccCurve::MAX_LEN * 2],
                            curve,
                        },
                    })
                });

            pka.expect_begin_montgomery_constant_calculation()
                .times(2)
                .returning(|_tag, _curve| Ok(()));
            pka.expect_peek_tag()
                .times(2)
                .returning(|| Some(TagId::default()));
            pka.expect_end_montgomery_constant_calculation()
                .times(2)
                .returning(|_tag| Ok(()));

            pka.expect_begin_ecdh_compute_zc().times(2).returning(
                move |_tag, _curve, _privkey, _pubkey| {
                    Ok(PkaEccCmd {
                        curve: ecc_curve_type,
                    })
                },
            );
            pka.expect_peek_tag()
                .times(2)
                .returning(|| Some(TagId::default()));
            pka.expect_end_ecdh_compute()
                .times(2)
                .returning(move |_tag, _op| {
                    Ok(PkaEccSecretValue {
                        curve: ecc_curve_type,
                        secret: [0; PkaEccCurve::MAX_LEN],
                    })
                });

            pka
        }
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();
    let mut sha = MockSha::new();

    sha.expect_clone().times(1).returning(MockSha::new);
    hal.expect_sha().return_const(sha);
    hal.expect_dma_heap().times(2).return_const(heap);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_clone().times(1).returning(move || {
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

    let public_key = key_pair.pub_key;
    let result =
        app_session.begin_ecc_pct_validation(tag, key_id, EccKeyUsage::KeyAgreement, public_key);
    assert!(result.is_ok());
    let mut pct_op = result.unwrap();

    // Keep calling continue_ecc_pct_validation() until we reach the final state
    while !app_session.is_pct_final_state(&pct_op) {
        let continue_result = app_session.continue_ecc_pct_validation(tag, &mut pct_op);
        assert!(continue_result.is_ok());
    }

    let end_result = app_session.end_ecc_pct_validation(tag, &mut pct_op);
    assert!(end_result.is_ok());
    assert!(end_result.unwrap());
}

#[test]
fn test_ecc_pct_validation_key_agreement_256() {
    test_ecc_pct_validation_key_agreement(PkaEccCurve::Ecc256);
}

#[test]
fn test_ecc_pct_validation_key_agreement_384() {
    test_ecc_pct_validation_key_agreement(PkaEccCurve::Ecc384);
}

#[test]
fn test_ecc_pct_validation_key_agreement_521() {
    test_ecc_pct_validation_key_agreement(PkaEccCurve::Ecc521);
}

#[test]
fn test_ecc_structural_validation_fail_invalid_pub_key_length() {
    let ecc_curve_type = PkaEccCurve::Ecc256;
    pub(crate) const TOTAL_TABLE_LEN: usize = 18 * 1024;
    let tag = TagId::default();
    let key_id = KeyId::from(0_u16);

    let mut pka = MockPka::new();

    // Setup mock PKA
    pka.expect_clone().times(1).returning({
        let curve = ecc_curve_type;
        move || {
            let mut pka = MockPka::new();

            pka.expect_begin_ecc_gen_key()
                .times(1)
                .returning(move |_tag, _curve| Ok(PkaEccCmd { curve }));

            pka.expect_peek_tag()
                .times(1)
                .returning(|| Some(TagId::default()));

            pka.expect_end_ecc_gen_key()
                .times(1)
                .returning(move |_tag, _op| {
                    Ok(PkaEccKeyPair {
                        priv_key: PkaEccPrivateKey {
                            k: [1; PkaEccCurve::MAX_LEN],
                            curve,
                        },
                        pub_key: PkaEccPublicKey {
                            data: [1; PkaEccCurve::MAX_LEN * 2],
                            curve,
                        },
                    })
                });

            pka
        }
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

    // Step 1: Generate ECC Key
    let result = app_session.begin_ecc_gen_key(
        tag,
        None,
        ecc_curve_type.into(),
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());

    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);

    // Step 2: Structural validation
    let curve_len = key_pair.pub_key.curve.len();
    //invlaid length
    let pub_key_blob = key_pair.pub_key.data[..2 * curve_len + 1].to_vec();

    let key_usage = DdiKeyUsage::SignVerify;
    let result = app_session.begin_ecc_structural_validation(tag, key_id, key_usage, pub_key_blob);
    assert!(result.is_err());
}

fn test_ecc_structural_validation_fail_with_scalar_d_and_curve(
    ecc_curve_type: PkaEccCurve,
    d: &[u8],
) {
    pub(crate) const TOTAL_TABLE_LEN: usize = 18 * 1024;
    let tag = TagId::default();
    let key_id = KeyId::from(0_u16);

    let mut pka = MockPka::new();

    // Setup mock PKA
    pka.expect_clone().times(1).returning({
        let curve = ecc_curve_type;
        let d_arc = Arc::new(d.to_vec());

        move || {
            let mut pka = MockPka::new();

            pka.expect_begin_ecc_gen_key()
                .times(1)
                .returning(move |_tag, _curve| Ok(PkaEccCmd { curve }));

            pka.expect_peek_tag()
                .times(1)
                .returning(|| Some(TagId::default()));

            let d_clone = Arc::clone(&d_arc);
            pka.expect_end_ecc_gen_key()
                .times(1)
                .returning(move |_tag, _op| Ok(mock_ecc_keypair(curve, &d_clone)));

            pka
        }
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

    // Step 1: Generate ECC Key
    let result = app_session.begin_ecc_gen_key(
        tag,
        None,
        ecc_curve_type.into(),
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());

    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);

    // Step 2: Structural validation
    let curve_len = key_pair.pub_key.curve.len();
    let pub_key_blob = key_pair.pub_key.data[..2 * curve_len].to_vec();

    let key_usage = DdiKeyUsage::SignVerify;
    let result = app_session.begin_ecc_structural_validation(tag, key_id, key_usage, pub_key_blob);
    assert!(result.is_err());
}

fn test_ecc_structural_validation_pass_with_scalar_d_and_curve(
    ecc_curve_type: PkaEccCurve,
    d: &[u8],
) {
    pub(crate) const TOTAL_TABLE_LEN: usize = 18 * 1024;
    let tag = TagId::default();
    let key_id = KeyId::from(0_u16);

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(1)
        .return_once(|s| Some(MockDmaAlloc::new(s)));

    let mut pka = MockPka::new();

    // Setup mock PKA
    pka.expect_clone().times(1).returning({
        let curve = ecc_curve_type;
        let d_arc = Arc::new(d.to_vec());

        move || {
            let mut pka = MockPka::new();

            pka.expect_begin_ecc_gen_key()
                .times(1)
                .returning(move |_tag, _curve| Ok(PkaEccCmd { curve }));

            pka.expect_peek_tag()
                .times(3)
                .returning(|| Some(TagId::default()));

            let d_clone = Arc::clone(&d_arc);
            pka.expect_end_ecc_gen_key()
                .times(1)
                .returning(move |_tag, _op| Ok(mock_ecc_keypair(curve, &d_clone)));

            pka.expect_begin_montgomery_constant_calculation()
                .times(1)
                .returning(|_tag, _curve| Ok(()));
            pka.expect_end_montgomery_constant_calculation()
                .times(1)
                .returning(|_tag| Ok(()));

            pka.expect_begin_ecc_gen_pub_key_zc().times(1).returning(
                move |_tag, _curve, _privkey, pubkey: &IoMemRange| {
                    unsafe {
                        let buf =
                            core::slice::from_raw_parts_mut(pubkey.addr() as *mut u8, pubkey.len());
                        for b in buf.iter_mut() {
                            *b = 2u8;
                        }
                    }
                    Ok(PkaEccCmd {
                        curve: ecc_curve_type,
                    })
                },
            );
            pka.expect_end_ecc_gen_pub_key_zc()
                .times(1)
                .returning(move |_tag, _op| Ok(()));

            pka
        }
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_dma_heap().times(1).return_const(heap);
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

    let mut app_session = UserSession::new(rev(), 10, state);

    // Step 1: Generate ECC Key
    let result = app_session.begin_ecc_gen_key(
        tag,
        None,
        ecc_curve_type.into(),
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());

    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);

    // Step 2: Structural validation
    let curve_len = key_pair.pub_key.curve.len();
    let pub_key_blob = key_pair.pub_key.data[..2 * curve_len].to_vec();

    let key_usage = DdiKeyUsage::SignVerify;
    let result = app_session.begin_ecc_structural_validation(tag, key_id, key_usage, pub_key_blob);
    assert!(result.is_ok());
    let validation_op = result.unwrap();

    let continue_result = app_session.continue_ecc_structural_validation(validation_op);
    assert!(continue_result.is_ok());
    let validation_op = continue_result.unwrap();

    let result = app_session.end_ecc_structural_validation(validation_op);
    assert!(result.is_ok());
}

#[test]
fn test_structural_validation_fail_p256_zero() {
    let d = [0u8; 32];
    test_ecc_structural_validation_fail_with_scalar_d_and_curve(PkaEccCurve::Ecc256, &d)
}

#[test]
fn test_structural_validation_fail_p384_zero() {
    let d = [0u8; 48];
    test_ecc_structural_validation_fail_with_scalar_d_and_curve(PkaEccCurve::Ecc384, &d)
}

#[test]
fn test_structural_validation_fail_p521_zero() {
    let d = [0u8; 68];
    test_ecc_structural_validation_fail_with_scalar_d_and_curve(PkaEccCurve::Ecc521, &d)
}

#[test]
fn test_structural_validation_fail_p256_large() {
    let d = [0xFFu8; 32];
    test_ecc_structural_validation_fail_with_scalar_d_and_curve(PkaEccCurve::Ecc256, &d)
}

#[test]
fn test_structural_validation_fail_p384_large() {
    let d = [0xFFu8; 48];
    test_ecc_structural_validation_fail_with_scalar_d_and_curve(PkaEccCurve::Ecc384, &d)
}

#[test]
fn test_structural_validation_fail_p521_large() {
    let d = [0xFFu8; 68];
    test_ecc_structural_validation_fail_with_scalar_d_and_curve(PkaEccCurve::Ecc521, &d)
}

#[test]
fn test_structural_validation_pass_p256_d_one() {
    let mut d = [0u8; 32];
    // little-endian
    d[0] = 1;
    test_ecc_structural_validation_pass_with_scalar_d_and_curve(PkaEccCurve::Ecc256, &d)
}

#[test]
fn test_structural_validation_pass_p384_d_one() {
    let mut d = [0u8; 48];
    d[0] = 1;
    test_ecc_structural_validation_pass_with_scalar_d_and_curve(PkaEccCurve::Ecc384, &d)
}

#[test]
fn test_structural_validation_pass_p521_d_one() {
    let mut d = [0u8; 68];
    d[0] = 1;
    test_ecc_structural_validation_pass_with_scalar_d_and_curve(PkaEccCurve::Ecc521, &d)
}

#[test]
fn test_structural_validation_fail_p256_d_equal_order() {
    let d = PkaEccCurve::get_order(PkaEccCurve::Ecc256).n;
    test_ecc_structural_validation_fail_with_scalar_d_and_curve(PkaEccCurve::Ecc256, &d)
}

#[test]
fn test_structural_validation_fail_p384_d_equal_order() {
    let d = PkaEccCurve::get_order(PkaEccCurve::Ecc384).n;
    test_ecc_structural_validation_fail_with_scalar_d_and_curve(PkaEccCurve::Ecc384, &d)
}

#[test]
fn test_structural_validation_fail_p521_d_equal_order() {
    let d = PkaEccCurve::get_order(PkaEccCurve::Ecc521).n;
    test_ecc_structural_validation_fail_with_scalar_d_and_curve(PkaEccCurve::Ecc521, &d)
}

fn subtract_one(buf: &mut [u8]) {
    for byte in buf.iter_mut().rev() {
        if *byte > 0 {
            *byte -= 1;
            break;
        } else {
            *byte = 0xFF; // Borrow
        }
    }
}

#[test]
fn test_structural_validation_pass_p256_d_equal_order_minus_one() {
    let mut d = PkaEccCurve::get_order(PkaEccCurve::Ecc256).n[0..32].to_vec();
    subtract_one(&mut d);
    test_ecc_structural_validation_pass_with_scalar_d_and_curve(PkaEccCurve::Ecc256, &d)
}

#[test]
fn test_structural_validation_pass_p384_d_equal_order_minus_one() {
    let mut d = PkaEccCurve::get_order(PkaEccCurve::Ecc384).n[0..48].to_vec();
    subtract_one(&mut d);
    test_ecc_structural_validation_pass_with_scalar_d_and_curve(PkaEccCurve::Ecc384, &d)
}

#[test]
fn test_structural_validation_pass_p521_d_equal_order_minus_one() {
    let mut d = PkaEccCurve::get_order(PkaEccCurve::Ecc521).n;
    subtract_one(&mut d);
    test_ecc_structural_validation_pass_with_scalar_d_and_curve(PkaEccCurve::Ecc521, &d)
}

#[test]
fn test_ecc_gen_pub_key_256() {
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
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Ok(()));

        pka.expect_begin_ecc_gen_pub_key_zc().times(1).returning(
            move |_tag, _curve, _privkey, pubkey: &IoMemRange| {
                unsafe {
                    let buf =
                        core::slice::from_raw_parts_mut(pubkey.addr() as *mut u8, pubkey.len());
                    for b in buf.iter_mut() {
                        *b = 2u8;
                    }
                }
                Ok(PkaEccCmd {
                    curve: ecc_curve_type,
                })
            },
        );
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_pub_key_zc()
            .times(1)
            .returning(move |_tag, _op| Ok(()));

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
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 2);

    let result = app_session.begin_ecc_gen_pub_key(tag, 0);
    assert!(result.is_ok());
    let ecc_gen_pub_key_info = result.unwrap();

    let mut pub_key_buf = [0u8; PkaEccCurve::MAX_LEN * 2];
    let pub_key_range = IoMemRange::from(pub_key_buf.as_mut_slice());
    let continue_ecc_gen_key_result =
        app_session.continue_ecc_gen_pub_key_zc(ecc_gen_pub_key_info, &pub_key_range);
    assert!(continue_ecc_gen_key_result.is_ok());
    let op = continue_ecc_gen_key_result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_pub_key_zc(op);
    assert!(end_ecc_gen_key_result.is_ok());
    // The PKA mock writes the public key into the provided IoMemRange; construct
    // a PkaEccPublicKey from the filled buffer and assert on its coordinates.
    let pub_key_result = PkaEccPublicKey {
        data: pub_key_buf,
        curve: ecc_curve_type,
    };
    assert_eq!(pub_key_result.x()[pub_key_result.curve.len() - 1], 2);
    assert_eq!(pub_key_result.y()[pub_key_result.curve.len() - 1], 2);
}

#[test]
fn test_ecc_gen_pub_key_384() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type = PkaEccCurve::Ecc384;
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
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Ok(()));

        pka.expect_begin_ecc_gen_pub_key_zc().times(1).returning(
            move |_tag, _curve, _privkey, pubkey: &IoMemRange| {
                unsafe {
                    let buf =
                        core::slice::from_raw_parts_mut(pubkey.addr() as *mut u8, pubkey.len());
                    for b in buf.iter_mut() {
                        *b = 2u8;
                    }
                }
                Ok(PkaEccCmd {
                    curve: ecc_curve_type,
                })
            },
        );
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_pub_key_zc()
            .times(1)
            .returning(move |_tag, _op| Ok(()));

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
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 2);

    let result = app_session.begin_ecc_gen_pub_key(tag, 0);
    assert!(result.is_ok());
    let ecc_gen_pub_key_info = result.unwrap();

    let mut pub_key_buf = [0u8; PkaEccCurve::MAX_LEN * 2];
    let pub_key_range = IoMemRange::from(pub_key_buf.as_mut_slice());
    let continue_ecc_gen_key_result =
        app_session.continue_ecc_gen_pub_key_zc(ecc_gen_pub_key_info, &pub_key_range);
    assert!(continue_ecc_gen_key_result.is_ok());
    let op = continue_ecc_gen_key_result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_pub_key_zc(op);
    assert!(end_ecc_gen_key_result.is_ok());
    let pub_key_result = PkaEccPublicKey {
        data: pub_key_buf,
        curve: ecc_curve_type,
    };
    assert_eq!(pub_key_result.x()[pub_key_result.curve.len() - 1], 2);
    assert_eq!(pub_key_result.y()[pub_key_result.curve.len() - 1], 2);
}

#[test]
fn test_ecc_gen_pub_key_521() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type = PkaEccCurve::Ecc521;
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
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Ok(()));

        pka.expect_begin_ecc_gen_pub_key_zc().times(1).returning(
            move |_tag, _curve, _privkey, pubkey: &IoMemRange| {
                unsafe {
                    let buf =
                        core::slice::from_raw_parts_mut(pubkey.addr() as *mut u8, pubkey.len());
                    for b in buf.iter_mut() {
                        *b = 2u8;
                    }
                }
                Ok(PkaEccCmd {
                    curve: ecc_curve_type,
                })
            },
        );
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_pub_key_zc()
            .times(1)
            .returning(move |_tag, _op| Ok(()));

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
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 2);

    let result = app_session.begin_ecc_gen_pub_key(tag, 0);
    assert!(result.is_ok());
    let ecc_gen_pub_key_info = result.unwrap();

    let mut pub_key_buf = [0u8; PkaEccCurve::MAX_LEN * 2];
    let pub_key_range = IoMemRange::from(pub_key_buf.as_mut_slice());
    let continue_ecc_gen_key_result =
        app_session.continue_ecc_gen_pub_key_zc(ecc_gen_pub_key_info, &pub_key_range);
    assert!(continue_ecc_gen_key_result.is_ok());
    let op = continue_ecc_gen_key_result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_pub_key_zc(op);
    assert!(end_ecc_gen_key_result.is_ok());
    let pub_key_result = PkaEccPublicKey {
        data: pub_key_buf,
        curve: ecc_curve_type,
    };
    assert_eq!(pub_key_result.x()[pub_key_result.curve.len() - 1], 2);
    assert_eq!(pub_key_result.y()[pub_key_result.curve.len() - 1], 2);
}

#[test]
fn test_ecc_gen_pub_key_256_zc() {
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
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Ok(()));

        pka.expect_begin_ecc_gen_pub_key_zc().times(1).returning(
            move |_tag, _curve, _privkey, pubkey: &IoMemRange| {
                unsafe {
                    let buf =
                        core::slice::from_raw_parts_mut(pubkey.addr() as *mut u8, pubkey.len());
                    for b in buf.iter_mut() {
                        *b = 2u8;
                    }
                }
                Ok(PkaEccCmd {
                    curve: ecc_curve_type,
                })
            },
        );
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_pub_key_zc()
            .times(1)
            .returning(move |_tag, _op| Ok(()));

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
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 2);

    let result = app_session.begin_ecc_gen_pub_key(tag, 0);
    assert!(result.is_ok());
    let ecc_gen_pub_key_info = result.unwrap();

    let mut pub_key_buf = [0u8; PkaEccCurve::MAX_LEN * 2];
    let pub_key_range = IoMemRange::from(pub_key_buf.as_mut_slice());
    let continue_ecc_gen_key_result =
        app_session.continue_ecc_gen_pub_key_zc(ecc_gen_pub_key_info, &pub_key_range);
    assert!(continue_ecc_gen_key_result.is_ok());
    let op = continue_ecc_gen_key_result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_pub_key_zc(op);
    assert!(end_ecc_gen_key_result.is_ok());
}

#[test]
fn test_ecc_gen_pub_key_invalid_key() {
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

    let begin_ecc_gen_key_result = app_session.begin_ecc_gen_key(
        tag,
        None,
        ecc_curve_type.into(),
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(begin_ecc_gen_key_result.is_ok());
    let cmd_info = begin_ecc_gen_key_result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 2);

    let result = app_session.begin_ecc_gen_pub_key(tag, 1);
    assert!(result.is_err());
    if let Err(err) = result {
        assert_eq!(err, HsmErr::InvalidKeyIndex);
    }
}

#[test]
fn test_begin_montgomery_const_calc_fail() {
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
            .returning(|_tag, _curve| Err(u32::MAX));

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
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 2);

    let result = app_session.begin_ecc_gen_pub_key(tag, 0);
    assert!(result.is_err());
    if let Err(err) = result {
        assert_eq!(err, HsmErr::EccMontgomeryConstCalcFailed);
    }
}

#[test]
fn test_end_montgomery_const_calc_fail() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type = PkaEccCurve::Ecc521;
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
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Err(u32::MAX));

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
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 2);

    let result = app_session.begin_ecc_gen_pub_key(tag, 0);
    assert!(result.is_ok());
    let ecc_gen_pub_key_info = result.unwrap();

    let mut pub_key_buf = [0u8; PkaEccCurve::MAX_LEN * 2];
    let pub_key_range = IoMemRange::from(pub_key_buf.as_mut_slice());
    let continue_ecc_gen_key_result =
        app_session.continue_ecc_gen_pub_key_zc(ecc_gen_pub_key_info, &pub_key_range);
    assert!(continue_ecc_gen_key_result.is_err());
    if let Err(err) = continue_ecc_gen_key_result {
        assert_eq!(err, HsmErr::EccMontgomeryConstCalcFailed);
    }
}

#[test]
fn test_ecc_pct_validation_sign_verify_failure() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 18 * 1024;
    let ecc_curve_type = PkaEccCurve::Ecc256;
    let tag = TagId::default();
    let key_id = KeyId::from(0_u16);

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut pka = MockPka::new();

    pka.expect_clone().times(1).returning({
        let curve = ecc_curve_type;
        move || {
            let mut pka = MockPka::new();

            pka.expect_begin_ecc_gen_key()
                .times(1)
                .returning(move |_tag, _curve| Ok(PkaEccCmd { curve }));
            pka.expect_peek_tag()
                .times(4)
                .returning(|| Some(TagId::default()));
            pka.expect_end_ecc_gen_key()
                .times(1)
                .returning(move |_tag, _op| {
                    Ok(PkaEccKeyPair {
                        priv_key: PkaEccPrivateKey {
                            k: [1; PkaEccCurve::MAX_LEN],
                            curve,
                        },
                        pub_key: PkaEccPublicKey {
                            data: [1; PkaEccCurve::MAX_LEN * 2],
                            curve,
                        },
                    })
                });
            pka.expect_begin_ecc_sign_zc().times(1).returning(
                move |_tag, _curve, _key, _digest, _sig| {
                    Ok(PkaEccCmd {
                        curve: ecc_curve_type,
                    })
                },
            );
            pka.expect_end_ecc_sign_zc()
                .times(1)
                .returning(move |_tag| Ok(()));
            pka.expect_begin_ecc_verify_zc()
                .times(1)
                .returning(move |_tag, _curve, _pubkey, _digest, _sig| Ok(()));
            pka.expect_end_ecc_verify_zc()
                .times(1)
                .returning(move |_tag| Ok(false));

            pka
        }
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();
    let mut sha = MockSha::new();

    sha.expect_clone().times(1).returning(|| {
        let mut sha = MockSha::new();
        sha.expect_digest_zc().once().returning(move |_| Ok(()));

        sha
    });
    hal.expect_sha().return_const(sha);
    hal.expect_dma_heap().times(2).return_const(heap);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_is_fips_approved().return_const(true);
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
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());

    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);

    let public_key = key_pair.pub_key;
    let result =
        app_session.begin_ecc_pct_validation(tag, key_id, EccKeyUsage::SignVerify, public_key);
    assert!(result.is_ok());
    let mut pct_op = result.unwrap();

    let continue_result = app_session.continue_ecc_pct_validation(tag, &mut pct_op);
    assert!(continue_result.is_ok());

    let end_result = app_session.end_ecc_pct_validation(tag, &mut pct_op);
    assert!(end_result.is_ok());
    assert!(!end_result.unwrap());
}

#[test]
fn test_ecc_pct_validation_key_agreement_failure() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 18 * 1024;
    let ecc_curve_type = PkaEccCurve::Ecc256;
    let tag = TagId::default();
    let key_id = KeyId::from(0_u16);

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut pka = MockPka::new();

    pka.expect_clone().times(1).returning({
        let curve = ecc_curve_type;
        move || {
            let mut pka = MockPka::new();
            pka.expect_begin_ecc_gen_key()
                .times(1)
                .returning(move |_tag, _curve| Ok(PkaEccCmd { curve }));
            pka.expect_peek_tag()
                .times(1)
                .returning(|| Some(TagId::default()));
            pka.expect_end_ecc_gen_key()
                .times(1)
                .returning(move |_tag, _op| {
                    Ok(PkaEccKeyPair {
                        priv_key: PkaEccPrivateKey {
                            k: [1; PkaEccCurve::MAX_LEN],
                            curve,
                        },
                        pub_key: PkaEccPublicKey {
                            data: [1; PkaEccCurve::MAX_LEN * 2],
                            curve,
                        },
                    })
                });

            pka.expect_begin_montgomery_constant_calculation()
                .times(2)
                .returning(|_tag, _curve| Ok(()));
            pka.expect_peek_tag()
                .times(2)
                .returning(|| Some(TagId::default()));
            pka.expect_end_montgomery_constant_calculation()
                .times(2)
                .returning(|_tag| Ok(()));

            pka.expect_begin_ecdh_compute_zc().times(2).returning(
                move |_tag, _curve, _privkey, _pubkey| {
                    Ok(PkaEccCmd {
                        curve: ecc_curve_type,
                    })
                },
            );
            pka.expect_peek_tag()
                .times(2)
                .returning(|| Some(TagId::default()));
            pka.expect_end_ecdh_compute()
                .times(1)
                .returning(move |_tag, _op| {
                    Ok(PkaEccSecretValue {
                        curve: ecc_curve_type,
                        secret: [0; PkaEccCurve::MAX_LEN],
                    })
                });
            pka.expect_end_ecdh_compute()
                .times(1)
                .returning(move |_tag, _op| {
                    Ok(PkaEccSecretValue {
                        curve: ecc_curve_type,
                        secret: [1; PkaEccCurve::MAX_LEN],
                    })
                });

            pka
        }
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();
    let mut sha = MockSha::new();

    sha.expect_clone().times(1).returning(MockSha::new);
    hal.expect_sha().return_const(sha);
    hal.expect_dma_heap().times(2).return_const(heap);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_clone().times(1).returning(move || {
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

    let public_key = key_pair.pub_key;
    let result =
        app_session.begin_ecc_pct_validation(tag, key_id, EccKeyUsage::KeyAgreement, public_key);
    assert!(result.is_ok());
    let mut pct_op = result.unwrap();

    // Keep calling continue_ecc_pct_validation() until we reach the final state
    while !app_session.is_pct_final_state(&pct_op) {
        let continue_result = app_session.continue_ecc_pct_validation(tag, &mut pct_op);
        assert!(continue_result.is_ok());
    }

    let end_result = app_session.end_ecc_pct_validation(tag, &mut pct_op);
    assert!(end_result.is_ok());
    assert!(!end_result.unwrap());
}

#[test]
fn test_begin_ecc_gen_pub_key_fail() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type = PkaEccCurve::Ecc521;
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
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Ok(()));

        pka.expect_begin_ecc_gen_pub_key_zc()
            .times(1)
            .returning(move |_tag, _curve, _privkey, _pubkey| Err(u32::MAX));

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
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 2);

    let result = app_session.begin_ecc_gen_pub_key(tag, 0);
    assert!(result.is_ok());
    let ecc_gen_pub_key_info = result.unwrap();

    let mut pub_key_buf = [0u8; PkaEccCurve::MAX_LEN * 2];
    let pub_key_range = IoMemRange::from(pub_key_buf.as_mut_slice());
    let continue_ecc_gen_key_result =
        app_session.continue_ecc_gen_pub_key_zc(ecc_gen_pub_key_info, &pub_key_range);
    assert!(continue_ecc_gen_key_result.is_err());
    if let Err(err) = continue_ecc_gen_key_result {
        assert_eq!(err, HsmErr::EccGenPubKeyFailed);
    }
}

#[test]
fn test_end_ecc_gen_pub_key_fail() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type = PkaEccCurve::Ecc521;
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
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Ok(()));

        pka.expect_begin_ecc_gen_pub_key_zc().times(1).returning(
            move |_tag, _curve, _privkey, pubkey: &IoMemRange| {
                unsafe {
                    let buf =
                        core::slice::from_raw_parts_mut(pubkey.addr() as *mut u8, pubkey.len());
                    for b in buf.iter_mut() {
                        *b = 2u8;
                    }
                }
                Ok(PkaEccCmd {
                    curve: ecc_curve_type,
                })
            },
        );
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_pub_key_zc()
            .times(1)
            .returning(move |_tag, _op| Err(u32::MAX));

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
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert_eq!(key_pair.ecc_key.id(), 0);
    assert_eq!(key_pair.pub_key.data[EccCurve::MAX_LEN - 1], 2);

    let result = app_session.begin_ecc_gen_pub_key(tag, 0);
    assert!(result.is_ok());
    let ecc_gen_pub_key_info = result.unwrap();

    // allocate a local buffer so the IoMemRange points to live memory
    let mut pub_key_buf = [0u8; 64];
    let pub_key_range = IoMemRange::from(pub_key_buf.as_mut_slice());
    let continue_ecc_gen_key_result =
        app_session.continue_ecc_gen_pub_key_zc(ecc_gen_pub_key_info, &pub_key_range);
    assert!(continue_ecc_gen_key_result.is_ok());
    let op = continue_ecc_gen_key_result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_pub_key_zc(op);
    assert!(end_ecc_gen_key_result.is_err());
    if let Err(err) = end_ecc_gen_key_result {
        assert_eq!(err, HsmErr::EccGenPubKeyFailed);
    }
}

#[test]
fn test_ecdh_compute_256_zc() {
    test_ecdh_compute_helper(PkaEccCurve::Ecc256);
}

#[test]
fn test_ecdh_compute_384_zc() {
    test_ecdh_compute_helper(PkaEccCurve::Ecc384);
}

#[test]
fn test_ecdh_compute_521_zc() {
    test_ecdh_compute_helper(PkaEccCurve::Ecc521);
}

#[test]
fn test_begin_ecdh_compute_key_not_found() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type: PkaEccCurve = PkaEccCurve::Ecc256;
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

    // convert the public key der to a input ref to IoMemRange
    let pub_key_der = IoMemRange::from(TEST_RAW_ECC_256_PUBLIC_KEY.as_slice());

    // Deliberately choose a key ID that does not exist.
    let result = app_session.begin_ecdh_compute_with_pub_key_validation(
        tag,
        2,
        DdiKeyType::Secret256,
        &pub_key_der,
    );
    assert!(result.is_err());
}

fn test_ecdh_compute_helper(ecc_curve_type: PkaEccCurve) {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
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

    let (key_type, raw_pub_key) = match ecc_curve_type {
        PkaEccCurve::Ecc256 => (DdiKeyType::Secret256, TEST_RAW_ECC_256_PUBLIC_KEY.to_vec()),
        PkaEccCurve::Ecc384 => (DdiKeyType::Secret384, TEST_RAW_ECC_384_PUBLIC_KEY.to_vec()),
        PkaEccCurve::Ecc521 => (DdiKeyType::Secret521, TEST_RAW_ECC_521_PUBLIC_KEY.to_vec()),
    };
    // convert the public key der to a input ref to IoMemRange
    let raw_pub_key = IoMemRange::from(raw_pub_key.as_slice());
    let result =
        app_session.begin_ecdh_compute_with_pub_key_validation(tag, 0, key_type, &raw_pub_key);
    let op = match result {
        Ok(op) => op,
        Err(err) => panic!("Failed to begin ECDH compute: {:?}", err),
    };

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
}

#[test]
fn test_begin_ecdh_compute_begin_mont_const_failed() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type: PkaEccCurve = PkaEccCurve::Ecc256;
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
            .returning(|_tag, _curve| Err(u32::MAX));

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
    if let Err(err) = result {
        assert_eq!(err, HsmErr::EccMontgomeryConstCalcFailed);
    }
}

#[test]
fn test_continue_ecdh_compute_invalid_state() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type: PkaEccCurve = PkaEccCurve::Ecc256;
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

    let mut result = app_session
        .begin_ecdh_compute_with_pub_key_validation(tag, 0, DdiKeyType::Secret256, &raw_pub_key)
        .unwrap();
    result.state = EcdhComputeCmdState::EcdhCompute;

    let continue_result = app_session.continue_ecdh_compute_zc(result, &raw_pub_key);
    if let Err(result) = continue_result {
        assert_eq!(result, HsmErr::InvalidState);
    }
}

#[test]
fn test_begin_ecdh_compute_fail() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type: PkaEccCurve = PkaEccCurve::Ecc256;
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

        pka.expect_begin_ecdh_compute_zc()
            .times(1)
            .returning(move |_tag, _curve, _privkey, _pubkey| Err(u32::MAX));

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
    let ecdh_info = result.unwrap();

    let continue_ecdh_result = app_session.continue_ecdh_compute_zc(ecdh_info, &raw_pub_key);
    assert!(continue_ecdh_result.is_ok());

    let ecdh_info = continue_ecdh_result.unwrap();

    let continue_ecdh_result = app_session.continue_ecdh_compute_zc(ecdh_info, &raw_pub_key);
    if let Err(err) = continue_ecdh_result {
        assert_eq!(err, HsmErr::EcdhComputeFailed);
    }
}

#[test]
fn test_end_ecdh_compute_fail() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type: PkaEccCurve = PkaEccCurve::Ecc256;
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
            .returning(move |_tag, _op| Err(u32::MAX));

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

    let pub_key_der = match ecc_curve_type {
        PkaEccCurve::Ecc256 => IoMemRange::from(TEST_RAW_ECC_256_PUBLIC_KEY.as_slice()),
        PkaEccCurve::Ecc384 => IoMemRange::from(TEST_RAW_ECC_384_PUBLIC_KEY.as_slice()),
        PkaEccCurve::Ecc521 => IoMemRange::from(TEST_RAW_ECC_521_PUBLIC_KEY.as_slice()),
    };

    let result = app_session.begin_ecdh_compute_with_pub_key_validation(
        tag,
        0,
        DdiKeyType::Secret256,
        &pub_key_der,
    );
    assert!(result.is_ok());
    let op = result.unwrap();

    let continue_ecdh_result = app_session.continue_ecdh_compute_zc(op, &pub_key_der);
    assert!(continue_ecdh_result.is_ok());
    let op = continue_ecdh_result.unwrap();

    let continue_ecdh_result = app_session.continue_ecdh_compute_zc(op, &pub_key_der);
    assert!(continue_ecdh_result.is_ok());
    let op = continue_ecdh_result.unwrap();

    let end_ecdh_compute_result =
        app_session.end_ecdh_compute(op, DdiKeyUsage::Derive, None, KeyAvailability::App);
    if let Err(err) = end_ecdh_compute_result {
        assert_eq!(err, HsmErr::EcdhComputeFailed);
    }
}

#[test]
fn test_ecdh_compute_begin_point_validation_failed() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type: PkaEccCurve = PkaEccCurve::Ecc256;
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
            .returning(|_, _, _| Err(u32::MAX));

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

    let pub_key_der = match ecc_curve_type {
        PkaEccCurve::Ecc256 => IoMemRange::from(TEST_RAW_ECC_256_PUBLIC_KEY.as_slice()),
        PkaEccCurve::Ecc384 => IoMemRange::from(TEST_RAW_ECC_384_PUBLIC_KEY.as_slice()),
        PkaEccCurve::Ecc521 => IoMemRange::from(TEST_RAW_ECC_521_PUBLIC_KEY.as_slice()),
    };

    let result = app_session.begin_ecdh_compute_with_pub_key_validation(
        tag,
        0,
        DdiKeyType::Secret256,
        &pub_key_der,
    );
    assert!(result.is_ok());
    let op = result.unwrap();

    let continue_ecdh_result = app_session.continue_ecdh_compute_zc(op, &pub_key_der);
    assert!(continue_ecdh_result.is_err());
}

#[test]
fn test_ecdh_compute_end_point_validation_failed() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type: PkaEccCurve = PkaEccCurve::Ecc256;
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
            .returning(|_| Err(u32::MAX));

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

    let pub_key_der = match ecc_curve_type {
        PkaEccCurve::Ecc256 => IoMemRange::from(TEST_RAW_ECC_256_PUBLIC_KEY.as_slice()),
        PkaEccCurve::Ecc384 => IoMemRange::from(TEST_RAW_ECC_384_PUBLIC_KEY.as_slice()),
        PkaEccCurve::Ecc521 => IoMemRange::from(TEST_RAW_ECC_521_PUBLIC_KEY.as_slice()),
    };

    let result = app_session.begin_ecdh_compute_with_pub_key_validation(
        tag,
        0,
        DdiKeyType::Secret256,
        &pub_key_der,
    );
    assert!(result.is_ok());
    let op = result.unwrap();

    let op = app_session
        .continue_ecdh_compute_zc(op, &pub_key_der)
        .unwrap();

    assert!(app_session
        .continue_ecdh_compute_zc(op, &pub_key_der)
        .is_err());
}

#[test]
fn test_ecdh_compute_point_validation_failed_with_invalid_public_key() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type: PkaEccCurve = PkaEccCurve::Ecc256;
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
            .returning(|_| Ok(false));

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

    let pub_key_der = match ecc_curve_type {
        PkaEccCurve::Ecc256 => IoMemRange::from(TEST_RAW_ECC_256_PUBLIC_KEY.as_slice()),
        PkaEccCurve::Ecc384 => IoMemRange::from(TEST_RAW_ECC_384_PUBLIC_KEY.as_slice()),
        PkaEccCurve::Ecc521 => IoMemRange::from(TEST_RAW_ECC_521_PUBLIC_KEY.as_slice()),
    };

    let result = app_session.begin_ecdh_compute_with_pub_key_validation(
        tag,
        0,
        DdiKeyType::Secret256,
        &pub_key_der,
    );
    assert!(result.is_ok());
    let op = result.unwrap();

    let op = app_session
        .continue_ecdh_compute_zc(op, &pub_key_der)
        .unwrap();

    assert!(app_session
        .continue_ecdh_compute_zc(op, &pub_key_der)
        .is_err());
}

#[test]
fn test_ecdh_compute_pub_key_assurance_failed() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type: PkaEccCurve = PkaEccCurve::Ecc256;
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

    let raw_pub_key = IoMemRange::from([0x00, 64].as_slice());

    let result = app_session.begin_ecdh_compute_with_pub_key_validation(
        tag,
        0,
        DdiKeyType::Secret256,
        &raw_pub_key,
    );
    assert!(result.is_err());
}

#[test]
fn test_end_ecdh_compute_import_key_fail() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type: PkaEccCurve = PkaEccCurve::Ecc256;
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
            .returning(|_| Ok(true));
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
        app_session.end_ecdh_compute(op, DdiKeyUsage::Derive, Some(100), KeyAvailability::Session);
    if let Err(err) = end_ecdh_compute_result {
        assert_eq!(err, HsmErr::InvalidArgument);
    }
}

#[test]
fn test_end_ecdh_compute_invalid_permissions() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let ecc_curve_type: PkaEccCurve = PkaEccCurve::Ecc256;
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
        app_session.end_ecdh_compute(op, DdiKeyUsage::EncryptDecrypt, None, KeyAvailability::App);
    if let Err(err) = end_ecdh_compute_result {
        assert_eq!(err, HsmErr::InvalidPermissions);
    }
}

fn mock_ecc_keypair(curve: PkaEccCurve, d: &[u8]) -> PkaEccKeyPair {
    let mut k = [1u8; PkaEccCurve::MAX_LEN];
    k[..d.len()].copy_from_slice(d);
    PkaEccKeyPair {
        priv_key: PkaEccPrivateKey { k, curve },
        pub_key: PkaEccPublicKey {
            data: [2; PkaEccCurve::MAX_LEN * 2],
            curve,
        },
    }
}
