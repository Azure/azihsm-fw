// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod sha;

use mcr_error::mcr_err_decl;
use mcr_error::McrResult;
use mcr_types::*;
pub use sha::Mgf1Output;
pub use sha::Sha;

// SHA block size for SHA384 and SHA512
pub const SHA_MAX_BLOCK_SIZE: usize = 128;

// HMAC max message size
const HMAC_MAX_MSG_SIZE: usize = 128;
// HMAC max input buffer size
pub const HMAC_MAX_INPUT_BUF_SIZE: usize = SHA_MAX_BLOCK_SIZE + HMAC_MAX_MSG_SIZE;

// Maximum length for HKDF info
pub const HKDF_MAX_INFO_SIZE: usize = 256;
// Maximum length for HKDF salt
pub const HKDF_MAX_SALT_SIZE: usize = 256;
// Maximum length for HKDF input data:
pub const HKDF_MAX_INPUT_BUF_SIZE: usize = SHA_MAX_BLOCK_SIZE + HKDF_MAX_INFO_SIZE + 1;

// KBKDF RLEN_BYTES is the size of the counter as u32
// and size of out_len as u32
const KBKDF_RLEN_BYTES: usize = 4;
// Length of the output must be less than 255 * hash length. Reference: https://www.rfc-editor.org/rfc/rfc5869
pub const KDF_MAX_LENGTH_MULTIPLIER: usize = 255;
// Maximum length for KBKDF label
pub const KBKDF_MAX_LABEL_SIZE: usize = 256;
// Maximum length for KBKDF context
pub const KBKDF_MAX_CONTEXT_SIZE: usize = 256;
// Maximum length for KBKDF input data:
// ([i]2 || Label (up to 256 Bytes) || 0x00 || Context (up to 256 Bytes) || [L]2)
pub const KBKDF_MAX_INPUT_DATA_SIZE: usize =
    KBKDF_RLEN_BYTES + KBKDF_MAX_LABEL_SIZE + 1 + KBKDF_MAX_CONTEXT_SIZE + KBKDF_RLEN_BYTES;
// Maximum input buffer length for KBKDF
pub const KBKDF_MAX_INPUT_BUF_SIZE: usize = SHA_MAX_BLOCK_SIZE + KBKDF_MAX_INPUT_DATA_SIZE;

/// Key size for unwrapping operations
pub const UNWRAPPING_KEY_SIZE: usize = 256;

/// Size of MGF1 counter in bytes
pub const MGF1_COUNTER_SIZE: usize = 4;

/// Maximum output mask size for OAEP MGF1
/// Calculated as RSA key size (512 bytes) - minimum hash size (SHA1: 20 bytes) - 1
pub const OAEP_MGF1_MAX_OUTPUT_MASK_SIZE: usize = 512 - 20 - 1;

/// Maximum hash input size for MGF1
/// Includes the maximum output mask size plus counter size
pub const OAEP_MGF1_MAX_HASH_INPUT_SIZE: usize = OAEP_MGF1_MAX_OUTPUT_MASK_SIZE + MGF1_COUNTER_SIZE;

/// Maximum T buffer size for MGF1 operations
/// Conservative estimate to handle various mask lengths
pub const OAEP_MGF1_MAX_T_BUFFER_SIZE: usize = 500;

/// RSA padding types for OAEP operations
#[derive(Clone, Copy, PartialEq)]
pub enum OaepPadding {
    /// OAEP padding
    Oaep,

    /// PKCS#1 v1.5 padding (not supported by this decoder)
    Pkcs1v15,
}

/// Hash algorithms supported for OAEP
#[derive(Clone, Copy, PartialEq)]
pub enum HashAlgorithm {
    /// SHA-1
    Sha1,

    /// SHA-256
    Sha256,

    /// SHA-384
    Sha384,

    /// SHA-512
    Sha512,
}

impl From<HashAlgorithm> for ShaMode {
    fn from(hash_alg: HashAlgorithm) -> Self {
        match hash_alg {
            HashAlgorithm::Sha1 => ShaMode::Sha1,
            HashAlgorithm::Sha256 => ShaMode::Sha256,
            HashAlgorithm::Sha384 => ShaMode::Sha384,
            HashAlgorithm::Sha512 => ShaMode::Sha512,
        }
    }
}

impl HashAlgorithm {
    /// Get the digest size in bytes for this hash algorithm
    pub fn digest_size(&self) -> usize {
        match self {
            HashAlgorithm::Sha1 => 20,
            HashAlgorithm::Sha256 => 32,
            HashAlgorithm::Sha384 => 48,
            HashAlgorithm::Sha512 => 64,
        }
    }
}

/// SHA command mode
#[derive(Copy, Clone, PartialEq)]
pub enum ShaMode {
    /// SHA_1
    Sha1 = 1,

    /// SHA_256
    Sha256 = 3,

    /// SHA_384
    Sha384 = 4,

    /// SHA_512
    Sha512 = 5,
}

impl From<ShaMode> for u32 {
    /// Converts to this type from the input type.
    fn from(value: ShaMode) -> Self {
        value as Self
    }
}

impl ShaMode {
    /// Returns the SHA block size for the specific ShaMode
    pub fn get_block_size(&self) -> usize {
        match self {
            ShaMode::Sha1 => 64,
            ShaMode::Sha256 => 64,
            ShaMode::Sha384 => 128,
            ShaMode::Sha512 => 128,
        }
    }

    /// Returns the standard digest size for the specific ShaMode
    pub fn get_digest_size(&self) -> usize {
        match self {
            ShaMode::Sha1 => 20,
            ShaMode::Sha256 => 32,
            ShaMode::Sha384 => 48,
            ShaMode::Sha512 => 64,
        }
    }

    /// This returns the digest size as required by the SHA HW.
    /// HS SHA requires the initial digest for SHA-384 and SHA-512 to be 64 bytes.
    pub fn get_digest_size_hw(&self) -> usize {
        match self {
            ShaMode::Sha1 => 20,
            ShaMode::Sha256 => 32,
            // HS SHA requires the initial digest for SHA-384 to be 64 bytes.
            ShaMode::Sha384 => 64,
            ShaMode::Sha512 => 64,
        }
    }
}

/// Info/context required for HKDF
#[derive(Default)]
pub struct HkdfInfo<'a> {
    /// Key for HKDF
    pub key: &'a [u8],

    /// Salt for HKDF
    pub salt: &'a [u8],

    /// Info for HKDF
    pub info: &'a [u8],

    /// Expected output key length
    pub out_len: u16,
}

/// Input data/msg for KBKDF
pub enum KbkdfInputData<'a> {
    /// Concatenated data provided by app session
    ConcatData {
        /// Label for KBKDF Input Data
        label: &'a [u8],

        /// Context for KBKDF Input Data
        context: &'a [u8],
    },

    /// Fixed input data provided by KAT vector
    SelfTestData { fixed_input: &'a [u8] },
}

/// Info/context required for KBKDF
pub struct KbkdfInfo<'a> {
    /// Key for KBKDF
    pub key: &'a [u8],

    /// KBKDF input data/msg
    pub input_data: KbkdfInputData<'a>,

    /// Expected output key length
    pub out_len: u16,
}

impl Default for KbkdfInfo<'_> {
    fn default() -> Self {
        static DEFAULT_SLICE: [u8; 0] = [];
        KbkdfInfo {
            key: &DEFAULT_SLICE,
            input_data: KbkdfInputData::ConcatData {
                label: &DEFAULT_SLICE,
                context: &DEFAULT_SLICE,
            },
            out_len: 0,
        }
    }
}

/// Sha completion status
#[derive(PartialEq, Eq, Copy, Clone)]
pub enum ShaCompletionStatus {
    /// Indicates busy executing last command.
    Busy = 0x1,

    /// Successful digest computation.
    Complete = 0x2,

    /// An error was detected in the command struct
    CmdError = 0x4,

    /// A slave bus error was detected while mastering
    BusError = 0x8,

    /// A logic fault was detected during execution
    FaultError = 0x10,

    /// The status register is read by an engine that did not issue the last command
    NotOwner = 0x20,

    /// The reference digest matched the computed digest
    DigestMatch = 0x40,

    /// An unknown error.
    Unknown = 0x80,
}

impl From<u8> for ShaCompletionStatus {
    fn from(value: u8) -> Self {
        match value {
            x if x == ShaCompletionStatus::Busy as u8 => ShaCompletionStatus::Busy,
            x if x == ShaCompletionStatus::Complete as u8 => ShaCompletionStatus::Complete,
            x if x == ShaCompletionStatus::CmdError as u8 => ShaCompletionStatus::CmdError,
            x if x == ShaCompletionStatus::BusError as u8 => ShaCompletionStatus::BusError,
            x if x == ShaCompletionStatus::FaultError as u8 => ShaCompletionStatus::FaultError,
            x if x == ShaCompletionStatus::NotOwner as u8 => ShaCompletionStatus::NotOwner,
            x if x == ShaCompletionStatus::DigestMatch as u8 => ShaCompletionStatus::DigestMatch,
            _ => ShaCompletionStatus::Unknown,
        }
    }
}

/// The completion details for a SHA command.
pub struct ShaCompletionDesc {
    /// Result of the SHA transaction.
    pub status: ShaCompletionStatus,

    /// Tag to track the transaction completion
    pub tag: u16,
}

pub struct ShaDigest {
    pub mode: ShaMode,
    pub data: [u8; SHA_DIGEST_MAX_SIZE_BYTES],
}

/// The command structure to use for issuing HS-SHA commands.
pub struct ShaDigestCmdInfo<'a> {
    /// Buffer containing the data to hash.
    pub buffer: &'a [u8],

    /// Input context to continue calculation of an existing digest.
    pub init_digest: Option<&'a [u8]>,

    /// The hash algorithm to use to generate the digest.
    pub mode: ShaMode,

    /// Flag to indicate if the final digest should be calculated. If
    /// this is false, the input data must be aligned to the hash algorithm
    /// block size and must contain some data (i.e. 0 length is not supported).
    pub last: bool,

    /// Length of bytes to be hashed in the command.
    pub len: u32,

    /// Total length of message hashed.
    pub total_len: u32,
}

pub struct ShaDigestCmdInfoZc<'a> {
    /// Buffer containing the data to hash.
    pub buffer: &'a IoMemRange,

    /// Input context to continue calculation of an existing digest.
    pub init_digest: Option<&'a IoMemRange>,

    /// The hash algorithm to use to generate the digest.
    pub mode: ShaMode,

    /// Flag to indicate if the final digest should be calculated. If
    /// this is false, the input data must be aligned to the hash algorithm
    /// block size and must contain some data (i.e. 0 length is not supported).
    pub last: bool,

    /// Length of bytes to be hashed in the command.
    pub len: u32,

    /// Total length of message hashed.
    pub total_len: u32,

    /// Output buffer.
    /// Buffer size should match values in get_digest_size_hw.
    pub output_buffer: &'a mut IoMemRange,
}

pub trait ShaTrait {
    ///
    /// Compute an intermediate digest for a partial message buffer. This can either be
    /// an extended hash calculation from a previous message buffer or the start of a new message.
    /// Data is output to an output_buffer in command_info.
    ///
    /// # Arguments
    ///
    /// * `self` - The Sha object contains the required parameters for programming the HS SHA
    ///   HW interface.
    /// * `command_info` - The SHA digest command related information.
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn digest_zc(&self, command_info: &ShaDigestCmdInfoZc) -> McrResult<()>;

    /// Compute HMAC
    /// HMAC is calculated as per the standard at: https://www.rfc-editor.org/rfc/rfc2104.
    ///
    /// # Arguments
    ///
    /// * `self` - The Sha object contains the required parameters for programming the HS SHA
    ///   HW interface.
    /// * `key` - input key.
    /// * `data` - input data key.
    /// * `sha_mode` - sha hash algorithm.
    /// * `in_buf` - allocated buffer used for input/working data.
    /// * `out_buf` - allocated buffer used for HMAC output.
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if HMAC is successful, error otherwise.
    fn hmac(
        &self,
        key: &[u8],
        data: &[u8],
        sha_mode: ShaMode,
        in_buf: &mut IoMemRange,
        out_buf: &mut IoMemRange,
    ) -> McrResult<()>;

    /// Compute HKDF
    /// HKDF is implemented per: https://www.rfc-editor.org/rfc/rfc5869
    ///
    /// # Arguments
    ///
    /// * `self` - The Sha object contains the required parameters for programming the HS SHA
    ///   HW interface.
    /// * `hkdf_info` - struct containing info/context needed for hkdf operation.
    /// * `sha_mode` - sha hash algorithm.
    /// * `prk_buf` - allocated IoMemRange buffer to store the HMAC generated Pseudorandom Key.
    /// * `in_buf` - allocated IoMemRange buffer used for input/working data.
    /// * `output` - allocated buffer used for output.
    ///
    /// # Returns
    ///
    /// * `Ok()` - Ok if KBKDF is successful, error otherwise.
    fn hkdf(
        &self,
        hkdf_info: HkdfInfo,
        sha_mode: ShaMode,
        prk_buf: &mut IoMemRange,
        in_buf: &mut IoMemRange,
        output: &mut [u8],
    ) -> McrResult<()>;

    /// Compute KBKDF
    /// KBKDF is implemented per the standard at: https://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-108r1-upd1.pdf
    ///
    /// # Arguments
    ///
    /// * `self` - The Sha object contains the required parameters for programming the HS SHA
    ///   HW interface.
    /// * `key` - input key.
    /// * `fixed_input_data` - input data without the prepended counter value;
    ///   (Label || 0x00 || Context || [L]2).
    /// * `sha_mode` - sha hash algorithm.
    /// * `out_len` - expected output key length.
    /// * `in_buf` - allocated IoMemRange buffer used for input/working data.
    /// * `output` - allocated buffer used for output.
    ///
    /// # Returns
    ///
    /// * `Ok()` - Ok if KBKDF is successful, error otherwise.
    fn kbkdf_counter_hmac(
        &self,
        kbkdf_info: KbkdfInfo,
        sha_mode: ShaMode,
        in_buf: &mut IoMemRange,
        output: &mut [u8],
    ) -> McrResult<()>;

    /// Perform a HKDF-SHA-256 self test with standard known test vectors.
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the self test is successful, error otherwise.
    fn hkdf_self_test_256(&self) -> McrResult<()>;

    /// Perform a KBKDF HMAC-SHA-512 self test with standard known test vectors.
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the self test is successful, error otherwise.
    fn kbkdf_self_test_512(&self) -> McrResult<()>;

    /// Decode OAEP KEK from unwrapped RSA data
    ///
    /// This function handles the specific case of unwrapping a KEK using OAEP padding.
    /// It's designed for HSM key unwrapping operations.
    ///
    /// # Arguments
    ///
    /// * `unwrapped_data` - The data after RSA decryption but before OAEP decoding
    /// * `hash_alg` - Hash algorithm used in OAEP
    ///
    /// # Returns
    ///
    /// * `Ok(SecureByteVec)` - The decoded KEK
    /// * `Err(ShaErr)` - If decoding fails
    fn decode_oaep_kek(
        &self,
        unwrapped_data: &[u8],
        hash_alg: HashAlgorithm,
    ) -> McrResult<SecureByteVec>;

    /// Perform self-test for OAEP KEK decoding
    /// This function is used to validate the OAEP KEK decoding implementation
    /// by running a self-test with predefined data.
    ///
    /// # Arguments
    /// * `unwrapped_data` - The data after RSA decryption but before OAEP decoding
    /// * `hash_alg` - Hash algorithm used in OAEP
    ///
    /// # Returns
    ///
    /// * `Ok(())` - If the self-test passes
    /// * `Err(ShaErr)` - If the self-test fails
    fn decode_oaep_kek_self_test(
        &self,
        unwrapped_data: &[u8],
        hash_alg: HashAlgorithm,
    ) -> McrResult<()>;
}

// HS SHA engine status codes
mcr_err_decl! {
    Sha,
    ShaErr {
        // SHA engine is busy processing a command.
        EngineBusy = 1,

        // The message length is not block aligned.
        BlockAlignmentMismatch = 2,

        // SHA command timeout
        CmdTimeout = 3,

        // SHA command failed
        CmdFail = 4,

        // Invalid argument
        InvalidArgument = 5,

        // SHA-512 self test failed
        Sha512SelfTestFailed = 6,

        // HMAC-256 self test failed
        Hmac256SelfTestFailed = 7,

        // HMAC-384 self test failed
        Hmac384SelfTestFailed = 8,

        // HMAC-512 self test failed
        Hmac512SelfTestFailed = 9,

        // HMAC Invalid data length
        HmacInvalidData = 0xa,

        // HKDF Sanity check failed
        HkdfSanityCheckFailed = 0xb,

        // HKDF Derivation failed
        HkdfKeyDeriveFailed = 0xc,

        // HKDF Self test failed
        HkdfSelfTestFailed = 0xd,

        // KBKDF Sanity check failed
        KbkdfSanityCheckFailed = 0xe,

        // KBKDF Derivation failed
        KbkdfKeyDeriveFailed = 0xf,

        // KBKDF Self test failed
        KbkdfSelfTestFailed = 0x10,

        // OAEP Invalid padding
        OaepInvalidPadding = 0x11,

        // OAEP Input too small
        OaepInputTooSmall = 0x12,

        // OAEP Decode failed
        OaepDecodeFailed = 0x13,

        // OAEP SHA operation failed
        OaepShaFailed = 0x14,

        // OAEP DMA allocation failed
        OaepDmaAllocFailed = 0x15,

        // OAEP Invalid separator
        OaepInvalidSeparator = 0x16,

        // OAEP Self test failed
        OaepSelfTestFailed = 0x17,
    }
}
