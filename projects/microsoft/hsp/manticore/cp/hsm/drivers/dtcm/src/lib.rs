// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

use cfg_if::cfg_if;
use mcr_cpu::CpuId;
use mcr_registers::dual_cp_m7::RegisterBlock as DtcmRegs;

cfg_if! {
    if #[cfg(feature = "mcr_test_hooks")] {
        use log::info;
        use mcr_ddi_types::DdiTestActionSocCpuId;
        use mcr_mem_map::mem_addr_to_volatile_ptr;
        use mcr_mem_map::AdminDtcmMemMap;
        use mcr_mem_map::HsmDtcmMemMap;
    }
}

/// DTCM Controller
pub struct DtcmController {}

impl DtcmController {
    // dtcm ecc error interrupt enable, generate an interrupt every time 1bit error happens
    pub fn int_en(cpu: CpuId) {
        let reg = DtcmRegs::block();

        match cpu {
            CpuId::Admin => {
                reg.cp0_ctl_ecc().write(|w| w.d1tcm_err_en(0x2));
            }
            CpuId::Hsm => {
                reg.cp1_ctl_ecc().write(|w| w.d1tcm_err_en(0x2));
            }
            _ => (),
        }
    }

    // dtcm ecc error injection in memory address
    #[cfg(feature = "mcr_test_hooks")]
    pub fn inject_ecc_error(cpu: DdiTestActionSocCpuId) {
        let reg = DtcmRegs::block();

        match cpu {
            DdiTestActionSocCpuId::Admin => {
                let addr = AdminDtcmMemMap::_rsvd().as_ptr() as u32;
                let ptr = mem_addr_to_volatile_ptr(addr);

                info!("Injecting DTCM ECC Error on Admin");

                reg.cp0_ctl_errinj().write(|w| w.errinj_d1tcm(0x3));

                ptr.set(0xFF);
                ptr.get();
            }
            DdiTestActionSocCpuId::Hsm => {
                let addr = HsmDtcmMemMap::_rsvd().as_ptr() as u32;
                let ptr = mem_addr_to_volatile_ptr(addr);

                info!("Injecting DTCM ECC Error on HSM");

                reg.cp1_ctl_errinj().write(|w| w.errinj_d1tcm(0x3));

                ptr.set(0xFF);
                ptr.get();
            }
            _ => (),
        }
    }
}
