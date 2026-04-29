// Copyright (c) Microsoft Corporation. All rights reserved.
#![no_std]

use cfg_if::cfg_if;
use mcr_cpu::CpuId;
use mcr_registers::dual_cp_m7::RegisterBlock as ItcmRegs;

cfg_if! {
    if #[cfg(feature = "mcr_test_hooks")] {
        use mcr_mem_map::mem_addr_to_volatile_ptr;
    }
}

/// ITCM Controller
pub struct ItcmController {}

impl ItcmController {
    pub fn int_en(cpu: CpuId) {
        let reg = ItcmRegs::block();

        match cpu {
            CpuId::Admin => {
                // clear itcm pending errors
                reg.cp0_sts_tcm_err().write(|w| w.itcm_err(0x2));
                reg.cp0_ctl_itcm_uncorrectable()
                    .write(|w| w.error_count(0x0));
                reg.cp0_ctl_ecc().write(|w| w.itcm_err_en(0x2));
            }
            CpuId::Hsm => {
                // clear itcm pending errors
                reg.cp1_sts_tcm_err().write(|w| w.itcm_err(0x2));
                reg.cp1_ctl_itcm_uncorrectable()
                    .write(|w| w.error_count(0x0));
                reg.cp1_ctl_ecc().write(|w| w.itcm_err_en(0x2));
            }
            _ => (),
        }
    }

    // ITCM is shared and controlled by CP0 registers, so only CP0 error injection is allowed
    // ITCM ecc error injection in memory address
    #[cfg(feature = "mcr_test_hooks")]
    pub fn inject_ecc_error() {
        let reg = ItcmRegs::block();

        /// ITCM error-injection address (fixed)
        const ITCM_ERR_INJ_ADDR: u32 = 0x0006FFF8;
        reg.cp0_ctl_errinj().write(|w| w.errinj_itcm(0x0));
        reg.cp0_ctl_errinj().write(|w| w.errinj_itcm(0x3));

        // Allow error-injection register update to take effect
        mcr_cpu::dmb();

        let ptr = mem_addr_to_volatile_ptr(ITCM_ERR_INJ_ADDR);

        let val = ptr.get();
        ptr.set(val);
        let _val = ptr.get();
    }
}
