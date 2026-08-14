// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::rc::Rc;
use core::cell::RefCell;

use bitfield::Bit;
use bitfield::BitMut;
use mcr_error::McrResult;
use mcr_mem_map::*;
use zeroize::Zeroize;

use crate::*;

/// AES GCM 256 Test Vector
#[repr(C)]
pub struct AesGcm256SelfTestVectors {
    /// Length of aligned portion of src data in bytes
    pub aligned_data_len: u32,

    /// Length of src data in bytes
    pub text_len: usize,

    /// Key used for AES-256 encryption
    pub key: [u32; KEY_SIZE_IN_DWORDS],

    /// Initialization vector (nonce) for GCM mode
    pub iv: [u32; 3],

    /// IV vector as byte array
    pub iv_bytes: [u8; 12],

    /// Plaintext message to be encrypted
    pub plaintext: [u8; 51],

    /// Ciphertext output after encryption
    pub ciphertext: [u8; 51],

    /// Length of unpadded AAD
    pub unpadded_aad_len: Option<u32>,

    /// Additional authenticated data
    pub padded_aad: Option<[u8; 32]>,

    /// Authentication tag
    pub tag: GcmTag,
}

/// AES XTS 256 Test Vector
#[repr(C)]
pub struct AesXts256SelfTestVectors {
    // AES XTS 256 encryption key
    pub enc_key: [u32; KEY_SIZE_IN_DWORDS],

    // AES XTS 256 tweak key
    pub tweak_key: [u32; KEY_SIZE_IN_DWORDS],

    /// Tweak value
    pub tweak: [u32; 4],

    /// Plaintext message to be validated
    pub plaintext: [u8; 32],

    /// Ciphertext encrypted input
    pub ciphertext: [u8; 32],
}

/// AES FP Self Test
#[derive(Clone)]
pub struct CdmaIo {
    rimpl: Rc<RefCell<CdmaIoImpl>>,
}

impl CdmaIo {
    pub fn new(
        key_vault: &'static mut [u32],
        input_buffer: &'static mut [u8],
        output_buffer: &'static mut [u8],
    ) -> McrResult<Self> {
        let cdma_io_impl = CdmaIoImpl::new(key_vault, input_buffer, output_buffer)?;

        Ok(Self {
            rimpl: Rc::new(RefCell::new(cdma_io_impl)),
        })
    }
}

impl CdmaIoTrait for CdmaIo {
    fn import_key(&self, key_slice: &[u32], vault_id: u8) -> McrResult<AesBulk256KeyId> {
        self.rimpl.borrow_mut().import_key(key_slice, vault_id)
    }

    fn delete_key(&self, key_id: AesBulk256KeyId) -> McrResult<()> {
        self.rimpl.borrow_mut().delete_key(key_id)
    }

    fn clear_key_vault(&self) -> McrResult<()> {
        self.rimpl.borrow_mut().clear_key_vault()
    }

    fn get_entry(&self, key_id: AesBulk256KeyId) -> McrResult<SecureByteArray<32>> {
        self.rimpl.borrow_mut().get_entry(key_id)
    }

    fn begin_enc_dec(
        &self,
        tag_id: u16,
        cdma_io_config: &CdmaIoConfig,
        input_text: &[u8],
    ) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .begin_enc_dec(tag_id, cdma_io_config, input_text)
    }

    fn end_enc_dec(
        &self,
        tag_id: u16,
        cdma_io_config: &CdmaIoConfig,
        output_text: &mut [u8],
    ) -> McrResult<Option<[u8; 16]>> {
        self.rimpl
            .borrow_mut()
            .end_enc_dec(tag_id, cdma_io_config, output_text)
    }

    fn zeroize_buffers(&self) {
        self.rimpl.borrow_mut().zeroize_buffers();
    }
}

struct CdmaIoImpl {
    /// Cdma IO Key Vault
    key_vault: &'static mut [u32],

    /// Input buffer
    input_buffer: &'static mut [u8],

    /// Output buffer
    output_buffer: &'static mut [u8],

    /// Vault table available key slots
    table_availability: u8,

    /// Cdma IO SQE
    cdma_io_sqe: &'static mut CdmaIoSqe,

    /// Cdma IO CQE
    cdma_io_cqe: &'static CdmaIoCqe,

    /// Tag ID
    tag_id: Option<u16>,
}

impl CdmaIoImpl {
    /// Create an instance of AES FP Self Test helper object
    ///
    /// # Arguments
    /// * `key_table` - CDMA Key table
    ///
    /// # Returns
    ///
    /// * `McrResult<Self>` - AesFpSelfTestImpl if successful, error otherwise.
    ///
    fn new(
        key_table: &'static mut [u32],
        input_buffer: &'static mut [u8],
        output_buffer: &'static mut [u8],
    ) -> McrResult<Self> {
        let cdma_io = CdmaIoImpl {
            key_vault: key_table,
            input_buffer,
            output_buffer,
            table_availability: 0_u8,
            cdma_io_sqe: &mut PsRamMemMap::cdma_io_rx_entry()[0],
            cdma_io_cqe: &PsRamMemMap::cdma_io_tx_entry()[0],
            tag_id: None,
        };

        Ok(cdma_io)
    }

    /// Allocates a key slot in the CDMA Key Vault bitmap for CDMA IO operations
    /// and returns the AesBulk256KeyId. The actual key bytes are programmed into
    /// the vault by FP via the KeyUpdate IPC -- this layer is metadata-only.
    ///
    /// # Arguments
    /// * `self` - CDMA IO object
    /// * `key_slice` - input key slice (size validated only; bytes not written here)
    /// * `vault_id` - Vault ID; vault_id = 65 for self test
    ///
    /// # Returns
    ///
    /// * `Result<AesBulk256KeyId, Err>` - Ok if a slot was available, error otherwise.
    ///
    fn import_key(&mut self, key_slice: &[u32], vault_id: u8) -> McrResult<AesBulk256KeyId> {
        if key_slice.len() != KEY_SIZE_IN_DWORDS {
            Err(CdmaIoErr::InvalidArgument)?
        }

        for key_index in 0..MAX_KEYS_PER_TABLE {
            if !self.table_availability.bit(key_index) {
                self.table_availability.set_bit(key_index, true);

                let bulk_key_id = AesBulk256KeyId::new()
                    .with_key_index(key_index as u8)
                    .with_vault_id(vault_id)
                    .with_rsvd(0);

                return Ok(bulk_key_id);
            }
        }

        Err(CdmaIoErr::NoAvailableKeySlots)?
    }

    /// Deletes a key entry from the CDMA Key Vault
    ///
    /// # Arguments
    /// * `self` - CDMA IO object
    /// * `key_id` - AesBulk256KeyId for key in the key vault
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn delete_key(&mut self, key_id: AesBulk256KeyId) -> McrResult<()> {
        let key_index = key_id.key_index() as usize;

        if !self.table_availability.bit(key_index) {
            Err(CdmaIoErr::InvalidKeyIndex)?
        }

        // FP zeroizes the vault bytes when it processes the KeyUpdate Delete IPC.
        // This layer only releases the bitmap slot.
        self.table_availability.set_bit(key_index, false);

        Ok(())
    }

    /// Clear all bitmap slots in the CDMA Key Vault. FP zeroizes the actual
    /// vault bytes via the KeyUpdate DeleteAll IPC; this layer only resets
    /// the slot-availability bitmap.
    ///
    /// # Arguments
    /// * `self` - CDMA IO object
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful
    ///
    fn clear_key_vault(&mut self) -> McrResult<()> {
        self.table_availability = 0;

        Ok(())
    }

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
    fn get_entry(&mut self, key_id: AesBulk256KeyId) -> McrResult<SecureByteArray<32>> {
        let key_index = key_id.key_index() as usize;

        if !self.table_availability.bit(key_index) {
            Err(CdmaIoErr::InvalidKeyIndex)?
        }

        // Copy the key from key vault
        let offset = key_index * KEY_SIZE_IN_DWORDS;
        let key_in_dwords = &self.key_vault[offset..offset + KEY_SIZE_IN_DWORDS];

        // Convert key_in_dwords into byte array
        let mut key_blob = SecureByteArray::<KEY_SIZE>::new([0u8; KEY_SIZE]);
        let key_bytes = key_blob.as_mut_slice();
        for (&key_word, dst_bytes) in key_in_dwords.iter().zip(key_bytes.chunks_mut(4)) {
            dst_bytes.copy_from_slice(&key_word.to_le_bytes());
        }

        Ok(key_blob)
    }

    /// Initial step of AES GCM encryption/decryption
    ///
    /// # Arguments
    /// * `self` - CDMA IO object
    /// * `tag_id` - Tag ID for the AES operation
    /// * `cdma_io_config` - configuration for the CDMA IO operation
    /// * `input_text` - input data
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn begin_enc_dec(
        &mut self,
        tag_id: u16,
        cdma_io_config: &CdmaIoConfig,
        input_text: &[u8],
    ) -> McrResult<()> {
        // Check if tag_id is already set by another operation
        if self.tag_id.is_some() {
            Err(CdmaIoErr::InvalidState)?
        }

        self.tag_id = Some(tag_id);

        // Zeroize input and output buffers
        self.input_buffer.zeroize();

        // Load input text into input buffer for processing
        let mut index = 0usize;
        if let Some(aad) = cdma_io_config.padded_aad {
            index = aad.len();
            self.input_buffer[..index].copy_from_slice(&aad);
        }

        self.input_buffer[index..index + input_text.len()].copy_from_slice(input_text);

        // Configure the CDMA IO SQE
        match cdma_io_config.mode {
            AesFpCipher::Gcm => {
                self.prepare_gcm_sqe(cdma_io_config)?;
            }
            AesFpCipher::Xts => {
                self.prepare_xts_sqe(cdma_io_config)?;
            }
        }

        Ok(())
    }

    /// Step two of AES GCM encryption/decryption
    ///
    /// # Arguments
    /// * `self` - CDMA IO object
    /// * `tag_id` - Tag ID for the AES operation
    /// * `cdma_io_config` - configuration for the CDMA IO operation
    /// * `output_text` - output data
    ///
    /// # Returns
    ///
    /// * `Ok(Option<GcmTag>)` - Ok(Option<GcmTag>) if AES GCM encrypt operation was successful,
    ///   Ok(None) if other operation was successful, error otherwise.
    fn end_enc_dec(
        &mut self,
        tag_id: u16,
        cdma_io_config: &CdmaIoConfig,
        output_text: &mut [u8],
    ) -> McrResult<Option<GcmTag>> {
        // Check if tag_id matches the current tag_id
        let current_tag_id = self.tag_id.ok_or(CdmaIoErr::InvalidState)?;
        if tag_id != current_tag_id {
            Err(CdmaIoErr::TagMismatch)?
        }

        // Consume the tag_id
        self.tag_id.take();

        // Only check CQE status for encrypt operations. For GCM decrypt with unaligned source
        // data, we skip this check because the CDMA engine will report a tag mismatch error.
        // This happens because the CDMA engine only processes the aligned portion of the data,
        // and the CP must correct the tag afterward using the remaining unaligned data.
        if !(cdma_io_config.mode == AesFpCipher::Gcm && cdma_io_config.op == AesFpOp::Decrypt)
            && self.cdma_io_cqe.status.status_code() != 0
        {
            self.zeroize_buffers();
            Err(CdmaIoErr::CdmaIoEncDecFailed)?
        }

        // Verify the output text length is valid
        if output_text.len() > self.output_buffer.len() {
            self.zeroize_buffers();
            Err(CdmaIoErr::CdmaIoEncDecFailed)?
        }

        output_text.copy_from_slice(&self.output_buffer[..output_text.len()]);

        let mut maybe_gcm_tag = None;
        if cdma_io_config.mode == AesFpCipher::Gcm {
            // extract the tag from the cqe
            let gcm_tag: [u8; 16] = self.cdma_io_cqe.tag;

            maybe_gcm_tag = Some(gcm_tag);
        }

        Ok(maybe_gcm_tag)
    }

    /// Zeroize the input and output buffers
    ///
    /// # Arguments
    /// * `self` - CDMA IO object
    ///
    fn zeroize_buffers(&mut self) {
        self.input_buffer.zeroize();
        self.output_buffer.zeroize();
    }

    /// Configure AES GCM 256 SQE
    ///
    /// # Arguments
    /// * `self` - CDMA IO object
    /// * `cdma_io_config` - configuration for the CDMA IO operation
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn prepare_gcm_sqe(&mut self, cdma_io_config: &CdmaIoConfig) -> McrResult<()> {
        let aad = cdma_io_config
            .padded_aad
            .ok_or(CdmaIoErr::InvalidArgument)?;
        let unpadded_aad_len = cdma_io_config
            .unpadded_aad_len
            .ok_or(CdmaIoErr::InvalidArgument)?;
        let mut cdma_io_sqe = CdmaIoGcmSqe {
            info: CdmaIoSqeCommon {
                attr: CdmaIoSqeAttr::new()
                    .with_op(cdma_io_config.op.into())
                    .with_psdt(cdma_io_config.psdt)
                    .with_ctype(CDMA_CMD_TYPE_FAST_PATH)
                    .with_cipher(cdma_io_config.mode.into())
                    .with_du(cdma_io_config.data_unit_len),
                src_length: cdma_io_config.src_len,
                src_prp1: MemoryAddr {
                    lo: self.input_buffer.as_ptr() as u32,
                    ..Default::default()
                },
                src_prp2: MemoryAddr {
                    lo: cdma_io_config.src_len,
                    ..Default::default()
                },
                dst_length: cdma_io_config.dst_len,
                dst_prp1: MemoryAddr {
                    lo: self.output_buffer.as_ptr() as u32,
                    ..Default::default()
                },
                dst_prp2: MemoryAddr {
                    lo: cdma_io_config.dst_len,
                    ..Default::default()
                },
                frm_id: cdma_io_config.frm_id,
                ..Default::default()
            },
            cmd: GcmCmd {
                key_idx: u16::from(cdma_io_config.key1_id) as u32,
                unpadded_aad_length: unpadded_aad_len,
                aad_length: aad.len() as u32,
                iv: cdma_io_config.iv_bytes.ok_or(CdmaIoErr::InvalidArgument)?,
                ..Default::default()
            },
        };

        // Provide the tag as part of the SQE for decrypt
        if cdma_io_config.op == AesFpOp::Decrypt {
            cdma_io_sqe.cmd.tag = cdma_io_config.tag.ok_or(CdmaIoErr::InvalidArgument)?;
        }

        let cdma_io_u32_arr = CdmaIoSqe::from(cdma_io_sqe);

        for (index, item) in cdma_io_u32_arr.data.iter().enumerate() {
            self.cdma_io_sqe.data[index] = *item;
        }

        Ok(())
    }

    /// Configure AES XTS 256 SQE
    ///
    /// # Arguments
    /// * `self` - CDMA IO object
    /// * `cdma_io_config` - configuration for the CDMA IO operation
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn prepare_xts_sqe(&mut self, cdma_io_config: &CdmaIoConfig) -> McrResult<()> {
        let cdma_io_sqe = CdmaIoXtsSqe {
            info: CdmaIoSqeCommon {
                attr: CdmaIoSqeAttr::new()
                    .with_op(cdma_io_config.op.into())
                    .with_psdt(cdma_io_config.psdt)
                    .with_ctype(CDMA_CMD_TYPE_FAST_PATH)
                    .with_cipher(cdma_io_config.mode.into())
                    .with_du(cdma_io_config.data_unit_len),
                src_length: cdma_io_config.src_len,
                src_prp1: MemoryAddr {
                    lo: self.input_buffer.as_ptr() as u32,
                    ..Default::default()
                },
                src_prp2: MemoryAddr {
                    lo: 32,
                    ..Default::default()
                },
                dst_length: cdma_io_config.dst_len,
                dst_prp1: MemoryAddr {
                    lo: self.output_buffer.as_ptr() as u32,
                    ..Default::default()
                },
                dst_prp2: MemoryAddr {
                    lo: 32,
                    ..Default::default()
                },
                ..Default::default()
            },
            cmd: XtsCmd {
                key1_idx: u16::from(cdma_io_config.key1_id) as u32,
                key2_idx: u16::from(cdma_io_config.key2_id.ok_or(CdmaIoErr::InvalidArgument)?)
                    as u32,
                tweak: cdma_io_config.tweak.ok_or(CdmaIoErr::InvalidArgument)?,
                ..Default::default()
            },
        };

        let cdma_io_u32_arr = CdmaIoSqe::from(cdma_io_sqe);

        for (index, item) in cdma_io_u32_arr.data.iter().enumerate() {
            self.cdma_io_sqe.data[index] = *item;
        }

        Ok(())
    }
}
