// Copyright (c) Microsoft Corporation. All rights reserved.

#![warn(missing_docs)]

extern crate alloc;

use alloc::rc::Rc;
use core::cell::RefCell;
use mcr_interrupt_controller::Interrupt;

use crate::*;
use mcr_interrupt_controller::InterruptController;
use mcr_interrupt_controller::InterruptControllerTrait;
use mcr_registers::ipc_intc::ipc_intc_int_registers::RegisterBlock as IpcIntRegs;

/// IPC Controller
#[derive(Clone)]
pub struct IpcController {
    rimpl: Rc<RefCell<IpcControllerImpl>>,
}

impl IpcController {
    /// Create new instance of IPC controller
    pub fn new(ipc_int_id: IpcIntBlock, interrupt: Interrupt) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(IpcControllerImpl::new(ipc_int_id, interrupt))),
        }
    }

    /// Create an instance of IPC message channel
    ///
    /// # Arguments
    ///
    /// * `id` - Ipc message channel Id to be created
    /// * `config` - Ipc message channel configuration `IpcMessageChannelConfig`
    ///
    /// # Returns
    ///
    /// * `IpcMessageChannel` - An instance of Ipc message channel object
    pub fn create_message_channel(
        &self,
        id: IpcChannelId,
        config: IpcMessageChannelConfig,
    ) -> IpcMessageChannel {
        self.rimpl
            .borrow_mut()
            .create_message_channel(id, config, self.clone())
    }

    /// Create an instance of IPC event channel
    ///
    /// # Arguments
    ///
    /// * `id` - Ipc event channel Id to be created
    /// * `config` - Ipc event channel configuration `IpcEventChannelConfig`
    ///
    /// # Returns
    ///
    /// * `IpcEventChannel` - An instance of Ipc event channel object
    pub fn create_event_channel(
        &self,
        id: IpcChannelId,
        config: IpcEventChannelConfig,
    ) -> IpcEventChannel {
        self.rimpl
            .borrow_mut()
            .create_event_channel(id, config, self.clone())
    }

    /// Get the descriptor ID with pending event
    ///
    /// # Arguments
    ///
    /// * `ipc_int_id` - Ipc interrupt ID
    ///
    /// # Returns
    ///
    /// * `Option<IpcDescriptor>` - Ipc descriptor ID with a pending event
    pub fn descriptor(ipc_int_id: IpcIntBlock) -> Option<IpcDescriptor> {
        let reg_block = IpcIntRegs::block();
        let int_reg = reg_block.at(ipc_int_id.into());

        let event = int_reg.int_pend_set().read();

        let event_num = event.trailing_zeros();
        match IpcDescriptor::try_from(event_num as u8) {
            Ok(descriptor) => {
                let mask = 1u32 << event_num;
                int_reg.int_pend_clr().write(|_| mask);
                Some(descriptor)
            }
            Err(_) => None,
        }
    }

    /// Check if the given descriptor is pending and if pending, clear the interrupt
    ///
    /// # Arguments
    ///
    /// * `descriptor` - IPC descriptor to poll for
    ///
    /// # Returns
    ///
    /// * true if the interrupt became pending and the pending interrupt is cleared
    /// * false if the interrupt is not pending for the given descriptor
    pub fn is_pend_and_clr(&self, descriptor: IpcDescriptor) -> bool {
        self.rimpl.borrow().is_pend_and_clr(descriptor)
    }
}

/// IPC Controller Implementation
pub(crate) struct IpcControllerImpl {
    /// IPC Interrupt Block ID
    ipc_int_id: IpcIntBlock,

    /// IPC Interrupt Number
    interrupt: Interrupt,

    /// Interrupt Register Block
    reg_block: IpcIntRegs,

    /// Interrupt controller
    intc: InterruptController,
}

impl IpcControllerImpl {
    pub fn new(ipc_int_id: IpcIntBlock, interrupt: Interrupt) -> Self {
        let reg_block = IpcIntRegs::block();
        let int_reg = reg_block.at(ipc_int_id.into());

        // clear all the pending interrupts on this IntBlock
        int_reg.int_pend_clr().write(|_| 0xFFFFFFFF);

        // Clear the pending interrupt at interrupt controller for this IPC controller
        let intc = InterruptController::default();
        intc.clear(interrupt);

        Self {
            ipc_int_id,
            interrupt,
            reg_block,
            intc,
        }
    }

    fn create_message_channel(
        &mut self,
        id: IpcChannelId,
        config: IpcMessageChannelConfig,
        cntrl: IpcController,
    ) -> IpcMessageChannel {
        IpcMessageChannel::create(id, config, cntrl)
    }

    fn create_event_channel(
        &mut self,
        id: IpcChannelId,
        config: IpcEventChannelConfig,
        cntrl: IpcController,
    ) -> IpcEventChannel {
        IpcEventChannel::create(id, config, cntrl)
    }

    /// Check if the given descriptor is pending and if pending, clear the interrupt
    fn is_pend_and_clr(&self, descriptor: IpcDescriptor) -> bool {
        let int_reg = self.reg_block.at(self.ipc_int_id.into());
        let mask = 1 << descriptor as u32;

        let pending =
            self.intc.pending(self.interrupt) && ((int_reg.int_pend_set().read() & mask) != 0);

        // Clear the pending interrupt
        if pending {
            int_reg.int_pend_clr().write(|_| mask);
            self.intc.clear(self.interrupt);
        }

        pending
    }
}
