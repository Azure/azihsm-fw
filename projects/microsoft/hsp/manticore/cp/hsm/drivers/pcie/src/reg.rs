// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_registers::comphy0_soc;

use crate::PciePhyId;

pub(crate) trait PcieCntrlReg {
    fn phy_reg(phy_id: PciePhyId) -> Self;
}

impl PcieCntrlReg for comphy0_soc::RegisterBlock {
    fn phy_reg(phy_id: PciePhyId) -> Self {
        unsafe { Self::new((0xb0100000 + phy_id as u32) as *mut u32) }
    }
}
