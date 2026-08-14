// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::vec;
use mcr_crypto_cdma_io::aes_fp_self_test_constants::*;
use mcr_crypto_cdma_io::*;
use mcr_crypto_softaes::*;
use mcr_error::McrResult;
use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_logging::*;
#[cfg(feature = "fips_validation_hooks")]
use mcr_self_test::SelfTest;
#[cfg(feature = "fips_validation_hooks")]
use mcr_soc::SocInfo;
use mcr_types::*;

use cdma_io::AesGcm256SelfTestVectors;
use cdma_io::AesXts256SelfTestVectors;

use crate::error;
use crate::error::AdminErr;
use crate::AdminEnv;

pub(crate) const SELF_TEST_VAULT_ID: usize = 65;
pub(crate) const SELF_TEST_KV_BASE_OFFSET: usize =
    (SELF_TEST_VAULT_ID * MAX_KEYS_PER_TABLE * KEY_SIZE) / size_of::<u32>();

/// AES XTS/GCM self test names
#[derive(Debug, Clone, Copy, PartialEq)]
pub(crate) enum CdmaIoTestName {
    /// AES XTS Negative Encryption Test
    XtsNegEnc,

    /// AES XTS Negative Decryption Test
    XtsNegDec,

    /// AES GCM Aligned and Unaligned Data Test
    GcmAlignedAndUnalignedData,

    /// AES GCM Aligned Data Only Test
    GcmAlignedDataOnly,

    /// AES GCM AAD No Aligned Data Test
    GcmAadNoAlignedData,
}

/// CDMA Key Index
#[derive(Debug, Clone, Copy, PartialEq)]
pub(crate) enum CdmaKeyIndex {
    /// Aes Xts Neg Enc Encryption Key
    XtsNegEncEnc = 0,

    /// Aes Xts Neg Enc Tweak Key
    XtsNegEncTweak = 1,

    /// Aes Xts Neg Dec Encryption Key
    XtsNegDecEnc = 2,

    /// Aes Xts Neg Dec Tweak Key
    XtsNegDecTweak = 3,

    /// Aes Gcm Aligned and Unaligned Data Key
    GcmAlignedAndUnalignedData = 4,

    /// Aes Gcm Aligned Data Only Key
    GcmAlignedDataOnly = 5,

    /// Aes Gcm AAD No Aligned Data Key
    GcmAadNoAlignedData = 6,
}

pub(crate) struct PreOpAesFpSelfTest<'a> {
    /// CDMA IO object
    cdma_io: &'a CdmaIo,

    /// Soft AES object; used for AES GCM tag correction
    soft_aes: &'a SoftAes,

    /// Admin to FP IPC Channel for sending messages
    ipc_channel: &'a IpcMessageChannel,

    /// Self Test Key Table
    self_test_key_table: [Option<AesBulk256KeyId>; MAX_KEYS_PER_TABLE],
}

impl<'a> PreOpAesFpSelfTest<'a> {
    pub(crate) fn new(
        cdma_io: &'a CdmaIo,
        soft_aes: &'a SoftAes,
        ipc_channel: &'a IpcMessageChannel,
    ) -> McrResult<Self> {
        Ok(Self {
            cdma_io,
            soft_aes,
            ipc_channel,
            self_test_key_table: [None; MAX_KEYS_PER_TABLE],
        })
    }

    /// Clear all keys in the self test vault
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    pub(crate) fn clear_self_test_vault(&mut self) -> McrResult<()> {
        // Send IPC to delete all keys in the self test vault, ignore errors because keys may not
        // exist. DeleteAll deletes all the keys loaded with the AppID
        // regardless of the AesBulkKeyType
        let _ =
            self.send_key_update_ipc(KeyUpdateAction::DeleteAll, None, AesBulkKeyType::Xts, None);

        self.cdma_io.clear_key_vault()?;

        Ok(())
    }

    /// Execute AES GCM Bulk 256 Encrypt Self Tests through Fast Path.
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    pub(crate) fn aes_gcm_test(&mut self) -> McrResult<()> {
        let gcm_enc_test_vector = unsafe {
            #[allow(static_mut_refs)]
            &AES_GCM_256_TEST_VECTORS
        };

        // Test combination of Padded AAD + Aligned Data + Unaligned Data
        let gcm_enc_tag_id: u16 = 0x1234;
        self.aes_gcm_self_test(
            CdmaIoTestName::GcmAlignedAndUnalignedData,
            gcm_enc_test_vector,
            gcm_enc_tag_id,
        )?;

        Ok(())
    }

    /// Execute AES GCM Bulk 256 Encrypt Self Tests through Fast Path.
    /// This tests the case where there is only aligned data (no unaligned data).
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    pub(crate) fn aes_gcm_test_aligned_data(&mut self) -> McrResult<()> {
        let gcm_enc_test_vector_aligned_data = unsafe {
            #[allow(static_mut_refs)]
            &AES_GCM_256_AAD_ALIGNED_DATA_ONLY_TEST_VECTORS
        };

        // Test combination of Padded AAD + Aligned Data + No Unaligned Data
        let gcm_enc_tag_id: u16 = 0x1235;
        self.aes_gcm_self_test(
            CdmaIoTestName::GcmAlignedDataOnly,
            gcm_enc_test_vector_aligned_data,
            gcm_enc_tag_id,
        )?;

        Ok(())
    }

    /// Execute AES GCM Bulk 256 Encrypt Self Tests through Fast Path.
    /// This tests the case where there is no aligned data and only unaligned data.
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    pub(crate) fn aes_gcm_test_aad_no_aligned_data(&mut self) -> McrResult<()> {
        let gcm_enc_test_vector_no_aligned_data = unsafe {
            #[allow(static_mut_refs)]
            &AES_GCM_256_AAD_NO_ALIGNED_DATA_TEST_VECTORS
        };

        // Test combination of Padded AAD + No Aligned Data + Unaligned Data
        let gcm_enc_tag_id: u16 = 0x1236;
        self.aes_gcm_self_test(
            CdmaIoTestName::GcmAadNoAlignedData,
            gcm_enc_test_vector_no_aligned_data,
            gcm_enc_tag_id,
        )?;

        Ok(())
    }

    /// Execute XTS Bulk 256 Encrypt Self Tests through Fast Path.
    /// # Arguments
    /// * `cdma_io` - Cdma IO object
    /// * `ipc_channel` - Admin to FP IPC Channel for sending messages
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    pub(crate) fn aes_xts_neg_enc_test(&mut self) -> McrResult<()> {
        let xts_dec_test_vector = unsafe {
            #[allow(static_mut_refs)]
            &AES_XTS_256_TEST_VECTORS
        };
        let xts_dec_tag_id: u16 = 0x4567;
        self.aes_xts_self_test(
            CdmaIoTestName::XtsNegEnc,
            xts_dec_test_vector,
            xts_dec_tag_id,
        )?;

        Ok(())
    }

    /// Execute XTS Bulk 256 Decrypt Self Tests through Fast Path.
    /// # Arguments
    /// * `cdma_io` - Cdma IO object
    /// * `ipc_channel` - Admin to FP IPC Channel for sending messages
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    pub(crate) fn aes_xts_neg_dec_test(&mut self) -> McrResult<()> {
        let xts_dec_test_vector = unsafe {
            #[allow(static_mut_refs)]
            &AES_XTS_256_TEST_VECTORS
        };
        let xts_dec_tag_id: u16 = 0x4568;
        self.aes_xts_self_test(
            CdmaIoTestName::XtsNegDec,
            xts_dec_test_vector,
            xts_dec_tag_id,
        )?;

        Ok(())
    }

    /// Execute AES GCM Bulk 256 Self Test through Fast Path.
    /// <br>Loads validation keys into CDMA key vault,
    /// <br>notifies FP that keys have been updated through IPC message,
    /// <br>executes CDMA IO operation,
    /// <br>deletes self test/validation keys from CDMA keyvault
    ///
    /// # Arguments
    /// * `test_name` - The specific AES GCM self test that is being run
    /// * `test_vector` - Test vector containing key, iv, plaintext, ciphertext and tag
    /// * `tag_id` - Tag ID for the AES operation
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn aes_gcm_self_test(
        &mut self,
        test_name: CdmaIoTestName,
        test_vector: &AesGcm256SelfTestVectors,
        tag_id: u16,
    ) -> McrResult<()> {
        #[cfg(not(feature = "fips_validation_hooks"))]
        let key = &test_vector.key;

        #[cfg(feature = "fips_validation_hooks")]
        let mut mod_key;

        #[cfg(feature = "fips_validation_hooks")]
        let key = if (test_name == CdmaIoTestName::GcmAlignedDataOnly
            && SocInfo::default().induce_cast_failure(SelfTest::AesGcmAlignedData, None))
            || (test_name == CdmaIoTestName::GcmAadNoAlignedData
                && SocInfo::default().induce_cast_failure(SelfTest::AesGcmAadNoAlignedData, None))
        {
            mod_key = test_vector.key;
            mod_key[mod_key.len() - 1] = mod_key[mod_key.len() - 1].wrapping_add(1);
            &mod_key
        } else {
            &test_vector.key
        };

        // create Encyption key
        let key_id = self.cdma_io.import_key(key, SELF_TEST_VAULT_ID as u8)?;

        // setup and send key update IPC message; FP writes the key bytes into the vault.
        let key_bytes = u32_slice_to_key_bytes(key);
        self.send_key_update_ipc(
            KeyUpdateAction::Create,
            Some(key_id.key_index()),
            AesBulkKeyType::GcmUnapproved,
            Some(&key_bytes),
        )?;

        // track the imported key for periodic self test
        match test_name {
            CdmaIoTestName::GcmAlignedAndUnalignedData => {
                self.update_self_test_key_table(CdmaKeyIndex::GcmAlignedAndUnalignedData, key_id)?
            }
            CdmaIoTestName::GcmAlignedDataOnly => {
                self.update_self_test_key_table(CdmaKeyIndex::GcmAlignedDataOnly, key_id)?
            }
            CdmaIoTestName::GcmAadNoAlignedData => {
                self.update_self_test_key_table(CdmaKeyIndex::GcmAadNoAlignedData, key_id)?
            }
            _ => Err(AdminErr::CdmaIoAesGcmSelfTestFailed)?,
        }

        // execute CDMA IO operation Encryption test
        self.execute_gcm_cdma_io(
            key_id,
            test_vector,
            test_vector.ciphertext,
            AesFpMode::GcmEncrypt,
            tag_id,
        )
        .map_err(|_| {
            // zeroize input and output buffers
            self.cdma_io.zeroize_buffers();
            AdminErr::CdmaIoAesGcmSelfTestFailed
        })?;

        #[cfg(not(feature = "fips_validation_hooks"))]
        let ciphertext = test_vector.ciphertext;

        #[cfg(feature = "fips_validation_hooks")]
        let ciphertext = {
            let mut ciphertext = test_vector.ciphertext;
            if test_name == CdmaIoTestName::GcmAlignedAndUnalignedData
                && SocInfo::default()
                    .induce_cast_failure(SelfTest::AesGcmAlignedAndUnalignedData, None)
            {
                ciphertext[0] = ciphertext[0].wrapping_add(1);
                ciphertext[ciphertext.len() - 1] = ciphertext[ciphertext.len() - 1].wrapping_add(1);
            }
            ciphertext
        };

        // execute CDMA IO operation Decryption test
        self.execute_gcm_cdma_io(
            key_id,
            test_vector,
            ciphertext,
            AesFpMode::GcmDecrypt,
            tag_id,
        )
        .map_err(|_| {
            // zeroize input and output buffers
            self.cdma_io.zeroize_buffers();
            AdminErr::CdmaIoAesGcmSelfTestFailed
        })?;

        Ok(())
    }

    /// Setup and execute AES GCM CDMA IO operation.
    /// <br>set up and load the CDMA IO SQE into PSRAM,
    /// <br>notifies FP to process CDMA IO,
    /// <br>checks the CQE,
    /// <br>validates the AES operation output data with expected outputs,
    ///
    /// Production flow differences:
    /// - This self test AES GCM implementation tries to mimic the production flow as closely
    ///   as possible, but this is the key difference to note:
    /// - In the production flow, the inputs (AAD, input text, etc.) are first preprocessed
    ///   into its aligned, unaligned, and padded AAD components. Then the data is processed
    ///   by the FP directly to perform the AES GCM operation on the aligned data. The FP
    ///   determines whether software tag correction is necessary based on the presence of
    ///   unaligned data, and if so, the unaligned data is sent to CP0 for software tag
    ///   correction.
    ///
    /// However, in this self test implementation, the entire processing of the input text and the
    /// decision flow of whether the tag correction is necessary is handled inside
    /// CP0/Admin directly. Thus although the procedure and the same hardware and software
    /// algorithms are used for the processing, the branching decision is not identical
    /// to the production flow.
    ///
    /// # Arguments
    /// * `aes_fp_mode` - The specific AES mode that is being set up
    /// * `test_vector` - Test vector containing key, iv, plaintext, ciphertext and tag
    /// * `tag_id` - Tag ID for the AES operation
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn execute_gcm_cdma_io(
        &mut self,
        enc_key: AesBulk256KeyId,
        test_vector: &AesGcm256SelfTestVectors,
        ciphertext: [u8; 51],
        aes_fp_mode: AesFpMode,
        tag_id: u16,
    ) -> McrResult<()> {
        let op = AesFpOp::from(aes_fp_mode);
        let aes_mode = AesFpCipher::from(aes_fp_mode);
        let text_len = test_vector.text_len;

        let (input_text, output_text) = match op {
            AesFpOp::Decrypt => (&ciphertext[..text_len], &test_vector.plaintext[..text_len]),
            AesFpOp::Encrypt => (&test_vector.plaintext[..text_len], &ciphertext[..text_len]),
        };

        let aligned_data_len = test_vector.aligned_data_len as usize;
        let unaligned_data_len = input_text.len() - aligned_data_len;

        // Self-test detail: `aligned_data_len` is taken from the test vector so we can
        // deterministically exercise both cases:
        // - aligned-only: no software correction required, CDMA tag is final
        // - aligned+unaligned: CDMA tag is intermediate and must be corrected using the remaining
        //   unaligned suffix.
        // This differs from the production flow since the data is provided through a DMA transfer
        // from the host to CP0
        let aad = test_vector
            .padded_aad
            .ok_or(AdminErr::CdmaIoAesGcmSelfTestFailed)?;
        let source_data_len = (aad.len() + aligned_data_len) as u32;

        // setup sqe into PSRAM location
        let cdma_io_config = CdmaIoConfig {
            mode: AesFpCipher::Gcm,
            op,
            key1_id: enc_key,
            iv: Some(test_vector.iv),
            iv_bytes: Some(test_vector.iv_bytes),
            tag: Some(test_vector.tag),
            unpadded_aad_len: test_vector.unpadded_aad_len,
            padded_aad: test_vector.padded_aad,
            src_len: source_data_len,
            dst_len: source_data_len,
            frm_id: 1,
            ..Default::default()
        };

        // setup sqe into PSRAM location
        self.cdma_io
            .begin_enc_dec(tag_id, &cdma_io_config, &input_text[0..aligned_data_len])?;

        // setup and send cdma io ipc message
        self.send_cdma_io_ipc()?;

        // wait for message response
        let ipc_message = AdminEnv::wait_for_response(self.ipc_channel);
        let resp_message: IpcMessageCdmaIoResp = IpcMessageDecoder::decode(ipc_message)?;
        let resp_header = resp_message.header;

        // check cdma io ipc message response only for encryption
        // Self-test note: This check is done in the FP firmware to determine
        // whether software Tag correction is necessary
        if !(cdma_io_config.mode == AesFpCipher::Gcm && cdma_io_config.op == AesFpOp::Decrypt) {
            Self::check_resp_cdma_io_ipc(resp_header, aes_mode)?;
        }

        // validate cqe and output text
        let mut output_buf = vec![0u8; source_data_len as usize];

        let gcm_tag =
            self.cdma_io
                .end_enc_dec(tag_id, &cdma_io_config, output_buf.as_mut_slice())?;

        // If there is no unaligned data, we skip the tag correction
        // otherwise run the software tag correction
        // Note: This logic is to mimic the logic from the FP where it decides whether to send
        // the unaligned data to CP0 for tag correction processing based on
        // the unaligned data length. Here it is calculated based on whether
        // there is unaligned data or not.
        let final_tag = if unaligned_data_len == 0 {
            gcm_tag.ok_or(AdminErr::CdmaIoAesGcmSelfTestFailed)?
        } else {
            let key_blob: SecureByteArray<KEY_SIZE> = self.cdma_io.get_entry(enc_key)?;
            let mut tag_ext_out_buf = vec![0u8; unaligned_data_len];
            let intermediate_tag = gcm_tag.ok_or(AdminErr::CdmaIoAesGcmSelfTestFailed)?;

            let iv = cdma_io_config
                .iv_bytes
                .ok_or(AdminErr::CdmaIoAesGcmSelfTestFailed)?;

            let unpadded_aad_len = cdma_io_config
                .unpadded_aad_len
                .ok_or(AdminErr::CdmaIoAesGcmSelfTestFailed)?
                as u64;

            // run gcm tag correction for the remaining unaligned data
            let updated_tag = self.soft_aes.aes_gcm_tag_correction(
                op == AesFpOp::Encrypt,
                key_blob.as_slice(),
                &iv,
                unpadded_aad_len,
                input_text.len() as u64,
                None,
                Some(&intermediate_tag),
                &input_text[aligned_data_len..],
                aligned_data_len,
                tag_ext_out_buf.as_mut_slice(),
            )?;

            output_buf.extend_from_slice(&tag_ext_out_buf);

            updated_tag
        };

        // In the production flow, this step gets processed by the Host software
        output_buf.drain(0..aad.len());

        if final_tag != test_vector.tag {
            Err(AdminErr::CdmaIoAesGcmSelfTestFailed)?
        }

        if output_text != output_buf.as_slice() {
            Err(AdminErr::CdmaIoAesGcmSelfTestFailed)?
        }

        // Clear test buffers
        self.cdma_io.zeroize_buffers();

        Ok(())
    }

    /// Execute AES XTS Bulk 256 Self Test through Fast Path.
    /// <br>Loads validation keys into CDMA key vault,
    /// <br>notifies FP that keys have been updated through IPC message,
    /// <br>executes CDMA IO operation,
    /// <br>deletes self test/validation keys from CDMA keyvault
    ///
    /// # Arguments
    /// * `test_name` - The specific AES XTS self test that is being run
    /// * `test_vector` - Test vector containing key, iv, plaintext, ciphertext and tag
    /// * `tag_id` - Tag ID for the AES operation
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn aes_xts_self_test(
        &mut self,
        test_name: CdmaIoTestName,
        test_vector: &AesXts256SelfTestVectors,
        tag_id: u16,
    ) -> McrResult<()> {
        #[cfg(not(feature = "fips_validation_hooks"))]
        let key = &test_vector.enc_key;

        #[cfg(feature = "fips_validation_hooks")]
        let mut mod_key;

        #[cfg(feature = "fips_validation_hooks")]
        let key = if test_name == CdmaIoTestName::XtsNegEnc
            && SocInfo::default().induce_cast_failure(SelfTest::AesXtsNegEnc, None)
        {
            mod_key = test_vector.enc_key;
            mod_key[mod_key.len() - 1] = mod_key[mod_key.len() - 1].wrapping_add(1);
            &mod_key
        } else {
            &test_vector.enc_key
        };

        // create Encyption and Tweak keys
        let enc_key = self.cdma_io.import_key(key, SELF_TEST_VAULT_ID as u8)?;

        let tweak_key = self
            .cdma_io
            .import_key(&test_vector.tweak_key, SELF_TEST_VAULT_ID as u8)?;

        // track the imported key for periodic self test
        match test_name {
            CdmaIoTestName::XtsNegEnc => {
                self.update_self_test_key_table(CdmaKeyIndex::XtsNegEncEnc, enc_key)?;
                self.update_self_test_key_table(CdmaKeyIndex::XtsNegEncTweak, tweak_key)?;
            }
            CdmaIoTestName::XtsNegDec => {
                self.update_self_test_key_table(CdmaKeyIndex::XtsNegDecEnc, enc_key)?;
                self.update_self_test_key_table(CdmaKeyIndex::XtsNegDecTweak, tweak_key)?;
            }
            _ => Err(AdminErr::CdmaIoAesXtsSelfTestFailed)?,
        }

        // setup and send key update IPC messages; FP writes the key bytes into the vault.
        let enc_key_bytes = u32_slice_to_key_bytes(key);
        let tweak_key_bytes = u32_slice_to_key_bytes(&test_vector.tweak_key);
        self.send_key_update_ipc(
            KeyUpdateAction::Create,
            Some(enc_key.key_index()),
            AesBulkKeyType::Xts,
            Some(&enc_key_bytes),
        )?;
        self.send_key_update_ipc(
            KeyUpdateAction::Create,
            Some(tweak_key.key_index()),
            AesBulkKeyType::Xts,
            Some(&tweak_key_bytes),
        )?;

        // execute CDMA IO operation Encryption test
        self.execute_xts_cdma_io(
            enc_key,
            tweak_key,
            test_vector,
            test_vector.ciphertext,
            AesFpMode::XtsEncrypt,
            tag_id,
        )
        .map_err(|_| {
            // zeroize input and output buffers
            self.cdma_io.zeroize_buffers();
            AdminErr::CdmaIoAesXtsSelfTestFailed
        })?;

        #[cfg(not(feature = "fips_validation_hooks"))]
        let ciphertext = test_vector.ciphertext;

        #[cfg(feature = "fips_validation_hooks")]
        let ciphertext = {
            let mut ciphertext = test_vector.ciphertext;
            if test_name == CdmaIoTestName::XtsNegDec
                && SocInfo::default().induce_cast_failure(SelfTest::AesXtsNegDec, None)
            {
                ciphertext[ciphertext.len() - 1] = ciphertext[ciphertext.len() - 1].wrapping_add(1);
            }

            ciphertext
        };

        // execute CDMA IO operation Decryption test
        self.execute_xts_cdma_io(
            enc_key,
            tweak_key,
            test_vector,
            ciphertext,
            AesFpMode::XtsDecrypt,
            tag_id,
        )
        .map_err(|_| {
            // zeroize input and output buffers
            self.cdma_io.zeroize_buffers();
            AdminErr::CdmaIoAesXtsSelfTestFailed
        })?;

        Ok(())
    }

    /// Setup and execute AES XTS CDMA IO operation.
    /// <br>set up and load the CDMA IO SQE into PSRAM,
    /// <br>notifies FP to process CDMA IO,
    /// <br>checks the CQE,
    /// <br>validates the AES operation output data with expected outputs,
    ///
    /// # Arguments
    /// * `aes_fp_mode` - The specific AES mode that is being set up
    /// * `test_vector` - Test vector containing key, iv, plaintext, ciphertext and tag
    /// * `tag_id` - Tag ID for the AES operation
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn execute_xts_cdma_io(
        &mut self,
        enc_key: AesBulk256KeyId,
        tweak_key: AesBulk256KeyId,
        test_vector: &AesXts256SelfTestVectors,
        ciphertext: [u8; 32],
        aes_fp_mode: AesFpMode,
        tag_id: u16,
    ) -> McrResult<()> {
        let op = AesFpOp::from(aes_fp_mode);
        let aes_mode = AesFpCipher::from(aes_fp_mode);

        let (input_text, output_text) = match op {
            AesFpOp::Decrypt => (&ciphertext, &test_vector.plaintext),
            AesFpOp::Encrypt => (&test_vector.plaintext, &ciphertext),
        };

        let cdma_io_config = CdmaIoConfig {
            mode: AesFpCipher::Xts,
            op,
            key1_id: enc_key,
            key2_id: Some(tweak_key),
            tweak: Some(test_vector.tweak),
            ..Default::default()
        };

        // setup sqe into PSRAM location
        self.cdma_io
            .begin_enc_dec(tag_id, &cdma_io_config, input_text)?;

        // setup and send cdma io ipc message
        self.send_cdma_io_ipc()?;

        // wait for message response
        let ipc_message = AdminEnv::wait_for_response(self.ipc_channel);
        let resp_message: IpcMessageCdmaIoResp = IpcMessageDecoder::decode(ipc_message)?;
        let resp_header = resp_message.header;

        // check cdma io ipc message response
        Self::check_resp_cdma_io_ipc(resp_header, aes_mode)?;

        // validate cqe and output text
        let result_len = output_text.len();
        let mut res_buf = vec![0u8; result_len];

        self.cdma_io
            .end_enc_dec(tag_id, &cdma_io_config, res_buf.as_mut_slice())?;

        if output_text != res_buf.as_slice() {
            Err(AdminErr::CdmaIoAesXtsSelfTestFailed)?
        }

        // Clear test buffers
        self.cdma_io.zeroize_buffers();

        Ok(())
    }

    /// Update key in self test key table
    ///
    /// # Arguments
    /// * `key_index` - The CDMA key vault key index of updated key
    /// * `key_id` - The Aes Bulk 256 Key ID of the updated key
    ///
    /// # Returns
    /// `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn update_self_test_key_table(
        &mut self,
        key_index: CdmaKeyIndex,
        key_id: AesBulk256KeyId,
    ) -> McrResult<()> {
        let index = key_index as usize;
        if index >= MAX_KEYS_PER_TABLE {
            Err(AdminErr::CdmaIoKeyUpdateFailed)?;
        }
        self.self_test_key_table[index] = Some(key_id);
        Ok(())
    }

    pub(crate) fn cdma_io_self_test_key_table(
        self,
    ) -> [Option<AesBulk256KeyId>; MAX_KEYS_PER_TABLE] {
        self.self_test_key_table
    }

    /// Send IPC message to inform FP that keys have been updated in the CDMA key vault.
    /// FP is the sole writer of the CDMA vault contents (see ADR-0002): on Create,
    /// `key_bytes` carries the raw 32-byte AES key; FP writes it into the vault.
    /// On Delete / DeleteAll the parameter is ignored (caller passes `None`).
    ///
    /// # Arguments
    /// * `action` - Key update action (Create / Delete / DeleteAll)
    /// * `key_index` - The CDMA key vault key index of updated key
    /// * `key_type` - AES bulk key type
    /// * `key_bytes` - Raw 32-byte key for Create; `None` for Delete / DeleteAll
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn send_key_update_ipc(
        &self,
        action: KeyUpdateAction,
        key_index: Option<u8>,
        key_type: AesBulkKeyType,
        key_bytes: Option<&[u8; KEY_SIZE]>,
    ) -> McrResult<()> {
        let key_data = key_bytes.copied().unwrap_or([0u8; KEY_SIZE]);
        let message = IpcMessageKeyUpdate {
            info: KeyUpdateInfo {
                key_index: key_index.unwrap_or_default(),
                // Resource id 65 (66th resource) used for AES FP self test
                resource_id: 65,
                // Pfn index 65 (66th resource) used for AES FP self test
                pfn: PcieFunction(65),
                action,
                session_id: 0,
                app_id: 0,
                flag: AesKeyFlag::new()
                    .with_session_only(false)
                    .with_key_type(key_type),
                key_data,
            },
            ..Default::default()
        };

        self.ipc_channel.send_request(0, message.encode())?;

        let ipc_message = AdminEnv::wait_for_response(self.ipc_channel);
        let resp_message: IpcMessageKeyUpdate = IpcMessageDecoder::decode(ipc_message)?;

        if resp_message.header.status() != 0 {
            Err(AdminErr::CdmaIoKeyUpdateFailed)?;
        }

        Ok(())
    }

    /// Send IPC message to inform FP that keys have been updated in the CDMA key vault
    ///
    /// # Arguments
    /// * `ipc_channel` - Admin to FP IPC Channel for sending messages
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn send_cdma_io_ipc(&self) -> McrResult<()> {
        let message = IpcMessageCdmaIoReq {
            info: CdmaIoMsgDataReq {
                dw0: CdmaIoMsgDataReqDw0::new().with_vfid(SELF_TEST_VAULT_ID as u8),
                dw1: CdmaIoMsgDataReqDw1::new()
                    .with_src_desc_inter_sel(MemoryLocation::Soc.into())
                    .with_src_data_inter_sel(MemoryLocation::Soc.into())
                    .with_dst_desc_inter_sel(MemoryLocation::Soc.into())
                    .with_dst_data_inter_sel(MemoryLocation::Soc.into()),
            },
            ..Default::default()
        };

        let ipc_message = message.encode();
        self.ipc_channel.send_request(0, ipc_message)?;

        Ok(())
    }

    /// Check if FP sends a successful CDMA IO IPC response message
    ///
    /// # Arguments
    /// * `resp_header` - Response IPC message header from FP
    /// * `aes_mode` - The specific AES mode that is being set up
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Ok if the operation was successful, error otherwise.
    ///
    fn check_resp_cdma_io_ipc(
        resp_header: IpcMessageHeader,
        aes_mode: AesFpCipher,
    ) -> McrResult<()> {
        if resp_header.status() != 0 {
            error!("IPC response status: {:x}", resp_header.status());
            match aes_mode {
                AesFpCipher::Xts => Err(AdminErr::CdmaIoAesXtsSelfTestFailed)?,
                AesFpCipher::Gcm => Err(AdminErr::CdmaIoAesGcmSelfTestFailed)?,
            }
        }

        Ok(())
    }
}

/// Convert a `&[u32; 8]` AES-256 key into a `[u8; 32]` little-endian byte array
/// suitable for the IPC payload.
fn u32_slice_to_key_bytes(key: &[u32]) -> [u8; KEY_SIZE] {
    debug_assert_eq!(core::mem::size_of_val(key), KEY_SIZE);
    let mut out = [0u8; KEY_SIZE];
    for (chunk, &word) in out
        .chunks_exact_mut(core::mem::size_of::<u32>())
        .zip(key.iter())
    {
        chunk.copy_from_slice(&word.to_le_bytes());
    }
    out
}
