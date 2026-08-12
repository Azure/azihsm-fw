// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_types::DdiKeyUsage;

use crate::error::HsmErr;
use crate::partition::store::EntryAttributes;
use crate::partition::vault::key::HmacKeyKind;
use crate::partition::vault::key::VarLenHmacShaKeyKind;
use crate::partition::vault::store::EntryKind;
use crate::partition::HmacKey;
use crate::partition::HmacKeyImported;
use crate::partition::HmacKeyUsage;
use crate::partition::KeyAvailability;
use crate::partition::KeyStore;
use crate::partition::KeyVault;
use crate::partition::VarLenHmacShaKey;
use crate::partition::VarLenHmacShaKeyImported;
use crate::partition::VaultKey;

// ---------------------------------------------------------------------------
// TryFrom<DdiKeyUsage> for HmacKeyUsage
// ---------------------------------------------------------------------------

#[test]
fn test_hmac_key_usage_try_from_ddi_key_usage() {
    // SignVerify -> SignVerify
    assert!(HmacKeyUsage::try_from(DdiKeyUsage::SignVerify) == Ok(HmacKeyUsage::SignVerify));

    // Derive -> Derive (the newly-supported mapping)
    assert!(HmacKeyUsage::try_from(DdiKeyUsage::Derive) == Ok(HmacKeyUsage::Derive));

    // Unsupported usages must still be rejected with InvalidPermissions so
    // that a future regression of the mapping is caught.
    assert!(HmacKeyUsage::try_from(DdiKeyUsage::Unwrap) == Err(HsmErr::InvalidPermissions));
    assert!(HmacKeyUsage::try_from(DdiKeyUsage::EncryptDecrypt) == Err(HsmErr::InvalidPermissions));
}

// ---------------------------------------------------------------------------
// HmacKeyImported::new — creation with Derive usage succeeds
// ---------------------------------------------------------------------------

#[test]
fn test_hmac_key_imported_new_derive_usage_succeeds() {
    let val = [0u8; 32];
    let key = HmacKeyImported::new(HmacKeyKind::HmacKey256, HmacKeyUsage::Derive, &val);
    assert!(
        key.is_ok(),
        "HMAC key creation with Derive usage must succeed"
    );
    let key = key.unwrap();
    assert!(matches!(key.usage(), HmacKeyUsage::Derive));
}

// ---------------------------------------------------------------------------
// HmacKey::usage_allowed — Derive branch
// ---------------------------------------------------------------------------

#[test]
fn test_hmac_key_usage_allowed_derive_only() {
    let store_memory = [0u32; (17 * 1024 * 65) / 4];
    let mut key_store = KeyStore::new(store_memory.as_ptr() as usize, 1);
    let sess_id_or_key_tag: u16 = 12;
    let app_id: u8 = 1;
    let blob = [0u8; 32];

    // Key persisted with only the derive flag set.
    let mut attributes = EntryAttributes::default();
    attributes.common.flags.set_derive(true);

    let key_id = key_store
        .add_entry(
            &attributes,
            sess_id_or_key_tag,
            EntryKind::HmacSha256,
            app_id,
            &blob,
        )
        .expect("add_entry");
    let key = VaultKey::new(key_store, key_id);
    let hmac_key = HmacKey::new(key);

    // Derive intent is allowed.
    assert!(hmac_key.usage_allowed(HmacKeyUsage::Derive).is_ok());

    // The critical negative case: hmac() opens the key with SignVerify intent,
    // which must be rejected on a derive-only key.
    let err = hmac_key
        .usage_allowed(HmacKeyUsage::SignVerify)
        .expect_err("SignVerify on derive-only HMAC key must be rejected");
    assert_eq!(err, HsmErr::InvalidPermissions);
}

#[test]
fn test_hmac_key_usage_allowed_sign_verify_only() {
    let store_memory = [0u32; (17 * 1024 * 65) / 4];
    let mut key_store = KeyStore::new(store_memory.as_ptr() as usize, 1);
    let sess_id_or_key_tag: u16 = 12;
    let app_id: u8 = 1;
    let blob = [0u8; 32];

    // Key persisted with only sign/verify flags (no derive).
    let mut attributes = EntryAttributes::default();
    attributes.common.flags.set_sign(true);
    attributes.common.flags.set_verify(true);

    let key_id = key_store
        .add_entry(
            &attributes,
            sess_id_or_key_tag,
            EntryKind::HmacSha256,
            app_id,
            &blob,
        )
        .expect("add_entry");
    let key = VaultKey::new(key_store, key_id);
    let hmac_key = HmacKey::new(key);

    // SignVerify is allowed.
    assert!(hmac_key.usage_allowed(HmacKeyUsage::SignVerify).is_ok());

    // Derive is rejected when the derive flag is not set.
    let err = hmac_key
        .usage_allowed(HmacKeyUsage::Derive)
        .expect_err("Derive on sign/verify-only HMAC key must be rejected");
    assert_eq!(err, HsmErr::InvalidPermissions);
}

// ---------------------------------------------------------------------------
// VarLenHmacShaKeyImported::new — creation with Derive usage succeeds
// ---------------------------------------------------------------------------

#[test]
fn test_var_len_hmac_key_imported_new_derive_usage_succeeds() {
    let val = [0u8; 32]; // min length for VarLenHmacShaKey256
    let key = VarLenHmacShaKeyImported::new(
        VarLenHmacShaKeyKind::VarLenHmacShaKey256,
        HmacKeyUsage::Derive,
        &val,
    );
    assert!(
        key.is_ok(),
        "Var-len HMAC key creation with Derive usage must succeed"
    );
    let key = key.unwrap();
    assert!(matches!(key.usage(), HmacKeyUsage::Derive));
}

// ---------------------------------------------------------------------------
// VarLenHmacShaKey::usage_allowed — Derive branch
// ---------------------------------------------------------------------------

#[test]
fn test_var_len_hmac_key_usage_allowed_derive_only() {
    let store_memory = [0u32; (17 * 1024 * 65) / 4];
    let mut key_store = KeyStore::new(store_memory.as_ptr() as usize, 1);
    let sess_id_or_key_tag: u16 = 12;
    let app_id: u8 = 1;
    let blob = [0u8; 32];

    let mut attributes = EntryAttributes::default();
    attributes.common.flags.set_derive(true);
    attributes.entry_specific[VarLenHmacShaKey::KEY_LENGTH_INDEX] = blob.len() as u8;

    let key_id = key_store
        .add_entry(
            &attributes,
            sess_id_or_key_tag,
            EntryKind::VarLenHmacSha256,
            app_id,
            &blob,
        )
        .expect("add_entry");
    let key = VaultKey::new(key_store, key_id);
    let hmac_key = VarLenHmacShaKey::new(key);

    assert!(hmac_key.usage_allowed(HmacKeyUsage::Derive).is_ok());

    let err = hmac_key
        .usage_allowed(HmacKeyUsage::SignVerify)
        .expect_err("SignVerify on derive-only var-len HMAC key must be rejected");
    assert_eq!(err, HsmErr::InvalidPermissions);
}

#[test]
fn test_var_len_hmac_key_usage_allowed_sign_verify_only() {
    let store_memory = [0u32; (17 * 1024 * 65) / 4];
    let mut key_store = KeyStore::new(store_memory.as_ptr() as usize, 1);
    let sess_id_or_key_tag: u16 = 12;
    let app_id: u8 = 1;
    let blob = [0u8; 32];

    let mut attributes = EntryAttributes::default();
    attributes.common.flags.set_sign(true);
    attributes.common.flags.set_verify(true);
    attributes.entry_specific[VarLenHmacShaKey::KEY_LENGTH_INDEX] = blob.len() as u8;

    let key_id = key_store
        .add_entry(
            &attributes,
            sess_id_or_key_tag,
            EntryKind::VarLenHmacSha256,
            app_id,
            &blob,
        )
        .expect("add_entry");
    let key = VaultKey::new(key_store, key_id);
    let hmac_key = VarLenHmacShaKey::new(key);

    assert!(hmac_key.usage_allowed(HmacKeyUsage::SignVerify).is_ok());

    let err = hmac_key
        .usage_allowed(HmacKeyUsage::Derive)
        .expect_err("Derive on sign/verify-only var-len HMAC key must be rejected");
    assert_eq!(err, HsmErr::InvalidPermissions);
}

// ===========================================================================
// Integration tests — drive the actual import functions in mod.rs so the
// HmacKeyUsage::Derive => set_derive(true) glue is exercised end-to-end.
// A regression in that seam (wrong flag set, or set_derive omitted) would
// otherwise leave the unit tests above green while the real import path
// is broken.
// ===========================================================================

const APP: u8 = 1;
const SESSION: u16 = 12;
const VAULT_MASK: u128 = 0b1010110;

#[test]
fn test_hmac_import_key_with_derive_usage_end_to_end() {
    let store_memory = [0u32; (17 * 1024 * 65) / 4];
    let mut vault = KeyVault::new(store_memory.as_ptr() as usize, VAULT_MASK);

    let blob = [0u8; 32];
    let imported =
        HmacKeyImported::new(HmacKeyKind::HmacKey256, HmacKeyUsage::Derive, &blob).unwrap();

    let hmac_key = vault
        .hmac_import_key(
            APP,
            SESSION,
            None,
            false,
            &imported,
            KeyAvailability::Session,
        )
        .expect("HMAC import with Derive usage must succeed");
    let key_id = hmac_key.id();

    // Re-open the persisted key and assert the derive flag survived the
    // import glue in mod.rs.
    let reopened = vault
        .hmac_key(APP, SESSION, key_id, Some(HmacKeyUsage::Derive))
        .expect("re-open with Derive intent must succeed");
    assert!(reopened.usage_allowed(HmacKeyUsage::Derive).is_ok());

    // The critical negative case: hmac() opens the key with SignVerify
    // intent, which must be rejected because sign/verify flags were NOT
    // set by the import path for a Derive-usage key.
    let res = vault.hmac_key(APP, SESSION, key_id, Some(HmacKeyUsage::SignVerify));
    assert!(
        matches!(res.as_ref().err(), Some(&HsmErr::InvalidPermissions)),
        "hmac() intent on derive-only imported key must be rejected with InvalidPermissions"
    );
}

#[test]
fn test_hmac_import_key_with_sign_verify_usage_end_to_end() {
    let store_memory = [0u32; (17 * 1024 * 65) / 4];
    let mut vault = KeyVault::new(store_memory.as_ptr() as usize, VAULT_MASK);

    let blob = [0u8; 32];
    let imported =
        HmacKeyImported::new(HmacKeyKind::HmacKey256, HmacKeyUsage::SignVerify, &blob).unwrap();

    let hmac_key = vault
        .hmac_import_key(
            APP,
            SESSION,
            None,
            false,
            &imported,
            KeyAvailability::Session,
        )
        .expect("HMAC import with SignVerify usage must succeed");
    let key_id = hmac_key.id();

    // Reopen: SignVerify intent OK, Derive intent rejected — proves the
    // inverse glue (sign/verify flags set, derive flag NOT set).
    assert!(vault
        .hmac_key(APP, SESSION, key_id, Some(HmacKeyUsage::SignVerify))
        .is_ok());
    let res = vault.hmac_key(APP, SESSION, key_id, Some(HmacKeyUsage::Derive));
    assert!(
        matches!(res.as_ref().err(), Some(&HsmErr::InvalidPermissions)),
        "Derive intent on sign/verify-only imported key must be rejected"
    );
}

#[test]
fn test_import_var_hmac_key_with_derive_usage_end_to_end() {
    let store_memory = [0u32; (17 * 1024 * 65) / 4];
    let mut vault = KeyVault::new(store_memory.as_ptr() as usize, VAULT_MASK);

    let blob = [0u8; 32]; // min length for VarLenHmacShaKey256
    let imported = VarLenHmacShaKeyImported::new(
        VarLenHmacShaKeyKind::VarLenHmacShaKey256,
        HmacKeyUsage::Derive,
        &blob,
    )
    .unwrap();

    let hmac_key = vault
        .import_var_hmac_key(
            APP,
            SESSION,
            None,
            false,
            &imported,
            KeyAvailability::Session,
        )
        .expect("var-len HMAC import with Derive usage must succeed");
    let key_id = hmac_key.id();

    let reopened = vault
        .var_hmac_key(APP, SESSION, key_id, Some(HmacKeyUsage::Derive))
        .expect("re-open with Derive intent must succeed");
    assert!(reopened.usage_allowed(HmacKeyUsage::Derive).is_ok());

    let res = vault.var_hmac_key(APP, SESSION, key_id, Some(HmacKeyUsage::SignVerify));
    assert!(
        matches!(res.as_ref().err(), Some(&HsmErr::InvalidPermissions)),
        "hmac() intent on derive-only imported var-len key must be rejected"
    );
}

#[test]
fn test_import_var_hmac_key_with_sign_verify_usage_end_to_end() {
    let store_memory = [0u32; (17 * 1024 * 65) / 4];
    let mut vault = KeyVault::new(store_memory.as_ptr() as usize, VAULT_MASK);

    let blob = [0u8; 32];
    let imported = VarLenHmacShaKeyImported::new(
        VarLenHmacShaKeyKind::VarLenHmacShaKey256,
        HmacKeyUsage::SignVerify,
        &blob,
    )
    .unwrap();

    let hmac_key = vault
        .import_var_hmac_key(
            APP,
            SESSION,
            None,
            false,
            &imported,
            KeyAvailability::Session,
        )
        .expect("var-len HMAC import with SignVerify usage must succeed");
    let key_id = hmac_key.id();

    assert!(vault
        .var_hmac_key(APP, SESSION, key_id, Some(HmacKeyUsage::SignVerify))
        .is_ok());
    let res = vault.var_hmac_key(APP, SESSION, key_id, Some(HmacKeyUsage::Derive));
    assert!(
        matches!(res.as_ref().err(), Some(&HsmErr::InvalidPermissions)),
        "Derive intent on sign/verify-only imported var-len key must be rejected"
    );
}
