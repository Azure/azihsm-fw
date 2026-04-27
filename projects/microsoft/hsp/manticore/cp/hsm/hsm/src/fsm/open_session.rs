// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::lm_key_derive::MK_AES_CBC_256_HMAC384_SIZE_BYTES;

use super::*;

type OpenSessionCmdCtx<E> = OpenSessionCtx<HsmPartitionEnv<E>>;

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

struct DdiCommonField {
    /// API revision (optional)
    pub rev: Option<DdiApiRev>,

    /// Session ID (optional)
    pub sess_id: Option<u16>,

    /// Encrypted credential
    pub encrypted_credential: DdiEncryptedSessionCredential,

    /// Public Key (ECC 384)
    pub pub_key: DdiDerPublicKey,

    /// Backed up Session masking key
    pub bmk_session: Option<MborByteArray<1024>>,
}

/// Open session command
pub(crate) struct OpenSessionCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,

    /// Session ID
    sess_id: Option<u16>,

    /// App vault ID
    app_vault_id: Option<u8>,

    /// Open Session command context
    cmd_ctx: Option<OpenSessionCmdCtx<E>>,

    /// Api rev
    api_rev: Option<DdiApiRev>,

    /// Decoded request common field
    decoded_req_common: Option<DdiCommonField>,

    /// Flag to indicate if the OP is ReopenSession or not
    is_reopen: bool,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for OpenSessionCmd<E> {
    /// Take the response buffer if it is available
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (State::WaitForEngine, HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)) => {
                self.begin_open_session(tag)
            }
            (State::WaitForCmd, HsmFsmEvent::PkaDone(_))
            | (State::WaitForCmd, HsmFsmEvent::PkaError(_)) => self.on_cmd_complete(tag),
            (State::Final, _) => Err(HsmErr::InvalidState),
            (_, _) => Err(HsmErr::InvalidEvent),
        }
    }

    /// Get the session ID this command FSM operates on-behalf of
    fn session_id(&self) -> Option<u16> {
        self.sess_id
    }

    /// Get the app vault ID this command FSM operates on-behalf of
    fn app_vault_id(&self) -> Option<u8> {
        self.app_vault_id
    }

    /// Perform any rollback in case of error
    fn rollback(&mut self, _tag: TagId) -> HsmResult<()> {
        if let Some(sess_id) = self.sess_id {
            self.part.rollback_open_session(sess_id, self.is_reopen)
        } else {
            Ok(())
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
}

impl<E: HsmEnvTrait> OpenSessionCmd<E> {
    /// Create a new command FSM
    pub fn new(req: DmaBuffer<E>, heap: DmaHeap<E>, part: E::Partition, is_reopen: bool) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            req,
            resp: None,
            sess_id: None,
            app_vault_id: None,
            cmd_ctx: None,
            api_rev: None,
            decoded_req_common: None,
            is_reopen,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // Fail Fast by checking creds have been set so we don't even attempt crypto
        self.part.verify_cred_is_set()?;

        // Decode the request
        if self.is_reopen {
            let req = decode_buf::<DdiReopenSessionCmdReq, E>(&self.req)?;
            if req.data.bmk_session.is_empty() {
                Err(HsmErr::InvalidState)?;
            }
            // Fail fast: lengths exactly 16 for encrypted_id, encrypted_pin, iv, and 48 for seed
            self.validate_session_cred(&req.data.encrypted_credential)?;

            self.decoded_req_common = Some(DdiCommonField {
                rev: req.hdr.rev,
                sess_id: req.hdr.sess_id,
                encrypted_credential: req.data.encrypted_credential,
                pub_key: req.data.pub_key,
                bmk_session: Some(req.data.bmk_session),
            });
        } else {
            let req = decode_buf::<DdiOpenSessionCmdReq, E>(&self.req)?;
            // Fail fast: lengths exactly 16 for encrypted_id, encrypted_pin, iv, and 48 for seed
            self.validate_session_cred(&req.data.encrypted_credential)?;

            self.decoded_req_common = Some(DdiCommonField {
                rev: req.hdr.rev,
                sess_id: req.hdr.sess_id,
                encrypted_credential: req.data.encrypted_credential,
                pub_key: req.data.pub_key,
                bmk_session: None,
            });
        }

        let rev = self
            .decoded_req_common
            .as_ref()
            .and_then(|val| val.rev)
            .ok_or(HsmErr::UnsupportedRevision)?;
        self.api_rev = Some(rev);

        self.begin_open_session(tag)
    }

    /// Begin open session
    fn begin_open_session(&mut self, tag: TagId) -> HsmResult<()> {
        let pub_key = &self
            .decoded_req_common
            .as_ref()
            .ok_or(HsmErr::InvalidState)?
            .pub_key
            .der;

        match self.part.begin_open_user_session(tag, &(pub_key).into()) {
            Ok(ctx) => {
                self.cmd_ctx = Some(ctx);
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }
            Err(err) if err.pending() && self.state == State::WaitForEngine => {
                error!(
                    "[open_session] on_engine_ready: begin_open_session err: {:?}",
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

    /// Continue open session
    fn on_cmd_complete(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let ctx = self.cmd_ctx.take().ok_or(HsmErr::InvalidState)?;

        match ctx.state {
            OpenSessionCmdState::MontgomeryConstCalc | OpenSessionCmdState::PublicKeyValidation => {
                self.on_continue(ctx)
            }
            OpenSessionCmdState::EcdhCompute => self.on_end(tag, ctx),
        }
    }

    /// Continue open session
    fn on_continue(&mut self, ctx: OpenSessionCmdCtx<E>) -> Result<(), HsmErr> {
        let pub_key_der = &self
            .decoded_req_common
            .as_ref()
            .ok_or(HsmErr::InvalidState)?
            .pub_key
            .der;

        match self
            .part
            .continue_open_user_session(ctx, &(pub_key_der).into())
        {
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

    /// end open session
    fn on_end(&mut self, _tag: TagId, ctx: OpenSessionCmdCtx<E>) -> Result<(), HsmErr> {
        let rev = self.api_rev.ok_or(HsmErr::InvalidState)?;
        let common_field = self
            .decoded_req_common
            .as_ref()
            .ok_or(HsmErr::InvalidState)?;
        let encrypted_credential = &common_field.encrypted_credential;

        let bmk_session = common_field.bmk_session.as_ref().map(|arr| arr.as_slice());

        // Allocate buffer for BK session and MK session
        let mut bk_buf = self
            .heap
            .allocate(BK_AES_CBC_256_HMAC384_SIZE_BYTES)
            .ok_or(HsmErr::DmaAllocFailure)?;

        let mut mk_buf = self
            .heap
            .allocate(MK_AES_CBC_256_HMAC384_SIZE_BYTES)
            .ok_or(HsmErr::DmaAllocFailure)?;

        let reopen_sess_id = if self.is_reopen {
            common_field.sess_id
        } else {
            None
        };

        match self.part.end_open_user_session(
            ctx,
            rev,
            encrypted_credential,
            reopen_sess_id,
            bk_buf.as_ref_mut(),
            mk_buf.as_ref_mut(),
            bmk_session,
        ) {
            Ok(sess) => {
                // Note: This is needed for outer state machine for session hijack
                // protection.
                self.sess_id = Some(sess.id());

                // Save the app vault ID. Will be consumed by the HSM FSM
                self.app_vault_id = Some(sess.app_vault_id());

                #[cfg(feature = "mcr_test_hooks")]
                if self.is_reopen {
                    if let Some(action) = self.part.cmd_fsm_test_action(None) {
                        if action == DdiTestAction::TriggerIoFailure {
                            Err(HsmErr::InvalidKeyType)?;
                        } else {
                            let _ = self.part.hsm_fsm_test_action(Some(action));
                        }
                    }
                }

                // Generate bmk_session and send response
                self.generate_bmk_and_send_response(
                    Some(rev),
                    sess.id(),
                    sess.app_vault_id(),
                    bk_buf.as_ref(),
                    mk_buf.as_ref(),
                )?;

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
    fn validate_session_cred(&mut self, c: &DdiEncryptedSessionCredential) -> Result<(), HsmErr> {
        if c.encrypted_id.as_slice().len() != 16
            || c.encrypted_pin.as_slice().len() != 16
            || c.encrypted_seed.as_slice().len() != 48
            || c.iv.as_slice().len() != 16
        {
            return Err(HsmErr::InvalidArgument);
        }

        Ok(())
    }

    // Generate bmk_session and send response
    fn generate_bmk_and_send_response(
        &mut self,
        rev: Option<DdiApiRev>,
        sess_id: u16,
        short_app_id: u8,
        bk_buf: &[u8],
        masking_key: &[u8],
    ) -> Result<(), HsmErr> {
        let mut bmk_len = 0;
        match self
            .part
            .generate_bmk_session(bk_buf, masking_key, &mut bmk_len, &mut [])
        {
            Err(HsmErr::InsufficientBuffer) => (),
            other => other?,
        };

        if self.is_reopen {
            let resp = DdiReopenSessionCmdResp {
                hdr: DdiRespHdr {
                    rev,
                    op: DdiOp::ReopenSession,
                    sess_id: Some(sess_id),
                    status: DdiStatus::Success,
                    fips_approved: self.part.is_fips_approved(),
                },
                data: DdiReopenSessionResp {
                    sess_id,
                    short_app_id,
                    bmk_session: MborByteArray::new_with_len(core::ptr::null(), bmk_len),
                },
            };
            self.resp = Some(encode_buf(&resp, &self.heap)?);

            let mut bmk: IoMemRange = (&resp.data.bmk_session).into();
            self.part
                .generate_bmk_session(bk_buf, masking_key, &mut bmk_len, bmk.slice_mut())?;
        } else {
            let resp = DdiOpenSessionCmdResp {
                hdr: DdiRespHdr {
                    rev,
                    op: DdiOp::OpenSession,
                    sess_id: Some(sess_id),
                    status: DdiStatus::Success,
                    fips_approved: self.part.is_fips_approved(),
                },
                data: DdiOpenSessionResp {
                    sess_id,
                    short_app_id,
                    bmk_session: MborByteArray::new_with_len(core::ptr::null(), bmk_len),
                },
            };
            self.resp = Some(encode_buf(&resp, &self.heap)?);

            let mut bmk: IoMemRange = (&resp.data.bmk_session).into();
            self.part
                .generate_bmk_session(bk_buf, masking_key, &mut bmk_len, bmk.slice_mut())?;
        }

        Ok(())
    }
}
