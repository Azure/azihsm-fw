// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::rc::Rc;
use alloc::vec::Vec;
use core::cell::RefCell;

use mcr_error::McrResult;
use mcr_registers::ucd::inbound::core_ib_cmn::ib_cmn_cq::RegisterBlock as InboundCompletionQueueReg;
use mcr_registers::ucd::inbound::core_ib_cmn::ib_cmn_dfl::RegisterBlock as DestFreeListRegs;
use mcr_registers::ucd::outbound::core_ob_cmn::ob_cmn_cq::RegisterBlock as OutboundCompletionQueueReg;
use mcr_registers::ucd::outbound::core_ob_cmn::ob_cmn_osl::RegisterBlock as OutboundSourceListRegs;
use mcr_types::MemoryLocation;

use crate::reg::DestinationFreeList;
use crate::reg::InboundCompletionQueue;
use crate::reg::InboundQueue;
use crate::reg::IoCntrlReg;
use crate::reg::OutboundCompletionQueue;
use crate::reg::OutboundSourceList;
use crate::*;

/// Io Channel
#[derive(Clone)]
pub struct IoChannel {
    rimpl: Rc<RefCell<IoChannelImpl>>,
}

impl IoChannel {
    /// Create an instance of Io Channel and enable it
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller instance this channel belongs to
    /// * `config` - Io Channel configuration of type IoChannelConfig
    ///
    /// # Returns
    ///
    /// * `McrResult<IoChannel>` - Ok() with IoChannel object or an appropriate
    ///   error code
    pub(crate) fn new_with_enable(cntrl: IoController, config: IoChannelConfig) -> McrResult<Self> {
        Ok(Self {
            rimpl: Rc::new(RefCell::new(IoChannelImpl::new_with_enable(cntrl, config)?)),
        })
    }

    /// Create an instance of Io Channel that was previously created
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller instance this channel belongs to
    /// * `config` - Io Channel configuration of type IoChannelConfig
    ///
    /// # Returns
    ///
    /// * `McrResult<IoChannel>` - Ok() with IoChannel object or an appropriate
    pub(crate) fn open(cntrl: IoController, config: IoChannelConfig) -> McrResult<Self> {
        Ok(Self {
            rimpl: Rc::new(RefCell::new(IoChannelImpl::open(cntrl, config)?)),
        })
    }

    /// Returns the controller ID that this channel belongs to
    pub fn ctrl_id(&self) -> IoControllerId {
        self.rimpl.borrow().ctrl_id()
    }

    /// Returns this Io Channel ID
    pub fn id(&self) -> IoChannelId {
        self.rimpl.borrow().id()
    }
}

impl IoChannelTrait for IoChannel {
    /// Receive one new message from this channel if it is available
    fn begin_recv(&self) -> Option<IoRxDesc> {
        self.rimpl.borrow_mut().recv()
    }

    /// Complete the io receive queue operation by releasing `RxEntry` resource
    fn end_recv(&self, addr: u32, sq_id: DevSqId) {
        self.rimpl.borrow_mut().end_recv(addr, sq_id)
    }

    /// Send a message asynchronously through this channel
    fn begin_send(&self, desc: &IoTxDesc) -> McrResult<()> {
        self.rimpl.borrow_mut().send(desc)
    }

    /// Peek the tag of next message in transmit completion queue
    fn peek_tag(&self) -> Option<u16> {
        let desc = self.rimpl.borrow().desc()?;

        Some(desc.status.tag())
    }

    /// Process the send completion notification through this channel
    fn end_send(&self) -> Option<IoTxCompleteDesc> {
        self.rimpl.borrow_mut().send_complete()
    }
}

/// Io Channel Implementation
struct IoChannelImpl {
    /// Io Controller this channel belongs to
    cntrl: IoController,

    /// This Channel ID
    channel_id: IoChannelId,

    /// Inbound Completion Queue Registers
    rx_queue_reg: InboundCompletionQueueReg,

    /// Destination Free List Register
    dest_free_list_reg: DestFreeListRegs,

    /// Receive Queue
    rx_queue: &'static [IoRxQueueDesc],

    /// Receive Queue Shadow Producer Index
    rx_queue_pi: &'static VolatileCell<u32>,

    /// Receive Free List
    rx_free_list: &'static mut [IoRxFreeListDesc],

    /// Receive Entry pool
    rx_entry_pool: &'static [IoRxEntry],

    /// Outbound Completion Queue Registers
    tx_queue_reg: OutboundCompletionQueueReg,

    /// Outbound Source List Registers
    outbound_source_list_reg: OutboundSourceListRegs,

    /// Transmit queue
    tx_queue: &'static [IoTxQueueDesc],

    /// Transmit Queue Shadow Producer Index
    tx_queue_pi: &'static VolatileCell<u32>,

    /// Transmit Free List
    tx_free_list: &'static mut [IoTxFreeListDesc],

    // Tx Aux List
    tx_aux_list: TxAuxList,
}

impl IoChannelImpl {
    /// Create an instance of Io Channel implementation and enable it
    fn new_with_enable(cntrl: IoController, config: IoChannelConfig) -> McrResult<Self> {
        let ctrl_id = cntrl.id();

        // Check if the channel is already enabled, if so, return an error
        if InboundCompletionQueue::enabled(ctrl_id, config.channel_id) {
            Err(IoControllerErr::InboundCQAlreadyEnabled)?
        }
        if DestinationFreeList::enabled(ctrl_id, config.channel_id) {
            Err(IoControllerErr::DestFreeListAlreadyEnabled)?
        }
        if OutboundCompletionQueue::enabled(ctrl_id, config.channel_id) {
            Err(IoControllerErr::OutboundCQAlreadyEnabled)?
        }
        if OutboundSourceList::enabled(ctrl_id, config.channel_id) {
            Err(IoControllerErr::OslAlreadyEnabled)?
        }

        // Retrieve the RxEntry Pool from the RxEntryPoolInfo
        let rx_entry_pool = match config.rx_entry_pool_info {
            RxEntryPoolInfo::EntryPool(pool) => Ok(pool),
            RxEntryPoolInfo::Size(_) => Err(IoControllerErr::RxEntryPoolNotValid),
        };
        let rx_entry_pool = rx_entry_pool?;

        // Populate the free list entry with entry pool addresses for the hardware to consume
        for (fl_entry, rx_entry) in config.rx_free_list.iter_mut().zip(rx_entry_pool.iter()) {
            fl_entry.lo = rx_entry as *const IoRxEntry as u32;
            fl_entry.hi = 0
        }

        // Retrieve the TxEntry Pool from the TxEntryPoolInfo
        let tx_entry_pool = match config.tx_entry_pool_info {
            TxEntryPoolInfo::EntryPool(pool) => Ok(pool),
            TxEntryPoolInfo::Size(_) => Err(IoControllerErr::TxEntryPoolNotValid),
        };
        let tx_entry_pool = tx_entry_pool?;

        config.rx_queue_pi.set(0);
        InboundCompletionQueue::enable(
            ctrl_id,
            config.channel_id,
            config.enable_irq,
            config.rx_queue.as_ptr() as u32,
            config.rx_queue.len(),
            config.rx_queue_pi.as_ptr() as u32,
        );

        DestinationFreeList::enable(
            ctrl_id,
            config.channel_id,
            config.rx_free_list.as_ptr() as u32,
            config.rx_free_list.len() as u32,
            core::mem::size_of::<IoRxEntry>() as u32,
        );

        config.tx_queue_pi.set(0);
        OutboundCompletionQueue::enable(
            ctrl_id,
            config.channel_id,
            config.enable_irq,
            config.tx_queue.as_ptr() as u32,
            config.tx_queue.len(),
            config.tx_queue_pi.as_ptr() as u32,
        );

        OutboundSourceList::enable(
            ctrl_id,
            config.channel_id,
            config.tx_free_list.as_ptr() as u32,
            config.tx_free_list.len(),
        );

        Ok(Self {
            cntrl,
            channel_id: config.channel_id,
            rx_queue_reg: InboundCompletionQueueReg::cntrl_reg(ctrl_id),
            dest_free_list_reg: DestFreeListRegs::cntrl_reg(ctrl_id),
            rx_queue: config.rx_queue,
            rx_queue_pi: config.rx_queue_pi,
            rx_free_list: config.rx_free_list,
            rx_entry_pool,
            tx_queue_reg: OutboundCompletionQueueReg::cntrl_reg(ctrl_id),
            outbound_source_list_reg: OutboundSourceListRegs::cntrl_reg(ctrl_id),
            tx_queue: config.tx_queue,
            tx_queue_pi: config.tx_queue_pi,
            tx_free_list: config.tx_free_list,
            tx_aux_list: TxAuxList::new(tx_entry_pool),
        })
    }

    /// Open an instance of Io Channel that was previously created
    fn open(cntrl: IoController, config: IoChannelConfig) -> McrResult<Self> {
        let ctrl_id = cntrl.id();

        // Check if the channel is already enabled, if not, return an error
        if !InboundCompletionQueue::enabled(ctrl_id, config.channel_id) {
            Err(IoControllerErr::InboundCQNotEnabled)?
        }
        if !DestinationFreeList::enabled(ctrl_id, config.channel_id) {
            Err(IoControllerErr::DestFreeListNotEnabled)?
        }
        if !OutboundCompletionQueue::enabled(ctrl_id, config.channel_id) {
            Err(IoControllerErr::OutboundCQNotEnabled)?
        }
        if !OutboundSourceList::enabled(ctrl_id, config.channel_id) {
            Err(IoControllerErr::OslNotEnabled)?
        }

        // Retrieve the RxEntry Pool from the RxEntryPoolInfo
        let rx_entry_pool = match config.rx_entry_pool_info {
            RxEntryPoolInfo::EntryPool(pool) => Ok(pool),
            RxEntryPoolInfo::Size(_) => Err(IoControllerErr::RxEntryPoolNotValid),
        };
        let rx_entry_pool = rx_entry_pool?;

        // Retrieve the TxEntry Pool from the TxEntryPoolInfo
        let tx_entry_pool = match config.tx_entry_pool_info {
            TxEntryPoolInfo::EntryPool(pool) => Ok(pool),
            TxEntryPoolInfo::Size(_) => Err(IoControllerErr::TxEntryPoolNotValid),
        };
        let tx_entry_pool = tx_entry_pool?;

        Ok(Self {
            cntrl,
            channel_id: config.channel_id,
            rx_queue_reg: InboundCompletionQueueReg::cntrl_reg(ctrl_id),
            dest_free_list_reg: DestFreeListRegs::cntrl_reg(ctrl_id),
            rx_queue: config.rx_queue,
            rx_queue_pi: config.rx_queue_pi,
            rx_free_list: config.rx_free_list,
            rx_entry_pool,
            tx_queue_reg: OutboundCompletionQueueReg::cntrl_reg(ctrl_id),
            outbound_source_list_reg: OutboundSourceListRegs::cntrl_reg(ctrl_id),
            tx_queue: config.tx_queue,
            tx_queue_pi: config.tx_queue_pi,
            tx_free_list: config.tx_free_list,
            tx_aux_list: TxAuxList::new(tx_entry_pool),
        })
    }

    /// Return the controller ID that this channel belongs to
    fn ctrl_id(&self) -> IoControllerId {
        self.cntrl.id()
    }

    /// Returns this Io Channel ID
    fn id(&self) -> IoChannelId {
        self.channel_id
    }

    /// Get the RxEntry using the address of the entry
    fn entry(&self, addr: MemoryAddr) -> Option<&'static IoRxEntry> {
        let base = self.rx_entry_pool.as_ptr() as usize;
        let index = (addr.lo as usize - base) / core::mem::size_of::<IoRxEntry>();
        if index < self.rx_entry_pool.len() {
            Some(&self.rx_entry_pool[index])
        } else {
            None
        }
    }

    /// Receive one new message from this channel if it is available
    fn recv(&mut self) -> Option<IoRxDesc> {
        let regs = self.rx_queue_reg.at(self.id().0 as usize);

        // Perform queue empty check
        let pi = self.rx_queue_pi.get();
        let mut ci = regs.ci().read().cmpltn_q_ci();
        if pi == ci {
            return None;
        }

        // Copy the queue descriptor
        let desc = self.rx_queue[ci as usize];

        // Increment the completion index
        ci = (ci + 1) & (self.rx_queue.len() as u32 - 1);
        regs.ci().write(|w| w.cmpltn_q_ci(ci));

        let queue_entry = self.entry(desc.addr)?;

        Some(IoRxDesc {
            sq_id: DevSqId(desc.info.queue_id()),
            addr: queue_entry.as_ptr() as u32,
            entry: *queue_entry,
            pfn: self.pcie_fn(desc.info.axi_id())?,
            status: desc.status.success(),
        })
    }

    fn pcie_fn(&self, axi_id: u8) -> Option<PcieFunction> {
        if let Ok(location) = MemoryLocation::try_from(axi_id) {
            if let Ok(pcie_fn) = PcieFunction::try_from(location) {
                return Some(pcie_fn);
            }
        }
        None
    }

    /// Complete the io receive queue operation by releasing `RxEntry` resource
    fn end_recv(&mut self, addr: u32, sq_id: DevSqId) {
        let regs = self.dest_free_list_reg.at(self.id().0 as usize);

        let pi = regs.pi().read().ib_dest_free_list_pi();
        let new_pi = (pi + 1) & (self.rx_free_list.len() as u32 - 1);

        self.rx_free_list[new_pi as usize].lo = addr;
        self.rx_free_list[new_pi as usize].hi = 0;

        regs.pi().write(|w| w.ib_dest_free_list_pi(new_pi));

        InboundQueue::release_credit(self.ctrl_id(), sq_id.into());
    }

    /// Send a message asynchronously using TxQueue
    fn send(&mut self, desc: &IoTxDesc) -> McrResult<()> {
        let list_reg = self.outbound_source_list_reg.at(self.id().into());
        let pi = list_reg.pi().read().ob_src_list_pi();
        let ci = list_reg.ci().read().ob_src_list_ci();

        // Error out if the transmit queue is full (This is unlikely)
        let new_pi = (pi + 1) & (self.tx_free_list.len() as u32 - 1);
        if new_pi == ci {
            Err(IoControllerErr::TxQueueFull)?
        }

        // Acquire a TxEntry from the internal auxilary list
        let Some(tx_entry) = self.tx_aux_list.dequeue() else {
            return Err(IoControllerErr::TxEntryNotAvailable)?;
        };

        // Copy the TxEntry from user into the TxEntry acquired from auxiliary list
        *tx_entry = *desc.entry;

        // Get the next available Tx List descriptor to fill the new transmit message
        let tx_desc = &mut self.tx_free_list[pi as usize];

        // Fill the address of the message to be sent
        tx_desc.addr.lo = tx_entry as *const IoTxEntry as u32;

        // Fill the Tx Free List descriptor using user supplied Tx Descriptor
        tx_desc.addr.hi = 0;
        tx_desc.info.set_tx_queue_id(desc.tx_queue_id);
        tx_desc.info.set_rx_queue_id(desc.rx_queue_id);
        tx_desc.info.set_rx_credit(0);
        tx_desc.info.set_axi_id(MemoryLocation::Soc.into());
        tx_desc.control.set_tag(desc.tag);
        tx_desc.control.set_control(0);

        // Issue a memory barrier instruction to flush all the memory writes to target memory
        // before invoking the hardware in the next step to process the newly addes Tx message
        cortex_m::asm::dmb();

        // Write the updated producer index to transmit hardware to start sending the message
        list_reg.pi().write(|w| w.ob_src_list_pi(new_pi));

        Ok(())
    }

    /// Complete an asynchronous send operation previosuly sent using send() method
    fn send_complete(&mut self) -> Option<IoTxCompleteDesc> {
        // Get the Transmit Completion Queue Descriptor from the queue
        let desc = self.desc()?;

        // Release the TxEntry back to the auxiliary list, if the address returned by hardware
        // is out of bounds, this function will return false
        if !self.tx_aux_list.enqueue(desc.addr.lo) {
            return None;
        }

        // Increment the consumer index by 1 to indicate the hardware that one entry was consumed
        // by the firmware from the Tx Completion Queue with roll over condition
        let queue_reg = self.tx_queue_reg.at(self.id().into());
        let ci = queue_reg.ci().read().ob_cmpltn_q_ci();
        let ci = (ci + 1) & (self.tx_queue.len() as u32 - 1);
        queue_reg.ci().write(|w| w.ob_cmpltn_q_ci(ci));

        // Return Ok() with Optional IoTxCompleteDesc
        Some(IoTxCompleteDesc {
            queue_id: desc.info.queue_id(),
            queue_index: desc.info.queue_index(),
            tag: desc.status.tag(),
            status: desc.status.status().into(),
        })
    }

    /// Get the descriptor of the next available transmit complete message
    fn desc(&self) -> Option<IoTxQueueDesc> {
        let queue_reg = self.tx_queue_reg.at(self.id().into());
        let ci = queue_reg.ci().read().ob_cmpltn_q_ci();

        // If there is a completion entry available in the Transmit Completion Queue, then
        // return the descriptor of the entry, otherwise return None
        if self.tx_queue_pi.get() == ci {
            return None;
        }

        Some(self.tx_queue[ci as usize])
    }
}

impl Drop for IoChannelImpl {
    fn drop(&mut self) {
        let cntrl_id = self.ctrl_id();
        let channel_id = self.channel_id;

        InboundCompletionQueue::disable(cntrl_id, channel_id);
        DestinationFreeList::disable(cntrl_id, channel_id);
        OutboundCompletionQueue::disable(cntrl_id, channel_id);
        OutboundSourceList::disable(cntrl_id, channel_id);
    }
}

/// Transmit Auxiliary List
pub(crate) struct TxAuxList {
    /// Slice of entry pool of type TxEntry to manage in this auxiliary queue
    entry_pool: &'static mut [IoTxEntry],

    /// Vector to store the index of each TxEntry in the entry_pool slice
    aux_list: Vec<u16>,
}

impl TxAuxList {
    /// Create new instance of `TxAuxList<RxEntry>`
    ///
    /// # Arguments
    ///
    /// * `entry_pool` - A slice of mutable reference to TxEntry list
    ///
    /// # Returns
    ///
    /// * `TxAuxList<TxEntry>` - Transmit Auxiliary List instance
    pub(crate) fn new(entry_pool: &'static mut [IoTxEntry]) -> Self {
        let mut aux_list = Vec::with_capacity(entry_pool.len());

        // Populate the free list entry with the index of entry_pool
        for index in (0..entry_pool.len() as u16).rev() {
            aux_list.push(index);
        }

        Self {
            aux_list,
            entry_pool,
        }
    }

    /// Dequeue a `TxEntry` from Tx Auxiliary List
    ///
    /// # Returns
    ///
    /// * `Option<TxEntry>` - An optional mutable reference to a TxEntry if available
    pub(crate) fn dequeue(&mut self) -> Option<&mut IoTxEntry> {
        // Get the index of next free element in TxEntry pool
        let index = self.aux_list.pop()?;

        // Retrurn the reference of the next free TxEntry pool to be used for Transmit operation
        Some(&mut self.entry_pool[index as usize])
    }

    /// Enqueue a `TxEntry` to aux list
    ///
    /// # Arguments
    ///
    /// * `entry_addr` - Address of the TxEntry to be added to the Tx Auxiliary List
    ///
    /// # Returns
    ///
    /// * `bool` - true if the Tx entry addr is added back to the pool, false if the
    ///   addr is out of bounds.
    pub(crate) fn enqueue(&mut self, entry_addr: u32) -> bool {
        let base = self.entry_pool.as_ptr() as usize;

        // Determine the index of the supplied
        let index = (entry_addr as usize - base) / core::mem::size_of::<IoTxEntry>();

        // If the index derived from incoming entry_addr goes out of bound, return an error
        if index >= self.aux_list.capacity() {
            return false;
        }

        // Push the index of a free item back to Tx Auxiliary List
        self.aux_list.push(index as u16);

        true
    }
}
