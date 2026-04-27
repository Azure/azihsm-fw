// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaEccCurve;

use super::*;

/// FSM states
#[derive(Clone, Copy)]
enum State {
    /// Initial state
    Init,

    /// Waiting for PKA Engine
    WaitForEngine,

    /// Wait for PKA operation
    WaitForCmd,

    /// Final state
    Final,
}

/// Open key command
pub(crate) struct OpenKeyCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    /// Session
    session: E::UserSession,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Response DMA buffer
    resp_buf: Option<DmaBuffer<E>>,

    /// Response
    resp: Option<DdiOpenKeyCmdResp>,

    /// Request key tag
    key_tag: Option<u16>,

    /// Request key ID
    key_id: Option<KeyId>,

    /// Request key kind
    key_kind: Option<EntryKind>,

    /// Current open key operation phase
    phase: OpenKeyPhase,

    /// ECC operation
    ecc_op: Option<EccGenPubKeyCmd<E>>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for OpenKeyCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp_buf.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd)
            | (State::WaitForEngine, HsmFsmEvent::ResourceReady(_))
            | (State::WaitForCmd, HsmFsmEvent::PkaDone(_))
            | (State::WaitForCmd, HsmFsmEvent::PkaError(_)) => self.on_trigger(tag),
            (State::Final, _) => {
                error!(
                    "[open_key] Invalid State, state:{:?} event: {:?}",
                    self.state as u32,
                    u32::from(event)
                );
                Err(HsmErr::InvalidState)
            }
            (_, _) => {
                error!(
                    "[open_key] Invalid Event, state:{:?} event: {:?}",
                    self.state as u32,
                    u32::from(event)
                );
                Err(HsmErr::InvalidEvent)
            }
        }
    }

    /// Get the session ID this command FSM operates on-behalf of
    fn session_id(&self) -> Option<u16> {
        Some(self.session.id())
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

impl<E: HsmEnvTrait> OpenKeyCmd<E> {
    /// Create a new command FSM
    pub fn new(
        req: DmaBuffer<E>,
        heap: DmaHeap<E>,
        session: E::UserSession,
        part: E::Partition,
    ) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            session,
            req,
            resp_buf: None,
            resp: None,
            key_tag: None,
            key_id: None,
            key_kind: None,
            phase: OpenKeyPhase::default(),
            ecc_op: None,
        }
    }

    #[inline(always)]
    fn is_ecc_priv(&mut self, kind: EntryKind) -> bool {
        matches!(
            kind,
            EntryKind::Ecc256Private | EntryKind::Ecc384Private | EntryKind::Ecc521Private
        )
    }

    /// Handle the event
    fn on_trigger(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // Decode the request
        if self.key_tag.is_none() {
            let req = decode_buf::<DdiOpenKeyCmdReq, E>(&self.req)?;

            let data = &req.data;
            self.key_tag = Some(data.key_tag);
        }
        let key_tag = self.key_tag.ok_or(HsmErr::InvalidState)?;

        let mut der_range_opt: Option<IoMemRange> = None;
        if let Some(resp) = self.resp.as_ref() {
            if let Some(ddi_pub_key) = resp.data.pub_key.as_ref() {
                der_range_opt = Some((&ddi_pub_key.der).into());
            }
        }

        let empty_range = IoMemRange::from(&[] as &[u8]);
        let pub_key_ref: &IoMemRange = der_range_opt.as_ref().unwrap_or(&empty_range);

        let output = self.session.open_key_zc(
            tag,
            key_tag,
            self.key_id,
            self.key_kind,
            self.phase,
            false,
            &mut self.ecc_op,
            pub_key_ref,
        )?;

        self.key_id = Some(output.id);
        self.key_kind = Some(output.kind);
        self.phase = output.phase;

        match self.phase {
            OpenKeyPhase::PendingUpkaEngine => {
                self.state = State::WaitForEngine;
                Err(HsmErr::Pending)
            }
            OpenKeyPhase::PendingMontgomeryConstCalc => {
                if self.resp_buf.is_none() && self.is_ecc_priv(output.kind) {
                    self.create_cmd_response(output.id, output.kind, None, None)?;
                }
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }
            OpenKeyPhase::PendingPointMultiplication => {
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }
            OpenKeyPhase::Done => {
                if self.resp_buf.is_none() {
                    self.create_cmd_response(
                        output.id,
                        output.kind,
                        output.pub_key,
                        output.bulk_key_id,
                    )?;
                }
                self.state = State::Final;
                Ok(())
            }
            _ => {
                self.state = State::Final;
                Err(HsmErr::InvalidState)
            }
        }
    }

    fn create_cmd_response(
        &mut self,
        key_id: u16,
        key_kind: EntryKind,
        pub_key: Option<PublicKey>,
        bulk_key_id: Option<u16>,
    ) -> Result<(), HsmErr> {
        // Decide public key DDI representation
        let mut pub_key_ddi: Option<DdiDerPublicKey> = None;

        if let Some(pk) = pub_key.as_ref() {
            let pub_key_slice = pk.pka_as_slice()?;
            pub_key_ddi = Some(DdiDerPublicKey {
                der: MborByteArray::new_with_len(pub_key_slice.as_ptr(), pub_key_slice.len()),
                key_kind: pk.ddi_key_type(),
            });
        } else if self.is_ecc_priv(key_kind) {
            let (curve, pub_type) = match key_kind {
                EntryKind::Ecc256Private => (PkaEccCurve::Ecc256, DdiKeyType::Ecc256Public),
                EntryKind::Ecc384Private => (PkaEccCurve::Ecc384, DdiKeyType::Ecc384Public),
                EntryKind::Ecc521Private => (PkaEccCurve::Ecc521, DdiKeyType::Ecc521Public),
                _ => return Err(HsmErr::InvalidArgument),
            };
            pub_key_ddi = Some(DdiDerPublicKey {
                der: MborByteArray::new_with_len(core::ptr::null(), curve.len() * 2),
                key_kind: pub_type,
            });
        }

        let resp = self.cmd_resp(
            self.session.api_rev(),
            self.session.id(),
            key_id,
            key_kind.try_into()?,
            pub_key_ddi,
            bulk_key_id,
        );
        self.resp_buf = Some(encode_buf(&resp, &self.heap)?);
        self.resp = Some(resp);

        Ok(())
    }

    /// Create a command response
    fn cmd_resp(
        &self,
        rev: DdiApiRev,
        sess_id: u16,
        key_id: u16,
        key_kind: DdiKeyType,
        pub_key: Option<DdiDerPublicKey>,
        bulk_key_id: Option<u16>,
    ) -> DdiOpenKeyCmdResp {
        DdiOpenKeyCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::OpenKey,
                sess_id: Some(sess_id),
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiOpenKeyResp {
                key_id,
                key_kind,
                pub_key,
                bulk_key_id,
            },
        }
    }
}
