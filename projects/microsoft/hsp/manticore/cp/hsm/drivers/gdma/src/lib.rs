// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

use bitfield_struct::bitfield;

mod channel;
mod cntrl;

pub use channel::GdmaChannel;
pub use cntrl::GdmaController;
use mcr_error::mcr_err_decl;
use mcr_error::McrResult;
use mcr_types::MemoryAddr;
use mcr_types::MemoryLocation;
use mcr_types::VolatileCell;

#[cfg(feature = "mcr_test_hooks")]
use core::sync::atomic::{AtomicBool, Ordering};

#[cfg(feature = "mcr_test_hooks")]
pub(crate) static GDMA_DATA_STR_ERR_CONDITION: AtomicBool = AtomicBool::new(false);

/// GDMA Channel Identifier
#[derive(Clone, Copy)]
pub enum GdmaChannelId {
    /// Channel 0
    Channel0 = 0,

    /// Channel 1
    Channel1 = 1,

    /// Channel 2
    Channel2 = 2,

    /// Channel 3
    Channel3 = 3,

    /// Channel 4
    Channel4 = 4,

    /// Channel 5
    Channel5 = 5,

    /// Channel 6
    Channel6 = 6,

    /// Channel 7
    Channel7 = 7,
}

impl From<GdmaChannelId> for usize {
    fn from(value: GdmaChannelId) -> usize {
        value as Self
    }
}

/// GDMA Transmit Queue Descriptor Control Field
#[bitfield(u32)]
#[derive(Default)]
struct GdmaTxQueueDescControl {
    /// Reserved
    #[bits(11)]
    _rsvd1: u32,

    /// 0 is prp format, 1 is sgl format for source address
    src_fmt: bool,

    /// 0 is prp format, 1 is sgl format for destination address
    dst_fmt: bool,

    /// Reserved
    #[bits(3)]
    _rsvd3: u8,

    /// 16 bit software tag value, to identify at completion
    #[bits(16)]
    tag: u16,
}

/// GDMA Transmit Queue Descriptor Interface Select Field
#[bitfield(u32)]
#[derive(Default)]
struct GdmaTxQueueDescIfcSelect {
    /// AXI Port ID of the memory that contains the source buffer
    #[bits(8)]
    src_data: u8,

    /// AXI Port ID of the memory that contains the destination buffer
    #[bits(8)]
    dst_data: u8,

    /// AXI Port ID of the memory that contains the source PRP/SGL List(s)
    #[bits(8)]
    src_desc: u8,

    /// AXI Port ID of the memory that contains the destination PRP/SGL List(s)
    #[bits(8)]
    dst_desc: u8,
}

/// GDMA Trasnmit Queue Descriptor
#[repr(C)]
#[derive(Default, Clone, Copy)]
pub struct GdmaTxQueueDesc {
    /// DMA control fields
    ctrl: GdmaTxQueueDescControl,

    /// DMA source and destination interface select field
    ifc_select: GdmaTxQueueDescIfcSelect,

    /// Reserved
    _rsvd1: u32,

    /// Reserved
    _rsvd2: u32,

    /// Source length
    src_len: u32,

    /// Destination length
    dst_len: u32,

    /// Reserved
    _rsvd3: u32,

    /// Reserved
    _rsvd4: u32,

    /// Source PRP or SGL 0
    src_fst_desc_addr: MemoryAddr,

    /// Source PRP or SGL 1
    src_snd_desc_addr: MemoryAddr,

    /// Destination PRP or SGL 0
    dst_fst_desc_addr: MemoryAddr,

    /// Destination PRP or SGL 1
    dst_snd_desc_addr: MemoryAddr,
}

/// GDMA Receive Queue Descriptor Status
#[bitfield(u32)]
#[derive(Default)]
struct GdmaRxQueueDescStatus {
    /// Flag indicating if the GDMA
    success: bool,

    /// Reserved
    #[bits(15)]
    rsvd: u16,

    /// Tag
    #[bits(16)]
    tag: u16,
}

/// GDMA Receive Queue Descriptor
#[derive(Default, Clone, Copy)]
pub struct GdmaRxQueueDesc {
    /// Status
    status: GdmaRxQueueDescStatus,

    /// Rserved
    _rsvd1: u32,

    /// Rserved
    _rsvd2: u32,

    /// Rserved
    _rsvd3: u32,
}

/// GDMA Channel configuration
pub struct GdmaChannelConfig {
    /// Transmit queue
    pub tx_queue: &'static mut [GdmaTxQueueDesc],

    /// Transmit queue consumer index
    pub tx_queue_ci: &'static VolatileCell<u32>,

    /// Receive queue
    pub rx_queue: &'static [GdmaRxQueueDesc],

    /// Receive queue producer index
    pub rx_queue_pi: &'static VolatileCell<u32>,
}

/// GDMA Descriptor format
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum DmaDescFormat {
    /// Physical Region Page
    Prp,

    /// Scatter Gather List
    Sgl,
}

impl From<DmaDescFormat> for bool {
    fn from(value: DmaDescFormat) -> Self {
        value != DmaDescFormat::Prp
    }
}

/// GDMA Memory Descriptor
pub struct DmaMemoryDesc {
    /// Format
    pub fmt: DmaDescFormat,

    /// Location
    pub loc: MemoryLocation,

    /// Address
    pub addr: MemoryAddr,
}

/// GDMA Transaction
pub struct DmaTxnDesc {
    /// First source descriptor
    pub src_fst: DmaMemoryDesc,

    /// Second source descriptor
    pub src_snd: Option<DmaMemoryDesc>,

    /// First destination descriptor
    pub dst_fst: DmaMemoryDesc,

    /// Second destination descriptor
    pub dst_snd: Option<DmaMemoryDesc>,

    /// Length of the data to transafer
    pub len: u32,

    /// Tag to track the transaction completion
    pub tag: u16,
}

/// GDMA Transaction Completion Descriptor
pub struct DmaTxnCompletionDesc {
    /// Flag indicating if the GDMA completed successfully
    pub success: bool,

    /// Tag to track the transaction completion
    pub tag: u16,
}

pub trait GdmaChannelTrait {
    /// Start a Gdma transaction
    ///
    /// # Arguments
    ///
    /// * `txn` - Gdma Transaction Descriptor
    ///
    /// # Returns
    ///
    /// * `McrResult<())>` - Ok or an appropriate Err
    fn begin_txn(&self, txn: &mut DmaTxnDesc) -> McrResult<()>;

    /// Peek the tag of next completed transaction if available
    ///
    /// # Returns
    ///
    /// * `Option<u16>` - Tag of next completed transaction if available or None
    fn peek_tag(&self) -> Option<u16>;

    /// Complete a Gdma transaction
    ///
    /// # Returns
    ///
    /// * `Option<GdmaTransactionCompletionDesc>` - Gdma Transaction completion descriptor if
    ///   available or None
    fn end_txn(&self) -> Option<DmaTxnCompletionDesc>;
}

mcr_err_decl! {
    GdmaController,
    GdmaControllerErr {
        // Invalid descriptor
        InvalidDesc = 0x01,

        // Enabling a controller which was previously enabled
        AlreadyEnabled = 0x2,
    }
}
