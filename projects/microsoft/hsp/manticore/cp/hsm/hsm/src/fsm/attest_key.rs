// Copyright (c) Microsoft Corporation. All rights reserved.

//! Implementation of the `AttestKey` command.

use key_attestation::report::REPORT_DATA_SIZE;
use key_attestation::report::SIGNATURE_SIZE;

use super::*;
use crate::key_attestation::cose_key::CoseKeyEncoderTrait;
use crate::key_attestation::key_attestation_data::KeyAttestationData;
use mcr_crypto_pka::*;

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

/// Attest Key command
pub(crate) struct AttestKeyCmd<E: HsmEnvTrait + 'static> {
    /// Current state
    state: State,

    /// DMA heap
    heap: DmaHeap<E>,

    /// Partition
    part: E::Partition,

    // session id
    session_id: u16,

    /// Request DMA buffer
    req: DmaBuffer<E>,

    /// Response DMA buffer
    resp: Option<DmaBuffer<E>>,

    /// Key id of the target key
    key_id: Option<u16>,

    /// User data to be included in the report
    report_data: Option<MborByteArray<REPORT_DATA_SIZE>>,

    /// Current open key operation phase
    open_key_phase: OpenKeyPhase,

    /// ECC Sign Operation
    ecc_op: Option<EccSign<E>>,

    /// Key attestation data
    key_attestation_data: KeyAttestationData,

    /// ECC operation
    ecc_op_for_open: Option<EccGenPubKeyCmd<E>>,

    /// Signature DMA buffer
    signature_dma_buff: Option<DmaBuffer<E>>,

    /// TBS DMA buffer
    tbs_dma_buff: Option<DmaBuffer<E>>,

    /// Public Key buffer
    pub_key_dma_buff: Option<DmaBuffer<E>>,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for AttestKeyCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (State::WaitForEngine, HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)) => {
                self.on_engine_ready(tag)
            }
            (State::WaitForCmd, HsmFsmEvent::PkaDone(_))
            | (State::WaitForCmd, HsmFsmEvent::PkaError(_)) => self.on_cmd_complete(tag),
            (State::Final, _) => {
                error!(
                    "[attest_key] Invalid State, state:{:?}, event: {:?}",
                    self.state as u32,
                    u32::from(event)
                );
                Err(HsmErr::InvalidState)
            }
            (_, _) => {
                error!(
                    "[attest_key] Invalid Event, state:{:?}, event: {:?}",
                    self.state as u32,
                    u32::from(event)
                );
                Err(HsmErr::InvalidEvent)
            }
        }
    }

    /// Get the session ID
    fn session_id(&self) -> Option<u16> {
        Some(self.session_id)
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

impl<E: HsmEnvTrait> AttestKeyCmd<E> {
    /// Create a new command FSM
    pub fn new(req: DmaBuffer<E>, heap: DmaHeap<E>, part: E::Partition, session_id: u16) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            session_id,
            req,
            resp: None,
            key_id: None,
            report_data: None,
            open_key_phase: OpenKeyPhase::default(),
            ecc_op: None,
            key_attestation_data: KeyAttestationData::new(),
            ecc_op_for_open: None,
            signature_dma_buff: None,
            tbs_dma_buff: None,
            pub_key_dma_buff: None,
        }
    }

    /// Handle the start event
    fn on_start(&mut self, tag: TagId) -> HsmResult<()> {
        let req = decode_buf::<DdiAttestKeyCmdReq, E>(&self.req)?;
        let data = req.data;

        self.key_id = Some(data.key_id);
        self.report_data = Some(data.report_data);

        // Allocate the memory for the ECC public key
        let pub_key_dma_buff = self
            .heap
            .allocate(PkaEccCurve::MAX_LEN * 2)
            .ok_or(HsmErr::DmaAllocFailure)?;
        self.pub_key_dma_buff = Some(pub_key_dma_buff);

        // Start retrieving the target key information via `open_key`
        self.open_key(tag)?;

        Err(HsmErr::Pending)
    }

    /// Handle the PKA Engine ready event
    fn on_engine_ready(&mut self, tag: TagId) -> HsmResult<()> {
        if !self.key_attestation_data.ready_to_sign {
            self.open_key(tag)?;

            Err(HsmErr::Pending)
        } else {
            match self.begin_report_sign(tag) {
                Ok(op) => {
                    self.ecc_op = Some(op);
                    self.state = State::WaitForCmd;
                    Err(HsmErr::Pending)?
                }
                Err(mut err) => {
                    if err.pending() {
                        err = HsmErr::InvalidState;
                    }
                    self.state = State::Final;
                    Err(err)?
                }
            }
        }
    }

    /// Handle the PKA command done event
    fn on_cmd_complete(&mut self, tag: TagId) -> HsmResult<()> {
        if !self.key_attestation_data.ready_to_sign {
            self.open_key(tag)?;

            Err(HsmErr::Pending)
        } else {
            let op = self.ecc_op.take().ok_or(HsmErr::InvalidState)?;
            self.end_report_sign(tag, op)?;

            // Retrieve the signature from the buffer
            let signature_dma_buff = self.signature_dma_buff.take().ok_or(HsmErr::InvalidState)?;
            let signature_bytes = signature_dma_buff.as_ref();
            if signature_bytes.len() != SIGNATURE_SIZE {
                Err(HsmErr::EccSignFailed)?
            }

            let r = &signature_bytes[..48];
            let s = &signature_bytes[48..];

            let signature = {
                let mut signature = [0u8; 96];
                signature[..r.len()].copy_from_slice(r);
                signature[..r.len()].reverse();

                signature[r.len()..].copy_from_slice(s);
                signature[r.len()..].reverse();

                signature
            };

            let quote_len = self.key_attestation_data.encode_len();

            // Encode and save the buffer
            let resp = self.cmd_resp(self.get_session()?.api_rev(), self.session_id, quote_len);
            self.resp = Some(encode_buf(&resp, &self.heap)?);
            let mut quote_mborbytearray = resp.data.report;

            self.key_attestation_data
                .encode(signature, quote_mborbytearray.as_mut_slice())?;

            Ok(())
        }
    }

    /// Open key
    fn open_key(&mut self, tag: TagId) -> HsmResult<()> {
        let pub_key_range: IoMemRange = self.pub_key_dma_buff.as_ref().unwrap().as_ref().into();
        let unwrapping_key_id = self.part.unwrapping_key_id();

        // Allow unwrapping key to be attestable
        let is_unwrapping_key = unwrapping_key_id.is_some() && unwrapping_key_id == self.key_id;
        let output = self.get_session()?.open_key_zc(
            tag,
            0,
            self.key_id,
            None,
            self.open_key_phase,
            is_unwrapping_key,
            &mut self.ecc_op_for_open,
            &pub_key_range,
        )?;
        self.open_key_phase = output.phase;

        match self.open_key_phase {
            OpenKeyPhase::PendingUpkaEngine => {
                self.state = State::WaitForEngine;
            }
            OpenKeyPhase::PendingMontgomeryConstCalc | OpenKeyPhase::PendingPointMultiplication => {
                self.state = State::WaitForCmd;
            }
            OpenKeyPhase::Done => {
                self.generate_report(tag, &output)?;
            }
            _ => {
                self.state = State::Final;
                Err(HsmErr::InvalidState)?
            }
        }

        Ok(())
    }

    /// Helper function for SHA
    #[inline(always)]
    fn compute_sha(
        &self,
        digest_kind: ShaType,
        buf: &[u8],
        output: &mut IoMemRange,
    ) -> HsmResult<()> {
        let input_dma = self
            .heap
            .copy_allocate(buf)
            .ok_or(HsmErr::DmaAllocFailure)?;
        let input_range: IoMemRange = input_dma.as_ref().into();

        self.get_session()?
            .sha_single_block_zc(digest_kind, &input_range, output)?;

        Ok(())
    }

    /// Begin attestation report generation
    fn generate_report(&mut self, tag: TagId, open_key_data: &OpenKeyData) -> HsmResult<()> {
        // For ECC private keys, construct the public key from DMA buffer
        let ecc_public_key = match open_key_data.kind {
            // special handling for ECC private keys.
            EntryKind::Ecc256Private | EntryKind::Ecc384Private | EntryKind::Ecc521Private => {
                let pub_key_dma_buff = self.pub_key_dma_buff.take().ok_or(HsmErr::InvalidState)?;
                let pub_key_slice = pub_key_dma_buff.as_ref();

                let curve = match open_key_data.kind {
                    EntryKind::Ecc256Private => PkaEccCurve::Ecc256,
                    EntryKind::Ecc384Private => PkaEccCurve::Ecc384,
                    EntryKind::Ecc521Private => PkaEccCurve::Ecc521,
                    _ => return Err(HsmErr::InvalidArgument),
                };

                Some(PublicKey::EccPubKey(PkaEccPublicKey {
                    curve,
                    data: {
                        let mut data = [0; PkaEccCurve::MAX_LEN * 2];
                        data[..pub_key_slice.len()].copy_from_slice(pub_key_slice);
                        data
                    },
                }))
            }
            _ => None,
        };

        // Determine the public key reference: use constructed ECC key, or existing pub_key, or None
        let public_key = ecc_public_key.as_ref().or(open_key_data.pub_key.as_ref());

        // For asymmetric keys, encode the public key; for other keys, use empty encoded key
        let (encoded_key, encoded_key_len) = match public_key {
            Some(key) => key.to_cose_key()?,
            None => ([0u8; 525], 0u16),
        };
        let app_id = self.get_session()?.app_id();
        let report_data = self
            .report_data
            .as_ref()
            .ok_or(HsmErr::AttestKeyInternalErr)?;
        let report_array: [u8; REPORT_DATA_SIZE] = report_data
            .as_slice()
            .try_into()
            .map_err(|_| HsmErr::InvalidArgument)?;

        let vm_launch_id = self.part.vm_launch_guid();

        self.key_attestation_data.create_report_payload(
            &encoded_key,
            encoded_key_len,
            open_key_data.flags.into(),
            app_id,
            &report_array,
            &vm_launch_id,
        )?;

        match self.begin_report_sign(tag) {
            Ok(op) => {
                self.ecc_op = Some(op);
                self.state = State::WaitForCmd;
                Err(HsmErr::Pending)
            }
            Err(err) => {
                if err.pending() {
                    self.state = State::WaitForEngine;
                } else {
                    self.state = State::Final
                }
                Err(err)
            }
        }
    }

    /// Begin report signing
    fn begin_report_sign(&mut self, tag: TagId) -> HsmResult<EccSign<E>> {
        let payload = &self.key_attestation_data.payload[..self.key_attestation_data.payload_len];
        // Create the to-be-signed data blob.
        let (tbs_buffer, tbs_len) = crate::key_attestation::cose_sign1::create_tbs(
            &self.key_attestation_data.protected_header,
            payload,
        )?;

        // Allocate DMA buffer for the TBS hash for Max length for SHA384
        let mut tbs_hash_dma_buff = self
            .heap
            .allocate(PkaEccCurve::MAX_LEN)
            .ok_or(HsmErr::DmaAllocFailure)?;
        let mut tbs_hash_mem_range: IoMemRange = tbs_hash_dma_buff.as_ref().into();

        self.compute_sha(
            ShaType::Sha384,
            &tbs_buffer[..tbs_len],
            &mut tbs_hash_mem_range,
        )?;
        tbs_hash_dma_buff.as_ref_mut()[..ShaType::Sha384 as usize].reverse();
        self.tbs_dma_buff = Some(tbs_hash_dma_buff);

        let signature_dma_buff = self
            .heap
            .allocate(SIGNATURE_SIZE)
            .ok_or(HsmErr::DmaAllocFailure)?;
        let signature_mem_range: IoMemRange = signature_dma_buff.as_ref().into();
        self.signature_dma_buff = Some(signature_dma_buff);

        let part_priv_key_blob = self
            .part
            .get_partition_id_private_key_blob()
            .ok_or(HsmErr::AttestKeyInternalErr)?;

        // Begin ECC sign
        self.get_session()?.begin_ecc_sign_zc(
            tag,
            EccKeyIn::KeyBlobAndCurve(part_priv_key_blob, EccCurve::P384),
            &tbs_hash_mem_range,
            DdiHashAlgorithm::Sha384,
            &signature_mem_range,
        )
    }

    /// End attestation report signing
    fn end_report_sign(&mut self, tag: TagId, op: EccSign<E>) -> HsmResult<()> {
        self.get_session()?.end_ecc_sign_zc(tag, op)?;
        Ok(())
    }

    /// Create a command response
    fn cmd_resp(&self, rev: DdiApiRev, sess_id: u16, report_len: usize) -> DdiAttestKeyCmdResp {
        DdiAttestKeyCmdResp {
            hdr: DdiRespHdr {
                rev: Some(rev),
                op: DdiOp::EccSign,
                sess_id: Some(sess_id),
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiAttestKeyResp {
                report: MborByteArray::new_with_len(core::ptr::null(), report_len),
            },
        }
    }

    /// Get the session
    fn get_session(&self) -> HsmResult<E::UserSession> {
        self.part.user_session(self.session_id, false)
    }
}
