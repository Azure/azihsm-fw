// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_types::MaskedKey;
use mcr_ddi_types::MaskedKeyError;
use mcr_ddi_types::MaskingKeyAlgorithm;
use mcr_ddi_types::PreEncodeMaskedKeyType;
use mcr_types::IoMemRange;

use crate::crypto_env::CryptEnv;

use super::*;

/// A trait for encoding a `MaskedKey` into a raw byte representation.
pub trait MaskedKeyEncode<Env: CryptEnv> {
    /// Encodes a `MaskedKey` into a byte slice.
    ///
    /// # Arguments
    /// * `env`: The cryptographic environment used for encoding.
    /// * `pre_encoded_key`: A mutable reference to the pre-encoded masked key structure.
    /// * `plaintext_key`: The key to be masked.
    /// * `masking_key`: The key to be used for encrypting the plaintext key.
    /// * `metadata`: Metadata associated with the masked key.
    ///
    /// # Returns
    /// * `Result<(), MaskedKeyError>` - Ok if encoding is successful, or an error if it fails.
    fn encode(
        env: &Env,
        pre_encoded_key: &mut PreEncodeMaskedKeyType<'_>,
        plaintext_key: &[u8],
        masking_key: &[u8],
        metadata: &[u8],
    ) -> Result<(), MaskedKeyError>;
}

impl<Env: CryptEnv> MaskedKeyEncode<Env> for MaskedKey<'_> {
    fn encode(
        env: &Env,
        pre_encoded_key: &mut PreEncodeMaskedKeyType<'_>,
        plaintext_key: &[u8],
        masking_key: &[u8],
        metadata: &[u8],
    ) -> Result<(), MaskedKeyError> {
        match pre_encoded_key.algo() {
            MaskingKeyAlgorithm::AesCbc256Hmac384 => {
                encode_aescbc256(env, pre_encoded_key, plaintext_key, masking_key, metadata)?;
            }
            _ => Err(MaskedKeyError::InvalidMaskingKeyAlgorithm)?,
        }

        Ok(())
    }
}

/// Encodes a masked key using AES-CBC-256 with HMAC-384.
fn encode_aescbc256<Env: CryptEnv>(
    env: &Env,
    pre_encoded_key: &mut PreEncodeMaskedKeyType<'_>,
    plaintext_key: &[u8],
    masking_key: &[u8],
    metadata: &[u8],
) -> Result<(), MaskedKeyError> {
    // Extract the AES-specific key for type-safe operations
    let PreEncodeMaskedKeyType::Aes(aes_key) = pre_encoded_key;

    // Encrypt the plaintext key using AES-CBC-256.
    let (aes_key_bytes, hmac_key) = split_aes_hmac_key(masking_key)?;

    let mut iv: IoMemRange = aes_key.iv().into();
    env.generate_random(iv.slice_mut())
        .map_err(|_| MaskedKeyError::IvGenerationFailed)?;

    env.aescbc256_encrypt(
        aes_key_bytes,
        plaintext_key,
        iv.slice_mut(),
        aes_key.encrypted_key_mut(),
    )
    .map_err(|_| MaskedKeyError::AesEncryptionFailed)?;

    // Copy the metadata.
    let metadata_mut = aes_key.metadata_mut();

    if metadata.len() != metadata_mut.len() {
        Err(MaskedKeyError::MetadataEncodeFailed)?;
    }

    metadata_mut.copy_from_slice(metadata);

    // Lastly, generate the HMAC tag for the masked key structure.
    let data_to_tag = aes_key.tagged_data();
    let tag = env
        .hmac384_tag(hmac_key, data_to_tag)
        .map_err(|_| MaskedKeyError::HmacTagGenerationFailed)?;
    aes_key.tag_mut().copy_from_slice(tag.as_slice());

    Ok(())
}

#[cfg(test)]
mod tests {
    use mcr_ddi_mbor::MborByteArray;
    use mcr_ddi_mbor::MborEncode;
    use mcr_ddi_mbor::MborEncoder;
    use mcr_ddi_mbor::MborLen;
    use mcr_ddi_mbor::MborLenAccumulator;
    use mcr_ddi_types::DdiKeyType;
    use mcr_ddi_types::DdiMaskedKeyAttributes;
    use mcr_ddi_types::DdiMaskedKeyMetadata;
    use mcr_ddi_types::AES_CBC_IV_SIZE;
    use mcr_ddi_types::AES_CBC_TAG_SIZE;
    use mcr_ddi_types::AES_GCM_IV_SIZE;
    use mcr_ddi_types::AES_GCM_TAG_SIZE;
    use mcr_types::SecureByteArray;

    use crate::error::HsmErr;
    use crate::partition::EntryKind;
    use sha2::{Digest, Sha384};
    use zerocopy::IntoBytes;

    use crate::partition::store::EntryAttributes;

    use super::*;

    #[allow(dead_code)]
    pub struct TestCryptoEnv {
        pub plaintext: Vec<u8>,
        pub ciphertext: Vec<u8>,
        pub hmac384_tag: [u8; 48],
        pub error: Option<HsmErr>,
    }

    impl TestCryptoEnv {
        pub fn new() -> Self {
            TestCryptoEnv {
                plaintext: Vec::new(),
                ciphertext: Vec::new(),
                hmac384_tag: [0u8; 48],
                error: None,
            }
        }
    }
    impl Default for TestCryptoEnv {
        fn default() -> Self {
            TestCryptoEnv::new()
        }
    }

    impl CryptEnv for TestCryptoEnv {
        fn hmac384_tag(&self, _key: &[u8], _data: &[u8]) -> Result<SecureByteArray<48>, HsmErr> {
            Ok(SecureByteArray::new(self.hmac384_tag))
        }

        fn aescbc256_enc_data_len(&self, plaintext_len: usize) -> usize {
            plaintext_len
        }

        fn aescbc256_encrypt(
            &self,
            _key: &[u8],
            _plaintext: &[u8],
            _iv: &mut [u8],
            ciphertext: &mut [u8],
        ) -> Result<usize, HsmErr> {
            Ok(ciphertext.len())
        }

        fn aescbc256_decrypt(
            &self,
            _key: &[u8],
            _iv: &[u8],
            _ciphertext: &[u8],
            plaintext: &mut [u8],
        ) -> Result<usize, HsmErr> {
            if let Some(err) = self.error {
                return Err(err);
            }
            plaintext.copy_from_slice(self.plaintext.as_slice());
            Ok(plaintext.len())
        }

        fn kbkdf_sha384(
            &self,
            key: &[u8],
            label: Option<&[u8]>,
            context: Option<&[u8]>,
            out_len: usize,
            output: &mut [u8],
        ) -> Result<(), HsmErr> {
            // Simple KBKDF implementation for testing: fill output with a repeated pattern
            // This is NOT cryptographically secure and is only for test purposes.
            let mut ctr = 1u32;
            let mut written = 0;
            while written < out_len {
                // Compose: [ctr (4 bytes)] + key + label + context
                let mut block = Vec::new();
                block.extend_from_slice(&ctr.to_be_bytes());
                block.extend_from_slice(key);
                if let Some(l) = label {
                    block.extend_from_slice(l);
                }
                if let Some(c) = context {
                    block.extend_from_slice(c);
                }
                // Hash the block (use SHA-384 for test)
                let hash = Sha384::digest(&block);
                let to_copy = std::cmp::min(hash.len(), out_len - written);
                output[written..written + to_copy].copy_from_slice(&hash[..to_copy]);
                written += to_copy;
                ctr += 1;
            }
            Ok(())
        }

        fn generate_random(&self, output: &mut [u8]) -> Result<(), HsmErr> {
            // Simple custom random number generator using a linear congruential generator (LCG)
            // Note: This is NOT cryptographically secure - use only for testing/development

            // Global state for the RNG
            static mut SEED: u64 = 0x123456789ABCDEF0;

            unsafe {
                for byte in output.iter_mut() {
                    // LCG formula: next = (a * current + c) mod m
                    // Using constants from Numerical Recipes
                    SEED = SEED.wrapping_mul(1664525).wrapping_add(1013904223);
                    *byte = (SEED >> 24) as u8; // Use upper bits for better randomness
                }
            }

            Ok(())
        }
    }

    fn encode_metadata(metadata: &DdiMaskedKeyMetadata) -> Vec<u8> {
        // Get mbor encoded length for metadata
        let mut accumulator = MborLenAccumulator::default();
        metadata.mbor_len(&mut accumulator);
        let metadata_len = accumulator.len();

        // Mbor encode metadata
        let mut encoded_metadata = vec![0u8; metadata_len];

        let mut encoder = MborEncoder::new(&mut encoded_metadata);
        metadata.mbor_encode(&mut encoder).unwrap();
        encoded_metadata
    }

    #[test]
    fn test_get_encoded_length_aes_cbc256_hmac384() {
        let key_label = b"DummyOne";
        let mut entry_attributes = EntryAttributes::default();
        entry_attributes.common.flags.set_sign(true);
        entry_attributes.common.flags.set_verify(true);
        entry_attributes.common.flags.set_session(true);
        let metadata = DdiMaskedKeyMetadata {
            svn: Some(1),
            key_type: DdiKeyType::Ecc256Private,
            key_attributes: DdiMaskedKeyAttributes {
                blob: entry_attributes
                    .as_bytes()
                    .try_into()
                    .expect("Fail to convert to [u8;32]"),
            },
            bks2_index: None,
            key_tag: None,
            key_label: MborByteArray::new_with_len(key_label.as_ptr(), key_label.len()),
            key_length: EntryKind::try_from(DdiKeyType::Ecc256Private)
                .map(|entry| entry.raw_key_blob_size() as u16)
                .unwrap(),
        };
        let encoded_metadata = encode_metadata(&metadata);
        let encoded_length = MaskedKey::encoded_length(
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            encoded_metadata.len(),
            32,
        );
        assert!(encoded_length > 0);
    }

    #[test]
    fn test_get_encoded_length_aes_gcm256() {
        let key_label = b"DummyTwo";
        let mut entry_attributes = EntryAttributes::default();
        entry_attributes.common.flags.set_sign(true);
        entry_attributes.common.flags.set_verify(true);
        entry_attributes.common.flags.set_session(true);
        let metadata = DdiMaskedKeyMetadata {
            svn: Some(1),
            key_type: DdiKeyType::Ecc256Private,
            key_attributes: DdiMaskedKeyAttributes {
                blob: entry_attributes
                    .as_bytes()
                    .try_into()
                    .expect("Fail to convert to [u8;32]"),
            },
            bks2_index: None,
            key_tag: None,
            key_label: MborByteArray::new_with_len(key_label.as_ptr(), key_label.len()),
            key_length: EntryKind::try_from(DdiKeyType::Ecc256Private)
                .map(|entry| entry.raw_key_blob_size() as u16)
                .unwrap(),
        };
        let encoded_metadata = encode_metadata(&metadata);
        let encoded_length = MaskedKey::encoded_length(
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            encoded_metadata.len(),
            32,
        );
        assert!(encoded_length > 0);
    }

    #[test]
    fn test_pre_encode_aes_cbc256_hmac384() {
        let env = TestCryptoEnv::new();
        let key_label = b"DummyThree";
        let mut entry_attributes = EntryAttributes::default();
        entry_attributes.common.flags.set_sign(true);
        entry_attributes.common.flags.set_verify(true);
        entry_attributes.common.flags.set_session(true);
        let metadata = DdiMaskedKeyMetadata {
            svn: Some(1),
            key_type: DdiKeyType::Ecc256Private,
            key_attributes: DdiMaskedKeyAttributes {
                blob: entry_attributes
                    .as_bytes()
                    .try_into()
                    .expect("Fail to convert to [u8;32]"),
            },
            bks2_index: None,
            key_tag: None,
            key_label: MborByteArray::new_with_len(key_label.as_ptr(), key_label.len()),
            key_length: EntryKind::try_from(DdiKeyType::Ecc256Private)
                .map(|entry| entry.raw_key_blob_size() as u16)
                .unwrap(),
        };

        let encoded_metadata = encode_metadata(&metadata);

        // Get the encoded length for the masked key.
        let plaintext_key_len = 32;
        let encrypted_key_len = env.aescbc256_enc_data_len(plaintext_key_len);
        let encoded_length = MaskedKey::encoded_length(
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            encoded_metadata.len(),
            encrypted_key_len,
        );
        assert!(encoded_length > 0);
        assert!(encoded_length % 4 == 0);

        // Create a buffer of the required length.
        let mut buffer = vec![0u8; encoded_length];
        let result = MaskedKey::pre_encode(
            1,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            encoded_metadata.len(),
            encrypted_key_len,
            &mut buffer,
        );
        assert!(result.is_ok());
        let mut pre_encoded = result.unwrap();

        // Extract the AES key for testing
        let PreEncodeMaskedKeyType::Aes(aes_key) = &mut pre_encoded;

        assert_eq!(aes_key.iv().len(), AES_CBC_IV_SIZE);
        assert_eq!(aes_key.encrypted_key().len(), encrypted_key_len);
        assert_eq!(aes_key.metadata().len(), 79);
        assert_eq!(aes_key.tag().len(), AES_CBC_TAG_SIZE);

        aes_key.iv_mut().fill(0xAA);
        assert_eq!(aes_key.iv(), vec![0xAA; AES_CBC_IV_SIZE]);

        aes_key.encrypted_key_mut().fill(0xBB);
        assert_eq!(
            aes_key.encrypted_key(),
            vec![0xBB; aes_key.encrypted_key().len()]
        );

        aes_key.metadata_mut().fill(0xCC);
        assert_eq!(aes_key.metadata(), vec![0xCC; aes_key.metadata().len()]);

        aes_key.tag_mut().fill(0xDD);
        assert_eq!(aes_key.tag(), vec![0xDD; AES_CBC_TAG_SIZE]);
    }

    #[test]
    fn test_pre_encode_aes_gcm256() {
        let env = TestCryptoEnv::new();
        let key_label = b"DummyFour";
        let mut entry_attributes = EntryAttributes::default();
        entry_attributes.common.flags.set_sign(true);
        entry_attributes.common.flags.set_verify(true);
        entry_attributes.common.flags.set_session(true);
        let metadata = DdiMaskedKeyMetadata {
            svn: Some(1),
            key_type: DdiKeyType::Ecc256Private,
            key_attributes: DdiMaskedKeyAttributes {
                blob: entry_attributes
                    .as_bytes()
                    .try_into()
                    .expect("Fail to convert to [u8;32]"),
            },
            bks2_index: None,
            key_tag: None,
            key_label: MborByteArray::new_with_len(key_label.as_ptr(), key_label.len()),
            key_length: EntryKind::try_from(DdiKeyType::Ecc256Private)
                .map(|entry| entry.raw_key_blob_size() as u16)
                .unwrap(),
        };

        let encoded_metadata = encode_metadata(&metadata);

        // Get the encoded length for the masked key.
        let plaintext_key_len = 32;
        let encrypted_key_len = env.aescbc256_enc_data_len(plaintext_key_len);
        let encoded_length = MaskedKey::encoded_length(
            MaskingKeyAlgorithm::AesGcm256,
            encoded_metadata.len(),
            encrypted_key_len,
        );
        assert!(encoded_length > 0);
        assert!(encoded_length % 4 == 0);

        // Create a buffer of the required length.
        let mut buffer = vec![0u8; encoded_length];
        let result = MaskedKey::pre_encode(
            1,
            MaskingKeyAlgorithm::AesGcm256,
            encoded_metadata.len(),
            encrypted_key_len,
            &mut buffer,
        );
        assert!(result.is_ok());
        let mut pre_encoded = result.unwrap();

        // Extract the AES key for testing
        let PreEncodeMaskedKeyType::Aes(aes_key) = &mut pre_encoded;

        assert_eq!(aes_key.iv().len(), AES_GCM_IV_SIZE);
        assert_eq!(aes_key.encrypted_key().len(), plaintext_key_len);
        assert_eq!(aes_key.metadata().len(), 78);
        assert_eq!(aes_key.tag().len(), AES_GCM_TAG_SIZE);

        aes_key.iv_mut().fill(0xAA);
        assert_eq!(aes_key.iv(), vec![0xAA; AES_GCM_IV_SIZE]);

        aes_key.encrypted_key_mut().fill(0xBB);
        assert_eq!(aes_key.encrypted_key(), vec![0xBB; plaintext_key_len]);

        aes_key.metadata_mut().fill(0xCC);
        assert_eq!(aes_key.metadata(), vec![0xCC; aes_key.metadata().len()]);

        aes_key.tag_mut().fill(0xDD);
        assert_eq!(aes_key.tag(), vec![0xDD; AES_GCM_TAG_SIZE]);
    }

    #[test]
    fn test_encode_decode_aes_cbc256_hmac384() {
        let mut env = TestCryptoEnv::new();
        let key_label = b"DummyFive";
        let mut entry_attributes = EntryAttributes::default();
        entry_attributes.common.flags.set_sign(true);
        entry_attributes.common.flags.set_verify(true);
        entry_attributes.common.flags.set_session(true);
        let metadata = DdiMaskedKeyMetadata {
            svn: Some(1),
            key_type: DdiKeyType::Ecc256Private,
            key_attributes: DdiMaskedKeyAttributes {
                blob: entry_attributes
                    .as_bytes()
                    .try_into()
                    .expect("Fail to convert to [u8;32]"),
            },
            bks2_index: None,
            key_tag: None,
            key_label: MborByteArray::new_with_len(key_label.as_ptr(), key_label.len()),
            key_length: EntryKind::try_from(DdiKeyType::Ecc256Private)
                .map(|entry| entry.raw_key_blob_size() as u16)
                .unwrap(),
        };

        let encoded_metadata = encode_metadata(&metadata);

        let plaintext_key_len = 32;
        let encrypted_key_len = env.aescbc256_enc_data_len(plaintext_key_len);
        let encoded_length = MaskedKey::encoded_length(
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            encoded_metadata.len(),
            encrypted_key_len,
        );
        assert!(encoded_length % 4 == 0);
        let mut buffer = vec![0u8; encoded_length];
        let result = MaskedKey::pre_encode(
            1,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            encoded_metadata.len(),
            encrypted_key_len,
            &mut buffer,
        );
        assert!(result.is_ok());
        let mut pre_encoded = result.unwrap();

        // Extract the AES key for testing
        let PreEncodeMaskedKeyType::Aes(aes_key) = &pre_encoded;

        assert_eq!(aes_key.iv().len(), AES_CBC_IV_SIZE);
        assert_eq!(aes_key.encrypted_key().len(), encrypted_key_len);
        assert_eq!(aes_key.tag().len(), AES_CBC_TAG_SIZE);

        // Encode the MaskedKey
        let plaintext_key = vec![1u8; plaintext_key_len];
        env.plaintext = plaintext_key.clone();
        let result = MaskedKey::encode(
            &env,
            &mut pre_encoded,
            plaintext_key.as_slice(),
            &AES256_HMAC384_COMBO_KEY,
            &encoded_metadata,
        );
        assert!(result.is_ok());

        // Decode the MaskedKey from the byte slice.
        let result = MaskedKey::decode(
            &env,
            &AES256_HMAC384_COMBO_KEY,
            buffer.as_slice(),
            /*integrity_check = */ true,
        );
        assert!(result.is_ok());
        let decoded_key = result.unwrap(); // Keep the DecodedMaskedKey enum

        // Extract the AES key for individual field verification
        let aes_key = decoded_key.as_aes().unwrap();
        let algorithm = aes_key.header().algorithm;
        let version = aes_key.header().version;
        assert_eq!(algorithm, MaskingKeyAlgorithm::AesCbc256Hmac384);
        assert_eq!(version, 1);
        assert_eq!(aes_key.iv().len(), AES_CBC_IV_SIZE);
        assert_eq!(aes_key.tag().len(), AES_CBC_TAG_SIZE);

        // Decrypt the key to verify correctness.
        // Call decrypt_key on the DecodedMaskedKey enum, not on MaskedKeyAes
        let mut decrypted_key = vec![0u8; plaintext_key_len];
        let decrypt_result =
            decoded_key.decrypt_key(&env, &AES256_HMAC384_COMBO_KEY, &mut decrypted_key);
        assert!(decrypt_result.is_ok());
        assert_eq!(decrypted_key, plaintext_key);
    }

    #[test]
    fn test_encode_decode_aes_cbc256_hmac384_too_small_buffer() {
        let env = TestCryptoEnv::new();
        let key_label = b"DummyFive";
        let mut entry_attributes = EntryAttributes::default();
        entry_attributes.common.flags.set_sign(true);
        entry_attributes.common.flags.set_verify(true);
        entry_attributes.common.flags.set_session(true);
        let metadata = DdiMaskedKeyMetadata {
            svn: Some(1),
            key_type: DdiKeyType::Ecc256Private,
            key_attributes: DdiMaskedKeyAttributes {
                blob: entry_attributes
                    .as_bytes()
                    .try_into()
                    .expect("Fail to convert to [u8;32]"),
            },
            bks2_index: None,
            key_tag: None,
            // max size 128 bytes
            key_label: MborByteArray::new_with_len(key_label.as_ptr(), key_label.len()),
            key_length: EntryKind::try_from(DdiKeyType::Ecc256Private)
                .map(|entry| entry.raw_key_blob_size() as u16)
                .unwrap(),
        };

        let encoded_metadata = encode_metadata(&metadata);

        let plaintext_key_len = 32;
        let encrypted_key_len = env.aescbc256_enc_data_len(plaintext_key_len);
        let encoded_length = MaskedKey::encoded_length(
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            encoded_metadata.len(),
            encrypted_key_len,
        );
        assert!(encoded_length % 4 == 0);
        let mut buffer = vec![0u8; encoded_length - 1];
        let result = MaskedKey::pre_encode(
            1,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            encoded_metadata.len(),
            encrypted_key_len,
            &mut buffer,
        );

        assert!(matches!(result, Err(MaskedKeyError::InvalidLength)));
    }

    #[test]
    fn test_encode_decode_aes_cbc256_hmac384_wrong_combo_key() {
        let mut env = TestCryptoEnv::new();
        let key_label = b"DummyFive";
        let mut entry_attributes = EntryAttributes::default();
        entry_attributes.common.flags.set_sign(true);
        entry_attributes.common.flags.set_verify(true);
        entry_attributes.common.flags.set_session(true);
        let metadata = DdiMaskedKeyMetadata {
            svn: Some(1),
            key_type: DdiKeyType::Ecc256Private,
            key_attributes: DdiMaskedKeyAttributes {
                blob: entry_attributes
                    .as_bytes()
                    .try_into()
                    .expect("Fail to convert to [u8;32]"),
            },
            bks2_index: None,
            key_tag: None,
            // max size 128 bytes
            key_label: MborByteArray::new_with_len(key_label.as_ptr(), key_label.len()),
            key_length: EntryKind::try_from(DdiKeyType::Ecc256Private)
                .map(|entry| entry.raw_key_blob_size() as u16)
                .unwrap(),
        };

        let encoded_metadata = encode_metadata(&metadata);

        let plaintext_key_len = 32;
        let encrypted_key_len = env.aescbc256_enc_data_len(plaintext_key_len);
        let encoded_length = MaskedKey::encoded_length(
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            encoded_metadata.len(),
            encrypted_key_len,
        );
        assert!(encoded_length % 4 == 0);
        let mut buffer = vec![0u8; encoded_length];
        let result = MaskedKey::pre_encode(
            1,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            encoded_metadata.len(),
            encrypted_key_len,
            &mut buffer,
        );
        assert!(result.is_ok());
        let mut pre_encoded = result.unwrap();

        // Extract the AES key for testing
        let PreEncodeMaskedKeyType::Aes(aes_key) = &pre_encoded;

        assert_eq!(aes_key.iv().len(), AES_CBC_IV_SIZE);
        assert_eq!(aes_key.encrypted_key().len(), encrypted_key_len);
        assert_eq!(aes_key.tag().len(), AES_CBC_TAG_SIZE);

        let wrong_combo_key = vec![0xFF; 80];

        // Encode the MaskedKey
        let plaintext_key = vec![1u8; plaintext_key_len];
        let result = MaskedKey::encode(
            &env,
            &mut pre_encoded,
            plaintext_key.as_slice(),
            &wrong_combo_key,
            &encoded_metadata,
        );
        assert!(result.is_ok());

        // Decode the MaskedKey from the byte slice.
        let result = MaskedKey::decode(
            &env,
            &wrong_combo_key,
            buffer.as_slice(),
            /*integrity_check = */ true,
        );
        assert!(result.is_ok());
        let decoded_key = result.unwrap(); // Keep the DecodedMaskedKey enum

        // Extract the AES key for individual field verification
        let aes_key = decoded_key.as_aes().unwrap();
        let algorithm = aes_key.header().algorithm;
        let version = aes_key.header().version;
        assert_eq!(algorithm, MaskingKeyAlgorithm::AesCbc256Hmac384);
        assert_eq!(version, 1);
        assert_eq!(aes_key.iv().len(), AES_CBC_IV_SIZE);
        assert_eq!(aes_key.tag().len(), AES_CBC_TAG_SIZE);

        // Decrypt the key to verify correctness.
        // Call decrypt_key on the DecodedMaskedKey enum, not on MaskedKeyAes
        let mut decrypted_key = vec![0u8; plaintext_key_len];
        env.error = Some(HsmErr::AesDecryptFailed);
        let decrypt_result =
            decoded_key.decrypt_key(&env, &AES256_HMAC384_COMBO_KEY, &mut decrypted_key);
        assert!(decrypt_result.is_err());
    }
}
