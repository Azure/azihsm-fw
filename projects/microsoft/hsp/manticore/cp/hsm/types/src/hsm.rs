// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::*;
use mcr_ddi_types::DdiSoftAesOp;
use mcr_ddi_types::AES_CBC_256_KEY_SIZE;
use mcr_ddi_types::HMAC384_KEY_SIZE;
use zerocopy::{FromBytes, Immutable, IntoBytes};

pub const MAX_ALLOWED_PIN_AUTH_ATTEMPTS: u16 = 1000;

pub const MAX_PART_CERT_LENGTH: u32 = 800;

pub const MASKED_BK_BOOT_SIZE: usize = 300;

pub const SEALED_BK3_SIZE: usize = 512;

pub const BK3_SIZE: usize = 48;

pub const BK_AES_CBC_256_HMAC384_SIZE_BYTES: usize = AES_CBC_256_KEY_SIZE + HMAC384_KEY_SIZE;

/// Application Id
pub type AppId = [u8; 16];

/// Application Pin
pub type AppPin = [u8; 16];

/// Application Vault Id
pub type AppVaultId = u8;

/// Session Id
pub type SessionId = u16;

/// Virtual Machine Launch GUID
pub type VmLaunchGuid = [u8; 16];

/// Hsm Partition ID
pub type HsmPartitionId = [u8; 16];

/// User Credential
#[repr(C)]
#[derive(Default, Clone, Copy, PartialEq, Eq)]
pub struct UserCredential {
    /// User ID
    pub id: AppId,

    /// User PIN
    pub pin: AppPin,

    /// Vault ID
    pub vault_id: AppVaultId,
}

/// States for Pin Policy
#[repr(u8)]
#[derive(Default, Copy, Clone, PartialEq, Eq)]
pub enum PinPolicyState {
    /// Unrestricted state
    #[default]
    Ready,

    /// Lockout state; enforce delay factor
    Lockout,
}

/// Pin Policy Context
#[repr(C)]
#[derive(Default, Clone, Copy)]
pub struct PinPolicy {
    /// State
    pub state: PinPolicyState,

    /// Delay Factor
    pub delay_factor: u16,

    /// Allowed Attempts
    pub allowed_attempts: u16,

    /// Lockout Time
    pub lockout_time: [u8; 8],
}

static_assertions::const_assert_eq!(14, size_of::<PinPolicy>());

/// Partition data store
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub struct PartStore {
    /// Unwrapping Key ID
    pub unwrapping_key_id: Option<u16>,

    /// Establish Cred Encryption Key ID
    pub establish_cred_encryption_key_id: Option<u16>,

    /// Session Encryption Key ID
    pub session_encryption_key_id: Option<u16>,

    /// partition masking key
    pub masking_key: Option<u16>,

    /// User credential (ID, PIN, AppVaultId)
    pub user_cred: UserCredential,

    /// Function Enabled
    pub enabled: bool,

    /// Reserved
    pub _reserved2: [u8; 18],
}

static_assertions::const_assert_eq!(size_of::<PartStore>(), 68);

/// HSM Partition data store
#[repr(C)]
pub struct HsmPartDataStore {
    /// Data Store Version Major
    pub version_major: u16,

    /// Data Store Version Minior
    pub version_minor: u16,

    /// Partition specific data
    pub part: [PartStore; MAX_PCIE_FUNCTIONS],

    /// Reserved
    pub _reserved: [u8; 1024],
}

impl Default for HsmPartDataStore {
    fn default() -> Self {
        Self {
            version_minor: 0,
            version_major: 1,
            part: [Default::default(); MAX_PCIE_FUNCTIONS],
            _reserved: [Default::default(); 1024],
        }
    }
}

/// Soft AES off load Request
#[repr(C)]
#[derive(Clone, Copy)]
pub struct SoftAesOffloadReq {
    /// Key encryption key
    pub key: IoMemRange,

    /// Input
    pub inout: IoMemRange,

    /// Tag
    pub tag: u16,

    /// Op
    pub op: SoftAesOp,
}

/// AES KWP or ECB Op
#[repr(u8)]
#[derive(Default, Clone, Copy)]
pub enum SoftAesOp {
    /// Key Wrap with Padding Op
    #[default]
    Kwp = 0,

    /// AES ECB Decrypt Op
    EcbDecrypt = 1,
}

/// Convert DdiSoftAesOp to SoftAesOp
impl From<DdiSoftAesOp> for SoftAesOp {
    fn from(value: DdiSoftAesOp) -> Self {
        match value {
            DdiSoftAesOp::Kwp => SoftAesOp::Kwp,
            DdiSoftAesOp::EcbDecrypt => SoftAesOp::EcbDecrypt,
        }
    }
}

/// Convert from SoftAesOp to DdiSoftAesOp
impl From<SoftAesOp> for DdiSoftAesOp {
    fn from(value: SoftAesOp) -> Self {
        match value {
            SoftAesOp::Kwp => DdiSoftAesOp::Kwp,
            SoftAesOp::EcbDecrypt => DdiSoftAesOp::EcbDecrypt,
        }
    }
}

/// Soft AES response
#[repr(C)]
#[derive(Clone, Copy)]
pub struct SoftAesOffloadResp {
    /// Output start index
    pub range: Result<(usize, usize), u32>,

    /// Tag
    pub tag: u16,
}

/// Length of Session Table
pub const SESSION_TABLE_LEN: usize = 18;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct PartitionIdentifier {
    /// Partition ID
    pub id: HsmPartitionId,

    /// Partition ID Private Key
    pub priv_key: [u8; 48],

    /// Partition ID Public Key
    pub pub_key: [u8; 97],
}

/// Partition Certificate
#[repr(C)]
#[derive(Clone, Copy)]
pub struct PartitionCert {
    /// Cert length
    pub length: u32,

    /// Cert data
    pub data: [u8; MAX_PART_CERT_LENGTH as usize],
}

#[repr(C, packed)]
#[derive(FromBytes, IntoBytes, Immutable, Clone, Copy)]
pub struct MaskedBkBoot {
    /// Length
    pub len: u32,

    /// Boot Data
    pub data: [u8; MASKED_BK_BOOT_SIZE],
}

#[repr(C, packed)]
#[derive(FromBytes, IntoBytes, Immutable, Clone, Copy)]
pub struct SealedBk3 {
    /// Boot Info
    pub len: u32,

    /// Boot Data
    pub data: [u8; SEALED_BK3_SIZE],
}

/// BK3 Session Key
#[repr(C, packed)]
#[derive(Clone, Copy)]
pub struct Bk3SessionKey {
    /// Flag to indicate if the bk3_session_key buffer contains valid key
    pub is_valid: bool,

    /// Reserved for 4 byte alignment
    pub _reserved: [u8; 3],

    /// BK3 Session Key
    pub key: [u8; BK3_SIZE],
}

/// HSM Partition Persistent Data Store
#[repr(C)]
pub struct HsmPartPersistentStore {
    /// Version
    pub version: u8,

    /// Flags
    pub flags: [u8; 3],

    /// Session Table
    pub session_table: [u8; SESSION_TABLE_LEN],

    /// Reserved to allow 4 byte alignment for RSA Unwrapping Key
    pub reserved1: u8,

    /// Flag to indicate if the unwrapping RSA2K private key buffer contains valid key
    pub unwrapping_key_bk_valid: bool,

    /// Unwrapping RSA2K private key
    pub unwrapping_key_bk: [u8; 516],

    /// Pin Policy Context
    pub pin_policy: PinPolicy,

    /// VM launch GUID set by the host driver.
    pub vm_launch_guid: VmLaunchGuid,

    /// Partition ID valid
    pub partition_id_valid: bool,

    /// Padding to ensure 4-byte alignment for partition_identifier
    pub reserved2: u8,

    /// Partition Identifier
    pub partition_identifier: PartitionIdentifier,

    /// partition certificate valid
    pub partition_cert_valid: bool,

    /// partition certificate
    pub partition_cert: PartitionCert,

    /// Masked Boot Key
    pub masked_bk_boot: MaskedBkBoot,

    /// Sealed Backup Key 3
    pub sealed_bk3: SealedBk3,

    /// Nonce
    pub nonce: [u8; 32],

    /// BK3 Session Key
    pub bk3_session_key: Bk3SessionKey,

    /// Reserved for future enhancements
    pub reserved3: [u8; 626],
}

static_assertions::const_assert_eq!(
    0,
    core::mem::offset_of!(HsmPartPersistentStore, partition_identifier) % 4
);
static_assertions::const_assert_eq!(size_of::<HsmPartPersistentStore>(), 3072);
static_assertions::const_assert_eq!(
    0,
    core::mem::offset_of!(HsmPartPersistentStore, bk3_session_key.key) % 4
);
/// HSM POR measurement data
#[repr(C)]
pub struct PorMeasurementData {
    /// Measurement data
    pub por_measurement: [u8; 2799],

    /// FIPS Approved
    pub fips_approved: u8,
}
static_assertions::const_assert_eq!(size_of::<PorMeasurementData>(), 2800);

/// Entry representation in BKS table
#[repr(C)]
#[derive(Clone, Copy)]
pub struct BksTableEntry {
    /// Valid flag
    pub valid: u8,

    /// SVN in little-endian
    pub svn: [u8; 8],

    /// BKS
    pub bks: [u8; 32],
}
static_assertions::const_assert_eq!(size_of::<BksTableEntry>(), 41);
