// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_mbor::MborByteArray;
use mcr_types::*;

use crate::error::HsmErr;
use crate::mock::*;
use crate::partition::tests::cmd_scheduler;
use crate::partition::tests::rev;
use crate::partition::tests::set_ipc_expectations;
use crate::partition::AesEncDecIn;
use crate::partition::AesEncDecMode;
use crate::partition::AesEncDecOp;
use crate::partition::AesKeyIn;
use crate::partition::AesKeyKind;
use crate::partition::AesKeyUsage;
use crate::partition::HsmUserSession;
use crate::partition::KeyAvailability;
use crate::partition::PartEnv;
use crate::partition::PartState;
use crate::partition::UserSession;

#[test]
fn test_aes_gen_key_128() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 16usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        None,
        AesKeyKind::Aes128,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());
}

#[test]
fn test_aes_gen_key_128_with_name() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 16usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        Some(100),
        AesKeyKind::Aes128,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());
}

#[test]
fn test_aes_gen_key_192() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 24usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        None,
        AesKeyKind::Aes192,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());
}

#[test]
fn test_aes_gen_key_192_with_name() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 24usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        Some(100),
        AesKeyKind::Aes192,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());
}

#[test]
fn test_aes_gen_key_256() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        None,
        AesKeyKind::Aes256,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());
}

#[test]
fn test_aes_gen_key_256_with_name() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        Some(100),
        AesKeyKind::Aes256,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );
    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());
}

#[test]
fn test_aes_enc_dec() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut aes = MockAes::new();
    aes.expect_encrypt_decrypt().times(1).returning(|_| Ok(()));

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_aes().once().return_const(aes);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        None,
        AesKeyKind::Aes256,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );

    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());

    let iv_arr = [0u8; 16];
    let iv: MborByteArray<16> = MborByteArray::new_with_len(iv_arr.as_ptr(), iv_arr.len());

    let msg_in_arr = [0u8; 16];
    let msg_in: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_in_arr.as_ptr(), msg_in_arr.len());

    let msg_out_arr = [0u8; 16];
    let msg_out: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_out_arr.as_ptr(), msg_out_arr.len());

    let iv_binding = (&iv).into();
    let msg_in_binding = (&msg_in).into();
    let msg_out_binding = (&msg_out).into();

    let input = AesEncDecIn::new(
        AesEncDecMode::Cbc,
        AesEncDecOp::Encrypt,
        Some(&iv_binding),
        &msg_in_binding,
        &msg_out_binding,
    );

    let result = app_session.aes_enc_dec(1, AesKeyIn::KeyId(key.id()), &input);

    assert!(result.is_ok());
}

#[test]
fn test_aes_enc_dec_with_key_slice() {
    let mut aes = MockAes::new();
    aes.expect_encrypt_decrypt().times(1).returning(|_| Ok(()));

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_aes().once().return_const(aes);
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
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let iv_arr = [0u8; 16];
    let iv: MborByteArray<16> = MborByteArray::new_with_len(iv_arr.as_ptr(), iv_arr.len());

    let msg_in_arr = [0u8; 16];
    let msg_in: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_in_arr.as_ptr(), msg_in_arr.len());

    let msg_out_arr = [0u8; 16];
    let msg_out: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_out_arr.as_ptr(), msg_out_arr.len());

    let iv_binding = (&iv).into();
    let msg_in_binding = (&msg_in).into();
    let msg_out_binding = (&msg_out).into();

    let input = AesEncDecIn::new(
        AesEncDecMode::Cbc,
        AesEncDecOp::Encrypt,
        Some(&iv_binding),
        &msg_in_binding,
        &msg_out_binding,
    );

    let key_blob = [
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
        0x0ef,
    ];
    let result = app_session.aes_enc_dec(1, AesKeyIn::KeyBlob(&key_blob), &input);

    assert!(result.is_ok());
}

#[test]
fn test_aes_enc_dec_with_invalid_key() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        None,
        AesKeyKind::Aes256,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );

    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());

    let iv_arr = [0u8; 16];
    let iv: MborByteArray<16> = MborByteArray::new_with_len(iv_arr.as_ptr(), iv_arr.len());

    let msg_in_arr = [0u8; 16];
    let msg_in: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_in_arr.as_ptr(), msg_in_arr.len());

    let msg_out_arr = [0u8; 16];
    let msg_out: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_out_arr.as_ptr(), msg_out_arr.len());

    let iv_binding = (&iv).into();
    let msg_in_binding = (&msg_in).into();
    let msg_out_binding = (&msg_out).into();

    let input = AesEncDecIn::new(
        AesEncDecMode::Cbc,
        AesEncDecOp::Encrypt,
        Some(&iv_binding),
        &msg_in_binding,
        &msg_out_binding,
    );

    let result = app_session.aes_enc_dec(1, AesKeyIn::KeyId(100), &input);

    assert!(result.is_err());
}

#[test]
fn test_aes_enc_dec_with_invalid_iv_in_cbc() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        None,
        AesKeyKind::Aes256,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );

    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());

    let msg_in_arr = [0u8; 16];
    let msg_in: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_in_arr.as_ptr(), msg_in_arr.len());

    let msg_out_arr = [0u8; 16];
    let msg_out: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_out_arr.as_ptr(), msg_out_arr.len());

    let msg_in_binding = (&msg_in).into();
    let msg_out_binding = (&msg_out).into();

    let input = AesEncDecIn::new(
        AesEncDecMode::Cbc,
        AesEncDecOp::Encrypt,
        None,
        &msg_in_binding,
        &msg_out_binding,
    );

    let result = app_session.aes_enc_dec(1, AesKeyIn::KeyId(key.id()), &input);

    if let Err(HsmErr::InvalidArgument) = result {
    } else {
        panic!("Expected InvalidArgument error");
    }
}

#[test]
fn test_aes_enc_dec_with_invalid_iv_in_ecb() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        None,
        AesKeyKind::Aes256,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );

    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());

    let iv_arr = [0u8; 16];
    let iv: MborByteArray<16> = MborByteArray::new_with_len(iv_arr.as_ptr(), iv_arr.len());

    let msg_in_arr = [0u8; 16];
    let msg_in: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_in_arr.as_ptr(), msg_in_arr.len());

    let msg_out_arr = [0u8; 16];
    let msg_out: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_out_arr.as_ptr(), msg_out_arr.len());

    let iv_binding = (&iv).into();
    let msg_in_binding = (&msg_in).into();
    let msg_out_binding = (&msg_out).into();

    let input = AesEncDecIn::new(
        AesEncDecMode::Ecb,
        AesEncDecOp::Encrypt,
        Some(&iv_binding),
        &msg_in_binding,
        &msg_out_binding,
    );

    let result = app_session.aes_enc_dec(1, AesKeyIn::KeyId(key.id()), &input);

    if let Err(HsmErr::InvalidArgument) = result {
    } else {
        panic!("Expected InvalidArgument error");
    }
}

#[test]
fn test_aes_enc_dec_with_empty_plain_text() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        None,
        AesKeyKind::Aes256,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );

    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());

    let msg_in_arr = [];
    let msg_in: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_in_arr.as_ptr(), msg_in_arr.len());

    let msg_out_arr = [];
    let msg_out: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_out_arr.as_ptr(), msg_out_arr.len());

    let msg_in_binding = (&msg_in).into();
    let msg_out_binding = (&msg_out).into();

    let input = AesEncDecIn::new(
        AesEncDecMode::Ecb,
        AesEncDecOp::Encrypt,
        None,
        &msg_in_binding,
        &msg_out_binding,
    );

    let result = app_session.aes_enc_dec(1, AesKeyIn::KeyId(key.id()), &input);

    if let Err(HsmErr::InvalidArgument) = result {
    } else {
        panic!("Expected InvalidArgument error");
    }
}

#[test]
fn test_aes_enc_dec_with_unaligned_message() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        None,
        AesKeyKind::Aes256,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );

    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());

    let msg_in_arr = [0u8; 15];
    let msg_in: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_in_arr.as_ptr(), msg_in_arr.len());

    let msg_out_arr = [0u8; 15];
    let msg_out: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_out_arr.as_ptr(), msg_out_arr.len());

    let msg_in_binding = (&msg_in).into();
    let msg_out_binding = (&msg_out).into();

    let input = AesEncDecIn::new(
        AesEncDecMode::Ecb,
        AesEncDecOp::Encrypt,
        None,
        &msg_in_binding,
        &msg_out_binding,
    );

    let result = app_session.aes_enc_dec(1, AesKeyIn::KeyId(key.id()), &input);

    if let Err(HsmErr::InvalidArgument) = result {
    } else {
        panic!("Expected InvalidArgument error");
    }
}

#[test]
fn test_aes_enc_dec_hw_enc_dec_failed() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut aes = MockAes::new();
    aes.expect_encrypt_decrypt()
        .times(1)
        .returning(|_| Err(u32::MAX));

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_aes().once().return_const(aes);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        None,
        AesKeyKind::Aes256,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );

    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());

    let iv_arr = [0u8; 16];
    let iv: MborByteArray<16> = MborByteArray::new_with_len(iv_arr.as_ptr(), iv_arr.len());

    let msg_in_arr = [0u8; 16];
    let msg_in: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_in_arr.as_ptr(), msg_in_arr.len());

    let msg_out_arr = [0u8; 16];
    let msg_out: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_out_arr.as_ptr(), msg_out_arr.len());

    let iv_binding = (&iv).into();
    let msg_in_binding = (&msg_in).into();
    let msg_out_binding = (&msg_out).into();

    let input = AesEncDecIn::new(
        AesEncDecMode::Cbc,
        AesEncDecOp::Encrypt,
        Some(&iv_binding),
        &msg_in_binding,
        &msg_out_binding,
    );

    let result = app_session.aes_enc_dec(1, AesKeyIn::KeyId(key.id()), &input);

    if let Err(HsmErr::AesEncryptFailed) = result {
    } else {
        panic!("Expected AesEncryptFailed error");
    }
}

#[test]
fn test_aes_enc_dec_decrypt() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut aes = MockAes::new();
    aes.expect_encrypt_decrypt().times(1).returning(|_| Ok(()));

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_aes().once().return_const(aes);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        None,
        AesKeyKind::Aes256,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );

    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());

    let iv_arr = [0u8; 16];
    let iv: MborByteArray<16> = MborByteArray::new_with_len(iv_arr.as_ptr(), iv_arr.len());

    let msg_in_arr = [0u8; 16];
    let msg_in: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_in_arr.as_ptr(), msg_in_arr.len());

    let msg_out_arr = [0u8; 16];
    let msg_out: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_out_arr.as_ptr(), msg_out_arr.len());

    let iv_binding = (&iv).into();
    let msg_in_binding = (&msg_in).into();
    let msg_out_binding = (&msg_out).into();

    let input = AesEncDecIn::new(
        AesEncDecMode::Cbc,
        AesEncDecOp::Decrypt,
        Some(&iv_binding),
        &msg_in_binding,
        &msg_out_binding,
    );

    let result = app_session.aes_enc_dec(1, AesKeyIn::KeyId(key.id()), &input);

    assert!(result.is_ok());
}

#[test]
fn test_aes_enc_dec_mode_ecb() {
    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

    let mut aes = MockAes::new();
    aes.expect_encrypt_decrypt().times(1).returning(|_| Ok(()));

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        buf.copy_from_slice(&[0u8; 32usize]);
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_aes().once().return_const(aes);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(rng);
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

    let result = app_session.aes_gen_key(
        None,
        AesKeyKind::Aes256,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::App,
    );

    assert!(result.is_ok());
    let key = result.unwrap();

    assert!(key.usage_allowed(AesKeyUsage::EncryptDecrypt).is_ok());

    let msg_in_arr = [0u8; 16];
    let msg_in: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_in_arr.as_ptr(), msg_in_arr.len());

    let msg_out_arr = [0u8; 16];
    let msg_out: MborByteArray<1024> =
        MborByteArray::new_with_len(msg_out_arr.as_ptr(), msg_out_arr.len());

    let msg_in_binding = (&msg_in).into();
    let msg_out_binding = (&msg_out).into();

    let input = AesEncDecIn::new(
        AesEncDecMode::Ecb,
        AesEncDecOp::Encrypt,
        None,
        &msg_in_binding,
        &msg_out_binding,
    );

    let result = app_session.aes_enc_dec(1, AesKeyIn::KeyId(key.id()), &input);

    assert!(result.is_ok());
}

#[test]
fn test_aes_key_unwrap() {
    const TAG: u16 = 10;

    let mut aes_key_unwrap_req = MockSimplexPipe::new();
    aes_key_unwrap_req
        .expect_send()
        .once()
        .returning(|_| Ok(()));

    let mut aes_key_unwrap_resp = MockSimplexPipe::new();
    aes_key_unwrap_resp.expect_recv().once().returning(|| {
        Some(SoftAesOffloadResp {
            range: Ok((0, 16)),
            tag: TAG,
        })
    });

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_soft_aes_req()
        .once()
        .return_const(aes_key_unwrap_req);
    hal.expect_soft_aes_resp()
        .once()
        .return_const(aes_key_unwrap_resp);
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
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let kek = [0x1; 32];
    let unwrap_blob = [0; 3072];
    let result = app_session.begin_aes_key_unwrap(TAG, &kek, &unwrap_blob);

    assert_eq!(result, Err(HsmErr::Pending));

    let result = app_session.end_aes_key_unwrap(TAG);
    assert!(result.is_ok());
    let result = result.unwrap();

    assert_eq!(result.start, 0);
    assert_eq!(result.end, 16);
}

#[test]
fn test_begin_aes_key_unwrap_send_fail() {
    const TAG: u16 = 10;

    let mut aes_key_unwrap_req = MockSimplexPipe::new();
    aes_key_unwrap_req
        .expect_send()
        .once()
        .returning(|_| Err(u32::MAX));

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_soft_aes_req()
        .once()
        .return_const(aes_key_unwrap_req);
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
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let kek = [0x1; 32];
    let unwrap_blob = [0; 3072];
    let result = app_session.begin_aes_key_unwrap(TAG, &kek, &unwrap_blob);

    assert_eq!(result, Err(HsmErr::SoftAesReqSendFailed));
}

#[test]
fn test_end_aes_key_unwrap_recv_none() {
    const TAG: u16 = 10;

    let mut aes_key_unwrap_resp = MockSimplexPipe::new();
    aes_key_unwrap_resp.expect_recv().once().returning(|| None);

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_soft_aes_resp()
        .once()
        .return_const(aes_key_unwrap_resp);
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
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let result = app_session.end_aes_key_unwrap(TAG);
    assert_eq!(result, Err(HsmErr::SpuriousIpcMessageEvent));
}

#[test]
fn test_end_aes_key_unwrap_invalid_tag() {
    const TAG: u16 = 10;
    const MIS_MATCHED_TAG: u16 = 11;

    let mut aes_key_unwrap_resp = MockSimplexPipe::new();
    aes_key_unwrap_resp.expect_recv().once().returning(|| {
        Some(SoftAesOffloadResp {
            range: Ok((0, 16)),
            tag: MIS_MATCHED_TAG,
        })
    });

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_soft_aes_resp()
        .once()
        .return_const(aes_key_unwrap_resp);
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
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let result = app_session.end_aes_key_unwrap(TAG);
    assert_eq!(result, Err(HsmErr::IoTagMismatch));
}

#[test]
fn test_end_aes_key_unwrap_op_failed() {
    const TAG: u16 = 10;

    let mut aes_key_unwrap_resp = MockSimplexPipe::new();
    aes_key_unwrap_resp.expect_recv().once().returning(|| {
        Some(SoftAesOffloadResp {
            range: Err(u32::MAX),
            tag: TAG,
        })
    });

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();

    hal.expect_soft_aes_resp()
        .once()
        .return_const(aes_key_unwrap_resp);
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
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state);

    let result = app_session.end_aes_key_unwrap(TAG);
    assert_eq!(result, Err(HsmErr::RsaUnwrapAesUnwrapFailed));
}
