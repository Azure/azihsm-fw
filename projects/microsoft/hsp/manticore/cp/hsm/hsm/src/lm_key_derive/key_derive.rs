// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_mbor::MborByteArray;
#[cfg(test)]
use mcr_ddi_mbor::MborDecode;
#[cfg(test)]
use mcr_ddi_mbor::MborDecoder;
use mcr_ddi_mbor::MborEncode;
use mcr_ddi_mbor::MborEncoder;
use mcr_ddi_mbor::MborLen;
use mcr_ddi_mbor::MborLenAccumulator;
use mcr_ddi_types::DdiKeyType;
use mcr_ddi_types::DdiMaskedKeyAttributes;
use mcr_ddi_types::DdiMaskedKeyMetadata;
use mcr_ddi_types::MaskedKey;
use mcr_ddi_types::MaskingKeyAlgorithm;
use mcr_ddi_types::AES_CBC_256_KEY_SIZE;
use mcr_ddi_types::HMAC384_KEY_SIZE;
use mcr_types::SecureByteArray;
use mcr_types::BK_AES_CBC_256_HMAC384_SIZE_BYTES;

use crate::crypto_env::CryptEnv;
use crate::error::HsmErr;
#[cfg(test)]
use crate::masked_key::DecodedMaskedKey;
use crate::masked_key::MaskedKeyDecode;
use crate::masked_key::MaskedKeyEncode;

pub(crate) const BK3_SIZE_BYTES: usize = 48;
pub(crate) const FW_SECRET_SIZE_BYTES: usize = 48;
pub(crate) const MK_SEED_SIZE_BYTES: usize = 48;
pub(crate) const BK_SEED_SIZE_BYTES: usize = 32;
pub(crate) const SESSION_SEED_SIZE_BYTES: usize = 48;
pub(crate) const BK_LABEL_LENGTH: usize = 256;
pub(crate) const PARTITION_BK_LABEL: &[u8] = b"PARTITION_BK";
pub(crate) const SESSION_BK_LABEL: &[u8] = b"SESSION_BK";
pub(crate) const SESSION_BK3_LABEL: &[u8] = b"SESSION_BK3";
pub(crate) const MK_DEFAULT_LABEL: &[u8] = b"MK_DEFAULT";
pub(crate) const BK_BOOT_DEFAULT_LABEL: &[u8] = b"BK_BOOT_KEY_DEFAULT";
pub(crate) const BK_BOOT_MASKING_KEY_DEFAULT_LABEL: &[u8] = b"BK_BOOT_MK_DEFAULT";
pub(crate) const MK_AES_CBC_256_HMAC384_SIZE_BYTES: usize = AES_CBC_256_KEY_SIZE + HMAC384_KEY_SIZE;
pub(crate) const BK_BOOT_MK_AES_CBC_256_HMAC384_SIZE_BYTES: usize =
    AES_CBC_256_KEY_SIZE + HMAC384_KEY_SIZE;

/// Live Migration Key Derivation implementation
///
/// Provides key derivation functionality for HSM live migration operations,
/// including backup key generation, backup masking key generation, and masking key restoration.
pub struct LMKeyDerive;

impl LMKeyDerive {
    /// Generate Partition Backup Key (BK) using the provided seeds BKS1, BKS2, and backup key BK3.
    ///
    /// # Arguments
    ///
    /// * `crypto_env` - The cryptographic environment to use.
    /// * `algo` - Indicates the type of the BK to be generated.
    /// * `bks1` - The first backup seed (BKS1).
    /// * `bks2` - The second backup seed (BKS2).
    /// * `bk3` - The backup key (BK3).
    /// * `pota_pub_key`: POTA Public Key
    /// * `bk_partition_len` - In/out parameter for the length of the partition backup key.
    ///   On input, it specifies the bk_out buffer size.
    ///   On output, it will contain the actual length of the generated backup key.
    /// * `bk_partition_out` - Output buffer for the generated partition backup key.
    ///
    /// # Returns
    /// * `Ok(())` - If the backup key is successfully generated.
    /// * `Err(HsmErr)` - If there is an error during the generation process.
    #[allow(clippy::too_many_arguments)]
    pub fn bk_partition_gen<Env: CryptEnv>(
        crypto_env: &Env,
        algo: MaskingKeyAlgorithm,
        bks1: &[u8; BK_SEED_SIZE_BYTES],
        bks2: &[u8; BK_SEED_SIZE_BYTES],
        bk3: &[u8; BK3_SIZE_BYTES],
        pota_pub_key: &[u8],
        bk_partition_len: &mut usize,
        bk_partition_out: &mut [u8],
    ) -> Result<(), HsmErr> {
        // Check BK algo. Only AesCbc256Hmac384 is supported for now.
        if algo != MaskingKeyAlgorithm::AesCbc256Hmac384 {
            Err(HsmErr::InvalidAlgorithm)?;
        }

        if *bk_partition_len < BK_AES_CBC_256_HMAC384_SIZE_BYTES
            || bk_partition_out.len() < BK_AES_CBC_256_HMAC384_SIZE_BYTES
        {
            *bk_partition_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
            Err(HsmErr::InsufficientBuffer)?;
        }

        // BKS12 is the concatenation of BKS1 and BKS2 and is used as KBKDF context.
        let mut bks1_2 =
            SecureByteArray::<{ BK_SEED_SIZE_BYTES * 2 }>::new([0u8; BK_SEED_SIZE_BYTES * 2]);
        bks1_2[..BK_SEED_SIZE_BYTES].copy_from_slice(bks1);
        bks1_2[BK_SEED_SIZE_BYTES..].copy_from_slice(bks2);

        // Derive BK via KBKDF using BK3 as key and BKS1_2 as context.
        if BK_LABEL_LENGTH < PARTITION_BK_LABEL.len() + pota_pub_key.len() {
            Err(HsmErr::InvalidArgument)?;
        }
        let mut label = [0u8; BK_LABEL_LENGTH];
        label[..PARTITION_BK_LABEL.len()].copy_from_slice(PARTITION_BK_LABEL);
        label[PARTITION_BK_LABEL.len()..PARTITION_BK_LABEL.len() + pota_pub_key.len()]
            .copy_from_slice(pota_pub_key);
        crypto_env.kbkdf_sha384(
            bk3,
            Some(&label),
            Some(bks1_2.as_slice()),
            BK_AES_CBC_256_HMAC384_SIZE_BYTES,
            &mut bk_partition_out[..BK_AES_CBC_256_HMAC384_SIZE_BYTES],
        )?;

        *bk_partition_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;

        Ok(())
    }

    /// Generate Partition or Session Backup Masking Key (BMK).
    ///
    /// # Arguments
    /// * `env` - The cryptographic environment to use.
    /// * `algo` - The crypto algorithm to use for the masking the masking key.
    /// * `bk` - The Partition/Session backup key. This key is used to encrypt the generated Partition/Session masking key.
    /// * `metadata` - The metadata to be associated with the masking key.
    /// * `masking_key` - The masking key to be generated.
    /// * `bmk_len` - In/out parameter for the length of the Partition/Session BMK.
    /// * `bmk_out` - Output buffer for the encoded Partition/Session BMK.
    ///
    /// # Returns
    /// * `Ok(())` - If the Partition/Session BMK is successfully generated.
    /// * `Err(HsmErr)` - If there is an error during the generation process.
    pub fn bmk_gen<Env: CryptEnv>(
        env: &Env,
        algo: MaskingKeyAlgorithm,
        bk: &[u8],
        masking_key: &[u8],
        metadata: &[u8],
        bmk_len: &mut usize,
        bmk_out: &mut [u8],
    ) -> Result<(), HsmErr> {
        // Check BMK algo. Only AesCbc256Hmac384 is supported for now.
        if algo != MaskingKeyAlgorithm::AesCbc256Hmac384 {
            Err(HsmErr::InvalidAlgorithm)?;
        }

        // Validate BK length
        if bk.len() < BK_AES_CBC_256_HMAC384_SIZE_BYTES {
            Err(HsmErr::InvalidKeyLength)?;
        }

        // Get the encrypted key length
        let encrypted_key_len = env.aescbc256_enc_data_len(MK_AES_CBC_256_HMAC384_SIZE_BYTES);

        let encoded_length = MaskedKey::encoded_length(
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            metadata.len(),
            encrypted_key_len,
        );

        if *bmk_len < encoded_length || bmk_out.len() < encoded_length {
            *bmk_len = encoded_length;
            Err(HsmErr::InsufficientBuffer)?;
        }

        // Pre-encode the masking key.
        let mut pre_encoded = MaskedKey::pre_encode(
            1,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            metadata.len(),
            encrypted_key_len,
            bmk_out[..encoded_length].as_mut(),
        )
        .map_err(|_| HsmErr::MaskedKeyPreEncodeFailed)?;

        // Encode the masking key into bmk_out.
        MaskedKey::encode(env, &mut pre_encoded, masking_key, bk, metadata)
            .map_err(|_| HsmErr::MaskedKeyEncodeFailed)?;

        *bmk_len = encoded_length;

        Ok(())
    }

    /// Encode masked key metadata into MBOR format.
    ///
    /// # Arguments
    /// * `svn` - The security version number (optional).
    /// * `key_kind` - The type of the key.
    /// * `key_attributes` - Attributes associated with the key.
    /// * `bks2_index` - The index of the BKS2 (optional).
    /// * `key_tag` - A tag associated with the key (optional).
    /// * `key_label` - A label for the key.
    /// * `metadata_len` - In/out parameter for the length of the encoded metadata.
    /// * `encoded_metadata` - Output buffer for the encoded metadata.
    /// * `key_len` - Optional key length in bytes for variable length HMAC keys.
    ///
    /// # Returns
    /// * `Ok(())` - If the metadata is successfully encoded.
    /// * `Err(HsmErr)` - If there is an error during the encoding process.
    #[allow(clippy::too_many_arguments)]
    pub fn mbor_encode_masked_key_metadata(
        svn: Option<u64>,
        key_kind: DdiKeyType,
        key_attributes: DdiMaskedKeyAttributes,
        bks2_index: Option<u16>,
        key_tag: Option<u16>,
        key_label: &[u8],
        metadata_len: &mut usize,
        encoded_metadata: &mut [u8],
        key_length: u16,
    ) -> Result<(), HsmErr> {
        // Mbor encode the metadata.
        let metadata = DdiMaskedKeyMetadata {
            svn,
            key_type: key_kind,
            key_attributes,
            bks2_index,
            key_tag,
            key_label: MborByteArray::new_with_len(key_label.as_ptr(), key_label.len()),
            key_length,
        };

        let mut accumulator = MborLenAccumulator::default();
        metadata.mbor_len(&mut accumulator);
        let len = accumulator.len();

        if len > *metadata_len || len > encoded_metadata.len() {
            *metadata_len = len;
            Err(HsmErr::InsufficientBuffer)?;
        }

        let mut encoder = MborEncoder::new(&mut encoded_metadata[..len]);
        metadata
            .mbor_encode(&mut encoder)
            .map_err(|_| HsmErr::MetadataEncodeFailed)?;

        if encoder.position() != len {
            Err(HsmErr::MetadataEncodeFailed)?;
        }

        *metadata_len = len;

        Ok(())
    }

    /// Decode masked key metadata from MBOR format.
    ///
    /// # Arguments
    /// * `encoded_metadata` - The encoded metadata in MBOR format.
    ///
    /// # Returns
    /// * `Ok(DdiMaskedKeyMetadata)` - The decoded metadata.
    /// * `Err(HsmErr)` - If there is an error during the decoding process.
    #[cfg(test)]
    fn decode_masked_key_metadata(encoded_metadata: &[u8]) -> Result<DdiMaskedKeyMetadata, HsmErr> {
        let mut decoder = MborDecoder::new(encoded_metadata);
        let metadata = DdiMaskedKeyMetadata::mbor_decode(&mut decoder)
            .map_err(|_| HsmErr::MetadataDecodeFailed)?;

        Ok(metadata)
    }

    /// Restore the Backup Masking Key (BMK).
    ///
    /// # Arguments
    /// * `env` - The cryptographic environment to use.
    /// * `bk` - The backup key.
    /// * `bmk` - The masked key containing the encrypted masking key.
    ///
    /// # Returns
    /// * `Ok(DecodedMaskedKey)` - The decoded masking key.
    /// * `Err(HsmErr)` - If there is an error during the restoration process.
    ///
    /// # Note
    /// decrypt_key is expected to be called on the returned DecodedMaskedKey to decrypt the masking key with BK.
    #[cfg(test)]
    pub fn bmk_restore<'a, Env: CryptEnv>(
        env: &Env,
        bk: &[u8],
        bmk: &'a [u8],
    ) -> Result<DecodedMaskedKey<'a>, HsmErr> {
        MaskedKey::decode(env, bk, bmk, true).map_err(|_| HsmErr::MaskedKeyDecodeFailed)
    }

    /// Generate an BK boot key for masking BK3.
    ///
    /// # Arguments
    /// * `env` - The cryptographic environment to use.
    /// * `algo` - The crypto algorithm to use for the BK boot key generation.
    /// * `bk_boot_key_len` - The length of the BK boot key to be generated.
    /// * `bk_boot_key_out` - Output buffer for the generated BK boot key.
    ///
    /// # Returns
    /// * `Ok(())` - If the BK boot key is successfully generated.
    /// * `Err(HsmErr)` - If there is an error during the generation process.
    pub fn bk_boot_key_gen<Env: CryptEnv>(
        env: &Env,
        algo: MaskingKeyAlgorithm,
        bk_boot_key_len: usize,
        bk_boot_key_out: &mut [u8],
    ) -> Result<(), HsmErr> {
        // Check BK boot key algo. Only AesCbc256Hmac384 is supported for now.
        if algo != MaskingKeyAlgorithm::AesCbc256Hmac384 {
            Err(HsmErr::InvalidAlgorithm)?;
        }

        // Validate BK boot key length.
        if bk_boot_key_len != (AES_CBC_256_KEY_SIZE + HMAC384_KEY_SIZE) {
            Err(HsmErr::InvalidKeyLength)?;
        }

        // Validate output buffer size.
        if bk_boot_key_out.len() < bk_boot_key_len {
            Err(HsmErr::InsufficientBuffer)?;
        }

        // Generate random seed for BK Boot key.
        let mut bk_boot_key_seed = [0u8; MK_SEED_SIZE_BYTES];
        env.generate_random(&mut bk_boot_key_seed)?;

        // Derive BK Boot key using KBKDF.
        env.kbkdf_sha384(
            &bk_boot_key_seed,
            Some(BK_BOOT_DEFAULT_LABEL),
            None,
            bk_boot_key_len,
            bk_boot_key_out,
        )?;

        Ok(())
    }

    /// Mask the BK3 (MBK3) with the BK Boot key.
    ///
    /// # Arguments
    /// * `env` - The cryptographic environment to use.
    /// * `algo` - The crypto algorithm to use for the masking the BK3.
    /// * `bk_boot_key` - The secret key to use for masking the BK3.
    /// * `bk3` - The backup key 3 (BK3) to be masked.
    /// * `metadata` - The metadata to be associated with the masked BK3.
    /// * `mbk3_len` - In/out parameter for the length of the masked BK3.
    /// * `mbk3_out` - Output buffer for the masked BK3.
    ///
    /// # Returns
    /// * `Ok(())` - If the BMK is successfully generated.
    /// * `Err(HsmErr)` - If there is an error during the generation process.
    pub fn masked_bk3_gen<Env: CryptEnv>(
        env: &Env,
        algo: MaskingKeyAlgorithm,
        bk_boot_key: &[u8],
        bk3: &[u8],
        metadata: &[u8],
        mbk3_len: &mut usize,
        mbk3_out: &mut [u8],
    ) -> Result<(), HsmErr> {
        // Check bk boot key type. Only AesCbc256Hmac384 is supported for now.
        if algo != MaskingKeyAlgorithm::AesCbc256Hmac384 {
            Err(HsmErr::InvalidAlgorithm)?;
        }

        // Validate bk boot key length.
        if bk_boot_key.len() != (AES_CBC_256_KEY_SIZE + HMAC384_KEY_SIZE) {
            Err(HsmErr::InvalidKeyLength)?;
        }

        // Get the encrypted key length
        let encrypted_key_len = env.aescbc256_enc_data_len(bk3.len());

        let encoded_length = MaskedKey::encoded_length(
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            metadata.len(),
            encrypted_key_len,
        );

        if *mbk3_len < encoded_length || mbk3_out.len() < encoded_length {
            *mbk3_len = encoded_length;
            Err(HsmErr::InsufficientBuffer)?;
        }

        // 1. Pre-encode BK3.
        let mut pre_encoded = MaskedKey::pre_encode(
            1,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            metadata.len(),
            encrypted_key_len,
            mbk3_out[..encoded_length].as_mut(),
        )
        .map_err(|_| HsmErr::MaskedKeyPreEncodeFailed)?;

        // 2. Encode the BK3 in mbk3_out buffer.
        MaskedKey::encode(env, &mut pre_encoded, bk3, bk_boot_key, metadata)
            .map_err(|_| HsmErr::MaskedKeyEncodeFailed)?;

        *mbk3_len = encoded_length;

        Ok(())
    }

    /// Mask the Ephemeral Key (BK BOOT) with a temporary secret key generated from the BK seeds and UEFI key.
    ///
    /// # Arguments
    /// * `env` - The cryptographic environment to use.
    /// * `algo` - The crypto algorithm to use for the masking the Bk Boot.
    /// * `bk_boot_key` - The key the masks bk3 in the boot process.
    /// * `bkx` - The key the masks the bk boot key in the boot process.
    /// * `metadata` - The metadata to be associated with the masked BK BOOT.
    /// * `mbkboot_len` - In/out parameter for the length of the masked BK BOOT.
    /// * `mbkboot_out` - Output buffer for the masked BK BOOT.
    ///
    /// # Returns
    /// * `Ok(())` - If the EPHMRK is successfully generated.
    /// * `Err(HsmErr)` - If there is an error during the generation process.
    #[allow(clippy::too_many_arguments)]
    pub fn masked_bkboot_gen<Env: CryptEnv>(
        env: &Env,
        algo: MaskingKeyAlgorithm,
        bk_boot_key: &[u8],
        bkx: &[u8; BK_BOOT_MK_AES_CBC_256_HMAC384_SIZE_BYTES],
        metadata: &[u8],
        mbkboot_len: &mut usize,
        mbkboot_out: &mut [u8],
    ) -> Result<(), HsmErr> {
        // Check bk boot key type. Only AesCbc256Hmac384 is supported for now.
        if algo != MaskingKeyAlgorithm::AesCbc256Hmac384 {
            Err(HsmErr::InvalidAlgorithm)?;
        }

        let encrypted_key_len = env.aescbc256_enc_data_len(bk_boot_key.len());

        let encoded_length = MaskedKey::encoded_length(
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            metadata.len(),
            encrypted_key_len,
        );

        if *mbkboot_len < encoded_length || mbkboot_out.len() < encoded_length {
            *mbkboot_len = encoded_length;
            Err(HsmErr::InsufficientBuffer)?;
        }

        // 2. Pre-encode the bk boot key.
        let mut pre_encoded = MaskedKey::pre_encode(
            1,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            metadata.len(),
            encrypted_key_len,
            mbkboot_out[..encoded_length].as_mut(),
        )
        .map_err(|_| HsmErr::MaskedKeyPreEncodeFailed)?;

        // 3. Encode the bk boot key into metadata.
        MaskedKey::encode(env, &mut pre_encoded, bk_boot_key, bkx, metadata)
            .map_err(|_| HsmErr::MaskedKeyEncodeFailed)?;

        *mbkboot_len = encoded_length;

        Ok(())
    }

    /// Generate the BKx key.
    ///
    /// This function generates the BKx key using the provided BKS1, BKS2, and FW_SECRET keys.
    ///
    /// # Arguments
    /// * `env` - The cryptographic environment.
    /// * `bks1` - The BKS1 key.
    /// * `bks2` - The BKS2 key.
    /// * `fw_secret` - The firmware secret key.
    /// * `masking_key_out` - The masking key buffer.
    pub fn generate_bkx<Env: CryptEnv>(
        env: &Env,
        bks1: &[u8; BK_SEED_SIZE_BYTES],
        bks2: &[u8; BK_SEED_SIZE_BYTES],
        fw_secret: &[u8; FW_SECRET_SIZE_BYTES],
        masking_key_out: &mut [u8; BK_BOOT_MK_AES_CBC_256_HMAC384_SIZE_BYTES],
    ) -> Result<(), HsmErr> {
        // BKS12 is the concatenation of BKS1 and BKS2 and is used as KBKDF context.
        let mut bks1_2 =
            SecureByteArray::<{ BK_SEED_SIZE_BYTES * 2 }>::new([0u8; BK_SEED_SIZE_BYTES * 2]);
        bks1_2[..BK_SEED_SIZE_BYTES].copy_from_slice(bks1);
        bks1_2[BK_SEED_SIZE_BYTES..].copy_from_slice(bks2);

        env.kbkdf_sha384(
            fw_secret,
            Some(BK_BOOT_MASKING_KEY_DEFAULT_LABEL),
            Some(bks1_2.as_slice()),
            BK_BOOT_MK_AES_CBC_256_HMAC384_SIZE_BYTES,
            masking_key_out.as_mut_slice(),
        )?;

        Ok(())
    }

    /// Generate a masking key
    ///
    /// This function generates a masking key with a default label and no context.
    /// # Arguments
    /// * `env` - The cryptographic environment.
    /// * `masking_key_out` - The masking key buffer.
    pub fn generate_mk<Env: CryptEnv>(
        env: &Env,
        masking_key_out: &mut [u8; MK_AES_CBC_256_HMAC384_SIZE_BYTES],
    ) -> Result<(), HsmErr> {
        let mut secret_key =
            SecureByteArray::<{ FW_SECRET_SIZE_BYTES }>::new([0u8; FW_SECRET_SIZE_BYTES]);
        env.generate_random(secret_key.as_mut_slice())?;

        env.kbkdf_sha384(
            secret_key.as_ref(),
            Some(MK_DEFAULT_LABEL),
            None,
            MK_AES_CBC_256_HMAC384_SIZE_BYTES,
            masking_key_out.as_mut_slice(),
        )?;

        Ok(())
    }

    /// Unmask the BK3 key.
    ///
    /// This function unmasks the BK3 key using the provided BKx and masked keys.
    ///
    /// # Arguments
    /// * `env` - The cryptographic environment.
    /// * `bkx` - The BKx key.
    /// * `masked_bk_boot` - The masked BKS boot key.
    /// * `masked_bk3` - The masked BK3 key.
    /// * `bk3` - The unmasked BK3 key.
    #[allow(clippy::too_many_arguments)]
    pub fn unmask_bk3<'a, Env: CryptEnv>(
        env: &Env,
        bkx: &[u8; BK_BOOT_MK_AES_CBC_256_HMAC384_SIZE_BYTES],
        masked_bk_boot: &'a [u8],
        bk_boot_key: &mut [u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES],
        masked_bk3: &'a [u8],
        bk3: &mut [u8],
    ) -> Result<(), HsmErr> {
        if bk3.len() != BK3_SIZE_BYTES {
            Err(HsmErr::InvalidArgument)?
        }

        // Decode and decrypt masked_bk_boot.
        let decoded_boot_key = MaskedKey::decode(env, bkx, masked_bk_boot, true)
            .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;

        decoded_boot_key
            .decrypt_key(env, bkx, bk_boot_key)
            .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;

        // Decode and decrypt masked_bk3 with unmasked BK boot key.
        let decoded_bk3 = MaskedKey::decode(env, bk_boot_key, masked_bk3, true)
            .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;

        decoded_bk3
            .decrypt_key(env, bk_boot_key, bk3)
            .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;

        Ok(())
    }

    /// Derive BK3 Session Key from BK3 Parition Key.
    ///
    /// # Arguments
    ///
    /// * `crypto_env` - The cryptographic environment providing KBKDF-SHA384 functionality
    /// * `bk3_partition` - The master BK3 partition key used as the key derivation key (KDK)
    /// * `bk3_session_len` - Mutable reference to store the length of the generated session BK3.
    ///   On input, it specifies the size of the `bk_session_out` buffer.
    ///   On output, it contains the actual length of the derived session BK3.
    /// * `bk_session_out` - Output buffer for the generated session-specific BK3 key.
    ///   Must be at least BK3_SIZE_BYTES in length.
    ///
    /// # Returns
    ///
    /// * `Ok(())` - Session BK3 successfully derived and written to output buffer
    /// * `Err(HsmErr)` - If there is an error during the generation process.
    pub fn bk3_session_gen<Env: CryptEnv>(
        crypto_env: &Env,
        bk3_partition: &[u8; BK3_SIZE_BYTES],
        bk3_session_len: &mut usize,
        bk3_session_out: &mut [u8],
    ) -> Result<(), HsmErr> {
        // Validate input parameter
        if *bk3_session_len < BK3_SIZE_BYTES || bk3_session_out.len() < BK3_SIZE_BYTES {
            *bk3_session_len = BK3_SIZE_BYTES;
            Err(HsmErr::InsufficientBuffer)?
        }

        // Use KBKDF-SHA384 to derive BK3 session key from BK3 partition key.
        crypto_env.kbkdf_sha384(
            bk3_partition,
            Some(SESSION_BK3_LABEL),
            None,
            BK3_SIZE_BYTES,
            &mut bk3_session_out[..BK3_SIZE_BYTES],
        )?;

        *bk3_session_len = BK3_SIZE_BYTES;

        Ok(())
    }

    /// Generate the Session Backup Key using the provided bks1, bks2, session seed and the bk3 session key.
    ///
    /// # Arguments
    ///
    /// * `crypto_env` - The cryptographic environment to use.
    /// * `algo` - Indicates the type of the BK to be generated.
    /// * `session_seed` - Unique session value.
    /// * `bks1` - The first backup seed (BKS1).
    /// * `bks2` - The second backup seed (BKS2).
    /// * `bk3_session` - The session backup key 3 (BK3 Session).
    /// * `bk_session_len` - In/out parameter for the length of the session backup key.
    ///   On input, it specifies the bk_session_out buffer size.
    ///   On output, it will contain the actual length of the generated backup key.
    /// * `bk_session_out` - Output buffer for the generated session backup key.
    ///
    /// # Returns
    /// * `Ok(())` - If the session backup key is successfully generated.
    /// * `Err(HsmErr)` - If there is an error during the generation process.
    #[allow(clippy::too_many_arguments)]
    pub fn bk_session_gen<Env: CryptEnv>(
        crypto_env: &Env,
        algo: MaskingKeyAlgorithm,
        session_seed: &[u8; SESSION_SEED_SIZE_BYTES],
        bks1: &[u8; BK_SEED_SIZE_BYTES],
        bks2: &[u8; BK_SEED_SIZE_BYTES],
        bk3_session: &[u8; BK3_SIZE_BYTES],
        bk_session_len: &mut usize,
        bk_session_out: &mut [u8],
    ) -> Result<(), HsmErr> {
        // Check BK session key algo. Only AesCbc256Hmac384 is supported for now.
        if algo != MaskingKeyAlgorithm::AesCbc256Hmac384 {
            Err(HsmErr::InvalidAlgorithm)?;
        }

        if *bk_session_len < BK_AES_CBC_256_HMAC384_SIZE_BYTES
            || bk_session_out.len() < BK_AES_CBC_256_HMAC384_SIZE_BYTES
        {
            *bk_session_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
            Err(HsmErr::InsufficientBuffer)?;
        }

        // BKS12_SEED is the concatenation of BKS1, BKS2 and Session seed and is used as KBKDF context.
        const BKS1_2_SEED_LEN: usize = BK_SEED_SIZE_BYTES * 2 + SESSION_SEED_SIZE_BYTES;
        let mut bks1_2_seed = SecureByteArray::<{ BKS1_2_SEED_LEN }>::new([0u8; BKS1_2_SEED_LEN]);

        bks1_2_seed[..BK_SEED_SIZE_BYTES].copy_from_slice(bks1);
        bks1_2_seed[BK_SEED_SIZE_BYTES..BK_SEED_SIZE_BYTES * 2].copy_from_slice(bks2);
        bks1_2_seed[BK_SEED_SIZE_BYTES * 2..].copy_from_slice(session_seed);

        // Derive BK Session via KBKDF; BK3 Session key is used as the the KBKDF key and BKS12_SEED as the context.
        crypto_env.kbkdf_sha384(
            bk3_session,
            Some(SESSION_BK_LABEL),
            Some(bks1_2_seed.as_slice()),
            BK_AES_CBC_256_HMAC384_SIZE_BYTES,
            &mut bk_session_out[..BK_AES_CBC_256_HMAC384_SIZE_BYTES],
        )?;

        *bk_session_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use mcr_ddi_mbor::MborDecode;
    use mcr_ddi_mbor::MborDecoder;
    use mcr_ddi_types::MaskedKeyError;
    use mcr_types::SecureByteArray;
    use sha2::Digest;
    use sha2::Sha384;

    use crate::error::HsmErr;

    use super::*;

    const TEST_BKS1: [u8; BK_SEED_SIZE_BYTES] = [0x01; BK_SEED_SIZE_BYTES];
    const TEST_BKS2: [u8; BK_SEED_SIZE_BYTES] = [0x02; BK_SEED_SIZE_BYTES];
    const TEST_BK3_PARTITION: [u8; BK3_SIZE_BYTES] = [0x03; BK3_SIZE_BYTES];
    const TEST_BK3_SESSION: [u8; BK3_SIZE_BYTES] = [0x0B; BK3_SIZE_BYTES];
    const TEST_SESSION_SEED: [u8; SESSION_SEED_SIZE_BYTES] = [0x04; SESSION_SEED_SIZE_BYTES];
    const TEST_BK_BOOT_KEY: [u8; AES_CBC_256_KEY_SIZE + HMAC384_KEY_SIZE] =
        [0xEE; AES_CBC_256_KEY_SIZE + HMAC384_KEY_SIZE];
    const TEST_UEFI_KEY: [u8; FW_SECRET_SIZE_BYTES] = [0xFF; FW_SECRET_SIZE_BYTES];
    const TEST_OUTPUT_BUFFER_SIZE: usize = 1024;
    const TEST_METADATA_MAX_SIZE_BYTES: usize = 128;

    #[allow(unused)]
    const TEST_POTA_ECC_PRIVATE_KEY: [u8; 185] = [
        0x30, 0x81, 0xb6, 0x02, 0x01, 0x00, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d,
        0x02, 0x01, 0x06, 0x05, 0x2b, 0x81, 0x04, 0x00, 0x22, 0x04, 0x81, 0x9e, 0x30, 0x81, 0x9b,
        0x02, 0x01, 0x01, 0x04, 0x30, 0x17, 0xe9, 0x1c, 0xac, 0xf7, 0xb7, 0x21, 0xd7, 0x75, 0x20,
        0x02, 0x07, 0xbc, 0xaa, 0x94, 0x2c, 0xe3, 0xb5, 0x5b, 0x78, 0x13, 0xcc, 0x8b, 0xde, 0x87,
        0x65, 0x6b, 0xe1, 0x7b, 0xc2, 0xa8, 0xcc, 0x89, 0x33, 0x4e, 0xcd, 0xaa, 0x9d, 0x1d, 0x09,
        0xf1, 0xc7, 0x01, 0x1b, 0x64, 0xeb, 0x78, 0x5b, 0xa1, 0x64, 0x03, 0x62, 0x00, 0x04, 0x1f,
        0x42, 0x0d, 0x73, 0xeb, 0xf0, 0x67, 0xc2, 0xf9, 0x77, 0xbd, 0x51, 0xab, 0xfb, 0xe1, 0xf6,
        0x53, 0x19, 0xb7, 0x57, 0xe0, 0xa9, 0x20, 0xce, 0x4f, 0x21, 0xbb, 0xd4, 0xa7, 0x84, 0x1c,
        0x93, 0x45, 0xf1, 0xea, 0xd9, 0x5f, 0xe5, 0x90, 0xab, 0x57, 0xe1, 0xea, 0xfc, 0xd2, 0x06,
        0xef, 0x21, 0xa2, 0xad, 0x10, 0xd3, 0x17, 0x6e, 0x99, 0xc8, 0x22, 0x26, 0x23, 0x08, 0x57,
        0xa7, 0x56, 0x08, 0x45, 0xe3, 0xda, 0x12, 0xc7, 0xdc, 0x3a, 0xee, 0x01, 0xfc, 0x37, 0xab,
        0x1c, 0x8d, 0xc6, 0xd0, 0x64, 0x7a, 0x7d, 0xc2, 0x67, 0xfc, 0x02, 0x7d, 0x8d, 0xa3, 0xc8,
        0x01, 0x4b, 0xa4, 0x0d, 0x98,
    ];

    const TEST_POTA_ECC_PUB_KEY: [u8; 120] = [
        0x30, 0x76, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x05,
        0x2b, 0x81, 0x04, 0x00, 0x22, 0x03, 0x62, 0x00, 0x04, 0x1f, 0x42, 0x0d, 0x73, 0xeb, 0xf0,
        0x67, 0xc2, 0xf9, 0x77, 0xbd, 0x51, 0xab, 0xfb, 0xe1, 0xf6, 0x53, 0x19, 0xb7, 0x57, 0xe0,
        0xa9, 0x20, 0xce, 0x4f, 0x21, 0xbb, 0xd4, 0xa7, 0x84, 0x1c, 0x93, 0x45, 0xf1, 0xea, 0xd9,
        0x5f, 0xe5, 0x90, 0xab, 0x57, 0xe1, 0xea, 0xfc, 0xd2, 0x06, 0xef, 0x21, 0xa2, 0xad, 0x10,
        0xd3, 0x17, 0x6e, 0x99, 0xc8, 0x22, 0x26, 0x23, 0x08, 0x57, 0xa7, 0x56, 0x08, 0x45, 0xe3,
        0xda, 0x12, 0xc7, 0xdc, 0x3a, 0xee, 0x01, 0xfc, 0x37, 0xab, 0x1c, 0x8d, 0xc6, 0xd0, 0x64,
        0x7a, 0x7d, 0xc2, 0x67, 0xfc, 0x02, 0x7d, 0x8d, 0xa3, 0xc8, 0x01, 0x4b, 0xa4, 0x0d, 0x98,
    ];

    #[allow(dead_code)]
    enum CryptoFunc {
        Hmac384Tag,
        AesCbc256Encrypt,
        AesCbc256Decrypt,
        KbkdfSha384,
        GenerateRandom,
    }

    struct TestCryptoEnv {
        plaintext: Vec<u8>,
        ciphertext: Vec<u8>,
        hmac384_tag: [u8; 48],
        error: Option<(CryptoFunc, HsmErr)>,
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
            if let Some((CryptoFunc::Hmac384Tag, err)) = self.error {
                return Err(err);
            }
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
            if let Some((CryptoFunc::AesCbc256Encrypt, err)) = self.error {
                return Err(err);
            }
            Ok(ciphertext.len())
        }

        fn aescbc256_decrypt(
            &self,
            _key: &[u8],
            _iv: &[u8],
            _ciphertext: &[u8],
            plaintext: &mut [u8],
        ) -> Result<usize, HsmErr> {
            if let Some((CryptoFunc::AesCbc256Decrypt, err)) = self.error {
                return Err(err);
            }
            plaintext.copy_from_slice(&self.plaintext.as_slice()[..plaintext.len()]);
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
            if let Some((CryptoFunc::KbkdfSha384, err)) = self.error {
                return Err(err);
            }

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
            if let Some((CryptoFunc::GenerateRandom, err)) = self.error {
                return Err(err);
            }

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
    fn test_bk_partition_gen_success() {
        let crypto_env = TestCryptoEnv::new();

        let mut bk_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
        let mut bk_out = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];

        let result = LMKeyDerive::bk_partition_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_PARTITION,
            &TEST_POTA_ECC_PUB_KEY,
            &mut bk_len,
            &mut bk_out,
        );

        assert!(result.is_ok(), "bk_partition_gen should succeed");
        assert_eq!(bk_len, BK_AES_CBC_256_HMAC384_SIZE_BYTES);

        // Verify the output is not all zeros (indicating successful derivation)
        assert!(
            bk_out.iter().any(|&b| b != 0),
            "Derived BK should not be all zeros"
        );
    }

    #[test]
    fn test_bk_partition_gen_deterministic() {
        let crypto_env = TestCryptoEnv::new();

        // Generate BK twice with same inputs
        let mut bk_len1 = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
        let mut bk_out1 = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];

        let mut bk_len2 = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
        let mut bk_out2 = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];

        let result1 = LMKeyDerive::bk_partition_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_PARTITION,
            &TEST_POTA_ECC_PUB_KEY,
            &mut bk_len1,
            &mut bk_out1,
        );

        let result2 = LMKeyDerive::bk_partition_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_PARTITION,
            &TEST_POTA_ECC_PUB_KEY,
            &mut bk_len2,
            &mut bk_out2,
        );

        assert!(
            result1.is_ok() && result2.is_ok(),
            "Both BK generations should succeed"
        );
        assert_eq!(bk_out1, bk_out2, "BK derivation should be deterministic");
    }

    #[test]
    fn test_bk_partition_gen_different_inputs_different_outputs() {
        let crypto_env = TestCryptoEnv::new();

        // Different BK3
        let bk3_alt = [0x04; BK3_SIZE_BYTES];

        let mut bk_out1 = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let mut bk_out2 = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let mut bk_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;

        let result1 = LMKeyDerive::bk_partition_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_PARTITION,
            &TEST_POTA_ECC_PUB_KEY,
            &mut bk_len,
            &mut bk_out1,
        );
        assert!(result1.is_ok(), "bk_partition_gen should succeed");

        let result2 = LMKeyDerive::bk_partition_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BKS1,
            &TEST_BKS2,
            &bk3_alt,
            &TEST_POTA_ECC_PUB_KEY,
            &mut bk_len,
            &mut bk_out2,
        );
        assert!(
            result2.is_ok(),
            "bk_partition_gen with different BK3 should succeed"
        );

        assert_ne!(
            bk_out1, bk_out2,
            "Different inputs should produce different BKs"
        );
    }

    #[test]
    fn test_bk_partition_gen_insufficient_buffer() {
        let crypto_env = TestCryptoEnv::new();

        let mut bk_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES - 1; // Too small
        let mut bk_out = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES - 1];

        let result = LMKeyDerive::bk_partition_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_PARTITION,
            &TEST_POTA_ECC_PUB_KEY,
            &mut bk_len,
            &mut bk_out,
        );

        assert!(matches!(result, Err(HsmErr::InsufficientBuffer)));
        assert_eq!(bk_len, BK_AES_CBC_256_HMAC384_SIZE_BYTES);
    }

    #[test]
    fn test_bk3_session_gen_success() {
        let crypto_env = TestCryptoEnv::new();
        let bk3_partition = [0x42; BK3_SIZE_BYTES];
        let mut bk3_session_len = BK3_SIZE_BYTES;
        let mut bk_session_out = vec![0u8; BK3_SIZE_BYTES];

        let result = LMKeyDerive::bk3_session_gen(
            &crypto_env,
            &bk3_partition,
            &mut bk3_session_len,
            &mut bk_session_out,
        );

        assert!(result.is_ok());
        assert_eq!(bk3_session_len, BK3_SIZE_BYTES);
        // Verify output is not all zeros.
        assert_ne!(bk_session_out, vec![0u8; BK3_SIZE_BYTES]);
    }

    #[test]
    fn test_bk3_session_gen_buffer_too_small() {
        let crypto_env = TestCryptoEnv::new();
        let bk3_partition = [0x42; BK3_SIZE_BYTES];
        let mut bk3_session_len = BK3_SIZE_BYTES - 1; // Too small
        let mut bk_session_out = vec![0u8; BK3_SIZE_BYTES - 1]; // Too small

        let result = LMKeyDerive::bk3_session_gen(
            &crypto_env,
            &bk3_partition,
            &mut bk3_session_len,
            &mut bk_session_out,
        );

        assert!(result.is_err());
        assert_eq!(result.unwrap_err(), HsmErr::InsufficientBuffer);
        assert_eq!(bk3_session_len, BK3_SIZE_BYTES);
    }

    #[test]
    fn test_bk3_session_gen_output_buffer_too_small() {
        let crypto_env = TestCryptoEnv::new();
        let bk3_partition = [0x42; BK3_SIZE_BYTES];
        let mut bk3_session_len = BK3_SIZE_BYTES; // Correct size
        let mut bk_session_out = vec![0u8; BK3_SIZE_BYTES - 1]; // Too small

        let result = LMKeyDerive::bk3_session_gen(
            &crypto_env,
            &bk3_partition,
            &mut bk3_session_len,
            &mut bk_session_out,
        );

        assert!(result.is_err());
        assert_eq!(result.unwrap_err(), HsmErr::InsufficientBuffer);
    }

    #[test]
    fn test_bk3_session_gen_deterministic() {
        let crypto_env = TestCryptoEnv::new();
        let bk3_partition = [0x55; BK3_SIZE_BYTES];

        // First derivation
        let mut bk3_session_len1 = BK3_SIZE_BYTES;
        let mut bk_session_out1 = vec![0u8; BK3_SIZE_BYTES];
        let result1 = LMKeyDerive::bk3_session_gen(
            &crypto_env,
            &bk3_partition,
            &mut bk3_session_len1,
            &mut bk_session_out1,
        );

        // Second derivation with same inputs
        let mut bk3_session_len2 = BK3_SIZE_BYTES;
        let mut bk_session_out2 = vec![0u8; BK3_SIZE_BYTES];
        let result2 = LMKeyDerive::bk3_session_gen(
            &crypto_env,
            &bk3_partition,
            &mut bk3_session_len2,
            &mut bk_session_out2,
        );

        assert!(result1.is_ok());
        assert!(result2.is_ok());
        assert_eq!(bk_session_out1, bk_session_out2);
    }

    #[test]
    fn test_bk3_session_gen_different_inputs_different_outputs() {
        let crypto_env = TestCryptoEnv::new();

        // First partition key
        let bk3_partition1 = [0x11; BK3_SIZE_BYTES];
        let mut bk3_session_len1 = BK3_SIZE_BYTES;
        let mut bk_session_out1 = vec![0u8; BK3_SIZE_BYTES];
        let result1 = LMKeyDerive::bk3_session_gen(
            &crypto_env,
            &bk3_partition1,
            &mut bk3_session_len1,
            &mut bk_session_out1,
        );

        // Second partition key (different)
        let bk3_partition2 = [0x22; BK3_SIZE_BYTES];
        let mut bk3_session_len2 = BK3_SIZE_BYTES;
        let mut bk_session_out2 = vec![0u8; BK3_SIZE_BYTES];
        let result2 = LMKeyDerive::bk3_session_gen(
            &crypto_env,
            &bk3_partition2,
            &mut bk3_session_len2,
            &mut bk_session_out2,
        );

        assert!(result1.is_ok());
        assert!(result2.is_ok());
        assert_ne!(bk_session_out1, bk_session_out2);
    }

    #[test]
    fn test_bk3_session_gen_large_buffer() {
        let crypto_env = TestCryptoEnv::new();
        let bk3_partition = [0x77; BK3_SIZE_BYTES];
        let mut bk3_session_len = BK3_SIZE_BYTES * 2; // Larger than needed
        let mut bk_session_out = vec![0u8; BK3_SIZE_BYTES * 2]; // Larger buffer

        let result = LMKeyDerive::bk3_session_gen(
            &crypto_env,
            &bk3_partition,
            &mut bk3_session_len,
            &mut bk_session_out,
        );

        assert!(result.is_ok());
        assert_eq!(bk3_session_len, BK3_SIZE_BYTES); // Should be set to actual output size
                                                     // Only first BK3_SIZE_BYTES should be modified
        assert_ne!(
            &bk_session_out[..BK3_SIZE_BYTES],
            &vec![0u8; BK3_SIZE_BYTES][..]
        );
        assert_eq!(
            &bk_session_out[BK3_SIZE_BYTES..],
            &vec![0u8; BK3_SIZE_BYTES][..]
        );
    }

    #[test]
    fn test_bk3_session_gen_max_partition_key() {
        let crypto_env = TestCryptoEnv::new();
        let bk3_partition = [0xFF; BK3_SIZE_BYTES];
        let mut bk3_session_len = BK3_SIZE_BYTES;
        let mut bk_session_out = vec![0u8; BK3_SIZE_BYTES];

        let result = LMKeyDerive::bk3_session_gen(
            &crypto_env,
            &bk3_partition,
            &mut bk3_session_len,
            &mut bk_session_out,
        );

        assert!(result.is_ok());
        assert_eq!(bk3_session_len, BK3_SIZE_BYTES);
        assert_ne!(bk_session_out, vec![0u8; BK3_SIZE_BYTES]);
    }

    #[test]
    fn test_bk_session_gen_success() {
        let crypto_env = TestCryptoEnv::new();

        let mut bk_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
        let mut bk_out = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];

        let result = LMKeyDerive::bk_session_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_SESSION_SEED,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_SESSION,
            &mut bk_len,
            &mut bk_out,
        );

        assert!(result.is_ok(), "bk_session_gen should succeed");
        assert_eq!(bk_len, BK_AES_CBC_256_HMAC384_SIZE_BYTES);

        // Verify the output is not all zeros (indicating successful derivation)
        assert!(
            bk_out.iter().any(|&b| b != 0),
            "Derived Session BK should not be all zeros"
        );
    }

    #[test]
    fn test_bk_session_gen_deterministic() {
        let crypto_env = TestCryptoEnv::new();

        // Generate BK Session twice with same inputs
        let mut bk_len1 = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
        let mut bk_out1 = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];

        let mut bk_len2 = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
        let mut bk_out2 = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];

        let result1 = LMKeyDerive::bk_session_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_SESSION_SEED,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_SESSION,
            &mut bk_len1,
            &mut bk_out1,
        );

        let result2 = LMKeyDerive::bk_session_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_SESSION_SEED,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_SESSION,
            &mut bk_len2,
            &mut bk_out2,
        );

        assert!(
            result1.is_ok() && result2.is_ok(),
            "Both BK Session generations should succeed"
        );
        assert_eq!(
            bk_out1, bk_out2,
            "BK Session derivation should be deterministic"
        );
    }

    #[test]
    fn test_bk_session_gen_different_inputs_different_outputs() {
        let crypto_env = TestCryptoEnv::new();

        // Different BK3
        let session_alt = [0x05; SESSION_SEED_SIZE_BYTES];

        let mut bk_out1 = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let mut bk_out2 = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let mut bk_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;

        let result1 = LMKeyDerive::bk_session_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_SESSION_SEED,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_SESSION,
            &mut bk_len,
            &mut bk_out1,
        );
        assert!(result1.is_ok(), "bk_session_gen should succeed");

        let result2 = LMKeyDerive::bk_session_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &session_alt,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_SESSION,
            &mut bk_len,
            &mut bk_out2,
        );
        assert!(
            result2.is_ok(),
            "bk_session_gen with different BK3 should succeed"
        );

        assert_ne!(
            bk_out1, bk_out2,
            "Different inputs should produce different BKs"
        );
    }

    #[test]
    fn test_bk_session_gen_insufficient_buffer() {
        let crypto_env = TestCryptoEnv::new();

        let mut bk_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES - 1; // Too small
        let mut bk_out = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES - 1];

        let result = LMKeyDerive::bk_session_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_SESSION_SEED,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_SESSION,
            &mut bk_len,
            &mut bk_out,
        );

        assert!(matches!(result, Err(HsmErr::InsufficientBuffer)));
        assert_eq!(bk_len, BK_AES_CBC_256_HMAC384_SIZE_BYTES);
    }

    #[test]
    fn test_bmk_gen_success() {
        let crypto_env = TestCryptoEnv::new();

        // First generate a BK
        let mut bk_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
        let mut bk = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];

        let result = LMKeyDerive::bk_partition_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_PARTITION,
            &TEST_POTA_ECC_PUB_KEY,
            &mut bk_len,
            &mut bk,
        );
        assert!(result.is_ok(), "BK generation should succeed");

        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            Some(1),
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test BMK",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        // Get the required length for BMK
        let mut bmk_len = 0;
        let masking_key = [0u8; MK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let result = LMKeyDerive::bmk_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bk,
            &masking_key,
            &metadata[..metadata.len()],
            &mut bmk_len,
            &mut [0u8; 0],
        );
        assert!(
            result.is_err(),
            "BMK generation should fail due to insufficient buffer"
        );
        assert!(matches!(result, Err(HsmErr::InsufficientBuffer)));

        // Now set bmk_len to the required size and try again
        let required_len = bmk_len;
        let mut bmk_out = vec![0u8; bmk_len];
        let masking_key = [0u8; MK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let result = LMKeyDerive::bmk_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bk,
            &masking_key,
            &metadata[..metadata.len()],
            &mut bmk_len,
            &mut bmk_out,
        );

        assert!(result.is_ok(), "BMK generation should succeed");
        assert_eq!(bmk_len, required_len);

        // Verify the output is not all zeros
        assert!(
            bmk_out.iter().any(|&b| b != 0),
            "Generated BMK should not be all zeros"
        );
    }

    #[test]
    fn test_bmk_gen_insufficient_buffer() {
        let crypto_env = TestCryptoEnv::new();
        let bk = [0x01; BK_AES_CBC_256_HMAC384_SIZE_BYTES];

        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            Some(1),
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test BMK",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        let mut bmk_len = 10; // Too small
        let mut bmk_out = vec![0u8; 10];

        let masking_key = [0u8; MK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let result = LMKeyDerive::bmk_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bk,
            &masking_key,
            &metadata[..metadata.len()],
            &mut bmk_len,
            &mut bmk_out,
        );

        assert!(matches!(result, Err(HsmErr::InsufficientBuffer)));
        // bmk_len should be updated to the required size
        assert!(bmk_len > 10);
    }

    #[test]
    fn test_bmk_gen_invalid_bk_length() {
        let crypto_env = TestCryptoEnv::new();
        let bk_short = [0x01; BK_AES_CBC_256_HMAC384_SIZE_BYTES - 1]; // Too short

        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            Some(1),
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test BMK",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        let mut bmk_len = 1000;
        let mut bmk_out = vec![0u8; 1000];

        let masking_key = [0u8; MK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let result = LMKeyDerive::bmk_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bk_short,
            &masking_key,
            &metadata[..metadata.len()],
            &mut bmk_len,
            &mut bmk_out,
        );

        assert!(matches!(result, Err(HsmErr::InvalidKeyLength)));
    }

    #[test]
    fn test_bmk_restore_success() {
        let crypto_env = TestCryptoEnv::new();

        // Generate BK
        let mut bk_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
        let mut bk = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];

        let result = LMKeyDerive::bk_partition_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_PARTITION,
            &TEST_POTA_ECC_PUB_KEY,
            &mut bk_len,
            &mut bk,
        );
        assert!(result.is_ok(), "BK generation should succeed");

        let svn = 42;
        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            Some(svn),
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test BMK",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        // Get the required length for BMK
        let mut bmk_len = 0;
        let masking_key = [0u8; MK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let result = LMKeyDerive::bmk_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bk,
            &masking_key,
            &metadata[..metadata.len()],
            &mut bmk_len,
            &mut [0u8; 0],
        );
        assert!(
            result.is_err(),
            "BMK generation should fail due to insufficient buffer"
        );
        assert!(matches!(result, Err(HsmErr::InsufficientBuffer)));

        // Now set bmk_len to the required size and try again
        let required_len = bmk_len;
        let mut bmk = vec![0u8; bmk_len];
        let result = LMKeyDerive::bmk_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bk,
            &masking_key,
            &metadata[..metadata.len()],
            &mut bmk_len,
            &mut bmk,
        );

        assert!(result.is_ok(), "BMK generation should succeed");
        assert_eq!(bmk_len, required_len);

        // Now test restore
        let result = LMKeyDerive::bmk_restore(&crypto_env, &bk, &bmk);

        assert!(result.is_ok(), "BMK restore should succeed");
        let decoded_key = result.unwrap();

        // Extract the AES key for individual field verification
        let aes_key = decoded_key.as_aes().unwrap();

        // Convert the metadata to a DdiMaskedKeyMetadata
        let decoded_data = LMKeyDerive::decode_masked_key_metadata(aes_key.metadata());
        assert!(decoded_data.is_ok(), "Metadata should decode successfully");

        let decoded_metadata = decoded_data.unwrap();
        assert_eq!(decoded_metadata.key_type, DdiKeyType::AesCbc256Hmac384);
        assert_eq!(decoded_metadata.svn, Some(svn));
        assert_eq!(decoded_metadata.key_label.as_slice(), b"Test BMK");
    }

    #[test]
    fn test_bmk_restore_with_wrong_bk() {
        let mut crypto_env = TestCryptoEnv::new();

        // Generate BK
        let mut bk_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
        let mut bk = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];

        let result = LMKeyDerive::bk_partition_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_PARTITION,
            &TEST_POTA_ECC_PUB_KEY,
            &mut bk_len,
            &mut bk,
        );
        assert!(result.is_ok());

        let svn: u64 = 100;
        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            Some(svn),
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test BMK",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        // Get the required length for BMK
        let mut bmk_len = 0;
        let masking_key = [0u8; MK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let result = LMKeyDerive::bmk_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bk,
            &masking_key,
            &metadata[..metadata.len()],
            &mut bmk_len,
            &mut [0u8; 0],
        );
        assert!(
            result.is_err(),
            "BMK generation should fail due to insufficient buffer"
        );
        assert!(matches!(result, Err(HsmErr::InsufficientBuffer)));

        // Now set bmk_len to the required size and try again
        let required_len = bmk_len;
        let mut bmk = vec![0u8; bmk_len];
        let masking_key = [0u8; MK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let result = LMKeyDerive::bmk_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bk,
            &masking_key,
            &metadata[..metadata.len()],
            &mut bmk_len,
            &mut bmk,
        );

        assert!(result.is_ok(), "BMK generation should succeed");
        assert_eq!(bmk_len, required_len);

        // Try to restore with wrong BK
        let wrong_bk = [0xFF; BK_AES_CBC_256_HMAC384_SIZE_BYTES];
        crypto_env.error = Some((CryptoFunc::AesCbc256Decrypt, HsmErr::AesDecryptFailed));
        let result = LMKeyDerive::bmk_restore(&crypto_env, &wrong_bk, &bmk);
        assert!(result.is_ok(), "BMK restore should succeed");
        let decoded_key = result.unwrap();

        let mut decrypted_key = vec![0u8; bmk_len];
        let result = decoded_key.decrypt_key(&crypto_env, &wrong_bk, decrypted_key.as_mut_slice());
        assert!(
            matches!(result, Err(MaskedKeyError::AesDecryptionFailed)),
            "Restore with wrong BK should fail"
        );
    }

    #[test]
    fn test_full_live_migration_workflow() {
        let crypto_env = TestCryptoEnv::new();

        // Step 1: Generate BK (Backup Key)
        let mut bk_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
        let mut bk = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];

        let result = LMKeyDerive::bk_partition_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BKS1,
            &TEST_BKS2,
            &TEST_BK3_PARTITION,
            &TEST_POTA_ECC_PUB_KEY,
            &mut bk_len,
            &mut bk,
        );
        assert!(result.is_ok(), "Step 1: BK generation should succeed");

        let svn: u64 = 2;
        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            Some(svn),
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test BMK",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        // Step 2: Generate BMK (Backup Masking Key)
        // Get the required length for BMK
        let mut bmk_len = 0;
        let masking_key = [0u8; MK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let result = LMKeyDerive::bmk_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bk,
            &masking_key,
            &metadata[..metadata.len()],
            &mut bmk_len,
            &mut [0u8; 0],
        );
        assert!(
            result.is_err(),
            "BMK generation should fail due to insufficient buffer"
        );
        assert!(matches!(result, Err(HsmErr::InsufficientBuffer)));

        // Now set bmk_len to the required size and try again
        let required_len = bmk_len;
        let mut bmk = vec![0u8; bmk_len];
        let result = LMKeyDerive::bmk_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bk,
            &masking_key,
            &metadata[..metadata.len()],
            &mut bmk_len,
            &mut bmk,
        );

        assert!(result.is_ok(), "Step 2: BMK generation should succeed");
        assert_eq!(bmk_len, required_len);

        // Step 3: Restore MK (Masking Key) from BMK
        let result = LMKeyDerive::bmk_restore(&crypto_env, &bk, &bmk);
        assert!(result.is_ok(), "Step 3: BMK restore should succeed");

        let decoded_key = result.unwrap();

        // Extract the AES key for individual field verification
        let aes_key = decoded_key.as_aes().unwrap();
        let metadata = aes_key.metadata();

        // Convert the metadata to a DdiMaskedKeyMetadata
        let mut decoder = MborDecoder::new(metadata);
        let decoded_data = DdiMaskedKeyMetadata::mbor_decode(&mut decoder);
        assert!(decoded_data.is_ok(), "Metadata should decode successfully");
        let decoded_metadata = decoded_data.unwrap();
        assert_eq!(decoded_metadata.key_type, DdiKeyType::AesCbc256Hmac384);
        assert_eq!(decoded_metadata.svn, Some(svn));
        assert_eq!(decoded_metadata.key_label.as_slice(), b"Test BMK");
    }

    #[test]
    fn test_bk_boot_key_gen_invalid_algorithm() {
        let crypto_env = TestCryptoEnv::new();
        let mut output = vec![0u8; 32];

        let result = LMKeyDerive::bk_boot_key_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesGcm256, // Unsupported algorithm
            32,
            &mut output,
        );

        assert!(result.is_err(), "Should return error for invalid algorithm");
        assert!(matches!(result, Err(HsmErr::InvalidAlgorithm)));
    }

    #[test]
    fn test_bk_boot_key_gen_invalid_key_length() {
        let crypto_env = TestCryptoEnv::new();
        let mut output = vec![0u8; 16]; // Invalid length for AesCbc256Hmac384

        let result = LMKeyDerive::bk_boot_key_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            16, // Invalid length
            &mut output,
        );

        assert!(
            result.is_err(),
            "Should return error for invalid key length"
        );
        assert!(matches!(result, Err(HsmErr::InvalidKeyLength)));
    }

    #[test]
    fn test_bk_boot_key_gen_insufficient_buffer() {
        let crypto_env = TestCryptoEnv::new();
        let mut output = vec![0u8; AES_CBC_256_KEY_SIZE + HMAC384_KEY_SIZE - 1];
        let result = LMKeyDerive::bk_boot_key_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            AES_CBC_256_KEY_SIZE + HMAC384_KEY_SIZE,
            &mut output,
        );
        assert!(
            result.is_err(),
            "Should return error for insufficient buffer"
        );
        assert!(matches!(result, Err(HsmErr::InsufficientBuffer)));
    }

    #[test]
    fn test_bk_boot_key_gen_crypto_env_random_error() {
        let mut crypto_env = TestCryptoEnv::new();
        let mut output = vec![0u8; AES_CBC_256_KEY_SIZE + HMAC384_KEY_SIZE];

        crypto_env.error = Some((CryptoFunc::GenerateRandom, HsmErr::InvalidArgument));
        let result = LMKeyDerive::bk_boot_key_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            output.len(),
            &mut output,
        );

        assert!(result.is_err());
    }

    #[test]
    fn test_bk_boot_key_gen_crypto_env_kbkdf_error() {
        let mut crypto_env = TestCryptoEnv::new();
        let mut output = vec![0u8; AES_CBC_256_KEY_SIZE + HMAC384_KEY_SIZE];

        crypto_env.error = Some((CryptoFunc::KbkdfSha384, HsmErr::KbkdfError));
        let result = LMKeyDerive::bk_boot_key_gen(
            &crypto_env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            output.len(),
            &mut output,
        );

        assert!(result.is_err());
    }

    #[test]
    fn test_masked_bk3_gen_success() {
        let env = TestCryptoEnv::new();
        let mut mbk3_out = [0u8; TEST_OUTPUT_BUFFER_SIZE];
        let mut mbk3_len = mbk3_out.len();

        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            None,
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test BK3",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        let result = LMKeyDerive::masked_bk3_gen(
            &env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BK_BOOT_KEY,
            &TEST_BK3_PARTITION,
            metadata[..metadata.len()].as_ref(),
            &mut mbk3_len,
            &mut mbk3_out,
        );

        assert!(result.is_ok());
        assert!(mbk3_len > 0);
        assert!(mbk3_len <= 1024);
        assert!(
            mbk3_out.iter().any(|&b| b != 0),
            "Masked BK3 should not be all zeros"
        );
    }

    #[test]
    fn test_mask_unmask_bk3_success() {
        let mut env = TestCryptoEnv::new();
        env.ciphertext = vec![0xF; BK_AES_CBC_256_HMAC384_SIZE_BYTES];
        env.plaintext = vec![TEST_BK3_PARTITION[0]; BK_AES_CBC_256_HMAC384_SIZE_BYTES];

        // lets generate a bk boot
        let mut bk_boot = vec![0u8; AES_CBC_256_KEY_SIZE + HMAC384_KEY_SIZE];
        let result = LMKeyDerive::bk_boot_key_gen(
            &env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            bk_boot.len(),
            &mut bk_boot,
        );
        assert!(result.is_ok());

        let mut mbk3_out = [0u8; TEST_OUTPUT_BUFFER_SIZE];
        let mut mbk3_len = mbk3_out.len();

        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            None,
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test BK3",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        let result = LMKeyDerive::masked_bk3_gen(
            &env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bk_boot,
            &TEST_BK3_PARTITION,
            metadata[..metadata.len()].as_ref(),
            &mut mbk3_len,
            &mut mbk3_out,
        );

        assert!(result.is_ok());
        assert!(mbk3_len > 0);
        assert!(mbk3_len <= 1024);
        assert!(
            mbk3_out.iter().any(|&b| b != 0),
            "Masked BK3 should not be all zeros"
        );

        // generate BKX (temp key) and mask bk boot.
        let mut bkx = [0u8; BK_BOOT_MK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let result =
            LMKeyDerive::generate_bkx(&env, &TEST_BKS1, &TEST_BKS2, &TEST_UEFI_KEY, &mut bkx);
        assert!(result.is_ok());

        // mask bk boot with bkx
        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            None,
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test BK BOOT",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");
        let mut masked_bk_boot = [0u8; TEST_OUTPUT_BUFFER_SIZE];
        let mut masked_bk_boot_len = masked_bk_boot.len();
        let result = LMKeyDerive::masked_bkboot_gen(
            &env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bk_boot,
            &bkx,
            metadata.as_slice(),
            &mut masked_bk_boot_len,
            &mut masked_bk_boot,
        );
        assert!(result.is_ok());

        let mut bk_boot_mem = [0u8; BK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let mut bk3_unmasked = [0u8; BK3_SIZE_BYTES];
        let result = LMKeyDerive::unmask_bk3(
            &env,
            &bkx,
            &masked_bk_boot[..masked_bk_boot_len],
            &mut bk_boot_mem,
            &mbk3_out[..mbk3_len],
            &mut bk3_unmasked,
        );
        assert!(result.is_ok());
        assert_eq!(bk3_unmasked, TEST_BK3_PARTITION);
    }

    #[test]
    fn test_masked_bk3_gen_deterministic() {
        let env = TestCryptoEnv::new();

        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            None,
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test BK3",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        // First call
        let mut mbk3_out1 = [0u8; TEST_OUTPUT_BUFFER_SIZE];
        let mut mbk3_len1 = mbk3_out1.len();
        let result1 = LMKeyDerive::masked_bk3_gen(
            &env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BK_BOOT_KEY,
            &TEST_BK3_PARTITION,
            metadata[..metadata.len()].as_ref(),
            &mut mbk3_len1,
            &mut mbk3_out1,
        );

        // Second call with same inputs
        let mut mbk3_out2 = [0u8; TEST_OUTPUT_BUFFER_SIZE];
        let mut mbk3_len2 = mbk3_out2.len();
        let result2 = LMKeyDerive::masked_bk3_gen(
            &env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BK_BOOT_KEY,
            &TEST_BK3_PARTITION,
            metadata[..metadata.len()].as_ref(),
            &mut mbk3_len2,
            &mut mbk3_out2,
        );

        assert!(result1.is_ok());
        assert!(result2.is_ok());
        assert_eq!(mbk3_len1, mbk3_len2);
        assert!(
            mbk3_out1.iter().any(|&b| b != 0),
            "Masked BK3 should not be all zeros"
        );

        // IV is randomized, so skip IV portion.
        const FORMAT_OFFSET: usize = 2;
        const ALGORITHM_OFFSET: usize = FORMAT_OFFSET + 2;
        const IV_LEN_OFFSET: usize = ALGORITHM_OFFSET + 2;
        const IV_PADDING_OFFSET: usize = IV_LEN_OFFSET + 2;
        const METADATA_LEN_OFFSET: usize = IV_PADDING_OFFSET + 2;
        const METADATA_PADDING_OFFSET: usize = METADATA_LEN_OFFSET + 2;
        const ENCRYPTED_KEY_LEN_OFFSET: usize = METADATA_PADDING_OFFSET + 2;
        const ENCRYPTED_KEY_PADDING_OFFSET: usize = ENCRYPTED_KEY_LEN_OFFSET + 2;
        const TAG_LEN_OFFSET: usize = ENCRYPTED_KEY_PADDING_OFFSET + 2;
        const RESERVED_OFFSET: usize = TAG_LEN_OFFSET + 34;

        let iv_len: usize = u16::from_le_bytes(
            mbk3_out1[ALGORITHM_OFFSET..IV_LEN_OFFSET]
                .try_into()
                .unwrap(),
        )
        .into();

        assert_eq!(
            &mbk3_out1[RESERVED_OFFSET + iv_len..mbk3_len1],
            &mbk3_out2[RESERVED_OFFSET + iv_len..mbk3_len2]
        );
    }

    #[test]
    fn test_masked_bk3_gen_empty_bkboot_key() {
        let env = TestCryptoEnv::new();
        let empty_bkboot_key = [];
        let mut mbk3_out = [0u8; TEST_OUTPUT_BUFFER_SIZE];
        let mut mbk3_len = mbk3_out.len();

        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            None,
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test BK3",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        let result = LMKeyDerive::masked_bk3_gen(
            &env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &empty_bkboot_key,
            &TEST_BK3_PARTITION,
            metadata[..metadata.len()].as_ref(),
            &mut mbk3_len,
            &mut mbk3_out,
        );

        // Should fail with invalid parameter error
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::InvalidKeyLength)));
    }

    #[test]
    fn test_masked_bk3_gen_empty_bk3() {
        let env = TestCryptoEnv::new();
        let empty_bk3 = [];
        let mut mbk3_out = [0u8; TEST_OUTPUT_BUFFER_SIZE];
        let mut mbk3_len = mbk3_out.len();

        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            None,
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test BK3",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        let result = LMKeyDerive::masked_bk3_gen(
            &env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BK_BOOT_KEY,
            &empty_bk3,
            metadata[..metadata.len()].as_ref(),
            &mut mbk3_len,
            &mut mbk3_out,
        );

        // Should fail with invalid parameter error
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::MaskedKeyPreEncodeFailed)));
    }

    #[test]
    fn test_masked_bk3_gen_small_output_buffer() {
        let env = TestCryptoEnv::new();
        let mut small_buffer = [0u8; 10]; // Very small buffer
        let mut mbk3_len = small_buffer.len();

        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            None,
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test BK3",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        let result = LMKeyDerive::masked_bk3_gen(
            &env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BK_BOOT_KEY,
            &TEST_BK3_PARTITION,
            metadata[..metadata.len()].as_ref(),
            &mut mbk3_len,
            &mut small_buffer,
        );

        // Should fail due to insufficient buffer size
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::InsufficientBuffer)));
    }

    #[test]
    fn test_masked_emphk_gen_success() {
        let env = TestCryptoEnv::new();
        let mut mbkboot_out = [0u8; TEST_OUTPUT_BUFFER_SIZE];
        let mut mbkboot_len = mbkboot_out.len();

        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            None,
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test EMPHK",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        let mut bkx = [0u8; BK_BOOT_MK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let result =
            LMKeyDerive::generate_bkx(&env, &TEST_BKS1, &TEST_BKS2, &TEST_UEFI_KEY, &mut bkx);
        assert!(result.is_ok(), "BKX generation should succeed");

        let result = LMKeyDerive::masked_bkboot_gen(
            &env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BK_BOOT_KEY,
            &bkx,
            metadata[..metadata.len()].as_ref(),
            &mut mbkboot_len,
            &mut mbkboot_out,
        );

        assert!(result.is_ok());
        assert!(mbkboot_len > 0);
        assert!(mbkboot_len <= TEST_OUTPUT_BUFFER_SIZE);
        assert!(
            mbkboot_out.iter().any(|&b| b != 0),
            "Masked bk boot key should not be all zeros"
        );
    }

    #[test]
    fn test_masked_emphk_gen_deterministic() {
        let env = TestCryptoEnv::new();

        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            None,
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test EMPHK",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        // First call
        let mut mbkboot_out1 = [0u8; TEST_OUTPUT_BUFFER_SIZE];
        let mut mbkboot_len1 = mbkboot_out1.len();

        let mut bkx = [0u8; BK_BOOT_MK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let result =
            LMKeyDerive::generate_bkx(&env, &TEST_BKS1, &TEST_BKS2, &TEST_UEFI_KEY, &mut bkx);
        assert!(result.is_ok(), "BKX generation should succeed");

        let result1 = LMKeyDerive::masked_bkboot_gen(
            &env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BK_BOOT_KEY,
            &bkx,
            metadata[..metadata.len()].as_ref(),
            &mut mbkboot_len1,
            &mut mbkboot_out1,
        );

        // Second call with same inputs
        let bkboot_key2 = [0xFFu8; AES_CBC_256_KEY_SIZE + HMAC384_KEY_SIZE];
        let mut mbkboot_out2 = [0u8; TEST_OUTPUT_BUFFER_SIZE];
        let mut mbkboot_len2 = mbkboot_out2.len();

        let mut bkx = [0u8; BK_BOOT_MK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let result =
            LMKeyDerive::generate_bkx(&env, &TEST_BKS1, &TEST_BKS2, &TEST_UEFI_KEY, &mut bkx);
        assert!(result.is_ok(), "BKX generation should succeed");

        let result2 = LMKeyDerive::masked_bkboot_gen(
            &env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bkboot_key2,
            &bkx,
            metadata[..metadata.len()].as_ref(),
            &mut mbkboot_len2,
            &mut mbkboot_out2,
        );

        assert!(result1.is_ok());
        assert!(result2.is_ok());
        assert_eq!(mbkboot_len1, mbkboot_len2);
    }

    #[test]
    fn test_masked_emphk_gen_small_output_buffer() {
        let env = TestCryptoEnv::new();
        let mut small_buffer = [0u8; 10]; // Very small buffer
        let mut mbkboot_len = small_buffer.len();

        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            None,
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test EMPHK",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        let mut bkx = [0u8; BK_BOOT_MK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let result =
            LMKeyDerive::generate_bkx(&env, &TEST_BKS1, &TEST_BKS2, &TEST_UEFI_KEY, &mut bkx);
        assert!(result.is_ok(), "BKX generation should succeed");

        let result = LMKeyDerive::masked_bkboot_gen(
            &env,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &TEST_BK_BOOT_KEY,
            &bkx,
            metadata[..metadata.len()].as_ref(),
            &mut mbkboot_len,
            &mut small_buffer,
        );

        // Should fail due to insufficient buffer size
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::InsufficientBuffer)));
    }

    #[test]
    fn test_masked_emphk_gen_crypto_env_kbkdf_error() {
        let mut env = TestCryptoEnv::new();
        let mut metadata = [0u8; TEST_METADATA_MAX_SIZE_BYTES];
        let result = LMKeyDerive::mbor_encode_masked_key_metadata(
            None,
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"Test EMPHK",
            &mut metadata.len(),
            &mut metadata,
            BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
        );
        assert!(result.is_ok(), "Metadata encoding should succeed");

        env.error = Some((CryptoFunc::KbkdfSha384, HsmErr::KbkdfError));

        let mut bkx = [0u8; BK_BOOT_MK_AES_CBC_256_HMAC384_SIZE_BYTES];
        let result =
            LMKeyDerive::generate_bkx(&env, &TEST_BKS1, &TEST_BKS2, &TEST_UEFI_KEY, &mut bkx);
        assert!(result.is_err());
    }

    #[cfg(test)]
    mod encode_masked_key_metadata_tests {
        use zerocopy::IntoBytes;

        use super::*;
        use crate::partition::store::EntryAttributes;

        #[test]
        fn test_encode_decode_masked_key_metadata_success() {
            let svn = Some(1u64);
            let key_kind = DdiKeyType::AesCbc256Hmac384;
            let mut entry_attributes = EntryAttributes::default();
            entry_attributes.common.flags.set_encrypt(true);
            let key_attributes = DdiMaskedKeyAttributes {
                blob: entry_attributes.as_bytes().try_into().unwrap(),
            };
            let bks2_index = Some(0u16);
            let key_tag = Some(0x1234u16);
            let key_label = b"test_key_label";
            let mut encoded_metadata = vec![0u8; 256];
            let mut metadata_len = encoded_metadata.len();

            let result = LMKeyDerive::mbor_encode_masked_key_metadata(
                svn,
                key_kind,
                key_attributes,
                bks2_index,
                key_tag,
                key_label,
                &mut metadata_len,
                &mut encoded_metadata,
                BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
            );

            assert!(result.is_ok());
            assert!(metadata_len > 0);
            // Check is buffer is not all zeros
            assert!(encoded_metadata[..metadata_len].iter().any(|&b| b != 0));

            // Now decode it back
            let decode_result =
                LMKeyDerive::decode_masked_key_metadata(&encoded_metadata[..metadata_len]);
            assert!(decode_result.is_ok());

            let decoded = decode_result.unwrap();
            assert_eq!(decoded.svn, svn);
            assert_eq!(decoded.key_type, key_kind);
            assert_eq!(decoded.bks2_index, bks2_index);
            assert_eq!(decoded.key_tag, key_tag);
            assert_eq!(decoded.key_label.as_slice(), key_label);
        }

        #[test]
        fn test_encode_masked_key_metadata_none_svn() {
            let svn = None; // Test with no SVN
            let key_kind = DdiKeyType::Aes256;
            let entry_attributes = EntryAttributes::default();
            let key_attributes = DdiMaskedKeyAttributes {
                blob: entry_attributes.as_bytes().try_into().unwrap(),
            };
            let bks2_index = Some(0u16);
            let key_tag = Some(0x1234u16);
            let key_label = b"test_key";
            let mut encoded_metadata = vec![0u8; 256];
            let mut metadata_len = encoded_metadata.len();

            let result = LMKeyDerive::mbor_encode_masked_key_metadata(
                svn,
                key_kind,
                key_attributes,
                bks2_index,
                key_tag,
                key_label,
                &mut metadata_len,
                &mut encoded_metadata,
                BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
            );

            assert!(result.is_ok());
            assert!(metadata_len > 0);
            // Check is buffer is not all zeros
            assert!(encoded_metadata[..metadata_len].iter().any(|&b| b != 0));
        }

        #[test]
        fn test_encode_masked_key_metadata_none_bks2_index() {
            let svn = Some(1u64);
            let key_kind = DdiKeyType::Aes256;
            let entry_attributes = EntryAttributes::default();
            let key_attributes = DdiMaskedKeyAttributes {
                blob: entry_attributes.as_bytes().try_into().unwrap(),
            };
            let bks2_index = None; // Test with no BKS2 index
            let key_tag = Some(0x1234u16);
            let key_label = b"test_key";
            let mut encoded_metadata = vec![0u8; 256];
            let mut metadata_len = encoded_metadata.len();

            let result = LMKeyDerive::mbor_encode_masked_key_metadata(
                svn,
                key_kind,
                key_attributes,
                bks2_index,
                key_tag,
                key_label,
                &mut metadata_len,
                &mut encoded_metadata,
                BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
            );

            assert!(result.is_ok());
            assert!(metadata_len > 0);
            // Check is buffer is not all zeros
            assert!(encoded_metadata[..metadata_len].iter().any(|&b| b != 0));
        }

        #[test]
        fn test_encode_masked_key_metadata_none_key_tag() {
            let svn = Some(1u64);
            let key_kind = DdiKeyType::Aes256;
            let entry_attributes = EntryAttributes::default();
            let key_attributes = DdiMaskedKeyAttributes {
                blob: entry_attributes.as_bytes().try_into().unwrap(),
            };
            let bks2_index = Some(0u16);
            let key_tag = None; // Test with no key tag
            let key_label = b"test_key";
            let mut encoded_metadata = vec![0u8; 256];
            let mut metadata_len = encoded_metadata.len();

            let result = LMKeyDerive::mbor_encode_masked_key_metadata(
                svn,
                key_kind,
                key_attributes,
                bks2_index,
                key_tag,
                key_label,
                &mut metadata_len,
                &mut encoded_metadata,
                BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
            );

            assert!(result.is_ok());
            assert!(metadata_len > 0);
            // Check is buffer is not all zeros
            assert!(encoded_metadata[..metadata_len].iter().any(|&b| b != 0));
        }

        #[test]
        fn test_encode_masked_key_metadata_small_buffer() {
            let svn = Some(1u64);
            let key_kind = DdiKeyType::Aes256;
            let entry_attributes = EntryAttributes::default();
            let key_attributes = DdiMaskedKeyAttributes {
                blob: entry_attributes.as_bytes().try_into().unwrap(),
            };
            let bks2_index = Some(0u16);
            let key_tag = Some(0x1234u16);
            let key_label = b"test_key";
            let mut metadata_len = 0usize;
            let mut encoded_metadata = vec![0u8; 10]; // Small buffer

            let result = LMKeyDerive::mbor_encode_masked_key_metadata(
                svn,
                key_kind,
                key_attributes,
                bks2_index,
                key_tag,
                key_label,
                &mut metadata_len,
                &mut encoded_metadata,
                BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
            );

            assert!(result.is_err());
            assert!(matches!(result, Err(HsmErr::InsufficientBuffer)));
            assert!(metadata_len > 0);
        }

        #[test]
        fn test_encode_masked_key_metadata_empty_key_label() {
            let svn = Some(1u64);
            let key_kind = DdiKeyType::Aes256;
            let entry_attributes = EntryAttributes::default();
            let key_attributes = DdiMaskedKeyAttributes {
                blob: entry_attributes.as_bytes().try_into().unwrap(),
            };
            let bks2_index = Some(0u16);
            let key_tag = Some(0x1234u16);
            let key_label = b""; // Empty label
            let mut encoded_metadata = vec![0u8; 256];
            let mut metadata_len = encoded_metadata.len();

            let result = LMKeyDerive::mbor_encode_masked_key_metadata(
                svn,
                key_kind,
                key_attributes,
                bks2_index,
                key_tag,
                key_label,
                &mut metadata_len,
                &mut encoded_metadata,
                BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
            );

            assert!(result.is_ok());
            assert!(metadata_len > 0);
            // Check is buffer is not all zeros
            assert!(encoded_metadata[..metadata_len].iter().any(|&b| b != 0));
        }

        #[test]
        fn test_decode_masked_key_metadata_empty_buffer() {
            let empty_buffer = &[];
            let result = LMKeyDerive::decode_masked_key_metadata(empty_buffer);
            assert!(result.is_err());
        }

        #[test]
        fn test_decode_masked_key_metadata_invalid_buffer() {
            let invalid_buffer = &[0x00, 0x01, 0x02]; // Too small/invalid
            let result = LMKeyDerive::decode_masked_key_metadata(invalid_buffer);
            assert!(result.is_err());
        }

        #[test]
        fn test_decode_masked_key_metadata_corrupted_buffer() {
            // Create valid encoded metadata first
            let svn = Some(1u64);
            let key_kind = DdiKeyType::Aes256;
            let entry_attributes = EntryAttributes::default();
            let key_attributes = DdiMaskedKeyAttributes {
                blob: entry_attributes.as_bytes().try_into().unwrap(),
            };
            let bks2_index = Some(0u16);
            let key_tag = Some(0x1234u16);
            let key_label = b"test_key";
            let mut encoded_metadata = vec![0u8; 256];
            let mut metadata_len = encoded_metadata.len();

            let encode_result = LMKeyDerive::mbor_encode_masked_key_metadata(
                svn,
                key_kind,
                key_attributes,
                bks2_index,
                key_tag,
                key_label,
                &mut metadata_len,
                &mut encoded_metadata,
                BK_AES_CBC_256_HMAC384_SIZE_BYTES as u16,
            );
            assert!(encode_result.is_ok());

            // Corrupt the buffer
            encoded_metadata[0] = 0xFF;
            encoded_metadata[1] = 0xFF;

            let decode_result =
                LMKeyDerive::decode_masked_key_metadata(&encoded_metadata[..metadata_len]);
            assert!(decode_result.is_err());
        }
    }
}
