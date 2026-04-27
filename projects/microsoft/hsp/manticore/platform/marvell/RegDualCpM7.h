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
//! @brief DUAL_CP_M7 Registers
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
        uint32_t INITIATOR_ID                :6;      ///<BIT [5:0] initiator_id
        uint32_t RSVD_6_31                   :26;     ///<BIT [31:6] rsvd_6_31
    } b;
} CpId_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PARITY_ERR_INJ              :1;      ///<BIT [0] ParityErrInj
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} CpParityerrinj_t;

/// @brief 0x100
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CLKEN                       :1;      ///<BIT [0] clken
        uint32_t FCLKEN                      :1;      ///<BIT [1] fclken
        uint32_t HCLKEN                      :1;      ///<BIT [2] hclken
        uint32_t ETMCLKEN                    :1;      ///<BIT [3] etmclken
        uint32_t STCLKEN                     :1;      ///<BIT [4] stclken
        uint32_t RSVD_5_7                    :3;      ///<BIT [7:5] rsvd_5_7
        uint32_t SYSRESET_N                  :1;      ///<BIT [8] sysreset_n
        uint32_t DBGETMRESET_N               :1;      ///<BIT [9] dbgetmreset_n
        uint32_t NMI                         :1;      ///<BIT [10] nmi
        uint32_t RSVD_11_15                  :5;      ///<BIT [15:11] rsvd_11_15
        uint32_t TCM_WTSEL                   :2;      ///<BIT [17:16] tcm_wtsel
        uint32_t TCM_RTSEL                   :2;      ///<BIT [19:18] tcm_rtsel
        uint32_t TCM_ECC_EN                  :1;      ///<BIT [20] tcm_ecc_en
        uint32_t TCM_SD_EN                   :1;      ///<BIT [21] tcm_sd_en
        uint32_t TCM_SLP_EN                  :1;      ///<BIT [22] tcm_slp_en
        uint32_t RSVD_23                     :1;      ///<BIT [23] rsvd_23
        uint32_t CACHE_WTSEL                 :2;      ///<BIT [25:24] cache_wtsel
        uint32_t CACHE_RTSEL                 :2;      ///<BIT [27:26] cache_rtsel
        uint32_t RSVD_28                     :1;      ///<BIT [28] rsvd_28
        uint32_t CACHE_SD_EN                 :1;      ///<BIT [29] cache_sd_en
        uint32_t CACHE_SLP_EN                :1;      ///<BIT [30] cache_slp_en
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Cp0Ctl0_t;

/// @brief 0x104
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CTLPPBLOCK                  :4;      ///<BIT [3:0] ctlppblock
        uint32_t TSCLKCHANGE                 :1;      ///<BIT [4] tsclkchange
        uint32_t WICENREQ                    :1;      ///<BIT [5] wicenreq
        uint32_t SLEEPHOLDREQ_N              :1;      ///<BIT [6] sleepholdreq_n
        uint32_t SYSRESETREQ_EN              :1;      ///<BIT [7] sysresetreq_en
        uint32_t AWQOS                       :4;      ///<BIT [11:8] awqos
        uint32_t ARQOS                       :4;      ///<BIT [15:12] arqos
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp0Ctl1_t;

/// @brief 0x108
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_6                    :7;      ///<BIT [6:0] rsvd_0_6
        uint32_t INITVTOR                    :25;     ///<BIT [31:7] initvtor
    } b;
} Cp0Ctl2_t;

/// @brief 0x10C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFGSTCALIB                  :26;     ///<BIT [25:0] cfgstcalib
        uint32_t RSVD_26_31                  :6;      ///<BIT [31:26] rsvd_26_31
    } b;
} Cp0Ctl3_t;

/// @brief 0x114
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ECOREVNUM35_32              :4;      ///<BIT [3:0] ecorevnum35_32
        uint32_t RSVD_4_31                   :28;     ///<BIT [31:4] rsvd_4_31
    } b;
} Cp0Ctl5_t;

/// @brief 0x118
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LOCKUP                      :1;      ///<BIT [0] lockup
        uint32_t SLEEPING                    :1;      ///<BIT [1] sleeping
        uint32_t SLEEPDEEP                   :1;      ///<BIT [2] sleepdeep
        uint32_t SLEEPHOLDACK_N              :1;      ///<BIT [3] sleepholdack_n
        uint32_t GATEHCLK                    :1;      ///<BIT [4] gatehclk
        uint32_t ETMPWRUPREQ                 :1;      ///<BIT [5] etmpwrupreq
        uint32_t WICENACK                    :1;      ///<BIT [6] wicenack
        uint32_t WAKEUP                      :1;      ///<BIT [7] wakeup
        uint32_t TRCENA                      :1;      ///<BIT [8] trcena
        uint32_t RSVD_9_23                   :15;     ///<BIT [23:9] rsvd_9_23
        uint32_t SYSRESETREQ_STATUS          :1;      ///<BIT [24] sysresetreq_status
        uint32_t RSVD_25_31                  :7;      ///<BIT [31:25] rsvd_25_31
    } b;
} Cp0Sts0_t;

/// @brief 0x11C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PARITYERRINJ_AXIM           :1;      ///<BIT [0] parityerrinj_axim
        uint32_t PARITYERRINJ_AHBS           :1;      ///<BIT [1] parityerrinj_ahbs
        uint32_t RSVD_2_7                    :6;      ///<BIT [7:2] rsvd_2_7
        uint32_t ERRINJ_ITCM                 :8;      ///<BIT [15:8] errinj_itcm
        uint32_t ERRINJ_D0TCM                :7;      ///<BIT [22:16] errinj_d0tcm
        uint32_t RSVD_23                     :1;      ///<BIT [23] rsvd_23
        uint32_t ERRINJ_D1TCM                :7;      ///<BIT [30:24] errinj_d1tcm
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Cp0CtlErrinj_t;

/// @brief 0x120
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ITCM_ERR_EN                 :4;      ///<BIT [3:0] itcm_err_en
        uint32_t RSVD_4_7                    :4;      ///<BIT [7:4] rsvd_4_7
        uint32_t D0TCM_ERR_EN                :4;      ///<BIT [11:8] d0tcm_err_en
        uint32_t RSVD_12_15                  :4;      ///<BIT [15:12] rsvd_12_15
        uint32_t D1TCM_ERR_EN                :4;      ///<BIT [19:16] d1tcm_err_en
        uint32_t RSVD_20_31                  :12;     ///<BIT [31:20] rsvd_20_31
    } b;
} Cp0CtlEcc_t;

/// @brief 0x124
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ITCM_ERR                    :2;      ///<BIT [1:0] itcm_err
        uint32_t RSVD_2_7                    :6;      ///<BIT [7:2] rsvd_2_7
        uint32_t D0TCM_ERR                   :2;      ///<BIT [9:8] d0tcm_err
        uint32_t RSVD_10_15                  :6;      ///<BIT [15:10] rsvd_10_15
        uint32_t D1TCM_ERR                   :2;      ///<BIT [17:16] d1tcm_err
        uint32_t RSVD_18_31                  :14;     ///<BIT [31:18] rsvd_18_31
    } b;
} Cp0StsTcmErr_t;

/// @brief 0x128
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_COUNT                 :16;     ///<BIT [15:0] error_count
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp0CtlItcmCorrectable_t;

/// @brief 0x12C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_COUNT                 :16;     ///<BIT [15:0] error_count
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp0CtlItcmUncorrectable_t;

/// @brief 0x130
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_THRESHOLD             :16;     ///<BIT [15:0] error_threshold
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp0CtlItcmThr_t;

/// @brief 0x134
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_2                    :3;      ///<BIT [2:0] rsvd_0_2
        uint32_t ERROR_TCMADDR               :21;     ///<BIT [23:3] error_tcmaddr
        uint32_t ERROR_TCMMASTER             :4;      ///<BIT [27:24] error_tcmmaster
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Cp0StsItcm_t;

/// @brief 0x138
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_COUNT                 :16;     ///<BIT [15:0] error_count
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp0CtlD0tcmCorrectable_t;

/// @brief 0x13C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_COUNT                 :16;     ///<BIT [15:0] error_count
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp0CtlD0tcmUncorrectable_t;

/// @brief 0x140
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_THRESHOLD             :16;     ///<BIT [15:0] error_threshold
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp0CtlD0tcmThr_t;

/// @brief 0x144
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_2                    :3;      ///<BIT [2:0] rsvd_0_2
        uint32_t ERROR_TCMADDR               :21;     ///<BIT [23:3] error_tcmaddr
        uint32_t ERROR_TCMMASTER             :4;      ///<BIT [27:24] error_tcmmaster
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Cp0StsD0tcm_t;

/// @brief 0x148
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_COUNT                 :16;     ///<BIT [15:0] error_count
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp0CtlD1tcmCorrectable_t;

/// @brief 0x14C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_COUNT                 :16;     ///<BIT [15:0] error_count
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp0CtlD1tcmUncorrectable_t;

/// @brief 0x150
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_THRESHOLD             :16;     ///<BIT [15:0] error_threshold
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp0CtlD1tcmThr_t;

/// @brief 0x154
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_2                    :3;      ///<BIT [2:0] rsvd_0_2
        uint32_t ERROR_TCMADDR               :21;     ///<BIT [23:3] error_tcmaddr
        uint32_t ERROR_TCMMASTER             :4;      ///<BIT [27:24] error_tcmmaster
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Cp0StsD1tcm_t;

/// @brief 0x158
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ICERR                       :22;     ///<BIT [21:0] icerr
        uint32_t RSVD_22_23                  :2;      ///<BIT [23:22] rsvd_22_23
        uint32_t ICDET                       :4;      ///<BIT [27:24] icdet
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Cp0StsIcacheError_t;

/// @brief 0x15C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DCERR                       :22;     ///<BIT [21:0] dcerr
        uint32_t RSVD_22_23                  :2;      ///<BIT [23:22] rsvd_22_23
        uint32_t DCDET                       :4;      ///<BIT [27:24] dcdet
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Cp0StsDcacheError_t;

/// @brief 0x200
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CLKEN                       :1;      ///<BIT [0] clken
        uint32_t FCLKEN                      :1;      ///<BIT [1] fclken
        uint32_t HCLKEN                      :1;      ///<BIT [2] hclken
        uint32_t ETMCLKEN                    :1;      ///<BIT [3] etmclken
        uint32_t STCLKEN                     :1;      ///<BIT [4] stclken
        uint32_t RSVD_5_7                    :3;      ///<BIT [7:5] rsvd_5_7
        uint32_t SYSRESET_N                  :1;      ///<BIT [8] sysreset_n
        uint32_t DBGETMRESET_N               :1;      ///<BIT [9] dbgetmreset_n
        uint32_t NMI                         :1;      ///<BIT [10] nmi
        uint32_t RSVD_11_15                  :5;      ///<BIT [15:11] rsvd_11_15
        uint32_t TCM_WTSEL                   :2;      ///<BIT [17:16] tcm_wtsel
        uint32_t TCM_RTSEL                   :2;      ///<BIT [19:18] tcm_rtsel
        uint32_t TCM_ECC_EN                  :1;      ///<BIT [20] tcm_ecc_en
        uint32_t TCM_SD_EN                   :1;      ///<BIT [21] tcm_sd_en
        uint32_t TCM_SLP_EN                  :1;      ///<BIT [22] tcm_slp_en
        uint32_t RSVD_23                     :1;      ///<BIT [23] rsvd_23
        uint32_t CACHE_WTSEL                 :2;      ///<BIT [25:24] cache_wtsel
        uint32_t CACHE_RTSEL                 :2;      ///<BIT [27:26] cache_rtsel
        uint32_t RSVD_28                     :1;      ///<BIT [28] rsvd_28
        uint32_t CACHE_SD_EN                 :1;      ///<BIT [29] cache_sd_en
        uint32_t CACHE_SLP_EN                :1;      ///<BIT [30] cache_slp_en
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Cp1Ctl0_t;

/// @brief 0x204
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CTLPPBLOCK                  :4;      ///<BIT [3:0] ctlppblock
        uint32_t TSCLKCHANGE                 :1;      ///<BIT [4] tsclkchange
        uint32_t WICENREQ                    :1;      ///<BIT [5] wicenreq
        uint32_t SLEEPHOLDREQ_N              :1;      ///<BIT [6] sleepholdreq_n
        uint32_t SYSRESETREQ_EN              :1;      ///<BIT [7] sysresetreq_en
        uint32_t AWQOS                       :4;      ///<BIT [11:8] awqos
        uint32_t ARQOS                       :4;      ///<BIT [15:12] arqos
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp1Ctl1_t;

/// @brief 0x208
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_6                    :7;      ///<BIT [6:0] rsvd_0_6
        uint32_t INITVTOR                    :25;     ///<BIT [31:7] initvtor
    } b;
} Cp1Ctl2_t;

/// @brief 0x20C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFGSTCALIB                  :26;     ///<BIT [25:0] cfgstcalib
        uint32_t RSVD_26_31                  :6;      ///<BIT [31:26] rsvd_26_31
    } b;
} Cp1Ctl3_t;

/// @brief 0x214
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ECOREVNUM35_32              :4;      ///<BIT [3:0] ecorevnum35_32
        uint32_t RSVD_4_31                   :28;     ///<BIT [31:4] rsvd_4_31
    } b;
} Cp1Ctl5_t;

/// @brief 0x218
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LOCKUP                      :1;      ///<BIT [0] lockup
        uint32_t SLEEPING                    :1;      ///<BIT [1] sleeping
        uint32_t SLEEPDEEP                   :1;      ///<BIT [2] sleepdeep
        uint32_t SLEEPHOLDACK_N              :1;      ///<BIT [3] sleepholdack_n
        uint32_t GATEHCLK                    :1;      ///<BIT [4] gatehclk
        uint32_t ETMPWRUPREQ                 :1;      ///<BIT [5] etmpwrupreq
        uint32_t WICENACK                    :1;      ///<BIT [6] wicenack
        uint32_t WAKEUP                      :1;      ///<BIT [7] wakeup
        uint32_t TRCENA                      :1;      ///<BIT [8] trcena
        uint32_t RSVD_9_23                   :15;     ///<BIT [23:9] rsvd_9_23
        uint32_t SYSRESETREQ_STATUS          :1;      ///<BIT [24] sysresetreq_status
        uint32_t RSVD_25_31                  :7;      ///<BIT [31:25] rsvd_25_31
    } b;
} Cp1Sts0_t;

/// @brief 0x21C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PARITYERRINJ_AXIM           :1;      ///<BIT [0] parityerrinj_axim
        uint32_t PARITYERRINJ_AHBS           :1;      ///<BIT [1] parityerrinj_ahbs
        uint32_t RSVD_2_7                    :6;      ///<BIT [7:2] rsvd_2_7
        uint32_t ERRINJ_ITCM                 :8;      ///<BIT [15:8] errinj_itcm
        uint32_t ERRINJ_D0TCM                :7;      ///<BIT [22:16] errinj_d0tcm
        uint32_t RSVD_23                     :1;      ///<BIT [23] rsvd_23
        uint32_t ERRINJ_D1TCM                :7;      ///<BIT [30:24] errinj_d1tcm
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Cp1CtlErrinj_t;

/// @brief 0x220
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ITCM_ERR_EN                 :4;      ///<BIT [3:0] itcm_err_en
        uint32_t RSVD_4_7                    :4;      ///<BIT [7:4] rsvd_4_7
        uint32_t D0TCM_ERR_EN                :4;      ///<BIT [11:8] d0tcm_err_en
        uint32_t RSVD_12_15                  :4;      ///<BIT [15:12] rsvd_12_15
        uint32_t D1TCM_ERR_EN                :4;      ///<BIT [19:16] d1tcm_err_en
        uint32_t RSVD_20_31                  :12;     ///<BIT [31:20] rsvd_20_31
    } b;
} Cp1CtlEcc_t;

/// @brief 0x224
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ITCM_ERR                    :2;      ///<BIT [1:0] itcm_err
        uint32_t RSVD_2_7                    :6;      ///<BIT [7:2] rsvd_2_7
        uint32_t D0TCM_ERR                   :2;      ///<BIT [9:8] d0tcm_err
        uint32_t RSVD_10_15                  :6;      ///<BIT [15:10] rsvd_10_15
        uint32_t D1TCM_ERR                   :2;      ///<BIT [17:16] d1tcm_err
        uint32_t RSVD_18_31                  :14;     ///<BIT [31:18] rsvd_18_31
    } b;
} Cp1StsTcmErr_t;

/// @brief 0x228
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_COUNT                 :16;     ///<BIT [15:0] error_count
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp1CtlItcmCorrectable_t;

/// @brief 0x22C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_COUNT                 :16;     ///<BIT [15:0] error_count
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp1CtlItcmUncorrectable_t;

/// @brief 0x230
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_THRESHOLD             :16;     ///<BIT [15:0] error_threshold
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp1CtlItcmThr_t;

/// @brief 0x234
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_2                    :3;      ///<BIT [2:0] rsvd_0_2
        uint32_t ERROR_TCMADDR               :21;     ///<BIT [23:3] error_tcmaddr
        uint32_t ERROR_TCMMASTER             :4;      ///<BIT [27:24] error_tcmmaster
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Cp1StsItcm_t;

/// @brief 0x238
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_COUNT                 :16;     ///<BIT [15:0] error_count
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp1CtlD0tcmCorrectable_t;

/// @brief 0x23C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_COUNT                 :16;     ///<BIT [15:0] error_count
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp1CtlD0tcmUncorrectable_t;

/// @brief 0x240
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_THRESHOLD             :16;     ///<BIT [15:0] error_threshold
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp1CtlD0tcmThr_t;

/// @brief 0x244
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_2                    :3;      ///<BIT [2:0] rsvd_0_2
        uint32_t ERROR_TCMADDR               :21;     ///<BIT [23:3] error_tcmaddr
        uint32_t ERROR_TCMMASTER             :4;      ///<BIT [27:24] error_tcmmaster
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Cp1StsD0tcm_t;

/// @brief 0x248
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_COUNT                 :16;     ///<BIT [15:0] error_count
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp1CtlD1tcmCorrectable_t;

/// @brief 0x24C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_COUNT                 :16;     ///<BIT [15:0] error_count
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp1CtlD1tcmUncorrectable_t;

/// @brief 0x250
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_THRESHOLD             :16;     ///<BIT [15:0] error_threshold
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} Cp1CtlD1tcmThr_t;

/// @brief 0x254
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_2                    :3;      ///<BIT [2:0] rsvd_0_2
        uint32_t ERROR_TCMADDR               :21;     ///<BIT [23:3] error_tcmaddr
        uint32_t ERROR_TCMMASTER             :4;      ///<BIT [27:24] error_tcmmaster
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Cp1StsD1tcm_t;

/// @brief 0x258
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ICERR                       :22;     ///<BIT [21:0] icerr
        uint32_t RSVD_22_23                  :2;      ///<BIT [23:22] rsvd_22_23
        uint32_t ICDET                       :4;      ///<BIT [27:24] icdet
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Cp1StsIcacheError_t;

/// @brief 0x25C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DCERR                       :22;     ///<BIT [21:0] dcerr
        uint32_t RSVD_22_23                  :2;      ///<BIT [23:22] rsvd_22_23
        uint32_t DCDET                       :4;      ///<BIT [27:24] dcdet
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} Cp1StsDcacheError_t;

typedef struct
{
    CpId_t cpId;                                                            // 0x0 : cp_id / 
    CpParityerrinj_t cpParityerrinj;                                        // 0x4 : cp_parityerrinj / 
    uint8_t rsvd8[248];                                                     // 0x8 : rsvd_8 / rsvd_8
    Cp0Ctl0_t cp0Ctl0;                                                      // 0x100 : cp0_ctl0 / 
    Cp0Ctl1_t cp0Ctl1;                                                      // 0x104 : cp0_ctl1 / 
    Cp0Ctl2_t cp0Ctl2;                                                      // 0x108 : cp0_ctl2 / 
    Cp0Ctl3_t cp0Ctl3;                                                      // 0x10C : cp0_ctl3 / 
    uint32_t cp0Ctl4Ecorevnum310;                                           // 0x110 : cp0_ctl4 / 
    Cp0Ctl5_t cp0Ctl5;                                                      // 0x114 : cp0_ctl5 / 
    Cp0Sts0_t cp0Sts0;                                                      // 0x118 : cp0_sts0 / 
    Cp0CtlErrinj_t cp0CtlErrinj;                                            // 0x11C : cp0_ctl_errinj / 
    Cp0CtlEcc_t cp0CtlEcc;                                                  // 0x120 : cp0_ctl_ecc / 
    Cp0StsTcmErr_t cp0StsTcmErr;                                            // 0x124 : cp0_sts_tcm_err / 
    Cp0CtlItcmCorrectable_t cp0CtlItcmCorrectable;                          // 0x128 : cp0_ctl_itcm_correctable / 
    Cp0CtlItcmUncorrectable_t cp0CtlItcmUncorrectable;                      // 0x12C : cp0_ctl_itcm_uncorrectable / 
    Cp0CtlItcmThr_t cp0CtlItcmThr;                                          // 0x130 : cp0_ctl_itcm_threshold / 
    Cp0StsItcm_t cp0StsItcm;                                                // 0x134 : cp0_sts_itcm / 
    Cp0CtlD0tcmCorrectable_t cp0CtlD0tcmCorrectable;                        // 0x138 : cp0_ctl_d0tcm_correctable / 
    Cp0CtlD0tcmUncorrectable_t cp0CtlD0tcmUncorrectable;                    // 0x13C : cp0_ctl_d0tcm_uncorrectable / 
    Cp0CtlD0tcmThr_t cp0CtlD0tcmThr;                                        // 0x140 : cp0_ctl_d0tcm_threshold / 
    Cp0StsD0tcm_t cp0StsD0tcm;                                              // 0x144 : cp0_sts_d0tcm / 
    Cp0CtlD1tcmCorrectable_t cp0CtlD1tcmCorrectable;                        // 0x148 : cp0_ctl_d1tcm_correctable / 
    Cp0CtlD1tcmUncorrectable_t cp0CtlD1tcmUncorrectable;                    // 0x14C : cp0_ctl_d1tcm_uncorrectable / 
    Cp0CtlD1tcmThr_t cp0CtlD1tcmThr;                                        // 0x150 : cp0_ctl_d1tcm_threshold / 
    Cp0StsD1tcm_t cp0StsD1tcm;                                              // 0x154 : cp0_sts_d1tcm / 
    Cp0StsIcacheError_t cp0StsIcacheError;                                  // 0x158 : cp0_sts_icache_error / 
    Cp0StsDcacheError_t cp0StsDcacheError;                                  // 0x15C : cp0_sts_dcache_error / 
    uint8_t rsvd160[160];                                                   // 0x160 : rsvd_160 / rsvd_160
    Cp1Ctl0_t cp1Ctl0;                                                      // 0x200 : cp1_ctl0 / 
    Cp1Ctl1_t cp1Ctl1;                                                      // 0x204 : cp1_ctl1 / 
    Cp1Ctl2_t cp1Ctl2;                                                      // 0x208 : cp1_ctl2 / 
    Cp1Ctl3_t cp1Ctl3;                                                      // 0x20C : cp1_ctl3 / 
    uint32_t cp1Ctl4Ecorevnum310;                                           // 0x210 : cp1_ctl4 / 
    Cp1Ctl5_t cp1Ctl5;                                                      // 0x214 : cp1_ctl5 / 
    Cp1Sts0_t cp1Sts0;                                                      // 0x218 : cp1_sts0 / 
    Cp1CtlErrinj_t cp1CtlErrinj;                                            // 0x21C : cp1_ctl_errinj / 
    Cp1CtlEcc_t cp1CtlEcc;                                                  // 0x220 : cp1_ctl_ecc / 
    Cp1StsTcmErr_t cp1StsTcmErr;                                            // 0x224 : cp1_sts_tcm_err / 
    Cp1CtlItcmCorrectable_t cp1CtlItcmCorrectable;                          // 0x228 : cp1_ctl_itcm_correctable / 
    Cp1CtlItcmUncorrectable_t cp1CtlItcmUncorrectable;                      // 0x22C : cp1_ctl_itcm_uncorrectable / 
    Cp1CtlItcmThr_t cp1CtlItcmThr;                                          // 0x230 : cp1_ctl_itcm_threshold / 
    Cp1StsItcm_t cp1StsItcm;                                                // 0x234 : cp1_sts_itcm / 
    Cp1CtlD0tcmCorrectable_t cp1CtlD0tcmCorrectable;                        // 0x238 : cp1_ctl_d0tcm_correctable / 
    Cp1CtlD0tcmUncorrectable_t cp1CtlD0tcmUncorrectable;                    // 0x23C : cp1_ctl_d0tcm_uncorrectable / 
    Cp1CtlD0tcmThr_t cp1CtlD0tcmThr;                                        // 0x240 : cp1_ctl_d0tcm_threshold / 
    Cp1StsD0tcm_t cp1StsD0tcm;                                              // 0x244 : cp1_sts_d0tcm / 
    Cp1CtlD1tcmCorrectable_t cp1CtlD1tcmCorrectable;                        // 0x248 : cp1_ctl_d1tcm_correctable / 
    Cp1CtlD1tcmUncorrectable_t cp1CtlD1tcmUncorrectable;                    // 0x24C : cp1_ctl_d1tcm_uncorrectable / 
    Cp1CtlD1tcmThr_t cp1CtlD1tcmThr;                                        // 0x250 : cp1_ctl_d1tcm_threshold / 
    Cp1StsD1tcm_t cp1StsD1tcm;                                              // 0x254 : cp1_sts_d1tcm / 
    Cp1StsIcacheError_t cp1StsIcacheError;                                  // 0x258 : cp1_sts_icache_error / 
    Cp1StsDcacheError_t cp1StsDcacheError;                                  // 0x25C : cp1_sts_dcache_error / 
} DualCpM7_t;

COMPILE_ASSERT(offsetof(DualCpM7_t,cpId)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cpParityerrinj)==0x4,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0Ctl0)==0x100,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0Ctl1)==0x104,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0Ctl2)==0x108,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0Ctl3)==0x10C,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0Ctl4Ecorevnum310)==0x110,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0Ctl5)==0x114,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0Sts0)==0x118,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0CtlErrinj)==0x11C,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0CtlEcc)==0x120,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0StsTcmErr)==0x124,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0CtlItcmCorrectable)==0x128,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0CtlItcmUncorrectable)==0x12C,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0CtlItcmThr)==0x130,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0StsItcm)==0x134,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0CtlD0tcmCorrectable)==0x138,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0CtlD0tcmUncorrectable)==0x13C,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0CtlD0tcmThr)==0x140,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0StsD0tcm)==0x144,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0CtlD1tcmCorrectable)==0x148,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0CtlD1tcmUncorrectable)==0x14C,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0CtlD1tcmThr)==0x150,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0StsD1tcm)==0x154,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0StsIcacheError)==0x158,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp0StsDcacheError)==0x15C,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1Ctl0)==0x200,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1Ctl1)==0x204,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1Ctl2)==0x208,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1Ctl3)==0x20C,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1Ctl4Ecorevnum310)==0x210,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1Ctl5)==0x214,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1Sts0)==0x218,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1CtlErrinj)==0x21C,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1CtlEcc)==0x220,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1StsTcmErr)==0x224,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1CtlItcmCorrectable)==0x228,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1CtlItcmUncorrectable)==0x22C,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1CtlItcmThr)==0x230,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1StsItcm)==0x234,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1CtlD0tcmCorrectable)==0x238,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1CtlD0tcmUncorrectable)==0x23C,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1CtlD0tcmThr)==0x240,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1StsD0tcm)==0x244,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1CtlD1tcmCorrectable)==0x248,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1CtlD1tcmUncorrectable)==0x24C,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1CtlD1tcmThr)==0x250,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1StsD1tcm)==0x254,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1StsIcacheError)==0x258,"check register structure offset");
COMPILE_ASSERT(offsetof(DualCpM7_t,cp1StsDcacheError)==0x25C,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile DualCpM7_t rDualCpM7; ///< 0xB0200000
