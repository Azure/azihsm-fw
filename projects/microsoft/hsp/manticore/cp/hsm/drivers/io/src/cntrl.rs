// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::rc::Rc;
use core::cell::RefCell;

use mcr_error::McrResult;
use mcr_registers::ucd::inbound::core_ib_cmn::RegisterBlock as InboundCommonReg;
use mcr_registers::ucd::outbound::core_ob_cmn::RegisterBlock as OutboundCommonReg;
use mcr_types::IoControllerId;

use crate::reg::IoCntrlReg;
use crate::*;

/// Queue Entry Length
enum QueueEntryLen {
    /// 16 bytes
    Len16 = 1,

    /// 64 bytes
    Len64 = 4,
}

impl From<QueueEntryLen> for u32 {
    /// Converts to this type from the input type.
    fn from(value: QueueEntryLen) -> Self {
        value as Self
    }
}

/// Io Controller
#[derive(Clone)]
pub struct IoController {
    rimpl: Rc<RefCell<IoControllerImpl>>,
    ctrl_id: IoControllerId,
}

impl IoController {
    /// Create an instance of Io Controller
    ///
    /// # Arguments
    ///
    /// * `ctrl_id` - Io Controller ID of type IoControllerId
    ///
    /// # Returns
    ///
    /// * `IoController` - An instance of Io Controller object
    pub fn new(ctrl_id: IoControllerId) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(IoControllerImpl::new(ctrl_id))),
            ctrl_id,
        }
    }

    /// Create an instance of Io Controller with enable
    ///
    /// # Arguments
    ///
    /// * `ctrl_id` - Io Controller ID of type IoControllerId
    ///
    /// # Returns
    ///
    /// * `McrResult<IoController>` - Ok() with IoController object or an appropriate error code
    pub fn new_with_enable(ctrl_id: IoControllerId) -> McrResult<Self> {
        let controller_impl = IoControllerImpl::new_with_enable(ctrl_id)?;

        Ok(Self {
            rimpl: Rc::new(RefCell::new(controller_impl)),
            ctrl_id,
        })
    }

    /// Return the Io Controller ID of this instance
    ///
    /// # Returns
    ///
    /// * `IoControllerId` - Io Controller ID
    pub fn id(&self) -> IoControllerId {
        self.ctrl_id
    }

    /// Create an instance of Io Channel
    ///
    /// # Arguments
    ///
    /// * `config` - IoChannel configuration of type IoChannelConfig
    ///
    /// # Returns
    ///
    /// * `McrResult<IoChannel>` - Ok() with IoChannel object or an appropriate error code
    pub fn create_channel(&self, config: IoChannelConfig) -> McrResult<IoChannel> {
        self.rimpl.borrow_mut().create_channel(self.clone(), config)
    }

    /// Create an instance of Io proxy channel
    ///
    /// # Arguments
    ///
    /// * `config` - IoChannel configuration of type IoChannelConfig
    ///
    /// # Returns
    ///
    /// * `McrResult<IoProxyChannel>` - Ok() with IoProxyChannel object or an appropriate error code
    pub fn create_proxy_channel(&self, config: IoChannelConfig) -> McrResult<IoProxyChannel> {
        self.rimpl
            .borrow_mut()
            .create_proxy_channel(self.clone(), config)
    }

    /// Open an instance of Io Channel
    ///
    /// # Arguments
    ///
    /// * `config` - IoChannel configuration of type IoChannelConfig
    ///
    /// # Returns
    ///
    /// * `McrResult<IoChannel>` - Ok() with IoChannel object or an appropriate error code
    pub fn open_channel(&self, config: IoChannelConfig) -> McrResult<IoChannel> {
        self.rimpl.borrow_mut().open_channel(self.clone(), config)
    }

    /// Open an instance of Io proxy channel
    ///
    /// # Arguments
    ///
    /// * `channel_id` - IoProxyChannel Id to be opened for this controller
    ///
    /// # Returns
    ///
    /// * `McrResult<IoProxyChannel>` - Ok() with IoProxyChannel object or an appropriate error code
    pub fn open_proxy_channel(&self, channel_id: IoChannelId) -> McrResult<IoProxyChannel> {
        self.rimpl
            .borrow_mut()
            .open_proxy_channel(self.clone(), channel_id)
    }
}

impl IoControllerTrait for IoController {
    /// Pause the inbound engine for this IO Controller
    #[allow(dead_code)]
    fn pause_inbound(&self) {
        self.rimpl
            .borrow()
            .ib_cmn_reg
            .control()
            .read_and_modify(|_, w| w.ib_ucd_pause(true));
    }

    /// Resume the inbound engine for this IO Controller
    fn resume_inbound(&self) {
        self.rimpl
            .borrow()
            .ib_cmn_reg
            .control()
            .read_and_modify(|_, w| w.ib_ucd_pause(false))
    }
}

/// Io Controller Implementation
struct IoControllerImpl {
    /// Inbound Register block for Io Controller
    ib_cmn_reg: InboundCommonReg,

    /// Outbound Register block for Io Controller
    ob_cmn_reg: OutboundCommonReg,
}

impl IoControllerImpl {
    /// Create an instance of Io Controller implementation
    fn new(ctrl_id: IoControllerId) -> Self {
        Self {
            ib_cmn_reg: InboundCommonReg::cntrl_reg(ctrl_id),
            ob_cmn_reg: OutboundCommonReg::cntrl_reg(ctrl_id),
        }
    }

    /// Create an instance of Io Controller implementation with enable
    fn new_with_enable(ctrl_id: IoControllerId) -> McrResult<Self> {
        let mut controller_impl = Self::new(ctrl_id);

        controller_impl.reset_inbound_outbound();
        controller_impl.enable(ctrl_id)?;

        Ok(controller_impl)
    }

    /// Create an instance of Io Channel
    fn create_channel(
        &mut self,
        cntrl: IoController,
        config: IoChannelConfig,
    ) -> McrResult<IoChannel> {
        IoChannel::new_with_enable(cntrl, config)
    }

    /// Create an instance of Io Channel
    fn create_proxy_channel(
        &mut self,
        cntrl: IoController,
        config: IoChannelConfig,
    ) -> McrResult<IoProxyChannel> {
        IoProxyChannel::new(cntrl, config)
    }

    // Open an instance of Io Channel
    fn open_channel(
        &mut self,
        cntrl: IoController,
        config: IoChannelConfig,
    ) -> McrResult<IoChannel> {
        IoChannel::open(cntrl, config)
    }

    /// Open an instance of Io Channel
    fn open_proxy_channel(
        &mut self,
        cntrl: IoController,
        channel_id: IoChannelId,
    ) -> McrResult<IoProxyChannel> {
        IoProxyChannel::open(cntrl, channel_id)
    }

    /// Enable the Io Controller
    fn enable(&mut self, ctrl_id: IoControllerId) -> McrResult<()> {
        if self.ib_cmn_reg.control().read().ib_ucd_enbl()
            || self.ob_cmn_reg.control().read().ob_ucd_enbl()
        {
            Err(IoControllerErr::IoControllerAlreadyEnabled)?;
        }

        // Enable SRAM error interrupts on inbound queue
        self.ib_cmn_reg
            .sram_parity_error_enable()
            .write(|w| w.ib_ucd_sram_perr_en(true));

        // Enable SRAM error interrupts on outbound queue
        self.ob_cmn_reg
            .sram_parity_error_enable()
            .write(|w| w.ob_ucd_sram_perr_en(true));

        let entry_len = match ctrl_id {
            // For Io Controller Core0, CQE entry length is 16 bytes
            IoControllerId::Core0 => QueueEntryLen::Len16,
            // For Io Controller Core1, CQE entry length is 64 bytes
            IoControllerId::Core1 => QueueEntryLen::Len64,
            _ => return Err(IoControllerErr::InvalidControllerId)?,
        };

        // Populate the size select 0;
        self.ob_cmn_reg
            .size_select_0()
            .write(|w| w.ob_queue_elmnt_lngth_0(entry_len.into()));

        // Enable inbound and outbound hardware queues
        self.ib_cmn_reg.control().write(|w| w.ib_ucd_enbl(true));
        self.ob_cmn_reg.control().write(|w| w.ob_ucd_enbl(true));

        // Disable AXI error propagation to UCD cores to avoid AXI errors during FLR handling.
        // The correct way to handle this error is to handle the error completion status in IBCQ
        // entry. FP task: 23241
        self.ib_cmn_reg
            .miscellaneous_control()
            .read_and_modify(|_, w| w.dsbl_axi_err_propagation(true));

        Ok(())
    }

    /// Reset the Inbound and Outbound queues of this controller
    fn reset_inbound_outbound(&self) {
        self.ib_cmn_reg
            .configuration()
            .read_and_modify(|_, w| w.ib_ucd_rst(true));
        self.ob_cmn_reg
            .configuration()
            .read_and_modify(|_, w| w.ob_ucd_rst(true));
    }
}

impl Drop for IoControllerImpl {
    fn drop(&mut self) {
        // Disable UCD inbound queue operations
        self.ib_cmn_reg
            .control()
            .read_and_modify(|_, w| w.ib_ucd_enbl(false));

        // Disable UCD outbound queue operations
        self.ob_cmn_reg
            .control()
            .read_and_modify(|_, w| w.ob_ucd_enbl(false));
    }
}
