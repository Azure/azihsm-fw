// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaRsaCmd;
use mcr_crypto_pka::PkaRsaSize;
use mcr_ddi_mbor::MborByteArray;
use mcr_types::*;

use crate::cmd_scheduler::TagId;
use crate::mock::*;
use crate::partition::cred_mgr::APP_VAULT_ID_FOR_INTERNAL_KEYS;
use crate::partition::tests::cmd_scheduler;
use crate::partition::tests::rev;
use crate::partition::tests::set_ipc_expectations;
use crate::partition::EntryKind;
use crate::partition::HsmSession;
use crate::partition::HsmUserSession;
use crate::partition::KeyAvailability;
use crate::partition::PartEnv;
use crate::partition::PartState;
use crate::partition::RsaKeyImported;
use crate::partition::RsaKeyKind;
use crate::partition::RsaKeyUsage;
use crate::partition::RsaPubKey;
use crate::partition::RsaSize;
use crate::partition::UserSession;
use crate::HsmErr;
use mcr_ddi_types::DdiHashAlgorithm;
use mcr_ddi_types::DdiRsaCryptoPadding;

#[test]
fn test_rsa_mod_exp_pct_validation_2k() {
    test_rsa_mod_exp_pct_validation(PkaRsaSize::Rsa2k, false, RsaKeyUsage::SignVerify);
}

#[test]
fn test_rsa_mod_exp_pct_validation_2k_crt() {
    test_rsa_mod_exp_pct_validation(PkaRsaSize::Rsa2k, true, RsaKeyUsage::SignVerify);
}

#[test]
fn test_rsa_mod_exp_pct_validation_3k() {
    test_rsa_mod_exp_pct_validation(PkaRsaSize::Rsa3k, false, RsaKeyUsage::SignVerify);
}

#[test]
fn test_rsa_mod_exp_pct_validation_3k_crt() {
    test_rsa_mod_exp_pct_validation(PkaRsaSize::Rsa3k, true, RsaKeyUsage::SignVerify);
}

#[test]
fn test_rsa_mod_exp_pct_validation_4k() {
    test_rsa_mod_exp_pct_validation(PkaRsaSize::Rsa4k, false, RsaKeyUsage::SignVerify);
}

#[test]
fn test_rsa_mod_exp_pct_validation_4k_crt() {
    test_rsa_mod_exp_pct_validation(PkaRsaSize::Rsa4k, true, RsaKeyUsage::SignVerify);
}

#[test]
fn test_rsa_mod_exp_pct_encrypt_decrypt_2k() {
    test_rsa_mod_exp_pct_validation(PkaRsaSize::Rsa2k, false, RsaKeyUsage::EncryptDecrypt);
}

#[test]
fn test_rsa_mod_exp_pct_encrypt_decrypt_2k_crt() {
    test_rsa_mod_exp_pct_validation(PkaRsaSize::Rsa2k, true, RsaKeyUsage::EncryptDecrypt);
}

fn test_rsa_mod_exp_pct_validation(rsa_type: PkaRsaSize, is_crt: bool, key_usage: RsaKeyUsage) {
    let rsa_size = RsaSize::try_from(rsa_type).unwrap();
    let data_length = rsa_size.into();
    let entry_kind = if is_crt {
        match rsa_size {
            RsaSize::Rsa2k => EntryKind::Rsa2kPrivateCrt,
            RsaSize::Rsa3k => EntryKind::Rsa3kPrivateCrt,
            RsaSize::Rsa4k => EntryKind::Rsa4kPrivateCrt,
        }
    } else {
        match rsa_size {
            RsaSize::Rsa2k => EntryKind::Rsa2kPrivate,
            RsaSize::Rsa3k => EntryKind::Rsa3kPrivate,
            RsaSize::Rsa4k => EntryKind::Rsa4kPrivate,
        }
    };

    let mut output_data = [0; 512];
    for (i, item) in output_data.iter_mut().enumerate().take(data_length) {
        *item = i as u8;
    }

    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(1)
        .return_once(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .return_once(|s| Some(MockDmaAlloc::new(s)));

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(move || {
        let mut pka = MockPka::new();

        if is_crt {
            pka.expect_begin_rsa_private_key_op_crt_zc()
                .times(2)
                .returning(
                    move |_tag, _rsa_type, _crt_param1, _crt_param2, _input, _output| {
                        Ok(PkaRsaCmd { rsa_type })
                    },
                );
            pka.expect_end_rsa_private_key_op_crt_zc()
                .times(2)
                .returning(move |_tag, _op| Ok(()));
        } else {
            pka.expect_begin_rsa_private_key_op_zc().times(2).returning(
                move |_tag, rsa_type, _private_key, _input, _output| Ok(PkaRsaCmd { rsa_type }),
            );
            pka.expect_end_rsa_private_key_op_zc()
                .times(2)
                .returning(move |_tag, _op| Ok(()));
        }

        pka.expect_peek_tag()
            .times(2)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_public_key_op_zc().times(2).returning(
            move |_tag, rsa_type, _public_key, input, output| {
                let output_slice = unsafe {
                    core::slice::from_raw_parts_mut(output.addr() as *mut u8, output.len())
                };
                output_slice.copy_from_slice(input.slice());
                Ok(PkaRsaCmd { rsa_type })
            },
        );

        pka.expect_peek_tag()
            .times(2)
            .returning(|| Some(TagId::default()));

        pka.expect_end_rsa_public_key_op_zc()
            .times(2)
            .returning(move |_tag, _op| Ok(()));

        pka
    });

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();

    hal.expect_dma_heap().times(2).return_const(heap);

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
    if key_usage == RsaKeyUsage::SignVerify {
        let mut sha = MockSha::new();
        sha.expect_digest_zc().times(2).returning(move |cmd_info| {
            let output_slice = cmd_info.output_buffer.slice();

            let mut modifiable_mem_range = IoMemRange::from(output_slice);
            let hw_digest_len = cmd_info.mode.get_digest_size_hw();
            // fill the output buffer with 1s to ensure the validity check 1 < m < n - 1 passes
            modifiable_mem_range
                .slice_mut()
                .copy_from_slice(&[1u8; 512][..hw_digest_len]);

            Ok(())
        });
        hal.expect_sha().return_const(sha);
    }

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let app_session = UserSession::new(rev(), 10, state.clone());

    let key_kind = RsaKeyKind::try_from(entry_kind).unwrap();
    let mut key_blob = vec![0xFF; entry_kind.raw_key_blob_size()];
    key_blob[0] = 0x01;

    let rsa_key = RsaKeyImported::new(key_kind, key_usage, &key_blob).unwrap();

    let vault_id = match key_usage {
        RsaKeyUsage::Unwrap => APP_VAULT_ID_FOR_INTERNAL_KEYS,
        _ => app_session.app_vault_id(),
    };

    let result = app_session.state.vault().rsa_import_key(
        vault_id,
        app_session.id(),
        None,
        false,
        &rsa_key,
        KeyAvailability::App,
    );

    assert!(result.is_ok());
    let key = result.unwrap();

    let pub_key = match rsa_size {
        RsaSize::Rsa2k => RsaPubKey::from_priv_pka_slice(&[2; 516], RsaSize::Rsa2k).unwrap(),
        RsaSize::Rsa3k => RsaPubKey::from_priv_pka_slice(&[2; 772], RsaSize::Rsa3k).unwrap(),
        RsaSize::Rsa4k => RsaPubKey::from_priv_pka_slice(&[2; 1028], RsaSize::Rsa4k).unwrap(),
    };

    // Extract n and e slices
    let n_slice = &pub_key.data[..pub_key.n_len];
    let e = &pub_key.data[pub_key.n_len..pub_key.n_len + 4];
    let mut n = n_slice.to_vec();

    let begin_rsa_pct_validation_result = app_session.begin_rsa_pct_validation(
        tag,
        key.id(),
        key_usage,
        rsa_type,
        n.as_mut_slice(),
        e,
    );

    assert!(begin_rsa_pct_validation_result.is_ok());
    let mut pct_cmd = begin_rsa_pct_validation_result.unwrap();

    let continue_rsa_pct_validation_result = app_session.continue_rsa_pct_validation(pct_cmd);
    assert!(continue_rsa_pct_validation_result.is_ok());
    pct_cmd = continue_rsa_pct_validation_result.unwrap();

    let end_rsa_pct_validation_result = app_session.end_rsa_pct_validation(pct_cmd);
    assert!(end_rsa_pct_validation_result.is_ok());

    // Verify the other API to get pub key
    let pub_key = match rsa_size {
        RsaSize::Rsa2k => RsaPubKey::from_priv_crt_pka_slice(&[2; 1284], RsaSize::Rsa2k).unwrap(),
        RsaSize::Rsa3k => RsaPubKey::from_priv_crt_pka_slice(&[2; 1924], RsaSize::Rsa3k).unwrap(),
        RsaSize::Rsa4k => RsaPubKey::from_priv_crt_pka_slice(&[2; 2564], RsaSize::Rsa4k).unwrap(),
    };

    // Extract n and e slices
    let n_slice = &pub_key.data[..pub_key.n_len];
    let e = &pub_key.data[pub_key.n_len..pub_key.n_len + 4];
    let mut n = n_slice.to_vec();

    let begin_rsa_pct_validation_result = app_session.begin_rsa_pct_validation(
        tag,
        key.id(),
        key_usage,
        rsa_type,
        n.as_mut_slice(),
        e,
    );

    assert!(begin_rsa_pct_validation_result.is_ok());
    let mut pct_cmd = begin_rsa_pct_validation_result.unwrap();

    let continue_rsa_pct_validation_result = app_session.continue_rsa_pct_validation(pct_cmd);
    assert!(continue_rsa_pct_validation_result.is_ok());
    pct_cmd = continue_rsa_pct_validation_result.unwrap();

    let end_rsa_pct_validation_result = app_session.end_rsa_pct_validation(pct_cmd);
    assert!(end_rsa_pct_validation_result.is_ok());
}

/// Enum for testing validation function 0 < m < n - 1
enum MsgInputConfig {
    AllZero,
    NormalMessage,
    ExceededDataValue,
}

#[test]
fn test_rsa_mod_exp_2k_crt() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa2k, true, MsgInputConfig::NormalMessage);
}

#[test]
fn test_rsa_mod_exp_3k_crt() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa3k, true, MsgInputConfig::NormalMessage);
}

#[test]
fn test_rsa_mod_exp_4k_crt() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa4k, true, MsgInputConfig::NormalMessage);
}

#[test]
fn test_rsa_mod_exp_2k_no_crt_zc() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa2k, false, MsgInputConfig::NormalMessage);
}

#[test]
#[should_panic]
fn test_rsa_mod_exp_2k_no_crt_zc_zero_msg() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa2k, false, MsgInputConfig::AllZero);
}

#[test]
#[should_panic]
fn test_rsa_mod_exp_2k_no_crt_zc_large_msg() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa2k, false, MsgInputConfig::ExceededDataValue);
}

#[test]
fn test_rsa_mod_exp_3k_no_crt_zc() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa3k, false, MsgInputConfig::NormalMessage);
}

#[test]
#[should_panic]
fn test_rsa_mod_exp_3k_no_crt_zc_zero_msg() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa3k, false, MsgInputConfig::AllZero);
}

#[test]
#[should_panic]
fn test_rsa_mod_exp_3k_no_crt_zc_large_msg() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa3k, false, MsgInputConfig::ExceededDataValue);
}

#[test]
fn test_rsa_mod_exp_4k_no_crt_zc() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa4k, false, MsgInputConfig::NormalMessage);
}

#[test]
#[should_panic]
fn test_rsa_mod_exp_4k_no_crt_zc_zero_msg() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa4k, false, MsgInputConfig::AllZero);
}

#[test]
#[should_panic]
fn test_rsa_mod_exp_4k_no_crt_zc_large_msg() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa4k, false, MsgInputConfig::ExceededDataValue);
}

#[test]
fn test_rsa_mod_exp_2k_crt_zc() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa2k, true, MsgInputConfig::NormalMessage);
}

#[test]
#[should_panic]
fn test_rsa_mod_exp_2k_crt_zc_zero_msg() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa2k, true, MsgInputConfig::AllZero);
}

#[test]
#[should_panic]
fn test_rsa_mod_exp_2k_crt_zc_large_msg() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa2k, true, MsgInputConfig::ExceededDataValue);
}

#[test]
fn test_rsa_mod_exp_3k_crt_zc() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa3k, true, MsgInputConfig::NormalMessage);
}

#[test]
#[should_panic]
fn test_rsa_mod_exp_3k_crt_zc_zero_msg() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa3k, true, MsgInputConfig::AllZero);
}

#[test]
#[should_panic]
fn test_rsa_mod_exp_3k_crt_zc_large_msg() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa3k, true, MsgInputConfig::ExceededDataValue);
}

#[test]
fn test_rsa_mod_exp_4k_crt_zc() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa4k, true, MsgInputConfig::NormalMessage);
}

#[test]
#[should_panic]
fn test_rsa_mod_exp_4k_crt_zc_zero_msg() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa4k, true, MsgInputConfig::AllZero);
}

#[test]
#[should_panic]
fn test_rsa_mod_exp_4k_crt_zc_large_msg() {
    test_rsa_mod_exp_generic(PkaRsaSize::Rsa4k, true, MsgInputConfig::ExceededDataValue);
}

fn test_rsa_mod_exp_generic(rsa_type: PkaRsaSize, is_crt: bool, m_config: MsgInputConfig) {
    let rsa_size = RsaSize::try_from(rsa_type).unwrap();
    let data_length = rsa_size.into();
    let mut entry_kind = match rsa_size {
        RsaSize::Rsa2k => EntryKind::Rsa2kPrivate,
        RsaSize::Rsa3k => EntryKind::Rsa3kPrivate,
        RsaSize::Rsa4k => EntryKind::Rsa4kPrivate,
    };

    if is_crt {
        entry_kind = match rsa_size {
            RsaSize::Rsa2k => EntryKind::Rsa2kPrivateCrt,
            RsaSize::Rsa3k => EntryKind::Rsa3kPrivateCrt,
            RsaSize::Rsa4k => EntryKind::Rsa4kPrivateCrt,
        };
    }

    let mut output_data = [0; 512];
    for (i, item) in output_data.iter_mut().enumerate().take(data_length) {
        *item = i as u8;
    }

    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let tag = TagId::default();

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(move || {
        let mut pka = MockPka::new();

        if is_crt {
            pka.expect_begin_rsa_private_key_op_crt_zc()
                .times(1)
                .returning(
                    move |_tag, _rsa_type, _crt_param1, _crt_param2, _input, _output| {
                        Ok(PkaRsaCmd { rsa_type })
                    },
                );
        } else {
            pka.expect_begin_rsa_private_key_op_zc().times(1).returning(
                move |_tag, _rsa_type, _private_key, _input, _output| Ok(PkaRsaCmd { rsa_type }),
            );
        }

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        if is_crt {
            pka.expect_end_rsa_private_key_op_crt_zc()
                .times(1)
                .returning(move |_tag, _op| Ok(()));
        } else {
            pka.expect_end_rsa_private_key_op_zc()
                .times(1)
                .returning(move |_tag, _op| Ok(()));
        }

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

    let key_kind = RsaKeyKind::try_from(entry_kind).unwrap();
    let key_blob = vec![100u8; entry_kind.raw_key_blob_size()];
    let mut key_usage = RsaKeyUsage::SignVerify;
    if rsa_size == RsaSize::Rsa3k {
        key_usage = RsaKeyUsage::EncryptDecrypt;
    }

    let rsa_key = RsaKeyImported::new(key_kind, key_usage, &key_blob).unwrap();

    let result = app_session.state.vault().rsa_import_key(
        app_session.app_vault_id(),
        app_session.id(),
        None,
        false,
        &rsa_key,
        KeyAvailability::App,
    );

    assert!(result.is_ok());
    let key = result.unwrap();

    let buf = match m_config {
        MsgInputConfig::AllZero => [0u8; RsaSize::MAX_LEN],
        MsgInputConfig::NormalMessage => {
            // Test case where m == n - 2, satisfying 0 < m < n - 1
            let mut buf = [100u8; RsaSize::MAX_LEN];
            buf[0] -= 2;
            buf
        }
        MsgInputConfig::ExceededDataValue => {
            // Test case where m == n - 1, exceeding m < n - 1
            let mut buf = [100u8; RsaSize::MAX_LEN];
            buf[0] -= 1;
            buf
        }
    };

    let mborbytearray: MborByteArray<{ RsaSize::MAX_LEN }> =
        MborByteArray::new_with_len(buf.as_ptr(), rsa_size.into());

    let begin_rsa_mod_exp_result = app_session.begin_rsa_mod_exp_zc(
        tag,
        key.id(),
        Some(key_usage),
        &(&mborbytearray).into(),
        &(&mborbytearray).into(),
    );
    println!("{:?}", begin_rsa_mod_exp_result.as_ref().err());
    assert!(begin_rsa_mod_exp_result.is_ok());
    let end_rsa_mod_exp_result =
        app_session.end_rsa_mod_exp_zc(tag, begin_rsa_mod_exp_result.unwrap());
    assert!(end_rsa_mod_exp_result.is_ok());
}

#[test]
fn test_check_valid_input() {
    let test_cases = vec![
        (
            vec![65, 83, 44, 67, 65, 87, 98, 80, 49, 75],
            vec![64, 83, 44, 67, 65, 87, 98, 80, 49, 75],
            false,
        ), // m == n - 1
        (
            vec![65, 83, 44, 67, 65, 87, 98, 80, 49, 75],
            vec![65, 83, 44, 67, 65, 87, 98, 80, 49, 75],
            false,
        ), // m == n
        (
            vec![65, 83, 44, 67, 65, 87, 98, 80, 49, 75],
            vec![66, 83, 44, 67, 65, 87, 98, 80, 49, 75],
            false,
        ), // m > n - 1
        (
            vec![65, 83, 44, 67, 65, 87, 98, 80, 49, 75],
            vec![0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
            false,
        ), // m == 0
        (
            vec![65, 83, 44, 67, 65, 87, 98, 80, 49, 75],
            vec![1, 0, 0, 0, 0, 0, 0, 0, 0, 0],
            false,
        ), // m == 1
        (
            vec![3, 0, 0, 0, 0, 0, 0, 0, 0, 0],
            vec![0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
            false,
        ), // m == 0, m < n - 1
        (
            vec![3, 0, 0, 0, 0, 0, 0, 0, 0, 0],
            vec![1, 0, 0, 0, 0, 0, 0, 0, 0, 0],
            false,
        ), // m == 1, m < n - 1
        (
            vec![65, 83, 44, 67, 65, 87, 98, 80, 49, 75],
            vec![63, 83, 44, 67, 65, 87, 98, 80, 49, 75],
            true,
        ), // 1 < m < n - 1
        (
            vec![65, 83, 44, 67, 65, 87, 98, 80, 49, 75],
            vec![65, 82, 44, 67, 65, 87, 98, 80, 49, 75],
            true,
        ), // 1 < m < n - 1
        (
            vec![4, 83, 44, 67, 65, 87, 98, 80, 49, 75],
            vec![1, 83, 44, 67, 65, 87, 98, 80, 49, 75],
            true,
        ), // 1 < m < n - 1
        (vec![0, 2], vec![255, 1], false),
    ];

    for (n, m, expected) in test_cases {
        let result = UserSession::<MockEnv>::check_valid_input(&n, &m);
        println!(
            "n: {:?}, m: {:?}, expected: {}, result: {}",
            n, m, expected, result
        );
        assert_eq!(result, expected);
    }
}

//add mock test for decode_oaep_kek_inner
#[test]
fn test_decode_oaep_kek_success() {
    // Setup mock environment
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(..) // Allow any number of calls
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut hal = MockHal::new();
    hal.expect_dma_heap().times(..).return_const(heap);

    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut sha = MockSha::new();
    sha.expect_digest_zc().times(..).returning(move |cmd_info| {
        // Allow any number of calls - write directly to output buffer for zero-copy API
        let output_slice = cmd_info.output_buffer.slice();
        let mut modifiable_mem_range = IoMemRange::from(output_slice);
        let sha1_output = openssl::sha::sha1(cmd_info.buffer.slice());
        let output_len = sha1_output.len().min(output_slice.len());
        modifiable_mem_range.slice_mut()[..output_len].copy_from_slice(&sha1_output[..output_len]);
        Ok(())
    });
    sha.expect_decode_oaep_kek()
        .once()
        .returning(|_wrapped, _hash| Ok(vec![0x01, 0x02, 0x03, 0x04].into()));
    hal.expect_sha().return_const(sha);
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

    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    let session = UserSession::new(rev(), 10, state);

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
    let padding = DdiRsaCryptoPadding::Oaep;
    let hash_alg = DdiHashAlgorithm::Sha1;
    // Convert from big endian to little endian by reversing the bytes
    let wrapped_data: Vec<u8> = RSA_2K_MOD_EXP_OUTPUT.iter().rev().cloned().collect();

    // Call the function under test
    let result = session.decode_oaep_kek(&wrapped_data, padding, hash_alg);

    // Check result is Ok
    assert!(result.is_ok());
    let output = result.unwrap();
    assert!(!output.is_empty());
}

#[test]
fn test_decode_oaep_kek_invalid_length() {
    // Setup mock environment
    let mut hal = MockHal::new();

    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut sha = MockSha::new();
    sha.expect_decode_oaep_kek()
        .once()
        .returning(|_wrapped, _hash| {
            // Mock error for invalid length
            Err(HsmErr::RsaUnwrapOaepDecodeFailed as u32)
        });
    hal.expect_sha().return_const(sha);

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

    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    let session = UserSession::new(rev(), 10, state);

    const RSA_2K_MOD_EXP_OUTPUT_INVALID: [u8; 32] = [
        0x00, 0xcc, 0x5a, 0x42, 0xaa, 0xd0, 0x37, 0x18, 0xb7, 0x84, 0x58, 0xa7, 0x8f, 0x05, 0xc3,
        0x7f, 0xaf, 0xec, 0x42, 0x45, 0x93, 0x23, 0xf5, 0x4b, 0xe6, 0x45, 0xca, 0x95, 0x85, 0x14,
        0x4a, 0x1c,
    ];
    let padding = DdiRsaCryptoPadding::Oaep;
    let hash_alg = DdiHashAlgorithm::Sha1;
    // Convert from big endian to little endian by reversing the bytes
    let wrapped_data: Vec<u8> = RSA_2K_MOD_EXP_OUTPUT_INVALID
        .iter()
        .rev()
        .cloned()
        .collect();

    // Call the function under test
    let result = session.decode_oaep_kek(&wrapped_data, padding, hash_alg);

    // Check result is failed
    assert!(result.is_err());
}

#[test]
fn test_decode_oaep_kek_invalid_lhash() {
    // Setup mock environment
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(..) // Allow any number of calls
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut hal = MockHal::new();
    hal.expect_dma_heap().times(..).return_const(heap);

    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut sha = MockSha::new();
    sha.expect_digest_zc().times(..).returning(move |cmd_info| {
        // Allow any number of calls - write directly to output buffer for zero-copy API
        let output_slice = cmd_info.output_buffer.slice();
        let mut modifiable_mem_range = IoMemRange::from(output_slice);
        let sha1_output = openssl::sha::sha1(cmd_info.buffer.slice());
        let output_len = sha1_output.len().min(output_slice.len());
        modifiable_mem_range.slice_mut()[..output_len].copy_from_slice(&sha1_output[..output_len]);
        Ok(())
    });
    sha.expect_decode_oaep_kek()
        .once()
        .returning(|_wrapped, _hash| {
            // Mock error for invalid lhash
            Err(HsmErr::RsaUnwrapOaepDecodeFailed as u32)
        });
    hal.expect_sha().return_const(sha);

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

    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    let session = UserSession::new(rev(), 10, state);

    const RSA_2K_MOD_EXP_OUTPUT_TAMPERED: [u8; 256] = [
        0x00, 0xcc, 0x5a, 0x42, 0x00, 0x00, 0x37, 0x18, 0xb7, 0x84, 0x58, 0xa7, 0x8f, 0x05, 0xc3,
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
    let padding = DdiRsaCryptoPadding::Oaep;
    let hash_alg = DdiHashAlgorithm::Sha1;
    // Convert from big endian to little endian by reversing the bytes
    let wrapped_data: Vec<u8> = RSA_2K_MOD_EXP_OUTPUT_TAMPERED
        .iter()
        .rev()
        .cloned()
        .collect();

    // Call the function under test
    let result = session.decode_oaep_kek(&wrapped_data, padding, hash_alg);

    // Check result is failed
    assert!(result.is_err());
}
