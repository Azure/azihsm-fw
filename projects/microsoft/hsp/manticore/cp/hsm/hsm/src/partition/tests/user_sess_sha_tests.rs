// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use crate::mock::*;
use crate::partition::tests::cmd_scheduler;
use crate::partition::tests::rev;
use crate::partition::tests::set_ipc_expectations;
use crate::partition::HsmUserSession;
use crate::partition::PartEnv;
use crate::partition::PartState;
use crate::partition::ShaType;
use crate::partition::UserSession;

const TOTAL_TABLE_LEN: usize = 17 * 1024;

fn create_app_session(sha: Option<MockSha>) -> UserSession<MockEnv> {
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    if let Some(sha_val) = sha {
        hal.expect_sha().once().return_const(sha_val);
    }
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

    UserSession::new(rev(), 10, state)
}

#[test]
fn test_sha_single_block_sha1() {
    let mode = ShaType::Sha1;

    let mut sha = MockSha::new();
    sha.expect_digest_zc().once().returning(move |_| Ok(()));

    let input_data = vec![0; 64];
    let input_range = IoMemRange::from(input_data.as_slice());
    let output_data = vec![0; SHA_DIGEST_MAX_SIZE_BYTES];
    let mut output_range = IoMemRange::from(output_data.as_slice());

    let app_session = create_app_session(Some(sha));
    let result = app_session.sha_single_block_zc(mode, &input_range, &mut output_range);
    assert!(result.is_ok());
}

#[test]
fn test_sha_single_block_256() {
    let mode = ShaType::Sha256;

    let mut sha = MockSha::new();
    sha.expect_digest_zc().once().returning(move |_| Ok(()));

    let input_data = vec![0; 64];
    let input_range = IoMemRange::from(input_data.as_slice());
    let output_data = vec![0; SHA_DIGEST_MAX_SIZE_BYTES];
    let mut output_range = IoMemRange::from(output_data.as_slice());

    let app_session = create_app_session(Some(sha));
    let result = app_session.sha_single_block_zc(mode, &input_range, &mut output_range);
    assert!(result.is_ok());
}

#[test]
fn test_sha_single_block_384() {
    let mode = ShaType::Sha384;

    let mut sha = MockSha::new();
    sha.expect_digest_zc().once().returning(move |_| Ok(()));

    let input_data = vec![0; 128];
    let input_range = IoMemRange::from(input_data.as_slice());
    let output_data = vec![0; SHA_DIGEST_MAX_SIZE_BYTES];
    let mut output_range = IoMemRange::from(output_data.as_slice());

    let app_session = create_app_session(Some(sha));
    let result = app_session.sha_single_block_zc(mode, &input_range, &mut output_range);
    assert!(result.is_ok());
}

#[test]
fn test_sha_single_block_512() {
    let mode = ShaType::Sha512;

    let mut sha = MockSha::new();
    sha.expect_digest_zc().once().returning(move |_| Ok(()));

    let input_data = vec![0; 128];
    let input_range = IoMemRange::from(input_data.as_slice());
    let output_data = vec![0; SHA_DIGEST_MAX_SIZE_BYTES];
    let mut output_range = IoMemRange::from(output_data.as_slice());
    let app_session = create_app_session(Some(sha));

    let result = app_session.sha_single_block_zc(mode, &input_range, &mut output_range);
    assert!(result.is_ok());
}

#[test]
fn test_sha_single_block_512_multiple() {
    let mode = ShaType::Sha512;

    let mut sha = MockSha::new();
    sha.expect_digest_zc().times(2).returning(move |_| Ok(()));

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    let mut hal = MockHal::new();
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_sha().times(2).return_const(sha);
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

    let input_data = vec![0; 23];
    let input_range = IoMemRange::from(input_data.as_slice());
    let output_data = vec![0; SHA_DIGEST_MAX_SIZE_BYTES];
    let mut output_range = IoMemRange::from(output_data.as_slice());

    // Call 1: single block sha
    let result = app_session.sha_single_block_zc(mode, &input_range, &mut output_range);
    assert!(result.is_ok());

    let input_data = vec![0; 40];
    let input_range = IoMemRange::from(input_data.as_slice());

    // Call 2: single block sha
    let result = app_session.sha_single_block_zc(mode, &input_range, &mut output_range);
    assert!(result.is_ok());
}

#[test]
fn test_sha_single_block_small_input() {
    let mode = ShaType::Sha512;

    let mut sha = MockSha::new();
    sha.expect_digest_zc().once().returning(move |_| Ok(()));

    let input_data = vec![0; 12];
    let input_range = IoMemRange::from(input_data.as_slice());
    let output_data = vec![0; SHA_DIGEST_MAX_SIZE_BYTES];
    let mut output_range = IoMemRange::from(output_data.as_slice());

    let app_session = create_app_session(Some(sha));
    let result = app_session.sha_single_block_zc(mode, &input_range, &mut output_range);
    assert!(result.is_ok());
}

#[test]
fn test_sha_digest_driver_fail() {
    let mode = ShaType::Sha256;

    let mut sha = MockSha::new();
    sha.expect_digest_zc()
        .once()
        .returning(move |_| Err(u32::MAX)?);

    let app_session = create_app_session(Some(sha));

    let input_data = vec![0; 64];
    let input_range = IoMemRange::from(input_data.as_slice());
    let output_data = vec![0; SHA_DIGEST_MAX_SIZE_BYTES];
    let mut output_range = IoMemRange::from(output_data.as_slice());

    let result = app_session.sha_single_block_zc(mode, &input_range, &mut output_range);
    assert!(result.is_err());
}
