// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use mcr_error::McrResult;
use mcr_registers::comphy0_soc::RegisterBlock as PhyRegisterBlock;
use mcr_registers::pcie_top_reg::RegisterBlock as TopRegisterBlock;

use crate::reg::PcieCntrlReg;
use crate::PciePhyId;

/// PCIe Controller Implementation
pub struct PciePhy {
    top_regs: TopRegisterBlock,
    phy_id: PciePhyId,
    phy_regs: PhyRegisterBlock,
}

impl PciePhy {
    /// Create an instance of `PciePhy`
    pub fn new(id: PciePhyId) -> Self {
        Self {
            phy_id: id,
            top_regs: TopRegisterBlock::block(),
            phy_regs: PcieCntrlReg::phy_reg(id),
        }
    }

    /// Enable Pcie PHY by configuring, resetting, and enabling PHY0 or PHY1 selectively
    ///
    /// # Retruns
    ///
    /// * `McrResult<()>` - Ok() or an appropriate module specific error code
    ///
    pub fn init(&self) -> McrResult<()> {
        self.config()
    }

    /// Configure PHY0 or PHY1
    ///
    /// # Retruns
    ///
    /// * `McrResult<()>` - Ok() or an appropriate module specific error code
    ///
    fn config(&self) -> McrResult<()> {
        let is_phy0 = self.phy_id == PciePhyId::ComPhy0;

        // Step 1 - Program the LANE START, BREAK and MASTER for both PHYs

        // Clear broadcast mode = 0
        self.phy_regs
            .system()
            .read_and_modify(|_, w| w.broadcast(false));
        self.phy_regs
            .system()
            .read_and_modify(|_, w| w.lane_sel_2_0(0));

        self.phy_regs.glob_clk_src_hi().read_and_modify(|_, w| {
            w.mode_pipe4_if_lane(true)
                .lane_start_lane(false)
                .lane_master_lane(false)
                .lane_break_lane(false)
        });

        // Configure Lane
        self.phy_regs
            .system()
            .read_and_modify(|_, w| w.lane_sel_2_0(0x1));

        self.phy_regs.glob_clk_src_hi().read_and_modify(|_, w| {
            w.mode_pipe4_if_lane(true)
                .lane_start_lane(is_phy0)
                .lane_master_lane(is_phy0)
                .lane_break_lane(!is_phy0)
        });

        self.phy_regs
            .system()
            .read_and_modify(|_, w| w.lane_sel_2_0(0));

        // Step 2 - Set broadcast mode, remove if set by default
        self.phy_regs
            .system()
            .read_and_modify(|_, w| w.broadcast(true));

        // Step 3 - MCU lanes disabled
        self.phy_regs
            .mcu_control_0()
            .read_and_modify(|_, w| w.mcu_en_cmn(false).mcu_en_lane0(false).mcu_en_lane1(false));

        // Step 8.a - Beacon divider
        self.phy_regs
            .spd_cmn_reg1()
            .read_and_modify(|_, w| w.beacon_divider_1_0(0x1));
        self.phy_regs
            .pm_ctrl_tx_lane_reg2_lane()
            .read_and_modify(|_, w| w.cnt_ini_lane_7_0(0xC7));

        // Step 8.b - MCU frequency
        // 0x190 - 400 MHz
        self.phy_regs
            .mcu_config1()
            .read_and_modify(|_, w| w.mcu_freq_15_0(0x190));

        // Step 9 - Enable Lane 0 & 1
        self.phy_regs
            .clkgen_cmn_reg1()
            .read_and_modify(|_, w| w.en_cmn(true).en_lane0(true).en_lane1(true));

        // Step 10 - PIN_PHY_MODE = 0x3 for PCIe
        if is_phy0 {
            self.top_regs
                .ctrl0_p0()
                .read_and_modify(|_, w| w.phy_mode_p0(0x3));
        } else {
            self.top_regs
                .ctrl0_p1()
                .read_and_modify(|_, w| w.phy_mode_p1(0x3));
        }

        // Step 11 - REFCLK selection
        self.phy_regs
            .pm_cmn_reg1()
            .read_and_modify(|_, w| w.refclk_sel(true));

        //  Step 12 - Program CFG_BAL_WEIGHT_LANE[47:42] = 0x30
        self.phy_regs
            .glob_dp_bal_cfg2()
            .read_and_modify(|_, w| w.cfg_bal_weight_lane_47_42(0x30));

        // Step 13 - PIN_PHY_MODE
        self.phy_regs
            .system()
            .read_and_modify(|_, w| w.phy_mode_2_0(0x3));

        //  * Step 14 - PHY_GEN_MAX (32G)
        self.phy_regs
            .control_config8()
            .read_and_modify(|_, w| w.phy_gen_max_3_0(0x4));

        // Step 15 - Pipe spec version 4.4.1
        self.phy_regs
            .lane_eq_16g_cfg0_lane()
            .read_and_modify(|_, w| w.cfg_preset_index_sel_lane(true));

        self.phy_regs
            .glob_protocol_cfg0()
            .read_and_modify(|_, w| w.cfg_pipe_msg_bus_protocol_sel_lane(true));

        // Step 16 - PIPE4_EN, MODE_PIPE4_IF
        self.phy_regs
            .train_if_config()
            .read_and_modify(|_, w| w.pipe4_en(true));

        // Step 17 - CFG_RC_EP_LANE EP:0, RC:1
        self.phy_regs
            .lane_eq_cfg0_lane()
            .read_and_modify(|_, w| w.cfg_phy_rc_ep_lane(false));

        //  * Step 18 - CFG_CLK_SRC_MSK
        self.phy_regs
            .glob_clk_src_lo()
            .read_and_modify(|_, w| w.cfg_clk_src_mask_lane(true));

        //  * Step 19 - DET_BYPASS_LANE
        self.phy_regs
            .rx_data_path_reg()
            .read_and_modify(|_, w| w.det_bypass_lane(true));

        // Step 20 - LINK_TRAIN_MODE_LANE (GEN>=GEN3)
        self.phy_regs
            .tx_train_ctrl_lane()
            .read_and_modify(|_, w| w.link_train_mode_lane(true));

        //  * Step 21 - Analog Idle Sync Enable
        self.phy_regs
            .pm_ctrl_tx_lane_reg1_lane()
            .read_and_modify(|_, w| w.ana_idle_sync_en_lane(true));

        // Step 22 - RX EQ training control
        self.phy_regs
            .lane_cfg4()
            .read_and_modify(|_, w| w.cfg_rx_eq_ctrl_lane(true));

        // Step 23 - Support force x1 mode from MAC
        // NOP

        // Step 24 Configuration 0
        self.phy_regs.glob_dp_sal_cfg().read_and_modify(|_, w| {
            w.cfg_txelecidle_assert_lane(false)
                .cfg_gen1_txelecidle_dly_lane_1_0(0x0)
        });

        self.phy_regs.system().read_and_modify(|_, w| {
            w.slave_align_refclk_fm_side_a(!is_phy0)
                .ana_cmn_phy_x2_master_en_1_0(0x3)
        });
        self.phy_regs
            .mcu_config()
            .read_and_modify(|_, w| w.master_mcu_sel_7_0(if is_phy0 { 0x1 } else { 0x0 }));
        self.phy_regs
            .glob_clk_src_lo()
            .read_and_modify(|_, w| w.cfg_use_align_clk_lane(true));

        // Step 25 - Configuration 1 for PHY0 & PHY1
        self.phy_regs.system().read_and_modify(|_, w| {
            w.phy_align_off(false)
                .master_phy_en(is_phy0)
                .phy_config_1_0(0x0)
        });
        self.phy_regs
            .uphy14_trx_anareg_bot_25()
            .read_and_modify(|_, w| w.tx_txclk_align_en_lane(true));

        // Step 26 - Lane alignment
        // Set PLL READY DLY LANE to 7.  Increases delay between PLL lock
        // and PLL ready indicator to allow more time for PLL lock to settle.
        self.phy_regs.glob_clk_src_lo().read_and_modify(|_, w| {
            w.pll_ready_dly_lane_2_0(0x7)
                .cfg_use_lane_align_rdy_lane(true)
                .bundle_pll_rdy_lane(true)
        });

        // Step 27 - Fine tune the lane alignment
        self.phy_regs
            .dtx_phy_align_reg0()
            .read_and_modify(|_, w| w.lane_align_fast_done_sel_1_0(0x0));
        self.phy_regs
            .dtx_phy_align_reg1()
            .read_and_modify(|_, w| w.align_accurate_step_1_0(0x3).align_accurate_en(true));

        // 12 nm
        self.phy_regs
            .uphy14_cmn_anareg_top_210()
            .read_and_modify(|_, w| w.vddvco_vth_12nm_sel(true));

        // Step 28 - Tx and Rx impedance
        self.phy_regs
            .uphy14_cmn_anareg_top_138()
            .read_and_modify(|_, w| w.vth_rximpcal_2_0(0x0));
        self.phy_regs
            .uphy14_cmn_anareg_top_129()
            .write(|w| w.vth_tximpcal_2_0(0x0));

        // Step 29 - Gen3\Gen4 CDR
        self.phy_regs.autospeed425().write(|_| 0x02020202.into());

        // Gen4 CDR
        self.phy_regs
            .autospeed445()
            .read_and_modify(|_, w| w.rx_selmufi_g3_lane_2_0(0x3).rx_selmuff_g3_lane_2_0(0x3));
        self.phy_regs.autospeed444().read_and_modify(|_, w| {
            w.rx_reg0p9_speed_track_data_g3_lane_2_0(0x1)
                .rx_reg0p9_speed_track_clk_half_g3_lane(true)
        });
        self.phy_regs.autospeed465().write(|_| 0x04040303.into());

        //  * Step 30 - TxDetectRx parameters
        self.phy_regs
            .uphy14_trx_anareg_top_148()
            .read_and_modify(|_, w| w.txdetrx_vth_lane_1_0(0x2));

        self.phy_regs
            .pm_ctrl_tx_lane_reg1_lane()
            .read_and_modify(|_, w| w.txdetrx_sampling_point_lane_2_0(0x1));

        // Step 31 - Keep PCLK running in hot-reset
        self.phy_regs
            .lane_system0()
            .read_and_modify(|_, w| w.reset_core_fm_pipe_lane(true));
        self.phy_regs
            .glob_rst_clk_ctrl()
            .read_and_modify(|_, w| w.mode_core_clk_ctrl_lane(true));
        self.phy_regs
            .glob_rst_clk_ctrl()
            .read_and_modify(|_, w| w.mode_p3_osc_pclk_en_lane(true));

        // Step 33 - FOM mode settings
        // G4 Tx training disable
        self.phy_regs.local_tx_preset_tb5().read_and_modify(|_, w| {
            w.tx_adapt_g1_en_pcie_gen4(false)
                .tx_adapt_gn1_en_pcie_gen4(false)
                .tx_adapt_g0_en_pcie_gen4(false)
        });

        self.phy_regs
            .autospeed159()
            .read_and_modify(|_, w| w.icp_ring_pion_rate1_3_0(0xE));

        Ok(())
    }

    /// Reset and enable PHY0 & PHY1 lane
    pub fn phy_mcu_reset_and_enable_lane(&self) {
        // Step 35 - PIPE_SFT_RESET
        self.phy_regs
            .glob_rst_clk_ctrl()
            .read_and_modify(|_, w| w.pipe_sft_reset_lane(false));

        // Step 36 - MCU Enable LanesS
        // Set MCU_EN_CMN to 0 functional change
        self.phy_regs
            .mcu_control_0()
            .read_and_modify(|_, w| w.mcu_en_cmn(false).mcu_en_lane0(true).mcu_en_lane1(true));
    }
}
