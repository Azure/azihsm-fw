// Copyright (c) Microsoft Corporation. All rights reserved.

mod aes;
mod ecc;
mod ecc_constants;
mod hmac;
mod kdf;
mod rsa;
mod rsa_crt;
mod sha;

use core::ops::Range;

use crate::crypto_env::CryptEnv;
#[cfg(any(feature = "mcr_test_hooks", feature = "mcr_manual_test_hooks"))]
use crate::error;
use crate::masked_key::MaskedKeyDecode;
use crate::masked_key::MaskedKeyEncode;
use crate::partition::store::EntryAttributes;
pub(crate) use aes::*;
use alloc::boxed::Box;
use alloc::vec;
pub(crate) use ecc::*;
use mcr_crypto_aes::AesCommand;
use mcr_crypto_aes::AesMode;
use mcr_crypto_aes::AesOp;
use mcr_crypto_aes::AesTrait;
use mcr_crypto_pka::*;
use mcr_ddi_mbor::MborDecode;
use mcr_ddi_mbor::MborDecoder;
use mcr_ddi_mbor::MborEncode;
use mcr_ddi_mbor::MborEncoder;
use mcr_ddi_mbor::MborLen;
use mcr_ddi_mbor::MborLenAccumulator;
use mcr_ddi_types::DdiKeyType;
use mcr_ddi_types::DdiMaskedKeyAttributes;
use mcr_ddi_types::DdiMaskedKeyMetadata;
use mcr_ddi_types::DdiRsaCryptoPadding;
use mcr_ddi_types::MaskedKey;
use mcr_ddi_types::MaskingKeyAlgorithm;
use mcr_ddi_types::AES_BLOCK_SIZE;
use mcr_ddi_types::AES_CBC_IV_SIZE;
use mcr_ddi_types::HMAC384_TAG_SIZE;
#[cfg(any(feature = "mcr_test_hooks", feature = "mcr_manual_test_hooks"))]
use mcr_ipc_controller::IpcMessageChannelTrait;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ipc_message::IpcMessageNegSelfTestReq;
#[cfg(feature = "mcr_manual_test_hooks")]
use mcr_ipc_message::IpcMessageTdispInterrupt;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ipc_message::IpcMessageTriggerCrash;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ipc_message::IpcMessageTriggerStackValidation;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ipc_message::StackErrorType;
#[cfg(any(feature = "mcr_test_hooks", feature = "mcr_manual_test_hooks"))]
use mcr_ipc_message::{IpcMessageDecoder, IpcMessageEncoderTrait};
#[cfg(any(feature = "mcr_test_hooks", feature = "mcr_manual_test_hooks"))]
use mcr_logging::*;
use pct::pct_engine::*;
use pct::pct_engine_impl::*;
pub(crate) use rsa::*;
pub(crate) use rsa_crt::*;
pub(crate) use sha::*;
use zerocopy::FromBytes;
use zerocopy::IntoBytes;
use zeroize::Zeroize;

use self::cred_mgr::APP_VAULT_ID_FOR_INTERNAL_KEYS;
use super::*;
use crate::der::DerDecoderTrait;
#[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
use crate::partition::pct::ecc_pct_constants::NEGATIVE_PCT_ECC_PUBLIC_KEYS;
#[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
use crate::partition::pct::rsa_pct_constants::*;

/// User session
pub(crate) struct UserSession<E: HsmEnvTrait + 'static> {
    /// Session ID
    pub(crate) id: SessionId,

    /// API Revision
    pub(crate) api_rev: DdiApiRev,

    /// Partition state
    pub(crate) state: PartState<E>,
}

impl<E: HsmEnvTrait> Clone for UserSession<E> {
    fn clone(&self) -> Self {
        Self {
            id: self.id,
            api_rev: self.api_rev,
            state: self.state.clone(),
        }
    }
}

impl<E: HsmEnvTrait> HsmSession for UserSession<E> {
    fn id(&self) -> SessionId {
        self.id
    }

    fn physical_session_id(&self) -> SessionId {
        self.state
            .session_table()
            .get_target_session(self.id)
            .unwrap_or(u16::MAX)
    }

    fn api_rev(&self) -> DdiApiRev {
        self.api_rev
    }

    fn invalidate(&mut self) {
        let _ = self.state.vault().disable_key(self.physical_session_id());
    }

    fn valid(&self) -> bool {
        // We will get error if the session is disabled/ invalidated
        let result = self.state.vault().open_session_key(
            self.state.cred_mgr().get_user_vault_id(),
            self.physical_session_id(),
            false,
        );

        self.state.session_table().valid(self.id()).is_ok() && result.is_ok()
    }
}

impl<E: HsmEnvTrait> CryptEnv for UserSession<E> {
    fn aescbc256_decrypt(
        &self,
        key: &[u8],
        iv: &[u8],
        ciphertext: &[u8],
        plaintext: &mut [u8],
    ) -> Result<usize, HsmErr> {
        let iv_binding = IoMemRange::from(iv);
        let msg_in_binding = IoMemRange::from(ciphertext);
        let msg_out_binding = IoMemRange::from(plaintext);

        let input = AesEncDecIn::new(
            AesEncDecMode::Cbc,
            AesEncDecOp::Decrypt,
            Some(&iv_binding),
            &msg_in_binding,
            &msg_out_binding,
        );

        self.aes_enc_dec(1, AesKeyIn::KeyBlob(key), &input)?;

        Ok(msg_out_binding.len())
    }

    fn aescbc256_enc_data_len(&self, plaintext_len: usize) -> usize {
        plaintext_len + (AES_BLOCK_SIZE - (plaintext_len % AES_BLOCK_SIZE))
    }

    fn aescbc256_encrypt(
        &self,
        key: &[u8],
        plaintext: &[u8],
        iv: &mut [u8],
        ciphertext: &mut [u8],
    ) -> Result<usize, HsmErr> {
        let mut iv_binding = IoMemRange::from(iv);
        let msg_in_binding = IoMemRange::from(plaintext);
        let msg_out_binding = IoMemRange::from(ciphertext);

        let input = AesEncDecIn::new(
            AesEncDecMode::Cbc,
            AesEncDecOp::Encrypt,
            Some(&iv_binding),
            &msg_in_binding,
            &msg_out_binding,
        );

        // Temp workaround: save a copy of iv, since it will be overwritten
        let mut iv_copy = [0u8; AES_CBC_IV_SIZE];
        iv_copy.copy_from_slice(iv_binding.slice());

        self.aes_enc_dec(1, AesKeyIn::KeyBlob(key), &input)?;

        // Copy the iv back
        // TODO: The driver implementation always writes back the IV with cipher text
        // Need to replace this by implementing an interface that leaves the IV intact for single invocation of encrypt or decrypt.
        iv_binding.slice_mut().copy_from_slice(&iv_copy[..]);

        Ok(msg_out_binding.len())
    }

    fn hmac384_tag(
        &self,
        key: &[u8],
        data: &[u8],
    ) -> Result<SecureByteArray<HMAC384_TAG_SIZE>, HsmErr> {
        let output = SecureByteArray::new([0u8; HMAC384_TAG_SIZE]);

        self.hmac_impl(
            key,
            data,
            DdiHashAlgorithm::Sha384,
            &mut IoMemRange::from(&output[..]),
        )?;

        Ok(output)
    }

    fn kbkdf_sha384(
        &self,
        _key: &[u8],
        _label: Option<&[u8]>,
        _context: Option<&[u8]>,
        _out_len: usize,
        _output: &mut [u8],
    ) -> Result<(), HsmErr> {
        todo!()
    }

    fn generate_random(&self, output: &mut [u8]) -> Result<(), HsmErr> {
        self.state.env().rng().bytes(output);
        Ok(())
    }
}

/// Data contained in the Import DER key result.
pub(crate) struct ImportDerKeyResult {
    // Private key ID of the key.
    pub(crate) priv_key_id: KeyId,

    // DER encoded public key blob.
    pub(crate) pub_key_data: Option<Vec<u8>>,

    // DDI key type.
    pub(crate) key_type: DdiKeyType,
}

/// Change Pin command context
pub(crate) struct ChangePinCmdCtx<E: HsmEnvTrait + 'static> {
    /// Tag
    pub(crate) tag: TagId,

    /// Reference to the PKA engine.
    pub(crate) engine_ref: PkaEngineRef<E>,

    /// Context for PkaEccCmd.
    pub(crate) cmd_info: PkaEccCmd,
}

/// KDF info struct
pub(crate) struct KdfInfo {
    /// Key ID
    pub(crate) key_id: KeyId,

    /// Hash algorithm
    pub(crate) hash_algo: DdiHashAlgorithm,

    /// Key type
    pub(crate) key_type: DdiKeyType,

    /// Key properties
    pub(crate) key_properties: DdiKeyProperties,

    /// Key tag
    pub(crate) key_tag: Option<u16>,
}

impl<E: HsmEnvTrait> HsmUserSession for UserSession<E> {
    type Env = E;

    /// Get the application vault id
    fn app_vault_id(&self) -> AppVaultId {
        self.state.cred_mgr().get_user_vault_id()
    }

    /// Get the application id
    fn app_id(&self) -> AppId {
        self.state.cred_mgr().user_cred().id
    }

    #[cfg(feature = "mcr_test_hooks")]
    fn send_crashdump_request(
        &self,
        tag: TagId,
        cpu_id: SocCpuId,
        crash_type: CrashType,
    ) -> HsmResult<()> {
        let msg = IpcMessageTriggerCrash {
            crash_type,
            cpu_id,
            ..Default::default()
        };

        match cpu_id {
            SocCpuId::Admin => {
                let admin_channel_ref: HsmToAdminIpcChannelRef<E> = self
                    .state
                    .env()
                    .hsm_to_admin_ipc_channel()
                    .acquire(tag, ())
                    .ok_or(HsmErr::Pending)?;

                admin_channel_ref
                    .map(|c| c.send_request(tag, msg.encode()))
                    .map_err(|err| {
                        error!(
                            "send_crashdump_request: Failed to send IPC message to Admin: {:?}",
                            err
                        );
                        HsmErr::IpcSendFailure
                    })?;
            }
            SocCpuId::Fp0 | SocCpuId::Fp1 | SocCpuId::Fp2 => {
                let fp_channel_ref: FpIpcChannelRef<E> = self
                    .state
                    .env()
                    .fp_ipc_channel()
                    .acquire(tag, ())
                    .ok_or(HsmErr::Pending)?;

                fp_channel_ref
                    .map(|c| c.send_request(tag, msg.encode()))
                    .map_err(|err| {
                        error!("[mod] Failed to send IPC message to FP: {:?}", err);
                        HsmErr::IpcSendFailure
                    })?;
            }
            _ => {
                error!("Invalid core id");
                return Err(HsmErr::InvalidArgument);
            }
        };

        Ok(())
    }

    #[cfg(feature = "mcr_test_hooks")]
    fn send_stack_validation_request(
        &self,
        tag: TagId,
        cpu_id: SocCpuId,
        error_type: StackErrorType,
    ) -> HsmResult<()> {
        let msg = IpcMessageTriggerStackValidation {
            error_type,
            cpu_id,
            ..Default::default()
        };

        match cpu_id {
            SocCpuId::Admin => {
                let admin_channel_ref: HsmToAdminIpcChannelRef<E> = self
                    .state
                    .env()
                    .hsm_to_admin_ipc_channel()
                    .acquire(tag, ())
                    .ok_or(HsmErr::Pending)?;

                admin_channel_ref
                    .map(|c| c.send_request(tag, msg.encode()))
                    .map_err(|err| {
                        error!(
                            "send_stack_validation_request: Failed to send IPC message to Admin: {:?}",
                            err
                        );
                        HsmErr::IpcSendFailure
                    })?;
            }
            _ => {
                error!("Invalid core id for stack validation");
                return Err(HsmErr::InvalidArgument);
            }
        };

        Ok(())
    }

    #[cfg(feature = "mcr_manual_test_hooks")]
    fn send_tdisp_interrupt_request(&self, tag: TagId, info: TdispInterruptInfo) -> HsmResult<()> {
        let msg = IpcMessageTdispInterrupt {
            info,
            ..Default::default()
        };

        let admin_channel_ref: HsmToAdminIpcChannelRef<E> = self
            .state
            .env()
            .hsm_to_admin_ipc_channel()
            .acquire(tag, ())
            .ok_or(HsmErr::Pending)?;

        admin_channel_ref
            .map(|c| c.send_request(tag, msg.encode()))
            .map_err(|_| HsmErr::IpcSendFailure)?;

        Ok(())
    }

    /// Get private key data from the key_id
    #[cfg(feature = "fips_validation_hooks")]
    fn get_priv_key(&mut self, key_id: KeyId, key_data: &mut IoMemRange) -> HsmResult<()> {
        // Retrieve the key from the vault
        let key = self
            .state
            .vault()
            .key(self.app_vault_id(), self.id(), key_id, true)?;

        let key_kind = key.kind()?;

        if key_kind.is_bulk_key() {
            let blob = key.blob()?;
            if blob.len() != 2 {
                return Err(HsmErr::InvalidKeyType);
            }

            let key_id = AesBulk256KeyId::new()
                .with_key_index(blob[0] & 0x07)
                .with_vault_id(((blob[0] >> 3) & 0x1F) | ((blob[1] & 0x03) << 5))
                .with_rsvd(0);

            let aes_key_array = self.state.cdma_vault().get_key_entry(key_id)?;

            key_data.slice_mut().copy_from_slice(aes_key_array.slice());
        } else {
            let blob = key.blob()?;

            key_data.slice_mut()[..blob.len()].copy_from_slice(blob.as_bytes());
        }

        Ok(())
    }

    /// Send negative self test request
    #[cfg(feature = "mcr_test_hooks")]
    fn begin_neg_self_test_req(&self, neg_self_test: SelfTest, tag: TagId) -> HsmResult<()> {
        let msg = IpcMessageNegSelfTestReq {
            test_id: neg_self_test
                .try_into()
                .map_err(|_| HsmErr::InvalidArgument)?,
            ..Default::default()
        };

        let admin_channel_ref: HsmToAdminIpcChannelRef<E> = self
            .state
            .env()
            .hsm_to_admin_ipc_channel()
            .acquire(tag, ())
            .ok_or(HsmErr::Pending)?;

        admin_channel_ref
            .map(|c| c.send_request(tag, msg.encode()))
            .map_err(|err| {
                error!(
                    "begin_neg_self_test_req: Failed to send IPC message to Admin: {:?}",
                    err
                );
                HsmErr::IpcSendFailure
            })?;

        Ok(())
    }

    /// Receive negative self test request response
    #[cfg(any(feature = "mcr_test_hooks", feature = "mcr_manual_test_hooks"))]
    fn end_neg_self_test_resp(&self, tag: TagId) -> HsmResult<()> {
        let admin_channel_ref: HsmToAdminIpcChannelRef<E> = self
            .state
            .env()
            .hsm_to_admin_ipc_channel()
            .acquire(tag, ())
            .ok_or(HsmErr::Pending)?;

        match admin_channel_ref.map(|c| c.receive_message()) {
            Some(msg) => {
                let header = IpcMessageDecoder::decode_header(&msg).map_err(|err| {
                    error!("[mod] Failed to decode IPC message header: {:?}", err);
                    HsmErr::IpcResponseError
                })?;

                if header.status() != 0 {
                    error!("Negative self test failed with status: {}", header.status());
                    Err(HsmErr::IpcResponseError)?;
                }

                Ok(())
            }
            None => Err(HsmErr::Pending),
        }
    }

    /// Open key
    fn open_key_zc(
        &mut self,
        tag: TagId,
        key_tag: u16,
        key_id: Option<KeyId>,
        key_kind: Option<EntryKind>,
        phase: OpenKeyPhase,
        is_unwrapping_key: bool,
        ecc_op: &mut Option<EccGenPubKeyCmd<E>>,
        pub_key: &IoMemRange,
    ) -> HsmResult<OpenKeyData> {
        let key_id = key_id.ok_or(HsmErr::InvalidArgument).or_else(|_| {
            self.state
                .vault()
                .get_entry_index_by_tag(self.app_vault_id(), key_tag)
        })?;

        let key = self.state.vault().key_unchecked(key_id);
        let key_kind = key_kind
            .ok_or(HsmErr::InvalidArgument)
            .or_else(|_| key.kind())?;
        let flags = key.attributes()?.common.flags;

        let (phase_out, pub_key) = match key_kind {
            EntryKind::Ecc256Private | EntryKind::Ecc384Private | EntryKind::Ecc521Private => {
                let phase = match phase {
                    OpenKeyPhase::Init => self.init_open_ecc_pub_key(tag, key_id, ecc_op, false)?,
                    _ => self.open_ecc_pub_key_zc(tag, key_id, phase, ecc_op, pub_key)?,
                };
                (phase, None)
            }

            EntryKind::Rsa2kPrivate | EntryKind::Rsa3kPrivate | EntryKind::Rsa4kPrivate => {
                let pub_key = self.open_rsa_non_crt_pub_key(key_id, is_unwrapping_key)?;
                (OpenKeyPhase::Done, Some(pub_key))
            }

            EntryKind::Rsa2kPrivateCrt
            | EntryKind::Rsa3kPrivateCrt
            | EntryKind::Rsa4kPrivateCrt => {
                let pub_key = self.open_rsa_crt_pub_key(key_id)?;
                (OpenKeyPhase::Done, Some(pub_key))
            }

            EntryKind::Aes128 | EntryKind::Aes192 | EntryKind::Aes256 => (OpenKeyPhase::Done, None),

            EntryKind::Secret256 | EntryKind::Secret384 | EntryKind::Secret521 => {
                (OpenKeyPhase::Done, None)
            }

            EntryKind::AesXtsBulk256
            | EntryKind::AesGcmBulk256
            | EntryKind::AesGcmBulk256Unapproved => (OpenKeyPhase::Done, None),

            EntryKind::HmacSha256
            | EntryKind::HmacSha384
            | EntryKind::HmacSha512
            | EntryKind::VarLenHmacSha256
            | EntryKind::VarLenHmacSha384
            | EntryKind::VarLenHmacSha512 => (OpenKeyPhase::Done, None),

            _ => Err(HsmErr::InvalidKeyType)?,
        };

        let bulk_key_id = if key_kind.is_bulk_key() {
            let key_blob = key.blob()?;
            Some(u16::from_le_bytes([key_blob[0], key_blob[1]]))
        } else {
            None
        };

        Ok(OpenKeyData {
            phase: phase_out,
            id: key_id,
            kind: key_kind,
            flags,
            pub_key,
            bulk_key_id,
        })
    }

    /// Get key kind
    fn get_key_kind(&self, key_id: KeyId) -> HsmResult<EntryKind> {
        // Allow get_key_kind for internal keys
        let internal_key =
            self.state
                .vault()
                .key(APP_VAULT_ID_FOR_INTERNAL_KEYS, self.id(), key_id, true);

        if internal_key.is_ok() {
            internal_key?.kind()
        } else {
            self.state
                .vault()
                .key(self.app_vault_id(), self.id(), key_id, true)?
                .kind()
        }
    }

    /// Get key length
    #[cfg(feature = "fips_validation_hooks")]
    fn get_key_length(&self, key_id: KeyId) -> HsmResult<u16> {
        let vault_key = self.state.vault().key_unchecked(key_id);
        let entry_attributes = vault_key.attributes()?;

        let key_kind: DdiKeyType = vault_key.kind()?.try_into()?;

        let key_length = if key_kind.is_var_hmac() {
            entry_attributes.entry_specific[VarLenHmacShaKey::KEY_LENGTH_INDEX] as u16
        } else {
            EntryKind::try_from(key_kind).map(|entry| entry.raw_key_blob_size() as u16)?
        };

        Ok(key_length)
    }

    /// Delete Key
    fn delete_key(&self, key_id: KeyId) -> HsmResult<()> {
        // Return error if key is an internal key
        if self
            .state
            .vault()
            .key(APP_VAULT_ID_FOR_INTERNAL_KEYS, self.id(), key_id, true)
            .is_ok()
        {
            Err(HsmErr::CannotDeleteInternalKeys)?
        }

        self.state
            .vault()
            .delete_key(self.app_vault_id(), self.id(), key_id, |k| {
                self.key_in_use(k)
            })
    }

    /// Generate AES Key
    fn aes_gen_key(
        &self,
        tag: Option<u16>,
        kind: AesKeyKind,
        usage: AesKeyUsage,
        availability: KeyAvailability,
    ) -> HsmResult<AesKey> {
        self.aes_gen_key_inner(kind, usage, tag, availability)
    }

    /// AES encrypt/decrypt operation
    fn aes_enc_dec(&self, tag: TagId, key: AesKeyIn, input: &AesEncDecIn) -> HsmResult<()> {
        self.aes_enc_dec_inner(tag, key, input)
    }

    /// In-place AES unwrap with padding
    fn begin_aes_key_unwrap(&self, tag: TagId, kek: &[u8], inout: &[u8]) -> HsmResult<()> {
        self.begin_soft_aes_inner(tag, kek, inout, SoftAesOp::Kwp)
    }

    /// End In-place AES unwrap with padding
    fn end_aes_key_unwrap(&self, tag: TagId) -> HsmResult<Range<usize>> {
        self.end_soft_aes_inner(tag)
    }

    /// In-place SoftAes operation
    #[cfg(feature = "fips_validation_hooks")]
    fn begin_soft_aes(&self, tag: TagId, key: &[u8], inout: &[u8], op: SoftAesOp) -> HsmResult<()> {
        self.begin_soft_aes_inner(tag, key, inout, op)
    }

    /// End In-place SoftAes operation
    #[cfg(feature = "fips_validation_hooks")]
    fn end_soft_aes(&self, tag: TagId) -> HsmResult<Range<usize>> {
        self.end_soft_aes_inner(tag)
    }

    /// Begin ECC key generation op
    fn begin_ecc_gen_key(
        &self,
        tag: TagId,
        key_tag: Option<u16>,
        curve: EccCurve,
        usage: EccKeyUsage,
        availability: KeyAvailability,
    ) -> HsmResult<EccGenKey<Self::Env>> {
        self._begin_ecc_gen_key(tag, key_tag, curve, usage, availability)
    }

    /// End ECC key generation op
    fn end_ecc_gen_key(&self, tag: TagId, op: EccGenKey<Self::Env>) -> HsmResult<EccGenKeyOut> {
        self._end_ecc_gen_key(tag, op)
    }

    /// Begin ECC sign op
    fn begin_ecc_sign_zc(
        &self,
        tag: TagId,
        key_in: EccKeyIn,
        digest: &IoMemRange,
        digest_algo: DdiHashAlgorithm,
        signature: &IoMemRange,
    ) -> HsmResult<EccSign<Self::Env>> {
        self._begin_ecc_sign_zc(tag, key_in, digest, digest_algo, signature)
    }

    /// End ECC sign op
    fn end_ecc_sign_zc(&self, tag: TagId, op: EccSign<Self::Env>) -> HsmResult<()> {
        self._end_ecc_sign_zc(tag, &op)
    }

    /// Begin generate ECC Public key from private key
    fn begin_ecc_gen_pub_key(
        &self,
        tag: TagId,
        key_id: KeyId,
    ) -> HsmResult<EccGenPubKeyCmd<Self::Env>> {
        self._begin_ecc_gen_pub_key(tag, key_id)
    }

    /// Continue generate ECC Public key from private key
    fn continue_ecc_gen_pub_key_zc(
        &self,
        op: EccGenPubKeyCmd<Self::Env>,
        pub_key: &IoMemRange,
    ) -> HsmResult<EccGenPubKeyCmd<Self::Env>> {
        self._continue_ecc_gen_pub_key_zc(op, pub_key)
    }

    /// End generate ECC Public key from private key
    fn end_ecc_gen_pub_key_zc(&self, op: EccGenPubKeyCmd<Self::Env>) -> HsmResult<()> {
        self._end_ecc_gen_pub_key_zc(op)
    }

    /// Begin ECDH compute operation.
    fn begin_ecdh_compute_with_pub_key_validation(
        &self,
        tag: TagId,
        key_id: KeyId,
        target_key_type: DdiKeyType,
        pub_key: &IoMemRange,
    ) -> HsmResult<EcdhComputeCmd<Self::Env>> {
        self.begin_ecdh_compute_inner(tag, key_id, target_key_type, pub_key)
    }

    /// Continue an ECDH compute operation with PKA-format public key data.
    fn continue_ecdh_compute_zc(
        &self,
        op: EcdhComputeCmd<Self::Env>,
        pub_key: &IoMemRange,
    ) -> HsmResult<EcdhComputeCmd<Self::Env>> {
        self.continue_ecdh_compute_zc_inner(op, pub_key)
    }

    /// End ECDH compute operation.
    fn end_ecdh_compute(
        &self,
        op: EcdhComputeCmd<Self::Env>,
        key_usage: DdiKeyUsage,
        key_tag: Option<u16>,
        key_availability: KeyAvailability,
    ) -> HsmResult<KeyId> {
        self._end_ecdh_compute(op, key_usage, key_tag, key_availability)
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

        let pka_curve = public_key.curve;

        let priv_key_size = PkaEccPrivateKey::data_len(pka_curve);
        let pub_key_data_len = public_key.data.len();

        let mut dma_buf = self.dma_alloc(pub_key_data_len + priv_key_size)?;

        // get the private key from the vault
        let ecc_key =
            self.state
                .vault()
                .ecc_key(self.app_vault_id(), self.id(), key_id, Some(usage))?;

        // copy private key into buffer
        dma_buf.as_ref_mut()[pub_key_data_len..pub_key_data_len + priv_key_size]
            .copy_from_slice(&ecc_key.blob()?[..priv_key_size]);

        let priv_key_blob =
            IoMemRange::from(&dma_buf.as_ref()[pub_key_data_len..pub_key_data_len + priv_key_size]);

        // copy public key into buffer
        #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
        {
            if let Some(neg_pct_skip_cnt) = self.neg_pct_skip_cnt(None) {
                if neg_pct_skip_cnt == 0 {
                    // Modify the data field by replacing with incorrect test public key
                    match public_key.curve {
                        PkaEccCurve::Ecc256 => dma_buf.as_ref_mut()[..pub_key_data_len]
                            .copy_from_slice(&NEGATIVE_PCT_ECC_PUBLIC_KEYS[0].data),
                        PkaEccCurve::Ecc384 => dma_buf.as_ref_mut()[..pub_key_data_len]
                            .copy_from_slice(&NEGATIVE_PCT_ECC_PUBLIC_KEYS[1].data),
                        PkaEccCurve::Ecc521 => dma_buf.as_ref_mut()[..pub_key_data_len]
                            .copy_from_slice(&NEGATIVE_PCT_ECC_PUBLIC_KEYS[2].data),
                    };
                } else {
                    let _ = self.neg_pct_skip_cnt(Some(neg_pct_skip_cnt - 1));
                    dma_buf.as_ref_mut()[..pub_key_data_len].copy_from_slice(&public_key.data);
                }
            } else {
                dma_buf.as_ref_mut()[..pub_key_data_len].copy_from_slice(&public_key.data);
            }
        }
        #[cfg(any(
            not(feature = "mcr_test_hooks"),
            not(feature = "fips_validation_hooks")
        ))]
        dma_buf.as_ref_mut()[..pub_key_data_len].copy_from_slice(&public_key.data);

        // convert public key into IoMemRange
        let pub_key_blob = IoMemRange::from(&dma_buf.as_ref()[..pub_key_data_len]);

        // The DMA buffer size is determined based on the type of ECC operation
        // - For Sign/Verify: The buffer includes space for the digest and signature
        // - For Key Agreement: The buffer includes space for test public key (2 * max len),
        //                      test private key blob (max_len), and the generated
        //                      shared secrets (max_len)
        let ecc_data_buffer_size = match usage {
            EccKeyUsage::SignVerify => PkaEccCurve::MAX_LEN + ECC_SIGNATURE_MAX_LEN,
            EccKeyUsage::KeyAgreement => {
                (PkaEccCurve::MAX_LEN * 2) + PkaEccCurve::MAX_LEN + PkaEccCurve::MAX_LEN
            }
        };

        // Allocate the DMA buffer for operations
        let op_dma_buf = self.dma_alloc(ecc_data_buffer_size)?;

        // Build erased engine from env (PKA + SHA)
        let sha = self.state.env().sha().clone();
        let engine: Box<dyn PctEngine> = Box::new(PctEngineImpl::new(engine_ref, sha));

        // set up EccKeyPct instance
        let mut ecc_key_pct = EccKeyPct::new(
            priv_key_blob,
            pub_key_blob,
            dma_buf,
            pka_curve,
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

    /// Begin ECC structural validation operation.
    fn begin_ecc_structural_validation(
        &self,
        tag: TagId,
        key_id: KeyId,
        entry_usage: DdiKeyUsage,
        pub_key_blob: Vec<u8>,
    ) -> HsmResult<EccStructuralValidationCmd<Self::Env>> {
        self.begin_ecc_structural_validation_inner(tag, key_id, entry_usage, pub_key_blob)
    }

    /// Continue an ECC structural validation operation.
    fn continue_ecc_structural_validation(
        &self,
        op: EccStructuralValidationCmd<Self::Env>,
    ) -> HsmResult<EccStructuralValidationCmd<Self::Env>> {
        self.continue_ecc_structural_validation_inner(op)
    }

    /// End an ECC structural validation operation.
    fn end_ecc_structural_validation(
        &mut self,
        op: EccStructuralValidationCmd<Self::Env>,
    ) -> HsmResult<()> {
        self.end_ecc_structural_validation_inner(op)
    }

    /// Begin RSA modular exponentiation op
    fn begin_rsa_mod_exp_zc(
        &self,
        tag: TagId,
        key_id: KeyId,
        usage: Option<RsaKeyUsage>,
        input: &IoMemRange,
        output: &IoMemRange,
    ) -> HsmResult<RsaModExp<Self::Env>> {
        self._begin_rsa_mod_exp_zc(tag, key_id, usage, input, output, self.app_vault_id())
    }

    /// End RSA modular exponentiation op
    fn end_rsa_mod_exp_zc(&self, tag: TagId, op: RsaModExp<Self::Env>) -> HsmResult<()> {
        self._end_rsa_mod_exp_zc(tag, &op)
    }

    /// Begin RSA PCT validation operation.
    fn begin_rsa_pct_validation(
        &self,
        tag: TagId,
        key_id: KeyId,
        usage: RsaKeyUsage,
        rsa_type: PkaRsaSize,
        n: &mut [u8],
        e: &[u8],
    ) -> HsmResult<RsaPctValidationCmd<E>> {
        #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
        {
            if let Some(neg_pct_skip_cnt) = self.neg_pct_skip_cnt(None) {
                if neg_pct_skip_cnt == 0 {
                    // Modify the public key data to an incorrect test public key
                    match rsa_type {
                        PkaRsaSize::Rsa2k => {
                            n[..rsa_type.len()].copy_from_slice(&NEG_PCT_RSA_2K_MOD)
                        }
                        PkaRsaSize::Rsa3k => {
                            n[..rsa_type.len()].copy_from_slice(&NEG_PCT_RSA_3K_MOD)
                        }
                        PkaRsaSize::Rsa4k => {
                            n[..rsa_type.len()].copy_from_slice(&NEG_PCT_RSA_4K_MOD)
                        }
                        _ => (),
                    };
                } else {
                    let _ = self.neg_pct_skip_cnt(Some(neg_pct_skip_cnt - 1));
                }
            }
        }

        self.begin_rsa_pct_validation_inner(tag, key_id, usage, rsa_type, n, e)
    }

    /// Continue an RSA PCT validation operation.
    fn continue_rsa_pct_validation(
        &self,
        op: RsaPctValidationCmd<E>,
    ) -> HsmResult<RsaPctValidationCmd<E>> {
        self.continue_rsa_pct_validation_inner(op)
    }

    /// End an RSA PCT validation operation.
    fn end_rsa_pct_validation(&self, op: RsaPctValidationCmd<E>) -> HsmResult<bool> {
        self.end_rsa_pct_validation_inner(op)
    }

    /// Checks if the PCT validation state requires final verification
    fn is_rsa_pct_final_state(&self, pct_op: &RsaPctValidationCmd<E>) -> bool {
        matches!(
            pct_op.state,
            RsaPctValidationState::WaitForDecrypt | RsaPctValidationState::WaitForVerify
        )
    }

    /// Begin RSA unwrap, zero copy
    fn begin_rsa_unwrap_mod_exp_zc(
        &self,
        tag: TagId,
        key_id: KeyId,
        input: &IoMemRange,
        output: &IoMemRange,
        usage: Option<RsaKeyUsage>,
    ) -> HsmResult<RsaModExp<Self::Env>> {
        self._begin_rsa_mod_exp_zc(
            tag,
            key_id,
            usage,
            input,
            output,
            APP_VAULT_ID_FOR_INTERNAL_KEYS,
        )
    }

    /// End RSA unwrap, zero copy
    fn end_rsa_unwrap_mod_exp_zc(&self, tag: TagId, op: RsaModExp<Self::Env>) -> HsmResult<()> {
        self._end_rsa_mod_exp_zc(tag, &op)
    }

    fn decode_oaep_kek(
        &self,
        unwrapped_data: &[u8],
        padding: DdiRsaCryptoPadding,
        hash_alg: DdiHashAlgorithm,
    ) -> HsmResult<SecureByteVec> {
        self.decode_oaep_kek_inner(unwrapped_data, padding, hash_alg)
    }

    /// Get random number using RNG
    #[cfg(feature = "fips_validation_hooks")]
    fn get_random_number(&self, rng_number: &mut IoMemRange) -> HsmResult<()> {
        self.state.env().rng().bytes(rng_number.slice_mut());

        Ok(())
    }

    fn sha_single_block_zc(
        &self,
        mode: ShaType,
        buffer: &IoMemRange,
        output_buffer: &mut IoMemRange,
    ) -> HsmResult<()> {
        self.sha_single_block_inner_zc(mode, buffer, output_buffer)
    }

    /// Import a RAW key into the HSM (only Private key).
    ///
    /// Side-effect: This function will delete the existing unwrapping key while importing a
    /// new one.
    #[cfg(feature = "fips_validation_hooks")]
    fn import_raw_key(
        &self,
        key_type: DdiKeyType,
        key_properties: DdiKeyProperties,
        key_tag: Option<u16>,
        raw_key: &[u8],
    ) -> HsmResult<KeyId> {
        let key_usage = key_properties.key_usage;
        let key_availability = key_properties.key_availability.try_into()?;

        match key_type {
            DdiKeyType::Rsa2kPrivate => {
                if key_usage != DdiKeyUsage::Unwrap {
                    Err(HsmErr::InvalidPermissions)?
                }

                // If we have an existing unwrapping key, delete the key before importing a new key
                if let Some(unwrapping_key_id) = self.state.unwrapping_key_id() {
                    self.state.vault().delete_key(
                        APP_VAULT_ID_FOR_INTERNAL_KEYS,
                        self.id(),
                        unwrapping_key_id,
                        |k| self.key_in_use(k),
                    )?;
                }

                // Import the incoming unwrapping key
                let key_id = self.import_unwrapping_key(raw_key)?;

                self.state.set_unwrapping_key_id(Some(key_id));

                Ok(key_id)
            }
            DdiKeyType::Secret256 | DdiKeyType::Secret384 | DdiKeyType::Secret521 => {
                // Convert DdiKeyType to EcdhKeyKind
                let ecdh_key_kind = key_type.try_into()?;

                // Convert DdiKeyUsage to EcdhKeyUsage
                let ecdh_key_usage: EcdhKeyUsage = key_usage.try_into()?;

                // Create new ECDH key
                let ecdh_key = EcdhKeyImported::new(ecdh_key_kind, ecdh_key_usage, raw_key)?;

                // Import the key into the vault
                let key = self.state.vault().ecdh_import_key(
                    self.app_vault_id(),
                    self.id(),
                    key_tag,
                    false,
                    &ecdh_key,
                    key_availability,
                )?;

                // Return the key ID
                Ok(key.id())
            }
            DdiKeyType::HmacSha256 | DdiKeyType::HmacSha384 | DdiKeyType::HmacSha512 => {
                // Create the key to be imported into the key vault
                let key = HmacKeyImported::new(
                    key_type.try_into()?,
                    key_properties.key_usage.try_into()?,
                    raw_key,
                )?;

                // Import the HMAC key into thekey  vault
                let hmac_key = self.state.vault().hmac_import_key(
                    self.app_vault_id(),
                    self.id(),
                    key_tag,
                    false,
                    &key,
                    key_availability,
                )?;

                Ok(hmac_key.id())
            }
            DdiKeyType::VarLenHmacSha256
            | DdiKeyType::VarLenHmacSha384
            | DdiKeyType::VarLenHmacSha512 => {
                // Create the key to be imported into the key vault
                let key = VarLenHmacShaKeyImported::new(
                    key_type.try_into()?,
                    key_properties.key_usage.try_into()?,
                    raw_key,
                )?;

                // Import the variable length HMAC key into thekey  vault
                let var_len_hmac_key = self.state.vault().import_var_hmac_key(
                    self.app_vault_id(),
                    self.id(),
                    key_tag,
                    false,
                    &key,
                    key_availability,
                )?;

                Ok(var_len_hmac_key.id())
            }
            _ => return Err(HsmErr::InvalidKeyType),
        }
    }

    /// Import a DER key into the HSM (except RSA CRT type keys).
    fn import_der_key(
        &self,
        entry_class: EntryClass,
        entry_usage: DdiKeyUsage,
        entry_tag: Option<u16>,
        entry_availability: KeyAvailability,
        der: &[u8],
    ) -> HsmResult<ImportDerKeyResult> {
        let (priv_key_id, pub_key_data, key_type) = match entry_class {
            EntryClass::Rsa => {
                // RSA DER decoding
                let rsa_key_data = der.rsa_der_to_raw()?;
                let key_blob = rsa_key_data.priv_key().to_pka_bytes()?;

                let key_kind = match rsa_key_data.rsa_type {
                    RsaSize::Rsa2k => RsaKeyKind::Rsa2kPrivate,
                    RsaSize::Rsa3k => RsaKeyKind::Rsa3kPrivate,
                    RsaSize::Rsa4k => RsaKeyKind::Rsa4kPrivate,
                };

                let entry_kind: EntryKind = key_kind.into();
                let ddi_key_type: DdiKeyType = entry_kind.try_into()?;

                let key_usage = RsaKeyUsage::try_from(entry_usage)?;

                // Get the pub key pka bytes
                let pub_key = RsaPubKey::from_priv_pka_slice(&key_blob, rsa_key_data.rsa_type)?;
                let pub_key_blob = pub_key.to_pka_bytes()?;

                let rsa_key = RsaKeyImported::new(key_kind, key_usage, &key_blob)?;

                // Import the key into the vault
                let imported_key = self.state.vault().rsa_import_key(
                    self.app_vault_id(),
                    self.id(),
                    entry_tag,
                    false,
                    &rsa_key,
                    entry_availability,
                )?;

                (imported_key.id(), Some(pub_key_blob), ddi_key_type)
            }

            EntryClass::Ecc => {
                // ECC DER decoding
                let ecc_key_data = der.ecc_priv_key_pkcs8_der_to_raw()?;
                let priv_key = ecc_key_data.priv_key().ok_or(HsmErr::DerDecodeFailed)?;
                let key_blob = priv_key.to_pka_bytes()?;

                let key_kind = match priv_key.curve() {
                    EccCurve::P256 => EccKeyKind::Ecc256Private,
                    EccCurve::P384 => EccKeyKind::Ecc384Private,
                    EccCurve::P521 => EccKeyKind::Ecc521Private,
                };
                let key_usage = EccKeyUsage::try_from(entry_usage)?;

                let entry_kind: EntryKind = key_kind.into();
                let ddi_key_type: DdiKeyType = entry_kind.try_into()?;

                // Get the pub key pka bytes
                let pub_key = ecc_key_data.pub_key().ok_or(HsmErr::DerEncodeFailed)?;
                let pub_key_blob = pub_key.to_pka_bytes()?;

                let ecc_key = EccKeyImported::new(key_kind, key_usage, &key_blob)?;

                // Import the key into the vault
                let imported_key = self.state.vault().ecc_import_key(
                    self.app_vault_id(),
                    self.id(),
                    entry_tag,
                    false,
                    &ecc_key,
                    entry_availability,
                )?;

                (imported_key.id(), Some(pub_key_blob), ddi_key_type)
            }

            EntryClass::Aes => {
                let key_kind = match der.len() {
                    16 => AesKeyKind::Aes128,
                    24 => AesKeyKind::Aes192,
                    32 => AesKeyKind::Aes256,
                    _ => Err(HsmErr::InvalidArgument)?,
                };
                let key_usage = AesKeyUsage::try_from(entry_usage)?;

                let entry_kind: EntryKind = key_kind.into();
                let ddi_key_type: DdiKeyType = entry_kind.try_into()?;

                // There is no DER decoding needed for AES keys
                let key_blob = der;

                let aes_key = AesKeyImported::new(key_kind, key_usage, key_blob)?;

                // Import the key into the vault
                let attributes = aes_entry_attributes(entry_availability, false, key_usage);
                let imported_key = self.state.vault().aes_import_key(
                    self.app_vault_id(),
                    self.id(),
                    entry_tag,
                    &aes_key,
                    &attributes,
                )?;

                // There is no public key for AES keys
                (imported_key.id(), None, ddi_key_type)
            }

            // Note: RSA CRT DER key import follow a different approach (and
            // hence a separate API) as there are multiple async steps
            // involved in generating the CRT parameters and then storing
            // the key in the key vault.
            // Note: For AES bulk key import, we don't import the actual raw key
            // directly in to HSM vault. Instead, we store the key id (also
            // referred to as a forwarding key) from the CDMA key vault as
            // an AES-GCM-256 key type into the HSM key vault.
            _ => Err(HsmErr::InvalidKeyType)?,
        };

        Ok(ImportDerKeyResult {
            priv_key_id,
            pub_key_data,
            key_type,
        })
    }

    // Import a CRT DER key.
    fn begin_import_der_crt_key(
        &self,
        tag: TagId,
        der: &[u8],
    ) -> HsmResult<(RsaCrtParamComputeCmd<E>, Vec<u8>)> {
        // RSA DER decoding
        let rsa_crt_key_data = der.rsa_der_to_raw()?;
        let priv_key_crt_partial = rsa_crt_key_data.priv_key_crt();

        // Get the pub key pka bytes
        let pub_key = rsa_crt_key_data.pub_key();
        let pub_key_blob = pub_key.to_pka_bytes()?;

        // Kick off the CRT param generation.
        Ok((
            self.begin_compute_rsa_crt_params(tag, priv_key_crt_partial)?,
            pub_key_blob,
        ))
    }

    // Continue importing a CRT DER key.
    fn continue_import_der_crt_key(
        &self,
        op: RsaCrtParamComputeCmd<E>,
    ) -> HsmResult<RsaCrtParamComputeCmd<E>> {
        self.continue_compute_rsa_crt_params(op.tag, op)
    }

    // End importing a CRT DER key.
    fn end_import_der_crt_key(
        &self,
        op: RsaCrtParamComputeCmd<E>,
        key_usage: DdiKeyUsage,
        key_tag: Option<u16>,
        key_availabilty: KeyAvailability,
    ) -> HsmResult<(KeyId, DdiKeyType)> {
        let rsa_crt_result = self.end_compute_rsa_crt_params(op)?;
        let rsa_crt_priv_key_blob = rsa_crt_result.to_pka_bytes()?;

        let key_kind = match rsa_crt_result.rsa_type {
            RsaSize::Rsa2k => RsaKeyKind::Rsa2kPrivateCrt,
            RsaSize::Rsa3k => RsaKeyKind::Rsa3kPrivateCrt,
            RsaSize::Rsa4k => RsaKeyKind::Rsa4kPrivateCrt,
        };

        let key_usage = RsaKeyUsage::try_from(key_usage)?;

        let entry_kind: EntryKind = key_kind.into();
        let ddi_key_type: DdiKeyType = entry_kind.try_into()?;

        let rsa_key = RsaKeyImported::new(key_kind, key_usage, &rsa_crt_priv_key_blob)?;

        // Import the key into the vault
        let imported_key = self.state.vault().rsa_import_key(
            self.app_vault_id(),
            self.id(),
            key_tag,
            false,
            &rsa_key,
            key_availabilty,
        )?;

        Ok((imported_key.id(), ddi_key_type))
    }

    /// Begin getting the unwrapping key
    fn begin_get_unwrapping_key(
        &self,
        tag: TagId,
        key_id: Option<KeyId>,
        pfn: PcieFunction,
    ) -> HsmResult<GetUnwrappingKeyCtx<Self::Env>> {
        self.begin_get_unwrapping_key_inner(tag, key_id, pfn)
    }

    /// End the process to get unwrapping key
    fn end_get_unwrapping_key(
        &self,
        ctx: &GetUnwrappingKeyCtx<Self::Env>,
    ) -> HsmResult<GetUnwrappingKeyOut> {
        self.end_get_unwrapping_key_inner(ctx)
    }

    /// Begin computing CRT parameters (n1q, n2p) for the RSA CRT private key.
    fn begin_compute_rsa_crt_params(
        &self,
        tag: TagId,
        priv_key_crt: RsaPrivKeyCrt,
    ) -> HsmResult<RsaCrtParamComputeCmd<E>> {
        self.begin_compute_rsa_crt_params_inner(tag, priv_key_crt)
    }

    /// Continue computing CRT parameters (n1q, n2p) for the RSA CRT private key.
    fn continue_compute_rsa_crt_params(
        &self,
        tag: TagId,
        op: RsaCrtParamComputeCmd<E>,
    ) -> HsmResult<RsaCrtParamComputeCmd<E>> {
        self.continue_compute_rsa_crt_params_inner(tag, op)
    }

    /// End computing CRT parameters (n1q, n2p) for the RSA CRT private key.
    fn end_compute_rsa_crt_params(&self, op: RsaCrtParamComputeCmd<E>) -> HsmResult<RsaPrivKeyCrt> {
        self.end_compute_rsa_crt_params_inner(op)
    }

    /// Compute the HMAC for a given message and secret key.
    fn hmac(&self, key_id: KeyId, msg: &[u8], output: &mut IoMemRange) -> HsmResult<()> {
        let hmac_key = self.hmac_key(key_id)?;
        let hmac_key_blob = hmac_key.blob()?;

        self.hmac_impl(&hmac_key_blob, msg, hmac_key.kind()?.into(), output)
    }

    /// Compute variable length HMAC
    fn var_hmac(&self, key_id: KeyId, msg: &[u8], output_buffer: &mut IoMemRange) -> HsmResult<()> {
        let hmac_key = self.var_hmac_key(key_id)?;
        let hmac_key_blob = hmac_key.blob()?;

        self.hmac_impl(&hmac_key_blob, msg, hmac_key.kind()?.into(), output_buffer)
    }

    /// Compute hkdf
    fn hkdf_derive(
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
        self.hkdf_inner(
            key_id,
            salt,
            info,
            hash_algo,
            key_type,
            key_properties,
            key_tag,
            key_len,
        )
    }

    /// Compute kbkdf counter mode with HMAC
    fn kbkdf_derive(
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
        self.kbkdf_inner(
            key_id,
            label,
            context,
            hash_algo,
            key_type,
            key_properties,
            key_tag,
            key_len,
        )
    }

    /// Begin HKDF AES Bulk 256 key derivation
    fn begin_hkdf_aesbulk256_derive(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        salt: &[u8],
        info: &[u8],
        kdf_info: KdfInfo,
    ) -> HsmResult<AesBulk256Cmd<Self::Env>> {
        self.begin_hkdf_aesbulk256_derive_inner(tag, pfn, salt, info, kdf_info)
    }

    /// Begin KBKDF AES Bulk 256 key derivation
    fn begin_kbkdf_aesbulk256_derive(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        label: &[u8],
        context: &[u8],
        kdf_info: KdfInfo,
    ) -> HsmResult<AesBulk256Cmd<Self::Env>> {
        self.begin_kbkdf_aesbulk256_derive_inner(tag, pfn, label, context, kdf_info)
    }

    /// End KDF AES Bulk 256 key derivation
    fn end_kdf_aesbulk256_derive(&self, op: &AesBulk256Cmd<Self::Env>) -> HsmResult<()> {
        self.end_kdf_aesbulk256_derive_inner(op)
    }

    /// Aes bulk 256 key import. First store the raw key in CDMA vault managed
    fn begin_import_der_aesbulk256_key(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        entry_usage: DdiKeyUsage,
        entry_tag: Option<u16>,
        key_type: DdiKeyType,
        entry_availability: KeyAvailability,
        der: &[u8],
    ) -> HsmResult<AesBulk256Cmd<E>> {
        let key_usage = AesKeyUsage::try_from(entry_usage)?;
        let attributes = aes_entry_attributes(entry_availability, false, key_usage);
        self.begin_import_der_aesbulk256_key_inner(
            tag,
            pfn,
            entry_tag,
            key_type,
            key_usage,
            &attributes,
            der,
        )
    }

    /// Import an AES bulk 256 key during unmask, preserving original attributes.
    fn unmask_import_der_aesbulk256_key(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        entry_usage: DdiKeyUsage,
        entry_tag: Option<u16>,
        key_type: DdiKeyType,
        original_attributes: &EntryAttributes,
        der: &[u8],
    ) -> HsmResult<AesBulk256Cmd<E>> {
        let key_usage = AesKeyUsage::try_from(entry_usage)?;
        self.begin_import_der_aesbulk256_key_inner(
            tag,
            pfn,
            entry_tag,
            key_type,
            key_usage,
            original_attributes,
            der,
        )
    }

    /// End importing a DER key by processing IPC.
    fn end_import_der_aesbulk256_key(&self, op: &AesBulk256Cmd<E>) -> HsmResult<()> {
        self.end_import_der_aesbulk256_key_inner(op)
    }

    /// Begin rollback of AES bulk 256 key
    fn begin_rollback_aesbulk256_key(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        op: &AesBulk256Cmd<Self::Env>,
    ) -> HsmResult<()> {
        self.begin_rollback_aesbulk256_key_inner(tag, pfn, op)
    }

    /// End rollback of AES bulk 256 key
    fn end_rollback_aesbulk256_key(&self, op: &AesBulk256Cmd<Self::Env>) -> HsmResult<()> {
        self.end_rollback_aesbulk256_key_inner(op)
    }

    /// Aes bulk 256 key delete. Delete the key from CDMA vault and also from HSM vault.
    fn begin_delete_aesbulk256_key(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        key_id: KeyId,
    ) -> HsmResult<AesBulk256Cmd<E>> {
        self.begin_delete_aesbulk256_key_inner(tag, pfn, key_id)
    }

    /// End delete key by processing IPC.
    fn end_delete_aesbulk256_key(&self, op: &AesBulk256Cmd<E>) -> HsmResult<()> {
        self.end_delete_aesbulk256_key_inner(op)
    }

    /// Generate AES Bulk 256
    fn begin_aesbulk256_gen_key(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        key_tag: Option<u16>,
        key_type: DdiKeyType,
        availability: KeyAvailability,
    ) -> HsmResult<AesBulk256Cmd<E>> {
        self.begin_aesbulk256_gen_key_inner(tag, pfn, key_tag, key_type, availability)
    }

    /// Receive IPC from FP and finish the gen key operation for AES Bulk 256
    fn end_aesbulk256_gen_key(&self, op: &AesBulk256Cmd<E>) -> HsmResult<()> {
        self.end_aesbulk256_gen_key_inner(op)
    }

    /// Begin change pin
    fn begin_change_pin(&self, tag: TagId) -> HsmResult<ChangePinCmdCtx<Self::Env>> {
        self.begin_change_pin_inner(tag)
    }

    /// Continue change pin
    fn continue_change_pin(
        &self,
        ctx: ChangePinCmdCtx<Self::Env>,
        pub_key: &IoMemRange,
    ) -> HsmResult<ChangePinCmdCtx<Self::Env>> {
        self.continue_change_pin_inner(ctx, pub_key)
    }

    /// End change pin
    fn end_change_pin(
        &self,
        ctx: ChangePinCmdCtx<Self::Env>,
        encrypted_pin: &DdiEncryptedPin,
    ) -> HsmResult<()> {
        self.end_change_pin_inner(ctx, encrypted_pin)
    }

    fn notify_pct_validation_failure(&self, err: u32) {
        self.state.env().notify_pct_validation_failure(err);
    }

    #[cfg(feature = "mcr_test_hooks")]
    fn cmd_fsm_test_action(&self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction> {
        self.state.cmd_fsm_test_action(test_action)
    }

    #[cfg(feature = "mcr_test_hooks")]
    fn hsm_fsm_test_action(&self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction> {
        self.state.hsm_fsm_test_action(test_action)
    }

    /// Set the number of FSMs to skip before triggering Negative PCT Failure
    #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
    fn neg_pct_skip_cnt(&self, cnt: Option<u8>) -> Option<u8> {
        self.state.neg_pct_skip_cnt(cnt)
    }

    /// Force the PKA instance during engine allocation
    ///
    /// # Arguments
    ///
    /// * `Option<usize>` - Some(PKA instance) to be forced, None to use the default allocation.
    #[cfg(feature = "fips_validation_hooks")]
    fn force_pka_instance(&self, pka_instance: Option<usize>) {
        self.state
            .env()
            .pka_engine()
            .operate_on_fixed_resource(pka_instance);
    }

    /// Get the length of the masked key based on key label, key id and optionally public key data
    fn get_masked_key_len_from_vault(
        &self,
        key_label: &[u8],
        key_id: KeyId,
        pub_data: Option<&[u8]>,
    ) -> HsmResult<usize> {
        let metadata = self.get_metadata_from_vault(key_label, key_id)?;

        let vault_key = self.state.vault().key_unchecked(key_id);
        let entry_class = vault_key.class()?;
        let key_blob = vault_key.blob()?;

        if entry_class.is_bulk_key() {
            // Use the key blob as the key id
            let cdma_key_id = AesBulk256KeyId::from(u16::from_le_bytes(
                key_blob[..2]
                    .try_into()
                    .map_err(|_| HsmErr::InvalidKeyIndex)?,
            ));

            // Get the key from CDMA vault
            let cdma_key = self.state.cdma_vault().get_key_entry(cdma_key_id)?;
            self.get_masked_key_len(metadata.len(), self.aescbc256_enc_data_len(cdma_key.len()))
        } else if entry_class == EntryClass::Ecc {
            // Note ECC is a special case that we mask both the private key and public key
            self.get_masked_key_len(
                metadata.len(),
                self.aescbc256_enc_data_len(
                    key_blob.len() + pub_data.map(|data| data.len()).unwrap_or_default(),
                ),
            )
        } else {
            self.get_masked_key_len(metadata.len(), self.aescbc256_enc_data_len(key_blob.len()))
        }
    }

    /// Get the length of the masked key based on the length of metadata and length of the encrypted key
    fn get_masked_key_len(
        &self,
        metadata_len: usize,
        encrypted_key_len: usize,
    ) -> HsmResult<usize> {
        Ok(MaskedKey::encoded_length(
            self.get_masking_key_algo(),
            metadata_len,
            encrypted_key_len,
        ))
    }

    /// Mask a key based on the key label, key id and optionally public key data
    fn mask_key_from_vault(
        &self,
        key_label: &[u8],
        key_id: KeyId,
        pub_data: Option<&[u8]>,
        masked_key: &mut [u8],
    ) -> HsmResult<()> {
        let metadata = self.get_metadata_from_vault(key_label, key_id)?;
        let vault_key = self.state.vault().key_unchecked(key_id);
        let entry_class = vault_key.class()?;
        let key_blob = vault_key.blob()?;

        let key_len;
        let mut padded_buffer;

        if entry_class.is_bulk_key() {
            // Use the key blob as the key id
            let cdma_key_id = AesBulk256KeyId::from(u16::from_le_bytes(
                key_blob[..2]
                    .try_into()
                    .map_err(|_| HsmErr::InvalidKeyIndex)?,
            ));

            // Get the key from CDMA vault
            let cdma_key = self.state.cdma_vault().get_key_entry(cdma_key_id)?;
            key_len = cdma_key.len();

            let enc_data_len = self.aescbc256_enc_data_len(key_len);
            padded_buffer = self.dma_alloc(enc_data_len)?;
            padded_buffer.as_ref_mut()[..key_len].copy_from_slice(cdma_key.slice());
        } else if entry_class == EntryClass::Ecc {
            let pub_data_len = pub_data.map(|data| data.len()).unwrap_or_default();
            key_len = key_blob.len() + pub_data_len;

            let enc_data_len = self.aescbc256_enc_data_len(key_len);
            padded_buffer = self.dma_alloc(enc_data_len)?;

            padded_buffer.as_ref_mut()[..key_blob.len()].copy_from_slice(key_blob.as_ref());
            if let Some(pub_data) = pub_data {
                padded_buffer.as_ref_mut()[key_blob.len()..key_len].copy_from_slice(pub_data);
            }
        } else {
            key_len = key_blob.len();

            let enc_data_len = self.aescbc256_enc_data_len(key_len);
            padded_buffer = self.dma_alloc(enc_data_len)?;

            padded_buffer.as_ref_mut()[..key_len].copy_from_slice(key_blob.as_ref());
        }

        let masking_key = if vault_key.attributes()?.common.flags.session() {
            self.get_session_masking_key()
        } else {
            self.get_partition_masking_key()
        }?;

        self.mask_key(
            metadata.as_ref(),
            masking_key.slice(),
            padded_buffer.as_ref(),
            masked_key,
        )
    }

    /// Mask a key based on the metadata, key length and input padded buffer
    fn mask_key(
        &self,
        metadata: &[u8],
        masking_key: &[u8],
        padded_key_buffer: &[u8],
        masked_key: &mut [u8],
    ) -> HsmResult<()> {
        // Get the total length of masked key
        let encoded_length = self.get_masked_key_len(metadata.len(), padded_key_buffer.len())?;
        if encoded_length % 4 != 0 || masked_key.len() != encoded_length {
            Err(HsmErr::MaskedKeyInvalidLength)?
        }

        // Pre-encode
        let mut pre_encoded = MaskedKey::pre_encode(
            1, // TODO: masked key format versioning
            self.get_masking_key_algo(),
            metadata.len(),
            padded_key_buffer.len(),
            masked_key,
        )
        .map_err(|_| HsmErr::MaskedKeyPreEncodeFailed)?;

        // Encode
        MaskedKey::encode(
            self,
            &mut pre_encoded,
            padded_key_buffer,
            masking_key,
            metadata,
        )
        .map_err(|_| HsmErr::MaskedKeyEncodeFailed)
    }

    fn unmask_key(&self, masked_key: &[u8]) -> HsmResult<UnmaskedKeyRawResult<E>> {
        // Decode masked key
        let decoded_masked_key =
            MaskedKey::decode(self, &[], masked_key, /*integrity_check = */ false)
                .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;
        let decoded_aes_key = decoded_masked_key
            .as_aes()
            .ok_or(HsmErr::MaskedKeyDecodeFailed)?;

        let metadata_slice = decoded_aes_key.metadata();
        let mut decoder = MborDecoder::new(metadata_slice);
        let metadata = DdiMaskedKeyMetadata::mbor_decode(&mut decoder)
            .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;

        let entry_attributes =
            EntryAttributes::ref_from_bytes(metadata.key_attributes.blob.as_ref())
                .map_err(|_| HsmErr::InvalidArgument)?;

        let masking_key = if entry_attributes.common.flags.session() {
            self.get_session_masking_key()
        } else {
            self.get_partition_masking_key()
        }?;

        // Integrity check
        let _ = MaskedKey::decode(
            self,
            masking_key.slice(),
            masked_key,
            /*integrity_check = */ true,
        )
        .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;

        // Decrypt key
        let mut decrypted_key = self
            .state
            .env()
            .dma_heap()
            .allocate(decoded_aes_key.encrypted_key().len())
            .ok_or(HsmErr::DmaAllocFailure)?;
        decoded_masked_key
            .decrypt_key(self, masking_key.slice(), decrypted_key.as_ref_mut())
            .map_err(|_| HsmErr::MaskedKeyDecodeFailed)?;

        Ok(UnmaskedKeyRawResult {
            metadata,
            decrypted_key,
        })
    }

    fn unmask_key_and_import(&self, masked_key: &[u8]) -> HsmResult<UnmaskedKeyResult<E>> {
        let raw_result = self.unmask_key(masked_key)?;

        match raw_result.metadata.key_type {
            DdiKeyType::AesXtsBulk256
            | DdiKeyType::AesGcmBulk256
            | DdiKeyType::AesGcmBulk256Unapproved => Ok(UnmaskedKeyResult {
                import_result: None,
                raw_result: Some(raw_result),
            }),
            _ => Ok(UnmaskedKeyResult {
                import_result: Some(UnmaskedKeyImportResult {
                    import_result: self.import_der_key_from_raw_result(&raw_result)?,
                    key_label: raw_result.metadata.key_label,
                }),
                raw_result: None,
            }),
        }
    }
}

impl<E: HsmEnvTrait> UserSession<E> {
    /// Create a new user session
    pub(crate) fn new(api_rev: DdiApiRev, id: SessionId, state: PartState<E>) -> Self {
        Self { api_rev, id, state }
    }

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

    /// Open ECC public key given a Key ID
    fn open_ecc_pub_key_zc(
        &mut self,
        tag: TagId,
        key_id: u16,
        phase: OpenKeyPhase,
        ecc_op: &mut Option<EccGenPubKeyCmd<E>>,
        pub_key: &IoMemRange,
    ) -> HsmResult<OpenKeyPhase> {
        match phase {
            OpenKeyPhase::Init => self.init_open_ecc_pub_key(tag, key_id, ecc_op, false),
            OpenKeyPhase::PendingUpkaEngine => {
                self.init_open_ecc_pub_key(tag, key_id, ecc_op, true)
            }
            OpenKeyPhase::PendingMontgomeryConstCalc => {
                self.continue_open_ecc_pub_key_zc(ecc_op, pub_key)
            }
            OpenKeyPhase::PendingPointMultiplication => {
                self.end_open_ecc_pub_key_zc(ecc_op)?;
                Ok(OpenKeyPhase::Done)
            }
            OpenKeyPhase::Done => Err(HsmErr::InvalidState),
        }
    }

    /// Init phase of open ECC public key
    fn init_open_ecc_pub_key(
        &mut self,
        tag: TagId,
        key_id: u16,
        ecc_op: &mut Option<EccGenPubKeyCmd<E>>,
        eng_pending: bool,
    ) -> HsmResult<OpenKeyPhase> {
        let mut phase_out = OpenKeyPhase::default();
        match self.begin_ecc_gen_pub_key(tag, key_id) {
            Ok(op) => {
                if op.state == EccPtMultiplicationState::WaitForMontgomeryConstCalc {
                    phase_out = op.state.clone().into();
                } else {
                    Err(HsmErr::InvalidState)?
                }

                ecc_op.replace(op);
            }
            Err(err) => {
                phase_out = OpenKeyPhase::Done;

                if err.pending() & !eng_pending {
                    phase_out = OpenKeyPhase::PendingUpkaEngine;
                } else {
                    Err(err)?
                }
            }
        }

        Ok(phase_out)
    }

    /// Continue the open ECC public key operation
    fn continue_open_ecc_pub_key_zc(
        &mut self,
        ecc_op: &mut Option<EccGenPubKeyCmd<E>>,
        pub_key: &IoMemRange,
    ) -> HsmResult<OpenKeyPhase> {
        let op = ecc_op.take().ok_or(HsmErr::InvalidState)?;
        let op_out = self.continue_ecc_gen_pub_key_zc(op, pub_key)?;
        let op_out_state = op_out.state.clone();
        ecc_op.replace(op_out);
        Ok(op_out_state.into())
    }

    /// End the open ECC public key operation
    fn end_open_ecc_pub_key_zc(
        &mut self,
        ecc_op: &mut Option<EccGenPubKeyCmd<E>>,
    ) -> HsmResult<()> {
        let op = ecc_op.take().ok_or(HsmErr::InvalidState)?;
        self.end_ecc_gen_pub_key_zc(op)
    }

    /// Open non-crt RSA public key
    fn open_rsa_non_crt_pub_key(
        &mut self,
        key_id: u16,
        is_unwrapping_key: bool,
    ) -> HsmResult<PublicKey> {
        // Get the RSA key from the key vault.
        let key = if !is_unwrapping_key {
            self.rsa_key(key_id, self.app_vault_id(), None)?
        } else {
            self.rsa_key(key_id, APP_VAULT_ID_FOR_INTERNAL_KEYS, None)?
        };

        // Get the key kind.
        let key_kind = key.kind()?;

        // Filter out the RSA type keys only here.
        let rsa_type: RsaSize = match key_kind {
            EntryKind::Rsa2kPrivate => RsaSize::Rsa2k,
            EntryKind::Rsa3kPrivate => RsaSize::Rsa3k,
            EntryKind::Rsa4kPrivate => RsaSize::Rsa4k,
            _ => Err(HsmErr::InvalidArgument)?,
        };

        // Get the RSA non-CRT key blob.
        let blob = key.blob()?;

        // Build a RsaPubKey structure from the constructed private key structure.
        let pub_key = RsaPubKey::from_priv_pka_slice(&blob, rsa_type)?;

        Ok(PublicKey::RsaPubKey(pub_key))
    }

    /// Open crt RSA public key
    fn open_rsa_crt_pub_key(&mut self, key_id: u16) -> HsmResult<PublicKey> {
        // Get the RSA key from the key vault.
        let key = self.rsa_key(key_id, self.app_vault_id(), None)?;

        // Get the key kind.
        let key_kind = key.kind()?;

        // Filter out the RSA CRT type keys only here.
        let rsa_type: RsaSize = match key_kind {
            EntryKind::Rsa2kPrivateCrt => RsaSize::Rsa2k,
            EntryKind::Rsa3kPrivateCrt => RsaSize::Rsa3k,
            EntryKind::Rsa4kPrivateCrt => RsaSize::Rsa4k,
            _ => Err(HsmErr::InvalidArgument)?,
        };

        // Get the RSA CRT key blob.
        let blob = key.crt_param2()?;

        // Extract the public key from the RSA CRT data.
        let pub_key = pub_key_from_rsa_crt_param2_pka(&blob, rsa_type)?;

        Ok(PublicKey::RsaPubKey(pub_key))
    }

    /// Verify the incoming tag with the current tag used by the crypto FSM.
    fn pka_engine_verify_tag(
        &self,
        engine_ref: &PkaEngineRef<E>,
        op_tag: u16,
        tag: u16,
    ) -> Result<(), HsmErr> {
        let engine_tag = engine_ref
            .deref()
            .peek_tag()
            .ok_or(HsmErr::PkaEngineNotBusy)?;

        if engine_tag != tag {
            Err(HsmErr::PkaTagMismatch)?
        }

        if op_tag != tag {
            Err(HsmErr::PkaTagMismatch)?
        }

        Ok(())
    }

    /// Attempt to acquire the PKA Engine.
    fn pka_engine_acquire(&self, tag: TagId, key_id: Option<u16>) -> HsmResult<PkaEngineRef<E>> {
        self.state
            .env()
            .pka_engine()
            .acquire(tag, key_id)
            .ok_or(HsmErr::Pending)
    }

    /// DMA allocation for a give length
    fn dma_alloc(&self, len: usize) -> HsmResult<DmaBuffer<E>> {
        self.state
            .env()
            .dma_heap()
            .allocate(len)
            .ok_or(HsmErr::DmaAllocFailure)
    }

    /// Import unwrapping key
    fn import_unwrapping_key(&self, key_blob: &[u8]) -> HsmResult<KeyId> {
        let key_kind = RsaKeyKind::Rsa2kPrivate;
        let key_usage = RsaKeyUsage::Unwrap;

        let unwrapping_rsa_key = RsaKeyImported::new(key_kind, key_usage, key_blob)?;

        // Import the unwrapping key into the vault
        let unwrapping_key = self.state.vault().rsa_import_key(
            APP_VAULT_ID_FOR_INTERNAL_KEYS,
            self.id(), // This value does not matter since the key availability is set to App
            None,
            true,
            &unwrapping_rsa_key,
            KeyAvailability::App,
        )?;

        Ok(unwrapping_key.id())
    }

    /// Begin change pin
    fn begin_change_pin_inner(&self, tag: TagId) -> HsmResult<ChangePinCmdCtx<E>> {
        let engine_ref = self
            .state
            .env()
            .pka_engine()
            .acquire(tag, None)
            .ok_or(HsmErr::Pending)?;

        // Submit the PKA command to the engine
        engine_ref
            .deref()
            .begin_montgomery_constant_calculation(tag, PkaEccCurve::Ecc384)
            .map_err(|_| HsmErr::EccGenKeyFailed)?;

        Ok(ChangePinCmdCtx {
            tag,
            engine_ref,
            cmd_info: PkaEccCmd {
                curve: PkaEccCurve::Ecc384,
            },
        })
    }

    /// Continue change pin
    fn continue_change_pin_inner(
        &self,
        ctx: ChangePinCmdCtx<E>,
        pub_key: &IoMemRange,
    ) -> HsmResult<ChangePinCmdCtx<E>> {
        // Verify the tag.
        let engine_tag = ctx
            .engine_ref
            .deref()
            .peek_tag()
            .ok_or(HsmErr::PkaEngineNotBusy)?;
        if engine_tag != ctx.tag {
            Err(HsmErr::PkaTagMismatch)?
        }

        // End montgomery constant calculation command in PKA HW.
        ctx.engine_ref
            .deref()
            .end_montgomery_constant_calculation(ctx.tag)
            .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

        // Get the Session Encryption private key blob.
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

        Ok(ChangePinCmdCtx {
            tag: ctx.tag,
            engine_ref: ctx.engine_ref,
            cmd_info,
        })
    }

    /// End change pin
    fn end_change_pin_inner(
        &self,
        ctx: ChangePinCmdCtx<E>,
        encrypted_pin: &DdiEncryptedPin,
    ) -> HsmResult<()> {
        // Verify the tag.
        let engine_tag = ctx
            .engine_ref
            .deref()
            .peek_tag()
            .ok_or(HsmErr::PkaEngineNotBusy)?;
        if engine_tag != ctx.tag {
            Err(HsmErr::PkaTagMismatch)?
        }

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

        // Zeroize the nonce for security
        nonce.zeroize();

        // Verify nonce one more time. We need to check this again as another FSM could
        // have reached here first.
        self.verify_nonce(encrypted_pin.nonce)?;

        // Verify tag
        let hmac_key = &aes_hmac_key_buffer.as_ref()[32..80];
        self.verify_encrypted_pin_tag(hmac_key, encrypted_pin)?;

        // Reset nonce
        self.state.reset_nonce();

        // Decrypt credential
        let aes_key = &aes_hmac_key_buffer.as_ref()[..32];
        let iv_mem_range: IoMemRange = (&encrypted_pin.iv).into();

        // Decrypt PIN
        let decrypted_pin_buffer = self.dma_alloc(16)?;
        let decrypted_pin_mborbytearray =
            MborByteArray::<16>::new_with_len(decrypted_pin_buffer.as_ref().as_ptr(), 16);
        self.decrypt_credential(
            ctx.tag,
            aes_key,
            &encrypted_pin.encrypted_pin,
            &iv_mem_range,
            &(&decrypted_pin_mborbytearray).into(),
        )?;

        let mut decrypted_pin = SecureByteArray::<16>::new([0u8; 16]);
        decrypted_pin.copy_from_slice(decrypted_pin_buffer.as_ref());

        let current_user_id = self.state.cred_mgr().user_cred().id;

        // Change the user credential
        let result = self
            .state
            .cred_mgr_mut()
            .change_user_cred(&current_user_id, decrypted_pin.as_slice());

        result
    }

    // Verify nonce is the same
    fn verify_nonce(&self, nonce: [u8; 32]) -> HsmResult<()> {
        if nonce != self.state.nonce() {
            Err(HsmErr::NonceMismatch)?;
        }

        Ok(())
    }

    fn verify_encrypted_pin_tag(
        &self,
        hmac_key: &[u8],
        encrypted_pin: &DdiEncryptedPin,
    ) -> HsmResult<()> {
        // Verify hash using aes_vec[32..]
        const HASH_LEN: usize = 64;
        let hash_gsram = self.dma_alloc(HASH_LEN)?;
        let hash_mborbytearray =
            MborByteArray::<HASH_LEN>::new_with_len(hash_gsram.as_ref().as_ptr(), HASH_LEN);

        let mut current_nonce = self.state.nonce();

        let mut pin_iv_nonce_gsram = self.dma_alloc(64)?;
        let pin_iv_nonce_gsram_slice = pin_iv_nonce_gsram.as_ref_mut();
        pin_iv_nonce_gsram_slice[..16].copy_from_slice(encrypted_pin.encrypted_pin.as_slice());
        pin_iv_nonce_gsram_slice[16..32].copy_from_slice(encrypted_pin.iv.as_slice());
        pin_iv_nonce_gsram_slice[32..].copy_from_slice(&current_nonce);

        // Zeroize the nonce for security
        current_nonce.zeroize();

        self.hmac_impl(
            hmac_key,
            pin_iv_nonce_gsram.as_ref(),
            DdiHashAlgorithm::Sha384,
            &mut (&hash_mborbytearray).into(),
        )?;

        // HMAC SHA384 is 48 bytes but HW is configured to return 64 bytes for HSSHA.
        // HW can be configured to return 48 bytes but it will mean modifying lot of
        // existing code.
        let hash_gsram_slice = hash_gsram.as_ref();
        if encrypted_pin.tag != hash_gsram_slice[..48] {
            Err(HsmErr::PinDecryptionFailed)?
        }

        Ok(())
    }

    fn decrypt_credential(
        &self,
        tag: TagId,
        aes_key: &[u8],
        cipher_text: &MborByteArray<16>,
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

    // Get masking key algorithem
    fn get_masking_key_algo(&self) -> MaskingKeyAlgorithm {
        MaskingKeyAlgorithm::AesCbc256Hmac384
    }

    // Get session masking key
    fn get_session_masking_key(&self) -> HsmResult<IoMemRange> {
        let sess_id = self.id();
        let key_id = self.state.session_table().get_target_session(sess_id)?;
        let session_key =
            self.state
                .vault()
                .open_session_key(self.app_vault_id(), key_id, false)?;
        let masking_key_blob = session_key.masking_key_blob()?;

        Ok(IoMemRange::from(masking_key_blob.as_ref()))
    }

    // Get partition masking key
    fn get_partition_masking_key(&self) -> HsmResult<IoMemRange> {
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

        Ok(IoMemRange::from(masking_key_blob.as_ref()))
    }

    // Assemble DdiMaskedKeyMetadata from key vault given a key id
    fn get_metadata_from_vault(&self, key_label: &[u8], key_id: KeyId) -> HsmResult<Vec<u8>> {
        let vault_key = self.state.vault().key_unchecked(key_id);
        let entry_attributes = vault_key.attributes()?;

        // Check if this key is RSA Unwrap key
        let is_rsa_unwrap_key = self
            .state
            .unwrapping_key_id()
            .map(move |id| key_id == id)
            .unwrap_or_default();

        let key_kind = if is_rsa_unwrap_key {
            DdiKeyType::RsaUnwrap
        } else {
            vault_key.kind()?.try_into()?
        };

        let metadata = DdiMaskedKeyMetadata {
            svn: Some(self.state.bks_table().get_current_svn()),
            key_type: key_kind,
            key_attributes: DdiMaskedKeyAttributes {
                blob: entry_attributes
                    .as_bytes()
                    .try_into()
                    .map_err(|_| HsmErr::InvalidArgument)?,
            },
            bks2_index: Some(0),
            key_tag: vault_key.key_tag()?,
            key_label: MborByteArray::new_with_len(key_label.as_ptr(), key_label.len()),
            key_length: if key_kind.is_var_hmac() {
                entry_attributes.entry_specific[VarLenHmacShaKey::KEY_LENGTH_INDEX] as u16
            } else {
                EntryKind::try_from(key_kind).map(|entry| entry.raw_key_blob_size() as u16)?
            },
        };

        // Get mbor encoded lenght for metadata
        let mut accumulator = MborLenAccumulator::default();
        metadata.mbor_len(&mut accumulator);
        let metadata_len = accumulator.len();

        // Mbor encode metadata
        let mut encoded_metadata = vec![0u8; metadata_len];

        let mut encoder = MborEncoder::new(&mut encoded_metadata);
        metadata.mbor_encode(&mut encoder).unwrap();

        Ok(encoded_metadata)
    }

    fn import_der_key_from_raw_result(
        &self,
        raw_result: &UnmaskedKeyRawResult<E>,
    ) -> HsmResult<ImportDerKeyResult> {
        let key_type = raw_result.metadata.key_type;

        if key_type == DdiKeyType::RsaUnwrap {
            Err(HsmErr::UnmaskUnwrappingKeyNotAllowed)?;
        }

        let entry_kind = EntryKind::try_from(key_type)?;
        let key_len = raw_result.metadata.key_length as usize;

        let raw_key = &raw_result.decrypted_key.as_ref()[..key_len];
        let mut attributes =
            EntryAttributes::read_from_bytes(raw_result.metadata.key_attributes.blob.as_ref())
                .map_err(|_| HsmErr::InvalidArgument)?;
        if entry_kind.is_var_hmac_key() {
            attributes.entry_specific[VarLenHmacShaKey::KEY_LENGTH_INDEX] = key_len as u8;
        }

        let key_id = self.state.vault().import_raw_key(
            self.app_vault_id(),
            self.id(),
            raw_result.metadata.key_tag,
            entry_kind,
            &attributes,
            raw_key,
        )?;

        let entry_class = entry_kind.into();

        match entry_class {
            EntryClass::Aes | EntryClass::Hmac | EntryClass::Secret | EntryClass::VarLenHmacSha => {
                Ok(ImportDerKeyResult {
                    priv_key_id: key_id,
                    pub_key_data: None,
                    key_type,
                })
            }

            EntryClass::Ecc => {
                let pub_key_data = &raw_result.decrypted_key.as_ref()[entry_kind.raw_key_blob_size()
                    ..entry_kind.raw_key_blob_size()
                        + EntryKind::try_from(key_type.to_public())?.raw_key_blob_size()];

                Ok(ImportDerKeyResult {
                    priv_key_id: key_id,
                    pub_key_data: Some(pub_key_data.to_vec()),
                    key_type,
                })
            }
            EntryClass::Rsa => {
                let rsa_type: RsaSize = match entry_kind {
                    EntryKind::Rsa2kPrivate => RsaSize::Rsa2k,
                    EntryKind::Rsa3kPrivate => RsaSize::Rsa3k,
                    EntryKind::Rsa4kPrivate => RsaSize::Rsa4k,
                    _ => Err(HsmErr::InvalidArgument)?,
                };

                // Get the pub key pka bytes
                let pub_key = RsaPubKey::from_priv_pka_slice(raw_key, rsa_type)?;
                let pub_key_blob = pub_key.to_pka_bytes()?;

                Ok(ImportDerKeyResult {
                    priv_key_id: key_id,
                    pub_key_data: Some(pub_key_blob),
                    key_type,
                })
            }
            EntryClass::RsaCrt => {
                let rsa_type: RsaSize = match entry_kind {
                    EntryKind::Rsa2kPrivateCrt => RsaSize::Rsa2k,
                    EntryKind::Rsa3kPrivateCrt => RsaSize::Rsa3k,
                    EntryKind::Rsa4kPrivateCrt => RsaSize::Rsa4k,
                    _ => Err(HsmErr::InvalidArgument)?,
                };

                let pub_key = RsaPubKey::from_priv_crt_pka_slice(raw_key, rsa_type)?;
                let pub_key_blob = pub_key.to_pka_bytes()?;

                Ok(ImportDerKeyResult {
                    priv_key_id: key_id,
                    pub_key_data: Some(pub_key_blob),
                    key_type,
                })
            }
            EntryClass::AesXtsBulk
            | EntryClass::AesGcmBulk
            | EntryClass::AesGcmBulkUnapproved
            | EntryClass::Free
            | EntryClass::Session
            | EntryClass::MaskingKey => unreachable!(),
        }
    }
}

/// Reverse copy a slice from src to destination
///
/// # Arguments
///
/// * `dst` - Destination slice
/// * `src` - Source slice
pub fn reverse_copy(dst: &mut [u8], src: &[u8]) {
    for (item1, item2) in src.iter().rev().zip(dst.iter_mut()) {
        *item2 = *item1;
    }
}
