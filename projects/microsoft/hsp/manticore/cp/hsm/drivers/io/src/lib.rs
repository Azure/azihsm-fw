// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod channel;
mod cntrl;
mod proxy_channel;
mod reg;

use bitfield_struct::bitfield;
pub use channel::IoChannel;
pub use cntrl::IoController;
use mcr_error::mcr_err_decl;
use mcr_error::McrResult;
use mcr_types::DevSqId;
use mcr_types::IoChannelId;
use mcr_types::IoControllerId;
use mcr_types::MemoryAddr;
use mcr_types::PcieFunction;
use mcr_types::VolatileCell;
pub use proxy_channel::IoProxyChannel;

/// Receive queue descriptor info
#[bitfield(u32)]
#[derive(Default)]
struct IoRxQueueDescInfo {
    /// Index of the entry in the queue
    queue_index: u16,

    /// Queue Id
    queue_id: u8,

    /// AXI Bus ID of the entry
    axi_id: u8,
}

/// Receive queue descriptor status
#[bitfield(u32)]
#[derive(Default)]
struct IoRxQueueDescStatus {
    /// Reserved
    #[bits(31)]
    _rsvd: u32,

    /// Flag indicating success or failure
    success: bool,
}

/// Receive queue descriptor
#[derive(Default, Clone, Copy)]
pub struct IoRxQueueDesc {
    /// Entry address
    addr: MemoryAddr,

    /// Entry Info
    info: IoRxQueueDescInfo,

    /// Entry Status
    status: IoRxQueueDescStatus,
}

/// Receive free list descriptor
pub type IoRxFreeListDesc = MemoryAddr;

/// Transmit Free List descriptor info
#[bitfield(u32)]
#[derive(Default)]
struct IoTxQueueDescInfo {
    /// Index of the entry in the queue
    queue_index: u16,

    /// Queue Id
    queue_id: u8,

    /// AXI Bus ID of the entry
    axi_id: u8,
}

/// Transmit Free List descriptor status
#[bitfield(u32)]
#[derive(Default)]
struct IoTxQueueDescStatus {
    /// Tag value to associate completion
    tag: u16,

    /// Reserved
    _fw_reserved: u8,

    /// Status bit for Tx Free List entry
    status: u8,
}

#[repr(C)]
/// Transmit Queue Descriptor
#[derive(Clone, Copy)]
pub struct IoTxQueueDesc {
    /// Entry address
    addr: MemoryAddr,

    /// Entry Info
    info: IoTxQueueDescInfo,

    // Status field
    status: IoTxQueueDescStatus,
}

/// Transmit queue descriptor info
#[bitfield(u32)]
#[derive(Default)]
struct IoTxFreeListDescInfo {
    /// Receive Queue Id
    rx_queue_id: u8,

    /// RxQueue credit increment value
    rx_credit: u8,

    /// Transmit Queue Id
    tx_queue_id: u8,

    /// AXI Bus ID of the destination
    axi_id: u8,
}

/// Transmit queue descriptor control
#[bitfield(u32)]
#[derive(Default)]
struct IoTxFreeListDescControl {
    /// Tag value to associate with incoming Tx direction
    tag: u16,

    /// Reserved
    _fw_reserved: u8,

    /// Control bits for Tx queue entry
    control: u8,
}

#[repr(C)]
/// Transmit Free List Descriptor
#[derive(Clone, Copy)]
pub struct IoTxFreeListDesc {
    /// Entry address
    addr: MemoryAddr,

    /// Entry Info
    info: IoTxFreeListDescInfo,

    // Entry Control
    control: IoTxFreeListDescControl,
}

// Transmit Aux list descriptor
pub type IoTxAuxListDesc = u16;

// Receive Entry
pub const IO_RX_ENTRY_SIZE: usize = 64;
pub type IoRxEntry = [u8; IO_RX_ENTRY_SIZE];

// Transmit Entry
pub const IO_TX_ENTRY_SIZE: usize = 16;
pub type IoTxEntry = [u8; IO_TX_ENTRY_SIZE];

/// Receive Entry Pool information
pub enum RxEntryPoolInfo {
    /// Receive Entry Pool
    EntryPool(&'static [IoRxEntry]),

    /// Receive Entry Pool Size
    Size(usize),
}

/// Transmit Entry Pool information
pub enum TxEntryPoolInfo {
    /// Transmit Entry Pool
    EntryPool(&'static mut [IoTxEntry]),

    /// Transmit Entry Pool Size
    Size(usize),
}

/// Channel Configuration
pub struct IoChannelConfig {
    /// Channel Id
    pub channel_id: IoChannelId,

    /// Recieve Queue
    pub rx_queue: &'static [IoRxQueueDesc],

    /// Receive Queue Shadow Producer Index
    pub rx_queue_pi: &'static VolatileCell<u32>,

    /// Receive Free List
    pub rx_free_list: &'static mut [IoRxFreeListDesc],

    /// Receive Entry pool info
    pub rx_entry_pool_info: RxEntryPoolInfo,

    /// Transmit Queue
    pub tx_queue: &'static [IoTxQueueDesc],

    /// Transmit Queue Shadow Producer Index
    pub tx_queue_pi: &'static VolatileCell<u32>,

    /// Receive Free List
    pub tx_free_list: &'static mut [IoTxFreeListDesc],

    /// Transmit Entry pool info
    pub tx_entry_pool_info: TxEntryPoolInfo,

    /// Enable or disable Transmit complete interrupts
    pub enable_irq: bool,
}

/// Io Descriptor
pub struct IoRxDesc {
    /// Submission Queue ID
    pub sq_id: DevSqId,

    /// Queue entry eaddress
    pub addr: u32,

    /// Queue Entry
    pub entry: IoRxEntry,

    /// PCIe Function
    pub pfn: PcieFunction,

    /// Status
    pub status: bool,
}

/// Transmit Descriptor
pub struct IoTxDesc<'a> {
    /// Transmit Queue ID
    pub tx_queue_id: u8,

    /// Receive Queue ID
    pub rx_queue_id: u8,

    /// User Tag for tracking this aynchronous Tx
    pub tag: u16,

    /// Queue Entry
    pub entry: &'a IoTxEntry,
}

/// Outbound completion queue status
#[derive(PartialEq, Eq)]
pub enum IoTxCompleteStatus {
    // No data was transferred. OQ PI was not incremented
    NoData = 0x1,

    // IoTxQueueFull
    TxQueueFull = 0x2,

    // IoTxQueue Overflow
    TxQueueOverFlow = 0x4,

    // IoTxEntry skipped
    TxEntrySkipped = 0x8,

    // Reserved
    Reserved0 = 0x10,

    // Reserved
    Reserved1 = 0x20,

    // Internal CPU interrupt control bit was set in IoTxQueueDesc
    LocalCpuInterrupted = 0x40,

    // TxQueue operation completed successfully
    Success = 0x80,

    // Undefined status returned from Tx completion hw
    Undefined = 0xFF,
}

impl From<u8> for IoTxCompleteStatus {
    fn from(value: u8) -> Self {
        match value {
            x if x == IoTxCompleteStatus::NoData as u8 => IoTxCompleteStatus::NoData,
            x if x == IoTxCompleteStatus::TxQueueFull as u8 => IoTxCompleteStatus::TxQueueFull,
            x if x == IoTxCompleteStatus::TxQueueOverFlow as u8 => {
                IoTxCompleteStatus::TxQueueOverFlow
            }
            x if x == IoTxCompleteStatus::TxEntrySkipped as u8 => {
                IoTxCompleteStatus::TxEntrySkipped
            }
            x if x == IoTxCompleteStatus::Reserved0 as u8 => IoTxCompleteStatus::Reserved0,
            x if x == IoTxCompleteStatus::Reserved1 as u8 => IoTxCompleteStatus::Reserved1,
            x if x == IoTxCompleteStatus::LocalCpuInterrupted as u8 => {
                IoTxCompleteStatus::LocalCpuInterrupted
            }
            x if x == IoTxCompleteStatus::Success as u8 => IoTxCompleteStatus::Success,
            _ => IoTxCompleteStatus::Undefined,
        }
    }
}

/// Transmit Complete
pub struct IoTxCompleteDesc {
    /// Queue ID
    pub queue_id: u8,

    /// Queue Index
    pub queue_index: u16,

    /// User Tag for tracking this aynchronous Tx
    pub tag: u16,

    /// Queue Entry
    pub status: IoTxCompleteStatus,
}

pub trait IoControllerTrait {
    /// Pause the inbound engine for this IO Controller
    fn pause_inbound(&self);

    /// Resume the inbound engine for this IO Controller
    fn resume_inbound(&self);
}

pub trait IoChannelTrait {
    /// Receive one new message from this channel if it is available
    ///
    /// # Returns
    ///
    /// * `McrResult<Option<IoRxDesc<RxEntry>>>` - A tuple with McrResult Ok or Err and an optional
    ///   Io Receive Descriptor consist of a template type
    fn begin_recv(&self) -> Option<IoRxDesc>;

    /// Complete the io receive queue operation by releasing `RxEntry` resource
    ///
    /// # Arguments
    ///
    /// * `addr`  - Address of the `RxEntry` to be released back to the FreeList pool
    /// * `sq_id` - Device submission queue id
    fn end_recv(&self, addr: u32, sq_id: DevSqId);

    /// Send a message asynchronously through this channel
    ///
    /// # Arguments
    ///
    /// * `desc` - Io Transmit descriptor to send an entry of type IoTxDesc<TxEntry>
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok or an appropriate Err
    fn begin_send(&self, desc: &IoTxDesc) -> McrResult<()>;

    /// Peek the tag of next completed transaction if available
    ///
    /// # Returns
    ///
    /// * `Option<u16>` - Tag of next completed Io transaction if available or None
    fn peek_tag(&self) -> Option<u16>;

    /// Process the send completion notification through this channel
    ///
    /// # Returns
    ///
    /// * `McrResult<Option<IoTxCompleteDesc>>` - A tuple with McrResult Ok or Err and an optional
    ///   Io Transmit Complete Descriptor
    fn end_send(&self) -> Option<IoTxCompleteDesc>;
}

mcr_err_decl! {
    IoController,
    IoControllerErr
    {
        // Tx Queue Full
        TxQueueFull = 0x1,

        // No Free TxEntry is available
        TxEntryNotAvailable = 0x2,

        // Invalid TxEntry supplied during release operations
        InvalidTxEntry = 0x3,

        // Receive operation invalid on this channel
        RxEntryNotValid = 0x4,

        // Receive operation invalid on this channel
        TxEntryNotPermitted = 0x5,

        // Invalid Controller Id
        InvalidControllerId = 0x6,

        // Rx Entry pool not valid
        RxEntryPoolNotValid = 0x7,

        // Tx Entry pool not valid
        TxEntryPoolNotValid = 0x8,

        // Io Controller already enabled
        IoControllerAlreadyEnabled = 0x9,

        // Inbound Completion Queue is already enabled
        InboundCQAlreadyEnabled = 0xA,

        // Destination Freelist is already enabled
        DestFreeListAlreadyEnabled = 0xB,

        // outbound Completion Queue is already enabled
        OutboundCQAlreadyEnabled = 0xC,

        // Outbound source list is already enabled
        OslAlreadyEnabled = 0xD,

        // Inbound Completion Queue is not enabled
        InboundCQNotEnabled = 0xE,

        // Destination Freelist is already enabled
        DestFreeListNotEnabled = 0xF,

        // outbound Completion Queue is already enabled
        OutboundCQNotEnabled = 0x10,

        // Outbound source list is already enabled
        OslNotEnabled = 0x11,
    }
}
