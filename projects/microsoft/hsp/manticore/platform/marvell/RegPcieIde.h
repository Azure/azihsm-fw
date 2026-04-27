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
//! @brief PCIE_IDE Registers
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
        uint32_t VERSION_NUM                 :16;     ///<BIT [15:0] VERSION_NUM
        uint32_t RESERVED_31_16              :16;     ///<BIT [31:16] RESERVED_31_16
    } b;
} PcieIdeCoreVerNum_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TYPE_NUM                    :8;      ///<BIT [7:0] TYPE_NUM
        uint32_t PKG_NUM                     :4;      ///<BIT [11:8] PKG_NUM
        uint32_t TYPE_ENUM                   :4;      ///<BIT [15:12] TYPE_ENUM
        uint32_t RESERVED_31_16              :16;     ///<BIT [31:16] RESERVED_31_16
    } b;
} PcieIdeCoreVerType_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IDE_TX_BYPASS_EN            :1;      ///<BIT [0] IDE_TX_BYPASS_EN
        uint32_t RESERVED_1                  :1;      ///<BIT [1] RESERVED_1
        uint32_t IDE_RX_BYPASS_EN            :1;      ///<BIT [2] IDE_RX_BYPASS_EN
        uint32_t RESERVED_3                  :1;      ///<BIT [3] RESERVED_3
        uint32_t IDE_SRAM_ECC_EN             :1;      ///<BIT [4] IDE_SRAM_ECC_EN
        uint32_t RESERVED_5                  :1;      ///<BIT [5] RESERVED_5
        uint32_t IDE_TBIT_SRC_SEL            :1;      ///<BIT [6] IDE_TBIT_SRC_SEL
        uint32_t IDE_TBIT_IF_SRC_SEL         :1;      ///<BIT [7] IDE_TBIT_IF_SRC_SEL
        uint32_t IDE_DATAPATH_PROT_EN        :1;      ///<BIT [8] IDE_DATAPATH_PROT_EN
        uint32_t RSVD_9                      :1;      ///<BIT [9] rsvd_9
        uint32_t RESERVED_31_10              :22;     ///<BIT [31:10] RESERVED_31_10
    } b;
} IdeGlblCfg_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IDE_SYNC_MSG_REQ_THRESHOLD  :9;      ///<BIT [8:0] IDE_SYNC_MSG_REQ_THRESHOLD
        uint32_t RESERVED_31_9               :23;     ///<BIT [31:9] RESERVED_31_9
    } b;
} IdeSyncMsgCfg_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TBIT_CFG_S0_PR              :1;      ///<BIT [0] TBIT_CFG_S0_PR
        uint32_t TBIT_CFG_S0_NPR             :1;      ///<BIT [1] TBIT_CFG_S0_NPR
        uint32_t TBIT_CFG_S0_CPL             :1;      ///<BIT [2] TBIT_CFG_S0_CPL
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} TxLinkTbitCfg_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TBIT_CFG_S0_PR              :1;      ///<BIT [0] TBIT_CFG_S0_PR
        uint32_t TBIT_CFG_S0_NPR             :1;      ///<BIT [1] TBIT_CFG_S0_NPR
        uint32_t TBIT_CFG_S0_CPL             :1;      ///<BIT [2] TBIT_CFG_S0_CPL
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} TxSltTbit1Cfg_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t KBIT_CFG_S0_PR              :1;      ///<BIT [0] KBIT_CFG_S0_PR
        uint32_t KBIT_CFG_S0_NPR             :1;      ///<BIT [1] KBIT_CFG_S0_NPR
        uint32_t KBIT_CFG_S0_CPL             :1;      ///<BIT [2] KBIT_CFG_S0_CPL
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} TxLnkKbitCfg_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t KBIT_CFG_S0_PR              :1;      ///<BIT [0] KBIT_CFG_S0_PR
        uint32_t KBIT_CFG_S0_NPR             :1;      ///<BIT [1] KBIT_CFG_S0_NPR
        uint32_t KBIT_CFG_S0_CPL             :1;      ///<BIT [2] KBIT_CFG_S0_CPL
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} TxSltKbit1Cfg_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_KBIT_CFG_S0_PR           :1;      ///<BIT [0] RX_KBIT_CFG_S0_PR
        uint32_t RX_KBIT_CFG_S0_NPR          :1;      ///<BIT [1] RX_KBIT_CFG_S0_NPR
        uint32_t RX_KBIT_CFG_S0_CPL          :1;      ///<BIT [2] RX_KBIT_CFG_S0_CPL
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} RxLnkKbitCfg_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_KBIT_CFG_S0_PR           :1;      ///<BIT [0] RX_KBIT_CFG_S0_PR
        uint32_t RX_KBIT_CFG_S0_NPR          :1;      ///<BIT [1] RX_KBIT_CFG_S0_NPR
        uint32_t RX_KBIT_CFG_S0_CPL          :1;      ///<BIT [2] RX_KBIT_CFG_S0_CPL
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} RxSltKbit1Cfg_t;

/// @brief 0x2C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_KEY_THRESH_EN            :1;      ///<BIT [0] TX_KEY_THRESH_EN
        uint32_t RX_KEY_THRESH_EN            :1;      ///<BIT [1] RX_KEY_THRESH_EN
        uint32_t TX_KEY_COUNTER_AUTOCLEAR_EN :1;      ///<BIT [2] TX_KEY_COUNTER_AUTOCLEAR_EN
        uint32_t RX_KEY_COUNTER_AUTOCLEAR_EN :1;      ///<BIT [3] RX_KEY_COUNTER_AUTOCLEAR_EN
        uint32_t RESERVED_31_4               :28;     ///<BIT [31:4] RESERVED_31_4
    } b;
} KeyThreshEn_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_REKEY_REQ_STATUS_S0      :1;      ///<BIT [0] TX_REKEY_REQ_STATUS_S0
        uint32_t TX_REKEY_REQ_STATUS_S1      :1;      ///<BIT [1] TX_REKEY_REQ_STATUS_S1
        uint32_t RSVD_2_15                   :14;     ///<BIT [15:2] rsvd_2_15
        uint32_t RX_REKEY_REQ_STATUS_S0      :1;      ///<BIT [16] RX_REKEY_REQ_STATUS_S0
        uint32_t RX_REKEY_REQ_STATUS_S1      :1;      ///<BIT [17] RX_REKEY_REQ_STATUS_S1
        uint32_t RSVD_18_31                  :14;     ///<BIT [31:18] rsvd_18_31
    } b;
} RekeyReqStat_t;

/// @brief 0x24C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_CIPHER_KEY_MEM_ECC_STAT  :2;      ///<BIT [1:0] TX_CIPHER_KEY_MEM_ECC_STAT
        uint32_t TX_HASH_KEY_MEM_ECC_STAT    :2;      ///<BIT [3:2] TX_HASH_KEY_MEM_ECC_STAT
        uint32_t RSVD_4_5                    :2;      ///<BIT [5:4] rsvd_4_5
        uint32_t RX_CIPHER_KEY_MEM_ECC_STAT  :2;      ///<BIT [7:6] RX_CIPHER_KEY_MEM_ECC_STAT
        uint32_t RX_HASH_KEY_MEM_ECC_STAT    :2;      ///<BIT [9:8] RX_HASH_KEY_MEM_ECC_STAT
        uint32_t RSVD_10_11                  :2;      ///<BIT [11:10] rsvd_10_11
        uint32_t RX2_CIPHER_KEY_MEM_ECC_STAT :2;      ///<BIT [13:12] RX2_CIPHER_KEY_MEM_ECC_STAT
        uint32_t RX2_HASH_KEY_MEM_ECC_STAT   :2;      ///<BIT [15:14] RX2_HASH_KEY_MEM_ECC_STAT
        uint32_t RSVD_16_17                  :2;      ///<BIT [17:16] rsvd_16_17
        uint32_t RESERVED_31_18              :14;     ///<BIT [31:18] RESERVED_31_18
    } b;
} SramEccStatus_t;

/// @brief 0x250
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_KEY_SWAP_STAT_S0_PR      :1;      ///<BIT [0] TX_KEY_SWAP_STAT_S0_PR
        uint32_t TX_KEY_SWAP_STAT_S0_NPR     :1;      ///<BIT [1] TX_KEY_SWAP_STAT_S0_NPR
        uint32_t TX_KEY_SWAP_STAT_S0_CPL     :1;      ///<BIT [2] TX_KEY_SWAP_STAT_S0_CPL
        uint32_t RX_KEY_SWAP_STAT_S0_PR      :1;      ///<BIT [3] RX_KEY_SWAP_STAT_S0_PR
        uint32_t RX_KEY_SWAP_STAT_S0_NPR     :1;      ///<BIT [4] RX_KEY_SWAP_STAT_S0_NPR
        uint32_t RX_KEY_SWAP_STAT_S0_CPL     :1;      ///<BIT [5] RX_KEY_SWAP_STAT_S0_CPL
        uint32_t RESERVED_31_6               :26;     ///<BIT [31:6] RESERVED_31_6
    } b;
} KeySecurityStatusS0_t;

/// @brief 0x254
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_KEY_SWAP_STAT_S1_PR      :1;      ///<BIT [0] TX_KEY_SWAP_STAT_S1_PR
        uint32_t TX_KEY_SWAP_STAT_S1_NPR     :1;      ///<BIT [1] TX_KEY_SWAP_STAT_S1_NPR
        uint32_t TX_KEY_SWAP_STAT_S1_CPL     :1;      ///<BIT [2] TX_KEY_SWAP_STAT_S1_CPL
        uint32_t RX_KEY_SWAP_STAT_S1_PR      :1;      ///<BIT [3] RX_KEY_SWAP_STAT_S1_PR
        uint32_t RX_KEY_SWAP_STAT_S1_NPR     :1;      ///<BIT [4] RX_KEY_SWAP_STAT_S1_NPR
        uint32_t RX_KEY_SWAP_STAT_S1_CPL     :1;      ///<BIT [5] RX_KEY_SWAP_STAT_S1_CPL
        uint32_t RESERVED_31_6               :26;     ///<BIT [31:6] RESERVED_31_6
    } b;
} KeySecurityStatusS1_t;

/// @brief 0x2D4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OVERFLOW_STATUS             :1;      ///<BIT [0] OVERFLOW_STATUS
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} IdeCounterOverflow_t;

/// @brief 0x2D8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SRAM_ECC_IRQ_EN_GLBL        :1;      ///<BIT [0] SRAM_ECC_IRQ_EN_GLBL
        uint32_t KEY_SECURITY_IRQ_EN_GLB     :1;      ///<BIT [1] KEY_SECURITY_IRQ_EN_GLB
        uint32_t IDE_CNT_OVF_IRQ_EN_GLB      :1;      ///<BIT [2] IDE_CNT_OVF_IRQ_EN_GLB
        uint32_t RX_MISROUTED_IRQ_EN_GLBL    :1;      ///<BIT [3] RX_MISROUTED_IRQ_EN_GLBL
        uint32_t RX_CHECK_FAILED_IRQ_EN_GLBL :1;      ///<BIT [4] RX_CHECK_FAILED_IRQ_EN_GLBL
        uint32_t RSVD_5                      :1;      ///<BIT [5] rsvd_5
        uint32_t RX_PCRC_ERR_IRQ_EN_GLBL     :1;      ///<BIT [6] RX_PCRC_ERR_IRQ_EN_GLBL
        uint32_t TX_KBIT_TOGGLED_IRQ_EN_GLBL :1;      ///<BIT [7] TX_KBIT_TOGGLED_IRQ_EN_GLBL
        uint32_t RX_KBIT_TOGGLED_IRQ_EN_GLBL :1;      ///<BIT [8] RX_KBIT_TOGGLED_IRQ_EN_GLBL
        uint32_t TX_REKEY_REQ_IRQ_EN_GLBL    :1;      ///<BIT [9] TX_REKEY_REQ_IRQ_EN_GLBL
        uint32_t RX_REKEY_REQ_IRQ_EN_GLBL    :1;      ///<BIT [10] RX_REKEY_REQ_IRQ_EN_GLBL
        uint32_t DATAPATH_PROT_IRQ_EN_GLBL   :1;      ///<BIT [11] DATAPATH_PROT_IRQ_EN_GLBL
        uint32_t INSEC_STREAM_IRQ_EN_GLBL    :1;      ///<BIT [12] INSEC_STREAM_IRQ_EN_GLBL
        uint32_t RESERVED_30_13              :18;     ///<BIT [30:13] RESERVED_30_13
        uint32_t IRQ_EN_GLBL                 :1;      ///<BIT [31] IRQ_EN_GLBL
    } b;
} IdeIrqEn_t;

/// @brief 0x2DC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_CIPHER_KEY_MEM_ECC_EN    :1;      ///<BIT [0] TX_CIPHER_KEY_MEM_ECC_EN
        uint32_t TX_HASH_KEY_MEM_ECC_EN      :1;      ///<BIT [1] TX_HASH_KEY_MEM_ECC_EN
        uint32_t RSVD_2                      :1;      ///<BIT [2] rsvd_2
        uint32_t RX_CIPHER_KEY_MEM_ECC_EN    :1;      ///<BIT [3] RX_CIPHER_KEY_MEM_ECC_EN
        uint32_t RX_HASH_KEY_MEM_ECC_EN      :1;      ///<BIT [4] RX_HASH_KEY_MEM_ECC_EN
        uint32_t RSVD_5                      :1;      ///<BIT [5] rsvd_5
        uint32_t RX2_CIPHER_KEY_MEM_ECC_EN   :1;      ///<BIT [6] RX2_CIPHER_KEY_MEM_ECC_EN
        uint32_t RX2_HASH_KEY_MEM_ECC_EN     :1;      ///<BIT [7] RX2_HASH_KEY_MEM_ECC_EN
        uint32_t RSVD_8                      :1;      ///<BIT [8] rsvd_8
        uint32_t RESERVED_31_9               :23;     ///<BIT [31:9] RESERVED_31_9
    } b;
} IdeSramEccIrqEn_t;

/// @brief 0x2E0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t KEY_SECURITY_S0_IRQ_EN      :1;      ///<BIT [0] KEY_SECURITY_S0_IRQ_EN
        uint32_t KEY_SECURITY_S1_IRQ_EN      :1;      ///<BIT [1] KEY_SECURITY_S1_IRQ_EN
        uint32_t RESERVED_S2                 :1;      ///<BIT [2] RESERVED_S2
        uint32_t RESERVED_S3                 :1;      ///<BIT [3] RESERVED_S3
        uint32_t RESERVED_S4                 :1;      ///<BIT [4] RESERVED_S4
        uint32_t RESERVED_S5                 :1;      ///<BIT [5] RESERVED_S5
        uint32_t RESERVED_S6                 :1;      ///<BIT [6] RESERVED_S6
        uint32_t RESERVED_S7                 :1;      ///<BIT [7] RESERVED_S7
        uint32_t RESERVED_S8                 :1;      ///<BIT [8] RESERVED_S8
        uint32_t RESERVED_S9                 :1;      ///<BIT [9] RESERVED_S9
        uint32_t RESERVED_S10                :1;      ///<BIT [10] RESERVED_S10
        uint32_t RESERVED_S11                :1;      ///<BIT [11] RESERVED_S11
        uint32_t RESERVED_S12                :1;      ///<BIT [12] RESERVED_S12
        uint32_t RESERVED_S13                :1;      ///<BIT [13] RESERVED_S13
        uint32_t RESERVED_S14                :1;      ///<BIT [14] RESERVED_S14
        uint32_t RESERVED_S15                :1;      ///<BIT [15] RESERVED_S15
        uint32_t RESERVED_S16                :1;      ///<BIT [16] RESERVED_S16
        uint32_t RESERVED_S17                :1;      ///<BIT [17] RESERVED_S17
        uint32_t RESERVED_S18                :1;      ///<BIT [18] RESERVED_S18
        uint32_t RESERVED_S19                :1;      ///<BIT [19] RESERVED_S19
        uint32_t RESERVED_S20                :1;      ///<BIT [20] RESERVED_S20
        uint32_t RESERVED_S21                :1;      ///<BIT [21] RESERVED_S21
        uint32_t RESERVED_S22                :1;      ///<BIT [22] RESERVED_S22
        uint32_t RESERVED_S23                :1;      ///<BIT [23] RESERVED_S23
        uint32_t RESERVED_S24                :1;      ///<BIT [24] RESERVED_S24
        uint32_t RESERVED_S25                :1;      ///<BIT [25] RESERVED_S25
        uint32_t RESERVED_S26                :1;      ///<BIT [26] RESERVED_S26
        uint32_t RESERVED_S27                :1;      ///<BIT [27] RESERVED_S27
        uint32_t RESERVED_S28                :1;      ///<BIT [28] RESERVED_S28
        uint32_t RESERVED_S29                :1;      ///<BIT [29] RESERVED_S29
        uint32_t RESERVED_S30                :1;      ///<BIT [30] RESERVED_S30
        uint32_t RESERVED_S31                :1;      ///<BIT [31] RESERVED_S31
    } b;
} KeySecurityIrqEn_t;

/// @brief 0x2E4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_MISROUTED_IRQ_GLBL       :1;      ///<BIT [0] RX_MISROUTED_IRQ_GLBL
        uint32_t RX2_MISROUTED_IRQ_GLBL      :1;      ///<BIT [1] RX2_MISROUTED_IRQ_GLBL
        uint32_t RX_CHECK_FAILED_IRQ_GLBL    :1;      ///<BIT [2] RX_CHECK_FAILED_IRQ_GLBL
        uint32_t RX2_CHECK_FAILED_IRQ_GLBL   :1;      ///<BIT [3] RX2_CHECK_FAILED_IRQ_GLBL
        uint32_t RSVD_4                      :1;      ///<BIT [4] rsvd_4
        uint32_t RX_PCRC_ERR_IRQ_GLBL        :1;      ///<BIT [5] RX_PCRC_ERR_IRQ_GLBL
        uint32_t RX2_PCRC_ERR_IRQ_GLBL       :1;      ///<BIT [6] RX2_PCRC_ERR_IRQ_GLBL
        uint32_t TX_KBIT_TOGGLED_IRQ_GLBL    :1;      ///<BIT [7] TX_KBIT_TOGGLED_IRQ_GLBL
        uint32_t RX_KBIT_TOGGLED_IRQ_GLBL    :1;      ///<BIT [8] RX_KBIT_TOGGLED_IRQ_GLBL
        uint32_t RESERVED_9                  :1;      ///<BIT [9] RESERVED_9
        uint32_t TX_REKEY_REQ_IRQ_GLBL       :1;      ///<BIT [10] TX_REKEY_REQ_IRQ_GLBL
        uint32_t RX_REKEY_REQ_IRQ_GLBL       :1;      ///<BIT [11] RX_REKEY_REQ_IRQ_GLBL
        uint32_t INSEC_STREAM_IRQ_GLBL       :1;      ///<BIT [12] INSEC_STREAM_IRQ_GLBL
        uint32_t RESERVED_31_13              :19;     ///<BIT [31:13] RESERVED_31_13
    } b;
} IdeIoIrqStatus_t;

/// @brief 0x2E8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LNK_SND_STREAM_0_INSECURE   :1;      ///<BIT [0] LNK_SND_STREAM_0_INSECURE
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} LnkSndStreamInsecure_t;

/// @brief 0x2EC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SLT_SND_STREAM_0_INSECURE   :1;      ///<BIT [0] SLT_SND_STREAM_0_INSECURE
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} SltSndStreamInsecure_t;

/// @brief 0x300
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IDE_ECC_ERROR_INJ_OFFSET    :6;      ///<BIT [5:0] IDE_ECC_ERROR_INJ_OFFSET
        uint32_t RESERVED_15_6               :10;     ///<BIT [15:6] RESERVED_15_6
        uint32_t IDE_TX_ECC_ERROR_INJ_CKEY   :1;      ///<BIT [16] IDE_TX_ECC_ERROR_INJ_CKEY
        uint32_t IDE_TX_ECC_ERROR_INJ_HKEY   :1;      ///<BIT [17] IDE_TX_ECC_ERROR_INJ_HKEY
        uint32_t RSVD_18                     :1;      ///<BIT [18] rsvd_18
        uint32_t IDE_RX_ECC_ERROR_INJ_CKEY   :1;      ///<BIT [19] IDE_RX_ECC_ERROR_INJ_CKEY
        uint32_t IDE_RX_ECC_ERROR_INJ_HKEY   :1;      ///<BIT [20] IDE_RX_ECC_ERROR_INJ_HKEY
        uint32_t RSVD_21                     :1;      ///<BIT [21] rsvd_21
        uint32_t IDE_RX2_ECC_ERROR_INJ_CKEY  :1;      ///<BIT [22] IDE_RX2_ECC_ERROR_INJ_CKEY
        uint32_t IDE_RX2_ECC_ERROR_INJ_HKEY  :1;      ///<BIT [23] IDE_RX2_ECC_ERROR_INJ_HKEY
        uint32_t RSVD_24                     :1;      ///<BIT [24] rsvd_24
        uint32_t RESERVED_31_25              :7;      ///<BIT [31:25] RESERVED_31_25
    } b;
} IdeEccErrorInjCtrl_t;

/// @brief 0x304
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IDE_TX_ECC_ERROR_INJ_STAT   :1;      ///<BIT [0] IDE_TX_ECC_ERROR_INJ_STAT
        uint32_t IDE_RX_ECC_ERROR_INJ_STAT   :1;      ///<BIT [1] IDE_RX_ECC_ERROR_INJ_STAT
        uint32_t RESERVED_31_2               :30;     ///<BIT [31:2] RESERVED_31_2
    } b;
} IdeEccErrorInjStat_t;

/// @brief 0x308
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_LNK_KBIT_TOGGLED_S0_PR   :1;      ///<BIT [0] TX_LNK_KBIT_TOGGLED_S0_PR
        uint32_t TX_LNK_KBIT_TOGGLED_S0_NPR  :1;      ///<BIT [1] TX_LNK_KBIT_TOGGLED_S0_NPR
        uint32_t TX_LNK_KBIT_TOGGLED_S0_CPL  :1;      ///<BIT [2] TX_LNK_KBIT_TOGGLED_S0_CPL
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} TxLnkKbitToggled_t;

/// @brief 0x30C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_LNK_KBIT_TOGGLED_S0_PR   :1;      ///<BIT [0] RX_LNK_KBIT_TOGGLED_S0_PR
        uint32_t RX_LNK_KBIT_TOGGLED_S0_NPR  :1;      ///<BIT [1] RX_LNK_KBIT_TOGGLED_S0_NPR
        uint32_t RX_LNK_KBIT_TOGGLED_S0_CPL  :1;      ///<BIT [2] RX_LNK_KBIT_TOGGLED_S0_CPL
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} RxLnkKbitToggled_t;

/// @brief 0x310
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_LNK_KBIT_CURRENT_S0_PR   :1;      ///<BIT [0] RX_LNK_KBIT_CURRENT_S0_PR
        uint32_t RX_LNK_KBIT_CURRENT_S0_NPR  :1;      ///<BIT [1] RX_LNK_KBIT_CURRENT_S0_NPR
        uint32_t RX_LNK_KBIT_CURRENT_S0_CPL  :1;      ///<BIT [2] RX_LNK_KBIT_CURRENT_S0_CPL
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} RxLnkKbitCurrent_t;

/// @brief 0x314
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_SLT_KBIT_TOGGLED_S0_PR   :1;      ///<BIT [0] TX_SLT_KBIT_TOGGLED_S0_PR
        uint32_t TX_SLT_KBIT_TOGGLED_S0_NPR  :1;      ///<BIT [1] TX_SLT_KBIT_TOGGLED_S0_NPR
        uint32_t TX_SLT_KBIT_TOGGLED_S0_CPL  :1;      ///<BIT [2] TX_SLT_KBIT_TOGGLED_S0_CPL
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} TxSltKbitToggled_t;

/// @brief 0x318
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_SLT_KBIT_TOGGLED_S0_PR   :1;      ///<BIT [0] RX_SLT_KBIT_TOGGLED_S0_PR
        uint32_t RX_SLT_KBIT_TOGGLED_S0_NPR  :1;      ///<BIT [1] RX_SLT_KBIT_TOGGLED_S0_NPR
        uint32_t RX_SLT_KBIT_TOGGLED_S0_CPL  :1;      ///<BIT [2] RX_SLT_KBIT_TOGGLED_S0_CPL
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} RxSltKbitToggled_t;

/// @brief 0x31C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_SLT_KBIT_CURRENT_S0_PR   :1;      ///<BIT [0] RX_SLT_KBIT_CURRENT_S0_PR
        uint32_t RX_SLT_KBIT_CURRENT_S0_NPR  :1;      ///<BIT [1] RX_SLT_KBIT_CURRENT_S0_NPR
        uint32_t RX_SLT_KBIT_CURRENT_S0_CPL  :1;      ///<BIT [2] RX_SLT_KBIT_CURRENT_S0_CPL
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} RxSltKbitCurrent_t;

/// @brief 0x320
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DATAPATH_PROT_TX_PREFIX_IRQ_EN :1;      ///<BIT [0] DATAPATH_PROT_TX_PREFIX_IRQ_EN
        uint32_t DATAPATH_PROT_TX_HDR_IRQ_EN :1;      ///<BIT [1] DATAPATH_PROT_TX_HDR_IRQ_EN
        uint32_t DATAPATH_PROT_TX_DATA_IRQ_EN :1;      ///<BIT [2] DATAPATH_PROT_TX_DATA_IRQ_EN
        uint32_t DATAPATH_PROT_TX_MAC_IRQ_EN :1;      ///<BIT [3] DATAPATH_PROT_TX_MAC_IRQ_EN
        uint32_t DATAPATH_PROT_TX_PCRC_IRQ_EN :1;      ///<BIT [4] DATAPATH_PROT_TX_PCRC_IRQ_EN
        uint32_t DATAPATH_PROT_RX_PREFIX_IRQ_EN :1;      ///<BIT [5] DATAPATH_PROT_RX_PREFIX_IRQ_EN
        uint32_t DATAPATH_PROT_RX_HDR_IRQ_EN :1;      ///<BIT [6] DATAPATH_PROT_RX_HDR_IRQ_EN
        uint32_t DATAPATH_PROT_RX_DATA_IRQ_EN :1;      ///<BIT [7] DATAPATH_PROT_RX_DATA_IRQ_EN
        uint32_t DATAPATH_PROT_RX_PCRC_MAC_BUS_IRQ_EN :1;      ///<BIT [8] DATAPATH_PROT_RX_PCRC_MAC_BUS_IRQ_EN
        uint32_t DATAPATH_PROT_RX2_PREFIX_IRQ_EN :1;      ///<BIT [9] DATAPATH_PROT_RX2_PREFIX_IRQ_EN
        uint32_t DATAPATH_PROT_RX2_HDR_IRQ_EN :1;      ///<BIT [10] DATAPATH_PROT_RX2_HDR_IRQ_EN
        uint32_t DATAPATH_PROT_RX2_DATA_IRQ_EN :1;      ///<BIT [11] DATAPATH_PROT_RX2_DATA_IRQ_EN
        uint32_t DATAPATH_PROT_RX2_PCRC_MAC_BUS_IRQ_EN :1;      ///<BIT [12] DATAPATH_PROT_RX2_PCRC_MAC_BUS_IRQ_EN
        uint32_t RESERVED_31_13              :19;     ///<BIT [31:13] RESERVED_31_13
    } b;
} DatapathProtIrqEn_t;

/// @brief 0x324
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DATAPATH_PROT_TX_PREFIX_IRQ_STATUS :2;      ///<BIT [1:0] DATAPATH_PROT_TX_PREFIX_IRQ_STATUS
        uint32_t DATAPATH_PROT_TX_HDR_IRQ_STATUS :2;      ///<BIT [3:2] DATAPATH_PROT_TX_HDR_IRQ_STATUS
        uint32_t DATAPATH_PROT_TX_DATA_IRQ_STATUS :2;      ///<BIT [5:4] DATAPATH_PROT_TX_DATA_IRQ_STATUS
        uint32_t DATAPATH_PROT_TX_MAC_IRQ_STATUS :2;      ///<BIT [7:6] DATAPATH_PROT_TX_MAC_IRQ_STATUS
        uint32_t DATAPATH_PROT_TX_PCRC_IRQ_STATUS :2;      ///<BIT [9:8] DATAPATH_PROT_TX_PCRC_IRQ_STATUS
        uint32_t DATAPATH_PROT_RX_PREFIX_IRQ_STATUS :2;      ///<BIT [11:10] DATAPATH_PROT_RX_PREFIX_IRQ_STATUS
        uint32_t DATAPATH_PROT_RX_HDR_IRQ_STATUS :2;      ///<BIT [13:12] DATAPATH_PROT_RX_HDR_IRQ_STATUS
        uint32_t DATAPATH_PROT_RX_DATA_IRQ_STATUS :2;      ///<BIT [15:14] DATAPATH_PROT_RX_DATA_IRQ_STATUS
        uint32_t DATAPATH_PROT_RX_PCRC_MAC_BUS_IRQ_STATUS :2;      ///<BIT [17:16] DATAPATH_PROT_RX_PCRC_MAC_BUS_IRQ_STATUS
        uint32_t DATAPATH_PROT_RX2_PREFIX_IRQ_STATUS :2;      ///<BIT [19:18] DATAPATH_PROT_RX2_PREFIX_IRQ_STATUS
        uint32_t DATAPATH_PROT_RX2_HDR_IRQ_STATUS :2;      ///<BIT [21:20] DATAPATH_PROT_RX2_HDR_IRQ_STATUS
        uint32_t DATAPATH_PROT_RX2_DATA_IRQ_STATUS :2;      ///<BIT [23:22] DATAPATH_PROT_RX2_DATA_IRQ_STATUS
        uint32_t DATAPATH_PROT_RX2_PCRC_MAC_BUS_IRQ_STATUS :2;      ///<BIT [25:24] DATAPATH_PROT_RX2_PCRC_MAC_BUS_IRQ_STATUS
        uint32_t RESERVED_31_26              :6;      ///<BIT [31:26] RESERVED_31_26
    } b;
} DatapathProtIrqStat_t;

/// @brief 0x328
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DATAPATH_PROT_ERROR_INJ_EN  :1;      ///<BIT [0] DATAPATH_PROT_ERROR_INJ_EN
        uint32_t DATAPATH_PROT_ERROR_INJ_TYPE :2;      ///<BIT [2:1] DATAPATH_PROT_ERROR_INJ_TYPE
        uint32_t DATAPATH_PROT_ERROR_INJ_COUNT :8;      ///<BIT [10:3] DATAPATH_PROT_ERROR_INJ_COUNT
        uint32_t DATAPATH_PROT_ERROR_INJ_LOC :5;      ///<BIT [15:11] DATAPATH_PROT_ERROR_INJ_LOC
        uint32_t RESERVED_31_16              :16;     ///<BIT [31:16] RESERVED_31_16
    } b;
} DatapathProtErrorInjCtrl_t;

/// @brief 0x430
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_PCRC_ERROR_CNT           :8;      ///<BIT [7:0] TX_PCRC_ERROR_CNT
        uint32_t RESERVED_31_8               :24;     ///<BIT [31:8] RESERVED_31_8
    } b;
} IdeTxPcrcErrCnt_t;

/// @brief 0x438
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_PCRC_ERROR_CNT           :8;      ///<BIT [7:0] RX_PCRC_ERROR_CNT
        uint32_t RESERVED_31_8               :24;     ///<BIT [31:8] RESERVED_31_8
    } b;
} IdeRxPcrcErrCnt_t;

/// @brief 0x440
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX2_PCRC_ERROR_CNT          :8;      ///<BIT [7:0] RX2_PCRC_ERROR_CNT
        uint32_t RESERVED_31_8               :24;     ///<BIT [31:8] RESERVED_31_8
    } b;
} IdeRx2PcrcErrCnt_t;

/// @brief 0x448
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LINK_0_TDISP_ON             :1;      ///<BIT [0] LINK_0_TDISP_ON
        uint32_t RSVD_1_15                   :15;     ///<BIT [15:1] rsvd_1_15
        uint32_t SLCT_0_TDISP_ON             :1;      ///<BIT [16] SLCT_0_TDISP_ON
        uint32_t RSVD_17_31                  :15;     ///<BIT [31:17] rsvd_17_31
    } b;
} IdeTdispOn_t;

/// @brief 0x44C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DISCARD_OVERRIDE            :1;      ///<BIT [0] DISCARD_OVERRIDE
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} IdeDiscardOverride_t;

/// @brief 0x500
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IDE_MSG_ON_SELECTIVE_EN     :1;      ///<BIT [0] IDE_MSG_ON_SELECTIVE_EN
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} IdeMsgOnSelectiveEn_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CAP_ID                      :16;     ///<BIT [15:0] CAP_ID
        uint32_t CAP_VER                     :4;      ///<BIT [19:16] CAP_VER
        uint32_t NEXT_CAP_OFFSET             :12;     ///<BIT [31:20] NEXT_CAP_OFFSET
    } b;
} IdeExtCapHdr_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LINK_IDE_STREAM_SUPPORTED   :1;      ///<BIT [0] LINK_IDE_STREAM_SUPPORTED
        uint32_t SLCT_IDE_STREAM_SUPPORTED   :1;      ///<BIT [1] SLCT_IDE_STREAM_SUPPORTED
        uint32_t FLOWTHROUGH_IDE_STREAM_SUPPORTED :1;      ///<BIT [2] FLOWTHROUGH_IDE_STREAM_SUPPORTED
        uint32_t IDE_PARTIAL_HEADER_ENCRYPTION_SUPPORTED :1;      ///<BIT [3] IDE_PARTIAL_HEADER_ENCRYPTION_SUPPORTED
        uint32_t AGGREGATION_SUPPORTED       :1;      ///<BIT [4] AGGREGATION_SUPPORTED
        uint32_t PCRC_SUPPORTED              :1;      ///<BIT [5] PCRC_SUPPORTED
        uint32_t IDE_KM_PROTOCOL_SUPPORTED   :1;      ///<BIT [6] IDE_KM_PROTOCOL_SUPPORTED
        uint32_t SLCT_IDE_CONF_REQ_SUPPORTED :1;      ///<BIT [7] SLCT_IDE_CONF_REQ_SUPPORTED
        uint32_t SUPPORTED_ALGORITHM         :5;      ///<BIT [12:8] SUPPORTED_ALGORITHM
        uint32_t NUM_TC_SUPPORTED_FOR_LINK_IDE :3;      ///<BIT [15:13] NUM_TC_SUPPORTED_FOR_LINK_IDE
        uint32_t NUM_SLCT_IDE_STREAMS_SUPPORTED :8;      ///<BIT [23:16] NUM_SLCT_IDE_STREAMS_SUPPORTED
        uint32_t TEE_LIMITED_STREAM_SUPPORTED :1;      ///<BIT [24] TEE_LIMITED_STREAM_SUPPORTED
        uint32_t RESERVED_31_25              :7;      ///<BIT [31:25] RESERVED_31_25
    } b;
} IdeCap_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_1_0                :2;      ///<BIT [1:0] RESERVED_1_0
        uint32_t FLOWTHROUGH_IDE_STREAM_ENABLED :1;      ///<BIT [2] FLOWTHROUGH_IDE_STREAM_ENABLED
        uint32_t RESERVED_31_3               :29;     ///<BIT [31:3] RESERVED_31_3
    } b;
} IdeCtrl_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LINK_IDE_STREAM_ENABLED     :1;      ///<BIT [0] LINK_IDE_STREAM_ENABLED
        uint32_t RESERVED_1                  :1;      ///<BIT [1] RESERVED_1
        uint32_t RSVD_2_7                    :6;      ///<BIT [7:2] rsvd_2_7
        uint32_t PCRC_ENABLE                 :1;      ///<BIT [8] PCRC_ENABLE
        uint32_t RESERVED_9                  :1;      ///<BIT [9] RESERVED_9
        uint32_t RSVD_10_13                  :4;      ///<BIT [13:10] rsvd_10_13
        uint32_t SELECTED_ALGORITHM          :5;      ///<BIT [18:14] SELECTED_ALGORITHM
        uint32_t TC                          :3;      ///<BIT [21:19] TC
        uint32_t RESERVED_23_22              :2;      ///<BIT [23:22] RESERVED_23_22
        uint32_t STREAM_ID                   :8;      ///<BIT [31:24] STREAM_ID
    } b;
} IdeLinkStreamCtrl0_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LINK_IDE_STREAM_STATE       :4;      ///<BIT [3:0] LINK_IDE_STREAM_STATE
        uint32_t RESERVED_30_4               :27;     ///<BIT [30:4] RESERVED_30_4
        uint32_t RECEIVED_INTEGRITY_CHECK_FAIL_MSG :1;      ///<BIT [31] RECEIVED_INTEGRITY_CHECK_FAIL_MSG
    } b;
} IdeLinkStatus0_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t NUM_ADDR_ASSOS_REG_BLOCKS   :4;      ///<BIT [3:0] NUM_ADDR_ASSOS_REG_BLOCKS
        uint32_t RESERVED_31_4               :28;     ///<BIT [31:4] RESERVED_31_4
    } b;
} IdeSlctIdeStreamCap0_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SLCT_IDE_STREAM_ENABLED     :1;      ///<BIT [0] SLCT_IDE_STREAM_ENABLED
        uint32_t RESERVED_1                  :1;      ///<BIT [1] RESERVED_1
        uint32_t RSVD_2_7                    :6;      ///<BIT [7:2] rsvd_2_7
        uint32_t PCRC_ENABLE                 :1;      ///<BIT [8] PCRC_ENABLE
        uint32_t SLCR_IDE_CFG_REQ_ENABLE     :1;      ///<BIT [9] SLCR_IDE_CFG_REQ_ENABLE
        uint32_t RSVD_10_13                  :4;      ///<BIT [13:10] rsvd_10_13
        uint32_t SELECTED_ALGORITHM          :5;      ///<BIT [18:14] SELECTED_ALGORITHM
        uint32_t TC                          :3;      ///<BIT [21:19] TC
        uint32_t DEFAULT_STREAM              :1;      ///<BIT [22] DEFAULT_STREAM
        uint32_t TEE_LIMITED_STREAM          :1;      ///<BIT [23] TEE_LIMITED_STREAM
        uint32_t STREAM_ID                   :8;      ///<BIT [31:24] STREAM_ID
    } b;
} IdeSlctIdeStreamCtrl0_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SLCT_IDE_STREAM_STATE       :4;      ///<BIT [3:0] SLCT_IDE_STREAM_STATE
        uint32_t RESERVED_31_4               :27;     ///<BIT [30:4] RESERVED_31_4
        uint32_t RECEIVED_INTEGRITY_CHECK_FAIL_MSG :1;      ///<BIT [31] RECEIVED_INTEGRITY_CHECK_FAIL_MSG
    } b;
} IdeSlctIdeStreamStatus0_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_7_0                :8;      ///<BIT [7:0] RESERVED_7_0
        uint32_t RID_LIMIT                   :16;     ///<BIT [23:8] RID_LIMIT
        uint32_t RSVD_24_31                  :8;      ///<BIT [31:24] rsvd_24_31
    } b;
} IdeRidAssosReg10_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RID_VALID                   :1;      ///<BIT [0] RID_VALID
        uint32_t RESERVED_7_1                :7;      ///<BIT [7:1] RESERVED_7_1
        uint32_t RID_BASE                    :16;     ///<BIT [23:8] RID_BASE
        uint32_t RSVD_24_31                  :8;      ///<BIT [31:24] rsvd_24_31
    } b;
} IdeRidAssosReg20_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ASSOC_BLOCK_VALID           :1;      ///<BIT [0] ASSOC_BLOCK_VALID
        uint32_t RESERVED_7_1                :7;      ///<BIT [7:1] RESERVED_7_1
        uint32_t MEM_BASE_LOWER              :12;     ///<BIT [19:8] MEM_BASE_LOWER
        uint32_t MEM_LIMIT_LOWER             :12;     ///<BIT [31:20] MEM_LIMIT_LOWER
    } b;
} IdeAddrAssosReg100_t;

/// @brief 0x2000
typedef struct
{
    IdeExtCapHdr_t ideExtCapHdr;          //IDE_EXT_CAP_HDR
    IdeCap_t ideCap;                      //IDE_CAP
    IdeCtrl_t ideCtrl;                    //IDE_CTRL
    IdeLinkStreamCtrl0_t ideLinkStreamCtrl0; //IDE_LINK_STREAM_CTRL_0
    IdeLinkStatus0_t ideLinkStatus0;      //IDE_LINK_STATUS_0
    IdeSlctIdeStreamCap0_t ideSlctIdeStreamCap0; //IDE_SLCT_IDE_STREAM_CAP_0
    IdeSlctIdeStreamCtrl0_t ideSlctIdeStreamCtrl0; //IDE_SLCT_IDE_STREAM_CTRL_0
    IdeSlctIdeStreamStatus0_t ideSlctIdeStreamStatus0; //IDE_SLCT_IDE_STREAM_STATUS_0
    IdeRidAssosReg10_t ideRidAssosReg10;  //IDE_RID_ASSOS_REG1_0
    IdeRidAssosReg20_t ideRidAssosReg20;  //IDE_RID_ASSOS_REG2_0
    IdeAddrAssosReg100_t ideAddrAssosReg100; //IDE_ADDR_ASSOS_REG1_0_0
    uint32_t ideAddrAssosReg200MemLimitUpper; //IDE_ADDR_ASSOS_REG2_0_0
    uint32_t ideAddrAssosReg300MemBaseUpper; //IDE_ADDR_ASSOS_REG3_0_0
} IdIdeCaps_t;

/// @brief 0x0
typedef struct
{
    PcieIdeCoreVerNum_t coreVerNum;       //CORE_VER_NUM
    PcieIdeCoreVerType_t coreVerType;     //CORE_VER_TYPE
    IdeGlblCfg_t ideGlblCfg;              //IDE_GLBL_CFG
    IdeSyncMsgCfg_t ideSyncMsgCfg;        //IDE_SYNC_MSG_CFG
    TxLinkTbitCfg_t txLinkTbitCfg;        //TX_LINK_TBIT_CFG
    TxSltTbit1Cfg_t txSltTbit1Cfg;        //TX_SLT_TBIT1_CFG
    uint8_t rsvd18[4];                    //rsvd_18
    TxLnkKbitCfg_t txLnkKbitCfg;          //TX_LNK_KBIT_CFG
    TxSltKbit1Cfg_t txSltKbit1Cfg;        //TX_SLT_KBIT1_CFG
    RxLnkKbitCfg_t rxLnkKbitCfg;          //RX_LNK_KBIT_CFG
    RxSltKbit1Cfg_t rxSltKbit1Cfg;        //RX_SLT_KBIT1_CFG
    KeyThreshEn_t keyThreshEn;            //KEY_THRESH_EN
    RekeyReqStat_t rekeyReqStat;          //REKEY_REQ_STAT
    uint8_t rsvd34[12];                   //rsvd_34
    uint32_t txKeyThrLowS0TxKeyThresholdLowS0; //TX_KEY_THRESHOLD_LOW_S0
    uint32_t txKeyThrHighS0TxKeyThresholdHighS0; //TX_KEY_THRESHOLD_HIGH_S0
    uint32_t rxKeyThrLowS0RxKeyThresholdLowS0; //RX_KEY_THRESHOLD_LOW_S0
    uint32_t rxKeyThrHighS0RxKeyThresholdHighS0; //RX_KEY_THRESHOLD_HIGH_S0
    uint32_t txKeyThrLowS1TxKeyThresholdLowS1; //TX_KEY_THRESHOLD_LOW_S1
    uint32_t txKeyThrHighS1TxKeyThresholdHighS1; //TX_KEY_THRESHOLD_HIGH_S1
    uint32_t rxKeyThrLowS1RxKeyThresholdLowS1; //RX_KEY_THRESHOLD_LOW_S1
    uint32_t rxKeyThrHighS1RxKeyThresholdHighS1; //RX_KEY_THRESHOLD_HIGH_S1
    uint8_t rsvd60[492];                  //rsvd_60
    SramEccStatus_t sramEccStatus;        //SRAM_ECC_STATUS
    KeySecurityStatusS0_t keySecurityStatusS0; //KEY_SECURITY_STATUS_S0
    KeySecurityStatusS1_t keySecurityStatusS1; //KEY_SECURITY_STATUS_S1
    uint8_t rsvd258[124];                 //rsvd_258
    IdeCounterOverflow_t ideCounterOverflow; //IDE_COUNTER_OVERFLOW
    IdeIrqEn_t ideIrqEn;                  //IDE_IRQ_EN
    IdeSramEccIrqEn_t ideSramEccIrqEn;    //IDE_SRAM_ECC_IRQ_EN
    KeySecurityIrqEn_t keySecurityIrqEn;  //KEY_SECURITY_IRQ_EN
    IdeIoIrqStatus_t ideIoIrqStatus;      //IDE_IO_IRQ_STATUS
    LnkSndStreamInsecure_t lnkSndStreamInsecure; //LNK_SND_STREAM_INSECURE
    SltSndStreamInsecure_t sltSndStreamInsecure; //SLT_SND_STREAM_INSECURE
    uint8_t rsvd2f0[12];                  //rsvd_2f0
    uint32_t ideEccErrorInj;              //IDE_ECC_ERROR_INJ
    IdeEccErrorInjCtrl_t ideEccErrorInjCtrl; //IDE_ECC_ERROR_INJ_CTRL
    IdeEccErrorInjStat_t ideEccErrorInjStat; //IDE_ECC_ERROR_INJ_STAT
    TxLnkKbitToggled_t txLnkKbitToggled;  //TX_LNK_KBIT_TOGGLED
    RxLnkKbitToggled_t rxLnkKbitToggled;  //RX_LNK_KBIT_TOGGLED
    RxLnkKbitCurrent_t rxLnkKbitCurrent;  //RX_LNK_KBIT_CURRENT
    TxSltKbitToggled_t txSltKbitToggled;  //TX_SLT_KBIT_TOGGLED
    RxSltKbitToggled_t rxSltKbitToggled;  //RX_SLT_KBIT_TOGGLED
    RxSltKbitCurrent_t rxSltKbitCurrent;  //RX_SLT_KBIT_CURRENT
    DatapathProtIrqEn_t datapathProtIrqEn; //DATAPATH_PROT_IRQ_EN
    DatapathProtIrqStat_t datapathProtIrqStat; //DATAPATH_PROT_IRQ_STAT
    DatapathProtErrorInjCtrl_t datapathProtErrorInjCtrl; //DATAPATH_PROT_ERROR_INJ_CTRL
    uint32_t txKeyUsageCounterLowS0;      //TX_KEY_USAGE_COUNTER_LOW_S0
    uint32_t txKeyUsageCounterHighS0;     //TX_KEY_USAGE_COUNTER_HIGH_S0
    uint32_t rxKeyUsageCounterLowS0;      //RX_KEY_USAGE_COUNTER_LOW_S0
    uint32_t rxKeyUsageCounterHighS0;     //RX_KEY_USAGE_COUNTER_HIGH_S0
    uint32_t txKeyUsageCounterLowS1;      //TX_KEY_USAGE_COUNTER_LOW_S1
    uint32_t txKeyUsageCounterHighS1;     //TX_KEY_USAGE_COUNTER_HIGH_S1
    uint32_t rxKeyUsageCounterLowS1;      //RX_KEY_USAGE_COUNTER_LOW_S1
    uint32_t rxKeyUsageCounterHighS1;     //RX_KEY_USAGE_COUNTER_HIGH_S1
    uint8_t rsvd34c[224];                 //rsvd_34c
    uint32_t ideTxPcrcErrMaskTxPcrcErrorMask; //IDE_TX_PCRC_ERR_MASK
    IdeTxPcrcErrCnt_t ideTxPcrcErrCnt;    //IDE_TX_PCRC_ERR_CNT
    uint32_t ideRxPcrcErrMaskRxPcrcErrorMask; //IDE_RX_PCRC_ERR_MASK
    IdeRxPcrcErrCnt_t ideRxPcrcErrCnt;    //IDE_RX_PCRC_ERR_CNT
    uint32_t ideRx2PcrcErrMaskRx2PcrcErrorMask; //IDE_RX2_PCRC_ERR_MASK
    IdeRx2PcrcErrCnt_t ideRx2PcrcErrCnt;  //IDE_RX2_PCRC_ERR_CNT
    uint8_t rsvd444[4];                   //rsvd_444
    IdeTdispOn_t ideTdispOn;              //IDE_TDISP_ON
    IdeDiscardOverride_t ideDiscardOverride; //IDE_DISCARD_OVERRIDE
    uint8_t rsvd450[176];                 //rsvd_450
    IdeMsgOnSelectiveEn_t ideMsgOnSelectiveEn; //IDE_MSG_ON_SELECTIVE_EN
} IdIdeCfg_t;

/// @brief 0x0
typedef struct
{
    IdIdeCfg_t idIdeCfg;                  //id_ide_cfg
    uint8_t rsvd504[6908];                //rsvd_504
    IdIdeCaps_t idIdeCaps;                //id_ide_caps
} DwcPcieIdeApb_t;

typedef struct
{
    DwcPcieIdeApb_t dwcPcieIdeApb;                                          // 0x0 : DWC_pcie_ide_apb / 
} PcieIde_t;

COMPILE_ASSERT(offsetof(PcieIde_t,dwcPcieIdeApb)==0x0,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile PcieIde_t rPcieIde; ///< 0xB01E0000
