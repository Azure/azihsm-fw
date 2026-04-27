// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod doe;

pub use doe::PcieDoe;
use mcr_error::mcr_err_decl;
use mcr_error::McrResult;
use mcr_registers::pcie_doe::regs::DoeInterruptStatusWriteVal;
use mcr_types::MemoryAddr;

/// Number of DWORDs in 1MB
const NUM_DWORDS_IN_1MB: usize = 0x0003FFFF;

/// Number of DWORDs in 2KB
const NUM_DWORDS_IN_2KB: usize = 512;

/// DOE Status interrupt mask
const DOE_STATUS_INT_MASK: u32 = 0x2;

/// DOE interrupt status register mask
#[repr(u32)]
enum DoeIntStatusRegMask {
    /// RxReady interrupt
    RxReady = 0x2,

    /// TxReady interrupt
    TxReady = 0x4,

    /// DOE GO interrupt
    DoeGo = 0x8,

    /// DOE Abort interrupt
    DoeAbort = 0x10,

    /// TxDone interrupt
    TxDone = 0x80,

    /// Rx FIFO underflow interrupt
    RxUnderflow = 0x100,

    /// Tx FIFO overflow interrupt
    TxOverflow = 0x200,

    /// Poisoned Configuration Write interrupt
    PoisonCfgWr = 0x400,

    /// Mask for all interrupts
    All = 0x7FF,
}

impl From<DoeIntStatusRegMask> for DoeInterruptStatusWriteVal {
    fn from(mask: DoeIntStatusRegMask) -> Self {
        (mask as u32).into()
    }
}

/// DOE Events
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum DoeEvents {
    /// DOE Tx Done
    TxDone,

    /// DOE Abort issued by host
    ///
    /// At any time, the system firmware/software is permitted to set the DOE Abort bit in the DOE
    /// Control register, and the DOE instance must Clear the Data Object Ready bit, if not already
    /// Clear, and Clear the DOE Error bit, if already Set, in the DOE Status Register, within
    /// 1 second
    DoeAbort,

    /// DOE Go issued by host
    DoeGo,

    /// DOE Tx Ready
    TxReady,

    /// DOE Rx Ready
    ///
    /// A DOE instance must complete processing a received data object and, if a data object is
    /// required in response, must generate the response and Set the Data Object Ready bit in the
    /// DOE Status register within 1 second after the DOE Go bit was Set in the DOE Control
    /// register, otherwise the DOE instance must Set the DOE Error bit in the DOE Status
    /// register within the same time limit
    RxReady,

    /// Poisoned Configuration Write
    ///
    /// An unauthorized write has been made to DOE configuration
    PoisonedConfigWrite,

    /// Tx Overflow
    ///
    /// FW has pushed more than 2KB of data to Tx FIFO
    TxOverflow,

    /// Rx Overflow
    ///
    /// FW has read more data from Rx FIFO than is available
    RxUnderflow,

    /// Invalid event
    Invalid,
}

/// PCIe DOE Trait
pub trait PcieDoeTrait {
    /// Receive a DOE message
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok if the message was received completely, else an Err
    fn recv(&self) -> McrResult<()>;

    /// End receive
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok if the message receive was ended successfully, else an Err
    fn end_recv(&self) -> McrResult<()>;

    /// Send a DOE message
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok if the message was sent successfully, else an Err
    fn send(&self) -> McrResult<()>;

    /// End send
    fn end_send(&self);

    /// Abort DOE message transfer
    fn abort(&self);

    /// Reset and re-enable DOE
    fn reset(&self);

    /// Set error status
    fn set_err(&self);

    /// Set DOE status busy bit
    ///
    /// # Arguments
    ///
    /// * `val` - Value to set busy bit to
    fn set_busy(&self, val: bool);

    /// Get the DOE message buffer address
    ///
    /// # Returns
    ///
    /// * `MemoryAddr` - Memory address of the DOE message buffer
    fn buffer_addr(&self) -> MemoryAddr;
}

mcr_err_decl! {
    Doe,
    DoeErr {
        // Generic error
        Pending = 0x1,

        // Invalid Length
        InvalidLength = 0x2,
    }
}
