// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::error::HsmErr;
use bitfield_struct::bitfield;
use mcr_ddi_types::DdiKeyClass;
use mcr_ddi_types::DdiKeyType;

/// Alignment of Entry blob blocks.
pub(crate) const ENTRY_BLOB_BLOCK_ALIGNMENT: usize = 8;

/// Key tag is not assigned.
pub(crate) const KEY_TAG_UNASSIGNED: u16 = 0;

/// Metadata flags for an Entry.
#[bitfield(u16)]
#[derive(Default, PartialEq, Eq)]
pub(crate) struct EntryFlags {
    /// Tells if the Entry was disabled or not
    disabled: bool,

    /// Tells if the Entry is available for session only or not
    pub(crate) session: bool,

    // Private flags internal to Entry.
    #[bits(14)]
    reserved: u16,
}

impl From<EntryAttributeFlags> for EntryFlags {
    fn from(flags: EntryAttributeFlags) -> Self {
        Self::default().with_session(flags.session())
    }
}

/// Kind of Cryptographic Key.
#[repr(u8)]
#[derive(Copy, Clone, PartialEq, Eq)]
pub(crate) enum EntryKind {
    /// Entry is available to be assigned.
    Free,

    /// RSA 2048-bit Public Key.
    Rsa2kPublic,

    /// RSA 3072-bit Public Key.
    Rsa3kPublic,

    /// RSA 4096-bit Public Key.
    Rsa4kPublic,

    /// RSA 2048-bit Private Key.
    Rsa2kPrivate,

    /// RSA 3072-bit Private Key.
    Rsa3kPrivate,

    /// RSA 4096-bit Private Key.
    Rsa4kPrivate,

    /// RSA 2048-bit Private CRT Key.
    Rsa2kPrivateCrt,

    /// RSA 3072-bit Private CRT Key.
    Rsa3kPrivateCrt,

    /// RSA 4096-bit Private CRT Key.
    Rsa4kPrivateCrt,

    /// ECC 256 Public Key
    Ecc256Public,

    /// ECC 384 Public Key
    Ecc384Public,

    /// ECC 521 Public Key
    Ecc521Public,

    /// ECC 256 Private Key
    Ecc256Private,

    /// ECC 384 Private Key
    Ecc384Private,

    /// ECC 521 Private Key
    Ecc521Private,

    /// AES 128-bit Key.
    Aes128,

    /// AES 192-bit Key.
    Aes192,

    /// AES 256-bit Key.
    Aes256,

    /// AES XTS 256-bit Key for Bulk Operations.
    AesXtsBulk256,

    /// AES GCM 256-bit Key for Bulk Operations.
    AesGcmBulk256,

    /// AES GCM 256-bit Unapproved Key for Bulk Operations.
    AesGcmBulk256Unapproved,

    /// 256-bit Secret from key exchange
    Secret256,

    /// 384-bit Secret from key exchange
    Secret384,

    /// 521-bit Secret from key exchange
    Secret521,

    /// ECC 384 Private + Public key used for establish cred
    EstablishCred,

    /// ECC 384 Private + Public key used for session encryption
    SessionEncryption,

    /// User session stores 8 bytes of API Revision + 80 bytes of masking key
    Session,

    /// HMAC 256-bit Key
    HmacSha256,

    /// HMAC 384-bit Key
    HmacSha384,

    /// HMAC 512-bit Key
    HmacSha512,

    /// Masking key.
    MaskingKey,

    /// Variable Length HMAC 256-bit Key
    VarLenHmacSha256,

    /// Variable Length HMAC 384-bit Key
    VarLenHmacSha384,

    /// Variable Length HMAC 512-bit Key
    VarLenHmacSha512,
}

impl EntryKind {
    /// Returns the raw size of the Entry.
    pub(crate) fn raw_key_blob_size(self) -> usize {
        match self {
            EntryKind::Free => 0,
            EntryKind::Rsa2kPublic => 260,
            EntryKind::Rsa3kPublic => 388,
            EntryKind::Rsa4kPublic => 516,
            EntryKind::Rsa2kPrivate => 516,
            EntryKind::Rsa3kPrivate => 772,
            EntryKind::Rsa4kPrivate => 1028,
            EntryKind::Rsa2kPrivateCrt => 1284,
            EntryKind::Rsa3kPrivateCrt => 1924,
            EntryKind::Rsa4kPrivateCrt => 2564,
            EntryKind::Ecc256Public => 64,
            EntryKind::Ecc384Public => 96,
            EntryKind::Ecc521Public => 136,
            EntryKind::Ecc256Private => 32,
            EntryKind::Ecc384Private => 48,
            EntryKind::Ecc521Private => 68,
            EntryKind::Aes128 => 16,
            EntryKind::Aes192 => 24,
            EntryKind::Aes256 => 32,
            EntryKind::AesXtsBulk256 => 2,
            EntryKind::AesGcmBulk256 => 2,
            EntryKind::AesGcmBulk256Unapproved => 2,
            EntryKind::Secret256 => 32,
            EntryKind::Secret384 => 48,
            EntryKind::Secret521 => 68,
            EntryKind::EstablishCred => 96 + 48,
            EntryKind::SessionEncryption => 96 + 48,
            EntryKind::Session => 8 + 80,
            EntryKind::HmacSha256 => 32,
            EntryKind::HmacSha384 => 48,
            EntryKind::HmacSha512 => 64,
            EntryKind::MaskingKey => 80,
            EntryKind::VarLenHmacSha256
            | EntryKind::VarLenHmacSha384
            | EntryKind::VarLenHmacSha512 => unreachable!(),
        }
    }

    /// Returns the aligned size of the Entry.
    pub(crate) fn aligned_key_blob_size(&self, key_len: Option<usize>) -> usize {
        let raw_size = if let Some(len) = key_len {
            len
        } else {
            self.raw_key_blob_size()
        };

        let alignment = ENTRY_BLOB_BLOCK_ALIGNMENT;
        let remainder = raw_size % alignment;
        if remainder == 0 {
            raw_size
        } else {
            raw_size + (alignment - remainder)
        }
    }

    /// Returns the total storage size of the Entry, which is the sum of
    /// the attributes blob size and the aligned key blob size.
    pub(crate) fn storage_size(&self, key_len: Option<usize>) -> usize {
        ATTRIBUTES_BLOB_SIZE + self.aligned_key_blob_size(key_len)
    }

    /// Tells if the EntryKind is a bulk key type.
    #[inline(never)]
    pub(crate) fn is_bulk_key(&self) -> bool {
        matches!(
            self,
            EntryKind::AesXtsBulk256
                | EntryKind::AesGcmBulk256
                | EntryKind::AesGcmBulk256Unapproved
        )
    }

    /// Tells if the EntryKind is a variable length HMAC key type.
    pub(crate) fn is_var_hmac_key(&self) -> bool {
        matches!(
            self,
            EntryKind::VarLenHmacSha256 | EntryKind::VarLenHmacSha384 | EntryKind::VarLenHmacSha512
        )
    }
}

/// Class of Cryptographic Key.
#[derive(Copy, Clone, PartialEq, Eq)]
pub(crate) enum EntryClass {
    /// Entry is available to be assigned.
    Free,

    /// RSA Key.
    Rsa,

    /// RSA CRT Key.
    RsaCrt,

    /// ECC Key.
    Ecc,

    /// AES Key.
    Aes,

    /// AES XTS Bulk Key.
    AesXtsBulk,

    /// AES GCM Bulk Key.
    AesGcmBulk,

    /// AES GCM Bulk Unapproved Key.
    AesGcmBulkUnapproved,

    /// Secret value from ECDH key exchange.
    Secret,

    /// Session
    Session,

    /// HMAC Key
    Hmac,

    /// Variable Length HMAC Key
    VarLenHmacSha,

    /// Masking Key
    MaskingKey,
}

impl EntryClass {
    /// Tells if the EntryClass is a bulk key type.
    #[inline(never)]
    pub(crate) fn is_bulk_key(&self) -> bool {
        matches!(
            self,
            EntryClass::AesXtsBulk | EntryClass::AesGcmBulk | EntryClass::AesGcmBulkUnapproved
        )
    }

    /// Converts the EntryClass AES Bulk key types to DdiKeyType bulk key types.
    #[inline(never)]
    pub(crate) fn aes_bulk_ddi_key_type(&self) -> Result<DdiKeyType, HsmErr> {
        match self {
            EntryClass::AesXtsBulk => Ok(DdiKeyType::AesXtsBulk256),
            EntryClass::AesGcmBulk => Ok(DdiKeyType::AesGcmBulk256),
            EntryClass::AesGcmBulkUnapproved => Ok(DdiKeyType::AesGcmBulk256Unapproved),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

impl TryFrom<DdiKeyClass> for EntryClass {
    type Error = HsmErr;

    /// Performs the conversion.
    fn try_from(key_class: DdiKeyClass) -> Result<Self, Self::Error> {
        match key_class {
            DdiKeyClass::Rsa => Ok(EntryClass::Rsa),
            DdiKeyClass::RsaCrt => Ok(EntryClass::RsaCrt),
            DdiKeyClass::Ecc => Ok(EntryClass::Ecc),
            DdiKeyClass::Aes => Ok(EntryClass::Aes),
            DdiKeyClass::AesXtsBulk => Ok(EntryClass::AesXtsBulk),
            DdiKeyClass::AesGcmBulk => Ok(EntryClass::AesGcmBulk),
            DdiKeyClass::AesGcmUnApprovedBulk => Ok(EntryClass::AesGcmBulkUnapproved),
            _ => Err(HsmErr::InvalidArgument),
        }
    }
}

impl From<EntryKind> for EntryClass {
    fn from(key_kind: EntryKind) -> Self {
        match key_kind {
            EntryKind::Rsa2kPublic
            | EntryKind::Rsa3kPublic
            | EntryKind::Rsa4kPublic
            | EntryKind::Rsa2kPrivate
            | EntryKind::Rsa3kPrivate
            | EntryKind::Rsa4kPrivate => EntryClass::Rsa,

            EntryKind::Rsa2kPrivateCrt
            | EntryKind::Rsa3kPrivateCrt
            | EntryKind::Rsa4kPrivateCrt => EntryClass::RsaCrt,

            EntryKind::Ecc256Public
            | EntryKind::Ecc384Public
            | EntryKind::Ecc521Public
            | EntryKind::Ecc256Private
            | EntryKind::Ecc384Private
            | EntryKind::Ecc521Private
            | EntryKind::EstablishCred
            | EntryKind::SessionEncryption => EntryClass::Ecc,

            EntryKind::Aes128 | EntryKind::Aes192 | EntryKind::Aes256 => EntryClass::Aes,

            EntryKind::AesXtsBulk256 => EntryClass::AesXtsBulk,
            EntryKind::AesGcmBulk256 => EntryClass::AesGcmBulk,
            EntryKind::AesGcmBulk256Unapproved => EntryClass::AesGcmBulkUnapproved,

            EntryKind::Secret256 | EntryKind::Secret384 | EntryKind::Secret521 => {
                EntryClass::Secret
            }

            EntryKind::Session => EntryClass::Session,

            EntryKind::HmacSha256 | EntryKind::HmacSha384 | EntryKind::HmacSha512 => {
                EntryClass::Hmac
            }

            EntryKind::Free => EntryClass::Free,

            EntryKind::MaskingKey => EntryClass::MaskingKey,

            EntryKind::VarLenHmacSha256
            | EntryKind::VarLenHmacSha384
            | EntryKind::VarLenHmacSha512 => EntryClass::VarLenHmacSha,
        }
    }
}

/// `Entry` is the metadata for a key in the table.
#[repr(C)]
#[derive(Clone)]
pub(crate) struct PhysicalEntry {
    /// This is the ENTRY_BLOB_BLOCK_ALIGNMENT byte aligned offset.
    /// We need 14 bits to do refer to each of 16K bytes. However, for 8 byte aligned (ENTRY_BLOB_BLOCK_ALIGNMENT)
    /// 3 bits can be saved as they will always be 0, so if needed making this 11 bits.
    offset: u16,

    /// Flags for the Entry.
    flags: EntryFlags,

    /// This is either the 1 byte session id or the 2 byte key tag.
    /// Session only keys will store the session id (130 possible app sessions).
    /// App keys will store the key tag.
    session_id_or_key_tag: u16,

    /// Kind of Entry. This is also used to determine the length of the Entry.
    /// Length can be upto 3K so 12 bits needed in addition to the type.
    /// Deriving the length from kind helps us save memory.
    kind: EntryKind,

    /// App id of the app that owns this Entry.
    /// There are 130 possible apps allowed so 1 byte will be needed.
    app_id: u8,
}

impl PhysicalEntry {
    /// Creates a new Entry.
    ///
    /// # Arguments
    /// * `offset` - Offset of the Entry in the table.
    /// * `flags` - Flags for the Entry.
    /// * `session_id_or_key_tag` - Session id (for session keys) or key tag (for app keys) for the Entry.
    /// * `kind` - Kind of Entry.
    /// * `app_id` - App id of the app that owns this Entry.
    pub(crate) fn new(
        offset: u16,
        flags: EntryFlags,
        session_id_or_key_tag: u16,
        kind: EntryKind,
        app_id: u8,
    ) -> Self {
        Self {
            offset,
            flags,
            session_id_or_key_tag,
            kind,
            app_id,
        }
    }

    pub(crate) fn raw_block_offset(&self) -> u16 {
        self.offset
    }

    pub(crate) fn attributes_bytes_offset(&self) -> usize {
        self.offset as usize * ENTRY_BLOB_BLOCK_ALIGNMENT
    }

    pub(crate) fn key_blob_bytes_offset(&self) -> usize {
        self.attributes_bytes_offset() + ATTRIBUTES_BLOB_SIZE
    }

    /// Fetches the flags of the Entry.
    pub(crate) fn flags(&self) -> EntryFlags {
        self.flags
    }

    /// Fetches the session id of the Entry.
    pub(crate) fn session_id(&self) -> Option<u16> {
        if self.flags.session() {
            Some(self.session_id_or_key_tag)
        } else {
            None
        }
    }

    /// Fetches the key tag of the Entry.
    pub(crate) fn key_tag(&self) -> Option<u16> {
        if self.flags.session() || self.session_id_or_key_tag == KEY_TAG_UNASSIGNED {
            None
        } else {
            Some(self.session_id_or_key_tag)
        }
    }

    /// Fetches the kind of the Entry.
    pub(crate) fn kind(&self) -> EntryKind {
        self.kind
    }

    /// Tells if the Entry is free.
    pub(crate) fn is_empty(&self) -> bool {
        self.kind == EntryKind::Free
    }

    /// Fetches the app id of the Entry.
    pub(crate) fn app_id(&self) -> u8 {
        self.app_id
    }

    /// Tells if the Entry is disabled.
    pub(crate) fn disabled(&self) -> bool {
        self.flags.disabled()
    }

    /// Disables the Entry.
    pub(crate) fn disable(&mut self) {
        self.flags.set_disabled(true);
    }

    /// Enables the Entry.
    pub(crate) fn enable(&mut self) {
        self.flags.set_disabled(false);
    }
}

#[cfg(test)]
mod tests {

    use super::*;

    #[test]
    fn test_table_entry_flags_size() {
        assert_eq!(core::mem::size_of::<EntryFlags>(), 2);
    }

    #[test]
    fn test_table_entry_kind_size() {
        assert_eq!(core::mem::size_of::<EntryKind>(), 1);
    }

    #[test]
    fn test_table_entry_size() {
        assert_eq!(core::mem::size_of::<PhysicalEntry>(), 8);
    }

    #[test]
    fn test_raw_key_blob_size() {
        assert_eq!(EntryKind::Free.raw_key_blob_size(), 0);
        assert_eq!(EntryKind::Rsa2kPublic.raw_key_blob_size(), 260);
        assert_eq!(EntryKind::Rsa3kPublic.raw_key_blob_size(), 388);
        assert_eq!(EntryKind::Rsa4kPublic.raw_key_blob_size(), 516);
        assert_eq!(EntryKind::Rsa2kPrivate.raw_key_blob_size(), 516);
        assert_eq!(EntryKind::Rsa3kPrivate.raw_key_blob_size(), 772);
        assert_eq!(EntryKind::Rsa4kPrivate.raw_key_blob_size(), 1028);
        assert_eq!(EntryKind::Rsa2kPrivateCrt.raw_key_blob_size(), 1284);
        assert_eq!(EntryKind::Rsa3kPrivateCrt.raw_key_blob_size(), 1924);
        assert_eq!(EntryKind::Rsa4kPrivateCrt.raw_key_blob_size(), 2564);
        assert_eq!(EntryKind::Ecc256Public.raw_key_blob_size(), 64);
        assert_eq!(EntryKind::Ecc384Public.raw_key_blob_size(), 96);
        assert_eq!(EntryKind::Ecc521Public.raw_key_blob_size(), 136);
        assert_eq!(EntryKind::Ecc256Private.raw_key_blob_size(), 32);
        assert_eq!(EntryKind::Ecc384Private.raw_key_blob_size(), 48);
        assert_eq!(EntryKind::Ecc521Private.raw_key_blob_size(), 68);
        assert_eq!(EntryKind::Aes128.raw_key_blob_size(), 16);
        assert_eq!(EntryKind::Aes192.raw_key_blob_size(), 24);
        assert_eq!(EntryKind::Aes256.raw_key_blob_size(), 32);
        assert_eq!(EntryKind::AesXtsBulk256.raw_key_blob_size(), 2);
        assert_eq!(EntryKind::AesGcmBulk256.raw_key_blob_size(), 2);
        assert_eq!(EntryKind::AesGcmBulk256Unapproved.raw_key_blob_size(), 2);
        assert_eq!(EntryKind::Secret256.raw_key_blob_size(), 32);
        assert_eq!(EntryKind::MaskingKey.raw_key_blob_size(), 80);
    }

    #[test]
    fn test_aligned_key_blob_size() {
        assert!(EntryKind::Free.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0);
        assert!(
            EntryKind::Rsa2kPublic.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0
        );
        assert!(
            EntryKind::Rsa3kPublic.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0
        );
        assert!(
            EntryKind::Rsa4kPublic.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0
        );
        assert!(
            EntryKind::Rsa2kPrivate.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0
        );
        assert!(
            EntryKind::Rsa3kPrivate.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0
        );
        assert!(
            EntryKind::Rsa4kPrivate.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0
        );
        assert!(
            EntryKind::Rsa2kPrivateCrt.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT
                == 0
        );
        assert!(
            EntryKind::Rsa3kPrivateCrt.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT
                == 0
        );
        assert!(
            EntryKind::Rsa4kPrivateCrt.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT
                == 0
        );
        assert!(
            EntryKind::Ecc256Public.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0
        );
        assert!(
            EntryKind::Ecc384Public.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0
        );
        assert!(
            EntryKind::Ecc521Public.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0
        );
        assert!(
            EntryKind::Ecc256Private.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0
        );
        assert!(
            EntryKind::Ecc384Private.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0
        );
        assert!(
            EntryKind::Ecc521Private.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0
        );
        assert!(EntryKind::Aes128.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0);
        assert!(EntryKind::Aes192.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0);
        assert!(EntryKind::Aes256.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0);
        assert!(
            EntryKind::AesXtsBulk256.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0
        );
        assert!(
            EntryKind::AesGcmBulk256.aligned_key_blob_size(None) % ENTRY_BLOB_BLOCK_ALIGNMENT == 0
        );
        assert!(
            EntryKind::AesGcmBulk256Unapproved.aligned_key_blob_size(None)
                % ENTRY_BLOB_BLOCK_ALIGNMENT
                == 0
        );
        assert!(
            EntryKind::VarLenHmacSha256.aligned_key_blob_size(Some(38))
                % ENTRY_BLOB_BLOCK_ALIGNMENT
                == 0
        );
        assert!(
            EntryKind::VarLenHmacSha256.aligned_key_blob_size(Some(55))
                % ENTRY_BLOB_BLOCK_ALIGNMENT
                == 0
        );
        assert!(
            EntryKind::VarLenHmacSha256.aligned_key_blob_size(Some(100))
                % ENTRY_BLOB_BLOCK_ALIGNMENT
                == 0
        );
    }

    #[test]
    fn test_entry_flags() {
        let mut entry_flags = EntryFlags::new();
        assert_eq!(entry_flags.0, 0);

        entry_flags.set_disabled(false);
        entry_flags.set_session(true);

        assert!(!entry_flags.disabled());
        assert!(entry_flags.session());
    }
}
