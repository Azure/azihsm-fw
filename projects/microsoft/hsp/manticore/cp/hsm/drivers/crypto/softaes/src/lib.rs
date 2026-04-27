// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

use core::ops::Range;

#[cfg(target_arch = "arm")]
mod aes;
mod decrypt;
mod encrypt;
mod engine;
mod keyschedule_128;
mod keyschedule_192;
mod keyschedule_256;
mod keyschedule_dec;
mod keywrap;
#[cfg(target_arch = "arm")]
mod lookup_tables;
mod tag_fix;

extern crate alloc;

pub use engine::*;
#[allow(unused)]
pub use keywrap::*;
pub use tag_fix::*;

use mcr_error::*;

pub const AES_SELF_TEST_BLOCK_SIZE: usize = 64;

pub trait SoftAesTrait {
    /// Wrapper function to expose the key_unwrap_inplace defined in keywrap.rs through the trait
    ///
    /// # Arguments
    ///
    /// * `kek` - the AES key that will be used to unwrap the key
    /// * `input` - a u8 slice of the wrapped key that is at least 16 bytes long,
    ///   which be used to contain the output
    ///
    /// # Returns
    ///
    /// * `McrResult<Range<usize>>` - A result indicating the range of the input that contains
    ///   the unwrapped key if completed successfully or an appropriate error code.
    fn key_unwrap_inplace(&self, kek: &[u8], input: &mut [u8]) -> McrResult<Range<usize>>;

    /// Wrapper function to expose the ECB decrypt through the trait
    ///
    /// # Arguments
    ///
    /// * `key` - the AES key that will be used for decryption
    /// * `inout` - a u8 slice of encrypted data, 16 byte aligned, for inplace decryption
    ///
    /// # Returns
    ///
    /// * `McrResult<Range<usize>>` - A result indicating the range of the input that contains
    ///   the unwrapped key if completed successfully or an appropriate error code.
    fn ecb_decrypt(&self, key: &[u8], inout: &mut [u8]) -> McrResult<Range<usize>>;

    /// Wrapper function to expose the GCM tag correction algorithm through the trait
    ///
    /// # Arguments
    ///
    /// * `encrypt` - Bool indicating encrypt (true) or decrypt (false) operation.
    /// * `key` - AES key used for tag correction and crypt operation for last input block.
    /// * `iv` - 12‑byte initialization vector (nonce) for GCM.
    /// * `aad_len` - Length of the Additional Authenticated Data (AAD).
    /// * `text_len` - Length of the plaintext or ciphertext.
    /// * `aad` - Optional AAD slice; must be None if `bad_tag` is provided.
    /// * `bad_tag` - Optional "bad" tag to correct. This is the tag produced for the
    ///   aligned portion of the data by the CDMA engine and must be
    ///   corrected to account for the final unaligned input block.
    /// * `unaligned_input_block` - The final partial (<=15 byte) input block.
    /// * `output` - Buffer receiving output bytes corresponding to the unaligned block.
    ///
    /// # Returns
    ///
    /// * `McrResult<[u8; 16]>` - A result containing the corrected tag if completed successfully
    ///   or an appropriate error code.
    #[allow(clippy::too_many_arguments)]
    fn aes_gcm_tag_correction(
        &self,
        encrypt: bool,
        key: &[u8],
        iv: &[u8; 12],
        aad_len: u64,
        text_len: u64,
        aad: Option<&[u8]>,
        bad_tag: Option<&[u8; 16]>,
        unaligned_input_block: &[u8],
        aligned_input_len: usize,
        output: &mut [u8],
    ) -> McrResult<[u8; 16]>;

    /// Function to perform AES ECB 256 decrypt with Soft AES using NIST validation vectors
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok(()) if completed successfully or an appropriate error code.
    fn aes_ecb_256_decrypt_self_test(&self) -> McrResult<()>;

    /// Function to perform AES Key Unwrap with Soft AES using NIST validation vectors
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok(()) if completed successfully or an appropriate error code.
    fn aes_256_key_unwrap_self_test(&self) -> McrResult<()>;
}

mcr_err_decl! {
    SoftAes,
    SoftAesErr
    {
        // Invalid Key Encryption Key length
        InvalidKekLength = 1,

        // Insufficient output buffer length
        InsufficientOutputBufferLength = 2,

        // Unaligned output buffer length
        UnalignedOutputBufferLength = 3,

        // Insufficient input length
        InsufficientInputLength = 4,

        // Unaligned input buffer length
        UnalignedInputBufferLength = 5,

        // Imcompatibile input and output buffer length
        IncompatibleInputAndOutputLength = 6,

        // Invalid AIV padding
        InvalidAivPadding = 7,

        // Invalid Message Length Indicator
        InvalidMli = 8,

        // Padding oracle attack detected
        PaddingOracleAttachDetected = 9,

        // Unpadded AIV found
        UnpaddedAiv = 0xA,

        // Engine Initialization Failed
        EngineInitFailed = 0xB,

        // Encrypt Failed
        EncryptFailed = 0xC,

        // Finalyzing Encrypt Failed
        FinalyzingEncryptFailed = 0xD,

        // Decrypt Failed
        DecryptFailed = 0xE,

        // Finalyzing Decrypt Failed
        FinalyzingDecryptFailed = 0xF,

        // Soft Aes Self Test Decrypt Failure
        SoftAesSelfTestDecryptFailure = 0x10,

        // Soft Aes Self Test AES Key Unwrap Failure
        SoftAesSelfTestKeyUnwrapFailure = 0x11,

        // Last input block to tag correction is too large
        LastInputBlockTooLarge = 0x12,

        // No AAD input allowed if bad tag is provided.
        AADInvalid = 0x13,

        // Intermediate tag is required for tag correction missing.
        IntermediateTagMissing = 0x14,

        // Input lengths are too large for internal counters or computations.
        LengthOverflow = 0x15,
    }
}
