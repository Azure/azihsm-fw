// Copyright (c) Microsoft Corporation. All rights reserved.

use x509::*;

use super::*;

type PartEnv<E> = <<E as env::HsmEnvTrait>::Partition as HsmPartition>::Env;

/// Max time tick count to wait for IPC resource
const MAX_RESOURCE_WAIT_TIME: u8 = 16;

/// FSM states
#[derive(Clone, Copy, Debug, PartialEq)]
enum State {
    /// Initial state
    Init,

    /// Waiting for HSP IPC Channel
    WaitForResource,

    /// Wait form completion of HSP IPC operation: GetCertChainLengths
    WaitForGetCertChainLengthsResponse,

    /// Wait for a resource to be ready for PID Cert generation
    WaitForResourceForPIDCert,

    /// Wait for PID cert response
    WaitForPIDCertResponse,

    /// Final state
    Final,
}

/// Get Certificate Chain Info command
pub(crate) struct GetCertChainInfoCmd<E: HsmEnvTrait + 'static> {
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

    /// API rev
    api_rev: Option<DdiApiRev>,

    /// Context for GetCertChainLengthsInfo
    cert_len_ctx: Option<GetCertLengthsContext<PartEnv<E>>>,

    /// PID certificate signing context
    cert_sign_ctx: Option<CertSignContext<PartEnv<E>>>,

    /// Raw Alias Key Buffer
    raw_alias_key: Option<DmaBuffer<E>>,

    /// Check Alive Counter
    check_alive_cnt: u8,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for GetCertChainInfoCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (
                State::WaitForResource,
                HsmFsmEvent::ResourceReady(HsmFsmResourceId::HspIpcChannel),
            ) => self.on_ipc_resource_ready(tag),
            (State::WaitForGetCertChainLengthsResponse, HsmFsmEvent::HspToHsmIpcResponse) => {
                self.continue_get_cert_chain_lengths(tag)
            }
            (
                State::WaitForResourceForPIDCert,
                HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            ) => self.begin_generate_pid_cert(tag),
            (State::WaitForPIDCertResponse, HsmFsmEvent::PkaDone(_)) => {
                self.end_generate_pid_cert(tag)
            }
            (_, HsmFsmEvent::CheckAlive) => self.check_alive(),
            (State::Final, _) => Err(HsmErr::InvalidState),
            (_, _) => Err(HsmErr::InvalidEvent),
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
            HsmFsmResourceId::HspIpcChannel => {
                HsmFsmEvent::ResourceReady(HsmFsmResourceId::HspIpcChannel)
            }
            _ => unreachable!(),
        }
    }
}

impl<E: HsmEnvTrait> GetCertChainInfoCmd<E> {
    /// Create a new command FSM
    pub fn new(req: DmaBuffer<E>, heap: DmaHeap<E>, part: E::Partition) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            req,
            resp: None,
            api_rev: None,
            cert_len_ctx: None,
            cert_sign_ctx: None,
            raw_alias_key: None,
            check_alive_cnt: 0,
        }
    }

    /// Handle the start event to get the certificate chain info
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let req = decode_buf::<DdiGetCertChainInfoCmdReq, E>(&self.req)?;

        let rev = req.hdr.rev.ok_or(HsmErr::UnsupportedRevision)?;
        self.api_rev = Some(rev);

        if req.data.slot_id != 0 {
            return Err(HsmErr::InvalidArgument);
        }

        self.begin_get_cert_chain_lengths(tag)
    }

    /// Handle the IPC resource ready
    fn on_ipc_resource_ready(&mut self, tag: TagId) -> Result<(), HsmErr> {
        self.begin_get_cert_chain_lengths(tag)
    }

    /// Get the lengths of individual certificates in the chain.
    fn begin_get_cert_chain_lengths(&mut self, tag: TagId) -> Result<(), HsmErr> {
        self.cert_len_ctx = Some(GetCertLengthsContext {
            cert_info: None,
            channel_ref: None,
        });

        match self.part.begin_get_dev_id_cert_chain_info(
            tag,
            self.cert_len_ctx.as_mut().ok_or(HsmErr::InvalidState)?,
        ) {
            Ok(_) => {
                if self
                    .cert_len_ctx
                    .as_ref()
                    .ok_or(HsmErr::InvalidState)?
                    .channel_ref
                    .is_some()
                {
                    self.state = State::WaitForGetCertChainLengthsResponse;

                    Err(HsmErr::Pending)
                } else {
                    self.resp = Some(encode_buf(
                        &self.cmd_resp(
                            self.api_rev,
                            self.cert_len_ctx
                                .as_ref()
                                .ok_or(HsmErr::InvalidState)?
                                .cert_info
                                .as_ref()
                                .ok_or(HsmErr::InvalidState)?,
                        ),
                        &self.heap,
                    )?);
                    self.state = State::Final;

                    Ok(())
                }
            }
            Err(mut err) => {
                if err.pending() && self.state == State::Init {
                    self.state = State::WaitForResource;
                } else if err.pending() && self.state == State::WaitForResource {
                    self.state = State::Final;
                    err = HsmErr::InvalidState;
                } else {
                    self.state = State::Final;
                }

                Err(err)
            }
        }
    }

    /// Handle the completion of the GetCertChainLengths IPC operation.
    fn continue_get_cert_chain_lengths(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let mut local_ctx = self.cert_len_ctx.take().ok_or(HsmErr::InvalidState)?;
        self.part.end_get_dev_id_cert_chain_info(&mut local_ctx)?;

        if !self.part.is_partition_cert_valid() {
            self.cert_len_ctx = Some(local_ctx);

            self.begin_generate_pid_cert(tag)
        } else {
            self.end_get_cert_chain_lengths(&mut local_ctx)
        }
    }

    /// Generate PID certificate for PID public key.
    fn begin_generate_pid_cert(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // Get the slice of the raw alias key blob
        let key_blob = match self.raw_alias_key.as_ref() {
            Some(raw_key) => raw_key.as_ref(),
            None => {
                let blob = self.part.get_raw_alias_key()?;
                let blob = self
                    .heap
                    .copy_allocate(&blob)
                    .ok_or(HsmErr::DmaAllocFailure)?;

                self.raw_alias_key = Some(blob);

                self.raw_alias_key
                    .as_ref()
                    .ok_or(HsmErr::InvalidAliasKey)?
                    .as_ref()
            }
        };

        match self.part.begin_generate_pid_cert(tag, key_blob) {
            Err(HsmErr::Pending) => {
                if self.state == State::WaitForResourceForPIDCert {
                    // We don't expect to wait for the PKA engine here, we should have already acquired it.
                    self.state = State::Final;
                    return Err(HsmErr::InvalidState);
                }

                self.state = State::WaitForResourceForPIDCert;

                Err(HsmErr::Pending)
            }
            Ok(cert_sign_ctx) => {
                self.state = State::WaitForPIDCertResponse;
                self.cert_sign_ctx = Some(cert_sign_ctx);
                Err(HsmErr::Pending)
            }
            Err(err) => {
                self.state = State::Final;
                Err(err)
            }
        }
    }

    /// End the PID certificate generation process.
    fn end_generate_pid_cert(&mut self, tag: TagId) -> Result<(), HsmErr> {
        self.state = State::Final;

        let cert_sign_ctx = self
            .cert_sign_ctx
            .take()
            .ok_or(HsmErr::PartitionCertGenerationFailed)?;

        match self.part.end_generate_pid_cert(tag, &cert_sign_ctx) {
            Ok(()) => {
                let signature = self
                    .part
                    .get_ecdsa384_signature_from_buffer(cert_sign_ctx.signature_buf.as_ref())?;

                let tbs = cert_sign_ctx.tbs;

                let builder = Ecdsa384CertBuilder::new(tbs.tbs(), &signature)
                    .ok_or(HsmErr::PartitionCertGenerationFailed)?;

                self.part.set_partition_cert_length(builder.len() as u32)?;

                let mut part_mem = self.part.partition_cert();
                let buf = part_mem.slice_mut();

                let _cert_size = builder
                    .build(buf)
                    .ok_or(HsmErr::PartitionCertGenerationFailed)?;

                self.part.set_partition_cert_valid(true);

                let mut local_ctx = self.cert_len_ctx.take().ok_or(HsmErr::InvalidState)?;
                self.end_get_cert_chain_lengths(&mut local_ctx)
            }
            Err(err) => Err(err),
        }
    }

    fn end_get_cert_chain_lengths(
        &mut self,
        cert_chain_len_ctx: &mut GetCertLengthsContext<
            <<E as env::HsmEnvTrait>::Partition as HsmPartition>::Env,
        >,
    ) -> Result<(), HsmErr> {
        let mut cert_info = cert_chain_len_ctx
            .cert_info
            .take()
            .ok_or(HsmErr::InvalidState)?;

        self.part.update_cert_chain_lengths_info(&mut cert_info)?;

        // Prepare the response buffer
        self.resp = Some(encode_buf(
            &self.cmd_resp(self.api_rev, &cert_info),
            &self.heap,
        )?);

        // Store this in the partition.
        self.part.set_cert_chain_lengths_info(Some(cert_info));

        Ok(())
    }

    /// On timer event response
    fn check_alive(&mut self) -> Result<(), HsmErr> {
        if (self.state == State::WaitForResource
            || self.state == State::WaitForGetCertChainLengthsResponse)
            && self.check_alive_cnt < MAX_RESOURCE_WAIT_TIME
        {
            self.check_alive_cnt += 1;

            Err(HsmErr::Pending)
        } else {
            self.check_alive_cnt = 0;

            self.state = State::Final;

            Err(HsmErr::IoTimeOut)
        }
    }

    /// Create a command response for certificate chain length
    fn cmd_resp(
        &self,
        rev: Option<DdiApiRev>,
        cert_info: &GetCertChainLengthsInfo,
    ) -> DdiGetCertChainInfoCmdResp {
        DdiGetCertChainInfoCmdResp {
            hdr: DdiRespHdr {
                rev,
                op: DdiOp::GetCertChainInfo,
                sess_id: None,
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiGetCertChainInfoResp {
                num_certs: cert_info.num_certs,
                thumbprint: MborByteArray::new_with_len(cert_info.hash.as_ptr(), 32),
            },
        }
    }
}
