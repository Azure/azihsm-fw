// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

/// Hmac Key Kind
#[derive(Clone, Copy, PartialEq)]
pub(crate) enum HmacKeyKind {
    /// HMAC256 key
    HmacKey256 = 32,

    /// HMAC384 key
    HmacKey384 = 48,

    /// HMAC512 key
    HmacKey512 = 64,
}

impl From<HmacKeyKind> for usize {
    fn from(kind: HmacKeyKind) -> Self {
        kind as usize
    }
}

impl From<HmacKeyKind> for EntryKind {
    fn from(kind: HmacKeyKind) -> Self {
        match kind {
            HmacKeyKind::HmacKey256 => EntryKind::HmacSha256,
            HmacKeyKind::HmacKey384 => EntryKind::HmacSha384,
            HmacKeyKind::HmacKey512 => EntryKind::HmacSha512,
        }
    }
}

impl TryFrom<DdiKeyType> for HmacKeyKind {
    type Error = HsmErr;

    fn try_from(value: DdiKeyType) -> Result<Self, Self::Error> {
        match value {
            DdiKeyType::HmacSha256 => Ok(HmacKeyKind::HmacKey256),
            DdiKeyType::HmacSha384 => Ok(HmacKeyKind::HmacKey384),
            DdiKeyType::HmacSha512 => Ok(HmacKeyKind::HmacKey512),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

impl From<HmacKeyKind> for DdiHashAlgorithm {
    fn from(kind: HmacKeyKind) -> Self {
        match kind {
            HmacKeyKind::HmacKey256 => DdiHashAlgorithm::Sha256,
            HmacKeyKind::HmacKey384 => DdiHashAlgorithm::Sha384,
            HmacKeyKind::HmacKey512 => DdiHashAlgorithm::Sha512,
        }
    }
}

#[derive(Clone, Copy, PartialEq)]
pub(crate) enum HmacKeyUsage {
    SignVerify,
    Derive,
}

impl TryFrom<DdiKeyUsage> for HmacKeyUsage {
    type Error = HsmErr;

    fn try_from(usage: DdiKeyUsage) -> Result<Self, Self::Error> {
        match usage {
            DdiKeyUsage::SignVerify => Ok(HmacKeyUsage::SignVerify),
            DdiKeyUsage::Derive => Ok(HmacKeyUsage::Derive),
            _ => Err(HsmErr::InvalidPermissions),
        }
    }
}

/// HMAC Key
pub(crate) struct HmacKey {
    /// Physical vault key
    key: VaultKey,
}

impl HmacKey {
    /// Create a new HMAC key
    pub fn new(key: VaultKey) -> Self {
        Self { key }
    }

    /// Get the key ID
    pub fn id(&self) -> u16 {
        self.key.id()
    }

    /// Get the key blob
    pub fn blob(&self) -> HsmResult<Ref<'_, [u8]>> {
        self.key.blob()
    }

    /// Check if the key is disabled
    pub fn disabled(&self) -> HsmResult<bool> {
        self.key.disabled()
    }

    /// Check if the key usage is allowed
    pub fn usage_allowed(&self, usage: HmacKeyUsage) -> Result<(), HsmErr> {
        let attributes = self.key.attributes()?;
        match usage {
            HmacKeyUsage::SignVerify => {
                if !attributes.common.flags.sign() || !attributes.common.flags.verify() {
                    Err(HsmErr::InvalidPermissions)?
                }
            }
            HmacKeyUsage::Derive => {
                if !attributes.common.flags.derive() {
                    Err(HsmErr::InvalidPermissions)?
                }
            }
        }
        Ok(())
    }

    /// Get the key kind
    pub fn kind(&self) -> HsmResult<HmacKeyKind> {
        match self.key.kind()? {
            EntryKind::HmacSha256 => Ok(HmacKeyKind::HmacKey256),
            EntryKind::HmacSha384 => Ok(HmacKeyKind::HmacKey384),
            EntryKind::HmacSha512 => Ok(HmacKeyKind::HmacKey512),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

impl TryFrom<VaultKey> for HmacKey {
    type Error = HsmErr;

    fn try_from(key: VaultKey) -> Result<Self, Self::Error> {
        let class = key.class()?;
        if class != EntryClass::Hmac {
            Err(HsmErr::InvalidKeyType)?
        }

        Ok(Self::new(key))
    }
}

/// Imported HMAC Key
pub(crate) struct HmacKeyImported<'a> {
    kind: HmacKeyKind,
    usage: HmacKeyUsage,
    blob: &'a [u8],
}

impl<'a> HmacKeyImported<'a> {
    /// Create a new imported HMAC key
    pub fn new(kind: HmacKeyKind, usage: HmacKeyUsage, val: &'a [u8]) -> Result<Self, HsmErr> {
        if val.len() != usize::from(kind) {
            Err(HsmErr::InvalidKeyType)?
        }

        Ok(Self {
            kind,
            usage,
            blob: val,
        })
    }

    /// Get the key kind
    pub fn kind(&self) -> HmacKeyKind {
        self.kind
    }

    /// Get the key usage
    pub fn usage(&self) -> HmacKeyUsage {
        self.usage
    }

    /// Get the key blob
    pub fn blob(&self) -> &[u8] {
        &self.blob[..usize::from(self.kind)]
    }
}

/// Variable Length HMAC Key Kind
#[derive(Clone, Copy)]
pub(crate) enum VarLenHmacShaKeyKind {
    /// Variable Length HMAC256 key
    VarLenHmacShaKey256,

    /// Variable Length HMAC384 key
    VarLenHmacShaKey384,

    /// Variable Length HMAC512 key
    VarLenHmacShaKey512,
}

impl VarLenHmacShaKeyKind {
    /// Get the maximum key length in bytes
    pub fn max_length(&self) -> usize {
        match self {
            VarLenHmacShaKeyKind::VarLenHmacShaKey256 => 64,
            VarLenHmacShaKeyKind::VarLenHmacShaKey384 => 128,
            VarLenHmacShaKeyKind::VarLenHmacShaKey512 => 128,
        }
    }

    pub fn min_length(&self) -> usize {
        match self {
            VarLenHmacShaKeyKind::VarLenHmacShaKey256 => 32,
            VarLenHmacShaKeyKind::VarLenHmacShaKey384 => 48,
            VarLenHmacShaKeyKind::VarLenHmacShaKey512 => 64,
        }
    }
}

impl From<VarLenHmacShaKeyKind> for EntryKind {
    fn from(kind: VarLenHmacShaKeyKind) -> Self {
        match kind {
            VarLenHmacShaKeyKind::VarLenHmacShaKey256 => EntryKind::VarLenHmacSha256,
            VarLenHmacShaKeyKind::VarLenHmacShaKey384 => EntryKind::VarLenHmacSha384,
            VarLenHmacShaKeyKind::VarLenHmacShaKey512 => EntryKind::VarLenHmacSha512,
        }
    }
}

impl TryFrom<EntryKind> for VarLenHmacShaKeyKind {
    type Error = HsmErr;

    fn try_from(value: EntryKind) -> Result<Self, Self::Error> {
        match value {
            EntryKind::VarLenHmacSha256 => Ok(VarLenHmacShaKeyKind::VarLenHmacShaKey256),
            EntryKind::VarLenHmacSha384 => Ok(VarLenHmacShaKeyKind::VarLenHmacShaKey384),
            EntryKind::VarLenHmacSha512 => Ok(VarLenHmacShaKeyKind::VarLenHmacShaKey512),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

impl TryFrom<DdiKeyType> for VarLenHmacShaKeyKind {
    type Error = HsmErr;

    fn try_from(value: DdiKeyType) -> Result<Self, Self::Error> {
        match value {
            DdiKeyType::VarLenHmacSha256 => Ok(VarLenHmacShaKeyKind::VarLenHmacShaKey256),
            DdiKeyType::VarLenHmacSha384 => Ok(VarLenHmacShaKeyKind::VarLenHmacShaKey384),
            DdiKeyType::VarLenHmacSha512 => Ok(VarLenHmacShaKeyKind::VarLenHmacShaKey512),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

impl From<VarLenHmacShaKeyKind> for DdiHashAlgorithm {
    fn from(kind: VarLenHmacShaKeyKind) -> Self {
        match kind {
            VarLenHmacShaKeyKind::VarLenHmacShaKey256 => DdiHashAlgorithm::Sha256,
            VarLenHmacShaKeyKind::VarLenHmacShaKey384 => DdiHashAlgorithm::Sha384,
            VarLenHmacShaKeyKind::VarLenHmacShaKey512 => DdiHashAlgorithm::Sha512,
        }
    }
}

/// Variable Length HMAC Key
pub(crate) struct VarLenHmacShaKey {
    /// Physical vault key
    key: VaultKey,
}

impl VarLenHmacShaKey {
    pub const KEY_LENGTH_INDEX: usize = 0;

    /// Create a new HMAC key
    pub fn new(key: VaultKey) -> Self {
        Self { key }
    }

    /// Get the key ID
    pub fn id(&self) -> u16 {
        self.key.id()
    }

    /// Get the key blob
    pub fn blob(&self) -> HsmResult<Ref<'_, [u8]>> {
        self.key.blob()
    }

    /// Check if the key is disabled
    pub fn disabled(&self) -> HsmResult<bool> {
        self.key.disabled()
    }

    /// Check if the key usage is allowed
    pub fn usage_allowed(&self, usage: HmacKeyUsage) -> Result<(), HsmErr> {
        let attributes = self.key.attributes()?;
        match usage {
            HmacKeyUsage::SignVerify => {
                if !attributes.common.flags.sign() || !attributes.common.flags.verify() {
                    Err(HsmErr::InvalidPermissions)?
                }
            }
            HmacKeyUsage::Derive => {
                if !attributes.common.flags.derive() {
                    Err(HsmErr::InvalidPermissions)?
                }
            }
        }
        Ok(())
    }

    /// Get the key kind
    pub fn kind(&self) -> HsmResult<VarLenHmacShaKeyKind> {
        match self.key.kind()? {
            EntryKind::VarLenHmacSha256 => Ok(VarLenHmacShaKeyKind::VarLenHmacShaKey256),
            EntryKind::VarLenHmacSha384 => Ok(VarLenHmacShaKeyKind::VarLenHmacShaKey384),
            EntryKind::VarLenHmacSha512 => Ok(VarLenHmacShaKeyKind::VarLenHmacShaKey512),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

impl TryFrom<VaultKey> for VarLenHmacShaKey {
    type Error = HsmErr;

    fn try_from(key: VaultKey) -> Result<Self, Self::Error> {
        let class = key.class()?;
        if class != EntryClass::VarLenHmacSha {
            Err(HsmErr::InvalidKeyType)?
        }

        Ok(Self::new(key))
    }
}

/// Imported Variable length HMAC Key
pub(crate) struct VarLenHmacShaKeyImported<'a> {
    kind: VarLenHmacShaKeyKind,
    usage: HmacKeyUsage,
    blob: &'a [u8],
    length: u8,
}

impl<'a> VarLenHmacShaKeyImported<'a> {
    /// Create a new imported variable length HMAC key
    pub fn new(
        kind: VarLenHmacShaKeyKind,
        usage: HmacKeyUsage,
        val: &'a [u8],
    ) -> Result<Self, HsmErr> {
        if val.len() < kind.min_length() || val.len() > kind.max_length() {
            Err(HsmErr::InvalidKeyLength)?
        }

        Ok(Self {
            kind,
            usage,
            blob: val,
            length: val.len() as u8,
        })
    }

    /// Get the key kind
    pub fn kind(&self) -> VarLenHmacShaKeyKind {
        self.kind
    }

    /// Get the key blob
    pub fn blob(&self) -> &[u8] {
        &self.blob[..self.length as usize]
    }

    /// Get the key usage
    pub fn usage(&self) -> HmacKeyUsage {
        self.usage
    }

    /// Get the key length
    pub fn length(&self) -> u8 {
        self.length
    }
}
