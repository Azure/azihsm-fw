// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::string::String;
use alloc::vec;
use cred_mgr::APP_VAULT_ID_FOR_INTERNAL_KEYS;
use hex::ToHex;
use mcr_crypto_aes::AesCommand;
use mcr_crypto_aes::AesMode;
use mcr_crypto_aes::AesOp;
use mcr_crypto_aes::AesTrait;
use mcr_crypto_pka::reverse_copy_from_slice;
use mcr_crypto_pka::PkaEccPrivateKey;
use mcr_crypto_pka::PkaTrait;
use mcr_crypto_rng::RngTrait;
use mcr_crypto_sha::ShaDigestCmdInfoZc;
use mcr_crypto_sha::ShaMode;
use mcr_crypto_sha::ShaTrait;
use mcr_ddi_mbor::MborDecode;
use mcr_ddi_mbor::MborDecoder;
use mcr_ddi_types::DdiMaskedKeyAttributes;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ddi_types::DdiTestAction;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ddi_types::DdiTestActionPinPolicyConfig;
use mcr_ddi_types::MaskedKey;
use mcr_ddi_types::MaskingKeyAlgorithm;
use mcr_ddi_types::AES_BLOCK_SIZE;
use mcr_ipc_controller::IpcMessageChannelTrait;
use mcr_ipc_message::AesKeyFlag;
use mcr_ipc_message::IpcMessageDecoder;
use mcr_ipc_message::IpcMessageEncoderTrait;
use mcr_ipc_message::IpcMessageKeyUpdate;
use mcr_ipc_message::IpcMessageStatusCode;
use mcr_ipc_message::KeyUpdateAction;
use mcr_ipc_message::KeyUpdateInfo;
use mcr_logging::*;
use mcr_types::*;
use store::KEY_TAG_UNASSIGNED;
use zeroize::Zeroize;

use super::*;
use crate::cmd_scheduler::*;
use crate::crypto_env::BK_BOOT_MASKING_KEY;
use crate::der;
use crate::der::DerDecoderTrait;
use crate::env::HsmEnvTrait;
use crate::error;
use crate::error::HsmResult;
use crate::key_attestation::report::SIGNATURE_SIZE;
use crate::lm_key_derive::LMKeyDerive;
use crate::lm_key_derive::BK3_SIZE_BYTES;
use crate::lm_key_derive::BK_BOOT_MK_AES_CBC_256_HMAC384_SIZE_BYTES;
use crate::lm_key_derive::BK_SEED_SIZE_BYTES;
use crate::lm_key_derive::MK_AES_CBC_256_HMAC384_SIZE_BYTES;
use crate::masked_key::MaskedKeyDecode;
#[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
use crate::partition::pct::ecc_pct_constants::NEGATIVE_PCT_ECC_PUBLIC_KEYS;
use crate::x509::AzihsmLeafCertTbsParams;
use crate::x509::Ecdsa384Signature;
use crate::x509::NotAfter;
use crate::x509::NotBefore;
use alloc::boxed::Box;
use pct::pct_engine::*;
use pct::pct_engine_impl::*;

/// HSM Partition
pub(crate) struct Partition<E: HsmEnvTrait + 'static> {
    pub(crate) state: PartState<E>,
}

impl<E: HsmEnvTrait> Clone for Partition<E> {
    fn clone(&self) -> Self {
        Self {
            state: self.state.clone(),
        }
    }
}

impl<E: HsmEnvTrait> Partition<E> {
    /// Check if the key is in use identified by key_id
    fn key_in_use(&self, id: KeyId) -> bool {
        let used_by_pka = self
            .state
            .env()
            .pka_engine()
            .find_ctx(|c| Some(id) == *c)
            .is_some();

        used_by_pka
    }

    fn delete_session_keys(&self, id: SessionId) -> HsmResult<()> {
        self.state.vault().delete_all_session_keys(id, |key_id| {
            let vault_key = self.state.vault().key_unchecked(key_id);

            let entry_kind = match vault_key.kind() {
                Ok(kind) => kind,
                Err(_) => return true,
            };

            if !entry_kind.is_bulk_key() {
                return self.key_in_use(key_id);
            }

            // Bulk key: CDMA vault zeroized by FP via DeleteEphemeral IPC.
            // Free bitmap slot + HSM vault entry.
            let key_blob = match vault_key.blob() {
                Ok(blob) => blob,
                Err(_) => return true,
            };

            let cdma_key_id =
                AesBulk256KeyId::from(u16::from_le_bytes(key_blob[..2].try_into().unwrap()));

            if self.state.cdma_vault().delete_key(cdma_key_id).is_err() {
                return true;
            }

            false
        })?;

        Ok(())
    }

    pub(crate) fn sha_single_block_zc_internal(
        &self,
        mode: ShaMode,
        buffer: &IoMemRange,
        output_buffer: &mut IoMemRange,
    ) -> HsmResult<()> {
        // Prepare the command packet.
        let cmd_info = ShaDigestCmdInfoZc {
            buffer,
            init_digest: None,
            mode,
            last: true,
            len: buffer.len() as u32,
            total_len: buffer.len() as u32,
            output_buffer,
        };

        // Compute the SHA digest
        self.state
            .env()
            .sha()
            .digest_zc(&cmd_info)
            .map_err(|_| HsmErr::ShaCmdFailed)
    }

    fn begin_ecc_sign_with_priv_key_internal_with_engine_acquired(
        &self,
        tag: TagId,
        digest: &IoMemRange,
        key_blob: &[u8],
        engine_ref: &PkaEngineRef<E>,
        signature: &IoMemRange,
    ) -> HsmResult<()> {
        // Digest should not be empty.
        if digest.is_empty() {
            Err(HsmErr::InvalidArgument)?
        }

        let curve = PkaEccCurve::Ecc384;

        if digest.len() > curve.len() {
            Err(HsmErr::InvalidArgument)?
        }

        // Submit the PKA command to the engine
        let _cmd_info = engine_ref
            .deref()
            .begin_ecc_sign_zc(tag, curve, key_blob, digest, signature);

        Ok(())
    }

    fn end_ecc_sign_with_priv_key_internal(
        &self,
        tag: TagId,
        engine_ref: &PkaEngineRef<E>,
    ) -> HsmResult<()> {
        let pka_result = engine_ref
            .deref()
            .end_ecc_sign_zc(tag)
            .map_err(|_| HsmErr::EccSignFailed);

        pka_result
    }

    /// AES encrypt/decrypt operation.
    /// TODO: this is duplicate code with session object. Need to have a single copy.
    pub(crate) fn aes_enc_dec_internal_with_key_blob<'a>(
        &self,
        tag: TagId,
        key_blob: &'a [u8],
        input: &'a AesEncDecIn,
    ) -> HsmResult<()> {
        // Variable bindings for `key` and `key_blob` references.

        match input.mode() {
            AesEncDecMode::Cbc => {
                let iv = input.iv().ok_or(HsmErr::InvalidArgument)?;

                if iv.len() != Self::AES_IV_SIZE {
                    Err(HsmErr::InvalidArgument)?
                }
            }
            AesEncDecMode::Ecb => {
                if input.iv().is_some() {
                    Err(HsmErr::InvalidArgument)?
                }
            }
        }

        let plaintext = input.msg_in();

        if plaintext.is_empty() {
            Err(HsmErr::InvalidArgument)?
        }

        if plaintext.len() % AES_BLOCK_SIZE != 0 {
            Err(HsmErr::InvalidArgument)?
        }

        if input.msg_in().len() != input.msg_out().len() {
            Err(HsmErr::InvalidArgument)?
        }

        let cmd = AesCommand {
            tag,
            message: input.msg_in(),
            iv: input.iv(),
            key: key_blob,
            update_iv: true,
            op: input.op().into(),
            mode: input.mode().into(),
            result: input.msg_out(),
        };

        // Submit the AES command the engine
        self.state
            .env()
            .aes()
            .encrypt_decrypt(&cmd)
            .map_err(|_| match cmd.op {
                AesOp::Decrypt => HsmErr::AesDecryptFailed,
                AesOp::Encrypt => HsmErr::AesEncryptFailed,
            })?;

        Ok(())
    }

    #[allow(clippy::too_many_arguments)]
    pub(crate) fn get_mbor_encoded_metadata(
        svn: Option<u64>,
        key_kind: DdiKeyType,
        key_attributes: DdiMaskedKeyAttributes,
        bks2_index: Option<u16>,
        key_tag: Option<u16>,
        key_label: &[u8],
        key_length: u16,
    ) -> HsmResult<Vec<u8>> {
        let mut metadata_len = 0;
        if LMKeyDerive::mbor_encode_masked_key_metadata(
            svn,
            key_kind,
            key_attributes.clone(),
            bks2_index,
            key_tag,
            key_label,
            &mut metadata_len,
            &mut [],
            key_length,
        ) != Err(HsmErr::InsufficientBuffer)
        {
            return Err(HsmErr::MetadataEncodeFailed);
        }

        // now get metadata for real.
        let mut metadata_buf = vec![0; metadata_len];
        LMKeyDerive::mbor_encode_masked_key_metadata(
            svn,
            key_kind,
            key_attributes,
            bks2_index,
            key_tag,
            key_label,
            &mut metadata_len,
            metadata_buf.as_mut_slice(),
            key_length,
        )?;

        Ok(metadata_buf)
    }
}

#[derive(PartialEq)]
pub(crate) enum HmacHashAlgorithm {
    // SHA-1
    Sha1,

    // SHA-256
    Sha256,

    // SHA-384
    Sha384,

    // SHA-512
    Sha512,
}

impl TryFrom<DdiHashAlgorithm> for HmacHashAlgorithm {
    type Error = HsmErr;

    fn try_from(algorithm: DdiHashAlgorithm) -> Result<Self, Self::Error> {
        match algorithm {
            DdiHashAlgorithm::Sha1 => Ok(HmacHashAlgorithm::Sha1),
            DdiHashAlgorithm::Sha256 => Ok(HmacHashAlgorithm::Sha256),
            DdiHashAlgorithm::Sha384 => Ok(HmacHashAlgorithm::Sha384),
            DdiHashAlgorithm::Sha512 => Ok(HmacHashAlgorithm::Sha512),
            _ => Err(HsmErr::InvalidArgument),
        }
    }
}

impl From<HmacHashAlgorithm> for ShaMode {
    fn from(algorithm: HmacHashAlgorithm) -> Self {
        match algorithm {
            HmacHashAlgorithm::Sha1 => ShaMode::Sha1,
            HmacHashAlgorithm::Sha256 => ShaMode::Sha256,
            HmacHashAlgorithm::Sha384 => ShaMode::Sha384,
            HmacHashAlgorithm::Sha512 => ShaMode::Sha512,
        }
    }
}

// HMAC implementation:
impl<E: HsmEnvTrait> Partition<E> {
    /// DMA allocation for a given length
    pub(crate) fn dma_alloc(&self, len: usize) -> HsmResult<DmaBuffer<E>> {
        self.state
            .env()
            .dma_heap()
            .allocate(len)
            .ok_or(HsmErr::DmaAllocFailure)
    }

    /// DMA copy allocates a buffer
    pub(crate) fn dma_copy_alloc(&self, slice: &[u8]) -> HsmResult<DmaBuffer<E>> {
        self.state
            .env()
            .dma_heap()
            .copy_allocate(slice)
            .ok_or(HsmErr::DmaAllocFailure)
    }

    fn verify_encrypted_establish_credential_tag(
        &self,
        hmac_key: &[u8],
        encrypted_credential: &DdiEncryptedEstablishCredential,
    ) -> HsmResult<()> {
        // fail-fast length checks
        if encrypted_credential.encrypted_id.len() != 16
            || encrypted_credential.encrypted_pin.len() != 16
            || encrypted_credential.iv.len() != 16
        {
            return Err(HsmErr::InvalidArgument);
        }

        // Verify hash using aes_vec[32..]
        const HASH_LEN: usize = 64;
        let hash_gsram = self.dma_alloc(HASH_LEN)?;
        let mut hash_range = (hash_gsram.as_ref()).into();

        let mut current_nonce = self.state.nonce();

        let mut id_pin_iv_nonce_gsram = self.dma_alloc(80)?;
        let id_pin_iv_nonce_gsram_slice = id_pin_iv_nonce_gsram.as_ref_mut();
        id_pin_iv_nonce_gsram_slice[..16]
            .copy_from_slice(encrypted_credential.encrypted_id.as_slice());
        id_pin_iv_nonce_gsram_slice[16..32]
            .copy_from_slice(encrypted_credential.encrypted_pin.as_slice());
        id_pin_iv_nonce_gsram_slice[32..48].copy_from_slice(encrypted_credential.iv.as_slice());
        id_pin_iv_nonce_gsram_slice[48..].copy_from_slice(&current_nonce);

        // zeroize the buffer for security
        current_nonce.zeroize();

        self.hmac_impl(
            hmac_key,
            id_pin_iv_nonce_gsram.as_ref(),
            DdiHashAlgorithm::Sha384,
            &mut hash_range,
        )?;

        // HMAC SHA384 is 48 bytes but HW is configured to return 64 bytes for HSSHA.
        // HW can be configured to return 48 bytes but it will mean modifying lot of
        // existing code.
        let hash_gsram_slice = hash_gsram.as_ref();
        if encrypted_credential.tag != hash_gsram_slice[..48] {
            Err(HsmErr::PinDecryptionFailed)?
        }

        Ok(())
    }

    fn verify_encrypted_session_credential_tag(
        &self,
        hmac_key: &[u8],
        encrypted_credential: &DdiEncryptedSessionCredential,
    ) -> HsmResult<()> {
        // fail-fast length checks
        if encrypted_credential.encrypted_id.len() != 16
            || encrypted_credential.encrypted_pin.len() != 16
            || encrypted_credential.encrypted_seed.len() != 48
            || encrypted_credential.iv.len() != 16
        {
            return Err(HsmErr::InvalidArgument);
        }

        // Verify hash using aes_vec[32..]
        const HASH_LEN: usize = 64;
        let hash_gsram = self.dma_alloc(HASH_LEN)?;
        let mut hash_range = (hash_gsram.as_ref()).into();

        let mut current_nonce = self.state.nonce();

        let mut id_pin_seed_iv_nonce_gsram = self.dma_alloc(128)?;
        let id_pin_seed_iv_nonce_gsram_slice = id_pin_seed_iv_nonce_gsram.as_ref_mut();
        id_pin_seed_iv_nonce_gsram_slice[..16]
            .copy_from_slice(encrypted_credential.encrypted_id.as_slice());
        id_pin_seed_iv_nonce_gsram_slice[16..32]
            .copy_from_slice(encrypted_credential.encrypted_pin.as_slice());
        id_pin_seed_iv_nonce_gsram_slice[32..80]
            .copy_from_slice(encrypted_credential.encrypted_seed.as_slice());
        id_pin_seed_iv_nonce_gsram_slice[80..96]
            .copy_from_slice(encrypted_credential.iv.as_slice());
        id_pin_seed_iv_nonce_gsram_slice[96..].copy_from_slice(&current_nonce);

        // zeroize the buffer for security
        current_nonce.zeroize();

        self.hmac_impl(
            hmac_key,
            id_pin_seed_iv_nonce_gsram.as_ref(),
            DdiHashAlgorithm::Sha384,
            &mut hash_range,
        )?;

        // HMAC SHA384 is 48 bytes but HW is configured to return 64 bytes for HSSHA.
        // HW can be configured to return 48 bytes but it will mean modifying lot of
        // existing code.
        let hash_gsram_slice = hash_gsram.as_ref();
        if encrypted_credential.tag != hash_gsram_slice[..48] {
            Err(HsmErr::PinDecryptionFailed)?
        }

        Ok(())
    }

    fn decrypt_credential(
        &self,
        tag: TagId,
        aes_key: &[u8],
        cipher_text: &[u8],
        iv: &IoMemRange,
        decrypted_buffer: &IoMemRange,
    ) -> HsmResult<()> {
        let cmd = AesCommand {
            tag,
            message: &(cipher_text).into(),
            iv: Some(iv),
            key: aes_key,
            mode: AesMode::Cbc,
            op: AesOp::Decrypt,
            update_iv: true,
            result: decrypted_buffer,
        };

        self.state
            .env()
            .aes()
            .encrypt_decrypt(&cmd)
            .map_err(|_| HsmErr::AesDecryptFailed)?;

        Ok(())
    }

    fn set_user_credential(&self, id: &[u8], pin: &[u8]) -> HsmResult<()> {
        self.state.change_user_cred(id, pin)
    }
}

impl<E: HsmEnvTrait> HsmPartition for Partition<E> {
    type Env = E;
    type UserSession = UserSession<E>;

    /// Returns the minimum API revision supported by the function
    fn min_api_rev(&self) -> DdiApiRev {
        Self::MIN_API_REV
    }

    /// Returns the maximum API revision supported by the function
    fn max_api_rev(&self) -> DdiApiRev {
        Self::MAX_API_REV
    }

    /// Enable the function
    fn enable(&self) {
        self.state.enable();
    }

    /// Disable the function
    fn disable(&self, delete_ctx: Option<IoQueueDeleteContext>) -> bool {
        let pending = self.state.ioq_mgr_mut().disable_all_io_queues(delete_ctx);
        self.state.disable();

        pending
    }

    /// Reset the function
    fn reset(&self) {
        self.disable(None);
        self.state.part_persistent_store_ref().vm_launch_guid = VmLaunchGuid::default();
        self.state.rgs_reset();
        self.state.clear_partition_info();
    }

    /// Migrate the partition
    fn begin_migrate(&self, delete_ctx: Option<IoQueueDeleteContext>) -> bool {
        self.state.ioq_mgr_mut().disable_all_io_queues(delete_ctx)
    }

    /// End migration of the function
    fn end_migrate(&self) {
        self.state.migrate();
    }

    /// Check if the function is enabled
    fn enabled(&self) -> bool {
        self.state.enabled()
    }

    /// Set the resource mask
    fn set_resource_mask(&self, mask: u128) {
        self.state.rgs_mut().set_mask(mask)
    }

    /// Arm/disarm the ephemeral unwrapping-key staging gate for this PFN.
    fn set_unwrapping_key_required(&self, required: bool) {
        self.state.set_unwrapping_key_required(required)
    }

    /// Get the resource mask
    fn resource_mask(&self) -> u128 {
        self.state.rgs().mask()
    }

    /// Enable IO queue
    fn enable_io_queue(&self, sq_id: DevSqId, cq_id: DevCqId) {
        self.state.ioq_mgr_mut().enable_io_queue(sq_id, cq_id)
    }

    /// Disable IO queue
    fn disable_io_queue(&self, sq_id: DevSqId, delete_ctx: Option<IoQueueDeleteContext>) -> bool {
        self.state.ioq_mgr_mut().disable_io_queue(sq_id, delete_ctx)
    }

    /// Get IO queue
    fn io_queue(&self, sq_id: DevSqId) -> Option<IoQueue> {
        self.state.ioq_mgr().io_queue(sq_id)
    }

    /// Rollback open user session
    fn rollback_open_session(&self, id: SessionId, is_reopen: bool) -> HsmResult<()> {
        self.state.vault().delete_key(
            self.state.cred_mgr().get_user_vault_id(),
            KEY_TAG_UNASSIGNED,
            self.state.session_table().get_target_session(id)?,
            |k| self.key_in_use(k),
        )?;

        if is_reopen {
            self.state.session_table().rollback_recreation(id);
        } else {
            self.state.session_table().delete(id);
        }

        Ok(())
    }

    /// Close user session
    fn close_user_session(&self, id: SessionId) -> HsmResult<()> {
        // Delete all session keys before deleting the session
        self.delete_session_keys(id)?;

        // Delete the session key
        self.state.vault().delete_key(
            self.state.cred_mgr().get_user_vault_id(),
            KEY_TAG_UNASSIGNED,
            self.state.session_table().get_target_session(id)?,
            |k| self.key_in_use(k),
        )?;

        self.delete_user_session(id);

        Ok(())
    }

    fn delete_user_session(&self, id: SessionId) {
        self.state.session_table().delete(id);
    }

    /// Get user session
    fn user_session(&self, id: SessionId, allow_disabled: bool) -> HsmResult<Self::UserSession> {
        let key_id = self.state.session_table().get_target_session(id)?;

        let key = self.state.vault().open_session_key(
            self.state.cred_mgr().get_user_vault_id(),
            key_id,
            allow_disabled,
        )?;

        let key_blob = key.session_key_blob()?;

        let mut rev_buf_major = [0; 4];
        rev_buf_major.copy_from_slice(&key_blob[..4]);

        let mut rev_buf_minor = [0; 4];
        rev_buf_minor.copy_from_slice(&key_blob[4..]);

        let api_rev = DdiApiRev {
            major: u32::from_le_bytes(rev_buf_major),
            minor: u32::from_le_bytes(rev_buf_minor),
        };

        Ok(Self::UserSession::new(api_rev, id, self.state.clone()))
    }

    /// Clear the unwrapping key from both key vault and persistent store
    fn clear_unwrapping_key(&mut self) -> HsmResult<()> {
        let key_id = self.state.unwrapping_key_id();

        if let Some(id) = key_id {
            self.state.vault().delete_key(
                APP_VAULT_ID_FOR_INTERNAL_KEYS,
                u16::MAX, // This value does not matter since the key availability is set to App
                id,
                |k| self.key_in_use(k),
            )?;
        }

        // Zeroize the payload before clearing the valid flag so HSP never sees a (invalid, non-zero) slot.
        self.state
            .part_persistent_store_ref()
            .unwrapping_key_bk
            .zeroize();
        self.state
            .part_persistent_store_ref()
            .unwrapping_key_bk_valid = UnwrappingKeyValidity::Empty as u8;

        self.state.set_unwrapping_key_id(None);

        Ok(())
    }

    /// Get the unwrapping key id
    fn unwrapping_key_id(&self) -> Option<KeyId> {
        self.state.unwrapping_key_id()
    }

    fn is_unwrapping_key_pct_verified(&self) -> bool {
        self.state
            .part_persistent_store_ref()
            .unwrapping_key_bk_valid
            == UnwrappingKeyValidity::PctPassed as u8
    }

    fn mark_unwrapping_key_pct_verified(&mut self) {
        self.state
            .part_persistent_store_ref()
            .unwrapping_key_bk_valid = UnwrappingKeyValidity::PctPassed as u8;
    }

    /// Get the alias cert length
    fn get_alias_cert_len(&self) -> usize {
        self.state.env().alias_cert_len()
    }

    /// Get the alias certificate from GSRAM
    fn get_alias_cert(&self) -> IoMemRange {
        let env = self.state.env();
        let alias_cert = &env.alias_cert()[..self.get_alias_cert_len()];
        IoMemRange::from(alias_cert)
    }

    /// Get the alias key from GSRAM and decode it.
    fn get_raw_alias_key(&self) -> HsmResult<SecureByteVec> {
        let env = self.state.env();
        let alias_key_len = env.alias_key_len();
        let alias_key_buf = env.alias_key();
        let alias_key = &alias_key_buf[..alias_key_len as usize];

        let ecc_key_data = alias_key.ecc_priv_key_pkcs1_der_to_raw()?;
        let priv_key = ecc_key_data.priv_key().ok_or(HsmErr::DerDecodeFailed)?;
        let key_blob = priv_key.to_pka_bytes()?;

        Ok(key_blob)
    }

    /// Close user session to handle both non aes bulk and aes bulk 256 keys
    fn begin_close_user_session(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        id: SessionId,
    ) -> HsmResult<AesBulk256Cmd<E>> {
        // Step 1: Get the app session
        let mut session = self.user_session(id, true)?;

        // Step 2: Invalidate the session
        session.invalidate();

        let vault_id = session.app_vault_id();

        // Step 3: Acquire fp IPC channel
        let channel_ref: FpIpcChannelRef<E> = self
            .state
            .env()
            .fp_ipc_channel()
            .acquire(tag, ())
            .ok_or(HsmErr::Pending)?;

        // Step 4: Send delete ephemeral(session) IPC to FP
        let msg: IpcMessageKeyUpdate = IpcMessageKeyUpdate {
            info: KeyUpdateInfo {
                key_index: 0,
                resource_id: 0,
                pfn,
                action: KeyUpdateAction::DeleteEphemeral,
                session_id: id,
                app_id: vault_id,
                flag: AesKeyFlag::new().with_session_only(true),
                ..Default::default()
            },
            ..Default::default()
        };

        channel_ref
            .map(|c| c.send_request(tag, msg.encode()))
            .map_err(|err| {
                error!("[part] Failed to send IPC message to FP: {}", { err });
                HsmErr::IpcSendFailure
            })?;

        Ok(AesBulk256Cmd::CloseAppSession(id, channel_ref))
    }

    /// Receive IPC from FP and finish the close user session operation
    fn end_close_user_session(&self, op: &AesBulk256Cmd<E>) -> HsmResult<()> {
        let AesBulk256Cmd::CloseAppSession(id, ref channel_ref) = *op else {
            return Err(HsmErr::AesBulk256InvalidParameter);
        };

        // Step 1: Validate response from FP
        let message = channel_ref.map(|c| c.receive_message());
        if let Some(message) = message {
            let header = IpcMessageDecoder::decode_header(&message).map_err(|err| {
                error!("[part] Failed to decode IPC message header: {:?}", err);
                HsmErr::IpcResponseError
            })?;

            if header.status() == IpcMessageStatusCode::Success.into()
                || header.status() == IpcMessageStatusCode::InvalidField.into()
            {
                // Step 2: Receive success from FP for aes bulk keys,
                //         Receive InvalidField for non aes bulk keys
                // The later InvalidField is a bug and is being tracked as part
                // of Bug 1885477: FP should return a specific error code or
                // success when no session/appid found for DeleteAll and
                // DeleteEphemeral keys). So now proceed with the actual
                // close_app_session()
                self.close_user_session(id)?;
                Ok(())
            } else {
                error!(
                    "[part] Invalid IPC response with status {}",
                    header.status()
                );
                Err(HsmErr::IpcResponseError)?
            }
        } else {
            error!("[part] Spurious Message");
            Err(HsmErr::IpcResponseError)?
        }
    }

    /// Return establish cred encryption key if it exists, generate it otherwise.
    fn begin_get_establish_cred_encryption_key(
        &self,
        tag: TagId,
    ) -> HsmResult<GetEstablishCredEncryptionKeyCtx<E>> {
        // Acquire the PKA engine
        //
        // Note: The engine acquisition can fail with HsmErr::Pending if the
        // engine is busy.
        let engine_ref = self
            .state
            .env()
            .pka_engine()
            .acquire(tag, None)
            .ok_or(HsmErr::Pending)?;

        // Check if creds have already been established.
        self.verify_cred_is_not_set()?;

        match self.state.get_establish_cred_encryption_key_id() {
            Some(key_id) => {
                let key = self
                    .state
                    .vault()
                    .open_establish_cred_encryption_key(APP_VAULT_ID_FOR_INTERNAL_KEYS, key_id)?;
                let pub_key_size = PkaEccPublicKey::data_len(PkaEccCurve::Ecc384);
                let pub_key_data =
                    PkaEccPublicKey::from_bytes(PkaEccCurve::Ecc384, &key.blob()?[..pub_key_size])
                        .map_err(|_| HsmErr::EccGenKeyFailed)?;

                let key_data = CredentialEncryptionKeyData {
                    pub_key_data,
                    nonce: self.state.nonce(),
                };

                Ok(GetEstablishCredEncryptionKeyCtx {
                    tag,
                    engine_ref: Some(engine_ref),
                    cmd_info: None,
                    key_data: Some(key_data),
                })
            }
            None => {
                // Submit the PKA command to the engine
                let cmd_info = engine_ref
                    .deref()
                    .begin_ecc_gen_key(tag, PkaEccCurve::Ecc384)
                    .map_err(|_| HsmErr::EccGenKeyFailed)?;

                Ok(GetEstablishCredEncryptionKeyCtx {
                    tag,
                    engine_ref: Some(engine_ref),
                    cmd_info: Some(cmd_info),
                    key_data: None,
                })
            }
        }
    }

    /// Complete generating establish cred encryption key
    fn end_get_establish_cred_encryption_key(
        &self,
        tag: TagId,
        ctx: GetEstablishCredEncryptionKeyCtx<E>,
    ) -> HsmResult<GetEstablishCredEncryptionKeyOut> {
        // Perform sanity check on the tag
        let engine_ref = ctx.engine_ref.as_ref().ok_or(HsmErr::InvalidState)?;

        if ctx.tag != tag {
            Err(HsmErr::PkaTagMismatch)?
        }

        // Complete the command
        let cmd_info = ctx.cmd_info.ok_or(HsmErr::InvalidState)?;
        let pka_result = engine_ref
            .deref()
            .end_ecc_gen_key(tag, cmd_info)
            .map_err(|_| HsmErr::EccGenKeyFailed)?;

        let pub_key_size = PkaEccPublicKey::data_len(PkaEccCurve::Ecc384);
        let priv_key_size = PkaEccPrivateKey::data_len(PkaEccCurve::Ecc384);

        // Check if creds have already been established.
        self.verify_cred_is_not_set()?;

        if let Some(key_id) = self.state.get_establish_cred_encryption_key_id() {
            // Establish cred encryption key must have already been set in a different thread,
            // throw out the result from pka
            let key = self
                .state
                .vault()
                .open_establish_cred_encryption_key(APP_VAULT_ID_FOR_INTERNAL_KEYS, key_id)?;
            let pub_key_data =
                PkaEccPublicKey::from_bytes(PkaEccCurve::Ecc384, &key.blob()?[..pub_key_size])
                    .map_err(|_| HsmErr::EccGenKeyFailed)?;

            Ok(GetEstablishCredEncryptionKeyOut {
                pub_key: pub_key_data,
                nonce: self.state.nonce(),
                new_key_id: None,
            })
        } else {
            let mut establish_cred_encryption_key_data =
                [0u8; EstablishCredEncryptionKeyKind::Ecc384 as usize];
            establish_cred_encryption_key_data[..pub_key_size]
                .copy_from_slice(&pka_result.pub_key.data[..pub_key_size]);
            establish_cred_encryption_key_data[pub_key_size..pub_key_size + priv_key_size]
                .copy_from_slice(&pka_result.priv_key.k[..priv_key_size]);

            // Create a key to import
            let key_imported = EstablishCredEncryptionKeyToImport::new(
                EstablishCredEncryptionKeyKind::Ecc384,
                EstablishCredEncryptionKeyUsage::KeyAgreement,
                &establish_cred_encryption_key_data,
            )?;

            // Import the key into the vault
            let key = self.state.vault().import_establish_cred_encryption_key(
                APP_VAULT_ID_FOR_INTERNAL_KEYS,
                &key_imported,
                KeyAvailability::App,
            )?;

            // Add the key_id to partition
            self.state
                .set_establish_cred_encryption_key_id(Some(key.id()));

            Ok(GetEstablishCredEncryptionKeyOut {
                pub_key: pka_result.pub_key,
                nonce: self.state.nonce(),
                new_key_id: Some(key.id()),
            })
        }
    }

    /// Return session encryption key if it exists, generate it otherwise.
    fn begin_get_session_encryption_key(
        &self,
        tag: TagId,
    ) -> HsmResult<GetSessionEncryptionKeyCtx<E>> {
        // Acquire the PKA engine
        //
        // Note: The engine acquisition can fail with HsmErr::Pending if the
        // engine is busy.
        let engine_ref = self
            .state
            .env()
            .pka_engine()
            .acquire(tag, None)
            .ok_or(HsmErr::Pending)?;

        // Check if creds have already been established.
        self.verify_cred_is_set()?;

        match self.state.get_session_encryption_key_id() {
            Some(key_id) => {
                let key = self
                    .state
                    .vault()
                    .open_session_encryption_key(APP_VAULT_ID_FOR_INTERNAL_KEYS, key_id)?;
                let pub_key_size = PkaEccPublicKey::data_len(PkaEccCurve::Ecc384);
                let pub_key_data =
                    PkaEccPublicKey::from_bytes(PkaEccCurve::Ecc384, &key.blob()?[..pub_key_size])
                        .map_err(|_| HsmErr::EccGenKeyFailed)?;

                let key_data = CredentialEncryptionKeyData {
                    pub_key_data,
                    nonce: self.state.nonce(),
                };

                Ok(GetSessionEncryptionKeyCtx {
                    tag,
                    engine_ref: Some(engine_ref),
                    cmd_info: None,
                    key_data: Some(key_data),
                })
            }
            None => {
                // Submit the PKA command to the engine
                let cmd_info = engine_ref
                    .deref()
                    .begin_ecc_gen_key(tag, PkaEccCurve::Ecc384)
                    .map_err(|_| HsmErr::EccGenKeyFailed)?;

                Ok(GetSessionEncryptionKeyCtx {
                    tag,
                    engine_ref: Some(engine_ref),
                    cmd_info: Some(cmd_info),
                    key_data: None,
                })
            }
        }
    }

    /// Complete generating session encryption key
    fn end_get_session_encryption_key(
        &self,
        tag: TagId,
        ctx: GetSessionEncryptionKeyCtx<E>,
    ) -> HsmResult<GetSessionEncryptionKeyOut> {
        // Perform sanity check on the tag
        let engine_ref = ctx.engine_ref.as_ref().ok_or(HsmErr::InvalidState)?;

        if ctx.tag != tag {
            Err(HsmErr::PkaTagMismatch)?
        }

        // Complete the command
        let cmd_info = ctx.cmd_info.ok_or(HsmErr::InvalidState)?;
        let pka_result = engine_ref
            .deref()
            .end_ecc_gen_key(tag, cmd_info)
            .map_err(|_| HsmErr::EccGenKeyFailed)?;

        let pub_key_size = PkaEccPublicKey::data_len(PkaEccCurve::Ecc384);
        let priv_key_size = PkaEccPrivateKey::data_len(PkaEccCurve::Ecc384);

        if let Some(key_id) = self.state.get_session_encryption_key_id() {
            // Session encryption key must have already been set in a different thread,
            // throw out the result from pka
            let key = self
                .state
                .vault()
                .open_session_encryption_key(APP_VAULT_ID_FOR_INTERNAL_KEYS, key_id)?;
            let pub_key_data =
                PkaEccPublicKey::from_bytes(PkaEccCurve::Ecc384, &key.blob()?[..pub_key_size])
                    .map_err(|_| HsmErr::EccGenKeyFailed)?;

            Ok(GetSessionEncryptionKeyOut {
                pub_key: pub_key_data,
                nonce: self.state.nonce(),
                new_key_id: None,
            })
        } else {
            let mut session_encryption_key_data = [0u8; SessionEncryptionKeyKind::Ecc384 as usize];
            session_encryption_key_data[..pub_key_size]
                .copy_from_slice(&pka_result.pub_key.data[..pub_key_size]);
            session_encryption_key_data[pub_key_size..pub_key_size + priv_key_size]
                .copy_from_slice(&pka_result.priv_key.k[..priv_key_size]);

            // Create a key to import
            let key_imported = SessionEncryptionKeyToImport::new(
                SessionEncryptionKeyKind::Ecc384,
                SessionEncryptionKeyUsage::KeyAgreement,
                &session_encryption_key_data,
            )?;

            // Import the key into the vault
            let key = self.state.vault().import_session_encryption_key(
                APP_VAULT_ID_FOR_INTERNAL_KEYS,
                &key_imported,
                KeyAvailability::App,
            )?;

            // Add the key_id to partition
            self.state.set_session_encryption_key_id(Some(key.id()));

            Ok(GetSessionEncryptionKeyOut {
                pub_key: pka_result.pub_key,
                nonce: self.state.nonce(),
                new_key_id: Some(key.id()),
            })
        }
    }

    /// Begin establish credential command.
    fn begin_establish_credential(
        &self,
        tag: TagId,
        pub_key: &IoMemRange,
        pota_pub_key: &IoMemRange,
    ) -> HsmResult<EstablishCredentialCtx<Self::Env>> {
        let engine_ref = self
            .state
            .env()
            .pka_engine()
            .acquire(tag, None)
            .ok_or(HsmErr::Pending)?;

        let curve_len = EccCurve::P384.len();
        let pub_key_slice = pub_key.slice();
        let pota_pub_key_slice = pota_pub_key.slice();

        if pub_key_slice.len() < curve_len * 2 {
            Err(HsmErr::EccDerKeyShorterThanCurve)?
        }

        if pota_pub_key_slice.len() < curve_len * 2 {
            Err(HsmErr::EccDerKeyShorterThanCurve)?
        }

        let (x, y) = pub_key_slice.split_at(curve_len);
        if !EccPublicKeyRangeValidation::validate(x, y, EccCurve::P384) {
            Err(HsmErr::EccPublicKeyValidationFailed)?
        }

        let (x, y) = pota_pub_key_slice.split_at(curve_len);
        if !EccPublicKeyRangeValidation::validate(x, y, EccCurve::P384) {
            Err(HsmErr::EccPublicKeyValidationFailed)?
        }

        // STEP #1: Begin montgomery constant calculation
        engine_ref
            .deref()
            .begin_montgomery_constant_calculation(tag, PkaEccCurve::Ecc384)
            .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

        // Allocate DMA buffer that SHA hardware engine can use to produce the digest data
        let digest_buf = self.dma_alloc(ShaMode::Sha384.get_digest_size_hw())?;

        Ok(EstablishCredentialCtx {
            tag,
            engine_ref,
            cmd_info: PkaEccCmd {
                curve: PkaEccCurve::Ecc384,
            },
            state: EstablishCredentialCmdState::MontgomeryConstCalc,
            digest_buf,
        })
    }

    /// Continue establish credential command.
    fn continue_establish_credential(
        &self,
        ctx: EstablishCredentialCtx<Self::Env>,
        pub_key: &IoMemRange,
        pota_pub_key: &IoMemRange,
        pota_sig: &IoMemRange,
    ) -> HsmResult<EstablishCredentialCtx<Self::Env>> {
        let (cmd_info, state) = match ctx.state {
            EstablishCredentialCmdState::MontgomeryConstCalc => {
                // STEP #2: End Montgomery constant calculation and begin ECC point validation
                ctx.engine_ref
                    .deref()
                    .end_montgomery_constant_calculation(ctx.tag)
                    .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

                ctx.engine_ref
                    .deref()
                    .begin_ecc_point_validation_zc(ctx.tag, PkaEccCurve::Ecc384, pub_key)
                    .map_err(|_| HsmErr::BeginEccPointValidationFailed)?;

                (
                    ctx.cmd_info,
                    EstablishCredentialCmdState::PublicKeyValidation,
                )
            }
            EstablishCredentialCmdState::PublicKeyValidation => {
                // STEP #3: End ECC point validation and begin POTA ECC point validation
                let result = ctx
                    .engine_ref
                    .deref()
                    .end_ecc_point_validation_zc(ctx.tag)
                    .map_err(|_| HsmErr::EndEccPointValidationFailed)?;

                // Check the result of the ECC public key validation
                // If the result is false, it means the public key validation failed.
                if !result {
                    Err(HsmErr::EccPointValidationFailed)?
                }

                ctx.engine_ref
                    .deref()
                    .begin_ecc_point_validation_zc(ctx.tag, PkaEccCurve::Ecc384, pota_pub_key)
                    .map_err(|_| HsmErr::BeginEccPointValidationFailed)?;

                (
                    ctx.cmd_info,
                    EstablishCredentialCmdState::PotaPublicKeyValidation,
                )
            }
            EstablishCredentialCmdState::PotaPublicKeyValidation => {
                // STEP #4: End POTA ECC point validation and begin ECC Verify signature
                let result = ctx
                    .engine_ref
                    .deref()
                    .end_ecc_point_validation_zc(ctx.tag)
                    .map_err(|_| HsmErr::EndEccPointValidationFailed)?;

                // Check the result of the ECC public key validation
                // If the result is false, it means the public key validation failed.
                if !result {
                    Err(HsmErr::EccPointValidationFailed)?
                }

                let partition_id_pub_key = self.state.partition_id_pub_key();

                self.sha_single_block_zc_internal(
                    ShaMode::Sha384,
                    &IoMemRange::from(partition_id_pub_key.as_ref()),
                    &mut IoMemRange::from(ctx.digest_buf.as_ref()),
                )?;

                let mut digest_mem =
                    IoMemRange::from(&ctx.digest_buf.as_ref()[..ShaMode::Sha384.get_digest_size()]);
                digest_mem.slice_mut().reverse();

                // Do ECC Verify of pota_sig with the POTA public key
                ctx.engine_ref
                    .deref()
                    .begin_ecc_verify_zc(
                        ctx.tag,
                        PkaEccCurve::Ecc384,
                        pota_pub_key,
                        &IoMemRange::from(
                            &ctx.digest_buf.as_ref()[..ShaMode::Sha384.get_digest_size()],
                        ),
                        pota_sig,
                    )
                    .map_err(|_| HsmErr::EccVerifyFailed)?;

                (ctx.cmd_info, EstablishCredentialCmdState::VerifySignature)
            }
            EstablishCredentialCmdState::VerifySignature => {
                // STEP #5: End Verify Signature and begin ECDH compute
                let result = ctx
                    .engine_ref
                    .deref()
                    .end_ecc_verify_zc(ctx.tag)
                    .map_err(|_| HsmErr::EccVerifyFailed)?;

                // Check the result of the ECC signature validation
                // If the result is false, it means the signature validation failed.
                if !result {
                    Err(HsmErr::EccVerifyFailed)?
                }

                // Begin montgomery constant calculation
                ctx.engine_ref
                    .deref()
                    .begin_montgomery_constant_calculation(ctx.tag, PkaEccCurve::Ecc384)
                    .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

                (
                    ctx.cmd_info,
                    EstablishCredentialCmdState::SecondMontgomeryConstCalc,
                )
            }
            EstablishCredentialCmdState::SecondMontgomeryConstCalc => {
                // STEP #6: End Montgomery constant calculation and begin ECC point validation
                ctx.engine_ref
                    .deref()
                    .end_montgomery_constant_calculation(ctx.tag)
                    .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

                // Get the Establish Credential encryption private key blob.
                let key_id = self
                    .state
                    .get_establish_cred_encryption_key_id()
                    .ok_or(HsmErr::EstablishCredEncryptionKeyGenerateFailed)?;
                let key = self
                    .state
                    .vault()
                    .open_establish_cred_encryption_key(APP_VAULT_ID_FOR_INTERNAL_KEYS, key_id)?;

                let pub_key_size = PkaEccPublicKey::data_len(PkaEccCurve::Ecc384);
                let priv_key_size = PkaEccPrivateKey::data_len(PkaEccCurve::Ecc384);

                let priv_key_blob = &key.blob()?[pub_key_size..pub_key_size + priv_key_size];

                // Begin ECDH compute command in PKA HW.
                let cmd_info = ctx
                    .engine_ref
                    .deref()
                    .begin_ecdh_compute_zc(ctx.tag, PkaEccCurve::Ecc384, priv_key_blob, pub_key)
                    .map_err(|_| HsmErr::EcdhComputeFailed)?;

                (cmd_info, EstablishCredentialCmdState::EcdhCompute)
            }

            _ => Err(HsmErr::InvalidState)?,
        };

        Ok(EstablishCredentialCtx {
            tag: ctx.tag,
            engine_ref: ctx.engine_ref,
            cmd_info,
            state,
            digest_buf: ctx.digest_buf,
        })
    }

    /// End establish credential command
    fn end_establish_credential(
        &self,
        ctx: EstablishCredentialCtx<Self::Env>,
        encrypted_credential: &DdiEncryptedEstablishCredential,
    ) -> HsmResult<()> {
        // Get the secret blob
        let secret_val = ctx
            .engine_ref
            .deref()
            .end_ecdh_compute(ctx.tag, ctx.cmd_info)
            .map_err(|_| HsmErr::EcdhComputeFailed)?;

        // Allocate DMA buffer for AESHMAC key data
        // hkdf_impl requires buffer with extra space for hash
        let mut aes_hmac_key_buffer = self.dma_alloc(80 + 64)?;

        let mut nonce = self.state.nonce();

        // Use hkdf on secret blob to extract 64 bytes
        self.hkdf_impl(
            secret_val.secret(),
            &[0u8; 0],
            &nonce,
            DdiHashAlgorithm::Sha384,
            aes_hmac_key_buffer.as_ref_mut(),
            80,
        )?;

        // zeroize the buffer for security
        nonce.zeroize();

        // Verify nonce one more time. We need to check this again as another FSM could
        // have reached here first.
        self.verify_nonce(encrypted_credential.nonce)?;

        // Verify cred is not set one more time. We need to check this again as another FSM could
        // have reached here first.
        self.verify_cred_is_not_set()?;

        // Verify tag
        let hmac_key = &aes_hmac_key_buffer.as_ref()[32..80];
        self.verify_encrypted_establish_credential_tag(hmac_key, encrypted_credential)?;

        // Reset nonce
        self.state.reset_nonce();

        // Decrypt credential
        let aes_key = &aes_hmac_key_buffer.as_ref()[..32];
        let iv_mem_range: IoMemRange = (&encrypted_credential.iv).into();

        // Decrypt ID
        let decrypted_id_buffer = self.dma_alloc(16)?;
        let decrypted_id_range: IoMemRange = (decrypted_id_buffer.as_ref()).into();

        self.decrypt_credential(
            ctx.tag,
            aes_key,
            encrypted_credential.encrypted_id.as_slice(),
            &iv_mem_range,
            &decrypted_id_range,
        )?;

        // Decrypt PIN
        let decrypted_pin_buffer = self.dma_alloc(16)?;
        let decrypted_pin_range: IoMemRange = (decrypted_pin_buffer.as_ref()).into();
        self.decrypt_credential(
            ctx.tag,
            aes_key,
            encrypted_credential.encrypted_pin.as_slice(),
            &iv_mem_range,
            &decrypted_pin_range,
        )?;

        // Set new credential
        self.set_user_credential(decrypted_id_range.slice(), decrypted_pin_range.slice())?;

        // Clear establish cred encryption key
        if let Some(key_id) = self.state.get_establish_cred_encryption_key_id() {
            self.state.set_establish_cred_encryption_key_id(None);

            // Delete key will never fail unless the arguments are incorrect.
            // User cannot do anything as it is all internal state.
            // Log and ignore the error
            let result = self.state.vault().delete_key(
                APP_VAULT_ID_FOR_INTERNAL_KEYS,
                KEY_TAG_UNASSIGNED,
                key_id,
                |k| self.key_in_use(k),
            );
            if result.is_err() {
                error!(
                    "Establish cred key deletion failed {:?}. Ignoring error.",
                    result.err().map(u32::from).expect("Error code")
                );
            }
        }

        Ok(())
    }

    /// Start open session command.
    fn begin_open_user_session(
        &self,
        tag: TagId,
        pub_key: &IoMemRange,
    ) -> HsmResult<OpenSessionCtx<Self::Env>> {
        let engine_ref = self
            .state
            .env()
            .pka_engine()
            .acquire(tag, None)
            .ok_or(HsmErr::Pending)?;

        // Check if pin policy allows for login
        if !self.state.pin_policy_mgr().can_login() {
            Err(HsmErr::LoginFailed)?;
        }

        let curve_len = EccCurve::P384.len();
        let pub_key_slice = pub_key.slice();

        if pub_key_slice.len() < curve_len * 2 {
            Err(HsmErr::EccDerKeyShorterThanCurve)?
        }

        let (x, y) = pub_key_slice.split_at(curve_len);
        if !EccPublicKeyRangeValidation::validate(x, y, EccCurve::P384) {
            Err(HsmErr::EccPublicKeyValidationFailed)?
        }

        // STEP #1: Begin montgomery constant calculation
        engine_ref
            .deref()
            .begin_montgomery_constant_calculation(tag, PkaEccCurve::Ecc384)
            .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

        Ok(OpenSessionCtx {
            tag,
            engine_ref,
            cmd_info: PkaEccCmd {
                curve: PkaEccCurve::Ecc384,
            },
            state: OpenSessionCmdState::MontgomeryConstCalc,
        })
    }

    /// Continue open session command.
    fn continue_open_user_session(
        &self,
        ctx: OpenSessionCtx<Self::Env>,
        pub_key: &IoMemRange,
    ) -> HsmResult<OpenSessionCtx<Self::Env>> {
        let (cmd_info, state) = match ctx.state {
            OpenSessionCmdState::MontgomeryConstCalc => {
                // STEP #2: End Montgomery constant calculation and begin ECC point validation
                ctx.engine_ref
                    .deref()
                    .end_montgomery_constant_calculation(ctx.tag)
                    .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

                ctx.engine_ref
                    .deref()
                    .begin_ecc_point_validation_zc(ctx.tag, PkaEccCurve::Ecc384, pub_key)
                    .map_err(|_| HsmErr::BeginEccPointValidationFailed)?;

                (ctx.cmd_info, OpenSessionCmdState::PublicKeyValidation)
            }
            OpenSessionCmdState::PublicKeyValidation => {
                // STEP #3: End ECC point validation and begin ECDH compute
                let result = ctx
                    .engine_ref
                    .deref()
                    .end_ecc_point_validation_zc(ctx.tag)
                    .map_err(|_| HsmErr::EndEccPointValidationFailed)?;

                // Check the result of the ECC public key validation
                // If the result is false, it means the public key validation failed.
                if !result {
                    Err(HsmErr::EccPointValidationFailed)?
                }

                // Get the Param Encryption private key blob.
                let key_id = self
                    .state
                    .get_session_encryption_key_id()
                    .ok_or(HsmErr::SessionEncryptionKeyGenerateFailed)?;
                let key = self
                    .state
                    .vault()
                    .open_session_encryption_key(APP_VAULT_ID_FOR_INTERNAL_KEYS, key_id)?;

                let pub_key_size = PkaEccPublicKey::data_len(PkaEccCurve::Ecc384);
                let priv_key_size = PkaEccPrivateKey::data_len(PkaEccCurve::Ecc384);

                let priv_key_blob = &key.blob()?[pub_key_size..pub_key_size + priv_key_size];

                // Begin ECDH compute command in PKA HW.
                let cmd_info = ctx
                    .engine_ref
                    .deref()
                    .begin_ecdh_compute_zc(ctx.tag, PkaEccCurve::Ecc384, priv_key_blob, pub_key)
                    .map_err(|_| HsmErr::EcdhComputeFailed)?;

                (cmd_info, OpenSessionCmdState::EcdhCompute)
            }
            _ => Err(HsmErr::InvalidState)?,
        };

        Ok(OpenSessionCtx {
            tag: ctx.tag,
            engine_ref: ctx.engine_ref,
            cmd_info,
            state,
        })
    }

    /// End open app session command, after calling begin_open_session and continue_open_session
    fn end_open_user_session(
        &self,
        ctx: OpenSessionCtx<Self::Env>,
        rev: DdiApiRev,
        encrypted_credential: &DdiEncryptedSessionCredential,
        reopen_sess_id: Option<u16>,
        bk_session_buf: &mut [u8],
        mk_session_buf: &mut [u8],
        bmk_session: Option<&[u8]>,
    ) -> HsmResult<Self::UserSession> {
        // STEP #3: End ECDH secret computation and perform key derivation to decrypt credential.

        // Get the secret blob
        let secret_val = ctx
            .engine_ref
            .deref()
            .end_ecdh_compute(ctx.tag, ctx.cmd_info)
            .map_err(|_| HsmErr::EcdhComputeFailed)?;

        // Allocate DMA buffer for AESHMAC key data
        // hkdf_impl requires buffer with extra space for hash
        let mut aes_hmac_key_buffer = self.dma_alloc(80 + 64)?;

        let mut nonce = self.state.nonce();

        // Use hkdf on secret blob to extract 64 bytes
        self.hkdf_impl(
            secret_val.secret(),
            &[0u8; 0],
            &nonce,
            DdiHashAlgorithm::Sha384,
            aes_hmac_key_buffer.as_ref_mut(),
            80,
        )?;

        // zeroize the buffer for security
        nonce.zeroize();

        // Verify nonce one more time. We need to check this again as another FSM could
        // have reached here first.
        self.verify_nonce(encrypted_credential.nonce)?;

        // Verify tag
        let hmac_key = &aes_hmac_key_buffer.as_ref()[32..80];
        self.verify_encrypted_session_credential_tag(hmac_key, encrypted_credential)?;

        // Reset nonce
        self.state.reset_nonce();

        // Decrypt credential
        let aes_key = &aes_hmac_key_buffer.as_ref()[..32];
        let iv_mem_range: IoMemRange = (&encrypted_credential.iv).into();

        // Decrypt ID
        let decrypted_id_buffer = self.dma_alloc(16)?;
        let decrypted_id_range: IoMemRange = (decrypted_id_buffer.as_ref()).into();
        self.decrypt_credential(
            ctx.tag,
            aes_key,
            encrypted_credential.encrypted_id.as_slice(),
            &iv_mem_range,
            &decrypted_id_range,
        )?;

        // Decrypt PIN
        let decrypted_pin_buffer = self.dma_alloc(16)?;
        let decrypted_pin_range: IoMemRange = (decrypted_pin_buffer.as_ref()).into();
        self.decrypt_credential(
            ctx.tag,
            aes_key,
            encrypted_credential.encrypted_pin.as_slice(),
            &iv_mem_range,
            &decrypted_pin_range,
        )?;

        // Decrypt session seed
        let decrypted_seed_buffer = self.dma_alloc(48)?;
        let decrypted_seed_range: IoMemRange = (decrypted_seed_buffer.as_ref()).into();
        self.decrypt_credential(
            ctx.tag,
            aes_key,
            encrypted_credential.encrypted_seed.as_slice(),
            &iv_mem_range,
            &decrypted_seed_range,
        )?;

        let mut decrypted_id: SecureByteArray<16> = [0u8; 16].into();
        decrypted_id.copy_from_slice(decrypted_id_buffer.as_ref());

        let mut decrypted_pin = [0u8; 16];
        decrypted_pin.copy_from_slice(decrypted_pin_buffer.as_ref());

        // Authorize the user credential
        self.authorize_user_with_pin_policy(&decrypted_id, &decrypted_pin)?;

        let mut rev_buf: SecureByteArray<8> = [0; 8].into();
        rev_buf[..4].copy_from_slice(&rev.major.to_le_bytes());
        rev_buf[4..].copy_from_slice(&rev.minor.to_le_bytes());

        // Generate BK session
        let bks1 = self.state.bks_table().get_bks1_current();
        self.generate_bk_session(&bks1, decrypted_seed_buffer.as_ref(), bk_session_buf)?;

        if reopen_sess_id.is_some() {
            // This is a reopen session operation, import session masking key from BMK
            self.import_smk_from_bmk(
                decrypted_seed_buffer.as_ref(),
                bmk_session.ok_or(HsmErr::InvalidState)?,
                mk_session_buf,
            )?;
        } else {
            // This is an open session operation, generate session masking
            LMKeyDerive::generate_mk(
                self,
                mk_session_buf
                    .try_into()
                    .map_err(|_| HsmErr::InvalidArgument)?,
            )?;
        }

        // Create a new user session
        let mut vault = self.state.vault();
        let key_to_import = SessionKeyToImport::new(
            SessionKeyKind::Session,
            SessionKeyUsage::Session,
            rev_buf.as_ref(),
            mk_session_buf,
        )?;

        // Slot availability only applies to a *new* OpenSession. A ReopenSession
        // targets a session_id whose slot is already allocated (alloc=1, reneg=1):
        // `recreate_session` simply clears the renegotiation bit on the existing
        // slot, so it does not need a free slot. Gating this check is required to
        // support lazy renegotiation after Live Migration when other sessions may
        // have filled the table in the meantime.
        if reopen_sess_id.is_none() && self.state.session_table().get_available_session_count() == 0
        {
            Err(HsmErr::SessionLimitReached)?
        }

        let session_key =
            vault.import_session_key(self.state.cred_mgr().get_user_vault_id(), &key_to_import)?;

        let session_id = if let Some(sess_id) = reopen_sess_id {
            self.state
                .session_table()
                .recreate_session(sess_id, session_key.id())?;

            sess_id
        } else {
            // This should never fail as we already checked if session is available to be created
            self.state
                .session_table()
                .create_session(session_key.id())?
        };

        Ok(Self::UserSession::new(rev, session_id, self.state.clone()))
    }

    /// Begin ECC PCT validation operation.
    fn begin_ecc_pct_validation(
        &self,
        tag: TagId,
        key_id: KeyId,
        usage: EccKeyUsage,
        public_key: PkaEccPublicKey,
    ) -> HsmResult<EccKeyPct<Self::Env>> {
        // Acquire the PKA engine
        //
        // Note: The engine acquisition can fail with HsmErr::Pending if the
        // engine is busy.
        let engine_ref = self
            .state
            .env()
            .pka_engine()
            .acquire(tag, Some(key_id))
            .ok_or(HsmErr::Pending)?;

        // Fetch priv bytes from vault, then delegate to *_raw
        let key = self.state.vault().key(
            APP_VAULT_ID_FOR_INTERNAL_KEYS,
            KEY_TAG_UNASSIGNED,
            key_id,
            false,
        )?;
        let curve = public_key.curve;
        let pub_len = PkaEccPublicKey::data_len(curve);
        let priv_len = PkaEccPrivateKey::data_len(curve);
        let priv_d = &key.blob()?[pub_len..pub_len + priv_len];

        self.begin_ecc_pct_validation_with_engine(tag, usage, &public_key, priv_d, engine_ref)
    }

    /// Begin ECC PCT validation operation with raw private key bytes.
    fn begin_ecc_pct_validation_raw(
        &self,
        tag: TagId,
        usage: EccKeyUsage,
        public_key: &PkaEccPublicKey,
        priv_d: &[u8],
    ) -> HsmResult<EccKeyPct<Self::Env>> {
        // Acquire the PKA engine
        //
        // Note: The engine acquisition can fail with HsmErr::Pending if the
        // engine is busy.
        let engine_ref = self
            .state
            .env()
            .pka_engine()
            .acquire(tag, None)
            .ok_or(HsmErr::Pending)?;

        self.begin_ecc_pct_validation_with_engine(tag, usage, public_key, priv_d, engine_ref)
    }

    /// Begin ECC PCT validation operation with provided PKA engine.
    fn begin_ecc_pct_validation_with_engine(
        &self,
        tag: TagId,
        usage: EccKeyUsage,
        public_key: &PkaEccPublicKey,
        priv_d: &[u8],
        engine_ref: PkaEngineRef<Self::Env>,
    ) -> HsmResult<EccKeyPct<Self::Env>> {
        let curve = public_key.curve;
        let pub_len = PkaEccPublicKey::data_len(curve);
        let priv_len = PkaEccPrivateKey::data_len(curve);
        if priv_d.len() != priv_len {
            return Err(HsmErr::InvalidArgument);
        }

        // Single contiguous DMA buffer: [ pub | priv ]
        let mut dma_buf = self.dma_alloc(pub_len + priv_len)?;
        let buf = dma_buf.as_ref_mut();

        // copy public key into buffer
        #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
        {
            if let Some(neg_cnt) = self.neg_pct_skip_cnt(None) {
                if neg_cnt == 0 {
                    match curve {
                        PkaEccCurve::Ecc256 => buf[..pub_len]
                            .copy_from_slice(&NEGATIVE_PCT_ECC_PUBLIC_KEYS[0].data[..pub_len]),
                        PkaEccCurve::Ecc384 => buf[..pub_len]
                            .copy_from_slice(&NEGATIVE_PCT_ECC_PUBLIC_KEYS[1].data[..pub_len]),
                        PkaEccCurve::Ecc521 => buf[..pub_len]
                            .copy_from_slice(&NEGATIVE_PCT_ECC_PUBLIC_KEYS[2].data[..pub_len]),
                    }
                } else {
                    let _ = self.neg_pct_skip_cnt(Some(neg_cnt - 1));
                    buf[..pub_len].copy_from_slice(&public_key.data[..pub_len]);
                }
            } else {
                buf[..pub_len].copy_from_slice(&public_key.data[..pub_len]);
            }
        }
        #[cfg(any(
            not(feature = "mcr_test_hooks"),
            not(feature = "fips_validation_hooks")
        ))]
        {
            buf[..pub_len].copy_from_slice(&public_key.data[..pub_len]);
        }

        // Private key
        buf[pub_len..pub_len + priv_len].copy_from_slice(priv_d);

        let pub_key_blob = IoMemRange::from(&buf[..pub_len]);
        let priv_key_blob = IoMemRange::from(&buf[pub_len..pub_len + priv_len]);
        // Op scratch DMA (same sizing you already use)
        let ecc_data_buffer_size =
            (PkaEccCurve::MAX_LEN * 2) + PkaEccCurve::MAX_LEN + PkaEccCurve::MAX_LEN;
        let op_dma_buf = self.dma_alloc(ecc_data_buffer_size)?;

        // Build erased engine from env (PKA + SHA)
        let sha = self.state.env().sha().clone();
        let engine: Box<dyn PctEngine> = Box::new(PctEngineImpl::new(engine_ref, sha));

        let mut ecc_key_pct = EccKeyPct::new(
            priv_key_blob,
            pub_key_blob,
            dma_buf,
            curve,
            op_dma_buf,
            engine,
        );

        ecc_key_pct.begin_ecc_pct_validation_inner(tag, usage)?;
        Ok(ecc_key_pct)
    }

    /// Continue an ECC PCT validation operation.
    fn continue_ecc_pct_validation(
        &self,
        tag: TagId,
        ecc_key_pct: &mut EccKeyPct<Self::Env>,
    ) -> HsmResult<()> {
        ecc_key_pct.continue_ecc_pct_validation_inner(tag)
    }

    /// End an ECC PCT validation operation.
    fn end_ecc_pct_validation(
        &self,
        tag: TagId,
        ecc_key_pct: &mut EccKeyPct<Self::Env>,
    ) -> HsmResult<bool> {
        ecc_key_pct.end_ecc_pct_validation_inner(tag)
    }

    /// Checks if the PCT validation state requires final verification
    fn is_pct_final_state(&self, op: &EccKeyPct<E>) -> bool {
        matches!(
            op.pct_state,
            EccPctValidationState::WaitForVerify | EccPctValidationState::EcdhComputeSecond
        )
    }

    // Authorize user with pin policy
    fn authorize_user_with_pin_policy(&self, id: &AppId, pin: &AppPin) -> HsmResult<()> {
        let auth_result = self.state.cred_mgr().authorize_user(id, pin);

        self.state
            .pin_policy_mgr_mut()
            .enforce_pin_policy(auth_result)
    }

    // Verify nonce is the same
    fn verify_nonce(&self, nonce: [u8; 32]) -> HsmResult<()> {
        if nonce != self.state.nonce() {
            Err(HsmErr::NonceMismatch)?;
        }

        Ok(())
    }

    // Verify credential has not been established
    fn verify_cred_is_not_set(&self) -> HsmResult<()> {
        if self.state.verify_user_cred_is_set() {
            Err(HsmErr::AppLimitReached)?;
        }

        Ok(())
    }

    // Verify credential has been established
    fn verify_cred_is_set(&self) -> HsmResult<()> {
        if !self.state.verify_user_cred_is_set() {
            Err(HsmErr::CredentialsNotEstablished)?;
        }

        Ok(())
    }

    /// Clear credentials
    fn clear_credentials(&self) -> HsmResult<()> {
        self.state.cred_mgr_mut().clear();

        Ok(())
    }

    /// Clear provisioning state
    fn clear_provisioning_state(&self) -> HsmResult<()> {
        self.state.clear_partition_provisioning_state();

        Ok(())
    }

    #[cfg(feature = "mcr_test_hooks")]
    /// Set current SVN value
    ///
    /// # Arguments
    /// * `svn` - SVN value to set
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)`
    fn set_current_svn(&self, svn: u64) -> HsmResult<()> {
        let table = self.state.bks_table();
        table.set_current_svn(svn);
        self.state.set_bks_table(table);

        Ok(())
    }

    /// Store the partition data in GSRAM store
    fn store_data(&self) {
        self.state.store_date()
    }

    /// Set cert chain lengths context
    fn set_cert_chain_lengths_info(&self, ctx: Option<GetCertChainLengthsInfo>) {
        self.state.set_cert_chain_lengths_info(ctx);
    }

    /// Get certificate length
    fn get_cert_len(&self, cert_id: u8) -> Option<usize> {
        let info = self.state.get_cert_chain_lengths_info()?;

        let num_certs = info.num_certs;
        if cert_id < num_certs.saturating_sub(2) {
            Some(info.cert_lengths[cert_id as usize] as usize)
        } else if cert_id == num_certs.saturating_sub(2) {
            Some(self.get_alias_cert_len())
        } else if cert_id == num_certs.saturating_sub(1) {
            Some(self.partition_cert_length() as usize)
        } else {
            None
        }
    }

    /// Delete internal key from key vault
    fn delete_internal_key(&self, key_id: KeyId) -> HsmResult<()> {
        self.state.vault().delete_key(
            APP_VAULT_ID_FOR_INTERNAL_KEYS,
            KEY_TAG_UNASSIGNED,
            key_id,
            |k| self.key_in_use(k),
        )
    }

    /// Unset the establish credential encryption key id
    fn unset_establish_cred_encryption_key_id(&self) {
        self.state.set_establish_cred_encryption_key_id(None);
    }

    /// Unset the session encryption key id
    fn unset_session_encryption_key_id(&self) {
        self.state.set_session_encryption_key_id(None);
    }

    /// Check if the module is FIPS approved.
    fn is_fips_approved(&self) -> bool {
        self.state.is_fips_approved()
    }

    /// Set the partition GUID
    fn set_vm_launch_guid(&self, guid: &VmLaunchGuid) {
        self.state.set_vm_launch_guid(guid);
    }

    /// Get the partition GUID
    fn vm_launch_guid(&self) -> VmLaunchGuid {
        self.state.vm_launch_guid()
    }

    /// Return partition identifiers if they exist, otherwise generate them.
    fn begin_generate_partition_identifiers(&self, tag: TagId) -> HsmResult<GetPartitionIdCtx> {
        // Acquire the PKA engine
        //
        // Note: The engine acquisition can fail with HsmErr::Pending if the
        // engine is busy.
        let engine_ref = self
            .state
            .env()
            .pka_engine()
            .acquire(tag, None)
            .ok_or(HsmErr::Pending)?;

        // check if partition identifiers are already present
        if self.state.partition_id_valid() {
            // If partition identifiers are already present, return the context
            return Ok(GetPartitionIdCtx {
                tag,
                cmd_info: None,
                identifiers_present: true,
                engine: None,
            });
        }

        // First, lets generate the Partition ID.
        let mut pid: SecureByteArray<16> = [0u8; 16].into();
        self.state.env().rng().bytes(pid.as_mut());
        self.state.set_partition_id(&pid);

        // Submit the PKA command to the engine
        let cmd_info = engine_ref
            .deref()
            .begin_ecc_gen_key(tag, PkaEccCurve::Ecc384)
            .map_err(|_| HsmErr::EccGenKeyFailed)?;

        Ok(GetPartitionIdCtx {
            tag,
            cmd_info: Some(cmd_info),
            identifiers_present: false,
            engine: Some(Box::new(PkaEngineRefBox(engine_ref))),
        })
    }

    // End Get Partition Identifiers
    fn continue_generate_partition_identifiers(
        &self,
        tag: TagId,
        mut ctx: GetPartitionIdCtx,
    ) -> HsmResult<PartitionIdGenResult> {
        // Perform sanity checks
        let engine = ctx.engine.take().ok_or(HsmErr::InvalidState)?;

        if ctx.tag != tag {
            Err(HsmErr::PkaTagMismatch)?
        }

        // Complete the command
        let cmd_info = ctx.cmd_info.ok_or(HsmErr::InvalidState)?;
        let pka_result = engine
            .end_ecc_gen_key(tag, cmd_info)
            .map_err(|_| HsmErr::EccGenKeyFailed)?;

        let pub_key_size = PkaEccPublicKey::data_len(PkaEccCurve::Ecc384);
        let priv_key_size = PkaEccPrivateKey::data_len(PkaEccCurve::Ecc384);

        Ok(PartitionIdGenResult {
            curve: PkaEccCurve::Ecc384,
            // keep native order for PCT; don't reverse yet
            pub_xy: SecureByteVec::from(&pka_result.pub_key.data[..pub_key_size]),
            priv_d: SecureByteVec::from(&pka_result.priv_key.k[..priv_key_size]),
        })
    }

    /// Commit the partition identifiers to persistent storage
    fn end_generate_partition_identifiers(&self, mut km: PartitionIdGenResult) -> HsmResult<()> {
        let n = km.curve.len();

        // transform for storage (reverse limbs)
        km.pub_xy[..n].reverse();
        km.pub_xy[n..2 * n].reverse();

        self.state.set_partition_id_priv_key(&km.priv_d);
        self.state.set_partition_id_pub_key(&km.pub_xy);
        self.state.set_partition_id_valid(true);

        Ok(())
    }

    /// Get the partition ID private key
    fn get_partition_id_private_key_blob(&self) -> Option<&[u8]> {
        if self.state.partition_id_valid() {
            Some(self.state.partition_id_priv_key())
        } else {
            None
        }
    }

    /// Begin generate PID certificate command
    fn begin_generate_pid_cert(
        &self,
        tag: TagId,
        key_blob: &[u8],
    ) -> HsmResult<CertSignContext<E>> {
        let engine_ref = self
            .state
            .env()
            .pka_engine()
            .acquire(tag, None)
            .ok_or(HsmErr::Pending)?;

        // Allocate the signature DMA buffer for the partition certificate.
        let signature_buf = self.dma_alloc(SIGNATURE_SIZE)?;
        let signature_range = IoMemRange::from(signature_buf.as_ref());

        // Allocate the TBS digest buffer for SHA384
        let tbs_digest_buf = self.dma_alloc(ShaMode::Sha384.get_digest_size_hw())?;

        let public_key: &[u8; 97] = match self.state.partition_id_valid() {
            true => self.state.partition_id_pub_key(),
            false => return Err(HsmErr::InvalidState),
        };

        let dma_buf_for_subject = self.dma_copy_alloc(public_key.as_ref())?;

        let mut serial_number_bytes = SecureByteArray::<20>::new([0u8; 20]);
        self.state.env().rng().bytes(serial_number_bytes.as_mut());

        // In certain cases, rng may generate leading zeros. ASN.1 encoding does not like
        // leading zeros in integer. Additionally, the serial number must be a 2's complement
        // positive integer (MSB is 0).
        serial_number_bytes[0] = (serial_number_bytes[0] & 0x7F) | 0x01;

        // Set the subject_sn to the partition ID
        let part_id: [u8; 16] = self.state.partition_id();
        let subject_sn_hex: SecureByteVec = part_id
            .encode_hex::<String>()
            .to_uppercase()
            .into_bytes()
            .into();
        let subject_sn: &[u8; 32] = subject_sn_hex
            .as_slice()
            .try_into()
            .map_err(|_| HsmErr::PartitionCertInvalidTypeConversion)?;

        // Get alias cert from GSRAM and extract subject CN and subject key ID from it.
        // These fields become the current certificates issuer CN and issuer key ID respectively.
        let subject_key_id_buffer = self.dma_alloc(ShaMode::Sha1.get_digest_size_hw())?;
        let mut subject_key_id_range = subject_key_id_buffer.as_ref().into();

        self.sha_single_block_zc_internal(
            ShaMode::Sha1,
            &dma_buf_for_subject.as_ref().into(),
            &mut subject_key_id_range,
        )?;

        // Convert DMA buffer to fixed-size array
        let subject_key_id: &[u8; 20] = subject_key_id_buffer.as_ref()[..20]
            .try_into()
            .map_err(|_| HsmErr::PartitionCertInvalidTypeConversion)?;

        let tbs = {
            let alias_cert_range = self.get_alias_cert();
            let alias_cert = alias_cert_range.slice();

            let issuer_sn: &[u8; 64usize] = der::get_subject_cn(alias_cert)
                .ok_or(HsmErr::InvalidArgument)?
                .try_into()
                .map_err(|_| HsmErr::PartitionCertInvalidTypeConversion)?;

            // Authority ID key is not present in the alias key, lets resort to default.
            let authority_key_id: [u8; 20] = der::get_subject_key_identifier(alias_cert)
                .unwrap_or_default()
                .try_into()
                .map_err(|_| HsmErr::PartitionCertInvalidTypeConversion)?;

            let not_before = &NotBefore::default().value;
            let not_after = &NotAfter::default().value;

            let params = AzihsmLeafCertTbsParams {
                serial_number: &serial_number_bytes,
                public_key,
                subject_sn,
                issuer_sn,
                subject_key_id,
                authority_key_id: &authority_key_id,
                not_before,
                not_after,
            };

            let tbs = AzihsmLeafCertTbs::new(&params);

            // Generate the ECC-SHA384 digest of the TBS and create the signature from it.
            let sha384_buff = self.dma_copy_alloc(tbs.tbs())?;

            // Use direct buffer allocation for SHA384 result
            let mut digest_hw_range = tbs_digest_buf.as_ref().into();
            self.sha_single_block_zc_internal(
                ShaMode::Sha384,
                &sha384_buff.as_ref().into(),
                &mut digest_hw_range,
            )?;

            // Digest is output as big endian from SHA HW.
            // Digest needs to be reversed before passing to PKA for signing.
            let mut digest_range =
                IoMemRange::from(&tbs_digest_buf.as_ref()[..ShaMode::Sha384.get_digest_size()]);
            let digest_slice = digest_range.slice_mut();
            digest_slice.reverse();

            self.begin_ecc_sign_with_priv_key_internal_with_engine_acquired(
                tag,
                &digest_range,
                key_blob,
                &engine_ref,
                &signature_range,
            )?;

            tbs
        };

        Ok(CertSignContext {
            tbs,
            _tbs_digest_buf: tbs_digest_buf,
            signature_buf,
            engine_ref,
        })
    }

    fn end_generate_pid_cert(
        &self,
        tag: TagId,
        cert_sign_ctx: &CertSignContext<E>,
    ) -> HsmResult<()> {
        let engine_ref = &cert_sign_ctx.engine_ref;

        self.end_ecc_sign_with_priv_key_internal(tag, engine_ref)
    }

    /// Get Partition Certificate's IoMemRange
    fn partition_cert(&self) -> IoMemRange {
        self.state.get_partition_cert()
    }

    /// Set Partition Certificate length
    fn set_partition_cert_length(&self, len: u32) -> HsmResult<()> {
        self.state.set_partition_cert_length(len)
    }

    /// Get Partition Certificate length
    fn partition_cert_length(&self) -> u32 {
        self.state.get_partition_cert_length()
    }

    /// Check if partition certificate is valid
    fn is_partition_cert_valid(&self) -> bool {
        self.state.is_partition_cert_valid()
    }

    /// Set Partition Certificate validity
    fn set_partition_cert_valid(&self, valid: bool) {
        self.state.set_partition_cert_valid(valid);
    }

    /// Get masked boot length
    fn get_masked_bk_boot_len(&self) -> u32 {
        self.state.get_masked_bk_boot_len()
    }

    /// Set Masked BK Boot length
    fn set_masked_bk_boot_len(&self, len: u32) {
        self.state.set_masked_bk_boot_len(len)
    }

    /// Get Masked BK Boot address
    fn masked_bk_boot(&self) -> IoMemRange {
        self.state.masked_bk_boot()
    }

    /// Get sealed BK3 length
    fn get_sealed_bk3_len(&self) -> u32 {
        self.state.get_sealed_bk3_len()
    }

    /// Set sealed BK3 length
    fn set_sealed_bk3_len(&self, len: u32) {
        self.state.set_sealed_bk3_len(len)
    }

    /// Sealed BK3
    fn sealed_bk3(&self) -> IoMemRange {
        self.state.sealed_bk3()
    }

    /// Begin get device ID certificate chain
    fn begin_get_dev_id_cert_chain_info(
        &self,
        tag: TagId,
        get_cert_chain_lengths_ctx: &mut GetCertLengthsContext<E>,
    ) -> HsmResult<()> {
        self.begin_get_dev_id_cert_chain_inner(tag, get_cert_chain_lengths_ctx)
    }

    /// End get device ID certificate chain
    fn end_get_dev_id_cert_chain_info(
        &self,
        get_cert_chain_lengths_ctx: &mut GetCertLengthsContext<E>,
    ) -> HsmResult<()> {
        self.end_get_dev_id_cert_chain_info_inner(get_cert_chain_lengths_ctx)
    }

    /// Begin get certificate.
    fn begin_get_cert(&self, tag: TagId, get_cert_ctx: &mut GetCertContext<E>) -> HsmResult<()> {
        self.begin_get_cert_inner(tag, get_cert_ctx)
    }

    /// End get certificate.
    fn end_get_cert(&self, get_cert_ctx: &mut GetCertContext<E>) -> HsmResult<()> {
        self.end_get_cert_inner(get_cert_ctx)
    }

    /// Update Cert Chain Lengths Info
    fn update_cert_chain_lengths_info(
        &self,
        cert_info: &mut GetCertChainLengthsInfo,
    ) -> HsmResult<()> {
        self.update_cert_chain_lengths_info_inner(cert_info)
    }

    /// Get ECDSA384 signature from buffer
    fn get_ecdsa384_signature_from_buffer(
        &self,
        signature_buffer: &[u8],
    ) -> HsmResult<Ecdsa384Signature> {
        if signature_buffer.len() != SIGNATURE_SIZE {
            Err(HsmErr::InvalidArgument)?
        }

        let curve = PkaEccCurve::Ecc384;

        let r = &signature_buffer[..curve.len()];
        let s = &signature_buffer[curve.len()..];

        let signature = Ecdsa384Signature {
            // PKA is little endian HW. Crypto standard uses big endian format for public keys.
            // Convert the public key to big endian here.
            r: {
                let mut k = [0u8; 48];
                reverse_copy_from_slice(&mut k, r);
                k
            },
            s: {
                let mut k = [0u8; 48];
                reverse_copy_from_slice(&mut k, s);
                k
            },
        };

        Ok(signature)
    }

    fn begin_signature_with_part_priv_key(
        &self,
        tag: TagId,
        key_data: &IoMemRange,
        signature: &IoMemRange,
    ) -> HsmResult<KeySignContext<E>> {
        let engine_ref = self
            .state
            .env()
            .pka_engine()
            .acquire(tag, None)
            .ok_or(HsmErr::Pending)?;

        let part_priv_key_blob = self
            .get_partition_id_private_key_blob()
            .ok_or(HsmErr::InvalidPartIdPrivKeyInternalError)?;

        let curve = PkaEccCurve::Ecc384;
        let sha_mode = ShaMode::Sha384;

        // SHA single block zero copy requires a hardware block digest sized output buffer
        let digest_buf = self.dma_alloc(sha_mode.get_digest_size_hw())?;
        let mut sha_output = IoMemRange::from(digest_buf.as_ref());

        self.sha_single_block_zc_internal(sha_mode, key_data, &mut sha_output)?;

        // Resize the digest to the actual digest size for the SHA type
        let mut digest = IoMemRange::from(&digest_buf.as_ref()[..sha_mode.get_digest_size()]);

        // Reverse endianness of digest since UPKA engine expects little endian
        let digest_slice = digest.slice_mut();
        digest_slice.reverse();

        // Submit the PKA command to the engine
        let _cmd_info = engine_ref
            .deref()
            .begin_ecc_sign_zc(tag, curve, part_priv_key_blob, &digest, signature)
            .map_err(|_| HsmErr::EccSignFailed)?;

        Ok(KeySignContext {
            engine_ref,
            _digest_buf: digest_buf,
        })
    }

    fn end_signature_with_key_blob(
        &self,
        tag: TagId,
        sign_ctx: KeySignContext<E>,
    ) -> HsmResult<()> {
        let engine_ref = sign_ctx.engine_ref;

        engine_ref
            .deref()
            .end_ecc_sign_zc(tag)
            .map_err(|_| HsmErr::EccSignFailed)?;

        Ok(())
    }

    /// Notify PCT validation failure
    fn notify_pct_validation_failure(&self, err: u32) {
        self.state.env().notify_pct_validation_failure(err);
    }

    /// Generate BK Boot
    fn generate_bk_boot(&self, bk_boot: &mut [u8]) -> HsmResult<()> {
        LMKeyDerive::bk_boot_key_gen(
            self,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            bk_boot.len(),
            bk_boot,
        )
    }

    // Mask BK3
    fn mask_bk3(
        &self,
        bk3: &[u8],
        masking_key: &[u8],
        output_len: &mut usize,
        output_buf: &mut [u8],
    ) -> HsmResult<()> {
        let curr_svn = self.state.bks_table().get_current_svn();
        let metadata = Self::get_mbor_encoded_metadata(
            Some(curr_svn),
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"BK3",
            bk3.len() as u16,
        )?;

        LMKeyDerive::masked_bk3_gen(
            self,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            masking_key,
            bk3,
            metadata.as_slice(),
            output_len,
            output_buf,
        )
    }

    /// Mask BK Boot
    fn mask_bk_boot(
        &self,
        bk_boot: &[u8],
        output_len: &mut usize,
        output_buf: &mut [u8],
    ) -> HsmResult<()> {
        let fw_secret_key_buff = self.dma_copy_alloc(&BK_BOOT_MASKING_KEY)?;
        let fw_secret_key: &[u8; 48] = fw_secret_key_buff
            .as_ref()
            .try_into()
            .map_err(|_| HsmErr::MaskingBkBootFailed)?;

        let mut bkx_buffer = self.dma_alloc(BK_BOOT_MK_AES_CBC_256_HMAC384_SIZE_BYTES)?;
        let bkx: &mut [u8; 80] = bkx_buffer
            .as_ref_mut()
            .try_into()
            .map_err(|_| HsmErr::MaskingBkBootFailed)?;

        let bks1 = self.state.bks_table().get_bks1_current();
        let bks2 = self.state.bks_table().get_bks2();
        LMKeyDerive::generate_bkx(self, &bks1, &bks2, fw_secret_key, bkx)?;

        let curr_svn = self.state.bks_table().get_current_svn();
        let metadata = Self::get_mbor_encoded_metadata(
            Some(curr_svn),
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"BKBoot",
            bk_boot.len() as u16,
        )?;

        LMKeyDerive::masked_bkboot_gen(
            self,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            bk_boot,
            bkx,
            metadata.as_slice(),
            output_len,
            output_buf,
        )
    }

    // Unmask BK3 using BK Boot. Masked BK boot is present in the part persistent store.
    fn unmask_bk3(&self, masked_bk3: &[u8], bk3: &mut [u8]) -> HsmResult<()> {
        let fw_secret_key_buff = self.dma_copy_alloc(&BK_BOOT_MASKING_KEY)?;
        let fw_secret_key: &[u8; 48] = fw_secret_key_buff
            .as_ref()
            .try_into()
            .map_err(|_| HsmErr::UnmaskingBk3Failed)?;

        // allocate a temporary storage for temporary masking key.
        let mut bkx_buffer = self.dma_alloc(BK_BOOT_MK_AES_CBC_256_HMAC384_SIZE_BYTES)?;
        let bkx: &mut [u8; 80] = bkx_buffer
            .as_ref_mut()
            .try_into()
            .map_err(|_| HsmErr::UnmaskingBk3Failed)?;
        let bks1 = self
            .get_bks1_from_bmk(masked_bk3)
            .map_err(|_| HsmErr::UnmaskingBk3Failed)?;
        let bks2 = self.state.bks_table().get_bks2();
        LMKeyDerive::generate_bkx(self, &bks1, &bks2, fw_secret_key, bkx)?;

        let mut bk_boot_mem = self.dma_alloc(BK_AES_CBC_256_HMAC384_SIZE_BYTES)?;
        let bk_boot_mem_array: &mut [u8; 80] = bk_boot_mem
            .as_ref_mut()
            .try_into()
            .map_err(|_| HsmErr::UnmaskingBk3Failed)?;

        let masked_bk_boot_len = self.get_masked_bk_boot_len();
        if masked_bk_boot_len == 0 {
            return Err(HsmErr::MaskedBkBootNotPresent);
        }

        let mut masked_bk_boot = self.masked_bk_boot();
        let masked_bk_boot_arr = &masked_bk_boot.slice_mut()[..masked_bk_boot_len as usize];
        LMKeyDerive::unmask_bk3(
            self,
            bkx,
            masked_bk_boot_arr,
            bk_boot_mem_array,
            masked_bk3,
            bk3,
        )?;

        Ok(())
    }

    /// Generates BK3 session data and stores it in the partition persistent storage.
    fn generate_and_store_bk3_session(&self, bk3: &[u8]) -> HsmResult<()> {
        if bk3.len() != BK3_SIZE_BYTES {
            Err(HsmErr::InvalidArgument)?;
        }

        let mut bk3_session_len = BK3_SIZE_BYTES;
        let mut bk3_session = self.dma_alloc(BK3_SIZE_BYTES)?;

        LMKeyDerive::bk3_session_gen(
            self,
            bk3.try_into().map_err(|_| HsmErr::InvalidArgument)?,
            &mut bk3_session_len,
            bk3_session.as_ref_mut(),
        )?;

        // store the BK3 session key in the part state.
        self.state.set_bk3_session(
            <&[u8] as TryInto<[u8; BK3_SIZE_BYTES]>>::try_into(bk3_session.as_ref())
                .map_err(|_| HsmErr::InvalidArgument)?
                .into(),
        );

        Ok(())
    }

    /// Generate BK.
    fn generate_bk(&self, bk3: &[u8], pota_pub_key: &[u8], bk: &mut [u8]) -> HsmResult<()> {
        if bk3.len() != BK3_SIZE_BYTES || bk.len() != BK_AES_CBC_256_HMAC384_SIZE_BYTES {
            Err(HsmErr::InvalidArgument)?;
        }

        let mut bk_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
        let bks1 = self.state.bks_table().get_bks1_current();
        let bks2 = self.state.bks_table().get_bks2();

        let bk3_array: &[u8; 48] = bk3.try_into().map_err(|_| HsmErr::InvalidArgument)?;

        LMKeyDerive::bk_partition_gen(
            self,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bks1,
            &bks2,
            bk3_array,
            pota_pub_key,
            &mut bk_len,
            bk,
        )
    }

    /// Generates a new MK and imports it to the key vault.
    fn generate_new_mk_and_import(&self) -> HsmResult<()> {
        // 1. Generate a masking key
        let mut masking_key = self.dma_alloc(MK_AES_CBC_256_HMAC384_SIZE_BYTES)?;
        let masking_key_array: &mut [u8; MK_AES_CBC_256_HMAC384_SIZE_BYTES] = masking_key
            .as_ref_mut()
            .try_into()
            .map_err(|_| HsmErr::InvalidArgument)?;
        LMKeyDerive::generate_mk(self, masking_key_array)?;

        // 2. Store the masking key in the key vault and remember the vault key ID.
        let key_imported = MaskingKeyToImport::new(
            MaskingKeyKind::AesCbc256Hmac384,
            MaskingKeyUsage::EncryptDecrypt,
            masking_key_array,
        )?;

        let key = self.state.vault().import_masking_key(
            APP_VAULT_ID_FOR_INTERNAL_KEYS,
            &key_imported,
            KeyAvailability::App,
        )?;

        // Add the key to the partition.
        self.state.set_partition_mk_id(Some(key.id()));

        Ok(())
    }

    /// This function unmasks BMK by generating the BK corresponding the right SVN
    /// that BMK came from.
    fn import_mk_from_bmk(&self, bk3: &[u8], pota_pub_key: &[u8], bmk: &[u8]) -> HsmResult<()> {
        let bks1 = self.get_bks1_from_bmk(bmk)?;
        let bks2 = self.state.bks_table().get_bks2();
        let bk3_array: &[u8; 48] = bk3.try_into().map_err(|_| HsmErr::InvalidArgument)?;

        let mut bk_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
        let mut bk = self.dma_alloc(BK_AES_CBC_256_HMAC384_SIZE_BYTES)?;
        LMKeyDerive::bk_partition_gen(
            self,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            &bks1,
            &bks2,
            bk3_array,
            pota_pub_key,
            &mut bk_len,
            bk.as_ref_mut(),
        )?;

        // Use this BK to unmask the bmk.
        let mut masking_key_raw = self.dma_alloc(MK_AES_CBC_256_HMAC384_SIZE_BYTES)?;
        let decoded_mk =
            MaskedKey::decode(self, bk.as_ref(), bmk, /*integrity_check = */ true)
                .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;
        decoded_mk
            .decrypt_key(self, bk.as_ref(), masking_key_raw.as_ref_mut())
            .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;

        // Import the masking key to key vault
        let key_imported = MaskingKeyToImport::new(
            MaskingKeyKind::AesCbc256Hmac384,
            MaskingKeyUsage::EncryptDecrypt,
            masking_key_raw.as_ref(),
        )?;

        let key = self.state.vault().import_masking_key(
            APP_VAULT_ID_FOR_INTERNAL_KEYS,
            &key_imported,
            KeyAvailability::App,
        )?;

        // Add the key to the partition.
        self.state.set_partition_mk_id(Some(key.id()));

        Ok(())
    }

    /// Generates the BMK by masking masking key with BK.
    fn generate_bmk(&self, bk: &[u8], bmk_len: &mut usize, bmk_out: &mut [u8]) -> HsmResult<()> {
        // get the masking key from key vault .
        let masking_key_id = self
            .state
            .get_partition_mk_id()
            .ok_or(HsmErr::PartitionNotProvisioned)?;
        let masking_key = self
            .state
            .vault()
            .open_masking_key(APP_VAULT_ID_FOR_INTERNAL_KEYS, masking_key_id)?;

        let masking_key_blob = masking_key.blob()?;

        //  Generate BMK.
        let curr_svn = self.state.bks_table().get_current_svn();
        let metadata = Self::get_mbor_encoded_metadata(
            Some(curr_svn),
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            Some(0),
            None,
            b"MK",
            bk.len() as u16,
        )?;

        LMKeyDerive::bmk_gen(
            self,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            bk,
            &masking_key_blob,
            &metadata,
            bmk_len,
            bmk_out,
        )?;

        Ok(())
    }

    /// Check if this partition has been provisioned already
    fn is_partition_provisioned(&self) -> bool {
        self.state.is_partition_provisioned()
    }

    /// Returns whether the session needs to be reestablished e.g. after a live migration
    fn needs_renegotiation(&self, sess_id: u16) -> bool {
        self.state.session_table().needs_renegotiation(sess_id)
    }

    /// Unmask the unwrapping key and import it into the key vault
    fn unmask_unwrapping_key_and_import(&self, masked_uk: &[u8]) -> HsmResult<()> {
        // Get the masking key from key vault
        let masking_key_id = self
            .state
            .get_partition_mk_id()
            .ok_or(HsmErr::PartitionNotProvisioned)?;
        let masking_key = self
            .state
            .vault()
            .open_masking_key(APP_VAULT_ID_FOR_INTERNAL_KEYS, masking_key_id)?;

        let masking_key_blob = masking_key.blob()?;

        // Decode masked key
        let decoded_masked_key =
            MaskedKey::decode(self, masking_key_blob.as_ref(), masked_uk, true)
                .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;

        let decoded_aes_key = decoded_masked_key
            .as_aes()
            .ok_or(HsmErr::MaskedKeyDecodeFailed)?;

        // Extract metadata
        let metadata_slice = decoded_aes_key.metadata();
        let mut decoder = MborDecoder::new(metadata_slice);
        let metadata = DdiMaskedKeyMetadata::mbor_decode(&mut decoder)
            .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;

        // Decrypt key
        let entry_kind = EntryKind::try_from(metadata.key_type)?;
        let mut decrypted_key = self
            .state
            .env()
            .dma_heap()
            .allocate(entry_kind.raw_key_blob_size())
            .ok_or(HsmErr::DmaAllocFailure)?;
        decoded_masked_key
            .decrypt_key(self, masking_key_blob.as_ref(), decrypted_key.as_ref_mut())
            .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;

        if self.state.unwrapping_key_id().is_some() {
            Err(HsmErr::InvalidState)?;
        }
        // Set unwrapping key in the persistent store
        self.state
            .part_persistent_store_ref()
            .unwrapping_key_bk
            .copy_from_slice(decrypted_key.as_ref());

        // Mark `PendingPct`: the freshly decrypted key must still run PCT on the next `GetUnwrappingKey`.
        self.state
            .part_persistent_store_ref()
            .unwrapping_key_bk_valid = UnwrappingKeyValidity::PendingPct as u8;

        // Import the incoming unwrapping key
        let key_id = self.import_unwrapping_key(decrypted_key.as_ref())?;

        // Set unwrapping key id
        self.state.set_unwrapping_key_id(Some(key_id));

        Ok(())
    }

    /// Generates the Backup Masking Key (BMK) from the provided Backup Key (BK).
    fn generate_bmk_session(
        &self,
        bk: &[u8],
        smk: &[u8],
        bmk_len: &mut usize,
        bmk_out: &mut [u8],
    ) -> HsmResult<()> {
        let curr_svn = self.state.bks_table().get_current_svn();
        let metadata = Self::get_mbor_encoded_metadata(
            Some(curr_svn),
            DdiKeyType::AesCbc256Hmac384,
            DdiMaskedKeyAttributes { blob: [0u8; 32] },
            None,
            None,
            b"SMK",
            bk.len() as u16,
        )?;

        LMKeyDerive::bmk_gen(
            self,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            bk,
            smk,
            &metadata,
            bmk_len,
            bmk_out,
        )
    }

    /// Flush session from session table
    fn flush_session(&self, session_id: u16) {
        self.state.session_table().delete(session_id)
    }

    /// Toggle current FIPS approved state
    #[cfg(feature = "fips_validation_hooks")]
    fn toggle_fips_approved_state(&self) {
        self.state.toggle_fips_approved_state()
    }

    /// Clear BK3 info, including masked bk boot and sealed bk3
    #[cfg(feature = "fips_validation_hooks")]
    fn clear_bk3_info(&self) {
        self.state.clear_bk3_info()
    }

    /// Set Test hook to trigger level 2 abort
    #[cfg(feature = "mcr_test_hooks")]
    fn set_test_hook_to_trigger_level2_abort(&self, level2_trigger: bool) {
        self.state
            .set_test_hook_to_trigger_level2_abort(level2_trigger)
    }

    /// Get Test hook to trigger level 2 abort
    #[cfg(feature = "mcr_test_hooks")]
    fn test_hook_to_trigger_level2_abort(&self) -> bool {
        self.state.test_hook_to_trigger_level2_abort()
    }

    /// Override Pin Policy Context
    #[cfg(feature = "mcr_test_hooks")]
    fn override_pin_policy_context(&self, pin_policy_config: DdiTestActionPinPolicyConfig) {
        self.state
            .pin_policy_mgr_mut()
            .override_pin_policy_context(pin_policy_config);
    }

    /// Clear Pin Policy
    #[cfg(feature = "mcr_test_hooks")]
    fn clear_pin_policy(&self) {
        self.state.pin_policy_mgr_mut().clear();
    }

    /// Set or Get the Test hook test action
    #[cfg(feature = "mcr_test_hooks")]
    fn cmd_fsm_test_action(&self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction> {
        self.state.cmd_fsm_test_action(test_action)
    }

    /// Set or Get the Test hook test action
    #[cfg(feature = "mcr_test_hooks")]
    fn hsm_fsm_test_action(&self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction> {
        self.state.hsm_fsm_test_action(test_action)
    }

    #[cfg(feature = "fips_validation_hooks")]
    fn inject_rng_hw_failure(&self, rng_hw_self_test_id: u32) {
        self.state.inject_rng_hw_failure(rng_hw_self_test_id);
    }

    /// Set the number of FSMs to skip before triggering Negative PCT Failure
    #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
    fn neg_pct_skip_cnt(&self, cnt: Option<u8>) -> Option<u8> {
        self.state.neg_pct_skip_cnt(cnt)
    }

    /// Test hook: downgrade a `PctPassed` unwrapping key back to `PendingPct` so the next
    /// `GetUnwrappingKey` re-runs the RSA PCT.  No-op for `Empty` / `PendingPct` slots.
    #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
    fn reset_unwrapping_key_pct(&self) {
        if self
            .state
            .part_persistent_store_ref()
            .unwrapping_key_bk_valid
            == UnwrappingKeyValidity::PctPassed as u8
        {
            self.state
                .part_persistent_store_ref()
                .unwrapping_key_bk_valid = UnwrappingKeyValidity::PendingPct as u8;
        }
    }

    #[cfg(all(feature = "mcr_test_hooks", feature = "mcr_test_hooks_cdma_ecc_err"))]
    fn get_corr_ecc_err_intr_count(&self) -> Option<u32> {
        self.state.env().get_corr_ecc_err_intr_count()
    }
}

impl<E: HsmEnvTrait> Partition<E> {
    const MIN_API_REV: DdiApiRev = DdiApiRev { major: 1, minor: 0 };
    const MAX_API_REV: DdiApiRev = DdiApiRev { major: 1, minor: 0 };
    const AES_IV_SIZE: usize = 16;

    /// Create a new partition
    pub fn new(pfn: PcieFunction, ctx: PartEnv<E>) -> Self {
        let state = PartState::new(pfn, ctx);
        // Generate a new nonce for the partition
        state.reset_nonce();

        Self { state }
    }

    /// Create a new partition and restore the resource table masks
    pub fn new_with_resource_table(pfn: PcieFunction, ctx: PartEnv<E>) -> Self {
        let state = PartState::new_with_resource_table(pfn, ctx);
        // Generate a new nonce for the partition
        state.reset_nonce();

        Self { state }
    }

    /// Create a new partition from backup store
    pub fn restore(pfn: PcieFunction, ctx: PartEnv<E>) -> Self {
        Self {
            state: PartState::restore(pfn, ctx),
        }
    }

    /// Import unwrapping key
    fn import_unwrapping_key(&self, key_blob: &[u8]) -> HsmResult<KeyId> {
        let key_kind = RsaKeyKind::Rsa2kPrivate;
        let key_usage = RsaKeyUsage::Unwrap;

        let unwrapping_rsa_key = RsaKeyImported::new(key_kind, key_usage, key_blob)?;

        // Import the unwrapping key into the vault
        let unwrapping_key = self.state.vault().rsa_import_key(
            APP_VAULT_ID_FOR_INTERNAL_KEYS,
            u16::MAX, // This value does not matter since the key availability is set to App
            None,
            true,
            &unwrapping_rsa_key,
            KeyAvailability::App,
        )?;

        Ok(unwrapping_key.id())
    }

    /// Generate BK Session data
    fn generate_bk_session(
        &self,
        bks1: &[u8; BK_SEED_SIZE_BYTES],
        session_seed: &[u8],
        bk_session: &mut [u8],
    ) -> HsmResult<()> {
        let bks2 = self.state.bks_table().get_bks2();
        let bk3_session = self
            .state
            .get_bk3_session()
            .ok_or(HsmErr::PartitionNotProvisioned)?;

        let mut bk_session_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;

        LMKeyDerive::bk_session_gen(
            self,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            session_seed
                .try_into()
                .map_err(|_| HsmErr::InvalidArgument)?,
            bks1,
            &bks2,
            bk3_session,
            &mut bk_session_len,
            bk_session,
        )
    }

    /// Imports the Session Masking Key (SMK) from the provided BK and BMK data.
    fn import_smk_from_bmk(
        &self,
        session_seed: &[u8],
        bmk: &[u8],
        smk: &mut [u8],
    ) -> HsmResult<()> {
        let bks1 = self.get_bks1_from_bmk(bmk)?;
        let bks2 = self.state.bks_table().get_bks2();
        let bk3_session = self
            .state
            .get_bk3_session()
            .ok_or(HsmErr::PartitionNotProvisioned)?;

        let mut bk_session_len = BK_AES_CBC_256_HMAC384_SIZE_BYTES;
        let mut bk_session = self.dma_alloc(bk_session_len)?;

        LMKeyDerive::bk_session_gen(
            self,
            MaskingKeyAlgorithm::AesCbc256Hmac384,
            session_seed
                .try_into()
                .map_err(|_| HsmErr::InvalidArgument)?,
            &bks1,
            &bks2,
            bk3_session,
            &mut bk_session_len,
            bk_session.as_ref_mut(),
        )?;

        let decoded_mk = MaskedKey::decode(
            self,
            bk_session.as_ref(),
            bmk,
            /*integrity_check = */ true,
        )
        .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;
        decoded_mk
            .decrypt_key(self, bk_session.as_ref(), smk)
            .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;

        Ok(())
    }

    /// Get BKS1 using the SVN value from BMK metadata
    fn get_bks1_from_bmk(&self, bmk: &[u8]) -> HsmResult<SecureByteArray<BK_SEED_SIZE_BYTES>> {
        // 1. Extract the SVN from BMK
        let decoded_key = MaskedKey::decode(self, &[], bmk, /*integrity_check = */ false)
            .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;
        let aes_key = decoded_key.as_aes().ok_or(HsmErr::MaskedKeyDecodeFailed)?;
        let metadata_slice = aes_key.metadata();
        let mut decoder = MborDecoder::new(metadata_slice);
        let metadata = DdiMaskedKeyMetadata::mbor_decode(&mut decoder)
            .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;

        let svn = metadata.svn.ok_or(HsmErr::MaskedKeyDecodeFailed)?;

        // 2. Generate BK for that SVN - BK'
        self.state
            .bks_table()
            .get_bks1(svn)
            .ok_or(HsmErr::MaskingBkBootFailed)
    }
}
