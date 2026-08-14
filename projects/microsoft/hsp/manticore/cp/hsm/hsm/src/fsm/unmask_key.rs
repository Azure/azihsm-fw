// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::partition::store::EntryAttributes;

use super::*;

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

/// Unmask key command
pub(crate) struct UnmaskKeyCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// User Session
    session: E::UserSession,

    /// Request DMA buffer
    req_buf: DmaBuffer<E>,

    /// Response DMA buffer
    resp_buf: Option<DmaBuffer<E>>,

    /// AES Bulk 256 Command data
    aes_bulk256_cmd_data: Option<AesBulk256Cmd<E>>,

    /// Key ID in case of rollback
    key_id: Option<KeyId>,

    /// Pfn required to send AES Bulk 256 IPC to FP
    pfn: PcieFunction,

    /// Store the raw result in case of rollback
    raw_result: Option<UnmaskedKeyRawResult<E>>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for UnmaskKeyCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp_buf.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (State::WaitForResource, HsmFsmEvent::ResourceReady(_res)) => self.on_engine_ready(tag),
            (State::WaitForCmd, HsmFsmEvent::FpToHsmIpcResponse) => self.on_cmd_complete(),
            (State::Rollback, HsmFsmEvent::FpToHsmIpcResponse) => self.on_rollback_response(),
            (State::Final, _) | (State::WaitForIoCompletion, _) => Err(HsmErr::InvalidState),
            (_, _) => {
                error!(
                    "[unmask_key] Invalid Event, state:{:?}, event: {:?}",
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

    /// Perform any rollback in case of error
    fn rollback(&mut self, tag: TagId) -> HsmResult<()> {
        if self.state != State::WaitForIoCompletion {
            return Ok(());
        }

        // Set the state to Final incase if the rollback process experienced an error or it is a
        // synchronous operations
        self.state = State::Final;

        if self.raw_result.is_none() {
            if self.key_id.is_none() {
                return Err(HsmErr::InvalidState);
            }
            self.session.delete_key(self.key_id.unwrap())
        } else {
            let raw_result = self.raw_result.as_ref().ok_or(HsmErr::InvalidState)?;
            if !raw_result.metadata.key_type.is_bulk_key() {
                return Err(HsmErr::InvalidState);
            }

            let aes_bulk256_cmd_data = self
                .aes_bulk256_cmd_data
                .as_ref()
                .ok_or(HsmErr::InvalidState)?;

            self.session
                .begin_rollback_aesbulk256_key(tag, self.pfn, aes_bulk256_cmd_data)?;

            self.state = State::Rollback;

            Err(HsmErr::Pending)
        }
    }

    /// Check if the command requires resource
    fn requires_resource(&self, _tag: TagId, _res_id: ResId) -> bool {
        true
    }

    /// Acquire a resource
    fn acquire_resource(&mut self, _tag: TagId, res_id: ResId) -> HsmFsmEvent {
        match res_id {
            HsmFsmResourceId::FpIpcChannel => {
                HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
            }
            _ => unreachable!(),
        }
    }
}

impl<E: HsmEnvTrait> UnmaskKeyCmd<E> {
    /// Create a new command FSM
    pub fn new(
        req_buf: DmaBuffer<E>,
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
            req_buf,
            resp_buf: None,
            pfn,
            key_id: None,
            aes_bulk256_cmd_data: None,
            raw_result: None,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;

        let req = decode_buf::<DdiUnmaskKeyCmdReq, E>(&self.req_buf)?;
        let masked_key = req.data.masked_key;
        let result = self.session.unmask_key_and_import(masked_key.as_slice())?;

        if let Some(res) = result.import_result {
            self.key_id = Some(res.import_result.priv_key_id);
            self.state = State::WaitForIoCompletion;

            self.resp_buf = self.generate_response_with_mk(
                res.key_label.as_slice(),
                res.import_result.priv_key_id,
                res.import_result.pub_key_data.as_deref(),
                None,
                res.import_result.key_type,
                None,
            )?;
        } else {
            if result.raw_result.is_none() {
                return Err(HsmErr::InvalidState);
            }

            self.raw_result = result.raw_result;

            let raw_result = self.raw_result.as_ref().ok_or(HsmErr::InvalidState)?;
            let key_type = raw_result.metadata.key_type;

            if !key_type.is_bulk_key() {
                return Err(HsmErr::InvalidState);
            }

            let aes_key_size: DdiAesKeySize =
                key_type.try_into().map_err(|_| HsmErr::InvalidKeyType)?;
            let raw_size = aes_key_size
                .try_into()
                .map_err(|_| HsmErr::InvalidKeyType)?;
            let original_attributes =
                EntryAttributes::read_from_bytes(raw_result.metadata.key_attributes.blob.as_ref())
                    .map_err(|_| HsmErr::InvalidArgument)?;
            let usage = Self::get_key_usage_from_attributes(&raw_result.metadata.key_attributes)?;

            match self.session.unmask_import_der_aesbulk256_key(
                tag,
                self.pfn,
                usage,
                raw_result.metadata.key_tag,
                key_type,
                &original_attributes,
                &raw_result.decrypted_key.as_ref()[..raw_size],
            ) {
                Ok(op) => {
                    self.aes_bulk256_cmd_data = Some(op);
                    self.state = State::WaitForCmd;

                    Err(HsmErr::Pending)?
                }
                Err(err) => {
                    if err.pending() {
                        self.state = State::WaitForResource;
                    }

                    Err(err)?
                }
            }
        }

        Ok(())
    }

    // Handle resource ready event
    fn on_engine_ready(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;

        let raw_result = self.raw_result.as_ref().ok_or(HsmErr::InvalidState)?;
        let key_type = raw_result.metadata.key_type;

        if !key_type.is_bulk_key() {
            return Err(HsmErr::InvalidState);
        }

        let aes_key_size: DdiAesKeySize =
            key_type.try_into().map_err(|_| HsmErr::InvalidKeyType)?;
        let raw_size = aes_key_size
            .try_into()
            .map_err(|_| HsmErr::InvalidKeyType)?;
        let original_attributes =
            EntryAttributes::read_from_bytes(raw_result.metadata.key_attributes.blob.as_ref())
                .map_err(|_| HsmErr::InvalidArgument)?;
        let usage = Self::get_key_usage_from_attributes(&raw_result.metadata.key_attributes)?;

        match self.session.unmask_import_der_aesbulk256_key(
            tag,
            self.pfn,
            usage,
            raw_result.metadata.key_tag,
            key_type,
            &original_attributes,
            &raw_result.decrypted_key.as_ref()[..raw_size],
        ) {
            Ok(op) => {
                self.aes_bulk256_cmd_data = Some(op);
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }
            Err(mut err) => {
                if err.pending() {
                    err = HsmErr::InvalidState;
                }

                Err(err)
            }
        }
    }

    /// Handle the complete event
    fn on_cmd_complete(&mut self) -> Result<(), HsmErr> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;

        let raw_result = self.raw_result.as_ref().ok_or(HsmErr::InvalidState)?;
        let key_type = raw_result.metadata.key_type;

        if !key_type.is_bulk_key() {
            return Err(HsmErr::InvalidState);
        }

        let aes_bulk256_cmd_data = self
            .aes_bulk256_cmd_data
            .as_ref()
            .ok_or(HsmErr::InvalidState)?;
        self.session
            .end_import_der_aesbulk256_key(aes_bulk256_cmd_data)?;

        let AesBulk256Cmd::DerKeyImport(cdma_key_id, key_id, _, ref raw_key) =
            *aes_bulk256_cmd_data
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
        self.resp_buf = self.generate_response_with_mk(
            raw_result.metadata.key_label.as_slice(),
            key_id,
            None,
            Some(AesBulk256KeyId::into(cdma_key_id)),
            key_type,
            Some(raw_key.as_slice()),
        )?;

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

    /// Create a command response
    #[allow(clippy::too_many_arguments)]
    fn cmd_resp(
        &self,
        rev: DdiApiRev,
        sess_id: u16,
        key_id: u16,
        data_pub_key: Option<&[u8]>,
        bulk_key_id: Option<u16>,
        ddi_key_type: DdiKeyType,
        masked_key_len: usize,
    ) -> DdiUnmaskKeyCmdResp {
        let pub_key = data_pub_key.map(|der_slice| DdiDerPublicKey {
            der: MborByteArray::new_with_len(der_slice.as_ptr(), der_slice.len()),
            key_kind: ddi_key_type.to_public(),
        });

        DdiUnmaskKeyCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::UnmaskKey,
                sess_id: Some(sess_id),
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiUnmaskKeyResp {
                key_id,
                pub_key,
                bulk_key_id,
                kind: ddi_key_type,
                masked_key: MborByteArray::new_with_len(core::ptr::null(), masked_key_len),
            },
        }
    }

    /// Generate the masked key and encode the response
    /// Step:
    /// 1. Get the encoded length
    /// 2. Pre encode the response
    /// 3. Generate the masked key in the pre-encoded field `masked_key`
    fn generate_response_with_mk(
        &self,
        key_label: &[u8],
        key_id: KeyId,
        data_pub_key: Option<&[u8]>,
        bulk_key_id: Option<u16>,
        ddi_key_type: DdiKeyType,
        raw_key: Option<&[u8]>,
    ) -> Result<Option<DmaBuffer<E>>, HsmErr> {
        let masked_key_len = if let Some(key) = raw_key {
            self.session
                .get_masked_bulk_key_len(key_label, key_id, key.len())?
        } else {
            self.session
                .get_masked_key_len_from_vault(key_label, key_id, data_pub_key)?
        };
        let mut resp = self.cmd_resp(
            self.session.api_rev(),
            self.session.id(),
            key_id,
            data_pub_key,
            bulk_key_id,
            ddi_key_type,
            masked_key_len,
        );

        let buf = Some(encode_buf(&resp, &self.heap)?);

        if let Some(key) = raw_key {
            self.session.mask_bulk_key(
                key_label,
                key_id,
                key,
                resp.data.masked_key.as_mut_slice(),
            )?;
        } else {
            self.session.mask_key_from_vault(
                key_label,
                key_id,
                data_pub_key,
                resp.data.masked_key.as_mut_slice(),
            )?;
        }

        Ok(buf)
    }

    fn get_key_usage_from_attributes(
        attributes: &DdiMaskedKeyAttributes,
    ) -> HsmResult<DdiKeyUsage> {
        let entry_attributes = EntryAttributes::ref_from_bytes(attributes.blob.as_ref())
            .map_err(|_| HsmErr::InvalidArgument)?;

        if entry_attributes.common.flags.derive() {
            Ok(DdiKeyUsage::Derive)
        } else if entry_attributes.common.flags.encrypt() && entry_attributes.common.flags.decrypt()
        {
            Ok(DdiKeyUsage::EncryptDecrypt)
        } else if entry_attributes.common.flags.sign() && entry_attributes.common.flags.verify() {
            Ok(DdiKeyUsage::SignVerify)
        } else if entry_attributes.common.flags.unwrap() {
            Ok(DdiKeyUsage::Unwrap)
        } else {
            Err(HsmErr::InvalidArgument)
        }
    }
}
