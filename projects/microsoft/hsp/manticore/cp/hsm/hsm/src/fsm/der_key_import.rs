// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::vec::Vec;

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

/// DER Key Import command
pub(crate) struct DerKeyImportCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Session
    session: E::UserSession,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Cmd struct to avoid multiple decode operations on request DMA buffer
    decoded_req: Option<DdiDerKeyImportCmdReq>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,

    /// Key ID in case of rollback
    key_id: Option<KeyId>,

    /// RSA Command data
    cmd_data: Option<RsaCrtParamComputeCmd<E>>,

    /// DER Public key
    der_pub_key: Option<Vec<u8>>,

    /// AES Bulk 256 Command data
    aes_bulk256_cmd_data: Option<AesBulk256Cmd<E>>,

    /// Pfn required to send AES Bulk 256 IPC to FP
    pfn: PcieFunction,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for DerKeyImportCmd<E> {
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
            | (State::WaitForCmd, HsmFsmEvent::FpToHsmIpcResponse) => self.on_cmd_complete(),
            (State::Rollback, HsmFsmEvent::FpToHsmIpcResponse) => self.on_rollback_response(),
            (State::Final, _) | (State::WaitForIoCompletion, _) => {
                error!(
                    "[der_key_import] Invalid State, state:{:?}, event: {:?}",
                    self.state as u32,
                    u32::from(event)
                );
                Err(HsmErr::InvalidState)
            }
            (_, _) => {
                error!(
                    "[der_key_import] Invalid Event, state:{:?}, event: {:?}",
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
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;
        let entry_class: EntryClass = decoded_req.data.key_class.try_into()?;

        if self.state != State::WaitForIoCompletion {
            return Ok(());
        }

        // Set the state to Final incase if the rollback process experienced an error or it is a
        // synchronous operations
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
}

impl<E: HsmEnvTrait> DerKeyImportCmd<E> {
    /// Create a new command FSM
    pub fn new(
        req: DmaBuffer<E>,
        heap: DmaHeap<E>,
        session: E::UserSession,
        pfn: PcieFunction,
    ) -> Self {
        Self {
            state: State::Init,
            heap,
            session,
            req,
            decoded_req: None,
            resp: None,
            key_id: None,
            cmd_data: None,
            der_pub_key: None,
            aes_bulk256_cmd_data: None,
            pfn,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;

        // Decode the request
        self.decoded_req = Some(decode_buf::<DdiDerKeyImportCmdReq, E>(&self.req)?);
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;
        let entry_class: EntryClass = decoded_req.data.key_class.try_into()?;

        // Validate the key metadata to have a valid usage
        let _key_usage: DdiKeyUsage = decoded_req
            .data
            .key_properties
            .key_metadata
            .try_into()
            .map_err(|_| HsmErr::InvalidPermissions)?;

        match entry_class {
            EntryClass::RsaCrt
            | EntryClass::AesXtsBulk
            | EntryClass::AesGcmBulk
            | EntryClass::AesGcmBulkUnapproved => {
                if let Err(e) = self.begin_async_key_import(tag) {
                    if e.pending() {
                        self.state = State::WaitForResource;
                    }

                    Err(e)?
                }

                Err(HsmErr::Pending)
            }
            _ => self.sync_key_import(),
        }
    }

    /// Handle the PKA Engine ready event
    fn on_engine_ready(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;

        if let Err(mut e) = self.begin_async_key_import(tag) {
            if e.pending() {
                e = HsmErr::InvalidState;
            }

            Err(e)?
        }

        Err(HsmErr::Pending)
    }

    // Helper function to handle async key import
    fn begin_async_key_import(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // Decode the request
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;
        let entry_class: EntryClass = decoded_req.data.key_class.try_into()?;

        match entry_class {
            EntryClass::RsaCrt => {
                let op = self
                    .session
                    .begin_import_der_crt_key(tag, decoded_req.data.der.as_slice())?;

                self.update_state(op)?;
            }
            EntryClass::AesXtsBulk | EntryClass::AesGcmBulk | EntryClass::AesGcmBulkUnapproved => {
                let key_usage: DdiKeyUsage = decoded_req
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

                let op = self.session.begin_import_der_aesbulk256_key(
                    tag,
                    self.pfn,
                    key_usage,
                    decoded_req.data.key_tag,
                    entry_class.aes_bulk_ddi_key_type()?,
                    key_availability,
                    decoded_req.data.der.as_slice(),
                )?;

                self.aes_bulk256_cmd_data = Some(op);
            }
            _ => {
                unreachable!();
            }
        }

        self.state = State::WaitForCmd;

        Ok(())
    }

    // Helper function to handle sync key import
    fn sync_key_import(&mut self) -> Result<(), HsmErr> {
        // Decode the request
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;

        let key_usage: DdiKeyUsage = decoded_req
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

        let import_der_key_data = self.session.import_der_key(
            decoded_req.data.key_class.try_into()?,
            key_usage,
            decoded_req.data.key_tag,
            key_availability,
            decoded_req.data.der.as_slice(),
        )?;

        self.key_id = Some(import_der_key_data.priv_key_id);
        self.state = State::WaitForIoCompletion;

        // Encode and save the buffer
        self.resp = self.generate_response_with_mk(
            decoded_req.hdr.rev,
            decoded_req.hdr.sess_id,
            import_der_key_data.priv_key_id,
            import_der_key_data.pub_key_data.as_deref(),
            None,
            import_der_key_data.key_type,
            decoded_req.data.key_properties.key_label.as_slice(),
            None,
        )?;

        Ok(())
    }

    // Helper to update the internal command state parameters.
    fn update_state(&mut self, op: (RsaCrtParamComputeCmd<E>, Vec<u8>)) -> Result<(), HsmErr> {
        self.cmd_data = Some(op.0);
        self.der_pub_key = Some(op.1);
        Ok(())
    }

    /// Handle the PKA command done event
    fn on_cmd_complete(&mut self) -> Result<(), HsmErr> {
        // Mark the state as Final in case this FSM experienced any error
        self.state = State::Final;

        // Decode the request
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;
        let entry_class: EntryClass = decoded_req.data.key_class.try_into()?;

        match entry_class {
            EntryClass::RsaCrt => {
                let cmd_data = self.cmd_data.take().ok_or(HsmErr::InvalidState)?;
                let continue_import_data = self.session.continue_import_der_crt_key(cmd_data)?;

                if continue_import_data.state != RsaCrtParamCalcState::Idle {
                    self.cmd_data = Some(continue_import_data);
                    self.state = State::WaitForCmd;
                    return Err(HsmErr::Pending);
                }

                let key_usage: DdiKeyUsage = decoded_req
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
                self.state = State::WaitForIoCompletion;

                // Encode and save the buffer
                self.resp = self.generate_response_with_mk(
                    Some(self.session.api_rev()),
                    Some(self.session.id()),
                    key_id,
                    self.der_pub_key.as_deref(),
                    None,
                    ddi_key_type,
                    decoded_req.data.key_properties.key_label.as_slice(),
                    None,
                )?;
            }
            EntryClass::AesXtsBulk | EntryClass::AesGcmBulk | EntryClass::AesGcmBulkUnapproved => {
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
                self.resp = self.generate_response_with_mk(
                    Some(self.session.api_rev()),
                    Some(self.session.id()),
                    key_id,
                    None,
                    Some(AesBulk256KeyId::into(cdma_key_id)),
                    entry_class.aes_bulk_ddi_key_type()?,
                    decoded_req.data.key_properties.key_label.as_slice(),
                    Some(raw_key.as_slice()),
                )?;
            }
            _ => {
                unreachable!();
            }
        }

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
        rev: Option<DdiApiRev>,
        sess_id: Option<u16>,
        key_id: u16,
        data_pub_key: Option<&[u8]>,
        bulk_key_id: Option<u16>,
        ddi_key_type: DdiKeyType,
        masked_key_len: usize,
    ) -> DdiDerKeyImportCmdResp {
        let pub_key = data_pub_key.map(|der_slice| DdiDerPublicKey {
            der: MborByteArray::new_with_len(der_slice.as_ptr(), der_slice.len()),
            key_kind: ddi_key_type.to_public(),
        });

        DdiDerKeyImportCmdResp {
            hdr: DdiRespHdr {
                rev,
                op: DdiOp::DerKeyImport,
                sess_id,
                status: DdiStatus::Success,
                fips_approved: false,
            },
            data: DdiDerKeyImportResp {
                key_id,
                pub_key,
                bulk_key_id,
                key_type: ddi_key_type,
                masked_key: MborByteArray::new_with_len(core::ptr::null(), masked_key_len),
            },
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
        rev: Option<DdiApiRev>,
        sess_id: Option<u16>,
        key_id: u16,
        data_pub_key: Option<&[u8]>,
        bulk_key_id: Option<u16>,
        ddi_key_type: DdiKeyType,
        key_label: &[u8],
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
            rev,
            sess_id,
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
}
