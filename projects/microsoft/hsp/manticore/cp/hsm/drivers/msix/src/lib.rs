// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod cntrl;
mod reg;

pub use cntrl::MsixController;
use mcr_types::PcieFunction;

pub trait MsixControllerTrait {
    /// Enable MSIX for a given PCIe function
    ///
    /// # Arguments
    ///
    /// * `pfn` - PCIe function
    fn enable_pcie_fn(&self, pfn: PcieFunction);
}
