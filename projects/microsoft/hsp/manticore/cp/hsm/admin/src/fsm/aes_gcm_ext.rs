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

/// FSM states for per-request handling
#[derive(Clone, Copy, PartialEq, Eq)]
enum AesGcmExtState {
    /// Initial State
    Idle,

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

    /// RAII guard that zeros SQE[20..51] when the request completes (covers
    /// Pending yields across DMA stages). FP is the single source of truth for
    /// restoring the IV and ServiceIndicator from meta.IV / key flags after
    /// Admin signals completion. Installed in process_request_queue, dropped
    /// in update_response_queue.
    sqe_restore: Option<SqeRestore>,
}

/// RAII helper: on Drop, wipes the 32 bytes of bulk key that FP placed at
/// SQE[20..51], then writes `unaligned_dst_data_len` to CQE offset 44-47.
/// The write must happen AFTER the wipe because offset 44-47 falls inside
/// the [20..52) key zone. Combining both operations in Drop guarantees the
/// correct ordering in a single place.
///
/// FP -- the single source of truth for host-visible CQE fields -- restores
/// the IV at SQE[20..31] and recomputes the ServiceIndicator at SQE[48..51]
/// after Admin signals completion.
struct SqeRestore {
    addr: usize,
    /// Value to write at CQE offset 44-47 after the key wipe.
    unaligned_dst_data_len: u32,
}

impl Drop for SqeRestore {
    fn drop(&mut self) {
        let sqe: &mut [u8] = mcr_mem_map::mem_addr_to_slice(self.addr, 128);
        sqe[20..52].fill(0);
        // CQE offset 44-47 was just zeroed; write the real value now.
        sqe[44..48].copy_from_slice(&self.unaligned_dst_data_len.to_le_bytes());
    }
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
                self.process_request_queue(tag)
            }
            (AesGcmExtState::DmaInInputData, AdminFsmEvent::DmaComplete) => {
                self.on_in_dma_complete(tag)
            }
            (AesGcmExtState::DmaOut, AdminFsmEvent::DmaComplete) => self.on_out_dma_complete(tag),
            (AesGcmExtState::DmaInInputData, AdminFsmEvent::AesGcmExtRequest)
            | (AesGcmExtState::DmaOut, AdminFsmEvent::AesGcmExtRequest) => Err(AdminErr::Pending),
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
            sqe_restore: None,
        }
    }

    /// Process AES GCM request queue
    fn process_request_queue(&mut self, tag: TagId) -> Result<(), AdminErr> {
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

        // FP placed the 32-byte AES bulk key into SQE[20..51] before
        // forwarding this request. Install the RAII wipe guard now -- it runs
        // when the request completes (success or error path) via
        // update_response_queue, which drops sqe_restore to fire its Drop
        // impl. The guard zeros SQE[20..51]. FP restores the IV and
        // ServiceIndicator after Admin signals completion -- Admin never
        // writes host-visible CQE fields.
        self.sqe_restore = Some(SqeRestore {
            addr: self.sqe_address,
            unaligned_dst_data_len: 0,
        });

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

        // GCM: Perform AES-GCM tag correction SoftAES operation. The bulk key
        // lives in SQE[20..51] (placed there by FP). We borrow it directly as
        // a slice -- no copy. The borrow is dropped before any mutable access
        // to the SQE via cqe_ref_mut(). The RAII guard installed in
        // process_request_queue wipes SQE[20..51] when the request completes;
        // FP restores the IV and ServiceIndicator after Admin signals completion.
        let sqe_for_key: &[u8] = mcr_mem_map::mem_addr_to_slice(self.sqe_address, 128);
        let bulk_key: &[u8] = &sqe_for_key[20..52];
        let corrected_tag = match self.ctx.soft_aes().aes_gcm_tag_correction(
            !is_decrypt,
            bulk_key,
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

        // Store the unaligned_dst_data_length on the RAII guard so it gets
        // written to CQE offset 44-47 after the key wipe in Drop.
        if let Some(ref mut restore) = self.sqe_restore {
            restore.unaligned_dst_data_len = unaligned_data_len as u32;
        }

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
        // CRITICAL: drop the SQE wipe guard BEFORE sending the response to FP.
        // Otherwise FP could read the GcmResponseEntry and hand the SQE address
        // to UCD before SqeRestore::drop runs, leaking key bytes to host via the
        // CQE DMA. SqeRestore::drop zeros SQE[20..51] then writes the deferred
        // unaligned_dst_data_length at offset 44-47 (inside the wiped zone).
        // FP will restore the IV and recompute ServiceIndicator after it sees
        // the GcmResponseEntry; Admin must not touch host-visible CQE fields.
        drop(self.sqe_restore.take());

        let gcm_resp = AesGcmRespEntry::new()
            .with_sqe_idx(self.sqe_idx)
            .with_status(self.status as u8);

        let _ = self.ctx.aes_gcm_resp_queue().send(gcm_resp).map_err(|_e| {
            error!("Failed to send AES GCM response entry to response queue");
        });

        let _ = self.unaligned_data_dma_buf.take();

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
