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
//! @brief POR_HSP_ONLY Registers
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


/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_3                    :4;      ///<BIT [3:0] rsvd_0_3
        uint32_t SPEED_OSC                   :2;      ///<BIT [5:4] SPEED_OSC
        uint32_t RSVD_6_15                   :10;     ///<BIT [15:6] rsvd_6_15
        uint32_t PU_XTL                      :1;      ///<BIT [16] PU_XTL
        uint32_t PU_OSC                      :1;      ///<BIT [17] PU_OSC
        uint32_t PU                          :1;      ///<BIT [18] PU
        uint32_t IXTAL                       :2;      ///<BIT [20:19] IXTAL
        uint32_t GAINX2                      :1;      ///<BIT [21] GAINX2
        uint32_t SEL_CLKDIG_DIV3             :2;      ///<BIT [23:22] SEL_CLKDIG_DIV3
        uint32_t SEL_CLKDIG_DIV2             :2;      ///<BIT [25:24] SEL_CLKDIG_DIV2
        uint32_t SEL_CLKDIG_DIV1             :2;      ///<BIT [27:26] SEL_CLKDIG_DIV1
        uint32_t SEL_CLKDIG_DIV0             :2;      ///<BIT [29:28] SEL_CLKDIG_DIV0
        uint32_t XTAL_OSC_BYPASS             :1;      ///<BIT [30] XTAL_OSC_BYPASS
        uint32_t AVDD1815_SEL                :1;      ///<BIT [31] AVDD1815_SEL
    } b;
} AnalogControl1_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_5                    :6;      ///<BIT [5:0] rsvd_0_5
        uint32_t RESERVED1                   :14;     ///<BIT [19:6] Reserved1
        uint32_t ICC_ADJ                     :2;      ///<BIT [21:20] ICC_ADJ
        uint32_t IPP_ADJ                     :2;      ///<BIT [23:22] IPP_ADJ
        uint32_t RESERVED0                   :3;      ///<BIT [26:24] reserved0
        uint32_t VREG_1P4V_SEL               :2;      ///<BIT [28:27] VREG_1P4V_SEL
        uint32_t VREG_0P9V_SEL_XTL           :3;      ///<BIT [31:29] VREG_0P9V_SEL_XTL
    } b;
} AnalogControl2_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CP0_CPUWAIT                 :1;      ///<BIT [0] CP0_CPUWAIT
        uint32_t CP1_CPUWAIT                 :1;      ///<BIT [1] CP1_CPUWAIT
        uint32_t RESERVED                    :30;     ///<BIT [31:2] Reserved
    } b;
} CpRunstall_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SOC_STICKY_RESET            :1;      ///<BIT [0] soc_sticky_reset
        uint32_t RESERVED0                   :15;     ///<BIT [15:1] Reserved0
        uint32_t THERMTRIP_EN                :1;      ///<BIT [16] ThermTrip_en
        uint32_t CHIP_RST_EN                 :1;      ///<BIT [17] chip_rst_en
        uint32_t RESERVED1                   :14;     ///<BIT [31:18] Reserved1
    } b;
} StickyReset_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t THERM_TRIP                  :1;      ///<BIT [0] ThermTrip
        uint32_t CHIP_RST                    :1;      ///<BIT [1] chip_rst
        uint32_t RESERVED0                   :30;     ///<BIT [31:2] Reserved0
    } b;
} StickyStatus_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CPLL_LOCK                   :1;      ///<BIT [0] CPLL_LOCK
        uint32_t RESERVED0                   :1;      ///<BIT [1] reserved0
        uint32_t CPLL_CTUNE                  :2;      ///<BIT [3:2] CPLL_CTUNE
        uint32_t CPLL_CLK_DET_EN             :1;      ///<BIT [4] CPLL_CLK_DET_EN
        uint32_t CPLL_CLKOUT_SRC_SEL         :1;      ///<BIT [5] CPLL_CLKOUT_SRC_SEL
        uint32_t CPLL_CLKOUT_SE_GATE_EN      :1;      ///<BIT [6] CPLL_CLKOUT_SE_GATE_EN
        uint32_t CPLL_BYPASS_EN              :1;      ///<BIT [7] CPLL_BYPASS_EN
        uint32_t CPLL_BW_SEL                 :1;      ///<BIT [8] CPLL_BW_SEL
        uint32_t CPLL_AVDD1815_SEL           :1;      ///<BIT [9] CPLL_AVDD1815_SEL
        uint32_t CPLL_FBDIV                  :9;      ///<BIT [18:10] CPLL_FBDIV
        uint32_t CPLL_KVCO                   :4;      ///<BIT [22:19] CPLL_KVCO
        uint32_t CPLL_CLKOUT_SE_DIV_SEL      :9;      ///<BIT [31:23] CPLL_CLKOUT_SE_DIV_SEL
    } b;
} CpllControl1_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED0                   :1;      ///<BIT [0] reserved0
        uint32_t CPLL_INTPI                  :4;      ///<BIT [4:1] CPLL_INTPI
        uint32_t CPLL_ICP                    :4;      ///<BIT [8:5] CPLL_ICP
        uint32_t CPLL_FREQ_OFFSET_VALID      :1;      ///<BIT [9] CPLL_FREQ_OFFSET_VALID
        uint32_t CPLL_FREQ_OFFSET_MODE_SEL   :1;      ///<BIT [10] CPLL_FREQ_OFFSET_MODE_SEL
        uint32_t CPLL_FREQ_OFFSET_EN         :1;      ///<BIT [11] CPLL_FREQ_OFFSET_EN
        uint32_t CPLL_FREQ_OFFSET            :17;     ///<BIT [28:12] CPLL_FREQ_OFFSET
        uint32_t CPLL_FD                     :3;      ///<BIT [31:29] CPLL_FD
    } b;
} CpllControl2_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED2                   :7;      ///<BIT [6:0] reserved2
        uint32_t CPLL_RST                    :1;      ///<BIT [7] CPLL_RST
        uint32_t CPLL_RST_PI                 :1;      ///<BIT [8] CPLL_RST_PI
        uint32_t RESERVED1                   :8;      ///<BIT [16:9] reserved1
        uint32_t CPLL_REFDIV                 :9;      ///<BIT [25:17] CPLL_REFDIV
        uint32_t RESERVED0                   :1;      ///<BIT [26] reserved0
        uint32_t CPLL_PI_LOOP_MD             :1;      ///<BIT [27] CPLL_PI_LOOP_MD
        uint32_t CPLL_PI_EN                  :1;      ///<BIT [28] CPLL_PI_EN
        uint32_t CPLL_INTPR                  :3;      ///<BIT [31:29] CPLL_INTPR
    } b;
} CpllControl3_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CPLL_SSC_RNGE               :13;     ///<BIT [12:0] CPLL_SSC_RNGE
        uint32_t CPLL_SSC_MD                 :1;      ///<BIT [13] CPLL_SSC_MD
        uint32_t CPLL_SSC_FREQ_DIV           :16;     ///<BIT [29:14] CPLL_SSC_FREQ_DIV
        uint32_t CPLL_SSC_CLK_EN             :1;      ///<BIT [30] CPLL_SSC_CLK_EN
        uint32_t CPLL_RST_SSC                :1;      ///<BIT [31] CPLL_RST_SSC
    } b;
} CpllControl4_t;

/// @brief 0x40
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_12                   :13;     ///<BIT [12:0] rsvd_0_12
        uint32_t CPLL_CLKOUT_DIFF_EN         :1;      ///<BIT [13] CPLL_CLKOUT_DIFF_EN
        uint32_t CPLL_CLKOUT_DIFF_DIV_SEL    :9;      ///<BIT [22:14] CPLL_CLKOUT_DIFF_DIV_SEL
        uint32_t CPLL_VDDM                   :2;      ///<BIT [24:23] CPLL_VDDM
        uint32_t CPLL_VDDL                   :3;      ///<BIT [27:25] CPLL_VDDL
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} CpllControl5_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPLL_LOCK                   :1;      ///<BIT [0] UPLL_LOCK
        uint32_t RESERVED0                   :1;      ///<BIT [1] reserved0
        uint32_t UPLL_CTUNE                  :2;      ///<BIT [3:2] UPLL_CTUNE
        uint32_t UPLL_CLK_DET_EN             :1;      ///<BIT [4] UPLL_CLK_DET_EN
        uint32_t UPLL_CLKOUT_SRC_SEL         :1;      ///<BIT [5] UPLL_CLKOUT_SRC_SEL
        uint32_t UPLL_CLKOUT_SE_GATE_EN      :1;      ///<BIT [6] UPLL_CLKOUT_SE_GATE_EN
        uint32_t UPLL_BYPASS_EN              :1;      ///<BIT [7] UPLL_BYPASS_EN
        uint32_t UPLL_BW_SEL                 :1;      ///<BIT [8] UPLL_BW_SEL
        uint32_t UPLL_AVDD1815_SEL           :1;      ///<BIT [9] UPLL_AVDD1815_SEL
        uint32_t UPLL_FBDIV                  :9;      ///<BIT [18:10] UPLL_FBDIV
        uint32_t UPLL_KVCO                   :4;      ///<BIT [22:19] UPLL_KVCO
        uint32_t UPLL_CLKOUT_SE_DIV_SEL      :9;      ///<BIT [31:23] UPLL_CLKOUT_SE_DIV_SEL
    } b;
} UpllControl1_t;

/// @brief 0x54
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED0                   :1;      ///<BIT [0] reserved0
        uint32_t UPLL_INTPI                  :4;      ///<BIT [4:1] UPLL_INTPI
        uint32_t UPLL_ICP                    :4;      ///<BIT [8:5] UPLL_ICP
        uint32_t UPLL_FREQ_OFFSET_VALID      :1;      ///<BIT [9] UPLL_FREQ_OFFSET_VALID
        uint32_t UPLL_FREQ_OFFSET_MODE_SEL   :1;      ///<BIT [10] UPLL_FREQ_OFFSET_MODE_SEL
        uint32_t UPLL_FREQ_OFFSET_EN         :1;      ///<BIT [11] UPLL_FREQ_OFFSET_EN
        uint32_t UPLL_FREQ_OFFSET            :17;     ///<BIT [28:12] UPLL_FREQ_OFFSET
        uint32_t UPLL_FD                     :3;      ///<BIT [31:29] UPLL_FD
    } b;
} UpllControl2_t;

/// @brief 0x58
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED2                   :7;      ///<BIT [6:0] reserved2
        uint32_t UPLL_RST                    :1;      ///<BIT [7] UPLL_RST
        uint32_t UPLL_RST_PI                 :1;      ///<BIT [8] UPLL_RST_PI
        uint32_t RESERVED1                   :8;      ///<BIT [16:9] reserved1
        uint32_t UPLL_REFDIV                 :9;      ///<BIT [25:17] UPLL_REFDIV
        uint32_t RESERVED0                   :1;      ///<BIT [26] reserved0
        uint32_t UPLL_PI_LOOP_MD             :1;      ///<BIT [27] UPLL_PI_LOOP_MD
        uint32_t UPLL_PI_EN                  :1;      ///<BIT [28] UPLL_PI_EN
        uint32_t UPLL_INTPR                  :3;      ///<BIT [31:29] UPLL_INTPR
    } b;
} UpllControl3_t;

/// @brief 0x5C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPLL_SSC_RNGE               :13;     ///<BIT [12:0] UPLL_SSC_RNGE
        uint32_t UPLL_SSC_MD                 :1;      ///<BIT [13] UPLL_SSC_MD
        uint32_t UPLL_SSC_FREQ_DIV           :16;     ///<BIT [29:14] UPLL_SSC_FREQ_DIV
        uint32_t UPLL_SSC_CLK_EN             :1;      ///<BIT [30] UPLL_SSC_CLK_EN
        uint32_t UPLL_RST_SSC                :1;      ///<BIT [31] UPLL_RST_SSC
    } b;
} UpllControl4_t;

/// @brief 0x60
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_12                   :13;     ///<BIT [12:0] rsvd_0_12
        uint32_t UPLL_CLKOUT_DIFF_EN         :1;      ///<BIT [13] UPLL_CLKOUT_DIFF_EN
        uint32_t UPLL_CLKOUT_DIFF_DIV_SEL    :9;      ///<BIT [22:14] UPLL_CLKOUT_DIFF_DIV_SEL
        uint32_t UPLL_VDDM                   :2;      ///<BIT [24:23] UPLL_VDDM
        uint32_t UPLL_VDDL                   :3;      ///<BIT [27:25] UPLL_VDDL
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} UpllControl5_t;

/// @brief 0x70
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CP_PLL_LOCK                 :1;      ///<BIT [0] CP_PLL_LOCK
        uint32_t RESERVED0                   :1;      ///<BIT [1] reserved0
        uint32_t CP_PLL_CTUNE                :2;      ///<BIT [3:2] CP_PLL_CTUNE
        uint32_t CP_PLL_CLK_DET_EN           :1;      ///<BIT [4] CP_PLL_CLK_DET_EN
        uint32_t CP_PLL_CLKOUT_SRC_SEL       :1;      ///<BIT [5] CP_PLL_CLKOUT_SRC_SEL
        uint32_t CP_PLL_CLKOUT_SE_GATE_EN    :1;      ///<BIT [6] CP_PLL_CLKOUT_SE_GATE_EN
        uint32_t CP_PLL_BYPASS_EN            :1;      ///<BIT [7] CP_PLL_BYPASS_EN
        uint32_t CP_PLL_BW_SEL               :1;      ///<BIT [8] CP_PLL_BW_SEL
        uint32_t CP_PLL_AVDD1815_SEL         :1;      ///<BIT [9] CP_PLL_AVDD1815_SEL
        uint32_t CP_PLL_FBDIV                :9;      ///<BIT [18:10] CP_PLL_FBDIV
        uint32_t CP_PLL_KVCO                 :4;      ///<BIT [22:19] CP_PLL_KVCO
        uint32_t CP_PLL_CLKOUT_SE_DIV_SEL    :9;      ///<BIT [31:23] CP_PLL_CLKOUT_SE_DIV_SEL
    } b;
} CpPllControl1_t;

/// @brief 0x74
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED0                   :1;      ///<BIT [0] reserved0
        uint32_t CP_PLL_INTPI                :4;      ///<BIT [4:1] CP_PLL_INTPI
        uint32_t CP_PLL_ICP                  :4;      ///<BIT [8:5] CP_PLL_ICP
        uint32_t CP_PLL_FREQ_OFFSET_VALID    :1;      ///<BIT [9] CP_PLL_FREQ_OFFSET_VALID
        uint32_t CP_PLL_FREQ_OFFSET_MODE_SEL :1;      ///<BIT [10] CP_PLL_FREQ_OFFSET_MODE_SEL
        uint32_t CP_PLL_FREQ_OFFSET_EN       :1;      ///<BIT [11] CP_PLL_FREQ_OFFSET_EN
        uint32_t CP_PLL_FREQ_OFFSET          :17;     ///<BIT [28:12] CP_PLL_FREQ_OFFSET
        uint32_t CP_PLL_FD                   :3;      ///<BIT [31:29] CP_PLL_FD
    } b;
} CpPllControl2_t;

/// @brief 0x78
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED2                   :7;      ///<BIT [6:0] reserved2
        uint32_t CP_PLL_RST                  :1;      ///<BIT [7] CP_PLL_RST
        uint32_t CP_PLL_RST_PI               :1;      ///<BIT [8] CP_PLL_RST_PI
        uint32_t RESERVED1                   :8;      ///<BIT [16:9] reserved1
        uint32_t CP_PLL_REFDIV               :9;      ///<BIT [25:17] CP_PLL_REFDIV
        uint32_t RESERVED0                   :1;      ///<BIT [26] reserved0
        uint32_t CP_PLL_PI_LOOP_MD           :1;      ///<BIT [27] CP_PLL_PI_LOOP_MD
        uint32_t CP_PLL_PI_EN                :1;      ///<BIT [28] CP_PLL_PI_EN
        uint32_t CP_PLL_INTPR                :3;      ///<BIT [31:29] CP_PLL_INTPR
    } b;
} CpPllControl3_t;

/// @brief 0x7C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CP_PLL_SSC_RNGE             :13;     ///<BIT [12:0] CP_PLL_SSC_RNGE
        uint32_t CP_PLL_SSC_MD               :1;      ///<BIT [13] CP_PLL_SSC_MD
        uint32_t CP_PLL_SSC_FREQ_DIV         :16;     ///<BIT [29:14] CP_PLL_SSC_FREQ_DIV
        uint32_t CP_PLL_SSC_CLK_EN           :1;      ///<BIT [30] CP_PLL_SSC_CLK_EN
        uint32_t CP_PLL_RST_SSC              :1;      ///<BIT [31] CP_PLL_RST_SSC
    } b;
} CpPllControl4_t;

/// @brief 0x80
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_12                   :13;     ///<BIT [12:0] rsvd_0_12
        uint32_t CP_PLL_CLKOUT_DIFF_EN       :1;      ///<BIT [13] CP_PLL_CLKOUT_DIFF_EN
        uint32_t CP_PLL_CLKOUT_DIFF_DIV_SEL  :9;      ///<BIT [22:14] CP_PLL_CLKOUT_DIFF_DIV_SEL
        uint32_t CP_PLL_VDDM                 :2;      ///<BIT [24:23] CP_PLL_VDDM
        uint32_t CP_PLL_VDDL                 :3;      ///<BIT [27:25] CP_PLL_VDDL
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} CpPllControl5_t;

/// @brief 0x90
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED0                   :1;      ///<BIT [0] RESERVED0
        uint32_t RESERVED1                   :1;      ///<BIT [1] RESERVED1
        uint32_t CRYPTO_CLK_DIS              :1;      ///<BIT [2] crypto_clk_dis
        uint32_t RESERVED2                   :2;      ///<BIT [4:3] RESERVED2
        uint32_t MINIAXI_CLK_DIS             :1;      ///<BIT [5] miniaxi_clk_dis
        uint32_t FPS_SLOW_CLK_DIS            :1;      ///<BIT [6] fps_slow_clk_dis
        uint32_t UART_CLK_DIS                :1;      ///<BIT [7] uart_clk_dis
        uint32_t CPUCS_CLK_DIS               :1;      ///<BIT [8] cpucs_clk_dis
        uint32_t SYSAXI_CLK_DIS              :1;      ///<BIT [9] sysaxi_clk_dis
        uint32_t TRACE_CLK_DIS               :1;      ///<BIT [10] trace_clk_dis
        uint32_t FPS_CLK_DIS                 :1;      ///<BIT [11] fps_clk_dis
        uint32_t GSRAM_CLK_DIS               :1;      ///<BIT [12] gsram_clk_dis
        uint32_t NQM_CLK_DIS                 :1;      ///<BIT [13] nqm_clk_dis
        uint32_t APB_CLK_DIS                 :1;      ///<BIT [14] apb_clk_dis
        uint32_t BCP_CLK_DIS                 :1;      ///<BIT [15] bcp_clk_dis
        uint32_t RESERVED3                   :4;      ///<BIT [19:16] RESERVED3
        uint32_t MCU_CLK_DIS                 :1;      ///<BIT [20] mcu_clk_dis
        uint32_t GDMA_CLK_DIS                :1;      ///<BIT [21] gdma_clk_dis
        uint32_t CPUCP_P0_CLK_DIS            :1;      ///<BIT [22] cpucp_p0_clk_dis
        uint32_t CPUCP_P1_CLK_DIS            :1;      ///<BIT [23] cpucp_p1_clk_dis
        uint32_t CPUCP_2X_CLK_DIS            :1;      ///<BIT [24] cpucp_2x_clk_dis
        uint32_t RESERVED4                   :2;      ///<BIT [26:25] RESERVED4
        uint32_t OSC_CLK_DIS                 :1;      ///<BIT [27] osc_clk_dis
        uint32_t SPI_SYSAXI_CLK_DIS          :1;      ///<BIT [28] spi_sysaxi_clk_dis
        uint32_t SPI_CLK_DIS                 :1;      ///<BIT [29] spi_clk_dis
        uint32_t OSC_0P5M_CLK_DIS            :1;      ///<BIT [30] osc_0p5m_clk_dis
        uint32_t EMC_CLK_DIS                 :1;      ///<BIT [31] emc_clk_dis
    } b;
} ClockDisableControl0_t;

/// @brief 0x94
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPKA_DIS                    :16;     ///<BIT [15:0] upka_dis
        uint32_t RESERVED0                   :15;     ///<BIT [30:16] RESERVED0
        uint32_t CKGEN_MON_DIS               :1;      ///<BIT [31] ckgen_mon_dis
    } b;
} ClockDisableControl1_t;

/// @brief 0x9C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TCON_OSC_CLK_SEL            :1;      ///<BIT [0] tcon_osc_clk_sel
        uint32_t UPKA_OSC_CLK_SEL            :1;      ///<BIT [1] upka_osc_clk_sel
        uint32_t MINIAXI_OSC_CLK_SEL         :1;      ///<BIT [2] miniaxi_osc_clk_sel
        uint32_t CRYPTO_OSC_CLK_SEL          :1;      ///<BIT [3] crypto_osc_clk_sel
        uint32_t RESERVED2                   :3;      ///<BIT [6:4] RESERVED2
        uint32_t UART_OSC_CLK_SEL            :1;      ///<BIT [7] uart_osc_clk_sel
        uint32_t CPUCS_OSC_CLK_SEL           :1;      ///<BIT [8] cpucs_osc_clk_sel
        uint32_t SYSAXI_OSC_CLK_SEL          :1;      ///<BIT [9] sysaxi_osc_clk_sel
        uint32_t TRACE_OSC_CLK_SEL           :1;      ///<BIT [10] trace_osc_clk_sel
        uint32_t FPS_OSC_CLK_SEL             :1;      ///<BIT [11] fps_osc_clk_sel
        uint32_t GSRAM_OSC_CLK_SEL           :1;      ///<BIT [12] gsram_osc_clk_sel
        uint32_t NQM_OSC_CLK_SEL             :1;      ///<BIT [13] nqm_osc_clk_sel
        uint32_t APB_OSC_CLK_SEL             :1;      ///<BIT [14] apb_osc_clk_sel
        uint32_t BCP_OSC_CLK_SEL             :1;      ///<BIT [15] bcp_osc_clk_sel
        uint32_t RESERVED1                   :4;      ///<BIT [19:16] RESERVED1
        uint32_t MCU_OSC_CLK_SEL             :1;      ///<BIT [20] mcu_osc_clk_sel
        uint32_t GDMA_OSC_CLK_SEL            :1;      ///<BIT [21] gdma_osc_clk_sel
        uint32_t CPUCP_OSC_CLK_SEL           :1;      ///<BIT [22] cpucp_osc_clk_sel
        uint32_t HSP_OSC_CLK_SEL             :1;      ///<BIT [23] hsp_osc_clk_sel
        uint32_t EMC_OSC_CLK_SEL             :1;      ///<BIT [24] emc_osc_clk_sel
        uint32_t SPI_OSC_CLK_SEL             :1;      ///<BIT [25] spi_osc_clk_sel
        uint32_t RESERVED0                   :6;      ///<BIT [31:26] RESERVED0
    } b;
} ClockSelectionControl_t;

/// @brief 0xA0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TRACE_CLKDIV_SEL            :4;      ///<BIT [3:0] trace_clkdiv_sel
        uint32_t FPS_CLKDIV_SEL              :4;      ///<BIT [7:4] fps_clkdiv_sel
        uint32_t GSRAM_CLKDIV_SEL            :4;      ///<BIT [11:8] gsram_clkdiv_sel
        uint32_t NQM_CLKDIV_SEL              :4;      ///<BIT [15:12] nqm_clkdiv_sel
        uint32_t APB_CLKDIV_SEL              :4;      ///<BIT [19:16] apb_clkdiv_sel
        uint32_t BCP_CLKDIV_SEL              :4;      ///<BIT [23:20] bcp_clkdiv_sel
        uint32_t MCU_CLKDIV_SEL              :4;      ///<BIT [27:24] mcu_clkdiv_sel
        uint32_t GDMA_DIV_SEL                :4;      ///<BIT [31:28] gdma_div_sel
    } b;
} ClockDividerControl0_t;

/// @brief 0xA4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UART_CLKDIV_SEL             :4;      ///<BIT [3:0] uart_clkdiv_sel
        uint32_t CPUCS_CLKDIV_SEL            :4;      ///<BIT [7:4] cpucs_clkdiv_sel
        uint32_t TCON_CLKDIV_SEL             :4;      ///<BIT [11:8] tcon_clkdiv_sel
        uint32_t UPKA_CLKDIV_SEL             :4;      ///<BIT [15:12] upka_clkdiv_sel
        uint32_t MINIAXI_CLKDIV_SEL          :4;      ///<BIT [19:16] miniaxi_clkdiv_sel
        uint32_t SYSAXI_CLKDIV_SEL           :4;      ///<BIT [23:20] sysaxi_clkdiv_sel
        uint32_t CPUCP_CLKDIV_SEL            :4;      ///<BIT [27:24] cpucp_clkdiv_sel
        uint32_t HSP_CLKDIV_SEL              :4;      ///<BIT [31:28] hsp_clkdiv_sel
    } b;
} ClockDividerControl1_t;

/// @brief 0xA8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OSC_CLKDIV_SEL              :2;      ///<BIT [1:0] osc_clkdiv_sel
        uint32_t OSC_0P5M_CLKDIV_SEL2        :3;      ///<BIT [4:2] osc_0p5m_clkdiv_sel2
        uint32_t OSC_0P5M_CLKDIV_SEL1        :3;      ///<BIT [7:5] osc_0p5m_clkdiv_sel1
        uint32_t OSC_0P5M_CLKDIV_SEL0        :3;      ///<BIT [10:8] osc_0p5m_clkdiv_sel0
        uint32_t CRYPTO_CLKDIV_SEL           :4;      ///<BIT [14:11] crypto_clkdiv_sel
        uint32_t TRACE_DIV2_CLK_SEL          :1;      ///<BIT [15] trace_div2_clk_sel
        uint32_t EMC_CLKDIV_SEL0             :4;      ///<BIT [19:16] emc_clkdiv_sel0
        uint32_t EMC_CLKDIV_SEL1             :3;      ///<BIT [22:20] emc_clkdiv_sel1
        uint32_t SPI_CLKDIV_SEL0             :4;      ///<BIT [26:23] spi_clkdiv_sel0
        uint32_t SPI_CLKDIV_SEL1             :3;      ///<BIT [29:27] spi_clkdiv_sel1
        uint32_t RESERVED1                   :2;      ///<BIT [31:30] RESERVED1
    } b;
} ClockDividerControl2_t;

/// @brief 0xAC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED3                   :2;      ///<BIT [1:0] RESERVED3
        uint32_t TCON_TBG_SEL                :1;      ///<BIT [2] tcon_tbg_sel
        uint32_t UPKA_TBG_SEL                :1;      ///<BIT [3] upka_tbg_sel
        uint32_t MINIAXI_TBG_SEL             :1;      ///<BIT [4] miniaxi_tbg_sel
        uint32_t CRYPTO_TBG_SEL              :1;      ///<BIT [5] crypto_tbg_sel
        uint32_t RESERVED2                   :1;      ///<BIT [6] RESERVED2
        uint32_t UART_TBG_SEL                :1;      ///<BIT [7] uart_tbg_sel
        uint32_t CPUCS_TBG_SEL               :1;      ///<BIT [8] cpucs_tbg_sel
        uint32_t SYSAXI_TBG_SEL              :1;      ///<BIT [9] sysaxi_tbg_sel
        uint32_t TRACE_TBG_SEL               :1;      ///<BIT [10] trace_tbg_sel
        uint32_t FPS_TBG_SEL                 :1;      ///<BIT [11] fps_tbg_sel
        uint32_t GSRAM_TBG_SEL               :1;      ///<BIT [12] gsram_tbg_sel
        uint32_t NQM_TBG_SEL                 :1;      ///<BIT [13] nqm_tbg_sel
        uint32_t APB_TBG_SEL                 :1;      ///<BIT [14] apb_tbg_sel
        uint32_t BCP_TBG_SEL                 :1;      ///<BIT [15] bcp_tbg_sel
        uint32_t RESERVED1                   :4;      ///<BIT [19:16] RESERVED1
        uint32_t MCU_TBG_SEL                 :1;      ///<BIT [20] mcu_tbg_sel
        uint32_t GDMA_TBG_SEL                :1;      ///<BIT [21] gdma_tbg_sel
        uint32_t CPUCP_TBG_SEL               :1;      ///<BIT [22] cpucp_tbg_sel
        uint32_t HSP_TBG_SEL                 :1;      ///<BIT [23] hsp_tbg_sel
        uint32_t RESERVED0                   :8;      ///<BIT [31:24] RESERVED0
    } b;
} TbgSelection_t;

/// @brief 0xB0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TBG_CP_PLL_DIV_RST          :1;      ///<BIT [0] tbg_cp_pll_div_rst
        uint32_t TBG_UPLL_DIV_RST            :1;      ///<BIT [1] tbg_upll_div_rst
        uint32_t TBG_CPLL_DIV_RST            :1;      ///<BIT [2] tbg_cpll_div_rst
        uint32_t RESERVED0                   :29;     ///<BIT [31:3] Reserved0
    } b;
} TbgController_t;

/// @brief 0xD0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_SRAM1P_RTC              :2;      ///<BIT [1:0] HSP_SRAM1P_RTC
        uint32_t HSP_SRAM1P_WTC              :2;      ///<BIT [3:2] HSP_SRAM1P_WTC
        uint32_t HSP_IROM_RTSEL              :2;      ///<BIT [5:4] HSP_IROM_RTSEL
        uint32_t GSRAM_SLP                   :2;      ///<BIT [7:6] GSRAM_SLP
        uint32_t HSP_SRAM_SD                 :1;      ///<BIT [8] HSP_SRAM_SD
        uint32_t HSP_SRAM_SLP                :1;      ///<BIT [9] HSP_SRAM_SLP
        uint32_t HSP_IROM_SD                 :1;      ///<BIT [10] HSP_IROM_SD
        uint32_t HSP_IROM_TM                 :1;      ///<BIT [11] HSP_IROM_TM
        uint32_t HSP_IROM_TRB                :2;      ///<BIT [13:12] HSP_IROM_TRB
        uint32_t HSP_RF1P_RTC                :2;      ///<BIT [15:14] HSP_RF1P_RTC
        uint32_t HSP_RF1P_WTC                :2;      ///<BIT [17:16] HSP_RF1P_WTC
        uint32_t RESERVED                    :14;     ///<BIT [31:18] RESERVED
    } b;
} HspSramControl_t;

typedef struct
{
    AnalogControl1_t analogControl1;                                        // 0x0 : Analog_Control_1 / 
    AnalogControl2_t analogControl2;                                        // 0x4 : Analog_Control_2 / 
    CpRunstall_t cpRunstall;                                                // 0x8 : CP_Runstall / 
    uint8_t rsvdC[4];                                                       // 0xC : rsvd_c / rsvd_c
    StickyReset_t stickyReset;                                              // 0x10 : STICKY_RESET / 
    StickyStatus_t stickyStatus;                                            // 0x14 : STICKY_STATUS / 
    uint32_t stickyCookieCookieValue;                                       // 0x18 : STICKY_COOKIE / 
    uint8_t rsvd1c[20];                                                     // 0x1C : rsvd_1c / rsvd_1c
    CpllControl1_t cpllControl1;                                            // 0x30 : CPLL_Control_1 / 
    CpllControl2_t cpllControl2;                                            // 0x34 : CPLL_Control_2 / 
    CpllControl3_t cpllControl3;                                            // 0x38 : CPLL_Control_3 / 
    CpllControl4_t cpllControl4;                                            // 0x3C : CPLL_Control_4 / 
    CpllControl5_t cpllControl5;                                            // 0x40 : CPLL_Control_5 / 
    uint8_t rsvd44[12];                                                     // 0x44 : rsvd_44 / rsvd_44
    UpllControl1_t upllControl1;                                            // 0x50 : UPLL_Control_1 / 
    UpllControl2_t upllControl2;                                            // 0x54 : UPLL_Control_2 / 
    UpllControl3_t upllControl3;                                            // 0x58 : UPLL_Control_3 / 
    UpllControl4_t upllControl4;                                            // 0x5C : UPLL_Control_4 / 
    UpllControl5_t upllControl5;                                            // 0x60 : UPLL_Control_5 / 
    uint8_t rsvd64[12];                                                     // 0x64 : rsvd_64 / rsvd_64
    CpPllControl1_t cpPllControl1;                                          // 0x70 : CP_PLL_Control_1 / 
    CpPllControl2_t cpPllControl2;                                          // 0x74 : CP_PLL_Control_2 / 
    CpPllControl3_t cpPllControl3;                                          // 0x78 : CP_PLL_Control_3 / 
    CpPllControl4_t cpPllControl4;                                          // 0x7C : CP_PLL_Control_4 / 
    CpPllControl5_t cpPllControl5;                                          // 0x80 : CP_PLL_Control_5 / 
    uint8_t rsvd84[12];                                                     // 0x84 : rsvd_84 / rsvd_84
    ClockDisableControl0_t clockDisableControl0;                            // 0x90 : Clock_Disable_Control_0 / 
    ClockDisableControl1_t clockDisableControl1;                            // 0x94 : Clock_Disable_Control_1 / 
    uint8_t rsvd98[4];                                                      // 0x98 : rsvd_98 / rsvd_98
    ClockSelectionControl_t clockSelectionControl;                          // 0x9C : Clock_Selection_Control / 
    ClockDividerControl0_t clockDividerControl0;                            // 0xA0 : Clock_Divider_Control_0 / 
    ClockDividerControl1_t clockDividerControl1;                            // 0xA4 : Clock_Divider_Control_1 / 
    ClockDividerControl2_t clockDividerControl2;                            // 0xA8 : Clock_Divider_Control_2 / 
    TbgSelection_t tbgSelection;                                            // 0xAC : TBG_Selection / 
    TbgController_t tbgController;                                          // 0xB0 : TBG_Controller / 
    uint8_t rsvdB4[28];                                                     // 0xB4 : rsvd_b4 / rsvd_b4
    HspSramControl_t hspSramControl;                                        // 0xD0 : HSP_SRAM_Control / 
} PorHspOnly_t;

COMPILE_ASSERT(offsetof(PorHspOnly_t,analogControl1)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,analogControl2)==0x4,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,cpRunstall)==0x8,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,stickyReset)==0x10,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,stickyStatus)==0x14,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,stickyCookieCookieValue)==0x18,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,cpllControl1)==0x30,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,cpllControl2)==0x34,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,cpllControl3)==0x38,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,cpllControl4)==0x3C,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,cpllControl5)==0x40,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,upllControl1)==0x50,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,upllControl2)==0x54,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,upllControl3)==0x58,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,upllControl4)==0x5C,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,upllControl5)==0x60,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,cpPllControl1)==0x70,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,cpPllControl2)==0x74,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,cpPllControl3)==0x78,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,cpPllControl4)==0x7C,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,cpPllControl5)==0x80,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,clockDisableControl0)==0x90,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,clockDisableControl1)==0x94,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,clockSelectionControl)==0x9C,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,clockDividerControl0)==0xA0,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,clockDividerControl1)==0xA4,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,clockDividerControl2)==0xA8,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,tbgSelection)==0xAC,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,tbgController)==0xB0,"check register structure offset");
COMPILE_ASSERT(offsetof(PorHspOnly_t,hspSramControl)==0xD0,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile PorHspOnly_t rPorHspOnly; ///< 0xB0003000
