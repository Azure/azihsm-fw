// Copyright (c) Microsoft Corporation. All rights reserved.

#![warn(missing_docs)]

extern crate alloc;

use alloc::rc::Rc;
use core::cell::RefCell;

use bitfield::BitMut;
use mcr_error::McrResult;
use mcr_registers::gdma::RegisterBlock as GdmaRegs;

#[cfg(feature = "mcr_test_hooks")]
use mcr_ddi_types::DdiTestActionGDMAErrorType;

use crate::*;

/// GDMA Controller
#[derive(Clone)]
pub struct GdmaController {
    rimpl: Rc<RefCell<GdmaControllerImpl>>,
}

impl GdmaController {
    /// Create an instance of Gdma Controller
    ///
    /// # Arguments
    ///
    /// # Returns
    ///
    /// * `GdmaController` - Gdma Controller instance
    pub fn new() -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(GdmaControllerImpl::new())),
        }
    }

    /// Create an instance of Gdma controller with enable
    ///
    /// # Returns
    ///
    /// * `McrResult<GdmaController>` - Ok() with GdmaController or an appropriate error code.
    pub fn new_with_enable() -> McrResult<Self> {
        let controller_rimpl = GdmaControllerImpl::new_with_enable()?;

        Ok(Self {
            rimpl: Rc::new(RefCell::new(controller_rimpl)),
        })
    }

    /// Create a Gdma Channel using this Gdma Controller
    ///
    /// # Arguments
    ///
    /// * `id` - Gdma Channel Id to be created of type GdmaChannelId
    /// * `config` - Gdma Channel configuration data to be used for creation of GdmaChannel
    ///
    /// # Returns
    ///
    /// * `McrResult<GdmaChannel>` - Ok with Gdma Channel instance or an Err
    pub fn create_channel(
        &self,
        id: GdmaChannelId,
        config: GdmaChannelConfig,
    ) -> McrResult<GdmaChannel> {
        self.rimpl
            .borrow_mut()
            .create_channel(self.clone(), id, config)
    }

    /// Reset GDMA hardware
    ///
    /// # Arguments
    ///
    /// # Returns
    ///
    /// Reset the GDMA hardware and disable all GDMA interrupts
    pub fn reset_gdma() {
        let reg = GdmaRegs::block();
        reg.control_status().write(|w| {
            w.gdma_rst(true)
                .axi_rd_en(true)
                .axi_wr_en(true)
                .gdma_en(false)
        });

        // Disable all GDMA interrupts
        reg.interrupt_enable_0().write(|_| 0);
        reg.interrupt_enable_1().write(|_| 0);
    }

    /// Read raw interrupt cause bits as a `u32`.
    ///
    /// # Arguments
    ///
    /// # Returns
    ///
    /// * `u32` - Raw GDMA interrupt cause bits
    pub fn read_gdma_interrupt_cause() -> u32 {
        let reg = GdmaRegs::block();
        u32::from(reg.interrupt_cause().read())
    }

    /// Clear the data structure error cause.
    ///
    /// # Arguments
    ///
    /// # Returns
    pub fn clear_data_structure_error_cause() {
        let reg = GdmaRegs::block();

        // Clear data structure error cause reg
        const DATA_STR_CAUSE_CLR_MASK: u32 = 0x701ff;
        reg.data_structure_error_cause()
            .write(|_| DATA_STR_CAUSE_CLR_MASK.into());
    }

    /// Enable GDMA error interrupts
    ///
    /// # Arguments
    ///
    /// # Returns
    pub fn enable_gdma_err_int() {
        let reg = GdmaRegs::block();
        let mut data_access_err = 0u32;
        data_access_err.set_bit(1, true);
        data_access_err.set_bit(2, true);

        // RESET 1:2 bit first and Setting it. Bit 2, 4:5 getting SET in cntrl.rs GDMA init sequence
        reg.data_access_error_enable()
            .read_and_modify(|r, _| r | (data_access_err));

        // Enable the following GDMA error interrupts per GDMA_INIT sequence:
        // bit23:data_strctr_err, bit22:data_access_err; bit16:dq_err, bit17:cq_err
        let mut int_en_1 = 0u32;
        int_en_1.set_bit(16, true);
        int_en_1.set_bit(17, true);
        int_en_1.set_bit(22, true);
        int_en_1.set_bit(23, true);
        reg.interrupt_enable_1()
            .read_and_modify(|r, _| r | (int_en_1));
    }

    /// Gdma Error Injection for enables on mcr_test_hooks feature
    ///
    /// # Arguments
    /// * `error_type` - Type of GDMA error to be injected
    ///
    /// # Returns
    #[cfg(feature = "mcr_test_hooks")]
    pub fn inject_gdma_error(error_type: DdiTestActionGDMAErrorType) {
        let reg = GdmaRegs::block();

        match error_type {
            DdiTestActionGDMAErrorType::GdmaDataAccessErrorBit => {
                // Set Error injection bits for data access error
                reg.error_injection().write(|w| w.frc_dp_axis_err_w(true));
            }
            DdiTestActionGDMAErrorType::GdmaDataStructureErrorBit => {
                // Enable data structure error detection
                let mut ds_err_en = 0u32;
                ds_err_en.set_bit(8, true);
                reg.data_structure_error_enable()
                    .read_and_modify(|r, _| r | ds_err_en);

                // Set the GDMA_DATA_STR_ERR_CONDITION flag to true
                crate::GDMA_DATA_STR_ERR_CONDITION
                    .store(true, core::sync::atomic::Ordering::Relaxed);
            }
            DdiTestActionGDMAErrorType::GdmaCompletionQueueErrorBit => {
                // Enable completion queue write error detection
                reg.data_path_error_control()
                    .read_and_modify(|_, w| w.cq_axis_err_w_en(true));

                // Force data path parity error at write interface
                reg.error_injection().write(|w| w.frc_dp_perr_w(true));
            }
            DdiTestActionGDMAErrorType::GdmaDeliveryQueueErrorBit => {
                // Enable delivery queue error set dq_axis_err_r_en
                reg.data_path_error_control()
                    .read_and_modify(|_, w| w.dq_axis_err_r_en(true));

                // Inject AXI read error for delivery queue operations
                reg.error_injection().write(|w| w.frc_dp_axis_err_r(true));
            }
            // Catch-all: the DDI enum is open/represents u32; ignore unknown values
            _ => {
                // Unknown/unsupported error type - no-op
            }
        }
    }
}

impl Default for GdmaController {
    /// Returns the "default value" for a type.
    fn default() -> Self {
        Self::new()
    }
}

/// GDMA Controller Implementation
pub(crate) struct GdmaControllerImpl {
    regs: GdmaRegs,
}

impl GdmaControllerImpl {
    /// Create an instance of Gdma Controller
    fn new() -> Self {
        Self {
            regs: GdmaRegs::block(),
        }
    }

    /// Create an instance of Gdma controller with enable
    fn new_with_enable() -> McrResult<Self> {
        let controller_impl = Self::new();

        controller_impl.enable()?;

        Ok(controller_impl)
    }

    /// Enable the controller if not previously enabled
    fn enable(&self) -> McrResult<()> {
        if self.regs.control_status().read().gdma_en() {
            Err(GdmaControllerErr::AlreadyEnabled)?;
        }

        // Start the controller reset
        self.regs.control_status().write(|w| {
            w.gdma_rst(true)
                .axi_rd_en(true)
                .axi_wr_en(true)
                .gdma_en(false)
        });

        // Clear interrupt enable
        self.regs.interrupt_enable_0().write(|_| 0);
        self.regs.interrupt_enable_1().write(|_| 0);

        // TODO: Fix Register XML
        // Enable data_access_err [22:22] and 	data_strctr_err [23:23] error interrupts
        let mut int_en = 0u32;
        int_en.set_bit(22, true);
        int_en.set_bit(23, true);

        // Enable Gdma error reporting
        self.regs
            .interrupt_enable_1()
            .read_and_modify(|r, _| r | int_en);

        // Release Gdma controller from reset
        self.regs
            .control_status()
            .read_and_modify(|_, w| w.gdma_rst(false));

        // Enable data path error checking
        self.regs.data_path_error_control().read_and_modify(|_, w| {
            w.axi_reg_par_err_check_en(true)
                .dq_parity_err_en(true)
                .mem_par_err_check_en(true)
        });

        // TODO: Fix Register XML
        // Enable dpe_parity_err [0:0], src_mem_perr [4:4] & dst_mem_perr [5:5]
        let mut err_en = 0u32;
        err_en.set_bit(0, true);
        err_en.set_bit(4, true);
        err_en.set_bit(5, true);

        // Enable Parity error reporting
        self.regs
            .data_access_error_enable()
            .read_and_modify(|r, _| r | err_en);

        // Enable Gdma Controller
        self.regs
            .control_status()
            .read_and_modify(|_, w| w.gdma_en(true));

        Ok(())
    }

    /// Create a Gdma Channel using this Gdma Controller
    fn create_channel(
        &mut self,
        cntrl: GdmaController,
        id: GdmaChannelId,
        config: GdmaChannelConfig,
    ) -> McrResult<GdmaChannel> {
        GdmaChannel::create(cntrl, id, config)
    }
}

impl Drop for GdmaControllerImpl {
    fn drop(&mut self) {
        self.regs
            .control_status()
            .read_and_modify(|_, w| w.gdma_en(false));
    }
}
