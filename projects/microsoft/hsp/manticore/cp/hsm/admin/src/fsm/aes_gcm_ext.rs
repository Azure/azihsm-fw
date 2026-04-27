// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

use crate::context::AdminFsmContext;
use crate::env::AdminEnvTrait;
use crate::error::AdminErr;
use crate::recorder::AdminFsmEventRecorder;
use crate::resource::AdminFsmResourceId;
use crate::AdminFsmEvent;

use mcr_crypto_softaes::*;
use mcr_gdma_controller::DmaDescFormat;
use mcr_gdma_controller::DmaMemoryDesc;
use mcr_gdma_controller::DmaTxnDesc;
use mcr_gdma_controller::GdmaChannelTrait;
use mcr_simplex::SimplexPipeTrait;
use mcr_types::AesGcmExtRespErr;
use mcr_types::GetBulkKeyReqEntry;
use zerocopy::IntoBytes;
use zeroize::Zeroize;

/// FSM states for per-request handling
#[derive(Clone, Copy, PartialEq, Eq)]
enum AesGcmExtState {
    /// Initial State
    Idle,

    /// Waiting for bulk key response from HSM via simplex queue
    WaitKeyResponse,

    /// DMA input data
    DmaInInputData,

    /// DMA output data
    DmaOut,
}

/// FSM that handles AES-GCM bulk operations tag correction
pub(crate) struct AesGcmExtFsm<E: AdminEnvTrait + 'static> {
    /// Context
    ctx: AdminFsmContext<E>,

    /// PCIe function number this command belongs to
    pfn: Option<PcieFunction>,

    /// DMA buffer for unaligned input data
    unaligned_data_dma_buf: Option<DmaBuffer<E>>,

    /// Internal FSM state for an in-flight GCM request
    state: AesGcmExtState,

    /// SQE index
    sqe_idx: u32,

    /// SQE address to read various fields from SQE
    sqe_address: usize,

    /// Status field to write back in response CQE
    status: AesGcmExtRespErr,

    /// AES GCM 256-bit bulk key
    key: [u32; 8],
}

impl<E: AdminEnvTrait> CmdFsm for AesGcmExtFsm<E> {
    type Error = AdminErr;
    type ResourceId = AdminFsmResourceId;
    type Event = AdminFsmEvent;
    type Recorder = AdminFsmEventRecorder;

    /// Handle an event
    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        let result = match (self.state, event) {
            (AesGcmExtState::Idle, AdminFsmEvent::AesGcmExtRequest) => {
                self.pre_process_request_queue(tag)
            }
            (AesGcmExtState::WaitKeyResponse, AdminFsmEvent::GetBulkKeyResponse) => {
                self.process_request_queue(tag)
            }
            (AesGcmExtState::DmaInInputData, AdminFsmEvent::DmaComplete) => {
                self.on_in_dma_complete(tag)
            }
            (AesGcmExtState::DmaOut, AdminFsmEvent::DmaComplete) => self.on_out_dma_complete(tag),
            (AesGcmExtState::DmaInInputData, AdminFsmEvent::AesGcmExtRequest)
            | (AesGcmExtState::DmaOut, AdminFsmEvent::AesGcmExtRequest)
            | (AesGcmExtState::WaitKeyResponse, AdminFsmEvent::AesGcmExtRequest) => {
                Err(AdminErr::Pending)
            }
            _ => self.on_unexpected_event(tag, event),
        };

        // In case of error (i.e. not Pending), send a response to FP core
        if let Err(ref e) = result {
            if !matches!(e, AdminErr::Pending) {
                info!("Statemachine error {}", *e as u32);
                if self.status == AesGcmExtRespErr::Success {
                    self.status = AesGcmExtRespErr::AesGcmTagCorrectionFailed;
                }
                self.update_response_queue();
            }
        }

        Err(AdminErr::Pending)
    }
}

impl<E: AdminEnvTrait> AesGcmExtFsm<E> {
    /// Create a new AesGcmExtFsm
    pub fn new(ctx: AdminFsmContext<E>) -> Self {
        Self {
            ctx,
            pfn: None,
            unaligned_data_dma_buf: None,
            state: AesGcmExtState::Idle,
            sqe_idx: 0,
            sqe_address: 0,
            status: AesGcmExtRespErr::Success,
            key: [0u32; 8],
        }
    }

    /// Pre-Process request queue
    fn pre_process_request_queue(&mut self, _tag: TagId) -> Result<(), AdminErr> {
        self.status = AesGcmExtRespErr::Success;

        // If Spurious request detected, return Pending but dont send any response.
        let gcm_req = self
            .ctx
            .aes_gcm_req_queue()
            .recv()
            .ok_or(AdminErr::Pending)?;

        // Store SQE index from Request
        self.sqe_idx = gcm_req.sqe_idx();

        // Store pfn value from Request
        self.pfn = match PcieFunction::try_from(gcm_req.pfn()) {
            Ok(pfn) => Some(pfn),
            Err(_) => {
                self.status = AesGcmExtRespErr::InvalidPcieFn;
                Err(AdminErr::ExpectedPcieFn)?
            }
        };

        // Get SQE content from sqe_addr; validate non-zero
        self.sqe_address = gcm_req.sqe_addr() as usize;
        if self.sqe_address == 0 {
            self.status = AesGcmExtRespErr::InvalidSqeAddrPtr;
            Err(AdminErr::InvalidEvent)?
        }

        self.send_bulk_key_request()
    }

    /// Send a bulk key request to HSM over dedicated simplex queue
    fn send_bulk_key_request(&mut self) -> Result<(), AdminErr> {
        let cdma_key_id = AesBulk256KeyId::from(self.sqe_ref().cmd.key_idx as u16);

        let req = GetBulkKeyReqEntry {
            key_index: cdma_key_id.key_index(),
            resource_id: cdma_key_id.vault_id(),
            pfn: self.pfn.unwrap().0,
            _rsvd: 0,
        };

        self.ctx
            .get_bulk_key_req_queue()
            .send(req)
            .map_err(|_| AdminErr::IpcSendRequestError)?;

        self.state = AesGcmExtState::WaitKeyResponse;

        Err(AdminErr::Pending)
    }

    /// Process bulk key response from HSM simplex queue
    fn process_bulk_key_response(&mut self) -> Result<(), AdminErr> {
        let mut response = self
            .ctx
            .get_bulk_key_resp_queue()
            .recv()
            .ok_or(AdminErr::SpuriousIpcMessage)?;

        if response.status != 0 {
            self.status = AesGcmExtRespErr::AesGcmKeyBlobReadFailed;
            Err(AdminErr::IpcResponseError)?;
        }

        self.key = response.key;

        response.key.zeroize();

        Ok(())
    }

    /// Process AES GCM request queue
    fn process_request_queue(&mut self, tag: TagId) -> Result<(), AdminErr> {
        self.process_bulk_key_response()?;

        let sqe_ref = self.sqe_ref();

        // Sanity check for unaligned data length
        let unaligned_data_length = sqe_ref.cmd.unaligned_src_data_length as usize;

        if unaligned_data_length >= 16 {
            self.status = AesGcmExtRespErr::InvalidUnalignedDataLength;
            Err(AdminErr::InvalidEvent)?
        }

        if unaligned_data_length == 0 {
            // If there is no unaligned data, proceed with tag correction directly
            self.correct_incoming_tag().inspect_err(|_e| {
                self.status = AesGcmExtRespErr::AesGcmTagCorrectionFailed;
            })?;

            // Success response
            self.update_response_queue();
        } else {
            // Allocate local DMA buffer
            self.unaligned_data_dma_buf = self.ctx.dma_heap().allocate(unaligned_data_length);

            // Begin inbound DMA transaction (host -> SOC local buffer)
            self.begin_in_dma(tag)?;
        }

        Err(AdminErr::Pending)
    }

    /// Tag Correction
    fn correct_incoming_tag(&mut self) -> Result<(), AdminErr> {
        let sqe_ref = self.sqe_ref();
        let cqe_ref = self.cqe_ref();

        let supplied_tag = sqe_ref.cmd.tag;

        // Handle unaligned data: if buffer exists, use it; otherwise use empty data
        let empty = [0u8; 0];
        let (unaligned_input_data, unaligned_data_len) =
            if let Some(ref buf) = self.unaligned_data_dma_buf {
                (buf.as_ref(), buf.len())
            } else {
                (empty.as_slice(), 0)
            };

        // Get Enc/Dec command type from CQE
        // For decrypt: op == true, encrypt: op == false
        let is_decrypt = cqe_ref.attr.op();

        // Output buffer using SoftAes GCM tag correction function
        let source_data_len = cqe_ref.output_data_length as usize;
        let aligned_text_len = source_data_len.saturating_sub(sqe_ref.cmd.aad_length as usize);
        let input_text_len = aligned_text_len + unaligned_data_len;

        let intermediate_tag = if source_data_len == 0 {
            None
        } else {
            Some(&cqe_ref.tag)
        };

        // Prepare output buffer for unaligned data (if any)
        let mut output_buffer = self
            .ctx
            .dma_heap()
            .allocate(unaligned_data_len)
            .ok_or(AdminErr::NoMemory)?;

        // GCM: Perform AES-GCM tag correction SoftAES operation
        let corrected_tag = match self.ctx.soft_aes().aes_gcm_tag_correction(
            !is_decrypt,
            self.key.as_bytes(),
            &sqe_ref.cmd.iv,
            sqe_ref.cmd.unpadded_aad_length as u64,
            input_text_len as u64,
            None,
            intermediate_tag,
            unaligned_input_data,
            aligned_text_len,
            output_buffer.as_ref_mut(),
        ) {
            Ok(tag) => tag,
            Err(_e) => {
                self.status = AesGcmExtRespErr::AesGcmTagCorrectionFailed;
                Err(AdminErr::AesGcmTagCorrectionFailed)?
            }
        };

        if is_decrypt && corrected_tag != supplied_tag {
            info!("AES-GCM Decrypt Tag Mismatch");
            self.status = AesGcmExtRespErr::AesGcmInvalidDecryptTag;
        }

        // Write corrected_tag on CQE tag field
        let cqe_ref_mut = self.cqe_ref_mut();
        cqe_ref_mut.tag.copy_from_slice(&corrected_tag);
        cqe_ref_mut.unaligned_dst_data_length = unaligned_data_len as u32;

        // Copy output back to DMA buffer if it exists
        if let Some(ref buf) = self.unaligned_data_dma_buf {
            let mut range = IoMemRange::from(buf.as_ref());
            range.slice_mut().copy_from_slice(output_buffer.as_ref());
        }

        Ok(())
    }

    /// Handle inbound GDMA completion: modify buffer and start outbound GDMA
    fn on_in_dma_complete(&mut self, tag: TagId) -> Result<(), AdminErr> {
        // Complete DMA transaction first
        self.end_dma_txn(tag).inspect_err(|_e| {
            self.status = AesGcmExtRespErr::DmaInOperationErr;
        })?;

        self.correct_incoming_tag().inspect_err(|_e| {
            self.status = AesGcmExtRespErr::AesGcmTagCorrectionFailed;
        })?;

        // Build outbound TX descriptor and begin outbound DMA (SOC -> host)
        self.begin_out_dma(tag)
    }

    /// Handle outbound GDMA completion: cleanup and enqueue response
    fn on_out_dma_complete(&mut self, tag: TagId) -> Result<(), AdminErr> {
        // Complete DMA operation
        if let Err(e) = self.end_dma_txn(tag) {
            self.status = AesGcmExtRespErr::DmaOutOperationErr;
            Err(e)?
        }

        // Success response
        self.update_response_queue();

        Err(AdminErr::Pending)
    }

    /// Begin inbound DMA: host (PF) -> SOC local buffer
    fn begin_in_dma(&mut self, tag: TagId) -> Result<(), AdminErr> {
        let mut rx_desc = self.prepare_in_dma_desc(tag).map_err(|e| {
            self.map_desc_prep_error(
                e,
                AesGcmExtRespErr::DmaInOperationErr,
                AesGcmExtRespErr::InvalidUnalignedSrcDataPtr,
            )
        })?;

        if self.ctx.dma_channel().begin_txn(&mut rx_desc).is_err() {
            self.status = AesGcmExtRespErr::DmaInOperationErr;
            Err(AdminErr::NoMemory)?
        }

        self.state = AesGcmExtState::DmaInInputData;

        Err(AdminErr::Pending)
    }

    /// Begin outbound DMA: SOC local buffer -> host (PF)
    fn begin_out_dma(&mut self, tag: TagId) -> Result<(), AdminErr> {
        let mut tx_desc = self.prepare_out_dma_desc(tag).map_err(|e| {
            self.map_desc_prep_error(
                e,
                AesGcmExtRespErr::DmaOutOperationErr,
                AesGcmExtRespErr::InvalidUnalignedDstDataPtr,
            )
        })?;

        self.ctx
            .dma_channel()
            .begin_txn(&mut tx_desc)
            .map_err(|_| {
                self.status = AesGcmExtRespErr::DmaOutOperationErr;
                AdminErr::DmaStartError
            })?;

        self.state = AesGcmExtState::DmaOut;

        Ok(())
    }

    /// Helper method to map errors for DMA descriptor preparation
    fn map_desc_prep_error(
        &mut self,
        e: AdminErr,
        dma_op_err: AesGcmExtRespErr,
        unaligned_ptr_err: AesGcmExtRespErr,
    ) -> AdminErr {
        self.status = match e {
            AdminErr::ExpectedPcieFn => AesGcmExtRespErr::InvalidPcieFn,
            AdminErr::ExpectedDmaBuf => dma_op_err,
            AdminErr::InvalidEvent => AesGcmExtRespErr::InvalidSqeAddrPtr,
            AdminErr::InvalidAesGcmUnalignedDataPtr => unaligned_ptr_err,
            _ => dma_op_err,
        };
        e
    }

    /// Prepare inbound DMA descriptor: host (PF) -> SOC local buffer
    fn prepare_in_dma_desc(&mut self, tag: TagId) -> Result<DmaTxnDesc, AdminErr> {
        let pfn = self.pfn.ok_or(AdminErr::ExpectedPcieFn)?;

        let (unaligned_src_data_ptr, unaligned_src_data_length) = {
            let sqe_ref = self.sqe_ref();
            let src_ptr = sqe_ref.cmd.unaligned_src_data_ptr;
            let src_len = sqe_ref.cmd.unaligned_src_data_length;
            (src_ptr, src_len)
        };

        if unaligned_src_data_ptr.hi == 0 && unaligned_src_data_ptr.lo == 0 {
            Err(AdminErr::InvalidAesGcmUnalignedDataPtr)?
        }

        let rx_desc = DmaTxnDesc {
            src_fst: DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: pfn.into(),
                addr: unaligned_src_data_ptr,
            },
            src_snd: Some(DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: pfn.into(),
                addr: MemoryAddr {
                    lo: unaligned_src_data_length as u32,
                    hi: 0,
                },
            }),
            dst_fst: DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: MemoryLocation::Soc,
                addr: self
                    .unaligned_data_dma_buf
                    .as_ref()
                    .ok_or(AdminErr::ExpectedDmaBuf)?
                    .as_ref()
                    .into(),
            },
            dst_snd: Some(DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: MemoryLocation::Soc,
                addr: MemoryAddr {
                    lo: unaligned_src_data_length as u32,
                    hi: 0,
                },
            }),
            len: unaligned_src_data_length as u32,
            tag,
        };

        Ok(rx_desc)
    }

    /// Prepare outbound DMA descriptor: SOC local buffer -> host (PF)
    fn prepare_out_dma_desc(&self, tag: TagId) -> Result<DmaTxnDesc, AdminErr> {
        let pfn = self.pfn.ok_or(AdminErr::ExpectedPcieFn)?;
        let out_buf = self
            .unaligned_data_dma_buf
            .as_ref()
            .ok_or(AdminErr::ExpectedDmaBuf)?;

        let sqe_ref = self.sqe_ref();
        if sqe_ref.cmd.unaligned_dst_data_ptr.hi == 0 && sqe_ref.cmd.unaligned_dst_data_ptr.lo == 0
        {
            Err(AdminErr::InvalidAesGcmUnalignedDataPtr)?
        }

        if sqe_ref.cmd.unaligned_dst_data_length != out_buf.len() as u8 {
            Err(AdminErr::InvalidAesGcmUnalignedDataPtr)?
        }

        let tx_desc = DmaTxnDesc {
            src_fst: DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: MemoryLocation::Soc,
                addr: out_buf.as_ref().into(),
            },
            src_snd: Some(DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: MemoryLocation::Soc,
                addr: MemoryAddr {
                    lo: out_buf.len() as u32,
                    hi: 0,
                },
            }),
            dst_fst: DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: pfn.into(),
                addr: sqe_ref.cmd.unaligned_dst_data_ptr,
            },
            dst_snd: Some(DmaMemoryDesc {
                fmt: DmaDescFormat::Sgl,
                loc: pfn.into(),
                addr: MemoryAddr {
                    lo: out_buf.len() as u32,
                    hi: 0,
                },
            }),
            len: sqe_ref.cmd.unaligned_dst_data_length as u32,
            tag,
        };

        Ok(tx_desc)
    }

    /// Complete DMA transaction: call end_txn() and validate result
    fn end_dma_txn(&mut self, tag: u16) -> Result<(), AdminErr> {
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
            None => Err(AdminErr::DmaCompletionEmpty)?,
        };

        Ok(())
    }

    /// Send response to FP core over AES GCM response queue
    fn update_response_queue(&mut self) {
        let gcm_resp = AesGcmRespEntry::new()
            .with_sqe_idx(self.sqe_idx)
            .with_status(self.status as u8);

        let _ = self.ctx.aes_gcm_resp_queue().send(gcm_resp).map_err(|_e| {
            error!("Failed to send AES GCM response entry to response queue");
        });

        let _ = self.unaligned_data_dma_buf.take();

        // Zeroize the key after use
        self.key = [0u32; 8];

        // Set to Idle state for next request processing
        self.state = AesGcmExtState::Idle;
    }

    /// Immutable access to SQE structure.
    fn sqe_ref(&self) -> &CdmaIoGcmSqe {
        let sqe_slice: &'static [CdmaIoGcmSqe] =
            mcr_mem_map::mem_addr_to_slice(self.sqe_address, 1);

        &sqe_slice[0]
    }

    /// Immutable access to CQE structure.
    fn cqe_ref(&self) -> &CdmaIoCqe {
        let cqe_slice: &'static [CdmaIoCqe] = mcr_mem_map::mem_addr_to_slice(self.sqe_address, 1);

        &cqe_slice[0]
    }

    /// Mutable access to CQE structure.
    fn cqe_ref_mut(&mut self) -> &mut CdmaIoCqe {
        let cqe_slice: &'static mut [CdmaIoCqe] =
            mcr_mem_map::mem_addr_to_slice(self.sqe_address, 1);

        &mut cqe_slice[0]
    }

    /// Handle an unexpected event to the state machine
    fn on_unexpected_event(&self, _tag: u16, _event: AdminFsmEvent) -> Result<(), AdminErr> {
        error!(
            "Invalid state transition in AesGcmExtFsm. Current state = {}",
            self.state as u32
        );

        Err(AdminErr::Pending)
    }
}
