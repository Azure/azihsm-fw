// Copyright (c) Microsoft Corporation. All rights reserved.

use cred_mgr::APP_VAULT_ID_FOR_INTERNAL_KEYS;
use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;
use mcr_crypto_pka::PkaEccKeyPair;
use mcr_crypto_pka::PkaEccPrivateKey;
use mcr_crypto_pka::PkaEccPublicKey;
use mcr_crypto_pka::PkaEccSecretValue;
use mcr_crypto_pka::PkaRsaCmd;
use mcr_crypto_pka::PkaRsaData;
use mcr_crypto_pka::PkaRsaMontData;
use mcr_crypto_pka::PkaRsaSize;
use mcr_ddi_mbor::MborByteArray;
use mcr_ddi_types::DdiKeyType;
use mcr_ddi_types::DdiKeyUsage;
use mcr_types::*;
use openssl::nid::*;

use super::*;
use crate::cmd_scheduler::TagId;
use crate::error::HsmErr;
use crate::heap::*;
use crate::mock::MockAes;
use crate::mock::MockDmaAlloc;
use crate::mock::MockDmaHeap;
use crate::mock::MockHal;
use crate::mock::MockPka;
use crate::mock::MockSha;
use crate::mock::*;
use crate::partition::AesKeyImported;
use crate::partition::AesKeyKind;
use crate::partition::AesKeyUsage;
use crate::partition::EccCurve;
use crate::partition::EccGenPubKeyCmd;
use crate::partition::EccKeyUsage;
use crate::partition::EntryClass;
use crate::partition::EntryKind;
use crate::partition::HsmSession;
use crate::partition::HsmUserSession;
use crate::partition::KeyAvailability;
use crate::partition::OpenKeyPhase;
use crate::partition::PartEnv;
use crate::partition::PartState;
use crate::partition::RsaCrtParamCalcState;
use crate::partition::SessionKeyKind;
use crate::partition::SessionKeyToImport;
use crate::partition::SessionKeyUsage;
use crate::partition::UserSession;

const TEST_DIGEST_VAL: u8 = 1;

#[test]
fn test_clone() {
    let mut hal = MockHal::new();

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
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
    let app_session = UserSession::new(rev(), 10, state);

    let cloned_app_session = app_session.clone();

    assert_eq!(cloned_app_session.id(), app_session.id());
}

#[test]
fn test_api_rev() {
    let mut hal = MockHal::new();

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
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
    let app_session = UserSession::new(rev(), 10, state);

    assert_eq!(app_session.api_rev(), rev());
}

#[test]
fn test_valid() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
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

    state.rgs_mut().set_mask(1);
    let mut vault = state.vault();
    let key_to_import = SessionKeyToImport::new(
        SessionKeyKind::Session,
        SessionKeyUsage::Session,
        &[1, 0, 0, 0, 0, 0, 0, 0],
        &[1u8; 80],
    )
    .unwrap();
    let session_key = vault
        .import_session_key(state.cred_mgr().get_user_vault_id(), &key_to_import)
        .unwrap();
    let session_id = state
        .session_table()
        .create_session(session_key.id())
        .unwrap();

    let app_session = UserSession::new(rev(), session_id, state);

    assert!(app_session.valid());
}

#[test]
fn test_invalidate() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
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

    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());

    let state = PartState::new(PcieFunction(0), env);

    state.rgs_mut().set_mask(1);
    let mut vault = state.vault();
    let key_to_import = SessionKeyToImport::new(
        SessionKeyKind::Session,
        SessionKeyUsage::Session,
        &[1, 0, 0, 0, 0, 0, 0, 0],
        &[1u8; 80],
    )
    .unwrap();
    let session_key = vault
        .import_session_key(state.cred_mgr().get_user_vault_id(), &key_to_import)
        .unwrap();
    let session_id = state
        .session_table()
        .create_session(session_key.id())
        .unwrap();

    let mut app_session = UserSession::new(rev(), session_id, state);

    assert!(app_session.valid());
    app_session.invalidate();

    assert!(!app_session.valid());
}

#[test]
fn test_app_vault_id() {
    let app_vault_id = 0;
    let mut hal = MockHal::new();

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
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
    let app_session = UserSession::new(rev(), 10, state);

    assert_eq!(app_session.app_vault_id(), app_vault_id);
}

#[test]
fn test_open_key_aes128() {
    let key_kind = AesKeyKind::Aes128;

    let mut hal = set_up_hal_for_key_tests();

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 16usize]);
    });
    hal.expect_rng().once().return_const(rng);
    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let mut app_session = UserSession::new(rev(), 10, state);

    let gen_key_result = app_session.aes_gen_key(
        Some(1),
        key_kind,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );
    assert!(gen_key_result.is_ok());
    let key = gen_key_result.unwrap();
    let mut ecc_op: Option<EccGenPubKeyCmd<MockEnv>> = None;

    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key.id()),
        Some(EntryKind::Aes128),
        OpenKeyPhase::default(),
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.id == key.id());
    assert!(open_key_data.kind == key_kind.into());
    assert!(open_key_data.pub_key.is_none());
}

#[test]
fn test_open_key_aes192() {
    let key_kind = AesKeyKind::Aes192;

    let mut hal = set_up_hal_for_key_tests();

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 24usize]);
    });
    hal.expect_rng().once().return_const(rng);
    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let mut app_session = UserSession::new(rev(), 10, state);

    let gen_key_result = app_session.aes_gen_key(
        Some(1),
        key_kind,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );
    assert!(gen_key_result.is_ok());
    let key = gen_key_result.unwrap();
    let mut ecc_op: Option<EccGenPubKeyCmd<MockEnv>> = None;

    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key.id()),
        Some(EntryKind::Aes192),
        OpenKeyPhase::default(),
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.id == key.id());
    assert!(open_key_data.kind == key_kind.into());
    assert!(open_key_data.pub_key.is_none());
}

#[test]
fn test_open_key_aes256() {
    let key_kind = AesKeyKind::Aes256;

    let mut hal = set_up_hal_for_key_tests();

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });
    hal.expect_rng().once().return_const(rng);
    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let mut app_session = UserSession::new(rev(), 10, state);

    // Add AES key to vault
    let gen_key_result = app_session.aes_gen_key(
        Some(1),
        key_kind,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );
    assert!(gen_key_result.is_ok());
    let key = gen_key_result.unwrap();
    let mut ecc_op: Option<EccGenPubKeyCmd<MockEnv>> = None;

    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key.id()),
        Some(EntryKind::Aes256),
        OpenKeyPhase::default(),
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.id == key.id());
    assert!(open_key_data.kind == key_kind.into());
    assert!(open_key_data.pub_key.is_none());
}

#[test]
fn test_open_key_ecc256priv() {
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
                // simulate PKA writing the public key into the provided IoMemRange
                unsafe {
                    let buf =
                        core::slice::from_raw_parts_mut(pubkey.addr() as *mut u8, pubkey.len());
                    for b in buf.iter_mut() {
                        *b = 2u8;
                    }
                }
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
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

    let mut app_session = UserSession::new(rev(), 10, state);

    // Add ECC key to vault
    let result = app_session.begin_ecc_gen_key(
        tag,
        Some(1),
        EccCurve::P256,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert!(key_pair.ecc_key.id() == 0);
    assert!(key_pair.pub_key.curve == PkaEccCurve::Ecc256);

    let mut ecc_op: Option<EccGenPubKeyCmd<MockEnv>> = None;
    // Init
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        None,
        None,
        OpenKeyPhase::default(),
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.phase == OpenKeyPhase::PendingMontgomeryConstCalc);
    assert!(open_key_data.id == key_pair.ecc_key.id());
    assert!(open_key_data.kind == EntryKind::Ecc256Private);
    assert!(open_key_data.pub_key.is_none());

    // Prepare a single backing buffer for the zero-copy continuation steps
    let mut pub_key_buf = [0u8; PkaEccCurve::MAX_LEN * 2];
    let pub_key_range = IoMemRange::from(pub_key_buf.as_mut_slice());
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc256Private),
        open_key_data.phase,
        false,
        &mut ecc_op,
        &pub_key_range,
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.phase == OpenKeyPhase::PendingPointMultiplication);
    assert!(open_key_data.id == key_pair.ecc_key.id());
    assert!(open_key_data.kind == EntryKind::Ecc256Private);
    assert!(open_key_data.pub_key.is_none());

    // Complete point multiplication and return public key (reuse same buffer)
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc256Private),
        open_key_data.phase,
        false,
        &mut ecc_op,
        &pub_key_range,
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.phase == OpenKeyPhase::Done);
    assert!(open_key_data.id == key_pair.ecc_key.id());
    assert!(open_key_data.kind == EntryKind::Ecc256Private);
    // Zero-copy API: pub key bytes are written into the provided IoMemRange (pub_key_buf)
    assert!(pub_key_buf.iter().all(|&b| b == 2u8));
}

#[test]
fn test_open_key_ecc384priv() {
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
                    curve: PkaEccCurve::Ecc384,
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

    let mut app_session = UserSession::new(rev(), 10, state);

    // Add ECC key to vault
    let result = app_session.begin_ecc_gen_key(
        tag,
        Some(1),
        EccCurve::P384,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert!(key_pair.ecc_key.id() == 0);
    assert!(key_pair.pub_key.curve == PkaEccCurve::Ecc384);

    let mut ecc_op: Option<EccGenPubKeyCmd<MockEnv>> = None;

    // Init
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc384Private),
        OpenKeyPhase::default(),
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.phase == OpenKeyPhase::PendingMontgomeryConstCalc);
    assert!(open_key_data.id == key_pair.ecc_key.id());
    assert!(open_key_data.kind == EntryKind::Ecc384Private);
    assert!(open_key_data.pub_key.is_none());

    // Prepare a single backing buffer for the zero-copy continuation steps
    let mut pub_key_buf = [0u8; PkaEccCurve::MAX_LEN * 2];
    let pub_key_range = IoMemRange::from(pub_key_buf.as_mut_slice());
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc384Private),
        open_key_data.phase,
        false,
        &mut ecc_op,
        &pub_key_range,
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.phase == OpenKeyPhase::PendingPointMultiplication);
    assert!(open_key_data.id == key_pair.ecc_key.id());
    assert!(open_key_data.kind == EntryKind::Ecc384Private);
    assert!(open_key_data.pub_key.is_none());

    // Complete point multiplication and return public key (reuse same buffer)
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc384Private),
        open_key_data.phase,
        false,
        &mut ecc_op,
        &pub_key_range,
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.phase == OpenKeyPhase::Done);
    assert!(open_key_data.id == key_pair.ecc_key.id());
    assert!(open_key_data.kind == EntryKind::Ecc384Private);
    // Zero-copy API: pub key bytes are written into the provided IoMemRange (pub_key_buf)
    assert!(pub_key_buf.iter().all(|&b| b == 2u8));
}

#[test]
fn test_open_key_ecc521priv() {
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
                    curve: PkaEccCurve::Ecc521,
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

    let mut app_session = UserSession::new(rev(), 10, state);

    // Add ECC key to vault
    let result = app_session.begin_ecc_gen_key(
        tag,
        Some(1),
        EccCurve::P521,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert!(key_pair.ecc_key.id() == 0);
    assert!(key_pair.pub_key.curve == PkaEccCurve::Ecc521);

    let mut ecc_op: Option<EccGenPubKeyCmd<MockEnv>> = None;
    // Init
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc521Private),
        OpenKeyPhase::default(),
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.phase == OpenKeyPhase::PendingMontgomeryConstCalc);
    assert!(open_key_data.id == key_pair.ecc_key.id());
    assert!(open_key_data.kind == EntryKind::Ecc521Private);
    assert!(open_key_data.pub_key.is_none());

    // Prepare a single backing buffer for the zero-copy continuation steps
    let mut pub_key_buf = [0u8; PkaEccCurve::MAX_LEN * 2];
    let pub_key_range = IoMemRange::from(pub_key_buf.as_mut_slice());
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc521Private),
        open_key_data.phase,
        false,
        &mut ecc_op,
        &pub_key_range,
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.phase == OpenKeyPhase::PendingPointMultiplication);
    assert!(open_key_data.id == key_pair.ecc_key.id());
    assert!(open_key_data.kind == EntryKind::Ecc521Private);
    assert!(open_key_data.pub_key.is_none());

    // Complete point multiplication and return public key (reuse same buffer)
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc521Private),
        open_key_data.phase,
        false,
        &mut ecc_op,
        &pub_key_range,
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.phase == OpenKeyPhase::Done);
    assert!(open_key_data.id == key_pair.ecc_key.id());
    assert!(open_key_data.kind == EntryKind::Ecc521Private);
    // Zero-copy API: pub key bytes are written into the provided IoMemRange (pub_key_buf)
    assert!(pub_key_buf.iter().all(|&b| b == 2u8));
}

#[test]
fn test_open_key_secret256() {
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
            move |_tag, _curve, _privkey, _key_type| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                })
            },
        );
        pka.expect_end_ecdh_compute()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaEccSecretValue {
                    curve: PkaEccCurve::Ecc256,
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

    let mut app_session = UserSession::new(rev(), 10, state);

    // Add ECC key to vault
    let key_tag = 3;
    let result = app_session.begin_ecc_gen_key(
        tag,
        None,
        EccCurve::P256,
        EccKeyUsage::KeyAgreement,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert!(key_pair.ecc_key.id() == 0);
    assert!(key_pair.pub_key.curve == PkaEccCurve::Ecc256);

    let raw_pub_key = IoMemRange::from(TEST_RAW_ECC_256_PUBLIC_KEY.as_slice());

    // Add secret to vault with ECDH operation
    let op = app_session
        .begin_ecdh_compute_with_pub_key_validation(tag, 0, DdiKeyType::Secret256, &raw_pub_key)
        .unwrap();

    let op = app_session
        .continue_ecdh_compute_zc(op, &raw_pub_key)
        .unwrap();

    let op = app_session
        .continue_ecdh_compute_zc(op, &raw_pub_key)
        .unwrap();

    let end_ecdh_compute_result =
        app_session.end_ecdh_compute(op, DdiKeyUsage::Derive, Some(key_tag), KeyAvailability::App);
    assert!(end_ecdh_compute_result.is_ok());

    let mut ecc_op: Option<EccGenPubKeyCmd<MockEnv>> = None;
    // Open ECDH secret from vault
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        key_tag,
        None,
        None,
        OpenKeyPhase::default(),
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.kind == EntryKind::Secret256);
    assert!(open_key_data.pub_key.is_none());
}

#[test]
fn test_get_key_kind() {
    let key_kind = AesKeyKind::Aes256;

    let mut hal = set_up_hal_for_key_tests();

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });
    hal.expect_rng().once().return_const(rng);
    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    // Add AES key to vault
    let gen_key_result = app_session.aes_gen_key(
        Some(1),
        key_kind,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );
    assert!(gen_key_result.is_ok());
    let key = gen_key_result.unwrap();

    let get_key_kind_result = app_session.get_key_kind(key.id());
    assert!(get_key_kind_result.is_ok());
    assert!(matches!(get_key_kind_result, Ok(EntryKind::Aes256)));
}

#[test]
fn test_get_key_kind_err() {
    let mut hal = set_up_hal_for_key_tests();

    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let get_key_kind_result = app_session.get_key_kind(100);
    assert!(get_key_kind_result.is_err());
}

#[test]
fn test_open_key_unwrapping_key() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    let part_persistent_store_memory = [0u8; 2048 * 65];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_ipc_expectations(&mut hal);
    let persistent_store: &'static mut [HsmPartPersistentStore] = mcr_mem_map::mem_addr_to_slice(
        part_persistent_store_memory.as_ptr() as usize,
        MAX_PCIE_FUNCTIONS,
    );
    persistent_store[0].unwrapping_key_bk_valid = UnwrappingKeyValidity::PendingPct as u8;

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let mut app_session = UserSession::new(rev(), 10, state);

    let result = app_session.get_unwrapping_key(0, None, PcieFunction::Pf);
    assert!(result.is_ok());
    let unwrapping_key_ctx = result.unwrap();

    assert!(unwrapping_key_ctx.output.is_some());

    let output = unwrapping_key_ctx.output.as_ref().unwrap();
    assert_eq!(output.id, 0);

    let mut ecc_op = None;
    let result = app_session.open_key_zc(
        TagId::default(),
        0,
        Some(output.id),
        None,
        OpenKeyPhase::default(),
        true,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(result.is_ok());
}

#[test]
fn test_open_key_unwrapping_key_invalid_permission() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    let part_persistent_store_memory = [0u8; 2048 * 65];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_ipc_expectations(&mut hal);
    let persistent_store: &'static mut [HsmPartPersistentStore] = mcr_mem_map::mem_addr_to_slice(
        part_persistent_store_memory.as_ptr() as usize,
        MAX_PCIE_FUNCTIONS,
    );
    persistent_store[0].unwrapping_key_bk_valid = UnwrappingKeyValidity::PendingPct as u8;

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let mut app_session = UserSession::new(rev(), 10, state);

    let result = app_session.get_unwrapping_key(0, None, PcieFunction::Pf);
    assert!(result.is_ok());
    let unwrapping_key_ctx = result.unwrap();

    assert!(unwrapping_key_ctx.output.is_some());

    let output = unwrapping_key_ctx.output.as_ref().unwrap();
    assert_eq!(output.id, 0);

    let mut ecc_op = None;
    let result = app_session.open_key_zc(
        TagId::default(),
        0,
        Some(output.id),
        None,
        OpenKeyPhase::default(),
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(result.is_err());
    assert!(matches!(result, Err(HsmErr::InvalidPermissions)));
}

#[test]
fn test_open_key_unwrapping_key_invalid_id() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    let part_persistent_store_memory = [0u8; 2048 * 65];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_ipc_expectations(&mut hal);
    let persistent_store: &'static mut [HsmPartPersistentStore] = mcr_mem_map::mem_addr_to_slice(
        part_persistent_store_memory.as_ptr() as usize,
        MAX_PCIE_FUNCTIONS,
    );
    persistent_store[0].unwrapping_key_bk_valid = UnwrappingKeyValidity::PendingPct as u8;

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let mut app_session = UserSession::new(rev(), 10, state);

    let result = app_session.get_unwrapping_key(0, None, PcieFunction::Pf);
    assert!(result.is_ok());
    let unwrapping_key_ctx = result.unwrap();

    assert!(unwrapping_key_ctx.output.is_some());

    let output = unwrapping_key_ctx.output.as_ref().unwrap();
    assert_eq!(output.id, 0);

    let mut ecc_op = None;
    let result = app_session.open_key_zc(
        TagId::default(),
        0,
        Some(output.id + 1),
        None,
        OpenKeyPhase::default(),
        true,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(result.is_err());
    assert!(matches!(result, Err(HsmErr::InvalidKeyIndex)));
}

#[test]
fn test_open_key_pka_engine_busy_persistent() {
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

    let mut app_session = UserSession::new(rev(), 10, state);

    // Add ECC key to vault
    let result = app_session.begin_ecc_gen_key(
        tag,
        Some(1),
        EccCurve::P256,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert!(key_pair.ecc_key.id() == 0);
    assert!(key_pair.pub_key.curve == PkaEccCurve::Ecc256);

    let mut ecc_op: Option<EccGenPubKeyCmd<MockEnv>> = None;
    // Op #1: Init
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        0,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc256Private),
        OpenKeyPhase::default(),
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_ok());

    let open_key_data_op1 = open_key_result.unwrap();
    assert!(open_key_data_op1.phase == OpenKeyPhase::PendingMontgomeryConstCalc);

    // Op #2: Try Init again to ensure PKA engine is unavailable
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc256Private),
        OpenKeyPhase::default(),
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_ok());

    let open_key_data_op2 = open_key_result.unwrap();
    assert!(open_key_data_op2.phase == OpenKeyPhase::PendingUpkaEngine);

    // Op #2: Try to begin operation again from PendingEngine phase to encounter Pending error
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc256Private),
        open_key_data_op2.phase,
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_err());
    assert!(matches!(open_key_result, Err(HsmErr::Pending)));
}

#[test]
fn test_open_key_ecc_begin_err() {
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

        pka.expect_begin_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag, _curve| Err(HsmErr::EccMontgomeryConstCalcFailed.into()));

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

    let mut app_session = UserSession::new(rev(), 10, state);

    // Add ECC key to vault
    let result = app_session.begin_ecc_gen_key(
        tag,
        Some(1),
        EccCurve::P256,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert!(key_pair.ecc_key.id() == 0);
    assert!(key_pair.pub_key.curve == PkaEccCurve::Ecc256);

    let mut ecc_op: Option<EccGenPubKeyCmd<MockEnv>> = None;
    // Verify error in Init phase
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc256Private),
        OpenKeyPhase::default(),
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_err());
    assert!(matches!(
        open_key_result,
        Err(HsmErr::EccMontgomeryConstCalcFailed)
    ));
}

#[test]
fn test_open_key_ecc_continue_err() {
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

        pka.expect_begin_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag, _curve| Ok(()));
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Err(HsmErr::EccMontgomeryConstCalcFailed.into()));

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

    let mut app_session = UserSession::new(rev(), 10, state);

    // Add ECC key to vault
    let result = app_session.begin_ecc_gen_key(
        tag,
        Some(1),
        EccCurve::P256,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert!(key_pair.ecc_key.id() == 0);
    assert!(key_pair.pub_key.curve == PkaEccCurve::Ecc256);

    let mut ecc_op: Option<EccGenPubKeyCmd<MockEnv>> = None;
    // Init
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc256Private),
        OpenKeyPhase::default(),
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.phase == OpenKeyPhase::PendingMontgomeryConstCalc);
    assert!(open_key_data.id == key_pair.ecc_key.id());
    assert!(open_key_data.kind == EntryKind::Ecc256Private);
    assert!(open_key_data.pub_key.is_none());

    // Verify error on Montgomery Constant calculation
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc256Private),
        open_key_data.phase,
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_err());
    assert!(matches!(
        open_key_result,
        Err(HsmErr::EccMontgomeryConstCalcFailed)
    ));
}

#[test]
fn test_open_key_ecc_end_err() {
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
            move |_tag, _curve, _privkey, _pubkey| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                })
            },
        );
        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));
        pka.expect_end_ecc_gen_pub_key_zc()
            .times(1)
            .returning(move |_tag, _op| Err(HsmErr::EccGenPubKeyFailed.into()));

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

    let mut app_session = UserSession::new(rev(), 10, state);

    // Add ECC key to vault
    let result = app_session.begin_ecc_gen_key(
        tag,
        Some(1),
        EccCurve::P256,
        EccKeyUsage::SignVerify,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let cmd_info = result.unwrap();

    let end_ecc_gen_key_result = app_session.end_ecc_gen_key(tag, cmd_info);
    assert!(end_ecc_gen_key_result.is_ok());
    let key_pair = end_ecc_gen_key_result.unwrap();
    assert!(key_pair.ecc_key.id() == 0);
    assert!(key_pair.pub_key.curve == PkaEccCurve::Ecc256);

    let mut ecc_op: Option<EccGenPubKeyCmd<MockEnv>> = None;
    // Init
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc256Private),
        OpenKeyPhase::default(),
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.phase == OpenKeyPhase::PendingMontgomeryConstCalc);
    assert!(open_key_data.id == key_pair.ecc_key.id());
    assert!(open_key_data.kind == EntryKind::Ecc256Private);
    assert!(open_key_data.pub_key.is_none());

    // Complete Montgomery Constant calculation
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc256Private),
        open_key_data.phase,
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_ok());

    let open_key_data = open_key_result.unwrap();
    assert!(open_key_data.phase == OpenKeyPhase::PendingPointMultiplication);
    assert!(open_key_data.id == key_pair.ecc_key.id());
    assert!(open_key_data.kind == EntryKind::Ecc256Private);
    assert!(open_key_data.pub_key.is_none());

    // Complete point multiplication and verify error
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        1,
        Some(key_pair.ecc_key.id()),
        Some(EntryKind::Ecc256Private),
        open_key_data.phase,
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_err());
    assert!(matches!(open_key_result, Err(HsmErr::EccGenPubKeyFailed)));
}

#[test]
fn test_rsa_non_crt_key() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

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

    let mut app_session = UserSession::new(rev(), 10, state);

    let result = app_session.import_der_key(
        EntryClass::Rsa,
        DdiKeyUsage::SignVerify,
        Some(0x5453),
        KeyAvailability::App,
        &TEST_RSA_3K_PRIVATE_KEY,
    );

    assert!(result.is_ok());
    let import_der_key_result = result.unwrap();

    assert!(import_der_key_result.pub_key_data.is_some());

    assert_eq!(import_der_key_result.key_type, DdiKeyType::Rsa3kPrivate);

    let mut ecc_op: Option<EccGenPubKeyCmd<MockEnv>> = None;
    // RSA non CRT open key
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        0x5453,
        Some(import_der_key_result.priv_key_id),
        Some(EntryKind::Rsa3kPrivate),
        OpenKeyPhase::Init,
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_ok());
}

#[test]
fn test_rsa_non_crt_key_invalid_key_id() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

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

    let mut app_session = UserSession::new(rev(), 10, state);

    let result = app_session.import_der_key(
        EntryClass::Rsa,
        DdiKeyUsage::SignVerify,
        Some(0x5453),
        KeyAvailability::App,
        &TEST_RSA_3K_PRIVATE_KEY,
    );

    assert!(result.is_ok());
    let import_der_key_result = result.unwrap();

    assert!(import_der_key_result.pub_key_data.is_some());

    assert_eq!(import_der_key_result.key_type, DdiKeyType::Rsa3kPrivate);

    let mut ecc_op: Option<EccGenPubKeyCmd<MockEnv>> = None;
    // RSA non CRT open key
    let open_key_result = app_session.open_key_zc(
        TagId::default(),
        0x5453,
        Some(import_der_key_result.priv_key_id + 1),
        Some(EntryKind::Rsa3kPrivate),
        OpenKeyPhase::Init,
        false,
        &mut ecc_op,
        &IoMemRange::from(&[] as &[u8]),
    );
    assert!(open_key_result.is_err());
}

#[test]
fn test_delete_key() {
    let mut hal = set_up_hal_for_key_tests();

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 16usize]);
    });
    hal.expect_rng().once().return_const(rng);
    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let result = app_session.aes_gen_key(
        None,
        AesKeyKind::Aes128,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(app_session.delete_key(key.id()).is_ok());
}

#[test]
fn test_get_unwrapping_key_returns_pending_when_slot_empty() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    let part_persistent_store_memory = [0u8; 2048 * 65];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
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

    // Slot[0] (PcieFunction(0)) is zeroed → `unwrapping_key_bk_valid` is false.
    let result = app_session.get_unwrapping_key(0, None, PcieFunction::Pf);

    assert_eq!(result.err(), Some(HsmErr::PendingKeyGeneration));
}

#[test]
fn test_get_unwrapping_key() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    let part_persistent_store_memory = [0u8; 2048 * 65];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_ipc_expectations(&mut hal);
    let persistent_store: &'static mut [HsmPartPersistentStore] = mcr_mem_map::mem_addr_to_slice(
        part_persistent_store_memory.as_ptr() as usize,
        MAX_PCIE_FUNCTIONS,
    );
    persistent_store[0].unwrapping_key_bk_valid = UnwrappingKeyValidity::PendingPct as u8;

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let result = app_session.get_unwrapping_key(0, None, PcieFunction::Pf);
    let unwrapping_key_ctx = result.unwrap();

    let GetUnwrappingKeyCtx { output } = unwrapping_key_ctx;
    assert!(output.is_some());
    let output = output.unwrap();

    let result = app_session.get_unwrapping_key(0, Some(output.id), PcieFunction::Pf);

    assert!(result.is_ok());
    let unwrapping_key_ctx = result.unwrap();

    assert!(unwrapping_key_ctx.output.is_some());
    let output2 = unwrapping_key_ctx.output.unwrap();

    assert!(output2.data == output.data);
    assert_eq!(output2.id, output.id);

    let result = app_session.get_unwrapping_key(0, Some(output.id), PcieFunction::Pf);

    assert!(result.is_ok());
    let unwrapping_key_ctx = result.unwrap();

    assert!(unwrapping_key_ctx.output.is_some());
    let output3 = unwrapping_key_ctx.output.unwrap();

    assert!(output3.data == output.data);
    assert!(output3.id == output.id);
}

#[test]
fn test_get_unwrapping_key_existing() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    let part_persistent_store_memory = [0u8; 2048 * 65];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_ipc_expectations(&mut hal);
    let persistent_store: &'static mut [HsmPartPersistentStore] = mcr_mem_map::mem_addr_to_slice(
        part_persistent_store_memory.as_ptr() as usize,
        MAX_PCIE_FUNCTIONS,
    );
    persistent_store[0].unwrapping_key_bk_valid = UnwrappingKeyValidity::PendingPct as u8;

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let result = app_session.get_unwrapping_key(0, None, PcieFunction::Pf);
    let unwrapping_key_ctx = result.unwrap();

    assert!(unwrapping_key_ctx.output.is_some());

    let output = unwrapping_key_ctx.output.as_ref().unwrap();

    // try to delete the unwrapping key
    let result = app_session.delete_key(output.id);
    assert!(result.is_err());
    assert_eq!(result, Err(HsmErr::CannotDeleteInternalKeys));
}

#[test]
fn test_der_key_import_rsa_no_crt() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

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

    let result = app_session.import_der_key(
        EntryClass::Rsa,
        DdiKeyUsage::SignVerify,
        Some(0x5453),
        KeyAvailability::App,
        &TEST_RSA_2K_PRIVATE_KEY,
    );

    assert!(result.is_ok());
    let import_der_key_result = result.unwrap();

    assert!(app_session
        .delete_key(import_der_key_result.priv_key_id)
        .is_ok());
    assert!(import_der_key_result.pub_key_data.is_some());

    assert_eq!(import_der_key_result.key_type, DdiKeyType::Rsa2kPrivate);
}

#[test]
fn test_der_key_import_rsa_crt() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

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

    let result = app_session.import_der_key(
        EntryClass::RsaCrt,
        DdiKeyUsage::EncryptDecrypt,
        None,
        KeyAvailability::Session,
        &TEST_RSA_2K_PRIVATE_KEY,
    );

    assert!(result.is_err());
}

#[test]
fn test_der_key_import_ecc() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

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

    let result = app_session.import_der_key(
        EntryClass::Ecc,
        DdiKeyUsage::SignVerify,
        None,
        KeyAvailability::App,
        &TEST_ECC_256_PRIVATE_KEY,
    );

    assert!(result.is_ok());
    let import_der_key_result = result.unwrap();

    assert!(app_session
        .delete_key(import_der_key_result.priv_key_id)
        .is_ok());
    assert!(import_der_key_result.pub_key_data.is_some());

    assert_eq!(import_der_key_result.key_type, DdiKeyType::Ecc256Private);
}

#[test]
fn test_der_key_import_aes() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

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

    let result = app_session.import_der_key(
        EntryClass::Aes,
        DdiKeyUsage::EncryptDecrypt,
        None,
        KeyAvailability::App,
        &[1; 32],
    );

    assert!(result.is_ok());
    let import_der_key_result = result.unwrap();

    assert!(app_session
        .delete_key(import_der_key_result.priv_key_id)
        .is_ok());
    assert!(import_der_key_result.pub_key_data.is_none());
    assert_eq!(import_der_key_result.key_type, DdiKeyType::Aes256);
}

#[test]
fn test_der_key_import_aes_bulk() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .times(1)
        .return_const(cdma_vault_meta_data.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_fp_ipc_send_recv_expectations(&mut hal, 1, 1);
    set_hsp_ipc_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let result = app_session.begin_import_der_aesbulk256_key(
        0,
        PcieFunction::Vf0,
        DdiKeyUsage::EncryptDecrypt,
        None,
        DdiKeyType::AesGcmBulk256Unapproved,
        KeyAvailability::App,
        &[1; 32],
    );

    assert!(result.is_ok());
    let result = result.unwrap();

    let result = app_session.end_import_der_aesbulk256_key(&result);
    assert!(result.is_ok());

    let result = app_session.state.vault().enable_key(0);
    assert!(result.is_ok());

    let result = app_session.state.vault().disable_key(0);
    assert!(result.is_ok());
}

#[test]
fn test_der_key_import_aes_bulk_failed() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .times(2)
        .return_const(cdma_vault_meta_data.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_hsp_ipc_expectations(&mut hal);
    set_fp_ipc_send_recv_failed_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let result = app_session.begin_import_der_aesbulk256_key(
        0,
        PcieFunction::Vf0,
        DdiKeyUsage::EncryptDecrypt,
        None,
        DdiKeyType::AesXtsBulk256,
        KeyAvailability::App,
        &[1; 32],
    );

    assert!(result.is_ok());
    let result = result.unwrap();

    let result = app_session.end_import_der_aesbulk256_key(&result);
    assert!(result.is_err());
}

#[test]
fn test_delete_key_aes_bulk() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [1; 65];
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .times(1)
        .return_const(cdma_vault_meta_data.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_hsp_ipc_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);
    set_fp_ipc_send_recv_expectations(&mut hal, 1, 1);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let key_blob = [0u8; 2];

    let aes_key = AesKeyImported::new(
        AesKeyKind::AesGcmBulk256Unapproved,
        AesKeyUsage::EncryptDecrypt,
        &key_blob,
    )
    .unwrap();

    state
        .vault()
        .aes_import_key(
            0,
            10,
            None,
            &aes_key,
            &aes_entry_attributes(KeyAvailability::App, false, AesKeyUsage::EncryptDecrypt),
        )
        .unwrap();

    let app_session = UserSession::new(rev(), 10, state);

    let result = app_session.begin_delete_aesbulk256_key(0, PcieFunction::Vf0, 0);

    assert!(result.is_ok());
    let result = result.unwrap();

    let result = app_session.end_delete_aesbulk256_key(&result);
    assert!(result.is_ok());
}

#[test]
fn test_delete_key_aes_bulk_failed() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .times(0)
        .return_const(cdma_vault_meta_data.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_hsp_ipc_expectations(&mut hal);
    set_fp_ipc_send_recv_failed_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let key_blob = [0u8; 2];

    let aes_key = AesKeyImported::new(
        AesKeyKind::AesGcmBulk256Unapproved,
        AesKeyUsage::EncryptDecrypt,
        &key_blob,
    )
    .unwrap();

    state
        .vault()
        .aes_import_key(
            0,
            10,
            None,
            &aes_key,
            &aes_entry_attributes(KeyAvailability::App, false, AesKeyUsage::EncryptDecrypt),
        )
        .unwrap();

    let app_session = UserSession::new(rev(), 10, state);

    let result = app_session.begin_delete_aesbulk256_key(0, PcieFunction::Vf0, 0);

    assert!(result.is_ok());
    let result = result.unwrap();

    let result = app_session.end_delete_aesbulk256_key(&result);
    assert!(result.is_err());
}

#[test]
fn test_gen_key_aes_bulk() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });

    let mut hal = MockHal::new();

    hal.expect_rng().once().return_const(rng);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .times(1)
        .return_const(cdma_vault_meta_data.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_hsp_ipc_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);
    set_fp_ipc_send_recv_expectations(&mut hal, 1, 1);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let result = app_session.begin_aesbulk256_gen_key(
        0,
        PcieFunction::Vf0,
        None,
        DdiKeyType::AesXtsBulk256,
        KeyAvailability::App,
    );

    assert!(result.is_ok());
    let result = result.unwrap();

    let result = app_session.end_aesbulk256_gen_key(&result);
    assert!(result.is_ok());

    let result = app_session.state.vault().enable_key(0);
    assert!(result.is_ok());

    let result = app_session.state.vault().disable_key(0);
    assert!(result.is_ok());
}

#[test]
fn test_gen_key_aes_bulk_failed() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });

    let mut hal = MockHal::new();

    hal.expect_rng().once().return_const(rng);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .times(2)
        .return_const(cdma_vault_meta_data.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_hsp_ipc_expectations(&mut hal);
    set_fp_ipc_send_recv_failed_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let result = app_session.begin_aesbulk256_gen_key(
        0,
        PcieFunction::Vf0,
        None,
        DdiKeyType::AesXtsBulk256,
        KeyAvailability::App,
    );

    assert!(result.is_ok());
    let result = result.unwrap();

    let result = app_session.end_aesbulk256_gen_key(&result);
    assert!(result.is_err());
}

#[test]
fn test_gen_key_aes_bulk_rollback() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });

    let mut hal = MockHal::new();

    hal.expect_rng().once().return_const(rng);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .times(2)
        .return_const(cdma_vault_meta_data.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_hsp_ipc_expectations(&mut hal);
    set_fp_ipc_send_recv_expectations(&mut hal, 2, 2);
    set_hsm_to_admin_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let result = app_session.begin_aesbulk256_gen_key(
        0,
        PcieFunction::Vf0,
        None,
        DdiKeyType::AesXtsBulk256,
        KeyAvailability::App,
    );

    assert!(result.is_ok());
    let aes_bulk_256_cmd = result.unwrap();

    let result = app_session.end_aesbulk256_gen_key(&aes_bulk_256_cmd);
    assert!(result.is_ok());

    let result = app_session.state.vault().enable_key(0);
    assert!(result.is_ok());

    let result = app_session.begin_rollback_aesbulk256_key(0, PcieFunction::Vf0, &aes_bulk_256_cmd);

    assert!(result.is_ok());

    let result = app_session.end_rollback_aesbulk256_key(&aes_bulk_256_cmd);
    assert!(result.is_ok());
}

#[test]
fn test_der_key_import_rsa_no_crt_err() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

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

    let result = app_session.import_der_key(
        EntryClass::Rsa,
        DdiKeyUsage::Derive,
        Some(0x5453),
        KeyAvailability::App,
        &TEST_RSA_2K_PRIVATE_KEY,
    );

    assert!(result.is_err());

    let result = app_session.import_der_key(
        EntryClass::Rsa,
        DdiKeyUsage::SignVerify,
        Some(0x5453),
        KeyAvailability::App,
        &TEST_ECC_256_PRIVATE_KEY,
    );

    assert!(result.is_err());

    let result = app_session.import_der_key(
        EntryClass::Rsa,
        DdiKeyUsage::SignVerify,
        Some(0x5453),
        KeyAvailability::Session,
        &TEST_RSA_2K_PRIVATE_KEY,
    );

    assert!(result.is_err());
}

#[test]
fn test_der_key_import_ecc_err() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

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

    let result = app_session.import_der_key(
        EntryClass::Ecc,
        DdiKeyUsage::Unwrap,
        None,
        KeyAvailability::App,
        &TEST_ECC_256_PRIVATE_KEY,
    );

    assert!(result.is_err());

    let result = app_session.import_der_key(
        EntryClass::Ecc,
        DdiKeyUsage::SignVerify,
        Some(0x5555),
        KeyAvailability::Session,
        &TEST_ECC_256_PRIVATE_KEY,
    );

    assert!(result.is_err());

    let result = app_session.import_der_key(
        EntryClass::Ecc,
        DdiKeyUsage::SignVerify,
        None,
        KeyAvailability::App,
        &TEST_RSA_2K_PRIVATE_KEY,
    );

    assert!(result.is_err());
}

#[test]
fn test_der_key_import_aes_err() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

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

    let result = app_session.import_der_key(
        EntryClass::Aes,
        DdiKeyUsage::SignVerify,
        None,
        KeyAvailability::App,
        &[1; 32],
    );

    assert!(result.is_err());

    let result = app_session.import_der_key(
        EntryClass::Aes,
        DdiKeyUsage::EncryptDecrypt,
        None,
        KeyAvailability::App,
        &[1; 17],
    );

    assert!(result.is_err());

    let result = app_session.import_der_key(
        EntryClass::Aes,
        DdiKeyUsage::EncryptDecrypt,
        Some(0x5555),
        KeyAvailability::Session,
        &[1; 32],
    );

    assert!(result.is_err());
}

#[test]
fn test_der_key_import_err() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

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

    let result = app_session.import_der_key(
        EntryClass::Free,
        DdiKeyUsage::SignVerify,
        None,
        KeyAvailability::App,
        &[1; 32],
    );

    assert!(result.is_err());
}

fn set_up_hal_for_key_tests() -> MockHal {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

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

    hal
}

#[test]
fn test_import_der_crt_key_2k() {
    test_import_der_crt_key(
        PkaRsaSize::Rsa2k,
        EntryKind::Rsa2kPrivateCrt,
        &TEST_RSA_2K_PRIVATE_KEY,
        &TEST_RSA_2K_PUBLIC_KEY,
    );
}

#[test]
fn test_import_der_crt_key_3k() {
    test_import_der_crt_key(
        PkaRsaSize::Rsa3k,
        EntryKind::Rsa3kPrivateCrt,
        &TEST_RSA_3K_PRIVATE_KEY,
        &TEST_RSA_3K_PUBLIC_KEY,
    );
}

#[test]
fn test_import_der_crt_key_4k() {
    test_import_der_crt_key(
        PkaRsaSize::Rsa4k,
        EntryKind::Rsa4kPrivateCrt,
        &TEST_RSA_4K_PRIVATE_KEY,
        &TEST_RSA_4K_PUBLIC_KEY,
    );
}

fn test_import_der_crt_key(
    rsa_type: PkaRsaSize,
    entry_kind: EntryKind,
    priv_key_der: &[u8],
    _pub_key_der: &[u8],
) {
    let data_length = rsa_type.into();
    let mut output_data = [0; 512];
    for (i, item) in output_data.iter_mut().enumerate().take(data_length) {
        *item = i as u8;
    }

    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(move || {
        let mut pka = MockPka::new();
        pka.expect_peek_tag()
            .times(14)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));
        pka.expect_end_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Ok(()));

        pka.expect_begin_rsa_montgomery_in()
            .times(1)
            .returning(move |_data, _rsa_type, _data_be| Ok(PkaRsaCmd { rsa_type }));
        pka.expect_end_rsa_montgomery_in()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
                })
            });

        pka.expect_begin_rsa_montgomery_in()
            .times(1)
            .returning(move |_data, _rsa_type, _data_be| Ok(PkaRsaCmd { rsa_type }));
        pka.expect_end_rsa_montgomery_in()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
                })
            });

        pka.expect_begin_rsa_modular_multiplication()
            .times(1)
            .returning(move |_tag, _rsa_type, _val1, _val2| Ok(PkaRsaCmd { rsa_type }));
        pka.expect_end_rsa_modular_multiplication()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
                })
            });

        pka.expect_begin_rsa_montgomery_out()
            .times(1)
            .returning(move |_data, _rsa_type, _data_be| Ok(PkaRsaCmd { rsa_type }));
        pka.expect_end_rsa_montgomery_out()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaRsaData {
                    size: rsa_type,
                    data_be: vec![0; PkaRsaSize::MAX_LEN].into(),
                })
            });

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));
        pka.expect_end_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Ok(()));

        pka.expect_begin_rsa_montgomery_in()
            .times(1)
            .returning(move |_data, _rsa_type, _data_be| Ok(PkaRsaCmd { rsa_type }));
        pka.expect_end_rsa_montgomery_in()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
                })
            });

        pka.expect_begin_rsa_modular_inverse()
            .times(1)
            .returning(move |_data, _rsa_type, _data_be| Ok(PkaRsaCmd { rsa_type }));
        pka.expect_end_rsa_modular_inverse()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
                })
            });

        pka.expect_begin_rsa_montgomery_out()
            .times(1)
            .returning(move |_data, _rsa_type, _data_be| Ok(PkaRsaCmd { rsa_type }));
        pka.expect_end_rsa_montgomery_out()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaRsaData {
                    size: rsa_type,
                    data_be: vec![0; PkaRsaSize::MAX_LEN].into(),
                })
            });

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));
        pka.expect_end_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Ok(()));

        pka.expect_begin_rsa_montgomery_in()
            .times(1)
            .returning(move |_data, _rsa_type, _data_be| Ok(PkaRsaCmd { rsa_type }));
        pka.expect_end_rsa_montgomery_in()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
                })
            });

        pka.expect_begin_rsa_montgomery_in()
            .times(1)
            .returning(move |_data, _rsa_type, _data_be| Ok(PkaRsaCmd { rsa_type }));
        pka.expect_end_rsa_montgomery_in()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
                })
            });

        pka.expect_begin_rsa_modular_multiplication()
            .times(1)
            .returning(move |_tag, _rsa_type, _val1, _val2| Ok(PkaRsaCmd { rsa_type }));
        pka.expect_end_rsa_modular_multiplication()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
                })
            });

        pka.expect_begin_rsa_montgomery_out()
            .times(1)
            .returning(move |_data, _rsa_type, _data_be| Ok(PkaRsaCmd { rsa_type }));
        pka.expect_end_rsa_montgomery_out()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaRsaData {
                    size: rsa_type,
                    data_be: vec![0; PkaRsaSize::MAX_LEN].into(),
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

    let (mut op, _pub_key_out) = app_session
        .begin_import_der_crt_key(TagId::default(), priv_key_der)
        .unwrap();
    //assert_eq!(der_pub_key_out, pub_key_der);

    let mut count = 0;
    loop {
        match count {
            0 => {
                assert!(op.state == RsaCrtParamCalcState::N1qWaitForMontgomeryFullMod);
            }
            1 => {
                assert!(op.state == RsaCrtParamCalcState::N1qWaitForQinvModPToMontIn);
            }
            2 => {
                assert!(op.state == RsaCrtParamCalcState::N1qWaitForQToMontIn);
            }
            3 => {
                assert!(op.state == RsaCrtParamCalcState::N1qWaitForModMultiplication);
            }
            4 => {
                assert!(op.state == RsaCrtParamCalcState::N1qWaitForMontOut);
            }
            5 => {
                assert!(op.n1q.is_some());
                assert!(op.state == RsaCrtParamCalcState::N2pWaitForMontgomeryModQ);
            }
            6 => {
                assert!(op.state == RsaCrtParamCalcState::N2pWaitForPToMontIn);
            }
            7 => {
                assert!(op.state == RsaCrtParamCalcState::N2pWaitForModInverseP);
            }
            8 => {
                assert!(op.state == RsaCrtParamCalcState::N2pWaitForPinvModQToMontOut);
            }
            9 => {
                assert!(op.state == RsaCrtParamCalcState::N2pWaitForMontgomeryFullMod);
            }
            10 => {
                assert!(op.state == RsaCrtParamCalcState::N2pWaitForPToMontIn2);
            }
            11 => {
                assert!(op.state == RsaCrtParamCalcState::N2pWaitForPInvModQToMontIn);
            }
            12 => {
                assert!(op.state == RsaCrtParamCalcState::N2pWaitForModMultiplication);
            }
            13 => {
                assert!(op.state == RsaCrtParamCalcState::N2pWaitForMontOut);
            }
            14 => {
                assert!(op.n2p.is_some());
                assert!(op.state == RsaCrtParamCalcState::Idle);
            }
            _ => {
                // Unexpected count
                panic!();
            }
        }
        let continue_compute_crt_result = app_session.continue_import_der_crt_key(op);
        assert!(continue_compute_crt_result.is_ok());
        op = continue_compute_crt_result.unwrap();
        if op.state == RsaCrtParamCalcState::Idle {
            break;
        }
        count += 1;
    }

    let (key_id_result, key_type) = app_session
        .end_import_der_crt_key(op, DdiKeyUsage::EncryptDecrypt, None, KeyAvailability::App)
        .unwrap();
    assert_eq!(key_id_result, 0);
    assert_eq!(key_type, entry_kind.try_into().unwrap());
}

#[test]
fn test_user_session_wrong_id() {
    // Partition should validate manager id, before calling aes or sha
    let mut pka = MockPka::new();
    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    let mut sha = MockSha::new();
    let mut aes = MockAes::new();

    aes.expect_encrypt_decrypt().times(3).returning(|_| Ok(()));
    hal.expect_aes().times(3).return_const(aes);

    sha.expect_hmac()
        .returning(move |_, _, _, _, out_buf: &mut IoMemRange| {
            let output_ptr = out_buf.addr();
            let mut_output_ptr = output_ptr as *mut u8;
            let out_slice =
                unsafe { std::slice::from_raw_parts_mut(mut_output_ptr, out_buf.len()) };

            out_slice.fill(TEST_DIGEST_VAL);

            Ok(())
        });
    hal.expect_sha().return_const(sha);

    rng_nonce.expect_bytes().times(2).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().times(2).return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    pka.expect_clone().times(1).returning(move || {
        let mut pka = MockPka::new();

        pka.expect_begin_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag, _curve| Ok(()));
        pka.expect_end_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Ok(()));

        pka.expect_begin_ecc_point_validation_zc()
            .times(1)
            .returning(move |_tag, _curve, _pubkey| Ok(()));
        pka.expect_end_ecc_point_validation_zc()
            .times(1)
            .returning(|_tag| Ok(true));
        pka.expect_begin_ecdh_compute_zc().times(1).returning(
            move |_tag, _curve, _privkey, _pubkey| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc384,
                })
            },
        );
        pka.expect_end_ecdh_compute()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaEccSecretValue {
                    curve: PkaEccCurve::Ecc384,
                    secret: [0; PkaEccCurve::MAX_LEN],
                })
            });

        pka
    });
    hal.expect_pka().times(1..).return_const(vec![pka]);
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate().times(15).returning(|s| {
        let mut alloc = MockDmaAlloc::new(s);
        // Return pin
        alloc.as_ref_mut()[..16].copy_from_slice(&[3; 16usize]);
        Some(alloc)
    });
    hal.expect_dma_heap().return_const(heap);
    hal.expect_clone().times(1).returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal.expect_tcon_tsc().times(1).return_const(0u64);

        hal
    });

    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    set_ipc_expectations(&mut hal);

    let part_env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());

    let part = Partition::<MockEnv>::new(PcieFunction(0), part_env);

    part.set_resource_mask(1);
    let result = part.state.change_user_cred(&[2; 16], &[2; 16]);
    assert!(result.is_ok());

    let result = SessionEncryptionKeyToImport::new(
        SessionEncryptionKeyKind::Ecc384,
        SessionEncryptionKeyUsage::KeyAgreement,
        &[1; 144],
    );
    assert!(result.is_ok());
    let key = result.unwrap();

    let result = part.state.vault().import_session_encryption_key(
        APP_VAULT_ID_FOR_INTERNAL_KEYS,
        &key,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let session_encryption_key = result.unwrap();
    part.state
        .set_session_encryption_key_id(Some(session_encryption_key.id()));

    let raw_pub_key = IoMemRange::from(TEST_RAW_ECC_384_PUBLIC_KEY.as_slice());

    let result = part.begin_open_user_session(TagId::default(), &raw_pub_key);
    assert!(result.is_ok());
    let sess_ctx = result.unwrap();

    let result = part.continue_open_user_session(sess_ctx, &raw_pub_key);
    assert!(result.is_ok());
    let sess_ctx = result.unwrap();

    let result = part.continue_open_user_session(sess_ctx, &raw_pub_key);
    assert!(result.is_ok());
    let sess_ctx = result.unwrap();

    let encrypted_id_buf = [1u8; 16];
    let encrypted_pin_buf = [1u8; 16];
    let encrypted_seed_buf = [1u8; 48];
    let iv_buf = [1u8; 16];

    let encrypted_cred = DdiEncryptedSessionCredential {
        encrypted_id: MborByteArray::new_with_len(encrypted_id_buf.as_ptr(), 16),
        encrypted_pin: MborByteArray::new_with_len(encrypted_pin_buf.as_ptr(), 16),
        encrypted_seed: MborByteArray::new_with_len(encrypted_seed_buf.as_ptr(), 48),
        iv: MborByteArray::new_with_len(iv_buf.as_ptr(), 16),
        nonce: (1..33u8).collect::<Vec<_>>().try_into().unwrap(),
        tag: [TEST_DIGEST_VAL; 48],
    };
    let mut bk_buf = [0u8; 80];
    let mut mk_buf = [0u8; 80];

    let bk3_session = [1u8; 48];
    part.state.set_bk3_session(bk3_session.into());

    let result = part.end_open_user_session(
        sess_ctx,
        rev(),
        &encrypted_cred,
        None,
        &mut bk_buf[..],
        &mut mk_buf[..],
        None,
    );
    assert!(result.is_err());
    if let Err(err) = result {
        assert_eq!(err, HsmErr::InvalidUserCredential);
    }
}

#[test]
fn test_user_cred() {
    let user_sess = UserSession::new(rev(), 0, part_state());

    assert_eq!(user_sess.app_id(), [0; 16]);
    assert_eq!(user_sess.app_vault_id(), 0);
}

#[test]
fn test_api_rev2() {
    let user_sess = UserSession::new(rev(), 0, part_state());

    assert_eq!(user_sess.api_rev(), rev());
}

#[test]
fn test_get_raw_alias_key() {
    // Generate ECC public key/private key using openssl.
    let res = openssl::ec::EcGroup::from_curve_name(Nid::SECP384R1);
    assert!(res.is_ok());
    let group = res.unwrap();

    let res = openssl::ec::EcKey::generate(&group);
    assert!(res.is_ok());
    let ecc_private = res.unwrap();

    let res = openssl::pkey::PKey::from_ec_key(ecc_private);
    assert!(res.is_ok());
    let pkey_private = res.unwrap();

    let res = pkey_private.ec_key().unwrap().private_key_to_der();
    assert!(res.is_ok());
    let priv_key_der_openssl = res.unwrap();

    let mut priv_key_raw_openssl = pkey_private.ec_key().unwrap().private_key().to_vec();
    priv_key_raw_openssl.reverse();

    const TOTAL_TABLE_LEN: usize = 18 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    hal.expect_rng().once().return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_alias_key_len()
        .return_const(priv_key_der_openssl.len() as u32);
    hal.expect_alias_key().return_const(priv_key_der_openssl);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());

    let part = Partition::<MockEnv>::new(PcieFunction(0), env);
    let res = part.get_raw_alias_key();
    assert!(res.is_ok());
    let alias_key = res.unwrap();

    assert!(alias_key == priv_key_raw_openssl.into());
}

#[test]
fn test_notify_pct_validation_failure() {
    let err = u32::from(HsmErr::PctValidationEccGenKeyFailed);

    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

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

    hal.expect_notify_pct_validation_failure()
        .once()
        .returning(move |_| {
            let mut hal = MockHal::new();
            let mut pka = MockPka::new();
            pka.expect_clone().once().returning(MockPka::new);
            hal.expect_pka().once().return_const(vec![pka]);
            hal.expect_notify_pct_validation_failure()
                .once()
                .returning(move |_| {
                    println!("Simulating PCT validation failure {:?}", err);
                });
            set_ipc_expectations(&mut hal);

            let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());

            env.notify_pct_validation_failure(err);
        });

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    app_session.notify_pct_validation_failure(err);
}
