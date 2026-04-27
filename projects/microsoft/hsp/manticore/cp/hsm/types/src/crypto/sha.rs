// Copyright (c) Microsoft Corporation. All rights reserved.

use bitfield_struct::bitfield;

/// Maximum size of the SHA digest.
pub const SHA_DIGEST_MAX_SIZE_BYTES: usize = 64;
pub const SHA_SELF_TEST_BUF_MAX_SIZE_BYTES: usize = 384;

/// SHA command bitfield definition.
#[bitfield(u32)]
#[derive(Default)]
pub struct ShaCommand {
    /// When 1’b1 the digest initial value is loaded from memory
    pub load_digest: bool,

    /// Not used, must be zero
    #[bits(3)]
    pub reserved4: u32,

    /// A 1’b1 means working variable size digest is written out. This only affects modes where
    /// the digest would normally be truncated from working variable size
    /// (SHA-224, SHA-384, SHA-512/224, SHA-512/256).
    pub no_truncate: bool,

    /// Not used, must be zero.
    #[bits(3)]
    pub reserved3: u32,

    /// Padding is automatic when 1’b1.
    pub auto_pad: bool,

    /// Addressing mode used to read message data.
    #[bits(2)]
    pub read_message_mode: u32,

    /// Not used, must be zero.
    #[bits(5)]
    pub reserved2: u32,

    /// SHA mode
    #[bits(4)]
    pub sha_mode: u32,

    /// A 1’b1 means the digest bytes are swapped within the digest field (i.e. little endian format)
    pub byte_swap: bool,

    /// An expected digest value in memory is compared.
    pub check_digest: bool,

    /// Write passthrough message output addressing mode
    #[bits(2)]
    pub pass_message_mode: u32,

    /// Not used, must be zero.
    #[bits(4)]
    pub reserved1: u32,

    /// Constant for all SHA commands, must be 0x3
    #[bits(4)]
    pub sha_cmd_id: u32,
}

/// The command structure to use for issuing HS-SHA commands.
#[derive(Default)]
pub struct ShaCommandDesc {
    /// HS-SHA command to execute.
    pub command_code: ShaCommand,

    /// Address for the hash result.
    pub digest: u32,

    /// Total byte count for the entire message.
    pub byte_count: u32,

    /// Byte count of the current message chunk.
    pub message_bytes: u32,

    /// Address of the message to hash.
    pub message_buffer: u32,

    /// Address to a hashing context to resume.
    pub initial_digest: u32,

    /// Address for an output buffer for the input message.
    pub pass_message_buffer: u32,

    /// Address to a reference digest used for comparison.
    pub ref_digest: u32,
}
