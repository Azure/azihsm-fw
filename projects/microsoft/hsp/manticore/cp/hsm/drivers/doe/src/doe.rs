// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::rc::Rc;
use core::cell::RefCell;

use mcr_error::McrResult;
use mcr_registers::pcie_doe::RegisterBlock as DoeRegisters;
use mcr_types::MemoryAddr;
use zeroize::Zeroize;

use crate::*;

/// PCIe DOE
#[derive(Clone)]
pub struct PcieDoe {
    rimpl: Rc<RefCell<PcieDoeImpl>>,
}

impl PcieDoe {
    /// Create an instance of PCIe DOE interface
    ///
    /// # Arguments
    ///
    /// * `msg_buf` - DOE message buffer
    ///
    /// # Returns
    ///
    /// * `PcieDoe` - PCIe DOE instance
    pub fn new(msg_buf: &'static mut [u32]) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(PcieDoeImpl::new(msg_buf))),
        }
    }

    /// Enable and reset DOE HW
    pub fn enable_hw() {
        PcieDoeImpl::enable()
    }

    /// Get the Doe event that is currently pending
    pub fn event() -> DoeEvents {
        let reg = DoeRegisters::block();
        let status = reg.doe_interrupt_status().read();

        if status.doe_abort_int() {
            reg.doe_interrupt_status()
                .write(|_| DoeIntStatusRegMask::DoeAbort.into());
            DoeEvents::DoeAbort
        } else if status.poison_cfgwr_int() {
            reg.doe_interrupt_status()
                .write(|_| DoeIntStatusRegMask::PoisonCfgWr.into());
            DoeEvents::PoisonedConfigWrite
        } else if status.tx_overflow_int() {
            reg.doe_interrupt_status()
                .write(|_| DoeIntStatusRegMask::TxOverflow.into());
            DoeEvents::TxOverflow
        } else if status.rx_underflow_int() {
            reg.doe_interrupt_status()
                .write(|_| DoeIntStatusRegMask::RxUnderflow.into());
            DoeEvents::RxUnderflow
        } else if status.tx_done_int() {
            reg.doe_interrupt_status()
                .write(|_| DoeIntStatusRegMask::TxDone.into());
            DoeEvents::TxDone
        } else if status.tx_rdy_int() {
            reg.doe_interrupt_status()
                .write(|_| DoeIntStatusRegMask::TxReady.into());
            DoeEvents::TxReady
        } else if status.doe_go_int() {
            reg.doe_interrupt_status()
                .write(|_| DoeIntStatusRegMask::DoeGo.into());
            DoeEvents::DoeGo
        } else if status.rx_rdy_int() {
            reg.doe_interrupt_status()
                .write(|_| DoeIntStatusRegMask::RxReady.into());
            DoeEvents::RxReady
        } else {
            reg.doe_interrupt_status()
                .write(|_| DoeIntStatusRegMask::All.into());
            DoeEvents::Invalid
        }
    }
}

impl PcieDoeTrait for PcieDoe {
    /// Receive a DOE message
    fn recv(&self) -> McrResult<()> {
        self.rimpl.borrow_mut().recv()
    }

    /// End DOE message receive
    fn end_recv(&self) -> McrResult<()> {
        self.rimpl.borrow_mut().end_recv()
    }

    /// Send a DOE message
    fn send(&self) -> McrResult<()> {
        self.rimpl.borrow_mut().send()
    }

    /// End DOE message send
    fn end_send(&self) {
        self.rimpl.borrow_mut().end_send()
    }

    /// Abort DOE message transfer
    fn abort(&self) {
        self.rimpl.borrow_mut().abort()
    }

    /// Reset and re-enable DOE
    fn reset(&self) {
        self.rimpl.borrow_mut().reset()
    }

    /// Set DOE status busy bit
    ///
    /// # Arguments
    ///
    /// * `val` - Value to set busy bit to
    fn set_busy(&self, val: bool) {
        self.rimpl.borrow_mut().set_busy(val)
    }

    /// Set error status
    fn set_err(&self) {
        self.rimpl.borrow_mut().set_err()
    }

    /// Get the DOE message buffer address
    fn buffer_addr(&self) -> MemoryAddr {
        self.rimpl.borrow().buffer_addr()
    }
}

/// DOE object
pub struct PcieDoeImpl {
    /// Register block
    reg: DoeRegisters,

    /// DOE message buffer
    msg_buf: &'static mut [u32],

    /// DOE message length in DWORDs
    len: usize,

    /// DOE message current offset
    offset: usize,
}

impl PcieDoeImpl {
    /// Initialization of DOE
    fn new(msg_buf: &'static mut [u32]) -> Self {
        Self {
            reg: DoeRegisters::block(),
            msg_buf,
            len: Default::default(),
            offset: Default::default(),
        }
    }

    /// Reset and enable DOE HW
    fn enable() {
        let reg = DoeRegisters::block();

        // Reset the DOE
        reg.doe_configuration_3()
            .read_and_modify(|_, w| w.doe_fw_rst_n(true));
        reg.doe_configuration_3()
            .read_and_modify(|_, w| w.doe_fw_rst_n(false));
        reg.doe_configuration_3()
            .read_and_modify(|_, w| w.doe_fw_rst_n(true));

        reg.doe_capability()
            .write(|w| w.msi_support(false).msi_num(0x0u32));

        // Configure Ready, Busy, and Error bits behavior
        reg.doe_configuration_1().write(|w| {
            w.go_busy_en(true)
                .tx_done_ready_en(true)
                .ecc_err_en(false)
                .rx_rdy_busy_en(true)
                .abort_err_en(true)
                .abort_ready_en(true)
                .abort_busy_en(true)
                .elbi_to_err_en(true)
                .tx_overflow_int_en(true)
                .rx_underflow_int_en(true)
        });

        // TODO: Enable the ECC and handle interrupt and get the procedure from Marvell
        reg.doe_configuration_2()
            .write(|w| w.rx_ecc_en(false).tx_ecc_en(false).rx_ecc_en(false));

        // Clear Interrupt status
        reg.doe_interrupt_status().write(|w| {
            w.elbi_to_int(true)
                .rx_rdy_int(true)
                .tx_rdy_int(true)
                .doe_go_int(true)
                .doe_abort_int(true)
                .ecc_err_rxfifo(true)
                .ecc_err_txfifo(true)
                .tx_done_int(true)
                .tx_overflow_int(true)
                .rx_underflow_int(true)
                .poison_cfgwr_int(true)
        });

        // Interrupt Enable
        reg.doe_interrupt_en().write(|w| {
            w.rx_rdy_int_en(true)
                .tx_rdy_int_en(true)
                .doe_go_int_en(true)
                .tx_done_int_en(true)
                .doe_abort_int_en(true)
                .txfifo_overflow_int_en(true)
                .rxfifo_underflow_int_en(true)
                .poison_cfgwr_int_en(true)
        });

        // Clear Busy bit to allow for incoming DOE messages
        Self::set_busy_int(reg, false);
    }

    /// Receive a DOE message
    fn recv(&mut self) -> McrResult<()> {
        // Set busy bit to block new incoming requests
        self.set_busy(true);

        // Get the current DWORD count in the Rx FIFO
        let rx_cnt = self.reg.doe_configuration_2().read().rx_fifo_cnt() as usize;

        // Read upto rx_fifo_cnt from Rx FIFO into the Rx buffer
        for _ in 0..rx_cnt {
            self.msg_buf[self.offset] = self.reg.rx_fifo_rd_data().read();
            self.offset += 1;
        }

        // Second DWORD contains the length of the entire DOE message
        if self.len == 0 {
            self.len = self.msg_buf[1] as usize & NUM_DWORDS_IN_1MB;
        }

        // Check if the Rx FIFO buffer can hold the incoming message
        if self.len > self.msg_buf.len() {
            self.set_err();
            Err(DoeErr::InvalidLength)?
        }

        // Clear busy bit to allow new incoming requests
        self.set_busy(false);

        Ok(())
    }

    /// End DOE message receive
    fn end_recv(&mut self) -> McrResult<()> {
        // Verify that the amount of data received matches the expected length
        if self.offset != self.len {
            self.set_err();
            Err(DoeErr::InvalidLength)?
        }

        self.len = 0;
        self.offset = 0;
        self.set_busy(true);

        Ok(())
    }

    /// Send a DOE message
    fn send(&mut self) -> McrResult<()> {
        // Second DWORD contains the length of the entire DOE message
        if self.len == 0 {
            self.len = self.msg_buf[1] as usize & NUM_DWORDS_IN_1MB;
        }

        // Verify that the amount of DWORDs to be transmitted does not exceed the Tx buffer size
        if self.len > self.msg_buf.len() {
            self.set_err();
            Err(DoeErr::InvalidLength)?
        }

        // Loop until the current offset is equal to number of DWORDs to be transmitted
        while self.offset < self.len {
            self.reg
                .tx_fifo_wr_data()
                .write(|_| self.msg_buf[self.offset]);
            self.offset += 1;

            // If a 2KB block is transferred, then break out of the loop to allow the host to
            // read the first 2KB block
            if self.offset % NUM_DWORDS_IN_2KB == 0 {
                break;
            }
        }

        // If the current offset is equal to the number of DWORDs to be transmitted, then set the
        // last_dw bit in the DOE Configuration 1 register
        if self.offset == self.len {
            self.reg
                .doe_configuration_1()
                .read_and_modify(|_, w| w.last_dw(true));
        }

        // Set the DOE Tx Ready bit in the DOE Status register for the host to read the Tx FIFO
        Self::set_ready(self.reg, true);

        Ok(())
    }

    /// End DOE message send
    fn end_send(&mut self) {
        self.msg_buf.zeroize();
        self.len = 0;
        self.offset = 0;
        self.set_busy(false);
    }

    /// Abort DOE message transfer
    fn abort(&mut self) {
        // When DOE Abort bit is set, DOE Ready bit & DOE Error bit are cleared and DOE Busy bit is
        // set per DOE_Configuration_1 register configuration.

        self.msg_buf.zeroize();
        self.len = 0;
        self.offset = 0;
    }

    /// Reset and re-enable DOE
    fn reset(&mut self) {
        Self::enable();
    }

    /// Set error status.  Note that error status cannot be cleared except by abort.
    ///
    /// # Arguments
    ///
    /// * `reg` - DOE registers
    fn set_err(&mut self) {
        self.reg.doe_status().write(|w| {
            // Avoid triggering W1C on DOE status interrupt bit
            let status: u32 = w.doe_status_err(true).into();
            let int_status = !DOE_STATUS_INT_MASK;
            (status & int_status).into()
        });
    }

    /// Set DOE status busy bit
    ///
    /// # Arguments
    ///
    /// * `val` - Value to set busy bit to
    fn set_busy(&self, val: bool) {
        Self::set_busy_int(self.reg, val)
    }

    /// Set DOE status busy bit
    ///
    /// # Arguments
    ///
    /// * `reg` - DOE registers
    /// * `val` - Value to set busy bit to
    fn set_busy_int(reg: DoeRegisters, val: bool) {
        reg.doe_status().write(|w| {
            // Avoid triggering W1C on DOE status interrupt bit
            let status: u32 = w.doe_status_busy(val).into();
            let int_status = !DOE_STATUS_INT_MASK;
            (status & int_status).into()
        });
    }

    /// Set DOE status ready bit
    ///
    /// # Arguments
    ///
    /// * `reg` - DOE registers
    /// * `val` - Value to set ready bit to
    fn set_ready(reg: DoeRegisters, val: bool) {
        reg.doe_status().write(|w| {
            // Avoid triggering W1C on DOE status interrupt bit
            let status: u32 = w.doe_status_ready(val).into();
            let int_status = !DOE_STATUS_INT_MASK;
            (status & int_status).into()
        });
    }

    /// Get the DOE message buffer address
    fn buffer_addr(&self) -> MemoryAddr {
        MemoryAddr {
            lo: self.msg_buf.as_ptr() as u32,
            ..Default::default()
        }
    }
}
