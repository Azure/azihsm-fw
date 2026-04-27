// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

pub(super) const COMMON_ATTRIBUTES_BLOB_SIZE: usize = 24;
pub(super) const ENTRY_SPECIFIC_ATTRIBUTES_BLOB_SIZE: usize = 8;

#[repr(C)]
#[derive(Default, FromBytes, IntoBytes, Immutable, KnownLayout)]
pub(crate) struct EntryAttributes {
    pub(crate) common: CommonEntryAttributes,
    pub(crate) entry_specific: [u8; ENTRY_SPECIFIC_ATTRIBUTES_BLOB_SIZE],
}

// Size of the EntryAttributes structure must be ATTRIBUTES_BLOB_SIZE bytes.
static_assertions::const_assert_eq!(size_of::<EntryAttributes>(), ATTRIBUTES_BLOB_SIZE);

#[repr(C)]
#[derive(Default, FromBytes, IntoBytes, Immutable, KnownLayout)]
pub(crate) struct CommonEntryAttributes {
    pub(crate) flags: EntryAttributeFlags,
    rsvd: [u8; COMMON_ATTRIBUTES_BLOB_SIZE - size_of::<EntryAttributeFlags>()],
}

// Size of the CommonEntryAttributes structure must be COMMON_ATTRIBUTES_BLOB_SIZE bytes.
static_assertions::const_assert_eq!(
    size_of::<CommonEntryAttributes>(),
    COMMON_ATTRIBUTES_BLOB_SIZE
);

#[bitfield(u64)]
#[derive(Default, PartialEq, Eq, FromBytes, IntoBytes, Immutable, KnownLayout)]
pub(crate) struct EntryAttributeFlags {
    /// Flag indicating if the key is internal or not. Internal keys are used by the device
    /// internally and are not destroyable by the user.
    pub(crate) internal: bool,

    /// Flag indicating if the key is a session key.
    pub(crate) session: bool,

    /// Flag indicating the key is private or not. If the key is private an authenticated session
    /// must be established. All keys generated within the session are private. This flag is set
    /// by the device for keys that can be accessed with establishing a session.
    pub(crate) private: bool,

    /// Flag indicating the key is modifiable or not.
    pub(crate) modifiable: bool,

    /// Flag indicating the key is destroyable or not. All keys created in a session are
    /// destroyable. Device generated keys may be marked as not destroyable.
    pub(crate) destroyable: bool,

    /// Flag indicating the key is locally generated or imported. The flag is set by the device
    /// and cannot be changed via the API.
    pub(crate) local: bool,

    /// Flag indicating the value of the key is extractable from the device or not. All session
    /// keys are always extractable. Device generated keys may be marked as not extractable.
    pub(crate) extractable: bool,

    /// Flag indicating the key has ever been marked not extractable. All session keys are
    /// marked always extractable. Device generated keys may be marked as never extractable.
    pub(crate) never_extractable: bool,

    /// Flag indicating the key can be trusted to wrap keys. This flag can only be specified for
    /// Public Keys. Private & Shared keys will report this flag as not set.
    pub(crate) trusted: bool,

    /// Flag indicating that a key can only be wrapped with a key that is marked trusted. This
    /// property is applicable to Private and Shared keys. All private and secret keys generate
    /// in session are marked with this property.
    pub(crate) wrap_with_trusted: bool,

    /// Flag indicating if the key can be used for encrypt operations. This flag can be
    /// specified only for Public Keys and Secret Keys.
    pub(crate) encrypt: bool,

    /// Flag indicating if the key can be used for decrypt operations. This flag can be
    /// specified only for Private and Secret Keys.
    pub(crate) decrypt: bool,

    /// Flag indicating if the key can be used for sign operations. This flag can be
    /// specified only for Private Keys and Secret Keys.
    pub(crate) sign: bool,

    /// Flag indicating if the key can be used for verify operations. This flag can be
    /// specified only for Public and Secret Keys.
    pub(crate) verify: bool,

    /// Flag indicating if the key can be used for wrap operations. This flag can be
    /// specified only for Public Keys and Secret Keys.
    pub(crate) wrap: bool,

    /// Flag indicating if the key can be used for unwrap operations. This flag can be
    /// specified only for Private and Secret Keys.
    pub(crate) unwrap: bool,

    /// Flag indicating if the key can be used for derive operations. This flag can be
    /// specified only for Secret Keys.
    pub(crate) derive: bool,

    // Private flags internal to Entry.
    #[bits(47)]
    rsvd: u64,
}
