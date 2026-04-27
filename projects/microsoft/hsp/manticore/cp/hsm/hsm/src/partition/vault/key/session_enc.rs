// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_types::DdiKeyUsage;

use super::*;

/// Session Encryption Key Kind
#[derive(Clone, Copy, PartialEq)]
pub(crate) enum SessionEncryptionKeyKind {
    /// Ecc 384-bit public + private key
    Ecc384 = 144,
}

impl SessionEncryptionKeyKind {
    const MAX_KEY_LEN: SessionEncryptionKeyKind = SessionEncryptionKeyKind::Ecc384;
}

impl From<SessionEncryptionKeyKind> for EntryKind {
    fn from(_: SessionEncryptionKeyKind) -> Self {
        EntryKind::SessionEncryption
    }
}

impl From<SessionEncryptionKeyKind> for usize {
    fn from(kind: SessionEncryptionKeyKind) -> Self {
        kind as usize
    }
}

/// Session Encryption Key Usage
#[derive(Copy, Clone, PartialEq)]
pub(crate) enum SessionEncryptionKeyUsage {
    /// Key Agreement usage
    KeyAgreement,
}

impl TryFrom<DdiKeyUsage> for SessionEncryptionKeyUsage {
    type Error = HsmErr;

    fn try_from(usage: DdiKeyUsage) -> Result<Self, Self::Error> {
        match usage {
            DdiKeyUsage::Derive => Ok(SessionEncryptionKeyUsage::KeyAgreement),
            _ => Err(HsmErr::InvalidPermissions),
        }
    }
}

pub(crate) struct SessionEncryptionKey {
    key: VaultKey,
}

impl SessionEncryptionKey {
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

impl TryFrom<VaultKey> for SessionEncryptionKey {
    type Error = HsmErr;

    fn try_from(key: VaultKey) -> Result<Self, Self::Error> {
        let kind = key.kind()?;
        if kind != EntryKind::SessionEncryption {
            Err(HsmErr::InvalidKeyType)?
        }

        Ok(Self::new(key))
    }
}

pub(crate) struct SessionEncryptionKeyToImport {
    kind: SessionEncryptionKeyKind,
    usage: SessionEncryptionKeyUsage,
    blob: [u8; SessionEncryptionKeyKind::MAX_KEY_LEN as usize],
}

impl SessionEncryptionKeyToImport {
    pub fn new(
        kind: SessionEncryptionKeyKind,
        usage: SessionEncryptionKeyUsage,
        val: &[u8],
    ) -> Result<Self, HsmErr> {
        if val.len() != usize::from(kind) {
            Err(HsmErr::InvalidKeyType)?
        }

        let mut blob = [0; SessionEncryptionKeyKind::MAX_KEY_LEN as usize];
        blob[..usize::from(kind)].copy_from_slice(val);

        Ok(Self { kind, usage, blob })
    }

    pub fn kind(&self) -> SessionEncryptionKeyKind {
        self.kind
    }

    pub fn usage(&self) -> SessionEncryptionKeyUsage {
        self.usage
    }

    pub fn blob(&self) -> &[u8] {
        &self.blob[..usize::from(self.kind)]
    }
}
