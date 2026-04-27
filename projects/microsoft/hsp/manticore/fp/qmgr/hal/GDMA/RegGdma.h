// SPDX-License-Identifier: MIT
// Copyright (c) 2022-2026 Marvell

//=============================================================================
//!
//! @brief GDMA Registers
//!
//=============================================================================

// Generated with Dullahan v2.3.0.b762bdd

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
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
        uint32_t RTC                         : 2;     ///<BIT [1:0] rtc
        uint32_t WTC                         : 2;     ///<BIT [3:2] wtc
        uint32_t RSVD_0                      : 27;    ///<BIT [30:4] rsvd_0
        uint32_t RSVD_31                     : 1;     ///<BIT [31] rsvd_31
    } b;
} GdmaGdmaCfg_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GDMA_EN                     : 1;     ///<BIT [0] gdma_en
        uint32_t GDMA_RST                    : 1;     ///<BIT [1] gdma_rst
        uint32_t AXI_RD_EN_GDMA              : 1;     ///<BIT [2] axi_rd_en // rename due to duplicated constant naming
        uint32_t AXI_WR_EN_GDMA              : 1;     ///<BIT [3] axi_wr_en // rename due to duplicated constant naming
        uint32_t DPE0_RST                    : 1;     ///<BIT [4] dpe0_rst
        uint32_t DPE1_RST                    : 1;     ///<BIT [5] dpe1_rst
        uint32_t DPE0_MAX_OUTSTD_READ_REQS   : 2;     ///<BIT [7:6] dpe0_max_outstd_read_reqs
        uint32_t DPE1_MAX_OUTSTD_READ_REQS   : 2;     ///<BIT [9:8] dpe1_max_outstd_read_reqs
        uint32_t DPE0_HALT_EN                : 1;     ///<BIT [10] dpe0_halt_en
        uint32_t DPE1_HALT_EN                : 1;     ///<BIT [11] dpe1_halt_en
        uint32_t RSVD_1                      : 4;     ///<BIT [15:12] rsvd_1
        uint32_t AXI_RD_IDLE                 : 1;     ///<BIT [16] axi_rd_idle
        uint32_t AXI_WR_IDLE                 : 1;     ///<BIT [17] axi_wr_idle
        uint32_t RSVD_0                      : 10;    ///<BIT [27:18] rsvd_0
        uint32_t DPE0_ERR_HALT               : 1;     ///<BIT [28] dpe0_err_halt
        uint32_t DPE1_ERR_HALT               : 1;     ///<BIT [29] dpe1_err_halt
        uint32_t DPE0_CMD_RNNG               : 1;     ///<BIT [30] dpe0_cmd_rnng
        uint32_t DPE1_CMD_RNNG               : 1;     ///<BIT [31] dpe1_cmd_rnng
    } b;
} GdmaGdmaControlStatus_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DQ0_PAUSE                   : 1;     ///<BIT [0] dq0_pause
        uint32_t DQ1_PAUSE                   : 1;     ///<BIT [1] dq1_pause
        uint32_t DQ2_PAUSE                   : 1;     ///<BIT [2] dq2_pause
        uint32_t DQ3_PAUSE                   : 1;     ///<BIT [3] dq3_pause
        uint32_t DQ4_PAUSE                   : 1;     ///<BIT [4] dq4_pause
        uint32_t DQ5_PAUSE                   : 1;     ///<BIT [5] dq5_pause
        uint32_t DQ6_PAUSE                   : 1;     ///<BIT [6] dq6_pause
        uint32_t DQ7_PAUSE                   : 1;     ///<BIT [7] dq7_pause
        uint32_t DQ0_RST                     : 1;     ///<BIT [8] dq0_rst
        uint32_t DQ1_RST                     : 1;     ///<BIT [9] dq1_rst
        uint32_t DQ2_RST                     : 1;     ///<BIT [10] dq2_rst
        uint32_t DQ3_RST                     : 1;     ///<BIT [11] dq3_rst
        uint32_t DQ4_RST                     : 1;     ///<BIT [12] dq4_rst
        uint32_t DQ5_RST                     : 1;     ///<BIT [13] dq5_rst
        uint32_t DQ6_RST                     : 1;     ///<BIT [14] dq6_rst
        uint32_t DQ7_RST                     : 1;     ///<BIT [15] dq7_rst
        uint32_t DQ0_PAUSING                 : 1;     ///<BIT [16] dq0_pausing
        uint32_t DQ1_PAUSING                 : 1;     ///<BIT [17] dq1_pausing
        uint32_t DQ2_PAUSING                 : 1;     ///<BIT [18] dq2_pausing
        uint32_t DQ3_PAUSING                 : 1;     ///<BIT [19] dq3_pausing
        uint32_t DQ4_PAUSING                 : 1;     ///<BIT [20] dq4_pausing
        uint32_t DQ5_PAUSING                 : 1;     ///<BIT [21] dq5_pausing
        uint32_t DQ6_PAUSING                 : 1;     ///<BIT [22] dq6_pausing
        uint32_t DQ7_PAUSING                 : 1;     ///<BIT [23] dq7_pausing
        uint32_t DQ0_ERR                     : 1;     ///<BIT [24] dq0_err
        uint32_t DQ1_ERR                     : 1;     ///<BIT [25] dq1_err
        uint32_t DQ2_ERR                     : 1;     ///<BIT [26] dq2_err
        uint32_t DQ3_ERR                     : 1;     ///<BIT [27] dq3_err
        uint32_t DQ4_ERR                     : 1;     ///<BIT [28] dq4_err
        uint32_t DQ5_ERR                     : 1;     ///<BIT [29] dq5_err
        uint32_t DQ6_ERR                     : 1;     ///<BIT [30] dq6_err
        uint32_t DQ7_ERR                     : 1;     ///<BIT [31] dq7_err
    } b;
} GdmaGdmaDeliveryQueueControlStatus_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AXI_MSTR_AWUSER_INFO        : 8;     ///<BIT [7:0] axi_mstr_awuser_info
        uint32_t RSVD_1                      : 8;     ///<BIT [15:8] rsvd_1
        uint32_t AXI_MSTR_ARUSER_INFO        : 8;     ///<BIT [23:16] axi_mstr_aruser_info
        uint32_t RSVD_0                      : 8;     ///<BIT [31:24] rsvd_0
    } b;
} GdmaGdmaAxiMasterAxuserInfo_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q0_SZ                 : 13;    ///<BIT [12:0] dlvry_q0_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t DLVRY_Q0_INTRFC_SEL         : 8;     ///<BIT [23:16] dlvry_q0_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t DLVRY_Q0_CNSMR_INDX_SHDW_EN : 1;     ///<BIT [30] dlvry_q0_cnsmr_indx_shdw_en
        uint32_t DLVRY_Q0_EN                 : 1;     ///<BIT [31] dlvry_q0_en
    } b;
} GdmaGdmaDeliveryQueue0Cfg_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q1_SZ                 : 13;    ///<BIT [12:0] dlvry_q1_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t DLVRY_Q1_INTRFC_SEL         : 8;     ///<BIT [23:16] dlvry_q1_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t DLVRY_Q1_CNSMR_INDX_SHDW_EN : 1;     ///<BIT [30] dlvry_q1_cnsmr_indx_shdw_en
        uint32_t DLVRY_Q1_EN                 : 1;     ///<BIT [31] dlvry_q1_en
    } b;
} GdmaGdmaDeliveryQueue1Cfg_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q2_SZ                 : 13;    ///<BIT [12:0] dlvry_q2_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t DLVRY_Q2_INTRFC_SEL         : 8;     ///<BIT [23:16] dlvry_q2_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t DLVRY_Q2_CNSMR_INDX_SHDW_EN : 1;     ///<BIT [30] dlvry_q2_cnsmr_indx_shdw_en
        uint32_t DLVRY_Q2_EN                 : 1;     ///<BIT [31] dlvry_q2_en
    } b;
} GdmaGdmaDeliveryQueue2Cfg_t;

/// @brief 0x2C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q3_SZ                 : 13;    ///<BIT [12:0] dlvry_q3_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t DLVRY_Q3_INTRFC_SEL         : 8;     ///<BIT [23:16] dlvry_q3_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t DLVRY_Q3_CNSMR_INDX_SHDW_EN : 1;     ///<BIT [30] dlvry_q3_cnsmr_indx_shdw_en
        uint32_t DLVRY_Q3_EN                 : 1;     ///<BIT [31] dlvry_q3_en
    } b;
} GdmaGdmaDeliveryQueue3Cfg_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLT_Q0_SZ                 : 13;    ///<BIT [12:0] cmplt_q0_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t CMPLT_Q0_INTRFC_SEL         : 8;     ///<BIT [23:16] cmplt_q0_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t CMPLT_Q0_PRDCR_INDX_SHDW_EN : 1;     ///<BIT [30] cmplt_q0_prdcr_indx_shdw_en
        uint32_t CMPLT_Q0_EN                 : 1;     ///<BIT [31] cmplt_q0_en
    } b;
} GdmaGdmaCompletionQueue0Cfg_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLT_Q1_SZ                 : 13;    ///<BIT [12:0] cmplt_q1_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t CMPLT_Q1_INTRFC_SEL         : 8;     ///<BIT [23:16] cmplt_q1_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t CMPLT_Q1_PRDCR_INDX_SHDW_EN : 1;     ///<BIT [30] cmplt_q1_prdcr_indx_shdw_en
        uint32_t CMPLT_Q1_EN                 : 1;     ///<BIT [31] cmplt_q1_en
    } b;
} GdmaGdmaCompletionQueue1Cfg_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLT_Q2_SZ                 : 13;    ///<BIT [12:0] cmplt_q2_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t CMPLT_Q2_INTRFC_SEL         : 8;     ///<BIT [23:16] cmplt_q2_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t CMPLT_Q2_PRDCR_INDX_SHDW_EN : 1;     ///<BIT [30] cmplt_q2_prdcr_indx_shdw_en
        uint32_t CMPLT_Q2_EN                 : 1;     ///<BIT [31] cmplt_q2_en
    } b;
} GdmaGdmaCompletionQueue2Cfg_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLT_Q3_SZ                 : 13;    ///<BIT [12:0] cmplt_q3_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t CMPLT_Q3_INTRFC_SEL         : 8;     ///<BIT [23:16] cmplt_q3_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t CMPLT_Q3_PRDCR_INDX_SHDW_EN : 1;     ///<BIT [30] cmplt_q3_prdcr_indx_shdw_en
        uint32_t CMPLT_Q3_EN                 : 1;     ///<BIT [31] cmplt_q3_en
    } b;
} GdmaGdmaCompletionQueue3Cfg_t;

/// @brief 0x4C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q0_PRDCR_INDX         : 12;    ///<BIT [11:0] dlvry_q0_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue0ProducerIndex_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q0_CNSMR_INDX         : 12;    ///<BIT [11:0] dlvry_q0_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue0ConsumerIndex_t;

/// @brief 0x6C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q1_PRDCR_INDX         : 12;    ///<BIT [11:0] dlvry_q1_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue1ProducerIndex_t;

/// @brief 0x70
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q1_CNSMR_INDX         : 12;    ///<BIT [11:0] dlvry_q1_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue1ConsumerIndex_t;

/// @brief 0x8C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q2_PRDCR_INDX         : 12;    ///<BIT [11:0] dlvry_q2_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue2ProducerIndex_t;

/// @brief 0x90
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q2_CNSMR_INDX         : 12;    ///<BIT [11:0] dlvry_q2_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue2ConsumerIndex_t;

/// @brief 0xAC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q3_PRDCR_INDX         : 12;    ///<BIT [11:0] dlvry_q3_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue3ProducerIndex_t;

/// @brief 0xB0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q3_CNSMR_INDX         : 12;    ///<BIT [11:0] dlvry_q3_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue3ConsumerIndex_t;

/// @brief 0x10C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q0_PRDCR_INDX        : 12;    ///<BIT [11:0] cmpltn_q0_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue0ProducerIndex_t;

/// @brief 0x110
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q0_CNSMR_INDX        : 12;    ///<BIT [11:0] cmpltn_q0_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue0ConsumerIndex_t;

/// @brief 0x12C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q1_PRDCR_INDX        : 12;    ///<BIT [11:0] cmpltn_q1_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue1ProducerIndex_t;

/// @brief 0x130
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q1_CNSMR_INDX        : 12;    ///<BIT [11:0] cmpltn_q1_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue1ConsumerIndex_t;

/// @brief 0x14C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q2_PRDCR_INDX        : 12;    ///<BIT [11:0] cmpltn_q2_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue2ProducerIndex_t;

/// @brief 0x150
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q2_CNSMR_INDX        : 12;    ///<BIT [11:0] cmpltn_q2_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue2ConsumerIndex_t;

/// @brief 0x16C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q3_PRDCR_INDX        : 12;    ///<BIT [11:0] cmpltn_q3_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue3ProducerIndex_t;

/// @brief 0x170
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q3_CNSMR_INDX        : 12;    ///<BIT [11:0] cmpltn_q3_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue3ConsumerIndex_t;

/// @brief 0x220
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q4_SZ                 : 13;    ///<BIT [12:0] dlvry_q4_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t DLVRY_Q4_INTRFC_SEL         : 8;     ///<BIT [23:16] dlvry_q4_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t DLVRY_Q4_CNSMR_INDX_SHDW_EN : 1;     ///<BIT [30] dlvry_q4_cnsmr_indx_shdw_en
        uint32_t DLVRY_Q4_EN                 : 1;     ///<BIT [31] dlvry_q4_en
    } b;
} GdmaGdmaDeliveryQueue4Cfg_t;

/// @brief 0x224
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q5_SZ                 : 13;    ///<BIT [12:0] dlvry_q5_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t DLVRY_Q5_INTRFC_SEL         : 8;     ///<BIT [23:16] dlvry_q5_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t DLVRY_Q5_CNSMR_INDX_SHDW_EN : 1;     ///<BIT [30] dlvry_q5_cnsmr_indx_shdw_en
        uint32_t DLVRY_Q5_EN                 : 1;     ///<BIT [31] dlvry_q5_en
    } b;
} GdmaGdmaDeliveryQueue5Cfg_t;

/// @brief 0x228
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q6_SZ                 : 13;    ///<BIT [12:0] dlvry_q6_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t DLVRY_Q6_INTRFC_SEL         : 8;     ///<BIT [23:16] dlvry_q6_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t DLVRY_Q6_CNSMR_INDX_SHDW_EN : 1;     ///<BIT [30] dlvry_q6_cnsmr_indx_shdw_en
        uint32_t DLVRY_Q6_EN                 : 1;     ///<BIT [31] dlvry_q6_en
    } b;
} GdmaGdmaDeliveryQueue6Cfg_t;

/// @brief 0x22C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q7_SZ                 : 13;    ///<BIT [12:0] dlvry_q7_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t DLVRY_Q7_INTRFC_SEL         : 8;     ///<BIT [23:16] dlvry_q7_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t DLVRY_Q7_CNSMR_INDX_SHDW_EN : 1;     ///<BIT [30] dlvry_q7_cnsmr_indx_shdw_en
        uint32_t DLVRY_Q7_EN                 : 1;     ///<BIT [31] dlvry_q7_en
    } b;
} GdmaGdmaDeliveryQueue7Cfg_t;

/// @brief 0x230
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLT_Q4_SZ                 : 13;    ///<BIT [12:0] cmplt_q4_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t CMPLT_Q4_INTRFC_SEL         : 8;     ///<BIT [23:16] cmplt_q4_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t CMPLT_Q4_PRDCR_INDX_SHDW_EN : 1;     ///<BIT [30] cmplt_q4_prdcr_indx_shdw_en
        uint32_t CMPLT_Q4_EN                 : 1;     ///<BIT [31] cmplt_q4_en
    } b;
} GdmaGdmaCompletionQueue4Cfg_t;

/// @brief 0x234
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLT_Q5_SZ                 : 13;    ///<BIT [12:0] cmplt_q5_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t CMPLT_Q5_INTRFC_SEL         : 8;     ///<BIT [23:16] cmplt_q5_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t CMPLT_Q5_PRDCR_INDX_SHDW_EN : 1;     ///<BIT [30] cmplt_q5_prdcr_indx_shdw_en
        uint32_t CMPLT_Q5_EN                 : 1;     ///<BIT [31] cmplt_q5_en
    } b;
} GdmaGdmaCompletionQueue5Cfg_t;

/// @brief 0x238
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLT_Q6_SZ                 : 13;    ///<BIT [12:0] cmplt_q6_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t CMPLT_Q6_INTRFC_SEL         : 8;     ///<BIT [23:16] cmplt_q6_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t CMPLT_Q6_PRDCR_INDX_SHDW_EN : 1;     ///<BIT [30] cmplt_q6_prdcr_indx_shdw_en
        uint32_t CMPLT_Q6_EN                 : 1;     ///<BIT [31] cmplt_q6_en
    } b;
} GdmaGdmaCompletionQueue6Cfg_t;

/// @brief 0x23C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLT_Q7_SZ                 : 13;    ///<BIT [12:0] cmplt_q7_sz
        uint32_t RSVD_1                      : 3;     ///<BIT [15:13] rsvd_1
        uint32_t CMPLT_Q7_INTRFC_SEL         : 8;     ///<BIT [23:16] cmplt_q7_intrfc_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [29:24] rsvd_0
        uint32_t CMPLT_Q7_PRDCR_INDX_SHDW_EN : 1;     ///<BIT [30] cmplt_q7_prdcr_indx_shdw_en
        uint32_t CMPLT_Q7_EN                 : 1;     ///<BIT [31] cmplt_q7_en
    } b;
} GdmaGdmaCompletionQueue7Cfg_t;

/// @brief 0x24C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q4_PRDCR_INDX         : 12;    ///<BIT [11:0] dlvry_q4_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue4ProducerIndex_t;

/// @brief 0x250
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q4_CNSMR_INDX         : 12;    ///<BIT [11:0] dlvry_q4_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue4ConsumerIndex_t;

/// @brief 0x26C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q5_PRDCR_INDX         : 12;    ///<BIT [11:0] dlvry_q5_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue5ProducerIndex_t;

/// @brief 0x270
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q5_CNSMR_INDX         : 12;    ///<BIT [11:0] dlvry_q5_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue5ConsumerIndex_t;

/// @brief 0x28C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q6_PRDCR_INDX         : 12;    ///<BIT [11:0] dlvry_q6_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue6ProducerIndex_t;

/// @brief 0x290
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q6_CNSMR_INDX         : 12;    ///<BIT [11:0] dlvry_q6_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue6ConsumerIndex_t;

/// @brief 0x2AC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q7_PRDCR_INDX         : 12;    ///<BIT [11:0] dlvry_q7_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue7ProducerIndex_t;

/// @brief 0x2B0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q7_CNSMR_INDX         : 12;    ///<BIT [11:0] dlvry_q7_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaDeliveryQueue7ConsumerIndex_t;

/// @brief 0x30C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q4_PRDCR_INDX        : 12;    ///<BIT [11:0] cmpltn_q4_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue4ProducerIndex_t;

/// @brief 0x310
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q4_CNSMR_INDX        : 12;    ///<BIT [11:0] cmpltn_q4_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue4ConsumerIndex_t;

/// @brief 0x32C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q5_PRDCR_INDX        : 12;    ///<BIT [11:0] cmpltn_q5_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue5ProducerIndex_t;

/// @brief 0x330
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q5_CNSMR_INDX        : 12;    ///<BIT [11:0] cmpltn_q5_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue5ConsumerIndex_t;

/// @brief 0x34C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q6_PRDCR_INDX        : 12;    ///<BIT [11:0] cmpltn_q6_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue6ProducerIndex_t;

/// @brief 0x350
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q6_CNSMR_INDX        : 12;    ///<BIT [11:0] cmpltn_q6_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue6ConsumerIndex_t;

/// @brief 0x36C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q7_PRDCR_INDX        : 12;    ///<BIT [11:0] cmpltn_q7_prdcr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue7ProducerIndex_t;

/// @brief 0x370
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q7_CNSMR_INDX        : 12;    ///<BIT [11:0] cmpltn_q7_cnsmr_indx
        uint32_t RSVD_0                      : 20;    ///<BIT [31:12] rsvd_0
    } b;
} GdmaGdmaCompletionQueue7ConsumerIndex_t;

/// @brief 0x440
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q0_MAX_COAL_TIME     : 16;    ///<BIT [15:0] cmpltn_q0_max_coal_time
        uint32_t CMPLTN_Q0_MIN_COAL_TIME     : 16;    ///<BIT [31:16] cmpltn_q0_min_coal_time
    } b;
} GdmaGdmaCompletionQueue0IntrCoalescingCfg0_t;

/// @brief 0x444
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q0_COAL_COUNT        : 16;    ///<BIT [15:0] cmpltn_q0_coal_count
        uint32_t CMPLTN_Q0_EN_INT_COAL       : 1;     ///<BIT [16] cmpltn_q0_en_int_coal
        uint32_t RSVD_1                      : 1;     ///<BIT [17] rsvd_1
        uint32_t CMPLTN_Q0_EN_RESTART_WHEN_CI_UPDTD : 1;     ///<BIT [18] cmpltn_q0_en_restart_when_ci_updtd
        uint32_t RSVD_0                      : 13;    ///<BIT [31:19] rsvd_0
    } b;
} GdmaGdmaCompletionQueue0IntrCoalescingCfg1_t;

/// @brief 0x448
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q1_MAX_COAL_TIME     : 16;    ///<BIT [15:0] cmpltn_q1_max_coal_time
        uint32_t CMPLTN_Q1_MIN_COAL_TIME     : 16;    ///<BIT [31:16] cmpltn_q1_min_coal_time
    } b;
} GdmaGdmaCompletionQueue1IntrCoalescingCfg0_t;

/// @brief 0x44C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q1_COAL_COUNT        : 16;    ///<BIT [15:0] cmpltn_q1_coal_count
        uint32_t CMPLTN_Q1_EN_INT_COAL       : 1;     ///<BIT [16] cmpltn_q1_en_int_coal
        uint32_t RSVD_1                      : 1;     ///<BIT [17] rsvd_1
        uint32_t CMPLTN_Q1_EN_RESTART_WHEN_CI_UPDTD : 1;     ///<BIT [18] cmpltn_q1_en_restart_when_ci_updtd
        uint32_t RSVD_0                      : 13;    ///<BIT [31:19] rsvd_0
    } b;
} GdmaGdmaCompletionQueue1IntrCoalescingCfg1_t;

/// @brief 0x450
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q2_MAX_COAL_TIME     : 16;    ///<BIT [15:0] cmpltn_q2_max_coal_time
        uint32_t CMPLTN_Q2_MIN_COAL_TIME     : 16;    ///<BIT [31:16] cmpltn_q2_min_coal_time
    } b;
} GdmaGdmaCompletionQueue2IntrCoalescingCfg0_t;

/// @brief 0x454
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q2_COAL_COUNT        : 16;    ///<BIT [15:0] cmpltn_q2_coal_count
        uint32_t CMPLTN_Q2_EN_INT_COAL       : 1;     ///<BIT [16] cmpltn_q2_en_int_coal
        uint32_t RSVD_1                      : 1;     ///<BIT [17] rsvd_1
        uint32_t CMPLTN_Q2_EN_RESTART_WHEN_CI_UPDTD : 1;     ///<BIT [18] cmpltn_q2_en_restart_when_ci_updtd
        uint32_t RSVD_0                      : 13;    ///<BIT [31:19] rsvd_0
    } b;
} GdmaGdmaCompletionQueue2IntrCoalescingCfg1_t;

/// @brief 0x458
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q3_MAX_COAL_TIME     : 16;    ///<BIT [15:0] cmpltn_q3_max_coal_time
        uint32_t CMPLTN_Q3_MIN_COAL_TIME     : 16;    ///<BIT [31:16] cmpltn_q3_min_coal_time
    } b;
} GdmaGdmaCompletionQueue3IntrCoalescingCfg0_t;

/// @brief 0x45C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q3_COAL_COUNT        : 16;    ///<BIT [15:0] cmpltn_q3_coal_count
        uint32_t CMPLTN_Q3_EN_INT_COAL       : 1;     ///<BIT [16] cmpltn_q3_en_int_coal
        uint32_t RSVD_1                      : 1;     ///<BIT [17] rsvd_1
        uint32_t CMPLTN_Q3_EN_RESTART_WHEN_CI_UPDTD : 1;     ///<BIT [18] cmpltn_q3_en_restart_when_ci_updtd
        uint32_t RSVD_0                      : 13;    ///<BIT [31:19] rsvd_0
    } b;
} GdmaGdmaCompletionQueue3IntrCoalescingCfg1_t;

/// @brief 0x460
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q4_MAX_COAL_TIME     : 16;    ///<BIT [15:0] cmpltn_q4_max_coal_time
        uint32_t CMPLTN_Q4_MIN_COAL_TIME     : 16;    ///<BIT [31:16] cmpltn_q4_min_coal_time
    } b;
} GdmaGdmaCompletionQueue4IntrCoalescingCfg0_t;

/// @brief 0x464
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q4_COAL_COUNT        : 16;    ///<BIT [15:0] cmpltn_q4_coal_count
        uint32_t CMPLTN_Q4_EN_INT_COAL       : 1;     ///<BIT [16] cmpltn_q4_en_int_coal
        uint32_t RSVD_1                      : 1;     ///<BIT [17] rsvd_1
        uint32_t CMPLTN_Q4_EN_RESTART_WHEN_CI_UPDTD : 1;     ///<BIT [18] cmpltn_q4_en_restart_when_ci_updtd
        uint32_t RSVD_0                      : 13;    ///<BIT [31:19] rsvd_0
    } b;
} GdmaGdmaCompletionQueue4IntrCoalescingCfg1_t;

/// @brief 0x468
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q5_MAX_COAL_TIME     : 16;    ///<BIT [15:0] cmpltn_q5_max_coal_time
        uint32_t CMPLTN_Q5_MIN_COAL_TIME     : 16;    ///<BIT [31:16] cmpltn_q5_min_coal_time
    } b;
} GdmaGdmaCompletionQueue5IntrCoalescingCfg0_t;

/// @brief 0x46C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q5_COAL_COUNT        : 16;    ///<BIT [15:0] cmpltn_q5_coal_count
        uint32_t CMPLTN_Q5_EN_INT_COAL       : 1;     ///<BIT [16] cmpltn_q5_en_int_coal
        uint32_t RSVD_1                      : 1;     ///<BIT [17] rsvd_1
        uint32_t CMPLTN_Q5_EN_RESTART_WHEN_CI_UPDTD : 1;     ///<BIT [18] cmpltn_q5_en_restart_when_ci_updtd
        uint32_t RSVD_0                      : 13;    ///<BIT [31:19] rsvd_0
    } b;
} GdmaGdmaCompletionQueue5IntrCoalescingCfg1_t;

/// @brief 0x470
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q6_MAX_COAL_TIME     : 16;    ///<BIT [15:0] cmpltn_q6_max_coal_time
        uint32_t CMPLTN_Q6_MIN_COAL_TIME     : 16;    ///<BIT [31:16] cmpltn_q6_min_coal_time
    } b;
} GdmaGdmaCompletionQueue6IntrCoalescingCfg0_t;

/// @brief 0x474
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q6_COAL_COUNT        : 16;    ///<BIT [15:0] cmpltn_q6_coal_count
        uint32_t CMPLTN_Q6_EN_INT_COAL       : 1;     ///<BIT [16] cmpltn_q6_en_int_coal
        uint32_t RSVD_1                      : 1;     ///<BIT [17] rsvd_1
        uint32_t CMPLTN_Q6_EN_RESTART_WHEN_CI_UPDTD : 1;     ///<BIT [18] cmpltn_q6_en_restart_when_ci_updtd
        uint32_t RSVD_0                      : 13;    ///<BIT [31:19] rsvd_0
    } b;
} GdmaGdmaCompletionQueue6IntrCoalescingCfg1_t;

/// @brief 0x478
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q7_MAX_COAL_TIME     : 16;    ///<BIT [15:0] cmpltn_q7_max_coal_time
        uint32_t CMPLTN_Q7_MIN_COAL_TIME     : 16;    ///<BIT [31:16] cmpltn_q7_min_coal_time
    } b;
} GdmaGdmaCompletionQueue7IntrCoalescingCfg0_t;

/// @brief 0x47C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q7_COAL_COUNT        : 16;    ///<BIT [15:0] cmpltn_q7_coal_count
        uint32_t CMPLTN_Q7_EN_INT_COAL       : 1;     ///<BIT [16] cmpltn_q7_en_int_coal
        uint32_t RSVD_1                      : 1;     ///<BIT [17] rsvd_1
        uint32_t CMPLTN_Q7_EN_RESTART_WHEN_CI_UPDTD : 1;     ///<BIT [18] cmpltn_q7_en_restart_when_ci_updtd
        uint32_t RSVD_0                      : 13;    ///<BIT [31:19] rsvd_0
    } b;
} GdmaGdmaCompletionQueue7IntrCoalescingCfg1_t;

/// @brief 0x480
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GDMA_CQ0_NOT_EMPTY          : 1;     ///<BIT [0] gdma_cq0_not_empty
        uint32_t GDMA_CQ1_NOT_EMPTY          : 1;     ///<BIT [1] gdma_cq1_not_empty
        uint32_t GDMA_CQ2_NOT_EMPTY          : 1;     ///<BIT [2] gdma_cq2_not_empty
        uint32_t GDMA_CQ3_NOT_EMPTY          : 1;     ///<BIT [3] gdma_cq3_not_empty
        uint32_t GDMA_CQ4_NOT_EMPTY          : 1;     ///<BIT [4] gdma_cq4_not_empty
        uint32_t GDMA_CQ5_NOT_EMPTY          : 1;     ///<BIT [5] gdma_cq5_not_empty
        uint32_t GDMA_CQ6_NOT_EMPTY          : 1;     ///<BIT [6] gdma_cq6_not_empty
        uint32_t GDMA_CQ7_NOT_EMPTY          : 1;     ///<BIT [7] gdma_cq7_not_empty
        uint32_t RSVD_8_15                   : 8;     ///<BIT [15:8] rsvd_8_15
        uint32_t GDMA_DQ_ERR                 : 1;     ///<BIT [16] gdma_dq_err
        uint32_t GDMA_CQ_ERR                 : 1;     ///<BIT [17] gdma_cq_err
        uint32_t RSVD_0                      : 4;     ///<BIT [21:18] rsvd_0
        uint32_t DATA_ACCESS_ERR             : 1;     ///<BIT [22] data_access_err
        uint32_t DATA_STRCTR_ERR             : 1;     ///<BIT [23] data_strctr_err
        uint32_t COAL_INT_GENERATED_Q0       : 1;     ///<BIT [24] coal_int_generated_q0
        uint32_t COAL_INT_GENERATED_Q1       : 1;     ///<BIT [25] coal_int_generated_q1
        uint32_t COAL_INT_GENERATED_Q2       : 1;     ///<BIT [26] coal_int_generated_q2
        uint32_t COAL_INT_GENERATED_Q3       : 1;     ///<BIT [27] coal_int_generated_q3
        uint32_t COAL_INT_GENERATED_Q4       : 1;     ///<BIT [28] coal_int_generated_q4
        uint32_t COAL_INT_GENERATED_Q5       : 1;     ///<BIT [29] coal_int_generated_q5
        uint32_t COAL_INT_GENERATED_Q6       : 1;     ///<BIT [30] coal_int_generated_q6
        uint32_t COAL_INT_GENERATED_Q7       : 1;     ///<BIT [31] coal_int_generated_q7
    } b;
} GdmaGdmaIntrCause_t;

/// @brief 0x48C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SGL_CROSS_4K_ERR            : 1;     ///<BIT [0] sgl_cross_4k_err
        uint32_t DATA_UNDRRUN_ERR            : 1;     ///<BIT [1] data_undrrun_err
        uint32_t ZRO_FLD_ERR                 : 1;     ///<BIT [2] zro_fld_err
        uint32_t ILLEGAL_SGL_DSCRPTR_ERR     : 1;     ///<BIT [3] illegal_sgl_dscrptr_err
        uint32_t UNDEFINED_SGL_DSCRPTR_TYPE  : 1;     ///<BIT [4] undefined_sgl_dscrptr_type
        uint32_t SGL_PRP_TBL_LNGTH_ERR       : 1;     ///<BIT [5] sgl_prp_tbl_lngth_err
        uint32_t LNGTH_ALLIGNMENT_ERR        : 1;     ///<BIT [6] lngth_allignment_err
        uint32_t DATA_OVERRUN_ERR            : 1;     ///<BIT [7] data_overrun_err
        uint32_t CMD_SRC_DST_CNT_ERR         : 1;     ///<BIT [8] cmd_src_dst_cnt_err
        uint32_t RSVD_1                      : 7;     ///<BIT [15:9] rsvd_1
        uint32_t PRP_OFST_ERR                : 1;     ///<BIT [16] prp_ofst_err
        uint32_t PRP_ALIGN_ERR               : 1;     ///<BIT [17] prp_align_err
        uint32_t INVLD_OP_CD_ERR             : 1;     ///<BIT [18] invld_op_cd_err
        uint32_t RSVD_0                      : 13;    ///<BIT [31:19] rsvd_0
    } b;
} GdmaGdmaDataStructureErrorCause_t;

/// @brief 0x494
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DATA_STRUCTURE_ERR_TAG      : 16;    ///<BIT [15:0] data_structure_err_tag
        uint32_t DATA_STRUCTURE_ERR_EID      : 2;     ///<BIT [17:16] data_structure_err_eid
        uint32_t RSVD_0                      : 14;    ///<BIT [31:18] rsvd_0
    } b;
} GdmaDataStructureErrorInformation1_t;

/// @brief 0x498
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SGL_PRP_LIST_OFFSET         : 10;    ///<BIT [9:0] sgl_prp_list_offset
        uint32_t RSVD_0                      : 22;    ///<BIT [31:10] rsvd_0
    } b;
} GdmaDataStructureErrorInformation2_t;

/// @brief 0x4A4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DP_PAR_SEL                  : 1;     ///<BIT [0] dp_par_sel
        uint32_t DQ_PARITY_ERR_EN            : 1;     ///<BIT [1] dq_parity_err_en
        uint32_t DQ_AXIS_ERR_R_EN            : 1;     ///<BIT [2] dq_axis_err_r_en
        uint32_t CQ_AXIS_ERR_W_EN            : 1;     ///<BIT [3] cq_axis_err_w_en
        uint32_t RSVD_1                      : 2;     ///<BIT [5:4] rsvd_1
        uint32_t MEM_PAR_ERR_CHECK_EN        : 1;     ///<BIT [6] mem_par_err_check_en
        uint32_t AXI_REG_PAR_ERR_CHECK_EN    : 1;     ///<BIT [7] axi_reg_par_err_check_en
        uint32_t AXI_READ_BUS_PAR_ERR_CHECK_EN : 1;     ///<BIT [8] axi_read_bus_par_err_check_en
        uint32_t AXI_READ_RRESP_CHECK_EN     : 1;     ///<BIT [9] axi_read_rresp_check_en
        uint32_t AXI_WRITE_BRESP_CHECK_EN    : 1;     ///<BIT [10] axi_write_bresp_check_en
        uint32_t RSVD_0                      : 21;    ///<BIT [31:11] rsvd_0
    } b;
} GdmaGdmaDataPathErrorControl_t;

/// @brief 0x4A8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_2                      : 1;     ///<BIT [0] rsvd_2
        uint32_t DQ_PARITY_ERR               : 1;     ///<BIT [1] dq_parity_err
        uint32_t DQ_AXIS_ERR_R               : 1;     ///<BIT [2] dq_axis_err_r
        uint32_t RSVD_1                      : 13;    ///<BIT [15:3] rsvd_1
        uint32_t DQ_ERR_QID                  : 3;     ///<BIT [18:16] dq_err_qid
        uint32_t RSVD_0                      : 13;    ///<BIT [31:19] rsvd_0
    } b;
} GdmaGdmaDeliveryQueueErrorStatus_t;

/// @brief 0x4B4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CQW_SLV_ERR                 : 1;     ///<BIT [0] cqw_slv_err
        uint32_t RSVD_2                      : 7;     ///<BIT [7:1] rsvd_2
        uint32_t CQW_SLV_ERR_SOURCE          : 2;     ///<BIT [9:8] cqw_slv_err_source
        uint32_t RSVD_1                      : 6;     ///<BIT [15:10] rsvd_1
        uint32_t CQW_SLV_ERR_QID             : 3;     ///<BIT [18:16] cqw_slv_err_qid
        uint32_t RSVD_0                      : 5;     ///<BIT [23:19] rsvd_0
        uint32_t CQW_SLV_ERR_BID             : 8;     ///<BIT [31:24] cqw_slv_err_bid
    } b;
} GdmaGdmaCompletionWriteErrorStatus_t;

/// @brief 0x4C0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DPE_PARITY_ERR              : 1;     ///<BIT [0] dpe_parity_err
        uint32_t DPE_AXIS_ERR_W              : 1;     ///<BIT [1] dpe_axis_err_w
        uint32_t DPE_AXIS_ERR_R              : 1;     ///<BIT [2] dpe_axis_err_r
        uint32_t RSVD_1                      : 1;     ///<BIT [3] rsvd_1
        uint32_t SRC_MEM_PERR                : 1;     ///<BIT [4] src_mem_perr
        uint32_t DST_MEM_PERR                : 1;     ///<BIT [5] dst_mem_perr
        uint32_t RSVD_0                      : 26;    ///<BIT [31:6] rsvd_0
    } b;
} GdmaGdmaDataAccessErrorCause_t;

/// @brief 0x4C8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DATA_ACCESS_ERR_TAG         : 16;    ///<BIT [15:0] data_access_err_tag
        uint32_t DATA_ACCESS_ERR_EID         : 2;     ///<BIT [17:16] data_access_err_eid
        uint32_t RSVD_0                      : 14;    ///<BIT [31:18] rsvd_0
    } b;
} GdmaDataAccessErrorInformation1_t;

/// @brief 0x4DC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MEM_PAR_INJ                 : 4;     ///<BIT [3:0] mem_par_inj
        uint32_t FRC_RS_PERR_W               : 1;     ///<BIT [4] frc_rs_perr_w
        uint32_t FRC_RS_PERR_R               : 1;     ///<BIT [5] frc_rs_perr_r
        uint32_t RSVD_1                      : 5;     ///<BIT [10:6] rsvd_1
        uint32_t FRC_DP_AXIS_ERR_W           : 1;     ///<BIT [11] frc_dp_axis_err_w
        uint32_t FRC_DP_AXIS_ERR_R           : 1;     ///<BIT [12] frc_dp_axis_err_r
        uint32_t FRC_DP_PERR_W               : 1;     ///<BIT [13] frc_dp_perr_w
        uint32_t FRC_DP_PERR_CONT            : 1;     ///<BIT [14] frc_dp_perr_cont
        uint32_t FRC_DP_PERR_ONCE            : 1;     ///<BIT [15] frc_dp_perr_once
        uint32_t RSVD_0                      : 16;    ///<BIT [31:16] rsvd_0
    } b;
} GdmaGdmaErrorInjection_t;

/// @brief 0x4F0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t Q_DEBUG_SELECT              : 3;     ///<BIT [2:0] q_debug_select
        uint32_t RSVD_0                      : 29;    ///<BIT [31:3] rsvd_0
    } b;
} GdmaGdmaInternalDebugSelection_t;

typedef struct
{
    GdmaGdmaCfg_t gdmaGdmaCfg;                                              // 0x0 : gdma_reg_gdma_configuration /
    GdmaGdmaControlStatus_t gdmaGdmaControlStatus;                          // 0x4 : gdma_reg_gdma_control_status /
    GdmaGdmaDeliveryQueueControlStatus_t gdmaGdmaDeliveryQueueControlStatus;// 0x8 : gdma_reg_gdma_delivery_queue_control_status /
    GdmaGdmaAxiMasterAxuserInfo_t gdmaGdmaAxiMasterAxuserInfo;              // 0xC : gdma_reg_gdma_axi_master_axuser_info /
    uint8_t rsvd10[16];                                                     // 0x10 : rsvd_10 / rsvd_10
    GdmaGdmaDeliveryQueue0Cfg_t gdmaGdmaDeliveryQueue0Cfg;                  // 0x20 : gdma_reg_gdma_delivery_queue_0_configuration /
    GdmaGdmaDeliveryQueue1Cfg_t gdmaGdmaDeliveryQueue1Cfg;                  // 0x24 : gdma_reg_gdma_delivery_queue_1_configuration /
    GdmaGdmaDeliveryQueue2Cfg_t gdmaGdmaDeliveryQueue2Cfg;                  // 0x28 : gdma_reg_gdma_delivery_queue_2_configuration /
    GdmaGdmaDeliveryQueue3Cfg_t gdmaGdmaDeliveryQueue3Cfg;                  // 0x2C : gdma_reg_gdma_delivery_queue_3_configuration /
    GdmaGdmaCompletionQueue0Cfg_t gdmaGdmaCompletionQueue0Cfg;              // 0x30 : gdma_reg_gdma_completion_queue_0_configuration /
    GdmaGdmaCompletionQueue1Cfg_t gdmaGdmaCompletionQueue1Cfg;              // 0x34 : gdma_reg_gdma_completion_queue_1_configuration /
    GdmaGdmaCompletionQueue2Cfg_t gdmaGdmaCompletionQueue2Cfg;              // 0x38 : gdma_reg_gdma_completion_queue_2_configuration /
    GdmaGdmaCompletionQueue3Cfg_t gdmaGdmaCompletionQueue3Cfg;              // 0x3C : gdma_reg_gdma_completion_queue_3_configuration /
    uint32_t gdmaGdmaDeliveryQueue0BaseAddressLowDlvryQ0BaseAdrsL;          // 0x40 : gdma_reg_gdma_delivery_queue_0_base_address_low /
    uint32_t gdmaGdmaDeliveryQueue0BaseAddressHighDlvryQ0BaseAdrsH;         // 0x44 : gdma_reg_gdma_delivery_queue_0_base_address_high /
    uint32_t gdmaGdmaDeliveryQueue0ConsumerIndexShadowAddressLowDlvryQ0CnsmrIndxShdwAddrL;// 0x48 : gdma_reg_gdma_delivery_queue_0_consumer_index_shadow_address_low /
    GdmaGdmaDeliveryQueue0ProducerIndex_t gdmaGdmaDeliveryQueue0ProducerIndex;// 0x4C : gdma_reg_gdma_delivery_queue_0_producer_index /
    GdmaGdmaDeliveryQueue0ConsumerIndex_t gdmaGdmaDeliveryQueue0ConsumerIndex;// 0x50 : gdma_reg_gdma_delivery_queue_0_consumer_index /
    uint8_t rsvd54[12];                                                     // 0x54 : rsvd_54 / rsvd_54
    uint32_t gdmaGdmaDeliveryQueue1BaseAddressLowDlvryQ1BaseAdrsL;          // 0x60 : gdma_reg_gdma_delivery_queue_1_base_address_low /
    uint32_t gdmaGdmaDeliveryQueue1BaseAddressHighDlvryQ1BaseAdrsH;         // 0x64 : gdma_reg_gdma_delivery_queue_1_base_address_high /
    uint32_t gdmaGdmaDeliveryQueue1ConsumerIndexShadowAddressLowDlvryQ1CnsmrIndxShdwAddrL;// 0x68 : gdma_reg_gdma_delivery_queue_1_consumer_index_shadow_address_low /
    GdmaGdmaDeliveryQueue1ProducerIndex_t gdmaGdmaDeliveryQueue1ProducerIndex;// 0x6C : gdma_reg_gdma_delivery_queue_1_producer_index /
    GdmaGdmaDeliveryQueue1ConsumerIndex_t gdmaGdmaDeliveryQueue1ConsumerIndex;// 0x70 : gdma_reg_gdma_delivery_queue_1_consumer_index /
    uint8_t rsvd74[12];                                                     // 0x74 : rsvd_74 / rsvd_74
    uint32_t gdmaGdmaDeliveryQueue2BaseAddressLowDlvryQ2BaseAdrsL;          // 0x80 : gdma_reg_gdma_delivery_queue_2_base_address_low /
    uint32_t gdmaGdmaDeliveryQueue2BaseAddressHighDlvryQ2BaseAdrsH;         // 0x84 : gdma_reg_gdma_delivery_queue_2_base_address_high /
    uint32_t gdmaGdmaDeliveryQueue2ConsumerIndexShadowAddressLowDlvryQ2CnsmrIndxShdwAddrL;// 0x88 : gdma_reg_gdma_delivery_queue_2_consumer_index_shadow_address_low /
    GdmaGdmaDeliveryQueue2ProducerIndex_t gdmaGdmaDeliveryQueue2ProducerIndex;// 0x8C : gdma_reg_gdma_delivery_queue_2_producer_index /
    GdmaGdmaDeliveryQueue2ConsumerIndex_t gdmaGdmaDeliveryQueue2ConsumerIndex;// 0x90 : gdma_reg_gdma_delivery_queue_2_consumer_index /
    uint8_t rsvd94[12];                                                     // 0x94 : rsvd_94 / rsvd_94
    uint32_t gdmaGdmaDeliveryQueue3BaseAddressLowDlvryQ3BaseAdrsL;          // 0xA0 : gdma_reg_gdma_delivery_queue_3_base_address_low /
    uint32_t gdmaGdmaDeliveryQueue3BaseAddressHighDlvryQ3BaseAdrsH;         // 0xA4 : gdma_reg_gdma_delivery_queue_3_base_address_high /
    uint32_t gdmaGdmaDeliveryQueue3ConsumerIndexShadowAddressLowDlvryQ3CnsmrIndxShdwAddrL;// 0xA8 : gdma_reg_gdma_delivery_queue_3_consumer_index_shadow_address_low /
    GdmaGdmaDeliveryQueue3ProducerIndex_t gdmaGdmaDeliveryQueue3ProducerIndex;// 0xAC : gdma_reg_gdma_delivery_queue_3_producer_index /
    GdmaGdmaDeliveryQueue3ConsumerIndex_t gdmaGdmaDeliveryQueue3ConsumerIndex;// 0xB0 : gdma_reg_gdma_delivery_queue_3_consumer_index /
    uint8_t rsvdB4[76];                                                     // 0xB4 : rsvd_b4 / rsvd_b4
    uint32_t gdmaGdmaCompletionQueue0BaseAddressLowCmpltnQ0BaseAdrsL;       // 0x100 : gdma_reg_gdma_completion_queue_0_base_address_low /
    uint32_t gdmaGdmaCompletionQueue0BaseAddressHighCmpltnQ0BaseAdrsH;      // 0x104 : gdma_reg_gdma_completion_queue_0_base_address_high /
    uint32_t gdmaGdmaCompletionQueue0ProducerIndexShadowAddressLowCmpltnQ0PrdcrIndxShdwAddrL;// 0x108 : gdma_reg_gdma_completion_queue_0_producer_index_shadow_address_low /
    GdmaGdmaCompletionQueue0ProducerIndex_t gdmaGdmaCompletionQueue0ProducerIndex;// 0x10C : gdma_reg_gdma_completion_queue_0_producer_index /
    GdmaGdmaCompletionQueue0ConsumerIndex_t gdmaGdmaCompletionQueue0ConsumerIndex;// 0x110 : gdma_reg_gdma_completion_queue_0_consumer_index /
    uint8_t rsvd114[12];                                                    // 0x114 : rsvd_114 / rsvd_114
    uint32_t gdmaGdmaCompletionQueue1BaseAddressLowCmpltnQ1BaseAdrsL;       // 0x120 : gdma_reg_gdma_completion_queue_1_base_address_low /
    uint32_t gdmaGdmaCompletionQueue1BaseAddressHighCmpltnQ1BaseAdrsH;      // 0x124 : gdma_reg_gdma_completion_queue_1_base_address_high /
    uint32_t gdmaGdmaCompletionQueue1ProducerIndexShadowAddressLowCmpltnQ1PrdcrIndxShdwAddrL;// 0x128 : gdma_reg_gdma_completion_queue_1_producer_index_shadow_address_low /
    GdmaGdmaCompletionQueue1ProducerIndex_t gdmaGdmaCompletionQueue1ProducerIndex;// 0x12C : gdma_reg_gdma_completion_queue_1_producer_index /
    GdmaGdmaCompletionQueue1ConsumerIndex_t gdmaGdmaCompletionQueue1ConsumerIndex;// 0x130 : gdma_reg_gdma_completion_queue_1_consumer_index /
    uint8_t rsvd134[12];                                                    // 0x134 : rsvd_134 / rsvd_134
    uint32_t gdmaGdmaCompletionQueue2BaseAddressLowCmpltnQ2BaseAdrsL;       // 0x140 : gdma_reg_gdma_completion_queue_2_base_address_low /
    uint32_t gdmaGdmaCompletionQueue2BaseAddressHighCmpltnQ2BaseAdrsH;      // 0x144 : gdma_reg_gdma_completion_queue_2_base_address_high /
    uint32_t gdmaGdmaCompletionQueue2ProducerIndexShadowAddressLowCmpltnQ2PrdcrIndxShdwAddrL;// 0x148 : gdma_reg_gdma_completion_queue_2_producer_index_shadow_address_low /
    GdmaGdmaCompletionQueue2ProducerIndex_t gdmaGdmaCompletionQueue2ProducerIndex;// 0x14C : gdma_reg_gdma_completion_queue_2_producer_index /
    GdmaGdmaCompletionQueue2ConsumerIndex_t gdmaGdmaCompletionQueue2ConsumerIndex;// 0x150 : gdma_reg_gdma_completion_queue_2_consumer_index /
    uint8_t rsvd154[12];                                                    // 0x154 : rsvd_154 / rsvd_154
    uint32_t gdmaGdmaCompletionQueue3BaseAddressLowCmpltnQ3BaseAdrsL;       // 0x160 : gdma_reg_gdma_completion_queue_3_base_address_low /
    uint32_t gdmaGdmaCompletionQueue3BaseAddressHighCmpltnQ3BaseAdrsH;      // 0x164 : gdma_reg_gdma_completion_queue_3_base_address_high /
    uint32_t gdmaGdmaCompletionQueue3ProducerIndexShadowAddressLowCmpltnQ3PrdcrIndxShdwAddrL;// 0x168 : gdma_reg_gdma_completion_queue_3_producer_index_shadow_address_low /
    GdmaGdmaCompletionQueue3ProducerIndex_t gdmaGdmaCompletionQueue3ProducerIndex;// 0x16C : gdma_reg_gdma_completion_queue_3_producer_index /
    GdmaGdmaCompletionQueue3ConsumerIndex_t gdmaGdmaCompletionQueue3ConsumerIndex;// 0x170 : gdma_reg_gdma_completion_queue_3_consumer_index /
    uint8_t rsvd174[172];                                                   // 0x174 : rsvd_174 / rsvd_174
    GdmaGdmaDeliveryQueue4Cfg_t gdmaGdmaDeliveryQueue4Cfg;                  // 0x220 : gdma_reg_gdma_delivery_queue_4_configuration /
    GdmaGdmaDeliveryQueue5Cfg_t gdmaGdmaDeliveryQueue5Cfg;                  // 0x224 : gdma_reg_gdma_delivery_queue_5_configuration /
    GdmaGdmaDeliveryQueue6Cfg_t gdmaGdmaDeliveryQueue6Cfg;                  // 0x228 : gdma_reg_gdma_delivery_queue_6_configuration /
    GdmaGdmaDeliveryQueue7Cfg_t gdmaGdmaDeliveryQueue7Cfg;                  // 0x22C : gdma_reg_gdma_delivery_queue_7_configuration /
    GdmaGdmaCompletionQueue4Cfg_t gdmaGdmaCompletionQueue4Cfg;              // 0x230 : gdma_reg_gdma_completion_queue_4_configuration /
    GdmaGdmaCompletionQueue5Cfg_t gdmaGdmaCompletionQueue5Cfg;              // 0x234 : gdma_reg_gdma_completion_queue_5_configuration /
    GdmaGdmaCompletionQueue6Cfg_t gdmaGdmaCompletionQueue6Cfg;              // 0x238 : gdma_reg_gdma_completion_queue_6_configuration /
    GdmaGdmaCompletionQueue7Cfg_t gdmaGdmaCompletionQueue7Cfg;              // 0x23C : gdma_reg_gdma_completion_queue_7_configuration /
    uint32_t gdmaGdmaDeliveryQueue4BaseAddressLowDlvryQ4BaseAdrsL;          // 0x240 : gdma_reg_gdma_delivery_queue_4_base_address_low /
    uint32_t gdmaGdmaDeliveryQueue4BaseAddressHighDlvryQ4BaseAdrsH;         // 0x244 : gdma_reg_gdma_delivery_queue_4_base_address_high /
    uint32_t gdmaGdmaDeliveryQueue4ConsumerIndexShadowAddressLowDlvryQ4CnsmrIndxShdwAddrL;// 0x248 : gdma_reg_gdma_delivery_queue_4_consumer_index_shadow_address_low /
    GdmaGdmaDeliveryQueue4ProducerIndex_t gdmaGdmaDeliveryQueue4ProducerIndex;// 0x24C : gdma_reg_gdma_delivery_queue_4_producer_index /
    GdmaGdmaDeliveryQueue4ConsumerIndex_t gdmaGdmaDeliveryQueue4ConsumerIndex;// 0x250 : gdma_reg_gdma_delivery_queue_4_consumer_index /
    uint8_t rsvd254[12];                                                    // 0x254 : rsvd_254 / rsvd_254
    uint32_t gdmaGdmaDeliveryQueue5BaseAddressLowDlvryQ5BaseAdrsL;          // 0x260 : gdma_reg_gdma_delivery_queue_5_base_address_low /
    uint32_t gdmaGdmaDeliveryQueue5BaseAddressHighDlvryQ5BaseAdrsH;         // 0x264 : gdma_reg_gdma_delivery_queue_5_base_address_high /
    uint32_t gdmaGdmaDeliveryQueue5ConsumerIndexShadowAddressLowDlvryQ5CnsmrIndxShdwAddrL;// 0x268 : gdma_reg_gdma_delivery_queue_5_consumer_index_shadow_address_low /
    GdmaGdmaDeliveryQueue5ProducerIndex_t gdmaGdmaDeliveryQueue5ProducerIndex;// 0x26C : gdma_reg_gdma_delivery_queue_5_producer_index /
    GdmaGdmaDeliveryQueue5ConsumerIndex_t gdmaGdmaDeliveryQueue5ConsumerIndex;// 0x270 : gdma_reg_gdma_delivery_queue_5_consumer_index /
    uint8_t rsvd274[12];                                                    // 0x274 : rsvd_274 / rsvd_274
    uint32_t gdmaGdmaDeliveryQueue6BaseAddressLowDlvryQ6BaseAdrsL;          // 0x280 : gdma_reg_gdma_delivery_queue_6_base_address_low /
    uint32_t gdmaGdmaDeliveryQueue6BaseAddressHighDlvryQ6BaseAdrsH;         // 0x284 : gdma_reg_gdma_delivery_queue_6_base_address_high /
    uint32_t gdmaGdmaDeliveryQueue6ConsumerIndexShadowAddressLowDlvryQ6CnsmrIndxShdwAddrL;// 0x288 : gdma_reg_gdma_delivery_queue_6_consumer_index_shadow_address_low /
    GdmaGdmaDeliveryQueue6ProducerIndex_t gdmaGdmaDeliveryQueue6ProducerIndex;// 0x28C : gdma_reg_gdma_delivery_queue_6_producer_index /
    GdmaGdmaDeliveryQueue6ConsumerIndex_t gdmaGdmaDeliveryQueue6ConsumerIndex;// 0x290 : gdma_reg_gdma_delivery_queue_6_consumer_index /
    uint8_t rsvd294[12];                                                    // 0x294 : rsvd_294 / rsvd_294
    uint32_t gdmaGdmaDeliveryQueue7BaseAddressLowDlvryQ7BaseAdrsL;          // 0x2A0 : gdma_reg_gdma_delivery_queue_7_base_address_low /
    uint32_t gdmaGdmaDeliveryQueue7BaseAddressHighDlvryQ7BaseAdrsH;         // 0x2A4 : gdma_reg_gdma_delivery_queue_7_base_address_high /
    uint32_t gdmaGdmaDeliveryQueue7ConsumerIndexShadowAddressLowDlvryQ7CnsmrIndxShdwAddrL;// 0x2A8 : gdma_reg_gdma_delivery_queue_7_consumer_index_shadow_address_low /
    GdmaGdmaDeliveryQueue7ProducerIndex_t gdmaGdmaDeliveryQueue7ProducerIndex;// 0x2AC : gdma_reg_gdma_delivery_queue_7_producer_index /
    GdmaGdmaDeliveryQueue7ConsumerIndex_t gdmaGdmaDeliveryQueue7ConsumerIndex;// 0x2B0 : gdma_reg_gdma_delivery_queue_7_consumer_index /
    uint8_t rsvd2b4[76];                                                    // 0x2B4 : rsvd_2b4 / rsvd_2b4
    uint32_t gdmaGdmaCompletionQueue4BaseAddressLowCmpltnQ4BaseAdrsL;       // 0x300 : gdma_reg_gdma_completion_queue_4_base_address_low /
    uint32_t gdmaGdmaCompletionQueue4BaseAddressHighCmpltnQ4BaseAdrsH;      // 0x304 : gdma_reg_gdma_completion_queue_4_base_address_high /
    uint32_t gdmaGdmaCompletionQueue4ProducerIndexShadowAddressLowCmpltnQ4PrdcrIndxShdwAddrL;// 0x308 : gdma_reg_gdma_completion_queue_4_producer_index_shadow_address_low /
    GdmaGdmaCompletionQueue4ProducerIndex_t gdmaGdmaCompletionQueue4ProducerIndex;// 0x30C : gdma_reg_gdma_completion_queue_4_producer_index /
    GdmaGdmaCompletionQueue4ConsumerIndex_t gdmaGdmaCompletionQueue4ConsumerIndex;// 0x310 : gdma_reg_gdma_completion_queue_4_consumer_index /
    uint8_t rsvd314[12];                                                    // 0x314 : rsvd_314 / rsvd_314
    uint32_t gdmaGdmaCompletionQueue5BaseAddressLowCmpltnQ5BaseAdrsL;       // 0x320 : gdma_reg_gdma_completion_queue_5_base_address_low /
    uint32_t gdmaGdmaCompletionQueue5BaseAddressHighCmpltnQ5BaseAdrsH;      // 0x324 : gdma_reg_gdma_completion_queue_5_base_address_high /
    uint32_t gdmaGdmaCompletionQueue5ProducerIndexShadowAddressLowCmpltnQ5PrdcrIndxShdwAddrL;// 0x328 : gdma_reg_gdma_completion_queue_5_producer_index_shadow_address_low /
    GdmaGdmaCompletionQueue5ProducerIndex_t gdmaGdmaCompletionQueue5ProducerIndex;// 0x32C : gdma_reg_gdma_completion_queue_5_producer_index /
    GdmaGdmaCompletionQueue5ConsumerIndex_t gdmaGdmaCompletionQueue5ConsumerIndex;// 0x330 : gdma_reg_gdma_completion_queue_5_consumer_index /
    uint8_t rsvd334[12];                                                    // 0x334 : rsvd_334 / rsvd_334
    uint32_t gdmaGdmaCompletionQueue6BaseAddressLowCmpltnQ6BaseAdrsL;       // 0x340 : gdma_reg_gdma_completion_queue_6_base_address_low /
    uint32_t gdmaGdmaCompletionQueue6BaseAddressHighCmpltnQ6BaseAdrsH;      // 0x344 : gdma_reg_gdma_completion_queue_6_base_address_high /
    uint32_t gdmaGdmaCompletionQueue6ProducerIndexShadowAddressLowCmpltnQ6PrdcrIndxShdwAddrL;// 0x348 : gdma_reg_gdma_completion_queue_6_producer_index_shadow_address_low /
    GdmaGdmaCompletionQueue6ProducerIndex_t gdmaGdmaCompletionQueue6ProducerIndex;// 0x34C : gdma_reg_gdma_completion_queue_6_producer_index /
    GdmaGdmaCompletionQueue6ConsumerIndex_t gdmaGdmaCompletionQueue6ConsumerIndex;// 0x350 : gdma_reg_gdma_completion_queue_6_consumer_index /
    uint8_t rsvd354[12];                                                    // 0x354 : rsvd_354 / rsvd_354
    uint32_t gdmaGdmaCompletionQueue7BaseAddressLowCmpltnQ7BaseAdrsL;       // 0x360 : gdma_reg_gdma_completion_queue_7_base_address_low /
    uint32_t gdmaGdmaCompletionQueue7BaseAddressHighCmpltnQ7BaseAdrsH;      // 0x364 : gdma_reg_gdma_completion_queue_7_base_address_high /
    uint32_t gdmaGdmaCompletionQueue7ProducerIndexShadowAddressLowCmpltnQ7PrdcrIndxShdwAddrL;// 0x368 : gdma_reg_gdma_completion_queue_7_producer_index_shadow_address_low /
    GdmaGdmaCompletionQueue7ProducerIndex_t gdmaGdmaCompletionQueue7ProducerIndex;// 0x36C : gdma_reg_gdma_completion_queue_7_producer_index /
    GdmaGdmaCompletionQueue7ConsumerIndex_t gdmaGdmaCompletionQueue7ConsumerIndex;// 0x370 : gdma_reg_gdma_completion_queue_7_consumer_index /
    uint8_t rsvd374[204];                                                   // 0x374 : rsvd_374 / rsvd_374
    GdmaGdmaCompletionQueue0IntrCoalescingCfg0_t gdmaGdmaCompletionQueue0IntrCoalescingCfg0;// 0x440 : gdma_reg_gdma_completion_queue_0_interrupt_coalescing_configuration_0 /
    GdmaGdmaCompletionQueue0IntrCoalescingCfg1_t gdmaGdmaCompletionQueue0IntrCoalescingCfg1;// 0x444 : gdma_reg_gdma_completion_queue_0_interrupt_coalescing_configuration_1 /
    GdmaGdmaCompletionQueue1IntrCoalescingCfg0_t gdmaGdmaCompletionQueue1IntrCoalescingCfg0;// 0x448 : gdma_reg_gdma_completion_queue_1_interrupt_coalescing_configuration_0 /
    GdmaGdmaCompletionQueue1IntrCoalescingCfg1_t gdmaGdmaCompletionQueue1IntrCoalescingCfg1;// 0x44C : gdma_reg_gdma_completion_queue_1_interrupt_coalescing_configuration_1 /
    GdmaGdmaCompletionQueue2IntrCoalescingCfg0_t gdmaGdmaCompletionQueue2IntrCoalescingCfg0;// 0x450 : gdma_reg_gdma_completion_queue_2_interrupt_coalescing_configuration_0 /
    GdmaGdmaCompletionQueue2IntrCoalescingCfg1_t gdmaGdmaCompletionQueue2IntrCoalescingCfg1;// 0x454 : gdma_reg_gdma_completion_queue_2_interrupt_coalescing_configuration_1 /
    GdmaGdmaCompletionQueue3IntrCoalescingCfg0_t gdmaGdmaCompletionQueue3IntrCoalescingCfg0;// 0x458 : gdma_reg_gdma_completion_queue_3_interrupt_coalescing_configuration_0 /
    GdmaGdmaCompletionQueue3IntrCoalescingCfg1_t gdmaGdmaCompletionQueue3IntrCoalescingCfg1;// 0x45C : gdma_reg_gdma_completion_queue_3_interrupt_coalescing_configuration_1 /
    GdmaGdmaCompletionQueue4IntrCoalescingCfg0_t gdmaGdmaCompletionQueue4IntrCoalescingCfg0;// 0x460 : gdma_reg_gdma_completion_queue_4_interrupt_coalescing_configuration_0 /
    GdmaGdmaCompletionQueue4IntrCoalescingCfg1_t gdmaGdmaCompletionQueue4IntrCoalescingCfg1;// 0x464 : gdma_reg_gdma_completion_queue_4_interrupt_coalescing_configuration_1 /
    GdmaGdmaCompletionQueue5IntrCoalescingCfg0_t gdmaGdmaCompletionQueue5IntrCoalescingCfg0;// 0x468 : gdma_reg_gdma_completion_queue_5_interrupt_coalescing_configuration_0 /
    GdmaGdmaCompletionQueue5IntrCoalescingCfg1_t gdmaGdmaCompletionQueue5IntrCoalescingCfg1;// 0x46C : gdma_reg_gdma_completion_queue_5_interrupt_coalescing_configuration_1 /
    GdmaGdmaCompletionQueue6IntrCoalescingCfg0_t gdmaGdmaCompletionQueue6IntrCoalescingCfg0;// 0x470 : gdma_reg_gdma_completion_queue_6_interrupt_coalescing_configuration_0 /
    GdmaGdmaCompletionQueue6IntrCoalescingCfg1_t gdmaGdmaCompletionQueue6IntrCoalescingCfg1;// 0x474 : gdma_reg_gdma_completion_queue_6_interrupt_coalescing_configuration_1 /
    GdmaGdmaCompletionQueue7IntrCoalescingCfg0_t gdmaGdmaCompletionQueue7IntrCoalescingCfg0;// 0x478 : gdma_reg_gdma_completion_queue_7_interrupt_coalescing_configuration_0 /
    GdmaGdmaCompletionQueue7IntrCoalescingCfg1_t gdmaGdmaCompletionQueue7IntrCoalescingCfg1;// 0x47C : gdma_reg_gdma_completion_queue_7_interrupt_coalescing_configuration_1 /
    GdmaGdmaIntrCause_t gdmaGdmaIntrCause;                                  // 0x480 : gdma_reg_gdma_interrupt_cause /
    uint32_t gdmaGdmaIntrEnable0GdmaIrqEn0310;                              // 0x484 : gdma_reg_gdma_interrupt_enable_0 /
    uint32_t gdmaGdmaIntrEnable1GdmaIrqEn1310;                              // 0x488 : gdma_reg_gdma_interrupt_enable_1 /
    GdmaGdmaDataStructureErrorCause_t gdmaGdmaDataStructureErrorCause;      // 0x48C : gdma_reg_gdma_data_structure_error_cause /
    uint32_t gdmaGdmaDataStructureErrorEnableDataStructureErrEn;            // 0x490 : gdma_reg_gdma_data_structure_error_enable /
    GdmaDataStructureErrorInformation1_t gdmaDataStructureErrorInformation1;// 0x494 : gdma_reg_data_structure_error_information_1 /
    GdmaDataStructureErrorInformation2_t gdmaDataStructureErrorInformation2;// 0x498 : gdma_reg_data_structure_error_information_2 /
    uint32_t gdmaErrorSglSegmentPrpListBaseAddressLowErrSglPrpAddrL;        // 0x49C : gdma_reg_error_sgl_segment_prp_list_base_address_low /
    uint32_t gdmaErrorSglSegmentPrpListBaseAddressHighErrSglPrpAddrH;       // 0x4A0 : gdma_reg_error_sgl_segment_prp_list_base_address_high /
    GdmaGdmaDataPathErrorControl_t gdmaGdmaDataPathErrorControl;            // 0x4A4 : gdma_reg_gdma_data_path_error_control /
    GdmaGdmaDeliveryQueueErrorStatus_t gdmaGdmaDeliveryQueueErrorStatus;    // 0x4A8 : gdma_reg_gdma_delivery_queue_error_status /
    uint32_t gdmaGdmaDeliveryQueueErrorAddressLowDqErrAddrL;                // 0x4AC : gdma_reg_gdma_delivery_queue_error_address_low /
    uint32_t gdmaGdmaDeliveryQueueErrorAddressHighDqErrAddrH;               // 0x4B0 : gdma_reg_gdma_delivery_queue_error_address_high /
    GdmaGdmaCompletionWriteErrorStatus_t gdmaGdmaCompletionWriteErrorStatus;// 0x4B4 : gdma_reg_gdma_completion_write_error_status /
    uint32_t gdmaGdmaCompletionQueueErrorAddressLowCqwErrAddrL;             // 0x4B8 : gdma_reg_gdma_completion_queue_error_address_low /
    uint32_t gdmaGdmaCompletionQueueErrorAddressHighCqwErrAddrH;            // 0x4BC : gdma_reg_gdma_completion_queue_error_address_high /
    GdmaGdmaDataAccessErrorCause_t gdmaGdmaDataAccessErrorCause;            // 0x4C0 : gdma_reg_gdma_data_access_error_cause /
    uint32_t gdmaGdmaDataAccessErrorEnableDataAccessErrEn;                  // 0x4C4 : gdma_reg_gdma_data_access_error_enable /
    GdmaDataAccessErrorInformation1_t gdmaDataAccessErrorInformation1;      // 0x4C8 : gdma_reg_data_access_error_information_1 /
    uint8_t rsvd4cc[4];                                                     // 0x4CC : rsvd_4cc / rsvd_4cc
    uint32_t gdmaDpeDataAccessErrorAddressLowDpeErrAddrL;                   // 0x4D0 : gdma_reg_dpe_data_access_error_address_low /
    uint32_t gdmaDpeDataAccessErrorAddressHighDpeErrAddrH;                  // 0x4D4 : gdma_reg_dpe_data_access_error_address_high /
    uint32_t gdmaGdmaParityErrorInjectionMaskPrtyErrorInjectionMask;        // 0x4D8 : gdma_reg_gdma_parity_error_injection_mask /
    GdmaGdmaErrorInjection_t gdmaGdmaErrorInjection;                        // 0x4DC : gdma_reg_gdma_error_injection /
    uint32_t gdmaDpe0MiscellaneousStatus0Dpe0MiscStatus0;                   // 0x4E0 : gdma_reg_dpe0_miscellaneous_status_0 /
    uint32_t gdmaDpe0MiscellaneousStatus1Dpe0MiscStatus1;                   // 0x4E4 : gdma_reg_dpe0_miscellaneous_status_1 /
    uint32_t gdmaDpe1MiscellaneousStatus0Dpe1MiscStatus0;                   // 0x4E8 : gdma_reg_dpe1_miscellaneous_status_0 /
    uint32_t gdmaDpe1MiscellaneousStatus1Dpe1MiscStatus1;                   // 0x4EC : gdma_reg_dpe1_miscellaneous_status_1 /
    GdmaGdmaInternalDebugSelection_t gdmaGdmaInternalDebugSelection;        // 0x4F0 : gdma_reg_gdma_internal_debug_selection /
    uint32_t gdmaQueueMiscellaneousStatusQueueMiscStatus;                   // 0x4F4 : gdma_reg_queue_miscellaneous_status /
    uint32_t gdmaGdmaMiscellaneousStatusGdmaMiscStatus;                     // 0x4F8 : gdma_reg_gdma_miscellaneous_status /
} Gdma_t;

COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCfg) == 0x0, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaControlStatus) == 0x4, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueueControlStatus) == 0x8, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaAxiMasterAxuserInfo) == 0xC, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue0Cfg) == 0x20, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue1Cfg) == 0x24, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue2Cfg) == 0x28, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue3Cfg) == 0x2C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue0Cfg) == 0x30, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue1Cfg) == 0x34, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue2Cfg) == 0x38, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue3Cfg) == 0x3C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue0BaseAddressLowDlvryQ0BaseAdrsL) == 0x40, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue0BaseAddressHighDlvryQ0BaseAdrsH) == 0x44, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue0ConsumerIndexShadowAddressLowDlvryQ0CnsmrIndxShdwAddrL) == 0x48, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue0ProducerIndex) == 0x4C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue0ConsumerIndex) == 0x50, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue1BaseAddressLowDlvryQ1BaseAdrsL) == 0x60, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue1BaseAddressHighDlvryQ1BaseAdrsH) == 0x64, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue1ConsumerIndexShadowAddressLowDlvryQ1CnsmrIndxShdwAddrL) == 0x68, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue1ProducerIndex) == 0x6C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue1ConsumerIndex) == 0x70, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue2BaseAddressLowDlvryQ2BaseAdrsL) == 0x80, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue2BaseAddressHighDlvryQ2BaseAdrsH) == 0x84, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue2ConsumerIndexShadowAddressLowDlvryQ2CnsmrIndxShdwAddrL) == 0x88, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue2ProducerIndex) == 0x8C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue2ConsumerIndex) == 0x90, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue3BaseAddressLowDlvryQ3BaseAdrsL) == 0xA0, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue3BaseAddressHighDlvryQ3BaseAdrsH) == 0xA4, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue3ConsumerIndexShadowAddressLowDlvryQ3CnsmrIndxShdwAddrL) == 0xA8, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue3ProducerIndex) == 0xAC, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue3ConsumerIndex) == 0xB0, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue0BaseAddressLowCmpltnQ0BaseAdrsL) == 0x100, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue0BaseAddressHighCmpltnQ0BaseAdrsH) == 0x104, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue0ProducerIndexShadowAddressLowCmpltnQ0PrdcrIndxShdwAddrL) == 0x108, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue0ProducerIndex) == 0x10C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue0ConsumerIndex) == 0x110, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue1BaseAddressLowCmpltnQ1BaseAdrsL) == 0x120, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue1BaseAddressHighCmpltnQ1BaseAdrsH) == 0x124, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue1ProducerIndexShadowAddressLowCmpltnQ1PrdcrIndxShdwAddrL) == 0x128, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue1ProducerIndex) == 0x12C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue1ConsumerIndex) == 0x130, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue2BaseAddressLowCmpltnQ2BaseAdrsL) == 0x140, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue2BaseAddressHighCmpltnQ2BaseAdrsH) == 0x144, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue2ProducerIndexShadowAddressLowCmpltnQ2PrdcrIndxShdwAddrL) == 0x148, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue2ProducerIndex) == 0x14C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue2ConsumerIndex) == 0x150, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue3BaseAddressLowCmpltnQ3BaseAdrsL) == 0x160, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue3BaseAddressHighCmpltnQ3BaseAdrsH) == 0x164, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue3ProducerIndexShadowAddressLowCmpltnQ3PrdcrIndxShdwAddrL) == 0x168, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue3ProducerIndex) == 0x16C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue3ConsumerIndex) == 0x170, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue4Cfg) == 0x220, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue5Cfg) == 0x224, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue6Cfg) == 0x228, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue7Cfg) == 0x22C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue4Cfg) == 0x230, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue5Cfg) == 0x234, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue6Cfg) == 0x238, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue7Cfg) == 0x23C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue4BaseAddressLowDlvryQ4BaseAdrsL) == 0x240, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue4BaseAddressHighDlvryQ4BaseAdrsH) == 0x244, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue4ConsumerIndexShadowAddressLowDlvryQ4CnsmrIndxShdwAddrL) == 0x248, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue4ProducerIndex) == 0x24C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue4ConsumerIndex) == 0x250, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue5BaseAddressLowDlvryQ5BaseAdrsL) == 0x260, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue5BaseAddressHighDlvryQ5BaseAdrsH) == 0x264, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue5ConsumerIndexShadowAddressLowDlvryQ5CnsmrIndxShdwAddrL) == 0x268, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue5ProducerIndex) == 0x26C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue5ConsumerIndex) == 0x270, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue6BaseAddressLowDlvryQ6BaseAdrsL) == 0x280, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue6BaseAddressHighDlvryQ6BaseAdrsH) == 0x284, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue6ConsumerIndexShadowAddressLowDlvryQ6CnsmrIndxShdwAddrL) == 0x288, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue6ProducerIndex) == 0x28C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue6ConsumerIndex) == 0x290, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue7BaseAddressLowDlvryQ7BaseAdrsL) == 0x2A0, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue7BaseAddressHighDlvryQ7BaseAdrsH) == 0x2A4, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue7ConsumerIndexShadowAddressLowDlvryQ7CnsmrIndxShdwAddrL) == 0x2A8, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue7ProducerIndex) == 0x2AC, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueue7ConsumerIndex) == 0x2B0, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue4BaseAddressLowCmpltnQ4BaseAdrsL) == 0x300, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue4BaseAddressHighCmpltnQ4BaseAdrsH) == 0x304, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue4ProducerIndexShadowAddressLowCmpltnQ4PrdcrIndxShdwAddrL) == 0x308, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue4ProducerIndex) == 0x30C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue4ConsumerIndex) == 0x310, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue5BaseAddressLowCmpltnQ5BaseAdrsL) == 0x320, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue5BaseAddressHighCmpltnQ5BaseAdrsH) == 0x324, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue5ProducerIndexShadowAddressLowCmpltnQ5PrdcrIndxShdwAddrL) == 0x328, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue5ProducerIndex) == 0x32C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue5ConsumerIndex) == 0x330, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue6BaseAddressLowCmpltnQ6BaseAdrsL) == 0x340, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue6BaseAddressHighCmpltnQ6BaseAdrsH) == 0x344, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue6ProducerIndexShadowAddressLowCmpltnQ6PrdcrIndxShdwAddrL) == 0x348, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue6ProducerIndex) == 0x34C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue6ConsumerIndex) == 0x350, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue7BaseAddressLowCmpltnQ7BaseAdrsL) == 0x360, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue7BaseAddressHighCmpltnQ7BaseAdrsH) == 0x364, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue7ProducerIndexShadowAddressLowCmpltnQ7PrdcrIndxShdwAddrL) == 0x368, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue7ProducerIndex) == 0x36C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue7ConsumerIndex) == 0x370, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue0IntrCoalescingCfg0) == 0x440, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue0IntrCoalescingCfg1) == 0x444, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue1IntrCoalescingCfg0) == 0x448, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue1IntrCoalescingCfg1) == 0x44C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue2IntrCoalescingCfg0) == 0x450, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue2IntrCoalescingCfg1) == 0x454, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue3IntrCoalescingCfg0) == 0x458, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue3IntrCoalescingCfg1) == 0x45C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue4IntrCoalescingCfg0) == 0x460, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue4IntrCoalescingCfg1) == 0x464, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue5IntrCoalescingCfg0) == 0x468, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue5IntrCoalescingCfg1) == 0x46C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue6IntrCoalescingCfg0) == 0x470, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue6IntrCoalescingCfg1) == 0x474, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue7IntrCoalescingCfg0) == 0x478, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueue7IntrCoalescingCfg1) == 0x47C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaIntrCause) == 0x480, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaIntrEnable0GdmaIrqEn0310) == 0x484, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaIntrEnable1GdmaIrqEn1310) == 0x488, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDataStructureErrorCause) == 0x48C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDataStructureErrorEnableDataStructureErrEn) == 0x490, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaDataStructureErrorInformation1) == 0x494, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaDataStructureErrorInformation2) == 0x498, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaErrorSglSegmentPrpListBaseAddressLowErrSglPrpAddrL) == 0x49C, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaErrorSglSegmentPrpListBaseAddressHighErrSglPrpAddrH) == 0x4A0, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDataPathErrorControl) == 0x4A4, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueueErrorStatus) == 0x4A8, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueueErrorAddressLowDqErrAddrL) == 0x4AC, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDeliveryQueueErrorAddressHighDqErrAddrH) == 0x4B0, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionWriteErrorStatus) == 0x4B4, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueueErrorAddressLowCqwErrAddrL) == 0x4B8, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaCompletionQueueErrorAddressHighCqwErrAddrH) == 0x4BC, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDataAccessErrorCause) == 0x4C0, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaDataAccessErrorEnableDataAccessErrEn) == 0x4C4, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaDataAccessErrorInformation1) == 0x4C8, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaDpeDataAccessErrorAddressLowDpeErrAddrL) == 0x4D0, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaDpeDataAccessErrorAddressHighDpeErrAddrH) == 0x4D4, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaParityErrorInjectionMaskPrtyErrorInjectionMask) == 0x4D8, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaErrorInjection) == 0x4DC, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaDpe0MiscellaneousStatus0Dpe0MiscStatus0) == 0x4E0, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaDpe0MiscellaneousStatus1Dpe0MiscStatus1) == 0x4E4, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaDpe1MiscellaneousStatus0Dpe1MiscStatus0) == 0x4E8, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaDpe1MiscellaneousStatus1Dpe1MiscStatus1) == 0x4EC, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaInternalDebugSelection) == 0x4F0, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaQueueMiscellaneousStatusQueueMiscStatus) == 0x4F4, "check register structure offset");
COMPILE_ASSERT(offsetof(Gdma_t, gdmaGdmaMiscellaneousStatusGdmaMiscStatus) == 0x4F8, "check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------

//extern volatile Gdma_t rGdma; ///< 0xA0000000
#ifdef LOGGING_NEW_SCHEME
#define GDMA_INSTANCE_NUM 8
#define GDMA_REG_DELIVERY_QUEUE_OFFSET 0x20
#endif
