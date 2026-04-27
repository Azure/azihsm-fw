// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use mcr_cpu::*;
use mcr_doe::PcieDoe;
use mcr_error::McrResult;
use mcr_registers::comphy0_soc::RegisterBlock as PhyRegisterBlock;
use mcr_registers::pcie_assist::RegisterBlock as Assist;
use mcr_registers::pcie_ep::dwc_pcie_usp::pf0_atu_cap::RegisterBlock as AtuCap;
use mcr_registers::pcie_ep::dwc_pcie_usp::pf0_l1sub_cap::RegisterBlock as L1SubCap;
use mcr_registers::pcie_ep::dwc_pcie_usp::pf0_msix_cap::RegisterBlock as PciMsixCap;
use mcr_registers::pcie_ep::dwc_pcie_usp::pf0_pcie_cap::RegisterBlock as PcieCap;
use mcr_registers::pcie_ep::dwc_pcie_usp::pf0_pl16g_cap::RegisterBlock as Pl16gCap;
use mcr_registers::pcie_ep::dwc_pcie_usp::pf0_pl32g_cap::RegisterBlock as Pl32gCap;
use mcr_registers::pcie_ep::dwc_pcie_usp::pf0_port_logic::RegisterBlock as PortLogic;
use mcr_registers::pcie_ep::dwc_pcie_usp::pf0_vsecras_cap::RegisterBlock as VsecrasCap;

use crate::PcieControllerErr;
use crate::PcieLinkStatus;

/// MAC Controller Implementation
pub struct PcieMac {
    phy0_soc: PhyRegisterBlock,
    assist_regs: Assist,
    vsecras_cap: VsecrasCap,
    port_logic: PortLogic,
    atu_cap: AtuCap,
    pcie_cap: PcieCap,
    l1sub_cap: L1SubCap,
    pl32g_cap: Pl32gCap,
    pl16g_cap: Pl16gCap,
    msix_cap: PciMsixCap,
}

impl PcieMac {
    /// Create an instance of `PcieMacImpl`
    pub fn new() -> Self {
        Self {
            phy0_soc: PhyRegisterBlock::block(),
            assist_regs: Assist::block(),
            vsecras_cap: VsecrasCap::block(),
            port_logic: PortLogic::block(),
            atu_cap: AtuCap::block(),
            pcie_cap: PcieCap::block(),
            l1sub_cap: L1SubCap::block(),
            pl32g_cap: Pl32gCap::block(),
            pl16g_cap: Pl16gCap::block(),
            msix_cap: PciMsixCap::block(),
        }
    }

    /// Enable MAC by configuring and enabling it
    ///
    /// # Retruns
    ///
    /// * `McrResult<()>` - Ok() or an appropriate module specific error code
    ///
    pub fn mac_init(&self) -> McrResult<()> {
        // wait for a max of 50 milliseconds
        let mut timeout_in_microseconds = 50000u32;
        while !self.phy0_soc.lane_status0().read().pm_txdclk_pclk_en_lane() {
            // wait for a microsecond
            cpu_stall(1);
            timeout_in_microseconds -= 1;
            if timeout_in_microseconds == 0 {
                return Err(PcieControllerErr::PhyPclkNotRunning)?;
            }
        }

        // Disable RAS_ERROR_MODE_EN bit
        self.vsecras_cap
            .rasdp_error_mode_en_off()
            .write(|_| 0x00000000u32.into());

        // Disable RADMx clock gate
        self.port_logic
            .clock_gating_ctrl_off()
            .read_and_modify(|_, w| w.radm_clk_gating_en(false));

        // Disable ECC for AXI data path
        self.vsecras_cap
            .rasdp_error_prot_ctrl_off()
            .write(|_| 0x00060006u32.into());

        // ACLK_FREQ
        self.port_logic
            .ack_f_aspm_ctrl_off()
            .read_and_modify(|_, w| w.ack_freq(0x1));

        // Disable direct speed change
        self.port_logic
            .gen2_ctrl_off()
            .read_and_modify(|_, w| w.direct_speed_change(false));

        // Pre-detrmined number of lanes
        self.port_logic
            .gen2_ctrl_off()
            .read_and_modify(|_, w| w.num_of_lanes(0x1));

        // Avoid auto-wake up
        self.port_logic
            .gen3_related_off()
            .read_and_modify(|_, w| w.gen3_zrxdc_noncompl(false));

        // Program Gen3 setting for DIR mode
        self.port_logic
            .gen3_related_off()
            .write(|_| 0x00002400u32.into());
        self.port_logic
            .gen3_eq_control_off()
            .write(|_| 0x00010020u32.into());

        // Program Gen4 setting for DIR mode
        // Set USP SEND EQ TS2 DISABLE to 0 for Gen4 EQ.  Enables USP
        // to request DSP to use initial TX EQ Preset when entering Gen4.
        self.port_logic
            .gen3_related_off()
            .write(|_| 0x01002400u32.into());
        self.port_logic
            .gen3_eq_control_off()
            .write(|_| 0x00002020u32.into());

        self.pl16g_cap
            .pl16_g_cap_off_20_h_reg()
            .write(|_| 0x50505050u32.into());

        // Program Gen5 setting for DIR mode
        // Update Gen5 TX EQ PRESET REQUEST VECTOR to include Preset 5 and Preset 9.
        // Previously only Preset 9 requested during Gen5 EQ Phase2.
        self.port_logic
            .gen3_related_off()
            .write(|_| 0x02002400u32.into());
        self.port_logic
            .gen3_eq_control_off()
            .write(|_| 0x0022020u32.into());
        self.pl32g_cap
            .pl32_g_cap_off_20_h_reg()
            .write(|_| 0x90909090u32.into());

        self.port_logic
            .gen3_related_off()
            .write(|_| 0x00002400u32.into());

        // Set EQ BYPASS HIGHEST RATE DISABLE to 1.  Do not skip equalization of intermediate data rates.
        self.pl32g_cap.pl32_g_control_reg().read_and_modify(|_, w| {
            w.eq_bypass_highest_rate_disable(true)
                .no_eq_needed_disable(true)
        });

        // Turn off Application transfer pending enable for ASPM
        //  SSDD-7360
        //  app_xfer_pending = EP_ASSIST+0x4[12] & (EP_ASSIST+0x4[11] | NVMe_xfer_pending)
        self.assist_regs
            .pcie_core_global_1()
            .read_and_modify(|_, w| w.app_xfer_pending_en(false));

        // Lane margining Gen4 / Gen5. Update Lane Margining settings for Gen4 and Gen5
        self.port_logic
            .gen4_lane_margining_1_off()
            .write(|_| 0x205f3220u32.into());
        self.port_logic
            .gen5_lane_margining_1_off()
            .write(|_| 0x0d3f3220u32.into());
        self.assist_regs
            .misc_status_2()
            .read_and_modify(|_, w| w.app_margining_software_ready(true));

        //
        // BAR configuration:
        //

        // iATU0 setup for VF CREG //VF BAR 0
        self.atu_cap
            .iatu_region_ctrl_2_off_inbound_0()
            .read_and_modify(|_, w| w.bar_num(0).vfbar_match_mode_en(true).region_en(true));
        self.atu_cap
            .iatu_lwr_target_addr_off_inbound_0()
            .write(|_| 0xA1400000u32.into());

        // iATU1 setup for VF MSIX// VF BAR 4
        self.atu_cap
            .iatu_region_ctrl_2_off_inbound_1()
            .read_and_modify(|_, w| w.bar_num(4).vfbar_match_mode_en(true).region_en(true));
        self.atu_cap
            .iatu_lwr_target_addr_off_inbound_1()
            .write(|_| 0xA1980000u32.into());

        // iATU2 setup for VF DB // VF BAR 2
        self.atu_cap
            .iatu_region_ctrl_2_off_inbound_2()
            .read_and_modify(|_, w| w.bar_num(2).vfbar_match_mode_en(true).region_en(true));
        self.atu_cap
            .iatu_lwr_target_addr_off_inbound_2()
            .write(|_| 0xA1440000u32.into());

        // iATU2 setup for PF MSIX //PF BAR 4
        self.atu_cap
            .iatu_region_ctrl_2_off_inbound_3()
            .read_and_modify(|_, w| w.bar_num(4).match_mode(true).region_en(true));
        self.atu_cap
            .iatu_lwr_target_addr_off_inbound_3()
            .write(|_| 0xA1900000u32.into());

        // iATU2 setup for PF CREG // PF BAR 0
        self.atu_cap
            .iatu_region_ctrl_2_off_inbound_4()
            .read_and_modify(|_, w| w.bar_num(0).match_mode(true).region_en(true));
        self.atu_cap
            .iatu_lwr_target_addr_off_inbound_4()
            .write(|_| 0xA1300000u32.into());

        // iATU2 setup for PF DB // PF BAR 2
        self.atu_cap
            .iatu_region_ctrl_2_off_inbound_5()
            .read_and_modify(|_, w| w.bar_num(2).match_mode(true).region_en(true));
        self.atu_cap
            .iatu_lwr_target_addr_off_inbound_5()
            .write(|_| 0xA1301000u32.into());

        // Hook EP interrupts
        self.interrupts_init();

        self.pcie_cap
            .device_capabilities_reg()
            .read_and_modify(|_, w| w.pcie_cap_flr_cap(true));

        self.pcie_cap
            .link_capabilities_reg()
            .read_and_modify(|_, w| w.pcie_cap_active_state_link_pm_support(0));

        self.msix_cap
            .pci_msix_cap_id_next_ctrl_reg()
            .read_and_modify(|_, w| w.pci_msix_cap_next_offset(0));

        self.l1sub_cap
            .l1_sub_capability_reg()
            .read_and_modify(|_, w| w.l1_1_aspm_support(false).l1_2_aspm_support(false));

        // Enable DOE
        PcieDoe::enable_hw();

        // Enable LTSSM
        self.assist_regs
            .pcie_core_global_0()
            .read_and_modify(|_, w| w.app_ltssm_enable(true));

        Ok(())
    }

    /// Enable PCIe interrupts
    fn interrupts_init(&self) {
        // Clear if any interrrupt stauts after enabling the interrupts
        self.assist_regs
            .pcie_core_int_status()
            .write(|_| 0xFFFFFFFFu32.into());
        self.assist_regs
            .flr_pf_active_int_status()
            .read_and_modify(|_, w| w.flr_pf_active_int_status(true));
        self.assist_regs.flr_vf_int_1().write(|_| 0xFFFFFFFFu32);
        self.assist_regs.flr_vf_int_2().write(|_| 0xFFFFFFFFu32);

        // Enanble interrupts
        self.assist_regs
            .pcie_core_int_enable()
            .read_and_modify(|_, w| w.flr_pf_active_en(true).flr_vf_en(true));
        self.assist_regs
            .flr_pf_active_int_en()
            .read_and_modify(|_, w| w.flr_pf_active_int_en(true));
        self.assist_regs.flr_vf_int_en_1().write(|_| 0xFFFFFFFFu32);
        self.assist_regs.flr_vf_int_en_2().write(|_| 0xFFFFFFFFu32);
    }

    /// Query PCIe link status, for speed and width
    pub fn link_status(&self) -> McrResult<PcieLinkStatus> {
        if !self.phy0_soc.lane_status0().read().pm_txdclk_pclk_en_lane() {
            Err(PcieControllerErr::PhyPclkNotRunning)?
        }

        if !self.assist_regs.pcie_core_global_0().read().rdlh_link_up() {
            Err(PcieControllerErr::PcieLinkNotInL0)?
        }

        Ok(PcieLinkStatus {
            speed: self
                .pcie_cap
                .link_control_link_status_reg()
                .read()
                .pcie_cap_link_speed(),
            width: self
                .pcie_cap
                .link_control_link_status_reg()
                .read()
                .pcie_cap_nego_link_width(),
        })
    }
}
