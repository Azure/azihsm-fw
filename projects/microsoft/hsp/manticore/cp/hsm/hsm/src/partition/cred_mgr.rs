// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

/// App Vault ID for internal keys
pub(crate) const APP_VAULT_ID_FOR_INTERNAL_KEYS: u8 = u8::MAX;

#[derive(Default)]
pub(crate) struct CredentialMgr {
    /// User Credential
    user_cred: UserCredential,
}

impl CredentialMgr {
    /// Verify if user credential is set
    ///
    /// # Returns
    ///
    /// * Returns `true` if the user credential has not been set
    pub fn verify_user_cred_is_set(&self) -> bool {
        !self.user_cred.id.eq(&[0; 16]) && !self.user_cred.pin.eq(&[0; 16])
    }

    /// Change user credential
    ///
    /// # Arguments
    ///
    /// * `id` - New user ID
    /// * `pin` - New user PIN
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful, else `Err(HsmErr)`
    pub fn change_user_cred(&mut self, id: &[u8], pin: &[u8]) -> HsmResult<()> {
        if id.len() != 16 || pin.len() != 16 {
            Err(HsmErr::InvalidUserCredential)?;
        }

        if id.eq(&[0; 16]) || pin.eq(&[0; 16]) {
            Err(HsmErr::InvalidUserCredential)?;
        }

        let mut new_id = [0; 16];
        new_id.copy_from_slice(id);
        let mut new_pin = [0; 16];
        new_pin.copy_from_slice(pin);

        let cred = UserCredential {
            id: new_id,
            pin: new_pin,
            vault_id: self.user_cred.vault_id,
        };

        self.user_cred = cred;

        Ok(())
    }

    /// Restore user credential
    ///
    /// # Arguments
    ///
    /// * `cred` - User credential to restore
    pub fn restore_user_cred(&mut self, cred: UserCredential) {
        self.user_cred = cred;
    }

    /// Verify whether given user credentials represent the authorized user credentials
    ///
    /// # Arguments
    ///
    /// * `id` - Application ID
    /// * `pin` - Application PIN
    ///
    /// # Returns
    ///
    /// * Returns `true` if the given credentials match the user credentials, else `false`
    pub fn authorize_user(&self, id: &AppId, pin: &AppPin) -> bool {
        if self.user_cred.id.eq(&[0; 16]) || self.user_cred.pin.eq(&[0; 16]) {
            return false;
        }

        self.user_cred.id == *id && self.user_cred.pin == *pin
    }

    /// Get user credential
    ///
    /// # Returns
    ///
    /// * Returns credential of the user
    pub fn user_cred(&self) -> UserCredential {
        self.user_cred
    }

    /// Get user vault ID
    pub fn get_user_vault_id(&self) -> AppVaultId {
        self.user_cred.vault_id
    }

    /// Clear all credentials
    pub fn clear(&mut self) {
        self.user_cred = UserCredential::default();
    }
}
