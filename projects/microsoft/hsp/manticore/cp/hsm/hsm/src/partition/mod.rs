// Copyright (c) Microsoft Corporation. All rights reserved.

mod bks_table;
mod cdma_vault;
mod cert_chain;
mod cred_mgr;
mod ecc_key_validation;
mod hmac;
mod kdf;
mod part;
mod part_env;
mod part_mgr;
mod pct;
mod pin_policy_mgr;
mod pka_ecc_keygen_dyn;
mod queue;
mod session;
mod session_table;
mod state;
mod tests;
mod vault;

use alloc::boxed::Box;
use alloc::vec::Vec;
use core::ops::Range;
use mcr_crypto_pka::PkaRsaSize;
use mcr_crypto_rng::RngTrait;
use mcr_ddi_types::DdiEncryptedEstablishCredential;
use mcr_ddi_types::DdiEncryptedPin;
use mcr_ddi_types::DdiEncryptedSessionCredential;
use mcr_ddi_types::DdiMaskedKeyMetadata;
use mcr_ddi_types::AES_BLOCK_SIZE;
use mcr_ddi_types::DDI_MAX_KEY_LABEL_LENGTH;
#[cfg(feature = "mcr_test_hooks")]
use mcr_self_test::SelfTest;

use cred_mgr::CredentialMgr;
use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;
use mcr_crypto_pka::PkaEccPublicKey;
use mcr_ddi_mbor::MborByteArray;
use mcr_ddi_types::DdiApiRev;
use mcr_ddi_types::DdiHashAlgorithm;
use mcr_ddi_types::DdiKeyAvailability;
use mcr_ddi_types::DdiKeyProperties;
use mcr_ddi_types::DdiKeyType;
use mcr_ddi_types::DdiKeyUsage;
use mcr_ddi_types::DdiRsaCryptoPadding;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ddi_types::DdiTestAction;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ddi_types::DdiTestActionPinPolicyConfig;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ipc_message::CrashType;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ipc_message::SocCpuId;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ipc_message::StackErrorType;
#[cfg(feature = "mcr_manual_test_hooks")]
use mcr_ipc_message::TdispInterruptInfo;
use mcr_types::*;

use mcr_ddi_types::HMAC384_TAG_SIZE;

pub(crate) use bks_table::*;
pub(crate) use cert_chain::*;
pub(crate) use ecc_key_validation::*;
pub(crate) use part::Partition;
pub(crate) use part_env::*;
pub(crate) use part_mgr::HsmPartitionMgr;
pub(crate) use pct::*;
pub(crate) use pin_policy_mgr::*;
pub(crate) use pka_ecc_keygen_dyn::*;
pub(crate) use queue::IoQueue;
pub(crate) use queue::IoQueueDeleteContext;
use queue::IoQueueMgr;
pub(crate) use session::*;
pub(crate) use state::*;
pub(crate) use vault::*;
use zeroize::Zeroize;

use crate::cmd_scheduler::*;
use crate::crypto_env::CryptEnv;
use crate::env::*;
use crate::error::*;
use crate::heap::*;
use crate::key_attestation::cose_key::CoseKeyEncoderTrait;
use crate::key_attestation::report::PUBLIC_KEY_MAX_SIZE;
use crate::partition::store::EntryAttributeFlags;
use crate::partition::store::EntryAttributes;
use crate::x509::AzihsmLeafCertTbs;
use crate::x509::Ecdsa384Signature;

/// Key Id
pub(crate) type KeyId = u16;

/// Output contents for Get Establish Credential Encryption key command
pub(crate) type GetEstablishCredEncryptionKeyOut = GetEncryptionKeyOut;

/// Output contents for Get Session Encryption key command
pub(crate) type GetSessionEncryptionKeyOut = GetEncryptionKeyOut;

type DmaBuffer<E> = <<<E as HsmEnvTrait>::Hal as HsmHalTrait>::DmaHeap as HsmDmaHeapTrait>::Alloc;

/// Open key output
pub(crate) struct OpenKeyData {
    /// Open key operation phase
    pub(crate) phase: OpenKeyPhase,

    /// ID of key
    pub(crate) id: KeyId,

    /// Kind of key
    pub(crate) kind: EntryKind,

    /// Flags of key
    pub(crate) flags: EntryAttributeFlags,

    /// Public key PKA format data, if applicable
    pub(crate) pub_key: Option<PublicKey>,

    /// Bulk key id, if applicable
    pub(crate) bulk_key_id: Option<u16>,
}

/// Open key operation phase
#[derive(Clone, Copy, Default, PartialEq)]
pub(crate) enum OpenKeyPhase {
    /// Init phase
    #[default]
    Init,

    /// Waiting for UPKA engine acquisition
    PendingUpkaEngine,

    /// Waiting for Montgomery constant calculation completion
    PendingMontgomeryConstCalc,

    /// Waiting for point multiplication completion
    PendingPointMultiplication,

    /// Done phase
    Done,
}

/// Convert EccGenPubKeyState to OpenKeyPhase for open key processing
impl From<EccPtMultiplicationState> for OpenKeyPhase {
    fn from(value: EccPtMultiplicationState) -> Self {
        match value {
            EccPtMultiplicationState::WaitForMontgomeryConstCalc => {
                OpenKeyPhase::PendingMontgomeryConstCalc
            }
            EccPtMultiplicationState::WaitForPointMultiplication => {
                OpenKeyPhase::PendingPointMultiplication
            }
        }
    }
}

/// Public key type
#[allow(clippy::large_enum_variant)]
pub(crate) enum PublicKey {
    EccPubKey(PkaEccPublicKey),
    RsaPubKey(RsaPubKey),
}

impl CoseKeyEncoderTrait for PublicKey {
    fn to_cose_key(&self) -> Result<([u8; PUBLIC_KEY_MAX_SIZE], u16), HsmErr> {
        match self {
            Self::EccPubKey(ecc_pub) => ecc_pub.to_cose_key(),
            Self::RsaPubKey(rsa_pub) => rsa_pub.to_cose_key(),
        }
    }
}

impl PkaConvertible for PublicKey {
    type Output = Vec<u8>;
    fn to_pka_bytes(&self) -> Result<Self::Output, HsmErr> {
        match self {
            Self::EccPubKey(ecc_pub) => ecc_pub.to_pka_bytes(),
            Self::RsaPubKey(rsa_pub) => rsa_pub.to_pka_bytes(),
        }
    }
}

impl PkaConvertibleZc for PublicKey {
    fn pka_as_slice(&self) -> Result<&[u8], HsmErr> {
        match self {
            Self::EccPubKey(ecc_pub) => ecc_pub.pka_as_slice(),
            Self::RsaPubKey(rsa_pub) => rsa_pub.pka_as_slice(),
        }
    }
}

impl PublicKey {
    pub fn ddi_key_type(&self) -> DdiKeyType {
        match self {
            Self::EccPubKey(ecc_pub) => match ecc_pub.curve {
                PkaEccCurve::Ecc256 => DdiKeyType::Ecc256Public,
                PkaEccCurve::Ecc384 => DdiKeyType::Ecc384Public,
                PkaEccCurve::Ecc521 => DdiKeyType::Ecc521Public,
            },
            Self::RsaPubKey(rsa_pub) => match rsa_pub.rsa_type {
                RsaSize::Rsa2k => DdiKeyType::Rsa2kPublic,
                RsaSize::Rsa3k => DdiKeyType::Rsa3kPublic,
                RsaSize::Rsa4k => DdiKeyType::Rsa4kPublic,
            },
        }
    }
}

/// Get Unwrapping key output
pub(crate) struct GetUnwrappingKeyOut {
    /// Unwrapping key ID
    pub(crate) id: KeyId,

    /// Public key PKA format data
    pub(crate) data: RsaPubKey,
}

/// Get Unwrapping key command context
pub(crate) struct GetUnwrappingKeyCtx<E: HsmEnvTrait + 'static> {
    /// Reference to the HSP IPC channel.
    pub(crate) channel_ref: Option<HspIpcChannelRef<E>>,

    /// Get Unwrapping key output
    pub(crate) output: Option<GetUnwrappingKeyOut>,
}
/// Get Partition Identifiers output
pub(crate) struct GetPartitionIdCtx {
    /// Tag identifier.
    pub(crate) tag: TagId,

    /// Command information returned by the PKA driver.
    pub(crate) cmd_info: Option<PkaEccCmd>,

    /// Key Data
    pub(crate) identifiers_present: bool,

    /// PKA engine for ECC key generation
    pub(crate) engine: Option<Box<dyn PkaEccKeygenDyn>>,
}

/// Result of generating partition identifiers
pub struct PartitionIdGenResult {
    /// ECC curve
    pub curve: PkaEccCurve,

    /// native X||Y, 2*n bytes
    pub pub_xy: SecureByteVec,

    // n bytes
    pub priv_d: SecureByteVec,
}

/// Credential Encryption Key Data
pub(crate) struct CredentialEncryptionKeyData {
    /// Public key information
    pub(crate) pub_key_data: PkaEccPublicKey,

    /// Nonce
    pub(crate) nonce: [u8; 32],
}

/// Get Establish Cred Encryption key command context
pub(crate) struct GetEstablishCredEncryptionKeyCtx<E: HsmEnvTrait + 'static> {
    /// Tag identifier.
    pub(crate) tag: TagId,

    /// Reference to the PKA engine.
    pub(crate) engine_ref: Option<PkaEngineRef<E>>,

    /// Command information returned by the PKA driver.
    pub(crate) cmd_info: Option<PkaEccCmd>,

    /// Key Data
    pub(crate) key_data: Option<CredentialEncryptionKeyData>,
}

/// Output for Get Establish Cred Encryption key command and Get Session Encryption key command
pub(crate) struct GetEncryptionKeyOut {
    /// Public key PkaEcc format data
    pub(crate) pub_key: PkaEccPublicKey,

    /// Nonce
    pub(crate) nonce: [u8; 32],

    /// Some(KeyId) if key is generated, None otherwise
    pub(crate) new_key_id: Option<KeyId>,
}

/// Get Session Encryption key command context
pub(crate) struct GetSessionEncryptionKeyCtx<E: HsmEnvTrait + 'static> {
    /// Tag identifier.
    pub(crate) tag: TagId,

    /// Reference to the PKA engine.
    pub(crate) engine_ref: Option<PkaEngineRef<E>>,

    /// Command information returned by the PKA driver.
    pub(crate) cmd_info: Option<PkaEccCmd>,

    /// Key Data
    pub(crate) key_data: Option<CredentialEncryptionKeyData>,
}

/// Context for Key Signing
pub(crate) struct KeySignContext<E: HsmEnvTrait + 'static> {
    /// Reference to the PKA engine.
    pub(crate) engine_ref: PkaEngineRef<E>,

    /// Buffer containing key digest
    pub(crate) _digest_buf: DmaBuffer<E>,
}

/// Establish Credential command states
pub(crate) enum EstablishCredentialCmdState {
    /// Wait for first Montgomery constant calculation
    MontgomeryConstCalc,

    /// Wait for Public Key Validation
    PublicKeyValidation,

    /// Wait for POTA Public Key Validation
    PotaPublicKeyValidation,

    /// Wait for signature verification
    VerifySignature,

    /// Wait for second Montgomery constant calculation
    SecondMontgomeryConstCalc,

    /// Wait for ECDH compute
    EcdhCompute,
}

/// Open Session command state
pub(crate) enum OpenSessionCmdState {
    /// Wait for first Montgomery constant calculation
    MontgomeryConstCalc,

    /// Wait for Public Key Validation
    PublicKeyValidation,

    /// Wait for ECDH compute
    EcdhCompute,
}

/// Open Session command context
pub(crate) struct OpenSessionCtx<E: HsmEnvTrait + 'static> {
    pub(crate) tag: TagId,

    /// Reference to the PKA engine.
    pub(crate) engine_ref: PkaEngineRef<E>,

    /// Context for PkaEccCmd.
    pub(crate) cmd_info: PkaEccCmd,

    /// Command state
    pub(crate) state: OpenSessionCmdState,
}

/// Establish credential command context
pub(crate) struct EstablishCredentialCtx<E: HsmEnvTrait + 'static> {
    /// Tag
    pub(crate) tag: TagId,

    /// Reference to the PKA engine.
    pub(crate) engine_ref: PkaEngineRef<E>,

    /// Context for PkaEccCmd.
    pub(crate) cmd_info: PkaEccCmd,

    /// Command state
    pub(crate) state: EstablishCredentialCmdState,

    /// Digest buffer
    pub(crate) digest_buf: DmaBuffer<E>,
}

/// Raw result from unmasking key operation
pub(crate) struct UnmaskedKeyRawResult<E: HsmEnvTrait + 'static> {
    /// Metadata
    pub(crate) metadata: DdiMaskedKeyMetadata,

    /// Decrypted key
    pub(crate) decrypted_key: DmaBuffer<E>,
}

/// Result after unmasking + import operation
pub(crate) struct UnmaskedKeyImportResult {
    /// Key imported result
    pub(crate) import_result: ImportDerKeyResult,

    /// Key label
    pub(crate) key_label: MborByteArray<DDI_MAX_KEY_LABEL_LENGTH>,
}

/// Final result of unmasking key operation
/// which includes unmasking a key and trying to synchronously import it into vault
/// If the import action is async, then returns the raw result
/// Otherwise return the key id fromt import operation
pub(crate) struct UnmaskedKeyResult<E: HsmEnvTrait + 'static> {
    /// Key imported result
    pub(crate) import_result: Option<UnmaskedKeyImportResult>,

    /// Raw result of unmasking key
    pub(crate) raw_result: Option<UnmaskedKeyRawResult<E>>,
}

/// TODO: Code duplication between Session and Partition. We would need to fix this.
impl<E: HsmEnvTrait> CryptEnv for Partition<E> {
    fn aescbc256_decrypt(
        &self,
        key: &[u8],
        iv: &[u8],
        ciphertext: &[u8],
        plaintext: &mut [u8],
    ) -> Result<usize, HsmErr> {
        let plaintext_len = self.aescbc256_enc_data_len(plaintext.len());
        let padded_plaintext = self.dma_alloc(plaintext_len)?;

        let local_iv = self.dma_copy_alloc(iv)?;

        let iv_binding = IoMemRange::from(local_iv.as_ref());
        let msg_in_binding = IoMemRange::from(ciphertext);
        let msg_out_binding = IoMemRange::from(padded_plaintext.as_ref());

        let input = AesEncDecIn::new(
            AesEncDecMode::Cbc,
            AesEncDecOp::Decrypt,
            Some(&iv_binding),
            &msg_in_binding,
            &msg_out_binding,
        );

        self.aes_enc_dec_internal_with_key_blob(1, key, &input)?;

        plaintext.copy_from_slice(&msg_out_binding.slice()[..plaintext.len()]);

        Ok(plaintext.len())
    }

    fn aescbc256_encrypt(
        &self,
        key: &[u8],
        plaintext: &[u8],
        iv: &mut [u8],
        ciphertext: &mut [u8],
    ) -> Result<usize, HsmErr> {
        let plaintext_len = self.aescbc256_enc_data_len(plaintext.len());
        let mut padded_plaintext = self.dma_alloc(plaintext_len)?;
        padded_plaintext.as_ref_mut()[..plaintext.len()].copy_from_slice(plaintext);

        let local_iv = self.dma_copy_alloc(iv)?;

        let iv_binding = IoMemRange::from(local_iv.as_ref());
        let msg_in_binding = IoMemRange::from(padded_plaintext.as_ref());
        let msg_out_binding = IoMemRange::from(ciphertext);

        let input = AesEncDecIn::new(
            AesEncDecMode::Cbc,
            AesEncDecOp::Encrypt,
            Some(&iv_binding),
            &msg_in_binding,
            &msg_out_binding,
        );

        self.aes_enc_dec_internal_with_key_blob(1, key, &input)?;

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
        )
        .map_err(|_| HsmErr::HmacComputeFailed)?;

        Ok(output)
    }

    fn kbkdf_sha384(
        &self,
        key: &[u8],
        label: Option<&[u8]>,
        context: Option<&[u8]>,
        out_len: usize,
        output: &mut [u8],
    ) -> Result<(), HsmErr> {
        if output.len() < out_len {
            return Err(HsmErr::KbkdfInvalidInputParam);
        }
        // Allocate GSRAM buffer for output to hold enough buffer for SHA digest + out_len
        let mut kbkdf_output_gsram = self.dma_alloc(SHA_DIGEST_MAX_SIZE_BYTES + out_len)?;

        self.kbkdf_impl(
            key,
            label.unwrap_or(&[]),
            context.unwrap_or(&[]),
            DdiHashAlgorithm::Sha384,
            kbkdf_output_gsram.as_ref_mut(),
            out_len as u16,
        )?;

        output[..out_len].copy_from_slice(&kbkdf_output_gsram.as_ref()[..out_len]);

        Ok(())
    }

    fn generate_random(&self, output: &mut [u8]) -> Result<(), HsmErr> {
        self.state.env().rng().bytes(output);
        Ok(())
    }

    fn aescbc256_enc_data_len(&self, plaintext_len: usize) -> usize {
        plaintext_len + (AES_BLOCK_SIZE - (plaintext_len % AES_BLOCK_SIZE))
    }
}

/// HSM Partition
pub(crate) trait HsmPartition {
    type Env: HsmEnvTrait;

    /// User session type
    type UserSession: HsmUserSession;

    /// Returns the minimum API revision supported by the function
    fn min_api_rev(&self) -> DdiApiRev;

    /// Returns the maximum API revision supported by the function
    fn max_api_rev(&self) -> DdiApiRev;

    /// Enable the partition
    fn enable(&self);

    /// Disable the function
    ///
    /// # Arguments
    ///
    /// * `delete_ctx` - IO queue delete context
    ///
    /// # Returns
    ///
    /// * Returns `true` if IO queue delete is pending else `false`
    ///
    /// # Note
    ///
    /// To be called to disable the function without resetting the resource mask.
    fn disable(&self, delete_ctx: Option<IoQueueDeleteContext>) -> bool;

    /// Begin migration of the partition
    ///
    /// # Arguments
    ///
    /// * `delete_ctx` - IO queue delete context
    ///
    /// # Returns
    ///
    /// * Returns `true` if IO queue delete is pending else `false`
    ///
    /// # Note
    ///
    /// if no IO queue delete is pending, `end_migrate` should be called to complete the migration
    /// if IO queue delete is pending, the caller should wait for IO and then call `end_migrate` to complete the migration
    fn begin_migrate(&self, delete_ctx: Option<IoQueueDeleteContext>) -> bool;

    /// End migration of the function
    fn end_migrate(&self);

    /// Reset the function
    ///
    /// # Caution
    ///
    /// To be called to reset the function and clear all resources.
    fn reset(&self);

    /// Set the resource mask
    fn set_resource_mask(&self, mask: u128);

    /// Get the resource mask
    fn resource_mask(&self) -> u128;

    /// Check if the function is enabled
    ///
    /// # Returns
    ///
    /// * Returns `true` if enabled else `false`
    fn enabled(&self) -> bool;

    /// Rollback open user session
    ///
    /// # Arguments
    ///
    /// * `id` - Session id
    /// * `is_reopen` - Flag indicates if this is a re-open session rollback
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn rollback_open_session(&self, id: SessionId, is_reopen: bool) -> HsmResult<()>;

    /// Close user session
    ///
    /// # Arguments
    ///
    /// * `id` - Session id
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn close_user_session(&self, id: SessionId) -> HsmResult<()>;

    /// Delete user session
    ///
    /// # Arguments
    ///
    /// * `id` - Session id
    fn delete_user_session(&self, id: SessionId);

    /// Begin establish credential command.
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `pub_key` - Public key data from client
    /// * `pota_pub_key`: POTA Public Key
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed.
    /// * Returns HsmError::Pending if PkaEngine is not available.
    fn begin_establish_credential(
        &self,
        tag: TagId,
        pub_key: &IoMemRange,
        pota_pub_key: &IoMemRange,
    ) -> HsmResult<EstablishCredentialCtx<Self::Env>>;

    /// Continue establish credential command.
    ///
    /// # Arguments
    ///
    /// * `ctx` - Ctx returned by begin_establish_credential
    /// * `pub_key` - Public key data from client
    /// * `pota_pub_key`: POTA Public Key
    /// * `pota_sig`: POTA signature
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed.
    fn continue_establish_credential(
        &self,
        ctx: EstablishCredentialCtx<Self::Env>,
        pub_key: &IoMemRange,
        pota_pub_key: &IoMemRange,
        pota_sig: &IoMemRange,
    ) -> HsmResult<EstablishCredentialCtx<Self::Env>>;

    /// End establish credential command
    ///
    /// # Arguments
    ///
    /// * `ctx` - Ctx returned by continue_establish_credential
    /// * `encrypted_credential` - Encrypted credential
    ///
    /// Returns `Ok(())` is successful else `Err(HsmError)` if failed
    fn end_establish_credential(
        &self,
        ctx: EstablishCredentialCtx<Self::Env>,
        encrypted_credential: &DdiEncryptedEstablishCredential,
    ) -> HsmResult<()>;

    /// Start open session command.
    /// Opening session requires ECDH, HKDF to eventually derive the encryption key,
    /// This function starts the Montgomery Constant operation on Pka Engine
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `pub_key` - Public key data from client
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed.
    /// * Returns HsmError::Pending if PkaEngine is not available.
    fn begin_open_user_session(
        &self,
        tag: TagId,
        pub_key: &IoMemRange,
    ) -> HsmResult<OpenSessionCtx<Self::Env>>;

    /// Continue open session command.
    /// Opening session requires ECDH, HKDF to eventually derive the encryption key,
    /// This function starts the Point Multiplication operation on Pka Engine.
    ///
    /// # Arguments
    ///
    /// * `ctx` - Ctx returned by begin_open_session
    /// * `pub_key` - Public key data from client
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed.
    fn continue_open_user_session(
        &self,
        ctx: OpenSessionCtx<Self::Env>,
        pub_key: &IoMemRange,
    ) -> HsmResult<OpenSessionCtx<Self::Env>>;

    /// End open app session command, after calling begin_open_session and continue_open_session
    ///
    /// # Arguments
    ///
    /// * `ctx` - Ctx returned by continue_open_session
    /// * `rev` - Api rev
    /// * `encrypted_credential` - Encrypted credential
    /// * `reopen_sess_id` - if Some, it will map a specific virtual session to the physical session id.
    /// * `bk_session_buf` - A buffer that stores the bk session data
    /// * `mk_session_buf` - A buffer that stores the session masking key
    /// * `bmk_session` - BMK session data for reopen operation to extract session masking key
    ///
    /// Returns `Ok(Self::AppSession)` is successful else `Err(HsmError)` if failed
    #[allow(clippy::too_many_arguments)]
    fn end_open_user_session(
        &self,
        ctx: OpenSessionCtx<Self::Env>,
        rev: DdiApiRev,
        encrypted_credential: &DdiEncryptedSessionCredential,
        reopen_sess_id: Option<u16>,
        bk_session_buf: &mut [u8],
        mk_session_buf: &mut [u8],
        bmk_session: Option<&[u8]>,
    ) -> HsmResult<Self::UserSession>;

    /// Get user session
    ///
    /// # Returns
    ///
    /// * Returns `Ok(Self::UserSession)` if session exists and is active
    /// * If `allow_disabled` is true, a disabled session can also be returned as `Ok(_)`.
    /// * If session does not exist, returns `Err(HsmError::SessionNotFound)`
    /// * If session exists but needs renegotiation, returns `Err(HsmError::SessionNeedsRenegotiation)`
    fn user_session(&self, id: SessionId, allow_disabled: bool) -> HsmResult<Self::UserSession>;

    /// Enable the IO queue
    ///
    /// * `sq_id` - Submission queue id
    /// * `cq_id` - Completion queue id
    fn enable_io_queue(&self, sq_id: DevSqId, cq_id: DevCqId);

    /// Disable the IO queue
    ///
    ///  # Arguments
    ///
    /// * `sq_id` - Submission queue id
    /// * `delete_ctx` - IO queue delete context
    ///
    /// # Returns
    ///
    /// `pending` - `true` if IO queue delete is pending
    fn disable_io_queue(&self, sq_id: DevSqId, delete_ctx: Option<IoQueueDeleteContext>) -> bool;

    /// Get the IO queue
    ///
    /// * `sq_id` - Submission queue id
    ///
    /// # Returns
    ///
    /// * Returns `Some(IoQueue)` if enabled else `None`
    fn io_queue(&self, sq_id: DevSqId) -> Option<IoQueue>;

    /// Clear the unwrapping key from both key vault and persistent store
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn clear_unwrapping_key(&mut self) -> HsmResult<()>;

    /// Get unwrapping key id
    ///
    /// # Returns
    ///
    /// * Returns `Some(KeyId)` if the key exists else `None`
    fn unwrapping_key_id(&self) -> Option<KeyId>;

    /// Get the alias certificate from GSRAM
    ///
    /// # Returns
    ///
    /// * Returns IoMemRange containing the alias certificate
    fn get_alias_cert(&self) -> IoMemRange;

    /// Get the alias key length
    ///
    /// # Returns
    ///
    /// * Returns the length of the alias key
    fn get_alias_cert_len(&self) -> usize;

    /// Read alias key from GSRAM and DER decode it.
    ///
    /// # Returns
    ///
    /// * Returns `Ok(alias key, alias key id)` if successful else `Err(HsmError)` if failed
    fn get_raw_alias_key(&self) -> HsmResult<SecureByteVec>;

    /// Begin close user session to handle aes bulk 256 keys
    ///
    /// # Arguments
    /// * `tag` - Tag ID
    /// * `pfn` - Used to in IPC message
    /// * `id` - Session ID
    ///
    /// # Returns
    /// * Returns `Ok(AesBulk256Cmd)` if successful else `Err(HsmError)` if failed
    fn begin_close_user_session(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        id: SessionId,
    ) -> HsmResult<AesBulk256Cmd<Self::Env>>;

    /// End close user session to handle aes bulk 256 keys
    ///
    /// # Arguments
    /// * `op` - Operational data
    /// * AesBulk256Cmd - AES Bulk 256 command
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn end_close_user_session(&self, op: &AesBulk256Cmd<Self::Env>) -> HsmResult<()>;

    /// Begin Get Partition Identifiers
    ///
    /// # Arguments
    /// * `tag` - Tag id
    /// # Returns
    /// * Returns Ok(GetPartitionIdCtx) if successful, else `Err(HsmError)`
    fn begin_generate_partition_identifiers(&self, tag: TagId) -> HsmResult<GetPartitionIdCtx>;

    /// Continue Get Partition Identifiers
    ///
    /// # Arguments
    /// * `tag` - Tag id
    /// * `ctx` - Ctx returned by begin_get_partition_identifiers with engine_ref
    /// # Returns
    /// * Returns Ok(()) if successful, else `Err(HsmError)`
    fn continue_generate_partition_identifiers(
        &self,
        tag: TagId,
        ctx: GetPartitionIdCtx,
    ) -> HsmResult<PartitionIdGenResult>;

    /// End Get Partition Identifiers
    /// Persist the generated partition identifiers and mark them valid.
    ///
    /// # Arguments
    /// * `km` - PartitionIdGenResult
    /// # Returns
    /// * Returns Ok(()) if successful, else `Err(HsmError)`
    fn end_generate_partition_identifiers(&self, km: PartitionIdGenResult) -> HsmResult<()>;

    /// Begin Generate PID Certificate
    ///
    /// # Arguments
    /// * `tag` - Tag id
    /// * `key_blob` - Alias Key blob used for signing the PID certificate
    /// * `signature_buffer` - DMA buffer to fill the signature
    ///
    /// # Returns
    /// * Returns Ok(CertSignContext) if successful, else `Err(HsmError)`
    fn begin_generate_pid_cert(
        &self,
        tag: TagId,
        key_blob: &[u8],
    ) -> HsmResult<CertSignContext<Self::Env>>;

    /// End Generate PID Certificate
    ///
    /// # Arguments
    /// * `tag` - Tag id
    /// * `cert_sign_ctx` - Context for certificate signing
    ///
    /// # Returns
    /// * Returns Ok(()) if successful, else `Err(HsmError)`
    fn end_generate_pid_cert(
        &self,
        tag: TagId,
        cert_sign_ctx: &CertSignContext<Self::Env>,
    ) -> HsmResult<()>;

    /// Check if partition certificate is valid
    ///
    /// # Returns
    ///
    /// * Returns `true` if partition certificate is valid, else `false`
    fn is_partition_cert_valid(&self) -> bool;

    /// Set partition certificate valid
    ///
    /// # Arguments
    /// * `valid` - `true` if partition certificate is valid, else `false`
    fn set_partition_cert_valid(&self, valid: bool);

    /// Get partition certificate
    ///
    /// # Returns
    ///
    /// * Returns IoMemRange containing the partition certificate addr and length
    fn partition_cert(&self) -> IoMemRange;

    /// Get partition certificate length
    ///
    /// # Returns
    /// * Returns length of the partition certificate
    fn partition_cert_length(&self) -> u32;

    /// Set partition certificate
    ///
    /// # Arguments
    /// * `len` - Length of the partition certificate
    ///
    /// # Returns
    /// * Returns Ok(()) if successful, else `Err(HsmError)`
    fn set_partition_cert_length(&self, len: u32) -> HsmResult<()>;

    /// Begin Get Establish Credential Encryption Key
    ///
    /// # Returns
    /// * Returns ctx with pub_key_data if successful, or ctx with engine_ref
    ///   if Ecc gen key command is required
    fn begin_get_establish_cred_encryption_key(
        &self,
        tag: TagId,
    ) -> HsmResult<GetEstablishCredEncryptionKeyCtx<Self::Env>>;

    /// End Get Establish Credential Encryption Key
    /// # Arguments
    /// * `ctx` - Ctx returned by begin_get_establish_cred_encryption_key with engine_ref populated
    ///
    /// # Returns
    /// * Returns Ok(GetEstablishCredEncryptionKeyOut) if successful else `Err(HsmError)` if failed
    fn end_get_establish_cred_encryption_key(
        &self,
        tag: TagId,
        ctx: GetEstablishCredEncryptionKeyCtx<Self::Env>,
    ) -> HsmResult<GetEstablishCredEncryptionKeyOut>;

    /// Begin Get Session Encryption Key
    ///
    /// # Returns
    /// * Returns ctx with pub_key_data if successful, or ctx with engine_ref
    ///   if Ecc gen key command is required
    fn begin_get_session_encryption_key(
        &self,
        tag: TagId,
    ) -> HsmResult<GetSessionEncryptionKeyCtx<Self::Env>>;

    /// End Get Session Encryption Key
    /// # Arguments
    /// * `ctx` - Ctx returned by begin_get_session_encryption_key with engine_ref populated
    ///
    /// # Returns
    /// * Returns Ok(GetSessionEncryptionKeyOut) if successful else `Err(HsmError)` if failed
    fn end_get_session_encryption_key(
        &self,
        tag: TagId,
        ctx: GetSessionEncryptionKeyCtx<Self::Env>,
    ) -> HsmResult<GetSessionEncryptionKeyOut>;

    /// Begin ECC PCT (Pairwise Consistency Test) validation operation
    ///
    /// This validation process ensures that a given ECC key is correctly generated
    /// and operates as expected for its assigned usage (Sign/Verify or ECDH Key Agreement)
    ///
    /// # Validation Process:
    ///
    /// - EccKeyUsage::SignVerify:
    ///   1. A deterministic digest is signed using the private key
    ///   2. The generated signature is verified using the corresponding public key
    ///   3. If the verification succeeds, the key is validated
    ///   4. If verification fails, a system reset/assert is triggered
    ///
    /// - EccKeyUsage::KeyAgreement(ECDH):
    ///   1. The private key performs an ECDH key agreement operation using a static test vector
    ///   2. The resulting shared secret is computed and stored
    ///   3. The ECDH operation is repeated using the opposite static keypair
    ///   4. The two computed shared secrets are compared to verify correctness
    ///   5. If the computed shared secret mismatches, a system reset/assert is triggered
    ///
    /// # Failure Handling:
    ///
    /// - Execution errors (e.g., hardware failures, invalid arguments, or resource issues)
    ///   - Return an appropriate `HsmErr` and halt validation gracefully
    ///
    /// - Verification result failures (e.g., mismatched signature or ECDH shared secret mismatch)
    ///   - Trigger a system reset for security enforcement
    ///
    /// This ensures that the ECC key behaves correctly and is compatible with standard cryptographic operations
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `key_id` - Key id
    /// * `usage` - The expected usage of the key (Sign/Verify or KeyAgreement)
    /// * `public_key` - The public key corresponding to the key being validated
    ///
    /// # Returns
    ///
    /// * Returns `Ok(EccKeyPct<Self::Env>)` if the validation process starts successfully
    /// * Returns `Err(HsmErr)` if validation setup fails due to an execution error
    fn begin_ecc_pct_validation(
        &self,
        tag: TagId,
        key_id: KeyId,
        usage: EccKeyUsage,
        public_key: PkaEccPublicKey,
    ) -> HsmResult<EccKeyPct<Self::Env>>;

    /// Begin ECC PCT (Pairwise Consistency Test) validation operation with raw private key
    fn begin_ecc_pct_validation_raw(
        &self,
        tag: TagId,
        usage: EccKeyUsage,
        public_key: &PkaEccPublicKey,
        priv_d: &[u8],
    ) -> HsmResult<EccKeyPct<Self::Env>>;

    /// Begin ECC PCT (Pairwise Consistency Test) validation operation with PKA engine reference
    fn begin_ecc_pct_validation_with_engine(
        &self,
        tag: TagId,
        usage: EccKeyUsage,
        public_key: &PkaEccPublicKey,
        priv_d: &[u8],
        engine_ref: PkaEngineRef<Self::Env>,
    ) -> HsmResult<EccKeyPct<Self::Env>>;

    /// Continue ECC PCT validation operation
    ///
    /// # Arguments
    ///
    /// * `ecc_key_pct` - Ongoing Ecc Key Pct Object
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful, else `Err(HsmErr)` if failed
    fn continue_ecc_pct_validation(
        &self,
        tag: TagId,
        ecc_key_pct: &mut EccKeyPct<Self::Env>,
    ) -> HsmResult<()>;

    /// End ECC PCT validation operation
    ///
    /// # Arguments
    ///
    /// * `ecc_key_pct` - Ongoing Ecc Key Pct Object
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful, else `Err(HsmErr)` if failed
    fn end_ecc_pct_validation(
        &self,
        tag: TagId,
        ecc_key_pct: &mut EccKeyPct<Self::Env>,
    ) -> HsmResult<bool>;

    /// Checks if the PCT validation state requires final verification
    ///
    /// # Arguments
    ///
    /// * `pct_op` - Reference to the ongoing PCT validation operation
    ///
    /// # Returns
    ///
    /// * Returns `true` if the validation is in its final state, else `false`
    fn is_pct_final_state(&self, pct_op: &EccKeyPct<Self::Env>) -> bool;

    /// Authenticate the User Credentials with Pin Policy enforcement
    ///
    /// # Arguments
    /// * `` -
    /// * `` -
    ///
    /// # Returns
    /// * Returns `Ok(())` if valid authentication, Err otherwise
    fn authorize_user_with_pin_policy(&self, id: &AppId, pin: &AppPin) -> HsmResult<()>;

    /// Verify that provided nonce matches the current nonce on device
    ///
    /// # Arguments
    /// * `nonce` - provided nonce
    ///
    /// # Returns
    /// * Returns `Ok(())` if same else `Err(HsmError)` if different
    fn verify_nonce(&self, nonce: [u8; 32]) -> HsmResult<()>;

    /// Verify credential has not been set
    ///
    /// # Returns
    /// * Returns `Ok(())` if no credential exists else `Err(HsmError)` if they do
    fn verify_cred_is_not_set(&self) -> HsmResult<()>;

    /// Verify credential has been set
    ///
    /// # Returns
    /// * Returns `Ok(())` if credential exists else `Err(HsmError)` if it doesn't
    fn verify_cred_is_set(&self) -> HsmResult<()>;

    /// Clear credentials
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)`
    fn clear_credentials(&self) -> HsmResult<()>;

    /// Clear Partition Provisioning State
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)`
    fn clear_provisioning_state(&self) -> HsmResult<()>;

    #[cfg(feature = "mcr_test_hooks")]
    /// Set current SVN value
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)`
    fn set_current_svn(&self, svn: u64) -> HsmResult<()>;

    /// Store the partition data in GSRAM store
    fn store_data(&self);

    /// Set certificate chain length context
    ///
    /// # Arguments
    ///
    /// * `ctx` - Certificate chain length context
    fn set_cert_chain_lengths_info(&self, info: Option<GetCertChainLengthsInfo>);

    /// Get certificate chain length
    ///
    /// # Arguments
    /// * `cert_id` - Certificate ID
    fn get_cert_len(&self, cert_id: u8) -> Option<usize>;

    /// Delete internal key from key vault
    ///
    /// # Arguments
    /// * `key_id` - Key ID
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn delete_internal_key(&self, key_id: KeyId) -> HsmResult<()>;

    /// Unset the establish credential encryption key id
    fn unset_establish_cred_encryption_key_id(&self);

    /// Unset the session encryption key id
    fn unset_session_encryption_key_id(&self);

    /// Get FIPS Approved status
    ///
    /// # Returns
    ///
    /// * Returns `true` if FIPS approved else `false`
    fn is_fips_approved(&self) -> bool;

    /// Notify PCT validation failure to HSP
    ///
    /// # Arguments
    ///
    /// * `err` - Error code indicating the failure reason
    fn notify_pct_validation_failure(&self, err: u32);

    /// Set partition GUID sent as a set resource parameter by the host.
    ///
    /// # Arguments
    ///
    /// * `guid` - GUID to be set
    fn set_vm_launch_guid(&self, guid: &VmLaunchGuid);

    /// Get partition GUID
    ///
    /// # Returns
    ///
    /// * Returns `Some(PartitionGuid)` if set else `None`
    fn vm_launch_guid(&self) -> VmLaunchGuid;

    /// Send IPC request to get the lengths of individual certificates in the chain.
    ///
    /// # Arguments
    /// * `tag` - Tag ID
    /// * `get_cert_chain_lengths_ctx` - The context for the get certificate chain lengths operation
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn begin_get_dev_id_cert_chain_info(
        &self,
        tag: TagId,
        get_cert_chain_lengths_ctx: &mut GetCertLengthsContext<Self::Env>,
    ) -> HsmResult<()>;

    /// IPC response to get the lengths of individual certificates in the chain.
    ///
    /// # Arguments
    /// * `get_cert_chain_lengths_ctx` - The context for the get certificate chain lengths operation
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn end_get_dev_id_cert_chain_info(
        &self,
        get_cert_chain_lengths_ctx: &mut GetCertLengthsContext<Self::Env>,
    ) -> HsmResult<()>;

    /// Update Cert Chain Lengths Info with Alias and Partition Id Cert info
    ///
    /// # Arguments
    /// * `cert_info` - Certificate chain lengths info
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn update_cert_chain_lengths_info(
        &self,
        cert_info: &mut GetCertChainLengthsInfo,
    ) -> HsmResult<()>;

    /// Generate ECDSA384 signature from the provided buffer
    ///
    /// # Arguments
    /// * `signature_buffer` - Buffer containing the signature data
    ///
    /// # Returns
    /// * Returns `Ok(Ecdsa384Signature)` if successful else `Err(HsmError)` if failed
    fn get_ecdsa384_signature_from_buffer(
        &self,
        signature_buffer: &[u8],
    ) -> HsmResult<Ecdsa384Signature>;

    /// Begin Get Certificate
    ///
    /// # Arguments
    /// * `tag` - Tag ID
    /// * `get_cert_ctx` - The context for the get certificate chain operation
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn begin_get_cert(
        &self,
        tag: TagId,
        get_cert_ctx: &mut GetCertContext<Self::Env>,
    ) -> HsmResult<()>;

    /// End Get Certificate
    ///
    /// # Arguments
    /// * `get_cert_ctx` - The context for the get certificate chain operation
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn end_get_cert(&self, get_cert_ctx: &mut GetCertContext<Self::Env>) -> HsmResult<()>;

    /// Get Partition Id private key.
    ///
    /// # Returns
    /// * Returns `Some(&[u8])` if the key is set else `None`
    fn get_partition_id_private_key_blob(&self) -> Option<&[u8]>;

    /// Generate BK Boot data
    ///
    /// # Arguments
    /// * `bk_boot` - Buffer to store the generated BK Boot data
    ///
    /// Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn generate_bk_boot(&self, bk_boot: &mut [u8]) -> HsmResult<()>;

    /// Mask BK3 using the masking key
    ///
    /// # Arguments
    /// * `bk3` - The BK3 data to be masked
    /// * `masking_key` - The masking key to be used
    /// * `output_len` - The length of the output buffer
    /// * `output_buf` - The buffer to store the masked BK3 data
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn mask_bk3(
        &self,
        bk3: &[u8],
        masking_key: &[u8],
        output_len: &mut usize,
        output_buf: &mut [u8],
    ) -> HsmResult<()>;

    /// Mask BK Boot using the masking key
    ///
    /// # Arguments
    /// * `bk_boot` - The BK Boot data to be masked
    /// * `output_len` - The length of the output buffer
    /// * `output_buf` - The buffer to store the masked BK Boot data
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn mask_bk_boot(
        &self,
        bk_boot: &[u8],
        output_len: &mut usize,
        output_buf: &mut [u8],
    ) -> HsmResult<()>;

    /// Get the masked BK Boot memory range
    ///
    /// # Returns
    /// * Returns the memory range of the masked BK Boot
    fn masked_bk_boot(&self) -> IoMemRange;

    /// Get masked BK Boot length
    ///
    /// # Returns
    /// * Returns the length of the masked BK Boot
    fn get_masked_bk_boot_len(&self) -> u32;

    /// Set masked BK Boot length
    ///
    /// # Arguments
    /// * `len` - The length of the masked BK Boot
    fn set_masked_bk_boot_len(&self, len: u32);

    /// Get the sealed BK3 memory range
    ///
    /// # Returns
    /// * Returns the memory range of the sealed BK3
    fn sealed_bk3(&self) -> IoMemRange;

    /// Get sealed BK3 length
    ///
    /// # Returns
    /// * Returns the length of the sealed BK3
    fn get_sealed_bk3_len(&self) -> u32;

    /// Set sealed BK3 length
    ///
    /// # Arguments
    /// * `len` - The length of the sealed BK3
    fn set_sealed_bk3_len(&self, len: u32);

    /// Unmask BK3 using the masking key
    ///
    /// # Arguments
    /// * `masked_bk3` - The masked BK3 data to be unmasked
    /// * `bk3` - The output buffer for the unmasked BK3
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn unmask_bk3(&self, masked_bk3: &[u8], bk3: &mut [u8]) -> HsmResult<()>;

    /// Generates BK3 session data and stores it in the partition persistent storage.
    ///
    /// # Arguments
    /// * `bk3` - The BK3 data to be used for generating the BK3 session data
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn generate_and_store_bk3_session(&self, bk3: &[u8]) -> HsmResult<()>;

    /// Generates Backup Key (BK) from the provided BK3 data.
    ///
    /// # Arguments
    /// * `bk3` - The BK3 data to be used for generating the BK
    /// * `bk` - The output buffer for the generated BK
    /// * `pota_pub_key` - POTA public key
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn generate_bk(&self, bk3: &[u8], pota_pub_key: &[u8], bk: &mut [u8]) -> HsmResult<()>;

    /// Generates the masking key for this partition, stores it in the key vault.
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn generate_new_mk_and_import(&self) -> HsmResult<()>;

    /// Generates the Backup Masking Key (BMK) from the provided Backup Key (BK).
    ///
    /// # Arguments
    /// * `bk` - The BK data to be used for generating the BMK
    /// * `bmk_len` - The length of the generated BMK
    /// * `bmk_out` - The output buffer for the generated BMK
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn generate_bmk(&self, bk: &[u8], bmk_len: &mut usize, bmk_out: &mut [u8]) -> HsmResult<()>;

    /// Imports the Masking Key (MK) from the provided BK3 and BMK data.
    ///
    /// # Arguments
    /// * `bk3` - The BK3 data to be used for importing the MK
    /// * `bmk` - The BMK data to be used for importing the MK
    /// * `pota_pub_key` - POTA public key
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn import_mk_from_bmk(&self, bk3: &[u8], pota_pub_key: &[u8], bmk: &[u8]) -> HsmResult<()>;

    /// Check if the partition is provisioned
    ///
    /// # Returns
    /// * Returns `true` if provisioned else `false`
    fn is_partition_provisioned(&self) -> bool;

    /// Begin signature operation using a key blob
    ///
    /// # Arguments
    /// * `tag` - Tag ID
    /// * `key_data` - IoMemRange of key data to be hashed
    /// * `signature` - IoMemRange to store the signature
    ///
    /// # Returns
    /// * Returns `Ok(KeySignContext)` if successful else `Err(HsmError)`
    fn begin_signature_with_part_priv_key(
        &self,
        tag: TagId,
        key_data: &IoMemRange,
        signature: &IoMemRange,
    ) -> HsmResult<KeySignContext<Self::Env>>;

    /// End signature operation using a key blob
    ///
    /// # Arguments
    /// * `tag` - Tag ID
    /// * `sign_ctx` - Context for signing operation
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)`
    fn end_signature_with_key_blob(
        &self,
        tag: TagId,
        sign_ctx: KeySignContext<Self::Env>,
    ) -> HsmResult<()>;

    /// Returns whether the session needs to be reestablished e.g. after a live migration
    ///
    /// # Arguments
    ///
    /// * `sess_id` - Virtual session id in the SessionTable
    ///
    /// # Returns
    ///
    /// * `bool` - True if needs to be reestablished, false otherwise
    fn needs_renegotiation(&self, sess_id: u16) -> bool;

    /// Unmask the unwrapping key and import it into the key vault
    ///
    /// # Arguments
    /// * `masked_uk` - The masked unwrapping key
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError
    fn unmask_unwrapping_key_and_import(&self, masked_uk: &[u8]) -> HsmResult<()>;

    /// Generates the Backup Masking Key (BMK) from the provided Backup Key (BK).
    ///
    /// # Arguments
    /// * `bk` - The BK data to be used for masking the SMK
    /// * `smk` - The SMK data to be used for generating the BMK
    /// * `bmk_len` - The length of the generated BMK
    /// * `bmk_out` - The output buffer for the generated BMK
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn generate_bmk_session(
        &self,
        bk: &[u8],
        smk: &[u8],
        bmk_len: &mut usize,
        bmk_out: &mut [u8],
    ) -> HsmResult<()>;

    /// Flush session from the session table
    ///
    /// # Arguments
    /// * `session_id` - The session ID to be flushed
    fn flush_session(&self, session_id: u16);

    /// Toggle current FIPS approved state
    #[cfg(feature = "fips_validation_hooks")]
    fn toggle_fips_approved_state(&self);

    /// Clear BK3 info, including masked bk boot and sealed bk3
    #[cfg(feature = "fips_validation_hooks")]
    fn clear_bk3_info(&self);

    /// Set Test hook to trigger level 2 abort
    ///
    /// # Arguments
    /// * `level2_trigger` - true if the level 2 abort is required to be triggered
    #[cfg(feature = "mcr_test_hooks")]
    fn set_test_hook_to_trigger_level2_abort(&self, level2_trigger: bool);

    /// Get Test hook to trigger level 2 abort
    ///
    /// # Returns
    /// * true if the level 2 abort need to be triggered, false otherwise
    #[cfg(feature = "mcr_test_hooks")]
    fn test_hook_to_trigger_level2_abort(&self) -> bool;

    /// Test hook to override Pin Policy Context
    #[cfg(feature = "mcr_test_hooks")]
    fn override_pin_policy_context(&self, pin_policy_config: DdiTestActionPinPolicyConfig);

    /// Test hook to clear the pin policy
    #[cfg(feature = "mcr_test_hooks")]
    fn clear_pin_policy(&self);

    /// Set or Get the Test hook test action for Cmd Fsm
    ///
    /// # Arguments
    ///
    /// * `test_action` - An Option of TestAction for this partition
    ///
    /// Returns
    /// * Option of Current test action of this parition
    #[cfg(feature = "mcr_test_hooks")]
    fn cmd_fsm_test_action(&self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction>;

    /// Set or Get the Test hook test action for Hsm Fsm
    ///
    /// # Arguments
    ///
    /// * `test_action` - An Option of TestAction for this partition
    ///
    /// Returns
    /// * Option of Current test action of this parition
    #[cfg(feature = "mcr_test_hooks")]
    fn hsm_fsm_test_action(&self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction>;

    /// Injects a hardware failure for the RNG
    ///
    /// # Arguments
    ///
    /// * `rng_hw_self_test_id` - The test id to inject
    #[cfg(feature = "fips_validation_hooks")]
    fn inject_rng_hw_failure(&self, rng_hw_self_test_id: u32);

    /// Set the number of FSMs to skip before triggering Negative PCT Failure
    ///
    /// # Arguments
    ///
    /// * `cnt` - An Option of the count before triggering Negative PCT Failure
    ///
    /// Returns
    /// * Option of current count of this parition
    #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
    fn neg_pct_skip_cnt(&self, cnt: Option<u8>) -> Option<u8>;

    #[cfg(all(feature = "mcr_test_hooks", feature = "mcr_test_hooks_cdma_ecc_err"))]
    fn get_corr_ecc_err_intr_count(&self) -> Option<u32>;

    /// Get CDMA Vault Key Entry
    ///
    /// # Arguments
    ///
    /// * `key_id` - The AesBulk256KeyId of the CDMA Vault Key
    ///
    /// # Returns
    ///
    /// * Returns `Ok(IoMemRange)` if successful else `Err(HsmError)` if failed
    fn get_cdma_vaultkey_entry(&self, key_id: AesBulk256KeyId) -> HsmResult<IoMemRange>;
}

/// Session Triat
pub trait HsmSession: Clone {
    /// Returns the logical session id
    fn id(&self) -> SessionId;

    /// Returns the physical key id
    fn physical_session_id(&self) -> SessionId;

    /// Get the session API revision
    fn api_rev(&self) -> DdiApiRev;

    /// Invalidate the session
    fn invalidate(&mut self);

    /// Check if the session is valid
    ///
    /// # Returns
    ///
    /// * Returns `true` if valid else `false`
    #[allow(unused)]
    fn valid(&self) -> bool;
}

/// HSM User Session
pub(crate) trait HsmUserSession: HsmSession {
    type Env: HsmEnvTrait;

    /// Get the application vault id
    fn app_vault_id(&self) -> AppVaultId;

    /// Get the application id
    fn app_id(&self) -> AppId;

    /// Send IPC message to trigger crash dump
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `cpu_id` - CPU ID
    /// * `crash_type` - Crash type
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    #[cfg(feature = "mcr_test_hooks")]
    fn send_crashdump_request(
        &self,
        tag: TagId,
        cpu_id: SocCpuId,
        crash_type: CrashType,
    ) -> HsmResult<()>;

    /// Send IPC message to trigger stack validation test
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `cpu_id` - Target CPU
    /// * `error_type` - Stack error type
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    #[cfg(feature = "mcr_test_hooks")]
    fn send_stack_validation_request(
        &self,
        tag: TagId,
        cpu_id: SocCpuId,
        error_type: StackErrorType,
    ) -> HsmResult<()>;

    /// Send IPC message to trigger tdisp interrupt
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `info` - Information about this TDISP interrupt
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    #[cfg(feature = "mcr_manual_test_hooks")]
    fn send_tdisp_interrupt_request(&self, tag: TagId, info: TdispInterruptInfo) -> HsmResult<()>;

    /// Open key Zero Copy
    ///
    /// # Arguments
    ///
    /// `tag` - Tag ID
    /// `key_tag` - The key tag of key to open
    /// `key_id` - The ID of key to open, if known
    /// `key_kind` - The kind of key to perform open operation with, if known
    /// `phase` - The current phase of the open key request
    /// `is_unwrapping_key` - Whether the requested key is an unwrapping key.
    /// `ecc_op` - ECC operation context
    /// `pub_key` - The buffer to fill public key data, if available
    ///
    /// # Returns
    ///
    /// * Returns `Ok(OpenKeyData)` containing new open key phase and key data, else `Err(HsmErr)`
    #[allow(clippy::too_many_arguments)]
    fn open_key_zc(
        &mut self,
        tag: TagId,
        key_tag: u16,
        key_id: Option<KeyId>,
        key_kind: Option<EntryKind>,
        phase: OpenKeyPhase,
        is_unwrapping_key: bool,
        ecc_op: &mut Option<EccGenPubKeyCmd<Self::Env>>,
        pub_key: &IoMemRange,
    ) -> HsmResult<OpenKeyData>;

    /// Get key kind for the given key id.
    ///
    /// # Arguments
    ///
    /// `key_id` - The key id whose key kind should be found
    ///
    /// # Returns
    ///
    /// * Returns `Ok(EntryKind)` if successful, else `Err(HsmErr)` if failed
    fn get_key_kind(&self, key_id: KeyId) -> HsmResult<EntryKind>;

    /// Get key length for the given key id.
    ///
    /// # Arguments
    ///
    /// `key_id` - The key id whose key length should be found
    ///
    /// # Returns
    ///
    /// * Returns `Ok(u16)` if successful, else `Err(HsmErr)` if failed
    #[cfg(feature = "fips_validation_hooks")]
    fn get_key_length(&self, key_id: KeyId) -> HsmResult<u16>;

    /// Delete key
    ///
    /// # Arguments
    ///
    /// * `key_id` - Key id
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn delete_key(&self, key_id: KeyId) -> HsmResult<()>;

    /// Generate AES key
    ///
    /// # Arguments
    ///
    /// * `tag` - Key tag
    /// * `kind` - Key kind
    /// * `usage` - Key usage
    ///
    /// # Returns
    ///
    /// * Returns `Ok(AesKey)` if successful else `Err(HsmError)` if failed
    fn aes_gen_key(
        &self,
        tag: Option<u16>,
        kind: AesKeyKind,
        usage: AesKeyUsage,
        availability: KeyAvailability,
    ) -> HsmResult<AesKey>;

    /// AES encrypt/decrypt operation
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `key_in` - Key id or blob
    /// * `input` - Input data
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn aes_enc_dec(&self, tag: TagId, key_in: AesKeyIn, input: &AesEncDecIn) -> HsmResult<()>;

    /// Begin In-place AES key unwrap with padding
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `kek` - The AES unwrapping key
    /// * `inout` - The blob that holds the wrapped payload as input, and will hold the
    ///   unwrapped key if the operation succeeds.
    ///
    /// # Returns
    ///
    /// * Returns HsmResult
    fn begin_aes_key_unwrap(&self, tag: TagId, kek: &[u8], inout: &[u8]) -> HsmResult<()>;

    /// End In-place AES key unwrap with padding
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    ///
    /// # Returns
    ///
    /// * Returns `Ok(Range<usize>)` if successful where the range reflects the start and end
    ///   indexes of the unwrapped key in the `inout` else `Err(HsmError)` if failed
    fn end_aes_key_unwrap(&self, tag: TagId) -> HsmResult<Range<usize>>;

    /// FIPS validation only function to expose in-place SoftAes operation
    ///
    /// # Arguments
    /// * `tag` - Tag id
    /// * `key` - The AES key
    /// * `inout` - The blob that holds the encrypted payload as input, and will hold the
    ///             decrypted payload if the operation succeeds.
    /// * `op` - SoftAes operation
    ///
    /// # Returns
    /// * Returns HsmResult
    #[cfg(feature = "fips_validation_hooks")]
    fn begin_soft_aes(&self, tag: TagId, key: &[u8], inout: &[u8], op: SoftAesOp) -> HsmResult<()>;

    /// End In-place SoftAes operation
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    ///
    /// # Returns
    ///
    /// * Returns `Ok(Range<usize>)` if successful where the range reflects the start and end
    ///   indexes of the unwrapped key in the `inout` else `Err(HsmError)` if failed
    #[cfg(feature = "fips_validation_hooks")]
    fn end_soft_aes(&self, tag: TagId) -> HsmResult<Range<usize>>;

    /// Begin ECC key generation op
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `name` - Key tag
    /// * `curve` - ECC curve
    /// * `usage` - ECC key usage
    /// * `availability` - ECC Key availability
    ///
    /// # Returns
    ///
    /// * Returns `Ok(EccGenKey<Self::Env>)` if successful else `Err(HsmError)` if failed
    fn begin_ecc_gen_key(
        &self,
        tag: TagId,
        key_tag: Option<u16>,
        curve: EccCurve,
        usage: EccKeyUsage,
        availability: KeyAvailability,
    ) -> HsmResult<EccGenKey<Self::Env>>;

    /// End ECC key generation op
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `op` - ECC key generation op
    ///
    /// # Returns
    ///
    /// * Returns `Ok(EccGenKeyOut)` if successful else `Err(HsmError)` if failed
    fn end_ecc_gen_key(&self, tag: TagId, op: EccGenKey<Self::Env>) -> HsmResult<EccGenKeyOut>;

    /// Begin ECC sign op
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `key_in` - Key id or blob
    /// * `digest` - Digest memory
    /// * `digest_algo` - Digest hash algorithm
    /// * `signature` - Signature memory
    ///
    /// # Returns
    ///
    /// * Returns `Ok(EccSign<Self::Env>)` if successful else `Err(HsmError)` if failed
    fn begin_ecc_sign_zc(
        &self,
        tag: TagId,
        key_in: EccKeyIn,
        digest: &IoMemRange,
        digest_algo: DdiHashAlgorithm,
        signature: &IoMemRange,
    ) -> HsmResult<EccSign<Self::Env>>;

    /// End ECC sign op
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `op` - ECC sign op
    ///
    /// # Returns
    ///
    /// * Returns `Ok(usize)` if successful else `Err(HsmError)` if failed
    fn end_ecc_sign_zc(&self, tag: TagId, op: EccSign<Self::Env>) -> HsmResult<()>;

    /// Begin generate ECC Public key from private key
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `key_id` - Key id
    ///
    /// # Returns
    ///
    /// * Returns `Ok(EccGenPubKey<Self::Env>)` if successful else `Err(HsmError)` if failed
    fn begin_ecc_gen_pub_key(
        &self,
        tag: TagId,
        key_id: KeyId,
    ) -> HsmResult<EccGenPubKeyCmd<Self::Env>>;

    /// Continue generate ECC Public key from private key
    ///
    /// # Arguments
    ///
    /// * `op` - Command metadata
    /// * `pub_key` - Memory address to output public key data to
    ///
    /// # Returns
    ///
    /// * Returns `Ok(EccGenPubKeyCmd)` if successful else `Err(HsmError)` if failed
    fn continue_ecc_gen_pub_key_zc(
        &self,
        op: EccGenPubKeyCmd<Self::Env>,
        pub_key: &IoMemRange,
    ) -> HsmResult<EccGenPubKeyCmd<Self::Env>>;

    /// End generate ECC Public key from private key
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `op` - ECC generate public key op
    ///
    /// # Returns
    ///
    /// * Returns `Ok(EccPubKey)` if successful else `Err(HsmError)` if failed
    fn end_ecc_gen_pub_key_zc(&self, op: EccGenPubKeyCmd<Self::Env>) -> HsmResult<()>;

    /// Begin ECDH compute operation with public key validation.
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `key_id` - Key id
    /// * `target_key_type` - Expected key type of key to import
    /// * `pub_key` - The public key in little endian PKA format
    ///
    /// # Returns
    ///
    /// * Returns `Ok(EcdhComputeCmd<Self::Env>)` if successful else `Err(HsmError)` if failed
    fn begin_ecdh_compute_with_pub_key_validation(
        &self,
        tag: TagId,
        key_id: KeyId,
        target_key_type: DdiKeyType,
        pub_key: &IoMemRange,
    ) -> HsmResult<EcdhComputeCmd<Self::Env>>;

    /// Continue an ECDH compute operation with zero copy public key data.
    ///
    /// # Arguments
    ///
    /// * `op` - Command metadata
    /// * `pub_key` - The public key in little endian PKA format
    ///
    /// # Returns
    ///
    /// * Returns `Ok(EcdhComputeCmd)` if successful else `Err(HsmError)` if failed
    fn continue_ecdh_compute_zc(
        &self,
        op: EcdhComputeCmd<Self::Env>,
        pub_key: &IoMemRange,
    ) -> HsmResult<EcdhComputeCmd<Self::Env>>;

    /// End ECDH compute operation.
    ///
    /// # Arguments
    ///
    /// * `op` - ECC generate public key op
    /// * `key_usage` - The usage of the key to import
    /// * `key_tag` - The tag of the key to import
    /// * `key_availabilty` - The availability of the key to import
    ///
    /// # Returns
    ///
    /// * Returns `Ok(EccPubKey)` if successful else `Err(HsmError)` if failed
    fn end_ecdh_compute(
        &self,
        op: EcdhComputeCmd<Self::Env>,
        key_usage: DdiKeyUsage,
        key_tag: Option<u16>,
        key_availabilty: KeyAvailability,
    ) -> HsmResult<KeyId>;

    /// Begin ECC PCT (Pairwise Consistency Test) validation operation
    ///
    /// This validation process ensures that a given ECC key is correctly generated
    /// and operates as expected for its assigned usage (Sign/Verify or ECDH Key Agreement)
    ///
    /// # Validation Process:
    ///
    /// - EccKeyUsage::SignVerify:
    ///   1. A deterministic digest is signed using the private key
    ///   2. The generated signature is verified using the corresponding public key
    ///   3. If the verification succeeds, the key is validated
    ///   4. If verification fails, a system reset/assert is triggered
    ///
    /// - EccKeyUsage::KeyAgreement(ECDH):
    ///   1. The private key performs an ECDH key agreement operation using a static test vector
    ///   2. The resulting shared secret is computed and stored
    ///   3. The ECDH operation is repeated using the opposite static keypair
    ///   4. The two computed shared secrets are compared to verify correctness
    ///   5. If the computed shared secret mismatches, a system reset/assert is triggered
    ///
    /// # Failure Handling:
    ///
    /// - Execution errors (e.g., hardware failures, invalid arguments, or resource issues)
    ///   - Return an appropriate `HsmErr` and halt validation gracefully
    ///
    /// - Verification result failures (e.g., mismatched signature or ECDH shared secret mismatch)
    ///   - Trigger a system reset for security enforcement
    ///
    /// This ensures that the ECC key behaves correctly and is compatible with standard cryptographic operations
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `key_id` - Key id
    /// * `usage` - The expected usage of the key (Sign/Verify or KeyAgreement)
    /// * `public_key` - The public key corresponding to the key being validated
    ///
    /// # Returns
    ///
    /// * Returns `Ok(EccKeyPct<Self::Env>)` if the validation process starts successfully
    /// * Returns `Err(HsmErr)` if validation setup fails due to an execution error
    fn begin_ecc_pct_validation(
        &self,
        tag: TagId,
        key_id: KeyId,
        usage: EccKeyUsage,
        public_key: PkaEccPublicKey,
    ) -> HsmResult<EccKeyPct<Self::Env>>;

    /// Continue ECC PCT validation operation
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `ecc_key_pct` - Reference to the EccKeyPct instance containing the ongoing operation
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful, else `Err(HsmErr)` if failed
    fn continue_ecc_pct_validation(
        &self,
        tag: TagId,
        ecc_key_pct: &mut EccKeyPct<Self::Env>,
    ) -> HsmResult<()>;

    /// End ECC PCT validation operation
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `ecc_key_pct` - Reference to the EccKeyPct instance containing the ongoing operation
    ///
    /// # Returns
    ///
    /// * Returns `Ok(true)` if PCT was completed successfully,
    ///   `Ok(false)` if successfully completed function but PCT failed
    ///   else `Err(HsmErr)` if failed with an error
    fn end_ecc_pct_validation(
        &self,
        tag: TagId,
        ecc_key_pct: &mut EccKeyPct<Self::Env>,
    ) -> HsmResult<bool>;

    /// Checks if the PCT validation state requires final verification
    ///
    /// # Arguments
    ///
    /// * `op` - Reference to the EccKeyPct instance containing the ongoing operation
    ///
    /// # Returns
    ///
    /// * Returns `true` if the validation is in its final state, else `false`
    fn is_pct_final_state(&self, op: &EccKeyPct<Self::Env>) -> bool;

    /// Begin ECC Structural Validation operation
    ///
    /// This ensures an imported ECC key is structurally correct by verifying:
    ///
    ///     Q = d × G
    ///
    /// where `d` is the private scalar and `Q` is the public key. This prevents
    /// malformed or malicious keys from being accepted into the system.
    ///
    /// # Steps:
    /// - Validate curve and scalar range (0 < d < order)
    /// - Compute Q' = d × G using PKA hardware
    /// - Compare Q' with provided public key (x, y)
    ///
    /// # Fails if:
    /// - Curve or scalar is invalid
    /// - Q' ≠ Q (mismatch)
    ///
    /// # Arguments
    /// * `tag` - Operation tag
    /// * `key_id` - Private key ID
    /// * `pub_key_blob` - Public key as concatenated x || y (big-endian)
    ///
    /// # Returns
    /// * `Ok(EccStructuralValidationCmd)` if validation starts
    /// * `Err(HsmErr)` on failure
    /// * Hardware resource is unavailable (`HsmErr::Pending`)
    fn begin_ecc_structural_validation(
        &self,
        tag: TagId,
        key_id: KeyId,
        entry_usage: DdiKeyUsage,
        pub_key_blob: Vec<u8>,
    ) -> HsmResult<EccStructuralValidationCmd<Self::Env>>;

    /// Continue ECC structural validation operation
    ///
    /// # Arguments
    ///
    /// * `op` - The ongoing ECC structural validation operation
    ///
    /// # Returns
    ///
    /// * Returns `Ok(EccStructuralValidationCmd<Self::Env>)` if successful, else `Err(HsmErr)` if failed
    fn continue_ecc_structural_validation(
        &self,
        op: EccStructuralValidationCmd<Self::Env>,
    ) -> HsmResult<EccStructuralValidationCmd<Self::Env>>;

    /// End ECC structural validation operation
    ///
    /// # Arguments
    ///
    /// * `op` - The completed ECC structural validation operation
    ///
    /// # Returns
    ///
    /// * Returns `Ok()` if successful, else `Err(HsmErr)` if failed
    fn end_ecc_structural_validation(
        &mut self,
        op: EccStructuralValidationCmd<Self::Env>,
    ) -> HsmResult<()>;

    /// Begin zero copy RSA modular exponentiation op
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `key_id` - Key id
    /// * `usage` - RSA key usage
    /// * `input` - Input data in little endian
    /// * `output` - Onput data in little endian
    ///
    /// # Returns
    ///
    /// * Returns `Ok(RsaModExp<Self::Env>)` if successful else `Err(HsmError)` if failed
    fn begin_rsa_mod_exp_zc(
        &self,
        tag: TagId,
        key_id: KeyId,
        usage: Option<RsaKeyUsage>,
        input: &IoMemRange,
        output: &IoMemRange,
    ) -> HsmResult<RsaModExp<Self::Env>>;

    /// End RSA modular exponentiation op
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `op` - RSA modular exponentiation op
    ///
    /// # Returns
    ///
    /// * Returns `Ok(RsaModExpOutput)` if successful else `Err(HsmError)` if failed
    fn end_rsa_mod_exp_zc(&self, tag: TagId, op: RsaModExp<Self::Env>) -> HsmResult<()>;

    /// Begin RSA PCT (Pairwise Consistency Test) validation operation
    ///
    /// This validation process ensures that a given RSA key is functioning correctly
    /// according to its assigned usage (`SignVerify`, `EncryptDecrypt`, or `Unwrap`).
    ///
    /// # Validation Process:
    ///
    /// - `RsaKeyUsage::SignVerify`:
    ///   1. A deterministic digest is signed using the private key (modular exponentiation)
    ///   2. The resulting signature is verified using the public key
    ///   3. If the verified result matches the original digest, the key is validated
    ///
    /// - `RsaKeyUsage::EncryptDecrypt` / `RsaKeyUsage::Unwrap`:
    ///   1. A known plaintext is encrypted using the public key
    ///   2. The ciphertext is then decrypted using the private key
    ///   3. If the decrypted result matches the original plaintext, the key is validated
    ///
    /// # Failure Handling:
    ///
    /// - If any operation (signing, encryption, etc.) fails due to hardware or execution errors:
    ///   - Returns an appropriate `HsmErr` and stops validation gracefully
    ///
    /// - If the output of verify/decrypt does not match the original input:
    ///   - Considered a validation failure (to be handled by higher-level reset/assert logic)
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID used for engine tracking
    /// * `key_id` - The ID of the RSA private key to validate
    /// * `usage` - Intended usage of the RSA key (e.g., SignVerify, EncryptDecrypt, Unwrap)
    /// * `rsa_type` - Size of the RSA key (e.g., Rsa2k, Rsa3k)
    /// * `n` - RSA modulus, expected to be `rsa_type.len()` bytes, little-endian
    /// * `e` - RSA public exponent, expected to be 4 bytes, little-endian
    ///
    /// # Returns
    ///
    /// * `Ok(RsaPctValidationCmd<Self::Env>)` — if validation begins successfully
    /// * `Err(HsmErr)` — on setup or operational failure
    fn begin_rsa_pct_validation(
        &self,
        tag: TagId,
        key_id: KeyId,
        usage: RsaKeyUsage,
        rsa_type: PkaRsaSize,
        n: &mut [u8],
        e: &[u8],
    ) -> HsmResult<RsaPctValidationCmd<Self::Env>>;

    /// Continue RSA PCT validation operation
    ///
    /// # Arguments
    ///
    /// * `op` - The ongoing PCT validation operation
    ///
    /// # Returns
    ///
    /// * Returns `Ok(RsaPctValidationCmd<Self::Env>)` if successful, else `Err(HsmErr)` if failed
    fn continue_rsa_pct_validation(
        &self,
        op: RsaPctValidationCmd<Self::Env>,
    ) -> HsmResult<RsaPctValidationCmd<Self::Env>>;

    /// End RSA PCT validation operation
    ///
    /// # Arguments
    ///
    /// * `op` - The completed PCT validation operation
    ///
    /// # Returns
    ///
    /// * Returns `Ok(bool)` if successful, else `Err(HsmErr)` if failed
    fn end_rsa_pct_validation(&self, op: RsaPctValidationCmd<Self::Env>) -> HsmResult<bool>;

    /// Checks if the PCT validation state requires final verification
    ///
    /// # Arguments
    ///
    /// * `pct_op` - Reference to the ongoing PCT validation operation
    ///
    /// # Returns
    ///
    /// * Returns `true` if the validation is in its final state, else `false`
    fn is_rsa_pct_final_state(&self, pct_op: &RsaPctValidationCmd<Self::Env>) -> bool;

    /// Begin RSA unwrap, zero copy
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `key_id` - Key id
    /// * `data` - Input data in big endian
    /// * `usage` - RSA key usage
    ///
    /// # Returns
    ///
    /// * Returns `Ok(RsaModExp<Self::Env>)` if successful else `Err(HsmError)` if failed
    fn begin_rsa_unwrap_mod_exp_zc(
        &self,
        tag: TagId,
        key_id: KeyId,
        input: &IoMemRange,
        output: &IoMemRange,
        usage: Option<RsaKeyUsage>,
    ) -> HsmResult<RsaModExp<Self::Env>>;

    /// End RSA unwrap, zero copy
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag id
    /// * `op` - RSA modular exponentiation op
    ///
    /// # Returns
    ///
    /// * Returns `Ok()` if successful else `Err(HsmError)` if failed
    fn end_rsa_unwrap_mod_exp_zc(&self, tag: TagId, op: RsaModExp<Self::Env>) -> HsmResult<()>;

    /// Decode OAEP KEK (Key Encryption Key) using RSA unwrapping
    /// /// # Arguments
    /// ///
    /// * `unwrapped_data` - The unwrapped data to decode
    /// * `padding` - The padding scheme used for the OAEP encoding
    /// * `hash_alg` - The hash algorithm used in the OAEP encoding
    /// # Returns
    /// ///
    /// * Returns `Ok(Vec<u8>)` if successful, containing the decoded KEK,
    ///   else `Err(HsmError)` if the decoding fails.
    fn decode_oaep_kek(
        &self,
        unwrapped_data: &[u8],
        padding: DdiRsaCryptoPadding,
        hash_alg: DdiHashAlgorithm,
    ) -> HsmResult<SecureByteVec>;

    /// Get random number using RNG
    ///
    /// # Arguments
    ///
    /// * `rng_len` - Random number length in Bytes
    /// * `rng_number` - Output Random number value
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    #[cfg(feature = "fips_validation_hooks")]
    fn get_random_number(&self, rng_number: &mut IoMemRange) -> HsmResult<()>;

    /// Compute SHA for a full single block, zero copy
    ///
    /// # Arguments
    ///
    /// * `mode` - SHA Mode
    /// * `buf` - The input buffer to perform the SHA operation on.
    /// * `output_buffer` - Output buffer, needs to be sized based on get_digest_size_hw
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn sha_single_block_zc(
        &self,
        mode: ShaType,
        buffer: &IoMemRange,
        output_buffer: &mut IoMemRange,
    ) -> HsmResult<()>;

    /// Import a RAW key into the HSM (only Private key).
    ///
    /// # Arguments
    ///
    /// * `key_kind` - The kind of key to import
    /// * `key_properties` - Properties of the key to be imported
    /// * `entry_name` - The name of the key to import
    /// * `raw_key` - The raw key bytes to be imported
    ///
    /// # Returns
    ///
    /// * Returns `Ok(KeyId)` if successful else `Err(HsmError)` if failed
    ///
    /// # Notes
    ///
    /// * While importing the unwrapping key, this function will delete the existing unwrapping
    ///   key and add the newly requested key
    #[cfg(feature = "fips_validation_hooks")]
    fn import_raw_key(
        &self,
        key_type: DdiKeyType,
        key_properties: DdiKeyProperties,
        key_tag: Option<u16>,
        raw_key: &[u8],
    ) -> HsmResult<KeyId>;

    /// Import a DER key into the HSM (except RSA CRT type keys).
    /// Note: RSA CRT DER key import follow a different approach (and
    /// hence a separate API) as there are multiple async steps
    /// involved in generating the CRT parameters and then storing
    /// the key in the key vault.
    ///
    /// # Arguments
    ///
    /// * `entry_class` - The class of key to import
    /// * `entry_usage` - The usage of the key to import
    /// * `entry_tag` - The tag of the key to import
    /// * `entry_availability` - The availability of the key to import
    /// * `der` - The DER encoded key to import
    ///
    /// # Returns
    ///
    /// * Returns `Ok(ImportDerKeyResult)` if successful else `Err(HsmError)` if failed
    fn import_der_key(
        &self,
        entry_class: EntryClass,
        entry_usage: DdiKeyUsage,
        entry_tag: Option<u16>,
        entry_availability: KeyAvailability,
        der: &[u8],
    ) -> HsmResult<ImportDerKeyResult>;

    /// Begin importing a DER key into the HSM.
    ///
    /// # Arguments
    ///
    /// * `der` - The DER encoded key to import
    /// * `tag` - The user tag.
    ///
    /// # Returns
    ///
    /// * Returns `Ok(RsaOp, public key data)` if successful else `Err(HsmError)` if failed
    fn begin_import_der_crt_key(
        &self,
        tag: TagId,
        der: &[u8],
    ) -> HsmResult<(RsaCrtParamComputeCmd<Self::Env>, Vec<u8>)>;

    /// Continue importing a DER key into the HSM.
    ///
    /// # Arguments
    ///
    /// * `op` - Operational data.
    ///
    /// # Returns
    ///
    /// * Returns `Ok(RsaOp)` if successful else `Err(HsmError)` if failed
    fn continue_import_der_crt_key(
        &self,
        op: RsaCrtParamComputeCmd<Self::Env>,
    ) -> HsmResult<RsaCrtParamComputeCmd<Self::Env>>;

    /// End importing a DER key into the HSM.
    ///
    /// # Arguments
    ///
    /// * `op` - Operational data.
    /// * `entry_kind` - The kind of key to import
    /// * `key_usage` - The usage of the key to import
    /// * `key_tag` - The key tag
    /// * `key_availability` - Key availability
    ///
    /// # Returns
    ///
    /// * Returns `Ok(key_id)` if successful else `Err(HsmError)` if failed
    fn end_import_der_crt_key(
        &self,
        op: RsaCrtParamComputeCmd<Self::Env>,
        key_usage: DdiKeyUsage,
        key_tag: Option<u16>,
        key_availabilty: KeyAvailability,
    ) -> HsmResult<(KeyId, DdiKeyType)>;

    /// Begin the process to get unwrapping key
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `key_id` - The key id of the current unwrapping key
    /// * `pfn - Pcie Function this unwrapping key belongs to
    ///
    /// # Returns
    ///
    /// * `Ok(GetUnwrappingKeyCtx)` if successful else `Err(HsmError)` if failed
    fn begin_get_unwrapping_key(
        &self,
        tag: TagId,
        key_id: Option<KeyId>,
        pfn: PcieFunction,
    ) -> HsmResult<GetUnwrappingKeyCtx<Self::Env>>;

    /// End the process to get unwrapping key, this is an optional step, if the unwrapping is
    /// already available in HSM key vault.
    ///
    /// # Arguments
    ///
    /// * `&mut ctx` - The context for the get unwrapping key operation
    ///
    /// # Returns
    ///
    /// * Returns `Ok(GetUnwrappingKeyOut)` if successful else `Err(HsmError)` if failed
    fn end_get_unwrapping_key(
        &self,
        ctx: &GetUnwrappingKeyCtx<Self::Env>,
    ) -> HsmResult<GetUnwrappingKeyOut>;

    /// Begin computing CRT parameters (n1q, n2p) for the RSA CRT private key.
    ///
    /// # Arguments
    ///
    /// * `rsa_crt_priv_key` - Reference to the RsaPrivKeyCrt
    ///
    /// # Returns
    /// * The updated operational data for computing the CRT parameters, error code otherwise.
    fn begin_compute_rsa_crt_params(
        &self,
        tag: TagId,
        priv_key_crt: RsaPrivKeyCrt,
    ) -> HsmResult<RsaCrtParamComputeCmd<Self::Env>>;

    /// Continue computing CRT parameters (n1q, n2p) for the RSA CRT private key.
    ///
    /// # Arguments
    ///
    /// * `op` - The operational data for computing the CRT parameters.
    ///
    /// # Returns
    /// * The updated operational data for computing the CRT parameters, error code otherwise.
    fn continue_compute_rsa_crt_params(
        &self,
        tag: TagId,
        op: RsaCrtParamComputeCmd<Self::Env>,
    ) -> HsmResult<RsaCrtParamComputeCmd<Self::Env>>;

    /// End computing CRT parameters (n1q, n2p) for the RSA CRT private key.
    ///
    /// # Arguments
    /// * `op` - Operational command data.
    ///
    /// * `cmd` - The operational data for computing the CRT parameters.
    ///
    /// # Returns
    /// * RSA CRT Private key if successful, error code otherwise.
    fn end_compute_rsa_crt_params(
        &self,
        op: RsaCrtParamComputeCmd<Self::Env>,
    ) -> HsmResult<RsaPrivKeyCrt>;

    /// Compute the HMAC for a given message and secret key.
    /// HMAC is calculated as per the standard at: https://www.rfc-editor.org/rfc/rfc2104.
    ///
    /// # Arguments
    /// * `key_id` - Secret key ID
    /// * `msg` - Message buffer
    /// * `output_buffer` - Output buffer memory range
    ///
    /// # Returns
    /// * Computed HMAC if the operation is successful, error code otherwise.
    fn hmac(&self, key_id: KeyId, msg: &[u8], output_buffer: &mut IoMemRange) -> HsmResult<()>;

    /// Compute the variable length HMAC for a given message and secret key.
    /// HMAC is calculated as per the standard at: https://www.rfc-editor.org/rfc/rfc2104.
    ///
    /// # Arguments
    /// * `key_id` - Secret key ID
    /// * `msg` - Message buffer
    /// * `output_buffer` - Output buffer memory range
    ///
    /// # Returns
    /// * Computed HMAC if the operation is successful, error code otherwise.
    fn var_hmac(&self, key_id: KeyId, msg: &[u8], output_buffer: &mut IoMemRange) -> HsmResult<()>;

    /// Complete HKDF for given key
    ///
    /// # Arguments
    /// * `key_id` - Secret key ID
    /// * `salt` - Optional salt value (non-secret random value)
    /// * `info` - Optional context and application specific information
    /// * `hash_algo` - The hash algorithm to use
    /// * `key_type` - Target key type for derived data
    /// * `key_properties` - Target key properties
    /// * `key_tag` - Target key tag (optional)
    /// * `key_len` - Target key length in bytes (optional)
    ///
    /// # Returns
    /// * Key id of derived data if successful, error code otherwise.
    #[allow(clippy::too_many_arguments)]
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
    ) -> HsmResult<KeyId>;

    /// Complete KBKDF using Counter mode and HMAC PRF
    ///
    /// # Arguments
    /// * `key_id` - Secret key ID
    /// * `label` - Optional byte array defined in spec
    /// * `context` - Optional byte array defined in spec
    /// * `hash_algo` - The hash algorithm to use
    /// * `key_type` - Target key type for derived data
    /// * `key_properties` - Target key properties
    /// * `key_tag` - Target key tag (optional)
    /// * `key_len` - Target key length in bytes (optional)
    ///
    /// # Returns
    /// * Key id of derived data if successful, error code otherwise.
    #[allow(clippy::too_many_arguments)]
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
    ) -> HsmResult<KeyId>;

    /// Complete HKDF for AES Bulk 256 key
    ///
    /// # Arguments
    /// * `tag` - Tag ID
    /// * `pfn` - Pcie Function
    /// * `salt` - Optional salt value (non-secret random value)
    /// * `info` - Optional context and application specific information
    /// * `kdf_info` - Info used for KDF operations
    ///
    /// # Returns
    /// * AesBulk256Cmd if successful, error code otherwise.
    fn begin_hkdf_aesbulk256_derive(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        salt: &[u8],
        info: &[u8],
        kdf_info: KdfInfo,
    ) -> HsmResult<AesBulk256Cmd<Self::Env>>;

    /// Complete KBKDF using Counter mode and HMAC PRF for AES Bulk 256 key
    ///
    /// # Arguments
    /// * `tag` - Tag ID
    /// * `pfn` - Pcie Function
    /// * `label` - Optional byte array defined in spec
    /// * `context` - Optional byte array defined in spec
    /// * `kdf_info` - Info used for KDF operations
    ///
    /// # Returns
    /// * AesBulk256Cmd if successful, error code otherwise.
    fn begin_kbkdf_aesbulk256_derive(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        label: &[u8],
        context: &[u8],
        kdf_info: KdfInfo,
    ) -> HsmResult<AesBulk256Cmd<Self::Env>>;

    /// End KDF AES Bulk 256 key derivation
    ///
    /// # Arguments
    /// * `op` - AesBulk256Cmd Operational data.
    ///
    /// # Returns
    /// * Returns `Ok(())` if successful, error otherwise.
    fn end_kdf_aesbulk256_derive(&self, op: &AesBulk256Cmd<Self::Env>) -> HsmResult<()>;

    /// Aes bulk 256 key import. First store the raw key in CDMA vault managed
    /// by HSM(Memory mapped by HSM but different from GSRAM - via add_entry()).
    /// This will return a 2 byte 'key id' like any other key import.
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `pfn` - Used to in IPC message
    /// * `entry_usage` - The usage of the key to import
    /// * `entry_tag` - The tag of the key to import
    /// * `key_type` - The type of key to import
    /// * `entry_availability` - The availability of the key to import
    /// * `der` - The DER encoded key to import
    /// # Returns
    ///
    /// * Returns `Ok(AesBulk256Cmd)` if successful else `Err(HsmError)` if failed
    #[allow(clippy::too_many_arguments)]
    fn begin_import_der_aesbulk256_key(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        entry_usage: DdiKeyUsage,
        entry_tag: Option<u16>,
        key_type: DdiKeyType,
        entry_availability: KeyAvailability,
        der: &[u8],
    ) -> HsmResult<AesBulk256Cmd<Self::Env>>;

    /// Import an AES bulk 256 key during unmask, preserving original attributes.
    ///
    /// Unlike `begin_import_der_aesbulk256_key` which reconstructs attributes
    /// from scratch, this method uses the original `EntryAttributes` stored in
    /// the masked key metadata.
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `pfn` - Used in IPC message
    /// * `entry_usage` - The usage of the key to import
    /// * `entry_tag` - The tag of the key to import
    /// * `key_type` - The type of key to import
    /// * `original_attributes` - The original key attributes from masked key metadata
    /// * `der` - The DER encoded key to import
    ///
    /// # Returns
    ///
    /// * Returns `Ok(AesBulk256Cmd)` if successful else `Err(HsmError)` if failed
    #[allow(clippy::too_many_arguments)]
    fn unmask_import_der_aesbulk256_key(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        entry_usage: DdiKeyUsage,
        entry_tag: Option<u16>,
        key_type: DdiKeyType,
        original_attributes: &EntryAttributes,
        der: &[u8],
    ) -> HsmResult<AesBulk256Cmd<Self::Env>>;

    /// End importing a DER key by processing IPC.
    ///
    /// # Arguments
    ///
    /// * `op` - Operational data.
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn end_import_der_aesbulk256_key(&self, op: &AesBulk256Cmd<Self::Env>) -> HsmResult<()>;

    /// Aes bulk 256 key delete. Send IPC to FP to delete the key metadata.
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `pfn` - Used to in IPC message
    /// * `key_id` - The HSM vault's key id to be deleted
    ///
    /// # Returns
    ///
    /// * Returns `Ok(AesBulk256Cmd)` if successful else `Err(HsmError)` if failed
    fn begin_delete_aesbulk256_key(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        key_id: KeyId,
    ) -> HsmResult<AesBulk256Cmd<Self::Env>>;

    /// Up on success from FP IPC, deletes HSM/CDMA vault key entries.
    ///
    /// # Arguments
    ///
    /// * `op` - Operational data.
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn end_delete_aesbulk256_key(&self, op: &AesBulk256Cmd<Self::Env>) -> HsmResult<()>;

    /// Aes bulk 256 key generate. Generate AES Bulk 256 32 byte key and imports
    /// in to CDMA vault and to HSM Vault and fires IPC to FP. This operation is
    /// basically key generate + bulk import
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `pfn` - Used to in IPC message
    /// * `key_tag` - Key tag
    /// * `key_type` - Key type
    /// * `availability` - Aes bulk 256 Key availability
    /// # Returns
    ///
    /// * Returns `Ok(AesBulk256Cmd)` if successful else `Err(HsmError)` if
    ///   failed
    fn begin_aesbulk256_gen_key(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        key_tag: Option<u16>,
        key_type: DdiKeyType,
        availability: KeyAvailability,
    ) -> HsmResult<AesBulk256Cmd<Self::Env>>;

    /// End generate operation by processing IPC. If failed will delete from
    /// HSM/CDMA vaults
    ///
    /// # Arguments
    ///
    /// * `op` - Operational data.
    ///
    /// # Returns
    ///
    /// * Returns `Ok(AesBulk256Cmd)` if successful else `Err(HsmError)` if
    ///   failed
    fn end_aesbulk256_gen_key(&self, op: &AesBulk256Cmd<Self::Env>) -> HsmResult<()>;

    /// Begin rollback of AES bulk 256 key
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `pfn` - Used to in IPC message
    /// * `op` - Operational data.
    ///
    /// # Returns
    ///
    /// * Returns `Ok(AesBulk256Cmd)` if successful else `Err(HsmError)` if failed
    fn begin_rollback_aesbulk256_key(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        op: &AesBulk256Cmd<Self::Env>,
    ) -> HsmResult<()>;

    /// End rollback of AES bulk 256 key
    ///
    /// # Arguments
    ///
    /// * `op` - Operational data.
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn end_rollback_aesbulk256_key(&self, op: &AesBulk256Cmd<Self::Env>) -> HsmResult<()>;

    /// Begin change pin
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    ///
    /// # Returns
    ///
    /// * Returns `Ok(ChangePinCmdCtx)` if successful else `Err(HsmError)` if failed
    fn begin_change_pin(&self, tag: TagId) -> HsmResult<ChangePinCmdCtx<Self::Env>>;

    /// Continue change pin
    ///
    /// # Arguments
    ///
    /// * `ctx` - ChangePinCmdCtx as returned by begin_change_pin
    /// * `pub_key` - ECC 384 Public Key received from client
    ///
    /// # Returns
    ///
    /// * Returns `Ok(ChangePinCmdCtx)` if successful else `Err(HsmError)` if failed
    fn continue_change_pin(
        &self,
        ctx: ChangePinCmdCtx<Self::Env>,
        pub_key: &IoMemRange,
    ) -> HsmResult<ChangePinCmdCtx<Self::Env>>;

    /// End change pin
    ///
    /// # Arguments
    ///
    /// * `ctx` - ChangePinCmdCtx as returned by continue_change_pin
    /// * `encrypted_pin` - Encrypted Pin as received from client
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn end_change_pin(
        &self,
        ctx: ChangePinCmdCtx<Self::Env>,
        encrypted_pin: &DdiEncryptedPin,
    ) -> HsmResult<()>;

    /// Notify PCT validation failure to HSP
    ///
    /// # Arguments
    /// * `err` - Error code indicating the failure reason
    fn notify_pct_validation_failure(&self, err: u32);

    /// Set or Get the Test hook test action for command FSM
    ///
    /// # Arguments
    ///
    /// * `test_action` - An Option of TestAction for this partition
    ///
    /// Returns
    /// * Option of Current test action of this parition
    #[cfg(feature = "mcr_test_hooks")]
    fn cmd_fsm_test_action(&self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction>;

    /// Set or Get the Test hook test action for HSM FSM
    ///
    /// # Arguments
    ///
    /// * `test_action` - An Option of TestAction for this partition
    ///
    /// Returns
    /// * Option of Current test action of this parition
    #[cfg(feature = "mcr_test_hooks")]
    fn hsm_fsm_test_action(&self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction>;

    /// Get the private key
    ///
    /// This function retrieves the private key associated with the given key ID.
    ///
    /// # Arguments
    ///
    /// * `key_id` - The ID of the key to retrieve.
    /// * `key_data` - The memory range where the private key data will be stored.
    ///
    /// # Returns
    ///
    /// * Returns `Ok(PrivateKeyData)` containing the private key data, else `Err(HsmErr)`.
    #[cfg(feature = "fips_validation_hooks")]
    fn get_priv_key(&mut self, key_id: KeyId, key_data: &mut IoMemRange) -> HsmResult<()>;

    /// Send Negative Self Test IPC Request
    ///
    /// # Arguments
    ///
    /// * `neg_self_test` - The neg self test Id
    /// * `tag` - Tag ID
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    #[cfg(feature = "mcr_test_hooks")]
    fn begin_neg_self_test_req(&self, neg_self_test: SelfTest, tag: TagId) -> HsmResult<()>;

    /// Receieve Negative Self Test IPC response
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    #[cfg(any(feature = "mcr_test_hooks", feature = "mcr_manual_test_hooks"))]
    fn end_neg_self_test_resp(&self, tag: TagId) -> HsmResult<()>;

    /// Force the PKA instance during engine allocation
    ///
    /// # Arguments
    ///
    /// * `Option<usize>` - Some(PKA instance) to be forced, None to use the default allocation.
    #[cfg(feature = "fips_validation_hooks")]
    fn force_pka_instance(&self, pka_instance: Option<usize>);

    /// Set the number of FSMs to skip before triggering Negative PCT Failure
    ///
    /// # Arguments
    ///
    /// * `cnt` - An Option of the count before triggering Negative PCT Failure
    ///
    /// Returns
    /// * Option of current count of this parition
    #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
    fn neg_pct_skip_cnt(&self, cnt: Option<u8>) -> Option<u8>;

    /// Get the length of the masked key based on key label, key id and optionally public key data
    ///
    /// This API will get the key info from vault based on the key id
    ///
    /// # Arguments
    ///
    /// * `key_label` - The key label
    /// * `key_id` - The key id of the key to be masked
    /// * `pub_data` - Potential public data that will be masked together with private data
    ///
    /// Returns
    /// * Returns `Ok(usize)` if successful else `Err(HsmError)` if failed
    fn get_masked_key_len_from_vault(
        &self,
        key_label: &[u8],
        key_id: KeyId,
        pub_data: Option<&[u8]>,
    ) -> HsmResult<usize>;

    /// Get the length of the masked key based on the length of metadata and length of the encrypted key
    ///
    /// # Arguments
    ///
    /// * `metadata_len` - The length of the metadata.
    /// * `encrypted_key_len` - The length of encrypted key
    ///
    /// Returns
    /// * Returns `Ok(usize)` if successful else `Err(HsmError)` if failed
    fn get_masked_key_len(&self, metadata_len: usize, encrypted_key_len: usize)
        -> HsmResult<usize>;

    /// Mask a key based on the key label, key id and optionally public key data
    ///
    /// This API will get the key info from vault based on the key id
    ///
    /// # Arguments
    ///
    /// * `key_label` - The key label
    /// * `key_id` - The key id of the key to be masked
    /// * `pub_data` - Potential public data that will be masked together with private data
    /// * `masked_key` - Output blob for masked key result
    ///
    /// Returns
    /// * Returns `Ok()` if successful else `Err(HsmError)` if failed
    fn mask_key_from_vault(
        &self,
        key_label: &[u8],
        key_id: KeyId,
        pub_data: Option<&[u8]>,
        masked_key: &mut [u8],
    ) -> HsmResult<()>;

    /// Mask a key based on the metadata, key length and input padded buffer
    ///
    /// # Arguments
    ///
    /// * `metadata` - Metadata contains information to properly layout the masked key
    /// * `masking_key` - Masking key for mask operation
    /// * `padded_key_buffer` - The buffer that contains the key to be masked, padded according to the algo
    /// * `masked_key` - Output blob for masked key result
    ///
    /// Returns
    /// * Returns `Ok()` if successful else `Err(HsmError)` if failed
    fn mask_key(
        &self,
        metadata: &[u8],
        masking_key: &[u8],
        padded_key_buffer: &[u8],
        masked_key: &mut [u8],
    ) -> HsmResult<()>;

    /// Unmask a masked key
    ///
    /// # Arguments
    ///
    /// * `masked_key` - The masked key to be unmasked
    ///
    /// Returns
    /// * Returns `Ok(UnmaskedKeyRawResult<Self::Env>)` if successful else `Err(HsmError)` if failed
    fn unmask_key(&self, masked_key: &[u8]) -> HsmResult<UnmaskedKeyRawResult<Self::Env>>;

    /// Unmask a masked key and try to import it into key vault
    ///
    /// # Arguments
    ///
    /// * `masked_key` - The masked key to be unmasked
    ///
    /// Returns
    /// * If the key is successfully imported into the key value, it will return Ok((Some(KeyId), None))
    /// * If the key can't be imported synchronously, it will return Ok((None, Some((DdiMaskedKeyMetadata, DmaBuffer<Self::Env>))))
    /// * Otherwise `Err(HsmError)`
    fn unmask_key_and_import(&self, masked_key: &[u8]) -> HsmResult<UnmaskedKeyResult<Self::Env>>;
}
