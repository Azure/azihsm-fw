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
//! @brief CDMA Registers
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
        uint32_t RTC                         :2;      ///<BIT [1:0] RTC
        uint32_t WTC                         :2;      ///<BIT [3:2] WTC
        uint32_t RSVD_4_15                   :12;     ///<BIT [15:4] rsvd_4_15
        uint32_t CDMA_OPERATING_MODE         :1;      ///<BIT [16] CDMA_OPERATING_MODE
        uint32_t CQ_OPERATING_MODE           :1;      ///<BIT [17] CQ_OPERATING_MODE
        uint32_t MAX_DESCR_ELMNT_CHK_EN      :1;      ///<BIT [18] MAX_DESCR_ELMNT_CHK_EN
        uint32_t RSVD_19_31                  :13;     ///<BIT [31:19] rsvd_19_31
    } b;
} Cfg_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CDMA_RST                    :1;      ///<BIT [0] CDMA_RST
        uint32_t CDMA_EN                     :1;      ///<BIT [1] CDMA_EN
        uint32_t AXI_RD_EN                   :1;      ///<BIT [2] AXI_RD_EN
        uint32_t AXI_WR_EN                   :1;      ///<BIT [3] AXI_WR_EN
        uint32_t HOST_CQE_CRYPTO_SECTION_UPDT_EN :1;      ///<BIT [4] HOST_CQE_CRYPTO_SECTION_UPDT_EN
        uint32_t HOST_CQE_NVME_SECTION_UPDT_EN :1;      ///<BIT [5] HOST_CQE_NVME_SECTION_UPDT_EN
        uint32_t RSVD_6_7                    :2;      ///<BIT [7:6] rsvd_6_7
        uint32_t DESCR_FETCH_ENGINE_PAUSE_REQ :1;      ///<BIT [8] DESCR_FETCH_ENGINE_PAUSE_REQ
        uint32_t DATA_FETCH_ENGINE_PAUSE_REQ :1;      ///<BIT [9] DATA_FETCH_ENGINE_PAUSE_REQ
        uint32_t CRYPTO_ENGINE_PAUSE_REQ     :1;      ///<BIT [10] CRYPTO_ENGINE_PAUSE_REQ
        uint32_t DATA_OUTPUT_ENGINE_PAUSE_REQ :1;      ///<BIT [11] DATA_OUTPUT_ENGINE_PAUSE_REQ
        uint32_t RSVD_12_31                  :20;     ///<BIT [31:12] rsvd_12_31
    } b;
} CdmaControl_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BCP_ERR_HALT                :1;      ///<BIT [0] BCP_ERR_HALT
        uint32_t CDMA_ACTIVE                 :1;      ///<BIT [1] CDMA_ACTIVE
        uint32_t AXI_RD_IDLE                 :1;      ///<BIT [2] AXI_RD_IDLE
        uint32_t AXI_WR_IDLE                 :1;      ///<BIT [3] AXI_WR_IDLE
        uint32_t RSVD_4_7                    :4;      ///<BIT [7:4] rsvd_4_7
        uint32_t DESCR_FETCH_ENGINE_PAUSED   :1;      ///<BIT [8] DESCR_FETCH_ENGINE_PAUSED
        uint32_t DATA_FETCH_ENGINE_PAUSED    :1;      ///<BIT [9] DATA_FETCH_ENGINE_PAUSED
        uint32_t CRYPTO_ENGINE_PAUSED        :1;      ///<BIT [10] CRYPTO_ENGINE_PAUSED
        uint32_t DATA_OUTPUT_ENGINE_PAUSED   :1;      ///<BIT [11] DATA_OUTPUT_ENGINE_PAUSED
        uint32_t RSVD_12_31                  :20;     ///<BIT [31:12] rsvd_12_31
    } b;
} CdmaStatus_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q0_NUM_ELEMENTS       :2;      ///<BIT [1:0] DLVRY_Q0_NUM_ELEMENTS
        uint32_t RSVD_2_7                    :6;      ///<BIT [7:2] rsvd_2_7
        uint32_t DLVRY_Q0_ELEMENT_SZ         :2;      ///<BIT [9:8] DLVRY_Q0_ELEMENT_SZ
        uint32_t RSVD_10_15                  :6;      ///<BIT [15:10] rsvd_10_15
        uint32_t DLVRY_Q0_INTRFC_SEL         :8;      ///<BIT [23:16] DLVRY_Q0_INTRFC_SEL
        uint32_t RSVD_24_29                  :6;      ///<BIT [29:24] rsvd_24_29
        uint32_t DLVRY_Q0_CNSMR_INDX_SHDW_EN :1;      ///<BIT [30] DLVRY_Q0_CNSMR_INDX_SHDW_EN
        uint32_t DLVRY_Q0_EN                 :1;      ///<BIT [31] DLVRY_Q0_EN
    } b;
} DeliveryQueue0Cfg_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q0_PRDCR_INDX         :12;     ///<BIT [11:0] DLVRY_Q0_PRDCR_INDX
        uint32_t RSVD_12_31                  :20;     ///<BIT [31:12] rsvd_12_31
    } b;
} DeliveryQueue0ProducerIndex_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLVRY_Q0_CNSMR_INDX         :12;     ///<BIT [11:0] DLVRY_Q0_CNSMR_INDX
        uint32_t RSVD_12_31                  :20;     ///<BIT [31:12] rsvd_12_31
    } b;
} DeliveryQueue0ConsumerIndex_t;

/// @brief 0x80
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLT_Q0_NUM_ELEMENTS       :2;      ///<BIT [1:0] CMPLT_Q0_NUM_ELEMENTS
        uint32_t RSVD_2_15                   :14;     ///<BIT [15:2] rsvd_2_15
        uint32_t CMPLT_Q0_INTRFC_SEL         :8;      ///<BIT [23:16] CMPLT_Q0_INTRFC_SEL
        uint32_t RSVD_24_29                  :6;      ///<BIT [29:24] rsvd_24_29
        uint32_t CMPLT_Q0_PRDCR_INDX_SHDW_EN :1;      ///<BIT [30] CMPLT_Q0_PRDCR_INDX_SHDW_EN
        uint32_t CMPLT_Q0_EN                 :1;      ///<BIT [31] CMPLT_Q0_EN
    } b;
} CompletionQueue0Cfg_t;

/// @brief 0x94
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q0_PRDCR_INDX        :12;     ///<BIT [11:0] CMPLTN_Q0_PRDCR_INDX
        uint32_t RSVD_12_31                  :20;     ///<BIT [31:12] rsvd_12_31
    } b;
} CompletionQueue0ProducerIndex_t;

/// @brief 0x98
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q0_CNSMR_INDX        :12;     ///<BIT [11:0] CMPLTN_Q0_CNSMR_INDX
        uint32_t RSVD_12_31                  :20;     ///<BIT [31:12] rsvd_12_31
    } b;
} CompletionQueue0ConsumerIndex_t;

/// @brief 0xE0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LIST0_ELEMENT_DWORD_SZ      :6;      ///<BIT [5:0] LIST0_ELEMENT_DWORD_SZ
        uint32_t LIST0_DESCR_DWORD_OFFSET    :6;      ///<BIT [11:6] LIST0_DESCR_DWORD_OFFSET
        uint32_t LIST0_CRYPTO_IN_DWORD_OFFSET :6;      ///<BIT [17:12] LIST0_CRYPTO_IN_DWORD_OFFSET
        uint32_t LIST0_CRYPTO_OUT_DWORD_OFFSET :6;      ///<BIT [23:18] LIST0_CRYPTO_OUT_DWORD_OFFSET
        uint32_t RSVD_24_31                  :8;      ///<BIT [31:24] rsvd_24_31
    } b;
} List0Cfg_t;

/// @brief 0xF0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LIST1_ELEMENT_DWORD_SZ      :6;      ///<BIT [5:0] LIST1_ELEMENT_DWORD_SZ
        uint32_t LIST1_DESCR_DWORD_OFFSET    :6;      ///<BIT [11:6] LIST1_DESCR_DWORD_OFFSET
        uint32_t LIST1_CRYPTO_IN_DWORD_OFFSET :6;      ///<BIT [17:12] LIST1_CRYPTO_IN_DWORD_OFFSET
        uint32_t LIST1_CRYPTO_OUT_DWORD_OFFSET :6;      ///<BIT [23:18] LIST1_CRYPTO_OUT_DWORD_OFFSET
        uint32_t RSVD_24_31                  :8;      ///<BIT [31:24] rsvd_24_31
    } b;
} List1Cfg_t;

/// @brief 0x100
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LIST2_ELEMENT_DWORD_SZ      :6;      ///<BIT [5:0] LIST2_ELEMENT_DWORD_SZ
        uint32_t LIST2_DESCR_DWORD_OFFSET    :6;      ///<BIT [11:6] LIST2_DESCR_DWORD_OFFSET
        uint32_t LIST2_CRYPTO_IN_DWORD_OFFSET :6;      ///<BIT [17:12] LIST2_CRYPTO_IN_DWORD_OFFSET
        uint32_t LIST2_CRYPTO_OUT_DWORD_OFFSET :6;      ///<BIT [23:18] LIST2_CRYPTO_OUT_DWORD_OFFSET
        uint32_t RSVD_24_31                  :8;      ///<BIT [31:24] rsvd_24_31
    } b;
} List2Cfg_t;

/// @brief 0x110
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LIST3_ELEMENT_DWORD_SZ      :6;      ///<BIT [5:0] LIST3_ELEMENT_DWORD_SZ
        uint32_t LIST3_DESCR_DWORD_OFFSET    :6;      ///<BIT [11:6] LIST3_DESCR_DWORD_OFFSET
        uint32_t LIST3_CRYPTO_IN_DWORD_OFFSET :6;      ///<BIT [17:12] LIST3_CRYPTO_IN_DWORD_OFFSET
        uint32_t LIST3_CRYPTO_OUT_DWORD_OFFSET :6;      ///<BIT [23:18] LIST3_CRYPTO_OUT_DWORD_OFFSET
        uint32_t RSVD_24_31                  :8;      ///<BIT [31:24] rsvd_24_31
    } b;
} List3Cfg_t;

/// @brief 0x148
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMD_SLOT_ERROR_STATE_STATUS2 :4;      ///<BIT [3:0] CMD_SLOT_ERROR_STATE_STATUS2
        uint32_t RSVD_4_31                   :28;     ///<BIT [31:4] rsvd_4_31
    } b;
} CommandSlotErrorStateStatus2_t;

/// @brief 0x158
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMD_SLOT_RESERVED_STATUS2   :4;      ///<BIT [3:0] CMD_SLOT_RESERVED_STATUS2
        uint32_t RSVD_4_31                   :28;     ///<BIT [31:4] rsvd_4_31
    } b;
} CommandSlotReservedStatus2_t;

/// @brief 0x160
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MAX_COAL_TIME               :16;     ///<BIT [15:0] MAX_COAL_TIME
        uint32_t MIN_COAL_TIME               :16;     ///<BIT [31:16] MIN_COAL_TIME
    } b;
} IntrCoalescingCfg0_t;

/// @brief 0x164
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t COAL_COUNT                  :16;     ///<BIT [15:0] COAL_COUNT
        uint32_t EN_INT_COAL                 :1;      ///<BIT [16] EN_INT_COAL
        uint32_t RSVD_17                     :1;      ///<BIT [17] rsvd_17
        uint32_t EN_RESTART_WHEN_CI_UPDTD    :1;      ///<BIT [18] EN_RESTART_WHEN_CI_UPDTD
        uint32_t RSVD_19_31                  :13;     ///<BIT [31:19] rsvd_19_31
    } b;
} IntrCoalescingCfg1_t;

/// @brief 0x170
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AXI_MSTR_AWUSER_INFO        :8;      ///<BIT [7:0] AXI_MSTR_AWUSER_INFO
        uint32_t RSVD_8_15                   :8;      ///<BIT [15:8] rsvd_8_15
        uint32_t AXI_MSTR_ARUSER_INFO        :8;      ///<BIT [23:16] AXI_MSTR_ARUSER_INFO
        uint32_t RSVD_24_31                  :8;      ///<BIT [31:24] rsvd_24_31
    } b;
} AxiMasterAxuserInfo_t;

/// @brief 0x180
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BCP_CQ_NOT_EMPTY            :1;      ///<BIT [0] BCP_CQ_NOT_EMPTY
        uint32_t BCP_CQ_COAL_THRESHOLD_MET   :1;      ///<BIT [1] BCP_CQ_COAL_THRESHOLD_MET
        uint32_t RSVD_2_7                    :6;      ///<BIT [7:2] rsvd_2_7
        uint32_t CMD_SLOT_NON_FATAL_ERROR    :1;      ///<BIT [8] CMD_SLOT_NON_FATAL_ERROR
        uint32_t CMD_SLOT_FATAL_ERROR        :1;      ///<BIT [9] CMD_SLOT_FATAL_ERROR
        uint32_t RSVD_10_14                  :5;      ///<BIT [14:10] rsvd_10_14
        uint32_t CMDE_FREE_SLOT_EMPTY_TIMEOUT_ERR :1;      ///<BIT [15] CMDE_FREE_SLOT_EMPTY_TIMEOUT_ERR
        uint32_t KV_MEM_CORR_EXCEED_THRESHOLD_ERR :1;      ///<BIT [16] KV_MEM_CORR_EXCEED_THRESHOLD_ERR
        uint32_t KV_MEM_UNCORRECTABLE_ECC_ERR :1;      ///<BIT [17] KV_MEM_UNCORRECTABLE_ECC_ERR
        uint32_t RSVD_18_19                  :2;      ///<BIT [19:18] rsvd_18_19
        uint32_t AXI_WRITE_DECODE_ERROR      :1;      ///<BIT [20] AXI_WRITE_DECODE_ERROR
        uint32_t AXI_WRITE_SLAVE_ERROR       :1;      ///<BIT [21] AXI_WRITE_SLAVE_ERROR
        uint32_t RSVD_22_23                  :2;      ///<BIT [23:22] rsvd_22_23
        uint32_t AXI_READ_BUS_PARITY_ERROR   :1;      ///<BIT [24] AXI_READ_BUS_PARITY_ERROR
        uint32_t AXI_READ_DECODE_ERROR       :1;      ///<BIT [25] AXI_READ_DECODE_ERROR
        uint32_t AXI_READ_SLAVE_ERROR        :1;      ///<BIT [26] AXI_READ_SLAVE_ERROR
        uint32_t RSVD_27_31                  :5;      ///<BIT [31:27] rsvd_27_31
    } b;
} IntrCause_t;

/// @brief 0x190
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t KV_MEM_CORR_ECC_ERR_THRESHOLD_COUNT :16;     ///<BIT [15:0] KV_MEM_CORR_ECC_ERR_THRESHOLD_COUNT
        uint32_t KV_MEM_ECC_ERR_CHECK_EN     :1;      ///<BIT [16] KV_MEM_ECC_ERR_CHECK_EN
        uint32_t RSVD_17_19                  :3;      ///<BIT [19:17] rsvd_17_19
        uint32_t AXI_WRITE_BRESP_CHECK_EN    :1;      ///<BIT [20] AXI_WRITE_BRESP_CHECK_EN
        uint32_t CMDE_FREE_SLOT_EMPTY_TIMEOUT_ERR_CHECK_EN :1;      ///<BIT [21] CMDE_FREE_SLOT_EMPTY_TIMEOUT_ERR_CHECK_EN
        uint32_t RSVD_22_23                  :2;      ///<BIT [23:22] rsvd_22_23
        uint32_t AXI_READ_BUS_PARITY_ERROR_CHECK_EN :1;      ///<BIT [24] AXI_READ_BUS_PARITY_ERROR_CHECK_EN
        uint32_t AXI_READ_RRESP_CHECK_EN     :1;      ///<BIT [25] AXI_READ_RRESP_CHECK_EN
        uint32_t RSVD_26_31                  :6;      ///<BIT [31:26] rsvd_26_31
    } b;
} GlobalErrorCheckEnable_t;

/// @brief 0x1AC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DP_PAR_SEL                  :1;      ///<BIT [0] DP_PAR_SEL
        uint32_t DP_PERR_EN                  :1;      ///<BIT [1] DP_PERR_EN
        uint32_t DP_FERR_EN                  :1;      ///<BIT [2] DP_FERR_EN
        uint32_t DP_PRTY_MASK_SEL            :1;      ///<BIT [3] DP_PRTY_MASK_SEL
        uint32_t DPE_RELEVANT_MAIN           :1;      ///<BIT [4] DPE_RELEVANT_MAIN
        uint32_t DPE_RELEVANT_FPS            :1;      ///<BIT [5] DPE_RELEVANT_FPS
        uint32_t RSVD_6_13                   :8;      ///<BIT [13:6] rsvd_6_13
        uint32_t FRC_DP_PERR_CONT            :1;      ///<BIT [14] FRC_DP_PERR_CONT
        uint32_t FRC_DP_PERR_ONCE            :1;      ///<BIT [15] FRC_DP_PERR_ONCE
        uint32_t DP_PRTY_MASK                :16;     ///<BIT [31:16] DP_PRTY_MASK
    } b;
} DataPathParityControl_t;

/// @brief 0x1B0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      :1;      ///<BIT [0] rsvd_0
        uint32_t DP_PERR                     :1;      ///<BIT [1] DP_PERR
        uint32_t DP_FERR                     :1;      ///<BIT [2] DP_FERR
        uint32_t RSVD_3_23                   :21;     ///<BIT [23:3] rsvd_3_23
        uint32_t ERR_PORT                    :4;      ///<BIT [27:24] ERR_PORT
        uint32_t ERR_TYPE_CPTRD              :4;      ///<BIT [31:28] ERR_TYPE_CPTRD
    } b;
} DataPathParityStatus_t;

/// @brief 0x1C0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_22                   :23;     ///<BIT [22:0] rsvd_0_22
        uint32_t SRC_DESC_ERR_INJECT_SEL     :4;      ///<BIT [26:23] SRC_DESC_ERR_INJECT_SEL
        uint32_t DEST_DESC_ERR_INJECT_SEL    :4;      ///<BIT [30:27] DEST_DESC_ERR_INJECT_SEL
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} DpeMiscellaneousControl0_t;

/// @brief 0x1C4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DESCR_AXI_RD_ERR_FATAL_EN   :1;      ///<BIT [0] DESCR_AXI_RD_ERR_FATAL_EN
        uint32_t DESCR_STRUCTURE_ERR_FATAL_DIS :1;      ///<BIT [1] DESCR_STRUCTURE_ERR_FATAL_DIS
        uint32_t DEST_SGL_CROSS_4K_ERR_FATAL_DIS :1;      ///<BIT [2] DEST_SGL_CROSS_4K_ERR_FATAL_DIS
        uint32_t SGL_ILLEGAL_DSCRPTR_ERR_FATAL_DIS :1;      ///<BIT [3] SGL_ILLEGAL_DSCRPTR_ERR_FATAL_DIS
        uint32_t SGL_UNDEFINED_DSCRPTR_TYPE_FATAL_DIS :1;      ///<BIT [4] SGL_UNDEFINED_DSCRPTR_TYPE_FATAL_DIS
        uint32_t SGL_TBL_LENGTH_ERR_FATAL_DIS :1;      ///<BIT [5] SGL_TBL_LENGTH_ERR_FATAL_DIS
        uint32_t SGL_LENGTH_ALIGNMENT_ERR_FATAL_DIS :1;      ///<BIT [6] SGL_LENGTH_ALIGNMENT_ERR_FATAL_DIS
        uint32_t PRP_OFST_ERR_FATAL_DIS      :1;      ///<BIT [7] PRP_OFST_ERR_FATAL_DIS
        uint32_t PRP_ALIGN_ERR_DIS           :1;      ///<BIT [8] PRP_ALIGN_ERR_DIS
        uint32_t RSVD_9_17                   :9;      ///<BIT [17:9] rsvd_9_17
        uint32_t WTC0_MEM                    :2;      ///<BIT [19:18] WTC0_MEM
        uint32_t RTC0_MEM                    :2;      ///<BIT [21:20] RTC0_MEM
        uint32_t WTC1_MEM                    :2;      ///<BIT [23:22] WTC1_MEM
        uint32_t RTC1_MEM                    :2;      ///<BIT [25:24] RTC1_MEM
        uint32_t CEB_MEM                     :1;      ///<BIT [26] CEB_MEM
        uint32_t SLP_MEM                     :1;      ///<BIT [27] SLP_MEM
        uint32_t DSLP_MEM                    :1;      ///<BIT [28] DSLP_MEM
        uint32_t SD_MEM                      :1;      ///<BIT [29] SD_MEM
        uint32_t SD_KEY_MEM                  :1;      ///<BIT [30] SD_KEY_MEM
        uint32_t SLP_KEY_MEM                 :1;      ///<BIT [31] SLP_KEY_MEM
    } b;
} DpeMiscellaneousControl1_t;

/// @brief 0x1D0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERR_INJECT_SRC_DESC         :1;      ///<BIT [0] ERR_INJECT_SRC_DESC
        uint32_t RSVD_1                      :1;      ///<BIT [1] rsvd_1
        uint32_t ERR_INJECT_DEST_DESC        :1;      ///<BIT [2] ERR_INJECT_DEST_DESC
        uint32_t RSVD_3                      :1;      ///<BIT [3] rsvd_3
        uint32_t ERR_INJECT_DEST_WRITE_BUFFER_OVERRUN :1;      ///<BIT [4] ERR_INJECT_DEST_WRITE_BUFFER_OVERRUN
        uint32_t ERR_INJECT_DEST_WRITE_BUFFER :1;      ///<BIT [5] ERR_INJECT_DEST_WRITE_BUFFER
        uint32_t ERR_INJECT_SOURCE_READ_BUFFER_UNDERRUN :1;      ///<BIT [6] ERR_INJECT_SOURCE_READ_BUFFER_UNDERRUN
        uint32_t ERR_INJECT_SOURCE_READ_BUFFER :1;      ///<BIT [7] ERR_INJECT_SOURCE_READ_BUFFER
        uint32_t ERR_INJECT_CRYPTO_ENGINE    :16;     ///<BIT [23:8] ERR_INJECT_CRYPTO_ENGINE
        uint32_t RSVD_24_28                  :5;      ///<BIT [28:24] rsvd_24_28
        uint32_t ERR_INJECT_CORR_ECC         :1;      ///<BIT [29] ERR_INJECT_CORR_ECC
        uint32_t ERR_INJECT_UNCORR_ECC       :1;      ///<BIT [30] ERR_INJECT_UNCORR_ECC
        uint32_t ERR_INJECT_AXI_SLAVE_RD     :1;      ///<BIT [31] ERR_INJECT_AXI_SLAVE_RD
    } b;
} ErrorInjection_t;

/// @brief 0x1D4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t KEY_VAULT_MEM_UNCORRECTABLE_ERROR_COUNT :16;     ///<BIT [15:0] KEY_VAULT_MEM_UNCORRECTABLE_ERROR_COUNT
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} KeyVaultMemoryUncorrectableErrorCount_t;

/// @brief 0x1D8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t KEY_VAULT_MEM_CORRECTABLE_ERROR_COUNT :16;     ///<BIT [15:0] KEY_VAULT_MEM_CORRECTABLE_ERROR_COUNT
        uint32_t RSVD_16_31                  :16;     ///<BIT [31:16] rsvd_16_31
    } b;
} KeyVaultMemoryCorrectableErrorCount_t;

/// @brief 0x208
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMDE_AXI_RD_ERR             :1;      ///<BIT [0] CMDE_AXI_RD_ERR
        uint32_t CMDE_STRUCTURE_ERR          :1;      ///<BIT [1] CMDE_STRUCTURE_ERR
        uint32_t CMDE_INVALID_OPCODE_ERR     :1;      ///<BIT [2] CMDE_INVALID_OPCODE_ERR
        uint32_t CMDE_UNEXPECTED_CMD_PHASE_ERR :1;      ///<BIT [3] CMDE_UNEXPECTED_CMD_PHASE_ERR
        uint32_t RSVD_4_8                    :5;      ///<BIT [8:4] rsvd_4_8
        uint32_t FUNC_IN_ERR_STATE           :1;      ///<BIT [9] FUNC_IN_ERR_STATE
        uint32_t DESCM_SRC_DESCR_SGL_SEG_ERR :1;      ///<BIT [10] DESCM_SRC_DESCR_SGL_SEG_ERR
        uint32_t DESCM_DEST_DESCR_SGL_SEG_ERR :1;      ///<BIT [11] DESCM_DEST_DESCR_SGL_SEG_ERR
        uint32_t QOS_LATENCY_TO_ERR          :1;      ///<BIT [12] QOS_LATENCY_TO_ERR
        uint32_t DEST_DATA_XFR_TIMEOUT_ERR   :1;      ///<BIT [13] DEST_DATA_XFR_TIMEOUT_ERR
        uint32_t DESCM_SRC_AXI_RD_ERR        :1;      ///<BIT [14] DESCM_SRC_AXI_RD_ERR
        uint32_t DESCM_SRC_DESCR_STRUCTURE_ERR :1;      ///<BIT [15] DESCM_SRC_DESCR_STRUCTURE_ERR
        uint32_t DESCM_SRC_SGL_CROSS_4K_ERR  :1;      ///<BIT [16] DESCM_SRC_SGL_CROSS_4K_ERR
        uint32_t DESCM_SRC_SGL_ILLEGAL_DSCRPTR_ERR :1;      ///<BIT [17] DESCM_SRC_SGL_ILLEGAL_DSCRPTR_ERR
        uint32_t DESCM_SRC_SGL_UNDEFINED_DSCRPTR_TYPE :1;      ///<BIT [18] DESCM_SRC_SGL_UNDEFINED_DSCRPTR_TYPE
        uint32_t DESCM_SRC_SGL_TBL_LENGTH_ERR :1;      ///<BIT [19] DESCM_SRC_SGL_TBL_LENGTH_ERR
        uint32_t DESCM_SRC_SGL_LENGTH_ALIGNMENT_ERR :1;      ///<BIT [20] DESCM_SRC_SGL_LENGTH_ALIGNMENT_ERR
        uint32_t DESCM_SRC_PRP_OFST_ERR      :1;      ///<BIT [21] DESCM_SRC_PRP_OFST_ERR
        uint32_t DESCM_SRC_PRP_ALIGN_ERR     :1;      ///<BIT [22] DESCM_SRC_PRP_ALIGN_ERR
        uint32_t DESCM_DEST_AXI_RD_ERR       :1;      ///<BIT [23] DESCM_DEST_AXI_RD_ERR
        uint32_t DESCM_DEST_DESCR_STRUCTURE_ERR :1;      ///<BIT [24] DESCM_DEST_DESCR_STRUCTURE_ERR
        uint32_t DESCM_DEST_SGL_CROSS_4K_ERR :1;      ///<BIT [25] DESCM_DEST_SGL_CROSS_4K_ERR
        uint32_t DESCM_DEST_SGL_ILLEGAL_DSCRPTR_ERR :1;      ///<BIT [26] DESCM_DEST_SGL_ILLEGAL_DSCRPTR_ERR
        uint32_t DESCM_DEST_SGL_UNDEFINED_DSCRPTR_TYPE :1;      ///<BIT [27] DESCM_DEST_SGL_UNDEFINED_DSCRPTR_TYPE
        uint32_t DESCM_DEST_SGL_TBL_LENGTH_ERR :1;      ///<BIT [28] DESCM_DEST_SGL_TBL_LENGTH_ERR
        uint32_t DESCM_DEST_SGL_LENGTH_ALIGNMENT_ERR :1;      ///<BIT [29] DESCM_DEST_SGL_LENGTH_ALIGNMENT_ERR
        uint32_t DESCM_DEST_PRP_OFST_ERR     :1;      ///<BIT [30] DESCM_DEST_PRP_OFST_ERR
        uint32_t DESCM_DEST_PRP_ALIGN_ERR    :1;      ///<BIT [31] DESCM_DEST_PRP_ALIGN_ERR
    } b;
} CommandSlotErrorStatus0_t;

/// @brief 0x20C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DFE_MAX_ELMNT_COUNT_ERR     :1;      ///<BIT [0] DFE_MAX_ELMNT_COUNT_ERR
        uint32_t DFE_UNDERRUN_ERR            :1;      ///<BIT [1] DFE_UNDERRUN_ERR
        uint32_t DFE_OVERRUN_ERR             :1;      ///<BIT [2] DFE_OVERRUN_ERR
        uint32_t RSVD_3_4                    :2;      ///<BIT [4:3] rsvd_3_4
        uint32_t DBM_AXI_RD_ERR              :1;      ///<BIT [5] DBM_AXI_RD_ERR
        uint32_t DBM_BUFFER_RD_PARITY_ERR    :1;      ///<BIT [6] DBM_BUFFER_RD_PARITY_ERR
        uint32_t RSVD_7_8                    :2;      ///<BIT [8:7] rsvd_7_8
        uint32_t CRYPTOE_KEY_VAULT_MEM_RD_ERR :1;      ///<BIT [9] CRYPTOE_KEY_VAULT_MEM_RD_ERR
        uint32_t CRYPTOE_REDUNDANCY_MISMATCH_ERR :1;      ///<BIT [10] CRYPTOE_REDUNDANCY_MISMATCH_ERR
        uint32_t CRYPTOE_TEXT_OUT_READ_ERR   :1;      ///<BIT [11] CRYPTOE_TEXT_OUT_READ_ERR
        uint32_t CRYPTOE_FATAL_ERR           :1;      ///<BIT [12] CRYPTOE_FATAL_ERR
        uint32_t CRYPTOE_TAG_MISMATCH_ERR    :1;      ///<BIT [13] CRYPTOE_TAG_MISMATCH_ERR
        uint32_t RSVD_14_15                  :2;      ///<BIT [15:14] rsvd_14_15
        uint32_t DOE_AXI_WR_ERR              :1;      ///<BIT [16] DOE_AXI_WR_ERR
        uint32_t DOE_BUFFER_RD_PARITY_ERR    :1;      ///<BIT [17] DOE_BUFFER_RD_PARITY_ERR
        uint32_t DOE_UNDERRUN_ERR            :1;      ///<BIT [18] DOE_UNDERRUN_ERR
        uint32_t DOE_OVERRUN_ERR             :1;      ///<BIT [19] DOE_OVERRUN_ERR
        uint32_t DOE_MAX_ELMNT_COUNT_ERR     :1;      ///<BIT [20] DOE_MAX_ELMNT_COUNT_ERR
        uint32_t RSVD_21_27                  :7;      ///<BIT [27:21] rsvd_21_27
        uint32_t CMD_TRANSFER_LENGTH_OVERRUN_ERR :1;      ///<BIT [28] CMD_TRANSFER_LENGTH_OVERRUN_ERR
        uint32_t CMD_TRANSFER_LENGTH_UNDERRUN_ERR :1;      ///<BIT [29] CMD_TRANSFER_LENGTH_UNDERRUN_ERR
        uint32_t CMPLE_AXI_WR_ERR            :1;      ///<BIT [30] CMPLE_AXI_WR_ERR
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} CommandSlotErrorStatus1_t;

/// @brief 0x210
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CPU_CMD_ID                  :10;     ///<BIT [9:0] CPU_CMD_ID
        uint32_t RSVD_10_31                  :22;     ///<BIT [31:10] rsvd_10_31
    } b;
} DiagnosticCommandSlotStatus0_t;

/// @brief 0x230
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t NO_PROCESS_QOS_LATENCY_TIMEOUT_AS_ERROR :1;      ///<BIT [2] NO_PROCESS_QOS_LATENCY_TIMEOUT_AS_ERROR
        uint32_t RSVD_3_30                   :28;     ///<BIT [30:3] rsvd_3_30
        uint32_t PF_VF_ERR_BIT_CHECK         :1;      ///<BIT [31] PF_VF_ERR_BIT_CHECK
    } b;
} DpeMiscellaneousControl2_t;

/// @brief 0x248
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PF_VF_ERR_STATE_2           :1;      ///<BIT [0] PF_VF_ERR_STATE_2
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} PfVfErrState2_t;

typedef struct
{
    Cfg_t cfg;                                                              // 0x0 : Configuration / 
    CdmaControl_t control;                                                  // 0x4 : Control / 
    CdmaStatus_t status;                                                    // 0x8 : Status / 
    uint8_t rsvdC[20];                                                      // 0xC : rsvd_c / rsvd_c
    DeliveryQueue0Cfg_t deliveryQueue0Cfg;                                  // 0x20 : Delivery_Queue_0_Configuration / 
    uint32_t deliveryQueue0BaseAddressLowDlvryQ0BaseAddrL;                  // 0x24 : Delivery_Queue_0_Base_Address_Low / 
    uint32_t deliveryQueue0BaseAddressHighDlvryQ0BaseAddrH;                 // 0x28 : Delivery_Queue_0_Base_Address_High / 
    uint32_t deliveryQueue0CiShadowAddressLowDlvryQ0CnsmrIndxShdwAddrL;     // 0x2C : Delivery_Queue_0_CI_Shadow_Address_Low / 
    uint8_t rsvd30[4];                                                      // 0x30 : rsvd_30 / rsvd_30
    DeliveryQueue0ProducerIndex_t deliveryQueue0ProducerIndex;              // 0x34 : Delivery_Queue_0_Producer_Index / 
    DeliveryQueue0ConsumerIndex_t deliveryQueue0ConsumerIndex;              // 0x38 : Delivery_Queue_0_Consumer_Index / 
    uint8_t rsvd3c[68];                                                     // 0x3C : rsvd_3c / rsvd_3c
    CompletionQueue0Cfg_t completionQueue0Cfg;                              // 0x80 : Completion_Queue_0_Configuration / 
    uint32_t completionQueue0BaseAddressLowCmpltnQ0BaseAddrL;               // 0x84 : Completion_Queue_0_Base_Address_Low / 
    uint32_t completionQueue0BaseAddressHighCmpltnQ0BaseAddrH;              // 0x88 : Completion_Queue_0_Base_Address_High / 
    uint32_t completionQueue0PiShadowAddressLowCmpltnQ0PrdcrIndxShdwAddrL;  // 0x8C : Completion_Queue_0_PI_Shadow_Address_Low / 
    uint8_t rsvd90[4];                                                      // 0x90 : rsvd_90 / rsvd_90
    CompletionQueue0ProducerIndex_t completionQueue0ProducerIndex;          // 0x94 : Completion_Queue_0_Producer_Index / 
    CompletionQueue0ConsumerIndex_t completionQueue0ConsumerIndex;          // 0x98 : Completion_Queue_0_Consumer_Index / 
    uint8_t rsvd9c[20];                                                     // 0x9C : rsvd_9c / rsvd_9c
    uint32_t dummySlavePortBaseAddressLowDummySlavePortBaseAddrL;           // 0xB0 : Dummy_Slave_Port_Base_Address_Low / 
    uint32_t dummySlavePortBaseAddressHighDummySlavePortBaseAddrH;          // 0xB4 : Dummy_Slave_Port_Base_Address_High / 
    uint8_t rsvdB8[40];                                                     // 0xB8 : rsvd_b8 / rsvd_b8
    List0Cfg_t list0Cfg;                                                    // 0xE0 : List_0_Configuration / 
    uint32_t list0BaseAddressLowList0BaseAddrL;                             // 0xE4 : List_0_Base_Address_Low / 
    uint32_t list0BaseAddressHighList0BaseAddrH;                            // 0xE8 : List_0_Base_Address_High / 
    uint8_t rsvdEc[4];                                                      // 0xEC : rsvd_ec / rsvd_ec
    List1Cfg_t list1Cfg;                                                    // 0xF0 : List_1_Configuration / 
    uint32_t list1BaseAddressLowList1BaseAddrL;                             // 0xF4 : List_1_Base_Address_Low / 
    uint32_t list1BaseAddressHighList1BaseAddrH;                            // 0xF8 : List_1_Base_Address_High / 
    uint8_t rsvdFc[4];                                                      // 0xFC : rsvd_fc / rsvd_fc
    List2Cfg_t list2Cfg;                                                    // 0x100 : List_2_Configuration / 
    uint32_t list2BaseAddressLowList2BaseAddrL;                             // 0x104 : List_2_Base_Address_Low / 
    uint32_t list2BaseAddressHighList2BaseAddrH;                            // 0x108 : List_2_Base_Address_High / 
    uint8_t rsvd10c[4];                                                     // 0x10C : rsvd_10c / rsvd_10c
    List3Cfg_t list3Cfg;                                                    // 0x110 : List_3_Configuration / 
    uint32_t list3BaseAddressLowList3BaseAddrL;                             // 0x114 : List_3_Base_Address_Low / 
    uint32_t list3BaseAddressHighList3BaseAddrH;                            // 0x118 : List_3_Base_Address_High / 
    uint8_t rsvd11c[8];                                                     // 0x11C : rsvd_11c / rsvd_11c
    uint32_t descMaxElementCountMaxDescrElmntCountPerChunk;                 // 0x124 : Descriptor_Max_Element_Count / 
    uint8_t rsvd128[24];                                                    // 0x128 : rsvd_128 / rsvd_128
    uint32_t commandSlotErrorStateStatus0CmdSlotErrorStateStatus0;          // 0x140 : Command_Slot_Error_State_Status_0 / 
    uint32_t commandSlotErrorStateStatus1CmdSlotErrorStateStatus1;          // 0x144 : Command_Slot_Error_State_Status_1 / 
    CommandSlotErrorStateStatus2_t commandSlotErrorStateStatus2;            // 0x148 : Command_Slot_Error_State_Status_2 / 
    uint8_t rsvd14c[4];                                                     // 0x14C : rsvd_14c / rsvd_14c
    uint32_t commandSlotReservedStatus0CmdSlotReservedStatus0;              // 0x150 : Command_Slot_Reserved_Status_0 / 
    uint32_t commandSlotReservedStatus1CmdSlotReservedStatus1;              // 0x154 : Command_Slot_Reserved_Status_1 / 
    CommandSlotReservedStatus2_t commandSlotReservedStatus2;                // 0x158 : Command_Slot_Reserved_Status_2 / 
    uint8_t rsvd15c[4];                                                     // 0x15C : rsvd_15c / rsvd_15c
    IntrCoalescingCfg0_t intrCoalescingCfg0;                                // 0x160 : Interrupt_Coalescing_Configuration_0 / 
    IntrCoalescingCfg1_t intrCoalescingCfg1;                                // 0x164 : Interrupt_Coalescing_Configuration_1 / 
    uint8_t rsvd168[8];                                                     // 0x168 : rsvd_168 / rsvd_168
    AxiMasterAxuserInfo_t axiMasterAxuserInfo;                              // 0x170 : AXI_Master_AxUser_Info / 
    uint8_t rsvd174[12];                                                    // 0x174 : rsvd_174 / rsvd_174
    IntrCause_t intrCause;                                                  // 0x180 : Interrupt_Cause / 
    uint32_t intrEnable0CdmaIrqEn0;                                         // 0x184 : Interrupt_Enable_0 / 
    uint32_t intrEnable1CdmaIrqEn1;                                         // 0x188 : Interrupt_Enable_1 / 
    uint32_t haltEnableBcpErrorHaltEn;                                      // 0x18C : Halt_Enable / 
    GlobalErrorCheckEnable_t globalErrorCheckEnable;                        // 0x190 : Global_Error_Check_Enable / 
    uint32_t commandFreePoolTimerCmdFreePoolEmptyTimeoutThresh;             // 0x194 : Command_Free_Pool_Timer / 
    uint32_t errorAxiTransactionInformationErrAxiTxnInfo;                   // 0x198 : Error_AXI_Transaction_Information / 
    uint8_t rsvd19c[16];                                                    // 0x19C : rsvd_19c / rsvd_19c
    DataPathParityControl_t dataPathParityControl;                          // 0x1AC : Data_Path_Parity_Control / 
    DataPathParityStatus_t dataPathParityStatus;                            // 0x1B0 : Data_Path_Parity_Status / 
    uint32_t dataPathErrorAddressLowErrAddrL;                               // 0x1B4 : Data_Path_Error_Address_Low / 
    uint32_t dataPathErrorAddressHighErrAddrH;                              // 0x1B8 : Data_Path_Error_Address_High / 
    uint8_t rsvd1bc[4];                                                     // 0x1BC : rsvd_1bc / rsvd_1bc
    DpeMiscellaneousControl0_t dpeMiscellaneousControl0;                    // 0x1C0 : DPE_miscellaneous_Control_0 / 
    DpeMiscellaneousControl1_t dpeMiscellaneousControl1;                    // 0x1C4 : DPE_miscellaneous_Control_1 / 
    uint32_t dpeMiscellaneousStatus0DpeMiscStatus0;                         // 0x1C8 : DPE_miscellaneous_Status_0 / 
    uint32_t dpeMiscellaneousStatus1DpeMiscStatus1;                         // 0x1CC : DPE_miscellaneous_Status_1 / 
    ErrorInjection_t errorInjection;                                        // 0x1D0 : Error_Injection / 
    KeyVaultMemoryUncorrectableErrorCount_t keyVaultMemoryUncorrectableErrorCount;// 0x1D4 : Key_Vault_Memory_Uncorrectable_Error_Count / 
    KeyVaultMemoryCorrectableErrorCount_t keyVaultMemoryCorrectableErrorCount;// 0x1D8 : Key_Vault_Memory_Correctable_Error_Count / 
    uint32_t commandAndCompletionEngineStatusCmdCmplEngineStatus;           // 0x1DC : Command_and_Completion_Engine_Status / 
    uint8_t rsvd1e0[16];                                                    // 0x1E0 : rsvd_1e0 / rsvd_1e0
    uint32_t hwDiagnosticTracePort;                                         // 0x1F0 : HW_Diagnostic_Traceport / 
    uint32_t diagnosticControlDiagnosticCmdSlotId;                          // 0x1F4 : Diagnostic_Control / 
    uint8_t rsvd1f8[8];                                                     // 0x1F8 : rsvd_1f8 / rsvd_1f8
    uint32_t commandSlotErrorCheckEnable0CmdSlotErrorCheckEnable0;          // 0x200 : Command_Slot_Error_Check_Enable_0 / 
    uint32_t commandSlotErrorCheckEnable1CmdSlotErrorCheckEnable1;          // 0x204 : Command_Slot_Error_Check_Enable_1 / 
    CommandSlotErrorStatus0_t commandSlotErrorStatus0;                      // 0x208 : Command_Slot_Error_Status_0 / 
    CommandSlotErrorStatus1_t commandSlotErrorStatus1;                      // 0x20C : Command_Slot_Error_Status_1 / 
    DiagnosticCommandSlotStatus0_t diagnosticCommandSlotStatus0;            // 0x210 : Diagnostic_Command_Slot_Status_0 / 
    uint8_t rsvd214[28];                                                    // 0x214 : rsvd_214 / rsvd_214
    DpeMiscellaneousControl2_t dpeMiscellaneousControl2;                    // 0x230 : DPE_miscellaneous_Control_2 / 
    uint32_t timerValue;                                                    // 0x234 : Timer_Value / 
    uint32_t timerValue2;                                                   // 0x238 : Timer_Value_2 / 
    uint32_t timerValue3;                                                   // 0x23C : Timer_Value_3 / 
    uint32_t pfVfErrState0;                                                 // 0x240 : PF_VF_err_state_0 / 
    uint32_t pfVfErrState1;                                                 // 0x244 : PF_VF_err_state_1 / 
    PfVfErrState2_t pfVfErrState2;                                          // 0x248 : PF_VF_err_state_2 / 
    uint32_t dpeMiscellaneousControl3DpeMiscControl3;                       // 0x24C : DPE_miscellaneous_Control_3 / 
    uint8_t rsvd250[3504];                                                  // 0x250 : rsvd_250 / rsvd_250
    uint32_t aesKeyVault;                                                   // 0x1000 : AES_Key_Vault / 
} Cdma_t;

COMPILE_ASSERT(offsetof(Cdma_t,cfg)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,control)==0x4,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,status)==0x8,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,deliveryQueue0Cfg)==0x20,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,deliveryQueue0BaseAddressLowDlvryQ0BaseAddrL)==0x24,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,deliveryQueue0BaseAddressHighDlvryQ0BaseAddrH)==0x28,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,deliveryQueue0CiShadowAddressLowDlvryQ0CnsmrIndxShdwAddrL)==0x2C,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,deliveryQueue0ProducerIndex)==0x34,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,deliveryQueue0ConsumerIndex)==0x38,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,completionQueue0Cfg)==0x80,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,completionQueue0BaseAddressLowCmpltnQ0BaseAddrL)==0x84,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,completionQueue0BaseAddressHighCmpltnQ0BaseAddrH)==0x88,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,completionQueue0PiShadowAddressLowCmpltnQ0PrdcrIndxShdwAddrL)==0x8C,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,completionQueue0ProducerIndex)==0x94,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,completionQueue0ConsumerIndex)==0x98,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,dummySlavePortBaseAddressLowDummySlavePortBaseAddrL)==0xB0,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,dummySlavePortBaseAddressHighDummySlavePortBaseAddrH)==0xB4,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,list0Cfg)==0xE0,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,list0BaseAddressLowList0BaseAddrL)==0xE4,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,list0BaseAddressHighList0BaseAddrH)==0xE8,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,list1Cfg)==0xF0,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,list1BaseAddressLowList1BaseAddrL)==0xF4,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,list1BaseAddressHighList1BaseAddrH)==0xF8,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,list2Cfg)==0x100,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,list2BaseAddressLowList2BaseAddrL)==0x104,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,list2BaseAddressHighList2BaseAddrH)==0x108,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,list3Cfg)==0x110,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,list3BaseAddressLowList3BaseAddrL)==0x114,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,list3BaseAddressHighList3BaseAddrH)==0x118,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,descMaxElementCountMaxDescrElmntCountPerChunk)==0x124,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,commandSlotErrorStateStatus0CmdSlotErrorStateStatus0)==0x140,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,commandSlotErrorStateStatus1CmdSlotErrorStateStatus1)==0x144,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,commandSlotErrorStateStatus2)==0x148,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,commandSlotReservedStatus0CmdSlotReservedStatus0)==0x150,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,commandSlotReservedStatus1CmdSlotReservedStatus1)==0x154,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,commandSlotReservedStatus2)==0x158,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,intrCoalescingCfg0)==0x160,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,intrCoalescingCfg1)==0x164,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,axiMasterAxuserInfo)==0x170,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,intrCause)==0x180,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,intrEnable0CdmaIrqEn0)==0x184,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,intrEnable1CdmaIrqEn1)==0x188,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,haltEnableBcpErrorHaltEn)==0x18C,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,globalErrorCheckEnable)==0x190,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,commandFreePoolTimerCmdFreePoolEmptyTimeoutThresh)==0x194,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,errorAxiTransactionInformationErrAxiTxnInfo)==0x198,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,dataPathParityControl)==0x1AC,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,dataPathParityStatus)==0x1B0,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,dataPathErrorAddressLowErrAddrL)==0x1B4,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,dataPathErrorAddressHighErrAddrH)==0x1B8,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,dpeMiscellaneousControl0)==0x1C0,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,dpeMiscellaneousControl1)==0x1C4,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,dpeMiscellaneousStatus0DpeMiscStatus0)==0x1C8,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,dpeMiscellaneousStatus1DpeMiscStatus1)==0x1CC,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,errorInjection)==0x1D0,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,keyVaultMemoryUncorrectableErrorCount)==0x1D4,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,keyVaultMemoryCorrectableErrorCount)==0x1D8,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,commandAndCompletionEngineStatusCmdCmplEngineStatus)==0x1DC,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,hwDiagnosticTracePort)==0x1F0,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,diagnosticControlDiagnosticCmdSlotId)==0x1F4,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,commandSlotErrorCheckEnable0CmdSlotErrorCheckEnable0)==0x200,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,commandSlotErrorCheckEnable1CmdSlotErrorCheckEnable1)==0x204,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,commandSlotErrorStatus0)==0x208,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,commandSlotErrorStatus1)==0x20C,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,diagnosticCommandSlotStatus0)==0x210,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,dpeMiscellaneousControl2)==0x230,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,timerValue)==0x234,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,timerValue2)==0x238,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,timerValue3)==0x23C,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,pfVfErrState0)==0x240,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,pfVfErrState1)==0x244,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,pfVfErrState2)==0x248,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,dpeMiscellaneousControl3DpeMiscControl3)==0x24C,"check register structure offset");
COMPILE_ASSERT(offsetof(Cdma_t,aesKeyVault)==0x1000,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile Cdma_t rCdma; ///< 0xA0C00000
