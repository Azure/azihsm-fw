// Copyright (c) Microsoft Corporation. All rights reserved.

#![warn(missing_docs)]

extern crate alloc;

use alloc::rc::Rc;
use core::cell::RefCell;

use bitfield::BitMut;
use mcr_error::McrResult;
use mcr_registers::gdma::completion_queue::RegisterBlock as CompletionQueueRegs;
use mcr_registers::gdma::delivery_queue::RegisterBlock as DeliveryQueueRegs;
use mcr_registers::gdma::RegisterBlock as GdmaCommonRegs;

use crate::*;

/// Gdma Channel
#[derive(Clone)]
pub struct GdmaChannel {
    rimpl: Rc<RefCell<GdmaChannelImpl>>,
}

impl GdmaChannel {
    /// Create a GdmaChannel using this Gdma Controller
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Gdma Controller this channel belongs to
    /// * `id` - Gdma Channel Id to be created of type GdmaChannelId
    /// * `config` - Gdma Channel configuration data to be used for creation of GdmaChannel
    ///
    /// # Returns
    ///
    /// * `McrResult<GdmaChannel>` - Ok with GdmaChannel instance or an Err
    pub(crate) fn create(
        cntrl: GdmaController,
        id: GdmaChannelId,
        config: GdmaChannelConfig,
    ) -> McrResult<GdmaChannel> {
        Ok(Self {
            rimpl: Rc::new(RefCell::new(GdmaChannelImpl::create(cntrl, id, config))),
        })
    }

    /// Returns the Gdma Channel Id of this Gdma Channel instance
    ///
    /// # Returns
    ///
    /// * `GdmaChannelId` - Id of this Gdma Channel
    pub fn id(&self) -> GdmaChannelId {
        self.rimpl.borrow().id()
    }
}

impl GdmaChannelTrait for GdmaChannel {
    /// Start a Gdma transaction
    fn begin_txn(&self, txn: &mut DmaTxnDesc) -> McrResult<()> {
        self.rimpl.borrow_mut().start_transaction(txn)
    }

    /// Peek the tag of next completed transaction if available
    fn peek_tag(&self) -> Option<u16> {
        self.rimpl.borrow_mut().tag()
    }

    /// Complete a Gdma transaction
    fn end_txn(&self) -> Option<DmaTxnCompletionDesc> {
        self.rimpl.borrow_mut().complete()
    }
}

struct GdmaChannelImpl {
    /// Gdma Controller
    _cntrl: GdmaController,

    /// Channel Id
    id: GdmaChannelId,

    /// Gdma TxQueue instance
    tx_queue: GdmaTxQueue,

    /// Gdma RxQueue instance
    rx_queue: GdmaRxQueue,
}

impl GdmaChannelImpl {
    /// Create a new instance of Gdma Channel impl
    fn new(cntrl: GdmaController, id: GdmaChannelId, config: GdmaChannelConfig) -> GdmaChannelImpl {
        Self {
            _cntrl: cntrl,
            id,
            tx_queue: GdmaTxQueue::create(id, config.tx_queue, config.tx_queue_ci),
            rx_queue: GdmaRxQueue::create(id, config.rx_queue, config.rx_queue_pi),
        }
    }

    // Create a new instance of Gdma Channel imple and enables it
    fn create(
        cntrl: GdmaController,
        id: GdmaChannelId,
        config: GdmaChannelConfig,
    ) -> GdmaChannelImpl {
        let mut channel_rimpl = Self::new(cntrl, id, config);

        channel_rimpl.tx_queue.enable();
        channel_rimpl.rx_queue.enable();

        channel_rimpl
    }

    fn id(&self) -> GdmaChannelId {
        self.id
    }

    fn start_transaction(&mut self, txn: &DmaTxnDesc) -> McrResult<()> {
        self.tx_queue.start_transaction(txn)
    }

    fn complete(&mut self) -> Option<DmaTxnCompletionDesc> {
        self.rx_queue.complete()
    }

    fn tag(&self) -> Option<u16> {
        let desc = self.rx_queue.desc()?;

        Some(desc.status.tag())
    }
}

/// Gdma Transmit Queue
struct GdmaTxQueue {
    /// Slice of Gdma Tx Descriptor Queue
    queue: &'static mut [GdmaTxQueueDesc],

    // Gdma Tx Queue consumer index
    ci: &'static VolatileCell<u32>,

    /// Gdma Tx Queue register interface
    regs: DeliveryQueueRegs,
}

impl GdmaTxQueue {
    /// Create an instance of Gdma Tx Queue
    ///
    /// # Arguments
    ///
    /// * `id` - Gdma Channel Id this Tx Queue belongs to
    /// * `queue` - Gdma Tx Descriptor Queue
    /// * `ci` - Gdma Tx Queue consumer index
    ///
    /// # Returns
    ///
    /// * `GdmaTxQueue` - Gdma Tx Queue instance
    fn create(
        id: GdmaChannelId,
        queue: &'static mut [GdmaTxQueueDesc],
        ci: &'static VolatileCell<u32>,
    ) -> GdmaTxQueue {
        Self {
            queue,
            ci,
            regs: DeliveryQueueRegs::block(id.into()),
        }
    }

    /// Enable Gdma Tx Queue
    fn enable(&mut self) {
        // Gdma Tx Queue Base address
        self.regs
            .base_address_low()
            .write(|_| self.queue.as_ptr() as u32);

        // Set the consumer index value to be at 0 to mark the beginning of the queue
        self.ci.set(0);

        // Program the Tx Queue consumer index
        self.regs
            .consumer_index_shadow_address_low()
            .write(|_| self.ci.as_ptr() as u32);

        // Program Gdma Tx Queue size, interface select and CI shadow enable fields
        self.regs.configuration().read_and_modify(|_, w| {
            w.dlvry_q0_cnsmr_indx_shdw_en(true)
                .dlvry_q0_intrfc_sel(0)
                .dlvry_q0_sz(self.queue.len() as u32)
        });

        // Enable Gdma Tx Queue
        self.regs
            .configuration()
            .read_and_modify(|_, w| w.dlvry_q0_en(true));
    }

    /// Start a Gdma transaction
    ///
    /// # Arguments
    ///
    /// * `txn` - Gdma Transaction Descriptor
    ///
    /// # Returns
    ///
    /// * `McrResult<())>` - Ok or an appropriate Err
    fn start_transaction(&mut self, txn: &DmaTxnDesc) -> McrResult<()> {
        let mut pi = self.regs.producer_index().read().dlvry_q0_prdcr_indx();

        let desc = &mut self.queue[pi as usize];

        // Clear the descriptor
        *desc = GdmaTxQueueDesc::default();

        // Set Source Descriptor
        desc.src_len = txn.len;
        desc.ctrl.set_src_fmt(txn.src_fst.fmt.into());
        desc.src_fst_desc_addr = txn.src_fst.addr;
        if let Some(ref src_desc1) = txn.src_snd {
            desc.src_snd_desc_addr = src_desc1.addr;
        }
        desc.ifc_select.set_src_data(txn.src_fst.loc.into());
        desc.ifc_select.set_src_desc(txn.src_fst.loc.into());

        // Set Destination Descriptor
        desc.dst_len = txn.len;
        #[cfg(feature = "mcr_test_hooks")]
        {
            if GDMA_DATA_STR_ERR_CONDITION.load(Ordering::Relaxed) {
                desc.dst_len = 0; // mismatch: src_len = txn.len, dst_len = 0
                GDMA_DATA_STR_ERR_CONDITION.store(false, Ordering::Relaxed); //auto clear
            }
        }
        desc.ctrl.set_dst_fmt(txn.dst_fst.fmt.into());
        desc.dst_fst_desc_addr = txn.dst_fst.addr;
        if let Some(ref dst_desc1) = txn.dst_snd {
            desc.dst_snd_desc_addr = dst_desc1.addr;
        }
        desc.ifc_select.set_dst_data(txn.dst_fst.loc.into());
        desc.ifc_select.set_dst_desc(txn.dst_fst.loc.into());

        // Set DMA Tag
        desc.ctrl.set_tag(txn.tag);

        // Increment producer index with rollover condition
        pi += 1;
        if pi as usize == self.queue.len() {
            pi = 0;
        }

        // Issue memory barrier instruction to flush all the writes before writing the producer
        // index to the Gdma Tx queue hardware
        cortex_m::asm::dmb();

        // Publish the new producer index
        self.regs
            .producer_index()
            .write(|w| w.dlvry_q0_prdcr_indx(pi));

        Ok(())
    }
}

impl Drop for GdmaTxQueue {
    fn drop(&mut self) {
        // Disable Gdma Tx Queue
        self.regs
            .configuration()
            .read_and_modify(|_, w| w.dlvry_q0_en(false));
    }
}

/// Gdma Receive Queue
struct GdmaRxQueue {
    /// Gdma Channel Id
    channel_id: GdmaChannelId,

    /// Gdma Rx Descriptor Queue
    queue: &'static [GdmaRxQueueDesc],

    /// Gdma Rx Queue producer index
    pi: &'static VolatileCell<u32>,

    /// Gdma Rx Queue register interface
    regs: CompletionQueueRegs,
}

impl GdmaRxQueue {
    /// Create an instance of Gdma Rx Queue
    ///
    /// # Arguments
    ///
    /// * `id` - Gdma Channel Id this Tx Queue belongs to
    /// * `queue` - Gdma Rx Descriptor Queue
    /// * `pi` - Gdma Rx Queue producer index
    ///
    /// # Returns
    ///
    /// * `GdmaRxQueue` - Gdma Rx Queue instance
    fn create(
        id: GdmaChannelId,
        queue: &'static [GdmaRxQueueDesc],
        pi: &'static VolatileCell<u32>,
    ) -> GdmaRxQueue {
        Self {
            channel_id: id,
            queue,
            pi,
            regs: CompletionQueueRegs::block(id.into()),
        }
    }

    //// Enable Gdma Rx Queue
    fn enable(&mut self) {
        // Program Gdma Rx queue Base Address
        self.regs
            .base_address_low()
            .write(|_| self.queue.as_ptr() as u32);
        self.regs.base_address_high().write(|_| 0);

        // Set the default value of producer index to 0 to mark the initial state of the queue
        self.pi.set(0);

        // Program the producer index address
        self.regs
            .producer_index_shadow_address_low()
            .write(|_| self.pi.as_ptr() as u32);

        // Set the consumer index value in the Gdma Rx queue register interface to be 0
        self.regs
            .consumer_index()
            .write(|w| w.cmpltn_q0_cnsmr_indx(0));

        // Enable interrupt for Gdma Rx Queue
        let mut int_en = 0u32;
        int_en.set_bit(self.channel_id as usize, true);

        let cmn_regs = GdmaCommonRegs::block();
        cmn_regs
            .interrupt_enable_0()
            .read_and_modify(|r, _| r | int_en);

        // Program Gdma Rx Queue size, interface select and PI shadow enable fields
        self.regs.configuration().read_and_modify(|_, w| {
            w.cmplt_q0_prdcr_indx_shdw_en(true)
                .cmplt_q0_intrfc_sel(0)
                .cmplt_q0_sz(self.queue.len() as u32)
        });

        // Enable the Gdma Rx Queue
        self.regs
            .configuration()
            .read_and_modify(|_, w| w.cmplt_q0_en(true));
    }

    /// Complete a Gdma transaction
    ///
    /// # Returns
    ///
    /// * `Option<GdmaTransactionCompletionDesc>` - Gdma Transaction completion descriptor if
    ///   available or None
    fn complete(&mut self) -> Option<DmaTxnCompletionDesc> {
        // Consume one new entry from Gdma Rx engine produced completions
        let desc = self.desc()?;

        // Increment the consumer index with rollover condition
        let mut ci = self.regs.consumer_index().read().cmpltn_q0_cnsmr_indx();
        ci += 1;
        if ci as usize == self.queue.len() {
            ci = 0;
        }

        self.regs
            .consumer_index()
            .write(|w| w.cmpltn_q0_cnsmr_indx(ci));

        Some(DmaTxnCompletionDesc {
            success: desc.status.success(),
            tag: desc.status.tag(),
        })
    }

    /// Get the descriptor of the next completed transaction
    ///
    /// # Returns
    ///
    /// * `Option<GdmaRxQueueDesc>` - Gdma Rx Queue descriptor if available or None
    fn desc(&self) -> Option<GdmaRxQueueDesc> {
        let ci = self.regs.consumer_index().read().cmpltn_q0_cnsmr_indx();

        // There is a new entry in the completion queue
        if self.pi.get() == ci {
            return None;
        }

        Some(self.queue[ci as usize])
    }
}

impl Drop for GdmaRxQueue {
    fn drop(&mut self) {
        // Disable Gdma Rx Queue
        self.regs.configuration().write(|w| w.cmplt_q0_en(false));
    }
}
