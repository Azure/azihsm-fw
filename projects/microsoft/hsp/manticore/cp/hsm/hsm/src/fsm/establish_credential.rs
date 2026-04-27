// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::lm_key_derive::BK3_SIZE_BYTES;

use super::*;

type EstablishCredentialCmdCtx<E> = EstablishCredentialCtx<HsmPartitionEnv<E>>;

/// FSM states
#[derive(Clone, Copy, PartialEq, Eq)]
enum State {
    /// Initial state
    Init,

    /// Waiting for PKA Engine
    WaitForEngine,

    /// Wait for PKA command complete
    WaitForCmd,

    /// Final state
    Final,
}

/// Establish credential command
pub(crate) struct EstablishCredentialCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Response DMA buffer
    resp_buf: Option<DmaBuffer<E>>,

    /// Response DMA buffer
    resp: Option<DdiEstablishCredentialCmdResp>,

    /// Api rev
    api_rev: Option<DdiApiRev>,

    /// Command context
    cmd_ctx: Option<EstablishCredentialCmdCtx<E>>,

    /// Flag indicating whether command logic has been executed
    committed: bool,

    /// Decoded request
    decoded_req: Option<DdiEstablishCredentialCmdReq>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for EstablishCredentialCmd<E> {
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp_buf.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (State::WaitForEngine, HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)) => {
                self.begin_establish_credential(tag)
            }
            (State::WaitForCmd, HsmFsmEvent::PkaDone(_))
            | (State::WaitForCmd, HsmFsmEvent::PkaError(_)) => self.on_cmd_complete(),
            (State::Final, _) => Err(HsmErr::InvalidState),
            (_, _) => Err(HsmErr::InvalidEvent),
        }
    }

    /// Check if the command requires resource
    fn requires_resource(&self, _tag: TagId, _res_id: ResId) -> bool {
        true
    }

    /// Acquire a resource
    fn acquire_resource(&mut self, _tag: TagId, _res_id: ResId) -> HsmFsmEvent {
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    }

    /// Check if the command needs to be retried
    fn retry(&self) -> bool {
        !self.committed
    }

    /// Perform any rollback in case of error
    fn rollback(&mut self, _tag: TagId) -> HsmResult<()> {
        if self.committed {
            let _ = self.part.clear_credentials();
            let _ = self.part.clear_provisioning_state();
            self.committed = false;
        }

        Ok(())
    }
}

impl<E: HsmEnvTrait> EstablishCredentialCmd<E> {
    /// Create a new command FSM
    pub fn new(req: DmaBuffer<E>, heap: DmaHeap<E>, part: E::Partition) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            req,
            resp_buf: None,
            resp: None,
            api_rev: None,
            cmd_ctx: None,
            committed: false,
            decoded_req: None,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // Decode the request
        let req = decode_buf::<DdiEstablishCredentialCmdReq, E>(&self.req)?;

        if req.hdr.rev.is_none() {
            Err(HsmErr::UnsupportedRevision)?;
        }

        self.api_rev = req.hdr.rev;

        // Fail Fast by doing the nonce check here. We still need to check this again when all
        // crypto finishes and we are ready to establish the credential as another FSM could
        // have reached there first.
        self.part
            .verify_nonce(req.data.encrypted_credential.nonce)?;

        // Fail fast: lengths exactly 16 for encrypted_id, encrypted_pin, iv
        self.validate_establish_cred(&req.data.encrypted_credential)?;

        // Fail Fast by checking creds have not been set. We still need to check this again when all
        // crypto finishes and we are ready to establish the credential as another FSM could
        // have reached there first.
        self.part.verify_cred_is_not_set()?;

        self.decoded_req = Some(req);

        self.begin_establish_credential(tag)
    }

    /// Begin establish credential
    fn begin_establish_credential(&mut self, tag: TagId) -> HsmResult<()> {
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;
        let pub_key = &decoded_req.data.pub_key.der;
        let pota_pub_key = &decoded_req.data.pota_pub_key.der;

        match self
            .part
            .begin_establish_credential(tag, &(pub_key).into(), &(pota_pub_key).into())
        {
            Ok(ctx) => {
                self.cmd_ctx = Some(ctx);
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }
            Err(err) if err.pending() && self.state == State::WaitForEngine => {
                error!(
                    "[establish_credential] on_engine_ready begin_establish_credential err: {:?}",
                    u32::from(err)
                );
                self.state = State::Final;

                Err(HsmErr::InvalidState)
            }
            Err(err) if err.pending() => {
                self.state = State::WaitForEngine;
                Err(err)
            }
            Err(err) => {
                self.state = State::Final;
                Err(err)
            }
        }
    }

    /// On command complete
    fn on_cmd_complete(&mut self) -> Result<(), HsmErr> {
        let ctx = self.cmd_ctx.take().ok_or(HsmErr::InvalidState)?;

        match ctx.state {
            EstablishCredentialCmdState::MontgomeryConstCalc
            | EstablishCredentialCmdState::PublicKeyValidation
            | EstablishCredentialCmdState::PotaPublicKeyValidation
            | EstablishCredentialCmdState::VerifySignature
            | EstablishCredentialCmdState::SecondMontgomeryConstCalc => self.on_continue(ctx),
            EstablishCredentialCmdState::EcdhCompute => self.on_end(ctx),
        }
    }

    /// Continue establish credential
    fn on_continue(&mut self, ctx: EstablishCredentialCmdCtx<E>) -> Result<(), HsmErr> {
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;
        let pub_key = &decoded_req.data.pub_key.der;
        let pota_pub_key = &decoded_req.data.pota_pub_key.der;
        let pota_sig = &decoded_req.data.pota_sig;

        match self.part.continue_establish_credential(
            ctx,
            &(pub_key).into(),
            &(pota_pub_key).into(),
            &(pota_sig).into(),
        ) {
            Ok(ctx) => {
                self.cmd_ctx = Some(ctx);
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }
            Err(mut err) => {
                if err.pending() {
                    // Pending error unexpected here
                    err = HsmErr::InvalidState;
                }
                self.state = State::Final;
                Err(err)
            }
        }
    }

    /// End establish credential
    fn on_end(&mut self, ctx: EstablishCredentialCmdCtx<E>) -> Result<(), HsmErr> {
        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;

        match self
            .part
            .end_establish_credential(ctx, &decoded_req.data.encrypted_credential)
        {
            Ok(_) => {
                // Command operation has succeeded; set committed flag to avoid retry
                self.committed = true;

                self.provision_partition_and_send_response()?;

                Ok(())
            }
            Err(mut err) => {
                if err.pending() {
                    // Pending error unexpected here
                    err = HsmErr::InvalidState;
                }
                self.state = State::Final;
                Err(err)
            }
        }
    }

    #[inline]
    fn validate_establish_cred(
        &mut self,
        c: &DdiEncryptedEstablishCredential,
    ) -> Result<(), HsmErr> {
        if c.encrypted_id.len() != 16 || c.encrypted_pin.len() != 16 || c.iv.len() != 16 {
            return Err(HsmErr::InvalidArgument);
        }

        Ok(())
    }

    /// Main logic to provision the partition and send response
    fn provision_partition_and_send_response(&mut self) -> Result<(), HsmErr> {
        if self.part.is_partition_provisioned() {
            Err(HsmErr::PartitionAlreadyProvisioned)?
        }

        let decoded_req = self.decoded_req.as_ref().ok_or(HsmErr::InvalidState)?;

        // 1. Unmask the BK3 key
        let mut bk3_buf = self
            .heap
            .allocate(BK3_SIZE_BYTES)
            .ok_or(HsmErr::DmaAllocFailure)?;
        let masked_bk3 = decoded_req.data.masked_bk3.as_slice();
        self.part.unmask_bk3(masked_bk3, bk3_buf.as_ref_mut())?;

        // 2. Generate BK3 session key and store it
        self.part.generate_and_store_bk3_session(bk3_buf.as_ref())?;

        // 3. Generate BK.
        let mut bk_buf = self
            .heap
            .allocate(BK_AES_CBC_256_HMAC384_SIZE_BYTES)
            .ok_or(HsmErr::DmaAllocFailure)?;
        self.part.generate_bk(
            bk3_buf.as_ref(),
            decoded_req.data.pota_pub_key.der.as_slice(),
            bk_buf.as_ref_mut(),
        )?;

        // 4. Next steps totally depends on if we have BMK or not.
        // On either cases, we import a masking key to the vault which indicates
        // that the partition is now provisioned.
        if decoded_req.data.bmk.is_empty() {
            self.part.generate_new_mk_and_import()?;
        } else {
            let bmk = decoded_req.data.bmk.as_slice();
            self.part.import_mk_from_bmk(
                bk3_buf.as_ref(),
                decoded_req.data.pota_pub_key.der.as_slice(),
                bmk,
            )?;
        }

        // 5. Prepare the response and generate bmk with current SVN
        let mut bmk_len = 0;
        match self
            .part
            .generate_bmk(bk_buf.as_ref(), &mut bmk_len, &mut [])
        {
            Err(HsmErr::InsufficientBuffer) => (),
            other => other?,
        }

        // 6. Unmask the masked_unwrapping_key if available
        if !decoded_req.data.masked_unwrapping_key.is_empty() {
            self.part.unmask_unwrapping_key_and_import(
                decoded_req.data.masked_unwrapping_key.as_slice(),
            )?;
        }

        self.prepare_response(bmk_len)?;

        let resp = self.resp.as_ref().ok_or(HsmErr::InvalidState)?;
        let mut bmk: IoMemRange = (&resp.data.bmk).into();
        self.part
            .generate_bmk(bk_buf.as_ref(), &mut bmk_len, bmk.slice_mut())?;

        Ok(())
    }

    /// Helper to prepare response
    fn prepare_response(&mut self, bmk_len: usize) -> Result<(), HsmErr> {
        let resp = self.cmd_resp(bmk_len);
        self.resp_buf = Some(encode_buf(&resp, &self.heap)?);
        self.resp = Some(resp);

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(&self, bmk_len: usize) -> DdiEstablishCredentialCmdResp {
        DdiEstablishCredentialCmdResp {
            hdr: DdiRespHdr {
                rev: self.api_rev,
                op: DdiOp::EstablishCredential,
                sess_id: None,
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiEstablishCredentialResp {
                bmk: MborByteArray::new_with_len(core::ptr::null(), bmk_len),
            },
        }
    }
}
