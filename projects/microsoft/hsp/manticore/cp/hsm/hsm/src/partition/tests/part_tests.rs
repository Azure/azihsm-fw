// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaEccSecretValue;
use mcr_ddi_mbor::MborByteArray;
use mcr_types::DevCqId;
use mcr_types::DevSqId;
use part::HmacHashAlgorithm;
use store::PhysicalTable;

use super::*;
use crate::mock::MockAes;
use crate::mock::MockDmaAlloc;
use crate::mock::MockDmaHeap;
use crate::mock::MockEnv;
use crate::mock::MockHal;
use crate::mock::MockPka;
use crate::mock::MockSha;
use crate::partition::tests::partition;
use crate::partition::tests::rev;
use crate::partition::tests::MAX_API_REV_TEST;
use crate::partition::tests::MIN_API_REV_TEST;
use crate::partition::vault::store::EntryAttributes;
use crate::partition::HsmPartition;
use crate::partition::HsmUserSession;
use crate::partition::MAX_CERTS;

const TEST_BK3: [u8; 48] = [0x42; 48];
const TEST_BK_BOOT_MASKING_KEY: [u8; 80] = [0x4B; 80];

#[allow(unused)]
const TEST_POTA_ECC_PRIVATE_KEY: [u8; 185] = [
    0x30, 0x81, 0xb6, 0x02, 0x01, 0x00, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
    0x01, 0x06, 0x05, 0x2b, 0x81, 0x04, 0x00, 0x22, 0x04, 0x81, 0x9e, 0x30, 0x81, 0x9b, 0x02, 0x01,
    0x01, 0x04, 0x30, 0x17, 0xe9, 0x1c, 0xac, 0xf7, 0xb7, 0x21, 0xd7, 0x75, 0x20, 0x02, 0x07, 0xbc,
    0xaa, 0x94, 0x2c, 0xe3, 0xb5, 0x5b, 0x78, 0x13, 0xcc, 0x8b, 0xde, 0x87, 0x65, 0x6b, 0xe1, 0x7b,
    0xc2, 0xa8, 0xcc, 0x89, 0x33, 0x4e, 0xcd, 0xaa, 0x9d, 0x1d, 0x09, 0xf1, 0xc7, 0x01, 0x1b, 0x64,
    0xeb, 0x78, 0x5b, 0xa1, 0x64, 0x03, 0x62, 0x00, 0x04, 0x1f, 0x42, 0x0d, 0x73, 0xeb, 0xf0, 0x67,
    0xc2, 0xf9, 0x77, 0xbd, 0x51, 0xab, 0xfb, 0xe1, 0xf6, 0x53, 0x19, 0xb7, 0x57, 0xe0, 0xa9, 0x20,
    0xce, 0x4f, 0x21, 0xbb, 0xd4, 0xa7, 0x84, 0x1c, 0x93, 0x45, 0xf1, 0xea, 0xd9, 0x5f, 0xe5, 0x90,
    0xab, 0x57, 0xe1, 0xea, 0xfc, 0xd2, 0x06, 0xef, 0x21, 0xa2, 0xad, 0x10, 0xd3, 0x17, 0x6e, 0x99,
    0xc8, 0x22, 0x26, 0x23, 0x08, 0x57, 0xa7, 0x56, 0x08, 0x45, 0xe3, 0xda, 0x12, 0xc7, 0xdc, 0x3a,
    0xee, 0x01, 0xfc, 0x37, 0xab, 0x1c, 0x8d, 0xc6, 0xd0, 0x64, 0x7a, 0x7d, 0xc2, 0x67, 0xfc, 0x02,
    0x7d, 0x8d, 0xa3, 0xc8, 0x01, 0x4b, 0xa4, 0x0d, 0x98,
];

const TEST_POTA_ECC_PUB_KEY: [u8; 120] = [
    0x30, 0x76, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x05, 0x2b,
    0x81, 0x04, 0x00, 0x22, 0x03, 0x62, 0x00, 0x04, 0x1f, 0x42, 0x0d, 0x73, 0xeb, 0xf0, 0x67, 0xc2,
    0xf9, 0x77, 0xbd, 0x51, 0xab, 0xfb, 0xe1, 0xf6, 0x53, 0x19, 0xb7, 0x57, 0xe0, 0xa9, 0x20, 0xce,
    0x4f, 0x21, 0xbb, 0xd4, 0xa7, 0x84, 0x1c, 0x93, 0x45, 0xf1, 0xea, 0xd9, 0x5f, 0xe5, 0x90, 0xab,
    0x57, 0xe1, 0xea, 0xfc, 0xd2, 0x06, 0xef, 0x21, 0xa2, 0xad, 0x10, 0xd3, 0x17, 0x6e, 0x99, 0xc8,
    0x22, 0x26, 0x23, 0x08, 0x57, 0xa7, 0x56, 0x08, 0x45, 0xe3, 0xda, 0x12, 0xc7, 0xdc, 0x3a, 0xee,
    0x01, 0xfc, 0x37, 0xab, 0x1c, 0x8d, 0xc6, 0xd0, 0x64, 0x7a, 0x7d, 0xc2, 0x67, 0xfc, 0x02, 0x7d,
    0x8d, 0xa3, 0xc8, 0x01, 0x4b, 0xa4, 0x0d, 0x98,
];

fn mgr_session_app_session_expectations(hal: &mut MockHal) {
    let mut aes = MockAes::new();
    let mut pka = MockPka::new();
    let mut sha = MockSha::new();

    aes.expect_encrypt_decrypt().times(3).returning(|_| Ok(()));
    hal.expect_aes().times(3).return_const(aes);

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

    sha.expect_kbkdf_counter_hmac().return_const(Ok(()));
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

    let mut heap = MockDmaHeap::new();
    let heap_allocate_per_session = 19;
    heap.expect_allocate()
        .times(heap_allocate_per_session)
        .returning(|s| {
            let mut alloc = MockDmaAlloc::new(s);
            // Return app pin
            alloc.as_ref_mut()[..16].copy_from_slice(&[TEST_APP_PIN_VAL; 16usize]);
            Some(alloc)
        });
    hal.expect_dma_heap().return_const(heap);
}

#[test]
fn test_clone() {
    let part = partition(true);
    let _ = part.clone();
}

#[test]
fn test_min_api_rev() {
    let part = partition(true);
    assert_eq!(part.min_api_rev(), MIN_API_REV_TEST);
}

#[test]
fn test_max_api_rev() {
    let part = partition(true);
    assert_eq!(part.max_api_rev(), MAX_API_REV_TEST);
}

#[test]
fn test_enable() {
    let part = partition(true);
    part.enable();
}

// The test hook downgrades a cached `PctPassed` unwrapping key back to `PendingPct` (so the next
// `GetUnwrappingKey` re-runs the RSA PCT), and is a no-op for `Empty` / `PendingPct` slots.
#[test]
#[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
fn test_reset_unwrapping_key_pct() {
    use mcr_types::UnwrappingKeyValidity;

    let part = partition(true);

    // PctPassed -> PendingPct
    part.state
        .part_persistent_store_ref()
        .unwrapping_key_bk_valid = UnwrappingKeyValidity::PctPassed as u8;
    part.reset_unwrapping_key_pct();
    assert_eq!(
        part.state
            .part_persistent_store_ref()
            .unwrapping_key_bk_valid,
        UnwrappingKeyValidity::PendingPct as u8
    );

    // PendingPct stays PendingPct (no-op)
    part.reset_unwrapping_key_pct();
    assert_eq!(
        part.state
            .part_persistent_store_ref()
            .unwrapping_key_bk_valid,
        UnwrappingKeyValidity::PendingPct as u8
    );

    // Empty stays Empty (no-op)
    part.state
        .part_persistent_store_ref()
        .unwrapping_key_bk_valid = UnwrappingKeyValidity::Empty as u8;
    part.reset_unwrapping_key_pct();
    assert_eq!(
        part.state
            .part_persistent_store_ref()
            .unwrapping_key_bk_valid,
        UnwrappingKeyValidity::Empty as u8
    );
}

#[test]
fn test_disable() {
    const VAULT_SIZE_DWORDS: usize = 1024 * 17 * 65 / 4;
    let vault: [u32; VAULT_SIZE_DWORDS] = [0; VAULT_SIZE_DWORDS];
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    let part_persistent_store_memory = [0u8; 2048 * 65];
    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(2).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().times(2).return_const(rng_nonce);

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .once()
        .return_const(vault.as_ptr() as usize);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .return_const(cdma_vault_meta_data.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_ipc_expectations(&mut hal);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    part.disable(None);
}

#[test]
fn test_enabled() {
    const VAULT_SIZE_DWORDS: usize = 1024 * 17 * 65 / 4;
    let vault: [u32; VAULT_SIZE_DWORDS] = [0; VAULT_SIZE_DWORDS];
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    let part_persistent_store_memory = [0u8; 2048 * 65];
    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(2).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().times(2).return_const(rng_nonce);

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .once()
        .return_const(vault.as_ptr() as usize);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .return_const(cdma_vault_meta_data.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_ipc_expectations(&mut hal);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    assert!(!part.enabled());
    part.enable();
    assert!(part.enabled());
    part.disable(None);
    assert!(!part.enabled());
}

#[test]
fn test_set_resource_mask() {
    let part = partition(true);
    part.set_resource_mask(1);
    assert_eq!(part.resource_mask(), 1);
}

#[test]
fn test_resource_mask() {
    let part = partition(true);
    assert_eq!(part.resource_mask(), 0);
}

#[test]
fn test_enable_io_queue() {
    let part = partition(true);
    part.enable_io_queue(DevSqId::Id65, DevCqId::Id65);
    assert!(part.io_queue(DevSqId::Id65).is_some());
}

#[test]
fn test_disable_io_queue() {
    let part = partition(true);
    let delete_ctx = Some(IoQueueDeleteContext::new(10, false));
    part.enable_io_queue(DevSqId::Id65, DevCqId::Id65);
    assert!(!part.disable_io_queue(DevSqId::Id65, delete_ctx));
    assert!(part.io_queue(DevSqId::Id65).is_none());
}

#[test]
fn test_io_queue() {
    let part = partition(true);
    part.enable_io_queue(DevSqId::Id65, DevCqId::Id65);
    let result = part.io_queue(DevSqId::Id65);
    assert!(result.is_some());
    let ioq = result.unwrap();
    assert!(ioq.sq_id() == DevSqId::Id65);
    assert!(ioq.cq_id() == DevCqId::Id65);
    assert!(ioq.valid());
}

#[test]
fn test_open_app_session() {
    let part = partition_with_open_sessions_expectations(
        1,
        ExpectedParams {
            aes_encrypt_decrypt: 3,
            hal_aes: 3,
            hal_rng: 3,
            hal_bks_table: 2,
            heap_allocate: 19,
            rng_32_bytes: 2,
            rng_48_bytes: 1,
            ..Default::default()
        },
    );
    part.set_resource_mask(1);

    let sess_ctx = begin_and_continue_open_session(&part);

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
    assert!(result.is_ok());
}

#[test]
fn test_open_app_session_invalid_credential_tag() {
    let part = partition_with_open_sessions_expectations(
        1,
        ExpectedParams {
            hal_rng: 1,
            heap_allocate: 12,
            rng_32_bytes: 1,
            ..Default::default()
        },
    );
    part.set_resource_mask(1);

    let sess_ctx = begin_and_continue_open_session(&part);

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
        tag: [0; 48],
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
}

#[test]
fn test_open_app_session_invalid_nonce() {
    let part = partition_with_open_sessions_expectations(
        1,
        ExpectedParams {
            hal_rng: 1,
            heap_allocate: 8,
            rng_32_bytes: 1,
            ..Default::default()
        },
    );
    part.set_resource_mask(1);

    let sess_ctx = begin_and_continue_open_session(&part);

    let encrypted_id_buf = [1u8; 16];
    let encrypted_pin_buf = [1u8; 16];
    let encrypted_seed_buf = [1u8; 48];
    let iv_buf = [1u8; 16];

    let encrypted_cred = DdiEncryptedSessionCredential {
        encrypted_id: MborByteArray::new_with_len(encrypted_id_buf.as_ptr(), 16),
        encrypted_pin: MborByteArray::new_with_len(encrypted_pin_buf.as_ptr(), 16),
        encrypted_seed: MborByteArray::new_with_len(encrypted_seed_buf.as_ptr(), 48),
        iv: MborByteArray::new_with_len(iv_buf.as_ptr(), 16),
        nonce: [8; 32],
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
}

#[test]
fn test_close_app_session() {
    let part = partition_with_open_sessions_expectations(
        1,
        ExpectedParams {
            aes_encrypt_decrypt: 3,
            hal_aes: 3,
            hal_rng: 3,
            hal_bks_table: 2,
            heap_allocate: 19,
            rng_32_bytes: 2,
            rng_48_bytes: 1,
            ..Default::default()
        },
    );
    part.set_resource_mask(1);

    let sess_ctx = begin_and_continue_open_session(&part);

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
    assert!(result.is_ok());

    let app_sess = result.unwrap();
    assert!(part.close_user_session(app_sess.id()).is_ok());
}

#[test]
fn test_close_app_session_delete_all_session_keys() {
    let mut hal = MockHal::new();
    let part_persistent_store_memory = [0u8; 2048 * 65];
    mgr_session_app_session_expectations(&mut hal);

    let mut rng = MockRng::new();
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 48] = (1..49u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().times(1).returning(|buf| {
        buf.copy_from_slice(&[1u8; 16usize]);
    });
    hal.expect_rng().times(4).return_const(rng);

    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    const TEST_BKS_TABLE: [u8; 492] = [0x43; 492];
    hal.expect_bks_table_addr()
        .times(2)
        .return_const(TEST_BKS_TABLE.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_ipc_expectations(&mut hal);

    let part_env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let part = Partition::<MockEnv>::new(PcieFunction(0), part_env);

    part.set_resource_mask(1);

    let sess_ctx = begin_and_continue_open_session(&part);

    let table_memory_pre_open_session = table_memory;

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
    assert!(result.is_ok());

    let app_sess = result.unwrap();

    let table_memory_pre_genkey = table_memory;

    let result = app_sess.aes_gen_key(
        None,
        AesKeyKind::Aes128,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::Session,
    );
    assert!(result.is_ok());

    // Table memory should have key populated
    assert_ne!(table_memory, table_memory_pre_genkey);

    let table_memory_pre_close_session = table_memory;

    assert!(part.close_user_session(app_sess.id()).is_ok());

    // Table memory should now be empty after deleting the session only key
    assert_ne!(table_memory, table_memory_pre_close_session);
    assert_eq!(table_memory, table_memory_pre_open_session);
}

#[test]
fn test_close_app_session_delete_all_session_keys_aes_bulk256() {
    let mut hal = MockHal::new();
    let part_persistent_store_memory = [0u8; 2048 * 65];
    mgr_session_app_session_expectations(&mut hal);

    let mut rng = MockRng::new();
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 48] = (1..49u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().times(1).returning(|buf| {
        buf.copy_from_slice(&[0u8; 2usize]);
    });
    hal.expect_rng().times(4).return_const(rng);

    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [1; 65];
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .times(1)
        .return_const(cdma_vault_meta_data.as_ptr() as usize);

    const TEST_BKS_TABLE: [u8; 492] = [0x43; 492];
    hal.expect_bks_table_addr()
        .times(2)
        .return_const(TEST_BKS_TABLE.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_ipc_expectations(&mut hal);
    let part_env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());

    let part = Partition::<MockEnv>::new(PcieFunction(0), part_env);
    part.set_resource_mask(1);

    let sess_ctx = begin_and_continue_open_session(&part);

    let table_memory_pre_open_session = table_memory;

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
    assert!(result.is_ok());

    let app_sess = result.unwrap();

    let table_memory_pre_genkey = table_memory;

    let result = app_sess.aes_gen_key(
        None,
        AesKeyKind::AesGcmBulk256Unapproved,
        AesKeyUsage::EncryptDecrypt,
        KeyAvailability::Session,
    );
    assert!(result.is_ok());

    // Table memory should have key populated
    assert_ne!(table_memory, table_memory_pre_genkey);

    let table_memory_pre_close_session = table_memory;

    assert!(part.close_user_session(app_sess.id()).is_ok());

    // Table memory should now be empty after deleting the session only key
    assert_ne!(table_memory, table_memory_pre_close_session);
    assert_eq!(table_memory, table_memory_pre_open_session);
}

#[test]
fn test_begin_close_app_session_aes_bulk() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };
    let mut attributes = EntryAttributes::default();
    attributes.common.flags.set_session(true);

    let kind = EntryKind::AesGcmBulk256Unapproved;
    let session_id_or_key_tag = 1;
    let app_id = 0;
    let key_blob = [1; 2];

    let result = table.add_entry(&attributes, session_id_or_key_tag, kind, app_id, &key_blob);
    assert!(result.is_ok());

    let mut hal = MockHal::new();
    let part_persistent_store_memory = [0u8; 2048 * 65];
    mgr_session_app_session_expectations(&mut hal);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 48] = (1..49u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    hal.expect_rng().times(3).return_const(rng);

    hal.expect_vault_addr()
        .times(7)
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .times(0)
        .return_const(cdma_vault_meta_data.as_ptr() as usize);

    const TEST_BKS_TABLE: [u8; 492] = [0x43; 492];
    hal.expect_bks_table_addr()
        .times(2)
        .return_const(TEST_BKS_TABLE.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_fp_ipc_send_recv_expectations(&mut hal, 1, 1);
    set_hsp_ipc_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);
    let part_env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());

    let part = Partition::<MockEnv>::new(PcieFunction(0), part_env);

    part.set_resource_mask(1);

    let sess_ctx = begin_and_continue_open_session(&part);

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

    let table_memory_pre_open_session = table_memory;
    let part_persistent_store_memory_pre_open_session = part_persistent_store_memory;

    let result = part.end_open_user_session(
        sess_ctx,
        rev(),
        &encrypted_cred,
        None,
        &mut bk_buf[..],
        &mut mk_buf[..],
        None,
    );
    assert!(result.is_ok());

    let app_sess = result.unwrap();

    let table_memory_pre_close_session = table_memory;
    let part_persistent_store_memory_pre_close_session = part_persistent_store_memory;

    let result = part.begin_close_user_session(0, PcieFunction::Vf0, app_sess.id());

    assert!(result.is_ok());
    let result = result.unwrap();

    let result = part.end_close_user_session(&result);
    assert!(result.is_ok());

    assert_ne!(table_memory, table_memory_pre_close_session);
    assert_eq!(table_memory, table_memory_pre_open_session);

    assert_ne!(
        part_persistent_store_memory,
        part_persistent_store_memory_pre_close_session
    );
    assert_eq!(
        part_persistent_store_memory,
        part_persistent_store_memory_pre_open_session
    );
}

#[test]
fn test_begin_close_app_session_aes_bulk_failed() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };
    let mut attributes = EntryAttributes::default();
    attributes.common.flags.set_session(true);

    let kind = EntryKind::AesGcmBulk256Unapproved;
    let session_id_or_key_tag = 1;
    let app_id = 0;
    let key_blob = [1; 2];

    let result = table.add_entry(&attributes, session_id_or_key_tag, kind, app_id, &key_blob);
    assert!(result.is_ok());

    let mut hal = MockHal::new();
    let part_persistent_store_memory = [0u8; 2048 * 65];
    mgr_session_app_session_expectations(&mut hal);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 48] = (1..49u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    hal.expect_rng().times(3).return_const(rng);

    hal.expect_vault_addr()
        .times(5)
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .times(0)
        .return_const(cdma_vault_meta_data.as_ptr() as usize);

    const TEST_BKS_TABLE: [u8; 492] = [0x43; 492];
    hal.expect_bks_table_addr()
        .times(2)
        .return_const(TEST_BKS_TABLE.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_hsp_ipc_expectations(&mut hal);
    set_fp_ipc_send_recv_failed_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);

    let part_env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());

    let part = Partition::<MockEnv>::new(PcieFunction(0), part_env);

    part.set_resource_mask(1);

    let sess_ctx = begin_and_continue_open_session(&part);

    let table_memory_pre_open_session = table_memory;

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
    assert!(result.is_ok());

    let app_sess = result.unwrap();

    let table_memory_pre_close_session = table_memory;

    let result = part.begin_close_user_session(0, PcieFunction::Vf0, app_sess.id());

    assert!(result.is_ok());
    let result = result.unwrap();

    let result = part.end_close_user_session(&result);
    assert!(result.is_err());

    assert_ne!(table_memory, table_memory_pre_close_session);
    assert_ne!(table_memory, table_memory_pre_open_session);
}

#[test]
fn test_begin_close_app_session_aes_bulk_ipc_send_failed() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };
    let mut attributes = EntryAttributes::default();
    attributes.common.flags.set_session(true);

    let kind = EntryKind::AesGcmBulk256Unapproved;
    let session_id_or_key_tag = 1;
    let app_id = 0;
    let key_blob = [1; 2];

    let result = table.add_entry(&attributes, session_id_or_key_tag, kind, app_id, &key_blob);
    assert!(result.is_ok());

    let mut hal = MockHal::new();
    let part_persistent_store_memory = [0u8; 2048 * 65];
    mgr_session_app_session_expectations(&mut hal);

    let mut rng = MockRng::new();
    rng.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 48] = (1..49u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    hal.expect_rng().times(3).return_const(rng);

    hal.expect_vault_addr()
        .times(5)
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .times(0)
        .return_const(cdma_vault_meta_data.as_ptr() as usize);

    const TEST_BKS_TABLE: [u8; 492] = [0x43; 492];
    hal.expect_bks_table_addr()
        .times(2)
        .return_const(TEST_BKS_TABLE.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_hsp_ipc_expectations(&mut hal);
    set_fp_ipc_send_failed_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);

    let part_env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());

    let part = Partition::<MockEnv>::new(PcieFunction(0), part_env);

    part.set_resource_mask(1);

    let sess_ctx = begin_and_continue_open_session(&part);

    let table_memory_pre_open_session = table_memory;

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
    assert!(result.is_ok());

    let app_sess = result.unwrap();

    let table_memory_pre_close_session = table_memory;

    let result = part.begin_close_user_session(0, PcieFunction::Vf0, app_sess.id());
    assert!(result.is_err());

    assert_ne!(table_memory, table_memory_pre_close_session);
    assert_ne!(table_memory, table_memory_pre_open_session);
}

#[test]
fn test_close_app_session_err() {
    let part = partition_with_vault_expectations();
    assert!(part.close_user_session(0).is_err());
}

#[test]
fn test_app_session() {
    let part = partition_with_open_sessions_expectations(
        1,
        ExpectedParams {
            aes_encrypt_decrypt: 3,
            hal_aes: 3,
            hal_rng: 3,
            hal_bks_table: 2,
            heap_allocate: 19,
            rng_32_bytes: 2,
            rng_48_bytes: 1,
            ..Default::default()
        },
    );
    part.set_resource_mask(1);

    let sess_ctx = begin_and_continue_open_session(&part);

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
    assert!(result.is_ok());

    let app_sess = result.unwrap();

    assert!(part.user_session(app_sess.id(), false).is_ok());
}

#[test]
fn test_reset_partition() {
    const VAULT_SIZE_DWORDS: usize = 1024 * 17 * 65 / 4;
    let vault: [u32; VAULT_SIZE_DWORDS] = [0; VAULT_SIZE_DWORDS];
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(2).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().times(2).return_const(rng_nonce);

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .once()
        .return_const(vault.as_ptr() as usize);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .return_const(cdma_vault_meta_data.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_ipc_expectations(&mut hal);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    part.set_resource_mask(1);
    part.reset();
    assert_eq!(part.resource_mask(), 0);
}

#[test]
fn test_migrate_partition() {
    const VAULT_SIZE_DWORDS: usize = 1024 * 17 * 65 / 4;
    let vault: [u32; VAULT_SIZE_DWORDS] = [0; VAULT_SIZE_DWORDS];
    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(2).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().times(2).return_const(rng_nonce);

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr()
        .once()
        .return_const(vault.as_ptr() as usize);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .return_const(cdma_vault_meta_data.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_ipc_expectations(&mut hal);

    let persistent_store: &'static mut [HsmPartPersistentStore] = mcr_mem_map::mem_addr_to_slice(
        part_persistent_store_memory.as_ptr() as usize,
        MAX_PCIE_FUNCTIONS,
    );

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    part.set_resource_mask(1);

    persistent_store[0].session_table[0] = 0xA5;
    assert_eq!(persistent_store[0].session_table[1], 0x00);

    part.begin_migrate(None);
    part.end_migrate();
    assert_eq!(part.resource_mask(), 1);

    assert_eq!(persistent_store[0].session_table[1], 0xA5);
}

#[test]
fn test_new_with_resource_table() {
    let mut part_store = HsmPartDataStore::default();
    let mut resource_table = vec![Resource::default(); 65].into_boxed_slice();
    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().once().return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    let mut pka = MockPka::new();

    // populate the partition data for PcieFunction(0)
    part_store.part[0].enabled = true;
    part_store.part[0].user_cred = UserCredential {
        id: [4; 16],
        pin: [9; 16],
        vault_id: 45,
    };

    for resource in resource_table.iter_mut() {
        *resource = Resource::default()
    }

    // Assign resource 0 and 1 to the PcieFunction(0)
    resource_table[0].pfn = Some(PcieFunction(0));
    resource_table[0].alloc_map = 0x3F;
    resource_table[0].id = 0;
    resource_table[1].pfn = Some(PcieFunction(0));
    resource_table[1].alloc_map = 0x3F;
    resource_table[1].id = 1;

    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_resource_table()
        .once()
        .return_const(resource_table.to_vec());
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_ipc_expectations(&mut hal);

    let part = Partition::<MockEnv>::new_with_resource_table(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    assert_eq!(part.resource_mask(), 0x3);

    // WarmReset for an allocated PFN must re-arm Gate 1 (offset 22) without a replayed `SetRes`.
    assert_eq!(part_persistent_store_memory[22], 1);
}

#[test]
fn test_new_with_resource_table_unallocated_pfn_not_armed() {
    let mut resource_table = vec![Resource::default(); 65].into_boxed_slice();
    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().once().return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    let mut pka = MockPka::new();

    // Empty resource table for PcieFunction(0): restored mask is 0 (idle slot, not armed).
    for resource in resource_table.iter_mut() {
        *resource = Resource::default()
    }

    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_resource_table()
        .once()
        .return_const(resource_table.to_vec());
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });
    set_ipc_expectations(&mut hal);

    let part = Partition::<MockEnv>::new_with_resource_table(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    // mask == 0 -> the PFN is not armed: an idle box stays drain-free.
    assert_eq!(part.resource_mask(), 0);
    assert_eq!(part_persistent_store_memory[22], 0);
}

#[test]
fn test_restore() {
    let mut part_store = HsmPartDataStore::default();
    let mut resource_table = vec![Resource::default(); 65].into_boxed_slice();
    let mut hal = MockHal::new();
    let mut pka = MockPka::new();

    // populate the partition data for PcieFunction(0)
    part_store.part[0].enabled = true;
    part_store.part[0].user_cred = UserCredential {
        id: [4; 16],
        pin: [9; 16],
        vault_id: 45,
    };

    for resource in resource_table.iter_mut() {
        *resource = Resource::default()
    }

    // Assign resource 0 and 1 to the PcieFunction(0)
    resource_table[0].pfn = Some(PcieFunction(0));
    resource_table[0].alloc_map = 0x3F;
    resource_table[0].id = 0;
    resource_table[1].pfn = Some(PcieFunction(0));
    resource_table[1].alloc_map = 0x3F;
    resource_table[1].id = 1;

    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_partition_data_store_addr()
        .times(2)
        .return_const(&part_store as *const HsmPartDataStore as usize);
    hal.expect_resource_table()
        .times(2)
        .return_const(resource_table.to_vec());
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

    let part = Partition::<MockEnv>::restore(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    assert_eq!(part.resource_mask(), 0x3);
}

#[test]
fn test_restore_on_disabled_function() {
    let mut part_store = HsmPartDataStore::default();
    let mut resource_table = vec![Resource::default(); 65].into_boxed_slice();
    let mut hal = MockHal::new();
    let mut pka = MockPka::new();

    // populate the partition data for PcieFunction(0)
    part_store.part[0].enabled = false;
    for resource in resource_table.iter_mut() {
        *resource = Resource::default()
    }

    // Assign resource 0 and 1 to the PcieFunction(0)
    resource_table[0].pfn = Some(PcieFunction(0));
    resource_table[0].alloc_map = 0x3F;
    resource_table[0].id = 0;
    resource_table[1].pfn = Some(PcieFunction(0));
    resource_table[1].alloc_map = 0x3F;
    resource_table[1].id = 1;

    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_partition_data_store_addr()
        .times(2)
        .return_const(&part_store as *const HsmPartDataStore as usize);
    hal.expect_resource_table()
        .times(1)
        .return_const(resource_table.to_vec());
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

    let part = Partition::<MockEnv>::restore(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    assert_eq!(part.resource_mask(), 0x3);
}

#[test]
fn test_restore_with_one_app_cred() {
    let mut data_store = HsmPartDataStore::default();
    let mut resource_table = vec![Resource::default(); 65].into_boxed_slice();
    let mut hal = MockHal::new();
    let mut pka = MockPka::new();

    // populate the partition data for PcieFunction(0)
    data_store.part[0].enabled = true;
    data_store.part[0].user_cred = UserCredential {
        id: [4; 16],
        pin: [9; 16],
        vault_id: 45,
    };

    for resource in resource_table.iter_mut() {
        *resource = Resource::default()
    }

    // Assign resource 0 and 1 to the PcieFunction(0)
    resource_table[0].pfn = Some(PcieFunction(0));
    resource_table[0].alloc_map = 0x3F;
    resource_table[0].id = 0;
    resource_table[1].pfn = Some(PcieFunction(0));
    resource_table[1].alloc_map = 0x3F;
    resource_table[1].id = 1;

    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_partition_data_store_addr()
        .times(2)
        .return_const(&data_store as *const HsmPartDataStore as usize);
    hal.expect_resource_table()
        .times(2)
        .return_const(resource_table.to_vec());
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

    let part = Partition::<MockEnv>::restore(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    assert_eq!(part.resource_mask(), 0x3);
}

#[test]
fn test_restore_multiple_app_creds() {
    let mut data_store = HsmPartDataStore::default();
    let mut resource_table = vec![Resource::default(); 65].into_boxed_slice();
    let mut hal = MockHal::new();
    let mut pka = MockPka::new();

    // populate the partition data for PcieFunction(0)
    data_store.part[0].enabled = true;
    data_store.part[0].user_cred = UserCredential {
        id: [4; 16],
        pin: [9; 16],
        vault_id: 45,
    };

    for resource in resource_table.iter_mut() {
        *resource = Resource::default()
    }

    // Assign resource id 0 and 1 to the PcieFunction(0)
    resource_table[0].pfn = Some(PcieFunction(0));
    resource_table[0].alloc_map = 0x3F;
    resource_table[0].id = 0;
    resource_table[1].pfn = Some(PcieFunction(0));
    resource_table[1].alloc_map = 0x3F;
    resource_table[1].id = 1;

    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_partition_data_store_addr()
        .times(2)
        .return_const(&data_store as *const HsmPartDataStore as usize);
    hal.expect_resource_table()
        .times(2)
        .return_const(resource_table.to_vec());
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

    let part = Partition::<MockEnv>::restore(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    assert_eq!(part.resource_mask(), 0x3);
}

#[test]
fn test_restore_with_one_app_and_mgr_session() {
    let mut data_store = HsmPartDataStore::default();
    let mut resource_table = vec![Resource::default(); 65].into_boxed_slice();
    let mut hal = MockHal::new();
    let mut pka = MockPka::new();

    // populate the partition data for PcieFunction(0)
    data_store.part[0].enabled = true;
    data_store.part[0].user_cred = UserCredential {
        id: [4; 16],
        pin: [9; 16],
        vault_id: 45,
    };

    for resource in resource_table.iter_mut() {
        *resource = Resource::default()
    }

    // Assign resource 0 and 1 to the PcieFunction(0)
    resource_table[0].pfn = Some(PcieFunction(0));
    resource_table[0].alloc_map = 0x3F;
    resource_table[0].id = 0;
    resource_table[1].pfn = Some(PcieFunction(0));
    resource_table[1].alloc_map = 0x3F;
    resource_table[1].id = 1;

    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_partition_data_store_addr()
        .times(2)
        .return_const(&data_store as *const HsmPartDataStore as usize);
    hal.expect_resource_table()
        .times(2)
        .return_const(resource_table.to_vec());
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

    let part = Partition::<MockEnv>::restore(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    assert_eq!(part.resource_mask(), 0x3);
}

#[test]
fn test_store_data_with_one_app_and_mgr_session() {
    let mut data_store = HsmPartDataStore::default();
    let mut resource_table = vec![Resource::default(); 65].into_boxed_slice();
    let mut hal = MockHal::new();
    let mut pka = MockPka::new();

    // populate the partition data for PcieFunction(0)
    data_store.part[0].enabled = true;
    data_store.part[0].user_cred = UserCredential {
        id: [4; 16],
        pin: [9; 16],
        vault_id: 45,
    };

    for resource in resource_table.iter_mut() {
        *resource = Resource::default()
    }

    // Assign resource 0 and 1 to the PcieFunction(0)
    resource_table[0].pfn = Some(PcieFunction(0));
    resource_table[0].alloc_map = 0x3F;
    resource_table[0].id = 0;
    resource_table[1].pfn = Some(PcieFunction(0));
    resource_table[1].alloc_map = 0x3F;
    resource_table[1].id = 1;

    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_partition_data_store_addr()
        .times(3)
        .return_const(&data_store as *const HsmPartDataStore as usize);
    hal.expect_resource_table()
        .times(2)
        .return_const(resource_table.to_vec());
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

    let part = Partition::<MockEnv>::restore(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    data_store = HsmPartDataStore::default();

    part.store_data();

    assert_eq!(part.resource_mask(), 0x3);

    assert!(data_store.part[0].enabled);
}

#[test]
fn test_store_data_with_no_sessions_no_app_creds() {
    let mut data_store = HsmPartDataStore::default();
    let mut resource_table = vec![Resource::default(); 65].into_boxed_slice();
    let mut hal = MockHal::new();
    let mut pka = MockPka::new();

    // populate the partition data for PcieFunction(0)
    data_store.part[0].enabled = true;
    data_store.part[0].user_cred = UserCredential {
        id: [4; 16],
        pin: [9; 16],
        vault_id: 45,
    };

    for resource in resource_table.iter_mut() {
        *resource = Resource::default()
    }

    // Assign resource 0 and 1 to the PcieFunction(0)
    resource_table[0].pfn = Some(PcieFunction(0));
    resource_table[0].alloc_map = 0x3F;
    resource_table[0].id = 0;
    resource_table[1].pfn = Some(PcieFunction(0));
    resource_table[1].alloc_map = 0x3F;
    resource_table[1].id = 1;

    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_partition_data_store_addr()
        .times(3)
        .return_const(&data_store as *const HsmPartDataStore as usize);
    hal.expect_resource_table()
        .times(2)
        .return_const(resource_table.to_vec());
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

    let part = Partition::<MockEnv>::restore(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    data_store = HsmPartDataStore::default();

    part.store_data();

    assert_eq!(part.resource_mask(), 0x3);

    assert!(data_store.part[0].enabled);
}

#[test]
fn test_set_cert_chain_lengths_info() {
    let part = partition(true);
    assert!(part.state.get_cert_chain_lengths_info().is_none());
    let mut cert_chain_lengths_info = GetCertChainLengthsInfo {
        hash: [0; 32],
        num_certs: 4,
        cert_lengths: [0; MAX_CERTS],
    };
    cert_chain_lengths_info.cert_lengths[0] = 512;
    cert_chain_lengths_info.cert_lengths[1] = 1024;
    cert_chain_lengths_info.cert_lengths[2] = 2048;
    cert_chain_lengths_info.cert_lengths[3] = 4096;

    part.set_cert_chain_lengths_info(Some(cert_chain_lengths_info));
    assert!(part.state.get_cert_chain_lengths_info() == Some(cert_chain_lengths_info));

    part.set_cert_chain_lengths_info(None);
    assert!(part.state.get_cert_chain_lengths_info().is_none());
}

#[test]
fn test_get_cert_len() {
    let mut pfn_memory = [0u8; 2048];
    // Cast pfn_memory to HsmPartPersistentStore using unsafe pointer casting
    let store = unsafe { &mut *(pfn_memory.as_mut_ptr() as *mut HsmPartPersistentStore) };
    store.partition_cert.length = 512;

    assert_ne!(pfn_memory, [0u8; 2048]);

    let mut part_persistent_store_memory = [0u8; 2048 * 65];
    part_persistent_store_memory[..2048].copy_from_slice(&pfn_memory[..]);

    let part = partition_with_alias_key_len_expectation(&part_persistent_store_memory);

    assert!(part.state.get_cert_chain_lengths_info().is_none());
    let mut cert_chain_lengths_info = GetCertChainLengthsInfo {
        hash: [0; 32],
        num_certs: 4,
        cert_lengths: [0; MAX_CERTS],
    };
    for i in 0..4 {
        cert_chain_lengths_info.cert_lengths[i] = 512;
    }

    part.set_cert_chain_lengths_info(Some(cert_chain_lengths_info));
    for i in 0..4 {
        assert_eq!(part.get_cert_len(i), Some(512));
    }
}

#[test]
fn test_get_cert_len_no_cert_info() {
    let part = partition(true);
    assert!(part.state.get_cert_chain_lengths_info().is_none());
    assert_eq!(part.get_cert_len(0), None);
}

#[test]
fn test_get_cert_len_incorrect_id() {
    let mut pfn_memory = [0u8; 2048];
    // Cast pfn_memory to HsmPartPersistentStore using unsafe pointer casting
    let store = unsafe { &mut *(pfn_memory.as_mut_ptr() as *mut HsmPartPersistentStore) };
    store.partition_cert.length = 512;

    assert_ne!(pfn_memory, [0u8; 2048]);

    let mut part_persistent_store_memory = [0u8; 2048 * 65];
    part_persistent_store_memory[..2048].copy_from_slice(&pfn_memory[..]);

    let part = partition_with_alias_key_len_expectation(&part_persistent_store_memory);
    assert!(part.state.get_cert_chain_lengths_info().is_none());
    let mut cert_chain_lengths_info = GetCertChainLengthsInfo {
        hash: [0; 32],
        num_certs: 4,
        cert_lengths: [0; MAX_CERTS],
    };
    for i in 0..4 {
        cert_chain_lengths_info.cert_lengths[i] = 512;
    }

    part.set_cert_chain_lengths_info(Some(cert_chain_lengths_info));
    for i in 0..4 {
        assert_eq!(part.get_cert_len(i), Some(512));
    }

    assert_eq!(part.get_cert_len(5), None);
}

#[test]
fn test_ddihashalgorithm_to_hmachashalgorithm() {
    let hash_algo = DdiHashAlgorithm::Sha1;
    let hmac_algo: HmacHashAlgorithm = hash_algo
        .try_into()
        .expect("Failed to convert DdiHashAlgorithm to HmacHashAlgorithm");
    assert!(hmac_algo == HmacHashAlgorithm::Sha1);

    let hash_algo = DdiHashAlgorithm::Sha256;
    let hmac_algo: HmacHashAlgorithm = hash_algo
        .try_into()
        .expect("Failed to convert DdiHashAlgorithm to HmacHashAlgorithm");
    assert!(hmac_algo == HmacHashAlgorithm::Sha256);

    let hash_algo = DdiHashAlgorithm::Sha384;
    let hmac_algo: HmacHashAlgorithm = hash_algo
        .try_into()
        .expect("Failed to convert DdiHashAlgorithm to HmacHashAlgorithm");
    assert!(hmac_algo == HmacHashAlgorithm::Sha384);

    let hash_algo = DdiHashAlgorithm::Sha512;
    let hmac_algo: HmacHashAlgorithm = hash_algo
        .try_into()
        .expect("Failed to convert DdiHashAlgorithm to HmacHashAlgorithm");
    assert!(hmac_algo == HmacHashAlgorithm::Sha512);
}

#[test]
fn test_hmachashalgorithm_to_shamode() {
    assert!(Into::<ShaMode>::into(HmacHashAlgorithm::Sha1) == ShaMode::Sha1);
    assert!(Into::<ShaMode>::into(HmacHashAlgorithm::Sha256) == ShaMode::Sha256);
    assert!(Into::<ShaMode>::into(HmacHashAlgorithm::Sha384) == ShaMode::Sha384);
    assert!(Into::<ShaMode>::into(HmacHashAlgorithm::Sha512) == ShaMode::Sha512);
}

#[test]
fn test_rollback_open_session() {
    let part = partition_with_open_sessions_expectations(
        1,
        ExpectedParams {
            aes_encrypt_decrypt: 3,
            hal_aes: 3,
            hal_rng: 3,
            hal_bks_table: 2,
            heap_allocate: 19,
            rng_32_bytes: 2,
            rng_48_bytes: 1,
            ..Default::default()
        },
    );
    part.set_resource_mask(1);

    let sess_ctx = begin_and_continue_open_session(&part);

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
    assert!(result.is_ok());

    let app_sess = result.unwrap();

    assert!(part.user_session(app_sess.id(), false).is_ok());

    let result = part.rollback_open_session(app_sess.id(), false);
    assert!(result.is_ok());
}

#[test]
fn test_get_establish_cred_encryption_key() {
    const VAULT_SIZE_DWORDS: usize = 1024 * 17 * 65 / 4;
    let vault: [u32; VAULT_SIZE_DWORDS] = [0; VAULT_SIZE_DWORDS];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(move || {
        let mut pka = MockPka::new();

        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(move |_tag, curve| Ok(PkaEccCmd { curve }));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(move |_tag, op| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [1; PkaEccCurve::MAX_LEN],
                        curve: op.curve,
                    },
                    pub_key: PkaEccPublicKey {
                        data: [2; PkaEccCurve::MAX_LEN * 2],
                        curve: op.curve,
                    },
                })
            });

        pka
    });

    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);
    hal.expect_rng().times(1).return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_vault_addr()
        .once()
        .return_const(vault.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );
    part.set_resource_mask(1);

    let result = part.begin_get_establish_cred_encryption_key(TagId::default());
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let result = part.end_get_establish_cred_encryption_key(TagId::default(), ctx);
    assert!(result.is_ok());
}

#[test]
fn test_get_establish_cred_encryption_key_err() {
    const VAULT_SIZE_DWORDS: usize = 1024 * 17 * 65 / 4;
    let vault: [u32; VAULT_SIZE_DWORDS] = [0; VAULT_SIZE_DWORDS];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(move || {
        let mut pka = MockPka::new();

        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(move |_tag, curve| Ok(PkaEccCmd { curve }));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(move |_tag, op| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [1; PkaEccCurve::MAX_LEN],
                        curve: op.curve,
                    },
                    pub_key: PkaEccPublicKey {
                        data: [2; PkaEccCurve::MAX_LEN * 2],
                        curve: op.curve,
                    },
                })
            });

        pka
    });

    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);
    hal.expect_rng().times(1).return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_vault_addr()
        .once()
        .return_const(vault.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let result = part.begin_get_establish_cred_encryption_key(TagId::default());
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let result = part.end_get_establish_cred_encryption_key(TagId::default(), ctx);
    assert!(result.is_err());
}

#[test]
fn test_get_session_encryption_key() {
    const VAULT_SIZE_DWORDS: usize = 1024 * 17 * 65 / 4;
    let vault: [u32; VAULT_SIZE_DWORDS] = [0; VAULT_SIZE_DWORDS];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(move || {
        let mut pka = MockPka::new();

        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(move |_tag, curve| Ok(PkaEccCmd { curve }));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(move |_tag, op| {
                Ok(PkaEccKeyPair {
                    priv_key: PkaEccPrivateKey {
                        k: [1; PkaEccCurve::MAX_LEN],
                        curve: op.curve,
                    },
                    pub_key: PkaEccPublicKey {
                        data: [2; PkaEccCurve::MAX_LEN * 2],
                        curve: op.curve,
                    },
                })
            });

        pka
    });

    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);
    hal.expect_rng().times(1).return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_vault_addr()
        .once()
        .return_const(vault.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );
    part.set_resource_mask(1);
    let result = part.state.change_user_cred(&[1; 16], &[1; 16]);
    assert!(result.is_ok());

    let result = part.begin_get_session_encryption_key(TagId::default());
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let result = part.end_get_session_encryption_key(TagId::default(), ctx);
    assert!(result.is_ok());
}

#[test]
fn test_get_session_encryption_key_err() {
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(move || {
        let mut pka = MockPka::new();

        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(move |_tag, curve| Ok(PkaEccCmd { curve }));
        pka.expect_end_ecc_gen_key()
            .times(1)
            .returning(|_, _| Err(HsmErr::InvalidArgument.into()));

        pka
    });

    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);
    hal.expect_rng().times(1).return_const(rng_nonce);
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

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );
    part.set_resource_mask(1);
    let result = part.state.change_user_cred(&[1; 16], &[1; 16]);
    assert!(result.is_ok());

    let result = part.begin_get_session_encryption_key(TagId::default());
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let result = part.end_get_session_encryption_key(TagId::default(), ctx);
    assert!(result.is_err());
}

#[test]
fn test_establish_credential() {
    const VAULT_SIZE_DWORDS: usize = 1024 * 17 * 65 / 4;
    let vault: [u32; VAULT_SIZE_DWORDS] = [0; VAULT_SIZE_DWORDS];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(move || {
        let mut pka = MockPka::new();

        pka.expect_begin_montgomery_constant_calculation()
            .times(2)
            .returning(|_tag, _curve| Ok(()));
        pka.expect_end_montgomery_constant_calculation()
            .times(2)
            .returning(|_tag| Ok(()));

        pka.expect_begin_ecc_point_validation_zc()
            .times(2)
            .returning(move |_tag, _curve, _pubkey| Ok(()));
        pka.expect_end_ecc_point_validation_zc()
            .times(2)
            .returning(|_tag| Ok(true));

        pka.expect_begin_ecc_verify_zc()
            .once()
            .returning(move |_, _, _, _, _| Ok(()));

        pka.expect_end_ecc_verify_zc()
            .once()
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

    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(2).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate().times(15).returning(|s| {
        let mut alloc = MockDmaAlloc::new(s);
        // Return app pin
        alloc.as_ref_mut().fill(TEST_APP_PIN_VAL);
        Some(alloc)
    });

    let mut sha = MockSha::new();
    sha.expect_digest_zc().once().returning(move |_| Ok(()));
    sha.expect_hmac()
        .returning(move |_, _, _, _, out_buf: &mut IoMemRange| {
            let output_ptr = out_buf.addr();
            let mut_output_ptr = output_ptr as *mut u8;
            let out_slice =
                unsafe { std::slice::from_raw_parts_mut(mut_output_ptr, out_buf.len()) };

            out_slice.fill(TEST_DIGEST_VAL);

            Ok(())
        });

    let mut aes = MockAes::new();
    aes.expect_encrypt_decrypt().times(2).returning(|_| Ok(()));

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);
    hal.expect_rng().times(2).return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_vault_addr()
        .times(3)
        .return_const(vault.as_ptr() as usize);
    hal.expect_dma_heap().return_const(heap);
    hal.expect_sha().return_const(sha);
    hal.expect_aes().times(2).return_const(aes);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );
    part.set_resource_mask(1);

    let establish_cred_encryption_key_data = [0u8; EstablishCredEncryptionKeyKind::Ecc384 as usize];

    // Create a key to import
    let key_imported = EstablishCredEncryptionKeyToImport::new(
        EstablishCredEncryptionKeyKind::Ecc384,
        EstablishCredEncryptionKeyUsage::KeyAgreement,
        &establish_cred_encryption_key_data,
    )
    .unwrap();

    // Import the key into the vault
    let key = part
        .state
        .vault()
        .import_establish_cred_encryption_key(
            APP_VAULT_ID_FOR_INTERNAL_KEYS,
            &key_imported,
            KeyAvailability::App,
        )
        .unwrap();

    // Add the key_id to partition
    part.state
        .set_establish_cred_encryption_key_id(Some(key.id()));

    let raw_pub_key = IoMemRange::from(TEST_RAW_ECC_384_PUBLIC_KEY.as_slice());
    let raw_pota_pub_key = IoMemRange::from(TEST_RAW_ECC_384_PUBLIC_KEY.as_slice());
    let pota_sig = IoMemRange::from([2; 96].as_slice());

    let result = part.begin_establish_credential(TagId::default(), &raw_pub_key, &raw_pota_pub_key);
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let result =
        part.continue_establish_credential(ctx, &raw_pub_key, &raw_pota_pub_key, &pota_sig);
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let result =
        part.continue_establish_credential(ctx, &raw_pub_key, &raw_pota_pub_key, &pota_sig);
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let result =
        part.continue_establish_credential(ctx, &raw_pub_key, &raw_pota_pub_key, &pota_sig);
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let result =
        part.continue_establish_credential(ctx, &raw_pub_key, &raw_pota_pub_key, &pota_sig);
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let result =
        part.continue_establish_credential(ctx, &raw_pub_key, &raw_pota_pub_key, &pota_sig);
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let encrypted_id_buf = [1u8; 16];
    let encrypted_pin_buf = [1u8; 16];
    let iv_buf = [1u8; 16];

    let encrypted_cred = DdiEncryptedEstablishCredential {
        encrypted_id: MborByteArray::new_with_len(encrypted_id_buf.as_ptr(), 16),
        encrypted_pin: MborByteArray::new_with_len(encrypted_pin_buf.as_ptr(), 16),
        iv: MborByteArray::new_with_len(iv_buf.as_ptr(), 16),
        nonce: (1..33u8).collect::<Vec<_>>().try_into().unwrap(),
        tag: [TEST_DIGEST_VAL; 48],
    };
    let result = part.end_establish_credential(ctx, &encrypted_cred);
    assert!(result.is_ok());
}

#[test]
fn test_establish_credential_null_pin() {
    const VAULT_SIZE_DWORDS: usize = 1024 * 17 * 65 / 4;
    let vault: [u32; VAULT_SIZE_DWORDS] = [0; VAULT_SIZE_DWORDS];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(move || {
        let mut pka = MockPka::new();

        pka.expect_begin_montgomery_constant_calculation()
            .times(2)
            .returning(|_tag, _curve| Ok(()));
        pka.expect_end_montgomery_constant_calculation()
            .times(2)
            .returning(|_tag| Ok(()));

        pka.expect_begin_ecc_point_validation_zc()
            .times(2)
            .returning(move |_tag, _curve, _pubkey| Ok(()));
        pka.expect_end_ecc_point_validation_zc()
            .times(2)
            .returning(|_tag| Ok(true));

        pka.expect_begin_ecc_verify_zc()
            .once()
            .returning(move |_, _, _, _, _| Ok(()));

        pka.expect_end_ecc_verify_zc()
            .once()
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

    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate().times(9).returning(|s| {
        let mut alloc = MockDmaAlloc::new(s);
        // Return app pin
        alloc.as_ref_mut().fill(TEST_APP_PIN_VAL);
        Some(alloc)
    });

    let mut sha = MockSha::new();
    sha.expect_digest_zc().once().returning(move |_| Ok(()));
    sha.expect_hmac()
        .returning(move |_, _, _, _, out_buf: &mut IoMemRange| {
            let output_ptr = out_buf.addr();
            let mut_output_ptr = output_ptr as *mut u8;
            let out_slice =
                unsafe { std::slice::from_raw_parts_mut(mut_output_ptr, out_buf.len()) };

            out_slice.fill(TEST_DIGEST_VAL);

            Ok(())
        });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);
    hal.expect_rng().times(1).return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_vault_addr()
        .times(2)
        .return_const(vault.as_ptr() as usize);
    hal.expect_dma_heap().return_const(heap);
    hal.expect_sha().return_const(sha);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );
    part.set_resource_mask(1);

    let establish_cred_encryption_key_data = [0u8; EstablishCredEncryptionKeyKind::Ecc384 as usize];

    // Create a key to import
    let key_imported = EstablishCredEncryptionKeyToImport::new(
        EstablishCredEncryptionKeyKind::Ecc384,
        EstablishCredEncryptionKeyUsage::KeyAgreement,
        &establish_cred_encryption_key_data,
    )
    .unwrap();

    // Import the key into the vault
    let key = part
        .state
        .vault()
        .import_establish_cred_encryption_key(
            APP_VAULT_ID_FOR_INTERNAL_KEYS,
            &key_imported,
            KeyAvailability::App,
        )
        .unwrap();

    // Add the key_id to partition
    part.state
        .set_establish_cred_encryption_key_id(Some(key.id()));

    let raw_pub_key = IoMemRange::from(TEST_RAW_ECC_384_PUBLIC_KEY.as_slice());
    let raw_pota_pub_key = IoMemRange::from(TEST_RAW_ECC_384_PUBLIC_KEY.as_slice());
    let pota_sig = IoMemRange::from([2; 96].as_slice());

    let result = part.begin_establish_credential(TagId::default(), &raw_pub_key, &raw_pota_pub_key);
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let result =
        part.continue_establish_credential(ctx, &raw_pub_key, &raw_pota_pub_key, &pota_sig);
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let result =
        part.continue_establish_credential(ctx, &raw_pub_key, &raw_pota_pub_key, &pota_sig);
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let result =
        part.continue_establish_credential(ctx, &raw_pub_key, &raw_pota_pub_key, &pota_sig);
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let result =
        part.continue_establish_credential(ctx, &raw_pub_key, &raw_pota_pub_key, &pota_sig);
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let result =
        part.continue_establish_credential(ctx, &raw_pub_key, &raw_pota_pub_key, &pota_sig);
    assert!(result.is_ok());
    let ctx = result.unwrap();

    let encrypted_id_buf = [1u8; 16];
    let encrypted_pin_buf = [1u8; 16];
    let iv_buf = [1u8; 16];

    let encrypted_cred = DdiEncryptedEstablishCredential {
        encrypted_id: MborByteArray::new_with_len(encrypted_id_buf.as_ptr(), 16),
        encrypted_pin: MborByteArray::new_with_len(encrypted_pin_buf.as_ptr(), 0),
        iv: MborByteArray::new_with_len(iv_buf.as_ptr(), 16),
        nonce: (1..33u8).collect::<Vec<_>>().try_into().unwrap(),
        tag: [TEST_DIGEST_VAL; 48],
    };
    let result = part.end_establish_credential(ctx, &encrypted_cred);
    assert!(matches!(result, Err(HsmErr::InvalidArgument)));
}

#[test]
fn test_ecc_pct_validation() {
    const VAULT_SIZE_DWORDS: usize = 1024 * 17 * 65 / 4;
    let vault: [u32; VAULT_SIZE_DWORDS] = [0; VAULT_SIZE_DWORDS];

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(move || {
        let mut pka = MockPka::new();

        pka.expect_begin_montgomery_constant_calculation()
            .times(2)
            .returning(|_tag, _curve| Ok(()));
        pka.expect_peek_tag()
            .times(4)
            .returning(|| Some(TagId::default()));
        pka.expect_end_montgomery_constant_calculation()
            .times(2)
            .returning(|_tag| Ok(()));

        pka.expect_begin_ecdh_compute_zc().times(2).returning(
            move |_tag, _curve, _privkey, _pubkey| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc384,
                })
            },
        );
        pka.expect_end_ecdh_compute()
            .times(2)
            .returning(move |_tag, _op| {
                Ok(PkaEccSecretValue {
                    curve: PkaEccCurve::Ecc384,
                    secret: [0; PkaEccCurve::MAX_LEN],
                })
            });

        pka
    });

    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut sha = MockSha::new();
    sha.expect_clone().times(1).returning(|| {
        let mut sha = MockSha::new();

        sha.expect_hmac()
            .returning(move |_, _, _, _, out_buf: &mut IoMemRange| {
                let output_ptr = out_buf.addr();
                let mut_output_ptr = output_ptr as *mut u8;
                let out_slice =
                    unsafe { std::slice::from_raw_parts_mut(mut_output_ptr, out_buf.len()) };

                out_slice.fill(TEST_DIGEST_VAL);

                Ok(())
            });

        sha
    });
    sha.expect_hmac()
        .returning(move |_, _, _, _, out_buf: &mut IoMemRange| {
            let output_ptr = out_buf.addr();
            let mut_output_ptr = output_ptr as *mut u8;
            let out_slice =
                unsafe { std::slice::from_raw_parts_mut(mut_output_ptr, out_buf.len()) };

            out_slice.fill(TEST_DIGEST_VAL);

            Ok(())
        });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);
    hal.expect_rng().times(1).return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_vault_addr()
        .times(2)
        .return_const(vault.as_ptr() as usize);
    hal.expect_sha().return_const(sha);
    hal.expect_dma_heap().return_const(heap);
    hal.expect_clone().times(1).returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );
    part.set_resource_mask(1);

    let establish_cred_encryption_key_data =
        [0xffu8; EstablishCredEncryptionKeyKind::Ecc384 as usize];

    // Create a key to import
    let key_imported = EstablishCredEncryptionKeyToImport::new(
        EstablishCredEncryptionKeyKind::Ecc384,
        EstablishCredEncryptionKeyUsage::KeyAgreement,
        &establish_cred_encryption_key_data,
    )
    .unwrap();

    // Import the key into the vault
    let imported_key = part
        .state
        .vault()
        .import_establish_cred_encryption_key(
            APP_VAULT_ID_FOR_INTERNAL_KEYS,
            &key_imported,
            KeyAvailability::App,
        )
        .unwrap();

    let pub_key = PkaEccPublicKey {
        data: [0xffu8; PkaEccCurve::MAX_LEN * 2],
        curve: PkaEccCurve::Ecc384,
    };
    let usage = EccKeyUsage::KeyAgreement;
    let tag = TagId::default();

    let pct_res = part.begin_ecc_pct_validation(tag, imported_key.id(), usage, pub_key);
    assert!(pct_res.is_ok());

    let mut ecc_key_pct = pct_res.unwrap();

    assert!(ecc_key_pct.pct_state == EccPctValidationState::EcdhMontgomeryConstCalculationFirst);
    assert!(!part.is_pct_final_state(&ecc_key_pct));

    let result = part.continue_ecc_pct_validation(tag, &mut ecc_key_pct);
    assert!(result.is_ok());

    assert!(ecc_key_pct.pct_state == EccPctValidationState::EcdhComputeFirst);
    assert!(!part.is_pct_final_state(&ecc_key_pct));

    let result = part.continue_ecc_pct_validation(tag, &mut ecc_key_pct);
    assert!(result.is_ok());

    assert!(ecc_key_pct.pct_state == EccPctValidationState::EcdhMontgomeryConstCalculationSecond);
    assert!(!part.is_pct_final_state(&ecc_key_pct));

    let result = part.continue_ecc_pct_validation(tag, &mut ecc_key_pct);
    assert!(result.is_ok());

    assert!(ecc_key_pct.pct_state == EccPctValidationState::EcdhComputeSecond);
    assert!(part.is_pct_final_state(&ecc_key_pct));

    let result = part.end_ecc_pct_validation(tag, &mut ecc_key_pct);
    assert!(result.is_ok());

    assert!(ecc_key_pct.pct_state == EccPctValidationState::ValidationComplete);
}

#[test]
fn test_delete_internal_key() {
    const KEY_TAG_UNASSIGNED: u16 = 0;

    const VAULT_SIZE_DWORDS: usize = 1024 * 17 * 65 / 4;
    let vault: [u32; VAULT_SIZE_DWORDS] = [0; VAULT_SIZE_DWORDS];

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(MockPka::new);

    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let mut sha = MockSha::new();
    sha.expect_hmac()
        .returning(move |_, _, _, _, out_buf: &mut IoMemRange| {
            let output_ptr = out_buf.addr();
            let mut_output_ptr = output_ptr as *mut u8;
            let out_slice =
                unsafe { std::slice::from_raw_parts_mut(mut_output_ptr, out_buf.len()) };

            out_slice.fill(TEST_DIGEST_VAL);

            Ok(())
        });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);
    hal.expect_rng().times(1).return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_vault_addr()
        .times(4)
        .return_const(vault.as_ptr() as usize);
    hal.expect_sha().return_const(sha);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );
    part.set_resource_mask(1);

    let establish_cred_encryption_key_data = [0u8; EstablishCredEncryptionKeyKind::Ecc384 as usize];

    // Create a key to import
    let key_imported = EstablishCredEncryptionKeyToImport::new(
        EstablishCredEncryptionKeyKind::Ecc384,
        EstablishCredEncryptionKeyUsage::KeyAgreement,
        &establish_cred_encryption_key_data,
    )
    .unwrap();

    // Import the key into the vault
    let imported_key = part
        .state
        .vault()
        .import_establish_cred_encryption_key(
            APP_VAULT_ID_FOR_INTERNAL_KEYS,
            &key_imported,
            KeyAvailability::App,
        )
        .unwrap();

    let key_id = imported_key.id();

    let key = part
        .state
        .vault()
        .key(
            APP_VAULT_ID_FOR_INTERNAL_KEYS,
            KEY_TAG_UNASSIGNED,
            key_id,
            false,
        )
        .unwrap();

    assert_eq!(key_id, key.id());

    part.delete_internal_key(key_id).unwrap();

    let result = part.state.vault().key(
        APP_VAULT_ID_FOR_INTERNAL_KEYS,
        KEY_TAG_UNASSIGNED,
        key_id,
        false,
    );

    assert!(result.is_err());
}

#[test]
fn test_unset_establish_cred_encryption_key() {
    let part = partition(true);

    part.state.set_establish_cred_encryption_key_id(Some(1));
    assert_eq!(part.state.get_establish_cred_encryption_key_id(), Some(1));

    part.unset_establish_cred_encryption_key_id();
    assert_eq!(part.state.get_establish_cred_encryption_key_id(), None);
}

#[test]
fn test_unset_session_encryption_key() {
    let part = partition(true);

    part.state.set_session_encryption_key_id(Some(1));
    assert_eq!(part.state.get_session_encryption_key_id(), Some(1));

    part.unset_session_encryption_key_id();
    assert_eq!(part.state.get_session_encryption_key_id(), None);
}

#[test]
fn test_notify_pct_validation_failure() {
    let err = u32::from(HsmErr::PctValidationEccGenKeyFailed);

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(MockPka::new);

    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let mut sha = MockSha::new();
    sha.expect_hmac()
        .returning(move |_, _, _, _, out_buf: &mut IoMemRange| {
            let output_ptr = out_buf.addr();
            let mut_output_ptr = output_ptr as *mut u8;
            let out_slice =
                unsafe { std::slice::from_raw_parts_mut(mut_output_ptr, out_buf.len()) };

            out_slice.fill(TEST_DIGEST_VAL);

            Ok(())
        });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);
    hal.expect_rng().times(1).return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
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

    hal.expect_sha().return_const(sha);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    part.notify_pct_validation_failure(err);
}

#[test]
fn test_begin_generate_part_identifiers() {
    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(move || {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_gen_key()
            .times(1)
            .returning(move |_tag, curve| Ok(PkaEccCmd { curve }));
        pka
    });

    let mut rng = MockRng::new();
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    rng.expect_bytes().times(1).returning(|buf| {
        buf.copy_from_slice(&[1u8; 16usize]);
    });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);

    hal.expect_rng().times(2).return_const(rng);

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

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    assert!(part
        .begin_generate_partition_identifiers(TagId::default())
        .is_ok());
}

#[test]
fn test_begin_generate_pid_cert() {
    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(move || {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_sign_zc()
            .times(1)
            .returning(move |_, _, _, _, _| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::from(EccCurve::P384),
                })
            });
        pka
    });

    let mut rng = MockRng::new();
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().times(1).returning(|buf| {
        buf.copy_from_slice(&[1u8; 20usize]);
    });

    let mut sha = MockSha::new();
    sha.expect_digest_zc().once().returning(move |_| Ok(()));
    sha.expect_digest_zc().once().returning(move |_| Ok(()));

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);

    hal.expect_rng().times(2).return_const(rng);

    hal.expect_sha().return_const(sha);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    // heap.
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(5)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    hal.expect_dma_heap().return_const(heap);

    let mut pfn_memory = [0u8; 2048];
    // Cast pfn_memory to HsmPartPersistentStore using unsafe pointer casting
    let store = unsafe { &mut *(pfn_memory.as_mut_ptr() as *mut HsmPartPersistentStore) };
    store.partition_id_valid = true;
    store.partition_identifier.pub_key = PUB_KEY_RAW;

    let mut part_persistent_store_memory = [0u8; 2048 * 65];
    part_persistent_store_memory[..2048].copy_from_slice(&pfn_memory[..]);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    hal.expect_alias_cert_len()
        .times(1)
        .return_const(TEST_CERTIFICATE.len());

    hal.expect_alias_cert()
        .times(1)
        .return_const(TEST_CERTIFICATE.into());

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let raw_alias_key = [0u8; 48];

    assert!(part
        .begin_generate_pid_cert(TagId::default(), raw_alias_key.as_slice(),)
        .is_ok());
}

#[test]
fn test_partition_cert_addr_and_len() {
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);

    hal.expect_rng().times(1).return_const(rng);

    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let mut pfn_memory = [0u8; 2048];
    // Cast pfn_memory to HsmPartPersistentStore using unsafe pointer casting
    let store = unsafe { &mut *(pfn_memory.as_mut_ptr() as *mut HsmPartPersistentStore) };
    store.partition_id_valid = true;
    store.partition_identifier.pub_key = PUB_KEY_RAW;
    store.partition_cert_valid = true;
    let mut cert_data = [0u8; 800];
    cert_data[..TEST_CERTIFICATE.len()].copy_from_slice(&TEST_CERTIFICATE);
    store.partition_cert = PartitionCert {
        length: TEST_CERTIFICATE.len() as u32,
        data: cert_data,
    };

    let mut part_persistent_store_memory = [0u8; 2048 * 65];
    part_persistent_store_memory[..2048].copy_from_slice(&pfn_memory[..]);
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let addr_range = part.partition_cert();
    let expected_addr = part_persistent_store_memory.as_ptr() as usize
        + core::mem::offset_of!(HsmPartPersistentStore, partition_cert)
        + core::mem::offset_of!(PartitionCert, data);

    assert_eq!(addr_range.addr() as usize, expected_addr);
    assert_eq!(addr_range.len(), TEST_CERTIFICATE.len());
    assert_eq!(addr_range.slice(), TEST_CERTIFICATE);

    let cert_length = 100;
    assert!(part.set_partition_cert_length(cert_length).is_ok());
    assert_eq!(part.partition_cert_length(), cert_length);

    let cert_length = 1024;
    assert!(part.set_partition_cert_length(cert_length).is_err());
    assert_eq!(part.partition_cert_length(), 100);

    assert!(part.is_partition_cert_valid());
    part.set_partition_cert_valid(false);
    assert!(!part.is_partition_cert_valid());
}

#[test]
fn test_mask_bk3() {
    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().times(1).returning(|buf| {
        buf.copy_from_slice(&[2u8; 16usize]);
    });

    let mut sha = MockSha::new();
    sha.expect_hmac()
        .returning(move |_, _, _, _, out_buf: &mut IoMemRange| {
            let output_ptr = out_buf.addr();
            let mut_output_ptr = output_ptr as *mut u8;
            let out_slice =
                unsafe { std::slice::from_raw_parts_mut(mut_output_ptr, out_buf.len()) };

            out_slice.fill(TEST_DIGEST_VAL);

            Ok(())
        });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);

    hal.expect_rng().times(2).return_const(rng);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_sha().return_const(sha);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    // heap.
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    hal.expect_dma_heap().return_const(heap);

    const TEST_BKS_TABLE: [u8; 492] = [0x43; 492];
    hal.expect_bks_table_addr()
        .times(1)
        .return_const(TEST_BKS_TABLE.as_ptr() as usize);

    let mut aes = MockAes::new();
    aes.expect_encrypt_decrypt().times(1).returning(|_| Ok(()));
    hal.expect_aes().times(1).return_const(aes);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let mut output = [0u8; 512];
    let mut output_len = 512;

    assert!(part
        .mask_bk3(
            &TEST_BK3,
            &TEST_BK_BOOT_MASKING_KEY,
            &mut output_len,
            &mut output[..]
        )
        .is_ok());
}

#[test]
fn test_generate_bk_boot() {
    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().times(1).returning(|buf| {
        buf.copy_from_slice(&[1u8; 48usize]);
    });

    let mut sha = MockSha::new();
    sha.expect_kbkdf_counter_hmac()
        .times(..)
        .returning(move |_, _, _, output: &mut [u8]| {
            output.fill(TEST_DIGEST_VAL);

            Ok(())
        });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);

    hal.expect_rng().times(2).return_const(rng);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_sha().return_const(sha);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    hal.expect_dma_heap().return_const(heap);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let mut output = [0u8; 80];
    assert!(part.generate_bk_boot(&mut output[..]).is_ok());
}

#[test]
fn test_mask_bk_boot() {
    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().times(1).returning(|buf| {
        buf.copy_from_slice(&[2u8; 16usize]);
    });

    let mut sha = MockSha::new();
    sha.expect_kbkdf_counter_hmac()
        .times(..)
        .returning(move |_, _, _, output: &mut [u8]| {
            output.fill(TEST_DIGEST_VAL);

            Ok(())
        });

    sha.expect_hmac()
        .returning(move |_, _, _, _, out_buf: &mut IoMemRange| {
            let output_ptr = out_buf.addr();
            let mut_output_ptr = output_ptr as *mut u8;
            let out_slice =
                unsafe { std::slice::from_raw_parts_mut(mut_output_ptr, out_buf.len()) };

            out_slice.fill(TEST_DIGEST_VAL);

            Ok(())
        });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);

    hal.expect_rng().times(2).return_const(rng);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_sha().return_const(sha);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    // heap.
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    hal.expect_dma_heap().return_const(heap);

    const TEST_BKS_TABLE: [u8; 492] = [0x43; 492];
    hal.expect_bks_table_addr()
        .times(3)
        .return_const(TEST_BKS_TABLE.as_ptr() as usize);

    let mut aes = MockAes::new();
    aes.expect_encrypt_decrypt().times(1).returning(|_| Ok(()));
    hal.expect_aes().times(1).return_const(aes);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let bk_boot = [1u8; 80];
    let mut output = [0u8; 512];
    let mut output_len = 512;

    assert!(part
        .mask_bk_boot(&bk_boot, &mut output_len, &mut output[..])
        .is_ok());
}

#[test]
fn test_unmask_bk3() {
    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().times(2).returning(|buf| {
        buf.copy_from_slice(&[2u8; 16usize]);
    });

    let mut sha = MockSha::new();
    sha.expect_kbkdf_counter_hmac()
        .times(..)
        .returning(move |_, _, _, output: &mut [u8]| {
            output.fill(TEST_DIGEST_VAL);

            Ok(())
        });
    sha.expect_hmac()
        .returning(move |_, _, _, _, out_buf: &mut IoMemRange| {
            let output_ptr = out_buf.addr();
            let mut_output_ptr = output_ptr as *mut u8;
            let out_slice =
                unsafe { std::slice::from_raw_parts_mut(mut_output_ptr, out_buf.len()) };

            out_slice.fill(TEST_DIGEST_VAL);

            Ok(())
        });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);

    hal.expect_rng().times(3).return_const(rng);
    hal.expect_sha().return_const(sha);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 3072 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    // heap.
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    hal.expect_dma_heap().return_const(heap);

    const TEST_BKS_TABLE: [u8; 492] = [0x43; 492];
    hal.expect_bks_table_addr()
        .times(..)
        .return_const(TEST_BKS_TABLE.as_ptr() as usize);

    let mut aes = MockAes::new();
    aes.expect_encrypt_decrypt().times(..).returning(|_| Ok(()));
    hal.expect_aes().times(..).return_const(aes);

    let mut pfn_memory = [0u8; 3072];
    hal.expect_part_persistent_store_addr()
        .return_const(pfn_memory.as_ptr() as usize);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let mut output = [0u8; 512];
    let mut output_len = 512;
    assert!(part
        .mask_bk3(
            &TEST_BK3,
            &TEST_BK_BOOT_MASKING_KEY,
            &mut output_len,
            &mut output[..]
        )
        .is_ok());
    let masked_bk3 = &output[..output_len];

    let mut output = [0u8; 512];
    let mut output_len = 512;
    let bk_boot = [2u8; 80];
    assert!(part
        .mask_bk_boot(&bk_boot, &mut output_len, &mut output[..])
        .is_ok());

    let store = unsafe { &mut *(pfn_memory.as_mut_ptr() as *mut HsmPartPersistentStore) };
    store.masked_bk_boot.len = output_len as u32;
    store.masked_bk_boot.data[..output_len].copy_from_slice(&output[..output_len]);

    let mut unmasked_bk3 = [0u8; 48];
    assert!(part.unmask_bk3(masked_bk3, &mut unmasked_bk3).is_ok());
}

#[test]
fn test_generate_bk() {
    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let mut sha = MockSha::new();
    sha.expect_kbkdf_counter_hmac()
        .times(..)
        .returning(move |_, _, _, output: &mut [u8]| {
            output.fill(TEST_DIGEST_VAL);

            Ok(())
        });

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);

    hal.expect_rng().times(1).return_const(rng);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_sha().return_const(sha);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    hal.expect_dma_heap().return_const(heap);

    const TEST_BKS_TABLE: [u8; 492] = [0x43; 492];
    hal.expect_bks_table_addr()
        .times(2)
        .return_const(TEST_BKS_TABLE.as_ptr() as usize);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let mut output = [0u8; 80];
    assert!(
        part.generate_bk(&[1u8; 32], &TEST_POTA_ECC_PUB_KEY, &mut output[..])
            == Err(HsmErr::InvalidArgument)
    );
    assert!(part
        .generate_bk(&TEST_BK3, &TEST_POTA_ECC_PUB_KEY, &mut output[..])
        .is_ok());
}

#[test]
fn test_generate_new_mk_and_import() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().times(1).returning(|buf| {
        buf.copy_from_slice(&[1u8; 48usize]);
    });

    let mut sha = MockSha::new();
    sha.expect_kbkdf_counter_hmac()
        .times(..)
        .returning(move |_, _, _, output: &mut [u8]| {
            output.fill(TEST_DIGEST_VAL);

            Ok(())
        });

    let mut hal = MockHal::new();
    hal.expect_vault_addr()
        .once()
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);

    hal.expect_rng().times(2).return_const(rng);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_sha().return_const(sha);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    hal.expect_dma_heap().return_const(heap);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );
    part.set_resource_mask(1);

    assert!(!part.is_partition_provisioned());
    assert!(part.generate_new_mk_and_import().is_ok());
    assert!(part.is_partition_provisioned());
}

#[test]
fn test_generate_bmk() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().times(1).returning(|buf| {
        buf.copy_from_slice(&[1u8; 48usize]);
    });
    rng.expect_bytes().times(1).returning(|buf| {
        buf.copy_from_slice(&[2u8; 16usize]);
    });

    let mut sha = MockSha::new();
    sha.expect_hmac()
        .returning(move |_, _, _, _, out_buf: &mut IoMemRange| {
            let output_ptr = out_buf.addr();
            let mut_output_ptr = output_ptr as *mut u8;
            let out_slice =
                unsafe { std::slice::from_raw_parts_mut(mut_output_ptr, out_buf.len()) };

            out_slice.fill(TEST_DIGEST_VAL);

            Ok(())
        });
    sha.expect_kbkdf_counter_hmac()
        .times(..)
        .returning(move |_, _, _, output: &mut [u8]| {
            output.fill(TEST_DIGEST_VAL);

            Ok(())
        });

    let mut aes = MockAes::new();
    aes.expect_encrypt_decrypt().times(..).returning(|_| Ok(()));

    let mut hal = MockHal::new();
    hal.expect_vault_addr()
        .times(2)
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_aes().times(..).return_const(aes);
    set_ipc_expectations(&mut hal);

    hal.expect_rng().times(3).return_const(rng);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_sha().return_const(sha);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    hal.expect_dma_heap().return_const(heap);

    const TEST_BKS_TABLE: [u8; 492] = [0x43; 492];
    hal.expect_bks_table_addr()
        .times(3)
        .return_const(TEST_BKS_TABLE.as_ptr() as usize);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );
    part.set_resource_mask(1);

    // generate an MK.
    assert!(part.generate_new_mk_and_import().is_ok());

    // generate the BK.
    let mut bk = [0u8; 80];
    assert!(part
        .generate_bk(&TEST_BK3, &TEST_POTA_ECC_PUB_KEY, &mut bk[..])
        .is_ok());

    // now generate the BMK.
    let mut bmk_out = [0u8; 300];
    let mut bmk_len = 300;
    assert!(part
        .generate_bmk(&bk, &mut bmk_len, &mut bmk_out[..])
        .is_ok());
}

#[test]
fn test_import_mk_from_bmk() {
    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });
    rng.expect_bytes().times(1).returning(|buf| {
        buf.copy_from_slice(&[1u8; 48usize]);
    });
    rng.expect_bytes().times(1).returning(|buf| {
        buf.copy_from_slice(&[2u8; 16usize]);
    });

    let mut sha = MockSha::new();
    sha.expect_hmac()
        .returning(move |_, _, _, _, out_buf: &mut IoMemRange| {
            let output_ptr = out_buf.addr();
            let mut_output_ptr = output_ptr as *mut u8;
            let out_slice =
                unsafe { std::slice::from_raw_parts_mut(mut_output_ptr, out_buf.len()) };

            out_slice.fill(TEST_DIGEST_VAL);

            Ok(())
        });
    sha.expect_kbkdf_counter_hmac()
        .times(..)
        .returning(move |_, _, _, output: &mut [u8]| {
            output.fill(TEST_DIGEST_VAL);

            Ok(())
        });

    let mut aes = MockAes::new();
    aes.expect_encrypt_decrypt().times(..).returning(|_| Ok(()));

    let mut hal = MockHal::new();
    hal.expect_vault_addr()
        .times(3)
        .return_const(table_memory.as_ptr() as usize);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_aes().times(..).return_const(aes);
    set_ipc_expectations(&mut hal);

    hal.expect_rng().times(3).return_const(rng);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_sha().return_const(sha);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    hal.expect_dma_heap().return_const(heap);

    const TEST_BKS_TABLE: [u8; 492] = [0x43; 492];
    hal.expect_bks_table_addr()
        .times(5)
        .return_const(TEST_BKS_TABLE.as_ptr() as usize);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );
    part.set_resource_mask(1);

    // generate an MK.
    assert!(!part.is_partition_provisioned());
    assert!(part.generate_new_mk_and_import().is_ok());
    assert!(part.is_partition_provisioned());

    // generate the BK.
    let mut bk = [0u8; 80];
    assert!(part
        .generate_bk(&TEST_BK3, &TEST_POTA_ECC_PUB_KEY, &mut bk[..])
        .is_ok());

    // now generate the BMK.
    let mut bmk_out = [0u8; 300];
    let mut bmk_len = 300;
    assert!(part
        .generate_bmk(&bk, &mut bmk_len, &mut bmk_out[..])
        .is_ok());

    // now this is where live migration happens.
    assert!(part
        .import_mk_from_bmk(&TEST_BK3, &TEST_POTA_ECC_PUB_KEY, &bmk_out[..bmk_len])
        .is_ok());
}

#[test]
fn test_sealed_bk3_and_masked_bk_boot_in_store() {
    let mut pka = MockPka::new();
    pka.expect_clone().times(1).returning(MockPka::new);

    let mut rng = MockRng::new();
    rng.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let sha = MockSha::new();
    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    set_ipc_expectations(&mut hal);

    hal.expect_rng().times(1).return_const(rng);
    hal.expect_sha().return_const(sha);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 3072 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    // heap.
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    hal.expect_dma_heap().return_const(heap);

    let mut pfn_memory = [0u8; 3072];
    hal.expect_part_persistent_store_addr()
        .return_const(pfn_memory.as_ptr() as usize);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let store = unsafe { &mut *(pfn_memory.as_mut_ptr() as *mut HsmPartPersistentStore) };

    let masked_bk_boot = [0x12; 300];
    part.set_masked_bk_boot_len(masked_bk_boot.len() as u32);
    assert!(part.get_masked_bk_boot_len() == masked_bk_boot.len() as u32);
    assert!(store.masked_bk_boot.len == masked_bk_boot.len() as u32);

    let mut masked_bk_boot_mem_range = part.masked_bk_boot();
    let store_masked_bk_boot = masked_bk_boot_mem_range.slice_mut();
    store_masked_bk_boot.copy_from_slice(&masked_bk_boot[..masked_bk_boot.len()]);
    assert!(store.masked_bk_boot.data[..masked_bk_boot.len()] == masked_bk_boot[..]);

    let sealed_bk3 = [0xAB; 512];
    part.set_sealed_bk3_len(sealed_bk3.len() as u32);
    assert!(part.get_sealed_bk3_len() == sealed_bk3.len() as u32);
    assert!(store.sealed_bk3.len == sealed_bk3.len() as u32);

    let mut sealed_bk3_mem_range = part.sealed_bk3();
    let store_sealed_bk3 = sealed_bk3_mem_range.slice_mut();
    store_sealed_bk3.copy_from_slice(&sealed_bk3[..sealed_bk3.len()]);
    assert!(store.sealed_bk3.data[..sealed_bk3.len()] == sealed_bk3[..]);
}

#[test]
fn test_generate_signature_with_key_blob() {
    let mut pfn_memory = [0u8; 2048];
    // Cast pfn_memory to HsmPartPersistentStore using unsafe pointer casting
    let store = unsafe { &mut *(pfn_memory.as_mut_ptr() as *mut HsmPartPersistentStore) };
    store.partition_id_valid = true;
    store.partition_identifier.priv_key = [0x10; 48];

    assert_ne!(pfn_memory, [0u8; 2048]);

    let mut part_persistent_store_memory = [0u8; 2048 * 65];
    part_persistent_store_memory[..2048].copy_from_slice(&pfn_memory[..]);

    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let mut hal = MockHal::new();
    let mut pka = MockPka::new();
    let mut sha = MockSha::new();
    let mut heap = MockDmaHeap::new();

    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    pka.expect_clone().times(1).returning(|| {
        let mut pka = MockPka::new();
        pka.expect_begin_ecc_sign_zc().times(1).returning(
            |_tag, _curve, _digest, _privkey, _output| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc256,
                })
            },
        );

        pka
    });

    hal.expect_rng().once().return_const(rng_nonce);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    sha.expect_digest_zc().once().returning(move |_| Ok(()));
    hal.expect_sha().return_const(sha);

    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    hal.expect_dma_heap().times(1).return_const(heap);

    set_ipc_expectations(&mut hal);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let public_key_digest = IoMemRange::from([100u8; 48].as_slice());
    let signature_buf = [0u8; 192];
    let signature_mem_range = IoMemRange::from(signature_buf.as_slice());

    assert!(part
        .begin_signature_with_part_priv_key(
            TagId::default(),
            &public_key_digest,
            &signature_mem_range,
        )
        .is_ok());
}
