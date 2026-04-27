//-----------------------------------------------------------------------------
//
// Copyright (c) 2022 Marvell. All rights reserved.
// The following file is subject to the limited use license agreement by and
// between Marvell and you, your employer or other entity on behalf of whom
// you act. In the absence of such license agreement the following file is
// subject to Marvell's standard Limited Use License Agreement.
//
//-----------------------------------------------------------------------------

//=============================================================================
//!
//! @brief COMPHY0_SOC Registers
//!
//=============================================================================

// Generated with Dullahan v2.4.3.

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "SharedStruct.h"
#include "SysTypes.h"

//-----------------------------------------------------------------------------
//  Public Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Public Data Type Definitions
//-----------------------------------------------------------------------------


/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t SQ_THRESH_LANE_5_0          :6;      ///<BIT [7:2] sq_thresh_lane_5_0
        uint32_t RSVD_8_31                   :24;     ///<BIT [31:8] rsvd_8_31
    } b;
} Comphy0Socuphy14TrxAnaregBot4_t;

/// @brief 0x4C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t SLEWCTRL1_LANE_1_0          :2;      ///<BIT [3:2] slewctrl1_lane_1_0
        uint32_t SLEWCTRL0_LANE_1_0          :2;      ///<BIT [5:4] slewctrl0_lane_1_0
        uint32_t SLEWRATE_EN_LANE_1_0        :2;      ///<BIT [7:6] slewrate_en_lane_1_0
        uint32_t RSVD_8_31                   :24;     ///<BIT [31:8] rsvd_8_31
    } b;
} Comphy0Socuphy14TrxAnaregBot19_t;

/// @brief 0x64
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      :1;      ///<BIT [0] rsvd_0
        uint32_t TX_TXCLK_ALIGN_EN_LANE      :1;      ///<BIT [1] tx_txclk_align_en_lane
        uint32_t RSVD_2_31                   :30;     ///<BIT [31:2] rsvd_2_31
    } b;
} Comphy0Socuphy14TrxAnaregBot25_t;

/// @brief 0x208
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t PU_LB_DLY_LANE              :1;      ///<BIT [2] pu_lb_dly_lane
        uint32_t PU_LB_LANE                  :1;      ///<BIT [3] pu_lb_lane
        uint32_t FFE_DATA_RATE_LANE_3_0      :4;      ///<BIT [7:4] ffe_data_rate_lane_3_0
        uint32_t RSVD_8_31                   :24;     ///<BIT [31:8] rsvd_8_31
    } b;
} Comphy0Socuphy14TrxAnaregTop130_t;

/// @brief 0x214
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t SQ_THRESH_CAL_EN_LANE       :1;      ///<BIT [2] sq_thresh_cal_en_lane
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} Comphy0Socuphy14TrxAnaregTop133_t;

/// @brief 0x244
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_5                    :6;      ///<BIT [5:0] rsvd_0_5
        uint32_t LOCAL_ANA_TX2RX_LPBK_EN_LANE :1;      ///<BIT [6] local_ana_tx2rx_lpbk_en_lane
        uint32_t RSVD_7_31                   :25;     ///<BIT [31:7] rsvd_7_31
    } b;
} Comphy0Socuphy14TrxAnaregTop145_t;

/// @brief 0x250
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t TXDETRX_VTH_LANE_1_0        :2;      ///<BIT [3:2] txdetrx_vth_lane_1_0
        uint32_t RSVD_4_31                   :28;     ///<BIT [31:4] rsvd_4_31
    } b;
} Comphy0Socuphy14TrxAnaregTop148_t;

/// @brief 0x2000
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_3                    :4;      ///<BIT [3:0] rsvd_0_3
        uint32_t BEACON_EN_DELAY_LANE_1_0    :2;      ///<BIT [5:4] beacon_en_delay_lane_1_0
        uint32_t RSVD_6_10                   :5;      ///<BIT [10:6] rsvd_6_10
        uint32_t TXDETRX_SAMPLING_POINT_LANE_2_0 :3;      ///<BIT [13:11] txdetrx_sampling_point_lane_2_0
        uint32_t RSVD_14_19                  :6;      ///<BIT [19:14] rsvd_14_19
        uint32_t PLL_READY_TX_LANE           :1;      ///<BIT [20] pll_ready_tx_lane
        uint32_t RSVD_21_22                  :2;      ///<BIT [22:21] rsvd_21_22
        uint32_t ANA_IDLE_SYNC_EN_LANE       :1;      ///<BIT [23] ana_idle_sync_en_lane
        uint32_t RSVD_24_31                  :8;      ///<BIT [31:24] rsvd_24_31
    } b;
} Comphy0SocpmCtrlTxLaneReg1Lane_t;

/// @brief 0x2004
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CNT_INI_LANE_7_0            :8;      ///<BIT [7:0] cnt_ini_lane_7_0
        uint32_t RSVD_8_31                   :24;     ///<BIT [31:8] rsvd_8_31
    } b;
} Comphy0SocpmCtrlTxLaneReg2Lane_t;

/// @brief 0x2008
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PU_TX_FM_REG_LANE           :1;      ///<BIT [0] pu_tx_fm_reg_lane
        uint32_t PU_TX_LANE                  :1;      ///<BIT [1] pu_tx_lane
        uint32_t PU_PLL_FM_REG_LANE          :1;      ///<BIT [2] pu_pll_fm_reg_lane
        uint32_t PU_PLL_LANE                 :1;      ///<BIT [3] pu_pll_lane
        uint32_t RSVD_4_26                   :23;     ///<BIT [26:4] rsvd_4_26
        uint32_t PHY_GEN_TX_FM_REG_LANE      :1;      ///<BIT [27] phy_gen_tx_fm_reg_lane
        uint32_t PHY_GEN_TX_LANE_3_0         :4;      ///<BIT [31:28] phy_gen_tx_lane_3_0
    } b;
} Comphy0SocinputTxPinReg0Lane_t;

/// @brief 0x200C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      :1;      ///<BIT [0] rsvd_0
        uint32_t SSC_EN_FM_REG_LANE          :1;      ///<BIT [1] ssc_en_fm_reg_lane
        uint32_t SSC_EN_LANE                 :1;      ///<BIT [2] ssc_en_lane
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} Comphy0SocinputTxPinReg1Lane_t;

/// @brief 0x2010
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_4                    :5;      ///<BIT [4:0] rsvd_0_4
        uint32_t TX_TRAIN_ENABLE_LANE        :1;      ///<BIT [5] tx_train_enable_lane
        uint32_t TX_TRAIN_ENABLE_FM_REG_LANE :1;      ///<BIT [6] tx_train_enable_fm_reg_lane
        uint32_t RSVD_7_31                   :25;     ///<BIT [31:7] rsvd_7_31
    } b;
} Comphy0SocinputTxPinReg2Lane_t;

/// @brief 0x2014
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_17                   :18;     ///<BIT [17:0] rsvd_0_17
        uint32_t TX_IDLE_LANE                :1;      ///<BIT [18] tx_idle_lane
        uint32_t RSVD_19_22                  :4;      ///<BIT [22:19] rsvd_19_22
        uint32_t REPEAT_MODE_EN_LANE         :1;      ///<BIT [23] repeat_mode_en_lane
        uint32_t RSVD_24_31                  :8;      ///<BIT [31:24] rsvd_24_31
    } b;
} Comphy0SocinputTxPinReg3Lane_t;

/// @brief 0x2020
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_27                   :28;     ///<BIT [27:0] rsvd_0_27
        uint32_t REFCLK_ON_DCLK_DIS_LANE     :1;      ///<BIT [28] refclk_on_dclk_dis_lane
        uint32_t RSVD_29                     :1;      ///<BIT [29] rsvd_29
        uint32_t TXDCLK_4X_EN_LANE           :1;      ///<BIT [30] txdclk_4x_en_lane
        uint32_t TXDCLK_NT_EN_LANE           :1;      ///<BIT [31] txdclk_nt_en_lane
    } b;
} Comphy0SocclkgenTxLaneReg1Lane_t;

/// @brief 0x2024
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_15                   :16;     ///<BIT [15:0] rsvd_0_15
        uint32_t TXDATA_LATENCY_REDUCE_EN_LANE :1;      ///<BIT [16] txdata_latency_reduce_en_lane
        uint32_t RSVD_17_25                  :9;      ///<BIT [25:17] rsvd_17_25
        uint32_t ADD_ERR_NUM_LANE_2_0        :3;      ///<BIT [28:26] add_err_num_lane_2_0
        uint32_t ADD_ERR_EN_LANE             :1;      ///<BIT [29] add_err_en_lane
        uint32_t TXD_INV_LANE                :1;      ///<BIT [30] txd_inv_lane
        uint32_t LOCAL_DIG_RX2TX_LPBK_EN_LANE :1;      ///<BIT [31] local_dig_rx2tx_lpbk_en_lane
    } b;
} Comphy0SoctxSpeedConvertLane_t;

/// @brief 0x2030
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_7                    :8;      ///<BIT [7:0] rsvd_0_7
        uint32_t REF1M_GEN_DIV_LANE_7_0      :8;      ///<BIT [15:8] ref1m_gen_div_lane_7_0
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Comphy0SocspdCtrlTxLaneReg1Lane_t;

/// @brief 0x2034
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_30                   :31;     ///<BIT [30:0] rsvd_0_30
        uint32_t TX_SEL_BITS_LANE            :1;      ///<BIT [31] tx_sel_bits_lane
    } b;
} Comphy0SoctxSystemLane_t;

/// @brief 0x205C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_7                    :8;      ///<BIT [7:0] rsvd_0_7
        uint32_t TESTBUS_SEL_LO0_LANE_5_0    :6;      ///<BIT [13:8] testbus_sel_lo0_lane_5_0
        uint32_t RSVD_14_23                  :10;     ///<BIT [23:14] rsvd_14_23
        uint32_t TESTBUS_SEL_HI0_LANE_5_0    :6;      ///<BIT [29:24] testbus_sel_hi0_lane_5_0
        uint32_t RSVD_30_31                  :2;      ///<BIT [31:30] rsvd_30_31
    } b;
} Comphy0SocmonTop_t;

/// @brief 0x2100
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_18                   :19;     ///<BIT [18:0] rsvd_0_18
        uint32_t RX_INIT_DONE_LANE           :1;      ///<BIT [19] rx_init_done_lane
        uint32_t RSVD_20_23                  :4;      ///<BIT [23:20] rsvd_20_23
        uint32_t PLL_READY_RX_LANE           :1;      ///<BIT [24] pll_ready_rx_lane
        uint32_t RSVD_25_31                  :7;      ///<BIT [31:25] rsvd_25_31
    } b;
} Comphy0SocpmCtrlRxLaneReg1Lane_t;

/// @brief 0x2104
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_30                   :31;     ///<BIT [30:0] rsvd_0_30
        uint32_t RX_SEL_BITS_LANE            :1;      ///<BIT [31] rx_sel_bits_lane
    } b;
} Comphy0SocrxSystemLane_t;

/// @brief 0x2108
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_26                   :27;     ///<BIT [26:0] rsvd_0_26
        uint32_t PHY_GEN_RX_FM_REG_LANE      :1;      ///<BIT [27] phy_gen_rx_fm_reg_lane
        uint32_t PHY_GEN_RX_LANE_3_0         :4;      ///<BIT [31:28] phy_gen_rx_lane_3_0
    } b;
} Comphy0SocinputRxPinReg0Lane_t;

/// @brief 0x210C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t RX_TRAIN_ENABLE_FM_REG_LANE :1;      ///<BIT [2] rx_train_enable_fm_reg_lane
        uint32_t RX_TRAIN_ENABLE_LANE        :1;      ///<BIT [3] rx_train_enable_lane
        uint32_t RSVD_4_6                    :3;      ///<BIT [6:4] rsvd_4_6
        uint32_t RX_INIT_LANE                :1;      ///<BIT [7] rx_init_lane
        uint32_t RSVD_8_23                   :16;     ///<BIT [23:8] rsvd_8_23
        uint32_t RX_DC_TERM_EN_FM_REG_LANE   :1;      ///<BIT [24] rx_dc_term_en_fm_reg_lane
        uint32_t RSVD_25_28                  :4;      ///<BIT [28:25] rsvd_25_28
        uint32_t RX_DC_TERM_EN_LANE          :1;      ///<BIT [29] rx_dc_term_en_lane
        uint32_t RSVD_30_31                  :2;      ///<BIT [31:30] rsvd_30_31
    } b;
} Comphy0SocinputRxPinReg1Lane_t;

/// @brief 0x2110
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_7                    :8;      ///<BIT [7:0] rsvd_0_7
        uint32_t PU_RX_LANE                  :1;      ///<BIT [8] pu_rx_lane
        uint32_t RSVD_9_31                   :23;     ///<BIT [31:9] rsvd_9_31
    } b;
} Comphy0SocinputRxPinReg2Lane_t;

/// @brief 0x211C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RXDCLK_4X_EN_LANE           :1;      ///<BIT [0] rxdclk_4x_en_lane
        uint32_t RXDCLK_NT_EN_LANE           :1;      ///<BIT [1] rxdclk_nt_en_lane
        uint32_t RXDCLK_25M_EN_LANE          :1;      ///<BIT [2] rxdclk_25m_en_lane
        uint32_t RSVD_3_25                   :23;     ///<BIT [25:3] rsvd_3_25
        uint32_t RST_FRAME_SYNC_DET_CLK_LANE :1;      ///<BIT [26] rst_frame_sync_det_clk_lane
        uint32_t RSVD_27_31                  :5;      ///<BIT [31:27] rsvd_27_31
    } b;
} Comphy0SocclkgenRxLaneReg1Lane_t;

/// @brief 0x2120
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_2                    :3;      ///<BIT [2:0] rsvd_0_2
        uint32_t FRAME_LOCK_SEL_LANE         :1;      ///<BIT [3] frame_lock_sel_lane
        uint32_t FRAME_DET_SIDE_LEVEL_LANE_1_0 :2;      ///<BIT [5:4] frame_det_side_level_lane_1_0
        uint32_t FRAME_DET_MIDD_LEVEL_LANE_1_0 :2;      ///<BIT [7:6] frame_det_midd_level_lane_1_0
        uint32_t RSVD_8                      :1;      ///<BIT [8] rsvd_8
        uint32_t SYNC_MASK_LANE_9_0          :10;     ///<BIT [18:9] sync_mask_lane_9_0
        uint32_t SYNC_CHAR_LANE_9_0          :10;     ///<BIT [28:19] sync_char_lane_9_0
        uint32_t SYNC_POL_LANE               :1;      ///<BIT [29] sync_pol_lane
        uint32_t RSVD_30                     :1;      ///<BIT [30] rsvd_30
        uint32_t SYNC_DET_EN_LANE            :1;      ///<BIT [31] sync_det_en_lane
    } b;
} Comphy0SocframeSyncDetReg0_t;

/// @brief 0x2124
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TRAIN_PAT_NUM_LANE_9_0      :10;     ///<BIT [9:0] train_pat_num_lane_9_0
        uint32_t RSVD_10_18                  :9;      ///<BIT [18:10] rsvd_10_18
        uint32_t FRAME_FOUND_LANE            :1;      ///<BIT [19] frame_found_lane
        uint32_t SYNC_FOUND_LANE             :1;      ///<BIT [20] sync_found_lane
        uint32_t RSVD_21                     :1;      ///<BIT [21] rsvd_21
        uint32_t ALIGN_STAT_RD_REQ_LANE      :1;      ///<BIT [22] align_stat_rd_req_lane
        uint32_t FRAME_DET_MODE_LANE         :1;      ///<BIT [23] frame_det_mode_lane
        uint32_t FRAME_REALIGN_MODE_LANE     :1;      ///<BIT [24] frame_realign_mode_lane
        uint32_t RSVD_25                     :1;      ///<BIT [25] rsvd_25
        uint32_t BAD_MARKER_NUM_LANE_2_0     :3;      ///<BIT [28:26] bad_marker_num_lane_2_0
        uint32_t GOOD_MARKER_NUM_LANE_2_0    :3;      ///<BIT [31:29] good_marker_num_lane_2_0
    } b;
} Comphy0SocframeSyncDetReg1_t;

/// @brief 0x213C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_29                   :30;     ///<BIT [29:0] rsvd_0_29
        uint32_t CDR_LOCK_LANE               :1;      ///<BIT [30] cdr_lock_lane
        uint32_t CDR_LOCK_MODE_LANE          :1;      ///<BIT [31] cdr_lock_mode_lane
    } b;
} Comphy0SoccdrLock_t;

/// @brief 0x2148
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_27                   :28;     ///<BIT [27:0] rsvd_0_27
        uint32_t RXDATA_LATENCY_REDUCE_EN_LANE :1;      ///<BIT [28] rxdata_latency_reduce_en_lane
        uint32_t RXD_INV_LANE                :1;      ///<BIT [29] rxd_inv_lane
        uint32_t DET_BYPASS_LANE             :1;      ///<BIT [30] det_bypass_lane
        uint32_t LOCAL_DIG_TX2RX_LPBK_EN_LANE :1;      ///<BIT [31] local_dig_tx2rx_lpbk_en_lane
    } b;
} Comphy0SocrxDataPath_t;

/// @brief 0x2160
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_12                   :13;     ///<BIT [12:0] rsvd_0_12
        uint32_t DTL_SQ_DET_EN_LANE          :1;      ///<BIT [13] dtl_sq_det_en_lane
        uint32_t DTL_FLOOP_EN_LANE           :1;      ///<BIT [14] dtl_floop_en_lane
        uint32_t SSC_DSPREAD_RX_LANE         :1;      ///<BIT [15] ssc_dspread_rx_lane
        uint32_t RX_FOFFSET_RD_REQ_LANE      :1;      ///<BIT [16] rx_foffset_rd_req_lane
        uint32_t RSVD_17_20                  :4;      ///<BIT [20:17] rsvd_17_20
        uint32_t DTL_CLAMPING_RATIO_NEG_LANE_1_0 :2;      ///<BIT [22:21] dtl_clamping_ratio_neg_lane_1_0
        uint32_t DTL_CLAMPING_SCALE_LANE     :1;      ///<BIT [23] dtl_clamping_scale_lane
        uint32_t DTL_CLAMPING_SEL_LANE_2_0   :3;      ///<BIT [26:24] dtl_clamping_sel_lane_2_0
        uint32_t RSVD_27_31                  :5;      ///<BIT [31:27] rsvd_27_31
    } b;
} Comphy0SocdtlReg0_t;

/// @brief 0x2164
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_9                    :10;     ///<BIT [9:0] rsvd_0_9
        uint32_t RX_SELMUFI_LANE_2_0         :3;      ///<BIT [12:10] rx_selmufi_lane_2_0
        uint32_t RX_SELMUFF_LANE_2_0         :3;      ///<BIT [15:13] rx_selmuff_lane_2_0
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Comphy0SocdtlReg1_t;

/// @brief 0x2168
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_18                   :19;     ///<BIT [18:0] rsvd_0_18
        uint32_t RX_FOFFSET_DISABLE_LANE     :1;      ///<BIT [19] rx_foffset_disable_lane
        uint32_t DTL_STEP_MODE_LANE          :1;      ///<BIT [20] dtl_step_mode_lane
        uint32_t RX_FOFFSET_RD_LANE_9_0      :10;     ///<BIT [30:21] rx_foffset_rd_lane_9_0
        uint32_t RX_FOFFSET_RDY_LANE         :1;      ///<BIT [31] rx_foffset_rdy_lane
    } b;
} Comphy0SocdtlReg2_t;

/// @brief 0x2170
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SQ_DEGLITCH_WIDTH_P_LANE_3_0 :4;      ///<BIT [3:0] sq_deglitch_width_p_lane_3_0
        uint32_t SQ_DEGLITCH_WIDTH_N_LANE_3_0 :4;      ///<BIT [7:4] sq_deglitch_width_n_lane_3_0
        uint32_t SQ_DEGLITCH_EN_LANE         :1;      ///<BIT [8] sq_deglitch_en_lane
        uint32_t RSVD_9_10                   :2;      ///<BIT [10:9] rsvd_9_10
        uint32_t INT_SQ_LPF_EN_LANE          :1;      ///<BIT [11] int_sq_lpf_en_lane
        uint32_t SQ_LPF_EN_LANE              :1;      ///<BIT [12] sq_lpf_en_lane
        uint32_t SQ_GATE_RXDATA_EN_LANE      :1;      ///<BIT [13] sq_gate_rxdata_en_lane
        uint32_t PIN_RX_SQ_OUT_LPF_RD_LANE   :1;      ///<BIT [14] pin_rx_sq_out_lpf_rd_lane
        uint32_t PIN_RX_SQ_OUT_RD_LANE       :1;      ///<BIT [15] pin_rx_sq_out_rd_lane
        uint32_t SQ_LPF_LANE_15_0            :16;     ///<BIT [31:16] sq_lpf_lane_15_0
    } b;
} Comphy0SocsqReg0_t;

/// @brief 0x2200
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MCU_ID_LANE_7_0             :8;      ///<BIT [7:0] mcu_id_lane_7_0
        uint32_t RSVD_8_31                   :24;     ///<BIT [31:8] rsvd_8_31
    } b;
} Comphy0SocmcuControlLane_t;

/// @brief 0x2210
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SFT_RST_NO_REG_LANE         :1;      ///<BIT [0] sft_rst_no_reg_lane
        uint32_t SFT_RST_NO_REG_FM_REG_LANE  :1;      ///<BIT [1] sft_rst_no_reg_fm_reg_lane
        uint32_t RSVD_2_3                    :2;      ///<BIT [3:2] rsvd_2_3
        uint32_t INIT_DONE_LANE              :1;      ///<BIT [4] init_done_lane
        uint32_t RESET_CORE_FM_PIPE_LANE     :1;      ///<BIT [5] reset_core_fm_pipe_lane
        uint32_t RSVD_6_31                   :26;     ///<BIT [31:6] rsvd_6_31
    } b;
} Comphy0SoclaneSystem0_t;

/// @brief 0x2294
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_3                    :4;      ///<BIT [3:0] rsvd_0_3
        uint32_t XDATA_ECC_1ERR_LANE         :1;      ///<BIT [4] xdata_ecc_1err_lane
        uint32_t CACHE_ECC_1ERR_LANE         :1;      ///<BIT [5] cache_ecc_1err_lane
        uint32_t IRAM_ECC_1ERR_LANE          :1;      ///<BIT [6] iram_ecc_1err_lane
        uint32_t XDATA_ECC_2ERR_LANE         :1;      ///<BIT [7] xdata_ecc_2err_lane
        uint32_t CACHE_ECC_2ERR_LANE         :1;      ///<BIT [8] cache_ecc_2err_lane
        uint32_t IRAM_ECC_2ERR_LANE          :1;      ///<BIT [9] iram_ecc_2err_lane
        uint32_t XDATA_ECC_1ERR_ENABLE_LANE  :1;      ///<BIT [10] xdata_ecc_1err_enable_lane
        uint32_t CACHE_ECC_1ERR_ENABLE_LANE  :1;      ///<BIT [11] cache_ecc_1err_enable_lane
        uint32_t IRAM_ECC_1ERR_ENABLE_LANE   :1;      ///<BIT [12] iram_ecc_1err_enable_lane
        uint32_t XDATA_ECC_2ERR_ENABLE_LANE  :1;      ///<BIT [13] xdata_ecc_2err_enable_lane
        uint32_t CACHE_ECC_2ERR_ENABLE_LANE  :1;      ///<BIT [14] cache_ecc_2err_enable_lane
        uint32_t IRAM_ECC_2ERR_ENABLE_LANE   :1;      ///<BIT [15] iram_ecc_2err_enable_lane
        uint32_t XDATA_ECC_1ERR_CLEAR_LANE   :1;      ///<BIT [16] xdata_ecc_1err_clear_lane
        uint32_t CACHE_ECC_1ERR_CLEAR_LANE   :1;      ///<BIT [17] cache_ecc_1err_clear_lane
        uint32_t IRAM_ECC_1ERR_CLEAR_LANE    :1;      ///<BIT [18] iram_ecc_1err_clear_lane
        uint32_t XDATA_ECC_2ERR_CLEAR_LANE   :1;      ///<BIT [19] xdata_ecc_2err_clear_lane
        uint32_t CACHE_ECC_2ERR_CLEAR_LANE   :1;      ///<BIT [20] cache_ecc_2err_clear_lane
        uint32_t IRAM_ECC_2ERR_CLEAR_LANE    :1;      ///<BIT [21] iram_ecc_2err_clear_lane
        uint32_t XDATA_ECC_1ERR_SET_LANE     :1;      ///<BIT [22] xdata_ecc_1err_set_lane
        uint32_t CACHE_ECC_1ERR_SET_LANE     :1;      ///<BIT [23] cache_ecc_1err_set_lane
        uint32_t IRAM_ECC_1ERR_SET_LANE      :1;      ///<BIT [24] iram_ecc_1err_set_lane
        uint32_t XDATA_ECC_2ERR_SET_LANE     :1;      ///<BIT [25] xdata_ecc_2err_set_lane
        uint32_t CACHE_ECC_2ERR_SET_LANE     :1;      ///<BIT [26] cache_ecc_2err_set_lane
        uint32_t IRAM_ECC_2ERR_SET_LANE      :1;      ///<BIT [27] iram_ecc_2err_set_lane
        uint32_t XDATA_MEM_CHECKSUM_RESET_LANE :1;      ///<BIT [28] xdata_mem_checksum_reset_lane
        uint32_t XDATA_MEM_CHECKSUM_PASS_LANE :1;      ///<BIT [29] xdata_mem_checksum_pass_lane
        uint32_t RSVD_30_31                  :2;      ///<BIT [31:30] rsvd_30_31
    } b;
} Comphy0SocmcuMemReg2Lane_t;

/// @brief 0x22E4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PHY_MCU_REMOTE_REQ_LANE     :1;      ///<BIT [0] phy_mcu_remote_req_lane
        uint32_t PHY_MCU_REMOTE_ACK_LANE     :1;      ///<BIT [1] phy_mcu_remote_ack_lane
        uint32_t RSVD_2_31                   :30;     ///<BIT [31:2] rsvd_2_31
    } b;
} Comphy0SocmcuCommand0_t;

/// @brief 0x22F4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t XDATA_ECC_ERR_ADDR_LANE_9_0 :10;     ///<BIT [9:0] xdata_ecc_err_addr_lane_9_0
        uint32_t IRAM_ECC_ERR_ADDR_LANE_7_0  :8;      ///<BIT [17:10] iram_ecc_err_addr_lane_7_0
        uint32_t CACHE_ECC_ERR_ADDR_LANE_7_0 :8;      ///<BIT [25:18] cache_ecc_err_addr_lane_7_0
        uint32_t RSVD_26_31                  :6;      ///<BIT [31:26] rsvd_26_31
    } b;
} Comphy0SocmemEccErrAddress0_t;

/// @brief 0x2300
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PT_RST_LANE                 :1;      ///<BIT [0] pt_rst_lane
        uint32_t RSVD_1_3                    :3;      ///<BIT [3:1] rsvd_1_3
        uint32_t TX_TRAIN_PAT_SEL_LANE_1_0   :2;      ///<BIT [5:4] tx_train_pat_sel_lane_1_0
        uint32_t PT_START_RD_LANE            :1;      ///<BIT [6] pt_start_rd_lane
        uint32_t PT_CNT_RST_LANE             :1;      ///<BIT [7] pt_cnt_rst_lane
        uint32_t PT_LOCK_CNT_LANE_7_0        :8;      ///<BIT [15:8] pt_lock_cnt_lane_7_0
        uint32_t PT_RX_PATTERN_SEL_LANE_5_0  :6;      ///<BIT [21:16] pt_rx_pattern_sel_lane_5_0
        uint32_t PT_TX_PATTERN_SEL_LANE_5_0  :6;      ///<BIT [27:22] pt_tx_pattern_sel_lane_5_0
        uint32_t PT_PHYREADY_FORCE_LANE      :1;      ///<BIT [28] pt_phyready_force_lane
        uint32_t PT_EN_MODE_LANE_1_0         :2;      ///<BIT [30:29] pt_en_mode_lane_1_0
        uint32_t PT_EN_LANE                  :1;      ///<BIT [31] pt_en_lane
    } b;
} Comphy0SocptControl0_t;

/// @brief 0x2304
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_21                   :22;     ///<BIT [21:0] rsvd_0_21
        uint32_t PT_SATA_LONG_LANE           :1;      ///<BIT [22] pt_sata_long_lane
        uint32_t PT_PRBS_ENC_EN_LANE         :1;      ///<BIT [23] pt_prbs_enc_en_lane
        uint32_t RSVD_24_31                  :8;      ///<BIT [31:24] rsvd_24_31
    } b;
} Comphy0SocptControl1_t;

/// @brief 0x2310
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PT_LOCK_LANE                :1;      ///<BIT [0] pt_lock_lane
        uint32_t PT_PASS_LANE                :1;      ///<BIT [1] pt_pass_lane
        uint32_t RSVD_2_7                    :6;      ///<BIT [7:2] rsvd_2_7
        uint32_t PT_USER_K_CHAR_LANE_7_0     :8;      ///<BIT [15:8] pt_user_k_char_lane_7_0
        uint32_t PT_USER_PATTERN_LANE_15_0   :16;     ///<BIT [31:16] pt_user_pattern_lane_15_0
    } b;
} Comphy0SocptUserPattern2_t;

/// @brief 0x2318
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_15                   :16;     ///<BIT [15:0] rsvd_0_15
        uint32_t PT_CNT_LANE_15_0            :16;     ///<BIT [31:16] pt_cnt_lane_15_0
    } b;
} Comphy0SocptCounter1_t;

/// @brief 0x2408
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DFE_UPDATE_EN_LANE_15_0     :16;     ///<BIT [15:0] dfe_update_en_lane_15_0
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Comphy0SocdfeCtrlReg2_t;

/// @brief 0x2410
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_2                    :3;      ///<BIT [2:0] rsvd_0_2
        uint32_t DFE_UPDATE_DIS_LANE         :1;      ///<BIT [3] dfe_update_dis_lane
        uint32_t DFE_EN_LANE                 :1;      ///<BIT [4] dfe_en_lane
        uint32_t DFE_PAT_DIS_LANE            :1;      ///<BIT [5] dfe_pat_dis_lane
        uint32_t RSVD_6_31                   :26;     ///<BIT [31:6] rsvd_6_31
    } b;
} Comphy0SocrxEqClkCtrl_t;

/// @brief 0x2490
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DFE_F7_E_SM_LANE_4_0        :5;      ///<BIT [4:0] dfe_f7_e_sm_lane_4_0
        uint32_t RSVD_5_7                    :3;      ///<BIT [7:5] rsvd_5_7
        uint32_t DFE_F8_E_SM_LANE_4_0        :5;      ///<BIT [12:8] dfe_f8_e_sm_lane_4_0
        uint32_t RSVD_13_15                  :3;      ///<BIT [15:13] rsvd_13_15
        uint32_t DFE_F9_E_SM_LANE_4_0        :5;      ///<BIT [20:16] dfe_f9_e_sm_lane_4_0
        uint32_t RSVD_21_23                  :3;      ///<BIT [23:21] rsvd_21_23
        uint32_t DFE_F10_E_SM_LANE_4_0       :5;      ///<BIT [28:24] dfe_f10_e_sm_lane_4_0
        uint32_t RSVD_29_31                  :3;      ///<BIT [31:29] rsvd_29_31
    } b;
} Comphy0SocdfeReadEvenSmReg4_t;

/// @brief 0x24A0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DFE_F0_D_P_O_SM_LANE_5_0    :6;      ///<BIT [5:0] dfe_f0_d_p_o_sm_lane_5_0
        uint32_t RSVD_6_31                   :26;     ///<BIT [31:6] rsvd_6_31
    } b;
} Comphy0SocdfeReadOddSmReg0_t;

/// @brief 0x24A4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DFE_F2_D_P_O_SM_LANE_5_0    :6;      ///<BIT [5:0] dfe_f2_d_p_o_sm_lane_5_0
        uint32_t RSVD_6_31                   :26;     ///<BIT [31:6] rsvd_6_31
    } b;
} Comphy0SocdfeReadOddSmReg1_t;

/// @brief 0x24A8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DFE_F3_D_P_O_SM_LANE_4_0    :5;      ///<BIT [4:0] dfe_f3_d_p_o_sm_lane_4_0
        uint32_t RSVD_5_15                   :11;     ///<BIT [15:5] rsvd_5_15
        uint32_t DFE_F3_S_P_O_SM_LANE_4_0    :5;      ///<BIT [20:16] dfe_f3_s_p_o_sm_lane_4_0
        uint32_t RSVD_21_31                  :11;     ///<BIT [31:21] rsvd_21_31
    } b;
} Comphy0SocdfeReadOddSmReg2_t;

/// @brief 0x24AC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DFE_F1_O_SM_LANE_5_0        :6;      ///<BIT [5:0] dfe_f1_o_sm_lane_5_0
        uint32_t RSVD_6_7                    :2;      ///<BIT [7:6] rsvd_6_7
        uint32_t DFE_F4_O_SM_LANE_5_0        :6;      ///<BIT [13:8] dfe_f4_o_sm_lane_5_0
        uint32_t RSVD_14_15                  :2;      ///<BIT [15:14] rsvd_14_15
        uint32_t DFE_F5_O_SM_LANE_5_0        :6;      ///<BIT [21:16] dfe_f5_o_sm_lane_5_0
        uint32_t RSVD_22_23                  :2;      ///<BIT [23:22] rsvd_22_23
        uint32_t DFE_F6_O_SM_LANE_5_0        :6;      ///<BIT [29:24] dfe_f6_o_sm_lane_5_0
        uint32_t RSVD_30_31                  :2;      ///<BIT [31:30] rsvd_30_31
    } b;
} Comphy0SocdfeReadOddSmReg3_t;

/// @brief 0x24B0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DFE_F7_O_SM_LANE_4_0        :5;      ///<BIT [4:0] dfe_f7_o_sm_lane_4_0
        uint32_t RSVD_5_7                    :3;      ///<BIT [7:5] rsvd_5_7
        uint32_t DFE_F8_O_SM_LANE_4_0        :5;      ///<BIT [12:8] dfe_f8_o_sm_lane_4_0
        uint32_t RSVD_13_15                  :3;      ///<BIT [15:13] rsvd_13_15
        uint32_t DFE_F9_O_SM_LANE_4_0        :5;      ///<BIT [20:16] dfe_f9_o_sm_lane_4_0
        uint32_t RSVD_21_23                  :3;      ///<BIT [23:21] rsvd_21_23
        uint32_t DFE_F10_O_SM_LANE_4_0       :5;      ///<BIT [28:24] dfe_f10_o_sm_lane_4_0
        uint32_t RSVD_29_31                  :3;      ///<BIT [31:29] rsvd_29_31
    } b;
} Comphy0SocdfeReadOddSmReg4_t;

/// @brief 0x2540
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_28                   :29;     ///<BIT [28:0] rsvd_0_28
        uint32_t DFE_SQ_EN_LANE              :1;      ///<BIT [29] dfe_sq_en_lane
        uint32_t RSVD_30_31                  :2;      ///<BIT [31:30] rsvd_30_31
    } b;
} Comphy0SocdfeStaticLaneReg0_t;

/// @brief 0x2580
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t EOM_CNT_CLR_LANE            :1;      ///<BIT [2] eom_cnt_clr_lane
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} Comphy0SoceomReg0_t;

/// @brief 0x2600
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t LOCAL_ERR_FIELD_VALID_LANE  :1;      ///<BIT [2] local_err_field_valid_lane
        uint32_t LOCAL_TRAIN_COMP_VALID_LANE :1;      ///<BIT [3] local_train_comp_valid_lane
        uint32_t LOCAL_TX_INIT_VALID_LANE    :1;      ///<BIT [4] local_tx_init_valid_lane
        uint32_t LOCAL_FIELD_VALID_LANE      :1;      ///<BIT [5] local_field_valid_lane
        uint32_t LOCAL_ERROR_EN_LANE         :1;      ///<BIT [6] local_error_en_lane
        uint32_t LOCAL_BALANCE_CAL_EN_LANE   :1;      ///<BIT [7] local_balance_cal_en_lane
        uint32_t LOCAL_RD_REQ_LANE           :1;      ///<BIT [8] local_rd_req_lane
        uint32_t RSVD_9_15                   :7;      ///<BIT [15:9] rsvd_9_15
        uint32_t ETHERNET_MODE_LANE          :1;      ///<BIT [16] ethernet_mode_lane
        uint32_t DME_ENC_EN_LANE             :1;      ///<BIT [17] dme_enc_en_lane
        uint32_t RSVD_18_20                  :3;      ///<BIT [20:18] rsvd_18_20
        uint32_t LOCAL_CTRL_FIELD_FORCE_LANE :1;      ///<BIT [21] local_ctrl_field_force_lane
        uint32_t LOCAL_STATUS_FIELD_FORCE_LANE :1;      ///<BIT [22] local_status_field_force_lane
        uint32_t LOCAL_ERR_FIELD_FORCE_LANE  :1;      ///<BIT [23] local_err_field_force_lane
        uint32_t LOCAL_TRAIN_COMP_FORCE_LANE :1;      ///<BIT [24] local_train_comp_force_lane
        uint32_t LOCAL_TX_INIT_FORCE_LANE    :1;      ///<BIT [25] local_tx_init_force_lane
        uint32_t LOCAL_FIELD_FORCE_LANE      :1;      ///<BIT [26] local_field_force_lane
        uint32_t RSVD_27_31                  :5;      ///<BIT [31:27] rsvd_27_31
    } b;
} Comphy0SocdmeEncReg0_t;

/// @brief 0x2604
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LOCAL_STATUS_BITS_LANE_15_0 :16;     ///<BIT [15:0] local_status_bits_lane_15_0
        uint32_t LOCAL_CTRL_BITS_LANE_15_0   :16;     ///<BIT [31:16] local_ctrl_bits_lane_15_0
    } b;
} Comphy0SocdmeEncReg1_t;

/// @brief 0x2608
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LOCAL_STATUS_BITS_RD_LANE_15_0 :16;     ///<BIT [15:0] local_status_bits_rd_lane_15_0
        uint32_t LOCAL_CTRL_BITS_RD_LANE_15_0 :16;     ///<BIT [31:16] local_ctrl_bits_rd_lane_15_0
    } b;
} Comphy0SocdmeEncReg2_t;

/// @brief 0x260C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_22                   :23;     ///<BIT [22:0] rsvd_0_22
        uint32_t REMOTE_RD_REQ_LANE          :1;      ///<BIT [23] remote_rd_req_lane
        uint32_t RSVD_24_31                  :8;      ///<BIT [31:24] rsvd_24_31
    } b;
} Comphy0SocdmeDecReg0_t;

/// @brief 0x2610
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t REMOTE_STATUS_BITS_LANE_15_0 :16;     ///<BIT [15:0] remote_status_bits_lane_15_0
        uint32_t REMOTE_CTRL_BITS_LANE_15_0  :16;     ///<BIT [31:16] remote_ctrl_bits_lane_15_0
    } b;
} Comphy0SocdmeDecReg1_t;

/// @brief 0x2614
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_9                    :10;     ///<BIT [9:0] rsvd_0_9
        uint32_t TRX_TRAIN_TIMEOUT_EN_LANE   :1;      ///<BIT [10] trx_train_timeout_en_lane
        uint32_t TX_TRAIN_START_WAIT_TIME_LANE_1_0 :2;      ///<BIT [12:11] tx_train_start_wait_time_lane_1_0
        uint32_t RSVD_13_19                  :7;      ///<BIT [19:13] rsvd_13_19
        uint32_t FRAME_DET_MAX_TIME_LANE_3_0 :4;      ///<BIT [23:20] frame_det_max_time_lane_3_0
        uint32_t PATTERN_LOCK_LOST_TIMEOUT_EN_LANE :1;      ///<BIT [24] pattern_lock_lost_timeout_en_lane
        uint32_t TX_TRAIN_CHK_INIT_LANE      :1;      ///<BIT [25] tx_train_chk_init_lane
        uint32_t RSVD_26                     :1;      ///<BIT [26] rsvd_26
        uint32_t REMOTE_STATUS_RECHK_EN_LANE :1;      ///<BIT [27] remote_status_rechk_en_lane
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Comphy0SoctxTrainIfReg0_t;

/// @brief 0x2618
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_TRAIN_TIMER_LANE_15_0    :16;     ///<BIT [15:0] rx_train_timer_lane_15_0
        uint32_t TRX_TRAIN_TIMER_LANE_15_0   :16;     ///<BIT [31:16] trx_train_timer_lane_15_0
    } b;
} Comphy0SoctxTrainIfReg1_t;

/// @brief 0x261C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_6                    :7;      ///<BIT [6:0] rsvd_0_6
        uint32_t PIN_TX_TRAIN_ERROR_LANE_1_0 :2;      ///<BIT [8:7] pin_tx_train_error_lane_1_0
        uint32_t RSVD_9_13                   :5;      ///<BIT [13:9] rsvd_9_13
        uint32_t LOCAL_CTRL_FM_REG_EN_LANE   :1;      ///<BIT [14] local_ctrl_fm_reg_en_lane
        uint32_t RSVD_15_31                  :17;     ///<BIT [31:15] rsvd_15_31
    } b;
} Comphy0SoctxTrainIfReg2_t;

/// @brief 0x2620
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TRX_TRAIN_TIMEOUT_LANE      :1;      ///<BIT [0] trx_train_timeout_lane
        uint32_t TX_TRAIN_ERROR_RD_LANE_1_0  :2;      ///<BIT [2:1] tx_train_error_rd_lane_1_0
        uint32_t RX_TRAIN_FAILED_RD_LANE     :1;      ///<BIT [3] rx_train_failed_rd_lane
        uint32_t RX_TRAIN_COMPLETE_RD_LANE   :1;      ///<BIT [4] rx_train_complete_rd_lane
        uint32_t TX_TRAIN_FAILED_RD_LANE     :1;      ///<BIT [5] tx_train_failed_rd_lane
        uint32_t TX_TRAIN_COMPLETE_RD_LANE   :1;      ///<BIT [6] tx_train_complete_rd_lane
        uint32_t LOCAL_TRAIN_COMP_RD_LANE    :1;      ///<BIT [7] local_train_comp_rd_lane
        uint32_t REMOTE_TRAIN_COMP_RD_LANE   :1;      ///<BIT [8] remote_train_comp_rd_lane
        uint32_t RSVD_9_31                   :23;     ///<BIT [31:9] rsvd_9_31
    } b;
} Comphy0SoctxTrainIfReg3_t;

/// @brief 0x2624
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_8                    :9;      ///<BIT [8:0] rsvd_0_8
        uint32_t TX_TRAIN_PAT_TOGGLE_LANE    :1;      ///<BIT [9] tx_train_pat_toggle_lane
        uint32_t TX_TRAIN_PAT_MODE_LANE      :1;      ///<BIT [10] tx_train_pat_mode_lane
        uint32_t TX_TRAIN_PAT_TWO_ZERO_LANE  :1;      ///<BIT [11] tx_train_pat_two_zero_lane
        uint32_t RSVD_12_31                  :20;     ///<BIT [31:12] rsvd_12_31
    } b;
} Comphy0SoctxTrainPattternReg0_t;

/// @brief 0x2628
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_AMP_MAX_LANE_5_0         :6;      ///<BIT [5:0] tx_amp_max_lane_5_0
        uint32_t RSVD_6_7                    :2;      ///<BIT [7:6] rsvd_6_7
        uint32_t TX_AMP_MIN_LANE_5_0         :6;      ///<BIT [13:8] tx_amp_min_lane_5_0
        uint32_t RSVD_14_23                  :10;     ///<BIT [23:14] rsvd_14_23
        uint32_t TX_POWER_MAX_LANE_5_0       :6;      ///<BIT [29:24] tx_power_max_lane_5_0
        uint32_t TX_POWER_PROTECT_EN_LANE    :1;      ///<BIT [30] tx_power_protect_en_lane
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Comphy0SoctxTrainDriverReg0_t;

/// @brief 0x262C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_9                    :10;     ///<BIT [9:0] rsvd_0_9
        uint32_t TX_PEAK_PROTECT_EN_LANE     :1;      ///<BIT [10] tx_peak_protect_en_lane
        uint32_t TX_VMA_PROTECT_EN_LANE      :1;      ///<BIT [11] tx_vma_protect_en_lane
        uint32_t TX_VMA_MIN_LANE_3_0         :4;      ///<BIT [15:12] tx_vma_min_lane_3_0
        uint32_t TX_EMPH1_MAX_LANE_3_0       :4;      ///<BIT [19:16] tx_emph1_max_lane_3_0
        uint32_t TX_EMPH0_MAX_LANE_3_0       :4;      ///<BIT [23:20] tx_emph0_max_lane_3_0
        uint32_t TX_EMPH0_MIN_LANE_3_0       :4;      ///<BIT [27:24] tx_emph0_min_lane_3_0
        uint32_t TX_EMPH1_MIN_LANE_3_0       :4;      ///<BIT [31:28] tx_emph1_min_lane_3_0
    } b;
} Comphy0SoctxTrainDriverReg1_t;

/// @brief 0x2630
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t TX_COE_FM_PIPE_LANE         :1;      ///<BIT [2] tx_coe_fm_pipe_lane
        uint32_t RSVD_3                      :1;      ///<BIT [3] rsvd_3
        uint32_t LOCAL_TX_PRESET_INDEX_LANE_3_0 :4;      ///<BIT [7:4] local_tx_preset_index_lane_3_0
        uint32_t RSVD_8_15                   :8;      ///<BIT [15:8] rsvd_8_15
        uint32_t FM_TRAIN_TX_EMPH1_LANE_3_0  :4;      ///<BIT [19:16] fm_train_tx_emph1_lane_3_0
        uint32_t FM_TRAIN_TX_EMPH0_LANE_3_0  :4;      ///<BIT [23:20] fm_train_tx_emph0_lane_3_0
        uint32_t FM_TRAIN_TX_AMP_LANE_5_0    :6;      ///<BIT [29:24] fm_train_tx_amp_lane_5_0
        uint32_t RSVD_30_31                  :2;      ///<BIT [31:30] rsvd_30_31
    } b;
} Comphy0SoctxTrainDriverReg2_t;

/// @brief 0x264C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_9                    :10;     ///<BIT [9:0] rsvd_0_9
        uint32_t PCIE_GEN12_SEL_LANE         :1;      ///<BIT [10] pcie_gen12_sel_lane
        uint32_t RSVD_11_31                  :21;     ///<BIT [31:11] rsvd_11_31
    } b;
} Comphy0SoctxEmphCtrlReg0_t;

/// @brief 0x2650
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_3                    :4;      ///<BIT [3:0] rsvd_0_3
        uint32_t TX_EM_POST_CTRL_LANE_3_0    :4;      ///<BIT [7:4] tx_em_post_ctrl_lane_3_0
        uint32_t TX_EM_PRE_CTRL_LANE_3_0     :4;      ///<BIT [11:8] tx_em_pre_ctrl_lane_3_0
        uint32_t RSVD_12_30                  :19;     ///<BIT [30:12] rsvd_12_30
        uint32_t TX_EM_CTRL_REG_EN_LANE      :1;      ///<BIT [31] tx_em_ctrl_reg_en_lane
    } b;
} Comphy0SoclinkTrainMode0_t;

/// @brief 0x265C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_12                   :13;     ///<BIT [12:0] rsvd_0_12
        uint32_t STATUS_DET_TIMEOUT_ISR_LANE :1;      ///<BIT [13] status_det_timeout_isr_lane
        uint32_t TRX_TRAIN_TIMEOUT_ISR_LANE  :1;      ///<BIT [14] trx_train_timeout_isr_lane
        uint32_t FRAME_DET_TIMEOUT_ISR_LANE  :1;      ///<BIT [15] frame_det_timeout_isr_lane
        uint32_t REMOTE_BALANCE_ERR_ISR_LANE :1;      ///<BIT [16] remote_balance_err_isr_lane
        uint32_t DME_DEC_ERROR_ISR_LANE      :1;      ///<BIT [17] dme_dec_error_isr_lane
        uint32_t RSVD_18_20                  :3;      ///<BIT [20:18] rsvd_18_20
        uint32_t REMOTE_ERROR_VALID_ISR_LANE :1;      ///<BIT [21] remote_error_valid_isr_lane
        uint32_t REMOTE_TX_INIT_ISR_LANE     :1;      ///<BIT [22] remote_tx_init_isr_lane
        uint32_t REMOTE_TRAIN_COMP_ISR_LANE  :1;      ///<BIT [23] remote_train_comp_isr_lane
        uint32_t LOCAL_TX_INIT_ISR_LANE      :1;      ///<BIT [24] local_tx_init_isr_lane
        uint32_t LOCAL_TRAIN_COMP_ISR_LANE   :1;      ///<BIT [25] local_train_comp_isr_lane
        uint32_t LOCAL_ERROR_VALID_ISR_LANE  :1;      ///<BIT [26] local_error_valid_isr_lane
        uint32_t LOCAL_STATUS_VALID_ISR_LANE :1;      ///<BIT [27] local_status_valid_isr_lane
        uint32_t LOCAL_CTRL_VALID_ISR_LANE   :1;      ///<BIT [28] local_ctrl_valid_isr_lane
        uint32_t LOCAL_FIELD_DONE_ISR_LANE   :1;      ///<BIT [29] local_field_done_isr_lane
        uint32_t TX_TRAIN_COMPLETE_ISR_LANE  :1;      ///<BIT [30] tx_train_complete_isr_lane
        uint32_t RX_TRAIN_COMPLETE_ISR_LANE  :1;      ///<BIT [31] rx_train_complete_isr_lane
    } b;
} Comphy0SoctrxTrainIfIntrLane_t;

/// @brief 0x2660
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_12                   :13;     ///<BIT [12:0] rsvd_0_12
        uint32_t STATUS_DET_TIMEOUT_MASK_LANE :1;      ///<BIT [13] status_det_timeout_mask_lane
        uint32_t TRX_TRAIN_TIMEOUT_MASK_LANE :1;      ///<BIT [14] trx_train_timeout_mask_lane
        uint32_t FRAME_DET_TIMEOUT_MASK_LANE :1;      ///<BIT [15] frame_det_timeout_mask_lane
        uint32_t REMOTE_BALANCE_ERR_MASK_LANE :1;      ///<BIT [16] remote_balance_err_mask_lane
        uint32_t DME_DEC_ERROR_MASK_LANE     :1;      ///<BIT [17] dme_dec_error_mask_lane
        uint32_t RSVD_18_20                  :3;      ///<BIT [20:18] rsvd_18_20
        uint32_t REMOTE_ERROR_VALID_MASK_LANE :1;      ///<BIT [21] remote_error_valid_mask_lane
        uint32_t REMOTE_TX_INIT_MASK_LANE    :1;      ///<BIT [22] remote_tx_init_mask_lane
        uint32_t REMOTE_TRAIN_COMP_MASK_LANE :1;      ///<BIT [23] remote_train_comp_mask_lane
        uint32_t LOCAL_TX_INIT_MASK_LANE     :1;      ///<BIT [24] local_tx_init_mask_lane
        uint32_t LOCAL_TRAIN_COMP_MASK_LANE  :1;      ///<BIT [25] local_train_comp_mask_lane
        uint32_t LOCAL_ERROR_VALID_MASK_LANE :1;      ///<BIT [26] local_error_valid_mask_lane
        uint32_t LOCAL_STATUS_VALID_MASK_LANE :1;      ///<BIT [27] local_status_valid_mask_lane
        uint32_t LOCAL_CTRL_VALID_MASK_LANE  :1;      ///<BIT [28] local_ctrl_valid_mask_lane
        uint32_t LOCAL_FIELD_DONE_MASK_LANE  :1;      ///<BIT [29] local_field_done_mask_lane
        uint32_t TX_TRAIN_COMPLETE_MASK_LANE :1;      ///<BIT [30] tx_train_complete_mask_lane
        uint32_t RX_TRAIN_COMPLETE_MASK_LANE :1;      ///<BIT [31] rx_train_complete_mask_lane
    } b;
} Comphy0SoctrxTrainIfIntrMask0Lane_t;

/// @brief 0x2664
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_12                   :13;     ///<BIT [12:0] rsvd_0_12
        uint32_t STATUS_DET_TIMEOUT_ISR_CLEAR_LANE :1;      ///<BIT [13] status_det_timeout_isr_clear_lane
        uint32_t TRX_TRAIN_TIMEOUT_ISR_CLEAR_LANE :1;      ///<BIT [14] trx_train_timeout_isr_clear_lane
        uint32_t FRAME_DET_TIMEOUT_ISR_CLEAR_LANE :1;      ///<BIT [15] frame_det_timeout_isr_clear_lane
        uint32_t REMOTE_BALANCE_ERR_ISR_CLEAR_LANE :1;      ///<BIT [16] remote_balance_err_isr_clear_lane
        uint32_t RSVD_17_20                  :4;      ///<BIT [20:17] rsvd_17_20
        uint32_t REMOTE_ERROR_VALID_ISR_CLEAR_LANE :1;      ///<BIT [21] remote_error_valid_isr_clear_lane
        uint32_t REMOTE_TX_INIT_ISR_CLEAR_LANE :1;      ///<BIT [22] remote_tx_init_isr_clear_lane
        uint32_t REMOTE_TRAIN_COMP_ISR_CLEAR_LANE :1;      ///<BIT [23] remote_train_comp_isr_clear_lane
        uint32_t LOCAL_TX_INIT_ISR_CLEAR_LANE :1;      ///<BIT [24] local_tx_init_isr_clear_lane
        uint32_t LOCAL_TRAIN_COMP_ISR_CLEAR_LANE :1;      ///<BIT [25] local_train_comp_isr_clear_lane
        uint32_t LOCAL_ERROR_VALID_ISR_CLEAR_LANE :1;      ///<BIT [26] local_error_valid_isr_clear_lane
        uint32_t LOCAL_STATUS_VALID_ISR_CLEAR_LANE :1;      ///<BIT [27] local_status_valid_isr_clear_lane
        uint32_t LOCAL_CTRL_VALID_ISR_CLEAR_LANE :1;      ///<BIT [28] local_ctrl_valid_isr_clear_lane
        uint32_t LOCAL_FIELD_DONE_ISR_CLEAR_LANE :1;      ///<BIT [29] local_field_done_isr_clear_lane
        uint32_t TX_TRAIN_COMPLETE_ISR_CLEAR_LANE :1;      ///<BIT [30] tx_train_complete_isr_clear_lane
        uint32_t RX_TRAIN_COMPLETE_ISR_CLEAR_LANE :1;      ///<BIT [31] rx_train_complete_isr_clear_lane
    } b;
} Comphy0SoctrxTrainIfIntrClearLane_t;

/// @brief 0x2678
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_10                   :11;     ///<BIT [10:0] rsvd_0_10
        uint32_t TX_TRAIN_MAX_TIMER_FRAME_LOCK_LANE :1;      ///<BIT [11] tx_train_max_timer_frame_lock_lane
        uint32_t RSVD_12_15                  :4;      ///<BIT [15:12] rsvd_12_15
        uint32_t PIN_TX_TRAIN_ENABLE_SEL_LANE :1;      ///<BIT [16] pin_tx_train_enable_sel_lane
        uint32_t RSVD_17_28                  :12;     ///<BIT [28:17] rsvd_17_28
        uint32_t PIN_TRAIN_COMPLETE_TYPE_LANE :1;      ///<BIT [29] pin_train_complete_type_lane
        uint32_t RSVD_30                     :1;      ///<BIT [30] rsvd_30
        uint32_t LINK_TRAIN_MODE_LANE        :1;      ///<BIT [31] link_train_mode_lane
    } b;
} Comphy0SoctxTrainCtrlLane_t;

/// @brief 0x267C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_23                   :24;     ///<BIT [23:0] rsvd_0_23
        uint32_t TX_PEAK_MAX_LANE_3_0        :4;      ///<BIT [27:24] tx_peak_max_lane_3_0
        uint32_t TX_PEAK_MIN_LANE_3_0        :4;      ///<BIT [31:28] tx_peak_min_lane_3_0
    } b;
} Comphy0SoctxTrainIfReg8_t;

/// @brief 0x4000
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PRD_TXDEEMPH0_LANE          :1;      ///<BIT [0] prd_txdeemph0_lane
        uint32_t PRD_TXDEEMPH1_LANE          :1;      ///<BIT [1] prd_txdeemph1_lane
        uint32_t PRD_TXMARGIN_LANE_2_0       :3;      ///<BIT [4:2] prd_txmargin_lane_2_0
        uint32_t PRD_TXSWING_LANE            :1;      ///<BIT [5] prd_txswing_lane
        uint32_t CFG_TX_ALIGN_POS_LANE_5_0   :6;      ///<BIT [11:6] cfg_tx_align_pos_lane_5_0
        uint32_t RSVD_12                     :1;      ///<BIT [12] rsvd_12
        uint32_t CFG_FAST_SYNCH_LANE         :1;      ///<BIT [13] cfg_fast_synch_lane
        uint32_t CFG_FORCE_RXPRESENT_LANE_1_0 :2;      ///<BIT [15:14] cfg_force_rxpresent_lane_1_0
        uint32_t CFG_TXELECIDLE_MODE_LANE    :1;      ///<BIT [16] cfg_txelecidle_mode_lane
        uint32_t CFG_GEN1_TXDATA_DLY_LANE_1_0 :2;      ///<BIT [18:17] cfg_gen1_txdata_dly_lane_1_0
        uint32_t CFG_GEN2_TXDATA_DLY_LANE_1_0 :2;      ///<BIT [20:19] cfg_gen2_txdata_dly_lane_1_0
        uint32_t CFG_ALIGN_IDLE_HIZ_LANE     :1;      ///<BIT [21] cfg_align_idle_hiz_lane
        uint32_t CFG_TXDETRX_MODE_LANE       :1;      ///<BIT [22] cfg_txdetrx_mode_lane
        uint32_t CFG_DISABLE_TXDETVAL_LANE   :1;      ///<BIT [23] cfg_disable_txdetval_lane
        uint32_t CFG_SPD_CHANGE_WAIT_LANE    :1;      ///<BIT [24] cfg_spd_change_wait_lane
        uint32_t CFG_USE_MAX_PLL_RATE_LANE   :1;      ///<BIT [25] cfg_use_max_pll_rate_lane
        uint32_t CFG_USE_GEN2_PLL_CAL_LANE   :1;      ///<BIT [26] cfg_use_gen2_pll_cal_lane
        uint32_t CFG_USE_GEN3_PLL_CAL_LANE   :1;      ///<BIT [27] cfg_use_gen3_pll_cal_lane
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Comphy0SoclaneCfg0_t;

/// @brief 0x4004
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PM_RESET_LANE               :1;      ///<BIT [0] pm_reset_lane
        uint32_t PM_PU_PLL_LANE              :1;      ///<BIT [1] pm_pu_pll_lane
        uint32_t PM_PU_TX_LANE               :1;      ///<BIT [2] pm_pu_tx_lane
        uint32_t PM_PU_RX_LANE               :1;      ///<BIT [3] pm_pu_rx_lane
        uint32_t PM_TX_RATE_SEL_LANE_2_0     :3;      ///<BIT [6:4] pm_tx_rate_sel_lane_2_0
        uint32_t PM_RX_RATE_SEL_LANE_2_0     :3;      ///<BIT [9:7] pm_rx_rate_sel_lane_2_0
        uint32_t PM_RX_INIT_LANE             :1;      ///<BIT [10] pm_rx_init_lane
        uint32_t PM_TX_IDLE_LOZ_LANE         :1;      ///<BIT [11] pm_tx_idle_loz_lane
        uint32_t PM_TX_IDLE_HIZ_LANE         :1;      ///<BIT [12] pm_tx_idle_hiz_lane
        uint32_t PM_TXDETECTRX_EN_LANE       :1;      ///<BIT [13] pm_txdetectrx_en_lane
        uint32_t PM_PU_IVREF_LANE            :1;      ///<BIT [14] pm_pu_ivref_lane
        uint32_t PM_BEACON_TX_EN_LANE        :1;      ///<BIT [15] pm_beacon_tx_en_lane
        uint32_t PM_BEACON_RX_EN_LANE        :1;      ///<BIT [16] pm_beacon_rx_en_lane
        uint32_t PM_TX_VCMHOLD_EN_LANE       :1;      ///<BIT [17] pm_tx_vcmhold_en_lane
        uint32_t PM_TXDCLK_PCLK_EN_LANE      :1;      ///<BIT [18] pm_txdclk_pclk_en_lane
        uint32_t PM_PCLK_DPCLK_EN_LANE       :1;      ///<BIT [19] pm_pclk_dpclk_en_lane
        uint32_t PM_OSCCLK_PCLK_EN_LANE      :1;      ///<BIT [20] pm_oscclk_pclk_en_lane
        uint32_t PM_OSCCLK_AUX_CLK_EN_LANE   :1;      ///<BIT [21] pm_oscclk_aux_clk_en_lane
        uint32_t PM_DP_RST_N_LANE            :1;      ///<BIT [22] pm_dp_rst_n_lane
        uint32_t PM_ASYNC_RST_N_LANE         :1;      ///<BIT [23] pm_async_rst_n_lane
        uint32_t PM_PIPE_64B_LANE            :1;      ///<BIT [24] pm_pipe_64b_lane
        uint32_t PM_CLK_REQ_N_LANE           :1;      ///<BIT [25] pm_clk_req_n_lane
        uint32_t PM_STATE_LANE_5_0           :6;      ///<BIT [31:26] pm_state_lane_5_0
    } b;
} Comphy0SoclaneStatus0_t;

/// @brief 0x4008
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ANA_DPHY_RXPRESENT_LANE     :1;      ///<BIT [0] ana_dphy_rxpresent_lane
        uint32_t ANA_DPHY_SQ_DETECTED_LANE   :1;      ///<BIT [1] ana_dphy_sq_detected_lane
        uint32_t ANA_DPHY_RX_INIT_DONE_LANE  :1;      ///<BIT [2] ana_dphy_rx_init_done_lane
        uint32_t ANA_DPHY_TXDETRX_VALID_LANE :1;      ///<BIT [3] ana_dphy_txdetrx_valid_lane
        uint32_t ANA_DPHY_PLL_READY_RX_LANE  :1;      ///<BIT [4] ana_dphy_pll_ready_rx_lane
        uint32_t MAC_PHY_RX_TERMINATION_RD_LANE :1;      ///<BIT [5] mac_phy_rx_termination_rd_lane
        uint32_t ANA_DPHY_PLL_READY_TX_LANE  :1;      ///<BIT [6] ana_dphy_pll_ready_tx_lane
        uint32_t PHY_MAC_RXELECIDLE_LANE     :1;      ///<BIT [7] phy_mac_rxelecidle_lane
        uint32_t PHY_MAC_RXVALID_LANE        :1;      ///<BIT [8] phy_mac_rxvalid_lane
        uint32_t MAC_PHY_RATE_RD_LANE_2_0    :3;      ///<BIT [11:9] mac_phy_rate_rd_lane_2_0
        uint32_t MAC_PHY_POWERDOWN_RD_LANE_1_0 :2;      ///<BIT [13:12] mac_phy_powerdown_rd_lane_1_0
        uint32_t MAC_PHY_TXELECIDLE_RD_LANE  :1;      ///<BIT [14] mac_phy_txelecidle_rd_lane
        uint32_t MAC_PHY_TXDETECTRX_LOOPBACK_RD_LANE :1;      ///<BIT [15] mac_phy_txdetectrx_loopback_rd_lane
        uint32_t CFG_BEACON_TX_EN_LANE       :1;      ///<BIT [16] cfg_beacon_tx_en_lane
        uint32_t CFG_BEACON_RX_EN_LANE       :1;      ///<BIT [17] cfg_beacon_rx_en_lane
        uint32_t CFG_BEACON_TXLOZ_WAIT_LANE_3_0 :4;      ///<BIT [21:18] cfg_beacon_txloz_wait_lane_3_0
        uint32_t CFG_BEACON_MODE_LANE        :1;      ///<BIT [22] cfg_beacon_mode_lane
        uint32_t CFG_IVREF_MODE_LANE         :1;      ///<BIT [23] cfg_ivref_mode_lane
        uint32_t CFG_RXEIDETECT_DLY_LANE_5_0 :6;      ///<BIT [29:24] cfg_rxeidetect_dly_lane_5_0
        uint32_t CFG_POWER_SETTLE_WAIT_LANE  :1;      ///<BIT [30] cfg_power_settle_wait_lane
        uint32_t BEACON_DETECTED_LANE        :1;      ///<BIT [31] beacon_detected_lane
    } b;
} Comphy0SoclaneCfgStatus2Lane_t;

/// @brief 0x400C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_USE_FTS_LOCK_LANE       :1;      ///<BIT [0] cfg_use_fts_lock_lane
        uint32_t RSVD_1_4                    :4;      ///<BIT [4:1] rsvd_1_4
        uint32_t CFG_GEN3_TXELECIDLE_DLY_LANE_1_0 :2;      ///<BIT [6:5] cfg_gen3_txelecidle_dly_lane_1_0
        uint32_t CFG_GEN3_TXDATA_DLY_LANE_1_0 :2;      ///<BIT [8:7] cfg_gen3_txdata_dly_lane_1_0
        uint32_t RSVD_9_10                   :2;      ///<BIT [10:9] rsvd_9_10
        uint32_t CFG_BLK_ALIGN_CTRL_LANE_1_0 :2;      ///<BIT [12:11] cfg_blk_align_ctrl_lane_1_0
        uint32_t CFG_BLK_ALIGN_CTRL_LANE_2   :1;      ///<BIT [13] cfg_blk_align_ctrl_lane_2
        uint32_t RSVD_14_15                  :2;      ///<BIT [15:14] rsvd_14_15
        uint32_t CFG_ELB_THRESHOLD_LANE_4_0  :5;      ///<BIT [20:16] cfg_elb_threshold_lane_4_0
        uint32_t RSVD_21_31                  :11;     ///<BIT [31:21] rsvd_21_31
    } b;
} Comphy0SoclaneCfg2Lane_t;

/// @brief 0x4010
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_DFE_CTRL_LANE_2_0       :3;      ///<BIT [2:0] cfg_dfe_ctrl_lane_2_0
        uint32_t CFG_DFE_EN_SEL_LANE         :1;      ///<BIT [3] cfg_dfe_en_sel_lane
        uint32_t CFG_DFE_PAT_SEL_LANE        :1;      ///<BIT [4] cfg_dfe_pat_sel_lane
        uint32_t CFG_DFE_UPDATE_SEL_LANE     :1;      ///<BIT [5] cfg_dfe_update_sel_lane
        uint32_t CFG_DFE_OVERRIDE_LANE       :1;      ///<BIT [6] cfg_dfe_override_lane
        uint32_t CFG_SSC_CTRL_LANE           :1;      ///<BIT [7] cfg_ssc_ctrl_lane
        uint32_t CFG_REF_FREF_SEL_LANE_4_0   :5;      ///<BIT [12:8] cfg_ref_fref_sel_lane_4_0
        uint32_t CFG_SRIS_CTRL_LANE          :1;      ///<BIT [13] cfg_sris_ctrl_lane
        uint32_t RSVD_14_15                  :2;      ///<BIT [15:14] rsvd_14_15
        uint32_t CFG_RX_INIT_SEL_LANE        :1;      ///<BIT [16] cfg_rx_init_sel_lane
        uint32_t CFG_SQ_DET_SEL_LANE         :1;      ///<BIT [17] cfg_sq_det_sel_lane
        uint32_t CFG_RX_EQ_CTRL_LANE         :1;      ///<BIT [18] cfg_rx_eq_ctrl_lane
        uint32_t CFG_RXEIDET_DG_EN_LANE      :1;      ///<BIT [19] cfg_rxeidet_dg_en_lane
        uint32_t CFG_RXEI_DG_WEIGHT_LANE     :1;      ///<BIT [20] cfg_rxei_dg_weight_lane
        uint32_t RSVD_21_31                  :11;     ///<BIT [31:21] rsvd_21_31
    } b;
} Comphy0SoclaneCfg4_t;

/// @brief 0x4014
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PM_STATUS_PCLK_LANE_8_0     :9;      ///<BIT [8:0] pm_status_pclk_lane_8_0
        uint32_t PM_RX_TRAIN_ENABLE_LANE     :1;      ///<BIT [9] pm_rx_train_enable_lane
        uint32_t PM_PU_SQ_LANE               :1;      ///<BIT [10] pm_pu_sq_lane
        uint32_t PM_REFCLK_DIS_LANE          :1;      ///<BIT [11] pm_refclk_dis_lane
        uint32_t ANA_REFCLK_DIS_ACK_LANE     :1;      ///<BIT [12] ana_refclk_dis_ack_lane
        uint32_t PM_REFCLK_VALID_LANE        :1;      ///<BIT [13] pm_refclk_valid_lane
        uint32_t PM_RX_HIZ_LANE              :1;      ///<BIT [14] pm_rx_hiz_lane
        uint32_t MAC_PHY_TXCOMPLIANCE_RD_LANE :1;      ///<BIT [15] mac_phy_txcompliance_rd_lane
        uint32_t CFG_TXCMN_DIS_DLY_LANE_5_0  :6;      ///<BIT [21:16] cfg_txcmn_dis_dly_lane_5_0
        uint32_t CFG_DELAY_TDR_PHYST_LANE    :1;      ///<BIT [22] cfg_delay_tdr_physt_lane
        uint32_t CFG_DELAY_P12_PHYST_LANE    :1;      ///<BIT [23] cfg_delay_p12_physt_lane
        uint32_t CFG_HIZ_CAL_WAIT_LANE_3_0   :4;      ///<BIT [27:24] cfg_hiz_cal_wait_lane_3_0
        uint32_t CFG_HIZ_CAL_TIMER_EN_LANE   :1;      ///<BIT [28] cfg_hiz_cal_timer_en_lane
        uint32_t CFG_P0S_IDLE_HIZ_DIS_LANE   :1;      ///<BIT [29] cfg_p0s_idle_hiz_dis_lane
        uint32_t CFG_P1_WAKEUP_LANE          :1;      ///<BIT [30] cfg_p1_wakeup_lane
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Comphy0SoclaneCfgStatus3Lane_t;

/// @brief 0x4018
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_7                    :8;      ///<BIT [7:0] rsvd_0_7
        uint32_t PHY_MAC_PHYSTATUS_LANE      :1;      ///<BIT [8] phy_mac_phystatus_lane
        uint32_t RSVD_9_15                   :7;      ///<BIT [15:9] rsvd_9_15
        uint32_t MODE_PIE8_IF_LANE           :1;      ///<BIT [16] mode_pie8_if_lane
        uint32_t RSVD_17_25                  :9;      ///<BIT [25:17] rsvd_17_25
        uint32_t MODE_PIE8_EQ_LANE           :1;      ///<BIT [26] mode_pie8_eq_lane
        uint32_t RSVD_27_31                  :5;      ///<BIT [31:27] rsvd_27_31
    } b;
} Comphy0SoclaneDpPie8Cfg0Lane_t;

/// @brief 0x4024
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_15                   :16;     ///<BIT [15:0] rsvd_0_15
        uint32_t CFG_EQ_FS_LANE_5_0          :6;      ///<BIT [21:16] cfg_eq_fs_lane_5_0
        uint32_t CFG_EQ_LF_LANE_5_0          :6;      ///<BIT [27:22] cfg_eq_lf_lane_5_0
        uint32_t CFG_PHY_RC_EP_LANE          :1;      ///<BIT [28] cfg_phy_rc_ep_lane
        uint32_t RSVD_29_31                  :3;      ///<BIT [31:29] rsvd_29_31
    } b;
} Comphy0SoclaneEqCfg0Lane_t;

/// @brief 0x4028
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_11                   :12;     ///<BIT [11:0] rsvd_0_11
        uint32_t CFG_UPDATE_POLARITY_LANE    :1;      ///<BIT [12] cfg_update_polarity_lane
        uint32_t RSVD_13_23                  :11;     ///<BIT [23:13] rsvd_13_23
        uint32_t CFG_TX_COEFF_OVERRIDE_LANE  :1;      ///<BIT [24] cfg_tx_coeff_override_lane
        uint32_t RSVD_25_29                  :5;      ///<BIT [29:25] rsvd_25_29
        uint32_t CFG_EQ_BUNDLE_DIS_LANE      :1;      ///<BIT [30] cfg_eq_bundle_dis_lane
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Comphy0SoclaneEqCfg1Lane_t;

/// @brief 0x4034
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_21                   :22;     ///<BIT [21:0] rsvd_0_21
        uint32_t CFG_CURSOR_PRESET11_LANE_5_0 :6;      ///<BIT [27:22] cfg_cursor_preset11_lane_5_0
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Comphy0SoclanePresetCfg4Lane_t;

/// @brief 0x404C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_15                   :16;     ///<BIT [15:0] rsvd_0_15
        uint32_t CFG_PRE_CURSOR_PRESET11_LANE_5_0 :6;      ///<BIT [21:16] cfg_pre_cursor_preset11_lane_5_0
        uint32_t CFG_POST_CURSOR_PRESET11_LANE_5_0 :6;      ///<BIT [27:22] cfg_post_cursor_preset11_lane_5_0
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Comphy0SoclanePresetCfg16Lane_t;

/// @brief 0x4050
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_11                   :12;     ///<BIT [11:0] rsvd_0_11
        uint32_t CFG_TX_MARGIN_EN_LANE       :1;      ///<BIT [12] cfg_tx_margin_en_lane
        uint32_t CFG_TX_SWING_EN_LANE        :1;      ///<BIT [13] cfg_tx_swing_en_lane
        uint32_t RSVD_14_30                  :17;     ///<BIT [30:14] rsvd_14_30
        uint32_t CFG_LINK_TRAIN_CTRL_LANE    :1;      ///<BIT [31] cfg_link_train_ctrl_lane
    } b;
} Comphy0SoclaneCoeffMax0Lane_t;

/// @brief 0x4054
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_7                    :8;      ///<BIT [7:0] rsvd_0_7
        uint32_t CFG_INVALID_REQ_SEL_LANE    :1;      ///<BIT [8] cfg_invalid_req_sel_lane
        uint32_t RSVD_9_31                   :23;     ///<BIT [31:9] rsvd_9_31
    } b;
} Comphy0SoclaneRemoteSetLane_t;

/// @brief 0x4058
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_EQ_16G_FS_LANE_5_0      :6;      ///<BIT [5:0] cfg_eq_16g_fs_lane_5_0
        uint32_t CFG_EQ_16G_LF_LANE_5_0      :6;      ///<BIT [11:6] cfg_eq_16g_lf_lane_5_0
        uint32_t CFG_PRESET_INDEX_SEL_LANE   :1;      ///<BIT [12] cfg_preset_index_sel_lane
        uint32_t RSVD_13_31                  :19;     ///<BIT [31:13] rsvd_13_31
    } b;
} Comphy0SoclaneEq16gCfg0Lane_t;

/// @brief 0x4080
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_EQ_32G_FS_LANE_5_0      :6;      ///<BIT [5:0] cfg_eq_32g_fs_lane_5_0
        uint32_t CFG_EQ_32G_LF_LANE_5_0      :6;      ///<BIT [11:6] cfg_eq_32g_lf_lane_5_0
        uint32_t CFG_32G_PRESET_INDEX_SEL_LANE :1;      ///<BIT [12] cfg_32g_preset_index_sel_lane
        uint32_t RSVD_13_31                  :19;     ///<BIT [31:13] rsvd_13_31
    } b;
} Comphy0SoclaneEq32gCfg0Lane_t;

/// @brief 0x4200
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SUB_REVISION_LANE_7_0       :8;      ///<BIT [7:0] sub_revision_lane_7_0
        uint32_t MAIN_REVISION_LANE_7_0      :8;      ///<BIT [15:8] main_revision_lane_7_0
        uint32_t PIPE_SFT_RESET_LANE         :1;      ///<BIT [16] pipe_sft_reset_lane
        uint32_t REG_RESET_LANE              :1;      ///<BIT [17] reg_reset_lane
        uint32_t MODE_MIXED_DW_DF_LANE       :1;      ///<BIT [18] mode_mixed_dw_df_lane
        uint32_t MODE_PIPE_WIDTH_32_LANE     :1;      ///<BIT [19] mode_pipe_width_32_lane
        uint32_t MODE_REFDIV_LANE_1_0        :2;      ///<BIT [21:20] mode_refdiv_lane_1_0
        uint32_t MODE_CORE_CLK_CTRL_LANE     :1;      ///<BIT [22] mode_core_clk_ctrl_lane
        uint32_t MODE_MULTICAST_LANE         :1;      ///<BIT [23] mode_multicast_lane
        uint32_t PHY_RESET_LANE              :1;      ///<BIT [24] phy_reset_lane
        uint32_t MODE_CORE_CLK_FREQ_SEL_LANE :1;      ///<BIT [25] mode_core_clk_freq_sel_lane
        uint32_t MODE_P3_OSC_PCLK_EN_LANE    :1;      ///<BIT [26] mode_p3_osc_pclk_en_lane
        uint32_t RSVD_27_31                  :5;      ///<BIT [31:27] rsvd_27_31
    } b;
} Comphy0SocglobRstClkCtrl_t;

/// @brief 0x4204
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MODE_BIST_LANE              :1;      ///<BIT [0] mode_bist_lane
        uint32_t MODE_PM_OVERRIDE_LANE       :1;      ///<BIT [1] mode_pm_override_lane
        uint32_t MODE_MARGIN_OVERRIDE_LANE   :1;      ///<BIT [2] mode_margin_override_lane
        uint32_t DBG_TESTBUS_SEL_LANE_3_0    :4;      ///<BIT [6:3] dbg_testbus_sel_lane_3_0
        uint32_t DBG_TESTBUS_SEL_LANE_4      :1;      ///<BIT [7] dbg_testbus_sel_lane_4
        uint32_t DBG_TESTBUS_SEL_LANE_5      :1;      ///<BIT [8] dbg_testbus_sel_lane_5
        uint32_t DBG_TESTBUS_SEL_LANE_6      :1;      ///<BIT [9] dbg_testbus_sel_lane_6
        uint32_t MODE_LB_SHALLOW_LANE        :1;      ///<BIT [10] mode_lb_shallow_lane
        uint32_t MODE_LB_DEEP_LANE           :1;      ///<BIT [11] mode_lb_deep_lane
        uint32_t MODE_LB_SERDES_LANE         :1;      ///<BIT [12] mode_lb_serdes_lane
        uint32_t MODE_RST_OVERRIDE_LANE      :1;      ///<BIT [13] mode_rst_override_lane
        uint32_t MODE_STATE_OVERRIDE_LANE    :1;      ///<BIT [14] mode_state_override_lane
        uint32_t RSVD_15                     :1;      ///<BIT [15] rsvd_15
        uint32_t MODE_CLK_SRC_LANE_3_0       :4;      ///<BIT [19:16] mode_clk_src_lane_3_0
        uint32_t BUNDLE_SAMPLE_CTRL_LANE     :1;      ///<BIT [20] bundle_sample_ctrl_lane
        uint32_t PLL_READY_DLY_LANE_2_0      :3;      ///<BIT [23:21] pll_ready_dly_lane_2_0
        uint32_t BUNDLE_PLL_RDY_LANE         :1;      ///<BIT [24] bundle_pll_rdy_lane
        uint32_t CFG_USE_ALIGN_CLK_LANE      :1;      ///<BIT [25] cfg_use_align_clk_lane
        uint32_t MODE_P2_OFF_LANE            :1;      ///<BIT [26] mode_p2_off_lane
        uint32_t CFG_FORCE_OCLK_EN_LANE      :1;      ///<BIT [27] cfg_force_oclk_en_lane
        uint32_t CFG_SLOW_LANE_ALIGN_LANE    :1;      ///<BIT [28] cfg_slow_lane_align_lane
        uint32_t CFG_USE_LANE_ALIGN_RDY_LANE :1;      ///<BIT [29] cfg_use_lane_align_rdy_lane
        uint32_t CFG_CLK_SRC_MASK_LANE       :1;      ///<BIT [30] cfg_clk_src_mask_lane
        uint32_t CFG_USE_ASYNC_CLKREQN_LANE  :1;      ///<BIT [31] cfg_use_async_clkreqn_lane
    } b;
} Comphy0SocglobClkSrcLo_t;

/// @brief 0x4208
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LANE_START_LANE             :1;      ///<BIT [0] lane_start_lane
        uint32_t LANE_BREAK_LANE             :1;      ///<BIT [1] lane_break_lane
        uint32_t LANE_MASTER_LANE            :1;      ///<BIT [2] lane_master_lane
        uint32_t BIFURCATION_SEL_LANE_1_0    :2;      ///<BIT [4:3] bifurcation_sel_lane_1_0
        uint32_t BUNDLE_PERIOD_SCALE_LANE_1_0 :2;      ///<BIT [6:5] bundle_period_scale_lane_1_0
        uint32_t MODE_PIPE4_IF_LANE          :1;      ///<BIT [7] mode_pipe4_if_lane
        uint32_t CFG_LANE_TURN_OFF_DIS_LANE  :1;      ///<BIT [8] cfg_lane_turn_off_dis_lane
        uint32_t CFG_OSC_WIN_LENGTH_LANE_1_0 :2;      ///<BIT [10:9] cfg_osc_win_length_lane_1_0
        uint32_t CFG_REFCLK_VALID_POL_LANE   :1;      ///<BIT [11] cfg_refclk_valid_pol_lane
        uint32_t BUNDLE_PERIOD_SEL_LANE      :1;      ///<BIT [12] bundle_period_sel_lane
        uint32_t RSVD_13                     :1;      ///<BIT [13] rsvd_13
        uint32_t CFG_RXTERM_ENABLE_LANE      :1;      ///<BIT [14] cfg_rxterm_enable_lane
        uint32_t CFG_SEL_20_BITS_LANE        :1;      ///<BIT [15] cfg_sel_20_bits_lane
        uint32_t PULSE_LENGTH_LANE_4_0       :5;      ///<BIT [20:16] pulse_length_lane_4_0
        uint32_t RSVD_21_23                  :3;      ///<BIT [23:21] rsvd_21_23
        uint32_t CFG_UPDATE_LANE             :1;      ///<BIT [24] cfg_update_lane
        uint32_t RSVD_25_27                  :3;      ///<BIT [27:25] rsvd_25_27
        uint32_t PMO_POWER_VALID_LANE        :1;      ///<BIT [28] pmo_power_valid_lane
        uint32_t RSVD_29_30                  :2;      ///<BIT [30:29] rsvd_29_30
        uint32_t PULSE_DONE_LANE             :1;      ///<BIT [31] pulse_done_lane
    } b;
} Comphy0SocglobClkSrcHi_t;

/// @brief 0x420C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MODE_P1_CLK_REQ_N_LANE      :1;      ///<BIT [0] mode_p1_clk_req_n_lane
        uint32_t MODE_P2_PHYSTATUS_LANE      :1;      ///<BIT [1] mode_p2_phystatus_lane
        uint32_t MODE_PCLK_CTRL_LANE         :1;      ///<BIT [2] mode_pclk_ctrl_lane
        uint32_t SQ_DETECT_SRC_LANE          :1;      ///<BIT [3] sq_detect_src_lane
        uint32_t SQ_DETECT_OVERRIDE_LANE     :1;      ///<BIT [4] sq_detect_override_lane
        uint32_t CFG_RX_HIZ_SRC_LANE         :1;      ///<BIT [5] cfg_rx_hiz_src_lane
        uint32_t MODE_P1_SNOOZ_LANE          :1;      ///<BIT [6] mode_p1_snooz_lane
        uint32_t MODE_P1_OFF_LANE            :1;      ///<BIT [7] mode_p1_off_lane
        uint32_t OSC_COUNT_SCALE_LANE_2_0    :3;      ///<BIT [10:8] osc_count_scale_lane_2_0
        uint32_t RCB_RXEN_SRC_LANE           :1;      ///<BIT [11] rcb_rxen_src_lane
        uint32_t CFG_FREE_OSC_SEL_LANE       :1;      ///<BIT [12] cfg_free_osc_sel_lane
        uint32_t MODE_REFCLK_DIS_LANE        :1;      ///<BIT [13] mode_refclk_dis_lane
        uint32_t CFG_REFCLK_DET_TYPE_LANE    :1;      ///<BIT [14] cfg_refclk_det_type_lane
        uint32_t CFG_CLK_ACK_TIMER_EN_LANE   :1;      ///<BIT [15] cfg_clk_ack_timer_en_lane
        uint32_t CLKREQ_N_SRC_LANE           :1;      ///<BIT [16] clkreq_n_src_lane
        uint32_t CLKREQ_N_OVERRIDE_LANE      :1;      ///<BIT [17] clkreq_n_override_lane
        uint32_t REFCLK_RESTORE_DLY_LANE_5_0 :6;      ///<BIT [23:18] refclk_restore_dly_lane_5_0
        uint32_t REFCLK_SHUTOFF_DLY_LANE_1_0 :2;      ///<BIT [25:24] refclk_shutoff_dly_lane_1_0
        uint32_t REFCLK_DISABLE_DLY_LANE_3_0 :4;      ///<BIT [29:26] refclk_disable_dly_lane_3_0
        uint32_t REFCLK_DISABLE_DLY_LANE_5_4 :2;      ///<BIT [31:30] refclk_disable_dly_lane_5_4
    } b;
} Comphy0SocglobMiscCtrl_t;

/// @brief 0x4210
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_IGNORE_PHY_RDY_LANE     :1;      ///<BIT [0] cfg_ignore_phy_rdy_lane
        uint32_t CFG_GEN1_TXELECIDLE_DLY_LANE_1_0 :2;      ///<BIT [2:1] cfg_gen1_txelecidle_dly_lane_1_0
        uint32_t CFG_PASS_RXINFO_LANE        :1;      ///<BIT [3] cfg_pass_rxinfo_lane
        uint32_t CFG_NO_DISPERROR_LANE       :1;      ///<BIT [4] cfg_no_disperror_lane
        uint32_t CFG_DISABLE_EDB_LANE        :1;      ///<BIT [5] cfg_disable_edb_lane
        uint32_t CFG_MASK_ERRORS_LANE        :1;      ///<BIT [6] cfg_mask_errors_lane
        uint32_t CFG_DISABLE_SKP_LANE        :1;      ///<BIT [7] cfg_disable_skp_lane
        uint32_t CFG_ALWAYS_ALIGN_LANE       :1;      ///<BIT [8] cfg_always_align_lane
        uint32_t CFG_SAL_FREEZE_LANE         :1;      ///<BIT [9] cfg_sal_freeze_lane
        uint32_t CFG_GEN2_TXELECIDLE_DLY_LANE_1_0 :2;      ///<BIT [11:10] cfg_gen2_txelecidle_dly_lane_1_0
        uint32_t CFG_TXELECIDLE_ASSERT_LANE  :1;      ///<BIT [12] cfg_txelecidle_assert_lane
        uint32_t CFG_SAL_IGNORE_SQ_LANE      :1;      ///<BIT [13] cfg_sal_ignore_sq_lane
        uint32_t RSVD_14_15                  :2;      ///<BIT [15:14] rsvd_14_15
        uint32_t CFG_SAL_LANE_4_0            :5;      ///<BIT [20:16] cfg_sal_lane_4_0
        uint32_t RSVD_21_23                  :3;      ///<BIT [23:21] rsvd_21_23
        uint32_t CFG_SAL_LANE_24_20          :5;      ///<BIT [28:24] cfg_sal_lane_24_20
        uint32_t RSVD_29_31                  :3;      ///<BIT [31:29] rsvd_29_31
    } b;
} Comphy0SocglobDpSalCfg_t;

/// @brief 0x4214
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_SAL_LANE_9_5            :5;      ///<BIT [4:0] cfg_sal_lane_9_5
        uint32_t RSVD_5_7                    :3;      ///<BIT [7:5] rsvd_5_7
        uint32_t CFG_SAL_LANE_29_25          :5;      ///<BIT [12:8] cfg_sal_lane_29_25
        uint32_t RSVD_13_15                  :3;      ///<BIT [15:13] rsvd_13_15
        uint32_t CFG_SAL_LANE_14_10          :5;      ///<BIT [20:16] cfg_sal_lane_14_10
        uint32_t RSVD_21_23                  :3;      ///<BIT [23:21] rsvd_21_23
        uint32_t CFG_SAL_LANE_34_30          :5;      ///<BIT [28:24] cfg_sal_lane_34_30
        uint32_t RSVD_29_31                  :3;      ///<BIT [31:29] rsvd_29_31
    } b;
} Comphy0SocglobDpSalCfg1_t;

/// @brief 0x4218
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_SAL_LANE_19_15          :5;      ///<BIT [4:0] cfg_sal_lane_19_15
        uint32_t RSVD_5_7                    :3;      ///<BIT [7:5] rsvd_5_7
        uint32_t CFG_SAL_LANE_39_35          :5;      ///<BIT [12:8] cfg_sal_lane_39_35
        uint32_t RSVD_13_15                  :3;      ///<BIT [15:13] rsvd_13_15
        uint32_t CFG_SAL_LANE_42_40          :3;      ///<BIT [18:16] cfg_sal_lane_42_40
        uint32_t RSVD_19_23                  :5;      ///<BIT [23:19] rsvd_19_23
        uint32_t CFG_SAL_LANE_45_43          :3;      ///<BIT [26:24] cfg_sal_lane_45_43
        uint32_t RSVD_27_31                  :5;      ///<BIT [31:27] rsvd_27_31
    } b;
} Comphy0SocglobDpSalCfg3_t;

/// @brief 0x421C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_16                   :17;     ///<BIT [16:0] rsvd_0_16
        uint32_t CFG_PIPE_MSG_BUS_PROTOCOL_SEL_LANE :1;      ///<BIT [17] cfg_pipe_msg_bus_protocol_sel_lane
        uint32_t RSVD_18_31                  :14;     ///<BIT [31:18] rsvd_18_31
    } b;
} Comphy0SocglobProtocolCfg0_t;

/// @brief 0x4220
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_PM_RXDLOZ_WAIT_LANE_7_0 :8;      ///<BIT [7:0] cfg_pm_rxdloz_wait_lane_7_0
        uint32_t CFG_PM_RXDEN_WAIT_LANE_3_0  :4;      ///<BIT [11:8] cfg_pm_rxden_wait_lane_3_0
        uint32_t CFG_PM_OSCCLK_WAIT_LANE_3_0 :4;      ///<BIT [15:12] cfg_pm_oscclk_wait_lane_3_0
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Comphy0SocglobPmCfg0_t;

/// @brief 0x4224
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_5                    :6;      ///<BIT [5:0] rsvd_0_5
        uint32_t COUNTER_SAMPLE_LANE         :1;      ///<BIT [6] counter_sample_lane
        uint32_t COUNTER_SAMPLE_CLEAR_LANE   :1;      ///<BIT [7] counter_sample_clear_lane
        uint32_t COUNTER_TYPE_LANE_5_0       :6;      ///<BIT [13:8] counter_type_lane_5_0
        uint32_t PMO_PU_SQ_LANE              :1;      ///<BIT [14] pmo_pu_sq_lane
        uint32_t PMO_REFCLK_DIS_LANE         :1;      ///<BIT [15] pmo_refclk_dis_lane
        uint32_t COUNTER_SAMPLED_LANE_15_0   :16;     ///<BIT [31:16] counter_sampled_lane_15_0
    } b;
} Comphy0SocglobCounterCtrl_t;

/// @brief 0x4228
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t COUNTER_SAMPLED_LANE_31_16  :16;     ///<BIT [15:0] counter_sampled_lane_31_16
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Comphy0SocglobCounterHi_t;

/// @brief 0x422C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_15                   :16;     ///<BIT [15:0] rsvd_0_15
        uint32_t LOW_FREQ_PERIOD_MIN_LANE_6_0 :7;      ///<BIT [22:16] low_freq_period_min_lane_6_0
        uint32_t LOW_FREQ_PERIOD_MAX_LANE_6_0 :7;      ///<BIT [29:23] low_freq_period_max_lane_6_0
        uint32_t LOW_FREQ_CNT_SCALE_LANE_1_0 :2;      ///<BIT [31:30] low_freq_cnt_scale_lane_1_0
    } b;
} Comphy0SocglobPmDpCtrl_t;

/// @brief 0x4230
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_BAL_WEIGHT_LANE_5_0     :6;      ///<BIT [5:0] cfg_bal_weight_lane_5_0
        uint32_t RSVD_6_7                    :2;      ///<BIT [7:6] rsvd_6_7
        uint32_t CFG_BAL_WEIGHT_LANE_29_24   :6;      ///<BIT [13:8] cfg_bal_weight_lane_29_24
        uint32_t RSVD_14_15                  :2;      ///<BIT [15:14] rsvd_14_15
        uint32_t CFG_BAL_WEIGHT_LANE_11_6    :6;      ///<BIT [21:16] cfg_bal_weight_lane_11_6
        uint32_t RSVD_22_23                  :2;      ///<BIT [23:22] rsvd_22_23
        uint32_t CFG_BAL_WEIGHT_LANE_35_30   :6;      ///<BIT [29:24] cfg_bal_weight_lane_35_30
        uint32_t RSVD_30_31                  :2;      ///<BIT [31:30] rsvd_30_31
    } b;
} Comphy0SocglobDpBalCfg0_t;

/// @brief 0x4234
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_BAL_WEIGHT_LANE_17_12   :6;      ///<BIT [5:0] cfg_bal_weight_lane_17_12
        uint32_t RSVD_6_7                    :2;      ///<BIT [7:6] rsvd_6_7
        uint32_t CFG_BAL_WEIGHT_LANE_41_36   :6;      ///<BIT [13:8] cfg_bal_weight_lane_41_36
        uint32_t RSVD_14_15                  :2;      ///<BIT [15:14] rsvd_14_15
        uint32_t CFG_BAL_WEIGHT_LANE_23_18   :6;      ///<BIT [21:16] cfg_bal_weight_lane_23_18
        uint32_t RSVD_22_23                  :2;      ///<BIT [23:22] rsvd_22_23
        uint32_t CFG_BAL_WEIGHT_LANE_47_42   :6;      ///<BIT [29:24] cfg_bal_weight_lane_47_42
        uint32_t RSVD_30_31                  :2;      ///<BIT [31:30] rsvd_30_31
    } b;
} Comphy0SocglobDpBalCfg2_t;

/// @brief 0x4238
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_BAL_WEIGHT_LANE_50_48   :3;      ///<BIT [2:0] cfg_bal_weight_lane_50_48
        uint32_t RSVD_3_7                    :5;      ///<BIT [7:3] rsvd_3_7
        uint32_t CFG_BAL_WEIGHT_LANE_53_51   :3;      ///<BIT [10:8] cfg_bal_weight_lane_53_51
        uint32_t RSVD_11_31                  :21;     ///<BIT [31:11] rsvd_11_31
    } b;
} Comphy0SocglobDpBalCfg4_t;

/// @brief 0x423C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BIST_POWERDOWN_LANE_1_0     :2;      ///<BIT [1:0] bist_powerdown_lane_1_0
        uint32_t BIST_RATE_LANE_2_0          :3;      ///<BIT [4:2] bist_rate_lane_2_0
        uint32_t BIST_TXDETECTRX_LOOPBACK_LANE :1;      ///<BIT [5] bist_txdetectrx_loopback_lane
        uint32_t BIST_TXELECIDLE_LANE        :1;      ///<BIT [6] bist_txelecidle_lane
        uint32_t BIST_TXCOMPLIANCE_LANE      :1;      ///<BIT [7] bist_txcompliance_lane
        uint32_t BIST_RXPOLARITY_LANE        :1;      ///<BIT [8] bist_rxpolarity_lane
        uint32_t BIST_RXEIDETECT_DIS_LANE    :1;      ///<BIT [9] bist_rxeidetect_dis_lane
        uint32_t BIST_TXCMN_MODE_DIS_LANE    :1;      ///<BIT [10] bist_txcmn_mode_dis_lane
        uint32_t BIST_CLK_REQ_N_LANE         :1;      ///<BIT [11] bist_clk_req_n_lane
        uint32_t BIST_TXDATAK_LANE_3_0       :4;      ///<BIT [15:12] bist_txdatak_lane_3_0
        uint32_t BIST_TX_ALIGN_POS_LANE_5_0  :6;      ///<BIT [21:16] bist_tx_align_pos_lane_5_0
        uint32_t BIST_ELB_THRESHOLD_LANE_3_0 :4;      ///<BIT [25:22] bist_elb_threshold_lane_3_0
        uint32_t BIST_RXEQTRAINING_LANE      :1;      ///<BIT [26] bist_rxeqtraining_lane
        uint32_t RSVD_27_29                  :3;      ///<BIT [29:27] rsvd_27_29
        uint32_t BIST_UPDATE_LANE            :1;      ///<BIT [30] bist_update_lane
        uint32_t BIST_START_LANE             :1;      ///<BIT [31] bist_start_lane
    } b;
} Comphy0SocglobBistCtrl_t;

/// @brief 0x4240
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BIST_TXDATA_LANE_15_0       :16;     ///<BIT [15:0] bist_txdata_lane_15_0
        uint32_t BIST_SELF_CHECK_LANE        :1;      ///<BIT [16] bist_self_check_lane
        uint32_t BIST_TYPE_LANE_1_0          :2;      ///<BIT [18:17] bist_type_lane_1_0
        uint32_t BIST_CONT_MONITR_LANE       :1;      ///<BIT [19] bist_cont_monitr_lane
        uint32_t RSVD_20_22                  :3;      ///<BIT [22:20] rsvd_20_22
        uint32_t BIST_PATTERN_SEL_LANE       :1;      ///<BIT [23] bist_pattern_sel_lane
        uint32_t BIST_SKPOS_SEL_LANE         :1;      ///<BIT [24] bist_skpos_sel_lane
        uint32_t BIST_SKPOS_NUM_LANE_2_0     :3;      ///<BIT [27:25] bist_skpos_num_lane_2_0
        uint32_t BIST_SKPOS_NUM_LANE_4_3     :2;      ///<BIT [29:28] bist_skpos_num_lane_4_3
        uint32_t BIST_PASS_LANE              :1;      ///<BIT [30] bist_pass_lane
        uint32_t BIST_DONE_LANE              :1;      ///<BIT [31] bist_done_lane
    } b;
} Comphy0SocglobBistLaneType_t;

/// @brief 0x4244
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BIST_WIN_DELAY_LANE_15_0    :16;     ///<BIT [15:0] bist_win_delay_lane_15_0
        uint32_t BIST_WIN_LENGTH_LANE_15_0   :16;     ///<BIT [31:16] bist_win_length_lane_15_0
    } b;
} Comphy0SocglobBistStart_t;

/// @brief 0x4248
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BIST_MASK_LANE_15_0         :16;     ///<BIT [15:0] bist_mask_lane_15_0
        uint32_t BIST_MASK_LANE_31_16        :16;     ///<BIT [31:16] bist_mask_lane_31_16
    } b;
} Comphy0SocglobBistMask_t;

/// @brief 0x424C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BIST_CRC32_RESULT_LANE_15_0 :16;     ///<BIT [15:0] bist_crc32_result_lane_15_0
        uint32_t BIST_CRC32_RESULT_LANE_31_16 :16;     ///<BIT [31:16] bist_crc32_result_lane_31_16
    } b;
} Comphy0SocglobBistResult_t;

/// @brief 0x4250
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BIST_SEQ_N_DATA_LANE_7_0    :8;      ///<BIT [7:0] bist_seq_n_data_lane_7_0
        uint32_t BIST_SEQ_N_FTS_LANE_7_0     :8;      ///<BIT [15:8] bist_seq_n_fts_lane_7_0
        uint32_t BIST_LFSR_SEED_LANE_15_0    :16;     ///<BIT [31:16] bist_lfsr_seed_lane_15_0
    } b;
} Comphy0SocglobBistSeqrCfg_t;

/// @brief 0x4254
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BIST_TXDATA_LANE_31_16      :16;     ///<BIT [15:0] bist_txdata_lane_31_16
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Comphy0SocglobBistDataHi_t;

/// @brief 0x4258
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BIST_PRESET_VECTOR_LANE_11_0 :12;     ///<BIT [11:0] bist_preset_vector_lane_11_0
        uint32_t BIST_EQ_FB_MODE_LANE        :1;      ///<BIT [12] bist_eq_fb_mode_lane
        uint32_t RSVD_13_15                  :3;      ///<BIT [15:13] rsvd_13_15
        uint32_t BIST_INIT_PRESET_LANE_3_0   :4;      ///<BIT [19:16] bist_init_preset_lane_3_0
        uint32_t BIST_EQ_SUCCESSFUL_LANE     :1;      ///<BIT [20] bist_eq_successful_lane
        uint32_t BIST_EQ_COMPLETE_LANE       :1;      ///<BIT [21] bist_eq_complete_lane
        uint32_t RSVD_22_31                  :10;     ///<BIT [31:22] rsvd_22_31
    } b;
} Comphy0SocglobBistLinkEq_t;

/// @brief 0x425C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BIST_MARGIN_PAYLOAD_LANE_7_0 :8;      ///<BIT [7:0] bist_margin_payload_lane_7_0
        uint32_t BIST_MARGIN_TYPE_LANE_2_0   :3;      ///<BIT [10:8] bist_margin_type_lane_2_0
        uint32_t RSVD_11_15                  :5;      ///<BIT [15:11] rsvd_11_15
        uint32_t MARGIN_PAYLOAD_STAT_LANE_7_0 :8;      ///<BIT [23:16] margin_payload_stat_lane_7_0
        uint32_t MARGIN_TYPE_STAT_LANE_2_0   :3;      ///<BIT [26:24] margin_type_stat_lane_2_0
        uint32_t RSVD_27_31                  :5;      ///<BIT [31:27] rsvd_27_31
    } b;
} Comphy0SocglobBistLaneMargin_t;

/// @brief 0x4260
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PIPE_REVISION_LANE_7_0      :8;      ///<BIT [7:0] pipe_revision_lane_7_0
        uint32_t RSVD_8_15                   :8;      ///<BIT [15:8] rsvd_8_15
        uint32_t DEBUG_BUS_OUT_LANE_15_0     :16;     ///<BIT [31:16] debug_bus_out_lane_15_0
    } b;
} Comphy0SocglobPipeRevision_t;

/// @brief 0x4264
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_PIPE43_P1CPM_ENC_LANE_3_0 :4;      ///<BIT [3:0] cfg_pipe43_p1cpm_enc_lane_3_0
        uint32_t CFG_PIPE43_P1_1_ENC_LANE_3_0 :4;      ///<BIT [7:4] cfg_pipe43_p1_1_enc_lane_3_0
        uint32_t CFG_PIPE43_P1_2_ENC_LANE_3_0 :4;      ///<BIT [11:8] cfg_pipe43_p1_2_enc_lane_3_0
        uint32_t MODE_PIPE4X_L1SUB_LANE      :1;      ///<BIT [12] mode_pipe4x_l1sub_lane
        uint32_t CFG_USE_SIDE_BAND_LANE      :1;      ///<BIT [13] cfg_use_side_band_lane
        uint32_t CFG_PIPE43_ASYNC_HS_BYPASS_LANE :1;      ///<BIT [14] cfg_pipe43_async_hs_bypass_lane
        uint32_t RSVD_15_31                  :17;     ///<BIT [31:15] rsvd_15_31
    } b;
} Comphy0SocglobL1SubstatesCfg_t;

/// @brief 0x5920
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_21                   :22;     ///<BIT [21:0] rsvd_0_21
        uint32_t MAC_PHY_TXCOMPLIANCE_LANE   :1;      ///<BIT [22] mac_phy_txcompliance_lane
        uint32_t RSVD_23_27                  :5;      ///<BIT [27:23] rsvd_23_27
        uint32_t MAC_PHY_RATE_LANE_2_0       :3;      ///<BIT [30:28] mac_phy_rate_lane_2_0
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Comphy0SocinputPinDebugPipeReg8_t;

/// @brief 0x5928
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MAC_PHY_TXDETECTRX_LOOPBACK_LANE :1;      ///<BIT [0] mac_phy_txdetectrx_loopback_lane
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} Comphy0SocinputPinDebugPipeReg10_t;

/// @brief 0x592C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_29                   :30;     ///<BIT [29:0] rsvd_0_29
        uint32_t MAC_PHY_TXELECIDLE_LANE     :1;      ///<BIT [30] mac_phy_txelecidle_lane
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Comphy0SocinputPinDebugPipeReg11_t;

/// @brief 0x5930
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_21                   :22;     ///<BIT [21:0] rsvd_0_21
        uint32_t MAC_PHY_RX_TERMINATION_LANE :1;      ///<BIT [22] mac_phy_rx_termination_lane
        uint32_t RSVD_23_31                  :9;      ///<BIT [31:23] rsvd_23_31
    } b;
} Comphy0SocinputPinDebugPipeReg12_t;

/// @brief 0x6000
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_19                   :20;     ///<BIT [19:0] rsvd_0_19
        uint32_t TXDCLK_NT_SEL_LANE_1_0      :2;      ///<BIT [21:20] txdclk_nt_sel_lane_1_0
        uint32_t RXDCLK_NT_SEL_LANE_1_0      :2;      ///<BIT [23:22] rxdclk_nt_sel_lane_1_0
        uint32_t RSVD_24_31                  :8;      ///<BIT [31:24] rsvd_24_31
    } b;
} Comphy0SoccalCtrl1Lane_t;

/// @brief 0x6008
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_7                    :8;      ///<BIT [7:0] rsvd_0_7
        uint32_t APTA_TERMINATE_REASON_LANE_7_0 :8;      ///<BIT [15:8] apta_terminate_reason_lane_7_0
        uint32_t ERROR_RESPONSE_TTIU_DETECTED_LANE :1;      ///<BIT [16] error_response_ttiu_detected_lane
        uint32_t RSVD_17_31                  :15;     ///<BIT [31:17] rsvd_17_31
    } b;
} Comphy0SoccalCtrl3Lane_t;

/// @brief 0x600C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_15                   :16;     ///<BIT [15:0] rsvd_0_15
        uint32_t TX_EM_PEAK_MAX_LANE_3_0     :4;      ///<BIT [19:16] tx_em_peak_max_lane_3_0
        uint32_t TX_EM_PEAK_MIN_LANE_3_0     :4;      ///<BIT [23:20] tx_em_peak_min_lane_3_0
        uint32_t TX_EM_PO_MAX_LANE_3_0       :4;      ///<BIT [27:24] tx_em_po_max_lane_3_0
        uint32_t TX_EM_PRE_MAX_LANE_3_0      :4;      ///<BIT [31:28] tx_em_pre_max_lane_3_0
    } b;
} Comphy0SoccalCtrl4Lane_t;

/// @brief 0x6014
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_23                   :24;     ///<BIT [23:0] rsvd_0_23
        uint32_t TX_EM_PRE_MIN_LANE_3_0      :4;      ///<BIT [27:24] tx_em_pre_min_lane_3_0
        uint32_t TX_EM_PO_MIN_LANE_3_0       :4;      ///<BIT [31:28] tx_em_po_min_lane_3_0
    } b;
} Comphy0SoccalSaveData2Lane_t;

/// @brief 0x601C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PHY_REMOTE_CTRL_COMMAND_CODE_LANE_15_0 :16;     ///<BIT [15:0] phy_remote_ctrl_command_code_lane_15_0
        uint32_t RX_TERM_FAIL_CNT_LANE_3_0   :4;      ///<BIT [19:16] rx_term_fail_cnt_lane_3_0
        uint32_t RX_TERM_PASS_CNT_LANE_3_0   :4;      ///<BIT [23:20] rx_term_pass_cnt_lane_3_0
        uint32_t PHY_REMOTE_CTRL_COMMAND_TYPE_LANE_7_0 :8;      ///<BIT [31:24] phy_remote_ctrl_command_type_lane_7_0
    } b;
} Comphy0SocphyRemoteCtrlCommandLane_t;

/// @brief 0x6028
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_15                   :16;     ///<BIT [15:0] rsvd_0_15
        uint32_t TX_TRAIN_FRAME_DET_TIMER_LANE_7_0 :8;      ///<BIT [23:16] tx_train_frame_det_timer_lane_7_0
        uint32_t RSVD_24_31                  :8;      ///<BIT [31:24] rsvd_24_31
    } b;
} Comphy0SoctrxTrainIfTimers1Lane_t;

/// @brief 0x602C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_15                   :16;     ///<BIT [15:0] rsvd_0_15
        uint32_t PHY_LOCAL_STATUS_LANE_7_0   :8;      ///<BIT [23:16] phy_local_status_lane_7_0
        uint32_t DFE_RES_F0A_HIGH_THRES_INIT_LANE_7_0 :8;      ///<BIT [31:24] dfe_res_f0a_high_thres_init_lane_7_0
    } b;
} Comphy0SoctrxTrainIfTimers2Lane_t;

/// @brief 0x6030
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TRAIN_GN1_LANE_7_0          :8;      ///<BIT [7:0] train_gn1_lane_7_0
        uint32_t TRAIN_G1_LANE_7_0           :8;      ///<BIT [15:8] train_g1_lane_7_0
        uint32_t TRAIN_G0_LANE_7_0           :8;      ///<BIT [23:16] train_g0_lane_7_0
        uint32_t PHY_MCU_LOCAL_ACK_LANE      :1;      ///<BIT [24] phy_mcu_local_ack_lane
        uint32_t TX_G1_MIDPOINT_EN_LANE      :1;      ///<BIT [25] tx_g1_midpoint_en_lane
        uint32_t RSVD_26_27                  :2;      ///<BIT [27:26] rsvd_26_27
        uint32_t TX_TRAIN_FRAME_DET_TIMER_ENABLE_LANE :1;      ///<BIT [28] tx_train_frame_det_timer_enable_lane
        uint32_t TX_TRAIN_TIMER_ENABLE_LANE  :1;      ///<BIT [29] tx_train_timer_enable_lane
        uint32_t RX_TRAIN_TIMER_ENABLE_LANE  :1;      ///<BIT [30] rx_train_timer_enable_lane
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Comphy0SoctrxTrainIfTimersEnableLane_t;

/// @brief 0x6038
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CDR_MIDPOINT_EN_LANE        :1;      ///<BIT [0] cdr_midpoint_en_lane
        uint32_t RSVD_1_2                    :2;      ///<BIT [2:1] rsvd_1_2
        uint32_t EOM_READY_LANE              :1;      ///<BIT [3] eom_ready_lane
        uint32_t EOM_DFE_CALL_LANE           :1;      ///<BIT [4] eom_dfe_call_lane
        uint32_t TX_GN1_MIDPOINT_EN_LANE     :1;      ///<BIT [5] tx_gn1_midpoint_en_lane
        uint32_t RSVD_6_7                    :2;      ///<BIT [7:6] rsvd_6_7
        uint32_t ESM_VOLTAGE_LANE_7_0        :8;      ///<BIT [15:8] esm_voltage_lane_7_0
        uint32_t GAIN_TRAIN_WITH_C_LANE      :1;      ///<BIT [16] gain_train_with_c_lane
        uint32_t GAIN_TRAIN_END_EN_LANE      :1;      ///<BIT [17] gain_train_end_en_lane
        uint32_t GAIN_TRAIN_INIT_EN_LANE     :1;      ///<BIT [18] gain_train_init_en_lane
        uint32_t RSVD_19_31                  :13;     ///<BIT [31:19] rsvd_19_31
    } b;
} Comphy0SocdfeControl1_t;

/// @brief 0x6044
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_2                    :3;      ///<BIT [2:0] rsvd_0_2
        uint32_t TX_TRAIN_P2P_HOLD_LANE      :1;      ///<BIT [3] tx_train_p2p_hold_lane
        uint32_t RSVD_4_31                   :28;     ///<BIT [31:4] rsvd_4_31
    } b;
} Comphy0SocdfeControl3_t;

/// @brief 0x604C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      :1;      ///<BIT [0] rsvd_0
        uint32_t RX_NO_INIT_LANE             :1;      ///<BIT [1] rx_no_init_lane
        uint32_t TX_NO_INIT_LANE             :1;      ///<BIT [2] tx_no_init_lane
        uint32_t RSVD_3_7                    :5;      ///<BIT [7:3] rsvd_3_7
        uint32_t THRE_GOOD_LANE_4_0          :5;      ///<BIT [12:8] thre_good_lane_4_0
        uint32_t RSVD_13                     :1;      ///<BIT [13] rsvd_13
        uint32_t SATURATE_DISABLE_LANE       :1;      ///<BIT [14] saturate_disable_lane
        uint32_t CDRPHASE_OPT_EN_LANE        :1;      ///<BIT [15] cdrphase_opt_en_lane
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Comphy0SocdfeControl5_t;

/// @brief 0x6058
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ESM_PHASE_LANE_9_0          :10;     ///<BIT [9:0] esm_phase_lane_9_0
        uint32_t RSVD_10_17                  :8;      ///<BIT [17:10] rsvd_10_17
        uint32_t ESM_EN_LANE                 :1;      ///<BIT [18] esm_en_lane
        uint32_t RSVD_19_20                  :2;      ///<BIT [20:19] rsvd_19_20
        uint32_t TX_ADAPT_G1_EN_LANE         :1;      ///<BIT [21] tx_adapt_g1_en_lane
        uint32_t TX_ADAPT_GN1_EN_LANE        :1;      ///<BIT [22] tx_adapt_gn1_en_lane
        uint32_t TX_ADAPT_G0_EN_LANE         :1;      ///<BIT [23] tx_adapt_g0_en_lane
        uint32_t RSVD_24_27                  :4;      ///<BIT [27:24] rsvd_24_27
        uint32_t RX_RXFFE_R_INI_LANE_3_0     :4;      ///<BIT [31:28] rx_rxffe_r_ini_lane_3_0
    } b;
} Comphy0SoctrainControl2_t;

/// @brief 0x6064
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_23                   :24;     ///<BIT [23:0] rsvd_0_23
        uint32_t DFE_F0_SAT_THRES_LANE_7_0   :8;      ///<BIT [31:24] dfe_f0_sat_thres_lane_7_0
    } b;
} Comphy0SocdllCal_t;

/// @brief 0x606C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DFE_RES_F0A_HIGH_THRES_END_LANE_7_0 :8;      ///<BIT [7:0] dfe_res_f0a_high_thres_end_lane_7_0
        uint32_t RSVD_8_31                   :24;     ///<BIT [31:8] rsvd_8_31
    } b;
} Comphy0SoctrainPara1_t;

/// @brief 0x6070
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_FFE_OVERBOOST_THRES_LANE_3_0 :4;      ///<BIT [3:0] rx_ffe_overboost_thres_lane_3_0
        uint32_t SUMF_BOOST_TARGET_C_FORCE_EN_LANE :1;      ///<BIT [4] sumf_boost_target_c_force_en_lane
        uint32_t RSVD_5_31                   :27;     ///<BIT [31:5] rsvd_5_31
    } b;
} Comphy0SoctrainPara2_t;

/// @brief 0x6074
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DFE_RES_F0A_LOW_THRES_2_END_LANE_7_0 :8;      ///<BIT [7:0] dfe_res_f0a_low_thres_2_end_lane_7_0
        uint32_t DFE_RES_F0A_LOW_THRES_3_END_LANE_7_0 :8;      ///<BIT [15:8] dfe_res_f0a_low_thres_3_end_lane_7_0
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Comphy0SoctrainPara3_t;

/// @brief 0x6078
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DFE_RES_F0A_LOW_THRES_01_INIT_LANE_7_0 :8;      ///<BIT [7:0] dfe_res_f0a_low_thres_01_init_lane_7_0
        uint32_t DFE_RES_F0A_LOW_THRES_2_INIT_LANE_7_0 :8;      ///<BIT [15:8] dfe_res_f0a_low_thres_2_init_lane_7_0
        uint32_t DFE_RES_F0A_LOW_THRES_3_INIT_LANE_7_0 :8;      ///<BIT [23:16] dfe_res_f0a_low_thres_3_init_lane_7_0
        uint32_t DFE_RES_F0A_LOW_THRES_01_END_LANE_7_0 :8;      ///<BIT [31:24] dfe_res_f0a_low_thres_01_end_lane_7_0
    } b;
} Comphy0SocdfeControl6_t;

/// @brief 0x6080
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_23                   :24;     ///<BIT [23:0] rsvd_0_23
        uint32_t RX_TRAIN_TIMEOUT_LANE       :1;      ///<BIT [24] rx_train_timeout_lane
        uint32_t TX_TRAIN_TIMEOUT_LANE       :1;      ///<BIT [25] tx_train_timeout_lane
        uint32_t TRX_TRAIN_DONE_INT_LANE     :1;      ///<BIT [26] trx_train_done_int_lane
        uint32_t RSVD_27                     :1;      ///<BIT [27] rsvd_27
        uint32_t RX_TRAIN_COMPLETE_INT_LANE  :1;      ///<BIT [28] rx_train_complete_int_lane
        uint32_t RX_TRAIN_FAIL_INT_LANE      :1;      ///<BIT [29] rx_train_fail_int_lane
        uint32_t RSVD_30_31                  :2;      ///<BIT [31:30] rsvd_30_31
    } b;
} Comphy0SocdfeTest1_t;

/// @brief 0x608C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_2                    :3;      ///<BIT [2:0] rsvd_0_2
        uint32_t TX_G1_STEP_NUM_LANE_4_0     :5;      ///<BIT [7:3] tx_g1_step_num_lane_4_0
        uint32_t RSVD_8_27                   :20;     ///<BIT [27:8] rsvd_8_27
        uint32_t RX_RXFFE_C_INI_LANE_3_0     :4;      ///<BIT [31:28] rx_rxffe_c_ini_lane_3_0
    } b;
} Comphy0SocdfeControl7_t;

/// @brief 0x60A0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_6                    :7;      ///<BIT [6:0] rsvd_0_6
        uint32_t CDR_MAXF0P_EN_LANE          :1;      ///<BIT [7] cdr_maxf0p_en_lane
        uint32_t RSVD_8_31                   :24;     ///<BIT [31:8] rsvd_8_31
    } b;
} Comphy0SoccdsCtrlReg0_t;

/// @brief 0x60A4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_5                    :6;      ///<BIT [5:0] rsvd_0_5
        uint32_t TX_GN1_MAXF0T_EN_LANE       :1;      ///<BIT [6] tx_gn1_maxf0t_en_lane
        uint32_t TX_G1_MAXF0T_EN_LANE        :1;      ///<BIT [7] tx_g1_maxf0t_en_lane
        uint32_t EOM_BER_LANE_3_0            :4;      ///<BIT [11:8] eom_ber_lane_3_0
        uint32_t CDR_STEP_NUM_LANE_3_0       :4;      ///<BIT [15:12] cdr_step_num_lane_3_0
        uint32_t TX_G0_STEP_NUM_LANE_3_0     :4;      ///<BIT [19:16] tx_g0_step_num_lane_3_0
        uint32_t TX_GN1_STEP_SIZE_LANE_1_0   :2;      ///<BIT [21:20] tx_gn1_step_size_lane_1_0
        uint32_t TX_G0_STEP_SIZE_LANE_1_0    :2;      ///<BIT [23:22] tx_g0_step_size_lane_1_0
        uint32_t TX_G1_STEP_SIZE_LANE_1_0    :2;      ///<BIT [25:24] tx_g1_step_size_lane_1_0
        uint32_t TX_GN1_STEP_NUM_LANE_4_0    :5;      ///<BIT [30:26] tx_gn1_step_num_lane_4_0
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Comphy0SoccdsCtrlReg1_t;

/// @brief 0x60B0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EOM_ERR_P_CNT_LANE_39_32    :8;      ///<BIT [7:0] eom_err_p_cnt_lane_39_32
        uint32_t EOM_ERR_N_CNT_LANE_39_32    :8;      ///<BIT [15:8] eom_err_n_cnt_lane_39_32
        uint32_t EOM_POP_P_CNT_LANE_39_32    :8;      ///<BIT [23:16] eom_pop_p_cnt_lane_39_32
        uint32_t EOM_POP_N_CNT_LANE_39_32    :8;      ///<BIT [31:24] eom_pop_n_cnt_lane_39_32
    } b;
} Comphy0SocesmErrPopCntHighLane_t;

/// @brief 0x6248
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_24                   :25;     ///<BIT [24:0] rsvd_0_24
        uint32_t STORED_FREE_OSC_SEL_LANE    :1;      ///<BIT [25] stored_free_osc_sel_lane
        uint32_t RSVD_26_31                  :6;      ///<BIT [31:26] rsvd_26_31
    } b;
} Comphy0SocdfeResetOverwrite_t;

/// @brief 0x6CC4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_15                   :16;     ///<BIT [15:0] rsvd_0_15
        uint32_t RX_SELMUFI_G2_LANE_2_0      :3;      ///<BIT [18:16] rx_selmufi_g2_lane_2_0
        uint32_t RSVD_19_23                  :5;      ///<BIT [23:19] rsvd_19_23
        uint32_t RX_SELMUFF_G2_LANE_2_0      :3;      ///<BIT [26:24] rx_selmuff_g2_lane_2_0
        uint32_t RSVD_27_31                  :5;      ///<BIT [31:27] rsvd_27_31
    } b;
} Comphy0Socautospeed424_t;

/// @brief 0x6CC8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t REG_SELMUPI_G2_LANE_3_0     :4;      ///<BIT [3:0] reg_selmupi_g2_lane_3_0
        uint32_t RSVD_4_7                    :4;      ///<BIT [7:4] rsvd_4_7
        uint32_t REG_SELMUPF_G2_LANE_3_0     :4;      ///<BIT [11:8] reg_selmupf_g2_lane_3_0
        uint32_t RSVD_12_31                  :20;     ///<BIT [31:12] rsvd_12_31
    } b;
} Comphy0Socautospeed425_t;

/// @brief 0x6D10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_23                   :24;     ///<BIT [23:0] rsvd_0_23
        uint32_t RX_REG0P9_SPEED_TRACK_CLK_G3_LANE_2_0 :3;      ///<BIT [26:24] rx_reg0p9_speed_track_clk_g3_lane_2_0
        uint32_t RSVD_27_31                  :5;      ///<BIT [31:27] rsvd_27_31
    } b;
} Comphy0Socautospeed443_t;

/// @brief 0x6D14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_REG0P9_SPEED_TRACK_CLK_HALF_G3_LANE :1;      ///<BIT [0] rx_reg0p9_speed_track_clk_half_g3_lane
        uint32_t RSVD_1_7                    :7;      ///<BIT [7:1] rsvd_1_7
        uint32_t RX_REG0P9_SPEED_TRACK_DATA_G3_LANE_2_0 :3;      ///<BIT [10:8] rx_reg0p9_speed_track_data_g3_lane_2_0
        uint32_t RSVD_11_15                  :5;      ///<BIT [15:11] rsvd_11_15
        uint32_t RX_SELMUFI_G3_LANE_2_0      :3;      ///<BIT [18:16] rx_selmufi_g3_lane_2_0
        uint32_t RSVD_19_23                  :5;      ///<BIT [23:19] rsvd_19_23
        uint32_t RX_SELMUFF_G3_LANE_2_0      :3;      ///<BIT [26:24] rx_selmuff_g3_lane_2_0
        uint32_t RSVD_27_31                  :5;      ///<BIT [31:27] rsvd_27_31
    } b;
} Comphy0Socautospeed444_t;

/// @brief 0x6D18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t REG_SELMUPI_G3_LANE_3_0     :4;      ///<BIT [3:0] reg_selmupi_g3_lane_3_0
        uint32_t RSVD_4_7                    :4;      ///<BIT [7:4] rsvd_4_7
        uint32_t REG_SELMUPF_G3_LANE_3_0     :4;      ///<BIT [11:8] reg_selmupf_g3_lane_3_0
        uint32_t RSVD_12_31                  :20;     ///<BIT [31:12] rsvd_12_31
    } b;
} Comphy0Socautospeed445_t;

/// @brief 0x6D64
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_15                   :16;     ///<BIT [15:0] rsvd_0_15
        uint32_t RX_SELMUFI_G4_LANE_2_0      :3;      ///<BIT [18:16] rx_selmufi_g4_lane_2_0
        uint32_t RSVD_19_23                  :5;      ///<BIT [23:19] rsvd_19_23
        uint32_t RX_SELMUFF_G4_LANE_2_0      :3;      ///<BIT [26:24] rx_selmuff_g4_lane_2_0
        uint32_t RSVD_27_31                  :5;      ///<BIT [31:27] rsvd_27_31
    } b;
} Comphy0Socautospeed464_t;

/// @brief 0x6D68
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t REG_SELMUPI_G4_LANE_3_0     :4;      ///<BIT [3:0] reg_selmupi_g4_lane_3_0
        uint32_t RSVD_4_7                    :4;      ///<BIT [7:4] rsvd_4_7
        uint32_t REG_SELMUPF_G4_LANE_3_0     :4;      ///<BIT [11:8] reg_selmupf_g4_lane_3_0
        uint32_t RSVD_12_31                  :20;     ///<BIT [31:12] rsvd_12_31
    } b;
} Comphy0Socautospeed465_t;

/// @brief 0x8204
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t VTH_TXIMPCAL_2_0            :3;      ///<BIT [4:2] vth_tximpcal_2_0
        uint32_t RSVD_5_31                   :27;     ///<BIT [31:5] rsvd_5_31
    } b;
} Comphy0Socuphy14CmnAnaregTop129_t;

/// @brief 0x8228
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t VTH_RXIMPCAL_2_0            :3;      ///<BIT [4:2] vth_rximpcal_2_0
        uint32_t RSVD_5_31                   :27;     ///<BIT [31:5] rsvd_5_31
    } b;
} Comphy0Socuphy14CmnAnaregTop138_t;

/// @brief 0x8348
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VDDVCO_VTH_12NM_SEL         :1;      ///<BIT [0] vddvco_vth_12nm_sel
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} Comphy0Socuphy14CmnAnaregTop210_t;

/// @brief 0xA008
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INIT_TXFOFFS_9_0            :10;     ///<BIT [9:0] init_txfoffs_9_0
        uint32_t RSVD_10_29                  :20;     ///<BIT [29:10] rsvd_10_29
        uint32_t SSC_DSPREAD_TX              :1;      ///<BIT [30] ssc_dspread_tx
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Comphy0SocdtxReg0_t;

/// @brief 0xA010
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INIT_TXFOFFS_RING_9_0       :10;     ///<BIT [9:0] init_txfoffs_ring_9_0
        uint32_t RSVD_10                     :1;      ///<BIT [10] rsvd_10
        uint32_t DTX_CLAMPING_TRIGGER        :1;      ///<BIT [11] dtx_clamping_trigger
        uint32_t DTX_CLAMPING_TRIGGER_CLEAR  :1;      ///<BIT [12] dtx_clamping_trigger_clear
        uint32_t DTX_CLAMPING_EN             :1;      ///<BIT [13] dtx_clamping_en
        uint32_t DTX_CLAMPING_SEL_1_0        :2;      ///<BIT [15:14] dtx_clamping_sel_1_0
        uint32_t RSVD_16_29                  :14;     ///<BIT [29:16] rsvd_16_29
        uint32_t SSC_DSPREAD_TX_RING         :1;      ///<BIT [30] ssc_dspread_tx_ring
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Comphy0SocdtxReg2_t;

/// @brief 0xA018
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_15                   :16;     ///<BIT [15:0] rsvd_0_15
        uint32_t DTX_FLOOP_EN                :1;      ///<BIT [16] dtx_floop_en
        uint32_t DTX_FLOOP_EN_RING           :1;      ///<BIT [17] dtx_floop_en_ring
        uint32_t DTX_FOFFSET_SEL             :1;      ///<BIT [18] dtx_foffset_sel
        uint32_t DTX_FOFFSET_SEL_RING        :1;      ///<BIT [19] dtx_foffset_sel_ring
        uint32_t RSVD_20_31                  :12;     ///<BIT [31:20] rsvd_20_31
    } b;
} Comphy0SocdtxReg4_t;

/// @brief 0xA01C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_27                   :28;     ///<BIT [27:0] rsvd_0_27
        uint32_t LANE_ALIGN_FAST_DONE_SEL_1_0 :2;      ///<BIT [29:28] lane_align_fast_done_sel_1_0
        uint32_t RSVD_30_31                  :2;      ///<BIT [31:30] rsvd_30_31
    } b;
} Comphy0SocdtxPhyAlignReg0_t;

/// @brief 0xA024
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_19                   :20;     ///<BIT [19:0] rsvd_0_19
        uint32_t ALIGN_ACCURATE_STEP_1_0     :2;      ///<BIT [21:20] align_accurate_step_1_0
        uint32_t RSVD_22_27                  :6;      ///<BIT [27:22] rsvd_22_27
        uint32_t ALIGN_ACCURATE_EN           :1;      ///<BIT [28] align_accurate_en
        uint32_t RSVD_29_31                  :3;      ///<BIT [31:29] rsvd_29_31
    } b;
} Comphy0SocdtxPhyAlignReg1_t;

/// @brief 0xA028
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SSC_AMP_10_0                :11;     ///<BIT [10:0] ssc_amp_10_0
        uint32_t RSVD_11_15                  :5;      ///<BIT [15:11] rsvd_11_15
        uint32_t SSC_AMP_RING_10_0           :11;     ///<BIT [26:16] ssc_amp_ring_10_0
        uint32_t RSVD_27_30                  :4;      ///<BIT [30:27] rsvd_27_30
        uint32_t SSC_AMP_UNIT_SEL            :1;      ///<BIT [31] ssc_amp_unit_sel
    } b;
} Comphy0SocdtxReg5_t;

/// @brief 0xA02C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_13                   :14;     ///<BIT [13:0] rsvd_0_13
        uint32_t SRIS_DIS                    :1;      ///<BIT [14] sris_dis
        uint32_t SRIS_DIS_FORCE              :1;      ///<BIT [15] sris_dis_force
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Comphy0SocsrisReg0_t;

/// @brief 0xA030
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_10                   :11;     ///<BIT [10:0] rsvd_0_10
        uint32_t SRIS_SSC_CYCLE_DISABLE      :1;      ///<BIT [11] sris_ssc_cycle_disable
        uint32_t RSVD_12_18                  :7;      ///<BIT [18:12] rsvd_12_18
        uint32_t INIT_TXFOFFS_EN             :1;      ///<BIT [19] init_txfoffs_en
        uint32_t RSVD_20_31                  :12;     ///<BIT [31:20] rsvd_20_31
    } b;
} Comphy0SocsrisReg1_t;

/// @brief 0xA200
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MCU_EN_LANE0                :1;      ///<BIT [0] mcu_en_lane0
        uint32_t MCU_EN_LANE1                :1;      ///<BIT [1] mcu_en_lane1
        uint32_t MCU_EN_LANE2                :1;      ///<BIT [2] mcu_en_lane2
        uint32_t MCU_EN_LANE3                :1;      ///<BIT [3] mcu_en_lane3
        uint32_t MCU_EN_CMN                  :1;      ///<BIT [4] mcu_en_cmn
        uint32_t RSVD_5_7                    :3;      ///<BIT [7:5] rsvd_5_7
        uint32_t INIT_DONE_CMN               :1;      ///<BIT [8] init_done_cmn
        uint32_t INIT_XDATA_FROM_PMEM        :1;      ///<BIT [9] init_xdata_from_pmem
        uint32_t RSVD_10_31                  :22;     ///<BIT [31:10] rsvd_10_31
    } b;
} Comphy0SocmcuControl0_t;

/// @brief 0xA204
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SET_MCU_INT_LANE0           :1;      ///<BIT [0] set_mcu_int_lane0
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} Comphy0SocmcuControl1_t;

/// @brief 0xA208
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SET_MCU_INT_LANE1           :1;      ///<BIT [0] set_mcu_int_lane1
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} Comphy0SocmcuControl2_t;

/// @brief 0xA21C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ROM_PARITY_ERR_SET          :1;      ///<BIT [0] rom_parity_err_set
        uint32_t ROM_PARITY_ERR_ENABLE       :1;      ///<BIT [1] rom_parity_err_enable
        uint32_t ROM_PARITY_ERR_CLEAR        :1;      ///<BIT [2] rom_parity_err_clear
        uint32_t PROG_RAM_SEL_1_0            :2;      ///<BIT [4:3] prog_ram_sel_1_0
        uint32_t RSVD_5_15                   :11;     ///<BIT [15:5] rsvd_5_15
        uint32_t PRAM_ECC_2ERR_SET           :1;      ///<BIT [16] pram_ecc_2err_set
        uint32_t PRAM_ECC_2ERR_ENABLE        :1;      ///<BIT [17] pram_ecc_2err_enable
        uint32_t PRAM_ECC_2ERR_CLEAR         :1;      ///<BIT [18] pram_ecc_2err_clear
        uint32_t PRAM_ECC_1ERR_SET           :1;      ///<BIT [19] pram_ecc_1err_set
        uint32_t PRAM_ECC_1ERR_ENABLE        :1;      ///<BIT [20] pram_ecc_1err_enable
        uint32_t PRAM_ECC_1ERR_CLEAR         :1;      ///<BIT [21] pram_ecc_1err_clear
        uint32_t ROM_PARITY_ERR              :1;      ///<BIT [22] rom_parity_err
        uint32_t PRAM_ECC_2ERR               :1;      ///<BIT [23] pram_ecc_2err
        uint32_t PRAM_ECC_1ERR               :1;      ///<BIT [24] pram_ecc_1err
        uint32_t RSVD_25_31                  :7;      ///<BIT [31:25] rsvd_25_31
    } b;
} Comphy0SocmemoryControl0_t;

/// @brief 0xA22C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PMEM_CHECKSUM_RESET         :1;      ///<BIT [0] pmem_checksum_reset
        uint32_t PMEM_CHECKSUM_PASS          :1;      ///<BIT [1] pmem_checksum_pass
        uint32_t ECC_ENABLE                  :1;      ///<BIT [2] ecc_enable
        uint32_t RSVD_3_4                    :2;      ///<BIT [4:3] rsvd_3_4
        uint32_t XDATA_ECC_1ERR_CMN          :1;      ///<BIT [5] xdata_ecc_1err_cmn
        uint32_t CACHE_ECC_1ERR_CMN          :1;      ///<BIT [6] cache_ecc_1err_cmn
        uint32_t IRAM_ECC_1ERR_CMN           :1;      ///<BIT [7] iram_ecc_1err_cmn
        uint32_t XDATA_ECC_2ERR_CMN          :1;      ///<BIT [8] xdata_ecc_2err_cmn
        uint32_t CACHE_ECC_2ERR_CMN          :1;      ///<BIT [9] cache_ecc_2err_cmn
        uint32_t IRAM_ECC_2ERR_CMN           :1;      ///<BIT [10] iram_ecc_2err_cmn
        uint32_t XDATA_ECC_1ERR_ENABLE_CMN   :1;      ///<BIT [11] xdata_ecc_1err_enable_cmn
        uint32_t CACHE_ECC_1ERR_ENABLE_CMN   :1;      ///<BIT [12] cache_ecc_1err_enable_cmn
        uint32_t IRAM_ECC_1ERR_ENABLE_CMN    :1;      ///<BIT [13] iram_ecc_1err_enable_cmn
        uint32_t XDATA_ECC_2ERR_ENABLE_CMN   :1;      ///<BIT [14] xdata_ecc_2err_enable_cmn
        uint32_t CACHE_ECC_2ERR_ENABLE_CMN   :1;      ///<BIT [15] cache_ecc_2err_enable_cmn
        uint32_t IRAM_ECC_2ERR_ENABLE_CMN    :1;      ///<BIT [16] iram_ecc_2err_enable_cmn
        uint32_t XDATA_ECC_1ERR_CLEAR_CMN    :1;      ///<BIT [17] xdata_ecc_1err_clear_cmn
        uint32_t CACHE_ECC_1ERR_CLEAR_CMN    :1;      ///<BIT [18] cache_ecc_1err_clear_cmn
        uint32_t IRAM_ECC_1ERR_CLEAR_CMN     :1;      ///<BIT [19] iram_ecc_1err_clear_cmn
        uint32_t XDATA_ECC_2ERR_CLEAR_CMN    :1;      ///<BIT [20] xdata_ecc_2err_clear_cmn
        uint32_t CACHE_ECC_2ERR_CLEAR_CMN    :1;      ///<BIT [21] cache_ecc_2err_clear_cmn
        uint32_t IRAM_ECC_2ERR_CLEAR_CMN     :1;      ///<BIT [22] iram_ecc_2err_clear_cmn
        uint32_t XDATA_ECC_1ERR_SET_CMN      :1;      ///<BIT [23] xdata_ecc_1err_set_cmn
        uint32_t CACHE_ECC_1ERR_SET_CMN      :1;      ///<BIT [24] cache_ecc_1err_set_cmn
        uint32_t IRAM_ECC_1ERR_SET_CMN       :1;      ///<BIT [25] iram_ecc_1err_set_cmn
        uint32_t XDATA_ECC_2ERR_SET_CMN      :1;      ///<BIT [26] xdata_ecc_2err_set_cmn
        uint32_t CACHE_ECC_2ERR_SET_CMN      :1;      ///<BIT [27] cache_ecc_2err_set_cmn
        uint32_t IRAM_ECC_2ERR_SET_CMN       :1;      ///<BIT [28] iram_ecc_2err_set_cmn
        uint32_t RSVD_29_31                  :3;      ///<BIT [31:29] rsvd_29_31
    } b;
} Comphy0SocmemoryControl4_t;

/// @brief 0xA244
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t XDATA_ECC_ERR_ADDR_CMN_8_0  :9;      ///<BIT [8:0] xdata_ecc_err_addr_cmn_8_0
        uint32_t IRAM_ECC_ERR_ADDR_CMN_7_0   :8;      ///<BIT [16:9] iram_ecc_err_addr_cmn_7_0
        uint32_t CACHE_ECC_ERR_ADDR_CMN_7_0  :8;      ///<BIT [24:17] cache_ecc_err_addr_cmn_7_0
        uint32_t RSVD_25_31                  :7;      ///<BIT [31:25] rsvd_25_31
    } b;
} Comphy0SocmemCmnEccErrAddress0_t;

/// @brief 0xA2F0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t APB_BURST_EN                :1;      ///<BIT [0] apb_burst_en
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} Comphy0SocapbControl_t;

/// @brief 0xA300
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DIG_INT_RSVD0_15_0          :16;     ///<BIT [15:0] dig_int_rsvd0_15_0
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Comphy0Soctest0_t;

/// @brief 0xA308
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_23                   :24;     ///<BIT [23:0] rsvd_0_23
        uint32_t TESTBUS_SEL_LO0_CMN_5_0     :6;      ///<BIT [29:24] testbus_sel_lo0_cmn_5_0
        uint32_t RSVD_30                     :1;      ///<BIT [30] rsvd_30
        uint32_t STRESSTEST_EN               :1;      ///<BIT [31] stresstest_en
    } b;
} Comphy0Soctest2_t;

/// @brief 0xA30C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_12                   :13;     ///<BIT [12:0] rsvd_0_12
        uint32_t TESTBUS_HI8BSEL_8BMODE      :1;      ///<BIT [13] testbus_hi8bsel_8bmode
        uint32_t RSVD_14_22                  :9;      ///<BIT [22:14] rsvd_14_22
        uint32_t TESTBUS_SEL_HI0_CMN_5_0     :6;      ///<BIT [28:23] testbus_sel_hi0_cmn_5_0
        uint32_t TESTBUS_LANE_SEL0_2_0       :3;      ///<BIT [31:29] testbus_lane_sel0_2_0
    } b;
} Comphy0Soctest3_t;

/// @brief 0xA310
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DIG_TEST_BUS_15_0           :16;     ///<BIT [15:0] dig_test_bus_15_0
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Comphy0Soctest4_t;

/// @brief 0xA314
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t PHY_ALIGN_OFF               :1;      ///<BIT [2] phy_align_off
        uint32_t MASTER_PHY_EN               :1;      ///<BIT [3] master_phy_en
        uint32_t RSVD_4                      :1;      ///<BIT [4] rsvd_4
        uint32_t SLAVE_ALIGN_REFCLK_FM_SIDE_A :1;      ///<BIT [5] slave_align_refclk_fm_side_a
        uint32_t PHY_CONFIG_1_0              :2;      ///<BIT [7:6] phy_config_1_0
        uint32_t ANA_CMN_PHY_X2_MASTER_EN_1_0 :2;      ///<BIT [9:8] ana_cmn_phy_x2_master_en_1_0
        uint32_t RSVD_10_18                  :9;      ///<BIT [18:10] rsvd_10_18
        uint32_t PHY_MODE_FM_REG             :1;      ///<BIT [19] phy_mode_fm_reg
        uint32_t SFT_RST_ONLY_REG            :1;      ///<BIT [20] sft_rst_only_reg
        uint32_t SFT_RST_NO_REG_CMN          :1;      ///<BIT [21] sft_rst_no_reg_cmn
        uint32_t RSVD_22                     :1;      ///<BIT [22] rsvd_22
        uint32_t PHY_ISOLATE_MODE            :1;      ///<BIT [23] phy_isolate_mode
        uint32_t PHY_MODE_2_0                :3;      ///<BIT [26:24] phy_mode_2_0
        uint32_t BROADCAST                   :1;      ///<BIT [27] broadcast
        uint32_t RSVD_28                     :1;      ///<BIT [28] rsvd_28
        uint32_t LANE_SEL_2_0                :3;      ///<BIT [31:29] lane_sel_2_0
    } b;
} Comphy0Socsystem_t;

/// @brief 0xA318
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t ANA_FBCK_SEL_RING           :1;      ///<BIT [2] ana_fbck_sel_ring
        uint32_t RSVD_3_8                    :6;      ///<BIT [8:3] rsvd_3_8
        uint32_t ANA_FBCK_SEL                :1;      ///<BIT [9] ana_fbck_sel
        uint32_t RSVD_10_12                  :3;      ///<BIT [12:10] rsvd_10_12
        uint32_t REFCLK_SEL                  :1;      ///<BIT [13] refclk_sel
        uint32_t ANA_PLL_LOCK_RD             :1;      ///<BIT [14] ana_pll_lock_rd
        uint32_t RSVD_15_31                  :17;     ///<BIT [31:15] rsvd_15_31
    } b;
} Comphy0SocpmCmnReg1_t;

/// @brief 0xA31C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      :1;      ///<BIT [0] rsvd_0
        uint32_t PU_IVREF                    :1;      ///<BIT [1] pu_ivref
        uint32_t RSVD_2_3                    :2;      ///<BIT [3:2] rsvd_2_3
        uint32_t BG_RDY_FM_REG               :1;      ///<BIT [4] bg_rdy_fm_reg
        uint32_t BG_RDY                      :1;      ///<BIT [5] bg_rdy
        uint32_t RSVD_6_31                   :26;     ///<BIT [31:6] rsvd_6_31
    } b;
} Comphy0SocinputCmnPinReg0_t;

/// @brief 0xA320
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t REF_FREF_SEL_4_0            :5;      ///<BIT [4:0] ref_fref_sel_4_0
        uint32_t RSVD_5_31                   :27;     ///<BIT [31:5] rsvd_5_31
    } b;
} Comphy0SocinputCmnPinReg1_t;

/// @brief 0xA324
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_7                    :8;      ///<BIT [7:0] rsvd_0_7
        uint32_t IDDQ                        :1;      ///<BIT [8] iddq
        uint32_t RSVD_9_27                   :19;     ///<BIT [27:9] rsvd_9_27
        uint32_t RESERVED_3_0                :4;      ///<BIT [31:28] reserved_3_0
    } b;
} Comphy0SocinputCmnPinReg2_t;

/// @brief 0xA330
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_3                    :4;      ///<BIT [3:0] rsvd_0_3
        uint32_t ANA_PROCESS_VALUE_3_0       :4;      ///<BIT [7:4] ana_process_value_3_0
        uint32_t RSVD_8_31                   :24;     ///<BIT [31:8] rsvd_8_31
    } b;
} Comphy0SocpllcalReg1_t;

/// @brief 0xA334
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EN_LANE0                    :1;      ///<BIT [0] en_lane0
        uint32_t EN_LANE1                    :1;      ///<BIT [1] en_lane1
        uint32_t RSVD_2_3                    :2;      ///<BIT [3:2] rsvd_2_3
        uint32_t EN_CMN                      :1;      ///<BIT [4] en_cmn
        uint32_t RSVD_5_31                   :27;     ///<BIT [31:5] rsvd_5_31
    } b;
} Comphy0SocclkgenCmnReg1_t;

/// @brief 0xA338
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_7                    :8;      ///<BIT [7:0] rsvd_0_7
        uint32_t BEACON_DIVIDER_1_0          :2;      ///<BIT [9:8] beacon_divider_1_0
        uint32_t RSVD_10_31                  :22;     ///<BIT [31:10] rsvd_10_31
    } b;
} Comphy0SocspdCmnReg1_t;

/// @brief 0xA33C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      :1;      ///<BIT [0] rsvd_0
        uint32_t ANA_CLK100M_125M_EN         :1;      ///<BIT [1] ana_clk100m_125m_en
        uint32_t ANA_CLK100M_125M_SEL        :1;      ///<BIT [2] ana_clk100m_125m_sel
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} Comphy0SocoutputCmnPinReg0_t;

/// @brief 0xA360
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t XDATA_MEM_CHECKSUM_RESET_CMN :1;      ///<BIT [0] xdata_mem_checksum_reset_cmn
        uint32_t XDATA_MEM_CHECKSUM_PASS_CMN :1;      ///<BIT [1] xdata_mem_checksum_pass_cmn
        uint32_t RSVD_2_31                   :30;     ///<BIT [31:2] rsvd_2_31
    } b;
} Comphy0SocxdataMemChecksumCmn2_t;

/// @brief 0xA364
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_15                   :16;     ///<BIT [15:0] rsvd_0_15
        uint32_t ROM_ERR_ADDR_15_0           :16;     ///<BIT [31:16] rom_err_addr_15_0
    } b;
} Comphy0SocmcuSdtCmn_t;

/// @brief 0xA3B4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SET_INT_ISR_LANE0           :1;      ///<BIT [0] set_int_isr_lane0
        uint32_t RSVD_1_7                    :7;      ///<BIT [7:1] rsvd_1_7
        uint32_t SET_INT_ISR_LANE1           :1;      ///<BIT [8] set_int_isr_lane1
        uint32_t RSVD_9_31                   :23;     ///<BIT [31:9] rsvd_9_31
    } b;
} Comphy0SocsetLaneIsr_t;

/// @brief 0xA3F4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PHY_MCU_REMOTE_REQ          :1;      ///<BIT [0] phy_mcu_remote_req
        uint32_t PHY_MCU_REMOTE_ACK          :1;      ///<BIT [1] phy_mcu_remote_ack
        uint32_t RSVD_2_15                   :14;     ///<BIT [15:2] rsvd_2_15
        uint32_t PRAM_ECC_ERR_ADDR_15_0      :16;     ///<BIT [31:16] pram_ecc_err_addr_15_0
    } b;
} Comphy0SoccmnMcu_t;

/// @brief 0xA3F8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_15                   :16;     ///<BIT [15:0] rsvd_0_15
        uint32_t CID1_7_0                    :8;      ///<BIT [23:16] cid1_7_0
        uint32_t CID0_3_0                    :4;      ///<BIT [27:24] cid0_3_0
        uint32_t CID0_7_4                    :4;      ///<BIT [31:28] cid0_7_4
    } b;
} Comphy0SoccidReg0_t;

/// @brief 0xA3FC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_28                   :29;     ///<BIT [28:0] rsvd_0_28
        uint32_t PHY_LANE_NUM_2_0            :3;      ///<BIT [31:29] phy_lane_num_2_0
    } b;
} Comphy0SoccidReg1_t;

/// @brief 0xE27C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ICP_RING_PION_RATE1_3_0     :4;      ///<BIT [3:0] icp_ring_pion_rate1_3_0
        uint32_t RSVD_4_7                    :4;      ///<BIT [7:4] rsvd_4_7
        uint32_t PLL_SPEED_THRESH_RING_PION_RATE1_7_0 :8;      ///<BIT [15:8] pll_speed_thresh_ring_pion_rate1_7_0
        uint32_t PLL_SPEED_THRESH_RING_PION_RATE1_8 :1;      ///<BIT [16] pll_speed_thresh_ring_pion_rate1_8
        uint32_t RSVD_17_23                  :7;      ///<BIT [23:17] rsvd_17_23
        uint32_t FBDIV_CAL_RING_PION_RATE1_7_0 :8;      ///<BIT [31:24] fbdiv_cal_ring_pion_rate1_7_0
    } b;
} Comphy0Socautospeed159_t;

/// @brief 0xE600
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FW_BUILD_VER_7_0            :8;      ///<BIT [7:0] fw_build_ver_7_0
        uint32_t FW_PATCH_VER_7_0            :8;      ///<BIT [15:8] fw_patch_ver_7_0
        uint32_t FW_MINOR_VER_7_0            :8;      ///<BIT [23:16] fw_minor_ver_7_0
        uint32_t FW_MAJOR_VER_7_0            :8;      ///<BIT [31:24] fw_major_ver_7_0
    } b;
} Comphy0SocfwRev_t;

/// @brief 0xE604
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ETHERNET_CFG_1_0            :2;      ///<BIT [1:0] ethernet_cfg_1_0
        uint32_t LCPLL_LANE_SEL              :1;      ///<BIT [2] lcpll_lane_sel
        uint32_t RSVD_3                      :1;      ///<BIT [3] rsvd_3
        uint32_t TX_LANE_ALIGN_OFF           :1;      ///<BIT [4] tx_lane_align_off
        uint32_t RSVD_5_6                    :2;      ///<BIT [6:5] rsvd_5_6
        uint32_t FORCE_PARTIAL_PU_RX_ON      :1;      ///<BIT [7] force_partial_pu_rx_on
        uint32_t CAL_DONE                    :1;      ///<BIT [8] cal_done
        uint32_t MCU_INIT_DONE               :1;      ///<BIT [9] mcu_init_done
        uint32_t RSVD_10_13                  :4;      ///<BIT [13:10] rsvd_10_13
        uint32_t PCIE_RXIMP_DELAY_EN         :1;      ///<BIT [14] pcie_rximp_delay_en
        uint32_t RSVD_15_16                  :2;      ///<BIT [16:15] rsvd_15_16
        uint32_t EXT_FORCE_CAL_DONE          :1;      ///<BIT [17] ext_force_cal_done
        uint32_t BYPASS_DELAY_2_0            :3;      ///<BIT [20:18] bypass_delay_2_0
        uint32_t BYPASS_POWER_ON_DELAY       :1;      ///<BIT [21] bypass_power_on_delay
        uint32_t BYPASS_XDAT_INIT            :1;      ///<BIT [22] bypass_xdat_init
        uint32_t BYPASS_SPEED_TABLE_LOAD     :1;      ///<BIT [23] bypass_speed_table_load
        uint32_t FORCE_CONT_CAL_SKIP         :1;      ///<BIT [24] force_cont_cal_skip
        uint32_t RSVD_25                     :1;      ///<BIT [25] rsvd_25
        uint32_t TRAIN_SIM_EN                :1;      ///<BIT [26] train_sim_en
        uint32_t RSVD_27                     :1;      ///<BIT [27] rsvd_27
        uint32_t FAST_POWER_ON_EN            :1;      ///<BIT [28] fast_power_on_en
        uint32_t APTA_TRAIN_SIM_EN           :1;      ///<BIT [29] apta_train_sim_en
        uint32_t APTA_TRAIN_CMD_IF_EN        :1;      ///<BIT [30] apta_train_cmd_if_en
        uint32_t VIRTUAL_TDR_SIM_EN          :1;      ///<BIT [31] virtual_tdr_sim_en
    } b;
} Comphy0SoccontrolConfig0_t;

/// @brief 0xE620
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CAL_PROC_TT2FF_7_0          :8;      ///<BIT [7:0] cal_proc_tt2ff_7_0
        uint32_t CAL_PROC_SS2TT_7_0          :8;      ///<BIT [15:8] cal_proc_ss2tt_7_0
        uint32_t CAL_PROC_SUBSS_7_0          :8;      ///<BIT [23:16] cal_proc_subss_7_0
        uint32_t CAL_SQ_THRESH_IN_7_0        :8;      ///<BIT [31:24] cal_sq_thresh_in_7_0
    } b;
} Comphy0SoccontrolConfig7_t;

/// @brief 0xE624
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_7                    :8;      ///<BIT [7:0] rsvd_0_7
        uint32_t CAL_TEMPC_DAC_SEL_7_0       :8;      ///<BIT [15:8] cal_tempc_dac_sel_7_0
        uint32_t CAL_TEMPC_MUX_SEL_7_0       :8;      ///<BIT [23:16] cal_tempc_mux_sel_7_0
        uint32_t CAL_TEMPC_MUX_HOLD_SEL_7_0  :8;      ///<BIT [31:24] cal_tempc_mux_hold_sel_7_0
    } b;
} Comphy0SoccalData0_t;

/// @brief 0xE628
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PIPE4_EN                    :1;      ///<BIT [0] pipe4_en
        uint32_t TX_TRAIN_MODE               :1;      ///<BIT [1] tx_train_mode
        uint32_t RSVD_2_23                   :22;     ///<BIT [23:2] rsvd_2_23
        uint32_t LOCAL_TX_PRESET_INDEX_3_0   :4;      ///<BIT [27:24] local_tx_preset_index_3_0
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Comphy0SoctrainIfConfig_t;

/// @brief 0xE62C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PHY_GEN_MAX_3_0             :4;      ///<BIT [3:0] phy_gen_max_3_0
        uint32_t RSVD_4_15                   :12;     ///<BIT [15:4] rsvd_4_15
        uint32_t AUTO_RX_INIT_EN             :1;      ///<BIT [16] auto_rx_init_en
        uint32_t RSVD_17_18                  :2;      ///<BIT [18:17] rsvd_17_18
        uint32_t BYPASS_P4_JUMP_GN1_TRAIN_GEN4 :1;      ///<BIT [19] bypass_p4_jump_gn1_train_gen4
        uint32_t BYPASS_P4_JUMP_G1_TRAIN_GEN4 :1;      ///<BIT [20] bypass_p4_jump_g1_train_gen4
        uint32_t RSVD_21_31                  :11;     ///<BIT [31:21] rsvd_21_31
    } b;
} Comphy0SoccontrolConfig8_t;

/// @brief 0xE630
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MIDPOINT_USER_PHASE_OFFSET_7_0 :8;      ///<BIT [7:0] midpoint_user_phase_offset_7_0
        uint32_t RSVD_8_11                   :4;      ///<BIT [11:8] rsvd_8_11
        uint32_t BYPASS_P4_JUMP_GN1_TRAIN_GEN5 :1;      ///<BIT [12] bypass_p4_jump_gn1_train_gen5
        uint32_t BYPASS_P4_JUMP_G1_TRAIN_GEN5 :1;      ///<BIT [13] bypass_p4_jump_g1_train_gen5
        uint32_t RSVD_14_31                  :18;     ///<BIT [31:14] rsvd_14_31
    } b;
} Comphy0SoccontrolConfig9_t;

/// @brief 0xE650
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MASTER_MCU_SEL_7_0          :8;      ///<BIT [7:0] master_mcu_sel_7_0
        uint32_t RSVD_8_31                   :24;     ///<BIT [31:8] rsvd_8_31
    } b;
} Comphy0SocmcuConfig_t;

/// @brief 0xE65C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MCU_FREQ_15_0               :16;     ///<BIT [15:0] mcu_freq_15_0
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Comphy0SocmcuConfig1_t;

/// @brief 0xE6C4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PLL_RATE_SEL_1_VALID        :1;      ///<BIT [0] pll_rate_sel_1_valid
        uint32_t TX_ADAPT_G1_EN_PCIE_GEN5    :1;      ///<BIT [1] tx_adapt_g1_en_pcie_gen5
        uint32_t TX_ADAPT_GN1_EN_PCIE_GEN5   :1;      ///<BIT [2] tx_adapt_gn1_en_pcie_gen5
        uint32_t TX_ADAPT_G0_EN_PCIE_GEN5    :1;      ///<BIT [3] tx_adapt_g0_en_pcie_gen5
        uint32_t FORCE_DFE_STEP_ACCU_FX_3_0  :4;      ///<BIT [7:4] force_dfe_step_accu_fx_3_0
        uint32_t FORCE_DFE_STEP_ACCU_FX_EN   :1;      ///<BIT [8] force_dfe_step_accu_fx_en
        uint32_t RSVD_9                      :1;      ///<BIT [9] rsvd_9
        uint32_t TX_ADAPT_G1_EN_PCIE_GEN4    :1;      ///<BIT [10] tx_adapt_g1_en_pcie_gen4
        uint32_t TX_ADAPT_GN1_EN_PCIE_GEN4   :1;      ///<BIT [11] tx_adapt_gn1_en_pcie_gen4
        uint32_t TX_ADAPT_G0_EN_PCIE_GEN4    :1;      ///<BIT [12] tx_adapt_g0_en_pcie_gen4
        uint32_t TX_ADAPT_G1_EN_PCIE_GEN3    :1;      ///<BIT [13] tx_adapt_g1_en_pcie_gen3
        uint32_t TX_ADAPT_GN1_EN_PCIE_GEN3   :1;      ///<BIT [14] tx_adapt_gn1_en_pcie_gen3
        uint32_t TX_ADAPT_G0_EN_PCIE_GEN3    :1;      ///<BIT [15] tx_adapt_g0_en_pcie_gen3
        uint32_t RXINIT_DONE_DELAY_15_0      :16;     ///<BIT [31:16] rxinit_done_delay_15_0
    } b;
} Comphy0SoclocalTxPresetTb5_t;

/// @brief 0xE6F0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GAINTRAIN_C_INDEX_TB_SIZE_PCIE_GEN4_USR_7_0 :8;      ///<BIT [7:0] gaintrain_c_index_tb_size_pcie_gen4_usr_7_0
        uint32_t GAINTRAIN_C_INDEX_TB_SIZE_PCIE_GEN3_USR_7_0 :8;      ///<BIT [15:8] gaintrain_c_index_tb_size_pcie_gen3_usr_7_0
        uint32_t RX_FFE_R_INDEX_TB_SIZE_PCIE_GEN4_USR_7_0 :8;      ///<BIT [23:16] rx_ffe_r_index_tb_size_pcie_gen4_usr_7_0
        uint32_t RX_FFE_R_INDEX_TB_SIZE_PCIE_GEN3_USR_7_0 :8;      ///<BIT [31:24] rx_ffe_r_index_tb_size_pcie_gen3_usr_7_0
    } b;
} Comphy0SoctrainParameter1_t;

/// @brief 0xE6F4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_15                   :16;     ///<BIT [15:0] rsvd_0_15
        uint32_t GAINTRAIN_C_INDEX_TB_SIZE_PCIE_GEN5_USR_7_0 :8;      ///<BIT [23:16] gaintrain_c_index_tb_size_pcie_gen5_usr_7_0
        uint32_t RX_FFE_R_INDEX_TB_SIZE_PCIE_GEN5_USR_7_0 :8;      ///<BIT [31:24] rx_ffe_r_index_tb_size_pcie_gen5_usr_7_0
    } b;
} Comphy0SoctrainParameter2_t;

/// @brief 0xE700
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SELMUPI_GEN5_3_0            :4;      ///<BIT [3:0] selmupi_gen5_3_0
        uint32_t SELMUPF_GEN5_3_0            :4;      ///<BIT [7:4] selmupf_gen5_3_0
        uint32_t SELMUFI_GEN5_2_0            :3;      ///<BIT [10:8] selmufi_gen5_2_0
        uint32_t SELMUFF_GEN5_2_0            :3;      ///<BIT [13:11] selmuff_gen5_2_0
        uint32_t RSVD_14_15                  :2;      ///<BIT [15:14] rsvd_14_15
        uint32_t SELMUPI_GEN4_3_0            :4;      ///<BIT [19:16] selmupi_gen4_3_0
        uint32_t SELMUPF_GEN4_3_0            :4;      ///<BIT [23:20] selmupf_gen4_3_0
        uint32_t SELMUFI_GEN4_2_0            :3;      ///<BIT [26:24] selmufi_gen4_2_0
        uint32_t SELMUFF_GEN4_2_0            :3;      ///<BIT [29:27] selmuff_gen4_2_0
        uint32_t RSVD_30_31                  :2;      ///<BIT [31:30] rsvd_30_31
    } b;
} Comphy0SocrxCdrBw_t;

/// @brief 0xE704
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SELMUPI_GEN3_3_0            :4;      ///<BIT [3:0] selmupi_gen3_3_0
        uint32_t SELMUPF_GEN3_3_0            :4;      ///<BIT [7:4] selmupf_gen3_3_0
        uint32_t SELMUFI_GEN3_2_0            :3;      ///<BIT [10:8] selmufi_gen3_2_0
        uint32_t SELMUFF_GEN3_2_0            :3;      ///<BIT [13:11] selmuff_gen3_2_0
        uint32_t RSVD_14_15                  :2;      ///<BIT [15:14] rsvd_14_15
        uint32_t SELMUPI_GEN2_3_0            :4;      ///<BIT [19:16] selmupi_gen2_3_0
        uint32_t SELMUPF_GEN2_3_0            :4;      ///<BIT [23:20] selmupf_gen2_3_0
        uint32_t SELMUFI_GEN2_2_0            :3;      ///<BIT [26:24] selmufi_gen2_2_0
        uint32_t SELMUFF_GEN2_2_0            :3;      ///<BIT [29:27] selmuff_gen2_2_0
        uint32_t RSVD_30_31                  :2;      ///<BIT [31:30] rsvd_30_31
    } b;
} Comphy0SocrxCdrBw1_t;

/// @brief 0xE708
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SELMUPI_GEN1_3_0            :4;      ///<BIT [3:0] selmupi_gen1_3_0
        uint32_t SELMUPF_GEN1_3_0            :4;      ///<BIT [7:4] selmupf_gen1_3_0
        uint32_t SELMUFI_GEN1_2_0            :3;      ///<BIT [10:8] selmufi_gen1_2_0
        uint32_t SELMUFF_GEN1_2_0            :3;      ///<BIT [13:11] selmuff_gen1_2_0
        uint32_t RSVD_14_31                  :18;     ///<BIT [31:14] rsvd_14_31
    } b;
} Comphy0SocrxCdrBw2_t;

/// @brief 0xE70C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_EM_PRE_CTRL_GEN5_3_0     :4;      ///<BIT [3:0] tx_em_pre_ctrl_gen5_3_0
        uint32_t TX_EM_PEAK_CTRL_GEN5_3_0    :4;      ///<BIT [7:4] tx_em_peak_ctrl_gen5_3_0
        uint32_t TX_EM_PO_CTRL_GEN5_3_0      :4;      ///<BIT [11:8] tx_em_po_ctrl_gen5_3_0
        uint32_t TX_EM_PRE_EN_GEN5           :1;      ///<BIT [12] tx_em_pre_en_gen5
        uint32_t TX_EM_PEAK_EN_GEN5          :1;      ///<BIT [13] tx_em_peak_en_gen5
        uint32_t TX_EM_PO_EN_GEN5            :1;      ///<BIT [14] tx_em_po_en_gen5
        uint32_t RSVD_15                     :1;      ///<BIT [15] rsvd_15
        uint32_t TX_EM_PRE_CTRL_GEN4_3_0     :4;      ///<BIT [19:16] tx_em_pre_ctrl_gen4_3_0
        uint32_t TX_EM_PEAK_CTRL_GEN4_3_0    :4;      ///<BIT [23:20] tx_em_peak_ctrl_gen4_3_0
        uint32_t TX_EM_PO_CTRL_GEN4_3_0      :4;      ///<BIT [27:24] tx_em_po_ctrl_gen4_3_0
        uint32_t TX_EM_PRE_EN_GEN4           :1;      ///<BIT [28] tx_em_pre_en_gen4
        uint32_t TX_EM_PEAK_EN_GEN4          :1;      ///<BIT [29] tx_em_peak_en_gen4
        uint32_t TX_EM_PO_EN_GEN4            :1;      ///<BIT [30] tx_em_po_en_gen4
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Comphy0SoctxFfeCtrl_t;

/// @brief 0xE710
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_EM_PRE_CTRL_GEN3_3_0     :4;      ///<BIT [3:0] tx_em_pre_ctrl_gen3_3_0
        uint32_t TX_EM_PEAK_CTRL_GEN3_3_0    :4;      ///<BIT [7:4] tx_em_peak_ctrl_gen3_3_0
        uint32_t TX_EM_PO_CTRL_GEN3_3_0      :4;      ///<BIT [11:8] tx_em_po_ctrl_gen3_3_0
        uint32_t TX_EM_PRE_EN_GEN3           :1;      ///<BIT [12] tx_em_pre_en_gen3
        uint32_t TX_EM_PEAK_EN_GEN3          :1;      ///<BIT [13] tx_em_peak_en_gen3
        uint32_t TX_EM_PO_EN_GEN3            :1;      ///<BIT [14] tx_em_po_en_gen3
        uint32_t RSVD_15                     :1;      ///<BIT [15] rsvd_15
        uint32_t TX_EM_PRE_CTRL_GEN2_3_0     :4;      ///<BIT [19:16] tx_em_pre_ctrl_gen2_3_0
        uint32_t TX_EM_PEAK_CTRL_GEN2_3_0    :4;      ///<BIT [23:20] tx_em_peak_ctrl_gen2_3_0
        uint32_t TX_EM_PO_CTRL_GEN2_3_0      :4;      ///<BIT [27:24] tx_em_po_ctrl_gen2_3_0
        uint32_t TX_EM_PRE_EN_GEN2           :1;      ///<BIT [28] tx_em_pre_en_gen2
        uint32_t TX_EM_PEAK_EN_GEN2          :1;      ///<BIT [29] tx_em_peak_en_gen2
        uint32_t TX_EM_PO_EN_GEN2            :1;      ///<BIT [30] tx_em_po_en_gen2
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Comphy0SoctxFfeCtrl1_t;

/// @brief 0xE714
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_EM_PRE_CTRL_GEN1_3_0     :4;      ///<BIT [3:0] tx_em_pre_ctrl_gen1_3_0
        uint32_t TX_EM_PEAK_CTRL_GEN1_3_0    :4;      ///<BIT [7:4] tx_em_peak_ctrl_gen1_3_0
        uint32_t TX_EM_PO_CTRL_GEN1_3_0      :4;      ///<BIT [11:8] tx_em_po_ctrl_gen1_3_0
        uint32_t TX_EM_PRE_EN_GEN1           :1;      ///<BIT [12] tx_em_pre_en_gen1
        uint32_t TX_EM_PEAK_EN_GEN1          :1;      ///<BIT [13] tx_em_peak_en_gen1
        uint32_t TX_EM_PO_EN_GEN1            :1;      ///<BIT [14] tx_em_po_en_gen1
        uint32_t RSVD_15_31                  :17;     ///<BIT [31:15] rsvd_15_31
    } b;
} Comphy0SoctxFfeCtrl2_t;

typedef struct
{
    uint8_t rsvd0[16];                                                      // 0x0 : rsvd_0 / rsvd_0
    Comphy0Socuphy14TrxAnaregBot4_t uphy14TrxAnaregBot4;                    // 0x10 : uphy14_trx_anareg_bot_4 / 
    uint8_t rsvd14[56];                                                     // 0x14 : rsvd_14 / rsvd_14
    Comphy0Socuphy14TrxAnaregBot19_t uphy14TrxAnaregBot19;                  // 0x4C : uphy14_trx_anareg_bot_19 / 
    uint8_t rsvd50[20];                                                     // 0x50 : rsvd_50 / rsvd_50
    Comphy0Socuphy14TrxAnaregBot25_t uphy14TrxAnaregBot25;                  // 0x64 : uphy14_trx_anareg_bot_25 / 
    uint8_t rsvd68[416];                                                    // 0x68 : rsvd_68 / rsvd_68
    Comphy0Socuphy14TrxAnaregTop130_t uphy14TrxAnaregTop130;                // 0x208 : uphy14_trx_anareg_top_130 / 
    uint8_t rsvd20c[8];                                                     // 0x20C : rsvd_20c / rsvd_20c
    Comphy0Socuphy14TrxAnaregTop133_t uphy14TrxAnaregTop133;                // 0x214 : uphy14_trx_anareg_top_133 / 
    uint8_t rsvd218[44];                                                    // 0x218 : rsvd_218 / rsvd_218
    Comphy0Socuphy14TrxAnaregTop145_t uphy14TrxAnaregTop145;                // 0x244 : uphy14_trx_anareg_top_145 / 
    uint8_t rsvd248[8];                                                     // 0x248 : rsvd_248 / rsvd_248
    Comphy0Socuphy14TrxAnaregTop148_t uphy14TrxAnaregTop148;                // 0x250 : uphy14_trx_anareg_top_148 / 
    uint8_t rsvd254[7596];                                                  // 0x254 : rsvd_254 / rsvd_254
    Comphy0SocpmCtrlTxLaneReg1Lane_t pmCtrlTxLaneReg1Lane;                  // 0x2000 : pm_ctrl_tx_lane_reg1_lane / 
    Comphy0SocpmCtrlTxLaneReg2Lane_t pmCtrlTxLaneReg2Lane;                  // 0x2004 : pm_ctrl_tx_lane_reg2_lane / 
    Comphy0SocinputTxPinReg0Lane_t inputTxPinReg0Lane;                      // 0x2008 : input_tx_pin_reg0_lane / 
    Comphy0SocinputTxPinReg1Lane_t inputTxPinReg1Lane;                      // 0x200C : input_tx_pin_reg1_lane / 
    Comphy0SocinputTxPinReg2Lane_t inputTxPinReg2Lane;                      // 0x2010 : input_tx_pin_reg2_lane / 
    Comphy0SocinputTxPinReg3Lane_t inputTxPinReg3Lane;                      // 0x2014 : input_tx_pin_reg3_lane / 
    uint8_t rsvd2018[8];                                                    // 0x2018 : rsvd_2018 / rsvd_2018
    Comphy0SocclkgenTxLaneReg1Lane_t clkgenTxLaneReg1Lane;                  // 0x2020 : clkgen_tx_lane_reg1_lane / 
    Comphy0SoctxSpeedConvertLane_t txSpeedConvertLane;                      // 0x2024 : tx_speed_convert_lane / 
    uint8_t rsvd2028[8];                                                    // 0x2028 : rsvd_2028 / rsvd_2028
    Comphy0SocspdCtrlTxLaneReg1Lane_t spdCtrlTxLaneReg1Lane;                // 0x2030 : spd_ctrl_tx_lane_reg1_lane / 
    Comphy0SoctxSystemLane_t txSystemLane;                                  // 0x2034 : tx_system_lane / 
    uint8_t rsvd2038[36];                                                   // 0x2038 : rsvd_2038 / rsvd_2038
    Comphy0SocmonTop_t monTop;                                              // 0x205C : mon_top / 
    uint8_t rsvd2060[160];                                                  // 0x2060 : rsvd_2060 / rsvd_2060
    Comphy0SocpmCtrlRxLaneReg1Lane_t pmCtrlRxLaneReg1Lane;                  // 0x2100 : pm_ctrl_rx_lane_reg1_lane / 
    Comphy0SocrxSystemLane_t rxSystemLane;                                  // 0x2104 : rx_system_lane / 
    Comphy0SocinputRxPinReg0Lane_t inputRxPinReg0Lane;                      // 0x2108 : input_rx_pin_reg0_lane / 
    Comphy0SocinputRxPinReg1Lane_t inputRxPinReg1Lane;                      // 0x210C : input_rx_pin_reg1_lane / 
    Comphy0SocinputRxPinReg2Lane_t inputRxPinReg2Lane;                      // 0x2110 : input_rx_pin_reg2_lane / 
    uint8_t rsvd2114[8];                                                    // 0x2114 : rsvd_2114 / rsvd_2114
    Comphy0SocclkgenRxLaneReg1Lane_t clkgenRxLaneReg1Lane;                  // 0x211C : clkgen_rx_lane_reg1_lane / 
    Comphy0SocframeSyncDetReg0_t frameSyncDetReg0;                          // 0x2120 : frame_sync_det_reg0 / 
    Comphy0SocframeSyncDetReg1_t frameSyncDetReg1;                          // 0x2124 : frame_sync_det_reg1 / 
    uint8_t rsvd2128[20];                                                   // 0x2128 : rsvd_2128 / rsvd_2128
    Comphy0SoccdrLock_t cdrLock;                                            // 0x213C : cdr_lock_reg / 
    uint8_t rsvd2140[8];                                                    // 0x2140 : rsvd_2140 / rsvd_2140
    Comphy0SocrxDataPath_t rxDataPath;                                      // 0x2148 : rx_data_path_reg / 
    uint8_t rsvd214c[20];                                                   // 0x214C : rsvd_214c / rsvd_214c
    Comphy0SocdtlReg0_t dtlReg0;                                            // 0x2160 : dtl_reg0 / 
    Comphy0SocdtlReg1_t dtlReg1;                                            // 0x2164 : dtl_reg1 / 
    Comphy0SocdtlReg2_t dtlReg2;                                            // 0x2168 : dtl_reg2 / 
    uint8_t rsvd216c[4];                                                    // 0x216C : rsvd_216c / rsvd_216c
    Comphy0SocsqReg0_t sqReg0;                                              // 0x2170 : sq_reg0 / 
    uint8_t rsvd2174[140];                                                  // 0x2174 : rsvd_2174 / rsvd_2174
    Comphy0SocmcuControlLane_t mcuControlLane;                              // 0x2200 : mcu_control_lane / 
    uint8_t rsvd2204[12];                                                   // 0x2204 : rsvd_2204 / rsvd_2204
    Comphy0SoclaneSystem0_t laneSystem0;                                    // 0x2210 : lane_system0 / 
    uint8_t rsvd2214[128];                                                  // 0x2214 : rsvd_2214 / rsvd_2214
    Comphy0SocmcuMemReg2Lane_t mcuMemReg2Lane;                              // 0x2294 : mcu_mem_reg2_lane / 
    uint8_t rsvd2298[76];                                                   // 0x2298 : rsvd_2298 / rsvd_2298
    Comphy0SocmcuCommand0_t mcuCommand0;                                    // 0x22E4 : mcu_command0 / 
    uint8_t rsvd22e8[12];                                                   // 0x22E8 : rsvd_22e8 / rsvd_22e8
    Comphy0SocmemEccErrAddress0_t memEccErrAddress0;                        // 0x22F4 : mem_ecc_err_address0 / 
    uint32_t xdataMemChecksumLane0XdataMemChecksumExpLane310;               // 0x22F8 : xdata_mem_checksum_lane0 / 
    uint32_t xdataMemChecksumLane1XdataMemChecksumLane310;                  // 0x22FC : xdata_mem_checksum_lane1 / 
    Comphy0SocptControl0_t ptControl0;                                      // 0x2300 : pt_control0 / 
    Comphy0SocptControl1_t ptControl1;                                      // 0x2304 : pt_control1 / 
    uint32_t ptUserPattern0PtUserPatternLane7948;                           // 0x2308 : pt_user_pattern0 / 
    uint32_t ptUserPattern1PtUserPatternLane4716;                           // 0x230C : pt_user_pattern1 / 
    Comphy0SocptUserPattern2_t ptUserPattern2;                              // 0x2310 : pt_user_pattern2 / 
    uint32_t ptCounter0PtCntLane4716;                                       // 0x2314 : pt_counter0 / 
    Comphy0SocptCounter1_t ptCounter1;                                      // 0x2318 : pt_counter1 / 
    uint32_t ptCounter2PtErrCntLane310;                                     // 0x231C : pt_counter2 / 
    uint8_t rsvd2320[232];                                                  // 0x2320 : rsvd_2320 / rsvd_2320
    Comphy0SocdfeCtrlReg2_t dfeCtrlReg2;                                    // 0x2408 : dfe_ctrl_reg2 / 
    uint8_t rsvd240c[4];                                                    // 0x240C : rsvd_240c / rsvd_240c
    Comphy0SocrxEqClkCtrl_t rxEqClkCtrl;                                    // 0x2410 : rx_eq_clk_ctrl / 
    uint8_t rsvd2414[124];                                                  // 0x2414 : rsvd_2414 / rsvd_2414
    Comphy0SocdfeReadEvenSmReg4_t dfeReadEvenSmReg4;                        // 0x2490 : dfe_read_even_sm_reg4 / 
    uint8_t rsvd2494[12];                                                   // 0x2494 : rsvd_2494 / rsvd_2494
    Comphy0SocdfeReadOddSmReg0_t dfeReadOddSmReg0;                          // 0x24A0 : dfe_read_odd_sm_reg0 / 
    Comphy0SocdfeReadOddSmReg1_t dfeReadOddSmReg1;                          // 0x24A4 : dfe_read_odd_sm_reg1 / 
    Comphy0SocdfeReadOddSmReg2_t dfeReadOddSmReg2;                          // 0x24A8 : dfe_read_odd_sm_reg2 / 
    Comphy0SocdfeReadOddSmReg3_t dfeReadOddSmReg3;                          // 0x24AC : dfe_read_odd_sm_reg3 / 
    Comphy0SocdfeReadOddSmReg4_t dfeReadOddSmReg4;                          // 0x24B0 : dfe_read_odd_sm_reg4 / 
    uint8_t rsvd24b4[140];                                                  // 0x24B4 : rsvd_24b4 / rsvd_24b4
    Comphy0SocdfeStaticLaneReg0_t dfeStaticLaneReg0;                        // 0x2540 : dfe_static_lane_reg0 / 
    uint8_t rsvd2544[60];                                                   // 0x2544 : rsvd_2544 / rsvd_2544
    Comphy0SoceomReg0_t eomReg0;                                            // 0x2580 : eom_reg0 / 
    uint8_t rsvd2584[124];                                                  // 0x2584 : rsvd_2584 / rsvd_2584
    Comphy0SocdmeEncReg0_t dmeEncReg0;                                      // 0x2600 : dme_enc_reg0 / 
    Comphy0SocdmeEncReg1_t dmeEncReg1;                                      // 0x2604 : dme_enc_reg1 / 
    Comphy0SocdmeEncReg2_t dmeEncReg2;                                      // 0x2608 : dme_enc_reg2 / 
    Comphy0SocdmeDecReg0_t dmeDecReg0;                                      // 0x260C : dme_dec_reg0 / 
    Comphy0SocdmeDecReg1_t dmeDecReg1;                                      // 0x2610 : dme_dec_reg1 / 
    Comphy0SoctxTrainIfReg0_t txTrainIfReg0;                                // 0x2614 : tx_train_if_reg0 / 
    Comphy0SoctxTrainIfReg1_t txTrainIfReg1;                                // 0x2618 : tx_train_if_reg1 / 
    Comphy0SoctxTrainIfReg2_t txTrainIfReg2;                                // 0x261C : tx_train_if_reg2 / 
    Comphy0SoctxTrainIfReg3_t txTrainIfReg3;                                // 0x2620 : tx_train_if_reg3 / 
    Comphy0SoctxTrainPattternReg0_t txTrainPattternReg0;                    // 0x2624 : tx_train_patttern_reg0 / 
    Comphy0SoctxTrainDriverReg0_t txTrainDriverReg0;                        // 0x2628 : tx_train_driver_reg0 / 
    Comphy0SoctxTrainDriverReg1_t txTrainDriverReg1;                        // 0x262C : tx_train_driver_reg1 / 
    Comphy0SoctxTrainDriverReg2_t txTrainDriverReg2;                        // 0x2630 : tx_train_driver_reg2 / 
    uint8_t rsvd2634[24];                                                   // 0x2634 : rsvd_2634 / rsvd_2634
    Comphy0SoctxEmphCtrlReg0_t txEmphCtrlReg0;                              // 0x264C : tx_emph_ctrl_reg0 / 
    Comphy0SoclinkTrainMode0_t linkTrainMode0;                              // 0x2650 : link_train_mode0 / 
    uint8_t rsvd2654[8];                                                    // 0x2654 : rsvd_2654 / rsvd_2654
    Comphy0SoctrxTrainIfIntrLane_t trxTrainIfIntrLane;                      // 0x265C : trx_train_if_interrupt_lane / 
    Comphy0SoctrxTrainIfIntrMask0Lane_t trxTrainIfIntrMask0Lane;            // 0x2660 : trx_train_if_interrupt_mask0_lane / 
    Comphy0SoctrxTrainIfIntrClearLane_t trxTrainIfIntrClearLane;            // 0x2664 : trx_train_if_interrupt_clear_lane / 
    uint8_t rsvd2668[16];                                                   // 0x2668 : rsvd_2668 / rsvd_2668
    Comphy0SoctxTrainCtrlLane_t txTrainCtrlLane;                            // 0x2678 : tx_train_ctrl_lane / 
    Comphy0SoctxTrainIfReg8_t txTrainIfReg8;                                // 0x267C : tx_train_if_reg8 / 
    uint8_t rsvd2680[6528];                                                 // 0x2680 : rsvd_2680 / rsvd_2680
    Comphy0SoclaneCfg0_t laneCfg0;                                          // 0x4000 : lane_cfg0 / 
    Comphy0SoclaneStatus0_t laneStatus0;                                    // 0x4004 : lane_status0 / 
    Comphy0SoclaneCfgStatus2Lane_t laneCfgStatus2Lane;                      // 0x4008 : lane_cfg_status2_lane / 
    Comphy0SoclaneCfg2Lane_t laneCfg2Lane;                                  // 0x400C : lane_cfg2_lane / 
    Comphy0SoclaneCfg4_t laneCfg4;                                          // 0x4010 : lane_cfg4 / 
    Comphy0SoclaneCfgStatus3Lane_t laneCfgStatus3Lane;                      // 0x4014 : lane_cfg_status3_lane / 
    Comphy0SoclaneDpPie8Cfg0Lane_t laneDpPie8Cfg0Lane;                      // 0x4018 : lane_dp_pie8_cfg0_lane / 
    uint8_t rsvd401c[8];                                                    // 0x401C : rsvd_401c / rsvd_401c
    Comphy0SoclaneEqCfg0Lane_t laneEqCfg0Lane;                              // 0x4024 : lane_eq_cfg0_lane / 
    Comphy0SoclaneEqCfg1Lane_t laneEqCfg1Lane;                              // 0x4028 : lane_eq_cfg1_lane / 
    uint8_t rsvd402c[8];                                                    // 0x402C : rsvd_402c / rsvd_402c
    Comphy0SoclanePresetCfg4Lane_t lanePresetCfg4Lane;                      // 0x4034 : lane_preset_cfg4_lane / 
    uint8_t rsvd4038[20];                                                   // 0x4038 : rsvd_4038 / rsvd_4038
    Comphy0SoclanePresetCfg16Lane_t lanePresetCfg16Lane;                    // 0x404C : lane_preset_cfg16_lane / 
    Comphy0SoclaneCoeffMax0Lane_t laneCoeffMax0Lane;                        // 0x4050 : lane_coeff_max0_lane / 
    Comphy0SoclaneRemoteSetLane_t laneRemoteSetLane;                        // 0x4054 : lane_remote_set_lane / 
    Comphy0SoclaneEq16gCfg0Lane_t laneEq16gCfg0Lane;                        // 0x4058 : lane_eq_16g_cfg0_lane / 
    uint8_t rsvd405c[36];                                                   // 0x405C : rsvd_405c / rsvd_405c
    Comphy0SoclaneEq32gCfg0Lane_t laneEq32gCfg0Lane;                        // 0x4080 : lane_eq_32g_cfg0_lane / 
    uint8_t rsvd4084[380];                                                  // 0x4084 : rsvd_4084 / rsvd_4084
    Comphy0SocglobRstClkCtrl_t globRstClkCtrl;                              // 0x4200 : glob_rst_clk_ctrl / 
    Comphy0SocglobClkSrcLo_t globClkSrcLo;                                  // 0x4204 : glob_clk_src_lo / 
    Comphy0SocglobClkSrcHi_t globClkSrcHi;                                  // 0x4208 : glob_clk_src_hi / 
    Comphy0SocglobMiscCtrl_t globMiscCtrl;                                  // 0x420C : glob_misc_ctrl / 
    Comphy0SocglobDpSalCfg_t globDpSalCfg;                                  // 0x4210 : glob_dp_sal_cfg / 
    Comphy0SocglobDpSalCfg1_t globDpSalCfg1;                                // 0x4214 : glob_dp_sal_cfg1 / 
    Comphy0SocglobDpSalCfg3_t globDpSalCfg3;                                // 0x4218 : glob_dp_sal_cfg3 / 
    Comphy0SocglobProtocolCfg0_t globProtocolCfg0;                          // 0x421C : glob_protocol_cfg0 / 
    Comphy0SocglobPmCfg0_t globPmCfg0;                                      // 0x4220 : glob_pm_cfg0 / 
    Comphy0SocglobCounterCtrl_t globCounterCtrl;                            // 0x4224 : glob_counter_ctrl / 
    Comphy0SocglobCounterHi_t globCounterHi;                                // 0x4228 : glob_counter_hi / 
    Comphy0SocglobPmDpCtrl_t globPmDpCtrl;                                  // 0x422C : glob_pm_dp_ctrl / 
    Comphy0SocglobDpBalCfg0_t globDpBalCfg0;                                // 0x4230 : glob_dp_bal_cfg0 / 
    Comphy0SocglobDpBalCfg2_t globDpBalCfg2;                                // 0x4234 : glob_dp_bal_cfg2 / 
    Comphy0SocglobDpBalCfg4_t globDpBalCfg4;                                // 0x4238 : glob_dp_bal_cfg4 / 
    Comphy0SocglobBistCtrl_t globBistCtrl;                                  // 0x423C : glob_bist_ctrl / 
    Comphy0SocglobBistLaneType_t globBistLaneType;                          // 0x4240 : glob_bist_lane_type / 
    Comphy0SocglobBistStart_t globBistStart;                                // 0x4244 : glob_bist_start / 
    Comphy0SocglobBistMask_t globBistMask;                                  // 0x4248 : glob_bist_mask / 
    Comphy0SocglobBistResult_t globBistResult;                              // 0x424C : glob_bist_result / 
    Comphy0SocglobBistSeqrCfg_t globBistSeqrCfg;                            // 0x4250 : glob_bist_seqr_cfg / 
    Comphy0SocglobBistDataHi_t globBistDataHi;                              // 0x4254 : glob_bist_data_hi / 
    Comphy0SocglobBistLinkEq_t globBistLinkEq;                              // 0x4258 : glob_bist_link_eq / 
    Comphy0SocglobBistLaneMargin_t globBistLaneMargin;                      // 0x425C : glob_bist_lane_margin / 
    Comphy0SocglobPipeRevision_t globPipeRevision;                          // 0x4260 : glob_pipe_revision / 
    Comphy0SocglobL1SubstatesCfg_t globL1SubstatesCfg;                      // 0x4264 : glob_l1_substates_cfg / 
    uint8_t rsvd4268[5816];                                                 // 0x4268 : rsvd_4268 / rsvd_4268
    Comphy0SocinputPinDebugPipeReg8_t inputPinDebugPipeReg8;                // 0x5920 : input_pin_debug_pipe_reg8 / 
    uint8_t rsvd5924[4];                                                    // 0x5924 : rsvd_5924 / rsvd_5924
    Comphy0SocinputPinDebugPipeReg10_t inputPinDebugPipeReg10;              // 0x5928 : input_pin_debug_pipe_reg10 / 
    Comphy0SocinputPinDebugPipeReg11_t inputPinDebugPipeReg11;              // 0x592C : input_pin_debug_pipe_reg11 / 
    Comphy0SocinputPinDebugPipeReg12_t inputPinDebugPipeReg12;              // 0x5930 : input_pin_debug_pipe_reg12 / 
    uint8_t rsvd5934[1740];                                                 // 0x5934 : rsvd_5934 / rsvd_5934
    Comphy0SoccalCtrl1Lane_t calCtrl1Lane;                                  // 0x6000 : cal_ctrl1_lane / 
    uint8_t rsvd6004[4];                                                    // 0x6004 : rsvd_6004 / rsvd_6004
    Comphy0SoccalCtrl3Lane_t calCtrl3Lane;                                  // 0x6008 : cal_ctrl3_lane / 
    Comphy0SoccalCtrl4Lane_t calCtrl4Lane;                                  // 0x600C : cal_ctrl4_lane / 
    uint8_t rsvd6010[4];                                                    // 0x6010 : rsvd_6010 / rsvd_6010
    Comphy0SoccalSaveData2Lane_t calSaveData2Lane;                          // 0x6014 : cal_save_data2_lane / 
    uint8_t rsvd6018[4];                                                    // 0x6018 : rsvd_6018 / rsvd_6018
    Comphy0SocphyRemoteCtrlCommandLane_t phyRemoteCtrlCommandLane;          // 0x601C : phy_remote_ctrl_command_lane / 
    uint32_t phyRemoteCtrlValueLanePhyRemoteCtrlValueLane310;               // 0x6020 : phy_remote_ctrl_value_lane / 
    uint32_t phyLocalValueLanePhyLocalValueLane310;                         // 0x6024 : phy_local_value_lane / 
    Comphy0SoctrxTrainIfTimers1Lane_t trxTrainIfTimers1Lane;                // 0x6028 : trx_train_if_timers1_lane / 
    Comphy0SoctrxTrainIfTimers2Lane_t trxTrainIfTimers2Lane;                // 0x602C : trx_train_if_timers2_lane / 
    Comphy0SoctrxTrainIfTimersEnableLane_t trxTrainIfTimersEnableLane;      // 0x6030 : trx_train_if_timers_enable_lane / 
    uint8_t rsvd6034[4];                                                    // 0x6034 : rsvd_6034 / rsvd_6034
    Comphy0SocdfeControl1_t dfeControl1;                                    // 0x6038 : dfe_control_1 / 
    uint8_t rsvd603c[8];                                                    // 0x603C : rsvd_603c / rsvd_603c
    Comphy0SocdfeControl3_t dfeControl3;                                    // 0x6044 : dfe_control_3 / 
    uint8_t rsvd6048[4];                                                    // 0x6048 : rsvd_6048 / rsvd_6048
    Comphy0SocdfeControl5_t dfeControl5;                                    // 0x604C : dfe_control_5 / 
    uint8_t rsvd6050[8];                                                    // 0x6050 : rsvd_6050 / rsvd_6050
    Comphy0SoctrainControl2_t trainControl2;                                // 0x6058 : train_control_2 / 
    uint8_t rsvd605c[8];                                                    // 0x605C : rsvd_605c / rsvd_605c
    Comphy0SocdllCal_t dllCal;                                              // 0x6064 : dll_cal / 
    uint8_t rsvd6068[4];                                                    // 0x6068 : rsvd_6068 / rsvd_6068
    Comphy0SoctrainPara1_t trainPara1;                                      // 0x606C : train_para_1 / 
    Comphy0SoctrainPara2_t trainPara2;                                      // 0x6070 : train_para_2 / 
    Comphy0SoctrainPara3_t trainPara3;                                      // 0x6074 : train_para_3 / 
    Comphy0SocdfeControl6_t dfeControl6;                                    // 0x6078 : dfe_control_6 / 
    uint8_t rsvd607c[4];                                                    // 0x607C : rsvd_607c / rsvd_607c
    Comphy0SocdfeTest1_t dfeTest1;                                          // 0x6080 : dfe_test_1 / 
    uint8_t rsvd6084[8];                                                    // 0x6084 : rsvd_6084 / rsvd_6084
    Comphy0SocdfeControl7_t dfeControl7;                                    // 0x608C : dfe_control_7 / 
    uint8_t rsvd6090[16];                                                   // 0x6090 : rsvd_6090 / rsvd_6090
    Comphy0SoccdsCtrlReg0_t cdsCtrlReg0;                                    // 0x60A0 : cds_ctrl_reg0 / 
    Comphy0SoccdsCtrlReg1_t cdsCtrlReg1;                                    // 0x60A4 : cds_ctrl_reg1 / 
    uint32_t esmPopPCntLowLaneEomPopPCntLane310;                            // 0x60A8 : esm_pop_p_cnt_low_lane / 
    uint32_t esmErrPCntLowLaneEomErrPCntLane310;                            // 0x60AC : esm_err_p_cnt_low_lane / 
    Comphy0SocesmErrPopCntHighLane_t esmErrPopCntHighLane;                  // 0x60B0 : esm_err_pop_cnt_high_lane / 
    uint8_t rsvd60b4[40];                                                   // 0x60B4 : rsvd_60b4 / rsvd_60b4
    uint32_t esmPopNCntLowLaneEomPopNCntLane310;                            // 0x60DC : esm_pop_n_cnt_low_lane / 
    uint32_t esmErrNCntLowLaneEomErrNCntLane310;                            // 0x60E0 : esm_err_n_cnt_low_lane / 
    uint8_t rsvd60e4[356];                                                  // 0x60E4 : rsvd_60e4 / rsvd_60e4
    Comphy0SocdfeResetOverwrite_t dfeResetOverwrite;                        // 0x6248 : dfe_reset_overwrite / 
    uint8_t rsvd624c[2680];                                                 // 0x624C : rsvd_624c / rsvd_624c
    Comphy0Socautospeed424_t autospeed424;                                  // 0x6CC4 : autospeed424 / 
    Comphy0Socautospeed425_t autospeed425;                                  // 0x6CC8 : autospeed425 / 
    uint8_t rsvd6ccc[68];                                                   // 0x6CCC : rsvd_6ccc / rsvd_6ccc
    Comphy0Socautospeed443_t autospeed443;                                  // 0x6D10 : autospeed443 / 
    Comphy0Socautospeed444_t autospeed444;                                  // 0x6D14 : autospeed444 / 
    Comphy0Socautospeed445_t autospeed445;                                  // 0x6D18 : autospeed445 / 
    uint8_t rsvd6d1c[72];                                                   // 0x6D1C : rsvd_6d1c / rsvd_6d1c
    Comphy0Socautospeed464_t autospeed464;                                  // 0x6D64 : autospeed464 / 
    Comphy0Socautospeed465_t autospeed465;                                  // 0x6D68 : autospeed465 / 
    uint8_t rsvd6d6c[5272];                                                 // 0x6D6C : rsvd_6d6c / rsvd_6d6c
    Comphy0Socuphy14CmnAnaregTop129_t uphy14CmnAnaregTop129;                // 0x8204 : uphy14_cmn_anareg_top_129 / 
    uint8_t rsvd8208[32];                                                   // 0x8208 : rsvd_8208 / rsvd_8208
    Comphy0Socuphy14CmnAnaregTop138_t uphy14CmnAnaregTop138;                // 0x8228 : uphy14_cmn_anareg_top_138 / 
    uint8_t rsvd822c[284];                                                  // 0x822C : rsvd_822c / rsvd_822c
    Comphy0Socuphy14CmnAnaregTop210_t uphy14CmnAnaregTop210;                // 0x8348 : uphy14_cmn_anareg_top_210 / 
    uint8_t rsvd834c[7356];                                                 // 0x834C : rsvd_834c / rsvd_834c
    Comphy0SocdtxReg0_t dtxReg0;                                            // 0xA008 : dtx_reg0 / 
    uint8_t rsvdA00c[4];                                                    // 0xA00C : rsvd_a00c / rsvd_a00c
    Comphy0SocdtxReg2_t dtxReg2;                                            // 0xA010 : dtx_reg2 / 
    uint8_t rsvdA014[4];                                                    // 0xA014 : rsvd_a014 / rsvd_a014
    Comphy0SocdtxReg4_t dtxReg4;                                            // 0xA018 : dtx_reg4 / 
    Comphy0SocdtxPhyAlignReg0_t dtxPhyAlignReg0;                            // 0xA01C : dtx_phy_align_reg0 / 
    uint8_t rsvdA020[4];                                                    // 0xA020 : rsvd_a020 / rsvd_a020
    Comphy0SocdtxPhyAlignReg1_t dtxPhyAlignReg1;                            // 0xA024 : dtx_phy_align_reg1 / 
    Comphy0SocdtxReg5_t dtxReg5;                                            // 0xA028 : dtx_reg5 / 
    Comphy0SocsrisReg0_t srisReg0;                                          // 0xA02C : sris_reg0 / 
    Comphy0SocsrisReg1_t srisReg1;                                          // 0xA030 : sris_reg1 / 
    uint8_t rsvdA034[460];                                                  // 0xA034 : rsvd_a034 / rsvd_a034
    Comphy0SocmcuControl0_t mcuControl0;                                    // 0xA200 : mcu_control_0 / 
    Comphy0SocmcuControl1_t mcuControl1;                                    // 0xA204 : mcu_control_1 / 
    Comphy0SocmcuControl2_t mcuControl2;                                    // 0xA208 : mcu_control_2 / 
    uint8_t rsvdA20c[16];                                                   // 0xA20C : rsvd_a20c / rsvd_a20c
    Comphy0SocmemoryControl0_t memoryControl0;                              // 0xA21C : memory_control_0 / 
    uint8_t rsvdA220[4];                                                    // 0xA220 : rsvd_a220 / rsvd_a220
    uint32_t memoryControl2PmemChecksumExp310;                              // 0xA224 : memory_control_2 / 
    uint32_t memoryControl3PmemChecksum310;                                 // 0xA228 : memory_control_3 / 
    Comphy0SocmemoryControl4_t memoryControl4;                              // 0xA22C : memory_control_4 / 
    uint8_t rsvdA230[20];                                                   // 0xA230 : rsvd_a230 / rsvd_a230
    Comphy0SocmemCmnEccErrAddress0_t memCmnEccErrAddress0;                  // 0xA244 : mem_cmn_ecc_err_address0 / 
    uint8_t rsvdA248[168];                                                  // 0xA248 : rsvd_a248 / rsvd_a248
    Comphy0SocapbControl_t apbControl;                                      // 0xA2F0 : apb_control_reg / 
    uint8_t rsvdA2f4[12];                                                   // 0xA2F4 : rsvd_a2f4 / rsvd_a2f4
    Comphy0Soctest0_t test0;                                                // 0xA300 : test0 / 
    uint8_t rsvdA304[4];                                                    // 0xA304 : rsvd_a304 / rsvd_a304
    Comphy0Soctest2_t test2;                                                // 0xA308 : test2 / 
    Comphy0Soctest3_t test3;                                                // 0xA30C : test3 / 
    Comphy0Soctest4_t test4;                                                // 0xA310 : test4 / 
    Comphy0Socsystem_t system;                                              // 0xA314 : system / 
    Comphy0SocpmCmnReg1_t pmCmnReg1;                                        // 0xA318 : pm_cmn_reg1 / 
    Comphy0SocinputCmnPinReg0_t inputCmnPinReg0;                            // 0xA31C : input_cmn_pin_reg0 / 
    Comphy0SocinputCmnPinReg1_t inputCmnPinReg1;                            // 0xA320 : input_cmn_pin_reg1 / 
    Comphy0SocinputCmnPinReg2_t inputCmnPinReg2;                            // 0xA324 : input_cmn_pin_reg2 / 
    uint8_t rsvdA328[8];                                                    // 0xA328 : rsvd_a328 / rsvd_a328
    Comphy0SocpllcalReg1_t pllcalReg1;                                      // 0xA330 : pllcal_reg1 / 
    Comphy0SocclkgenCmnReg1_t clkgenCmnReg1;                                // 0xA334 : clkgen_cmn_reg1 / 
    Comphy0SocspdCmnReg1_t spdCmnReg1;                                      // 0xA338 : spd_cmn_reg1 / 
    Comphy0SocoutputCmnPinReg0_t outputCmnPinReg0;                          // 0xA33C : output_cmn_pin_reg0 / 
    uint8_t rsvdA340[24];                                                   // 0xA340 : rsvd_a340 / rsvd_a340
    uint32_t xdataMemChecksumCmn0XdataMemChecksumExpCmn310;                 // 0xA358 : xdata_mem_checksum_cmn_0 / 
    uint32_t xdataMemChecksumCmn1XdataMemChecksumCmn310;                    // 0xA35C : xdata_mem_checksum_cmn_1 / 
    Comphy0SocxdataMemChecksumCmn2_t xdataMemChecksumCmn2;                  // 0xA360 : xdata_mem_checksum_cmn_2 / 
    Comphy0SocmcuSdtCmn_t mcuSdtCmn;                                        // 0xA364 : mcu_sdt_cmn / 
    uint8_t rsvdA368[76];                                                   // 0xA368 : rsvd_a368 / rsvd_a368
    Comphy0SocsetLaneIsr_t setLaneIsr;                                      // 0xA3B4 : set_lane_isr / 
    uint8_t rsvdA3b8[60];                                                   // 0xA3B8 : rsvd_a3b8 / rsvd_a3b8
    Comphy0SoccmnMcu_t cmnMcu;                                              // 0xA3F4 : cmn_mcu_reg / 
    Comphy0SoccidReg0_t cidReg0;                                            // 0xA3F8 : cid_reg0 / 
    Comphy0SoccidReg1_t cidReg1;                                            // 0xA3FC : cid_reg1 / 
    uint8_t rsvdA400[15996];                                                // 0xA400 : rsvd_a400 / rsvd_a400
    Comphy0Socautospeed159_t autospeed159;                                  // 0xE27C : autospeed159 / 
    uint8_t rsvdE280[896];                                                  // 0xE280 : rsvd_e280 / rsvd_e280
    Comphy0SocfwRev_t fwRev;                                                // 0xE600 : fw_rev / 
    Comphy0SoccontrolConfig0_t controlConfig0;                              // 0xE604 : control_config0 / 
    uint8_t rsvdE608[24];                                                   // 0xE608 : rsvd_e608 / rsvd_e608
    Comphy0SoccontrolConfig7_t controlConfig7;                              // 0xE620 : control_config7 / 
    Comphy0SoccalData0_t calData0;                                          // 0xE624 : cal_data0 / 
    Comphy0SoctrainIfConfig_t trainIfConfig;                                // 0xE628 : train_if_config / 
    Comphy0SoccontrolConfig8_t controlConfig8;                              // 0xE62C : control_config8 / 
    Comphy0SoccontrolConfig9_t controlConfig9;                              // 0xE630 : control_config9 / 
    uint8_t rsvdE634[28];                                                   // 0xE634 : rsvd_e634 / rsvd_e634
    Comphy0SocmcuConfig_t mcuConfig;                                        // 0xE650 : mcu_config / 
    uint8_t rsvdE654[8];                                                    // 0xE654 : rsvd_e654 / rsvd_e654
    Comphy0SocmcuConfig1_t mcuConfig1;                                      // 0xE65C : mcu_config1 / 
    uint8_t rsvdE660[100];                                                  // 0xE660 : rsvd_e660 / rsvd_e660
    Comphy0SoclocalTxPresetTb5_t localTxPresetTb5;                          // 0xE6C4 : local_tx_preset_tb5 / 
    uint8_t rsvdE6c8[40];                                                   // 0xE6C8 : rsvd_e6c8 / rsvd_e6c8
    Comphy0SoctrainParameter1_t trainParameter1;                            // 0xE6F0 : train_parameter_1 / 
    Comphy0SoctrainParameter2_t trainParameter2;                            // 0xE6F4 : train_parameter_2 / 
    uint8_t rsvdE6f8[8];                                                    // 0xE6F8 : rsvd_e6f8 / rsvd_e6f8
    Comphy0SocrxCdrBw_t rxCdrBw;                                            // 0xE700 : rx_cdr_bw / 
    Comphy0SocrxCdrBw1_t rxCdrBw1;                                          // 0xE704 : rx_cdr_bw_1 / 
    Comphy0SocrxCdrBw2_t rxCdrBw2;                                          // 0xE708 : rx_cdr_bw_2 / 
    Comphy0SoctxFfeCtrl_t txFfeCtrl;                                        // 0xE70C : tx_ffe_ctrl / 
    Comphy0SoctxFfeCtrl1_t txFfeCtrl1;                                      // 0xE710 : tx_ffe_ctrl_1 / 
    Comphy0SoctxFfeCtrl2_t txFfeCtrl2;                                      // 0xE714 : tx_ffe_ctrl_2 / 
} Comphy0Soc_t;

COMPILE_ASSERT(offsetof(Comphy0Soc_t,uphy14TrxAnaregBot4)==0x10,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,uphy14TrxAnaregBot19)==0x4C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,uphy14TrxAnaregBot25)==0x64,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,uphy14TrxAnaregTop130)==0x208,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,uphy14TrxAnaregTop133)==0x214,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,uphy14TrxAnaregTop145)==0x244,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,uphy14TrxAnaregTop148)==0x250,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,pmCtrlTxLaneReg1Lane)==0x2000,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,pmCtrlTxLaneReg2Lane)==0x2004,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,inputTxPinReg0Lane)==0x2008,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,inputTxPinReg1Lane)==0x200C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,inputTxPinReg2Lane)==0x2010,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,inputTxPinReg3Lane)==0x2014,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,clkgenTxLaneReg1Lane)==0x2020,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txSpeedConvertLane)==0x2024,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,spdCtrlTxLaneReg1Lane)==0x2030,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txSystemLane)==0x2034,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,monTop)==0x205C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,pmCtrlRxLaneReg1Lane)==0x2100,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,rxSystemLane)==0x2104,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,inputRxPinReg0Lane)==0x2108,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,inputRxPinReg1Lane)==0x210C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,inputRxPinReg2Lane)==0x2110,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,clkgenRxLaneReg1Lane)==0x211C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,frameSyncDetReg0)==0x2120,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,frameSyncDetReg1)==0x2124,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,cdrLock)==0x213C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,rxDataPath)==0x2148,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dtlReg0)==0x2160,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dtlReg1)==0x2164,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dtlReg2)==0x2168,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,sqReg0)==0x2170,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,mcuControlLane)==0x2200,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,laneSystem0)==0x2210,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,mcuMemReg2Lane)==0x2294,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,mcuCommand0)==0x22E4,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,memEccErrAddress0)==0x22F4,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,xdataMemChecksumLane0XdataMemChecksumExpLane310)==0x22F8,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,xdataMemChecksumLane1XdataMemChecksumLane310)==0x22FC,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,ptControl0)==0x2300,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,ptControl1)==0x2304,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,ptUserPattern0PtUserPatternLane7948)==0x2308,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,ptUserPattern1PtUserPatternLane4716)==0x230C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,ptUserPattern2)==0x2310,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,ptCounter0PtCntLane4716)==0x2314,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,ptCounter1)==0x2318,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,ptCounter2PtErrCntLane310)==0x231C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeCtrlReg2)==0x2408,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,rxEqClkCtrl)==0x2410,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeReadEvenSmReg4)==0x2490,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeReadOddSmReg0)==0x24A0,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeReadOddSmReg1)==0x24A4,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeReadOddSmReg2)==0x24A8,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeReadOddSmReg3)==0x24AC,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeReadOddSmReg4)==0x24B0,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeStaticLaneReg0)==0x2540,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,eomReg0)==0x2580,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dmeEncReg0)==0x2600,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dmeEncReg1)==0x2604,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dmeEncReg2)==0x2608,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dmeDecReg0)==0x260C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dmeDecReg1)==0x2610,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txTrainIfReg0)==0x2614,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txTrainIfReg1)==0x2618,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txTrainIfReg2)==0x261C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txTrainIfReg3)==0x2620,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txTrainPattternReg0)==0x2624,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txTrainDriverReg0)==0x2628,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txTrainDriverReg1)==0x262C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txTrainDriverReg2)==0x2630,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txEmphCtrlReg0)==0x264C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,linkTrainMode0)==0x2650,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,trxTrainIfIntrLane)==0x265C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,trxTrainIfIntrMask0Lane)==0x2660,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,trxTrainIfIntrClearLane)==0x2664,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txTrainCtrlLane)==0x2678,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txTrainIfReg8)==0x267C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,laneCfg0)==0x4000,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,laneStatus0)==0x4004,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,laneCfgStatus2Lane)==0x4008,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,laneCfg2Lane)==0x400C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,laneCfg4)==0x4010,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,laneCfgStatus3Lane)==0x4014,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,laneDpPie8Cfg0Lane)==0x4018,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,laneEqCfg0Lane)==0x4024,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,laneEqCfg1Lane)==0x4028,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,lanePresetCfg4Lane)==0x4034,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,lanePresetCfg16Lane)==0x404C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,laneCoeffMax0Lane)==0x4050,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,laneRemoteSetLane)==0x4054,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,laneEq16gCfg0Lane)==0x4058,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,laneEq32gCfg0Lane)==0x4080,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globRstClkCtrl)==0x4200,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globClkSrcLo)==0x4204,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globClkSrcHi)==0x4208,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globMiscCtrl)==0x420C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globDpSalCfg)==0x4210,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globDpSalCfg1)==0x4214,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globDpSalCfg3)==0x4218,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globProtocolCfg0)==0x421C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globPmCfg0)==0x4220,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globCounterCtrl)==0x4224,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globCounterHi)==0x4228,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globPmDpCtrl)==0x422C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globDpBalCfg0)==0x4230,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globDpBalCfg2)==0x4234,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globDpBalCfg4)==0x4238,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globBistCtrl)==0x423C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globBistLaneType)==0x4240,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globBistStart)==0x4244,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globBistMask)==0x4248,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globBistResult)==0x424C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globBistSeqrCfg)==0x4250,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globBistDataHi)==0x4254,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globBistLinkEq)==0x4258,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globBistLaneMargin)==0x425C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globPipeRevision)==0x4260,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,globL1SubstatesCfg)==0x4264,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,inputPinDebugPipeReg8)==0x5920,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,inputPinDebugPipeReg10)==0x5928,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,inputPinDebugPipeReg11)==0x592C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,inputPinDebugPipeReg12)==0x5930,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,calCtrl1Lane)==0x6000,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,calCtrl3Lane)==0x6008,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,calCtrl4Lane)==0x600C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,calSaveData2Lane)==0x6014,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,phyRemoteCtrlCommandLane)==0x601C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,phyRemoteCtrlValueLanePhyRemoteCtrlValueLane310)==0x6020,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,phyLocalValueLanePhyLocalValueLane310)==0x6024,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,trxTrainIfTimers1Lane)==0x6028,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,trxTrainIfTimers2Lane)==0x602C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,trxTrainIfTimersEnableLane)==0x6030,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeControl1)==0x6038,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeControl3)==0x6044,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeControl5)==0x604C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,trainControl2)==0x6058,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dllCal)==0x6064,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,trainPara1)==0x606C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,trainPara2)==0x6070,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,trainPara3)==0x6074,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeControl6)==0x6078,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeTest1)==0x6080,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeControl7)==0x608C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,cdsCtrlReg0)==0x60A0,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,cdsCtrlReg1)==0x60A4,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,esmPopPCntLowLaneEomPopPCntLane310)==0x60A8,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,esmErrPCntLowLaneEomErrPCntLane310)==0x60AC,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,esmErrPopCntHighLane)==0x60B0,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,esmPopNCntLowLaneEomPopNCntLane310)==0x60DC,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,esmErrNCntLowLaneEomErrNCntLane310)==0x60E0,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dfeResetOverwrite)==0x6248,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,autospeed424)==0x6CC4,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,autospeed425)==0x6CC8,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,autospeed443)==0x6D10,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,autospeed444)==0x6D14,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,autospeed445)==0x6D18,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,autospeed464)==0x6D64,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,autospeed465)==0x6D68,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,uphy14CmnAnaregTop129)==0x8204,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,uphy14CmnAnaregTop138)==0x8228,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,uphy14CmnAnaregTop210)==0x8348,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dtxReg0)==0xA008,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dtxReg2)==0xA010,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dtxReg4)==0xA018,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dtxPhyAlignReg0)==0xA01C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dtxPhyAlignReg1)==0xA024,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,dtxReg5)==0xA028,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,srisReg0)==0xA02C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,srisReg1)==0xA030,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,mcuControl0)==0xA200,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,mcuControl1)==0xA204,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,mcuControl2)==0xA208,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,memoryControl0)==0xA21C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,memoryControl2PmemChecksumExp310)==0xA224,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,memoryControl3PmemChecksum310)==0xA228,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,memoryControl4)==0xA22C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,memCmnEccErrAddress0)==0xA244,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,apbControl)==0xA2F0,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,test0)==0xA300,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,test2)==0xA308,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,test3)==0xA30C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,test4)==0xA310,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,system)==0xA314,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,pmCmnReg1)==0xA318,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,inputCmnPinReg0)==0xA31C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,inputCmnPinReg1)==0xA320,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,inputCmnPinReg2)==0xA324,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,pllcalReg1)==0xA330,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,clkgenCmnReg1)==0xA334,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,spdCmnReg1)==0xA338,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,outputCmnPinReg0)==0xA33C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,xdataMemChecksumCmn0XdataMemChecksumExpCmn310)==0xA358,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,xdataMemChecksumCmn1XdataMemChecksumCmn310)==0xA35C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,xdataMemChecksumCmn2)==0xA360,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,mcuSdtCmn)==0xA364,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,setLaneIsr)==0xA3B4,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,cmnMcu)==0xA3F4,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,cidReg0)==0xA3F8,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,cidReg1)==0xA3FC,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,autospeed159)==0xE27C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,fwRev)==0xE600,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,controlConfig0)==0xE604,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,controlConfig7)==0xE620,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,calData0)==0xE624,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,trainIfConfig)==0xE628,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,controlConfig8)==0xE62C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,controlConfig9)==0xE630,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,mcuConfig)==0xE650,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,mcuConfig1)==0xE65C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,localTxPresetTb5)==0xE6C4,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,trainParameter1)==0xE6F0,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,trainParameter2)==0xE6F4,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,rxCdrBw)==0xE700,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,rxCdrBw1)==0xE704,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,rxCdrBw2)==0xE708,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txFfeCtrl)==0xE70C,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txFfeCtrl1)==0xE710,"check register structure offset");
COMPILE_ASSERT(offsetof(Comphy0Soc_t,txFfeCtrl2)==0xE714,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile Comphy0Soc_t rComphy0Soc; ///< 0xB0100000
