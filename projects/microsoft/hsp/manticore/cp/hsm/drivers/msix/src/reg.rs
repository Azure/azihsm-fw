// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_registers::msix::msix_vf0_func_registers::RegisterBlock as MsixFunctionRegBlock;
use mcr_types::PcieFunction;

const MSIX_FUNCTION_REG_BASE: u32 = 0xA1800000;
const MSIX_FUNCTION_REG_STRIDE: u32 = 0x80;
const MSIX_PF_REG_INDEX: u32 = 64;

pub(crate) trait MsixFunctionRegs {
    /// Returns the MSIX function registers for the given PCIe function
    ///
    /// # Arguments
    ///
    /// * `pfn` - PCIe function
    fn msix_func_regs(pfn: PcieFunction) -> Self;
}

impl MsixFunctionRegs for MsixFunctionRegBlock {
    fn msix_func_regs(pfn: PcieFunction) -> Self {
        let offset = match pfn {
            PcieFunction::Pf => MSIX_PF_REG_INDEX,
            _ => pfn.into(),
        };

        unsafe {
            Self::new((MSIX_FUNCTION_REG_BASE + offset * MSIX_FUNCTION_REG_STRIDE) as *mut u32)
        }
    }
}
