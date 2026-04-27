// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_types::DdiKeyUsage;

use super::*;

/// Establish Cred Encryption Key Kind
#[derive(Clone, Copy, PartialEq)]
pub(crate) enum EstablishCredEncryptionKeyKind {
    /// Ecc 384-bit public + private key
    Ecc384 = 144,
}

impl EstablishCredEncryptionKeyKind {
    const MAX_KEY_LEN: EstablishCredEncryptionKeyKind = EstablishCredEncryptionKeyKind::Ecc384;
}

impl From<EstablishCredEncryptionKeyKind> for EntryKind {
    fn from(_: EstablishCredEncryptionKeyKind) -> Self {
        EntryKind::EstablishCred
    }
}

impl From<EstablishCredEncryptionKeyKind> for usize {
    fn from(kind: EstablishCredEncryptionKeyKind) -> Self {
        kind as usize
    }
}

/// Establish Cred Encryption Key Usage
#[derive(Copy, Clone, PartialEq)]
pub(crate) enum EstablishCredEncryptionKeyUsage {
    /// Key Agreement usage
    KeyAgreement,
}

impl TryFrom<DdiKeyUsage> for EstablishCredEncryptionKeyUsage {
    type Error = HsmErr;

    fn try_from(usage: DdiKeyUsage) -> Result<Self, Self::Error> {
        match usage {
            DdiKeyUsage::Derive => Ok(EstablishCredEncryptionKeyUsage::KeyAgreement),
            _ => Err(HsmErr::InvalidPermissions),
        }
    }
}

pub(crate) struct EstablishCredEncryptionKey {
    key: VaultKey,
}

impl EstablishCredEncryptionKey {
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

impl TryFrom<VaultKey> for EstablishCredEncryptionKey {
    type Error = HsmErr;

    fn try_from(key: VaultKey) -> Result<Self, Self::Error> {
        let class = key.class()?;
        if class != EntryClass::Ecc {
            Err(HsmErr::InvalidKeyType)?
        }

        Ok(Self::new(key))
    }
}

pub(crate) struct EstablishCredEncryptionKeyToImport {
    kind: EstablishCredEncryptionKeyKind,
    usage: EstablishCredEncryptionKeyUsage,
    blob: [u8; EstablishCredEncryptionKeyKind::MAX_KEY_LEN as usize],
}

impl EstablishCredEncryptionKeyToImport {
    pub fn new(
        kind: EstablishCredEncryptionKeyKind,
        usage: EstablishCredEncryptionKeyUsage,
        val: &[u8],
    ) -> Result<Self, HsmErr> {
        if val.len() != usize::from(kind) {
            Err(HsmErr::InvalidKeyType)?
        }

        let mut blob = [0; EstablishCredEncryptionKeyKind::MAX_KEY_LEN as usize];
        blob[..usize::from(kind)].copy_from_slice(val);

        Ok(Self { kind, usage, blob })
    }

    pub fn kind(&self) -> EstablishCredEncryptionKeyKind {
        self.kind
    }

    pub fn usage(&self) -> EstablishCredEncryptionKeyUsage {
        self.usage
    }

    pub fn blob(&self) -> &[u8] {
        &self.blob[..usize::from(self.kind)]
    }
}
