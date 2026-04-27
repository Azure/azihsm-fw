// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_types::DdiAesKeySize;
use mcr_ddi_types::DdiKeyUsage;

use super::*;
use crate::error::HsmResult;

/// AES Key kind. The value of the enum describes the key size
#[derive(Clone, Copy, PartialEq)]
pub(crate) enum AesKeyKind {
    /// AES XTS Bulk 256-bit key.
    AesXtsBulk256,

    /// AES GCM Bulk 256-bit key.
    AesGcmBulk256,

    /// AES GCM Bulk 256-bit Unapproved key.
    AesGcmBulk256Unapproved,

    /// AES 128-bit key
    Aes128,

    /// AES 192-bit key
    Aes192,

    /// AES 256-bit key
    Aes256,
}

impl AesKeyKind {
    const MAX_KEY_LEN: usize = 32;

    pub fn len(&self) -> usize {
        usize::from(*self)
    }

    pub const fn max_len() -> usize {
        Self::MAX_KEY_LEN
    }
}

impl From<AesKeyKind> for EntryKind {
    fn from(kind: AesKeyKind) -> Self {
        match kind {
            AesKeyKind::Aes128 => EntryKind::Aes128,
            AesKeyKind::Aes192 => EntryKind::Aes192,
            AesKeyKind::Aes256 => EntryKind::Aes256,
            AesKeyKind::AesXtsBulk256 => EntryKind::AesXtsBulk256,
            AesKeyKind::AesGcmBulk256 => EntryKind::AesGcmBulk256,
            AesKeyKind::AesGcmBulk256Unapproved => EntryKind::AesGcmBulk256Unapproved,
        }
    }
}

impl TryFrom<EntryKind> for AesKeyKind {
    type Error = HsmErr;

    fn try_from(kind: EntryKind) -> Result<Self, Self::Error> {
        match kind {
            EntryKind::Aes128 => Ok(AesKeyKind::Aes128),
            EntryKind::Aes192 => Ok(AesKeyKind::Aes192),
            EntryKind::Aes256 => Ok(AesKeyKind::Aes256),
            EntryKind::AesXtsBulk256 => Ok(AesKeyKind::AesXtsBulk256),
            EntryKind::AesGcmBulk256 => Ok(AesKeyKind::AesGcmBulk256),
            EntryKind::AesGcmBulk256Unapproved => Ok(AesKeyKind::AesGcmBulk256Unapproved),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

impl From<AesKeyKind> for usize {
    fn from(kind: AesKeyKind) -> Self {
        match kind {
            AesKeyKind::AesXtsBulk256 => 2,
            AesKeyKind::AesGcmBulk256 => 2,
            AesKeyKind::AesGcmBulk256Unapproved => 2,
            AesKeyKind::Aes128 => 16,
            AesKeyKind::Aes192 => 24,
            AesKeyKind::Aes256 => 32,
        }
    }
}

impl TryFrom<DdiAesKeySize> for AesKeyKind {
    type Error = HsmErr;

    fn try_from(size: DdiAesKeySize) -> Result<Self, Self::Error> {
        match size {
            DdiAesKeySize::Aes128 => Ok(AesKeyKind::Aes128),
            DdiAesKeySize::Aes192 => Ok(AesKeyKind::Aes192),
            DdiAesKeySize::Aes256 => Ok(AesKeyKind::Aes256),
            DdiAesKeySize::AesXtsBulk256 => Ok(AesKeyKind::AesXtsBulk256),
            DdiAesKeySize::AesGcmBulk256 => Ok(AesKeyKind::AesGcmBulk256),
            DdiAesKeySize::AesGcmBulk256Unapproved => Ok(AesKeyKind::AesGcmBulk256Unapproved),
            _ => Err(HsmErr::InvalidArgument),
        }
    }
}

#[derive(Clone, Copy, PartialEq)]
pub(crate) enum AesKeyUsage {
    EncryptDecrypt,
}

impl TryFrom<DdiKeyUsage> for AesKeyUsage {
    type Error = HsmErr;

    fn try_from(usage: DdiKeyUsage) -> Result<Self, Self::Error> {
        match usage {
            DdiKeyUsage::EncryptDecrypt => Ok(AesKeyUsage::EncryptDecrypt),
            _ => Err(HsmErr::InvalidPermissions),
        }
    }
}

#[derive(Clone)]
pub(crate) struct AesKey {
    key: VaultKey,
}

impl AesKey {
    pub fn new(key: VaultKey) -> Self {
        Self { key }
    }

    pub fn id(&self) -> u16 {
        self.key.id()
    }

    pub fn blob(&self) -> HsmResult<Ref<'_, [u8]>> {
        self.key.blob()
    }

    pub fn usage_allowed(&self, usage: AesKeyUsage) -> Result<(), HsmErr> {
        let attributes = self.key.attributes()?;
        match usage {
            AesKeyUsage::EncryptDecrypt => {
                if !attributes.common.flags.encrypt() || !attributes.common.flags.decrypt() {
                    Err(HsmErr::InvalidPermissions)?
                }
            }
        }
        Ok(())
    }

    pub fn disabled(&self) -> HsmResult<bool> {
        self.key.disabled()
    }
}

impl TryFrom<VaultKey> for AesKey {
    type Error = HsmErr;

    fn try_from(key: VaultKey) -> Result<Self, Self::Error> {
        let class = key.class()?;
        if class != EntryClass::Aes {
            Err(HsmErr::InvalidKeyType)?
        }
        Ok(Self::new(key))
    }
}

pub(crate) struct AesKeyImported<'a> {
    kind: AesKeyKind,
    usage: AesKeyUsage,
    blob: &'a [u8],
}

impl<'a> AesKeyImported<'a> {
    pub fn new(kind: AesKeyKind, usage: AesKeyUsage, val: &'a [u8]) -> Result<Self, HsmErr> {
        if val.len() != usize::from(kind) {
            Err(HsmErr::InvalidKeyType)?
        }

        Ok(Self {
            kind,
            usage,
            blob: val,
        })
    }

    pub fn kind(&self) -> AesKeyKind {
        self.kind
    }

    pub fn usage(&self) -> AesKeyUsage {
        self.usage
    }

    pub fn blob(&self) -> &[u8] {
        &self.blob[..usize::from(self.kind)]
    }
}

/// Build `EntryAttributes` for an AES key from its availability, generated flag,
/// and usage. Used by all non-unmask AES import paths. The unmask path passes
/// the original attributes directly instead.
pub(crate) fn aes_entry_attributes(
    availability: KeyAvailability,
    generated: bool,
    usage: AesKeyUsage,
) -> EntryAttributes {
    let mut attributes = EntryAttributes::default();
    attributes
        .common
        .flags
        .set_session(availability == KeyAvailability::Session);
    if generated {
        attributes.common.flags.set_local(true);
    }
    match usage {
        AesKeyUsage::EncryptDecrypt => {
            attributes.common.flags.set_encrypt(true);
            attributes.common.flags.set_decrypt(true);
        }
    }
    attributes
}
