// Copyright (c) Microsoft Corporation. All rights reserved.

use bitfield_struct::bitfield;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::MemoryAddr;

pub const CDMA_CMD_TYPE_FAST_PATH: u8 = 0x5;
const CDMA_IO_RX_ENTRY_SIZE: usize = 128;
#[allow(dead_code)]
const CDMA_IO_TX_ENTRY_SIZE: usize = 64;

/// AES Bulk 256 key ID
#[bitfield(u16)]
#[derive(Default, PartialEq, Eq, IntoBytes, Immutable, FromBytes)]
pub struct AesBulk256KeyId {
    /// Key Index
    #[bits(3)]
    pub key_index: u8,

    /// Vault Id
    #[bits(7)]
    pub vault_id: u8,

    /// Reserved
    #[bits(6)]
    pub rsvd: u8,
}

/// AES FP command mode
#[derive(Copy, Clone)]
pub enum AesFpMode {
    /// XTS Encrypt
    XtsEncrypt = 1,

    /// XTS Decrypt
    XtsDecrypt = 2,

    /// GCM Encrypt
    GcmEncrypt = 3,

    /// GCM Decrypt
    GcmDecrypt = 4,
}

/// Convert from AesFpMode to u8
impl From<AesFpMode> for u8 {
    fn from(value: AesFpMode) -> Self {
        value as Self
    }
}

/// u32 array to store SQE data
#[repr(C, align(4))]
#[derive(Default, Clone, Copy, FromBytes, IntoBytes, Immutable)]
pub struct CdmaIoSqe {
    pub data: [u32; CDMA_IO_RX_ENTRY_SIZE / 4],
}

static_assertions::const_assert_eq!(size_of::<CdmaIoSqe>(), CDMA_IO_RX_ENTRY_SIZE);

/// CDMA IO SQE attributes
#[bitfield(u16)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct CdmaIoSqeAttr {
    /// Operation type; true == decrypt, false == encrypt
    pub op: bool,

    /// Reserved bit
    pub rsvd0: bool,

    /// PRP or SGL for data transfer; true == SGL, false = PRP
    pub psdt: bool,

    /// Command type; FAST_PATH == 0x5
    #[bits(3)]
    pub ctype: u8,

    /// Reserved bits
    #[bits(2)]
    pub rsvd1: u8,

    /// Cipher flag; true == xts, false == gcm
    pub cipher: bool,

    /// Reserved bits
    #[bits(5)]
    pub rsvd2: u8,

    /// Data Unit length; transfer size FULL == 0x0
    #[bits(2)]
    pub du: u8,
}

impl Default for CdmaIoSqeAttr {
    fn default() -> Self {
        Self::new()
    }
}

/// SQE common structure for CDMA IO
#[repr(C)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
pub struct CdmaIoSqeCommon {
    /// PASID
    pub pasid: u32,

    /// SQE attributes
    pub attr: CdmaIoSqeAttr,

    /// CID
    pub cid: u16,

    /// Session ID
    pub sess_id: u16,

    /// App ID
    pub app_id: u8,

    /// Reserved
    pub dw2to5_rsvd: u8,

    /// Reserved
    pub dw3to5_rsvd: [u32; 3],

    /// Source data length
    pub src_length: u32,

    /// First page source
    pub src_prp1: MemoryAddr,

    /// Second page or page list source
    pub src_prp2: MemoryAddr,

    /// Destination data length
    pub dst_length: u32,

    /// First page destination
    pub dst_prp1: MemoryAddr,

    /// Second page or page list destination
    pub dst_prp2: MemoryAddr,

    /// Frame ID
    pub frm_id: u8,

    /// Reserved
    pub dw16_rsvd: [u8; 3],
}

/// XTS command specific structure for SQE
#[repr(C)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
pub struct XtsCmd {
    /// Key index for Encryption key
    pub key1_idx: u32,

    /// Key index for Tweak key
    pub key2_idx: u32,

    /// Tweak value
    pub tweak: [u32; 4],

    /// Reserved
    pub dw23to31_rsvd: [u32; 9],
}

/// GCM command specific structure for SQE
#[repr(C)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
pub struct GcmCmd {
    /// Key index for Encryption key
    pub key_idx: u32,

    /// Unaligned AAD length
    pub unpadded_aad_length: u32,

    /// Tag value
    pub tag: [u8; 16],

    /// IV
    pub iv: [u8; 12],

    /// Additional Authenticated Data (AAD) length
    pub aad_length: u32,

    /// Unaligned source data length
    pub unaligned_src_data_length: u8,

    /// Unaligned destination data length
    pub unaligned_dst_data_length: u8,

    /// Reserved; pad for 32 bit alignment
    pub dw27_rsvd: u16,

    /// Unaligned source data pointer
    pub unaligned_src_data_ptr: MemoryAddr,

    /// Unaligned destination data pointer
    pub unaligned_dst_data_ptr: MemoryAddr,
}

/// Define FP AES XTS SQE
#[repr(C, align(4))]
#[derive(Default, FromBytes, IntoBytes, Immutable)]
pub struct CdmaIoXtsSqe {
    /// Common SQE info
    pub info: CdmaIoSqeCommon,
    /// XTS command
    pub cmd: XtsCmd,
}

static_assertions::const_assert_eq!(size_of::<CdmaIoXtsSqe>(), CDMA_IO_RX_ENTRY_SIZE);

impl From<CdmaIoXtsSqe> for CdmaIoSqe {
    /// Converts to CdmaIoSqe from CdmaIoXtsSqe type.
    fn from(sqe_cmd: CdmaIoXtsSqe) -> Self {
        let mut sqe_io_u32_arr = CdmaIoSqe::default();
        sqe_io_u32_arr
            .as_mut_bytes()
            .copy_from_slice(sqe_cmd.as_bytes());
        sqe_io_u32_arr
    }
}

/// Define FP AES GCM SQE
#[repr(C, align(4))]
#[derive(Default, FromBytes, IntoBytes, Immutable)]
pub struct CdmaIoGcmSqe {
    /// Common SQE info
    pub info: CdmaIoSqeCommon,

    /// GCM command
    pub cmd: GcmCmd,
}

static_assertions::const_assert_eq!(size_of::<CdmaIoGcmSqe>(), CDMA_IO_RX_ENTRY_SIZE);

impl From<CdmaIoGcmSqe> for CdmaIoSqe {
    /// Converts to CdmaIoSqe from CdmaIoGcmSqe type.
    fn from(sqe_cmd: CdmaIoGcmSqe) -> Self {
        let mut sqe_io_u32_arr = CdmaIoSqe::default();
        sqe_io_u32_arr
            .as_mut_bytes()
            .copy_from_slice(sqe_cmd.as_bytes());
        sqe_io_u32_arr
    }
}

impl From<CdmaIoSqe> for CdmaIoGcmSqe {
    /// Converts to CdmaIoGcmSqe from CdmaIoSqe type. Only use if the SQE is known to be GCM type.
    fn from(raw: CdmaIoSqe) -> Self {
        let mut out = CdmaIoGcmSqe::default();
        out.as_mut_bytes().copy_from_slice(raw.as_bytes());
        out
    }
}

/// Define CDMA IO CQE Status
#[bitfield(u16)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct CdmaIoCqeStatus {
    /// Phase
    pub phase: bool,

    /// Status code
    #[bits(8)]
    pub status_code: u8,

    /// Reserved
    #[bits(7)]
    rsvd: u8,
}

impl Default for CdmaIoCqeStatus {
    fn default() -> Self {
        Self::new()
    }
}

/// Define CDMA IO CQE
#[repr(C, align(4))]
#[derive(Default, FromBytes, IntoBytes, Immutable)]
pub struct CdmaIoCqe {
    /// SQE attributes
    pub attr: CdmaIoSqeAttr,

    /// CID
    pub cid: u16,

    /// Tag
    pub tag: [u8; 16],

    /// IV
    pub iv: [u8; 12],

    /// Reserved
    pub _rsvd: [u32; 3],

    /// Unaligned destination data length
    pub unaligned_dst_data_length: u32,

    /// FIPS Approval Status
    pub fips_approval_status: u32,

    /// Output data length
    pub output_data_length: u32,

    /// SQ head
    pub sq_head: u16,

    /// SQ ID
    pub sq_id: u16,

    /// Error code
    pub error_code: u16,

    /// Status
    pub status: CdmaIoCqeStatus,
}

static_assertions::const_assert_eq!(size_of::<CdmaIoCqe>(), CDMA_IO_TX_ENTRY_SIZE);
