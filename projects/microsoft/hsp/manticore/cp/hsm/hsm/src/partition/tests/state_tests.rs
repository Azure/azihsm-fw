// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use crate::mock::MockEnv;
use crate::mock::MockHal;
use crate::mock::MockPka;
use crate::mock::MockRng;
use crate::partition::tests::cmd_scheduler;
use crate::partition::tests::part_state;
use crate::partition::tests::set_ipc_expectations;
use crate::partition::GetCertChainLengthsInfo;
use crate::partition::PartEnv;
use crate::partition::PartState;
use crate::partition::MAX_CERTS;

#[test]
fn test_enable() {
    let state = part_state();

    assert!(!state.enabled());
    state.enable();
    assert!(state.enabled());
}

#[test]
fn test_disable() {
    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().times(1).return_const(rng_nonce);

    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .return_const(cdma_vault_meta_data.as_ptr() as usize);

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

    let old_nonce = state.nonce();

    assert!(state.change_user_cred(&[1; 16], &[2; 16]).is_ok());
    state.set_unwrapping_key_id(Some(0x10));
    state.set_establish_cred_encryption_key_id(Some(0x11));
    state.set_session_encryption_key_id(Some(0x12));

    let expected_old_nonce: [u8; 32] = [0; 32usize];
    assert_eq!(old_nonce, expected_old_nonce);
    assert!(
        state.cred_mgr().user_cred()
            != UserCredential {
                id: [0; 16],
                pin: [0; 16],
                vault_id: 0
            }
    );

    assert!(state.verify_user_cred_is_set());
    assert!(state.unwrapping_key_id().is_some());
    assert!(state.get_establish_cred_encryption_key_id().is_some());
    assert!(state.get_session_encryption_key_id().is_some());

    state.enable();
    state.disable();
    assert!(!state.enabled());

    let new_nonce = state.nonce();
    let expected_new_nonce: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();

    assert_ne!(new_nonce, old_nonce);
    assert_eq!(new_nonce, expected_new_nonce);
    assert!(
        state.cred_mgr().user_cred()
            == UserCredential {
                id: [0; 16],
                pin: [0; 16],
                vault_id: 0
            }
    );

    assert!(!state.verify_user_cred_is_set());
    assert!(state.unwrapping_key_id().is_none());
    assert!(state.get_establish_cred_encryption_key_id().is_none());
    assert!(state.get_session_encryption_key_id().is_none());
}

#[test]
fn test_migrate() {
    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().times(1).return_const(rng_nonce);

    /// Maximum number of bytes in the table.
    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);
    hal.expect_cdma_vault_meta_data()
        .return_const(cdma_vault_meta_data.as_ptr() as usize);

    let mut pka = MockPka::new();

    let persistent_store: &'static mut [HsmPartPersistentStore] = mcr_mem_map::mem_addr_to_slice(
        part_persistent_store_memory.as_ptr() as usize,
        MAX_PCIE_FUNCTIONS,
    );

    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);

    let old_nonce = state.nonce();

    assert!(state.change_user_cred(&[1; 16], &[2; 16]).is_ok());
    state.set_unwrapping_key_id(Some(0x10));
    state.set_establish_cred_encryption_key_id(Some(0x11));
    state.set_session_encryption_key_id(Some(0x12));

    let expected_old_nonce: [u8; 32] = [0; 32usize];
    assert_eq!(old_nonce, expected_old_nonce);
    assert!(
        state.cred_mgr().user_cred()
            != UserCredential {
                id: [0; 16],
                pin: [0; 16],
                vault_id: 0
            }
    );

    assert!(state.verify_user_cred_is_set());
    assert!(state.unwrapping_key_id().is_some());
    assert!(state.get_establish_cred_encryption_key_id().is_some());
    assert!(state.get_session_encryption_key_id().is_some());

    state.enable();
    persistent_store[0].session_table[0] = 0xA5;
    assert_eq!(persistent_store[0].session_table[1], 0x00);

    state.migrate();

    assert!(state.enabled());
    assert_eq!(persistent_store[0].session_table[0], 0xA5);
    assert_eq!(persistent_store[0].session_table[1], 0xA5);

    let new_nonce = state.nonce();
    let expected_new_nonce: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();

    assert_ne!(new_nonce, old_nonce);
    assert_eq!(new_nonce, expected_new_nonce);
    assert!(
        state.cred_mgr().user_cred()
            == UserCredential {
                id: [0; 16],
                pin: [0; 16],
                vault_id: 0
            }
    );

    assert!(!state.verify_user_cred_is_set());
    assert!(state.unwrapping_key_id().is_none());
    assert!(state.get_establish_cred_encryption_key_id().is_none());
    assert!(state.get_session_encryption_key_id().is_none());
}

#[test]
fn test_clear_partition_info() {
    let mut hal = MockHal::new();

    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    let mut pka = MockPka::new();

    let persistent_store: &'static mut [HsmPartPersistentStore] = mcr_mem_map::mem_addr_to_slice(
        part_persistent_store_memory.as_ptr() as usize,
        MAX_PCIE_FUNCTIONS,
    );

    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);

    let old_nonce = state.nonce();

    assert!(state.change_user_cred(&[1; 16], &[2; 16]).is_ok());
    state.set_unwrapping_key_id(Some(0x10));
    state.set_establish_cred_encryption_key_id(Some(0x11));
    state.set_session_encryption_key_id(Some(0x12));

    let expected_old_nonce: [u8; 32] = [0; 32usize];
    assert_eq!(old_nonce, expected_old_nonce);
    assert!(
        state.cred_mgr().user_cred()
            != UserCredential {
                id: [0; 16],
                pin: [0; 16],
                vault_id: 0
            }
    );

    assert!(state.verify_user_cred_is_set());
    assert!(state.unwrapping_key_id().is_some());
    assert!(state.get_establish_cred_encryption_key_id().is_some());
    assert!(state.get_session_encryption_key_id().is_some());

    state.enable();
    let part_store_ref = &mut persistent_store[0];
    part_store_ref.session_table[0] = 0xA5;
    part_store_ref.vm_launch_guid = [0xA8; 16];
    part_store_ref.partition_id_valid = true;
    part_store_ref.partition_cert_valid = true;
    part_store_ref.partition_cert.length = 100;
    part_store_ref.partition_cert.data = [0xAC; 800];
    part_store_ref.partition_identifier.id = [0xA9; 16];
    part_store_ref.partition_identifier.priv_key = [0xAA; 48];
    part_store_ref.partition_identifier.pub_key = [0xAB; 97];
    part_store_ref.unwrapping_key_required = true;
    part_store_ref.unwrapping_key_bk_valid = UnwrappingKeyValidity::PendingPct as u8;
    part_store_ref.unwrapping_key_bk = [0xAC; 516];
    part_store_ref.nonce = (1..33u8).collect::<Vec<_>>().try_into().unwrap();

    let info = GetCertChainLengthsInfo {
        hash: [0xAA; 32],
        num_certs: 5,
        cert_lengths: [400; MAX_CERTS],
    };
    state.set_cert_chain_lengths_info(Some(info));

    state.clear_partition_info();

    let expected_nonce: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();

    assert!(state.enabled());
    assert_eq!(part_store_ref.session_table[0], 0x00);
    assert_eq!(part_store_ref.vm_launch_guid, [0x00; 16]);
    assert!(!part_store_ref.partition_id_valid);
    assert!(!part_store_ref.partition_cert_valid);
    assert_eq!(part_store_ref.partition_cert.length, 0);
    assert_eq!(part_store_ref.partition_cert.data, [0x00; 800]);
    assert_eq!(part_store_ref.partition_identifier.id, [0x00; 16]);
    assert_eq!(part_store_ref.partition_identifier.priv_key, [0x00; 48]);
    assert_eq!(part_store_ref.partition_identifier.pub_key, [0x00; 97]);
    assert_eq!(
        part_store_ref.unwrapping_key_bk_valid,
        UnwrappingKeyValidity::Empty as u8
    );
    assert!(!part_store_ref.unwrapping_key_required);
    assert_eq!(part_store_ref.unwrapping_key_bk, [0x00; 516]);
    assert_eq!(part_store_ref.nonce, expected_nonce);
    assert!(state.get_cert_chain_lengths_info().is_none());
}

#[test]
fn test_rgs() {
    let state = part_state();

    let rgs = state.rgs();
    assert_eq!(rgs.mask(), 0x1);
}

#[test]
fn test_rgs_mut() {
    let state = part_state();

    let mut rgs = state.rgs_mut();
    rgs.set_mask(1);
    assert_eq!(rgs.mask(), 1);
}

#[test]
fn test_ioq_mgr() {
    let state = part_state();

    let ioq_mgr = state.ioq_mgr();
    assert!(ioq_mgr.io_queue(DevSqId::Id65).is_none());
}

#[test]
fn test_ioq_mgr_mut() {
    let state = part_state();

    let mut ioq_mgr = state.ioq_mgr_mut();
    ioq_mgr.enable_io_queue(DevSqId::Id65, DevCqId::Id65);
    assert!(ioq_mgr.io_queue(DevSqId::Id65).is_some());
}

#[test]
fn test_cred_mgr() {
    let state = part_state();

    let cred_mgr = state.cred_mgr();
    assert!(
        cred_mgr.user_cred()
            == UserCredential {
                id: [0; 16],
                pin: [0; 16],
                vault_id: 0
            }
    );
}

#[test]
fn test_cred_mgr_mut() {
    let state = part_state();

    let mut cred_mgr = state.cred_mgr_mut();
    assert!(cred_mgr.change_user_cred(&[0x56; 16], &[0x57; 16]).is_ok());
    assert!(
        cred_mgr.user_cred()
            == UserCredential {
                id: [0x56; 16],
                pin: [0x57; 16],
                vault_id: 0
            }
    );
}

#[test]
fn test_env() {
    let mut hal = MockHal::new();

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr().once().returning(|| 0);
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

    let get_env = state.env();
    assert_eq!(get_env.vault_addr(), 0);
}

#[test]
fn test_vault() {
    let mut hal = MockHal::new();

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr().once().returning(|| 0);
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

    state.vault();
}

#[test]
fn test_cdma_vault_import_key_delete_key() {
    let mut hal = MockHal::new();

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);

    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    let cdma_vault_meta_data: [u8; 65] = [0; 65];
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

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let state = PartState::new(PcieFunction(0), env);
    state.rgs_mut().set_mask(0x1);

    let mut cdma_vault = state.cdma_vault();

    let key_blob = [0u8; 32];
    let result = cdma_vault.import_key(&key_blob[..]);
    let cdma_key_id = result.unwrap();

    assert_eq!(cdma_key_id.vault_id(), 0);
    assert_eq!(cdma_key_id.key_index(), 0);

    let result = cdma_vault.import_key(&key_blob[..]);

    let cdma_key_id = result.unwrap();
    assert_eq!(cdma_key_id.vault_id(), 0);
    assert_eq!(cdma_key_id.key_index(), 1);

    let result = cdma_vault.import_key(&key_blob[..]);
    let result = cdma_vault.delete_key(result.unwrap());

    assert!(result == Ok(()));
}

#[test]
fn test_cred_mgr_clear() {
    let state = part_state();

    let mut cred_mgr = state.cred_mgr_mut();
    assert!(cred_mgr.change_user_cred(&[0x56; 16], &[0x57; 16]).is_ok());
    assert!(
        cred_mgr.user_cred()
            == UserCredential {
                id: [0x56; 16],
                pin: [0x57; 16],
                vault_id: 0
            }
    );

    cred_mgr.clear();
    assert!(
        cred_mgr.user_cred()
            == UserCredential {
                id: [0; 16],
                pin: [0; 16],
                vault_id: 0
            }
    );

    assert!(cred_mgr.change_user_cred(&[0x56; 16], &[0x57; 16]).is_ok());
    assert!(
        cred_mgr.user_cred()
            == UserCredential {
                id: [0x56; 16],
                pin: [0x57; 16],
                vault_id: 0
            }
    );
}

#[test]
fn test_rgs_reset() {
    let state = part_state();

    let mut rgs = state.rgs_mut();
    rgs.set_mask(0x2);

    // drop the rgs to borrow the rgs as mutable again
    drop(rgs);

    state.rgs_reset();

    assert_eq!(state.rgs().mask(), 0x0);
}

#[test]
fn test_nonce_including_reset() {
    let mut hal = MockHal::new();

    let mut rng_nonce_reset = MockRng::new();
    rng_nonce_reset.expect_bytes().times(1).returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);
    hal.expect_rng().times(1).return_const(rng_nonce_reset);

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

    let old_nonce = state.nonce();
    state.reset_nonce();
    let new_nonce = state.nonce();

    assert_ne!(new_nonce, old_nonce);

    let expected_old_nonce: [u8; 32] = [0; 32];
    let expected_new_nonce: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
    assert_eq!(old_nonce, expected_old_nonce);
    assert_eq!(new_nonce, expected_new_nonce);
}

#[test]
fn test_verify_user_cred_is_set() {
    let state = part_state();

    assert!(!state.verify_user_cred_is_set());

    assert!(state.change_user_cred(&[1; 16], &[2; 16]).is_ok());

    assert!(state.verify_user_cred_is_set());
}

#[test]
fn test_unwrapping_key_id() {
    let state = part_state();

    assert!(state.unwrapping_key_id().is_none());

    state.set_unwrapping_key_id(Some(0x10));

    assert_eq!(state.unwrapping_key_id(), Some(0x10));
}

#[test]
fn test_establish_cred_encryption_key_id() {
    let state = part_state();

    assert!(state.get_establish_cred_encryption_key_id().is_none());

    state.set_establish_cred_encryption_key_id(Some(0x10));

    assert_eq!(state.get_establish_cred_encryption_key_id(), Some(0x10));
}

#[test]
fn test_session_encryption_key_id() {
    let state = part_state();

    assert!(state.get_session_encryption_key_id().is_none());

    state.set_session_encryption_key_id(Some(0x10));

    assert_eq!(state.get_session_encryption_key_id(), Some(0x10));
}

#[test]
fn test_vm_launch_guid() {
    let mut hal = MockHal::new();

    pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;
    let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
    hal.expect_vault_addr()
        .return_const(table_memory.as_ptr() as usize);

    let cdma_vault_memory: [u8; 16384] = [0; 16384];
    hal.expect_cdma_vault_addr()
        .return_const(cdma_vault_memory.as_ptr() as usize);

    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);

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

    state.set_vm_launch_guid(&[1u8; 16]);
    assert_eq!(state.vm_launch_guid(), [1u8; 16]);
}
