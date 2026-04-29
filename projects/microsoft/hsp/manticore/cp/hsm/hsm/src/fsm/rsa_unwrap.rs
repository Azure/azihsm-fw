// Copyright (c) Microsoft Corporation. All rights reserved.

//! The command implements the PKCS #11 CKM_RSA_AES_KEY_WRAP unwrap operation, which involves
//! RSA-OAEP decryption (RFC 8017) and AES Wrap with Padding (RFC 5649).

use alloc::vec::Vec;
use core::ops::Range;
use mcr_crypto_pka::PkaEccCurve;
use mcr_crypto_pka::PkaEccPublicKey;
use mcr_crypto_pka::PkaRsaSize;

use log::error;

use super::*;

// Hard-coded the unwrapping key size as RSA 2k.
// TODO: determine the size based on the key handle
const UNWRAPPING_KEY_SIZE: usize = 256;

// Counter for max retries before timeout
const AES_UNWRAP_MAX_WAIT_TIME: u8 = 4;

/// FSM states
#[derive(Clone, Copy, PartialEq)]
enum State {
    /// Initial state
    Init,

    /// Waiting for PKA Engine or CP/FP IPC Channel
    WaitForResource,

    /// Wait for PKA operation or CP/FP IPC operation
    WaitForCmd,

    /// WaitForIoCompletion
    WaitForIoCompletion,

    /// Rollback
    Rollback,

    /// Final state
    Final,
}

/// Enum to describe the various states of PKA engine or CP/FP IPC channel usage.
#[derive(PartialEq, Copy, Clone)]
pub(crate) enum ResourceStates {
    /// Init
    Init,

    /// Unwrap operation is completed
    DoneUnwrap,

    /// Begin AES key unwrap process
    BeginAesKeyUnwrap,

    /// Begin CRT/AES Bulk key import operation
    BeginImport,

    /// Continue CRT/AES Bulk Import operation
    ContinueImport,

    /// CRT/AES Bulk Import operation done
    DoneImport,

    /// Begin structural validation
    BeginStructuralValidation,

    /// Continue structural validation
    ContinueStructuralValidation,

    /// End structural validation
    EndStructuralValidation,

    /// Begin PCT validation
    BeginPctValidation,

    /// Continue PCT validation
    ContinuePctValidation,
}

/// RSA Unwrap command
pub(crate) struct RsaUnwrapCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// App session
    session: E::UserSession,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,

    /// Rsa Modular Exponentiation Operation
    rsa_op: Option<RsaModExp<E>>,

    /// Key ID in case of rollback
    key_id: Option<KeyId>,

    /// Key type
    key_type: Option<DdiKeyType>,

    /// Key usage
    key_usage: Option<DdiKeyUsage>,

    /// structural validation Operation
    structural_op: Option<EccStructuralValidationCmd<E>>,

    /// RSA pct validation operation
    rsa_pct_op: Option<RsaPctValidationCmd<E>>,

    /// ECC pct validation operation
    ecc_pct_op: Option<EccKeyPct<E>>,

    /// Public key blob
    pub_key_blob: Option<Vec<u8>>,

    /// State describing the PKA or CP/FP IPC channel operation
    res_op_state: Option<ResourceStates>,

    /// RSA Command data
    cmd_data: Option<RsaCrtParamComputeCmd<E>>,

    /// The range of the unwrapped key within the AES wrapped portion
    /// (with the offset of the RSA unwrapping key size) of `wrapped_blob`
    /// in the `decoded_req`.
    unwrapped_key_range: Option<Range<usize>>,

    /// AES Bulk 256 Command data
    aes_bulk256_cmd_data: Option<AesBulk256Cmd<E>>,

    /// Pfn required to send AES Bulk 256 IPC to FP
    pfn: PcieFunction,

    /// Unwrapped data
    unwrapped_data: Option<DmaBuffer<E>>,

    /// KEK
    kek: Option<DmaBuffer<E>>,

    /// Check Alive Counter
    check_alive_cnt: u8,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for RsaUnwrapCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (State::WaitForResource, HsmFsmEvent::ResourceReady(_res)) => self.on_engine_ready(tag),
            (State::WaitForCmd, HsmFsmEvent::PkaDone(_))
            | (State::WaitForCmd, HsmFsmEvent::PkaError(_))
            | (State::WaitForCmd, HsmFsmEvent::FpToHsmIpcResponse)
            | (State::WaitForCmd, HsmFsmEvent::SoftAesResp) => self.on_cmd_complete(tag),
            (_, HsmFsmEvent::CheckAlive) => self.check_alive(),
            (State::Rollback, HsmFsmEvent::FpToHsmIpcResponse) => self.on_rollback_response(),
            (State::Final, _) | (State::WaitForIoCompletion, _) => {
                error!(
                    "[rsa_unwrap] Invalid State, state:{:?} event: {:?}",
                    self.state as u32,
                    u32::from(event)
                );
                Err(HsmErr::InvalidState)
            }
            (_, _) => {
                error!(
                    "[rsa_unwrap] Invalid Event, state:{:?} event: {:?}",
                    self.state as u32,
                    u32::from(event)
                );
                Err(HsmErr::InvalidEvent)
            }
        }
    }

    /// Get the session ID
    fn session_id(&self) -> Option<u16> {
        Some(self.session.id())
    }

    /// Check if the command requires resource
    fn requires_resource(&self, _tag: TagId, _res_id: ResId) -> bool {
        true
    }

    /// Acquire a resource
    fn acquire_resource(&mut self, _tag: TagId, res_id: ResId) -> HsmFsmEvent {
        match res_id {
            HsmFsmResourceId::Pka => HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            HsmFsmResourceId::FpIpcChannel => {
                HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
            }
            _ => unreachable!(),
        }
    }

    /// Perform any rollback in case of error
    fn rollback(&mut self, tag: TagId) -> HsmResult<()> {
        let decoded_req = decode_buf::<DdiRsaUnwrapCmdReq, E>(&self.req)?;
        let entry_class: EntryClass = decoded_req.data.wrapped_blob_key_class.try_into()?;

        if self.state != State::WaitForIoCompletion {
            return Ok(());
        }

        // Set the state to Final incase if the rollback process experienced an error or it is a
        // synchronours operations
        self.state = State::Final;

        match entry_class {
            EntryClass::AesXtsBulk | EntryClass::AesGcmBulk | EntryClass::AesGcmBulkUnapproved => {
                let aes_bulk256_cmd_data = self
                    .aes_bulk256_cmd_data
                    .as_ref()
                    .ok_or(HsmErr::InvalidState)?;

                self.session
                    .begin_rollback_aesbulk256_key(tag, self.pfn, aes_bulk256_cmd_data)?;

                self.state = State::Rollback;

                Err(HsmErr::Pending)
            }
            _ => {
                if let Some(key_id) = self.key_id {
                    self.session.delete_key(key_id)
                } else {
                    Ok(())
                }
            }
        }
    }
}

impl<E: HsmEnvTrait> RsaUnwrapCmd<E> {
    /// Create a new command FSM
    pub fn new(
        req: DmaBuffer<E>,
        heap: DmaHeap<E>,
        session: E::UserSession,
        part: E::Partition,
        pfn: PcieFunction,
    ) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            session,
            req,
            resp: None,
            rsa_op: None,
            key_id: None,
            key_type: None,
            key_usage: None,
            structural_op: None,
            rsa_pct_op: None,
            ecc_pct_op: None,
            pub_key_blob: None,
            res_op_state: None,
            cmd_data: None,
            unwrapped_key_range: None,
            aes_bulk256_cmd_data: None,
            pfn,
            unwrapped_data: None,
            kek: None,
            check_alive_cnt: 0,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> HsmResult<()> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;

        match self.begin_rsa_unwrap(tag) {
            Ok(op) => {
                self.rsa_op = Some(op);
                self.state = State::WaitForCmd;
                self.res_op_state = Some(ResourceStates::DoneUnwrap);

                Err(HsmErr::Pending)
            }

            Err(err) => {
                if err.pending() {
                    self.state = State::WaitForResource;
                    self.res_op_state = Some(ResourceStates::Init);
                }

                Err(err)
            }
        }
    }

    /// Handle the PKA Engine ready event
    fn on_engine_ready(&mut self, tag: TagId) -> HsmResult<()> {
        match self.res_op_state {
            Some(ResourceStates::Init) => self.handle_begin_rsa_unwrap(tag),
            Some(ResourceStates::BeginImport) => self.handle_import_key(tag),
            Some(ResourceStates::BeginStructuralValidation) => {
                self.handle_begin_structural_validation(tag)
            }
            Some(ResourceStates::BeginPctValidation) => self.handle_begin_pct_validation(tag),
            _ => unreachable!(),
        }
    }

    /// Begin the RSA unwrap after the UPKA resource became ready
    fn handle_begin_rsa_unwrap(&mut self, tag: u16) -> Result<(), HsmErr> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;

        match self.begin_rsa_unwrap(tag) {
            Ok(op) => {
                self.rsa_op = Some(op);
                self.state = State::WaitForCmd;
                self.res_op_state = Some(ResourceStates::DoneUnwrap);

                Err(HsmErr::Pending)
            }
            Err(mut err) => {
                if err.pending() {
                    err = HsmErr::InvalidState;
                }
                self.res_op_state = None;

                Err(err)
            }
        }
    }

    /// Handle the PKA command done event
    fn on_cmd_complete(&mut self, tag: TagId) -> HsmResult<()> {
        match self.res_op_state {
            Some(ResourceStates::DoneUnwrap) => self.handle_end_rsa_unwrap(tag),
            Some(ResourceStates::BeginAesKeyUnwrap) => self.end_aes_key_unwrap(tag),
            Some(ResourceStates::ContinueImport) => self.continue_import_key(tag),
            Some(ResourceStates::ContinueStructuralValidation) => {
                self.handle_continue_structural_validation()
            }
            Some(ResourceStates::EndStructuralValidation) => {
                self.handle_end_structural_validation(tag)
            }
            Some(ResourceStates::ContinuePctValidation) => self.handle_continue_pct_validation(tag),
            _ => unreachable!(),
        }
    }

    fn handle_end_rsa_unwrap(&mut self, tag: u16) -> Result<(), HsmErr> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;

        // Extract the KEK by OAEP decoding and copy it to the GSRAM
        // so that it can be shared across cores.
        let mut decoded_req = decode_buf::<DdiRsaUnwrapCmdReq, E>(&self.req)?;
        let kek = {
            let op = self.rsa_op.take().ok_or(HsmErr::InvalidState)?;

            self.session.end_rsa_unwrap_mod_exp_zc(tag, op)?;

            let unwrapped_data = self
                .unwrapped_data
                .take()
                .ok_or(HsmErr::RsaUnwrapInternalErr)?;
            let kek = self.session.decode_oaep_kek(
                unwrapped_data.as_ref(),
                decoded_req.data.wrapped_blob_padding,
                decoded_req.data.wrapped_blob_hash_algorithm,
            )?;

            // The size of Key-encryption key is bounded by AES 256
            if kek.len() > AesKeyKind::Aes256.into() {
                Err(HsmErr::RsaUnwrapInvalidKek)?
            }

            self.heap
                .copy_allocate(&kek)
                .ok_or(HsmErr::DmaAllocFailure)?
        };

        let wrapped_blob = &mut decoded_req.data.wrapped_blob.as_mut_slice();

        self.kek = Some(kek);
        let kek_ref = self
            .kek
            .as_ref()
            .ok_or(HsmErr::RsaUnwrapInternalErr)?
            .as_ref();

        self.state = State::WaitForCmd;
        self.res_op_state = Some(ResourceStates::BeginAesKeyUnwrap);

        self.session
            .begin_aes_key_unwrap(tag, kek_ref, &wrapped_blob[UNWRAPPING_KEY_SIZE..])
    }

    /// Start the CKM_RSA_AES_KEY_WRAP unwrap operation.
    fn begin_rsa_unwrap(&mut self, tag: TagId) -> HsmResult<RsaModExp<E>> {
        // Decode the request
        let decoded_req = decode_buf::<DdiRsaUnwrapCmdReq, E>(&self.req)?;

        // Validate the key metadata to have a valid usage
        let _key_usage: DdiKeyUsage = decoded_req
            .data
            .key_properties
            .key_metadata
            .try_into()
            .map_err(|_| HsmErr::InvalidPermissions)?;

        let wrapped_blob: MborByteArray<256> =
            MborByteArray::new_with_len(decoded_req.data.wrapped_blob.ptr(), 256);

        // Create a buffer for rsa_unwrap_mod_exp_zc to output to
        self.unwrapped_data = Some(self.heap.allocate(256).ok_or(HsmErr::DmaAllocFailure)?);
        let unwrap_output_buffer = self
            .unwrapped_data
            .as_ref()
            .ok_or(HsmErr::RsaUnwrapInternalErr)?
            .as_ref();
        let unwrap_output_buffer_mbor: MborByteArray<256> =
            MborByteArray::new_with_len(unwrap_output_buffer.as_ptr(), 256);

        self.session.begin_rsa_unwrap_mod_exp_zc(
            tag,
            decoded_req.data.key_id,
            &(&wrapped_blob).into(),
            &(&unwrap_output_buffer_mbor).into(),
            Some(RsaKeyUsage::Unwrap),
        )
    }

    fn end_aes_key_unwrap(&mut self, tag: TagId) -> HsmResult<()> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;

        let range = self.session.end_aes_key_unwrap(tag)?;

        self.unwrapped_key_range = Some(range);

        self.handle_import_key(tag)
    }

    fn handle_import_key(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let decoded_req = decode_buf::<DdiRsaUnwrapCmdReq, E>(&self.req)?;

        // Get the entry kind.
        let entry_class: EntryClass = decoded_req.data.wrapped_blob_key_class.try_into()?;
        let unwrapped_key_range = self
            .unwrapped_key_range
            .as_ref()
            .ok_or(HsmErr::InvalidState)?
            .clone();

        let wrapped_blob =
            &decoded_req.data.wrapped_blob.as_slice()[..decoded_req.data.wrapped_blob.len()];
        let aes_wrapped_blob = &wrapped_blob[UNWRAPPING_KEY_SIZE..];
        let unwrapped_key = &aes_wrapped_blob[unwrapped_key_range];

        match entry_class {
            EntryClass::Rsa => {
                self.import_sync_key_and_update_state(&decoded_req, unwrapped_key)?;

                self.handle_begin_pct_validation(tag)
            }
            EntryClass::Ecc => {
                self.import_sync_key_and_update_state(&decoded_req, unwrapped_key)?;

                self.handle_begin_structural_validation(tag)
            }
            EntryClass::Aes => {
                let import_key_data =
                    self.import_sync_key_and_update_state(&decoded_req, unwrapped_key)?;

                self.state = State::WaitForIoCompletion;

                // Encode and save the buffer
                self.resp = self.generate_response_with_mk(
                    self.session.api_rev(),
                    self.session.id(),
                    import_key_data.priv_key_id,
                    import_key_data.pub_key_data.as_deref(),
                    None,
                    import_key_data.key_type,
                    decoded_req.data.key_properties.key_label.as_slice(),
                )?;

                Ok(())
            }
            EntryClass::RsaCrt
            | EntryClass::AesXtsBulk
            | EntryClass::AesGcmBulk
            | EntryClass::AesGcmBulkUnapproved => {
                self.begin_import_key(tag, &decoded_req, unwrapped_key)
            }
            _ => Err(HsmErr::InvalidKeyType),
        }
    }

    fn import_sync_key_and_update_state(
        &mut self,
        decoded_req: &DdiRsaUnwrapCmdReq,
        unwrapped_key: &[u8],
    ) -> Result<ImportDerKeyResult, HsmErr> {
        let key_usage = decoded_req
            .data
            .key_properties
            .key_metadata
            .try_into()
            .map_err(|_| HsmErr::InvalidPermissions)?;

        let key_availability = if decoded_req.data.key_properties.key_metadata.session() {
            KeyAvailability::Session
        } else {
            KeyAvailability::App
        };

        let import_key_data = self.session.import_der_key(
            decoded_req.data.wrapped_blob_key_class.try_into()?,
            key_usage,
            decoded_req.data.key_tag,
            key_availability,
            unwrapped_key,
        )?;

        self.key_id = Some(import_key_data.priv_key_id);
        self.pub_key_blob = import_key_data
            .pub_key_data
            .as_ref()
            .map(|data| data.to_vec());
        self.key_type = Some(import_key_data.key_type);
        self.key_usage = Some(key_usage);

        Ok(import_key_data)
    }

    fn begin_import_key(
        &mut self,
        tag: TagId,
        decoded_req: &DdiRsaUnwrapCmdReq,
        unwrapped_key: &[u8],
    ) -> Result<(), HsmErr> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;
        let entry_class = decoded_req.data.wrapped_blob_key_class.try_into()?;

        let key_usage = decoded_req
            .data
            .key_properties
            .key_metadata
            .try_into()
            .map_err(|_| HsmErr::InvalidPermissions)?;

        let key_availability = if decoded_req.data.key_properties.key_metadata.session() {
            KeyAvailability::Session
        } else {
            KeyAvailability::App
        };

        match entry_class {
            EntryClass::AesXtsBulk | EntryClass::AesGcmBulk | EntryClass::AesGcmBulkUnapproved => {
                match self.session.begin_import_der_aesbulk256_key(
                    tag,
                    self.pfn,
                    key_usage,
                    decoded_req.data.key_tag,
                    entry_class.aes_bulk_ddi_key_type()?,
                    key_availability,
                    unwrapped_key,
                ) {
                    Ok(op) => {
                        self.aes_bulk256_cmd_data = Some(op);
                        self.state = State::WaitForCmd;
                        self.res_op_state = Some(ResourceStates::ContinueImport);
                        Err(HsmErr::Pending)
                    }
                    Err(err) => {
                        if err.pending() {
                            self.state = State::WaitForResource;

                            self.res_op_state = Some(ResourceStates::BeginImport);
                        }

                        Err(err)
                    }
                }
            }
            EntryClass::RsaCrt => match self.session.begin_import_der_crt_key(tag, unwrapped_key) {
                Ok(op) => {
                    self.update_state(op)?;
                    self.state = State::WaitForCmd;
                    self.res_op_state = Some(ResourceStates::ContinueImport);
                    Err(HsmErr::Pending)
                }
                Err(err) => {
                    if err.pending() {
                        self.state = State::WaitForResource;

                        self.res_op_state = Some(ResourceStates::BeginImport);
                    }

                    Err(err)
                }
            },
            _ => unreachable!(),
        }
    }

    fn continue_import_key(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;

        let decoded_req = decode_buf::<DdiRsaUnwrapCmdReq, E>(&self.req)?;
        let entry_class = decoded_req.data.wrapped_blob_key_class.try_into()?;

        match entry_class {
            EntryClass::AesXtsBulk | EntryClass::AesGcmBulk | EntryClass::AesGcmBulkUnapproved => {
                let aes_bulk256_cmd_data = self
                    .aes_bulk256_cmd_data
                    .as_ref()
                    .ok_or(HsmErr::InvalidState)?;

                self.session
                    .end_import_der_aesbulk256_key(aes_bulk256_cmd_data)?;

                let AesBulk256Cmd::DerKeyImport(cdma_key_id, key_id, _) = *aes_bulk256_cmd_data
                else {
                    return Err(HsmErr::AesBulk256InvalidParameter);
                };

                self.key_id = Some(key_id);
                self.state = State::WaitForIoCompletion;

                #[cfg(feature = "mcr_test_hooks")]
                if let Some(action) = self.session.cmd_fsm_test_action(None) {
                    if action == DdiTestAction::TriggerIoFailure {
                        Err(HsmErr::InvalidKeyType)?;
                    } else {
                        let _ = self.session.hsm_fsm_test_action(Some(action));
                    }
                }

                // Encode and save the buffer
                self.resp = self.generate_response_with_mk(
                    self.session.api_rev(),
                    self.session.id(),
                    key_id,
                    None,
                    Some(AesBulk256KeyId::into(cdma_key_id)),
                    entry_class.aes_bulk_ddi_key_type()?,
                    decoded_req.data.key_properties.key_label.as_slice(),
                )?;

                self.res_op_state = Some(ResourceStates::DoneImport);

                Ok(())
            }
            EntryClass::RsaCrt => {
                let cmd_data = self.cmd_data.take().ok_or(HsmErr::InvalidState)?;
                let continue_import_data = self.session.continue_import_der_crt_key(cmd_data)?;

                if continue_import_data.state != RsaCrtParamCalcState::Idle {
                    self.cmd_data = Some(continue_import_data);
                    self.state = State::WaitForCmd;
                    return Err(HsmErr::Pending);
                }

                let key_usage = decoded_req
                    .data
                    .key_properties
                    .key_metadata
                    .try_into()
                    .map_err(|_| HsmErr::InvalidPermissions)?;

                let key_availability = if decoded_req.data.key_properties.key_metadata.session() {
                    KeyAvailability::Session
                } else {
                    KeyAvailability::App
                };

                let (key_id, ddi_key_type) = self.session.end_import_der_crt_key(
                    continue_import_data,
                    key_usage,
                    decoded_req.data.key_tag,
                    key_availability,
                )?;

                self.key_id = Some(key_id);
                self.key_type = Some(ddi_key_type);
                self.key_usage = Some(key_usage);
                self.state = State::WaitForIoCompletion;

                self.handle_begin_pct_validation(tag)
            }
            _ => unreachable!(),
        }
    }

    fn handle_begin_structural_validation(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let key_id = self.key_id.ok_or(HsmErr::InvalidState)?;
        let key_usage = self.key_usage.ok_or(HsmErr::InvalidState)?;
        let pub_key_blob = self.pub_key_blob.as_ref().ok_or(HsmErr::InvalidState)?;

        match self.session.begin_ecc_structural_validation(
            tag,
            key_id,
            key_usage,
            pub_key_blob.to_vec(),
        ) {
            Ok(structural_op) => {
                self.structural_op = Some(structural_op);
                self.state = State::WaitForCmd;
                self.res_op_state = Some(ResourceStates::ContinueStructuralValidation);

                Err(HsmErr::Pending)
            }

            Err(err) if err.pending() => {
                if self.res_op_state == Some(ResourceStates::BeginStructuralValidation) {
                    // Already retried once
                    self.on_error(err)?
                }
                self.res_op_state = Some(ResourceStates::BeginStructuralValidation);
                self.state = State::WaitForResource;

                Err(HsmErr::Pending)
            }

            Err(err) => self.on_error(err),
        }
    }

    fn handle_continue_structural_validation(&mut self) -> Result<(), HsmErr> {
        let op = self.structural_op.take().ok_or(HsmErr::InvalidState)?;

        match self.session.continue_ecc_structural_validation(op) {
            Ok(next_op) => {
                self.structural_op = Some(next_op);
                self.state = State::WaitForCmd;
                self.res_op_state = Some(ResourceStates::EndStructuralValidation);

                Err(HsmErr::Pending)
            }
            Err(err) => self.on_error(err),
        }
    }

    fn handle_end_structural_validation(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let op = self.structural_op.take().ok_or(HsmErr::InvalidState)?;

        match self.session.end_ecc_structural_validation(op) {
            Ok(()) => self.handle_begin_pct_validation(tag),
            Err(err) => self.on_error(err),
        }
    }

    fn handle_begin_pct_validation(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let decoded_req = decode_buf::<DdiRsaUnwrapCmdReq, E>(&self.req)?;
        let entry_class = decoded_req.data.wrapped_blob_key_class.try_into()?;

        match entry_class {
            EntryClass::Rsa | EntryClass::RsaCrt => self.handle_begin_rsa_pct_validation(tag),
            EntryClass::Ecc => self.handle_begin_ecc_pct_validation(tag),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }

    fn handle_begin_rsa_pct_validation(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let key_id = self.key_id.ok_or(HsmErr::InvalidArgument)?;
        let key_usage = self
            .key_usage
            .ok_or(HsmErr::InvalidArgument)?
            .try_into()
            .map_err(|_| HsmErr::InvalidKeyType)?;
        let key_type = self.key_type.ok_or(HsmErr::InvalidKeyType)?;
        let rsa_type = match key_type {
            DdiKeyType::Rsa2kPrivateCrt | DdiKeyType::Rsa2kPrivate => RsaSize::Rsa2k,
            DdiKeyType::Rsa3kPrivateCrt | DdiKeyType::Rsa3kPrivate => RsaSize::Rsa3k,
            DdiKeyType::Rsa4kPrivateCrt | DdiKeyType::Rsa4kPrivate => RsaSize::Rsa4k,
            _ => return Err(HsmErr::InvalidKeyType),
        };
        let public_key = self.pub_key_blob.as_mut().ok_or(HsmErr::InvalidArgument)?;
        let (n, e) = public_key.split_at_mut(rsa_type.len());
        let e = &e[..4];

        match self.session.begin_rsa_pct_validation(
            tag,
            key_id,
            key_usage,
            PkaRsaSize::from(rsa_type),
            n,
            e,
        ) {
            Ok(rsa_pct_cmd) => {
                self.rsa_pct_op = Some(rsa_pct_cmd);
                self.state = State::WaitForCmd;
                self.res_op_state = Some(ResourceStates::ContinuePctValidation);
                Err(HsmErr::Pending)
            }

            Err(err) if err.pending() => {
                if self.res_op_state == Some(ResourceStates::BeginPctValidation) {
                    // Already retried once, treat as error
                    self.on_error(err)?
                }
                self.res_op_state = Some(ResourceStates::BeginPctValidation);
                self.state = State::WaitForResource;
                Err(HsmErr::Pending)
            }

            Err(err) => self.on_error(err),
        }
    }

    fn handle_begin_ecc_pct_validation(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let key_id = self.key_id.ok_or(HsmErr::InvalidArgument)?;
        let key_usage = EccKeyUsage::try_from(self.key_usage.ok_or(HsmErr::InvalidArgument)?)?;
        let key_type = self.key_type.ok_or(HsmErr::InvalidKeyType)?;
        let curve = match key_type {
            DdiKeyType::Ecc256Private => PkaEccCurve::Ecc256,
            DdiKeyType::Ecc384Private => PkaEccCurve::Ecc384,
            DdiKeyType::Ecc521Private => PkaEccCurve::Ecc521,
            _ => return Err(HsmErr::InvalidKeyType),
        };
        let public_key = self.pub_key_blob.as_ref().ok_or(HsmErr::InvalidArgument)?;
        let public_key_copy =
            PkaEccPublicKey::from_bytes(curve, public_key).map_err(|_| HsmErr::InvalidArgument)?;

        match self
            .session
            .begin_ecc_pct_validation(tag, key_id, key_usage, public_key_copy)
        {
            Ok(ecc_pct_cmd) => {
                self.ecc_pct_op = Some(ecc_pct_cmd);
                self.state = State::WaitForCmd;
                self.res_op_state = Some(ResourceStates::ContinuePctValidation);
                Err(HsmErr::Pending)
            }

            Err(err) if err.pending() => {
                if self.res_op_state == Some(ResourceStates::BeginPctValidation) {
                    // Already retried once, treat as error
                    self.on_error(err)?
                }
                self.res_op_state = Some(ResourceStates::BeginPctValidation);
                self.state = State::WaitForResource;
                Err(HsmErr::Pending)
            }

            Err(err) => self.on_error(err),
        }
    }

    fn handle_continue_pct_validation(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let decoded_req = decode_buf::<DdiRsaUnwrapCmdReq, E>(&self.req)?;
        let entry_class = decoded_req.data.wrapped_blob_key_class.try_into()?;

        match entry_class {
            EntryClass::Rsa | EntryClass::RsaCrt => self.continue_rsa_pct_validation(),
            EntryClass::Ecc => self.continue_ecc_pct_validation(tag),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }

    fn continue_rsa_pct_validation(&mut self) -> Result<(), HsmErr> {
        let decoded_req = decode_buf::<DdiRsaUnwrapCmdReq, E>(&self.req)?;

        let rsa_op = self.rsa_pct_op.take().ok_or(HsmErr::InvalidState)?;
        if self.session.is_rsa_pct_final_state(&rsa_op) {
            match self.session.end_rsa_pct_validation(rsa_op) {
                Ok(success) => {
                    if !success {
                        return self.on_error(HsmErr::PctValidationRsaUnwrapRsaKeyFailed);
                    }
                }
                Err(err) => return self.on_error(err),
            }
            // Prepare response
            let key_id = self.key_id.ok_or(HsmErr::InvalidState)?;
            let public_key = self.pub_key_blob.as_ref().ok_or(HsmErr::InvalidState)?;
            self.state = State::WaitForIoCompletion;
            self.resp = self.generate_response_with_mk(
                self.session.api_rev(),
                self.session.id(),
                key_id,
                Some(public_key),
                None,
                self.key_type.ok_or(HsmErr::InvalidKeyType)?,
                decoded_req.data.key_properties.key_label.as_slice(),
            )?;

            Ok(())
        } else {
            match self.session.continue_rsa_pct_validation(rsa_op) {
                Ok(continue_op) => {
                    self.rsa_pct_op = Some(continue_op);
                    self.state = State::WaitForCmd;
                    self.res_op_state = Some(ResourceStates::ContinuePctValidation);

                    Err(HsmErr::Pending)
                }
                Err(err) => self.on_error(err),
            }
        }
    }

    fn continue_ecc_pct_validation(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let decoded_req = decode_buf::<DdiRsaUnwrapCmdReq, E>(&self.req)?;

        let mut ecc_op = self.ecc_pct_op.take().ok_or(HsmErr::InvalidState)?;
        if self.session.is_pct_final_state(&ecc_op) {
            match self.session.end_ecc_pct_validation(tag, &mut ecc_op) {
                Ok(success) => {
                    if !success {
                        return self.on_error(HsmErr::PctValidationRsaUnwrapEccKeyFailed);
                    }
                }
                Err(err) => return self.on_error(err),
            }

            let key_id = self.key_id.ok_or(HsmErr::InvalidState)?;
            let key_type = self.key_type.ok_or(HsmErr::InvalidKeyType)?;
            self.state = State::WaitForIoCompletion;
            self.resp = self.generate_response_with_mk(
                self.session.api_rev(),
                self.session.id(),
                key_id,
                self.pub_key_blob.as_deref(),
                None,
                key_type,
                decoded_req.data.key_properties.key_label.as_slice(),
            )?;

            Ok(())
        } else {
            match self.session.continue_ecc_pct_validation(tag, &mut ecc_op) {
                Ok(_) => {
                    self.ecc_pct_op = Some(ecc_op);
                    self.state = State::WaitForCmd;
                    self.res_op_state = Some(ResourceStates::ContinuePctValidation);
                    Err(HsmErr::Pending)
                }
                Err(err) => self.on_error(err),
            }
        }
    }

    // Helper to update the internal command state parameters.
    fn update_state(&mut self, op: (RsaCrtParamComputeCmd<E>, Vec<u8>)) -> Result<(), HsmErr> {
        self.cmd_data = Some(op.0);
        self.pub_key_blob = Some(op.1);

        Ok(())
    }

    /// Handle the FP IPC response event for rollback completion
    fn on_rollback_response(&mut self) -> Result<(), HsmErr> {
        self.state = State::Final;

        let aes_bulk256_cmd_data = self
            .aes_bulk256_cmd_data
            .as_ref()
            .ok_or(HsmErr::InvalidState)?;

        self.session
            .end_rollback_aesbulk256_key(aes_bulk256_cmd_data)
    }

    /// Handle operation errors
    fn on_error(&mut self, mut err: HsmErr) -> Result<(), HsmErr> {
        if err.pending() {
            err = HsmErr::InvalidState;
        }

        // Move FSM to Final state
        self.state = State::Final;

        Err(err)
    }

    /// Create a command response
    #[allow(clippy::too_many_arguments)]
    fn cmd_resp(
        &self,
        rev: DdiApiRev,
        sess_id: u16,
        key_id: u16,
        pub_key: Option<&[u8]>,
        bulk_key_id: Option<u16>,
        ddi_key_type: DdiKeyType,
        masked_key_len: usize,
    ) -> DdiRsaUnwrapCmdResp {
        DdiRsaUnwrapCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::RsaUnwrap,
                sess_id: Some(sess_id),
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiRsaUnwrapResp {
                key_id,
                pub_key: pub_key.map(|data| DdiDerPublicKey {
                    der: MborByteArray::new_with_len(data.as_ptr(), data.len()),
                    key_kind: ddi_key_type.to_public(),
                }),
                bulk_key_id,
                kind: ddi_key_type,
                masked_key: MborByteArray::new_with_len(core::ptr::null(), masked_key_len),
            },
        }
    }

    /// On timer event response
    fn check_alive(&mut self) -> Result<(), HsmErr> {
        if self.state == State::WaitForCmd
            && self.res_op_state == Some(ResourceStates::BeginAesKeyUnwrap)
            && self.check_alive_cnt < AES_UNWRAP_MAX_WAIT_TIME
        {
            self.check_alive_cnt += 1;
            Err(HsmErr::Pending)
        } else {
            self.check_alive_cnt = 0;

            error!(
                "Timeout!! State: {:?} Res Op State: {:?}",
                self.state as u32,
                self.res_op_state.map(|s| s as u32).unwrap_or(u32::MAX)
            );

            Err(HsmErr::IoTimeOut)
        }
    }

    /// Generate the masked key and encode the response
    /// Step:
    /// 1. Get the encoded length
    /// 2. Pre encode the response
    /// 3. Generate the masked key in the pre-encoded field `masked_key`
    #[allow(clippy::too_many_arguments)]
    fn generate_response_with_mk(
        &self,
        rev: DdiApiRev,
        sess_id: u16,
        key_id: u16,
        pub_key: Option<&[u8]>,
        bulk_key_id: Option<u16>,
        ddi_key_type: DdiKeyType,
        key_label: &[u8],
    ) -> Result<Option<DmaBuffer<E>>, HsmErr> {
        let masked_key_len = self
            .session
            .get_masked_key_len_from_vault(key_label, key_id, pub_key)?;

        let mut resp = self.cmd_resp(
            rev,
            sess_id,
            key_id,
            pub_key,
            bulk_key_id,
            ddi_key_type,
            masked_key_len,
        );

        let buf = Some(encode_buf(&resp, &self.heap)?);

        self.session.mask_key_from_vault(
            key_label,
            key_id,
            pub_key,
            resp.data.masked_key.as_mut_slice(),
        )?;

        Ok(buf)
    }
}
