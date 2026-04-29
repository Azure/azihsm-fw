// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

pub mod aes_fp_self_test_constants;
pub mod cdma_io;

pub use cdma_io::CdmaIo;
use mcr_error::mcr_err_decl;
use mcr_error::McrResult;
use mcr_types::*;

/// Key size in bytes
pub const KEY_SIZE: usize = 32;

/// Key size in dwords
pub const KEY_SIZE_IN_DWORDS: usize = KEY_SIZE / size_of::<u32>();

/// Each table can accommodate at the max 7 key slots to store AES Bulk 256 keys
pub const MAX_KEYS_PER_TABLE: usize = 7;

/// Type define GCM Tag
pub type GcmTag = [u8; 16];

/// AES Operation
#[derive(Default, Copy, Clone, PartialEq, Eq)]
pub enum AesFpOp {
    /// AES Encrypt
    #[default]
    Encrypt = 0,

    /// AES Decrypt
    Decrypt = 1,
}

/// Convert from AesFpOp to bool
impl From<AesFpOp> for bool {
    fn from(op: AesFpOp) -> bool {
        match op {
            AesFpOp::Encrypt => false,
            AesFpOp::Decrypt => true,
        }
    }
}

/// Convert from AesFpMode to AesFpOp
impl From<AesFpMode> for AesFpOp {
    fn from(mode: AesFpMode) -> Self {
        match mode {
            AesFpMode::XtsDecrypt | AesFpMode::GcmDecrypt => AesFpOp::Decrypt,
            AesFpMode::XtsEncrypt | AesFpMode::GcmEncrypt => AesFpOp::Encrypt,
        }
    }
}

#[derive(Default, Copy, Clone, PartialEq, Eq)]
pub enum AesFpCipher {
    /// AES Gcm
    #[default]
    Gcm = 0,

    /// AES Xts
    Xts = 1,
}

/// Convert from AesFpCipher to bool
impl From<AesFpCipher> for bool {
    fn from(op: AesFpCipher) -> bool {
        match op {
            AesFpCipher::Gcm => false,
            AesFpCipher::Xts => true,
        }
    }
}

/// Convert from AesFpMode to AesFpCipher
impl From<AesFpMode> for AesFpCipher {
    fn from(mode: AesFpMode) -> Self {
        match mode {
            AesFpMode::XtsDecrypt | AesFpMode::XtsEncrypt => AesFpCipher::Xts,
            AesFpMode::GcmDecrypt | AesFpMode::GcmEncrypt => AesFpCipher::Gcm,
        }
    }
}

/// CDMA IO configuration
pub struct CdmaIoConfig {
    /// AES FP cipher mode
    pub mode: AesFpCipher,

    /// AES FP operation
    pub op: AesFpOp,

    /// AES Bulk 256 key ID
    pub key1_id: AesBulk256KeyId,

    /// AES Bulk 256 key ID; used for XTS mode
    pub key2_id: Option<AesBulk256KeyId>,

    /// Initialization Vector; used for GCM mode
    pub iv: Option<[u32; 3]>,

    /// Initialization Vector bytes; used for GCM mode
    pub iv_bytes: Option<[u8; 12]>,

    /// Tag; used for GCM mode
    pub tag: Option<GcmTag>,

    /// Unpadded AAD length
    pub unpadded_aad_len: Option<u32>,

    /// AAD; used for GCM mode
    pub padded_aad: Option<[u8; 32]>,

    /// Tweak; used for XTS mode
    pub tweak: Option<[u32; 4]>,

    /// PRP or SGL for data transfer; true == SGL, false = PRP
    pub psdt: bool,

    /// Data unit length; transfer size FULL == 0x0
    pub data_unit_len: u8,

    /// Input data for encryption/decryption
    pub input_text: [u8; 51],

    /// Source data length
    pub src_len: u32,

    /// Destination data length
    pub dst_len: u32,

    /// Frame ID
    pub frm_id: u8,
}

impl Default for CdmaIoConfig {
    fn default() -> Self {
        Self {
            mode: AesFpCipher::default(),
            op: AesFpOp::default(),
            key1_id: AesBulk256KeyId::default(),
            key2_id: None,
            iv: None,
            iv_bytes: None,
            tag: None,
            unpadded_aad_len: None,
            padded_aad: None,
            tweak: None,
            psdt: true,
            data_unit_len: 0x0,
            input_text: [0u8; 51],
            src_len: 32,
            dst_len: 32,
            frm_id: 0,
        }
    }
}

pub trait CdmaIoTrait {
    /// Creates a key entry in the CDMA Key Vault for CDMA IO operations
    /// and returns the AesBulk256KeyId.
    ///
    /// # Arguments
    /// * `key_slice` - input key slice
    /// * `vault_id` - Vault ID; vault_id = 65 for self test
    ///
    /// # Returns
    ///
    /// * `Result<AesBulk256KeyId, Err>` - Ok if the operation was successful, error otherwise.
    ///
    fn import_key(&self, key_slice: &[u32], vault_id: u8) -> McrResult<AesBulk256KeyId>;

    /// Deletes a key entry from the CDMA Key Vault
    ///
    /// # Arguments
    /// * `key_id` - AesBulk256KeyId for key in the key vault
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn delete_key(&self, key_id: AesBulk256KeyId) -> McrResult<()>;

    /// Clear CDMA Key Vault
    ///
    /// # Arguments
    /// * `self` - CDMA IO object
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn clear_key_vault(&self) -> McrResult<()>;

    /// Returns a key entry from the CDMA Key Vault given a key ID
    ///
    /// # Arguments
    /// * `self` - CDMA IO object
    /// * `key_id` - AesBulk256KeyId for key in the key vault
    ///
    /// # Returns
    ///
    /// * `Result<SecureByteArray<32>, Err>` - Ok if the operation was successful returning the key blob, error otherwise.
    ///
    fn get_entry(&self, key_id: AesBulk256KeyId) -> McrResult<SecureByteArray<32>>;

    /// Initial step of AES GCM encryption/decryption
    ///
    /// # Arguments
    /// * `tag_id` - Tag ID for the AES operation
    /// * `cdma_io_config` - configuration for the CDMA IO operation
    /// * `input_text` - input message to encrypt/decrypt
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn begin_enc_dec(
        &self,
        tag_id: u16,
        cdma_io_config: &CdmaIoConfig,
        input_text: &[u8],
    ) -> McrResult<()>;

    /// Step two of AES GCM encryption/decryption
    ///
    /// # Arguments
    /// * `tag_id` - Tag ID for the AES operation
    /// * `cdma_io_config` - configuration for the CDMA IO operation
    /// * `output_text` - output message after encryption/decryption
    ///
    /// # Returns
    ///
    /// * `Ok(Option<GcmTag>)` - Ok(Option<GcmTag>) if AES GCM encrypt operation was successful,
    ///   Ok(None) if other operation was successful, error otherwise.
    fn end_enc_dec(
        &self,
        tag_id: u16,
        cdma_io_config: &CdmaIoConfig,
        output_text: &mut [u8],
    ) -> McrResult<Option<GcmTag>>;

    /// Zeroize the input and output buffers
    fn zeroize_buffers(&self);
}

mcr_err_decl! {
    CdmaIo,
    CdmaIoErr {
        // CDMA IO Encryption/Decryption Failed
        CdmaIoEncDecFailed = 0x1,

        // CDMA IO Invalid Argument
        InvalidArgument = 0x2,

        // CDMA IO Invalid Key Index
        InvalidKeyIndex = 0x3,

        // CDMA IO No Available Key Slots
        NoAvailableKeySlots = 0x4,

        // CDMA IO Invalid State
        InvalidState = 0x5,

        // CDMA IO Tag Mismatch
        TagMismatch = 0x6,
    }
}
