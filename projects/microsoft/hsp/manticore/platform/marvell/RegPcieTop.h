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
//! @brief PCIE_TOP_REG Registers
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
        uint32_t REF_FREF_SEL_P0             :5;      ///<BIT [4:0] REF_FREF_SEL_P0
        uint32_t RESERVED2                   :3;      ///<BIT [7:5] Reserved2
        uint32_t PHY_MODE_P0                 :3;      ///<BIT [10:8] PHY_MODE_P0
        uint32_t RESERVED1                   :5;      ///<BIT [15:11] Reserved1
        uint32_t REFCLK_SEL_P0               :1;      ///<BIT [16] REFCLK_SEL_P0
        uint32_t SRIS_ENABLE0_P0             :1;      ///<BIT [17] SRIS_ENABLE0_P0
        uint32_t SRIS_ENABLE1_P0             :1;      ///<BIT [18] SRIS_ENABLE1_P0
        uint32_t RESERVED0                   :5;      ///<BIT [23:19] Reserved0
        uint32_t RESERVED                    :8;      ///<BIT [31:24] Reserved
    } b;
} Ctrl0P0_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESET_P0                    :1;      ///<BIT [0] RESET_P0
        uint32_t RESET_CORE0_P0              :1;      ///<BIT [1] RESET_CORE0_P0
        uint32_t RESET_CORE1_P0              :1;      ///<BIT [2] RESET_CORE1_P0
        uint32_t DIRECT_ACCESS_EN_P0         :1;      ///<BIT [3] DIRECT_ACCESS_EN_P0
        uint32_t RESERVED0                   :12;     ///<BIT [15:4] Reserved0
        uint32_t RESERVED_INPUT_P0           :16;     ///<BIT [31:16] RESERVED_INPUT_P0
    } b;
} Ctrl1P0_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GPI0_P0                     :8;      ///<BIT [7:0] GPI0_P0
        uint32_t GPI1_P0                     :8;      ///<BIT [15:8] GPI1_P0
        uint32_t GPI_CMN_P0                  :8;      ///<BIT [23:16] GPI_CMN_P0
        uint32_t RESERVED0                   :8;      ///<BIT [31:24] Reserved0
    } b;
} Ctrl2P0_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_OUTPUT_P0          :16;     ///<BIT [15:0] RESERVED_OUTPUT_P0
        uint32_t RESERVED0                   :16;     ///<BIT [31:16] Reserved0
    } b;
} ReservedOutputP0_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_INPUT_RX0_P0       :16;     ///<BIT [15:0] RESERVED_INPUT_RX0_P0
        uint32_t RESERVED_INPUT_RX1_P0       :16;     ///<BIT [31:16] RESERVED_INPUT_RX1_P0
    } b;
} ReservedInputRxP0_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_INPUT_TX0_P0       :16;     ///<BIT [15:0] RESERVED_INPUT_TX0_P0
        uint32_t RESERVED_INPUT_TX1_P0       :16;     ///<BIT [31:16] RESERVED_INPUT_TX1_P0
    } b;
} ReservedInputTxP0_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_OUTPUT_RX0_P0      :16;     ///<BIT [15:0] RESERVED_OUTPUT_RX0_P0
        uint32_t RESERVED_OUTPUT_RX1_P0      :16;     ///<BIT [31:16] RESERVED_OUTPUT_RX1_P0
    } b;
} ReservedOutputRxP0_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_OUTPUT_TX0_P0      :16;     ///<BIT [15:0] RESERVED_OUTPUT_TX0_P0
        uint32_t RESERVED_OUTPUT_TX1_P0      :16;     ///<BIT [31:16] RESERVED_OUTPUT_TX1_P0
    } b;
} ReservedOutputTxP0_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PERST_DB_IN_BLK             :1;      ///<BIT [0] PERST_DB_IN_BLK
        uint32_t PERST_DB_RST                :1;      ///<BIT [1] PERST_DB_RST
        uint32_t PERST_DB_BYPASS             :1;      ///<BIT [2] PERST_DB_BYPASS
        uint32_t RESERVED1                   :13;     ///<BIT [15:3] Reserved1
        uint32_t RSVD_16                     :1;      ///<BIT [16] rsvd_16
        uint32_t BURN_IN_TEST_P0             :1;      ///<BIT [17] BURN_IN_TEST_P0
        uint32_t X4_FORCE_PREADY_PERSTN_EN   :1;      ///<BIT [18] X4_FORCE_PREADY_PERSTN_EN
        uint32_t RESERVED0                   :5;      ///<BIT [23:19] Reserved0
        uint32_t PERST_FILTER_EN             :1;      ///<BIT [24] PERST_FILTER_EN
        uint32_t UCD_MAP_DIS                 :1;      ///<BIT [25] UCD_MAP_DIS
        uint32_t IDDQ_P0                     :1;      ///<BIT [26] IDDQ_P0
        uint32_t IDDQ_P1                     :1;      ///<BIT [27] IDDQ_P1
        uint32_t PWR_PCIE_SRAM_SD            :1;      ///<BIT [28] PWR_PCIE_SRAM_SD
        uint32_t PWR_PCIE_SRAM_SLP           :1;      ///<BIT [29] PWR_PCIE_SRAM_SLP
        uint32_t PWR_PCIE_PORT0_CLKOFF       :1;      ///<BIT [30] PWR_PCIE_PORT0_CLKOFF
        uint32_t PWR_PCIE_SRAM_CNTL_DIS      :1;      ///<BIT [31] PWR_PCIE_SRAM_CNTL_DIS
    } b;
} MiscCtrl0_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED3                   :3;      ///<BIT [2:0] Reserved3
        uint32_t RESERVED2                   :13;     ///<BIT [15:3] Reserved2
        uint32_t RESET_DIS_P0                :1;      ///<BIT [16] RESET_DIS_P0
        uint32_t RESET_CORE0_DIS_P0          :1;      ///<BIT [17] RESET_CORE0_DIS_P0
        uint32_t RESET_CORE1_DIS_P0          :1;      ///<BIT [18] RESET_CORE1_DIS_P0
        uint32_t RESERVED1                   :1;      ///<BIT [19] Reserved1
        uint32_t RESET_DIS_P1                :1;      ///<BIT [20] RESET_DIS_P1
        uint32_t RESET_CORE0_DIS_P1          :1;      ///<BIT [21] RESET_CORE0_DIS_P1
        uint32_t RESET_CORE1_DIS_P1          :1;      ///<BIT [22] RESET_CORE1_DIS_P1
        uint32_t RESERVED0                   :9;      ///<BIT [31:23] Reserved0
    } b;
} ResetDis_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PIPE_STRAP_ENABLE0_P0       :1;      ///<BIT [0] PIPE_STRAP_ENABLE0_P0
        uint32_t PIPE_STRAP_ENABLE1_P0       :1;      ///<BIT [1] PIPE_STRAP_ENABLE1_P0
        uint32_t PIPE_LANE_MASTER0_P0        :1;      ///<BIT [2] PIPE_LANE_MASTER0_P0
        uint32_t PIPE_LANE_MASTER1_P0        :1;      ///<BIT [3] PIPE_LANE_MASTER1_P0
        uint32_t PIPE_LANE_START0_P0         :1;      ///<BIT [4] PIPE_LANE_START0_P0
        uint32_t PIPE_LANE_START1_P0         :1;      ///<BIT [5] PIPE_LANE_START1_P0
        uint32_t PIPE_LANE_BREAK0_P0         :1;      ///<BIT [6] PIPE_LANE_BREAK0_P0
        uint32_t PIPE_LANE_BREAK1_P0         :1;      ///<BIT [7] PIPE_LANE_BREAK1_P0
        uint32_t RESERVED1                   :8;      ///<BIT [15:8] Reserved1
        uint32_t PIPE_BIFURCATION_SEL0_P0    :2;      ///<BIT [17:16] PIPE_BIFURCATION_SEL0_P0
        uint32_t PIPE_BIFURCATION_SEL1_P0    :2;      ///<BIT [19:18] PIPE_BIFURCATION_SEL1_P0
        uint32_t RESERVED0                   :12;     ///<BIT [31:20] Reserved0
    } b;
} PhyStrapP0_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PERST_DB_L_TRHD             :20;     ///<BIT [19:0] PERST_DB_L_TRHD
        uint32_t RESERVED0                   :12;     ///<BIT [31:20] Reserved0
    } b;
} PerstDbLTrhd_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PERST_DB_H_TRHD             :20;     ///<BIT [19:0] PERST_DB_H_TRHD
        uint32_t RESERVED0                   :12;     ///<BIT [31:20] Reserved0
    } b;
} PerstDbHTrhd_t;

/// @brief 0x40
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED2                   :16;     ///<BIT [15:0] Reserved2
        uint32_t CK_RX_RDY_FILTER_EN         :1;      ///<BIT [16] CK_RX_RDY_FILTER_EN
        uint32_t BURN_IN_TEST_P1             :1;      ///<BIT [17] BURN_IN_TEST_P1
        uint32_t FORCE_PREADY_DBI_EN         :1;      ///<BIT [18] FORCE_PREADY_DBI_EN
        uint32_t RESERVED1                   :5;      ///<BIT [23:19] Reserved1
        uint32_t PERST_N_DIS                 :1;      ///<BIT [24] PERST_N_DIS
        uint32_t PERST_N_FW                  :1;      ///<BIT [25] PERST_N_FW
        uint32_t DOE_RST_DIS                 :1;      ///<BIT [26] DOE_RST_DIS
        uint32_t DOE_RST_N_FW                :1;      ///<BIT [27] DOE_RST_N_FW
        uint32_t RESERVED0                   :4;      ///<BIT [31:28] Reserved0
    } b;
} MiscCtrl1_t;

/// @brief 0x44
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED0                   :28;     ///<BIT [27:0] Reserved0
        uint32_t TESTMUX_SEL                 :4;      ///<BIT [31:28] TESTMUX_SEL
    } b;
} ElaCtrl_t;

/// @brief 0x48
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PIPE_STRAP_ENABLE0_P1       :1;      ///<BIT [0] PIPE_STRAP_ENABLE0_P1
        uint32_t PIPE_STRAP_ENABLE1_P1       :1;      ///<BIT [1] PIPE_STRAP_ENABLE1_P1
        uint32_t PIPE_LANE_MASTER0_P1        :1;      ///<BIT [2] PIPE_LANE_MASTER0_P1
        uint32_t PIPE_LANE_MASTER1_P1        :1;      ///<BIT [3] PIPE_LANE_MASTER1_P1
        uint32_t PIPE_LANE_START0_P1         :1;      ///<BIT [4] PIPE_LANE_START0_P1
        uint32_t PIPE_LANE_START1_P1         :1;      ///<BIT [5] PIPE_LANE_START1_P1
        uint32_t PIPE_LANE_BREAK0_P1         :1;      ///<BIT [6] PIPE_LANE_BREAK0_P1
        uint32_t PIPE_LANE_BREAK1_P1         :1;      ///<BIT [7] PIPE_LANE_BREAK1_P1
        uint32_t RESERVED1                   :8;      ///<BIT [15:8] Reserved1
        uint32_t PIPE_BIFURCATION_SEL0_P1    :2;      ///<BIT [17:16] PIPE_BIFURCATION_SEL0_P1
        uint32_t PIPE_BIFURCATION_SEL1_P1    :2;      ///<BIT [19:18] PIPE_BIFURCATION_SEL1_P1
        uint32_t RESERVED0                   :12;     ///<BIT [31:20] Reserved0
    } b;
} PhyStrap1_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      :1;      ///<BIT [0] rsvd_0
        uint32_t PCIE_0_CLKOUT_DIG_DIV_SEL   :2;      ///<BIT [2:1] PCIE_0_CLKOUT_DIG_DIV_SEL
        uint32_t PCIE_0_DELAY_RX_SEL         :2;      ///<BIT [4:3] PCIE_0_DELAY_RX_SEL
        uint32_t PCIE_0_DET_RX_MODE          :1;      ///<BIT [5] PCIE_0_DET_RX_MODE
        uint32_t PCIE_0_DIV_RX               :6;      ///<BIT [11:6] PCIE_0_DIV_RX
        uint32_t PCIE_0_GLITCH_RX_SEL        :1;      ///<BIT [12] PCIE_0_GLITCH_RX_SEL
        uint32_t PCIE_0_PORB                 :1;      ///<BIT [13] PCIE_0_PORB
        uint32_t PCIE_0_PU_OSC               :1;      ///<BIT [14] PCIE_0_PU_OSC
        uint32_t PCIE_0_RANGE_RX_SEL         :1;      ///<BIT [15] PCIE_0_RANGE_RX_SEL
        uint32_t PCIE_0_RESERVE_IN           :8;      ///<BIT [23:16] PCIE_0_RESERVE_IN
        uint32_t PCIE_0_RX_CLKDET_MODE       :1;      ///<BIT [24] PCIE_0_RX_CLKDET_MODE
        uint32_t PCIE_0_RX_HYSTERSIS_EN      :1;      ///<BIT [25] PCIE_0_RX_HYSTERSIS_EN
        uint32_t PCIE_0_RX_LOWPOWER_MODE     :1;      ///<BIT [26] PCIE_0_RX_LOWPOWER_MODE
        uint32_t PCIE_0_SPEED_OSC            :2;      ///<BIT [28:27] PCIE_0_SPEED_OSC
        uint32_t RESERVED0                   :3;      ///<BIT [31:29] Reserved0
    } b;
} Pcie0RefClockControl_t;

/// @brief 0x54
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_X4_BUTTON_RST          :1;      ///<BIT [0] PCIE_X4_BUTTON_RST
        uint32_t PCIE_X4_COLD_RST            :1;      ///<BIT [1] PCIE_X4_COLD_RST
        uint32_t RSVD_2                      :1;      ///<BIT [2] rsvd_2
        uint32_t RESERVED1                   :21;     ///<BIT [23:3] Reserved1
        uint32_t PHY_OFFSET_EN_ENABLE        :1;      ///<BIT [24] PHY_OFFSET_EN_ENABLE
        uint32_t CLKREQ_OFFSET_EN_ENABLE     :1;      ///<BIT [25] CLKREQ_OFFSET_EN_ENABLE
        uint32_t RESERVED0                   :6;      ///<BIT [31:26] Reserved0
    } b;
} PcieResetControl_t;

/// @brief 0x60
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t REF_FREF_SEL_P1             :5;      ///<BIT [4:0] REF_FREF_SEL_P1
        uint32_t RESERVED2                   :3;      ///<BIT [7:5] Reserved2
        uint32_t PHY_MODE_P1                 :3;      ///<BIT [10:8] PHY_MODE_P1
        uint32_t RESERVED1                   :5;      ///<BIT [15:11] Reserved1
        uint32_t REFCLK_SEL_P1               :1;      ///<BIT [16] REFCLK_SEL_P1
        uint32_t SRIS_ENABLE0_P1             :1;      ///<BIT [17] SRIS_ENABLE0_P1
        uint32_t SRIS_ENABLE1_P1             :1;      ///<BIT [18] SRIS_ENABLE1_P1
        uint32_t RESERVED0                   :5;      ///<BIT [23:19] Reserved0
        uint32_t RESERVED                    :8;      ///<BIT [31:24] Reserved
    } b;
} Ctrl0P1_t;

/// @brief 0x64
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESET_P1                    :1;      ///<BIT [0] RESET_P1
        uint32_t RESET_CORE0_P1              :1;      ///<BIT [1] RESET_CORE0_P1
        uint32_t RESET_CORE1_P1              :1;      ///<BIT [2] RESET_CORE1_P1
        uint32_t DIRECT_ACCESS_EN_P1         :1;      ///<BIT [3] DIRECT_ACCESS_EN_P1
        uint32_t RESERVED0                   :12;     ///<BIT [15:4] Reserved0
        uint32_t RESERVED_INPUT_P1           :16;     ///<BIT [31:16] RESERVED_INPUT_P1
    } b;
} Ctrl1P1_t;

/// @brief 0x68
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GPI0_P1                     :8;      ///<BIT [7:0] GPI0_P1
        uint32_t GPI1_P1                     :8;      ///<BIT [15:8] GPI1_P1
        uint32_t GPI_CMN_P1                  :8;      ///<BIT [23:16] GPI_CMN_P1
        uint32_t RESERVED0                   :8;      ///<BIT [31:24] Reserved0
    } b;
} Ctrl2P1_t;

/// @brief 0x6C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_OUTPUT_P1          :16;     ///<BIT [15:0] RESERVED_OUTPUT_P1
        uint32_t RESERVED0                   :16;     ///<BIT [31:16] Reserved0
    } b;
} ReservedOutputP1_t;

/// @brief 0x70
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_INPUT_RX0_P1       :16;     ///<BIT [15:0] RESERVED_INPUT_RX0_P1
        uint32_t RESERVED_INPUT_RX1_P1       :16;     ///<BIT [31:16] RESERVED_INPUT_RX1_P1
    } b;
} ReservedInputRxP1_t;

/// @brief 0x74
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_INPUT_TX0_P1       :16;     ///<BIT [15:0] RESERVED_INPUT_TX0_P1
        uint32_t RESERVED_INPUT_TX1_P1       :16;     ///<BIT [31:16] RESERVED_INPUT_TX1_P1
    } b;
} ReservedInputTxP1_t;

/// @brief 0x78
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_OUTPUT_RX0_P1      :16;     ///<BIT [15:0] RESERVED_OUTPUT_RX0_P1
        uint32_t RESERVED_OUTPUT_RX1_P1      :16;     ///<BIT [31:16] RESERVED_OUTPUT_RX1_P1
    } b;
} ReservedOutputRxP1_t;

/// @brief 0x7C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_OUTPUT_TX0_P1      :16;     ///<BIT [15:0] RESERVED_OUTPUT_TX0_P1
        uint32_t RESERVED_OUTPUT_TX1_P1      :16;     ///<BIT [31:16] RESERVED_OUTPUT_TX1_P1
    } b;
} ReservedOutputTxP1_t;

/// @brief 0x80
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PHY_INT_RAW_P0_EN           :1;      ///<BIT [0] PHY_INT_RAW_P0_EN
        uint32_t PHY_INT_EDG_P0_EN           :1;      ///<BIT [1] PHY_INT_EDG_P0_EN
        uint32_t MCU_WDT0_P0_EN              :1;      ///<BIT [2] MCU_WDT0_P0_EN
        uint32_t MCU_WDT1_P0_EN              :1;      ///<BIT [3] MCU_WDT1_P0_EN
        uint32_t MCU_WDT_CMN_P0_EN           :1;      ///<BIT [4] MCU_WDT_CMN_P0_EN
        uint32_t MEM_ECC_ERR0_P0_EN          :1;      ///<BIT [5] MEM_ECC_ERR0_P0_EN
        uint32_t MEM_ECC_ERR1_P0_EN          :1;      ///<BIT [6] MEM_ECC_ERR1_P0_EN
        uint32_t MEM_ECC_ERR_CMN_P0_EN       :1;      ///<BIT [7] MEM_ECC_ERR_CMN_P0_EN
        uint32_t RESERVED4                   :5;      ///<BIT [12:8] Reserved4
        uint32_t CK_RX_RDY_RDET_EN           :1;      ///<BIT [13] CK_RX_RDY_RDET_EN
        uint32_t CK_RX_RDY_FDET_EN           :1;      ///<BIT [14] CK_RX_RDY_FDET_EN
        uint32_t RESERVED3                   :1;      ///<BIT [15] Reserved3
        uint32_t PERST_N_RDET_AON_EN         :1;      ///<BIT [16] PERST_N_RDET_AON_EN
        uint32_t RESERVED2                   :1;      ///<BIT [17] Reserved2
        uint32_t PERST_N_FDET_AON_EN         :1;      ///<BIT [18] PERST_N_FDET_AON_EN
        uint32_t RESERVED1                   :1;      ///<BIT [19] Reserved1
        uint32_t PERST_N_H_EN                :1;      ///<BIT [20] PERST_N_H_EN
        uint32_t PERST_N_L_EN                :1;      ///<BIT [21] PERST_N_L_EN
        uint32_t RESERVED0                   :2;      ///<BIT [23:22] Reserved0
        uint32_t PHY_INT_RAW_P1_EN           :1;      ///<BIT [24] PHY_INT_RAW_P1_EN
        uint32_t PHY_INT_EDG_P1_EN           :1;      ///<BIT [25] PHY_INT_EDG_P1_EN
        uint32_t MCU_WDT0_P1_EN              :1;      ///<BIT [26] MCU_WDT0_P1_EN
        uint32_t MCU_WDT1_P1_EN              :1;      ///<BIT [27] MCU_WDT1_P1_EN
        uint32_t MCU_WDT_CMN_P1_EN           :1;      ///<BIT [28] MCU_WDT_CMN_P1_EN
        uint32_t MEM_ECC_ERR0_P1_EN          :1;      ///<BIT [29] MEM_ECC_ERR0_P1_EN
        uint32_t MEM_ECC_ERR1_P1_EN          :1;      ///<BIT [30] MEM_ECC_ERR1_P1_EN
        uint32_t MEM_ECC_ERR_CMN_P1_EN       :1;      ///<BIT [31] MEM_ECC_ERR_CMN_P1_EN
    } b;
} IntEn_t;

/// @brief 0x84
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PHY_INT_EDG_P0_STS          :1;      ///<BIT [0] PHY_INT_EDG_P0_STS
        uint32_t MCU_WDT0_P0_STS             :1;      ///<BIT [1] MCU_WDT0_P0_STS
        uint32_t MCU_WDT1_P0_STS             :1;      ///<BIT [2] MCU_WDT1_P0_STS
        uint32_t MCU_WDT_CMN_P0_STS          :1;      ///<BIT [3] MCU_WDT_CMN_P0_STS
        uint32_t MEM_ECC_ERR0_P0_STS         :1;      ///<BIT [4] MEM_ECC_ERR0_P0_STS
        uint32_t MEM_ECC_ERR1_P0_STS         :1;      ///<BIT [5] MEM_ECC_ERR1_P0_STS
        uint32_t MEM_ECC_ERR_CMN_P0_STS      :1;      ///<BIT [6] MEM_ECC_ERR_CMN_P0_STS
        uint32_t RESERVED3                   :1;      ///<BIT [7] Reserved3
        uint32_t RESERVED2                   :5;      ///<BIT [12:8] Reserved2
        uint32_t CK_RX_RDY_RDET_STS          :1;      ///<BIT [13] CK_RX_RDY_RDET_STS
        uint32_t CK_RX_RDY_FDET_STS          :1;      ///<BIT [14] CK_RX_RDY_FDET_STS
        uint32_t CK_RX_RDY_RAW_STS           :1;      ///<BIT [15] CK_RX_RDY_RAW_STS
        uint32_t PERST_N_RDET_AON_STS        :1;      ///<BIT [16] PERST_N_RDET_AON_STS
        uint32_t RSVD_17                     :1;      ///<BIT [17] rsvd_17
        uint32_t PERST_N_FDET_AON_STS        :1;      ///<BIT [18] PERST_N_FDET_AON_STS
        uint32_t RSVD_19                     :1;      ///<BIT [19] rsvd_19
        uint32_t PERST_N_RAW_STS             :1;      ///<BIT [20] PERST_N_RAW_STS
        uint32_t RESERVED1                   :3;      ///<BIT [23:21] Reserved1
        uint32_t PHY_INT_EDG_P1_STS          :1;      ///<BIT [24] PHY_INT_EDG_P1_STS
        uint32_t MCU_WDT0_P1_STS             :1;      ///<BIT [25] MCU_WDT0_P1_STS
        uint32_t MCU_WDT1_P1_STS             :1;      ///<BIT [26] MCU_WDT1_P1_STS
        uint32_t MCU_WDT_CMN_P1_STS          :1;      ///<BIT [27] MCU_WDT_CMN_P1_STS
        uint32_t MEM_ECC_ERR0_P1_STS         :1;      ///<BIT [28] MEM_ECC_ERR0_P1_STS
        uint32_t MEM_ECC_ERR1_P1_STS         :1;      ///<BIT [29] MEM_ECC_ERR1_P1_STS
        uint32_t MEM_ECC_ERR_CMN_P1_STS      :1;      ///<BIT [30] MEM_ECC_ERR_CMN_P1_STS
        uint32_t RESERVED0                   :1;      ///<BIT [31] Reserved0
    } b;
} IntSts_t;

/// @brief 0x88
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GPO0_P0                     :8;      ///<BIT [7:0] GPO0_P0
        uint32_t GPO1_P0                     :8;      ///<BIT [15:8] GPO1_P0
        uint32_t GPO_CMN_P0                  :8;      ///<BIT [23:16] GPO_CMN_P0
        uint32_t RESERVED0                   :8;      ///<BIT [31:24] Reserved0
    } b;
} GpoP0_t;

/// @brief 0x94
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GPO0_P1                     :8;      ///<BIT [7:0] GPO0_P1
        uint32_t GPO1_P1                     :8;      ///<BIT [15:8] GPO1_P1
        uint32_t GPO_CMN_P1                  :8;      ///<BIT [23:16] GPO_CMN_P1
        uint32_t RESERVED0                   :8;      ///<BIT [31:24] Reserved0
    } b;
} GpoP1_t;

/// @brief 0x98
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MCU_SEL                     :4;      ///<BIT [3:0] MCU_SEL
        uint32_t RESERVED0                   :28;     ///<BIT [31:4] Reserved0
    } b;
} McuCtrl_t;

/// @brief 0x9C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_2                  :2;      ///<BIT [1:0] reserved_2
        uint32_t REFCLK_RX_EN                :1;      ///<BIT [2] REFCLK_RX_EN
        uint32_t REFCLK_RX_EN_FWCTRL         :1;      ///<BIT [3] REFCLK_RX_EN_FWCTRL
        uint32_t RESERVED_1                  :4;      ///<BIT [7:4] reserved_1
        uint32_t REFCLK_RX_LOWOFFSET_MODE    :1;      ///<BIT [8] REFCLK_RX_LOWOFFSET_MODE
        uint32_t REFCLK_RX_OFFSET_POLARITY   :1;      ///<BIT [9] REFCLK_RX_OFFSET_POLARITY
        uint32_t REFCLK_RX_OFFSET_EN         :1;      ///<BIT [10] REFCLK_RX_OFFSET_EN
        uint32_t REFCLK_RX_SEL_EXT           :1;      ///<BIT [11] REFCLK_RX_SEL_EXT
        uint32_t REFCLK_RX_PU                :1;      ///<BIT [12] REFCLK_RX_PU
        uint32_t REFCLK_RX_TEST_ANA          :3;      ///<BIT [15:13] REFCLK_RX_TEST_ANA
        uint32_t REFCLK_RX_TEST_EN           :1;      ///<BIT [16] REFCLK_RX_TEST_EN
        uint32_t RESERVED_0                  :14;     ///<BIT [30:17] reserved_0
        uint32_t GLITCH_RX_SEL_H             :1;      ///<BIT [31] GLITCH_RX_SEL_H
    } b;
} Pcie0RefClockControlExt_t;

/// @brief 0xA0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AXI_REMAP_DIS               :1;      ///<BIT [0] AXI_REMAP_DIS
        uint32_t RSVD_1                      :1;      ///<BIT [1] rsvd_1
        uint32_t AXI_REMAP_CLR               :1;      ///<BIT [2] AXI_REMAP_CLR
        uint32_t FLR_VF_ACTIVE_SEL           :1;      ///<BIT [3] FLR_VF_ACTIVE_SEL
        uint32_t FLR_PF_ACTIVE_SEL           :1;      ///<BIT [4] FLR_PF_ACTIVE_SEL
        uint32_t DL_ACTIVE_EXIT_LINK_INSECURE :1;      ///<BIT [5] DL_ACTIVE_EXIT_LINK_INSECURE
        uint32_t RESERVED                    :26;     ///<BIT [31:6] reserved
    } b;
} MiscCtrl2_t;

/// @brief 0xBC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OSCCLK_DIS                  :1;      ///<BIT [0] OSCCLK_DIS
        uint32_t PIPE_PCLK_DIS               :1;      ///<BIT [1] PIPE_PCLK_DIS
        uint32_t RESERVED                    :30;     ///<BIT [31:2] reserved
    } b;
} DebugCtrl_t;

/// @brief 0xC0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSTR_WR_CNT                 :10;     ///<BIT [9:0] MSTR_WR_CNT
        uint32_t RESERVED1                   :6;      ///<BIT [15:10] reserved1
        uint32_t MSTR_RD_CNT                 :10;     ///<BIT [25:16] MSTR_RD_CNT
        uint32_t RESERVED0                   :6;      ///<BIT [31:26] reserved0
    } b;
} MstrCnt_t;

/// @brief 0xC4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SLV_WR_CNT                  :10;     ///<BIT [9:0] SLV_WR_CNT
        uint32_t RESERVED1                   :6;      ///<BIT [15:10] reserved1
        uint32_t SLV_RD_CNT                  :10;     ///<BIT [25:16] SLV_RD_CNT
        uint32_t RESERVED0                   :6;      ///<BIT [31:26] reserved0
    } b;
} SlvCnt_t;

/// @brief 0xD8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FLR_PF_ACTIVE_CLR           :1;      ///<BIT [0] FLR_PF_ACTIVE_CLR
        uint32_t RESERVED                    :31;     ///<BIT [31:1] reserved
    } b;
} FlrPfActiveClr_t;

/// @brief 0xDC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FLR_PF_ACTIVE_LATCH         :1;      ///<BIT [0] FLR_PF_ACTIVE_LATCH
        uint32_t RESERVED                    :31;     ///<BIT [31:1] reserved
    } b;
} FlrPfActiveLatch_t;

typedef struct
{
    Ctrl0P0_t ctrl0P0;                                                      // 0x0 : CTRL0_P0 / 
    Ctrl1P0_t ctrl1P0;                                                      // 0x4 : CTRL1_P0 / 
    Ctrl2P0_t ctrl2P0;                                                      // 0x8 : CTRL2_P0 / 
    ReservedOutputP0_t reservedOutputP0;                                    // 0xC : RESERVED_OUTPUT_P0 / 
    ReservedInputRxP0_t reservedInputRxP0;                                  // 0x10 : RESERVED_INPUT_RX_P0 / 
    ReservedInputTxP0_t reservedInputTxP0;                                  // 0x14 : RESERVED_INPUT_TX_P0 / 
    ReservedOutputRxP0_t reservedOutputRxP0;                                // 0x18 : RESERVED_OUTPUT_RX_P0 / 
    ReservedOutputTxP0_t reservedOutputTxP0;                                // 0x1C : RESERVED_OUTPUT_TX_P0 / 
    MiscCtrl0_t miscCtrl0;                                                  // 0x20 : MISC_CTRL0 / 
    ResetDis_t resetDis;                                                    // 0x24 : RESET_DIS / 
    PhyStrapP0_t phyStrapP0;                                                // 0x28 : PHY_STRAP_P0 / 
    uint8_t rsvd2c[4];                                                      // 0x2C : rsvd_2c / rsvd_2c
    PerstDbLTrhd_t perstDbLTrhd;                                            // 0x30 : PERST_DB_L_TRHD / 
    PerstDbHTrhd_t perstDbHTrhd;                                            // 0x34 : PERST_DB_H_TRHD / 
    uint8_t rsvd38[8];                                                      // 0x38 : rsvd_38 / rsvd_38
    MiscCtrl1_t miscCtrl1;                                                  // 0x40 : MISC_CTRL1 / 
    ElaCtrl_t elaCtrl;                                                      // 0x44 : ELA_CTRL / 
    PhyStrap1_t phyStrap1;                                                  // 0x48 : PHY_STRAP_1 / 
    uint8_t rsvd4c[4];                                                      // 0x4C : rsvd_4c / rsvd_4c
    Pcie0RefClockControl_t pcie0RefClockControl;                            // 0x50 : PCIE_0_Ref_Clock_Control / 
    PcieResetControl_t pcieResetControl;                                    // 0x54 : PCIe_Reset_Control / 
    uint8_t rsvd58[8];                                                      // 0x58 : rsvd_58 / rsvd_58
    Ctrl0P1_t ctrl0P1;                                                      // 0x60 : CTRL0_P1 / 
    Ctrl1P1_t ctrl1P1;                                                      // 0x64 : CTRL1_P1 / 
    Ctrl2P1_t ctrl2P1;                                                      // 0x68 : CTRL2_P1 / 
    ReservedOutputP1_t reservedOutputP1;                                    // 0x6C : RESERVED_OUTPUT_P1 / 
    ReservedInputRxP1_t reservedInputRxP1;                                  // 0x70 : RESERVED_INPUT_RX_P1 / 
    ReservedInputTxP1_t reservedInputTxP1;                                  // 0x74 : RESERVED_INPUT_TX_P1 / 
    ReservedOutputRxP1_t reservedOutputRxP1;                                // 0x78 : RESERVED_OUTPUT_RX_P1 / 
    ReservedOutputTxP1_t reservedOutputTxP1;                                // 0x7C : RESERVED_OUTPUT_TX_P1 / 
    IntEn_t intEn;                                                          // 0x80 : INT_EN / 
    IntSts_t intSts;                                                        // 0x84 : INT_STS / 
    GpoP0_t gpoP0;                                                          // 0x88 : GPO_P0 / 
    uint8_t rsvd8c[8];                                                      // 0x8C : rsvd_8c / rsvd_8c
    GpoP1_t gpoP1;                                                          // 0x94 : GPO_P1 / 
    McuCtrl_t mcuCtrl;                                                      // 0x98 : MCU_CTRL / 
    Pcie0RefClockControlExt_t pcie0RefClockControlExt;                      // 0x9C : PCIE_0_Ref_Clock_Control_ext / 
    MiscCtrl2_t miscCtrl2;                                                  // 0xA0 : MISC_CTRL2 / 
    uint8_t rsvdA4[24];                                                     // 0xA4 : rsvd_a4 / rsvd_a4
    DebugCtrl_t debugCtrl;                                                  // 0xBC : DEBUG_CTRL / 
    MstrCnt_t mstrCnt;                                                      // 0xC0 : MSTR_CNT / 
    SlvCnt_t slvCnt;                                                        // 0xC4 : SLV_CNT / 
    uint32_t flrVfActiveClrLo;                                              // 0xC8 : FLR_VF_ACTIVE_CLR_LO / 
    uint32_t flrVfActiveClrHi;                                              // 0xCC : FLR_VF_ACTIVE_CLR_HI / 
    uint32_t flrVfActiveLatchLo;                                            // 0xD0 : FLR_VF_ACTIVE_LATCH_LO / 
    uint32_t flrVfActiveLatchHi;                                            // 0xD4 : FLR_VF_ACTIVE_LATCH_HI / 
    FlrPfActiveClr_t flrPfActiveClr;                                        // 0xD8 : FLR_PF_ACTIVE_CLR / 
    FlrPfActiveLatch_t flrPfActiveLatch;                                    // 0xDC : FLR_PF_ACTIVE_LATCH / 
} PcieTop_t;

COMPILE_ASSERT(offsetof(PcieTop_t,ctrl0P0)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,ctrl1P0)==0x4,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,ctrl2P0)==0x8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,reservedOutputP0)==0xC,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,reservedInputRxP0)==0x10,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,reservedInputTxP0)==0x14,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,reservedOutputRxP0)==0x18,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,reservedOutputTxP0)==0x1C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,miscCtrl0)==0x20,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,resetDis)==0x24,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,phyStrapP0)==0x28,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,perstDbLTrhd)==0x30,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,perstDbHTrhd)==0x34,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,miscCtrl1)==0x40,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,elaCtrl)==0x44,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,phyStrap1)==0x48,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,pcie0RefClockControl)==0x50,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,pcieResetControl)==0x54,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,ctrl0P1)==0x60,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,ctrl1P1)==0x64,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,ctrl2P1)==0x68,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,reservedOutputP1)==0x6C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,reservedInputRxP1)==0x70,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,reservedInputTxP1)==0x74,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,reservedOutputRxP1)==0x78,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,reservedOutputTxP1)==0x7C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,intEn)==0x80,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,intSts)==0x84,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,gpoP0)==0x88,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,gpoP1)==0x94,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,mcuCtrl)==0x98,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,pcie0RefClockControlExt)==0x9C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,miscCtrl2)==0xA0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,debugCtrl)==0xBC,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,mstrCnt)==0xC0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,slvCnt)==0xC4,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,flrVfActiveClrLo)==0xC8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,flrVfActiveClrHi)==0xCC,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,flrVfActiveLatchLo)==0xD0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,flrVfActiveLatchHi)==0xD4,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,flrPfActiveClr)==0xD8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTop_t,flrPfActiveLatch)==0xDC,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile PcieTop_t rPcieTop; ///< 0xB0160000
