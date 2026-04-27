// Copyright (c) Microsoft Corporation. All rights reserved.

#![warn(missing_docs)]

extern crate alloc;

use alloc::rc::Rc;
use core::cell::RefCell;

use mcr_registers::msix::msix_common;
use mcr_registers::msix::msix_vf0_func_registers::RegisterBlock as MsixFunctionRegBlock;
use mcr_types::PcieFunction;

use crate::reg::MsixFunctionRegs;
use crate::MsixControllerTrait;

/// PCIe MSIX Controller
#[derive(Clone)]
pub struct MsixController {
    rimpl: Rc<RefCell<MsixControllerImpl>>,
}

impl Default for MsixController {
    fn default() -> Self {
        Self::new()
    }
}

impl MsixController {
    /// Create an instance of `MsixController`
    ///
    /// # Returns
    ///
    /// An instance of `MsixController`
    pub fn new() -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(MsixControllerImpl::new())),
        }
    }

    /// Enable MSI-x controller
    pub fn enable(&self) {
        self.rimpl.borrow().enable()
    }
}

impl MsixControllerTrait for MsixController {
    /// Enable MSIX for a given PCIe function
    fn enable_pcie_fn(&self, pfn: PcieFunction) {
        self.rimpl.borrow().enable_pcie_fn(pfn)
    }
}

/// PCIe MSIX Controller Implementation
struct MsixControllerImpl {
    /// MSIX registers
    regs: msix_common::RegisterBlock,
}

impl MsixControllerImpl {
    fn new() -> Self {
        Self {
            regs: msix_common::RegisterBlock::block(),
        }
    }

    fn enable(&self) {
        self.regs
            .state_machine_control_16()
            .read_and_modify(|_, w| w.msix_sm_en(true));
    }

    fn enable_pcie_fn(&self, pfn: PcieFunction) {
        let pfn_regs = MsixFunctionRegBlock::msix_func_regs(pfn);

        match pfn.0 {
            x if x == PcieFunction::Pf.0 => {
                self.regs
                    .state_machine_control_16()
                    .read_and_modify(|_, w| w.msix_rst(true).msix_sm_en(true).pf_msg_en(true));
                self.regs
                    .msi_x_enable_rising_edge_pf()
                    .write(|w| w.msi_x_en_rising_edge_pf(true));

                pfn_regs
                    .table_interface_select()
                    .write(|w| w.tbl_ifc_slct(4));
            }
            x if (PcieFunction::Vf0.0..=PcieFunction::Vf31.0).contains(&x) => {
                self.regs
                    .state_machine_control_16()
                    .read_and_modify(|_, w| w.msix_rst(true).msix_sm_en(true));
                self.regs
                    .state_machine_control_0()
                    .read_and_modify(|_, w| w | (1u32 << pfn.0 as u32));
                self.regs
                    .msi_x_enable_rising_edge_vf_0()
                    .read_and_modify(|_, w| w | (1u32 << pfn.0 as u32));
                pfn_regs
                    .table_interface_select()
                    .read_and_modify(|_, w| w.tbl_ifc_slct(0x20 + pfn.0 as u32));
            }
            x if (PcieFunction::Vf32.0..=PcieFunction::Vf63.0).contains(&x) => {
                self.regs
                    .state_machine_control_16()
                    .read_and_modify(|_, w| w.msix_rst(true).msix_sm_en(true));
                self.regs
                    .state_machine_control_1()
                    .read_and_modify(|_, w| w | (1u32 << (pfn.0 as u32 - 32)));
                self.regs
                    .msi_x_enable_rising_edge_vf_1()
                    .read_and_modify(|_, w| w | (1u32 << (pfn.0 as u32 - 32)));
                pfn_regs
                    .table_interface_select()
                    .read_and_modify(|_, w| w.tbl_ifc_slct(0x20 + pfn.0 as u32));
            }
            _ => unreachable!(),
        }
    }
}
