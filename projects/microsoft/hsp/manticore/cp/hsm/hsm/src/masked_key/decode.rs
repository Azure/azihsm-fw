// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_types::MaskedKey;
use mcr_ddi_types::MaskedKeyAes;
use mcr_ddi_types::MaskedKeyAesHeader;
use mcr_ddi_types::MaskedKeyAesLayout;
use mcr_ddi_types::MaskedKeyError;
use mcr_ddi_types::MaskedKeyHeader;
use mcr_ddi_types::MaskingKeyAlgorithm;
use zerocopy::TryFromBytes;

use crate::crypto_env::CryptEnv;

use super::*;

/// Enum to handle different decoded masked key types
pub enum DecodedMaskedKey<'a> {
    Aes(MaskedKeyAes<'a>),
}

impl<'a> DecodedMaskedKey<'a> {
    /// Returns the AES-specific masked key, if it exists.
    pub fn as_aes(&self) -> Option<&MaskedKeyAes<'a>> {
        match self {
            DecodedMaskedKey::Aes(aes) => Some(aes),
        }
    }

    /// Decrypts the key using the appropriate algorithm
    pub fn decrypt_key<Env: CryptEnv>(
        &self,
        env: &Env,
        key: &[u8],
        output: &mut [u8],
    ) -> Result<usize, MaskedKeyError> {
        match self {
            DecodedMaskedKey::Aes(aes_key) => match aes_key.header().algorithm {
                MaskingKeyAlgorithm::AesCbc256Hmac384 => {
                    let (aes_key_bytes, _) = split_aes_hmac_key(key)?;

                    let iv = aes_key.iv();
                    let encrypted_key = aes_key.encrypted_key();

                    let plaintext_len = env
                        .aescbc256_decrypt(aes_key_bytes, iv, encrypted_key, output)
                        .map_err(|_| MaskedKeyError::AesDecryptionFailed)?;
                    Ok(plaintext_len)
                }
                _ => Err(MaskedKeyError::InvalidMaskingKeyAlgorithm),
            },
        }
    }
}

/// A trait for decoding a `MaskedKey` from a raw byte representation.
pub trait MaskedKeyDecode<'a, Env: CryptEnv>: Sized {
    /// Decodes an `MaskedKey` from a byte slice.
    ///
    /// # Arguments
    /// * `env`: The cryptographic environment used for decoding.
    /// * `masking_key`: A reference to the masking key used for unmasking the secret key.
    /// * `data`: A reference to the byte slice containing the raw masked key data.
    /// * `integrity_check`: Whether to perform integrity checking on the masked key.
    ///
    /// # Returns
    /// * `Result<DecodedMaskedKey<'a>, MaskedKeyError>` - The decoded masked key on success, or an error if decoding fails.
    fn decode(
        env: &Env,
        masking_key: &[u8],
        data: &'a [u8],
        integrity_check: bool,
    ) -> Result<DecodedMaskedKey<'a>, MaskedKeyError>;
}

impl<'a, Env: CryptEnv> MaskedKeyDecode<'a, Env> for MaskedKey<'a> {
    fn decode(
        env: &Env,
        masking_key: &[u8],
        data: &'a [u8],
        integrity_check: bool,
    ) -> Result<DecodedMaskedKey<'a>, MaskedKeyError> {
        if data.len() < size_of::<MaskedKeyHeader>() {
            Err(MaskedKeyError::InvalidLength)?;
        }

        let (header, remaining) = MaskedKeyHeader::try_ref_from_prefix(data)
            .map_err(|_| MaskedKeyError::HeaderDecodeError)?;

        match header.algorithm {
            MaskingKeyAlgorithm::AesCbc256Hmac384 | MaskingKeyAlgorithm::AesGcm256 => {
                let aes_key =
                    decode_aes(env, masking_key, header, data, remaining, integrity_check)?;
                Ok(DecodedMaskedKey::Aes(aes_key))
            }
            _ => Err(MaskedKeyError::InvalidMaskingKeyAlgorithm),
        }
    }
}

/// Decodes an AES-based masked key from a byte slice.
///
/// # Arguments
/// * `env`: The cryptographic environment used for decoding.
/// * `masking_key`: The masking key used for unmasking the secret key.
/// * `header`: The already-parsed masked key header.
/// * `full_data`: The full byte slice containing the masked key data.
/// * `aes_data`: The byte slice containing the AES-specific data.
/// * `integrity_check`: Whether to perform integrity checking on the masked key.
///
/// # Returns
/// * `Result<MaskedKey<'a>, MaskedKeyError>` - The decoded masked key on success.
fn decode_aes<'a, Env: CryptEnv>(
    env: &Env,
    masking_key: &[u8],
    header: &MaskedKeyHeader,
    full_data: &'a [u8],
    aes_data: &'a [u8],
    integrity_check: bool,
) -> Result<MaskedKeyAes<'a>, MaskedKeyError> {
    // Parse the AES payload
    if aes_data.len() < size_of::<MaskedKeyAesHeader>() {
        return Err(MaskedKeyError::InvalidLength);
    }

    let (aes_header, payload_data) = MaskedKeyAesHeader::try_ref_from_prefix(aes_data)
        .map_err(|_| MaskedKeyError::HeaderDecodeError)?;

    validate_aes_header(aes_header)?;

    let layout = MaskedKeyAesLayout {
        metadata_len: aes_header.metadata_len as usize,
        post_metadata_pad_len: aes_header.post_metadata_pad_len as usize,
        encrypted_key_len: aes_header.encrypted_key_len as usize,
        post_encrypted_key_pad_len: aes_header.post_encrypted_key_pad_len as usize,
        iv_len: aes_header.iv_len as usize,
        post_iv_pad_len: aes_header.post_iv_pad_len as usize,
        tag_len: aes_header.tag_len as usize,
    };

    // Calculate the total expected length from the payload fields.
    let expected_payload_len = aes_header.iv_len as usize
        + aes_header.post_iv_pad_len as usize
        + aes_header.metadata_len as usize
        + aes_header.post_metadata_pad_len as usize
        + aes_header.encrypted_key_len as usize
        + aes_header.post_encrypted_key_pad_len as usize
        + aes_header.tag_len as usize;

    // Ensure the payload data length matches the expected length.
    if payload_data.len() != expected_payload_len {
        return Err(MaskedKeyError::InvalidLength);
    }

    let mut current_offset = size_of::<MaskedKeyHeader>() + size_of::<MaskedKeyAesHeader>();

    // Skip to the tag
    current_offset += aes_header.iv_len as usize
        + aes_header.post_iv_pad_len as usize
        + aes_header.metadata_len as usize
        + aes_header.post_metadata_pad_len as usize
        + aes_header.encrypted_key_len as usize
        + aes_header.post_encrypted_key_pad_len as usize;

    if integrity_check {
        if current_offset + aes_header.tag_len as usize <= full_data.len() {
            let tag = &full_data[current_offset..current_offset + aes_header.tag_len as usize];

            // Verify the integrity of masked key.
            match header.algorithm {
                MaskingKeyAlgorithm::AesCbc256Hmac384 => {
                    let (_, hmac_key) = split_aes_hmac_key(masking_key)?;

                    let expected_tag = env
                        .hmac384_tag(hmac_key, &full_data[..current_offset])
                        .map_err(|_| MaskedKeyError::HmacTagGenerationFailed)?;
                    if expected_tag.as_slice() != tag {
                        Err(MaskedKeyError::HmacTagVerificationFailed)?;
                    }
                }
                _ => return Err(MaskedKeyError::InvalidMaskingKeyAlgorithm),
            }
        } else {
            return Err(MaskedKeyError::InvalidLength);
        }
    }

    // Construct the MaskedKey struct with the layout and payload data
    Ok(MaskedKeyAes::new(*header, layout, aes_data))
}

#[cfg(test)]
mod tests {
    use mcr_ddi_types::MaskingKeyAlgorithm;
    use mcr_types::SecureByteArray;
    use sha2::{Digest, Sha384};
    use zerocopy::IntoBytes;

    use crate::error::HsmErr;

    use super::*;

    #[allow(dead_code)]
    pub struct TestCryptoEnv {
        pub plaintext: Vec<u8>,
        pub ciphertext: Vec<u8>,
        pub hmac384_tag: [u8; 48],
    }

    impl TestCryptoEnv {
        pub fn new() -> Self {
            TestCryptoEnv {
                plaintext: Vec::new(),
                ciphertext: Vec::new(),
                hmac384_tag: [0u8; 48],
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

        fn aescbc256_enc_data_len(&self, plaintext_key_len: usize) -> usize {
            plaintext_key_len
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

    #[test]
    fn test_masked_key_decode() {
        let mut data = vec![0u8; 3072];

        // Create the new header structure (only version and algorithm)
        let header = MaskedKeyHeader {
            version: 1,
            algorithm: MaskingKeyAlgorithm::AesCbc256Hmac384,
        };

        // Create the AES payload structure with length information
        let payload = MaskedKeyAesHeader {
            iv_len: 16,
            post_iv_pad_len: 0,
            metadata_len: 32,
            post_metadata_pad_len: 0,
            encrypted_key_len: 48, // CBC padded length
            post_encrypted_key_pad_len: 0,
            tag_len: 48,
            reserved: [0u8; 34],
        };

        // Write header and payload to buffer
        let header_bytes = header.as_bytes();
        let payload_bytes = payload.as_bytes();

        data[..header_bytes.len()].copy_from_slice(header_bytes);
        data[header_bytes.len()..header_bytes.len() + payload_bytes.len()]
            .copy_from_slice(payload_bytes);

        // Fill the rest of the data with dummy values.
        data[header_bytes.len() + payload_bytes.len()..].fill(0xFF);

        let env = TestCryptoEnv::new();

        // Compute the HMAC tag for the data.
        // The tag should be computed over header + payload + all data except the tag itself
        let tag_start = header_bytes.len()
            + payload_bytes.len()
            + payload.iv_len as usize
            + payload.post_iv_pad_len as usize
            + payload.metadata_len as usize
            + payload.post_metadata_pad_len as usize
            + payload.encrypted_key_len as usize
            + payload.post_encrypted_key_pad_len as usize;

        let (_, hmac_key) = split_aes_hmac_key(&AES256_HMAC384_COMBO_KEY).unwrap();
        let tag = env.hmac384_tag(hmac_key, &data[..tag_start]).unwrap();
        data[tag_start..tag_start + payload.tag_len as usize]
            .copy_from_slice(&tag[..payload.tag_len as usize]);

        let total_len = tag_start + payload.tag_len as usize;

        let decoded = MaskedKey::decode(&env, &AES256_HMAC384_COMBO_KEY, &data[..total_len], true);
        assert!(decoded.is_ok());
        let decoded = decoded.unwrap();

        let aes_key = decoded.as_aes().expect("Should be AES key");

        assert_eq!(aes_key.header(), &header);

        assert_eq!(aes_key.layout().iv_len, payload.iv_len as usize);
        assert_eq!(aes_key.layout().metadata_len, payload.metadata_len as usize);
        assert_eq!(
            aes_key.layout().encrypted_key_len,
            payload.encrypted_key_len as usize
        );
        assert_eq!(aes_key.layout().tag_len, payload.tag_len as usize);

        let iv = aes_key.iv();
        let encrypted_key = aes_key.encrypted_key();
        let tag_slice = aes_key.tag();

        assert_eq!(iv.len(), payload.iv_len as usize);
        assert_eq!(encrypted_key.len(), payload.encrypted_key_len as usize);
        assert_eq!(tag_slice.len(), payload.tag_len as usize);
        assert_eq!(tag_slice, &tag[..payload.tag_len as usize]);
    }

    #[test]
    fn test_masked_key_decode_insufficient_data() {
        let env = TestCryptoEnv::new();
        let result = MaskedKey::decode(&env, &AES256_HMAC384_COMBO_KEY, &[], true);
        assert!(matches!(result, Err(MaskedKeyError::InvalidLength)));
    }

    #[test]
    fn test_masked_key_decode_invalid_length() {
        let env = TestCryptoEnv::new();
        let mut data = vec![0u8; std::mem::size_of::<MaskedKeyHeader>() + 1];
        let header = MaskedKeyHeader {
            version: 1,
            algorithm: MaskingKeyAlgorithm::AesCbc256Hmac384,
        };

        // Only copy the header bytes that fit in the buffer
        let header_bytes = header.as_bytes();
        data[..header_bytes.len()].copy_from_slice(header_bytes);

        let result = MaskedKey::decode(&env, &AES256_HMAC384_COMBO_KEY, &data, true);
        assert!(matches!(result, Err(MaskedKeyError::InvalidLength)));
    }

    fn test_decode_payload_with_alignment(
        payload: MaskedKeyAesHeader,
        should_succeed: bool,
        _label: &str,
    ) {
        let header = MaskedKeyHeader {
            version: 1,
            algorithm: MaskingKeyAlgorithm::AesCbc256Hmac384,
        };

        let header_bytes = header.as_bytes();
        let payload_bytes = payload.as_bytes();

        let tag_start = header_bytes.len()
            + payload_bytes.len()
            + payload.iv_len as usize
            + payload.post_iv_pad_len as usize
            + payload.metadata_len as usize
            + payload.post_metadata_pad_len as usize
            + payload.encrypted_key_len as usize
            + payload.post_encrypted_key_pad_len as usize;

        let total_len = tag_start + payload.tag_len as usize;
        let mut data = vec![0u8; total_len];

        data[..header_bytes.len()].copy_from_slice(header_bytes);
        data[header_bytes.len()..header_bytes.len() + payload_bytes.len()]
            .copy_from_slice(payload_bytes);

        // Fill dummy data
        data[header_bytes.len() + payload_bytes.len()..tag_start].fill(0xAB);

        let env = TestCryptoEnv::new();
        let (_, hmac_key) = split_aes_hmac_key(&AES256_HMAC384_COMBO_KEY).unwrap();

        if payload.tag_len > 0 {
            let tag = env.hmac384_tag(hmac_key, &data[..tag_start]).unwrap();
            data[tag_start..total_len].copy_from_slice(&tag[..payload.tag_len as usize]);
        }

        let result = MaskedKey::decode(&env, &AES256_HMAC384_COMBO_KEY, &data[..], true);
        if should_succeed {
            assert!(result.is_ok());
        } else {
            assert!(result.is_err());
        }
    }

    #[test]
    fn test_masked_key_decode_invalid_tag() {
        let mut data = vec![0u8; 3072];

        let header = MaskedKeyHeader {
            version: 1,
            algorithm: MaskingKeyAlgorithm::AesCbc256Hmac384,
        };

        let payload = MaskedKeyAesHeader {
            iv_len: 16,
            post_iv_pad_len: 0,
            metadata_len: 32,
            post_metadata_pad_len: 0,
            encrypted_key_len: 48,
            post_encrypted_key_pad_len: 0,
            tag_len: 48,
            reserved: [0u8; 34],
        };

        let header_bytes = header.as_bytes();
        let payload_bytes = payload.as_bytes();
        data[..header_bytes.len()].copy_from_slice(header_bytes);
        data[header_bytes.len()..header_bytes.len() + payload_bytes.len()]
            .copy_from_slice(payload_bytes);

        data[header_bytes.len() + payload_bytes.len()..].fill(0xFF);

        // Insert an incorrect tag
        let tag_start = header_bytes.len()
            + payload_bytes.len()
            + payload.iv_len as usize
            + payload.post_iv_pad_len as usize
            + payload.metadata_len as usize
            + payload.post_metadata_pad_len as usize
            + payload.encrypted_key_len as usize
            + payload.post_encrypted_key_pad_len as usize;

        let tag = vec![0xAA; payload.tag_len as usize];
        data[tag_start..tag_start + payload.tag_len as usize].copy_from_slice(&tag);

        let env = TestCryptoEnv::new();
        let result = MaskedKey::decode(
            &env,
            &AES256_HMAC384_COMBO_KEY,
            &data[..tag_start + tag.len()],
            true,
        );
        assert!(matches!(
            result,
            Err(MaskedKeyError::HmacTagVerificationFailed)
        ));
    }

    #[test]
    fn test_decode_fails_on_too_short_iv() {
        let mut data = [0u8; 128];
        let header = MaskedKeyHeader {
            version: 1,
            algorithm: MaskingKeyAlgorithm::AesCbc256Hmac384,
        };
        let payload = MaskedKeyAesHeader {
            iv_len: 64, // says 64 bytes
            post_iv_pad_len: 0,
            metadata_len: 0,
            post_metadata_pad_len: 0,
            encrypted_key_len: 16,
            post_encrypted_key_pad_len: 0,
            tag_len: 48,
            reserved: [0u8; 34],
        };

        let header_bytes = header.as_bytes();
        let payload_bytes = payload.as_bytes();

        data[..header_bytes.len()].copy_from_slice(header_bytes);
        data[header_bytes.len()..header_bytes.len() + payload_bytes.len()]
            .copy_from_slice(payload_bytes);

        // Fill only 32 bytes total after headers (less than required 64 for IV)
        let total = header_bytes.len() + payload_bytes.len() + 32;
        let env = TestCryptoEnv::new();
        let result = MaskedKey::decode(&env, &[0u8; 80], &data[..total], true);

        assert!(matches!(result, Err(MaskedKeyError::InvalidLength)));
    }

    #[test]
    fn test_decode_fails_on_missing_post_iv_padding() {
        let mut data = vec![0u8; 256];
        let header = MaskedKeyHeader {
            version: 1,
            algorithm: MaskingKeyAlgorithm::AesCbc256Hmac384,
        };
        let payload = MaskedKeyAesHeader {
            iv_len: 16,
            post_iv_pad_len: 16, // expects padding
            metadata_len: 0,
            post_metadata_pad_len: 0,
            encrypted_key_len: 16,
            post_encrypted_key_pad_len: 0,
            tag_len: 48,
            reserved: [0u8; 34],
        };

        let header_bytes = header.as_bytes();
        let payload_bytes = payload.as_bytes();
        data[..header_bytes.len()].copy_from_slice(header_bytes);
        data[header_bytes.len()..header_bytes.len() + payload_bytes.len()]
            .copy_from_slice(payload_bytes);

        // Fill only the IV, not the padding
        let offset = header_bytes.len() + payload_bytes.len();
        data[offset..offset + 16].fill(0x11);

        let total = offset + 16; // missing the padding
        let env = TestCryptoEnv::new();
        let result = MaskedKey::decode(&env, &[0u8; 80], &data[..total], true);

        assert!(matches!(result, Err(MaskedKeyError::InvalidLength)));
    }

    #[test]
    fn test_decode_fails_on_too_short_encrypted_key() {
        let mut data = [0u8; 128];
        let header = MaskedKeyHeader {
            version: 1,
            algorithm: MaskingKeyAlgorithm::AesCbc256Hmac384,
        };
        let payload = MaskedKeyAesHeader {
            iv_len: 16,
            post_iv_pad_len: 0,
            metadata_len: 0,
            post_metadata_pad_len: 0,
            encrypted_key_len: 64, // says 64 bytes
            post_encrypted_key_pad_len: 0,
            tag_len: 48,
            reserved: [0u8; 34],
        };

        let header_bytes = header.as_bytes();
        let payload_bytes = payload.as_bytes();

        data[..header_bytes.len()].copy_from_slice(header_bytes);
        data[header_bytes.len()..header_bytes.len() + payload_bytes.len()]
            .copy_from_slice(payload_bytes);

        // Fill IV and only part of the encrypted key (e.g., 32 out of 64)
        let offset = header_bytes.len() + payload_bytes.len() + 16; // 16 = IV
        data[offset..offset + 32].fill(0xEF); // only 32 instead of 64

        let total = offset + 32;
        let env = TestCryptoEnv::new();
        let result = MaskedKey::decode(&env, &[0u8; 80], &data[..total], true);

        assert!(matches!(result, Err(MaskedKeyError::InvalidLength)));
    }

    #[test]
    fn test_decode_fails_on_missing_tag() {
        let mut data = vec![0u8; 256];
        let header = MaskedKeyHeader {
            version: 1,
            algorithm: MaskingKeyAlgorithm::AesCbc256Hmac384,
        };
        let payload = MaskedKeyAesHeader {
            iv_len: 16,
            post_iv_pad_len: 0,
            metadata_len: 0,
            post_metadata_pad_len: 0,
            encrypted_key_len: 32,
            post_encrypted_key_pad_len: 0,
            tag_len: 48, // required but will not be provided
            reserved: [0u8; 34],
        };

        let header_bytes = header.as_bytes();
        let payload_bytes = payload.as_bytes();
        data[..header_bytes.len()].copy_from_slice(header_bytes);
        data[header_bytes.len()..header_bytes.len() + payload_bytes.len()]
            .copy_from_slice(payload_bytes);

        // Fill IV + encrypted key only
        let offset = header_bytes.len() + payload_bytes.len() + 16 + 32;
        // Do not insert the tag (48 bytes missing)
        let total = offset; // shorter than expected

        let env = TestCryptoEnv::new();
        let result = MaskedKey::decode(&env, &[0u8; 80], &data[..total], true);

        assert!(matches!(result, Err(MaskedKeyError::InvalidLength)));
    }

    #[test]
    fn test_decode_fails_on_missing_post_metadata_padding() {
        let mut data = vec![0u8; 256];
        let header = MaskedKeyHeader {
            version: 1,
            algorithm: MaskingKeyAlgorithm::AesCbc256Hmac384,
        };
        let payload = MaskedKeyAesHeader {
            iv_len: 16,
            post_iv_pad_len: 0,
            metadata_len: 16,
            post_metadata_pad_len: 16, // declared but will not be provided
            encrypted_key_len: 16,
            post_encrypted_key_pad_len: 0,
            tag_len: 48,
            reserved: [0u8; 34],
        };

        let header_bytes = header.as_bytes();
        let payload_bytes = payload.as_bytes();
        data[..header_bytes.len()].copy_from_slice(header_bytes);
        data[header_bytes.len()..header_bytes.len() + payload_bytes.len()]
            .copy_from_slice(payload_bytes);

        // Fill up to metadata only (not post-metadata padding)
        let offset = header_bytes.len() + payload_bytes.len() + 16 + 16; // IV + metadata

        let total = offset; // missing post_metadata_pad_len
        let env = TestCryptoEnv::new();
        let result = MaskedKey::decode(&env, &[0u8; 80], &data[..total], true);

        assert!(matches!(result, Err(MaskedKeyError::InvalidLength)));
    }

    #[test]
    fn test_decode_fails_on_missing_post_encrypted_key_padding() {
        let mut data = vec![0u8; 256];
        let header = MaskedKeyHeader {
            version: 1,
            algorithm: MaskingKeyAlgorithm::AesCbc256Hmac384,
        };
        let payload = MaskedKeyAesHeader {
            iv_len: 16,
            post_iv_pad_len: 0,
            metadata_len: 0,
            post_metadata_pad_len: 0,
            encrypted_key_len: 16,
            post_encrypted_key_pad_len: 16, // declared but missing
            tag_len: 48,
            reserved: [0u8; 34],
        };

        let header_bytes = header.as_bytes();
        let payload_bytes = payload.as_bytes();
        data[..header_bytes.len()].copy_from_slice(header_bytes);
        data[header_bytes.len()..header_bytes.len() + payload_bytes.len()]
            .copy_from_slice(payload_bytes);

        // Fill only IV + encrypted key (no post-key padding)
        let offset = header_bytes.len() + payload_bytes.len() + 16 + 16;

        let total = offset; // missing post_encrypted_key_pad_len
        let env = TestCryptoEnv::new();
        let result = MaskedKey::decode(&env, &[0u8; 80], &data[..total], true);

        assert!(matches!(result, Err(MaskedKeyError::InvalidLength)));
    }

    #[test]
    fn test_decode_fails_on_tag_declared_but_not_present() {
        let mut data = [0u8; 128];
        let header = MaskedKeyHeader {
            version: 1,
            algorithm: MaskingKeyAlgorithm::AesCbc256Hmac384,
        };
        let payload = MaskedKeyAesHeader {
            iv_len: 8,
            post_iv_pad_len: 0,
            metadata_len: 0,
            post_metadata_pad_len: 0,
            encrypted_key_len: 8,
            post_encrypted_key_pad_len: 0,
            tag_len: 16, // declared but tag data is missing
            reserved: [0u8; 34],
        };

        let header_bytes = header.as_bytes();
        let payload_bytes = payload.as_bytes();
        data[..header_bytes.len()].copy_from_slice(header_bytes);
        data[header_bytes.len()..header_bytes.len() + payload_bytes.len()]
            .copy_from_slice(payload_bytes);

        let offset = header_bytes.len() + payload_bytes.len() + 8 + 8; // IV + ENC
        let total = offset; // missing tag

        let env = TestCryptoEnv::new();
        let result = MaskedKey::decode(&env, &[0u8; 80], &data[..total], true);

        assert!(matches!(result, Err(MaskedKeyError::InvalidLength)));
    }

    #[test]
    fn test_field_alignment_combinations() {
        //  ivlen + post_iv_pad_len = 16 (valid)
        test_decode_payload_with_alignment(
            MaskedKeyAesHeader {
                iv_len: 12,
                post_iv_pad_len: 4,
                metadata_len: 16,
                post_metadata_pad_len: 0,
                encrypted_key_len: 32,
                post_encrypted_key_pad_len: 0,
                tag_len: 48,
                reserved: [0u8; 34],
            },
            true,
            "iv_len + post_iv_pad_len = 16",
        );

        // metadata_len + post_metadata_pad_len = 16 (valid)
        test_decode_payload_with_alignment(
            MaskedKeyAesHeader {
                iv_len: 16,
                post_iv_pad_len: 0,
                metadata_len: 12,
                post_metadata_pad_len: 4,
                encrypted_key_len: 32,
                post_encrypted_key_pad_len: 0,
                tag_len: 48,
                reserved: [0u8; 34],
            },
            true,
            "metadata_len + post_metadata_pad_len = 16",
        );

        //  encrypted_key_len + post_encrypted_key_pad_len = 32 (valid)
        test_decode_payload_with_alignment(
            MaskedKeyAesHeader {
                iv_len: 16,
                post_iv_pad_len: 0,
                metadata_len: 16,
                post_metadata_pad_len: 0,
                encrypted_key_len: 28,
                post_encrypted_key_pad_len: 4,
                tag_len: 48,
                reserved: [0u8; 34],
            },
            true,
            "encrypted_key_len + post_encrypted_key_pad_len = 32",
        );

        // tag_len = 47 (invalid - not multiple of 4)

        test_decode_payload_with_alignment(
            MaskedKeyAesHeader {
                iv_len: 16,
                post_iv_pad_len: 0,
                metadata_len: 16,
                post_metadata_pad_len: 0,
                encrypted_key_len: 29,
                post_encrypted_key_pad_len: 3,
                tag_len: 47,
                reserved: [0u8; 34],
            },
            false,
            "tag_len = 47 (not multiple of 4)",
        );

        // encrypted_key_len + post_encrypted_key_pad_len = 31 (invalid)
        test_decode_payload_with_alignment(
            MaskedKeyAesHeader {
                iv_len: 16,
                post_iv_pad_len: 0,
                metadata_len: 16,
                post_metadata_pad_len: 0,
                encrypted_key_len: 29,
                post_encrypted_key_pad_len: 2,
                tag_len: 48,
                reserved: [0u8; 34],
            },
            false,
            "encrypted_key_len + post_encrypted_key_pad_len = 31",
        );

        // metadata_len + post_metadata_pad_len = 14 (invalid)
        test_decode_payload_with_alignment(
            MaskedKeyAesHeader {
                iv_len: 16,
                post_iv_pad_len: 0,
                metadata_len: 11,
                post_metadata_pad_len: 3,
                encrypted_key_len: 32,
                post_encrypted_key_pad_len: 0,
                tag_len: 48,
                reserved: [0u8; 34],
            },
            false,
            "metadata_len + post_metadata_pad_len = 14",
        );

        //  iv_len + post_iv_pad_len = 13 (invalid)
        test_decode_payload_with_alignment(
            MaskedKeyAesHeader {
                iv_len: 10,
                post_iv_pad_len: 3,
                metadata_len: 16,
                post_metadata_pad_len: 0,
                encrypted_key_len: 32,
                post_encrypted_key_pad_len: 0,
                tag_len: 48,
                reserved: [0u8; 34],
            },
            false,
            "iv_len + post_iv_pad_len = 13",
        );
    }

    #[test]
    fn test_payload_parameter_combinations() {
        let iv_lens = [0, 12, 16];
        let enc_lens = [0, 32];
        let tag_lens = [0, 48];
        let metadata_lens = [0, 16];

        for &iv_len in &iv_lens {
            for &enc_len in &enc_lens {
                for &tag_len in &tag_lens {
                    for &metadata_len in &metadata_lens {
                        let post_iv_pad_len = 0;
                        let post_metadata_pad_len = 0;
                        let post_encrypted_key_pad_len = 0;

                        let payload = MaskedKeyAesHeader {
                            iv_len,
                            post_iv_pad_len,
                            metadata_len,
                            post_metadata_pad_len,
                            encrypted_key_len: enc_len,
                            post_encrypted_key_pad_len,
                            tag_len,
                            reserved: [0u8; 34],
                        };

                        let header = MaskedKeyHeader {
                            version: 1,
                            algorithm: MaskingKeyAlgorithm::AesCbc256Hmac384,
                        };

                        let header_bytes = header.as_bytes();
                        let payload_bytes = payload.as_bytes();

                        let tag_start = header_bytes.len()
                            + payload_bytes.len()
                            + iv_len as usize
                            + post_iv_pad_len as usize
                            + metadata_len as usize
                            + post_metadata_pad_len as usize
                            + enc_len as usize
                            + post_encrypted_key_pad_len as usize;

                        let total_len = tag_start + tag_len as usize;
                        let mut data = vec![0u8; total_len];

                        // Write header and payload
                        data[..header_bytes.len()].copy_from_slice(header_bytes);
                        data[header_bytes.len()..header_bytes.len() + payload_bytes.len()]
                            .copy_from_slice(payload_bytes);

                        // Fill payload data (IV, metadata, encrypted key, etc.) with dummy bytes
                        data[header_bytes.len() + payload_bytes.len()..tag_start].fill(0xFF);

                        let env = TestCryptoEnv::new();
                        let (_, hmac_key) = split_aes_hmac_key(&AES256_HMAC384_COMBO_KEY).unwrap();

                        // Only insert valid tag if tag_len > 0
                        if tag_len > 0 && tag_start + tag_len as usize <= total_len {
                            let tag = env.hmac384_tag(hmac_key, &data[..tag_start]).unwrap();
                            data[tag_start..tag_start + tag_len as usize]
                                .copy_from_slice(&tag[..tag_len as usize]);
                        }

                        let result =
                            MaskedKey::decode(&env, &AES256_HMAC384_COMBO_KEY, &data[..], true);

                        if iv_len > 0 && metadata_len > 0 && enc_len > 0 && tag_len > 0 {
                            assert!(result.is_ok());
                        } else {
                            assert!(result.is_err());
                        }
                    }
                }
            }
        }
    }

    #[test]
    fn test_decode_fails_on_non_multiple_of_4_lengths() {
        let non_4_lengths = [1, 2, 3, 5, 6, 7, 9, 10];
        let base_valid = MaskedKeyAesHeader {
            iv_len: 16,
            post_iv_pad_len: 0,
            metadata_len: 32,
            post_metadata_pad_len: 0,
            encrypted_key_len: 48,
            post_encrypted_key_pad_len: 0,
            tag_len: 48,
            reserved: [0u8; 34],
        };

        let fields_to_test = [
            "iv_len",
            "post_iv_pad_len",
            "metadata_len",
            "post_metadata_pad_len",
            "encrypted_key_len",
            "post_encrypted_key_pad_len",
            "tag_len",
        ];

        for &field in &fields_to_test {
            for &bad_value in &non_4_lengths {
                let mut payload = base_valid;

                match field {
                    "iv_len" => payload.iv_len = bad_value,
                    "post_iv_pad_len" => payload.post_iv_pad_len = bad_value,
                    "metadata_len" => payload.metadata_len = bad_value,
                    "post_metadata_pad_len" => payload.post_metadata_pad_len = bad_value,
                    "encrypted_key_len" => payload.encrypted_key_len = bad_value,
                    "post_encrypted_key_pad_len" => payload.post_encrypted_key_pad_len = bad_value,
                    "tag_len" => payload.tag_len = bad_value,
                    _ => continue,
                }

                // Assemble the full buffer
                let header = MaskedKeyHeader {
                    version: 1,
                    algorithm: MaskingKeyAlgorithm::AesCbc256Hmac384,
                };

                let mut data = vec![0u8; 1024];
                let header_bytes = header.as_bytes();
                let payload_bytes = payload.as_bytes();
                data[..header_bytes.len()].copy_from_slice(header_bytes);
                data[header_bytes.len()..header_bytes.len() + payload_bytes.len()]
                    .copy_from_slice(payload_bytes);

                // Fill rest of buffer with dummy bytes
                data[header_bytes.len() + payload_bytes.len()..].fill(0xFF);

                let tag_start = header_bytes.len()
                    + payload_bytes.len()
                    + payload.iv_len as usize
                    + payload.post_iv_pad_len as usize
                    + payload.metadata_len as usize
                    + payload.post_metadata_pad_len as usize
                    + payload.encrypted_key_len as usize
                    + payload.post_encrypted_key_pad_len as usize;

                if tag_start + payload.tag_len as usize <= data.len() {
                    data[tag_start..tag_start + payload.tag_len as usize].fill(0xAB);

                    let env = TestCryptoEnv::new();
                    let result = MaskedKey::decode(
                        &env,
                        &[0u8; 80],
                        &data[..tag_start + payload.tag_len as usize],
                        true,
                    );

                    assert!(
                        result.is_err(),
                        "Expected failure when {} = {}, but decode succeeded",
                        field,
                        bad_value
                    );
                }
            }
        }
    }
}
