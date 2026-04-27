// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::boxed::Box;

use mcr_gdma_controller::*;
use mcr_logging::*;

use super::types::*;
use super::*;
use crate::env::*;
use crate::function::AdminQueue;
use crate::function::FunctionMgrTrait;
use crate::function::FunctionTrait;

/// Admin FSM State
#[derive(Clone, Copy, Default, PartialEq, Eq)]
enum AdminFsmState {
    /// Initial State
    #[default]
    Init,

    /// Dma In state
    DmaIn,

    /// Command state
    Cmd,

    /// DMA Out state
    DmaOut,

    /// Completion state
    Completion,
}

/// Admin FSM (Finite State Machine)
pub(crate) struct AdminCmdFsm<E: AdminEnvTrait + 'static> {
    /// Current state of the FSM
    state: AdminFsmState,

    /// PCIe function number this command belongs to
    pfn: Option<PcieFunction>,

    /// Admin SQE
    sqe: AdminSqe,

    /// Admin SQE address
    sqe_addr: u32,

    /// Admin Queue Object
    queue: Option<AdminQueue>,

    /// Admin CQE
    cqe: Option<AdminCqe>,

    /// DMA buffer
    dma_buf: Option<DmaBuffer<E>>,

    /// Command FSM
    cmd: Option<Box<dyn AdminCmdTrait<E>>>,

    /// Context
    ctx: AdminFsmContext<E>,
}

impl<E: AdminEnvTrait> CmdFsm for AdminCmdFsm<E> {
    type Error = AdminErr;
    type ResourceId = ResId;
    type Event = AdminFsmEvent;
    type Recorder = AdminFsmEventRecorder;

    /// On event
    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match (self.state, event) {
            (AdminFsmState::Init, AdminFsmEvent::RxReady) => self.on_rx_ready(tag),
            (AdminFsmState::DmaIn, AdminFsmEvent::DmaComplete) => self.on_in_dma_complete(tag),
            (AdminFsmState::Cmd, _) => self.on_cmd_event(event, tag),
            (AdminFsmState::DmaOut, AdminFsmEvent::DmaComplete) => self.on_out_dma_complete(tag),
            (AdminFsmState::Completion, AdminFsmEvent::TxComplete) => self.on_tx_complete(),
            _ => self.on_unexpected_event(tag, event),
        }
    }

    /// On resource acquisition
    fn acquire_resource(&mut self, tag: TagId, id: Self::ResourceId) -> Self::Event {
        if self.state != AdminFsmState::Cmd {
            unreachable!();
        }

        // We should have a valid fsm in this state
        let Some(fsm) = self.cmd.as_mut() else {
            unreachable!();
        };

        // Call the command FSM's acquire resource
        fsm.acquire_resource(tag, id)
    }
}

impl<E: AdminEnvTrait> AdminCmdFsm<E> {
    /// Create a new AdminFsm
    pub fn new(ctx: AdminFsmContext<E>) -> Self {
        Self {
            state: Default::default(),
            pfn: Default::default(),
            sqe: Default::default(),
            sqe_addr: Default::default(),
            queue: None,
            cqe: Default::default(),
            dma_buf: Default::default(),
            cmd: Default::default(),
            ctx,
        }
    }

    /// On a new Admin command entry receive
    fn on_rx_ready(&mut self, tag: TagId) -> Result<(), AdminErr> {
        // Begin recieve operation
        let desc = self.begin_recv_sqe()?;

        self.queue = self.ctx.function_mgr().function(desc.pfn).admin_queue();

        // Get the submission queue Id belong to this command
        let sq_id = self.sq_id().inspect_err(|_err| {
            self.ctx.io_channel().end_recv(desc.addr, desc.sq_id);
        })?;

        // Vadlidate if the SQE belongs to the Admin Queue
        if desc.sq_id != sq_id {
            self.ctx.io_channel().end_recv(desc.addr, desc.sq_id);
            Err(AdminErr::InvalidAdminQueue)?
        }

        // Setup the fsm context
        self.pfn = Some(desc.pfn);
        self.sqe = desc.entry.into();
        self.sqe_addr = desc.addr;

        if self.sqe.cmd.op.requires_in_dma() {
            self.begin_in_dma(tag)
        } else {
            // Initialize command FSM
            self.init_cmd()?;

            // Start the command FSM
            self.on_cmd_event(AdminFsmEvent::StartCmd, tag)
        }
    }

    /// On an inbound DMA completion event
    fn on_in_dma_complete(&mut self, tag: TagId) -> Result<(), AdminErr> {
        let sq_id = self.sq_id()?;

        self.process_in_dma_complete(tag).inspect_err(|err| {
            if !err.pending() {
                self.ctx.io_channel().end_recv(self.sqe_addr, sq_id);
            }
        })
    }

    /// On an event reception when the AdminFsm is in Cmd state
    fn on_cmd_event(&mut self, event: AdminFsmEvent, tag: TagId) -> Result<(), AdminErr> {
        let sq_id = self.sq_id()?;

        self.process_cmd_event(event, tag).inspect_err(|err| {
            if !err.pending() {
                self.ctx.io_channel().end_recv(self.sqe_addr, sq_id);
            }
        })
    }

    /// On DMA out complete event
    fn on_out_dma_complete(&mut self, tag: TagId) -> Result<(), AdminErr> {
        let sq_id = self.sq_id()?;

        self.process_out_dma_complete(tag).inspect_err(|err| {
            if !err.pending() {
                self.ctx.io_channel().end_recv(self.sqe_addr, sq_id);
            }
        })
    }

    /// On IO Transmit complete
    fn on_tx_complete(&mut self) -> Result<(), AdminErr> {
        // Complete the recieve operation.
        let result = self.end_recv_sqe();
        self.ctx.io_channel().end_recv(self.sqe_addr, self.sq_id()?);

        result
    }

    /// Handle an unexpected event to the state machine
    fn on_unexpected_event(&self, _tag: u16, _event: AdminFsmEvent) -> Result<(), AdminErr> {
        error!(
            "Invalid state transition. Current state = {}",
            self.state as u32
        );

        Err(AdminErr::Pending)
    }

    /// Begin receieve submission queue entry operation
    fn begin_recv_sqe(&self) -> Result<IoRxDesc, AdminErr> {
        let desc = self
            .ctx
            .io_channel()
            .begin_recv()
            .ok_or(AdminErr::IoChannelRecvNone)?;

        if !desc.status {
            Err(AdminErr::IoChannelRecvErr)?
        }

        Ok(desc)
    }

    /// Process DMA out complete event
    fn process_in_dma_complete(&mut self, tag: u16) -> Result<(), AdminErr> {
        // Complete DMA operation
        self.end_dma(tag)
            .or_else(|err| self.send_err_cqe(err, tag))?;

        // Initialize command FSM
        self.init_cmd()?;

        // Start the command FSM
        self.on_cmd_event(AdminFsmEvent::StartCmd, tag)
    }

    /// Process a command event receive while AdminFsm is in Cmd state
    fn process_cmd_event(&mut self, event: AdminFsmEvent, tag: TagId) -> Result<(), AdminErr> {
        // We should have a valid fsm in this state
        let fsm = self.cmd.as_mut().ok_or(AdminErr::ExpectedCmdFsm)?;

        (self.cqe, self.dma_buf) = match fsm.on_event(event, tag) {
            Err(AdminErr::Pending) => (None, None),

            // Pull the Completion Queue Entry out of Command FSM to end it
            Ok(_) | Err(_) => fsm.response(),
        };

        // If there is a completion queue entry (CQE) populated by the command FSM, then we can
        // end the Admin IO and send the CQE
        if self.cqe.is_some() {
            // Return if the Admin Queue owning the SQE became invalid
            self.queue_valid()?;

            // If we have a Cqe, command FSM is in complete state. Hence
            // we can drop it.
            self.cmd.take();

            // Start DMA out transaction
            if self.dma_buf.is_some() {
                self.begin_out_dma(tag)
                    .or_else(|err| self.send_err_cqe(err, tag))?;
            } else {
                self.send_cqe(tag)?;
            }
        }

        Err(AdminErr::Pending)
    }

    /// Process DMA out complete event
    fn process_out_dma_complete(&mut self, tag: u16) -> Result<(), AdminErr> {
        // Release the outbound DMA buffer
        self.dma_buf.take();

        // Complete DMA operation
        self.end_dma(tag)
            .or_else(|err| self.send_err_cqe(err, tag))?;

        // Send completion queue entry
        self.send_cqe(tag)?;

        Err(AdminErr::Pending)
    }

    /// Complete receive submission queue entry opetation
    fn end_recv_sqe(&mut self) -> Result<(), AdminErr> {
        let send_complete_desc = self
            .ctx
            .io_channel()
            .end_send()
            .ok_or(AdminErr::IoChannelSendCompleteNone)?;

        if send_complete_desc.status != IoTxCompleteStatus::Success {
            Err(AdminErr::IoChannelSendCompleteError)?;
        }

        Ok(())
    }

    /// Begin inbound DMA operation
    fn begin_in_dma(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.dma_buf = self
            .ctx
            .dma_heap()
            .allocate(core::mem::size_of::<VmLiveMigrationInfo>());
        let mut desc = self.make_in_dma_desc(tag)?;

        self.ctx
            .dma_channel()
            .begin_txn(&mut desc)
            .map_err(|_| AdminErr::DmaStartError)?;

        self.state = AdminFsmState::DmaIn;

        Err(AdminErr::Pending)
    }

    /// Initialize command processing
    fn init_cmd(&mut self) -> Result<(), AdminErr> {
        let pfn = self.pfn.ok_or(AdminErr::ExpectedPcieFn)?;
        let fsm: Box<dyn AdminCmdTrait<E>> = match self.sqe.cmd.op {
            AdminCommandOpCodes::DeleteSq => {
                Box::new(AdminDeleteSqCmd::new(self.ctx.clone(), pfn, self.sqe, None))
            }
            AdminCommandOpCodes::CreateSq => {
                Box::new(AdminCreateSqCmd::new(self.ctx.clone(), pfn, self.sqe, None))
            }
            AdminCommandOpCodes::DeleteCq => {
                Box::new(AdminDeleteCqCmd::new(self.ctx.clone(), pfn, self.sqe, None))
            }
            AdminCommandOpCodes::CreateCq => {
                Box::new(AdminCreateCqCmd::new(self.ctx.clone(), pfn, self.sqe, None))
            }
            AdminCommandOpCodes::Identify => {
                Box::new(AdminIdentifyCmd::new(self.ctx.clone(), pfn, self.sqe, None))
            }
            AdminCommandOpCodes::SetFeatures => Box::new(AdminSetFeaturesCmd::new(
                self.ctx.clone(),
                pfn,
                self.sqe,
                None,
            )),
            AdminCommandOpCodes::GetFeatures => Box::new(AdminGetFeaturesCmd::new(
                self.ctx.clone(),
                pfn,
                self.sqe,
                None,
            )),
            AdminCommandOpCodes::SetRes => {
                Box::new(AdminSetResCmd::new(self.ctx.clone(), pfn, self.sqe, None))
            }
            AdminCommandOpCodes::GetRes => {
                Box::new(AdminGetResCmd::new(self.ctx.clone(), pfn, self.sqe, None))
            }
            AdminCommandOpCodes::VfPrep => Box::new(AdminVfPrepCmd::new(pfn, self.sqe)),
            AdminCommandOpCodes::VfStop => {
                Box::new(AdminVfStopCmd::new(pfn, self.ctx.clone(), self.sqe))
            }
            AdminCommandOpCodes::VfStart => {
                Box::new(AdminVfStartCmd::new(pfn, self.ctx.clone(), self.sqe))
            }
            AdminCommandOpCodes::VfSave => {
                Box::new(AdminVfSaveCmd::new(pfn, self.ctx.clone(), self.sqe))
            }
            AdminCommandOpCodes::VfRestore => {
                let buf = self.dma_buf.take().ok_or(AdminErr::ExpectedDmaBuf)?;
                Box::new(AdminVfRestoreCmd::new(pfn, self.ctx.clone(), self.sqe, buf))
            }
            _ => Box::new(AdminUnsupportedCmd::new(self.sqe)),
        };

        self.cmd = Some(fsm);
        self.state = AdminFsmState::Cmd;

        Ok(())
    }

    /// Begin outbound DMA operation
    fn begin_out_dma(&mut self, tag: TagId) -> Result<(), AdminErr> {
        let mut desc = self.make_out_dma_desc(tag)?;

        self.ctx
            .dma_channel()
            .begin_txn(&mut desc)
            .map_err(|_| AdminErr::DmaStartError)?;

        self.state = AdminFsmState::DmaOut;

        Ok(())
    }

    /// Complete inbound or outbound DMA operation
    fn end_dma(&self, tag: TagId) -> Result<(), AdminErr> {
        let desc = self.ctx.dma_channel().end_txn();
        match desc {
            Some(desc) => {
                if !desc.success {
                    Err(AdminErr::DmaEndErr)?
                }
                if desc.tag != tag {
                    Err(AdminErr::DmaTagMismatch)?
                }
            }
            None => return Err(AdminErr::DmaCompletionEmpty)?,
        };
        Ok(())
    }

    /// Create inbound DMA descriptor
    fn make_in_dma_desc(&mut self, tag: u16) -> Result<DmaTxnDesc, AdminErr> {
        let pfn = self.pfn.ok_or(AdminErr::ExpectedPcieFn)?;
        let buf = self.dma_buf.as_ref().ok_or(AdminErr::ExpectedDmaBuf)?;

        let desc = DmaTxnDesc {
            src_fst: DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: pfn.into(),
                addr: self.sqe.prp1,
            },
            src_snd: Some(DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: pfn.into(),
                addr: MemoryAddr {
                    lo: buf.len() as u32,
                    hi: 0,
                },
            }),
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
    fn make_out_dma_desc(&mut self, tag: u16) -> Result<DmaTxnDesc, AdminErr> {
        let pfn = self.pfn.ok_or(AdminErr::ExpectedPcieFn)?;
        let buf = self.dma_buf.as_ref().ok_or(AdminErr::ExpectedDmaBuf)?;

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
                fmt: DmaDescFormat::Sgl,
                loc: pfn.into(),
                addr: self.sqe.prp1,
            },
            dst_snd: Some(DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: pfn.into(),
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

    /// Send error completion queue entry
    fn send_err_cqe<T>(&mut self, err: AdminErr, tag: TagId) -> Result<T, AdminErr> {
        // Check if the Admin queue is valid and get the submission and completion queue Ids
        self.queue_valid()?;
        let sq_id = self.sq_id()?;
        let cq_id = self.cq_id()?;

        let cqe = AdminCqe {
            sq_id: sq_id.into(),
            cmd_id: self.sqe.cmd.id,
            psf: StatusField::default().with_status(err.into()),
            ..Default::default()
        };

        let desc = IoTxDesc {
            tx_queue_id: cq_id.into(),
            rx_queue_id: sq_id.into(),
            tag,
            entry: &cqe.into(),
        };

        self.ctx
            .io_channel()
            .begin_send(&desc)
            .map_err(|_| AdminErr::IoChannelSendError)?;

        self.state = AdminFsmState::Completion;

        Err(AdminErr::Pending)
    }

    /// Send completion queue entry
    fn send_cqe(&mut self, tag: TagId) -> Result<(), AdminErr> {
        // Check if the Admin queue is valid and get the submission and completion queue Ids
        self.queue_valid()?;
        let sq_id = self.sq_id()?;
        let cq_id = self.cq_id()?;

        let mut cqe = self.cqe.take().ok_or(AdminErr::ExpectedCqe)?;

        cqe.sq_id = sq_id.into();

        let desc = IoTxDesc {
            tx_queue_id: cq_id.into(),
            rx_queue_id: sq_id.into(),
            tag,
            entry: &cqe.into(),
        };

        self.ctx
            .io_channel()
            .begin_send(&desc)
            .map_err(|_| AdminErr::IoChannelSendError)?;

        self.state = AdminFsmState::Completion;

        Ok(())
    }

    /// Check if the Admin Queue owning the SQE is valid
    fn queue_valid(&self) -> Result<(), AdminErr> {
        let queue = self.queue.as_ref().ok_or(AdminErr::ExpectedAdminQueue)?;

        if !queue.valid() {
            Err(AdminErr::InvalidAdminQueue)?
        }

        Ok(())
    }

    /// Submission Queue Id this command belongs to
    fn sq_id(&self) -> Result<DevSqId, AdminErr> {
        self.queue
            .as_ref()
            .map(|q| q.sq_id())
            .ok_or(AdminErr::ExpectedAdminQueue)
    }

    /// Completion Queue Id this command belongs to
    fn cq_id(&self) -> Result<DevCqId, AdminErr> {
        self.queue
            .as_ref()
            .map(|q| q.cq_id())
            .ok_or(AdminErr::ExpectedAdminQueue)
    }
}
