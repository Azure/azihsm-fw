// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::rc::Rc;
use core::cell::RefCell;

use mcr_cpu::*;
use mcr_error::McrResult;
use mcr_registers::pcie_assist::RegisterBlock as AssistRegisterBlock;
use mcr_registers::pcie_tdisp_cfg::RegisterBlock as TdispRegisterBlock;
use mcr_registers::pcie_top_reg::RegisterBlock as TopRegisterBlock;
use mcr_soc::*;
use mcr_types::PcieFunction;

use crate::mac_init::PcieMac;
use crate::phy_init::PciePhy;
use crate::PcieControllerTrait;
use crate::PcieEvent;
use crate::PcieLinkStatus;
use crate::PciePhyId;
use crate::TdispIntInfo;

/// Mask for TDISP_ERR_INT status in TDISP_INT_ST register.
const TDISP_ERR_INT_MASK: u32 = 0x1; // 1 << 0

/// PCIe Controller
#[derive(Clone)]
pub struct PcieController {
    rimpl: Rc<RefCell<PcieControllerImpl>>,
}

impl PcieController {
    /// Create an instance of `PcieController` and initialize the PHY
    ///
    /// # Returns
    ///
    /// * `McrResult<PcieController>` - PcieController instance or an error code if the
    ///   PHY is not initialized successfully
    pub fn new_with_phy_init() -> McrResult<Self> {
        let controller_rimpl = PcieControllerImpl::new_with_phy_init()?;

        Ok(Self {
            rimpl: Rc::new(RefCell::new(controller_rimpl)),
        })
    }

    /// Get PCIe event
    ///
    /// # Returns
    ///
    /// * `Option<PcieEvent>` - Pending PCIe event
    pub fn event() -> Option<PcieEvent> {
        let assist_regs = AssistRegisterBlock::block();
        let mut event: Option<PcieEvent> = None;

        if assist_regs.pcie_core_int_status().read().flr_pf_int() {
            // Clear the PCIe FLR interrupt status
            assist_regs
                .flr_pf_active_int_status()
                .read_and_modify(|_, w| w.flr_pf_active_int_status(true));

            assist_regs
                .pcie_core_global_0()
                .read_and_modify(|_, w| w.app_pf_req_retry_en(true));

            event = Some(PcieEvent::Flr);
        }

        if assist_regs.pcie_core_int_status().read().flr_vf_int() {
            #[allow(unused_assignments)]
            let mut vflr_status = u64::default();

            let reg = assist_regs.flr_vf_int_1().read();
            vflr_status = reg.into();
            if reg > 0 {
                assist_regs.flr_vf_int_1().write(|_| reg);
                assist_regs.app_vf_req_retry_en_1().write(|_| reg);
            }

            let reg = assist_regs.flr_vf_int_2().read();
            vflr_status |= (reg as u64) << 32u64;
            if reg > 0 {
                assist_regs.flr_vf_int_2().write(|_| reg);
                assist_regs.app_vf_req_retry_en_2().write(|_| reg);
            }

            event = Some(PcieEvent::VflrActive(vflr_status));
        }

        if assist_regs.pcie_core_int_status().read().tdisp_int() {
            let mut info = TdispIntInfo::default();

            let tdisp_regs = TdispRegisterBlock::block();

            // Only trigger TdispInterrupt event when TDISP_ERR is triggered.  This event should
            // override any previous event (e.g. VFLR).
            let int_status: u32 = tdisp_regs.tdisp_int_st().read().into();
            if (int_status & TDISP_ERR_INT_MASK) != 0 {
                let reg = tdisp_regs.tdisp_err_st0().read();
                info.vf_mask |= reg as u64;
                if reg > 0 {
                    tdisp_regs.tdisp_err_st0().write(|_| reg);
                }

                let reg = tdisp_regs.tdisp_err_st1().read();
                info.vf_mask |= (reg as u64) << 32u64;
                if reg > 0 {
                    tdisp_regs.tdisp_err_st1().write(|_| reg);
                }

                let reg = tdisp_regs.tdisp_err_st2().read().tdisp_err_st2();
                info.pf_mask = reg;
                if reg {
                    tdisp_regs.tdisp_err_st2().write(|_| 1u32.into());
                }

                // TODO: Consider adding additional registers (e.g. CII_STATUS, EC_STATUS) to the
                // info structure.
                info.info_regs[0] = int_status;
                info.info_regs[1] = tdisp_regs.lbc_cii_hdr0().read().into();
                info.info_regs[2] = tdisp_regs.lbc_cii_hdr1().read().into();
                info.info_regs[3] = tdisp_regs.lbc_cii_hdr2().read().into();
                info.info_regs[4] = tdisp_regs.lbc_cii_data().read();

                event = Some(PcieEvent::TdispInterrupt(info));
            }

            // Clear TDISP int status
            tdisp_regs.tdisp_int_st().write(|_| int_status.into());
        }

        event
    }

    /// Clear PERST UP interrupt
    pub fn clr_perst_up() {
        let top_regs = TopRegisterBlock::block();
        top_regs
            .int_sts()
            .read_and_modify(|_, w| w.perst_n_rdet_aon_sts(true));
    }

    /// Clear PERST DOWN interrupt
    pub fn clr_perst_down() {
        let top_regs = TopRegisterBlock::block();
        top_regs
            .int_sts()
            .read_and_modify(|_, w| w.perst_n_fdet_aon_sts(true));
    }
}

impl PcieControllerTrait for PcieController {
    /// Process PCIe PERST up event
    fn perst_up(&self) -> McrResult<()> {
        self.rimpl.borrow_mut().perst_up()
    }

    /// Process PCIe PERST down event
    fn perst_down(&self) {
        self.rimpl.borrow_mut().perst_down()
    }

    /// Query PCIe link status, for speed and width
    fn link_status(&self) -> McrResult<PcieLinkStatus> {
        self.rimpl.borrow_mut().link_status()
    }

    /// Complete PCIe FLR
    ///
    /// # Arguments
    ///
    /// * `pfn` - PcieFunction
    fn complete_flr(&self, pfn: PcieFunction) {
        self.rimpl.borrow_mut().complete_flr(pfn)
    }
}

/// PCIe Controller Implementation
struct PcieControllerImpl {
    top_regs: TopRegisterBlock,
    perst_glitch_detected: bool,
    perst_down_detected: bool,
    phy_ready: bool,
    pcie_mac: PcieMac,
    phy0: PciePhy,
    phy1: PciePhy,
}

impl PcieControllerImpl {
    /// Create an instance of `PcieControllerImpl` and initialize the PHY
    ///
    /// # Returns
    ///
    /// * `McrResult<PcieControllerImpl>` - PcieControllerImpl instance or an error code if the
    ///   PHY is not initialized successfully
    fn new_with_phy_init() -> McrResult<Self> {
        let top_regs = TopRegisterBlock::block();
        let soc_info = SocInfo::default();

        let cntrl = Self {
            top_regs,
            perst_glitch_detected: false,
            perst_down_detected: false,
            phy_ready: true,
            pcie_mac: PcieMac::new(),
            phy0: PciePhy::new(PciePhyId::ComPhy0),
            phy1: PciePhy::new(PciePhyId::ComPhy1),
        };

        if soc_info.reset_type() == SocResetType::Por {
            // Disable AXI remap. When enabled, AXI requests into PCIe are being
            // remapped to return DECERR to NVMe subsystem under PERST#, hot reset,
            // and FLR.
            top_regs.misc_ctrl2().write(|w| w.axi_remap_dis(true));

            // Block PERST going to PCIE let FW handle it
            top_regs.misc_ctrl1().write(|w| w.perst_n_dis(true));

            // Enable PERST falling and rising edge detection interrupts in PCIE hardware block
            top_regs
                .int_en()
                .write(|w| w.perst_n_fdet_aon_en(true).perst_n_rdet_aon_en(true));

            // Initialize PCIe PHY onetime during POR
            cntrl.one_time_phy_init()?;
        }

        Ok(cntrl)
    }

    /// Perform one-time PHY initialization
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok(()) if the PHY is initialized successfully
    fn one_time_phy_init(&self) -> McrResult<()> {
        // Configure PHY0 with following configuration
        //
        //  PHY Mode         - PCIe Mode
        //  Ref Clock Select - Ref Clock comes from Group 1
        //  Ref Clock Freq   - 100MHz
        self.top_regs
            .ctrl0_p0()
            .read_and_modify(|_, w| w.phy_mode_p0(0x3).refclk_sel_p0(true).ref_fref_sel_p0(5));

        // Configure PHY1 with following configuration
        //
        //  PHY Mode         - PCIe Mode
        //  Ref Clock Select - Ref Clock comes from Group 1
        //  Ref Clock Freq   - 100MHz
        self.top_regs
            .ctrl0_p1()
            .read_and_modify(|_, w| w.phy_mode_p1(0x3).refclk_sel_p1(true).ref_fref_sel_p1(5));

        // Execute PCIe Controller Cold Reset Sequence
        //
        //  Assert Cold Reset
        //  Stall for 1us
        //  Deassert Cold Reset
        //  Stall for 10us
        self.top_regs
            .pcie_reset_control()
            .read_and_modify(|_, w| w.pcie_x4_cold_rst(true));
        cpu_stall(1);
        self.top_regs
            .pcie_reset_control()
            .read_and_modify(|_, w| w.pcie_x4_cold_rst(false));
        cpu_stall(10);

        // Configure the PHY
        self.phy0.init()?;
        self.phy1.init()?;

        // Reset the PHY
        self.phy0.phy_mcu_reset_and_enable_lane();
        self.phy1.phy_mcu_reset_and_enable_lane();

        Ok(())
    }

    /// Process PCIe PERST up event
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok(()) if the PERST signal is processed successfully
    fn perst_up(&mut self) -> McrResult<()> {
        // Glitch detection logic for PERST Signal
        //
        // Note: This is needed for Eval Boards
        {
            if self.perst_glitch_detected && self.phy_ready {
                self.perst_glitch_detected = false;
                return Ok(());
            }

            if self.perst_down_detected && !self.phy_ready {
                cpu_stall(2);
                if !self.top_regs.int_sts().read().perst_n_raw_sts() {
                    cpu_stall(1);
                    if !self.top_regs.int_sts().read().perst_n_raw_sts() {
                        return Ok(());
                    }
                }
                self.perst_down_detected = false;
            }
        }

        // Execute PCIe Button Reset Sequence
        //
        //  Assert Button Reset
        //  Stall for 10us
        //  Deassert Button Reset
        //  Stall for 1us
        self.top_regs
            .pcie_reset_control()
            .read_and_modify(|_, w| w.pcie_x4_button_rst(true));
        cpu_stall(10);
        self.top_regs
            .pcie_reset_control()
            .read_and_modify(|_, w| w.pcie_x4_button_rst(false));
        cpu_stall(1);

        // Initialize the MAC
        self.pcie_mac.mac_init()?;

        Ok(())
    }

    /// Process PCIe PERST down event
    fn perst_down(&mut self) {
        // Glitch detection logic for PERST Signal
        //
        // Note: This is needed for Eval Boards
        {
            self.perst_glitch_detected = false;
            if self.perst_down_detected {
                return;
            }

            cpu_stall(2);

            if self.top_regs.int_sts().read().perst_n_raw_sts() {
                cpu_stall(1);
                self.perst_glitch_detected = self.top_regs.int_sts().read().perst_n_raw_sts();
            } else {
                self.perst_glitch_detected = false
            }

            if self.perst_glitch_detected {
                return;
            }

            self.perst_down_detected = true;
        }
    }

    /// Query PCIe link status, for speed and width
    ///
    /// # Retruns
    ///
    /// * `PcieLinkStatus` - instance of PcieLinkStatus or an error code if the link is not up
    ///
    fn link_status(&mut self) -> McrResult<PcieLinkStatus> {
        self.pcie_mac.link_status()
    }

    /// Complete PCIe FLR
    ///
    /// # Arguments
    ///
    /// * `pfn` - PcieFunction
    fn complete_flr(&mut self, pfn: PcieFunction) {
        let assist_regs = AssistRegisterBlock::block();

        let top_regs = TopRegisterBlock::block();

        if pfn == PcieFunction::Pf {
            assist_regs
                .flr_pf_done()
                .read_and_modify(|_, w| w.flr_pf_done(true));

            while assist_regs.flr_pf_active().read().flr_pf_active() {}

            assist_regs
                .flr_pf_done()
                .read_and_modify(|_, w| w.flr_pf_done(false));

            top_regs
                .flr_pf_active_clr()
                .read_and_modify(|_, w| w.flr_pf_active_clr(false));

            top_regs
                .flr_pf_active_clr()
                .read_and_modify(|_, w| w.flr_pf_active_clr(true));

            top_regs
                .flr_pf_active_clr()
                .read_and_modify(|_, w| w.flr_pf_active_clr(false));

            assist_regs
                .pcie_core_global_0()
                .read_and_modify(|_, w| w.app_pf_req_retry_en(false))
        }

        if u8::from(pfn) >= PcieFunction::Vf0.into() && u8::from(pfn) <= PcieFunction::Vf31.into() {
            let vf_mask = 1 << u32::from(pfn);

            assist_regs
                .flr_vf_done_1()
                .read_and_modify(|r, _| r | vf_mask);

            while (assist_regs.flr_vf_active_status_1().read() & vf_mask) != 0 {}

            assist_regs
                .flr_vf_done_1()
                .read_and_modify(|r, _| r & !vf_mask);

            top_regs
                .flr_vf_active_clr_lo()
                .read_and_modify(|r, _| r & !vf_mask);

            top_regs
                .flr_vf_active_clr_lo()
                .read_and_modify(|r, _| r | vf_mask);

            top_regs
                .flr_vf_active_clr_lo()
                .read_and_modify(|r, _| r & !vf_mask);

            assist_regs
                .app_vf_req_retry_en_1()
                .read_and_modify(|r, _| r & !vf_mask);
        }

        if u8::from(pfn) >= PcieFunction::Vf32.into() && u8::from(pfn) <= PcieFunction::Vf63.into()
        {
            let vf_mask = 1 << (u32::from(pfn) - u32::from(PcieFunction::Vf32));

            assist_regs
                .flr_vf_done_2()
                .read_and_modify(|r, _| r | vf_mask);

            while (assist_regs.flr_vf_active_status_2().read() & vf_mask) != 0 {}

            assist_regs
                .flr_vf_done_2()
                .read_and_modify(|r, _| r & !vf_mask);

            top_regs
                .flr_vf_active_clr_hi()
                .read_and_modify(|r, _| r & !vf_mask);

            top_regs
                .flr_vf_active_clr_hi()
                .read_and_modify(|r, _| r | vf_mask);

            top_regs
                .flr_vf_active_clr_hi()
                .read_and_modify(|r, _| r & !vf_mask);

            assist_regs
                .app_vf_req_retry_en_2()
                .read_and_modify(|r, _| r & !vf_mask);
        }
    }
}

impl Drop for PcieControllerImpl {
    /// Executes the destructor for PcieControllerImpl.
    fn drop(&mut self) {
        // Disable PERST falling edge and rising edge detection interrupts
        // in PCIE hardware block
        self.top_regs
            .int_en()
            .write(|w| w.perst_n_fdet_aon_en(false).perst_n_rdet_aon_en(false));
    }
}
