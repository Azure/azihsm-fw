// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod cntrl;
mod mac_init;
mod phy_init;
mod reg;

pub use cntrl::PcieController;
use mcr_error::mcr_err_decl;
use mcr_error::McrResult;
use mcr_types::PcieFunction;

/// PCIe Controller trait
pub trait PcieControllerTrait {
    /// Process PCIe PERST up event
    ///
    /// # Retruns
    ///
    /// * `McrResult<()>` - Ok() or an appropriate module specific error code
    ///
    fn perst_up(&self) -> McrResult<()>;

    /// Process PCIe PERST down event
    fn perst_down(&self);

    /// Query PCIe link status, for speed and width
    /// # Retruns
    ///
    /// * `McrResult<PcieLinkStatus>` - PcieLinkStatus object if the link is up or an Err
    ///
    fn link_status(&self) -> McrResult<PcieLinkStatus>;

    /// Complete PCIe FLR
    ///
    /// # Arguments
    ///
    /// * `pfn` - PcieFunction
    fn complete_flr(&self, pfn: PcieFunction);
}

/// Pcie Phy Identifier
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum PciePhyId {
    /// Com Phy 0
    ComPhy0 = 0x00000,

    /// Com Phy 1
    ComPhy1 = 0x40000,
}

impl From<PciePhyId> for u32 {
    fn from(value: PciePhyId) -> Self {
        value as Self
    }
}

/// Represent pcie link status
#[derive(Default, Clone, Copy)]
pub struct PcieLinkStatus {
    /// Pcie link speed
    pub speed: u32,

    /// Pcie link width
    pub width: u32,
}

#[derive(Clone, Copy, PartialEq, Eq, Default)]
pub struct TdispIntInfo {
    /// VF mask
    pub vf_mask: u64,

    /// PF mask
    pub pf_mask: bool,

    /// Information registers
    pub info_regs: [u32; 5],
}

/// Pcie event
pub enum PcieEvent {
    /// PCIe Physical function level reset
    Flr,

    /// PCIe Virtual function level reset
    VflrActive(u64),

    /// TDISP interrupt
    TdispInterrupt(TdispIntInfo),
}

mcr_err_decl! {
    PcieController,
    PcieControllerErr {
        // Phy microcode firmware length mismatch
        InvalidPhyMcuFwLen = 0x01,

        // Main phy microcode checksum mismatch
        MainPhyMcuChecksumMismatch = 0x02,

        // Common phy microcode checksum mismatch
        CommonPhyMcuChecksumMismatch = 0x03,

        // Lane phy microcode checksum mismatch
        LanePhyMcuChecksumMismatch = 0x04,

        // PHY0 PCLK not running
        Phy0PclkNotUpWithinSpecifiedTimeout = 0x05,

        // PHY1 PCLK not running
        Phy1PclkNotUpWithinSpecifiedTimeout = 0x06,

        // PHY PCLK not running before enabling Mac
        PhyPclkNotRunning = 0x07,

        // PCIe FLR PF active status could not cleared
        PcieFlrPfActiveStatusNotCleared = 0x08,

        // PCIe FLR VF active status1 could not cleared
        PcieFlrVfActiveStatus1NotCleared = 0x9,

        // PCIe FLR VF active status2 could not cleared
        PcieFlrVfActiveStatus2NotCleared = 0xA,

        // Link Not in L0
        PcieLinkNotInL0 = 0xB,
    }
}
