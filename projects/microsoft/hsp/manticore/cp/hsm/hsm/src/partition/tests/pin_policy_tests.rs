// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::partition::tests::rev;
use crate::partition::HsmPartition;
use mcr_ddi_types::DdiTestActionPinPolicyConfig;

const CLOCK_FREQ: u64 = 62500000;

#[test]
fn test_single_null_pin() {
    let part = partition_with_open_sessions_expectations(
        1,
        ExpectedParams {
            aes_encrypt_decrypt: 0,
            hal_aes: 0,
            hal_rng: 1,
            heap_allocate: 8,
            rng_32_bytes: 1,
            tcon_tsc: 0,
            invalid_pin: true,
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
        encrypted_pin: MborByteArray::new_with_len(encrypted_pin_buf.as_ptr(), 0),
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
    assert!(matches!(result, Err(HsmErr::InvalidArgument)));
}

#[test]
fn test_single_invalid_pin() {
    let part = partition_with_open_sessions_expectations(
        1,
        ExpectedParams {
            aes_encrypt_decrypt: 3,
            hal_aes: 3,
            hal_rng: 2,
            heap_allocate: 15,
            rng_32_bytes: 2,
            tcon_tsc: 1,
            invalid_pin: true,
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
    assert!(matches!(result, Err(HsmErr::InvalidUserCredential)));

    {
        let pin_policy_mgr = part.state.pin_policy_mgr();

        assert_eq!(pin_policy_mgr.pin_policy().allowed_attempts, 1);
        assert_eq!(
            u64::from_le_bytes(pin_policy_mgr.pin_policy().lockout_time),
            0
        );
        assert_eq!(pin_policy_mgr.pin_policy().delay_factor, 0);
    }
}

#[test]
fn test_multi_invalid_pin() {
    let total_attempts: usize = 2;
    let part = partition_with_open_sessions_expectations(
        total_attempts,
        ExpectedParams {
            aes_encrypt_decrypt: 3 * total_attempts,
            hal_aes: 3 * total_attempts,
            hal_rng: 2 + total_attempts - 1,
            heap_allocate: 15 * total_attempts,
            rng_32_bytes: 2 + total_attempts - 1,
            tcon_tsc: 3 * total_attempts - 1,
            invalid_pin: true,
            ..Default::default()
        },
    );
    part.set_resource_mask(1);

    {
        let mut pin_policy_mgr = part.state.pin_policy_mgr_mut();

        let override_allowed_attempts = MAX_ALLOWED_PIN_AUTH_ATTEMPTS - total_attempts as u16;
        pin_policy_mgr.override_pin_policy_context(DdiTestActionPinPolicyConfig {
            delay_increment: None,
            state: None,
            delay: None,
            allowed_attempts: Some(override_allowed_attempts),
            lockout_delay: None,
        });

        assert_eq!(
            pin_policy_mgr.pin_policy().allowed_attempts,
            override_allowed_attempts
        );
    }

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

    for _ in 0..total_attempts {
        let sess_ctx = begin_and_continue_open_session(&part);

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
        assert!(matches!(result, Err(HsmErr::InvalidUserCredential)));
    }

    {
        let pin_policy_mgr = part.state.pin_policy_mgr_mut();
        let context = pin_policy_mgr.pin_policy();

        assert_eq!(context.allowed_attempts, 0);
        assert_eq!(context.delay_factor, 1);
        assert_eq!(u64::from_le_bytes(context.lockout_time), 60 * CLOCK_FREQ);

        assert!(!pin_policy_mgr.can_login());
    }
}

#[test]
fn test_rollover_delay_factor() {
    let total_attempts: usize = 3;
    let part = partition_with_open_sessions_expectations(
        total_attempts,
        ExpectedParams {
            aes_encrypt_decrypt: 3 * total_attempts,
            hal_aes: 3 * total_attempts,
            hal_rng: 2 + total_attempts - 1,
            heap_allocate: 15 * total_attempts,
            rng_32_bytes: 2 + total_attempts - 1,
            tcon_tsc: 3 * total_attempts,
            invalid_pin: true,
            ..Default::default()
        },
    );
    part.set_resource_mask(1);

    {
        let mut pin_policy_mgr = part.state.pin_policy_mgr_mut();

        let override_allowed_attempts = MAX_ALLOWED_PIN_AUTH_ATTEMPTS - total_attempts as u16;

        pin_policy_mgr.override_pin_policy_context(DdiTestActionPinPolicyConfig {
            delay_increment: None,
            state: None,
            delay: Some(MAX_DELAY_FACTOR_IN_MINS),
            allowed_attempts: Some(override_allowed_attempts),
            lockout_delay: Some(0),
        });

        let context = pin_policy_mgr.pin_policy();
        assert_eq!(context.allowed_attempts, override_allowed_attempts);
        assert_eq!(context.delay_factor, MAX_DELAY_FACTOR_IN_MINS);
    }

    let encrypted_id_buf = [1u8; 16];
    let encrypted_pin_buf = [1u8; 16];
    let encrypted_seed_buf = [1u8; 48];
    let iv_buf = [1u8; 16];

    for _ in 0..total_attempts as u16 - 1 {
        let sess_ctx = begin_and_continue_open_session(&part);

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
        assert!(matches!(result, Err(HsmErr::InvalidUserCredential)));

        {
            let mut pin_policy_mgr = part.state.pin_policy_mgr_mut();
            let context = pin_policy_mgr.pin_policy();

            assert_eq!(u64::from_le_bytes(context.lockout_time), 1920 * CLOCK_FREQ);

            // Reset the wait time
            pin_policy_mgr.reset_lockout_time();
        }
    }

    let sess_ctx = begin_and_continue_open_session(&part);

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
    assert!(matches!(result, Err(HsmErr::InvalidUserCredential)));

    {
        let context = part.state.pin_policy_mgr_mut().pin_policy();
        assert_eq!(context.allowed_attempts, 0);
        assert_eq!(context.delay_factor, 0);
    }
}
