// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_sha::HKDF_MAX_SALT_SIZE;

use super::*;

/// FSM states
#[derive(Clone, Copy, PartialEq)]
enum State {
    /// Initial state
    Init,

    /// Waiting for CP/FP IPC Channel
    WaitForResource,

    /// Wait for CP/FP IPC operation
    WaitForCmd,

    /// WaitForIoCompletion
    WaitForIoCompletion,

    /// Rollback
    Rollback,

    /// Final state
    Final,
}

/// HKDF derive command
pub(crate) struct HkdfDeriveCmd<E: HsmEnvTrait + 'static> {
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

    /// Key ID in case of rollback
    key_id: Option<KeyId>,

    /// Buffer to hold salt data
    salt_heap: Option<DmaBuffer<E>>,

    /// AES Bulk 256 Command data
    aes_bulk256_cmd_data: Option<AesBulk256Cmd<E>>,

    /// Pfn required to send AES Bulk 256 IPC to FP
    pfn: PcieFunction,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for HkdfDeriveCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (State::WaitForResource, HsmFsmEvent::ResourceReady(_res)) => self.on_engine_ready(tag),
            (State::WaitForCmd, HsmFsmEvent::FpToHsmIpcResponse) => self.on_cmd_complete(),
            (State::Rollback, HsmFsmEvent::FpToHsmIpcResponse) => self.on_rollback_response(),
            (State::Final, _) => Err(HsmErr::InvalidState),
            (_, _) => Err(HsmErr::InvalidEvent),
        }
    }

    /// Get the session ID
    fn session_id(&self) -> Option<u16> {
        Some(self.session.id())
    }

    /// Check if the command requires resource
    fn requires_resource(&self, _tag: TagId, _res_id: ResId) -> bool {
        match decode_buf::<DdiHkdfDeriveCmdReq, E>(&self.req) {
            Ok(req) => req.data.key_type.is_bulk_key(),
            Err(_) => false,
        }
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

    /// Perform any rollback in case of error
    fn rollback(&mut self, tag: TagId) -> HsmResult<()> {
        let req = decode_buf::<DdiHkdfDeriveCmdReq, E>(&self.req)?;
        let data = &req.data;

        if self.state != State::WaitForIoCompletion {
            return Ok(());
        }

        // FSM can be called only once
        self.state = State::Final;

        if data.key_type.is_bulk_key() {
            let aes_bulk256_cmd_data = self
                .aes_bulk256_cmd_data
                .as_ref()
                .ok_or(HsmErr::InvalidState)?;

            self.session
                .begin_rollback_aesbulk256_key(tag, self.pfn, aes_bulk256_cmd_data)?;

            self.state = State::Rollback;

            Err(HsmErr::Pending)
        } else {
            if let Some(key_id) = self.key_id {
                self.session.delete_key(key_id)?;
            }

            Ok(())
        }
    }
}

impl<E: HsmEnvTrait> HkdfDeriveCmd<E> {
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
            key_id: None,
            salt_heap: None,
            aes_bulk256_cmd_data: None,
            pfn,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // FSM can be called only once
        self.state = State::Final;

        // Decode the request from DMA buffer
        let req = decode_buf::<DdiHkdfDeriveCmdReq, E>(&self.req)?;
        let data = &req.data;

        // Handle salt and info slices
        self.salt_heap = data
            .salt
            .is_none()
            .then(|| {
                self.heap
                    .copy_allocate(&[0u8; HKDF_MAX_SALT_SIZE])
                    .ok_or(HsmErr::DmaAllocFailure)
            })
            .transpose()?;

        let (salt_slice, info_slice) = (
            data.salt.as_ref().map_or_else(
                || self.salt_heap.as_ref().unwrap().as_ref(),
                |salt| salt.as_slice(),
            ),
            data.info.as_ref().map_or(&[][..], |info| info.as_slice()),
        );

        // Call appropriate hkdf derive function based on key type
        if data.key_type.is_bulk_key() {
            let kdf_info = KdfInfo {
                key_id: data.key_id,
                hash_algo: data.hash_algorithm,
                key_type: data.key_type,
                key_properties: data
                    .key_properties
                    .clone()
                    .try_into()
                    .map_err(|_| HsmErr::InvalidPermissions)?,
                key_tag: data.key_tag,
            };

            match self
                .session
                .begin_hkdf_aesbulk256_derive(tag, self.pfn, salt_slice, info_slice, kdf_info)
            {
                Ok(op) => {
                    // Save the command data for later use
                    self.aes_bulk256_cmd_data = Some(op);
                    self.state = State::WaitForCmd;
                    Err(HsmErr::Pending)
                }
                Err(err) => {
                    if err.pending() {
                        self.state = State::WaitForResource;
                    }
                    Err(err)
                }
            }
        } else {
            let key_id = self.session.hkdf_derive(
                data.key_id,
                salt_slice,
                info_slice,
                data.hash_algorithm,
                data.key_type,
                data.key_properties
                    .clone()
                    .try_into()
                    .map_err(|_| HsmErr::InvalidPermissions)?,
                data.key_tag,
                data.key_length,
            )?;

            // Save key ID in case of rollback
            self.key_id = Some(key_id);
            self.state = State::WaitForIoCompletion;

            // Encode and save the buffer
            self.resp = self.generate_response_with_mk(
                self.session.api_rev(),
                self.session.id(),
                key_id,
                None,
                data.key_properties.key_label.as_slice(),
            )?;

            Ok(())
        }
    }

    /// Handle the FP IPC ready event
    fn on_engine_ready(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // FSM can be called only once
        self.state = State::Final;

        // Decode the request from DMA buffer
        let req = decode_buf::<DdiHkdfDeriveCmdReq, E>(&self.req)?;
        let data = &req.data;

        let (salt_slice, info_slice) = (
            data.salt.as_ref().map_or_else(
                || self.salt_heap.as_ref().unwrap().as_ref(),
                |salt| salt.as_slice(),
            ),
            data.info.as_ref().map_or(&[][..], |info| info.as_slice()),
        );

        if data.key_type.is_bulk_key() {
            let kdf_info = KdfInfo {
                key_id: data.key_id,
                hash_algo: data.hash_algorithm,
                key_type: data.key_type,
                key_properties: data
                    .key_properties
                    .clone()
                    .try_into()
                    .map_err(|_| HsmErr::InvalidPermissions)?,
                key_tag: data.key_tag,
            };

            match self
                .session
                .begin_hkdf_aesbulk256_derive(tag, self.pfn, salt_slice, info_slice, kdf_info)
            {
                Ok(op) => {
                    // Save the command data for later use
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
        } else {
            unreachable!();
        }
    }

    /// Handle the FP IPC response event
    fn on_cmd_complete(&mut self) -> Result<(), HsmErr> {
        // FSM can be called only once
        self.state = State::Final;

        // Decode the request from DMA buffer
        let req = decode_buf::<DdiHkdfDeriveCmdReq, E>(&self.req)?;
        let data = &req.data;

        if data.key_type.is_bulk_key() {
            let aes_bulk256_cmd_data = self
                .aes_bulk256_cmd_data
                .as_ref()
                .ok_or(HsmErr::InvalidState)?;

            self.session
                .end_kdf_aesbulk256_derive(aes_bulk256_cmd_data)?;

            let AesBulk256Cmd::DerKeyImport(bulk_key_id, key_id, _) = *aes_bulk256_cmd_data else {
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
                Some(AesBulk256KeyId::into(bulk_key_id)),
                data.key_properties.key_label.as_slice(),
            )?;
        } else {
            unreachable!();
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
    fn cmd_resp(
        &self,
        rev: DdiApiRev,
        session_id: u16,
        key_id: u16,
        bulk_key_id: Option<u16>,
        masked_key_len: usize,
    ) -> DdiHkdfDeriveCmdResp {
        DdiHkdfDeriveCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::HkdfDerive,
                sess_id: Some(session_id),
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiHkdfDeriveResp {
                key_id,
                masked_key: MborByteArray::new_with_len(core::ptr::null(), masked_key_len),
                bulk_key_id,
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
        rev: DdiApiRev,
        session_id: u16,
        key_id: u16,
        bulk_key_id: Option<u16>,
        key_label: &[u8],
    ) -> Result<Option<DmaBuffer<E>>, HsmErr> {
        let masked_key_len = self
            .session
            .get_masked_key_len_from_vault(key_label, key_id, None)?;

        let mut resp = self.cmd_resp(rev, session_id, key_id, bulk_key_id, masked_key_len);

        let buf = Some(encode_buf(&resp, &self.heap)?);

        self.session.mask_key_from_vault(
            key_label,
            key_id,
            None,
            resp.data.masked_key.as_mut_slice(),
        )?;

        Ok(buf)
    }
}
