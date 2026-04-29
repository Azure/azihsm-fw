// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_types::DdiKeyUsage;
use mcr_ddi_types::DdiRsaOpType;

use super::*;

/// RSA Key kind
#[derive(Clone, Copy, PartialEq)]
pub(crate) enum RsaKeyKind {
    /// RSA 2048-bit private key
    Rsa2kPrivate = 516,

    /// RSA 3072-bit private key
    Rsa3kPrivate = 772,

    /// RSA 4096-bit private key
    Rsa4kPrivate = 1028,

    /// RSA 2048-bit private CRT key
    Rsa2kPrivateCrt = 1284,

    /// RSA 3072-bit private CRT key
    Rsa3kPrivateCrt = 1924,

    /// RSA 4096-bit private CRT key
    Rsa4kPrivateCrt = 2564,
}

impl RsaKeyKind {
    #[allow(unused)]
    const MAX_KEY_LEN: RsaKeyKind = RsaKeyKind::Rsa4kPrivateCrt;
}

impl From<RsaKeyKind> for EntryKind {
    fn from(kind: RsaKeyKind) -> Self {
        match kind {
            RsaKeyKind::Rsa2kPrivate => EntryKind::Rsa2kPrivate,
            RsaKeyKind::Rsa3kPrivate => EntryKind::Rsa3kPrivate,
            RsaKeyKind::Rsa4kPrivate => EntryKind::Rsa4kPrivate,
            RsaKeyKind::Rsa2kPrivateCrt => EntryKind::Rsa2kPrivateCrt,
            RsaKeyKind::Rsa3kPrivateCrt => EntryKind::Rsa3kPrivateCrt,
            RsaKeyKind::Rsa4kPrivateCrt => EntryKind::Rsa4kPrivateCrt,
        }
    }
}

impl TryFrom<EntryKind> for RsaKeyKind {
    type Error = HsmErr;

    fn try_from(kind: EntryKind) -> Result<Self, Self::Error> {
        match kind {
            EntryKind::Rsa2kPrivate => Ok(RsaKeyKind::Rsa2kPrivate),
            EntryKind::Rsa3kPrivate => Ok(RsaKeyKind::Rsa3kPrivate),
            EntryKind::Rsa4kPrivate => Ok(RsaKeyKind::Rsa4kPrivate),
            EntryKind::Rsa2kPrivateCrt => Ok(RsaKeyKind::Rsa2kPrivateCrt),
            EntryKind::Rsa3kPrivateCrt => Ok(RsaKeyKind::Rsa3kPrivateCrt),
            EntryKind::Rsa4kPrivateCrt => Ok(RsaKeyKind::Rsa4kPrivateCrt),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

impl From<RsaKeyKind> for usize {
    fn from(kind: RsaKeyKind) -> Self {
        kind as usize
    }
}

/// RSA Key Usage
#[derive(Copy, Clone, PartialEq)]
pub(crate) enum RsaKeyUsage {
    /// Sign and Verify usage
    SignVerify,

    /// Encrypt and Decrypt usage
    EncryptDecrypt,

    /// Unwrap usage
    Unwrap,
}

impl TryFrom<DdiRsaOpType> for RsaKeyUsage {
    type Error = HsmErr;

    fn try_from(op: DdiRsaOpType) -> Result<Self, Self::Error> {
        match op {
            DdiRsaOpType::Sign => Ok(RsaKeyUsage::SignVerify),
            DdiRsaOpType::Decrypt => Ok(RsaKeyUsage::EncryptDecrypt),
            _ => Err(HsmErr::InvalidArgument),
        }
    }
}

impl TryFrom<DdiKeyUsage> for RsaKeyUsage {
    type Error = HsmErr;

    fn try_from(usage: DdiKeyUsage) -> Result<Self, Self::Error> {
        match usage {
            DdiKeyUsage::SignVerify => Ok(RsaKeyUsage::SignVerify),
            DdiKeyUsage::EncryptDecrypt => Ok(RsaKeyUsage::EncryptDecrypt),
            _ => Err(HsmErr::InvalidPermissions),
        }
    }
}

pub(crate) struct RsaKey {
    key: VaultKey,
}

impl RsaKey {
    pub fn new(key: VaultKey) -> Self {
        Self { key }
    }

    pub fn id(&self) -> u16 {
        self.key.id()
    }

    pub fn blob(&self) -> HsmResult<Ref<'_, [u8]>> {
        if self.is_crt_key()? {
            Err(HsmErr::InvalidKeyType)?
        }

        self.key.blob()
    }

    /// Get the modulus, n
    pub fn n(&self) -> HsmResult<Ref<'_, [u8]>> {
        let blob = self.key.blob()?;
        let full_operand_len = self.expected_data_len()?;

        let (start, end) = if self.is_crt_key()? {
            // For crt keys the modulus n is the section of the keyblob
            // from crt_param1_len..crt_param1_len + full_operand_len
            let start = self.crt_param1_len()?;
            (start, start + full_operand_len)
        } else {
            // For non crt keys the modulus n is the section of the keyblob
            // from data_len..data_len + data_len
            (full_operand_len, full_operand_len * 2)
        };

        Ok(Ref::map(blob, |blob| &blob[start..end]))
    }

    fn crt_param1_len(&self) -> HsmResult<usize> {
        match self.kind()? {
            EntryKind::Rsa2kPrivateCrt => Ok(512),
            EntryKind::Rsa3kPrivateCrt => Ok(768),
            EntryKind::Rsa4kPrivateCrt => Ok(1024),
            _ => Err(HsmErr::InvalidKeyType)?,
        }
    }

    pub fn crt_param1(&self) -> HsmResult<Ref<'_, [u8]>> {
        let crt_param1_len = self.crt_param1_len()?;

        let blob = self.key.blob()?;

        let (crt_param1, _) = Ref::map_split(blob, |slice| slice.split_at(crt_param1_len));

        Ok(crt_param1)
    }

    pub fn crt_param2(&self) -> HsmResult<Ref<'_, [u8]>> {
        let crt_param1_len = self.crt_param1_len()?;

        let blob = self.key.blob()?;

        let (_, crt_param2) = Ref::map_split(blob, |slice| slice.split_at(crt_param1_len));

        Ok(crt_param2)
    }

    pub fn kind(&self) -> HsmResult<EntryKind> {
        self.key.kind()
    }

    pub fn usage_allowed(&self, usage: RsaKeyUsage) -> Result<(), HsmErr> {
        let attributes = self.key.attributes()?;
        match usage {
            RsaKeyUsage::SignVerify => {
                if !attributes.common.flags.sign() || !attributes.common.flags.verify() {
                    Err(HsmErr::InvalidPermissions)?
                }
            }
            RsaKeyUsage::EncryptDecrypt => {
                if !attributes.common.flags.encrypt() || !attributes.common.flags.decrypt() {
                    Err(HsmErr::InvalidPermissions)?
                }
            }
            RsaKeyUsage::Unwrap => {
                if !attributes.common.flags.unwrap() {
                    Err(HsmErr::InvalidPermissions)?
                }
            }
        }

        Ok(())
    }

    pub fn disabled(&self) -> HsmResult<bool> {
        self.key.disabled()
    }

    pub fn expected_data_len(&self) -> HsmResult<usize> {
        let kind = self.kind()?;
        let size = match kind {
            EntryKind::Rsa2kPrivate | EntryKind::Rsa2kPrivateCrt => 2048 / 8,
            EntryKind::Rsa3kPrivate | EntryKind::Rsa3kPrivateCrt => 3072 / 8,
            EntryKind::Rsa4kPrivate | EntryKind::Rsa4kPrivateCrt => 4096 / 8,
            _ => Err(HsmErr::InvalidKeyType)?,
        };

        Ok(size)
    }

    pub fn is_crt_key(&self) -> HsmResult<bool> {
        let kind = self.kind()?;
        Ok(kind == EntryKind::Rsa2kPrivateCrt
            || kind == EntryKind::Rsa3kPrivateCrt
            || kind == EntryKind::Rsa4kPrivateCrt)
    }
}

impl TryFrom<VaultKey> for RsaKey {
    type Error = HsmErr;

    fn try_from(key: VaultKey) -> Result<Self, Self::Error> {
        let class = key.class()?;
        if class != EntryClass::Rsa && class != EntryClass::RsaCrt {
            Err(HsmErr::InvalidKeyType)?
        }

        Ok(Self::new(key))
    }
}

pub(crate) struct RsaKeyImported<'a> {
    kind: RsaKeyKind,
    usage: RsaKeyUsage,
    blob: &'a [u8],
}

impl<'a> RsaKeyImported<'a> {
    pub fn new(kind: RsaKeyKind, usage: RsaKeyUsage, val: &'a [u8]) -> Result<Self, HsmErr> {
        if val.len() != usize::from(kind) {
            Err(HsmErr::InvalidKeyType)?
        }

        Ok(Self {
            kind,
            usage,
            blob: val,
        })
    }

    pub fn kind(&self) -> RsaKeyKind {
        self.kind
    }

    pub fn usage(&self) -> RsaKeyUsage {
        self.usage
    }

    pub fn blob(&self) -> &[u8] {
        &self.blob[..usize::from(self.kind)]
    }
}
