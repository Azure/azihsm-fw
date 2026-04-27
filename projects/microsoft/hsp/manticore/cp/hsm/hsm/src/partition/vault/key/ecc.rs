// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_types::DdiKeyUsage;

use super::*;

/// ECC Key kind
#[derive(Clone, Copy, PartialEq)]
pub(crate) enum EccKeyKind {
    /// Ecc 256-bit public key
    #[allow(dead_code)]
    Ecc256Public = 64,

    /// Ecc 384-bit key
    #[allow(dead_code)]
    Ecc384Public = 96,

    /// Ecc 521-bit key
    #[allow(dead_code)]
    Ecc521Public = 136,

    /// Ecc 256-bit private key
    Ecc256Private = 32,

    /// Ecc 384-bit private key
    Ecc384Private = 48,

    /// Ecc 521-bit private key
    Ecc521Private = 68,
}

impl EccKeyKind {
    #[allow(dead_code)]
    const MAX_KEY_LEN: EccKeyKind = EccKeyKind::Ecc521Public;
}

impl From<EccKeyKind> for EntryKind {
    fn from(kind: EccKeyKind) -> Self {
        match kind {
            EccKeyKind::Ecc256Public => EntryKind::Ecc256Public,
            EccKeyKind::Ecc384Public => EntryKind::Ecc384Public,
            EccKeyKind::Ecc521Public => EntryKind::Ecc521Public,
            EccKeyKind::Ecc256Private => EntryKind::Ecc256Private,
            EccKeyKind::Ecc384Private => EntryKind::Ecc384Private,
            EccKeyKind::Ecc521Private => EntryKind::Ecc521Private,
        }
    }
}

impl TryFrom<EntryKind> for EccKeyKind {
    type Error = HsmErr;

    fn try_from(kind: EntryKind) -> Result<Self, Self::Error> {
        match kind {
            EntryKind::Ecc256Private => Ok(EccKeyKind::Ecc256Private),
            EntryKind::Ecc384Private => Ok(EccKeyKind::Ecc384Private),
            EntryKind::Ecc521Private => Ok(EccKeyKind::Ecc521Private),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

impl From<EccCurve> for EccKeyKind {
    fn from(curve: EccCurve) -> Self {
        match curve {
            EccCurve::P256 => EccKeyKind::Ecc256Private,
            EccCurve::P384 => EccKeyKind::Ecc384Private,
            EccCurve::P521 => EccKeyKind::Ecc521Private,
        }
    }
}

impl From<EccKeyKind> for usize {
    fn from(kind: EccKeyKind) -> Self {
        kind as usize
    }
}

/// Elliptic Curve Cryptography (ECC) Key Usage
#[derive(Copy, Clone, PartialEq)]
pub(crate) enum EccKeyUsage {
    /// Sign and Verify usage
    SignVerify,

    /// Key Agreement usage
    KeyAgreement,
}

impl TryFrom<DdiKeyUsage> for EccKeyUsage {
    type Error = HsmErr;

    fn try_from(usage: DdiKeyUsage) -> Result<Self, Self::Error> {
        match usage {
            DdiKeyUsage::SignVerify => Ok(EccKeyUsage::SignVerify),
            DdiKeyUsage::Derive => Ok(EccKeyUsage::KeyAgreement),
            _ => Err(HsmErr::InvalidPermissions),
        }
    }
}

pub(crate) struct EccKey {
    key: VaultKey,
}

impl EccKey {
    pub fn new(key: VaultKey) -> Self {
        Self { key }
    }

    pub fn id(&self) -> u16 {
        self.key.id()
    }

    pub fn blob(&self) -> HsmResult<Ref<'_, [u8]>> {
        self.key.blob()
    }

    pub fn kind(&self) -> HsmResult<EntryKind> {
        self.key.kind()
    }

    pub fn usage_allowed(&self, usage: EccKeyUsage) -> Result<(), HsmErr> {
        let attributes = self.key.attributes()?;
        match usage {
            EccKeyUsage::SignVerify => {
                if !attributes.common.flags.sign() || !attributes.common.flags.verify() {
                    Err(HsmErr::InvalidPermissions)?
                }
            }
            EccKeyUsage::KeyAgreement => {
                if !attributes.common.flags.derive() {
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

impl TryFrom<VaultKey> for EccKey {
    type Error = HsmErr;

    fn try_from(key: VaultKey) -> Result<Self, Self::Error> {
        let class = key.class()?;
        if class != EntryClass::Ecc {
            Err(HsmErr::InvalidKeyType)?
        }

        Ok(Self::new(key))
    }
}

pub(crate) struct EccKeyImported<'a> {
    kind: EccKeyKind,
    usage: EccKeyUsage,
    blob: &'a [u8],
}

impl<'a> EccKeyImported<'a> {
    pub fn new(kind: EccKeyKind, usage: EccKeyUsage, val: &'a [u8]) -> Result<Self, HsmErr> {
        if val.len() != usize::from(kind) {
            Err(HsmErr::InvalidKeyType)?
        }

        Ok(Self {
            kind,
            usage,
            blob: val,
        })
    }

    pub fn kind(&self) -> EccKeyKind {
        self.kind
    }

    pub fn usage(&self) -> EccKeyUsage {
        self.usage
    }

    pub fn blob(&self) -> &[u8] {
        &self.blob[..usize::from(self.kind)]
    }
}

/// ECDH Key kind
#[derive(Clone, Copy, PartialEq)]
pub(crate) enum EcdhKeyKind {
    /// Ecdh 256-bit public key
    EcdhSecret256 = 32,

    /// Ecdh 384-bit key
    EcdhSecret384 = 48,

    /// Ecdh 521-bit key
    EcdhSecret521 = 68,
}

impl EcdhKeyKind {
    #[allow(dead_code)]
    const MAX_KEY_LEN: EcdhKeyKind = EcdhKeyKind::EcdhSecret521;
}

impl From<EcdhKeyKind> for EntryKind {
    fn from(kind: EcdhKeyKind) -> Self {
        match kind {
            EcdhKeyKind::EcdhSecret256 => EntryKind::Secret256,
            EcdhKeyKind::EcdhSecret384 => EntryKind::Secret384,
            EcdhKeyKind::EcdhSecret521 => EntryKind::Secret521,
        }
    }
}

impl TryFrom<EntryKind> for EcdhKeyKind {
    type Error = HsmErr;

    fn try_from(kind: EntryKind) -> Result<Self, Self::Error> {
        match kind {
            EntryKind::Secret256 => Ok(EcdhKeyKind::EcdhSecret256),
            EntryKind::Secret384 => Ok(EcdhKeyKind::EcdhSecret384),
            EntryKind::Secret521 => Ok(EcdhKeyKind::EcdhSecret521),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

impl From<EccCurve> for EcdhKeyKind {
    fn from(curve: EccCurve) -> Self {
        match curve {
            EccCurve::P256 => EcdhKeyKind::EcdhSecret256,
            EccCurve::P384 => EcdhKeyKind::EcdhSecret384,
            EccCurve::P521 => EcdhKeyKind::EcdhSecret521,
        }
    }
}

impl TryFrom<DdiKeyType> for EcdhKeyKind {
    type Error = HsmErr;

    fn try_from(kind: DdiKeyType) -> Result<Self, Self::Error> {
        match kind {
            DdiKeyType::Secret256 => Ok(EcdhKeyKind::EcdhSecret256),
            DdiKeyType::Secret384 => Ok(EcdhKeyKind::EcdhSecret384),
            DdiKeyType::Secret521 => Ok(EcdhKeyKind::EcdhSecret521),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

impl From<EcdhKeyKind> for usize {
    fn from(kind: EcdhKeyKind) -> Self {
        kind as usize
    }
}

/// Elliptic Curve Cryptography (ECC) Key Usage
#[derive(Copy, Clone, PartialEq)]
pub(crate) enum EcdhKeyUsage {
    /// Key Agreement usage
    KeyAgreement,
}

impl TryFrom<DdiKeyUsage> for EcdhKeyUsage {
    type Error = HsmErr;

    fn try_from(usage: DdiKeyUsage) -> Result<Self, Self::Error> {
        match usage {
            DdiKeyUsage::Derive => Ok(EcdhKeyUsage::KeyAgreement),
            _ => Err(HsmErr::InvalidPermissions),
        }
    }
}

pub(crate) struct EcdhKey {
    key: VaultKey,
}

impl EcdhKey {
    pub fn new(key: VaultKey) -> Self {
        Self { key }
    }

    pub fn id(&self) -> u16 {
        self.key.id()
    }

    pub fn blob(&self) -> HsmResult<Ref<'_, [u8]>> {
        self.key.blob()
    }

    pub fn usage_allowed(&self, usage: EcdhKeyUsage) -> Result<(), HsmErr> {
        let attributes = self.key.attributes()?;
        match usage {
            EcdhKeyUsage::KeyAgreement => {
                if !attributes.common.flags.derive() {
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

impl TryFrom<VaultKey> for EcdhKey {
    type Error = HsmErr;

    fn try_from(key: VaultKey) -> Result<Self, Self::Error> {
        let class = key.class()?;
        if class != EntryClass::Secret {
            Err(HsmErr::InvalidKeyType)?
        }

        Ok(Self::new(key))
    }
}

pub(crate) struct EcdhKeyImported<'a> {
    kind: EcdhKeyKind,
    usage: EcdhKeyUsage,
    blob: &'a [u8],
}

impl<'a> EcdhKeyImported<'a> {
    pub fn new(kind: EcdhKeyKind, usage: EcdhKeyUsage, val: &'a [u8]) -> Result<Self, HsmErr> {
        if val.len() != usize::from(kind) {
            Err(HsmErr::InvalidKeyType)?
        }

        Ok(Self {
            kind,
            usage,
            blob: val,
        })
    }

    pub fn kind(&self) -> EcdhKeyKind {
        self.kind
    }

    pub fn usage(&self) -> EcdhKeyUsage {
        self.usage
    }

    pub fn blob(&self) -> &[u8] {
        &self.blob[..usize::from(self.kind)]
    }
}
