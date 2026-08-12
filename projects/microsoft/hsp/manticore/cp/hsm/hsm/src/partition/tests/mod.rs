// Copyright (c) Microsoft Corporation. All rights reserved.

#![cfg(test)]

mod cred_mgr_tests;
mod ioq_mgr_tests;
mod ioq_tests;
mod part_cert_chain_tests;
mod part_env_tests;
mod part_mgr_tests;
mod part_tests;
mod pct_ecc_keys_tests;
mod pin_policy_tests;
mod state_tests;
mod user_sess_aes_tests;
mod user_sess_ecc_tests;
mod user_sess_hkdf_tests;
mod user_sess_hmac_tests;
mod user_sess_kbkdf_tests;
mod user_sess_rsa_crt_tests;
mod user_sess_rsa_tests;
mod user_sess_sha_tests;
mod user_sess_tests;

mod test_consts;

use ::hmac::Hmac;
use ::hmac::Mac;
use mcr_crypto_sha::ShaMode;
use sha2::Sha256;
use sha2::Sha384;
use sha2::Sha512;

use cred_mgr::APP_VAULT_ID_FOR_INTERNAL_KEYS;
use mcr_crypto_pka::*;
use mcr_ddi_types::DdiApiRev;
use mcr_ipc_controller::IpcMessage;
use mcr_ipc_controller::IPC_MESSAGE_LENGTH;
use mcr_types::*;
pub(crate) use test_consts::*;

use super::*;
use crate::cmd_scheduler::CmdScheduler;
use crate::fsm::ComboFsm;
use crate::mock::MockAes;
use crate::mock::MockDmaAlloc;
use crate::mock::MockDmaHeap;
use crate::mock::MockEnv;
use crate::mock::MockHal;
use crate::mock::MockIpcMessageChannel;
use crate::mock::MockPka;
use crate::mock::MockRng;
use crate::mock::MockSha;
use crate::recorder::HsmFsmEventRecorder;

fn cmd_scheduler() -> CmdScheduler<ComboFsm<MockEnv>> {
    CmdScheduler::new(128, 1, HsmFsmEventRecorder::default())
}

fn set_ipc_expectations(hal: &mut MockHal) {
    let mut mock_fp_ipc_message_channel = MockIpcMessageChannel::new();
    mock_fp_ipc_message_channel
        .expect_clone()
        .once()
        .returning(MockIpcMessageChannel::new);

    hal.expect_hsm_to_fp_ipc_channel()
        .once()
        .return_const(mock_fp_ipc_message_channel);

    let mut mock_hsp_ipc_message_channel = MockIpcMessageChannel::new();
    mock_hsp_ipc_message_channel
        .expect_clone()
        .once()
        .returning(MockIpcMessageChannel::new);

    hal.expect_hsp_ipc_channel()
        .once()
        .return_const(mock_hsp_ipc_message_channel);

    let mut mock_hsm_to_admin_ipc_message_channel = MockIpcMessageChannel::new();
    mock_hsm_to_admin_ipc_message_channel
        .expect_clone()
        .once()
        .returning(MockIpcMessageChannel::new);

    hal.expect_hsm_to_admin_ipc_channel()
        .once()
        .return_const(mock_hsm_to_admin_ipc_message_channel);
}

fn set_fp_ipc_expectations(hal: &mut MockHal) {
    let mut mock_fp_ipc_message_channel = MockIpcMessageChannel::new();
    mock_fp_ipc_message_channel
        .expect_clone()
        .once()
        .returning(MockIpcMessageChannel::new);

    hal.expect_hsm_to_fp_ipc_channel()
        .once()
        .return_const(mock_fp_ipc_message_channel);
}

fn set_hsp_ipc_expectations(hal: &mut MockHal) {
    let mut mock_hsp_ipc_message_channel = MockIpcMessageChannel::new();
    mock_hsp_ipc_message_channel
        .expect_clone()
        .once()
        .returning(MockIpcMessageChannel::new);

    hal.expect_hsp_ipc_channel()
        .once()
        .return_const(mock_hsp_ipc_message_channel);
}

fn set_hsm_to_admin_ipc_expectations(hal: &mut MockHal) {
    let mut mock_hsm_to_admin_ipc_message_channel = MockIpcMessageChannel::new();
    mock_hsm_to_admin_ipc_message_channel
        .expect_clone()
        .once()
        .returning(MockIpcMessageChannel::new);

    hal.expect_hsm_to_admin_ipc_channel()
        .once()
        .return_const(mock_hsm_to_admin_ipc_message_channel);
}

fn set_fp_ipc_send_recv_expectations(hal: &mut MockHal, send_cnt: usize, recv_cnt: usize) {
    let mut mock_ipc_message_channel = MockIpcMessageChannel::new();

    mock_ipc_message_channel
        .expect_clone()
        .once()
        .returning(move || {
            let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
            mock_ipc_message_channel
                .expect_send_request()
                .times(send_cnt)
                .returning(|_, _| Ok(()));

            mock_ipc_message_channel
                .expect_receive_message()
                .times(recv_cnt)
                .returning(move || {
                    let mut data = [0; IPC_MESSAGE_LENGTH];
                    //     If needed mark the status field as 3 to simulate
                    //     failure and trigger delete key flow  --v
                    data[0] = 0x00000000; // 0x00030000;
                    Some(IpcMessage { data })
                });
            mock_ipc_message_channel
        });

    hal.expect_hsm_to_fp_ipc_channel()
        .once()
        .return_const(mock_ipc_message_channel);
}

fn set_fp_ipc_send_recv_failed_expectations(hal: &mut MockHal) {
    let mut mock_ipc_message_channel = MockIpcMessageChannel::new();

    mock_ipc_message_channel
        .expect_clone()
        .once()
        .returning(|| {
            let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
            mock_ipc_message_channel
                .expect_send_request()
                .once()
                .returning(|_, _| Ok(()));

            mock_ipc_message_channel
                .expect_receive_message()
                .once()
                .returning(move || {
                    let mut data = [0; IPC_MESSAGE_LENGTH];
                    //     If needed mark the status field as 3 to simulate
                    //     failure and trigger delete key flow  --v
                    data[0] = 0x00030000;
                    Some(IpcMessage { data })
                });
            mock_ipc_message_channel
        });

    hal.expect_hsm_to_fp_ipc_channel()
        .once()
        .return_const(mock_ipc_message_channel);
}

fn set_fp_ipc_send_failed_expectations(hal: &mut MockHal) {
    let mut mock_ipc_message_channel = MockIpcMessageChannel::new();

    mock_ipc_message_channel
        .expect_clone()
        .once()
        .returning(|| {
            let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
            mock_ipc_message_channel
                .expect_send_request()
                .once()
                .returning(|_, _| Err(10)); // some random failure
            mock_ipc_message_channel
        });

    hal.expect_hsm_to_fp_ipc_channel()
        .once()
        .return_const(mock_ipc_message_channel);
}

fn set_hsp_ipc_send_failed_expectations(hal: &mut MockHal) {
    let mut mock_ipc_message_channel = MockIpcMessageChannel::new();

    mock_ipc_message_channel
        .expect_clone()
        .once()
        .returning(|| {
            let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
            mock_ipc_message_channel
                .expect_send_request()
                .once()
                .returning(|_, _| Err(10)); // some random failure
            mock_ipc_message_channel
        });

    hal.expect_hsp_ipc_channel()
        .once()
        .return_const(mock_ipc_message_channel);
}

fn set_hsp_ipc_send_recv_expectations_with_recv_data(hal: &mut MockHal, in_data: &[u32]) {
    let mut mock_ipc_message_channel = MockIpcMessageChannel::new();

    let mut data = [0; IPC_MESSAGE_LENGTH];
    data[0..in_data.len()].copy_from_slice(in_data);

    mock_ipc_message_channel
        .expect_clone()
        .once()
        .returning(move || {
            let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
            mock_ipc_message_channel
                .expect_send_request()
                .once()
                .returning(move |_, _| Ok(()));

            mock_ipc_message_channel
                .expect_receive_message()
                .once()
                .returning(move || Some(IpcMessage { data }));
            mock_ipc_message_channel
        });

    hal.expect_hsp_ipc_channel()
        .once()
        .return_const(mock_ipc_message_channel);
}

fn part_env_with_alias_key_len_expectation(
    part_persistent_store_memory: &[u8],
) -> PartEnv<MockEnv> {
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let mut hal = MockHal::new();
    let mut pka = MockPka::new();
    let mut sha = MockSha::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_rng().once().return_const(rng_nonce);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    sha.expect_hmac().returning(
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
    hal.expect_sha().return_const(sha);

    hal.expect_alias_cert_len().return_const(512usize);

    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    set_ipc_expectations(&mut hal);
    PartEnv::<MockEnv>::new(hal, cmd_scheduler())
}

fn part_env(expect_rng_nonce: bool) -> PartEnv<MockEnv> {
    let mut hal = MockHal::new();
    let mut pka = MockPka::new();
    let mut sha = MockSha::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);

    if expect_rng_nonce {
        let mut rng_nonce = MockRng::new();
        rng_nonce.expect_bytes().once().returning(|buf| {
            let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
            buf.copy_from_slice(&nonce_to_return);
        });

        hal.expect_rng().once().return_const(rng_nonce);

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);
    }

    sha.expect_hmac().returning(
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
    hal.expect_sha().return_const(sha);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    set_ipc_expectations(&mut hal);
    PartEnv::<MockEnv>::new(hal, cmd_scheduler())
}

fn part_state() -> PartState<MockEnv> {
    let part_state = PartState::new(PcieFunction(0), part_env(false));

    part_state.rgs_mut().set_mask(0x1);

    part_state
}

fn rev() -> DdiApiRev {
    DdiApiRev { major: 1, minor: 0 }
}

fn partition(expect_rng_nonce: bool) -> Partition<MockEnv> {
    Partition::<MockEnv>::new(PcieFunction(0), part_env(expect_rng_nonce))
}

fn partition_with_alias_key_len_expectation(
    part_persistent_store_memory: &[u8],
) -> Partition<MockEnv> {
    Partition::<MockEnv>::new(
        PcieFunction(0),
        part_env_with_alias_key_len_expectation(part_persistent_store_memory),
    )
}

fn part_env_with_vault_expectations() -> PartEnv<MockEnv> {
    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().once().return_const(rng_nonce);

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);

    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

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
    PartEnv::<MockEnv>::new(hal, cmd_scheduler())
}

fn partition_with_vault_expectations() -> Partition<MockEnv> {
    Partition::<MockEnv>::new(PcieFunction(0), part_env_with_vault_expectations())
}

const TEST_DIGEST_VAL: u8 = 1;
const TEST_APP_PIN_VAL: u8 = 2;
const TEST_INVALID_APP_PIN_VAL: u8 = 0xff;

#[derive(Default)]
struct ExpectedParams {
    aes_encrypt_decrypt: usize,
    hal_aes: usize,
    hal_rng: usize,
    hal_bks_table: usize,
    heap_allocate: usize,
    rng_32_bytes: usize,
    rng_48_bytes: usize,
    tcon_tsc: usize,
    invalid_pin: bool,
}

fn partition_with_open_sessions_expectations(
    num_of_sess: usize,
    expected_params: ExpectedParams,
) -> Partition<MockEnv> {
    Partition::<MockEnv>::new(
        PcieFunction(0),
        part_env_with_open_sessions_expectations(num_of_sess, expected_params),
    )
}

fn part_env_with_open_sessions_expectations(
    num_of_sess: usize,
    expected_params: ExpectedParams,
) -> PartEnv<MockEnv> {
    let mut rng_nonce = MockRng::new();
    rng_nonce
        .expect_bytes()
        .times(expected_params.rng_32_bytes)
        .returning(|buf| {
            let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
            buf.copy_from_slice(&nonce_to_return);
        });

    rng_nonce
        .expect_bytes()
        .times(expected_params.rng_48_bytes)
        .returning(|buf| {
            let nonce_to_return: [u8; 48] = (1..49u8).collect::<Vec<_>>().try_into().unwrap();
            buf.copy_from_slice(&nonce_to_return);
        });

    let mut hal = MockHal::new();
    let mut pka = MockPka::new();
    let mut sha = MockSha::new();
    let mut aes = MockAes::new();

    aes.expect_encrypt_decrypt()
        .times(expected_params.aes_encrypt_decrypt)
        .returning(|_| Ok(()));
    hal.expect_rng()
        .times(expected_params.hal_rng)
        .return_const(rng_nonce);
    hal.expect_aes()
        .times(expected_params.hal_aes)
        .return_const(aes);

    const TEST_BKS_TABLE: [u8; 492] = [0x43; 492];
    hal.expect_bks_table_addr()
        .times(expected_params.hal_bks_table)
        .return_const(TEST_BKS_TABLE.as_ptr() as usize);

    pka.expect_clone().times(1).returning(move || {
        let mut pka = MockPka::new();

        pka.expect_begin_montgomery_constant_calculation()
            .times(num_of_sess)
            .returning(|_tag, _curve| Ok(()));
        pka.expect_end_montgomery_constant_calculation()
            .times(num_of_sess)
            .returning(|_tag| Ok(()));

        pka.expect_begin_ecc_point_validation_zc()
            .times(num_of_sess)
            .returning(move |_tag, _curve, _pubkey| Ok(()));
        pka.expect_end_ecc_point_validation_zc()
            .times(num_of_sess)
            .returning(|_tag| Ok(true));
        pka.expect_begin_ecdh_compute_zc()
            .times(num_of_sess)
            .returning(move |_tag, _curve, _privkey, _pubkey| {
                Ok(PkaEccCmd {
                    curve: PkaEccCurve::Ecc384,
                })
            });
        pka.expect_end_ecdh_compute()
            .times(num_of_sess)
            .returning(move |_tag, _op| {
                Ok(PkaEccSecretValue {
                    curve: PkaEccCurve::Ecc384,
                    secret: [0; PkaEccCurve::MAX_LEN],
                })
            });

        pka
    });
    hal.expect_pka().times(1..).return_const(vec![pka]);

    sha.expect_kbkdf_counter_hmac()
        .times(..)
        .return_const(Ok(()));

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
    heap.expect_allocate()
        .times(expected_params.heap_allocate)
        .returning(move |s| {
            let mut alloc = MockDmaAlloc::new(s);
            // Return pin
            if expected_params.invalid_pin {
                alloc.as_ref_mut()[..16].copy_from_slice(&[TEST_INVALID_APP_PIN_VAL; 16usize]);
            } else {
                alloc.as_ref_mut()[..16].copy_from_slice(&[TEST_APP_PIN_VAL; 16usize]);
            }
            Some(alloc)
        });
    hal.expect_dma_heap().return_const(heap);

    const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal.expect_tcon_tsc()
            .times(expected_params.tcon_tsc)
            .return_const(0u64);

        hal
    });

    set_ipc_expectations(&mut hal);

    PartEnv::<MockEnv>::new(hal, cmd_scheduler())
}

fn begin_and_continue_open_session(part: &Partition<MockEnv>) -> OpenSessionCtx<MockEnv> {
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

    result.unwrap()
}
