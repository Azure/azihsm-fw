// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

/// FSM states
#[derive(Clone, Copy)]
enum State {
    /// Initial state
    Init,

    /// Waiting for CP/FP IPC Channel
    WaitForResource,

    /// Wait for CP/FP IPC operation
    WaitForCmd,

    /// Final state
    Final,
}

/// Change manager credential command
pub(crate) struct DeleteKeyCmd<E: HsmEnvTrait + 'static> {
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
    decoded_req: Option<DdiDeleteKeyCmdReq>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,

    /// Flag indicating whether command logic has been executed
    committed: bool,

    /// AES Bulk 256 Command data
    aes_bulk256_cmd_data: Option<AesBulk256Cmd<E>>,

    /// Pfn required to send AES Bulk 256 IPC to FP
    pfn: PcieFunction,

    /// entry kind
    entry_kind: EntryKind,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for DeleteKeyCmd<E> {
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
            (State::Final, _) => Err(HsmErr::InvalidState),
            (_, _) => {
                error!(
                    "[delete_key] Invalid event, state:{:?}, event: {:?}",
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

    /// Check if the command needs to be retried
    fn retry(&self) -> bool {
        !self.committed
    }

    /// Check if the command requires resource
    fn requires_resource(&self, _tag: TagId, _res_id: ResId) -> bool {
        self.entry_kind.is_bulk_key()
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

impl<E: HsmEnvTrait> DeleteKeyCmd<E> {
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
            committed: false,
            aes_bulk256_cmd_data: None,
            pfn,
            entry_kind: EntryKind::Free,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        self.state = State::Final;

        // Decode the request
        self.decoded_req = Some(decode_buf::<DdiDeleteKeyCmdReq, E>(&self.req)?);

        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;
        let key_id = decoded_req.data.key_id;
        // Step 1: Open key from HSM key vault
        self.entry_kind = self.session.get_key_kind(key_id)?;

        if self.entry_kind == EntryKind::Session {
            // Do not allow session key to be deleted directly from here. They must follow
            // close session api. Other internal keys use app id as APP_VAULT_ID_FOR_INTERNAL_KEYS
            // which prevents their deletion. Session keys use proper app id in case we ever have
            // multiple apps and wish to know which session belongs to which app.
            // This is why we need this check.
            Err(HsmErr::KeyNotFound)?
        }

        // Step 2: Determine key type
        if self.entry_kind.is_bulk_key() {
            // Step 3: Send IPC to FP for Delete Key operation
            match self
                .session
                .begin_delete_aesbulk256_key(tag, self.pfn, key_id)
            {
                Ok(op) => {
                    self.aes_bulk256_cmd_data = Some(op);
                    self.state = State::WaitForCmd;
                    Err(HsmErr::Pending)
                }
                Err(err) => {
                    if err.pending() {
                        self.state = State::WaitForResource;
                    } else {
                        self.state = State::Final;
                    }
                    Err(err)
                }
            }
        } else {
            // Delete the key
            self.session.delete_key(key_id)?;

            // Command operation has succeeded; set committed flag to avoid retry
            self.committed = true;

            // Encode and save the buffer
            self.resp = Some(encode_buf(
                &self.cmd_resp(decoded_req.hdr.rev, decoded_req.hdr.sess_id),
                &self.heap,
            )?);

            Ok(())
        }
    }

    /// Handle the FP IPC ready event
    fn on_engine_ready(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;
        let key_id = decoded_req.data.key_id;

        if self.entry_kind.is_bulk_key() {
            match self
                .session
                .begin_delete_aesbulk256_key(tag, self.pfn, key_id)
            {
                Ok(op) => {
                    self.aes_bulk256_cmd_data = Some(op);
                    self.state = State::WaitForCmd;
                    Err(HsmErr::Pending)
                }
                Err(mut err) => {
                    warn!(
                        "[delete_key], begin_delete_aesbulk256_key returned err: {:?}",
                        u32::from(err)
                    );
                    if err.pending() {
                        err = HsmErr::InvalidState;
                    }
                    self.state = State::Final;
                    Err(err)
                }
            }
        } else {
            unreachable!();
        }
    }

    /// Handle the FP IPC response event
    fn on_cmd_complete(&mut self) -> Result<(), HsmErr> {
        if self.entry_kind.is_bulk_key() {
            let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;
            let aes_bulk256_cmd_data = self
                .aes_bulk256_cmd_data
                .as_ref()
                .ok_or(HsmErr::InvalidState)?;
            // Step 4: Receive IPC response and appropriately delete the
            // keys from CDMA and HSM vaults
            self.session
                .end_delete_aesbulk256_key(aes_bulk256_cmd_data)?;

            // Command operation has succeeded; set committed flag to avoid retry
            self.committed = true;

            // Encode and save the buffer
            self.resp = Some(encode_buf(
                &self.cmd_resp(decoded_req.hdr.rev, decoded_req.hdr.sess_id),
                &self.heap,
            )?);

            self.state = State::Final;
        } else {
            unreachable!();
        }

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(&self, rev: Option<DdiApiRev>, sess_id: Option<u16>) -> DdiDeleteKeyCmdResp {
        DdiDeleteKeyCmdResp {
            hdr: DdiRespHdr {
                rev,
                op: DdiOp::DeleteKey,
                sess_id,
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiDeleteKeyResp {},
        }
    }
}
