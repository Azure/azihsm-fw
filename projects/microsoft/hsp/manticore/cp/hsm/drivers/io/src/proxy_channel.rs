// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::rc::Rc;
use core::cell::RefCell;

use mcr_error::McrResult;
use mcr_types::IoChannelId;

use crate::reg::DestinationFreeList;
use crate::reg::InboundCompletionQueue;
use crate::reg::OutboundCompletionQueue;
use crate::reg::OutboundSourceList;
use crate::IoChannelConfig;
use crate::IoController;
use crate::IoControllerErr;
use crate::RxEntryPoolInfo;

/// Io proxy channel
#[derive(Clone)]
pub struct IoProxyChannel {
    _rimpl: Rc<RefCell<IoProxyChannelImpl>>,
}

impl IoProxyChannel {
    /// Create and enable a new instance of Io proxy channel
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io controller instance this channel belongs to
    /// * `config` - Io Channel config
    ///
    /// # Returns
    ///
    /// * `McrResult<IoProxyChannel>` - Ok() with IoProxyChannel object or an appropriate error code
    pub fn new(cntrl: IoController, config: IoChannelConfig) -> McrResult<Self> {
        let rimpl = IoProxyChannelImpl::new(cntrl, config)?;
        Ok(Self {
            _rimpl: Rc::new(RefCell::new(rimpl)),
        })
    }

    /// Open an existing instance of Io proxy channel
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io controller instance this channel belongs to
    /// * `channel_id` - Io Channel ID to be be opened
    ///
    /// # Returns
    ///
    /// * `McrResult<IoProxyChannel>` - Ok() with IoProxyChannel object or an appropriate error code
    pub fn open(cntrl: IoController, channel_id: IoChannelId) -> McrResult<Self> {
        let rimpl = IoProxyChannelImpl::open(cntrl, channel_id)?;

        Ok(Self {
            _rimpl: Rc::new(RefCell::new(rimpl)),
        })
    }
}

/// Io Proxy channel implementation
pub struct IoProxyChannelImpl {
    /// Io controller this proxy channel belongs to
    cntrl: IoController,

    /// Channel of this proxy Io channel
    channel_id: IoChannelId,
}

impl IoProxyChannelImpl {
    fn new(cntrl: IoController, config: IoChannelConfig) -> McrResult<Self> {
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

        // Retrieve the RxEntry size for proxy IO channel
        let rx_entry_size = match config.rx_entry_pool_info {
            RxEntryPoolInfo::EntryPool(_) => Err(IoControllerErr::RxEntryPoolNotValid),
            RxEntryPoolInfo::Size(size) => Ok(size),
        };
        let rx_entry_size = rx_entry_size?;

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
            rx_entry_size as u32,
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
        })
    }

    fn open(cntrl: IoController, channel_id: IoChannelId) -> McrResult<Self> {
        let ctrl_id = cntrl.id();

        // Check if the channel is already enabled, if so, return an error
        if !InboundCompletionQueue::enabled(ctrl_id, channel_id) {
            Err(IoControllerErr::InboundCQAlreadyEnabled)?
        }
        if !DestinationFreeList::enabled(ctrl_id, channel_id) {
            Err(IoControllerErr::DestFreeListAlreadyEnabled)?
        }
        if !OutboundCompletionQueue::enabled(ctrl_id, channel_id) {
            Err(IoControllerErr::OutboundCQAlreadyEnabled)?
        }
        if !OutboundSourceList::enabled(ctrl_id, channel_id) {
            Err(IoControllerErr::OslAlreadyEnabled)?
        }

        Ok(Self { cntrl, channel_id })
    }
}

impl Drop for IoProxyChannelImpl {
    fn drop(&mut self) {
        InboundCompletionQueue::disable(self.cntrl.id(), self.channel_id);
        DestinationFreeList::disable(self.cntrl.id(), self.channel_id);
        OutboundCompletionQueue::disable(self.cntrl.id(), self.channel_id);
        OutboundSourceList::disable(self.cntrl.id(), self.channel_id);
    }
}
