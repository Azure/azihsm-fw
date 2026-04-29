// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::boxed::Box;

use mcr_ddi_types::*;
use mcr_gdma_controller::*;
use mcr_simplex::SimplexPipeTrait;
use mcr_types::*;

use super::*;
use crate::partition::*;
use crate::*;

use alloc::rc::Rc;
use core::cell::RefCell;

/// HSM FSM State
#[derive(Clone, Copy, PartialEq, Default)]
enum HsmFsmState {
    /// Initial State
    #[default]
    Init,

    /// DMA In state
    DmaIn,

    /// Command state
    Cmd,

    /// DMA Out state
    DmaOut,

    /// DMA Out Error state
    DmaOutErr,

    /// Completion state
    Completion,
}

/// HSM FSM (Finite State Machine)
pub(crate) struct HsmFsm<E: HsmEnvTrait + 'static> {
    /// Current state
    state: HsmFsmState,

    /// IO queue
    ioq: Option<IoQueue>,

    /// PCIe function
    pfn: Option<PcieFunction>,

    /// Submission queue entry
    sqe: HsmSqe,

    /// Submission queue entry address
    sqe_addr: u32,

    /// DMA buffer
    dma_buf: Option<DmaBuffer<E>>,

    /// Command FSM
    cmd: Option<Box<dyn HsmCmdTrait<E>>>,

    /// Session ID
    session_id: Option<u16>,

    /// Session flags
    session_flags: HsmSessionFlags,

    /// App vault ID
    app_vault_id: Option<u8>,

    /// Ddi Request Header
    req_hdr: DdiReqHdr,

    /// FSM incurred an Error
    err: Option<HsmErr>,

    /// Environment
    env: Rc<RefCell<E>>,
}

/// Implementation of the CmdFsm trait for the HsmFsm enumeration
impl<E: HsmEnvTrait> CmdFsm for HsmFsm<E> {
    type Error = HsmErr;
    type ResourceId = HsmFsmResourceId;
    type Event = HsmFsmEvent;
    type Recorder = HsmFsmEventRecorder;

    /// The callback to trigger when an event occurs.
    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match (self.state, event) {
            (HsmFsmState::Init, HsmFsmEvent::RxReady) => self.on_rx_ready(event, tag),
            (HsmFsmState::DmaIn, HsmFsmEvent::DmaComplete) => self.on_in_dma_complete(tag),
            (HsmFsmState::Cmd, _) => self.on_cmd_event(event, tag),
            (_, HsmFsmEvent::CheckAlive) => self.on_io_timeout(tag),
            (HsmFsmState::DmaOut, HsmFsmEvent::DmaComplete) => self.on_out_dma_complete(tag),
            (HsmFsmState::DmaOutErr, _) => self.on_dma_out_err(event, tag),
            (HsmFsmState::Completion, HsmFsmEvent::TxComplete) => self.on_tx_complete(tag),
            _ => Err(HsmErr::Pending),
        }
    }

    /// On resource acquisition
    fn acquire_resource(&mut self, tag: TagId, id: Self::ResourceId) -> Self::Event {
        let Some(fsm) = self.cmd.as_mut() else {
            unreachable!()
        };

        if !fsm.requires_resource(tag, id) {
            unreachable!()
        }

        fsm.acquire_resource(tag, id)
    }
}

impl<E: HsmEnvTrait> Drop for HsmFsm<E> {
    fn drop(&mut self) {
        let _ = self.retire_io();
    }
}

impl<E: HsmEnvTrait> HsmFsm<E> {
    /// Create a new HsmFsm
    pub(crate) fn new(env: Rc<RefCell<E>>) -> Self {
        Self {
            state: Default::default(),
            ioq: Default::default(),
            pfn: Default::default(),
            sqe: Default::default(),
            sqe_addr: Default::default(),
            dma_buf: Default::default(),
            cmd: Default::default(),
            session_id: None,
            session_flags: Default::default(),
            app_vault_id: None,
            req_hdr: DdiReqHdr {
                rev: None,
                op: DdiOp::Invalid,
                sess_id: None,
            },
            err: None,
            env,
        }
    }

    /// Recieve ready handler
    fn on_rx_ready(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        trace!("[tag: {}] on_rx_ready", tag);
        // Begin recieve operation
        let desc = self.begin_recv_sqe()?;

        // Setup the fsm context
        self.pfn = Some(desc.pfn);
        self.ioq = match self.ioq(&desc) {
            Ok(ioq) => Some(ioq),
            Err(err) => {
                self.env
                    .borrow()
                    .hal()
                    .io_channel()
                    .end_recv(desc.addr, desc.sq_id);
                Err(err)?
            }
        };

        // Retrieve and parse the submission queue entry
        self.sqe = desc.entry.into();
        self.sqe_addr = desc.addr;

        self.process_rx_entry(event, tag)
    }

    /// Input DMA completion handler
    fn on_in_dma_complete(&mut self, tag: TagId) -> Result<(), HsmErr> {
        trace!("[tag: {}] on_in_dma_complete", tag);

        self.process_in_dma_complete(tag)
    }

    /// Command fms event handler
    fn on_cmd_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        trace!("[tag: {}] on_cmd_event", tag);

        self.process_cmd_event(event, tag)
    }

    /// Out DMA completion handler
    fn on_out_dma_complete(&mut self, tag: TagId) -> Result<(), HsmErr> {
        trace!("[tag: {}] on_out_dma_complete", tag);

        self.process_out_dma_complete(tag)
    }

    /// Response complete handler
    fn on_tx_complete(&mut self, tag: TagId) -> Result<(), HsmErr> {
        trace!("[tag: {}] on_tx_complete", tag);

        // Complete the recieve operation.
        self.end_recv_sqe()
    }

    fn on_dma_out_err(&mut self, event: HsmFsmEvent, tag: TagId) -> HsmResult<()> {
        // We should have a valid fsm in this state
        let fsm = self.cmd.as_mut().ok_or(HsmErr::ExpectedCmdFsm)?;

        match fsm.on_event(event, tag) {
            Err(HsmErr::Pending) => Err(HsmErr::Pending),
            Ok(()) | Err(_) => self.send_err_cqe(self.err.unwrap_or(HsmErr::DmaStartError), tag),
        }
    }

    /// IO timed out
    fn on_io_timeout(&mut self, _tag: TagId) -> Result<(), HsmErr> {
        warn!("[hsm_fsm] IO Timed out while in {:?}", self.state as u32);

        Err(HsmErr::IoTimeOut)
    }

    /// Begin receieve submission queue entry operation
    fn begin_recv_sqe(&self) -> Result<IoRxDesc, HsmErr> {
        let desc = self
            .env
            .borrow()
            .hal()
            .io_channel()
            .begin_recv()
            .ok_or(HsmErr::IoChannelRecvNone)?;

        if !desc.status {
            Err(HsmErr::IoChannelRecvErr)?
        }

        Ok(desc)
    }

    /// Process the new Rx entry
    fn process_rx_entry(&mut self, _event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        // Validate submission queue entry
        self.validate_sqe()
            .or_else(|err| self.send_err_cqe(err, tag))?;

        if self.sqe.cmd.op() == HsmSqeCmdOpcode::Flush {
            // If the opcode is flush, then handle it
            self.flush_session(tag)?
        }

        // Allocate inbound DMA buffer
        let buf = self
            .env
            .borrow()
            .hal()
            .dma_heap()
            .allocate_from_pool(self.sqe.src_len())
            .ok_or(HsmErr::DmaAllocFailure);

        if let Err(err) = buf {
            self.send_err_cqe(err, tag)?
        }

        self.dma_buf = Some(buf.unwrap());

        // Begin inbound DMA
        self.begin_in_dma(tag)
            .or_else(|err| self.send_err_cqe(err, tag))?;

        Err(HsmErr::Pending)
    }

    /// Process DMA in completion event
    fn process_in_dma_complete(&mut self, tag: TagId) -> Result<(), HsmErr> {
        // Complete the DMA operation
        self.end_dma(tag)
            .or_else(|err| self.send_err_cqe(err, tag))?;

        self.init_cmd(tag).or_else(|err| match err {
            HsmErr::CmdError => match self.begin_out_dma(tag) {
                Ok(_) => Err(HsmErr::Pending),
                Err(err) => self.send_err_cqe(err, tag),
            },
            _ => self.send_err_cqe(err, tag),
        })?;

        // Start the command FSM
        self.process_cmd_event(HsmFsmEvent::StartCmd, tag)
    }

    /// Process command events once a HSM command state machine is initialized
    fn process_cmd_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr> {
        // We should have a valid fsm in this state
        let fsm = self.cmd.as_mut().ok_or(HsmErr::ExpectedCmdFsm)?;

        // Get the command response
        let fsm_resp = fsm.on_event(event, tag);

        if self.sqe.cmd.op() == HsmSqeCmdOpcode::Flush && event == HsmFsmEvent::FpToHsmIpcResponse {
            self.session_flags.set_session_closed(!fsm.retry());
            self.send_flush_cqe(self.sqe.session_id, self.session_flags, tag)?;
        } else {
            (self.dma_buf, self.session_id, self.app_vault_id) = match fsm_resp {
                Err(HsmErr::Pending) => (None, None, None),
                Err(err) => {
                    if let Some(err) = self.err {
                        let resp = self.cmd_err_resp(tag, err)?;
                        (resp.0, resp.1, None)
                    } else {
                        // Invoke the rollback process just once
                        self.err = Some(err);

                        match self.issue_command_rollback(tag) {
                            Err(HsmErr::Pending) => (None, None, None),
                            Err(_) | Ok(()) => {
                                let resp = self.cmd_err_resp(tag, err)?;
                                (resp.0, resp.1, None)
                            }
                        }
                    }
                }
                Ok(_) => {
                    if let Some(err) = self.err {
                        let resp = self.cmd_err_resp(tag, err)?;
                        (resp.0, resp.1, None)
                    } else {
                        (fsm.take_response(), fsm.session_id(), fsm.app_vault_id())
                    }
                }
            };

            // If the DMA buffer is populated, start DMA out request processing
            if self.dma_buf.is_some() {
                if self.session_flags.ctrl() == HsmSessionControlKind::Close {
                    let fsm = self.cmd.as_mut().ok_or(HsmErr::ExpectedCmdFsm)?;
                    self.session_flags.set_session_closed(!fsm.retry());
                }

                match self.begin_out_dma(tag) {
                    Ok(()) => Err(HsmErr::Pending)?,
                    Err(err) => {
                        // This is the first time HsmFsm is failing
                        if self.err.is_none() {
                            self.err = Some(err);
                            self.state = HsmFsmState::DmaOutErr;
                            if let Err(HsmErr::Pending) = self.issue_command_rollback(tag) {
                                Err(HsmErr::Pending)?
                            }
                        }
                        self.send_err_cqe(err, tag)?
                    }
                }
            }
        }

        Err(HsmErr::Pending)
    }

    /// Process the DMA out completion event
    fn process_out_dma_complete(&mut self, tag: u16) -> Result<(), HsmErr> {
        match self.end_dma(tag) {
            Ok(()) => self.send_cqe(tag),
            Err(err) => {
                // This is the first time HsmFsm is failing
                if self.err.is_none() {
                    self.err = Some(err);
                    self.state = HsmFsmState::DmaOutErr;
                    if let Err(HsmErr::Pending) = self.issue_command_rollback(tag) {
                        Err(HsmErr::Pending)?
                    }
                }
                self.send_err_cqe(err, tag)
            }
        }
    }

    /// Issue command rollback
    fn issue_command_rollback(&mut self, tag: TagId) -> HsmResult<()> {
        let fsm = self.cmd.as_mut().ok_or(HsmErr::ExpectedCmdFsm)?;

        fsm.rollback(tag)
    }

    /// Complete receive submission queue entry opetation
    fn end_recv_sqe(&mut self) -> Result<(), HsmErr> {
        if let Some(desc) = self.env.borrow().hal().io_channel().end_send() {
            if desc.status != IoTxCompleteStatus::Success {
                Err(HsmErr::IoChannelSendCompleteError)?
            }
        } else {
            Err(HsmErr::IoChannelSendCompleteNone)?
        }

        Ok(())
    }

    /// Begin inbound DMA operation
    fn begin_in_dma(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let mut desc = self.make_in_dma_desc(tag)?;

        self.env
            .borrow()
            .hal()
            .dma_channel()
            .begin_txn(&mut desc)
            .map_err(|err| {
                error!(
                    "[hsm_fsm] DMA begin_txn error. (0x{:08x}) in begin_in_dma",
                    err
                );
                HsmErr::DmaStartError
            })?;

        self.state = HsmFsmState::DmaIn;

        Ok(())
    }

    /// Begin outbound DMA operation
    fn begin_out_dma(&mut self, tag: TagId) -> Result<(), HsmErr> {
        #[cfg(feature = "mcr_test_hooks")]
        {
            let part = self
                .env
                .borrow()
                .partition(self.pfn.ok_or(HsmErr::ExpectedPcieFn)?);

            if let Some(action) = part.hsm_fsm_test_action(None) {
                if action == DdiTestAction::TriggerDmaOutFailure {
                    Err(HsmErr::DmaStartError)?;
                } else {
                    let _ = part.hsm_fsm_test_action(Some(action));
                }
            }
        }

        let mut desc = self.make_out_dma_desc(tag)?;

        self.env
            .borrow()
            .hal()
            .dma_channel()
            .begin_txn(&mut desc)
            .map_err(|err| {
                error!(
                    "[hsm_fsm] DMA begin_txn error. (0x{:08x}) in begin_out_dma",
                    err
                );
                HsmErr::DmaStartError
            })?;

        self.state = HsmFsmState::DmaOut;

        Ok(())
    }

    /// Complete inbound or outbound DMA operation
    fn end_dma(&self, tag: TagId) -> Result<(), HsmErr> {
        match self.env.borrow().hal().dma_channel().end_txn() {
            Some(desc) => {
                if !desc.success {
                    Err(HsmErr::DmaEndErr)?
                }
                if desc.tag != tag {
                    Err(HsmErr::DmaTagMismatch)?
                }
            }
            None => return Err(HsmErr::DmaCompletionEmpty)?,
        };

        #[cfg(feature = "mcr_test_hooks")]
        {
            let part = self
                .env
                .borrow()
                .partition(self.pfn.ok_or(HsmErr::ExpectedPcieFn)?);

            if let Some(action) = part.hsm_fsm_test_action(None) {
                if action == DdiTestAction::TriggerDmaEndFailure {
                    Err(HsmErr::DmaEndErr)?;
                }
            }
        }

        Ok(())
    }

    /// Create inbound DMA descriptor
    fn make_in_dma_desc(&mut self, tag: u16) -> Result<DmaTxnDesc, HsmErr> {
        let pfn = self.pfn.ok_or(HsmErr::ExpectedPcieFn)?;
        let buf = self.dma_buf.as_ref().ok_or(HsmErr::ExpectedDmaBuf)?;

        let desc = DmaTxnDesc {
            src_fst: DmaMemoryDesc {
                fmt: DmaDescFormat::Prp,
                loc: pfn.into(),
                addr: self.sqe.src.prp1,
            },
            src_snd: None,
            dst_fst: DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: MemoryLocation::Soc,
                addr: buf.as_ref().into(),
            },
            dst_snd: Some(DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: MemoryLocation::Soc,
                addr: MemoryAddr {
                    lo: buf.len() as u32,
                    hi: 0,
                },
            }),
            len: buf.len() as u32,
            tag,
        };

        Ok(desc)
    }

    /// Create outbound DMA descriptor
    fn make_out_dma_desc(&mut self, tag: u16) -> Result<DmaTxnDesc, HsmErr> {
        let pfn = self.pfn.ok_or(HsmErr::ExpectedPcieFn)?;
        let buf = self.dma_buf.as_ref().ok_or(HsmErr::ExpectedDmaBuf)?;

        let desc = DmaTxnDesc {
            src_fst: DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: MemoryLocation::Soc,
                addr: buf.as_ref().into(),
            },
            src_snd: Some(DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: MemoryLocation::Soc,
                addr: MemoryAddr {
                    lo: buf.len() as u32,
                    hi: 0,
                },
            }),
            dst_fst: DmaMemoryDesc {
                fmt: DmaDescFormat::Prp,
                loc: pfn.into(),
                addr: self.sqe.dst.prp1,
            },
            dst_snd: None,
            len: buf.len() as u32,
            tag,
        };

        Ok(desc)
    }

    /// Initialize command processing
    fn init_cmd(&mut self, _tag: TagId) -> Result<(), HsmErr> {
        // Retrieve the DMA buffer
        let buf = self.dma_buf.take().ok_or(HsmErr::ExpectedDmaBuf)?;

        // Decode the header
        let mut decoder = DdiDecoder::new(buf.as_ref());
        match decoder.decode_hdr() {
            Ok(hdr) => self.req_hdr = hdr,
            Err(_) => self.on_err(None, DdiStatus::DdiDecodeFailed, DdiOp::Invalid)?,
        }

        let pfn = self.pfn.ok_or(HsmErr::ExpectedPcieFn)?;
        let ddi_op = self.req_hdr.op;

        self.session_flags
            .set_ctrl(HsmSessionControlKind::from(ddi_op));

        self.validate_req_hdr(pfn)
            .or_else(|err| self.on_err(self.req_hdr.sess_id, err.into(), ddi_op))?;

        let partition = self.env.borrow().partition(pfn);
        let heap = self.env.borrow().hal().dma_heap().clone();

        let ctrl = self.session_flags.ctrl();
        if ctrl == HsmSessionControlKind::InSession && !partition.is_partition_provisioned() {
            self.on_err(None, DdiStatus::PartitionNotProvisioned, ddi_op)?
        }

        let fsm: Box<dyn HsmCmdTrait<E>> = match ddi_op {
            DdiOp::GetApiRev => Box::new(GetApiRevCmd::new(buf, heap, partition)),
            DdiOp::GetDeviceInfo => Box::new(GetDeviceInfoCmd::new(buf, heap, partition)),
            DdiOp::OpenKey => Box::new(OpenKeyCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
            )),
            DdiOp::DeleteKey => Box::new(DeleteKeyCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
                pfn,
            )),
            DdiOp::AesGenerateKey => Box::new(AesGenKeyCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
                pfn,
            )),
            DdiOp::AesEncryptDecrypt => Box::new(AesEncDecCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
            )),
            DdiOp::EccGenerateKeyPair => Box::new(EccGenKeyCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
            )),
            DdiOp::EccSign => Box::new(EccSignCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
            )),
            DdiOp::EcdhKeyExchange => Box::new(EcdhKeyExchangeCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
            )),
            DdiOp::HkdfDerive => Box::new(HkdfDeriveCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
                pfn,
            )),
            DdiOp::KbkdfCounterHmacDerive => Box::new(KbkdfDeriveCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
                pfn,
            )),
            DdiOp::RsaModExp => Box::new(RsaModExpCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
            )),
            DdiOp::GetUnwrappingKey => Box::new(GetUnwrappingKeyCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
                pfn,
            )),
            DdiOp::RsaUnwrap => Box::new(RsaUnwrapCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
                pfn,
            )),
            DdiOp::AttestKey => Box::new(AttestKeyCmd::new(
                buf,
                heap,
                partition,
                self.req_session_id()?,
            )),
            DdiOp::GetCertChainInfo => Box::new(GetCertChainInfoCmd::new(buf, heap, partition)),
            DdiOp::GetCertificate => Box::new(GetCertificateCmd::new(buf, heap, partition)),
            DdiOp::GetEstablishCredEncryptionKey => {
                Box::new(GetEstablishCredEncryptionKeyCmd::new(buf, heap, partition))
            }
            DdiOp::EstablishCredential => {
                Box::new(EstablishCredentialCmd::new(buf, heap, partition))
            }
            DdiOp::GetSessionEncryptionKey => {
                Box::new(GetSessionEncryptionKeyCmd::new(buf, heap, partition))
            }
            DdiOp::OpenSession | DdiOp::ReopenSession => Box::new(OpenSessionCmd::new(
                buf,
                heap,
                partition,
                ddi_op == DdiOp::ReopenSession,
            )),
            DdiOp::CloseSession => Box::new(CloseSessionCmd::new(buf, heap, partition, pfn)),
            DdiOp::ChangePin => Box::new(ChangePinCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
            )),
            DdiOp::Hmac => Box::new(HmacCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
            )),
            DdiOp::UnmaskKey => Box::new(UnmaskKeyCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
                pfn,
            )),
            DdiOp::InitBk3 => Box::new(InitBk3Cmd::new(buf, heap, partition)),
            DdiOp::SetSealedBk3 => Box::new(SetSealedBk3Cmd::new(buf, heap, partition)),
            DdiOp::GetSealedBk3 => Box::new(GetSealedBk3Cmd::new(buf, heap, partition)),
            // Test commands
            #[cfg(feature = "mcr_test_hooks")]
            DdiOp::DerKeyImport => Box::new(DerKeyImportCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                pfn,
            )),
            #[cfg(any(
                feature = "mcr_test_hooks",
                feature = "mcr_manual_test_hooks",
                feature = "fips_validation_hooks"
            ))]
            DdiOp::TestAction => Box::new(TestActionCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
                partition,
                pfn,
            )),
            #[cfg(feature = "fips_validation_hooks")]
            DdiOp::GetPrivKey => Box::new(GetPrivKeyCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
            )),
            #[cfg(feature = "fips_validation_hooks")]
            DdiOp::ShaDigest => {
                Box::new(ShaDigestCmd::new(buf, heap, self.user_session(&partition)?))
            }
            #[cfg(feature = "fips_validation_hooks")]
            DdiOp::GetRandomNumber => {
                Box::new(GetRngCmd::new(buf, heap, self.user_session(&partition)?))
            }
            #[cfg(feature = "fips_validation_hooks")]
            DdiOp::RawKeyImport => Box::new(RawKeyImportCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
            )),
            #[cfg(feature = "fips_validation_hooks")]
            DdiOp::SoftAes => Box::new(SoftAesCmd::new(buf, heap, self.user_session(&partition)?)),
            #[cfg(feature = "fips_validation_hooks")]
            DdiOp::RsaUnwrapKekTest => Box::new(RsaUnwrapKekTestCmd::new(
                buf,
                heap,
                self.user_session(&partition)?,
            )),
            _ => Box::new(UnsupportedCmd::new()),
        };

        self.cmd = Some(fsm);
        self.state = HsmFsmState::Cmd;

        Ok(())
    }

    /// Prepare command error response
    fn cmd_err_resp(
        &mut self,
        tag: TagId,
        err: HsmErr,
    ) -> Result<(Option<DmaBuffer<E>>, Option<u16>), HsmErr> {
        let resp = self.err_resp(self.req_hdr.sess_id, err.into(), self.req_hdr.op);

        let buf = encode_buf(&resp, self.env.borrow().hal().dma_heap());

        if let Err(err) = buf {
            self.send_err_cqe(err, tag)?
        }

        Ok((Some(buf.unwrap()), self.req_hdr.sess_id))
    }

    /// Send error completion queue entry
    fn send_err_cqe<T>(&mut self, err: HsmErr, tag: TagId) -> Result<T, HsmErr> {
        let ioq = self.ioq.as_ref().ok_or(HsmErr::ExpectedIoQueue)?;

        let session_flags = self
            .session_flags
            .with_id_valid(self.session_id.is_some())
            .with_app_vault_id_is_valid(self.app_vault_id.is_some());
        let cqe = HsmCqe {
            cmd_id: self.sqe.cmd.id(),
            sq_id: ioq.sq_id().into(),
            psf: PsfField::default().with_status(err.into()),
            session_flags,
            session_id: self.session_id.unwrap_or(0),
            app_vault_id: self.app_vault_id.unwrap_or(0),
            ..Default::default()
        }
        .into();

        let desc = IoTxDesc {
            tx_queue_id: ioq.cq_id().into(),
            rx_queue_id: ioq.sq_id().into(),
            tag,
            entry: &cqe,
        };

        self.env
            .borrow()
            .hal()
            .io_channel()
            .begin_send(&desc)
            .map_err(|err| {
                error!("send_err_cqe: IO channel send error. (0x{:08x})", err);
                HsmErr::IoChannelSendError
            })?;

        self.state = HsmFsmState::Completion;

        Err(HsmErr::Pending)
    }

    /// Send completion queue entry
    fn send_cqe(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let ioq = self.ioq.as_ref().ok_or(HsmErr::ExpectedIoQueue)?;
        let dst_len = if let Some(dma_buf) = self.dma_buf.take() {
            dma_buf.len() as u16
        } else {
            // This should not happen. However, if it does, we should not panic. Since this is
            // contained with this one IO, the safest option is to report the len() as 0 if the
            // DMA buffer is not found, so we don't affect other VF traffic.
            error!("send_cqe: DMA buffer not found");
            0
        };

        let session_flags = self
            .session_flags
            .with_id_valid(self.session_id.is_some())
            .with_app_vault_id_is_valid(self.app_vault_id.is_some());
        let cqe = HsmCqe {
            dst_len,
            cmd_id: self.sqe.cmd.id(),
            sq_id: ioq.sq_id().into(),
            psf: PsfField::default().with_status(HostStatusCode::Success),
            session_flags,
            session_id: self.session_id.unwrap_or(0),
            app_vault_id: self.app_vault_id.unwrap_or(0),
            ..Default::default()
        }
        .into();

        let desc = IoTxDesc {
            tx_queue_id: ioq.cq_id().into(),
            rx_queue_id: ioq.sq_id().into(),
            tag,
            entry: &cqe,
        };

        self.env
            .borrow()
            .hal()
            .io_channel()
            .begin_send(&desc)
            .map_err(|err| {
                error!("send_cqe: IO channel send error. (0x{:08x})", err);
                HsmErr::IoChannelSendError
            })?;

        self.state = HsmFsmState::Completion;

        Err(HsmErr::Pending)
    }

    /// Get the IO Queue from the `IoRxDesc`
    fn ioq(&self, desc: &IoRxDesc) -> Result<IoQueue, HsmErr> {
        // Retrieve the PCIe function
        let partition = self.env.borrow().partition(desc.pfn);

        if !partition.enabled() {
            Err(HsmErr::PartitionNotEnabled)?
        }

        // Retrieve the IO Queue from PCIe function
        let ioq = partition
            .io_queue(desc.sq_id)
            .ok_or(HsmErr::QueueNotEnabled)?;

        Ok(ioq)
    }

    /// Retire the IO operation and check if any deferred queue delete notification is pending
    fn retire_io(&self) -> Result<(), HsmErr> {
        let ioq = self.ioq.as_ref().ok_or(HsmErr::ExpectedIoq)?;

        // Retire the IO at the IO channel to free up space for new IOs to be retrieved from host.
        self.env
            .borrow()
            .hal()
            .io_channel()
            .end_recv(self.sqe_addr, ioq.sq_id());

        // If the IO queue that owns this IO becomes invalid, take action to send deferred
        // notification to management CPU to complete the queue delete command.
        if !ioq.valid() {
            // If this is the last IO handled by this IOQ then check if this is the last IO
            // tracked for this PCIe function, if both are true, then,
            // take action to send deferred queue delete notification
            if ioq.ref_cnt() == 1 {
                if let Some(delete_ctx) = ioq.take_delete_ctx() {
                    // If this is the last delete context for this PCIe function + IOQ, then
                    // notify the Admin Queue Manager to delete the IOQ
                    if delete_ctx.ref_cnt() == 1 {
                        let msg = QueueDeleteResponse {
                            tag: delete_ctx.tag(),
                            pfn: self.pfn.ok_or(HsmErr::ExpectedPcieFn)?,
                            _rsvd: Default::default(),
                        };

                        if delete_ctx.is_migration() {
                            let pfn = self.pfn.ok_or(HsmErr::ExpectedPcieFn)?;
                            let partition = self.env.borrow().partition(pfn);
                            partition.end_migrate();
                        }

                        self.env
                            .borrow()
                            .hal()
                            .queue_delete_notification()
                            .send(msg)
                            .map_err(|_| HsmErr::DeferredQueueDeleteNotifyErr)?;
                    }
                }
            }
        }

        Ok(())
    }

    /// Validate submission queue entry
    fn validate_sqe(&self) -> Result<(), HsmErr> {
        let sqe = &self.sqe;

        // Validate PRP or SGL Data Transfer Field
        if sqe.cmd.psdt() != 0 {
            Err(HsmErr::SqeInvalidPsdt)?
        }

        match sqe.cmd.op() {
            HsmSqeCmdOpcode::Generic => self.validate_sqe_with_generic_opcode()?,
            HsmSqeCmdOpcode::Flush => self.validate_sqe_with_flush_opcode()?,
            _ => Err(HsmErr::SqeUnknownOp)?,
        }

        Ok(())
    }

    /// Validate submission queue entry for generic opcode
    fn validate_sqe_with_generic_opcode(&self) -> Result<(), HsmErr> {
        let sqe = &self.sqe;
        // We support 4K pages only
        const PAGE_SIZE: usize = 4 * 1024;

        // Validate maximum source length
        const MIN_SRC_DATA_LEN: usize = 1;
        const MAX_SRC_DATA_LEN: usize = PAGE_SIZE;
        if sqe.src_len() < MIN_SRC_DATA_LEN || sqe.src_len() > MAX_SRC_DATA_LEN {
            Err(HsmErr::SqeInvalidSrcLen)?
        }

        // Validate maximum destination length
        const MIN_DST_DATA_LEN: usize = 1;
        const MAX_DST_DATA_LEN: usize = 2 * PAGE_SIZE;
        if sqe.dst_len() < MIN_DST_DATA_LEN || sqe.dst_len() > MAX_DST_DATA_LEN {
            Err(HsmErr::SqeInvalidDstLen)?
        }

        // Validate source PRP1 & PRP2 address are page aligend
        if (sqe.src.prp1.lo as usize) & (PAGE_SIZE - 1)
            | (sqe.src.prp2.lo as usize) & (PAGE_SIZE - 1)
            != 0
        {
            Err(HsmErr::SqeInvalidSrcPrpAlgin)?
        }

        // Validate destination PRP1 & PRP2 address are page aligend
        if (sqe.dst.prp1.lo as usize) & (PAGE_SIZE - 1)
            | (sqe.dst.prp2.lo as usize) & (PAGE_SIZE - 1)
            != 0
        {
            Err(HsmErr::SqeInvalidDstPrpAlgin)?
        }

        Ok(())
    }

    // Validate submission queue entry for flush opcode
    fn validate_sqe_with_flush_opcode(&self) -> Result<(), HsmErr> {
        let sqe = &self.sqe;

        if sqe.session_flags.ctrl() != HsmSessionControlKind::Close {
            Err(HsmErr::InvalidSessionControlOpcode)?;
        }

        if !sqe.session_flags.id_valid() {
            Err(HsmErr::SessionExpected)?
        }

        Ok(())
    }

    // Validate session hijack protection
    fn validate_session_hijack_protection(&self) -> Result<(), HsmErr> {
        // Make sure session control kind from SQE applies to the decoded CBOR opcode.
        if self.sqe.session_flags.ctrl() != self.req_hdr.op.into() {
            Err(HsmErr::InvalidArgument)?
        }

        // Make sure, if session control is NoSession or OpenSession then SQE does not have
        // session id present, otherwise, SQE must have session id present.
        match (
            self.sqe.session_flags.ctrl(),
            self.sqe.session_flags.id_valid(),
        ) {
            (HsmSessionControlKind::NoSession, true) => Err(HsmErr::InvalidArgument)?,
            (HsmSessionControlKind::Open, true) => Err(HsmErr::SessionNotExpected)?,
            (HsmSessionControlKind::Close, false) => Err(HsmErr::InvalidArgument)?,
            (HsmSessionControlKind::InSession, false) => Err(HsmErr::InvalidArgument)?,
            _ => (),
        }

        // If a session id was passed in SQE then it must match the session id passed in CBOR.
        // If no session id was passed in SQE then it must be empty in CBOR as well.
        if self.sqe.session_flags.id_valid() {
            if let Some(session_id) = self.req_hdr.sess_id {
                if self.sqe.session_id != session_id {
                    Err(HsmErr::InvalidArgument)?
                }
            } else {
                Err(HsmErr::InvalidArgument)?
            }
        } else if self.req_hdr.sess_id.is_some() {
            Err(HsmErr::InvalidArgument)?
        }

        Ok(())
    }

    /// Validate the request header.
    fn validate_req_hdr(&self, pfn: PcieFunction) -> Result<(), HsmErr> {
        // Check that partition is enabled.
        let part = self.env.borrow().partition(pfn);
        if !part.enabled() {
            Err(HsmErr::FunctionNotEnabled)?
        }

        self.validate_session_hijack_protection()?;

        // Check for supported revision.
        match self.req_hdr.op.into() {
            DdiSessionKind::User => {
                let sess_id = self.req_session_id()?;
                let needs_renegotiation = part.needs_renegotiation(sess_id);
                if needs_renegotiation
                    && (self.req_hdr.op == DdiOp::ReopenSession
                        || self.req_hdr.op == DdiOp::CloseSession)
                {
                    // Special case:
                    // 1. Reopen session: renegotiation operation
                    // 2. Close session: No need to regegotiate, just close the session
                    return Ok(());
                }

                let user_session = part.user_session(sess_id, true)?;
                let hdr_rev = self.req_hdr.rev.ok_or(HsmErr::UnsupportedRevision)?;
                if hdr_rev != user_session.api_rev() {
                    Err(HsmErr::UnsupportedRevision)?
                }
            }

            DdiSessionKind::None => {
                if self.req_hdr.op != DdiOp::GetApiRev {
                    let hdr_rev = self.req_hdr.rev.ok_or(HsmErr::UnsupportedRevision)?;

                    if hdr_rev < part.min_api_rev() || hdr_rev > part.max_api_rev() {
                        Err(HsmErr::UnsupportedRevision)?
                    }
                } else if self.req_hdr.rev.is_some() {
                    Err(HsmErr::UnsupportedRevision)?
                }
            }
        }

        Ok(())
    }

    /// Get the session Id
    fn req_session_id(&self) -> Result<u16, HsmErr> {
        self.req_hdr.sess_id.ok_or(HsmErr::SessionExpected)
    }

    /// Get the user session
    fn user_session(&self, partition: &E::Partition) -> Result<E::UserSession, HsmErr> {
        partition.user_session(self.req_session_id()?, false)
    }

    /// Handle an error
    fn on_err(
        &mut self,
        session_id: Option<u16>,
        status: DdiStatus,
        op: DdiOp,
    ) -> Result<(), HsmErr> {
        // Create error response
        let resp = self.err_resp(session_id, status, op);

        // Encode the DMA buffer
        let buf =
            encode_buf(&resp, self.env.borrow().hal().dma_heap()).or(Err(HsmErr::CmdError))?;

        // Save the buffer for parent state machine to send the response
        self.dma_buf = Some(buf);

        // Fail the command
        Err(HsmErr::CmdError)
    }

    /// Create an error response
    fn err_resp(&self, session_id: Option<u16>, status: DdiStatus, op: DdiOp) -> DdiErrCmdResp {
        DdiErrCmdResp {
            hdr: DdiRespHdr {
                rev: self.req_hdr.rev,
                op,
                sess_id: session_id,
                status,
                fips_approved: false,
            },
            data: DdiErrResp {},
        }
    }

    /// Send completion queue entry
    fn send_flush_cqe(
        &mut self,
        session_id: u16,
        session_flags: HsmSessionFlags,
        tag: TagId,
    ) -> Result<(), HsmErr> {
        trace!("[tag: {}] send_flush_cqe", tag);
        let ioq = self.ioq.as_ref().ok_or(HsmErr::ExpectedIoQueue)?;

        let cqe = HsmCqe {
            dst_len: 0,
            session_flags,
            session_id,
            cmd_id: self.sqe.cmd.id(),
            sq_id: ioq.sq_id().into(),
            psf: PsfField::default().with_status(HostStatusCode::Success),
            app_vault_id: 0,
            ..Default::default()
        }
        .into();

        let desc = IoTxDesc {
            tx_queue_id: ioq.cq_id().into(),
            rx_queue_id: ioq.sq_id().into(),
            tag,
            entry: &cqe,
        };

        self.env
            .borrow()
            .hal()
            .io_channel()
            .begin_send(&desc)
            .map_err(|err| {
                error!("IO channel send error. (0x{:08x})", err);
                HsmErr::IoChannelSendError
            })?;

        self.state = HsmFsmState::Completion;

        Ok(())
    }

    fn flush_session(&mut self, tag: TagId) -> Result<(), HsmErr> {
        trace!("[tag: {}] flush_session", tag);
        let pfn = self.pfn.ok_or(HsmErr::ExpectedPcieFn)?;
        let session_id = self.sqe.session_id;

        // Retrieve the PCIe function
        let part = self.env.borrow().partition(pfn);

        if part.user_session(session_id, true).is_ok() {
            trace!("[tag: {}] flush_session app session", tag);
            self.session_flags.set_id_valid(true);
            self.session_flags.set_ctrl(HsmSessionControlKind::Close);

            let fsm: Box<dyn HsmCmdTrait<E>> =
                Box::new(FlushSessionCmd::new(session_id, part, pfn));

            self.cmd = Some(fsm);
            self.state = HsmFsmState::Cmd;

            // Start the command FSM
            self.process_cmd_event(HsmFsmEvent::StartCmd, tag)
        } else {
            part.flush_session(session_id);

            // Synchronously send completion queue entry
            self.send_flush_cqe(session_id, self.session_flags, tag)?;

            Err(HsmErr::Pending)
        }
    }
}
