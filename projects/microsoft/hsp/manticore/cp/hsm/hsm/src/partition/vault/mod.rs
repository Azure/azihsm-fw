// Copyright (c) Microsoft Corporation. All rights reserved.

mod key;
pub(crate) mod store;
mod tests;

use alloc::rc::Rc;
use core::cell::RefCell;

pub(crate) use self::key::*;
pub(crate) use self::store::EntryClass;
pub(crate) use self::store::EntryFlags;
pub(crate) use self::store::EntryKind;
pub(crate) use self::store::KeyStore;
use super::*;
use crate::error::*;
use crate::partition::store::EntryAttributes;

const KEY_TAG_UNASSIGNED: u16 = 0;

#[derive(Clone)]
pub(crate) struct KeyVault {
    rimpl: Rc<RefCell<KeyVaultImpl>>,
}

#[allow(unused)]
impl KeyVault {
    pub(crate) fn new(vault_addr: usize, mask: u128) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(KeyVaultImpl::new(KeyStore::new(
                vault_addr, mask,
            )))),
        }
    }

    /// Fetch the key number of the entry with the given key tag for the specified application
    /// from the store.
    ///
    /// # Arguments
    /// * `app_id` - The app ID.
    /// * `key_tag` - The key tag.
    ///
    /// # Returns
    ///
    /// * `Ok(key_number)` containing key number of the entry with the given key tag for the
    ///   specified application if successful, else `Err(HsmErr)`.
    pub fn get_entry_index_by_tag(&self, app_id: u8, key_tag: u16) -> Result<u16, HsmErr> {
        self.rimpl.borrow().get_entry_index_by_tag(app_id, key_tag)
    }

    /// Enable key
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `key_id` - Key ID
    ///
    /// # Return
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    pub fn enable_key(&self, key_id: KeyId) -> Result<(), HsmErr> {
        self.rimpl.borrow_mut().enable_key(key_id)
    }

    /// Disable key
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `key_id` - Key ID
    ///
    /// # Return
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    pub fn disable_key(&self, key_id: KeyId) -> Result<(), HsmErr> {
        self.rimpl.borrow_mut().disable_key(key_id)
    }

    /// Delete key
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `key_id` - Key ID
    /// * `predicate` - Predicate to check if key is in use
    ///
    /// # Return
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    pub fn delete_key<F>(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: KeyId,
        predicate: F,
    ) -> Result<(), HsmErr>
    where
        F: Fn(KeyId) -> bool,
    {
        self.rimpl
            .borrow_mut()
            .delete_key(app, session, key_id, predicate)
    }

    /// Delete all session only entries for the given session ID from the store.
    ///
    /// # Arguments
    ///
    /// * `session` - Session ID
    /// * `predicate` - Predicate to check if key should be deleted
    ///
    /// # Return
    ///
    /// * Returns `Ok(())` on success else `Err(HsmError)` if failed
    pub fn delete_all_session_keys<F>(
        &mut self,
        session: SessionId,
        predicate: F,
    ) -> Result<(), HsmErr>
    where
        F: Fn(u16) -> bool,
    {
        self.rimpl
            .borrow_mut()
            .delete_all_session_keys(session, predicate)
    }

    /// Delete all app keys for the given app ID from the store.
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `predicate` - Predicate to check if key should be deleted
    ///
    /// # Return
    ///
    /// * Returns `Ok(())` on success else `Err(HsmError)` if failed
    pub fn delete_all_app_keys<F>(&mut self, app: AppVaultId, predicate: F) -> Result<(), HsmErr>
    where
        F: Fn(u16) -> bool,
    {
        self.rimpl.borrow_mut().delete_all_app_keys(app, predicate)
    }

    /// Import AES key into the key store with the given attributes.
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `key_tag` - Key tag
    /// * `key` - AES key to import
    /// * `attributes` - Entry attributes for the key
    ///
    /// # Return
    ///
    /// * AES key
    pub fn aes_import_key(
        &mut self,
        app: AppVaultId,
        session: SessionId,
        key_tag: Option<u16>,
        key: &AesKeyImported,
        attributes: &EntryAttributes,
    ) -> HsmResult<AesKey> {
        self.rimpl
            .borrow_mut()
            .aes_import_key(app, session, key_tag, key, attributes)
    }

    /// Open AES key
    ///
    /// # Arguments
    ///
    /// * `key_id` - Key ID
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `intent` - Key usage intent
    ///
    /// # Return
    ///
    /// * AES key
    pub fn aes_key(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: u16,
        intent: AesKeyUsage,
    ) -> HsmResult<AesKey> {
        self.rimpl
            .borrow()
            .aes_open_key(app, session, key_id, intent)
    }

    /// Get key with given key ID without verifying its existence in the vault.
    ///
    /// # Arguments
    ///
    /// * `key_id` - The key ID of the key to retrieve
    ///
    /// # Returns
    ///
    /// * The key with the given key ID
    pub fn key_unchecked(&self, key_id: u16) -> VaultKey {
        self.rimpl.borrow().key_unchecked(key_id)
    }

    /// Get key with given key ID with permission checks in the vault.
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `key_id` - The key ID of the key to retrieve
    ///
    /// # Returns
    ///
    /// * The key with the given key ID
    pub fn key(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: u16,
        allow_disabled: bool,
    ) -> HsmResult<VaultKey> {
        self.rimpl
            .borrow()
            .key(app, session, key_id, allow_disabled)
    }

    /// Clear all the key context in physical vault
    /// # Caution
    ///
    /// This function deletes everything in this vault.
    pub(crate) fn clear(&self) {
        self.rimpl.borrow_mut().clear()
    }

    /// Validate the key parameters
    ///
    /// # Arguments
    ///
    /// * `availability` - Key availability
    /// * `tag` - Key tag
    ///
    /// # Return
    ///
    /// * Result - Ok(()) if key parameters are valid. Error code otherwise.
    pub fn validate_key_params(
        &mut self,
        availability: KeyAvailability,
        tag: Option<u16>,
    ) -> HsmResult<()> {
        self.rimpl
            .borrow_mut()
            .validate_key_params(availability, tag)
    }

    /// Import ECC key into the key store
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `name` - Key tag
    /// * `generated` - Is key generated
    /// * `key` - ECC key to import
    /// * `availability` - Key availability
    ///
    /// # Return
    ///
    /// * ECC key
    pub fn ecc_import_key(
        &mut self,
        app: AppVaultId,
        session: SessionId,
        key_tag: Option<u16>,
        generated: bool,
        key: &EccKeyImported,
        availability: KeyAvailability,
    ) -> HsmResult<EccKey> {
        self.rimpl
            .borrow_mut()
            .ecc_import_key(app, session, key_tag, generated, key, availability)
    }

    /// Open ECC key
    ///
    /// # Arguments
    ///
    /// * `key_id` - Key ID
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `intent` - Optional Key usage intent
    ///
    /// # Return
    ///
    /// * ECC key
    pub fn ecc_key(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: u16,
        intent: Option<EccKeyUsage>,
    ) -> HsmResult<EccKey> {
        self.rimpl
            .borrow()
            .ecc_open_key(app, session, key_id, intent)
    }

    /// Import ECDH secret key into the key store
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `name` - Key tag
    /// * `generated` - Is key generated
    /// * `key` - ECC key to import
    /// * `availability` - Key availability
    ///
    /// # Return
    ///
    /// * ECDH key
    pub fn ecdh_import_key(
        &mut self,
        app: AppVaultId,
        session: SessionId,
        key_tag: Option<u16>,
        generated: bool,
        key: &EcdhKeyImported,
        availability: KeyAvailability,
    ) -> HsmResult<EcdhKey> {
        self.rimpl
            .borrow_mut()
            .ecdh_import_key(app, session, key_tag, generated, key, availability)
    }

    /// Open ECDH key
    ///
    /// # Arguments
    ///
    /// * `key_id` - Key ID
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `intent` - Optional Key usage intent
    ///
    /// # Return
    ///
    /// * ECDH key
    pub fn ecdh_key(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: u16,
        intent: Option<EcdhKeyUsage>,
    ) -> HsmResult<EcdhKey> {
        self.rimpl
            .borrow()
            .ecdh_open_key(app, session, key_id, intent)
    }

    /// Import HMAC key into the key store
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `name` - Key tag
    /// * `generated` - Is key generated
    /// * `key` - ECC key to import
    /// * `availability` - Key availability
    ///
    /// # Return
    ///
    /// * HMAC key
    pub fn hmac_import_key(
        &mut self,
        app: AppVaultId,
        session: SessionId,
        key_tag: Option<u16>,
        generated: bool,
        key: &HmacKeyImported,
        availability: KeyAvailability,
    ) -> HsmResult<HmacKey> {
        self.rimpl
            .borrow_mut()
            .hmac_import_key(app, session, key_tag, generated, key, availability)
    }

    /// Open Hmac key
    ///
    /// # Arguments
    ///
    /// * `key_id` - Key ID
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `intent` - Optional Key usage intent
    ///
    /// # Return
    ///
    /// * HMAC key
    pub fn hmac_key(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: u16,
        intent: Option<HmacKeyUsage>,
    ) -> HsmResult<HmacKey> {
        self.rimpl
            .borrow()
            .hmac_open_key(app, session, key_id, intent)
    }

    /// Import RSA key into the key store
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `name` - Key tag
    /// * `generated` - Is key generated
    /// * `key` - RSA key to import
    /// * `availability` - Key availability
    ///
    /// # Return
    ///
    /// * RSA key
    pub fn rsa_import_key(
        &mut self,
        app: AppVaultId,
        session: SessionId,
        key_tag: Option<u16>,
        generated: bool,
        key: &RsaKeyImported,
        availability: KeyAvailability,
    ) -> HsmResult<RsaKey> {
        self.rimpl
            .borrow_mut()
            .rsa_import_key(app, session, key_tag, generated, key, availability)
    }

    /// Open RSA key
    ///
    /// # Arguments
    ///
    /// * `key_id` - Key ID
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `intent` - Optional Key usage intent
    ///
    /// # Return
    ///
    /// * RSA key
    pub fn rsa_key(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: u16,
        intent: Option<RsaKeyUsage>,
    ) -> HsmResult<RsaKey> {
        self.rimpl
            .borrow()
            .rsa_open_key(app, session, key_id, intent)
    }

    /// Import Establish Cred Encryption Key (Ecc384 Public + Private) into the store
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `key` - Key to import
    /// * `availability` - Key availability
    ///
    /// # Return
    ///
    /// * Establish Cred Encryption key
    pub fn import_establish_cred_encryption_key(
        &mut self,
        app: AppVaultId,
        key: &EstablishCredEncryptionKeyToImport,
        availability: KeyAvailability,
    ) -> HsmResult<EstablishCredEncryptionKey> {
        self.rimpl
            .borrow_mut()
            .import_establish_cred_encryption_key(app, key)
    }

    /// Open Establish Cred Encryption Key (Ecc384 Public + Private)
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `key_id` - Key ID
    ///
    /// # Return
    ///
    /// * Establish Cred Encryption key
    pub fn open_establish_cred_encryption_key(
        &self,
        app: AppVaultId,
        key_id: u16,
    ) -> HsmResult<EstablishCredEncryptionKey> {
        self.rimpl
            .borrow()
            .open_establish_cred_encryption_key(app, key_id)
    }

    /// Import Session Encryption Key (Ecc384 Public + Private) into the store
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `key` - Key to import
    /// * `availability` - Key availability
    ///
    /// # Return
    ///
    /// * Session Encryption key
    pub fn import_session_encryption_key(
        &mut self,
        app: AppVaultId,
        key: &SessionEncryptionKeyToImport,
        availability: KeyAvailability,
    ) -> HsmResult<SessionEncryptionKey> {
        self.rimpl
            .borrow_mut()
            .import_session_encryption_key(app, key)
    }

    /// Open Session Encryption Key (Ecc384 Public + Private)
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `key_id` - Key ID
    ///
    /// # Return
    ///
    /// * Session Encryption key
    pub fn open_session_encryption_key(
        &self,
        app: AppVaultId,
        key_id: u16,
    ) -> HsmResult<SessionEncryptionKey> {
        self.rimpl.borrow().open_session_encryption_key(app, key_id)
    }

    /// Import session into the key store
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `key` - Session
    ///
    /// # Return
    ///
    /// * ECC key
    pub fn import_session_key(
        &mut self,
        app: AppVaultId,
        key: &SessionKeyToImport,
    ) -> HsmResult<SessionKey> {
        self.rimpl.borrow_mut().import_session_key(app, key)
    }

    /// Open session
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `key_id` - Session ID
    ///
    /// # Return
    ///
    /// * Session Encryption key
    pub fn open_session_key(
        &self,
        app: AppVaultId,
        key_id: u16,
        allow_disabled: bool,
    ) -> HsmResult<SessionKey> {
        self.rimpl
            .borrow()
            .open_session_key(app, key_id, allow_disabled)
    }

    /// Import the raw key into the vault
    ///
    /// # Arguments
    ///
    /// * `app_id` - Application ID
    /// * `session` - Session ID
    /// * `key_tag` - Key tag
    /// * `kind` - Entry kind
    /// * `attributes` - Entry atttributes
    /// * `blob` - Raw blob of the key
    ///
    /// # Return
    ///
    /// * Key id
    pub fn import_raw_key(
        &mut self,
        app_id: u8,
        session: SessionId,
        key_tag: Option<u16>,
        kind: EntryKind,
        attributes: &EntryAttributes,
        blob: &[u8],
    ) -> HsmResult<u16> {
        self.rimpl
            .borrow_mut()
            .import_raw_key(app_id, session, key_tag, kind, attributes, blob)
    }

    /// Import Masking Key (Ecc384 Public + Private) into the store
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `key` - Key to import
    /// * `availability` - Key availability
    ///
    /// # Return
    ///
    /// * Masking key
    pub fn import_masking_key(
        &mut self,
        app: AppVaultId,
        key: &MaskingKeyToImport,
        availability: KeyAvailability,
    ) -> HsmResult<MaskingKey> {
        self.rimpl.borrow_mut().import_masking_key(app, key)
    }

    /// Open Masking Key (Ecc384 Public + Private)
    ///
    /// # Arguments
    ///
    /// * `app` - Application ID
    /// * `key_id` - Key ID
    ///
    /// # Return
    ///
    /// * Masking key
    pub fn open_masking_key(&self, app: AppVaultId, key_id: u16) -> HsmResult<MaskingKey> {
        self.rimpl.borrow().open_masking_key(app, key_id)
    }

    /// Import Variable Length HMAC key into the key store
    ///
    /// # Arguments
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `key_tag` - Key tag
    /// * `generated` - Is key generated
    /// * `key` - Variable Length HMAC key to import
    /// * `availability` - Key availability
    /// # Return
    /// * Variable Length HMAC key
    pub fn import_var_hmac_key(
        &mut self,
        app: AppVaultId,
        session: SessionId,
        key_tag: Option<u16>,
        generated: bool,
        key: &VarLenHmacShaKeyImported,
        availability: KeyAvailability,
    ) -> HsmResult<VarLenHmacShaKey> {
        self.rimpl.borrow_mut().import_var_hmac_key(
            app,
            session,
            key_tag,
            generated,
            key,
            availability,
        )
    }

    /// Open Variable Length HMAC key
    ///
    /// # Arguments
    /// * `key_id` - Key ID
    /// * `app` - Application ID
    /// * `session` - Session ID
    /// * `intent` - Optional Key usage intent
    ///
    /// # Return
    ///
    /// * Variable Length HMAC key
    pub fn var_hmac_key(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: u16,
        intent: Option<HmacKeyUsage>,
    ) -> HsmResult<VarLenHmacShaKey> {
        self.rimpl
            .borrow()
            .var_hmac_open_key(app, session, key_id, intent)
    }
}

struct KeyVaultImpl {
    key_store: KeyStore,
}

impl KeyVaultImpl {
    pub fn new(key_store: KeyStore) -> Self {
        Self { key_store }
    }

    /// Fetch the key number of the entry with the given key tag for the specified application from
    /// the store.
    fn get_entry_index_by_tag(&self, app_id: u8, key_tag: u16) -> Result<u16, HsmErr> {
        self.key_store.get_entry_index_by_tag(app_id, key_tag)
    }

    /// Enable key
    fn enable_key(&mut self, key_id: KeyId) -> Result<(), HsmErr> {
        self.key_store.enable_entry(key_id)
    }

    /// Disable key
    fn disable_key(&mut self, key_id: KeyId) -> Result<(), HsmErr> {
        self.key_store.disable_entry(key_id)
    }

    /// Delete key
    fn delete_key<F>(
        &mut self,
        app: AppVaultId,
        session: SessionId,
        key_id: KeyId,
        predicate: F,
    ) -> Result<(), HsmErr>
    where
        F: Fn(u16) -> bool,
    {
        self.key(app, session, key_id, true)?;
        self.key_store.remove_entry(key_id, predicate)
    }

    /// Delete all session keys
    fn delete_all_session_keys<F>(&mut self, session: SessionId, predicate: F) -> Result<(), HsmErr>
    where
        F: Fn(u16) -> bool,
    {
        self.key_store
            .remove_all_session_only_entries(predicate, session)?;
        Ok(())
    }

    /// Delete all app keys
    fn delete_all_app_keys<F>(&mut self, app: AppVaultId, predicate: F) -> Result<(), HsmErr>
    where
        F: Fn(u16) -> bool,
    {
        self.key_store.remove_all_entries_for_app(predicate, app)?;
        Ok(())
    }

    /// Clear the key vault
    pub(crate) fn clear(&mut self) {
        self.key_store.nuke()
    }

    /// Import AES key into the key store with the given attributes.
    fn aes_import_key(
        &mut self,
        app: AppVaultId,
        session: SessionId,
        key_tag: Option<u16>,
        key: &AesKeyImported,
        attributes: &EntryAttributes,
    ) -> HsmResult<AesKey> {
        // Validate that attributes are consistent with the key's declared usage
        match key.usage() {
            AesKeyUsage::EncryptDecrypt => {
                if !attributes.common.flags.encrypt() || !attributes.common.flags.decrypt() {
                    return Err(HsmErr::InvalidArgument);
                }
            }
        }

        let availability = if attributes.common.flags.session() {
            KeyAvailability::Session
        } else {
            KeyAvailability::App
        };

        self.validate_key_params(availability, key_tag)?;

        let sess_id_or_tag: u16 = if availability == KeyAvailability::Session {
            session
        } else {
            key_tag.unwrap_or(KEY_TAG_UNASSIGNED)
        };

        let key_id = self.key_store.add_entry(
            attributes,
            sess_id_or_tag,
            key.kind().into(),
            app,
            key.blob(),
        )?;

        Ok(self.aes_key_unchecked(key_id))
    }

    /// Open key
    fn key(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: u16,
        allow_disabled: bool,
    ) -> HsmResult<VaultKey> {
        let entry = self.key_store.get_entry(key_id, allow_disabled)?;

        if entry.app_id() != app {
            Err(HsmErr::InvalidPermissions)?;
        }

        if entry.flags().session() && entry.session_id() != Some(session) {
            Err(HsmErr::InvalidPermissions)?
        }

        Ok(VaultKey::new(self.key_store.clone(), key_id))
    }

    /// Open AES key
    fn aes_open_key(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: u16,
        intent: AesKeyUsage,
    ) -> HsmResult<AesKey> {
        let aes_key = AesKey::try_from(self.key(app, session, key_id, false)?)?;

        aes_key.usage_allowed(intent)?;

        Ok(aes_key)
    }

    fn aes_key_unchecked(&self, key_id: u16) -> AesKey {
        AesKey::new(self.key_unchecked(key_id))
    }

    fn key_unchecked(&self, key_id: u16) -> VaultKey {
        VaultKey::new(self.key_store.clone(), key_id)
    }

    fn validate_key_params(
        &mut self,
        availability: KeyAvailability,
        tag: Option<u16>,
    ) -> HsmResult<()> {
        // Session only keys cannot have a key tag.
        if availability == KeyAvailability::Session && tag.is_some() {
            Err(HsmErr::InvalidArgument)?
        }

        // If a key tag is provided, it must not be KEY_TAG_UNASSIGNED.
        if let Some(key_tag_val) = tag {
            if key_tag_val == KEY_TAG_UNASSIGNED {
                Err(HsmErr::InvalidArgument)?
            }
        }

        Ok(())
    }

    /// Import ECC key into the key store
    fn ecc_import_key(
        &mut self,
        app: AppVaultId,
        session: SessionId,
        key_tag: Option<u16>,
        generated: bool,
        key: &EccKeyImported,
        availability: KeyAvailability,
    ) -> HsmResult<EccKey> {
        let mut attributes = EntryAttributes::default();

        self.validate_key_params(availability, key_tag)?;

        attributes
            .common
            .flags
            .set_session(availability == KeyAvailability::Session);

        // If a key is available as session-only, it should not have a key tag.
        // App keys can optionally have a key tag. If no name is specified,
        // KEY_TAG_UNASSIGNED will be assigned.
        let sess_id_or_tag: u16 = if availability == KeyAvailability::Session {
            session
        } else {
            key_tag.unwrap_or(KEY_TAG_UNASSIGNED)
        };

        if generated {
            attributes.common.flags.set_local(true);
        }

        match key.usage() {
            EccKeyUsage::SignVerify => {
                attributes.common.flags.set_sign(true);
                attributes.common.flags.set_verify(true);
            }
            EccKeyUsage::KeyAgreement => attributes.common.flags.set_derive(true),
        }

        let key_id = self.key_store.add_entry(
            &attributes,
            sess_id_or_tag,
            key.kind().into(),
            app,
            key.blob(),
        )?;

        Ok(self.ecc_key_unchecked(key_id))
    }

    /// Open ECC key
    fn ecc_open_key(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: u16,
        intent: Option<EccKeyUsage>,
    ) -> HsmResult<EccKey> {
        let ecc_key = EccKey::try_from(self.key(app, session, key_id, false)?)?;

        if let Some(intent_val) = intent {
            ecc_key.usage_allowed(intent_val)?;
        }

        Ok(ecc_key)
    }

    /// Import ECDH secret key into the key store
    fn ecdh_import_key(
        &mut self,
        app: AppVaultId,
        session: SessionId,
        key_tag: Option<u16>,
        generated: bool,
        key: &EcdhKeyImported,
        availability: KeyAvailability,
    ) -> HsmResult<EcdhKey> {
        let mut attributes = EntryAttributes::default();

        self.validate_key_params(availability, key_tag)?;

        attributes
            .common
            .flags
            .set_session(availability == KeyAvailability::Session);

        // If a key is available as session-only, it should not have a key tag.
        // App keys can optionally have a key tag. If no name is specified,
        // KEY_TAG_UNASSIGNED will be assigned.
        let sess_id_or_tag: u16 = if availability == KeyAvailability::Session {
            session
        } else {
            key_tag.unwrap_or(KEY_TAG_UNASSIGNED)
        };

        if generated {
            attributes.common.flags.set_local(true);
        }

        match key.usage() {
            EcdhKeyUsage::KeyAgreement => attributes.common.flags.set_derive(true),
        }

        let key_id = self.key_store.add_entry(
            &attributes,
            sess_id_or_tag,
            key.kind().into(),
            app,
            key.blob(),
        )?;

        Ok(self.ecdh_key_unchecked(key_id))
    }

    /// Open ECDH key
    fn ecdh_open_key(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: u16,
        intent: Option<EcdhKeyUsage>,
    ) -> HsmResult<EcdhKey> {
        let ecdh_key = EcdhKey::try_from(self.key(app, session, key_id, false)?)?;

        if let Some(intent_val) = intent {
            ecdh_key.usage_allowed(intent_val)?;
        }

        Ok(ecdh_key)
    }

    /// Import HMAC secret key into the key store
    fn hmac_import_key(
        &mut self,
        app: AppVaultId,
        session: SessionId,
        key_tag: Option<u16>,
        generated: bool,
        key: &HmacKeyImported,
        availability: KeyAvailability,
    ) -> HsmResult<HmacKey> {
        let mut attributes = EntryAttributes::default();

        self.validate_key_params(availability, key_tag)?;

        attributes
            .common
            .flags
            .set_session(availability == KeyAvailability::Session);

        // If a key is available as session-only, it should not have a key tag.
        // App keys can optionally have a key tag. If no name is specified,
        // KEY_TAG_UNASSIGNED will be assigned.
        let sess_id_or_tag: u16 = if availability == KeyAvailability::Session {
            session
        } else {
            key_tag.unwrap_or(KEY_TAG_UNASSIGNED)
        };

        if generated {
            attributes.common.flags.set_local(true);
        }

        match key.usage() {
            HmacKeyUsage::SignVerify => {
                attributes.common.flags.set_sign(true);
                attributes.common.flags.set_verify(true);
            }
        }

        let key_id = self.key_store.add_entry(
            &attributes,
            sess_id_or_tag,
            key.kind().into(),
            app,
            key.blob(),
        )?;

        Ok(HmacKey::new(self.key_unchecked(key_id)))
    }

    /// Open HMAC key
    fn hmac_open_key(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: u16,
        intent: Option<HmacKeyUsage>,
    ) -> HsmResult<HmacKey> {
        let hmac_key = HmacKey::try_from(self.key(app, session, key_id, false)?)?;

        if let Some(intent_val) = intent {
            hmac_key.usage_allowed(intent_val)?;
        }

        Ok(hmac_key)
    }

    fn ecc_key_unchecked(&self, key_id: u16) -> EccKey {
        EccKey::new(self.key_unchecked(key_id))
    }

    fn ecdh_key_unchecked(&self, key_id: u16) -> EcdhKey {
        EcdhKey::new(self.key_unchecked(key_id))
    }

    /// Import RSA key into the key store
    fn rsa_import_key(
        &mut self,
        app: AppVaultId,
        session: SessionId,
        key_tag: Option<u16>,
        generated: bool,
        key: &RsaKeyImported,
        availability: KeyAvailability,
    ) -> HsmResult<RsaKey> {
        let mut attributes = EntryAttributes::default();

        self.validate_key_params(availability, key_tag)?;

        attributes
            .common
            .flags
            .set_session(availability == KeyAvailability::Session);

        // If a key is available as session-only, it should not have a key tag.
        // App keys can optionally have a key tag. If no name is specified,
        // KEY_TAG_UNASSIGNED will be assigned.
        let sess_id_or_tag: u16 = if availability == KeyAvailability::Session {
            session
        } else {
            key_tag.unwrap_or(KEY_TAG_UNASSIGNED)
        };

        if generated {
            attributes.common.flags.set_local(true);
        }

        match key.usage() {
            RsaKeyUsage::SignVerify => {
                attributes.common.flags.set_sign(true);
                attributes.common.flags.set_verify(true);
            }
            RsaKeyUsage::EncryptDecrypt => {
                attributes.common.flags.set_encrypt(true);
                attributes.common.flags.set_decrypt(true);
            }
            RsaKeyUsage::Unwrap => attributes.common.flags.set_unwrap(true),
        }

        let key_id = self.key_store.add_entry(
            &attributes,
            sess_id_or_tag,
            key.kind().into(),
            app,
            key.blob(),
        )?;

        Ok(self.rsa_key_unchecked(key_id))
    }

    fn rsa_key_unchecked(&self, key_id: u16) -> RsaKey {
        RsaKey::new(self.key_unchecked(key_id))
    }

    /// Open RSA key
    fn rsa_open_key(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: u16,
        intent: Option<RsaKeyUsage>,
    ) -> HsmResult<RsaKey> {
        let rsa_key = RsaKey::try_from(self.key(app, session, key_id, false)?)?;

        if let Some(intent_val) = intent {
            rsa_key.usage_allowed(intent_val)?;
        }

        Ok(rsa_key)
    }

    fn import_establish_cred_encryption_key(
        &mut self,
        app: AppVaultId,
        key: &EstablishCredEncryptionKeyToImport,
    ) -> HsmResult<EstablishCredEncryptionKey> {
        let mut attributes = EntryAttributes::default();

        attributes.common.flags.set_session(false);
        let sess_id_or_tag = KEY_TAG_UNASSIGNED;

        attributes.common.flags.set_local(true);

        match key.usage() {
            EstablishCredEncryptionKeyUsage::KeyAgreement => {
                attributes.common.flags.set_derive(true)
            }
        }

        let key_id = self.key_store.add_entry(
            &attributes,
            sess_id_or_tag,
            key.kind().into(),
            app,
            key.blob(),
        )?;

        Ok(self.establish_cred_encryption_key_unchecked(key_id))
    }

    fn establish_cred_encryption_key_unchecked(&self, key_id: u16) -> EstablishCredEncryptionKey {
        EstablishCredEncryptionKey::new(self.key_unchecked(key_id))
    }

    fn open_establish_cred_encryption_key(
        &self,
        app: AppVaultId,
        key_id: u16,
    ) -> HsmResult<EstablishCredEncryptionKey> {
        let establish_cred_encryption_key = EstablishCredEncryptionKey::try_from(self.key(
            app,
            KEY_TAG_UNASSIGNED,
            key_id,
            false,
        )?)?;

        Ok(establish_cred_encryption_key)
    }

    fn import_session_encryption_key(
        &mut self,
        app: AppVaultId,
        key: &SessionEncryptionKeyToImport,
    ) -> HsmResult<SessionEncryptionKey> {
        let mut attributes = EntryAttributes::default();

        attributes.common.flags.set_session(false);
        let sess_id_or_tag = KEY_TAG_UNASSIGNED;

        attributes.common.flags.set_local(true);

        match key.usage() {
            SessionEncryptionKeyUsage::KeyAgreement => attributes.common.flags.set_derive(true),
        }

        let key_id = self.key_store.add_entry(
            &attributes,
            sess_id_or_tag,
            key.kind().into(),
            app,
            key.blob(),
        )?;

        Ok(self.session_encryption_key_unchecked(key_id))
    }

    fn session_encryption_key_unchecked(&self, key_id: u16) -> SessionEncryptionKey {
        SessionEncryptionKey::new(self.key_unchecked(key_id))
    }

    fn open_session_encryption_key(
        &self,
        app: AppVaultId,
        key_id: u16,
    ) -> HsmResult<SessionEncryptionKey> {
        let session_encryption_key =
            SessionEncryptionKey::try_from(self.key(app, KEY_TAG_UNASSIGNED, key_id, false)?)?;

        Ok(session_encryption_key)
    }

    fn import_session_key(
        &mut self,
        app: AppVaultId,
        key: &SessionKeyToImport,
    ) -> HsmResult<SessionKey> {
        let mut attributes = EntryAttributes::default();

        attributes.common.flags.set_session(false);
        let sess_id_or_tag = KEY_TAG_UNASSIGNED;

        attributes.common.flags.set_local(true);

        let key_id = self.key_store.add_entry(
            &attributes,
            sess_id_or_tag,
            key.kind().into(),
            app,
            key.blob(),
        )?;

        Ok(self.session_key_unchecked(key_id))
    }

    fn session_key_unchecked(&self, key_id: u16) -> SessionKey {
        SessionKey::new(self.key_unchecked(key_id))
    }

    fn open_session_key(
        &self,
        app: AppVaultId,
        key_id: u16,
        allow_disabled: bool,
    ) -> HsmResult<SessionKey> {
        let session_key =
            SessionKey::try_from(self.key(app, KEY_TAG_UNASSIGNED, key_id, allow_disabled)?)?;

        Ok(session_key)
    }

    fn import_raw_key(
        &mut self,
        app_id: u8,
        session: SessionId,
        key_tag: Option<u16>,
        kind: EntryKind,
        attributes: &EntryAttributes,
        blob: &[u8],
    ) -> HsmResult<u16> {
        let availability = if attributes.common.flags.session() {
            KeyAvailability::Session
        } else {
            KeyAvailability::App
        };

        self.validate_key_params(availability, key_tag)?;

        // If a key is available as session-only, it should not have a key tag.
        // App keys can optionally have a key tag. If no name is specified,
        // KEY_TAG_UNASSIGNED will be assigned.
        let sess_id_or_tag: u16 = if availability == KeyAvailability::Session {
            session
        } else {
            key_tag.unwrap_or(KEY_TAG_UNASSIGNED)
        };

        self.key_store
            .add_entry(attributes, sess_id_or_tag, kind, app_id, blob)
    }

    fn import_masking_key(
        &mut self,
        app: AppVaultId,
        key: &MaskingKeyToImport,
    ) -> HsmResult<MaskingKey> {
        let mut attributes = EntryAttributes::default();

        attributes.common.flags.set_session(false);
        let sess_id_or_tag = KEY_TAG_UNASSIGNED;

        attributes.common.flags.set_local(true);

        match key.usage() {
            MaskingKeyUsage::EncryptDecrypt => {
                attributes.common.flags.set_encrypt(true);
                attributes.common.flags.set_decrypt(true);
            }
        }

        let key_id = self.key_store.add_entry(
            &attributes,
            sess_id_or_tag,
            key.kind().into(),
            app,
            key.blob(),
        )?;

        Ok(MaskingKey::new(self.key_unchecked(key_id)))
    }

    fn open_masking_key(&self, app: AppVaultId, key_id: u16) -> HsmResult<MaskingKey> {
        let masking_key =
            MaskingKey::try_from(self.key(app, KEY_TAG_UNASSIGNED, key_id, false)?)?;

        Ok(masking_key)
    }

    /// Import Variable Length HMAC key into the key store
    fn import_var_hmac_key(
        &mut self,
        app: AppVaultId,
        session: SessionId,
        key_tag: Option<u16>,
        generated: bool,
        key: &VarLenHmacShaKeyImported,
        availability: KeyAvailability,
    ) -> HsmResult<VarLenHmacShaKey> {
        let mut attributes = EntryAttributes::default();

        self.validate_key_params(availability, key_tag)?;

        attributes
            .common
            .flags
            .set_session(availability == KeyAvailability::Session);

        // If a key is available as session-only, it should not have a key tag.
        // App keys can optionally have a key tag. If no name is specified,
        // KEY_TAG_UNASSIGNED will be assigned.
        let sess_id_or_tag: u16 = if availability == KeyAvailability::Session {
            session
        } else {
            key_tag.unwrap_or(KEY_TAG_UNASSIGNED)
        };

        if generated {
            attributes.common.flags.set_local(true);
        }

        match key.usage() {
            HmacKeyUsage::SignVerify => {
                attributes.common.flags.set_sign(true);
                attributes.common.flags.set_verify(true);
            }
        }

        attributes.entry_specific[VarLenHmacShaKey::KEY_LENGTH_INDEX] = key.length();

        let key_id = self.key_store.add_entry(
            &attributes,
            sess_id_or_tag,
            key.kind().into(),
            app,
            key.blob(),
        )?;

        Ok(VarLenHmacShaKey::new(self.key_unchecked(key_id)))
    }

    /// Open Variable Length HMAC key
    fn var_hmac_open_key(
        &self,
        app: AppVaultId,
        session: SessionId,
        key_id: u16,
        intent: Option<HmacKeyUsage>,
    ) -> HsmResult<VarLenHmacShaKey> {
        let var_hmac_key = VarLenHmacShaKey::try_from(self.key(app, session, key_id, false)?)?;

        if let Some(intent_val) = intent {
            var_hmac_key.usage_allowed(intent_val)?;
        }

        Ok(var_hmac_key)
    }
}
