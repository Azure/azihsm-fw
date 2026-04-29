// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_sha::HkdfInfo;
use mcr_crypto_sha::KbkdfInfo;
use mcr_crypto_sha::KbkdfInputData;
use mcr_crypto_sha::ShaMode;
use mcr_crypto_sha::ShaTrait;
use mcr_crypto_sha::HKDF_MAX_INFO_SIZE;
use mcr_crypto_sha::HKDF_MAX_INPUT_BUF_SIZE;
use mcr_crypto_sha::KBKDF_MAX_CONTEXT_SIZE;
use mcr_crypto_sha::KBKDF_MAX_INPUT_BUF_SIZE;
use mcr_crypto_sha::KBKDF_MAX_LABEL_SIZE;
use mcr_ddi_mbor::MborByteArray;
use mcr_types::SHA_DIGEST_MAX_SIZE_BYTES;

use self::hmac::HmacHashAlgorithm;
use super::*;

// Data size for Aes256 key type
const MAX_AES_SIZE: usize = 32;
// Maximum buffer size for sha_zc op
const MAX_SHA_OUT_BUFFERSIZE: usize = SHA_DIGEST_MAX_SIZE_BYTES;

impl<E: HsmEnvTrait> UserSession<E> {
    /// Helper to execute HKDF for Secret type key
    #[allow(clippy::too_many_arguments)]
    pub(super) fn hkdf_inner(
        &self,
        key_id: KeyId,
        salt: &[u8],
        info: &[u8],
        hash_algo: DdiHashAlgorithm,
        key_type: DdiKeyType,
        key_properties: DdiKeyProperties,
        key_tag: Option<u16>,
        key_len: Option<u8>,
    ) -> HsmResult<KeyId> {
        // Extract the key blob
        let secret_key = self.ecdh_key(key_id, Some(EcdhKeyUsage::KeyAgreement))?;
        let secret_key_blob = secret_key.blob()?;

        // Secret keys with 66 size are extended to 68 as they must be 4 byte aligned;
        // Only use the first 66 bytes.
        let secret_key_slice = if secret_key_blob.len() == 68 {
            &secret_key_blob[..66]
        } else {
            &secret_key_blob[..secret_key_blob.len()]
        };

        let key_kind: EntryKind = key_type.try_into().map_err(|_| HsmErr::InvalidKeyType)?;

        // Insert key into vault and get key_id
        let key_id = match key_type {
            DdiKeyType::Aes128 | DdiKeyType::Aes192 | DdiKeyType::Aes256 => {
                // Determine size of output
                let aes_kind: AesKeyKind = key_kind.try_into()?;
                let out_len = usize::from(aes_kind);

                // Allocate GSRAM buffer for output
                let mut hkdf_output_gsram =
                    self.dma_alloc(MAX_SHA_OUT_BUFFERSIZE + MAX_AES_SIZE)?;

                // Perform HKDF
                self.hkdf_impl(
                    secret_key_slice,
                    salt,
                    info,
                    hash_algo,
                    hkdf_output_gsram.as_ref_mut(),
                    out_len as u16,
                )?;

                // Create the key to be imported into the key vault
                let key = AesKeyImported::new(
                    aes_kind,
                    key_properties.key_usage.try_into()?,
                    &hkdf_output_gsram.as_ref()[..out_len],
                )?;

                // Import the AES key into the key vault
                let key_usage: AesKeyUsage = key_properties.key_usage.try_into()?;
                let attributes = aes_entry_attributes(
                    key_properties.key_availability.try_into()?,
                    true,
                    key_usage,
                );
                let aes_key = self.state.vault().aes_import_key(
                    self.app_vault_id(),
                    self.id(),
                    key_tag,
                    &key,
                    &attributes,
                )?;

                aes_key.id()
            }
            DdiKeyType::HmacSha256 | DdiKeyType::HmacSha384 | DdiKeyType::HmacSha512 => {
                // Determine size of output
                let out_len = key_kind.raw_key_blob_size();

                // Allocate GSRAM buffer for output
                let mut hkdf_output_gsram =
                    self.dma_alloc(MAX_SHA_OUT_BUFFERSIZE + usize::from(HmacKeyKind::HmacKey512))?;

                // Perform HKDF
                self.hkdf_impl(
                    secret_key_slice,
                    salt,
                    info,
                    hash_algo,
                    hkdf_output_gsram.as_ref_mut(),
                    out_len as u16,
                )?;

                // Create the key to be imported into the key vault
                let key = HmacKeyImported::new(
                    key_type.try_into()?,
                    key_properties.key_usage.try_into()?,
                    &hkdf_output_gsram.as_ref()[..out_len],
                )?;

                // Import the HMAC key into thekey  vault
                let hmac_key = self.state.vault().hmac_import_key(
                    self.app_vault_id(),
                    self.id(),
                    key_tag,
                    true,
                    &key,
                    key_properties.key_availability.try_into()?,
                )?;

                hmac_key.id()
            }
            DdiKeyType::VarLenHmacSha256
            | DdiKeyType::VarLenHmacSha384
            | DdiKeyType::VarLenHmacSha512 => {
                // Determine size of output
                let out_len = match key_len {
                    Some(len) => len as usize,
                    None => Err(HsmErr::InvalidKeyType)?,
                };

                // Allocate GSRAM buffer for output
                let mut hkdf_output_gsram = self.dma_alloc(
                    MAX_SHA_OUT_BUFFERSIZE + VarLenHmacShaKeyKind::VarLenHmacShaKey512.max_length(),
                )?;

                // Perform HKDF
                self.hkdf_impl(
                    secret_key_slice,
                    salt,
                    info,
                    hash_algo,
                    hkdf_output_gsram.as_ref_mut(),
                    out_len as u16,
                )?;

                // Create the key to be imported into the key vault
                let key = VarLenHmacShaKeyImported::new(
                    key_type.try_into()?,
                    key_properties.key_usage.try_into()?,
                    &hkdf_output_gsram.as_ref()[..out_len],
                )?;

                // Import the HMAC key into the key vault
                let hmac_key = self.state.vault().import_var_hmac_key(
                    self.app_vault_id(),
                    self.id(),
                    key_tag,
                    true,
                    &key,
                    key_properties.key_availability.try_into()?,
                )?;

                hmac_key.id()
            }
            _ => Err(HsmErr::InvalidKeyType)?,
        };

        // Return key id
        Ok(key_id)
    }

    /// Helper to execute HKDF operation for data blob
    /// HKDF is implemented per the standard at: https://www.rfc-editor.org/rfc/rfc5869
    ///
    /// # Notes
    ///
    /// Output needs to be GSRAM array allocated up to out_len + "digest_size_hw"
    pub(super) fn hkdf_impl(
        &self,
        secret_key_blob: &[u8],
        salt: &[u8],
        info: &[u8],
        hash_algo: DdiHashAlgorithm,
        output: &mut [u8],
        out_len: u16,
    ) -> HsmResult<SecureByteVec> {
        // Get hash output len via ShaType number values
        let hmac_hash_algo: HmacHashAlgorithm = hash_algo.try_into()?;
        let sha_mode: ShaMode = hmac_hash_algo.into();
        let sha_type: ShaType = sha_mode.into();

        let hash_buffer_len = sha_type.get_digest_size_hw();

        // Create buffer to store generated PRK
        let prk_gsram = self.dma_alloc(hash_buffer_len)?;
        let prk_mborbytearray = MborByteArray::<MAX_SHA_OUT_BUFFERSIZE>::new_with_len(
            prk_gsram.as_ref().as_ptr(),
            hash_buffer_len,
        );
        // allocate working buffer for hmac
        let input_buffer_gsram = self.dma_alloc(HKDF_MAX_INPUT_BUF_SIZE)?;
        let input_buffer_mborbytearray = MborByteArray::<HKDF_MAX_INPUT_BUF_SIZE>::new_with_len(
            input_buffer_gsram.as_ref().as_ptr(),
            HKDF_MAX_INPUT_BUF_SIZE,
        );

        // Sanity checks
        if info.len() > HKDF_MAX_INFO_SIZE {
            Err(HsmErr::HkdfInvalidInputParam)?
        }

        let hkdf_info = HkdfInfo {
            key: secret_key_blob,
            salt,
            info,
            out_len,
        };

        self.state
            .env()
            .sha()
            .hkdf(
                hkdf_info,
                sha_mode,
                &mut (&prk_mborbytearray).into(),
                &mut (&input_buffer_mborbytearray).into(),
                output,
            )
            .map_err(|_| HsmErr::HkdfError)?;

        // Return data
        Ok(output[..out_len as usize].into())
    }

    /// Helper to execute KBKDF Counter with HMAC for Secret type key
    #[allow(clippy::too_many_arguments)]
    pub(super) fn kbkdf_inner(
        &self,
        key_id: KeyId,
        label: &[u8],
        context: &[u8],
        hash_algo: DdiHashAlgorithm,
        key_type: DdiKeyType,
        key_properties: DdiKeyProperties,
        key_tag: Option<u16>,
        key_len: Option<u8>,
    ) -> HsmResult<KeyId> {
        // Extract the key blob
        // `key_id` may refer to a temporary HKDF-derived KDK used as input to KBKDF.
        // The key is scoped to this operation and deleted on completion or rollback.
        let secret_key = self
            .state
            .vault()
            .key(self.app_vault_id(), self.id(), key_id, false)?;
        let secret_key_blob = secret_key.blob()?;

        // Secret keys with 66 size are extended to 68 as they must be 4 byte aligned;
        // Only use the first 66 bytes.
        let secret_key_slice = if secret_key_blob.len() == 68 {
            &secret_key_blob[..66]
        } else {
            &secret_key_blob[..secret_key_blob.len()]
        };

        let key_kind: EntryKind = key_type.try_into().map_err(|_| HsmErr::InvalidKeyType)?;

        // Insert key into vault and get key_id
        let key_id = match key_type {
            DdiKeyType::Aes128 | DdiKeyType::Aes192 | DdiKeyType::Aes256 => {
                // Determine size of output
                let aes_kind: AesKeyKind = key_kind.try_into()?;
                let out_len = usize::from(aes_kind);

                // Allocate GSRAM buffer for output
                let mut kbkdf_output_gsram =
                    self.dma_alloc(MAX_SHA_OUT_BUFFERSIZE + MAX_AES_SIZE)?;

                // Perform KBKDF
                self.kbkdf_impl(
                    secret_key_slice,
                    label,
                    context,
                    hash_algo,
                    kbkdf_output_gsram.as_ref_mut(),
                    out_len as u16,
                )?;

                // Create the AES key to be imported into the key vault
                let key = AesKeyImported::new(
                    aes_kind,
                    key_properties.key_usage.try_into()?,
                    &kbkdf_output_gsram.as_ref()[..out_len],
                )?;

                // Import the AES key into the key vault
                let key_usage: AesKeyUsage = key_properties.key_usage.try_into()?;
                let attributes = aes_entry_attributes(
                    key_properties.key_availability.try_into()?,
                    true,
                    key_usage,
                );
                let aes_key = self.state.vault().aes_import_key(
                    self.app_vault_id(),
                    self.id(),
                    key_tag,
                    &key,
                    &attributes,
                )?;

                aes_key.id()
            }
            DdiKeyType::HmacSha256 | DdiKeyType::HmacSha384 | DdiKeyType::HmacSha512 => {
                // Determine size of output
                let out_len = key_kind.raw_key_blob_size();

                // Allocate GSRAM buffer for output
                let mut kbkdf_output_gsram =
                    self.dma_alloc(MAX_SHA_OUT_BUFFERSIZE + usize::from(HmacKeyKind::HmacKey512))?;

                // Perform KBKDF
                self.kbkdf_impl(
                    secret_key_slice,
                    label,
                    context,
                    hash_algo,
                    kbkdf_output_gsram.as_ref_mut(),
                    out_len as u16,
                )?;

                // Create the HMAC key to be imported into the key vault
                let key = HmacKeyImported::new(
                    key_type.try_into()?,
                    key_properties.key_usage.try_into()?,
                    &kbkdf_output_gsram.as_ref()[..out_len],
                )?;

                // Import the HMAC key into the key vault
                let hmac_key = self.state.vault().hmac_import_key(
                    self.app_vault_id(),
                    self.id(),
                    key_tag,
                    true,
                    &key,
                    key_properties.key_availability.try_into()?,
                )?;

                hmac_key.id()
            }
            DdiKeyType::VarLenHmacSha256
            | DdiKeyType::VarLenHmacSha384
            | DdiKeyType::VarLenHmacSha512 => {
                // Determine size of output
                let out_len = match key_len {
                    Some(len) => len as usize,
                    None => Err(HsmErr::InvalidKeyType)?,
                };

                // Allocate GSRAM buffer for output
                let mut kbkdf_output_gsram = self.dma_alloc(
                    MAX_SHA_OUT_BUFFERSIZE + VarLenHmacShaKeyKind::VarLenHmacShaKey512.max_length(),
                )?;

                // Perform KBKDF
                self.kbkdf_impl(
                    secret_key_slice,
                    label,
                    context,
                    hash_algo,
                    kbkdf_output_gsram.as_ref_mut(),
                    out_len as u16,
                )?;

                // Create the HMAC key to be imported into the key vault
                let key = VarLenHmacShaKeyImported::new(
                    key_type.try_into()?,
                    key_properties.key_usage.try_into()?,
                    &kbkdf_output_gsram.as_ref()[..out_len],
                )?;

                // Import the HMAC key into the key vault
                let hmac_key = self.state.vault().import_var_hmac_key(
                    self.app_vault_id(),
                    self.id(),
                    key_tag,
                    true,
                    &key,
                    key_properties.key_availability.try_into()?,
                )?;

                hmac_key.id()
            }
            _ => Err(HsmErr::InvalidKeyType)?,
        };

        // Return key id
        Ok(key_id)
    }

    /// Helper to execute KBKDF Counter with HMAC for data blob
    /// KBKDF is implemented per the standard at: https://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-108r1-upd1.pdf
    ///
    /// # Notes
    ///
    /// Output needs to be GSRAM array allocated up to out_len + "digest_size_hw"
    fn kbkdf_impl(
        &self,
        secret_key_blob: &[u8],
        label: &[u8],
        context: &[u8],
        hash_algo: DdiHashAlgorithm,
        output: &mut [u8],
        out_len: u16,
    ) -> HsmResult<()> {
        // Get hash output len via ShaType number values
        let hmac_hash_algo: HmacHashAlgorithm = hash_algo.try_into()?;
        let sha_mode: ShaMode = hmac_hash_algo.into();

        // allocate working buffer for hmac
        let input_buffer_gsram = self.dma_alloc(KBKDF_MAX_INPUT_BUF_SIZE)?;
        let input_buffer_mborbytearray = MborByteArray::<KBKDF_MAX_INPUT_BUF_SIZE>::new_with_len(
            input_buffer_gsram.as_ref().as_ptr(),
            KBKDF_MAX_INPUT_BUF_SIZE,
        );
        let input_buffer = &mut IoMemRange::from(&input_buffer_mborbytearray);

        // Sanity checks
        if label.len() > KBKDF_MAX_LABEL_SIZE {
            Err(HsmErr::KbkdfInvalidInputParam)?
        }

        if context.len() > KBKDF_MAX_CONTEXT_SIZE {
            Err(HsmErr::KbkdfInvalidInputParam)?
        }

        let kbkdf_info = KbkdfInfo {
            key: secret_key_blob,
            input_data: KbkdfInputData::ConcatData { label, context },
            out_len,
        };

        self.state
            .env()
            .sha()
            .kbkdf_counter_hmac(kbkdf_info, sha_mode, input_buffer, output)
            .map_err(|_| HsmErr::KbkdfError)?;

        Ok(())
    }

    /// Helper to execute HKDF for Secret type key with AES Bulk 256 key type as output
    #[allow(clippy::too_many_arguments)]
    pub(super) fn begin_hkdf_aesbulk256_derive_inner(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        salt: &[u8],
        info: &[u8],
        kdf_info: KdfInfo,
    ) -> HsmResult<AesBulk256Cmd<E>> {
        let key_id = kdf_info.key_id;
        let hash_algo = kdf_info.hash_algo;
        let key_type = kdf_info.key_type;

        if !key_type.is_bulk_key() {
            return Err(HsmErr::InvalidKeyType);
        }

        let key_properties = kdf_info.key_properties;
        let key_tag = kdf_info.key_tag;

        // Extract the key blob
        let secret_key = self.ecdh_key(key_id, Some(EcdhKeyUsage::KeyAgreement))?;
        let secret_key_blob = secret_key.blob()?;

        // Secret keys with 66 size are extended to 68 as they must be 4 byte aligned;
        // Only use the first 66 bytes.
        let secret_key_slice = if secret_key_blob.len() == 68 {
            &secret_key_blob[..66]
        } else {
            &secret_key_blob[..secret_key_blob.len()]
        };

        // Allocate GSRAM buffer for output
        let mut hkdf_output_gsram = self.dma_alloc(MAX_SHA_OUT_BUFFERSIZE + MAX_AES_SIZE)?;

        // Perform HKDF
        self.hkdf_impl(
            secret_key_slice,
            salt,
            info,
            hash_algo,
            hkdf_output_gsram.as_ref_mut(),
            MAX_AES_SIZE as u16,
        )?;

        let key_usage: AesKeyUsage = key_properties.key_usage.try_into()?;
        let attributes =
            aes_entry_attributes(key_properties.key_availability.try_into()?, true, key_usage);
        self.begin_import_der_aesbulk256_key_inner(
            tag,
            pfn,
            key_tag,
            key_type,
            key_usage,
            &attributes,
            &hkdf_output_gsram.as_ref()[..MAX_AES_SIZE],
        )
    }

    /// Helper to execute KBKDF for Secret type key with AES Bulk 256 key type as output
    pub(super) fn begin_kbkdf_aesbulk256_derive_inner(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        label: &[u8],
        context: &[u8],
        kdf_info: KdfInfo,
    ) -> HsmResult<AesBulk256Cmd<E>> {
        let key_id = kdf_info.key_id;
        let hash_algo = kdf_info.hash_algo;
        let key_type = kdf_info.key_type;

        if !key_type.is_bulk_key() {
            return Err(HsmErr::InvalidKeyType);
        }

        let key_properties = kdf_info.key_properties;
        let key_tag = kdf_info.key_tag;

        // Extract the key blob
        let secret_key = self
            .state
            .vault()
            .key(self.app_vault_id(), self.id(), key_id, false)?;
        let secret_key_blob = secret_key.blob()?;

        // Secret keys with 66 size are extended to 68 as they must be 4 byte aligned;
        // Only use the first 66 bytes.
        let secret_key_slice = if secret_key_blob.len() == 68 {
            &secret_key_blob[..66]
        } else {
            &secret_key_blob[..secret_key_blob.len()]
        };

        // Allocate GSRAM buffer for output
        let mut kbkdf_output_gsram = self.dma_alloc(MAX_SHA_OUT_BUFFERSIZE + MAX_AES_SIZE)?;

        // Perform KBKDF
        self.kbkdf_impl(
            secret_key_slice,
            label,
            context,
            hash_algo,
            kbkdf_output_gsram.as_ref_mut(),
            MAX_AES_SIZE as u16,
        )?;

        let key_usage: AesKeyUsage = key_properties.key_usage.try_into()?;
        let attributes =
            aes_entry_attributes(key_properties.key_availability.try_into()?, true, key_usage);
        self.begin_import_der_aesbulk256_key_inner(
            tag,
            pfn,
            key_tag,
            key_type,
            key_usage,
            &attributes,
            &kbkdf_output_gsram.as_ref()[..MAX_AES_SIZE],
        )
    }

    /// Helper function to receive and process the end of AesBulk256 key import
    pub(super) fn end_kdf_aesbulk256_derive_inner(&self, op: &AesBulk256Cmd<E>) -> HsmResult<()> {
        self.end_import_der_aesbulk256_key_inner(op)
    }
}

#[cfg(test)]
mod tests {
    use mcr_crypto_sha::HkdfInfo;
    use mcr_crypto_sha::KbkdfInfo;
    use mcr_crypto_sha::KbkdfInputData;
    use mcr_crypto_sha::ShaMode;
    use mcr_crypto_sha::KDF_MAX_LENGTH_MULTIPLIER;
    use mcr_ddi_types::DdiApiRev;
    use mcr_ddi_types::DdiHashAlgorithm;
    use mcr_ddi_types::DdiKeyType;
    use mcr_types::*;
    use openssl::md::Md;
    use openssl::pkey::Id;
    use openssl::pkey_ctx::HkdfMode;
    use openssl::pkey_ctx::PkeyCtx;

    use super::CmdScheduler;
    use crate::error::HsmErr;
    use crate::fsm::ComboFsm;
    use crate::mock::MockDmaAlloc;
    use crate::mock::MockDmaHeap;
    use crate::mock::MockEnv;
    use crate::mock::MockHal;
    use crate::mock::MockIpcMessageChannel;
    use crate::mock::MockPka;
    use crate::mock::MockSha;
    use crate::partition::session::app_sess::kdf::MAX_SHA_OUT_BUFFERSIZE;
    use crate::partition::PartEnv;
    use crate::partition::PartState;
    use crate::partition::ShaType;
    use crate::partition::UserSession;
    use crate::recorder::HsmFsmEventRecorder;

    fn test_hkdf_openssl(
        in_key_type: DdiKeyType,
        salt: Option<&[u8]>,
        info: Option<&[u8]>,
        hash_algo: DdiHashAlgorithm,
        target_key_type: DdiKeyType,
    ) {
        // Initialize parameters
        let openssl_algo = match hash_algo {
            DdiHashAlgorithm::Sha1 => Ok(Md::sha1()),
            DdiHashAlgorithm::Sha256 => Ok(Md::sha256()),
            DdiHashAlgorithm::Sha384 => Ok(Md::sha384()),
            DdiHashAlgorithm::Sha512 => Ok(Md::sha512()),
            _ => Err(HsmErr::InvalidArgument),
        };
        let in_len: usize = match in_key_type {
            DdiKeyType::Secret256 => 32,
            DdiKeyType::Secret384 => 48,
            DdiKeyType::Secret521 => 68,
            _ => panic!(),
        };
        let out_len: usize = match target_key_type {
            DdiKeyType::Aes128 => 16,
            DdiKeyType::Aes192 => 24,
            DdiKeyType::Aes256
            | DdiKeyType::Secret256
            | DdiKeyType::AesXtsBulk256
            | DdiKeyType::AesGcmBulk256
            | DdiKeyType::AesGcmBulk256Unapproved => 32,
            DdiKeyType::Secret384 => 48,
            DdiKeyType::Secret521 => 68,
            _ => panic!(),
        };

        // Generate secret key data
        let mut rand_bytes = [0u8; 128];
        assert!(openssl::rand::rand_bytes(&mut rand_bytes).is_ok());

        // Call openssl hkdf
        let mut ctx = PkeyCtx::new_id(Id::HKDF).unwrap();
        assert!(ctx.derive_init().is_ok());
        assert!(ctx.set_hkdf_key(&rand_bytes[..in_len]).is_ok());
        assert!(ctx
            .set_hkdf_md(openssl_algo.expect("Failed to get OpenSSL algorithm"))
            .is_ok());
        if let Some(salt) = salt {
            assert!(ctx.set_hkdf_salt(salt).is_ok());
        }

        if let Some(info) = info {
            assert!(ctx.add_hkdf_info(info).is_ok());
        }
        assert!(ctx.set_hkdf_mode(HkdfMode::EXTRACT_THEN_EXPAND).is_ok());

        let mut openssl_out_vec = vec![0u8; out_len];
        assert!(ctx.derive(Some(&mut openssl_out_vec)).is_ok());

        // Intialize session object
        let mut pka = MockPka::new();
        pka.expect_clone().times(1).returning(MockPka::new);

        const TOTAL_TABLE_LEN: usize = 17 * 1024;
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

        let mut hal = MockHal::new();
        hal.expect_pka().once().return_const(vec![pka]);
        hal.expect_vault_addr()
            .return_const(table_memory.as_ptr() as usize);
        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
        mock_ipc_message_channel
            .expect_clone()
            .once()
            .returning(MockIpcMessageChannel::new);

        hal.expect_hsm_to_fp_ipc_channel()
            .once()
            .return_const(mock_ipc_message_channel);

        let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
        mock_ipc_message_channel
            .expect_clone()
            .once()
            .returning(MockIpcMessageChannel::new);

        hal.expect_hsp_ipc_channel()
            .once()
            .return_const(mock_ipc_message_channel);

        let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
        mock_ipc_message_channel
            .expect_clone()
            .once()
            .returning(MockIpcMessageChannel::new);

        hal.expect_hsm_to_admin_ipc_channel()
            .once()
            .return_const(mock_ipc_message_channel);

        let mut sha = MockSha::new();
        sha.expect_hkdf().times(1).returning(
            move |hkdf_info: HkdfInfo, sha_mode: ShaMode, _, _, output: &mut [u8]| {
                // Call openssl hkdf with the given function parameters
                let sha_algo = match sha_mode {
                    ShaMode::Sha1 => Md::sha1(),
                    ShaMode::Sha256 => Md::sha256(),
                    ShaMode::Sha384 => Md::sha384(),
                    ShaMode::Sha512 => Md::sha512(),
                };

                let hash_len = ShaType::from(sha_mode) as usize;
                let hash_buffer_len = ShaType::from(sha_mode).get_digest_size_hw();

                if hash_len > SHA_DIGEST_MAX_SIZE_BYTES {
                    // HkdfSanityCheckFailed = 0xb,
                    Err(0xb_u32)?
                }

                if hkdf_info.out_len > (KDF_MAX_LENGTH_MULTIPLIER * hash_len) as u16 {
                    // HkdfSanityCheckFailed = 0xb,
                    Err(0xb_u32)?
                }

                if output.len() < hash_buffer_len + hkdf_info.out_len as usize {
                    // HkdfSanityCheckFailed = 0xb,
                    Err(0xb_u32)?
                }

                let mut ctx = PkeyCtx::new_id(Id::HKDF).unwrap();
                assert!(ctx.derive_init().is_ok());
                assert!(ctx.set_hkdf_key(hkdf_info.key).is_ok());
                assert!(ctx.set_hkdf_md(sha_algo).is_ok());
                assert!(ctx.set_hkdf_salt(hkdf_info.salt).is_ok());
                assert!(ctx.add_hkdf_info(hkdf_info.info).is_ok());
                assert!(ctx.set_hkdf_mode(HkdfMode::EXTRACT_THEN_EXPAND).is_ok());

                let mut openssl_out_vec = vec![0u8; hkdf_info.out_len as usize];
                assert!(ctx.derive(Some(&mut openssl_out_vec)).is_ok());

                let expected_out_len = hkdf_info.out_len as usize;
                output[..expected_out_len].copy_from_slice(&openssl_out_vec[..expected_out_len]);
                Ok(())
            },
        );
        hal.expect_sha().times(1).return_const(sha);

        let mut heap = MockDmaHeap::new();
        heap.expect_allocate()
            .times(2)
            .returning(|s| Some(MockDmaAlloc::new(s)));
        hal.expect_dma_heap().times(2).return_const(heap);

        hal.expect_clone().once().returning(move || {
            let mut hal = MockHal::new();

            let part_persistent_store_memory = [0u8; 2048 * 65];
            hal.expect_part_persistent_store_addr()
                .return_const(part_persistent_store_memory.as_ptr() as usize);

            hal
        });

        let cmd_scheduler =
            CmdScheduler::<ComboFsm<MockEnv>>::new(128, 1, HsmFsmEventRecorder::default());

        let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler);
        let state = PartState::new(PcieFunction(0), env);

        let app_session = UserSession::new(DdiApiRev { major: 1, minor: 0 }, 10, state);

        // Call app_session hkdf
        let salt = match salt {
            None => Vec::new(),
            Some(salt_inner) => salt_inner.to_vec(),
        };
        let info = match info {
            None => Vec::new(),
            Some(info_inner) => info_inner.to_vec(),
        };
        let mut mcr_out = [0u8; KDF_MAX_LENGTH_MULTIPLIER * 64 + MAX_SHA_OUT_BUFFERSIZE];
        let mcr_out_vec = app_session
            .hkdf_impl(
                &rand_bytes[..in_len],
                &salt,
                &info,
                hash_algo,
                &mut mcr_out,
                out_len as u16,
            )
            .unwrap();

        // Compare outputs
        assert!(mcr_out_vec == openssl_out_vec.into());
    }

    #[test]
    fn test_hkdf_sha1() {
        let mut salt = [0u8; 64];
        assert!(openssl::rand::rand_bytes(&mut salt).is_ok());
        let mut info = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut info).is_ok());

        test_hkdf_openssl(
            DdiKeyType::Secret256,
            Some(&salt),
            Some(&info),
            DdiHashAlgorithm::Sha1,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_hkdf_sha256_none() {
        test_hkdf_openssl(
            DdiKeyType::Secret384,
            None,
            None,
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_hkdf_sha256_salt() {
        let mut salt = [0u8; 64];
        assert!(openssl::rand::rand_bytes(&mut salt).is_ok());

        test_hkdf_openssl(
            DdiKeyType::Secret521,
            Some(&salt),
            None,
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_hkdf_sha256_info() {
        let mut info = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut info).is_ok());

        test_hkdf_openssl(
            DdiKeyType::Secret256,
            None,
            Some(&info),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_hkdf_sha256() {
        let mut salt = [0u8; 64];
        assert!(openssl::rand::rand_bytes(&mut salt).is_ok());
        let mut info = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut info).is_ok());

        test_hkdf_openssl(
            DdiKeyType::Secret384,
            Some(&salt),
            Some(&info),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_hkdf_sha384_none() {
        test_hkdf_openssl(
            DdiKeyType::Secret521,
            None,
            None,
            DdiHashAlgorithm::Sha384,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_hkdf_sha384() {
        let mut salt = [0u8; 64];
        assert!(openssl::rand::rand_bytes(&mut salt).is_ok());
        let mut info = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut info).is_ok());

        test_hkdf_openssl(
            DdiKeyType::Secret256,
            Some(&salt),
            Some(&info),
            DdiHashAlgorithm::Sha384,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_hkdf_sha512_none() {
        test_hkdf_openssl(
            DdiKeyType::Secret384,
            None,
            None,
            DdiHashAlgorithm::Sha512,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_hkdf_sha512() {
        let mut salt = [0u8; 64];
        assert!(openssl::rand::rand_bytes(&mut salt).is_ok());
        let mut info = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut info).is_ok());

        test_hkdf_openssl(
            DdiKeyType::Secret521,
            Some(&salt),
            Some(&info),
            DdiHashAlgorithm::Sha512,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_hkdf_sha256_secret384() {
        let mut salt = [0u8; 64];
        assert!(openssl::rand::rand_bytes(&mut salt).is_ok());
        let mut info = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut info).is_ok());

        test_hkdf_openssl(
            DdiKeyType::Secret256,
            Some(&salt),
            Some(&info),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Secret521,
        );
    }

    #[test]
    fn test_hkdf_sha256_secret521() {
        let mut salt = [0u8; 64];
        assert!(openssl::rand::rand_bytes(&mut salt).is_ok());
        let mut info = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut info).is_ok());

        test_hkdf_openssl(
            DdiKeyType::Secret384,
            Some(&salt),
            Some(&info),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Secret521,
        );
    }

    #[test]
    fn test_hkdf_sha256_aes128() {
        let mut salt = [0u8; 64];
        assert!(openssl::rand::rand_bytes(&mut salt).is_ok());
        let mut info = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut info).is_ok());

        test_hkdf_openssl(
            DdiKeyType::Secret521,
            Some(&salt),
            Some(&info),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Aes128,
        );
    }

    #[test]
    fn test_hkdf_sha256_aes192() {
        let mut salt = [0u8; 64];
        assert!(openssl::rand::rand_bytes(&mut salt).is_ok());
        let mut info = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut info).is_ok());

        test_hkdf_openssl(
            DdiKeyType::Secret256,
            Some(&salt),
            Some(&info),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Aes192,
        );
    }

    #[test]
    fn test_hkdf_sha256_aes256() {
        let mut salt = [0u8; 64];
        assert!(openssl::rand::rand_bytes(&mut salt).is_ok());
        let mut info = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut info).is_ok());

        test_hkdf_openssl(
            DdiKeyType::Secret256,
            Some(&salt),
            Some(&info),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::AesXtsBulk256,
        );
    }

    fn test_kbkdf(
        in_key_type: DdiKeyType,
        label: Option<&[u8]>,
        context: Option<&[u8]>,
        hash_algo: DdiHashAlgorithm,
        target_key_type: DdiKeyType,
    ) {
        // Initialize parameters
        let in_len: usize = match in_key_type {
            DdiKeyType::Secret256 => 32,
            DdiKeyType::Secret384 => 48,
            DdiKeyType::Secret521 => 68,
            _ => panic!(),
        };
        let out_len: usize = match target_key_type {
            DdiKeyType::Aes128 => 16,
            DdiKeyType::Aes192 => 24,
            DdiKeyType::Aes256
            | DdiKeyType::Secret256
            | DdiKeyType::AesXtsBulk256
            | DdiKeyType::AesGcmBulk256
            | DdiKeyType::AesGcmBulk256Unapproved => 32,
            DdiKeyType::Secret384 => 48,
            DdiKeyType::Secret521 => 68,
            _ => panic!(),
        };

        let label = match label {
            None => Vec::new(),
            Some(label_inner) => label_inner.to_vec(),
        };
        let context = match context {
            None => Vec::new(),
            Some(context_inner) => context_inner.to_vec(),
        };

        // Clone for use in mock
        let label_input = label.clone();
        let context_input = context.clone();

        // Generate secret key data
        let mut rand_bytes = [0u8; 128];
        assert!(openssl::rand::rand_bytes(&mut rand_bytes).is_ok());

        let openssl_out_vec = vec![0u8; out_len];

        // Intialize session object
        let mut pka = MockPka::new();
        pka.expect_clone().times(1).returning(MockPka::new);

        const TOTAL_TABLE_LEN: usize = 17 * 1024;
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

        let mut hal = MockHal::new();
        hal.expect_pka().once().return_const(vec![pka]);
        hal.expect_vault_addr()
            .return_const(table_memory.as_ptr() as usize);
        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
        mock_ipc_message_channel
            .expect_clone()
            .once()
            .returning(MockIpcMessageChannel::new);

        hal.expect_hsm_to_fp_ipc_channel()
            .once()
            .return_const(mock_ipc_message_channel);

        let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
        mock_ipc_message_channel
            .expect_clone()
            .once()
            .returning(MockIpcMessageChannel::new);

        hal.expect_hsp_ipc_channel()
            .once()
            .return_const(mock_ipc_message_channel);

        let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
        mock_ipc_message_channel
            .expect_clone()
            .once()
            .returning(MockIpcMessageChannel::new);

        hal.expect_hsm_to_admin_ipc_channel()
            .once()
            .return_const(mock_ipc_message_channel);

        let mut sha = MockSha::new();
        sha.expect_kbkdf_counter_hmac().times(1).returning(
            move |kbkdf_info: KbkdfInfo, sha_mode: ShaMode, _, output: &mut [u8]| {
                // As we have removed openssl kdf crate due to S360 issues, we no longer can compare
                // using the openssl_kdf::kdf::derive function.
                // Instead, we will compare the inputs which should not be modified when the flow reaches
                // here.

                let (label, context) = match kbkdf_info.input_data {
                    KbkdfInputData::SelfTestData { .. } => Err(0xe_u32)?,
                    KbkdfInputData::ConcatData { label, context } => (label, context),
                };

                let hash_len = ShaType::from(sha_mode) as usize;
                let hash_buffer_len = ShaType::from(sha_mode).get_digest_size_hw();

                // Sanity checks
                if hash_len > SHA_DIGEST_MAX_SIZE_BYTES || hash_len == 0 {
                    // KbkdfSanityCheckFailed = 0xe,
                    Err(0xe_u32)?
                }

                if kbkdf_info.out_len > (KDF_MAX_LENGTH_MULTIPLIER * hash_len) as u16 {
                    // KbkdfSanityCheckFailed = 0xe,
                    Err(0xe_u32)?
                }

                if output.len() < hash_buffer_len + kbkdf_info.out_len as usize {
                    // KbkdfSanityCheckFailed = 0xe,
                    Err(0xe_u32)?
                }

                assert_eq!(label_input, label);
                assert_eq!(context_input, context);
                assert_eq!(kbkdf_info.key, &rand_bytes[..in_len]);

                let out_vec = vec![0u8; kbkdf_info.out_len as usize];

                let expected_out_len = kbkdf_info.out_len as usize;
                output[..expected_out_len].copy_from_slice(&out_vec[..expected_out_len]);

                Ok(())
            },
        );
        hal.expect_sha().times(1).return_const(sha);

        let mut heap = MockDmaHeap::new();
        heap.expect_allocate()
            .times(1)
            .returning(|s| Some(MockDmaAlloc::new(s)));
        hal.expect_dma_heap().times(1).return_const(heap);

        hal.expect_clone().once().returning(move || {
            let mut hal = MockHal::new();

            let part_persistent_store_memory = [0u8; 2048 * 65];
            hal.expect_part_persistent_store_addr()
                .return_const(part_persistent_store_memory.as_ptr() as usize);

            hal
        });

        let cmd_scheduler =
            CmdScheduler::<ComboFsm<MockEnv>>::new(128, 1, HsmFsmEventRecorder::default());
        let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler);
        let state = PartState::new(PcieFunction(0), env);

        let app_session = UserSession::new(DdiApiRev { major: 1, minor: 0 }, 10, state);

        // Call app_session kbkdf
        let mut mcr_out = [0u8; KDF_MAX_LENGTH_MULTIPLIER * 64 + MAX_SHA_OUT_BUFFERSIZE];
        let kbkdf_result = app_session.kbkdf_impl(
            &rand_bytes[..in_len],
            &label,
            &context,
            hash_algo,
            &mut mcr_out,
            out_len as u16,
        );
        assert!(kbkdf_result.is_ok());

        // Compare outputs
        assert_eq!(openssl_out_vec, mcr_out[..out_len])
    }

    #[test]
    fn test_kbkdf_sha1() {
        let mut label = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut label).is_ok());
        let mut context = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut context).is_ok());

        test_kbkdf(
            DdiKeyType::Secret256,
            Some(&label),
            Some(&context),
            DdiHashAlgorithm::Sha1,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_kbkdf_sha256_none() {
        test_kbkdf(
            DdiKeyType::Secret384,
            None,
            None,
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_kbkdf_sha256_label() {
        let mut label = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut label).is_ok());

        test_kbkdf(
            DdiKeyType::Secret521,
            Some(&label),
            None,
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_kbkdf_sha256_context() {
        let mut context = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut context).is_ok());

        test_kbkdf(
            DdiKeyType::Secret256,
            None,
            Some(&context),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_kbkdf_sha256() {
        let mut label = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut label).is_ok());
        let mut context = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut context).is_ok());

        test_kbkdf(
            DdiKeyType::Secret384,
            Some(&label),
            Some(&context),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_kbkdf_sha384() {
        let mut label = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut label).is_ok());
        let mut context = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut context).is_ok());

        test_kbkdf(
            DdiKeyType::Secret521,
            Some(&label),
            Some(&context),
            DdiHashAlgorithm::Sha384,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_kbkdf_sha512() {
        let mut label = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut label).is_ok());
        let mut context = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut context).is_ok());

        test_kbkdf(
            DdiKeyType::Secret256,
            Some(&label),
            Some(&context),
            DdiHashAlgorithm::Sha512,
            DdiKeyType::Secret256,
        );
    }

    #[test]
    fn test_kbkdf_sha256_secret384() {
        let mut label = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut label).is_ok());
        let mut context = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut context).is_ok());

        test_kbkdf(
            DdiKeyType::Secret521,
            Some(&label),
            Some(&context),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Secret384,
        );
    }

    #[test]
    fn test_kbkdf_sha256_secret521() {
        let mut label = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut label).is_ok());
        let mut context = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut context).is_ok());

        test_kbkdf(
            DdiKeyType::Secret256,
            Some(&label),
            Some(&context),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Secret521,
        );
    }

    #[test]
    fn test_kbkdf_sha256_aes128() {
        let mut label = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut label).is_ok());
        let mut context = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut context).is_ok());

        test_kbkdf(
            DdiKeyType::Secret384,
            Some(&label),
            Some(&context),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Aes128,
        );
    }

    #[test]
    fn test_kbkdf_sha256_aes192() {
        let mut label = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut label).is_ok());
        let mut context = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut context).is_ok());

        test_kbkdf(
            DdiKeyType::Secret521,
            Some(&label),
            Some(&context),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::Aes128,
        );
    }

    #[test]
    fn test_kbkdf_sha256_aesbulk256() {
        let mut label = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut label).is_ok());
        let mut context = [0u8; 16];
        assert!(openssl::rand::rand_bytes(&mut context).is_ok());

        test_kbkdf(
            DdiKeyType::Secret521,
            Some(&label),
            Some(&context),
            DdiHashAlgorithm::Sha256,
            DdiKeyType::AesGcmBulk256,
        );
    }
}
