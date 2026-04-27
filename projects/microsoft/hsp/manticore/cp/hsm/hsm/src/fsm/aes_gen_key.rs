// Copyright (c) Microsoft Corporation. All rights reserved.

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

/// Change manager credential command
pub(crate) struct AesGenKeyCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// Partition
    session: E::UserSession,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Cmd struct to avoid multiple decode operations on request DMA buffer
    decoded_req: Option<DdiAesGenerateKeyCmdReq>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,

    /// Key ID
    key_id: Option<u16>,

    /// AES Bulk 256 Command data
    aes_bulk256_cmd_data: Option<AesBulk256Cmd<E>>,

    /// Pfn required to send AES Bulk 256 IPC to FP
    pfn: PcieFunction,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for AesGenKeyCmd<E> {
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
            (State::Final, _) | (State::WaitForIoCompletion, _) => Err(HsmErr::InvalidState),
            (_, _) => {
                error!(
                    "[aes_gen_key] Invalid Event, state:{:?}, event: {:?}",
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
        if let Some(decoded_req) = self.decoded_req.as_ref() {
            decoded_req.data.key_size.is_bulk_key()
        } else {
            false
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
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;

        if self.state != State::WaitForIoCompletion {
            return Ok(());
        }

        // Set the state to Final incase if the rollback process experienced an error or it is a
        // synchronours operations
        self.state = State::Final;

        if decoded_req.data.key_size.is_bulk_key() {
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

impl<E: HsmEnvTrait> AesGenKeyCmd<E> {
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
            decoded_req: None,
            resp: None,
            key_id: None,
            aes_bulk256_cmd_data: None,
            pfn,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // FSM can be called only once
        self.state = State::Final;

        // Decode the request
        self.decoded_req = Some(decode_buf::<DdiAesGenerateKeyCmdReq, E>(&self.req)?);

        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;

        let key_availability = if decoded_req.data.key_properties.key_metadata.session() {
            DdiKeyAvailability::Session
        } else {
            DdiKeyAvailability::App
        };

        let usage: DdiKeyUsage = decoded_req
            .data
            .key_properties
            .key_metadata
            .try_into()
            .map_err(|_| HsmErr::InvalidPermissions)?;

        let data = &decoded_req.data;

        if decoded_req.data.key_size.is_bulk_key() {
            let key_type: DdiKeyType = data
                .key_size
                .try_into()
                .map_err(|_| HsmErr::InvalidKeyType)?;

            match self.session.begin_aesbulk256_gen_key(
                tag,
                self.pfn,
                data.key_tag,
                key_type,
                key_availability.try_into()?,
            ) {
                Ok(op) => {
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
            // Generate the key
            let key = self.session.aes_gen_key(
                data.key_tag,
                data.key_size.try_into()?,
                usage.try_into()?,
                key_availability.try_into()?,
            )?;

            self.key_id = Some(key.id());
            self.state = State::WaitForIoCompletion;

            // Encode and save the buffer
            self.resp = self.generate_response_with_mk(
                decoded_req.hdr.rev,
                decoded_req.hdr.sess_id,
                key.id(),
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

        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;

        let key_availability = if decoded_req.data.key_properties.key_metadata.session() {
            DdiKeyAvailability::Session
        } else {
            DdiKeyAvailability::App
        };

        if decoded_req.data.key_size.is_bulk_key() {
            let data = &decoded_req.data;
            let key_type: DdiKeyType = data
                .key_size
                .try_into()
                .map_err(|_| HsmErr::InvalidKeyType)?;
            match self.session.begin_aesbulk256_gen_key(
                tag,
                self.pfn,
                data.key_tag,
                key_type,
                key_availability.try_into()?,
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
        } else {
            unreachable!();
        }
    }

    /// Handle the FP IPC response event
    fn on_cmd_complete(&mut self) -> Result<(), HsmErr> {
        // FSM can be called only once
        self.state = State::Final;

        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;

        if decoded_req.data.key_size.is_bulk_key() {
            let aes_bulk256_cmd_data = self
                .aes_bulk256_cmd_data
                .as_ref()
                .ok_or(HsmErr::InvalidState)?;
            // Step 4: Receive IPC response and appropriately delete the
            // keys from CDMA and HSM vaults
            self.session.end_aesbulk256_gen_key(aes_bulk256_cmd_data)?;

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
                decoded_req.hdr.rev,
                decoded_req.hdr.sess_id,
                key_id,
                Some(bulk_key_id.into()),
                decoded_req.data.key_properties.key_label.as_slice(),
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
        rev: Option<DdiApiRev>,
        sess_id: Option<u16>,
        key_id: u16,
        bulk_key_id: Option<u16>,
        masked_key_len: usize,
    ) -> DdiAesGenerateKeyCmdResp {
        DdiAesGenerateKeyCmdResp {
            hdr: DdiRespHdr {
                rev,
                op: DdiOp::AesGenerateKey,
                sess_id,
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiAesGenerateKeyResp {
                key_id,
                bulk_key_id,
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
        rev: Option<DdiApiRev>,
        sess_id: Option<u16>,
        key_id: u16,
        bulk_key_id: Option<u16>,
        key_label: &[u8],
    ) -> Result<Option<DmaBuffer<E>>, HsmErr> {
        let masked_key_len = self
            .session
            .get_masked_key_len_from_vault(key_label, key_id, None)?;

        let mut resp = self.cmd_resp(rev, sess_id, key_id, bulk_key_id, masked_key_len);

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
