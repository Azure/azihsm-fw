// Copyright (c) Microsoft Corporation. All rights reserved.

use bitfield_struct::bitfield;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

/// AES Command bitfield definition.
#[bitfield(u32)]
pub struct AesCommandCode {
    /// Key Length
    #[bits(4)]
    pub key_length: u32,

    #[bits(8)]
    pub _rsvd1: u32,

    /// Bit that controls write back of initialization vector
    pub update_iv: bool,

    #[bits(3)]
    pub _rsvd2: u32,

    /// Decrypt or encrypt operation
    pub decrypt_encrypt: bool,

    #[bits(7)]
    pub _rsvd3: u32,

    /// AES Mode
    #[bits(4)]
    pub mode: u32,

    /// Field is always set to 0x2
    #[bits(4)]
    pub two: u32,
}

impl Default for AesCommandCode {
    fn default() -> Self {
        AesCommandCode(0).with_two(0x2)
    }
}

/// The command structure to use for issuing AES commands.
#[repr(C)]
#[derive(Default)]
pub struct AesCommandDesc {
    /// AES command to execute.
    pub cmd_code: AesCommandCode,

    /// Address for the hash result.
    pub result: u32,

    /// Total byte count in multiples of 16 bits
    pub byte_count: u32,

    /// Message buffer address
    pub message: u32,

    /// Key address
    pub key: u32,

    /// Initialization vector
    pub iv: u32,
}

/// AES GCM IV data
#[derive(Copy, Clone, Default)]
pub struct AesGcmIV {
    /// IV for AES-GCM FIPS approved operation
    pub iv: [u8; 12],
}

/// AES GCM request entry
#[cfg(target_pointer_width = "32")]
#[bitfield(u64)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
pub struct AesGcmReqEntry {
    /// SQE index
    #[bits(10)]
    pub sqe_idx: u32,

    /// Reserved
    #[bits(6)]
    rsvd1: u32,

    /// Whether the request contains an invalid tag
    #[bits(1)]
    pub is_invalid_tag: bool,

    /// Reserved
    #[bits(7)]
    rsvd2: u32,

    /// PCIe Interface ID
    #[bits(8)]
    pub pfn: u8,

    /// FP GCM request SQE address
    pub sqe_addr: u32,
}

/// AES GCM request entry for unit test on 64bit platform
#[cfg(target_pointer_width = "64")]
#[bitfield(u128)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
pub struct AesGcmReqEntry {
    /// SQE index
    #[bits(10)]
    pub sqe_idx: u32,

    /// Reserved
    #[bits(6)]
    rsvd1: u32,

    /// Whether the request contains an invalid tag
    #[bits(1)]
    pub is_invalid_tag: bool,

    /// Reserved
    #[bits(7)]
    rsvd2: u32,

    /// PCIe Interface ID
    #[bits(8)]
    pub pfn: u8,

    /// FP GCM request SQE address
    pub sqe_addr: u64,

    pub _rsvd3: u32,
}

/// AES GCM request entry
#[bitfield(u32)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
pub struct AesGcmRespEntry {
    /// SQE index
    #[bits(10)]
    pub sqe_idx: u32,

    /// Reserved
    #[bits(6)]
    rsvd1: u8,

    /// MCM processing status
    #[bits(8)]
    pub status: u8,

    /// Reserved
    #[bits(8)]
    rsvd2: u8,
}

/// Errors given on AES-GCM response status field
#[derive(PartialEq, Clone, Copy)]
pub enum AesGcmExtRespErr {
    /// Successful completion
    Success,

    /// Invalid request pointer
    InvalidAesGcmRequestPtr,

    /// Invalid request pointer
    InvalidSqeAddrPtr,

    /// Invalid unaligned src data pointer
    InvalidUnalignedSrcDataPtr,

    /// Invalid unaligned dst data pointer
    InvalidUnalignedDstDataPtr,

    /// Invalid PFN value
    InvalidPcieFn,

    /// Dma In Operation failed
    DmaMemAllocationFailed,

    /// Dma In Operation failed
    DmaInOperationErr,

    /// Dma Out Operation failed
    DmaOutOperationErr,

    /// Key Blob read from CDMA Key Vault failed
    AesGcmKeyBlobReadFailed,

    /// GCM Tag correction failed
    AesGcmTagCorrectionFailed,

    /// Invalid GCM Tag in decrypt operation
    AesGcmInvalidDecryptTag,

    /// Invalid SQE Index
    InvalidSqeIndex,

    /// Invalid Unaligned Data Length
    InvalidUnalignedDataLength,
}

/// Request entry to get bulk key (Admin to HSM)
#[repr(C)]
#[derive(Copy, Clone, Default, IntoBytes, Immutable, FromBytes)]
pub struct GetBulkKeyReqEntry {
    /// Key index within the CDMA vault
    pub key_index: u8,

    /// Resource (vault) identifier
    pub resource_id: u8,

    /// PCIe function number
    pub pfn: u8,

    /// Reserved
    pub _rsvd: u8,
}

/// Response entry to get bulk key (HSM to Admin)
#[repr(C)]
#[derive(Copy, Clone, Default, IntoBytes, Immutable, FromBytes)]
pub struct GetBulkKeyRespEntry {
    /// Status code (0 = success)
    pub status: u8,

    /// Reserved
    pub _rsvd: [u8; 3],

    /// 256-bit AES bulk key
    pub key: [u32; 8],
}
