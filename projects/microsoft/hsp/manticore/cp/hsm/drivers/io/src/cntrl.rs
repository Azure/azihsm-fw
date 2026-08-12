// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::rc::Rc;
use cfg_if::cfg_if;
use core::cell::RefCell;

use mcr_error::McrResult;
use mcr_registers::ucd::inbound::core_ib_cmn::RegisterBlock as InboundCommonReg;
use mcr_registers::ucd::outbound::core_ob_cmn::RegisterBlock as OutboundCommonReg;
use mcr_types::IoControllerId;

use crate::reg::IoCntrlReg;
use crate::*;

cfg_if! {
    if #[cfg(feature = "mcr_test_hooks")] {
        use mcr_ddi_types::DdiTestActionUCDErrorType;
        use mcr_registers::ucd::inbound::core_ib_cmn::ib_cmn_cq;
        use mcr_registers::ucd::inbound::core_ib_cmn::ib_cmn_dfl;
        use mcr_registers::ucd::inbound::core0_ib_iq;
        use mcr_registers::ucd::outbound::core_ob_cmn::ob_cmn_cq;
    }
}

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

    /// Inject error for error handling validation
    #[cfg(feature = "mcr_test_hooks")]
    pub fn inject_ucd_io_error(error_type: DdiTestActionUCDErrorType) {
        match error_type {
            DdiTestActionUCDErrorType::UcdIbDflOverflowError => {
                // DFL overflow cannot be triggered by software PI/CI
                // register writes. The overflow state machine is internal
                // to the UCD fetch engine and only fires during real
                // command fetch operations.
                // Instead, we leverage the DFL empty condition (interrupt
                // cause bits 17-22) which is already active on idle queues.
                // This validates the same IRQ99 interrupt delivery path.
                let ib_cmn_reg: InboundCommonReg =
                    InboundCommonReg::cntrl_reg(IoControllerId::Core0);

                // Get the DFL register block for Core0.
                let dfl_regs = ib_cmn_dfl::RegisterBlock::cntrl_reg(IoControllerId::Core0);
                let dfl3 = dfl_regs.at(3);

                let ci = dfl3.ci().read().ib_dest_free_list_ci();
                let pi = dfl3.pi().read().ib_dest_free_list_pi();

                // If the DFL is not already empty, force PI = CI to make
                if pi != ci {
                    dfl3.pi().write(|w| w.ib_dest_free_list_pi(u32::from(ci)));
                }

                // Clear any pre-existing DFL empty status (W1C) so we
                // get a clean edge when re-enabling the interrupt.
                dfl3.status().write(|w| w.ib_dest_free_list_empty(true));

                // Enable DFL empty IRQ bits (17-22) in the UCD-internal
                // interrupt 1 enable register (error events).
                const DFL_EMPTY_IRQ_ENABLE_MASK: u32 = 0x3F << 17;
                ib_cmn_reg
                    .interrupt_1_enable_set()
                    .read_and_modify(|r, _| r | DFL_EMPTY_IRQ_ENABLE_MASK);
            }
            DdiTestActionUCDErrorType::UcdIbQueueOverflowError => {
                let ib_cmn_reg: InboundCommonReg =
                    InboundCommonReg::cntrl_reg(IoControllerId::Core0);

                // Enable IQ soft error IRQ (bit 13) in the UCD-internal
                // interrupt 1 enable register (error events).
                const IQ_SOFT_ERR_IRQ_ENABLE: u32 = 1 << 13;
                ib_cmn_reg
                    .interrupt_1_enable_set()
                    .read_and_modify(|r, _| r | IQ_SOFT_ERR_IRQ_ENABLE);

                // Get the IQ register block for Core0.
                // Queue index 0 is the first host submission queue.
                let iq_regs = core0_ib_iq::RegisterBlock::cntrl_reg(IoControllerId::Core0);
                let iq0 = iq_regs.at(0);

                // Read the current Producer Index (PI).
                let pi = iq0.pi().read().iq_prdcr_indx();

                // Trigger IQ soft error by writing the current PI value
                // back to the PI register. The hardware detects this as a
                // IQ_DBELL_WRITE_SAME_VALUE_ERR, status bit 6),
                // Which feeds into the aggregated ib_q_soft_err_irq (interrupt cause bit 13).
                // Direct overflow injection via PI/CI manipulation does not
                // work on empty queues — the overflow state machine is
                // internal to the fetch engine.
                iq0.pi().write(|w| w.iq_prdcr_indx(pi));
            }
            DdiTestActionUCDErrorType::UcdObQueueFullError => {
                let ob_cmn_reg: OutboundCommonReg =
                    OutboundCommonReg::cntrl_reg(IoControllerId::Core0);

                // Enable CQ full IRQ bits (17-21) in the UCD-internal
                // interrupt 1 enable register (error events). Interrupt 1
                // output drives IRQ 106 (ucd_ob_err_irq) at the NVIC.
                const CQ_FULL_IRQ_ENABLE_MASK: u32 = 0x1F << 17;
                ob_cmn_reg
                    .interrupt_1_enable_set()
                    .read_and_modify(|r, _| r | CQ_FULL_IRQ_ENABLE_MASK);

                // Get the Completion Queue register block for Core0.
                // Only CQ[3] is enabled (configured via Channel 3).
                let cq_regs = ob_cmn_cq::RegisterBlock::cntrl_reg(IoControllerId::Core0);
                let cq3 = cq_regs.at(3);

                // Read the current Consumer Index (CI).
                let ci = cq3.ci().read().ob_cmpltn_q_ci();
                let size_enc = cq3.configuration_control().read().ob_cmpltn_q_size();
                let depth = 32u32 << size_enc;

                // Force queue-full by writing PI = (CI + depth - 1) % depth.
                // This places PI one slot behind CI within the valid index
                // range, making the queue appear full. The hardware detects
                // the transition and sets OB_CMPLTN_Q_FULL (status bit 1) +
                // fires ob_rr2_cmpltn_q_full_irq (interrupt cause bit 20).
                let new_pi = (u32::from(ci) + depth - 1) % depth;
                cq3.pi().write(|w| w.ob_cmpltn_q_pi(new_pi));
            }
            DdiTestActionUCDErrorType::UcdIbCqFullError => {
                let ib_cmn_reg: InboundCommonReg =
                    InboundCommonReg::cntrl_reg(IoControllerId::Core0);

                // Enable IB CQ full IRQ bits (8-12) in the UCD-internal
                // interrupt 1 enable register (error events). Interrupt 1
                // output drives IRQ 99 (ucd_ib_err_irq) at the NVIC.
                const IB_CQ_FULL_IRQ_ENABLE_MASK: u32 = 0x1F << 8;
                ib_cmn_reg
                    .interrupt_1_enable_set()
                    .read_and_modify(|r, _| r | IB_CQ_FULL_IRQ_ENABLE_MASK);

                // Get the IB Completion Queue register block for Core0.
                // CQ[3] is enabled (configured via Channel 3 / HSM).
                let cq_regs = ib_cmn_cq::RegisterBlock::cntrl_reg(IoControllerId::Core0);
                let cq3 = cq_regs.at(3);

                // Read the current Consumer Index (CI).
                let ci = cq3.ci().read().cmpltn_q_ci();
                let size_enc = cq3.configuration_control().read().cmpltn_q_size();
                let depth = 32u32 << size_enc;

                // Force queue-full by writing PI = (CI + depth - 1) % depth.
                // This places PI one slot behind CI, making the queue
                // appear full. The hardware sets CMPLTN_Q_FULL (status
                // bit 1) and fires the CQ full IRQ (interrupt cause
                // bits 8-12). The IB engine pauses until the full
                // condition is cleared.
                let new_pi = (u32::from(ci) + depth - 1) % depth;
                cq3.pi().write(|w| w.cmpltn_q_pi(new_pi));
            }
            DdiTestActionUCDErrorType::UcdIbDataParityError => {
                let ib_cmn_reg = InboundCommonReg::cntrl_reg(IoControllerId::Core0);

                // Enable data path error IRQ (bit 31) in the UCD-internal
                // interrupt 1 enable register. This routes IB_DATA_PATH_ERR
                // to IRQ 99 (ucd_ib_err_irq) at the NVIC.
                const IB_DATA_PATH_ERR_ENABLE: u32 = 1 << 31;
                ib_cmn_reg
                    .interrupt_1_enable_set()
                    .read_and_modify(|r, _| r | IB_DATA_PATH_ERR_ENABLE);

                // Arm one-shot parity error injection via the Data Path
                // Error Control register. IB_DP_PRTY_MASK flips all 16
                // parity bits, causing a mismatch on the next IB data-path
                // write. IB_FRC_DP_PERR_ONCE ensures it fires exactly once.
                // IB_DP_PERR_EN enables the error capture path so the
                // error address registers are populated and interrupt
                // cause bit 31 is asserted.
                ib_cmn_reg
                    .data_path_error_control()
                    .read_and_modify(|_, w| {
                        w.ib_dp_perr_en(true)
                            .ib_dp_prty_mask(0xFFFF)
                            .ib_frc_dp_perr_once(true)
                    });
            }
            _ => {
                // Unknown/unsupported error type - no-op
            }
        }
    }

    /// Read UCD inbound interrupt cause for both cores.
    /// Returns (core0_cause, core1_cause) raw u32 values.
    pub fn read_ib_error_cause() -> (u32, u32) {
        let core0: InboundCommonReg = InboundCommonReg::cntrl_reg(IoControllerId::Core0);
        let core1: InboundCommonReg = InboundCommonReg::cntrl_reg(IoControllerId::Core1);
        (
            u32::from(core0.interrupt_cause().read()),
            u32::from(core1.interrupt_cause().read()),
        )
    }

    /// Read UCD outbound interrupt cause for both cores.
    /// Returns (core0_cause, core1_cause) raw u32 values.
    pub fn read_ob_error_cause() -> (u32, u32) {
        let core0: OutboundCommonReg = OutboundCommonReg::cntrl_reg(IoControllerId::Core0);
        let core1: OutboundCommonReg = OutboundCommonReg::cntrl_reg(IoControllerId::Core1);
        (
            u32::from(core0.interrupt_cause().read()),
            u32::from(core1.interrupt_cause().read()),
        )
    }

    /// Clear all UCD inbound interrupt 1 enable bits on both cores.
    pub fn clear_ib_interrupt_1_enable() {
        let ib_core0 = InboundCommonReg::cntrl_reg(IoControllerId::Core0);
        let ib_core1 = InboundCommonReg::cntrl_reg(IoControllerId::Core1);
        let core0_int1_en = ib_core0.interrupt_1_enable_set().read();
        let core1_int1_en = ib_core1.interrupt_1_enable_set().read();
        if core0_int1_en != 0 {
            ib_core0.interrupt_1_enable_clear().write(|_| core0_int1_en);
        }
        if core1_int1_en != 0 {
            ib_core1.interrupt_1_enable_clear().write(|_| core1_int1_en);
        }
    }

    /// Clear all UCD outbound interrupt 1 enable bits on both cores.
    pub fn clear_ob_interrupt_1_enable() {
        let ob_core0 = OutboundCommonReg::cntrl_reg(IoControllerId::Core0);
        let ob_core1 = OutboundCommonReg::cntrl_reg(IoControllerId::Core1);
        let core0_int1_en = ob_core0.interrupt_1_enable_set().read();
        let core1_int1_en = ob_core1.interrupt_1_enable_set().read();
        if core0_int1_en != 0 {
            ob_core0.interrupt_1_enable_clear().write(|_| core0_int1_en);
        }
        if core1_int1_en != 0 {
            ob_core1.interrupt_1_enable_clear().write(|_| core1_int1_en);
        }
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
