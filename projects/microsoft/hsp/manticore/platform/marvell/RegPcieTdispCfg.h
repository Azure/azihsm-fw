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
//! @brief PCIE_TDISP_CFG Registers
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
        uint32_t TDISP_EN                    :1;      ///<BIT [0] TDISP_EN
        uint32_t TDISP_RST                   :1;      ///<BIT [1] TDISP_RST
        uint32_t SEND_ST_INSECURE            :2;      ///<BIT [3:2] SEND_ST_INSECURE
        uint32_t APB_ERR_EN                  :1;      ///<BIT [4] APB_ERR_EN
        uint32_t RSVD_5_31                   :27;     ///<BIT [31:5] rsvd_5_31
    } b;
} TdispCtrl_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TDISP_ERR_INT               :1;      ///<BIT [0] TDISP_ERR_INT
        uint32_t CII_INT                     :1;      ///<BIT [1] CII_INT
        uint32_t CII_OUT_RNG_INT             :1;      ///<BIT [2] CII_OUT_RNG_INT
        uint32_t CII_OVRD_INT                :1;      ///<BIT [3] CII_OVRD_INT
        uint32_t TLP_RJT_US_INT              :1;      ///<BIT [4] TLP_RJT_US_INT
        uint32_t TLP_RJT_DS_INT              :1;      ///<BIT [5] TLP_RJT_DS_INT
        uint32_t ERR_DET                     :1;      ///<BIT [6] ERR_DET
        uint32_t UNDEF_ST_US_INT             :1;      ///<BIT [7] UNDEF_ST_US_INT
        uint32_t UNDEF_ST_DS_INT             :1;      ///<BIT [8] UNDEF_ST_DS_INT
        uint32_t UNDEF_ST_CII_INT            :1;      ///<BIT [9] UNDEF_ST_CII_INT
        uint32_t RSVD_10_31                  :22;     ///<BIT [31:10] rsvd_10_31
    } b;
} TdispIntSt_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TDISP_ERR_INT_EN            :1;      ///<BIT [0] TDISP_ERR_INT_EN
        uint32_t CII_INT_EN                  :1;      ///<BIT [1] CII_INT_EN
        uint32_t CII_OUT_RNG_INT_EN          :1;      ///<BIT [2] CII_OUT_RNG_INT_EN
        uint32_t CII_OVRD_INT_EN             :1;      ///<BIT [3] CII_OVRD_INT_EN
        uint32_t TLP_RJT_US_INT_EN           :1;      ///<BIT [4] TLP_RJT_US_INT_EN
        uint32_t TLP_RJT_DS_INT_EN           :1;      ///<BIT [5] TLP_RJT_DS_INT_EN
        uint32_t ERR_DET_EN                  :1;      ///<BIT [6] ERR_DET_EN
        uint32_t UNDEF_ST_US_INT_EN          :1;      ///<BIT [7] UNDEF_ST_US_INT_EN
        uint32_t UNDEF_ST_DS_INT_EN          :1;      ///<BIT [8] UNDEF_ST_DS_INT_EN
        uint32_t UNDEF_ST_CII_INT_EN         :1;      ///<BIT [9] UNDEF_ST_CII_INT_EN
        uint32_t RSVD_10_31                  :22;     ///<BIT [31:10] rsvd_10_31
    } b;
} TdispIntEn_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TDISP_ERR_ST2               :1;      ///<BIT [0] TDISP_ERR_ST2
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} TdispErrSt2_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TDISP_ERR_INT_EN2           :1;      ///<BIT [0] TDISP_ERR_INT_EN2
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} TdispErrIntEn2_t;

/// @brief 0x80
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t US_STATUS_CLR               :1;      ///<BIT [0] US_STATUS_CLR
        uint32_t US_ALLOW_REQ_CU             :1;      ///<BIT [1] US_ALLOW_REQ_CU
        uint32_t US_ALLOW_MSIX_CU            :1;      ///<BIT [2] US_ALLOW_MSIX_CU
        uint32_t US_ALLOW_MSIX_ER            :1;      ///<BIT [3] US_ALLOW_MSIX_ER
        uint32_t US_ET_EN_REQ_CL             :1;      ///<BIT [4] US_ET_EN_REQ_CL
        uint32_t US_ET_EN_CPL_CL             :1;      ///<BIT [5] US_ET_EN_CPL_CL
        uint32_t RSVD_6_30                   :25;     ///<BIT [30:6] rsvd_6_30
        uint32_t US_RST                      :1;      ///<BIT [31] US_RST
    } b;
} UsCtrl_t;

/// @brief 0x84
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t US_RJT_ST                   :1;      ///<BIT [0] US_RJT_ST
        uint32_t US_RJT_DIR                  :1;      ///<BIT [1] US_RJT_DIR
        uint32_t US_RJT_SZ                   :3;      ///<BIT [4:2] US_RJT_SZ
        uint32_t US_RJT_LN                   :8;      ///<BIT [12:5] US_RJT_LN
        uint32_t US_RJT_ID                   :12;     ///<BIT [24:13] US_RJT_ID
        uint32_t US_FUNC_NUM                 :7;      ///<BIT [31:25] US_FUNC_NUM
    } b;
} UsRjtSt_t;

/// @brief 0x90
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t US_RJT_ST_CNT               :10;     ///<BIT [9:0] US_RJT_ST_CNT
        uint32_t RSVD_10_31                  :22;     ///<BIT [31:10] rsvd_10_31
    } b;
} UsRjtStCnt_t;

/// @brief 0x100
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DS_STATUS_CLR               :1;      ///<BIT [0] DS_STATUS_CLR
        uint32_t DS_ALLOW_REQ_CU_NT0         :1;      ///<BIT [1] DS_ALLOW_REQ_CU_NT0
        uint32_t DS_ALLOW_REQ_CU_NT1         :1;      ///<BIT [2] DS_ALLOW_REQ_CU_NT1
        uint32_t DS_ALLOW_REQ_CU_TEE         :1;      ///<BIT [3] DS_ALLOW_REQ_CU_TEE
        uint32_t DS_ALLOW_REQ_CL_NT0         :1;      ///<BIT [4] DS_ALLOW_REQ_CL_NT0
        uint32_t DS_ALLOW_REQ_CL_NT1         :1;      ///<BIT [5] DS_ALLOW_REQ_CL_NT1
        uint32_t DS_ALLOW_REQ_RN_NT0         :1;      ///<BIT [6] DS_ALLOW_REQ_RN_NT0
        uint32_t DS_ALLOW_REQ_RN_NT1         :1;      ///<BIT [7] DS_ALLOW_REQ_RN_NT1
        uint32_t DS_ALLOW_REQ_ER_NT0         :1;      ///<BIT [8] DS_ALLOW_REQ_ER_NT0
        uint32_t DS_ALLOW_REQ_ER_NT1         :1;      ///<BIT [9] DS_ALLOW_REQ_ER_NT1
        uint32_t DS_MSIX_CHK_EN              :1;      ///<BIT [10] DS_MSIX_CHK_EN
        uint32_t DS_ALLOW_MSIX_L             :1;      ///<BIT [11] DS_ALLOW_MSIX_L
        uint32_t DS_MSIX_TRANS_ERR           :1;      ///<BIT [12] DS_MSIX_TRANS_ERR
        uint32_t DS_ET_EN_REQ_CL_NT0         :1;      ///<BIT [13] DS_ET_EN_REQ_CL_NT0
        uint32_t DS_ET_EN_REQ_CL_NT1         :1;      ///<BIT [14] DS_ET_EN_REQ_CL_NT1
        uint32_t DS_ET_EN_REQ_CL_TEE         :1;      ///<BIT [15] DS_ET_EN_REQ_CL_TEE
        uint32_t DS_ET_EN_REQ_RN_NT0         :1;      ///<BIT [16] DS_ET_EN_REQ_RN_NT0
        uint32_t DS_ET_EN_REQ_RN_NT1         :1;      ///<BIT [17] DS_ET_EN_REQ_RN_NT1
        uint32_t DS_ET_EN_REQ_RN_TEE         :1;      ///<BIT [18] DS_ET_EN_REQ_RN_TEE
        uint32_t DS_ALLOW_CPL_ER             :1;      ///<BIT [19] DS_ALLOW_CPL_ER
        uint32_t RSVD_20_30                  :11;     ///<BIT [30:20] rsvd_20_30
        uint32_t DS_RST                      :1;      ///<BIT [31] DS_RST
    } b;
} DsCtrl_t;

/// @brief 0x104
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DS_RJT_ST                   :1;      ///<BIT [0] DS_RJT_ST
        uint32_t DS_RJT_DIR                  :1;      ///<BIT [1] DS_RJT_DIR
        uint32_t DS_RJT_SZ                   :3;      ///<BIT [4:2] DS_RJT_SZ
        uint32_t DS_RJT_LN                   :8;      ///<BIT [12:5] DS_RJT_LN
        uint32_t DS_RJT_ID                   :3;      ///<BIT [15:13] DS_RJT_ID
        uint32_t DS_FUNC_NUM                 :7;      ///<BIT [22:16] DS_FUNC_NUM
        uint32_t RSVD_23_31                  :9;      ///<BIT [31:23] rsvd_23_31
    } b;
} DsRjtSt_t;

/// @brief 0x10C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DS_RJT_ST_CNT               :10;     ///<BIT [9:0] DS_RJT_ST_CNT
        uint32_t RSVD_10_31                  :22;     ///<BIT [31:10] rsvd_10_31
    } b;
} DsRjtStCnt_t;

/// @brief 0x150
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DS_LUT_DS_RST               :4;      ///<BIT [3:0] DS_LUT_DS_RST
        uint32_t RSVD_4_31                   :28;     ///<BIT [31:4] rsvd_4_31
    } b;
} DsLutDsRst_t;

/// @brief 0x154
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LUT_DS_DBG_SEL              :8;      ///<BIT [7:0] LUT_DS_DBG_SEL
        uint32_t LUT_DS_DBG_IDX              :8;      ///<BIT [15:8] LUT_DS_DBG_IDX
        uint32_t LUT_DS_DBG_CNTS             :8;      ///<BIT [23:16] LUT_DS_DBG_CNTS
        uint32_t RSVD_24_31                  :8;      ///<BIT [31:24] rsvd_24_31
    } b;
} DsLutDsDbg_t;

/// @brief 0x158
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DS_LUT_DS_ID3               :4;      ///<BIT [3:0] DS_LUT_DS_ID3
        uint32_t RSVD_4_7                    :4;      ///<BIT [7:4] rsvd_4_7
        uint32_t DS_LUT_DS_ID2               :4;      ///<BIT [11:8] DS_LUT_DS_ID2
        uint32_t RSVD_12_15                  :4;      ///<BIT [15:12] rsvd_12_15
        uint32_t DS_LUT_DS_ID1               :4;      ///<BIT [19:16] DS_LUT_DS_ID1
        uint32_t RSVD_20_23                  :4;      ///<BIT [23:20] rsvd_20_23
        uint32_t DS_LUT_DS_ID0               :4;      ///<BIT [27:24] DS_LUT_DS_ID0
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} DsLutDsId_t;

/// @brief 0x180
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t EC_CFG_ERR_CII_EN           :1;      ///<BIT [2] EC_CFG_ERR_CII_EN
        uint32_t EC_DET_REQ_ID_EN            :1;      ///<BIT [3] EC_DET_REQ_ID_EN
        uint32_t EC_DET_FLR_EN               :1;      ///<BIT [4] EC_DET_FLR_EN
        uint32_t RSVD_5_8                    :4;      ///<BIT [8:5] rsvd_5_8
        uint32_t EC_RCVD_CPL_RJT_EN          :1;      ///<BIT [9] EC_RCVD_CPL_RJT_EN
        uint32_t RSVD_10                     :1;      ///<BIT [10] rsvd_10
        uint32_t EC_RCVD_WREQ_PSN_EN         :1;      ///<BIT [11] EC_RCVD_WREQ_PSN_EN
        uint32_t EC_RCVD_CPL_PSN_EN          :1;      ///<BIT [12] EC_RCVD_CPL_PSN_EN
        uint32_t EC_RCVD_CPL_UR_EN           :1;      ///<BIT [13] EC_RCVD_CPL_UR_EN
        uint32_t EC_RCVD_CPL_CA_EN           :1;      ///<BIT [14] EC_RCVD_CPL_CA_EN
        uint32_t RSVD_15_18                  :4;      ///<BIT [18:15] rsvd_15_18
        uint32_t EC_LNK_DN_EN                :1;      ///<BIT [19] EC_LNK_DN_EN
        uint32_t RSVD_20_30                  :11;     ///<BIT [30:20] rsvd_20_30
        uint32_t EC_RST                      :1;      ///<BIT [31] EC_RST
    } b;
} EcCtrl_t;

/// @brief 0x184
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EC_REQ_RJT_US               :1;      ///<BIT [0] EC_REQ_RJT_US
        uint32_t EC_REQ_RJT_DS               :1;      ///<BIT [1] EC_REQ_RJT_DS
        uint32_t EC_CFG_ERR_CII              :1;      ///<BIT [2] EC_CFG_ERR_CII
        uint32_t EC_DET_REQ_ID               :1;      ///<BIT [3] EC_DET_REQ_ID
        uint32_t EC_DET_FLR                  :1;      ///<BIT [4] EC_DET_FLR
        uint32_t EC_DET_LNK_IDE_ST           :1;      ///<BIT [5] EC_DET_LNK_IDE_ST
        uint32_t EC_DET_SLT_IDE_ST           :1;      ///<BIT [6] EC_DET_SLT_IDE_ST
        uint32_t RSVD_7_8                    :2;      ///<BIT [8:7] rsvd_7_8
        uint32_t EC_RCVD_CPL_RJT             :1;      ///<BIT [9] EC_RCVD_CPL_RJT
        uint32_t RSVD_10                     :1;      ///<BIT [10] rsvd_10
        uint32_t EC_RCVD_WREQ_PSN            :1;      ///<BIT [11] EC_RCVD_WREQ_PSN
        uint32_t EC_RCVD_CPL_PSN             :1;      ///<BIT [12] EC_RCVD_CPL_PSN
        uint32_t EC_RCVD_CPL_UR              :1;      ///<BIT [13] EC_RCVD_CPL_UR
        uint32_t EC_RCVD_CPL_CA              :1;      ///<BIT [14] EC_RCVD_CPL_CA
        uint32_t RSVD_15_18                  :4;      ///<BIT [18:15] rsvd_15_18
        uint32_t EC_LNK_DN                   :1;      ///<BIT [19] EC_LNK_DN
        uint32_t RSVD_20_31                  :12;     ///<BIT [31:20] rsvd_20_31
    } b;
} EcStatus_t;

/// @brief 0x190
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EC_IDE_LNK_CTRL2            :1;      ///<BIT [0] EC_IDE_LNK_CTRL2
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} EcIdeCtrl2_t;

/// @brief 0x19C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EC_IDE_SLT_CTRL2            :1;      ///<BIT [0] EC_IDE_SLT_CTRL2
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} EcIdeCtrl5_t;

/// @brief 0x1A8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EC_ST_TR_EN2                :1;      ///<BIT [0] EC_ST_TR_EN2
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} EcStCtrl2_t;

/// @brief 0x200
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CII_CTRL_EN                 :1;      ///<BIT [0] CII_CTRL_EN
        uint32_t RSVD_1_3                    :3;      ///<BIT [3:1] rsvd_1_3
        uint32_t CII_FULL_FW_CTRL            :1;      ///<BIT [4] CII_FULL_FW_CTRL
        uint32_t CII_LUT_UPD_EN              :1;      ///<BIT [5] CII_LUT_UPD_EN
        uint32_t RSVD_6_30                   :25;     ///<BIT [30:6] rsvd_6_30
        uint32_t CII_RST                     :1;      ///<BIT [31] CII_RST
    } b;
} CiiCtrl_t;

/// @brief 0x204
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CII_FW_DONE                 :1;      ///<BIT [0] CII_FW_DONE
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} CiiFwDone_t;

/// @brief 0x208
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CII_DBG                     :16;     ///<BIT [15:0] CII_DBG
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} CiiStatus_t;

/// @brief 0x20C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LBC_CII_HDR_ADDR            :12;     ///<BIT [11:0] LBC_CII_HDR_ADDR
        uint32_t LBC_CII_HDR_VFUNC_NUM       :6;      ///<BIT [17:12] LBC_CII_HDR_VFUNC_NUM
        uint32_t LBC_CII_HDR_VFUNC_ACTIVE    :1;      ///<BIT [18] LBC_CII_HDR_VFUNC_ACTIVE
        uint32_t LBC_CII_HDR_T_BIT           :1;      ///<BIT [19] LBC_CII_HDR_T_BIT
        uint32_t LBC_CII_HDR_FIRST_BE        :4;      ///<BIT [23:20] LBC_CII_HDR_FIRST_BE
        uint32_t LBC_CII_HDR_TYPE            :5;      ///<BIT [28:24] LBC_CII_HDR_TYPE
        uint32_t CII_LBC_HALT                :1;      ///<BIT [29] CII_LBC_HALT
        uint32_t LBC_CII_DV                  :1;      ///<BIT [30] LBC_CII_DV
        uint32_t LBC_CII_HV                  :1;      ///<BIT [31] LBC_CII_HV
    } b;
} LbcCiiHdr0_t;

/// @brief 0x210
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LBC_CII_HDR_REQ_ID          :16;     ///<BIT [15:0] LBC_CII_HDR_REQ_ID
        uint32_t LBC_CII_HDR_TAG             :10;     ///<BIT [25:16] LBC_CII_HDR_TAG
        uint32_t LBC_CII_HDR_POISONED        :1;      ///<BIT [26] LBC_CII_HDR_POISONED
        uint32_t LBC_CII_HDR_FUNC_NUM        :1;      ///<BIT [27] LBC_CII_HDR_FUNC_NUM
        uint32_t RSVD_28_31                  :4;      ///<BIT [31:28] rsvd_28_31
    } b;
} LbcCiiHdr1_t;

/// @brief 0x214
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LBC_CII_HDR_DEV_NUM         :5;      ///<BIT [4:0] LBC_CII_HDR_DEV_NUM
        uint32_t LBC_CII_HDR_BUS_NUM         :8;      ///<BIT [12:5] LBC_CII_HDR_BUS_NUM
        uint32_t RSVD_13_31                  :19;     ///<BIT [31:13] rsvd_13_31
    } b;
} LbcCiiHdr2_t;

/// @brief 0x21C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CII_LBC_OVERRIDE_EN         :1;      ///<BIT [0] CII_LBC_OVERRIDE_EN
        uint32_t CII_LBC_CPL_STATUS          :3;      ///<BIT [3:1] CII_LBC_CPL_STATUS
        uint32_t CII_LBC_OVERRIDE_T_BIT      :1;      ///<BIT [4] CII_LBC_OVERRIDE_T_BIT
        uint32_t RSVD_5_31                   :27;     ///<BIT [31:5] rsvd_5_31
    } b;
} CiiLbcOvrdCtrl_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSIX_L                      :1;      ///<BIT [0] MSIX_L
        uint32_t NON_T_M2                    :1;      ///<BIT [1] NON_T_M2
        uint32_t NON_T_M1                    :1;      ///<BIT [2] NON_T_M1
        uint32_t NON_T_M0                    :1;      ///<BIT [3] NON_T_M0
        uint32_t TDISP_ST                    :4;      ///<BIT [7:4] TDISP_ST
        uint32_t ID                          :8;      ///<BIT [15:8] ID
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} LutT_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LUT_BF_ADDR                 :13;     ///<BIT [12:0] LUT_BF_ADDR
        uint32_t RSVD_13_31                  :19;     ///<BIT [31:13] rsvd_13_31
    } b;
} PcieTdispCfgLutBfAddr_t;

/// @brief 0x774
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp31_t;

/// @brief 0x768
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp30_t;

/// @brief 0x75C
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp29_t;

/// @brief 0x750
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp28_t;

/// @brief 0x744
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp27_t;

/// @brief 0x738
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp26_t;

/// @brief 0x72C
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp25_t;

/// @brief 0x720
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp24_t;

/// @brief 0x714
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp23_t;

/// @brief 0x708
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp22_t;

/// @brief 0x6FC
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp21_t;

/// @brief 0x6F0
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp20_t;

/// @brief 0x6E4
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp19_t;

/// @brief 0x6D8
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp18_t;

/// @brief 0x6CC
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp17_t;

/// @brief 0x6C0
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp16_t;

/// @brief 0x6B4
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp15_t;

/// @brief 0x6A8
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp14_t;

/// @brief 0x69C
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp13_t;

/// @brief 0x690
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp12_t;

/// @brief 0x684
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp11_t;

/// @brief 0x678
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp10_t;

/// @brief 0x66C
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp9_t;

/// @brief 0x660
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp8_t;

/// @brief 0x654
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp7_t;

/// @brief 0x648
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp6_t;

/// @brief 0x63C
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp5_t;

/// @brief 0x630
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp4_t;

/// @brief 0x624
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp3_t;

/// @brief 0x618
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp2_t;

/// @brief 0x60C
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp1_t;

/// @brief 0x600
typedef struct
{
    PcieTdispCfgLutBfAddr_t lutBfAddr;    //LUT_BF_ADDR
    uint32_t lutBfMask;                   //LUT_BF_MASK
    uint32_t lutBfValue;                  //LUT_BF_VALUE
} LutBfGrp0_t;

/// @brief 0x300
typedef struct
{
    LutT_t lutT;                          //LUT_T
} LutTGrp_t;

typedef struct
{
    TdispCtrl_t tdispCtrl;                                                  // 0x0 : TDISP_CTRL / 
    TdispIntSt_t tdispIntSt;                                                // 0x4 : TDISP_INT_ST / 
    TdispIntEn_t tdispIntEn;                                                // 0x8 : TDISP_INT_EN / 
    uint8_t rsvdC[4];                                                       // 0xC : rsvd_c / rsvd_c
    uint32_t tdispErrSt0;                                                   // 0x10 : TDISP_ERR_ST0 / 
    uint32_t tdispErrSt1;                                                   // 0x14 : TDISP_ERR_ST1 / 
    TdispErrSt2_t tdispErrSt2;                                              // 0x18 : TDISP_ERR_ST2 / 
    uint32_t tdispErrIntEn0;                                                // 0x1C : TDISP_ERR_INT_EN0 / 
    uint32_t tdispErrIntEn1;                                                // 0x20 : TDISP_ERR_INT_EN1 / 
    TdispErrIntEn2_t tdispErrIntEn2;                                        // 0x24 : TDISP_ERR_INT_EN2 / 
    uint8_t rsvd28[88];                                                     // 0x28 : rsvd_28 / rsvd_28
    UsCtrl_t usCtrl;                                                        // 0x80 : US_CTRL / 
    UsRjtSt_t usRjtSt;                                                      // 0x84 : US_RJT_ST / 
    uint32_t usRjtStAddrL;                                                  // 0x88 : US_RJT_ST_ADDR_L / 
    uint32_t usRjtStAddrH;                                                  // 0x8C : US_RJT_ST_ADDR_H / 
    UsRjtStCnt_t usRjtStCnt;                                                // 0x90 : US_RJT_ST_CNT / 
    uint8_t rsvd94[108];                                                    // 0x94 : rsvd_94 / rsvd_94
    DsCtrl_t dsCtrl;                                                        // 0x100 : DS_CTRL / 
    DsRjtSt_t dsRjtSt;                                                      // 0x104 : DS_RJT_ST / 
    uint32_t dsRjtStAddr;                                                   // 0x108 : DS_RJT_ST_ADDR / 
    DsRjtStCnt_t dsRjtStCnt;                                                // 0x10C : DS_RJT_ST_CNT / 
    uint8_t rsvd110[64];                                                    // 0x110 : rsvd_110 / rsvd_110
    DsLutDsRst_t dsLutDsRst;                                                // 0x150 : DS_LUT_DS_RST / 
    DsLutDsDbg_t dsLutDsDbg;                                                // 0x154 : DS_LUT_DS_DBG / 
    DsLutDsId_t dsLutDsId;                                                  // 0x158 : DS_LUT_DS_ID / 
    uint8_t rsvd15c[36];                                                    // 0x15C : rsvd_15c / rsvd_15c
    EcCtrl_t ecCtrl;                                                        // 0x180 : EC_CTRL / 
    EcStatus_t ecStatus;                                                    // 0x184 : EC_STATUS / 
    uint32_t ecIdeCtrl0EcIdeLnkCtrl0;                                       // 0x188 : EC_IDE_CTRL0 / 
    uint32_t ecIdeCtrl1EcIdeLnkCtrl1;                                       // 0x18C : EC_IDE_CTRL1 / 
    EcIdeCtrl2_t ecIdeCtrl2;                                                // 0x190 : EC_IDE_CTRL2 / 
    uint32_t ecIdeCtrl3EcIdeSltCtrl0;                                       // 0x194 : EC_IDE_CTRL3 / 
    uint32_t ecIdeCtrl4EcIdeSltCtrl1;                                       // 0x198 : EC_IDE_CTRL4 / 
    EcIdeCtrl5_t ecIdeCtrl5;                                                // 0x19C : EC_IDE_CTRL5 / 
    uint32_t ecStCtrl0EcStTrEn0;                                            // 0x1A0 : EC_ST_CTRL0 / 
    uint32_t ecStCtrl1EcStTrEn1;                                            // 0x1A4 : EC_ST_CTRL1 / 
    EcStCtrl2_t ecStCtrl2;                                                  // 0x1A8 : EC_ST_CTRL2 / 
    uint8_t rsvd1ac[84];                                                    // 0x1AC : rsvd_1ac / rsvd_1ac
    CiiCtrl_t ciiCtrl;                                                      // 0x200 : CII_CTRL / 
    CiiFwDone_t ciiFwDone;                                                  // 0x204 : CII_FW_DONE / 
    CiiStatus_t ciiStatus;                                                  // 0x208 : CII_STATUS / 
    LbcCiiHdr0_t lbcCiiHdr0;                                                // 0x20C : LBC_CII_HDR0 / 
    LbcCiiHdr1_t lbcCiiHdr1;                                                // 0x210 : LBC_CII_HDR1 / 
    LbcCiiHdr2_t lbcCiiHdr2;                                                // 0x214 : LBC_CII_HDR2 / 
    uint32_t lbcCiiData;                                                    // 0x218 : LBC_CII_DATA / 
    CiiLbcOvrdCtrl_t ciiLbcOvrdCtrl;                                        // 0x21C : CII_LBC_OVRD_CTRL / 
    uint32_t ciiLbcOvrdData;                                                // 0x220 : CII_LBC_OVRD_DATA / 
    uint8_t rsvd224[220];                                                   // 0x224 : rsvd_224 / rsvd_224
    LutTGrp_t lutTGrp[65];                                                  // 0x300 : LUT_T_GRP / 
    uint8_t rsvd404[252];                                                   // 0x404 : rsvd_404 / rsvd_404
    uint32_t lutCfgFc0LutCfgFc;                                             // 0x500 : LUT_CFG_FC0 / 
    uint32_t lutCfgFc1LutCfgFc;                                             // 0x504 : LUT_CFG_FC1 / 
    uint32_t lutCfgFc2LutCfgFc;                                             // 0x508 : LUT_CFG_FC2 / 
    uint32_t lutCfgFc3LutCfgFc;                                             // 0x50C : LUT_CFG_FC3 / 
    uint32_t lutCfgFc4LutCfgFc;                                             // 0x510 : LUT_CFG_FC4 / 
    uint32_t lutCfgFc5LutCfgFc;                                             // 0x514 : LUT_CFG_FC5 / 
    uint32_t lutCfgFc6LutCfgFc;                                             // 0x518 : LUT_CFG_FC6 / 
    uint32_t lutCfgFc7LutCfgFc;                                             // 0x51C : LUT_CFG_FC7 / 
    uint32_t lutCfgFc8LutCfgFc;                                             // 0x520 : LUT_CFG_FC8 / 
    uint32_t lutCfgFc9LutCfgFc;                                             // 0x524 : LUT_CFG_FC9 / 
    uint32_t lutCfgFc10LutCfgFc;                                            // 0x528 : LUT_CFG_FC10 / 
    uint32_t lutCfgFc11LutCfgFc;                                            // 0x52C : LUT_CFG_FC11 / 
    uint32_t lutCfgFc12LutCfgFc;                                            // 0x530 : LUT_CFG_FC12 / 
    uint32_t lutCfgFc13LutCfgFc;                                            // 0x534 : LUT_CFG_FC13 / 
    uint32_t lutCfgFc14LutCfgFc;                                            // 0x538 : LUT_CFG_FC14 / 
    uint32_t lutCfgFc15LutCfgFc;                                            // 0x53C : LUT_CFG_FC15 / 
    uint32_t lutCfgFc16LutCfgFc;                                            // 0x540 : LUT_CFG_FC16 / 
    uint32_t lutCfgFc17LutCfgFc;                                            // 0x544 : LUT_CFG_FC17 / 
    uint32_t lutCfgFc18LutCfgFc;                                            // 0x548 : LUT_CFG_FC18 / 
    uint32_t lutCfgFc19LutCfgFc;                                            // 0x54C : LUT_CFG_FC19 / 
    uint32_t lutCfgFc20LutCfgFc;                                            // 0x550 : LUT_CFG_FC20 / 
    uint32_t lutCfgFc21LutCfgFc;                                            // 0x554 : LUT_CFG_FC21 / 
    uint32_t lutCfgFc22LutCfgFc;                                            // 0x558 : LUT_CFG_FC22 / 
    uint8_t rsvd55c[12];                                                    // 0x55C : rsvd_55c / rsvd_55c
    uint32_t lutCfgHc0LutCfgHc;                                             // 0x568 : LUT_CFG_HC0 / 
    uint32_t lutCfgHc1LutCfgHc;                                             // 0x56C : LUT_CFG_HC1 / 
    uint32_t lutCfgHc2LutCfgHc;                                             // 0x570 : LUT_CFG_HC2 / 
    uint32_t lutCfgHc3LutCfgHc;                                             // 0x574 : LUT_CFG_HC3 / 
    uint32_t lutCfgHc4LutCfgHc;                                             // 0x578 : LUT_CFG_HC4 / 
    uint32_t lutCfgHc5LutCfgHc;                                             // 0x57C : LUT_CFG_HC5 / 
    uint32_t lutCfgHc6LutCfgHc;                                             // 0x580 : LUT_CFG_HC6 / 
    uint32_t lutCfgHc7LutCfgHc;                                             // 0x584 : LUT_CFG_HC7 / 
    uint32_t lutCfgHc8LutCfgHc;                                             // 0x588 : LUT_CFG_HC8 / 
    uint32_t lutCfgHc9LutCfgHc;                                             // 0x58C : LUT_CFG_HC9 / 
    uint32_t lutCfgHc10LutCfgHc;                                            // 0x590 : LUT_CFG_HC10 / 
    uint32_t lutCfgHc11LutCfgHc;                                            // 0x594 : LUT_CFG_HC11 / 
    uint32_t lutCfgHc12LutCfgHc;                                            // 0x598 : LUT_CFG_HC12 / 
    uint32_t lutCfgHc13LutCfgHc;                                            // 0x59C : LUT_CFG_HC13 / 
    uint32_t lutCfgHc14LutCfgHc;                                            // 0x5A0 : LUT_CFG_HC14 / 
    uint32_t lutCfgHc15LutCfgHc;                                            // 0x5A4 : LUT_CFG_HC15 / 
    uint32_t lutCfgHc16LutCfgHc;                                            // 0x5A8 : LUT_CFG_HC16 / 
    uint32_t lutCfgHc17LutCfgHc;                                            // 0x5AC : LUT_CFG_HC17 / 
    uint32_t lutCfgHc18LutCfgHc;                                            // 0x5B0 : LUT_CFG_HC18 / 
    uint32_t lutCfgHc19LutCfgHc;                                            // 0x5B4 : LUT_CFG_HC19 / 
    uint32_t lutCfgHc20LutCfgHc;                                            // 0x5B8 : LUT_CFG_HC20 / 
    uint32_t lutCfgHc21LutCfgHc;                                            // 0x5BC : LUT_CFG_HC21 / 
    uint32_t lutCfgHc22LutCfgHc;                                            // 0x5C0 : LUT_CFG_HC22 / 
    uint8_t rsvd5c4[60];                                                    // 0x5C4 : rsvd_5c4 / rsvd_5c4
    LutBfGrp0_t lutBfGrp0;                                                  // 0x600 : LUT_BF_GRP0 / 
    LutBfGrp1_t lutBfGrp1;                                                  // 0x60C : LUT_BF_GRP1 / 
    LutBfGrp2_t lutBfGrp2;                                                  // 0x618 : LUT_BF_GRP2 / 
    LutBfGrp3_t lutBfGrp3;                                                  // 0x624 : LUT_BF_GRP3 / 
    LutBfGrp4_t lutBfGrp4;                                                  // 0x630 : LUT_BF_GRP4 / 
    LutBfGrp5_t lutBfGrp5;                                                  // 0x63C : LUT_BF_GRP5 / 
    LutBfGrp6_t lutBfGrp6;                                                  // 0x648 : LUT_BF_GRP6 / 
    LutBfGrp7_t lutBfGrp7;                                                  // 0x654 : LUT_BF_GRP7 / 
    LutBfGrp8_t lutBfGrp8;                                                  // 0x660 : LUT_BF_GRP8 / 
    LutBfGrp9_t lutBfGrp9;                                                  // 0x66C : LUT_BF_GRP9 / 
    LutBfGrp10_t lutBfGrp10;                                                // 0x678 : LUT_BF_GRP10 / 
    LutBfGrp11_t lutBfGrp11;                                                // 0x684 : LUT_BF_GRP11 / 
    LutBfGrp12_t lutBfGrp12;                                                // 0x690 : LUT_BF_GRP12 / 
    LutBfGrp13_t lutBfGrp13;                                                // 0x69C : LUT_BF_GRP13 / 
    LutBfGrp14_t lutBfGrp14;                                                // 0x6A8 : LUT_BF_GRP14 / 
    LutBfGrp15_t lutBfGrp15;                                                // 0x6B4 : LUT_BF_GRP15 / 
    LutBfGrp16_t lutBfGrp16;                                                // 0x6C0 : LUT_BF_GRP16 / 
    LutBfGrp17_t lutBfGrp17;                                                // 0x6CC : LUT_BF_GRP17 / 
    LutBfGrp18_t lutBfGrp18;                                                // 0x6D8 : LUT_BF_GRP18 / 
    LutBfGrp19_t lutBfGrp19;                                                // 0x6E4 : LUT_BF_GRP19 / 
    LutBfGrp20_t lutBfGrp20;                                                // 0x6F0 : LUT_BF_GRP20 / 
    LutBfGrp21_t lutBfGrp21;                                                // 0x6FC : LUT_BF_GRP21 / 
    LutBfGrp22_t lutBfGrp22;                                                // 0x708 : LUT_BF_GRP22 / 
    LutBfGrp23_t lutBfGrp23;                                                // 0x714 : LUT_BF_GRP23 / 
    LutBfGrp24_t lutBfGrp24;                                                // 0x720 : LUT_BF_GRP24 / 
    LutBfGrp25_t lutBfGrp25;                                                // 0x72C : LUT_BF_GRP25 / 
    LutBfGrp26_t lutBfGrp26;                                                // 0x738 : LUT_BF_GRP26 / 
    LutBfGrp27_t lutBfGrp27;                                                // 0x744 : LUT_BF_GRP27 / 
    LutBfGrp28_t lutBfGrp28;                                                // 0x750 : LUT_BF_GRP28 / 
    LutBfGrp29_t lutBfGrp29;                                                // 0x75C : LUT_BF_GRP29 / 
    LutBfGrp30_t lutBfGrp30;                                                // 0x768 : LUT_BF_GRP30 / 
    LutBfGrp31_t lutBfGrp31;                                                // 0x774 : LUT_BF_GRP31 / 
} PcieTdispCfg_t;

COMPILE_ASSERT(offsetof(PcieTdispCfg_t,tdispCtrl)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,tdispIntSt)==0x4,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,tdispIntEn)==0x8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,tdispErrSt0)==0x10,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,tdispErrSt1)==0x14,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,tdispErrSt2)==0x18,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,tdispErrIntEn0)==0x1C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,tdispErrIntEn1)==0x20,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,tdispErrIntEn2)==0x24,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,usCtrl)==0x80,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,usRjtSt)==0x84,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,usRjtStAddrL)==0x88,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,usRjtStAddrH)==0x8C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,usRjtStCnt)==0x90,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,dsCtrl)==0x100,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,dsRjtSt)==0x104,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,dsRjtStAddr)==0x108,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,dsRjtStCnt)==0x10C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,dsLutDsRst)==0x150,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,dsLutDsDbg)==0x154,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,dsLutDsId)==0x158,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ecCtrl)==0x180,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ecStatus)==0x184,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ecIdeCtrl0EcIdeLnkCtrl0)==0x188,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ecIdeCtrl1EcIdeLnkCtrl1)==0x18C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ecIdeCtrl2)==0x190,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ecIdeCtrl3EcIdeSltCtrl0)==0x194,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ecIdeCtrl4EcIdeSltCtrl1)==0x198,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ecIdeCtrl5)==0x19C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ecStCtrl0EcStTrEn0)==0x1A0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ecStCtrl1EcStTrEn1)==0x1A4,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ecStCtrl2)==0x1A8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ciiCtrl)==0x200,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ciiFwDone)==0x204,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ciiStatus)==0x208,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lbcCiiHdr0)==0x20C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lbcCiiHdr1)==0x210,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lbcCiiHdr2)==0x214,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lbcCiiData)==0x218,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ciiLbcOvrdCtrl)==0x21C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,ciiLbcOvrdData)==0x220,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc0LutCfgFc)==0x500,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc1LutCfgFc)==0x504,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc2LutCfgFc)==0x508,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc3LutCfgFc)==0x50C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc4LutCfgFc)==0x510,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc5LutCfgFc)==0x514,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc6LutCfgFc)==0x518,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc7LutCfgFc)==0x51C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc8LutCfgFc)==0x520,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc9LutCfgFc)==0x524,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc10LutCfgFc)==0x528,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc11LutCfgFc)==0x52C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc12LutCfgFc)==0x530,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc13LutCfgFc)==0x534,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc14LutCfgFc)==0x538,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc15LutCfgFc)==0x53C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc16LutCfgFc)==0x540,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc17LutCfgFc)==0x544,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc18LutCfgFc)==0x548,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc19LutCfgFc)==0x54C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc20LutCfgFc)==0x550,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc21LutCfgFc)==0x554,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgFc22LutCfgFc)==0x558,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc0LutCfgHc)==0x568,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc1LutCfgHc)==0x56C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc2LutCfgHc)==0x570,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc3LutCfgHc)==0x574,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc4LutCfgHc)==0x578,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc5LutCfgHc)==0x57C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc6LutCfgHc)==0x580,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc7LutCfgHc)==0x584,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc8LutCfgHc)==0x588,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc9LutCfgHc)==0x58C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc10LutCfgHc)==0x590,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc11LutCfgHc)==0x594,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc12LutCfgHc)==0x598,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc13LutCfgHc)==0x59C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc14LutCfgHc)==0x5A0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc15LutCfgHc)==0x5A4,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc16LutCfgHc)==0x5A8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc17LutCfgHc)==0x5AC,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc18LutCfgHc)==0x5B0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc19LutCfgHc)==0x5B4,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc20LutCfgHc)==0x5B8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc21LutCfgHc)==0x5BC,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutCfgHc22LutCfgHc)==0x5C0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutTGrp)==0x300,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp0)==0x600,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp1)==0x60C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp2)==0x618,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp3)==0x624,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp4)==0x630,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp5)==0x63C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp6)==0x648,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp7)==0x654,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp8)==0x660,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp9)==0x66C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp10)==0x678,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp11)==0x684,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp12)==0x690,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp13)==0x69C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp14)==0x6A8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp15)==0x6B4,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp16)==0x6C0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp17)==0x6CC,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp18)==0x6D8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp19)==0x6E4,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp20)==0x6F0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp21)==0x6FC,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp22)==0x708,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp23)==0x714,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp24)==0x720,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp25)==0x72C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp26)==0x738,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp27)==0x744,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp28)==0x750,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp29)==0x75C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp30)==0x768,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispCfg_t,lutBfGrp31)==0x774,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile PcieTdispCfg_t rPcieTdispCfg; ///< 0xB01A0000
