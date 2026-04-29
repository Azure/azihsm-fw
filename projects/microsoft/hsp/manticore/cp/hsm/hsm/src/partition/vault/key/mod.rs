// Copyright (c) Microsoft Corporation. All rights reserved.

mod aes;
mod ecc;
mod establish_cred;
mod hmac;
mod masking;
mod rsa;
mod session;
mod session_enc;

use core::cell::Ref;

pub(crate) use aes::aes_entry_attributes;
pub(crate) use aes::AesKey;
pub(crate) use aes::AesKeyImported;
pub(crate) use aes::AesKeyKind;
pub(crate) use aes::AesKeyUsage;
pub(crate) use ecc::EccKey;
pub(crate) use ecc::EccKeyImported;
pub(crate) use ecc::EccKeyKind;
pub(crate) use ecc::EccKeyUsage;
pub(crate) use ecc::EcdhKey;
pub(crate) use ecc::EcdhKeyImported;
pub(crate) use ecc::EcdhKeyUsage;
pub(crate) use establish_cred::EstablishCredEncryptionKey;
pub(crate) use establish_cred::EstablishCredEncryptionKeyKind;
pub(crate) use establish_cred::EstablishCredEncryptionKeyToImport;
pub(crate) use establish_cred::EstablishCredEncryptionKeyUsage;
pub(crate) use hmac::HmacKey;
pub(crate) use hmac::HmacKeyImported;
pub(crate) use hmac::HmacKeyKind;
pub(crate) use hmac::HmacKeyUsage;
pub(crate) use hmac::VarLenHmacShaKey;
pub(crate) use hmac::VarLenHmacShaKeyImported;
pub(crate) use hmac::VarLenHmacShaKeyKind;
pub(crate) use masking::MaskingKey;
pub(crate) use masking::MaskingKeyKind;
pub(crate) use masking::MaskingKeyToImport;
pub(crate) use masking::MaskingKeyUsage;
use mcr_ddi_types::DdiKeyType;
pub(crate) use rsa::RsaKey;
pub(crate) use rsa::RsaKeyImported;
pub(crate) use rsa::RsaKeyKind;
pub(crate) use rsa::RsaKeyUsage;
pub(crate) use session::SessionKey;
pub(crate) use session::SessionKeyKind;
pub(crate) use session::SessionKeyToImport;
pub(crate) use session::SessionKeyUsage;
pub(crate) use session_enc::SessionEncryptionKey;
pub(crate) use session_enc::SessionEncryptionKeyKind;
pub(crate) use session_enc::SessionEncryptionKeyToImport;
pub(crate) use session_enc::SessionEncryptionKeyUsage;

use super::*;
use crate::error::HsmErr;
use crate::partition::store::EntryAttributes;

/// ECC Key availability
#[derive(Copy, Clone, PartialEq)]
pub(crate) enum KeyAvailability {
    /// The key will be available for all sessions for the current app.
    App,

    /// The key will be only available for the current session.
    Session,
}

impl TryFrom<DdiKeyAvailability> for KeyAvailability {
    type Error = HsmErr;

    fn try_from(avail: DdiKeyAvailability) -> Result<Self, Self::Error> {
        match avail {
            DdiKeyAvailability::App => Ok(KeyAvailability::App),
            DdiKeyAvailability::Session => Ok(KeyAvailability::Session),
            _ => Err(HsmErr::InvalidArgument),
        }
    }
}

impl TryFrom<DdiKeyType> for EntryKind {
    type Error = HsmErr;

    /// Performs the conversion.
    fn try_from(key_type: DdiKeyType) -> Result<Self, Self::Error> {
        match key_type {
            DdiKeyType::Rsa2kPublic => Ok(EntryKind::Rsa2kPublic),
            DdiKeyType::Rsa3kPublic => Ok(EntryKind::Rsa3kPublic),
            DdiKeyType::Rsa4kPublic => Ok(EntryKind::Rsa4kPublic),
            DdiKeyType::Rsa2kPrivate | DdiKeyType::RsaUnwrap => Ok(EntryKind::Rsa2kPrivate),
            DdiKeyType::Rsa3kPrivate => Ok(EntryKind::Rsa3kPrivate),
            DdiKeyType::Rsa4kPrivate => Ok(EntryKind::Rsa4kPrivate),
            DdiKeyType::Rsa2kPrivateCrt => Ok(EntryKind::Rsa2kPrivateCrt),
            DdiKeyType::Rsa3kPrivateCrt => Ok(EntryKind::Rsa3kPrivateCrt),
            DdiKeyType::Rsa4kPrivateCrt => Ok(EntryKind::Rsa4kPrivateCrt),
            DdiKeyType::Ecc256Public => Ok(EntryKind::Ecc256Public),
            DdiKeyType::Ecc384Public => Ok(EntryKind::Ecc384Public),
            DdiKeyType::Ecc521Public => Ok(EntryKind::Ecc521Public),
            DdiKeyType::Ecc256Private => Ok(EntryKind::Ecc256Private),
            DdiKeyType::Ecc384Private => Ok(EntryKind::Ecc384Private),
            DdiKeyType::Ecc521Private => Ok(EntryKind::Ecc521Private),
            DdiKeyType::Aes128 => Ok(EntryKind::Aes128),
            DdiKeyType::Aes192 => Ok(EntryKind::Aes192),
            DdiKeyType::Aes256 => Ok(EntryKind::Aes256),
            DdiKeyType::AesXtsBulk256 => Ok(EntryKind::AesXtsBulk256),
            DdiKeyType::AesGcmBulk256 => Ok(EntryKind::AesGcmBulk256),
            DdiKeyType::AesGcmBulk256Unapproved => Ok(EntryKind::AesGcmBulk256Unapproved),
            DdiKeyType::Secret256 => Ok(EntryKind::Secret256),
            DdiKeyType::Secret384 => Ok(EntryKind::Secret384),
            DdiKeyType::Secret521 => Ok(EntryKind::Secret521),
            DdiKeyType::HmacSha256 => Ok(EntryKind::HmacSha256),
            DdiKeyType::HmacSha384 => Ok(EntryKind::HmacSha384),
            DdiKeyType::HmacSha512 => Ok(EntryKind::HmacSha512),
            DdiKeyType::VarLenHmacSha256 => Ok(EntryKind::VarLenHmacSha256),
            DdiKeyType::VarLenHmacSha384 => Ok(EntryKind::VarLenHmacSha384),
            DdiKeyType::VarLenHmacSha512 => Ok(EntryKind::VarLenHmacSha512),
            _ => Err(HsmErr::InvalidArgument),
        }
    }
}

impl TryFrom<EntryKind> for DdiKeyType {
    type Error = HsmErr;

    /// Performs the conversion.
    fn try_from(key_kind: EntryKind) -> Result<Self, Self::Error> {
        match key_kind {
            EntryKind::Rsa2kPublic => Ok(DdiKeyType::Rsa2kPublic),
            EntryKind::Rsa3kPublic => Ok(DdiKeyType::Rsa3kPublic),
            EntryKind::Rsa4kPublic => Ok(DdiKeyType::Rsa4kPublic),
            EntryKind::Rsa2kPrivate => Ok(DdiKeyType::Rsa2kPrivate),
            EntryKind::Rsa3kPrivate => Ok(DdiKeyType::Rsa3kPrivate),
            EntryKind::Rsa4kPrivate => Ok(DdiKeyType::Rsa4kPrivate),
            EntryKind::Rsa2kPrivateCrt => Ok(DdiKeyType::Rsa2kPrivateCrt),
            EntryKind::Rsa3kPrivateCrt => Ok(DdiKeyType::Rsa3kPrivateCrt),
            EntryKind::Rsa4kPrivateCrt => Ok(DdiKeyType::Rsa4kPrivateCrt),
            EntryKind::Ecc256Public => Ok(DdiKeyType::Ecc256Public),
            EntryKind::Ecc384Public => Ok(DdiKeyType::Ecc384Public),
            EntryKind::Ecc521Public => Ok(DdiKeyType::Ecc521Public),
            EntryKind::Ecc256Private => Ok(DdiKeyType::Ecc256Private),
            EntryKind::Ecc384Private => Ok(DdiKeyType::Ecc384Private),
            EntryKind::Ecc521Private => Ok(DdiKeyType::Ecc521Private),
            EntryKind::Aes128 => Ok(DdiKeyType::Aes128),
            EntryKind::Aes192 => Ok(DdiKeyType::Aes192),
            EntryKind::Aes256 => Ok(DdiKeyType::Aes256),
            EntryKind::AesXtsBulk256 => Ok(DdiKeyType::AesXtsBulk256),
            EntryKind::AesGcmBulk256 => Ok(DdiKeyType::AesGcmBulk256),
            EntryKind::AesGcmBulk256Unapproved => Ok(DdiKeyType::AesGcmBulk256Unapproved),
            EntryKind::Secret256 => Ok(DdiKeyType::Secret256),
            EntryKind::Secret384 => Ok(DdiKeyType::Secret384),
            EntryKind::Secret521 => Ok(DdiKeyType::Secret521),
            EntryKind::HmacSha256 => Ok(DdiKeyType::HmacSha256),
            EntryKind::HmacSha384 => Ok(DdiKeyType::HmacSha384),
            EntryKind::HmacSha512 => Ok(DdiKeyType::HmacSha512),
            EntryKind::VarLenHmacSha256 => Ok(DdiKeyType::VarLenHmacSha256),
            EntryKind::VarLenHmacSha384 => Ok(DdiKeyType::VarLenHmacSha384),
            EntryKind::VarLenHmacSha512 => Ok(DdiKeyType::VarLenHmacSha512),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

#[derive(Clone)]
pub(crate) struct VaultKey {
    #[allow(unused)]
    key_store: KeyStore,

    #[allow(unused)]
    key_id: u16,
}

impl VaultKey {
    pub(crate) fn new(key_store: KeyStore, key_id: u16) -> Self {
        Self { key_store, key_id }
    }

    pub(crate) fn id(&self) -> u16 {
        self.key_id
    }

    #[allow(unused)]
    pub(crate) fn flags(&self) -> Result<EntryFlags, HsmErr> {
        let entry = self.key_store.get_entry(self.key_id, true)?;
        Ok(entry.flags())
    }

    pub(crate) fn attributes(&self) -> Result<Ref<'_, EntryAttributes>, HsmErr> {
        self.key_store.get_entry_attributes(self.key_id, true)
    }

    #[allow(unused)]
    pub(crate) fn session_id(&self) -> Result<Option<u16>, HsmErr> {
        let entry = self.key_store.get_entry(self.key_id, true)?;
        Ok(entry.session_id())
    }

    #[allow(unused)]
    pub(crate) fn key_tag(&self) -> Result<Option<u16>, HsmErr> {
        let entry = self.key_store.get_entry(self.key_id, true)?;
        Ok(entry.key_tag())
    }

    pub(crate) fn kind(&self) -> Result<EntryKind, HsmErr> {
        let entry = self.key_store.get_entry(self.key_id, true)?;
        Ok(entry.kind())
    }

    pub(crate) fn class(&self) -> Result<EntryClass, HsmErr> {
        let kind = self.kind()?;
        Ok(kind.into())
    }

    #[allow(unused)]
    pub(crate) fn app_kv_id(&self) -> Result<u8, HsmErr> {
        let entry = self.key_store.get_entry(self.key_id, true)?;
        Ok(entry.app_id())
    }

    pub(crate) fn disabled(&self) -> Result<bool, HsmErr> {
        let entry = self.key_store.get_entry(self.key_id, true)?;
        Ok(entry.disabled())
    }

    pub(crate) fn blob(&self) -> Result<Ref<'_, [u8]>, HsmErr> {
        self.key_store.get_entry_blob(self.key_id)
    }
}
