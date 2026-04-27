// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_types::DdiKeyUsage;

use crate::lm_key_derive::MK_AES_CBC_256_HMAC384_SIZE_BYTES;

use super::*;

const SESSION_KEY_SIZE: usize = 8;

/// Session Key Kind
#[derive(Clone, Copy, PartialEq)]
pub(crate) enum SessionKeyKind {
    /// Session stores 8 bytes of api version and 80 bytes of masking key
    Session = (SESSION_KEY_SIZE + MK_AES_CBC_256_HMAC384_SIZE_BYTES) as isize,
}

impl SessionKeyKind {
    const MAX_KEY_LEN: SessionKeyKind = SessionKeyKind::Session;
}

impl From<SessionKeyKind> for EntryKind {
    fn from(_: SessionKeyKind) -> Self {
        EntryKind::Session
    }
}

impl From<SessionKeyKind> for usize {
    fn from(kind: SessionKeyKind) -> Self {
        kind as usize
    }
}

/// Session Key Usage
#[derive(Copy, Clone, PartialEq)]
pub(crate) enum SessionKeyUsage {
    /// Key usage
    Session,
}

impl TryFrom<DdiKeyUsage> for SessionKeyUsage {
    type Error = HsmErr;

    fn try_from(_usage: DdiKeyUsage) -> Result<Self, Self::Error> {
        Err(HsmErr::InvalidPermissions)
    }
}

pub(crate) struct SessionKey {
    key: VaultKey,
}

impl SessionKey {
    pub fn new(key: VaultKey) -> Self {
        Self { key }
    }

    pub fn id(&self) -> u16 {
        self.key.id()
    }

    pub fn session_key_blob(&self) -> HsmResult<Ref<'_, [u8]>> {
        self.key
            .blob()
            .map(|blob| Ref::map(blob, |b| &b[..SESSION_KEY_SIZE]))
    }

    pub fn masking_key_blob(&self) -> HsmResult<Ref<'_, [u8]>> {
        self.key
            .blob()
            .map(|blob| Ref::map(blob, |b| &b[SESSION_KEY_SIZE..]))
    }
}

impl TryFrom<VaultKey> for SessionKey {
    type Error = HsmErr;

    fn try_from(key: VaultKey) -> Result<Self, Self::Error> {
        let kind = key.kind()?;
        if kind != EntryKind::Session {
            Err(HsmErr::InvalidKeyType)?
        }

        Ok(Self::new(key))
    }
}

pub(crate) struct SessionKeyToImport {
    kind: SessionKeyKind,
    _usage: SessionKeyUsage,
    blob: [u8; SessionKeyKind::MAX_KEY_LEN as usize],
}

impl SessionKeyToImport {
    pub fn new(
        kind: SessionKeyKind,
        usage: SessionKeyUsage,
        sk: &[u8],
        smk: &[u8],
    ) -> Result<Self, HsmErr> {
        if sk.len() != SESSION_KEY_SIZE || smk.len() != MK_AES_CBC_256_HMAC384_SIZE_BYTES {
            Err(HsmErr::InvalidKeyType)?
        }

        let mut blob = [0; SessionKeyKind::MAX_KEY_LEN as usize];
        blob[..SESSION_KEY_SIZE].copy_from_slice(sk);
        blob[SESSION_KEY_SIZE..].copy_from_slice(smk);

        Ok(Self {
            kind,
            _usage: usage,
            blob,
        })
    }

    pub fn kind(&self) -> SessionKeyKind {
        self.kind
    }

    pub fn _usage(&self) -> SessionKeyUsage {
        self._usage
    }

    pub fn blob(&self) -> &[u8] {
        &self.blob[..usize::from(self.kind)]
    }
}
