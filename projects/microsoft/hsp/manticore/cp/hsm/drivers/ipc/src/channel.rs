// Copyright (c) Microsoft Corporation. All rights reserved.

#![warn(missing_docs)]

extern crate alloc;

use alloc::rc::Rc;
use core::cell::RefCell;

use mcr_error::McrResult;
use mcr_registers::ipc_intc::ipc_intc_descriptor_registers::RegisterBlock as IpcDescriptorRegs;
use mcr_registers::ipc_intc::ipc_intc_int_registers::RegisterBlock as IpcIntRegs;

use crate::*;

/// Ipc Message Channel
#[derive(Clone)]
pub struct IpcMessageChannel {
    rimpl: Rc<RefCell<IpcMessageChannelImpl>>,
}

impl IpcMessageChannel {
    /// Create an instance of IPC message channel
    ///
    /// # Arguments
    ///
    /// * `id` - Ipc message channel Id to be created
    /// * `config` - Ipc message channel configuration `IpcMessageChannelConfig`
    /// * `cntrl` - Ipc controller
    ///
    /// # Returns
    ///
    /// * `IpcMessageChannel` - An instance of Ipc message channel object
    pub(crate) fn create(
        id: IpcChannelId,
        config: IpcMessageChannelConfig,
        cntrl: IpcController,
    ) -> IpcMessageChannel {
        Self {
            rimpl: Rc::new(RefCell::new(IpcMessageChannelImpl::create(
                id, config, cntrl,
            ))),
        }
    }

    /// Get the channel Id of this Ipc message channel
    ///
    /// # Returns
    ///
    /// * `IpcChannelId` - Ipc message channel Id
    pub fn id(&self) -> IpcChannelId {
        self.rimpl.borrow().id()
    }
}

impl IpcMessageChannelTrait for IpcMessageChannel {
    /// Send a request message of type `IpcMessage` using this Ipc channel
    fn send_request(&self, tag: u16, message: IpcMessage) -> McrResult<()> {
        self.rimpl.borrow_mut().send_request(tag, message)
    }

    /// Send a response message of type `IpcMessage` using this Ipc channel
    fn send_response(&self, message: IpcMessage) -> McrResult<()> {
        self.rimpl.borrow_mut().send_message(message)
    }

    /// Receive a message of type `IpcMessage` using this Ipc channel
    fn receive_message(&self) -> Option<IpcMessage> {
        self.rimpl.borrow_mut().receive_message()
    }

    /// Peek the tag for pending IPC essage in receive queue
    fn peek_tag(&self) -> Option<u16> {
        self.rimpl.borrow().tag()
    }

    /// Poll for a new message of type `IpcMessage` using this Ipc channel
    fn poll_message(&self) -> Option<IpcMessage> {
        self.rimpl.borrow_mut().poll_message()
    }
}

/// Ipc message channel implementation
struct IpcMessageChannelImpl {
    /// Ipc message channel Id
    id: IpcChannelId,

    /// Ipc message channel configuration
    config: IpcMessageChannelConfig,

    /// Ipc Controller
    cntrl: IpcController,

    /// Current messsage tag
    tag: Option<u16>,
}

impl IpcMessageChannelImpl {
    fn create(
        id: IpcChannelId,
        config: IpcMessageChannelConfig,
        cntrl: IpcController,
    ) -> IpcMessageChannelImpl {
        let int_reg_block = IpcIntRegs::block();
        let int_reg = int_reg_block.at(config.int_block.into());

        int_reg
            .int_enable_set()
            .read_and_modify(|r, _| r | (1u32 << (config.receive_message_descriptor as u32)));

        Self {
            id,
            config,
            cntrl,
            tag: None,
        }
    }

    fn id(&self) -> IpcChannelId {
        self.id
    }

    fn send_request(&mut self, tag: u16, message: IpcMessage) -> McrResult<()> {
        if self.tag.is_some() {
            Err(IpcControllerErr::ChannelBusy)?
        }

        self.send_message(message)?;

        self.tag = Some(tag);

        Ok(())
    }

    fn send_message(&mut self, message: IpcMessage) -> McrResult<()> {
        let ci = self.config.tx_queue.ci.get();
        let pi = self.config.tx_queue.pi.get();

        let mut new_pi = pi + 1;
        if new_pi as usize == self.config.tx_queue.queue.len() {
            new_pi = 0;
        }

        if new_pi == ci {
            Err(IpcControllerErr::MessageQueueFull)?
        }

        let message_dest = &mut self.config.tx_queue.queue[pi as usize];
        for (index, item) in message.data.iter().enumerate() {
            message_dest.data[index] = *item
        }

        self.config.tx_queue.pi.set(new_pi);

        let desc_reg_block = IpcDescriptorRegs::block();
        let desc_reg = desc_reg_block.at(self.config.send_message_descriptor as usize);

        cortex_m::asm::dmb();

        desc_reg.ipc_desc_reg_n().write(|_| new_pi);

        Ok(())
    }

    fn tag(&self) -> Option<u16> {
        self.tag
    }

    fn receive_message(&mut self) -> Option<IpcMessage> {
        let mut ci = self.config.rx_queue.ci.get();
        let pi = self.config.rx_queue.pi.get();

        if pi == ci {
            return None;
        }

        let mut message = IpcMessage {
            data: [0; IPC_MESSAGE_LENGTH],
        };

        let queue_entry = &mut self.config.rx_queue.queue[ci as usize];

        // Rx queue of IPC channel is in a memory that only allows 4 bytes aligned access
        for (index, item) in queue_entry.data.iter_mut().enumerate() {
            message.data[index] = *item;
            *item = 0; // Clear the message after reading
        }

        // Increment the consumer index with rollover condition
        ci += 1;
        if ci as usize == self.config.rx_queue.queue.len() {
            ci = 0;
        }

        self.config.rx_queue.ci.set(ci);

        // Release the tag
        self.tag.take();

        Some(message)
    }

    /// Poll for a new message of type `IpcMessage` using this Ipc channel
    fn poll_message(&mut self) -> Option<IpcMessage> {
        let rx_descriptor = self.config.receive_message_descriptor;

        if self.cntrl.is_pend_and_clr(rx_descriptor) {
            self.receive_message()
        } else {
            None
        }
    }
}

impl Drop for IpcMessageChannelImpl {
    fn drop(&mut self) {
        let int_reg_block = IpcIntRegs::block();
        let int_reg = int_reg_block.at(self.config.int_block.into());

        int_reg
            .int_enable_clr()
            .read_and_modify(|r, _| r | (1u32 << (self.config.receive_message_descriptor as u32)));
    }
}

/// Ipc Event Channel
#[derive(Clone)]
pub struct IpcEventChannel {
    rimpl: Rc<RefCell<IpcEventChannelImpl>>,
}

impl IpcEventChannel {
    /// Create a new Ipc event channel
    ///
    /// # Arguments
    ///
    /// * `id` - Ipc event channel Id
    /// * `config` - Ipc event channel configuration
    /// * `cntrl` - Ipc controller
    ///
    /// # Returns
    ///
    /// * `IpcEventChannel` - Ipc event channel
    pub(crate) fn create(
        id: IpcChannelId,
        config: IpcEventChannelConfig,
        cntrl: IpcController,
    ) -> IpcEventChannel {
        Self {
            rimpl: Rc::new(RefCell::new(IpcEventChannelImpl::create(id, config, cntrl))),
        }
    }

    /// Get the channel Id of this Ipc event channel
    ///
    /// # Returns
    ///
    /// * `IpcChannelId` - Ipc event channel Id
    pub fn id(&self) -> IpcChannelId {
        self.rimpl.borrow().id()
    }
}

impl IpcEventChannelTrait for IpcEventChannel {
    /// Begin event
    fn begin_event(&self, tag: u16, event_id: IpcDescriptor, event: u32) -> McrResult<()> {
        self.rimpl.borrow_mut().send_event(tag, event_id, event)
    }

    /// End event
    fn end_event(&self, event_id: IpcDescriptor, event: u32) -> McrResult<()> {
        self.rimpl.borrow_mut().end_event(event_id, event)
    }

    /// Retrieve the tag for pending IPC request
    fn peek_tag(&self) -> Option<u16> {
        self.rimpl.borrow().tag()
    }

    /// Receive a event of type `IpcDescriptor` using this Ipc event channel
    fn receive_event(&self, event_id: IpcDescriptor) -> Option<u32> {
        self.rimpl.borrow_mut().receive_event(event_id)
    }
}

/// Ipc event channel implementation
struct IpcEventChannelImpl {
    /// Ipc event channel Id
    id: IpcChannelId,

    /// Ipc event channel configuration
    config: IpcEventChannelConfig,

    /// Ipc Controller
    _cntrl: IpcController,

    /// Current event tag
    tag: Option<u16>,
}

impl IpcEventChannelImpl {
    fn create(
        id: IpcChannelId,
        config: IpcEventChannelConfig,
        cntrl: IpcController,
    ) -> IpcEventChannelImpl {
        let int_reg_block = IpcIntRegs::block();
        let int_reg = int_reg_block.at(config.int_block.into());

        int_reg
            .int_enable_set()
            .read_and_modify(|r, _| (r | config.receive_event_mask));

        Self {
            id,
            config,
            _cntrl: cntrl,
            tag: None,
        }
    }

    fn id(&self) -> IpcChannelId {
        self.id
    }

    fn send_event(&mut self, tag: u16, event_id: IpcDescriptor, event: u32) -> McrResult<()> {
        if self.tag.is_some() {
            Err(IpcControllerErr::ChannelBusy)?
        }

        self.end_event(event_id, event)?;

        self.tag = Some(tag);

        Ok(())
    }

    fn end_event(&mut self, event_id: IpcDescriptor, event: u32) -> McrResult<()> {
        let event_mask: u32 = 1u32 << (event_id as u32);

        let masked_event = event_mask & self.config.send_event_mask;
        if masked_event != event_mask {
            // Descriptor not configured for send event
            Err(IpcControllerErr::EventNotConfigured)?
        }

        let desc_reg_block = IpcDescriptorRegs::block();
        let desc_reg = desc_reg_block.at(event_id as usize);
        desc_reg.ipc_desc_reg_n().write(|_| event);

        Ok(())
    }

    fn tag(&self) -> Option<u16> {
        self.tag
    }

    fn receive_event(&mut self, event_id: IpcDescriptor) -> Option<u32> {
        let input_event_mask = 1u32 << (event_id as u32);
        let masked_event = input_event_mask & self.config.receive_event_mask;
        if masked_event != input_event_mask {
            return None;
        }

        let desc_reg_block = IpcDescriptorRegs::block();
        let desc_reg = desc_reg_block.at(event_id as usize);

        // Release the tag
        self.tag.take();

        Some(desc_reg.ipc_desc_reg_n().read())
    }
}

impl Drop for IpcEventChannelImpl {
    fn drop(&mut self) {
        let int_reg_block = IpcIntRegs::block();
        let int_reg = int_reg_block.at(self.config.int_block.into());

        int_reg
            .int_enable_clr()
            .read_and_modify(|r, _| (r | self.config.receive_event_mask));
    }
}
