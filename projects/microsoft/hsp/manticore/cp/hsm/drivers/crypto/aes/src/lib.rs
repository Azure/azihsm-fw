// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod aes;

pub use aes::Aes;
use mcr_error::mcr_err_decl;
use mcr_error::McrResult;
use mcr_types::IoMemRange;

/// Buffer size = AES-256 Key buffer (32) +
/// input message buffer (64) = 96 bytes
pub const AES_SELF_TEST_INPUT_BUF_MAX_SIZE_BYTES: usize = 96;

/// Output message buffer = 64 bytes
pub const AES_SELF_TEST_OUTPUT_BUF_MAX_SIZE_BYTES: usize = 64;

/// IV buffer max size = 16 bytes
pub const AES_SELF_TEST_IV_BUF_MAX_SIZE_BYTES: usize = 16;

/// AES command mode
#[derive(Default, Copy, Clone, PartialEq, Eq)]
pub enum AesMode {
    /// ECB
    Ecb = 1,

    /// CBC
    #[default]
    Cbc = 2,
}

impl From<AesMode> for u32 {
    /// Converts to this type from the input type.
    fn from(value: AesMode) -> Self {
        value as Self
    }
}

/// AES Key Length
#[derive(Copy, Clone)]
pub enum AesKeyLength {
    /// AES 128-bit Key
    Aes128 = 1,

    /// AES 192-bit Key
    Aes192 = 2,

    /// AES 256-bit Key
    Aes256 = 3,
}

impl TryFrom<usize> for AesKeyLength {
    /// The type returned in the event of a conversion error.
    type Error = u32;

    /// Performs the conversion.
    fn try_from(value: usize) -> Result<Self, Self::Error> {
        match value {
            16 => Ok(AesKeyLength::Aes128),
            24 => Ok(AesKeyLength::Aes192),
            32 => Ok(AesKeyLength::Aes256),
            _ => Err(AesErr::InvalidKeyLength)?,
        }
    }
}

impl From<AesKeyLength> for u32 {
    /// Converts to this type from the input type.
    fn from(value: AesKeyLength) -> Self {
        value as Self
    }
}

/// AES Operation
#[derive(Default, Copy, Clone, PartialEq, Eq)]
pub enum AesOp {
    /// AES Decrypt
    #[default]
    Decrypt = 0,

    /// AES Encrypt
    Encrypt = 1,
}

impl From<AesOp> for bool {
    /// Converts to this type from the input type.
    fn from(value: AesOp) -> Self {
        match value {
            AesOp::Decrypt => false,
            AesOp::Encrypt => true,
        }
    }
}

/// AES Command Info structure
pub struct AesCommand<'a> {
    /// Command Tag
    pub tag: u16,

    /// Message buffer
    /// NOTE: The message buffer must be 16 byte aligned.
    pub message: &'a IoMemRange,

    /// Initialization vector
    pub iv: Option<&'a IoMemRange>,

    /// Key
    pub key: &'a [u8],

    /// AES Mode
    pub mode: AesMode,

    /// AES Operation
    pub op: AesOp,

    /// Flag indicating whether to update IV or not
    pub update_iv: bool,

    /// Result buffer
    pub result: &'a IoMemRange,
}

/// AES completion status
#[derive(PartialEq, Eq, Copy, Clone)]
pub enum AesCompletionStatus {
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

    /// An unknown error.
    Unknown = 0x80,
}

impl From<u8> for AesCompletionStatus {
    /// Converts to this type from the input type.
    fn from(value: u8) -> Self {
        match value {
            x if x == AesCompletionStatus::Busy as u8 => AesCompletionStatus::Busy,
            x if x == AesCompletionStatus::Complete as u8 => AesCompletionStatus::Complete,
            x if x == AesCompletionStatus::CmdError as u8 => AesCompletionStatus::CmdError,
            x if x == AesCompletionStatus::BusError as u8 => AesCompletionStatus::BusError,
            x if x == AesCompletionStatus::FaultError as u8 => AesCompletionStatus::FaultError,
            x if x == AesCompletionStatus::NotOwner as u8 => AesCompletionStatus::NotOwner,
            _ => AesCompletionStatus::Unknown,
        }
    }
}

pub trait AesTrait {
    /// Perform encrypt or decrypt operation
    ///
    /// # Arguments
    ///
    /// * `cmd_info` - The AES command related information.
    fn encrypt_decrypt(&self, cmd_info: &AesCommand) -> McrResult<()>;

    /// Perform encrypt or decrypt operation
    ///
    /// # Arguments
    /// * `self` - AES instance
    /// * `self_test_input` - Buffer containing self test vector input
    /// * `self_test_output` - Self test output buffer
    /// * `self_test_iv` - Buffer containing self test IV
    ///
    /// # Returns
    ///
    /// * `McrResult` - Ok(()) or appropriate Err() value
    fn aes_cbc_self_test(
        &self,
        self_test_input: &mut [u8],
        self_test_output: &mut [u8],
        self_test_iv: &mut [u8],
    ) -> McrResult<()>;
}

/// AES Completion Descriptor
#[derive(Copy, Clone)]
pub struct AesCompletionDesc {
    /// Result of the SHA transaction.
    pub status: AesCompletionStatus,

    /// Tag to track the transaction completion
    pub tag: u16,
}

// AES Engine status codes
mcr_err_decl! {
    Aes,
    AesErr {
        // AES engine is busy processing a command.
        EngineBusy = 1,

        // Invalid AES Key Length
        InvalidKeyLength = 2,

        // Result buffer too small
        ResultBufferTooSmall = 3,

        // Invalid IV length
        InvalidIVLength = 4,

        // Invalid Message length
        InvalidMessageLength = 5,

        // Initialization Vector missing
        IVMissing = 6,

        // Unsupported Mode
        UnsupportedMode = 7,

        // Self test failed
        SelfTestFailed = 8,

        // Self test timed out
        SelfTestTimeout = 9,

        // Tag mismatch
        TagMismatch = 10,

        // Command Timeout
        CmdTimeout = 11,

        // Command Failure
        CmdFail = 12,
    }
}
