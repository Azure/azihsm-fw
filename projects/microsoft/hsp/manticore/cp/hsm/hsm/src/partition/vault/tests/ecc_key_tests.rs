// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_types::DdiKeyUsage;

use crate::error::HsmErr;
use crate::partition::store::EntryAttributes;
use crate::partition::vault::key::EccKeyKind;
use crate::partition::vault::store::EntryKind;
use crate::partition::EccCurve;
use crate::partition::EccKey;
use crate::partition::EccKeyImported;
use crate::partition::EccKeyUsage;
use crate::partition::KeyStore;
use crate::partition::VaultKey;

#[test]
fn test_ecc_key_kind_priv_key_kind() {
    assert!(EccKeyKind::from(EccCurve::P256) == EccKeyKind::Ecc256Private);
    assert!(EccKeyKind::from(EccCurve::P384) == EccKeyKind::Ecc384Private);
    assert!(EccKeyKind::from(EccCurve::P521) == EccKeyKind::Ecc521Private);
}

#[test]
fn test_ecc_key_kind_into_entry_kind() {
    assert!(EntryKind::from(EccKeyKind::Ecc256Public) == EntryKind::Ecc256Public);
    assert!(EntryKind::from(EccKeyKind::Ecc384Public) == EntryKind::Ecc384Public);
    assert!(EntryKind::from(EccKeyKind::Ecc521Public) == EntryKind::Ecc521Public);

    assert!(EntryKind::from(EccKeyKind::Ecc256Private) == EntryKind::Ecc256Private);
    assert!(EntryKind::from(EccKeyKind::Ecc384Private) == EntryKind::Ecc384Private);
    assert!(EntryKind::from(EccKeyKind::Ecc521Private) == EntryKind::Ecc521Private);
}

#[test]
fn test_ecc_key_kind_into_usize() {
    assert_eq!(usize::from(EccKeyKind::Ecc256Public), 64);
    assert_eq!(usize::from(EccKeyKind::Ecc384Public), 96);
    assert_eq!(usize::from(EccKeyKind::Ecc521Public), 136);
    assert_eq!(usize::from(EccKeyKind::Ecc256Private), 32);
    assert_eq!(usize::from(EccKeyKind::Ecc384Private), 48);
    assert_eq!(usize::from(EccKeyKind::Ecc521Private), 68);
}

#[test]
fn test_ecc_key_usage_try_from_ddi_key_usage() {
    assert!(EccKeyUsage::try_from(DdiKeyUsage::SignVerify) == Ok(EccKeyUsage::SignVerify));

    assert!(EccKeyUsage::try_from(DdiKeyUsage::Unwrap) == Err(HsmErr::InvalidPermissions));
}

#[test]
fn test_ecc_key_try_from() {
    let store_memory = [0u32; (17 * 1024 * 65) / 4];
    let mut key_store = KeyStore::new(store_memory.as_ptr() as usize, 1);
    let sess_id_or_key_tag: u16 = 12;
    let app_id: u8 = 1;
    let blob = [0; 32];
    let mut attributes = EntryAttributes::default();
    attributes.common.flags.set_sign(true);
    attributes.common.flags.set_verify(true);

    // Add a valid ECC key.
    let key_id_result = key_store.add_entry(
        &attributes,
        sess_id_or_key_tag,
        EntryKind::Ecc256Private,
        app_id,
        &blob,
    );
    assert!(key_id_result.is_ok());
    let key_id = key_id_result.unwrap();
    let key = VaultKey::new(key_store, key_id);

    let result = EccKey::try_from(key);
    assert!(result.is_ok());
}

#[test]
fn test_ecc_key_try_from_invalid_class() {
    let store_memory = [0u32; (17 * 1024 * 65) / 4];
    let mut key_store = KeyStore::new(store_memory.as_ptr() as usize, 1);
    let sess_id_or_key_tag: u16 = 12;
    let app_id: u8 = 1;
    let blob = [0; 32];
    let mut attributes = EntryAttributes::default();
    attributes.common.flags.set_sign(true);
    attributes.common.flags.set_verify(true);

    // Add a valid ECC key.
    let key_id_result = key_store.add_entry(
        &attributes,
        sess_id_or_key_tag,
        EntryKind::Ecc256Private,
        app_id,
        &blob,
    );
    assert!(key_id_result.is_ok());
    let key_id = key_id_result.unwrap();
    let key = VaultKey::new(key_store, key_id + 1);

    let result = EccKey::try_from(key);
    assert!(result.is_err());
}

#[test]
fn test_ecc_key_try_from_err() {
    let store_memory = [0u32; (17 * 1024 * 65) / 4];
    let mut key_store = KeyStore::new(store_memory.as_ptr() as usize, 1);
    let sess_id_or_key_tag: u16 = 12;
    let app_id: u8 = 1;
    let blob = [0; 16];
    let mut attributes = EntryAttributes::default();
    attributes.common.flags.set_sign(true);
    attributes.common.flags.set_verify(true);

    // Add an invalid AES key.
    let invalid_key_id_result = key_store.add_entry(
        &attributes,
        sess_id_or_key_tag,
        EntryKind::Aes128,
        app_id,
        &blob,
    );
    assert!(invalid_key_id_result.is_ok());
    let invalid_key_id = invalid_key_id_result.unwrap();
    let invalid_key = VaultKey::new(key_store, invalid_key_id);
    let invalid_result = EccKey::try_from(invalid_key);
    assert!(invalid_result.is_err());
}

#[test]
fn test_ecc_key_usage_allowed() {
    let store_memory = [0u32; (17 * 1024 * 65) / 4];
    let mut key_store = KeyStore::new(store_memory.as_ptr() as usize, 1);
    let sess_id_or_key_tag: u16 = 12;
    let app_id: u8 = 1;
    let blob = [0; 32];

    let mut attributes = EntryAttributes::default();
    attributes.common.flags.set_sign(true);
    attributes.common.flags.set_verify(true);
    let key_id_result = key_store.add_entry(
        &attributes,
        sess_id_or_key_tag,
        EntryKind::Ecc256Private,
        app_id,
        &blob,
    );
    assert!(key_id_result.is_ok());
    let key_id = key_id_result.unwrap();
    let key = VaultKey::new(key_store, key_id);
    let ecc_key = EccKey::new(key);

    // Test for allowed usage
    let result = ecc_key.usage_allowed(EccKeyUsage::SignVerify);
    assert!(result.is_ok());

    // Test for disallowed usage
    let result = ecc_key.usage_allowed(EccKeyUsage::KeyAgreement);
    assert!(result.is_err());
    if let Err(err) = result {
        assert_eq!(err, HsmErr::InvalidPermissions)
    }
}

#[test]
fn test_ecc_key_usage_not_allowed() {
    let store_memory = [0u32; (17 * 1024 * 65) / 4];
    let mut key_store = KeyStore::new(store_memory.as_ptr() as usize, 1);
    let sess_id_or_key_tag: u16 = 12;
    let app_id: u8 = 1;
    let blob = [0; 32];

    let attributes = EntryAttributes::default();
    let key_id_result = key_store.add_entry(
        &attributes,
        sess_id_or_key_tag,
        EntryKind::Ecc256Private,
        app_id,
        &blob,
    );
    assert!(key_id_result.is_ok());
    let key_id = key_id_result.unwrap();
    let key = VaultKey::new(key_store, key_id);
    let ecc_key = EccKey::new(key);

    // Test for allowed usage
    let result = ecc_key.usage_allowed(EccKeyUsage::SignVerify);
    assert!(result.is_err());
    if let Err(err) = result {
        assert_eq!(err, HsmErr::InvalidPermissions);
    }
}

#[test]
fn test_ecc_key_imported_new() {
    let val = vec![0; usize::from(EccKeyKind::Ecc256Private)];
    let key = EccKeyImported::new(EccKeyKind::Ecc256Private, EccKeyUsage::SignVerify, &val);
    assert!(key.is_ok());

    let invalid_val = vec![0; usize::from(EccKeyKind::Ecc256Private) + 1];
    let invalid_key = EccKeyImported::new(
        EccKeyKind::Ecc256Private,
        EccKeyUsage::SignVerify,
        &invalid_val,
    );
    assert!(invalid_key.is_err());
}

#[test]
fn test_ecc_key_id() {
    let store_memory = [0u32; (17 * 1024 * 65) / 4];
    let mut key_store = KeyStore::new(store_memory.as_ptr() as usize, 1);
    let sess_id_or_key_tag: u16 = 12;
    let app_id: u8 = 1;
    let blob = [0; 32];
    let mut attributes = EntryAttributes::default();
    attributes.common.flags.set_sign(true);
    attributes.common.flags.set_verify(true);

    // Add a valid ECC key.
    let key_id_result = key_store.add_entry(
        &attributes,
        sess_id_or_key_tag,
        EntryKind::Ecc256Private,
        app_id,
        &blob,
    );
    assert!(key_id_result.is_ok());
    let key_id = key_id_result.unwrap();
    let key = VaultKey::new(key_store, key_id);
    let ecc_key = EccKey::new(key);

    let id = ecc_key.id();
    assert_eq!(key_id, id);
}

#[test]
fn test_ecc_key_blob() {
    let key_kind = EccKeyKind::Ecc256Private;
    let key_usage = EccKeyUsage::SignVerify;
    let key_blob = [1; 32];

    let ecc_key = EccKeyImported::new(key_kind, key_usage, &key_blob).unwrap();

    assert_eq!(ecc_key.blob(), &key_blob[..usize::from(key_kind)]);
}

#[test]
fn test_ecc_key_kind() {
    let kind = EccKeyKind::Ecc384Private;
    let usage = EccKeyUsage::SignVerify;
    let val = [1; 48];
    let ecc_key = EccKeyImported::new(kind, usage, &val).unwrap();

    assert!(ecc_key.kind() == kind);
}

#[test]
fn test_ecc_key_usage() {
    let kind = EccKeyKind::Ecc384Public;
    let usage = EccKeyUsage::KeyAgreement;
    let val = [1; 96];
    let ecc_key = EccKeyImported::new(kind, usage, &val).unwrap();

    assert!(ecc_key.usage() == usage);
}
