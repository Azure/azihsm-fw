// Copyright (c) Microsoft Corporation. All rights reserved.

//! Implementation of the `GetCertificate` command.

use super::*;

/// Max time tick count to wait for IPC resource
const MAX_RESOURCE_WAIT_TIME: u8 = 16;

/// FSM states
#[derive(Clone, Copy, Debug, PartialEq)]
enum State {
    /// Initial state
    Init,

    /// Waiting for HSP IPC Channel
    WaitForResource,

    /// Wait form completion of HSP IPC operation: GetCert
    WaitForGetCertResponse,

    /// Final state
    Final,
}

/// Get Certificate command
pub(crate) struct GetCertificateCmd<E: HsmEnvTrait + 'static> {
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

    /// Command response
    cmd_resp: Option<DdiGetCertificateCmdResp>,

    /// API rev
    api_rev: Option<DdiApiRev>,

    /// Get Cert Chain command context
    cert_ctx: Option<GetCertContext<<<E as env::HsmEnvTrait>::Partition as HsmPartition>::Env>>,

    /// Check Alive Counter
    check_alive_cnt: u8,
}

impl<E: HsmEnvTrait> HsmCmdTrait<E> for GetCertificateCmd<E> {
    /// Take the response buffer
    fn take_response(&mut self) -> Option<DmaBuffer<E>> {
        self.resp.take()
    }

    /// Handle an event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        match (self.state, event) {
            (State::Init, HsmFsmEvent::StartCmd) => self.on_start(tag),
            (State::WaitForResource, HsmFsmEvent::ResourceReady(_)) => self.on_resource_ready(tag),
            (_, HsmFsmEvent::CheckAlive) => self.check_alive(),
            (State::WaitForGetCertResponse, HsmFsmEvent::HspToHsmIpcResponse) => {
                self.end_get_individual_cert()
            }
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
            HsmFsmResourceId::HspIpcChannel => {
                HsmFsmEvent::ResourceReady(HsmFsmResourceId::HspIpcChannel)
            }
            _ => unreachable!(),
        }
    }
}

impl<E: HsmEnvTrait> GetCertificateCmd<E> {
    /// Create a new command FSM
    pub fn new(req: DmaBuffer<E>, heap: DmaHeap<E>, part: E::Partition) -> Self {
        Self {
            state: State::Init,
            heap,
            part,
            req,
            resp: None,
            cmd_resp: None,
            api_rev: None,
            cert_ctx: None,
            check_alive_cnt: 0,
        }
    }

    /// Handle the start event to get the certificate chain info
    fn on_start(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let req = decode_buf::<DdiGetCertificateCmdReq, E>(&self.req)?;

        let rev = req.hdr.rev.ok_or(HsmErr::UnsupportedRevision)?;
        self.api_rev = Some(rev);

        if req.data.slot_id != 0 {
            return Err(HsmErr::InvalidArgument);
        }

        // Prep the command response and certificate context
        let cert_id = req.data.cert_id;
        let (cert_len, cert_buf, resp_buf) = match self.part.get_cert_len(cert_id) {
            Some(cert_size) => {
                let resp = self.cmd_resp(self.api_rev, cert_size);
                let resp_buf = encode_buf(&resp, &self.heap)?;
                let mem_range: IoMemRange = (&resp.data.certificate).into();
                self.cmd_resp = Some(resp);

                (Some(cert_size as u16), Some(mem_range), Some(resp_buf))
            }
            _ => (None, None, None),
        };

        self.resp = resp_buf;

        self.cert_ctx = Some(GetCertContext {
            cert_id,
            cert_len,
            cert_buf,
            channel_ref: None,
        });

        self.begin_get_individual_cert(tag)
    }

    /// Handle the IPC resource ready
    fn on_resource_ready(&mut self, tag: TagId) -> Result<(), HsmErr> {
        self.begin_get_individual_cert(tag)
    }

    /// Get the individual certificate.
    fn begin_get_individual_cert(&mut self, tag: TagId) -> Result<(), HsmErr> {
        match self
            .part
            .begin_get_cert(tag, self.cert_ctx.as_mut().ok_or(HsmErr::InvalidState)?)
        {
            Ok(_) => {
                if self
                    .cert_ctx
                    .as_ref()
                    .ok_or(HsmErr::InvalidState)?
                    .channel_ref
                    .is_some()
                {
                    self.state = State::WaitForGetCertResponse;

                    Err(HsmErr::Pending)
                } else {
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

    /// Handle the completion of the GetCert IPC operation.
    fn end_get_individual_cert(&mut self) -> Result<(), HsmErr> {
        // make sure the channel reference is dropped after this function exits.
        let mut local_ctx = self.cert_ctx.take().ok_or(HsmErr::InvalidState)?;
        self.part.end_get_cert(&mut local_ctx)?;

        self.state = State::Final;
        Ok(())
    }

    /// On timer event response
    fn check_alive(&mut self) -> Result<(), HsmErr> {
        if (self.state == State::WaitForResource || self.state == State::WaitForGetCertResponse)
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

    /// Create a command response for certificate
    fn cmd_resp(&self, rev: Option<DdiApiRev>, cert_len: usize) -> DdiGetCertificateCmdResp {
        DdiGetCertificateCmdResp {
            hdr: DdiRespHdr {
                rev,
                op: DdiOp::GetCertificate,
                sess_id: None,
                status: DdiStatus::Success,
                fips_approved: self.part.is_fips_approved(),
            },
            data: DdiGetCertificateResp {
                certificate: MborByteArray::new_with_len(core::ptr::null(), cert_len),
            },
        }
    }
}
