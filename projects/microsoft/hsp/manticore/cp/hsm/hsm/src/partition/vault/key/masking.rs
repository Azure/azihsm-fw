// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_types::DdiKeyUsage;

use super::*;
use crate::error::HsmResult;

/// Masking Key Kind
#[derive(Clone, Copy, PartialEq)]
pub(crate) enum MaskingKeyKind {
    /// AES-CBC-256 + HMAC-SHA-384 key
    AesCbc256Hmac384 = 80,
}

impl MaskingKeyKind {
    const MAX_KEY_LEN: MaskingKeyKind = MaskingKeyKind::AesCbc256Hmac384;
}

impl From<MaskingKeyKind> for EntryKind {
    fn from(_: MaskingKeyKind) -> Self {
        EntryKind::MaskingKey
    }
}

impl From<MaskingKeyKind> for usize {
    fn from(kind: MaskingKeyKind) -> Self {
        kind as usize
    }
}

/// Masking Key Usage
#[derive(Copy, Clone, PartialEq)]
pub(crate) enum MaskingKeyUsage {
    /// Encrypt decrypt usage
    EncryptDecrypt,
}

impl TryFrom<DdiKeyUsage> for MaskingKeyUsage {
    type Error = HsmErr;

    fn try_from(usage: DdiKeyUsage) -> Result<Self, Self::Error> {
        match usage {
            DdiKeyUsage::EncryptDecrypt => Ok(MaskingKeyUsage::EncryptDecrypt),
            _ => Err(HsmErr::InvalidPermissions),
        }
    }
}

pub(crate) struct MaskingKey {
    key: VaultKey,
}

impl MaskingKey {
    pub fn new(key: VaultKey) -> Self {
        Self { key }
    }

    pub fn id(&self) -> u16 {
        self.key.id()
    }

    pub fn blob(&self) -> HsmResult<Ref<'_, [u8]>> {
        self.key.blob()
    }
}

impl TryFrom<VaultKey> for MaskingKey {
    type Error = HsmErr;

    fn try_from(key: VaultKey) -> Result<Self, Self::Error> {
        let class = key.class()?;
        if class != EntryClass::MaskingKey {
            Err(HsmErr::InvalidKeyType)?
        }

        Ok(Self::new(key))
    }
}

pub(crate) struct MaskingKeyToImport {
    kind: MaskingKeyKind,
    usage: MaskingKeyUsage,
    blob: SecureByteArray<{ MaskingKeyKind::MAX_KEY_LEN as usize }>,
}

impl MaskingKeyToImport {
    pub fn new(kind: MaskingKeyKind, usage: MaskingKeyUsage, val: &[u8]) -> Result<Self, HsmErr> {
        if val.len() != usize::from(kind) {
            Err(HsmErr::InvalidKeyType)?
        }

        let mut blob = SecureByteArray::<{ MaskingKeyKind::MAX_KEY_LEN as usize }>::new(
            [0u8; MaskingKeyKind::MAX_KEY_LEN as usize],
        );
        blob[..usize::from(kind)].copy_from_slice(val);

        Ok(Self { kind, usage, blob })
    }

    pub fn kind(&self) -> MaskingKeyKind {
        self.kind
    }

    pub fn usage(&self) -> MaskingKeyUsage {
        self.usage
    }

    pub fn blob(&self) -> &[u8] {
        &self.blob[..usize::from(self.kind)]
    }
}
