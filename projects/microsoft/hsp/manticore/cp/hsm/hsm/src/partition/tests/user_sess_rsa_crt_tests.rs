// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaRsaCmd;
use mcr_crypto_pka::PkaRsaData;
use mcr_crypto_pka::PkaRsaMontData;
use mcr_crypto_pka::PkaRsaSize;
use mcr_types::*;

use crate::cmd_scheduler::TagId;
use crate::error::HsmErr;
use crate::mock::*;
use crate::partition::tests::cmd_scheduler;
use crate::partition::tests::rev;
use crate::partition::tests::set_ipc_expectations;
use crate::partition::HsmUserSession;
use crate::partition::PartEnv;
use crate::partition::PartState;
use crate::partition::PkaConvertible;
use crate::partition::RsaCrtParamCalcState;
use crate::partition::RsaPrivKeyCrt;
use crate::partition::RsaSize;
use crate::partition::UserSession;

#[test]
fn test_rsa_compute_crt_params_rsa_crt_2k() {
    test_rsa_compute_crt_params_generic(PkaRsaSize::Rsa2k);
}

#[test]
fn test_rsa_compute_crt_params_rsa_crt_3k() {
    test_rsa_compute_crt_params_generic(PkaRsaSize::Rsa3k);
}

#[test]
fn test_rsa_compute_crt_params_rsa_crt_4k() {
    test_rsa_compute_crt_params_generic(PkaRsaSize::Rsa4k);
}

fn test_rsa_compute_crt_params_generic(rsa_type: PkaRsaSize) {
    let data_length = rsa_type.into();
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
                    data_be: SecureByteVec::from(vec![0; PkaRsaMontData::MAX_LEN]),
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
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
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
                    data_be: SecureByteVec::from(vec![0; PkaRsaMontData::MAX_LEN]),
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
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
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
                    data_be: SecureByteVec::from(vec![0; PkaRsaMontData::MAX_LEN]),
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
                    data_be: SecureByteVec::from(vec![0; PkaRsaMontData::MAX_LEN]),
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

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };
    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());
    let mut op = begin_compute_crt_params_result.unwrap();
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
        let continue_compute_crt_result = app_session.continue_compute_rsa_crt_params(tag, op);
        assert!(continue_compute_crt_result.is_ok());
        op = continue_compute_crt_result.unwrap();
        if op.state == RsaCrtParamCalcState::Idle {
            break;
        }
        count += 1;
    }

    let end_compute_crt_result = app_session.end_compute_rsa_crt_params(op);
    assert!(end_compute_crt_result.is_ok());

    let priv_key_crt = end_compute_crt_result.unwrap();
    assert!(priv_key_crt.to_pka_bytes().is_ok());
}

#[test]
fn test_begin_compute_rsa_crt_params_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Err(u32::MAX));

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

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };
    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_err());
}

#[test]
fn test_continue_compute_rsa_crt_params_tag_mismatch() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let continue_compute_crt_params_result =
        app_session.continue_compute_rsa_crt_params(123, begin_compute_crt_params_result.unwrap());
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::PkaTagMismatch);
    }
}

#[test]
fn test_rsa_crt_step_1_end_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_constant_calculation()
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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N1qWaitForMontgomeryFullMod;

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryConstCalcFailed);
    }
}

#[test]
fn test_rsa_crt_step_2_begin_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Ok(()));

        pka.expect_begin_rsa_montgomery_in()
            .times(1)
            .returning(move |_data, _rsa_type, _data_be| Err(u32::MAX));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N1qWaitForMontgomeryFullMod;

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryInFailed);
    }
}

#[test]
fn test_rsa_crt_step_2_end_invalid_state_err() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N1qWaitForQinvModPToMontIn;

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::InvalidState);
    }
}

#[test]
fn test_rsa_crt_step_2_end_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_in()
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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N1qWaitForQinvModPToMontIn;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryInFailed);
    }
}

#[test]
fn test_rsa_crt_step_3_begin_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

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
            .returning(move |_data, _rsa_type, _data_be| Err(u32::MAX));
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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N1qWaitForQinvModPToMontIn;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryInFailed);
    }
}

#[test]
fn test_rsa_crt_step_3_end_invalid_state() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_in()
            .times(1)
            .returning(move |_tag, _op| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N1qWaitForQToMontIn;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::InvalidState);
    }
}

#[test]
fn test_rsa_crt_step_3_end_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N1qWaitForQToMontIn;

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::InvalidState);
    }
}

#[test]
fn test_rsa_crt_step_4_begin_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

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
            .returning(move |_data, _rsa_type, _val1, _val2| Err(u32::MAX));
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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N1qWaitForQToMontIn;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });
    op.mont_in_q_inv_mod_p = Some(PkaRsaMontData {
        size: rsa_type,
        data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
    });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaModularMultiplicationFailed);
    }
}

#[test]
fn test_rsa_crt_step_4_end_invalid_state() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N1qWaitForModMultiplication;

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::InvalidState);
    }
}

#[test]
fn test_rsa_crt_step_4_end_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_modular_multiplication()
            .times(1)
            .returning(move |_rsa_type, _op| Err(u32::MAX));
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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N1qWaitForModMultiplication;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaModularMultiplicationFailed);
    }
}

#[test]
fn test_rsa_crt_step_5_begin_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_modular_multiplication()
            .times(1)
            .returning(move |_rsa_type, _op| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
                })
            });
        pka.expect_begin_rsa_montgomery_out()
            .times(1)
            .returning(move |_rsa_type, _op, _data_be| Ok(PkaRsaCmd { rsa_type }));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N1qWaitForModMultiplication;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaModularMultiplicationFailed);
    }
}

#[test]
fn test_rsa_crt_step_5_end_invalid_state() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N1qWaitForMontOut;

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::InvalidState);
    }
}

#[test]
fn test_rsa_crt_step_5_end_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_out()
            .times(1)
            .returning(move |_rsa_type, _op| Err(u32::MAX));
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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N1qWaitForMontOut;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryOutFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_1_begin_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_out()
            .times(1)
            .returning(move |_rsa_type, _op| {
                Ok(PkaRsaData {
                    size: rsa_type,
                    data_be: vec![0; PkaRsaSize::MAX_LEN].into(),
                })
            });

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Err(u32::MAX));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N1qWaitForMontOut;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryConstCalcFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_1_end_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_constant_calculation()
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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForMontgomeryModQ;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryConstCalcFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_2_begin_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Ok(()));
        pka.expect_begin_rsa_montgomery_in()
            .times(1)
            .returning(|_tag, _type, _data_be| Err(u32::MAX));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForMontgomeryModQ;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryInFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_2_end_invalid_state() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForPToMontIn;

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::InvalidState);
    }
}

#[test]
fn test_rsa_crt_n2p_step_2_end_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_in()
            .times(1)
            .returning(|_tag, _data_be| Err(u32::MAX));
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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForPToMontIn;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryInFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_3_begin_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_in()
            .times(1)
            .returning(move |_tag, _data_be| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
                })
            });
        pka.expect_begin_rsa_modular_inverse()
            .times(1)
            .returning(move |_data, _rsa_type, _data_be| Err(u32::MAX));
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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForPToMontIn;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaModularInverseFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_3_end_invalid_state() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForModInverseP;

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::InvalidState);
    }
}

#[test]
fn test_rsa_crt_n2p_step_3_end_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_modular_inverse()
            .times(1)
            .returning(|_tag, _data_be| Err(u32::MAX));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForModInverseP;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaModularInverseFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_4_begin_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));
        pka.expect_end_rsa_modular_inverse()
            .times(1)
            .returning(move |_tag, _data_be| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
                })
            });
        pka.expect_begin_rsa_montgomery_out()
            .times(1)
            .returning(move |_tag, _type, _data_be| Ok(PkaRsaCmd { rsa_type }));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForModInverseP;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryOutFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_4_end_invalid_state() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForPinvModQToMontOut;

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::InvalidState);
    }
}

#[test]
fn test_rsa_crt_n2p_step_4_end_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_out()
            .times(1)
            .returning(|_tag, _data_be| Err(u32::MAX));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForPinvModQToMontOut;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryOutFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_5_begin_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));
        pka.expect_end_rsa_montgomery_out()
            .times(1)
            .returning(move |_tag, _data_be| {
                Ok(PkaRsaData {
                    size: rsa_type,
                    data_be: vec![0; PkaRsaSize::MAX_LEN].into(),
                })
            });
        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Err(u32::MAX));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForPinvModQToMontOut;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryConstCalcFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_5_end_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_constant_calculation()
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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForMontgomeryFullMod;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryConstCalcFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_6_begin_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));
        pka.expect_end_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_tag| Ok(()));

        pka.expect_begin_rsa_montgomery_in()
            .times(1)
            .returning(move |_data, _rsa_type, _data_be| Err(u32::MAX));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForMontgomeryFullMod;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryInFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_6_end_invalid_state() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForPToMontIn2;

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::InvalidState);
    }
}

#[test]
fn test_rsa_crt_n2p_step_6_end_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_in()
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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForPToMontIn2;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryInFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_7_begin_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));
        pka.expect_end_rsa_montgomery_in()
            .times(1)
            .returning(move |_tag, _data_be| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
                })
            });
        pka.expect_begin_rsa_montgomery_in()
            .times(1)
            .returning(move |_data, _rsa_type, _data_be| Err(u32::MAX));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForPToMontIn2;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });
    op.p_inv_mod_q = Some(PkaRsaData {
        size: rsa_type,
        data_be: vec![0; PkaRsaSize::MAX_LEN].into(),
    });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryInFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_7_end_invalid_state() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForPInvModQToMontIn;

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::InvalidState);
    }
}

#[test]
fn test_rsa_crt_n2p_step_7_end_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_in()
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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForPInvModQToMontIn;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryInFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_8_begin_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));
        pka.expect_end_rsa_montgomery_in()
            .times(1)
            .returning(move |_tag, _data_be| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
                })
            });
        pka.expect_begin_rsa_modular_multiplication()
            .times(1)
            .returning(move |_tag, _rsa_type, _val1, _val2| Err(u32::MAX));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForPInvModQToMontIn;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });
    op.mont_in_p_full = Some(PkaRsaMontData {
        size: rsa_type,
        data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
    });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaModularMultiplicationFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_8_end_invalid_state() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForModMultiplication;

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::InvalidState);
    }
}

#[test]
fn test_rsa_crt_n2p_step_8_end_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_modular_multiplication()
            .times(1)
            .returning(move |_rsa_type, _op| Err(u32::MAX));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForModMultiplication;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaModularMultiplicationFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_9_begin_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));
        pka.expect_end_rsa_modular_multiplication()
            .times(1)
            .returning(move |_rsa_type, _op| {
                Ok(PkaRsaMontData {
                    size: rsa_type,
                    data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
                })
            });
        pka.expect_begin_rsa_montgomery_out()
            .times(1)
            .returning(move |_data, _rsa_type, _data_be| Err(u32::MAX));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForModMultiplication;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });
    op.mont_in_p_full = Some(PkaRsaMontData {
        size: rsa_type,
        data_be: SecureByteVec::from(vec![0u8; PkaRsaMontData::MAX_LEN]),
    });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryOutFailed);
    }
}

#[test]
fn test_rsa_crt_n2p_step_9_end_fail() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

        pka.expect_end_rsa_montgomery_out()
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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::N2pWaitForMontOut;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result = app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result {
        assert_eq!(err, HsmErr::RsaMontgomeryOutFailed);
    }
}

#[test]
fn test_rsa_crt_unknown_state() {
    let rsa_type = PkaRsaSize::Rsa2k;
    let data_length = rsa_type.into();
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

        pka.expect_peek_tag()
            .times(1)
            .returning(|| Some(TagId::default()));

        pka.expect_begin_rsa_montgomery_constant_calculation()
            .times(1)
            .returning(|_data, _rsa_type, _modulus_be| Ok(()));

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

    let app_session = UserSession::new(rev(), 10, state.clone());

    let priv_key_crt = RsaPrivKeyCrt {
        rsa_type: RsaSize::try_from(rsa_type).unwrap(),
        p: vec![].into(),
        q: vec![].into(),
        dp: vec![].into(),
        dq: vec![].into(),
        n: vec![].into(),
        n1q: None,
        n2p: None,
        e: vec![].into(),
        coefficient: vec![].into(),
    };

    let begin_compute_crt_params_result =
        app_session.begin_compute_rsa_crt_params(tag, priv_key_crt);
    assert!(begin_compute_crt_params_result.is_ok());

    let mut op = begin_compute_crt_params_result.unwrap();
    op.state = RsaCrtParamCalcState::Idle;
    op.rsa_op_data = Some(PkaRsaCmd { rsa_type });

    let continue_compute_crt_params_result_err =
        app_session.continue_compute_rsa_crt_params(tag, op);
    if let Err(err) = continue_compute_crt_params_result_err {
        assert_eq!(err, HsmErr::InvalidState);
    }
}
