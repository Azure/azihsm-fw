// Copyright (c) Microsoft Corporation. All rights reserved.

use core::ops::Range;

use crate::error;
use mcr_crypto_aes::*;
use mcr_ddi_types::DdiAesOp;
use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_logging::*;
use mcr_simplex::*;
use mcr_types::AesBulk256KeyId;
use mcr_types::PcieFunction;
use mcr_types::SecureByteArray;

use super::*;

#[derive(Copy, Clone)]
pub(crate) enum AesEncDecMode {
    #[allow(unused)]
    Ecb,
    Cbc,
}

impl From<AesEncDecMode> for AesMode {
    fn from(mode: AesEncDecMode) -> Self {
        match mode {
            AesEncDecMode::Cbc => Self::Cbc,
            AesEncDecMode::Ecb => Self::Ecb,
        }
    }
}

#[derive(Copy, Clone)]
pub(crate) enum AesEncDecOp {
    Encrypt,
    Decrypt,
}

impl TryFrom<DdiAesOp> for AesEncDecOp {
    type Error = HsmErr;

    fn try_from(op: DdiAesOp) -> Result<Self, Self::Error> {
        match op {
            DdiAesOp::Encrypt => Ok(Self::Encrypt),
            DdiAesOp::Decrypt => Ok(Self::Decrypt),
            _ => Err(HsmErr::InvalidArgument),
        }
    }
}

impl From<AesEncDecOp> for AesOp {
    fn from(op: AesEncDecOp) -> Self {
        match op {
            AesEncDecOp::Encrypt => Self::Encrypt,
            AesEncDecOp::Decrypt => Self::Decrypt,
        }
    }
}

pub(crate) enum AesKeyIn<'a> {
    KeyId(KeyId),
    #[allow(unused)]
    KeyBlob(&'a [u8]),
}

pub(crate) struct AesEncDecIn<'a> {
    mode: AesEncDecMode,
    op: AesEncDecOp,
    iv: Option<&'a IoMemRange>,
    msg_in: &'a IoMemRange,
    msg_out: &'a IoMemRange,
}

impl<'a> AesEncDecIn<'a> {
    pub fn new(
        mode: AesEncDecMode,
        op: AesEncDecOp,
        iv: Option<&'a IoMemRange>,
        msg_in: &'a IoMemRange,
        msg_out: &'a IoMemRange,
    ) -> Self {
        Self {
            mode,
            op,
            iv,
            msg_in,
            msg_out,
        }
    }

    pub fn mode(&self) -> AesEncDecMode {
        self.mode
    }

    pub fn op(&self) -> AesEncDecOp {
        self.op
    }

    pub fn iv(&self) -> Option<&IoMemRange> {
        self.iv
    }

    pub fn msg_in(&self) -> &IoMemRange {
        self.msg_in
    }

    pub fn msg_out(&self) -> &IoMemRange {
        self.msg_out
    }
}

/// AES Bulk 256 command data.
pub(crate) enum AesBulk256Cmd<E: HsmEnvTrait + 'static> {
    CloseAppSession(SessionId, FpIpcChannelRef<E>),
    DeleteKey(AesBulk256KeyId, KeyId, FpIpcChannelRef<E>),
    DerKeyImport(AesBulk256KeyId, KeyId, FpIpcChannelRef<E>),
}

impl<E: HsmEnvTrait> UserSession<E> {
    /// AES Initialization Vector size
    const AES_IV_SIZE: usize = 16;

    /// AES block size
    const AES_BLOCK_SIZE: usize = 16;

    // Generate a random AES key
    pub(super) fn aes_gen_key_inner(
        &self,
        kind: AesKeyKind,
        usage: AesKeyUsage,
        tag: Option<u16>,
        availability: KeyAvailability,
    ) -> HsmResult<AesKey> {
        self.state.vault().validate_key_params(availability, tag)?;

        // Allocate buffer of max length on the stack
        let mut buf =
            SecureByteArray::<{ AesKeyKind::max_len() }>::new([0u8; AesKeyKind::max_len()]);

        // Adjust the length of the buffer to the key kind
        let key = &mut buf.as_mut_slice()[..kind.len()];

        // Generate a random key
        self.state.env().rng().bytes(key);

        // Create a key to import
        let aes_key = AesKeyImported::new(kind, usage, key)?;

        // Import the key into the vault
        let attributes = aes_entry_attributes(availability, true, usage);
        self.state.vault().aes_import_key(
            self.app_vault_id(),
            self.id(),
            tag,
            &aes_key,
            &attributes,
        )
    }

    /// AES encrypt/decrypt operation
    pub(super) fn aes_enc_dec_inner(
        &self,
        tag: TagId,
        key_in: AesKeyIn,
        input: &AesEncDecIn,
    ) -> HsmResult<()> {
        // Variable bindings for `key` and `key_blob` references.
        let key;
        let key_in_vault;

        // Determine the key is in the vault or memory.
        let (_key, _key_id, key_blob) = match key_in {
            AesKeyIn::KeyId(key_id) => {
                key = self.aes_key_inner(key_id)?;
                key_in_vault = key.blob()?;
                (Some(key.clone()), Some(key_id), &key_in_vault as &[u8])
            }
            AesKeyIn::KeyBlob(blob) => (None, None, blob),
        };

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

        if plaintext.len() % Self::AES_BLOCK_SIZE != 0 {
            Err(HsmErr::InvalidArgument)?
        }

        if input.msg_in().len() != input.msg_out().len() {
            Err(HsmErr::InvalidArgument)?
        }

        // Construct the AES Command
        let cmd = AesCommand {
            tag,
            message: input.msg_in,
            iv: input.iv,
            key: key_blob,
            update_iv: true,
            op: input.op().into(),
            mode: input.mode().into(),
            result: input.msg_out,
        };

        // Submit the AES command the engine
        self.aes_engine_cmd_inner(&cmd)
    }

    pub(super) fn begin_soft_aes_inner(
        &self,
        tag: TagId,
        key: &[u8],
        inout: &[u8],
        op: SoftAesOp,
    ) -> HsmResult<()> {
        let msg = SoftAesOffloadReq {
            key: key.into(),
            inout: inout.into(),
            tag,
            op,
        };

        match self.state.env().soft_aes_req().send(msg) {
            Ok(()) => Err(HsmErr::Pending),
            _ => Err(HsmErr::SoftAesReqSendFailed),
        }
    }

    pub(super) fn end_soft_aes_inner(&self, tag: TagId) -> HsmResult<Range<usize>> {
        let resp: SoftAesOffloadResp = self
            .state
            .env()
            .soft_aes_resp()
            .recv()
            .ok_or(HsmErr::SpuriousIpcMessageEvent)?;

        if resp.tag != tag {
            Err(HsmErr::IoTagMismatch)?
        }

        let range = resp.range.map_err(|_| HsmErr::RsaUnwrapAesUnwrapFailed)?;

        Ok(Range {
            start: range.0,
            end: range.1,
        })
    }

    fn aes_key_inner(&self, key_id: KeyId) -> HsmResult<AesKey> {
        let key = self.state.vault().aes_key(
            self.app_vault_id(),
            self.id(),
            key_id,
            AesKeyUsage::EncryptDecrypt,
        )?;

        if key.disabled()? {
            Err(HsmErr::KeyNotFound)?
        }

        Ok(key)
    }

    fn aes_engine_cmd_inner(&self, cmd: &AesCommand<'_>) -> Result<(), HsmErr> {
        let err = if cmd.op == AesOp::Decrypt {
            HsmErr::AesDecryptFailed
        } else {
            HsmErr::AesEncryptFailed
        };

        self.state
            .env()
            .aes()
            .encrypt_decrypt(cmd)
            .map_err(|_| err)?;

        Ok(())
    }

    // Get the AES Bulk 256 key type from DDI key type to send to FP
    fn aes_bulk_key_type(&self, key_type: DdiKeyType) -> HsmResult<AesBulkKeyType> {
        match key_type {
            DdiKeyType::AesXtsBulk256 => Ok(AesBulkKeyType::Xts),
            DdiKeyType::AesGcmBulk256 => Ok(AesBulkKeyType::Gcm),
            DdiKeyType::AesGcmBulk256Unapproved => Ok(AesBulkKeyType::GcmUnapproved),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }

    /// Import der key of type AES Bulk 256.
    #[allow(clippy::too_many_arguments)]
    pub(super) fn begin_import_der_aesbulk256_key_inner(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        entry_tag: Option<u16>,
        key_type: DdiKeyType,
        key_usage: AesKeyUsage,
        attributes: &EntryAttributes,
        der: &[u8],
    ) -> HsmResult<AesBulk256Cmd<E>> {
        // Steps to handle AesBulk256 key import
        // 0. Acquire the CP/FP IPC channel
        // 1. Store the raw key(32 bytes) in CDMA vault(Managed by HSM). This
        //    will return a 2 byte 'key id'. This key id encodes the table id
        //    and key slot index of CDMA vault to which the raw key is stored.
        // 2. Store the above cdma 2 byte key id in to HSM vault with
        //    AES-GCM-256 key type
        // 3. Send the table index, key slot index and along with other
        //    parameters to FP via IPC.
        // 4. Once IPC succeed report success to caller. If not, delete the key
        //    stored in CDMA vault and then from HSM vault and report the
        //    failure

        // Step 0: Acquire the FP Ipc channel
        let channel_ref: FpIpcChannelRef<E> = self
            .state
            .env()
            .fp_ipc_channel()
            .acquire(tag, ())
            .ok_or(HsmErr::Pending)?;

        // Step 1: Store the raw key in to CDMA key vault and get the 'key id'
        let key_blob = der;
        let cdma_key_id = self.state.cdma_vault().import_key(key_blob)?;

        // Step 2: Store this key id in to HSM key vault and get the 'forwarding key id'
        let key_blob = cdma_key_id.as_bytes();
        // Convert the DdiKeyType to AesKeyKind
        let key_kind = match key_type {
            DdiKeyType::AesXtsBulk256 => AesKeyKind::AesXtsBulk256,
            DdiKeyType::AesGcmBulk256 => AesKeyKind::AesGcmBulk256,
            DdiKeyType::AesGcmBulk256Unapproved => AesKeyKind::AesGcmBulk256Unapproved,
            _ => return Err(HsmErr::InvalidKeyType),
        };
        let aes_key = AesKeyImported::new(key_kind, key_usage, key_blob)?;
        let key_id = self
            .state
            .vault()
            .aes_import_key(
                self.app_vault_id(),
                self.id(),
                entry_tag,
                &aes_key,
                attributes,
            )
            .or_else(|err| {
                self.state.cdma_vault().delete_key(cdma_key_id)?;
                Err(err)
            })?;

        // Step 3: Send the table index, key slot index and along with other
        //    parameters to FP via IPC
        let session_only = attributes.common.flags.session();
        let msg: IpcMessageKeyUpdate = IpcMessageKeyUpdate {
            info: KeyUpdateInfo {
                key_index: cdma_key_id.key_index(),
                resource_id: cdma_key_id.vault_id(),
                pfn,
                action: KeyUpdateAction::Create,
                session_id: self.id(),
                app_id: self.app_vault_id(),
                flag: AesKeyFlag::new()
                    .with_session_only(session_only)
                    .with_key_type(self.aes_bulk_key_type(key_type)?),
            },
            ..Default::default()
        };

        channel_ref
            .map(|c| c.send_request(tag, msg.encode()))
            .map_err(|err| {
                let _ = self.undo_aesbulk256_key_import(key_id.id(), cdma_key_id);
                error!("[aes] begin_import_der_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}", err);
                HsmErr::IpcSendFailure
            })?;

        // Step 4. implemented in end_import_der_aesbulk256_key_inner()

        Ok(AesBulk256Cmd::DerKeyImport(
            cdma_key_id,
            key_id.id(),
            channel_ref,
        ))
    }

    /// Receive IPC from FP and finish the Import der key operation for AES Bulk
    /// 256
    pub(super) fn end_import_der_aesbulk256_key_inner(
        &self,
        op: &AesBulk256Cmd<E>,
    ) -> HsmResult<()> {
        let AesBulk256Cmd::DerKeyImport(cdma_key_id, key_id, ref channel_ref) = *op else {
            return Err(HsmErr::AesBulk256InvalidParameter);
        };

        let message = channel_ref.map(|c| c.receive_message());
        if let Some(message) = message {
            let header = IpcMessageDecoder::decode_header(&message).map_err(|err| {
                error!("[aes] end_import_der_aesbulk256_key_inner: Failed to decode IPC message header: {:?}", err);
                let _ = self.undo_aesbulk256_key_import(key_id, cdma_key_id);
                HsmErr::IpcResponseError
            })?;

            if header.status() != IpcMessageStatusCode::Success.into() {
                error!(
                    "[aes] Import der key: Invalid IPC response with status {}",
                    header.status()
                );
                let _ = self.undo_aesbulk256_key_import(key_id, cdma_key_id);
                Err(HsmErr::IpcResponseError)?
            }

            Ok(())
        } else {
            error!("[aes] end_import_der_aesbulk256_key_inner: Spurious Message");
            Err(HsmErr::IpcResponseError)?
        }
    }

    fn undo_aesbulk256_key_import(
        &self,
        key_id: KeyId,
        cdma_key_id: AesBulk256KeyId,
    ) -> HsmResult<()> {
        self.state
            .cdma_vault()
            .delete_key(cdma_key_id)
            .or_else(|err| {
                self.delete_key(key_id)?;
                Err(err)
            })?;

        self.delete_key(key_id)?;

        Ok(())
    }

    fn enable_key(&self, key_id: KeyId) -> HsmResult<()> {
        self.state.vault().enable_key(key_id)
    }

    fn disable_key(&self, key_id: KeyId) -> HsmResult<()> {
        self.state.vault().disable_key(key_id)
    }

    /// Delete AES Bulk 256 key. Used by delete key FSM
    pub(super) fn begin_delete_aesbulk256_key_inner(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        key_id: KeyId,
    ) -> HsmResult<AesBulk256Cmd<E>> {
        // Steps to handle AesBulk256 key delete
        // 0. Acquire the CP/FP IPC channel
        // 1. Disable the key to prevent concurrent usage from other fsm
        // 2. Send the table index, key slot index and along with other
        //    parameters to FP via IPC to delete the key.

        // Step 0: Acquire the FP Ipc channel
        let channel_ref = self
            .state
            .env()
            .fp_ipc_channel()
            .acquire(tag, ())
            .ok_or(HsmErr::Pending)?;

        // Step 1: Get entry and disable it to prevent concurrent usage from other fsm
        let vault_key = self
            .state
            .vault()
            .key(self.app_vault_id(), self.id(), key_id, true)?;

        let key_blob = vault_key.blob()?;

        let cdma_key_id =
            AesBulk256KeyId::from(u16::from_le_bytes(key_blob[..2].try_into().unwrap()));
        let entry_attributes = vault_key.attributes()?;

        let session_only = entry_attributes.common.flags.session();

        // Step 2: Send the table index, key slot index and along with other
        //    parameters to FP via IPC
        let msg: IpcMessageKeyUpdate = IpcMessageKeyUpdate {
            info: KeyUpdateInfo {
                key_index: cdma_key_id.key_index(),
                resource_id: cdma_key_id.vault_id(),
                pfn,
                action: KeyUpdateAction::Delete,
                session_id: self.id(),
                app_id: self.app_vault_id(),
                flag: AesKeyFlag::new().with_session_only(session_only),
            },
            ..Default::default()
        };

        channel_ref
            .map(|c| c.send_request(tag, msg.encode()))
            .map_err(|err| {
                error!("[aes] begin_delete_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}", err);
                HsmErr::IpcSendFailure
            })?;

        self.disable_key(key_id)?;

        Ok(AesBulk256Cmd::DeleteKey(cdma_key_id, key_id, channel_ref))
    }

    /// Receive IPC from FP and finish the delete key operation for AES Bulk 256
    pub(super) fn end_delete_aesbulk256_key_inner(&self, op: &AesBulk256Cmd<E>) -> HsmResult<()> {
        let AesBulk256Cmd::DeleteKey(cdma_key_id, key_id, ref channel_ref) = *op else {
            return Err(HsmErr::AesBulk256InvalidParameter);
        };

        // Step 1: Validate response from FP
        let message = channel_ref.map(|c| c.receive_message());
        if let Some(message) = message {
            let header = IpcMessageDecoder::decode_header(&message).map_err(|err| {
                error!("[aes] end_delete_aesbulk256_key_inner: Failed to decode IPC message header: {:?}", err);
                let _ = self.enable_key(key_id);
                HsmErr::IpcResponseError
            })?;

            if header.status() != IpcMessageStatusCode::Success.into() {
                error!(
                    "[aes] Delete Key Operation: Invalid IPC response with status {}",
                    header.status()
                );
                let _ = self.enable_key(key_id);
                Err(HsmErr::IpcResponseError)?
            }

            // Step 2: If we receive a success response, delete the keys from
            // HSM/CDMA vault
            let _ = self.enable_key(key_id);
            self.undo_aesbulk256_key_import(key_id, cdma_key_id)?;

            Ok(())
        } else {
            error!("[aes] end_delete_aesbulk256_key_inner: Spurious Message");
            let _ = self.enable_key(key_id);
            Err(HsmErr::IpcResponseError)?
        }
    }

    /// Generate AES Bulk 256
    pub(super) fn begin_aesbulk256_gen_key_inner(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        key_tag: Option<u16>,
        key_type: DdiKeyType,
        availability: KeyAvailability,
    ) -> HsmResult<AesBulk256Cmd<E>> {
        // Steps to handle AesBulk256 key gen
        // 0. Generate random key
        // 1. Perform AES Bulk 256 import

        self.state
            .vault()
            .validate_key_params(availability, key_tag)?;

        // Allocate buffer of max length on the stack
        let mut buf = SecureByteArray::<32>::new([0u8; 32]);
        let key = buf.as_mut_slice();

        // Step 0: Generate a random key
        self.state.env().rng().bytes(key);

        // Step 1: Perform AES bulk 256 import
        let attributes = aes_entry_attributes(availability, true, AesKeyUsage::EncryptDecrypt);
        self.begin_import_der_aesbulk256_key_inner(
            tag,
            pfn,
            key_tag,
            key_type,
            AesKeyUsage::EncryptDecrypt,
            &attributes,
            key,
        )
    }

    /// Receive IPC from FP and finish the gen key operation for AES Bulk 256
    pub(super) fn end_aesbulk256_gen_key_inner(&self, op: &AesBulk256Cmd<E>) -> HsmResult<()> {
        self.end_import_der_aesbulk256_key_inner(op)
    }

    /// Rollback AES Bulk 256 key. Used by Aes Bulk 256 key rollback scheme.
    pub(super) fn begin_rollback_aesbulk256_key_inner(
        &self,
        tag: TagId,
        pfn: PcieFunction,
        op: &AesBulk256Cmd<E>,
    ) -> HsmResult<()> {
        let AesBulk256Cmd::DerKeyImport(cdma_key_id, key_id, ref channel_ref) = *op else {
            return Err(HsmErr::AesBulk256InvalidParameter);
        };

        let vault_key = self
            .state
            .vault()
            .key(self.app_vault_id(), self.id(), key_id, true)?;

        let entry_attributes = vault_key.attributes()?;

        let session_only = entry_attributes.common.flags.session();

        let msg = IpcMessageKeyUpdate {
            info: KeyUpdateInfo {
                key_index: cdma_key_id.key_index(),
                resource_id: cdma_key_id.vault_id(),
                pfn,
                action: KeyUpdateAction::Delete,
                session_id: self.id(),
                app_id: self.app_vault_id(),
                flag: AesKeyFlag::new().with_session_only(session_only),
            },
            ..Default::default()
        };

        channel_ref
            .map(|c| c.send_request(tag, msg.encode()))
            .map_err(|err| {
                error!("[aes] begin_rollback_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}", err);
                HsmErr::IpcSendFailure
            })?;

        self.disable_key(key_id)?;

        Ok(())
    }

    /// Receive IPC from FP and finish the delete key operation for AES Bulk 256 due to rollback
    pub(super) fn end_rollback_aesbulk256_key_inner(&self, op: &AesBulk256Cmd<E>) -> HsmResult<()> {
        let AesBulk256Cmd::DerKeyImport(cdma_key_id, key_id, ref channel_ref) = *op else {
            return Err(HsmErr::AesBulk256InvalidParameter);
        };

        let message = channel_ref.map(|c| c.receive_message());
        if let Some(message) = message {
            let header = IpcMessageDecoder::decode_header(&message).map_err(|err| {
                error!("[aes] end_rollback_aesbulk256_key_inner: Failed to decode IPC message header: {:?}", err);
                let _ = self.enable_key(key_id);
                HsmErr::IpcResponseDecodeError
            })?;

            if header.status() != IpcMessageStatusCode::Success.into() {
                error!(
                    "[aes] Delete key operation rollback: Invalid IPC response with status {}",
                    header.status()
                );
                let _ = self.enable_key(key_id);
                Err(HsmErr::IpcResponseError)?
            }

            let _ = self.enable_key(key_id);
            self.undo_aesbulk256_key_import(key_id, cdma_key_id)?;

            Ok(())
        } else {
            error!("[aes] end_rollback_aesbulk256_key_inner: Spurious Message");
            let _ = self.enable_key(key_id);
            Err(HsmErr::IpcResponseError)?
        }
    }
}
