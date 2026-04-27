// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @brief UCD Registers
//!
//=============================================================================

// Generated with Dullahan v2.2.6.03a6f27

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "SysTypes.h"
#include "assert.h"

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
        uint32_t RSVD_0                      : 31;    ///<BIT [30:0] rsvd_0
        uint32_t OB_UCD_RST                  : 1;     ///<BIT [31] ob_ucd_rst
    } b;
} UcducdObCmnSnglOutboundUcdCfg_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_UCD_ENBL                 : 1;     ///<BIT [0] ob_ucd_enbl
        uint32_t OB_UCD_PAUSE                : 1;     ///<BIT [1] ob_ucd_pause
        uint32_t RSVD_0                      : 14;    ///<BIT [15:2] rsvd_0
        uint32_t OB_IRQ_SRVC_WAIT_TIME       : 16;    ///<BIT [31:16] ob_irq_srvc_wait_time
    } b;
} UcducdObCmnSnglOutboundUcdControl_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_UCD_ENBLD                : 1;     ///<BIT [0] ob_ucd_enbld
        uint32_t OB_UCD_PAUSED               : 1;     ///<BIT [1] ob_ucd_paused
        uint32_t OB_UCD_HALTED               : 1;     ///<BIT [2] ob_ucd_halted
        uint32_t RSVD_1                      : 5;     ///<BIT [7:3] rsvd_1
        uint32_t OB_UCD_PAUSED_RSN_TXN_SM    : 1;     ///<BIT [8] ob_ucd_paused_rsn_txn_sm
        uint32_t OB_UCD_PAUSED_RSN_CQ_FULL   : 1;     ///<BIT [9] ob_ucd_paused_rsn_cq_full
        uint32_t RSVD_0                      : 22;    ///<BIT [31:10] rsvd_0
    } b;
} UcducdObCmnSnglOutboundUcdStatus_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_FP1_CMPLTN_Q_IRQ         : 1;     ///<BIT [0] ob_fp1_cmpltn_q_irq
        uint32_t OB_FP2_CMPLTN_Q_IRQ         : 1;     ///<BIT [1] ob_fp2_cmpltn_q_irq
        uint32_t OB_RR1_CMPLTN_Q_IRQ         : 1;     ///<BIT [2] ob_rr1_cmpltn_q_irq
        uint32_t OB_RR2_CMPLTN_Q_IRQ         : 1;     ///<BIT [3] ob_rr2_cmpltn_q_irq
        uint32_t OB_RR3_CMPLTN_Q_IRQ         : 1;     ///<BIT [4] ob_rr3_cmpltn_q_irq
        uint32_t RSVD_2                      : 2;     ///<BIT [6:5] rsvd_2
        uint32_t OB_INTRNL_HW_ERR            : 1;     ///<BIT [7] ob_intrnl_hw_err
        uint32_t OB_Q_SOFT_ERR_IRQ           : 1;     ///<BIT [8] ob_q_soft_err_irq
        uint32_t RSVD_1                      : 3;     ///<BIT [11:9] rsvd_1
        uint32_t OB_FP1_SRC_LIST_OVRFLW_IRQ  : 1;     ///<BIT [12] ob_fp1_src_list_ovrflw_irq
        uint32_t OB_FP2_SRC_LIST_OVRFLW_IRQ  : 1;     ///<BIT [13] ob_fp2_src_list_ovrflw_irq
        uint32_t OB_RR1_SRC_LIST_OVRFLW_IRQ  : 1;     ///<BIT [14] ob_rr1_src_list_ovrflw_irq
        uint32_t OB_RR2_SRC_LIST_OVRFLW_IRQ  : 1;     ///<BIT [15] ob_rr2_src_list_ovrflw_irq
        uint32_t OB_RR3_SRC_LIST_OVRFLW_IRQ  : 1;     ///<BIT [16] ob_rr3_src_list_ovrflw_irq
        uint32_t OB_FP1_CMPLTN_Q_FULL_IRQ    : 1;     ///<BIT [17] ob_fp1_cmpltn_q_full_irq
        uint32_t OB_FP2_CMPLTN_Q_FULL_IRQ    : 1;     ///<BIT [18] ob_fp2_cmpltn_q_full_irq
        uint32_t OB_RR1_CMPLTN_Q_FULL_IRQ    : 1;     ///<BIT [19] ob_rr1_cmpltn_q_full_irq
        uint32_t OB_RR2_CMPLTN_Q_FULL_IRQ    : 1;     ///<BIT [20] ob_rr2_cmpltn_q_full_irq
        uint32_t OB_RR3_CMPLTN_Q_FULL_IRQ    : 1;     ///<BIT [21] ob_rr3_cmpltn_q_full_irq
        uint32_t RSVD_0                      : 7;     ///<BIT [28:22] rsvd_0
        uint32_t OB_CONTROL_PATH_ERR         : 1;     ///<BIT [29] ob_control_path_err
        uint32_t OB_INTRNL_MEM_PERR          : 1;     ///<BIT [30] ob_intrnl_mem_perr
        uint32_t OB_DATA_PATH_ERR            : 1;     ///<BIT [31] ob_data_path_err
    } b;
} UcducdObCmnSnglOutboundUcdIntrCause_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_UCD_SRAM_PERR            : 1;     ///<BIT [0] ob_ucd_sram_perr
        uint32_t RSVD_0                      : 31;    ///<BIT [31:1] rsvd_0
    } b;
} UcducdObCmnSnglOutboundUcdSramParityErrorCause_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_UCD_SRAM_PERR_EN         : 1;     ///<BIT [0] ob_ucd_sram_perr_en
        uint32_t RSVD_0                      : 31;    ///<BIT [31:1] rsvd_0
    } b;
} UcducdObCmnSnglOutboundUcdSramParityErrorEnable_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_DP_PAR                   : 1;     ///<BIT [0] ob_dp_par
        uint32_t OB_DP_PERR_EN               : 1;     ///<BIT [1] ob_dp_perr_en
        uint32_t OB_DP_FERR_EN               : 1;     ///<BIT [2] ob_dp_ferr_en
        uint32_t OB_DP_RD_TXN_ERR_DSBL       : 3;     ///<BIT [5:3] ob_dp_rd_txn_err_dsbl
        uint32_t OB_DP_WR_TXN_ERR_DSBL       : 2;     ///<BIT [7:6] ob_dp_wr_txn_err_dsbl
        uint32_t RSVD_0                      : 6;     ///<BIT [13:8] rsvd_0
        uint32_t OB_FRC_DP_PERR_CONT         : 1;     ///<BIT [14] ob_frc_dp_perr_cont
        uint32_t OB_FRC_DP_PERR_ONCE         : 1;     ///<BIT [15] ob_frc_dp_perr_once
        uint32_t OB_DP_PRTY_MASK             : 16;    ///<BIT [31:16] ob_dp_prty_mask
    } b;
} UcducdObCmnSnglOutboundUcdDataPathErrorControl_t;

/// @brief 0x40
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_1                      : 1;     ///<BIT [0] rsvd_1
        uint32_t OB_DP_PERR                  : 1;     ///<BIT [1] ob_dp_perr
        uint32_t OB_DP_FERR                  : 1;     ///<BIT [2] ob_dp_ferr
        uint32_t OB_DP_RD_TXN_ERR            : 3;     ///<BIT [5:3] ob_dp_rd_txn_err
        uint32_t OB_DP_WR_TXN_ERR            : 2;     ///<BIT [7:6] ob_dp_wr_txn_err
        uint32_t RSVD_0                      : 16;    ///<BIT [23:8] rsvd_0
        uint32_t OB_ERR_PORT                 : 8;     ///<BIT [31:24] ob_err_port
    } b;
} UcducdObCmnSnglOutboundUcdDataPathErrorStatus_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ARB_BURST                   : 3;     ///<BIT [2:0] arb_burst
        uint32_t RSVD_0                      : 5;     ///<BIT [7:3] rsvd_0
        uint32_t RR_3_WEIGHT                 : 8;     ///<BIT [15:8] rr_3_weight
        uint32_t RR_2_WEIGHT                 : 8;     ///<BIT [23:16] rr_2_weight
        uint32_t RR_1_WEIGHT                 : 8;     ///<BIT [31:24] rr_1_weight
    } b;
} UcducdObCmnSnglOutboundQueueArbitrationCfgRegister_t;

/// @brief 0x60
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_QUEUE_ELMNT_LNGTH_0      : 16;    ///<BIT [15:0] ob_queue_elmnt_lngth_0
        uint32_t OB_QUEUE_ELMNT_LNGTH_1      : 16;    ///<BIT [31:16] ob_queue_elmnt_lngth_1
    } b;
} UcducdObCmnSnglOutboundSizeSelect0_t;

/// @brief 0x64
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_QUEUE_ELMNT_LNGTH_2      : 16;    ///<BIT [15:0] ob_queue_elmnt_lngth_2
        uint32_t OB_QUEUE_ELMNT_LNGTH_3      : 16;    ///<BIT [31:16] ob_queue_elmnt_lngth_3
    } b;
} UcducdObCmnSnglOutboundSizeSelect1_t;

/// @brief 0x68
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_QUEUE_ELMNT_LNGTH_4      : 16;    ///<BIT [15:0] ob_queue_elmnt_lngth_4
        uint32_t OB_QUEUE_ELMNT_LNGTH_5      : 16;    ///<BIT [31:16] ob_queue_elmnt_lngth_5
    } b;
} UcducdObCmnSnglOutboundSizeSelect2_t;

/// @brief 0x6C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_QUEUE_ELMNT_LNGTH_6      : 16;    ///<BIT [15:0] ob_queue_elmnt_lngth_6
        uint32_t OB_QUEUE_ELMNT_LNGTH_7      : 16;    ///<BIT [31:16] ob_queue_elmnt_lngth_7
    } b;
} UcducdObCmnSnglOutboundSizeSelect3_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_SRC_LIST_EN              : 1;     ///<BIT [0] ob_src_list_en
        uint32_t RSVD_2                      : 7;     ///<BIT [7:1] rsvd_2
        uint32_t OB_SRC_LIST_SIZE            : 4;     ///<BIT [11:8] ob_src_list_size
        uint32_t RSVD_1                      : 4;     ///<BIT [15:12] rsvd_1
        uint32_t OB_SRC_LIST_IFC_SLCT        : 8;     ///<BIT [23:16] ob_src_list_ifc_slct
        uint32_t RSVD_0                      : 8;     ///<BIT [31:24] rsvd_0
    } b;
} UcducdObCmnOslOutboundSourceListCfg0_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      : 5;     ///<BIT [4:0] rsvd_0
        uint32_t OB_SRC_LIST_BASE_ADDR_LO    : 27;    ///<BIT [31:5] ob_src_list_base_addr_lo
    } b;
} UcducdObCmnOslOutboundSourceListBaseAddrLo_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_SRC_LIST_PI              : 16;    ///<BIT [15:0] ob_src_list_pi
        uint32_t RSVD_0                      : 16;    ///<BIT [31:16] rsvd_0
    } b;
} UcducdObCmnOslOutboundSourceListPi_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_SRC_LIST_CI              : 16;    ///<BIT [15:0] ob_src_list_ci
        uint32_t RSVD_0                      : 16;    ///<BIT [31:16] rsvd_0
    } b;
} UcducdObCmnOslOutboundSourceListCi_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_SRC_LIST_EMPTY           : 1;     ///<BIT [0] ob_src_list_empty
        uint32_t OB_SRC_LIST_FULL            : 1;     ///<BIT [1] ob_src_list_full
        uint32_t OB_SRC_LIST_OVRFLW          : 1;     ///<BIT [2] ob_src_list_ovrflw
        uint32_t RSVD_1                      : 6;     ///<BIT [8:3] rsvd_1
        uint32_t OB_SRC_LIST_FIFO_EMPTY      : 1;     ///<BIT [9] ob_src_list_fifo_empty
        uint32_t RSVD_0                      : 22;    ///<BIT [31:10] rsvd_0
    } b;
} UcducdObCmnOslOutboundSourceListStatus_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_CMPLTN_Q_EN              : 1;     ///<BIT [0] ob_cmpltn_q_en
        uint32_t OB_CMPLTN_Q_SHDW_EN         : 1;     ///<BIT [1] ob_cmpltn_q_shdw_en
        uint32_t RSVD_2                      : 6;     ///<BIT [7:2] rsvd_2
        uint32_t OB_CMPLTN_Q_SIZE            : 4;     ///<BIT [11:8] ob_cmpltn_q_size
        uint32_t RSVD_1                      : 4;     ///<BIT [15:12] rsvd_1
        uint32_t OB_CMPLTN_Q_IFC_SLCT        : 8;     ///<BIT [23:16] ob_cmpltn_q_ifc_slct
        uint32_t RSVD_0                      : 8;     ///<BIT [31:24] rsvd_0
    } b;
} UcducdObCmnCqOutboundCompletionQueueCfgControl_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_CMPLTN_Q_MAX_COAL_TIME   : 16;    ///<BIT [15:0] ob_cmpltn_q_max_coal_time
        uint32_t OB_CMPLTN_Q_MIN_COAL_TIME   : 16;    ///<BIT [31:16] ob_cmpltn_q_min_coal_time
    } b;
} UcducdObCmnCqOutboundCompletionQueueIntrCoalescing0_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_CMPLTN_Q_INT_COAL_COUNT  : 16;    ///<BIT [15:0] ob_cmpltn_q_int_coal_count
        uint32_t OB_CMPLTN_Q_EN_INT_COAL     : 1;     ///<BIT [16] ob_cmpltn_q_en_int_coal
        uint32_t RSVD_1                      : 1;     ///<BIT [17] rsvd_1
        uint32_t OB_CMPLTN_Q_RESTART_WHEN_CI_UPDT : 1;     ///<BIT [18] ob_cmpltn_q_restart_when_ci_updt
        uint32_t RSVD_0                      : 13;    ///<BIT [31:19] rsvd_0
    } b;
} UcducdObCmnCqOutboundCompletionQueueIntrCoalescing1_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      : 2;     ///<BIT [1:0] rsvd_0
        uint32_t OB_CMPLTN_Q_PI_SHDW_BASE_ADDR_LO : 30;    ///<BIT [31:2] ob_cmpltn_q_pi_shdw_base_addr_lo
    } b;
} UcducdObCmnCqOutboundCompletionQueuePiShadowBaseAddrLo_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      : 5;     ///<BIT [4:0] rsvd_0
        uint32_t OB_CMPLTN_Q_BASE_ADDR_LO    : 27;    ///<BIT [31:5] ob_cmpltn_q_base_addr_lo
    } b;
} UcducdObCmnCqOutboundCompletionQueueBaseAddrLo_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_CMPLTN_Q_PI              : 16;    ///<BIT [15:0] ob_cmpltn_q_pi
        uint32_t RSVD_0                      : 16;    ///<BIT [31:16] rsvd_0
    } b;
} UcducdObCmnCqOutboundCompletionQueuePi_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_CMPLTN_Q_CI              : 16;    ///<BIT [15:0] ob_cmpltn_q_ci
        uint32_t RSVD_1                      : 14;    ///<BIT [29:16] rsvd_1
        uint32_t OB_CMPLTN_Q_INTRPT_CLR      : 1;     ///<BIT [30] ob_cmpltn_q_intrpt_clr
        uint32_t RSVD_0                      : 1;     ///<BIT [31] rsvd_0
    } b;
} UcducdObCmnCqOutboundCompletionQueueCi_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_CMPLTN_Q_EMPTY           : 1;     ///<BIT [0] ob_cmpltn_q_empty
        uint32_t OB_CMPLTN_Q_FULL            : 1;     ///<BIT [1] ob_cmpltn_q_full
        uint32_t RSVD_1                      : 6;     ///<BIT [7:2] rsvd_1
        uint32_t OB_CMPLTN_Q_CS_LKHD_EMPTY   : 1;     ///<BIT [8] ob_cmpltn_q_cs_lkhd_empty
        uint32_t OB_CMPLTN_Q_CS_LKHD_FULL    : 1;     ///<BIT [9] ob_cmpltn_q_cs_lkhd_full
        uint32_t OB_CMPLTN_Q_CS_FULL         : 1;     ///<BIT [10] ob_cmpltn_q_cs_full
        uint32_t RSVD_0                      : 5;     ///<BIT [15:11] rsvd_0
        uint32_t OB_CMPLTN_Q_LKHD_PI         : 16;    ///<BIT [31:16] ob_cmpltn_q_lkhd_pi
    } b;
} UcducdObCmnCqOutboundCompletionQueueStatus_t;

/// @brief 0x300
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DATA_FIFO_MEM_RF2P_RTC      : 2;     ///<BIT [1:0] data_fifo_mem_rf2p_rtc
        uint32_t DATA_FIFO_MEM_RF2P_WTC      : 2;     ///<BIT [3:2] data_fifo_mem_rf2p_wtc
        uint32_t OQ_NOT_FULL_WAIT_TMR_SEL    : 3;     ///<BIT [6:4] oq_not_full_wait_tmr_sel
        uint32_t CQ_NOT_FULL_WAIT_TMR_SEL    : 3;     ///<BIT [9:7] cq_not_full_wait_tmr_sel
        uint32_t RSVD_0                      : 1;     ///<BIT [10] rsvd_0
        uint32_t ENBL_CQ_FULL_PAUSE          : 1;     ///<BIT [11] enbl_cq_full_pause
        uint32_t DSBL_CQ_CLR_CQ_UPDT         : 1;     ///<BIT [12] dsbl_cq_clr_cq_updt
        uint32_t ENBL_CQ_CLR_CQ_EMPTY        : 1;     ///<BIT [13] enbl_cq_clr_cq_empty
        uint32_t RSVD_1                      : 2;     ///<BIT [15:14] rsvd_1
        uint32_t DSBL_ELMNT_SKIP_OQ_FULL     : 1;     ///<BIT [16] dsbl_elmnt_skip_oq_full
        uint32_t DSBL_OQ_IRQ_CLR_CI_UPDT     : 1;     ///<BIT [17] dsbl_oq_irq_clr_ci_updt
        uint32_t DSBL_OQ_IRQ_CLR_ON_MASK_ENBL : 1;     ///<BIT [18] dsbl_oq_irq_clr_on_mask_enbl
        uint32_t DSBL_OQ_IRQ_CLR_ON_EMPTY    : 1;     ///<BIT [19] dsbl_oq_irq_clr_on_empty
        uint32_t DSBL_AXI_ERR_PROPAGATION    : 1;     ///<BIT [20] dsbl_axi_err_propagation
        uint32_t DP_DIAG_HALT                : 1;     ///<BIT [21] dp_diag_halt
        uint32_t ENBL_FSC_SM_SYNC            : 1;     ///<BIT [22] enbl_fsc_sm_sync
        uint32_t ENBL_Q_ACC_WHILE_DSBLD_ERR  : 1;     ///<BIT [23] enbl_q_acc_while_dsbld_err
        uint32_t DSBL_DBELL_COMPLIANCE_CHK_ERR : 3;     ///<BIT [26:24] dsbl_dbell_compliance_chk_err
        uint32_t RSVD_2                      : 4;     ///<BIT [30:27] rsvd_2
        uint32_t MISC_ERROR_STATUS_CLR       : 1;     ///<BIT [31] misc_error_status_clr
    } b;
} UcducdObCmnSnglOutboundMiscellaneousControl_t;

/// @brief 0x310
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DIAG_READ_GROUP_SEL         : 8;     ///<BIT [7:0] diag_read_group_sel
        uint32_t DIAG_FIFO_RD_PTR            : 8;     ///<BIT [15:8] diag_fifo_rd_ptr
        uint32_t DIAG_HW_CTL                 : 8;     ///<BIT [23:16] diag_hw_ctl
        uint32_t RSVD_0                      : 8;     ///<BIT [31:24] rsvd_0
    } b;
} UcducdObCmnSnglOutboundDiagnosticControl_t;

/// @brief 0x320
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DC_AXI_ARUSER_INFO          : 4;     ///<BIT [3:0] dc_axi_aruser_info
        uint32_t RSVD_2                      : 12;    ///<BIT [15:4] rsvd_2
        uint32_t DC_AXI_ARCACHE              : 4;     ///<BIT [19:16] dc_axi_arcache
        uint32_t RSVD_1                      : 4;     ///<BIT [23:20] rsvd_1
        uint32_t DC_AXI_MAX_RD_BURST_BYTE_CNT_SEL : 2;     ///<BIT [25:24] dc_axi_max_rd_burst_byte_cnt_sel
        uint32_t RSVD_0                      : 6;     ///<BIT [31:26] rsvd_0
    } b;
} UcducdObCmnSnglOutboundDataChannelAxiReadBusAttrs_t;

/// @brief 0x324
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DC_AXI_AWUSER_INFO          : 5;     ///<BIT [4:0] dc_axi_awuser_info
        uint32_t RSVD_2                      : 11;    ///<BIT [15:5] rsvd_2
        uint32_t DC_AXI_AWCACHE              : 4;     ///<BIT [19:16] dc_axi_awcache
        uint32_t RSVD_1                      : 4;     ///<BIT [23:20] rsvd_1
        uint32_t DC_AXI_MAX_WR_BURST_BYTE_CNT_SEL : 2;     ///<BIT [25:24] dc_axi_max_wr_burst_byte_cnt_sel
        uint32_t RSVD_0                      : 5;     ///<BIT [30:26] rsvd_0
        uint32_t DC_AXI_AWCACHE0_AUTO_GEN_DISABLE : 1;     ///<BIT [31] dc_axi_awcache0_auto_gen_disable
    } b;
} UcducdObCmnSnglOutboundDataChannelAxiWriteBusAttrs_t;

/// @brief 0x328
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CC_AXI_ARUSER_INFO          : 4;     ///<BIT [3:0] cc_axi_aruser_info
        uint32_t RSVD_1                      : 12;    ///<BIT [15:4] rsvd_1
        uint32_t CC_AXI_ARCACHE              : 4;     ///<BIT [19:16] cc_axi_arcache
        uint32_t RSVD_0                      : 12;    ///<BIT [31:20] rsvd_0
    } b;
} UcducdObCmnSnglOutboundControlChannelAxiReadBusAttrs_t;

/// @brief 0x32C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CC_AXI_AWUSER_INFO          : 5;     ///<BIT [4:0] cc_axi_awuser_info
        uint32_t RSVD_1                      : 11;    ///<BIT [15:5] rsvd_1
        uint32_t CC_AXI_AWCACHE              : 4;     ///<BIT [19:16] cc_axi_awcache
        uint32_t RSVD_0                      : 12;    ///<BIT [31:20] rsvd_0
    } b;
} UcducdObCmnSnglOutboundControlChannelAxiWriteBusAttrs_t;

/// @brief 0x350
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_Q_131_128_SOFT_ERR       : 4;     ///<BIT [3:0] ob_q_131_128_soft_err
        uint32_t RSVD_0                      : 28;    ///<BIT [31:4] rsvd_0
    } b;
} UcducdObCmnSnglOutboundQueueSoftError4_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OQ_EN                       : 1;     ///<BIT [0] oq_en
        uint32_t RSVD_1                      : 1;     ///<BIT [1] rsvd_1
        uint32_t OQ_PHS_BIT_EN               : 1;     ///<BIT [2] oq_phs_bit_en
        uint32_t OQ_RST                      : 1;     ///<BIT [3] oq_rst
        uint32_t OQ_ELMNT_SZ                 : 3;     ///<BIT [6:4] oq_elmnt_sz
        uint32_t OQ_PASS_THRU_MODE_EN        : 1;     ///<BIT [7] oq_pass_thru_mode_en
        uint32_t OQ_IQ_CI_UPDT_EN            : 1;     ///<BIT [8] oq_iq_ci_updt_en
        uint32_t OQ_OFFLINE                  : 1;     ///<BIT [9] oq_offline
        uint32_t OQ_IQ_ID_UPDT_EN            : 1;     ///<BIT [10] oq_iq_id_updt_en
        uint32_t RSVD_0                      : 5;     ///<BIT [15:11] rsvd_0
        uint32_t OQ_NM_ELMNTS                : 16;    ///<BIT [31:16] oq_nm_elmnts
    } b;
} UcducdObCmnOqOutboundQueueCfg0_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_1                      : 8;     ///<BIT [7:0] rsvd_1
        uint32_t OQ_IFC_SLCT                 : 8;     ///<BIT [15:8] oq_ifc_slct
        uint32_t RSVD_0                      : 16;    ///<BIT [31:16] rsvd_0
    } b;
} UcducdObCmnOqOutboundQueueCfg1_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OQ_MAX_COAL_TIME            : 16;    ///<BIT [15:0] oq_max_coal_time
        uint32_t OQ_MIN_COAL_TIME            : 16;    ///<BIT [31:16] oq_min_coal_time
    } b;
} UcducdObCmnOqOutboundQueueIntrCfg0_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OQ_INT_COAL_COUNT           : 16;    ///<BIT [15:0] oq_int_coal_count
        uint32_t OQ_EN_INT_COAL              : 1;     ///<BIT [16] oq_en_int_coal
        uint32_t OQ_WAIT_FOR_REARM           : 1;     ///<BIT [17] oq_wait_for_rearm
        uint32_t OQ_ENBL_CI_WRT_REARM        : 1;     ///<BIT [18] oq_enbl_ci_wrt_rearm
        uint32_t OQ_ENBL_CI_BIT_31_RESTRT_COALESC_TMR : 1;     ///<BIT [19] oq_enbl_ci_bit_31_restrt_coalesc_tmr
        uint32_t RSVD_0                      : 12;    ///<BIT [31:20] rsvd_0
    } b;
} UcducdObCmnOqOutboundQueueIntrCfg1_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OQ_MSI_X_VCTR_SLCT          : 5;     ///<BIT [4:0] oq_msi_x_vctr_slct
        uint32_t RSVD_1                      : 3;     ///<BIT [7:5] rsvd_1
        uint32_t OQ_MSI_X_TBL_SLCT           : 8;     ///<BIT [15:8] oq_msi_x_tbl_slct
        uint32_t RSVD_2                      : 14;    ///<BIT [29:16] rsvd_2
        uint32_t OQ_EN_GEN_MSI_X             : 1;     ///<BIT [30] oq_en_gen_msi_x
        uint32_t OQ_EN_EXTRNL_TMR_RSTRT      : 1;     ///<BIT [31] oq_en_extrnl_tmr_rstrt
    } b;
} UcducdObCmnOqOutboundQueueIntrCfg2_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      : 2;     ///<BIT [1:0] rsvd_0
        uint32_t OQ_BASE_ADDR_LO             : 30;    ///<BIT [31:2] oq_base_addr_lo
    } b;
} UcducdObCmnOqOutboundQueueBaseAddrLo_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OQ_PRDCR_INDX               : 16;    ///<BIT [15:0] oq_prdcr_indx
        uint32_t RSVD_0                      : 15;    ///<BIT [30:16] rsvd_0
        uint32_t OQ_PHASE                    : 1;     ///<BIT [31] oq_phase
    } b;
} UcducdObCmnOqOutboundQueuePi_t;

/// @brief 0x2C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OQ_CNSMR_INDX               : 16;    ///<BIT [15:0] oq_cnsmr_indx
        uint32_t RSVD_0                      : 14;    ///<BIT [29:16] rsvd_0
        uint32_t OQ_INTRPT_CLR               : 1;     ///<BIT [30] oq_intrpt_clr
        uint32_t OQ_RESTRT_COAL_TMR          : 1;     ///<BIT [31] oq_restrt_coal_tmr
    } b;
} UcducdObCmnOqOutboundQueueCi_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OQ_EMPTY                    : 1;     ///<BIT [0] oq_empty
        uint32_t OQ_FULL                     : 1;     ///<BIT [1] oq_full
        uint32_t OQ_OVRFLW                   : 1;     ///<BIT [2] oq_ovrflw
        uint32_t OQ_ELMNT_SKIPPED            : 1;     ///<BIT [3] oq_elmnt_skipped
        uint32_t OQ_STALLED                  : 1;     ///<BIT [4] oq_stalled
        uint32_t OQ_DBELL_WRITE_OUT_OF_RANGE_ERR : 1;     ///<BIT [5] oq_dbell_write_out_of_range_err
        uint32_t OQ_DBELL_WRITE_SAME_VALUE_ERR : 1;     ///<BIT [6] oq_dbell_write_same_value_err
        uint32_t OQ_DBELL_WRITE_RMV_FROM_EMPTY_CQ_ERR : 1;     ///<BIT [7] oq_dbell_write_rmv_from_empty_cq_err
        uint32_t OQ_PI_IRQ                   : 1;     ///<BIT [8] oq_pi_irq
        uint32_t RSVD_1                      : 1;     ///<BIT [9] rsvd_1
        uint32_t OQ_LKHD_EMPTY               : 1;     ///<BIT [10] oq_lkhd_empty
        uint32_t OQ_LKHD_FULL                : 1;     ///<BIT [11] oq_lkhd_full
        uint32_t RSVD_0                      : 4;     ///<BIT [15:12] rsvd_0
        uint32_t OQ_LKHD_PRDCR_INDX          : 16;    ///<BIT [31:16] oq_lkhd_prdcr_indx
    } b;
} UcducdObCmnOqOutboundQueueStatus_t;

/// @brief 0x5000
typedef struct
{
    UcducdObCmnOqOutboundQueueCfg0_t ucdObCmnOqOutboundQueueCfg0; //ucd_ob_cmn_oq_reg_outbound_queue_configuration_0
    UcducdObCmnOqOutboundQueueCfg1_t ucdObCmnOqOutboundQueueCfg1; //ucd_ob_cmn_oq_reg_outbound_queue_configuration_1
    UcducdObCmnOqOutboundQueueIntrCfg0_t ucdObCmnOqOutboundQueueIntrCfg0; //ucd_ob_cmn_oq_reg_outbound_queue_interrupt_configuration_0
    UcducdObCmnOqOutboundQueueIntrCfg1_t ucdObCmnOqOutboundQueueIntrCfg1; //ucd_ob_cmn_oq_reg_outbound_queue_interrupt_configuration_1
    UcducdObCmnOqOutboundQueueIntrCfg2_t ucdObCmnOqOutboundQueueIntrCfg2; //ucd_ob_cmn_oq_reg_outbound_queue_interrupt_configuration_2
    uint8_t rsvd14[4];  //rsvd_14 ///< 0x14-0x18
    uint32_t ucdObCmnOqOutboundQueuePiShadowBaseAddrLoRsvd0; //ucd_ob_cmn_oq_reg_outbound_queue_pi_shadow_base_addr_lo
    uint32_t ucdObCmnOqOutboundQueuePiShadowBaseAddrHiRsvd0; //ucd_ob_cmn_oq_reg_outbound_queue_pi_shadow_base_addr_hi
    UcducdObCmnOqOutboundQueueBaseAddrLo_t ucdObCmnOqOutboundQueueBaseAddrLo; //ucd_ob_cmn_oq_reg_outbound_queue_base_addr_lo
    uint32_t ucdObCmnOqOutboundQueueBaseAddrHiOqBaseAddrHi; //ucd_ob_cmn_oq_reg_outbound_queue_base_addr_hi
    UcducdObCmnOqOutboundQueuePi_t ucdObCmnOqOutboundQueuePi; //ucd_ob_cmn_oq_reg_outbound_queue_pi
    UcducdObCmnOqOutboundQueueCi_t ucdObCmnOqOutboundQueueCi; //ucd_ob_cmn_oq_reg_outbound_queue_ci
    UcducdObCmnOqOutboundQueueStatus_t ucdObCmnOqOutboundQueueStatus; //ucd_ob_cmn_oq_reg_outbound_queue_status
    uint8_t endPadding[12];               //end_padding ///< 0x34-0x40
} UcdCore1ObCmnOqRegisters_t;
static_assert(TYPE_OFFSET(UcdCore1ObCmnOqRegisters_t, ucdObCmnOqOutboundQueuePiShadowBaseAddrLoRsvd0) == 0x18, "check register structure offset 0x18");
static_assert(TYPE_OFFSET(UcdCore1ObCmnOqRegisters_t, ucdObCmnOqOutboundQueuePi) == 0x28, "check register structure offset 0x28");

/// @brief 0x120
typedef struct
{
    UcducdObCmnCqOutboundCompletionQueueCfgControl_t ucdObCmnCqOutboundCompletionQueueCfgControl; //ucd_ob_cmn_cq_reg_outbound_completion_queue_configuration_control
    UcducdObCmnCqOutboundCompletionQueueIntrCoalescing0_t ucdObCmnCqOutboundCompletionQueueIntrCoalescing0; //ucd_ob_cmn_cq_reg_outbound_completion_queue_interrupt_coalescing_0
    UcducdObCmnCqOutboundCompletionQueueIntrCoalescing1_t ucdObCmnCqOutboundCompletionQueueIntrCoalescing1; //ucd_ob_cmn_cq_reg_outbound_completion_queue_interrupt_coalescing_1
    UcducdObCmnCqOutboundCompletionQueuePiShadowBaseAddrLo_t ucdObCmnCqOutboundCompletionQueuePiShadowBaseAddrLo; //ucd_ob_cmn_cq_reg_outbound_completion_queue_pi_shadow_base_addr_lo
    uint32_t ucdObCmnCqOutboundCompletionQueuePiShadowBaseAddrHiObCmpltnQPiShdwBaseAddrHi; //ucd_ob_cmn_cq_reg_outbound_completion_queue_pi_shadow_base_addr_hi
    UcducdObCmnCqOutboundCompletionQueueBaseAddrLo_t ucdObCmnCqOutboundCompletionQueueBaseAddrLo; //ucd_ob_cmn_cq_reg_outbound_completion_queue_base_addr_lo
    uint32_t ucdObCmnCqOutboundCompletionQueueBaseAddrHiObCmpltnQBaseAddrHi; //ucd_ob_cmn_cq_reg_outbound_completion_queue_base_addr_hi
    UcducdObCmnCqOutboundCompletionQueuePi_t ucdObCmnCqOutboundCompletionQueuePi; //ucd_ob_cmn_cq_reg_outbound_completion_queue_pi
    UcducdObCmnCqOutboundCompletionQueueCi_t ucdObCmnCqOutboundCompletionQueueCi; //ucd_ob_cmn_cq_reg_outbound_completion_queue_ci
    UcducdObCmnCqOutboundCompletionQueueStatus_t ucdObCmnCqOutboundCompletionQueueStatus; //ucd_ob_cmn_cq_reg_outbound_completion_queue_status
    uint8_t endPadding[8];                //end_padding
} UcducdObCmnCqRegisters_t;
static_assert(TYPE_OFFSET(UcducdObCmnCqRegisters_t, ucdObCmnCqOutboundCompletionQueuePiShadowBaseAddrLo) == 0xc, "check register structure offset 0xc");
static_assert(TYPE_OFFSET(UcducdObCmnCqRegisters_t, ucdObCmnCqOutboundCompletionQueuePi) == 0x1c, "check register structure offset 0x1c");

/// @brief 0x70
typedef struct
{
    UcducdObCmnOslOutboundSourceListCfg0_t ucdObCmnOslOutboundSourceListCfg0; //ucd_ob_cmn_osl_reg_outbound_source_list_configuration_0
    uint8_t rsvd4[4];  //rsvd_4 //0x4-0x8
    UcducdObCmnOslOutboundSourceListBaseAddrLo_t ucdObCmnOslOutboundSourceListBaseAddrLo; //ucd_ob_cmn_osl_reg_outbound_source_list_base_addr_lo
    uint32_t ucdObCmnOslOutboundSourceListBaseAddrHiObSrcListBaseAddrHi; //ucd_ob_cmn_osl_reg_outbound_source_list_base_addr_hi
    UcducdObCmnOslOutboundSourceListPi_t ucdObCmnOslOutboundSourceListPi; //ucd_ob_cmn_osl_reg_outbound_source_list_pi
    UcducdObCmnOslOutboundSourceListCi_t ucdObCmnOslOutboundSourceListCi; //ucd_ob_cmn_osl_reg_outbound_source_list_ci
    UcducdObCmnOslOutboundSourceListStatus_t ucdObCmnOslOutboundSourceListStatus; //ucd_ob_cmn_osl_reg_outbound_source_list_status
    uint8_t endPadding[4];                //end_padding
} UcducdObCmnOslRegisters_t;
static_assert(TYPE_OFFSET(UcducdObCmnOslRegisters_t, ucdObCmnOslOutboundSourceListPi) == 0x10, "check register structure offset 0x10");

/// @brief 0x4000
typedef struct
{
    UcducdObCmnSnglOutboundUcdCfg_t ucdObCmnSnglOutboundUcdCfg; //ucd_ob_cmn_sngl_reg_outbound_ucd_configuration
    UcducdObCmnSnglOutboundUcdControl_t ucdObCmnSnglOutboundUcdControl; //ucd_ob_cmn_sngl_reg_outbound_ucd_control
    UcducdObCmnSnglOutboundUcdStatus_t ucdObCmnSnglOutboundUcdStatus; //ucd_ob_cmn_sngl_reg_outbound_ucd_status
    UcducdObCmnSnglOutboundUcdIntrCause_t ucdObCmnSnglOutboundUcdIntrCause; //ucd_ob_cmn_sngl_reg_outbound_ucd_interrupt_cause
    uint32_t ucdObCmnSnglOutboundUcdIntr0EnableSetObUcdIrq0EnblSet; //ucd_ob_cmn_sngl_reg_outbound_ucd_interrupt_0_enable_set
    uint32_t ucdObCmnSnglOutboundUcdIntr0EnableClearObUcdIrq0EnblClr; //ucd_ob_cmn_sngl_reg_outbound_ucd_interrupt_0_enable_clear
    uint32_t ucdObCmnSnglOutboundUcdIntr1EnableSetObUcdIrq1EnblSet; //ucd_ob_cmn_sngl_reg_outbound_ucd_interrupt_1_enable_set
    uint32_t ucdObCmnSnglOutboundUcdIntr1EnableClearObUcdIrq1EnblClr; //ucd_ob_cmn_sngl_reg_outbound_ucd_interrupt_1_enable_clear
    uint8_t rsvd20[16];                   //rsvd_20
    UcducdObCmnSnglOutboundUcdSramParityErrorCause_t ucdObCmnSnglOutboundUcdSramParityErrorCause; //ucd_ob_cmn_sngl_reg_outbound_ucd_sram_parity_error_cause
    UcducdObCmnSnglOutboundUcdSramParityErrorEnable_t ucdObCmnSnglOutboundUcdSramParityErrorEnable; //ucd_ob_cmn_sngl_reg_outbound_ucd_sram_parity_error_enable
    UcducdObCmnSnglOutboundUcdDataPathErrorControl_t ucdObCmnSnglOutboundUcdDataPathErrorControl; //ucd_ob_cmn_sngl_reg_outbound_ucd_data_path_error_control
    uint8_t rsvd3c[4];  //rsvd_3c ///< 0x3c-0x40
    UcducdObCmnSnglOutboundUcdDataPathErrorStatus_t ucdObCmnSnglOutboundUcdDataPathErrorStatus; //ucd_ob_cmn_sngl_reg_outbound_ucd_data_path_error_status
    uint8_t rsvd44[4];  //rsvd_44 ///< 0x44-0x48
    uint32_t ucdObCmnSnglOutboundUcdErrorAddressLowObErrAddrLow; //ucd_ob_cmn_sngl_reg_outbound_ucd_error_address_low
    uint32_t ucdObCmnSnglOutboundUcdErrorAddressHighObErrAddrHigh; //ucd_ob_cmn_sngl_reg_outbound_ucd_error_address_high
    UcducdObCmnSnglOutboundQueueArbitrationCfgRegister_t ucdObCmnSnglOutboundQueueArbitrationCfgRegister; //ucd_ob_cmn_sngl_reg_outbound_queue_arbitration_configuration_register
    uint8_t rsvd54[12];  //rsvd_54 ///< 0x54-0x60
    UcducdObCmnSnglOutboundSizeSelect0_t ucdObCmnSnglOutboundSizeSelect0; //ucd_ob_cmn_sngl_reg_outbound_size_select_0
    UcducdObCmnSnglOutboundSizeSelect1_t ucdObCmnSnglOutboundSizeSelect1; //ucd_ob_cmn_sngl_reg_outbound_size_select_1
    UcducdObCmnSnglOutboundSizeSelect2_t ucdObCmnSnglOutboundSizeSelect2; //ucd_ob_cmn_sngl_reg_outbound_size_select_2
    UcducdObCmnSnglOutboundSizeSelect3_t ucdObCmnSnglOutboundSizeSelect3; //ucd_ob_cmn_sngl_reg_outbound_size_select_3
    UcducdObCmnOslRegisters_t ucdObCmnOslRegisters[5]; //ucd_ob_cmn_osl_registers ///< 0x70-0x110
    uint8_t rsvd110[16];  //rsvd_110 ///< 0x110-0x120
    UcducdObCmnCqRegisters_t ucdObCmnCqRegisters[5]; //ucd_ob_cmn_cq_registers ///< 0x120-0x210
    uint8_t rsvd210[240];  //rsvd_210 ///< 0x210-0x300
    UcducdObCmnSnglOutboundMiscellaneousControl_t ucdObCmnSnglOutboundMiscellaneousControl; //ucd_ob_cmn_sngl_reg_outbound_miscellaneous_control
    uint32_t ucdObCmnSnglOutboundControlPathErrorStatusCpErrorStatus; //ucd_ob_cmn_sngl_reg_outbound_control_path_error_status
    uint32_t ucdObCmnSnglOutboundDatapathHaltErrorStatusDpathHaltErrorStatus; //ucd_ob_cmn_sngl_reg_outbound_datapath_halt_error_status
    uint32_t ucdObCmnSnglOutboundDatapathHaltErrorMaskDpathHaltErrorMask; //ucd_ob_cmn_sngl_reg_outbound_datapath_halt_error_mask
    UcducdObCmnSnglOutboundDiagnosticControl_t ucdObCmnSnglOutboundDiagnosticControl; //ucd_ob_cmn_sngl_reg_outbound_diagnostic_control
    uint32_t ucdObCmnSnglOutboundDiagnosticReadPortDiagMiscStatus; //ucd_ob_cmn_sngl_reg_outbound_diagnostic_read_port
    uint8_t rsvd318[8];   //rsvd_318 ///< 0x318-0x320
    UcducdObCmnSnglOutboundDataChannelAxiReadBusAttrs_t ucdObCmnSnglOutboundDataChannelAxiReadBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_data_channel_axi_read_bus_attributes
    UcducdObCmnSnglOutboundDataChannelAxiWriteBusAttrs_t ucdObCmnSnglOutboundDataChannelAxiWriteBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_data_channel_axi_write_bus_attributes
    UcducdObCmnSnglOutboundControlChannelAxiReadBusAttrs_t ucdObCmnSnglOutboundControlChannelAxiReadBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_control_channel_axi_read_bus_attributes
    UcducdObCmnSnglOutboundControlChannelAxiWriteBusAttrs_t ucdObCmnSnglOutboundControlChannelAxiWriteBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_control_channel_axi_write_bus_attributes ///< 0x32c-0x330
    uint8_t rsvd330[16];  //rsvd_330 ///< 0x330 - 0x340
    uint32_t ucdObCmnSnglOutboundQueueSoftError0ObQ310SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_0
    uint32_t ucdObCmnSnglOutboundQueueSoftError1ObQ6332SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_1
    uint32_t ucdObCmnSnglOutboundQueueSoftError2ObQ9564SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_2
    uint32_t ucdObCmnSnglOutboundQueueSoftError3ObQ12796SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_3
    UcducdObCmnSnglOutboundQueueSoftError4_t ucdObCmnSnglOutboundQueueSoftError4; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_4
} UcdCore1ObCmnRegisters_t;
static_assert(TYPE_OFFSET(UcdCore1ObCmnRegisters_t, ucdObCmnOslRegisters) == 0x70, "check register structure offset 0x70");
static_assert(TYPE_OFFSET(UcdCore1ObCmnRegisters_t, ucdObCmnCqRegisters) == 0x120, "check register structure offset 0x120");

/// @brief 0x1000
typedef struct
{
    UcducdObCmnOqOutboundQueueCfg0_t ucdObCmnOqOutboundQueueCfg0; //ucd_ob_cmn_oq_reg_outbound_queue_configuration_0
    UcducdObCmnOqOutboundQueueCfg1_t ucdObCmnOqOutboundQueueCfg1; //ucd_ob_cmn_oq_reg_outbound_queue_configuration_1
    UcducdObCmnOqOutboundQueueIntrCfg0_t ucdObCmnOqOutboundQueueIntrCfg0; //ucd_ob_cmn_oq_reg_outbound_queue_interrupt_configuration_0
    UcducdObCmnOqOutboundQueueIntrCfg1_t ucdObCmnOqOutboundQueueIntrCfg1; //ucd_ob_cmn_oq_reg_outbound_queue_interrupt_configuration_1
    UcducdObCmnOqOutboundQueueIntrCfg2_t ucdObCmnOqOutboundQueueIntrCfg2; //ucd_ob_cmn_oq_reg_outbound_queue_interrupt_configuration_2
    uint8_t rsvd14[4]; //rsvd_14 ///< 0x14-0x18
    uint32_t ucdObCmnOqOutboundQueuePiShadowBaseAddrLoRsvd0; //ucd_ob_cmn_oq_reg_outbound_queue_pi_shadow_base_addr_lo
    uint32_t ucdObCmnOqOutboundQueuePiShadowBaseAddrHiRsvd0; //ucd_ob_cmn_oq_reg_outbound_queue_pi_shadow_base_addr_hi
    UcducdObCmnOqOutboundQueueBaseAddrLo_t ucdObCmnOqOutboundQueueBaseAddrLo; //ucd_ob_cmn_oq_reg_outbound_queue_base_addr_lo
    uint32_t ucdObCmnOqOutboundQueueBaseAddrHiOqBaseAddrHi; //ucd_ob_cmn_oq_reg_outbound_queue_base_addr_hi
    UcducdObCmnOqOutboundQueuePi_t ucdObCmnOqOutboundQueuePi; //ucd_ob_cmn_oq_reg_outbound_queue_pi
    UcducdObCmnOqOutboundQueueCi_t ucdObCmnOqOutboundQueueCi; //ucd_ob_cmn_oq_reg_outbound_queue_ci
    UcducdObCmnOqOutboundQueueStatus_t ucdObCmnOqOutboundQueueStatus; //ucd_ob_cmn_oq_reg_outbound_queue_status
    uint8_t endPadding[12];               //end_padding ///< 0x34-0x40
} UcdCore0ObCmnOqRegisters_t;
static_assert(TYPE_OFFSET(UcdCore0ObCmnOqRegisters_t, ucdObCmnOqOutboundQueuePiShadowBaseAddrLoRsvd0) == 0x18, "check register structure offset 0x18");
static_assert(TYPE_OFFSET(UcdCore0ObCmnOqRegisters_t, ucdObCmnOqOutboundQueuePi) == 0x28, "check register structure offset 0x28");

/// @brief 0x0
typedef struct
{
    UcducdObCmnSnglOutboundUcdCfg_t ucdObCmnSnglOutboundUcdCfg; //ucd_ob_cmn_sngl_reg_outbound_ucd_configuration
    UcducdObCmnSnglOutboundUcdControl_t ucdObCmnSnglOutboundUcdControl; //ucd_ob_cmn_sngl_reg_outbound_ucd_control
    UcducdObCmnSnglOutboundUcdStatus_t ucdObCmnSnglOutboundUcdStatus; //ucd_ob_cmn_sngl_reg_outbound_ucd_status
    UcducdObCmnSnglOutboundUcdIntrCause_t ucdObCmnSnglOutboundUcdIntrCause; //ucd_ob_cmn_sngl_reg_outbound_ucd_interrupt_cause
    uint32_t ucdObCmnSnglOutboundUcdIntr0EnableSetObUcdIrq0EnblSet; //ucd_ob_cmn_sngl_reg_outbound_ucd_interrupt_0_enable_set
    uint32_t ucdObCmnSnglOutboundUcdIntr0EnableClearObUcdIrq0EnblClr; //ucd_ob_cmn_sngl_reg_outbound_ucd_interrupt_0_enable_clear
    uint32_t ucdObCmnSnglOutboundUcdIntr1EnableSetObUcdIrq1EnblSet; //ucd_ob_cmn_sngl_reg_outbound_ucd_interrupt_1_enable_set
    uint32_t ucdObCmnSnglOutboundUcdIntr1EnableClearObUcdIrq1EnblClr; //ucd_ob_cmn_sngl_reg_outbound_ucd_interrupt_1_enable_clear
    uint8_t rsvd20[16];                   //rsvd_20
    UcducdObCmnSnglOutboundUcdSramParityErrorCause_t ucdObCmnSnglOutboundUcdSramParityErrorCause; //ucd_ob_cmn_sngl_reg_outbound_ucd_sram_parity_error_cause
    UcducdObCmnSnglOutboundUcdSramParityErrorEnable_t ucdObCmnSnglOutboundUcdSramParityErrorEnable; //ucd_ob_cmn_sngl_reg_outbound_ucd_sram_parity_error_enable
    UcducdObCmnSnglOutboundUcdDataPathErrorControl_t ucdObCmnSnglOutboundUcdDataPathErrorControl; //ucd_ob_cmn_sngl_reg_outbound_ucd_data_path_error_control
    uint8_t rsvd3c[4];  //rsvd_3c ///< 0x3c-0x40
    UcducdObCmnSnglOutboundUcdDataPathErrorStatus_t ucdObCmnSnglOutboundUcdDataPathErrorStatus; //ucd_ob_cmn_sngl_reg_outbound_ucd_data_path_error_status
    uint8_t rsvd44[4];  //rsvd_44 ///< 0x44-0x48
    uint32_t ucdObCmnSnglOutboundUcdErrorAddressLowObErrAddrLow; //ucd_ob_cmn_sngl_reg_outbound_ucd_error_address_low
    uint32_t ucdObCmnSnglOutboundUcdErrorAddressHighObErrAddrHigh; //ucd_ob_cmn_sngl_reg_outbound_ucd_error_address_high
    UcducdObCmnSnglOutboundQueueArbitrationCfgRegister_t ucdObCmnSnglOutboundQueueArbitrationCfgRegister; //ucd_ob_cmn_sngl_reg_outbound_queue_arbitration_configuration_register
    uint8_t rsvd54[12];  //rsvd_54 ///< 0x54-0x60
    UcducdObCmnSnglOutboundSizeSelect0_t ucdObCmnSnglOutboundSizeSelect0; //ucd_ob_cmn_sngl_reg_outbound_size_select_0
    UcducdObCmnSnglOutboundSizeSelect1_t ucdObCmnSnglOutboundSizeSelect1; //ucd_ob_cmn_sngl_reg_outbound_size_select_1
    UcducdObCmnSnglOutboundSizeSelect2_t ucdObCmnSnglOutboundSizeSelect2; //ucd_ob_cmn_sngl_reg_outbound_size_select_2
    UcducdObCmnSnglOutboundSizeSelect3_t ucdObCmnSnglOutboundSizeSelect3; //ucd_ob_cmn_sngl_reg_outbound_size_select_3 ///< 0x6c-70
    UcducdObCmnOslRegisters_t ucdObCmnOslRegisters[5]; //ucd_ob_cmn_osl_registers ///< 0x70-110
    uint8_t rsvd110[16];  //rsvd_110 ///< 0x110-0x120
    UcducdObCmnCqRegisters_t ucdObCmnCqRegisters[5]; //ucd_ob_cmn_cq_registers ///< 0x120-0x210
    uint8_t rsvd210[240]; //rsvd_210  ///< 0x210-0x300
    UcducdObCmnSnglOutboundMiscellaneousControl_t ucdObCmnSnglOutboundMiscellaneousControl; //ucd_ob_cmn_sngl_reg_outbound_miscellaneous_control
    uint32_t ucdObCmnSnglOutboundControlPathErrorStatusCpErrorStatus; //ucd_ob_cmn_sngl_reg_outbound_control_path_error_status
    uint32_t ucdObCmnSnglOutboundDatapathHaltErrorStatusDpathHaltErrorStatus; //ucd_ob_cmn_sngl_reg_outbound_datapath_halt_error_status
    uint32_t ucdObCmnSnglOutboundDatapathHaltErrorMaskDpathHaltErrorMask; //ucd_ob_cmn_sngl_reg_outbound_datapath_halt_error_mask
    UcducdObCmnSnglOutboundDiagnosticControl_t ucdObCmnSnglOutboundDiagnosticControl; //ucd_ob_cmn_sngl_reg_outbound_diagnostic_control
    uint32_t ucdObCmnSnglOutboundDiagnosticReadPortDiagMiscStatus; //ucd_ob_cmn_sngl_reg_outbound_diagnostic_read_port
    uint8_t rsvd318[8];  //rsvd_318 ///< 0x318-0x320
    UcducdObCmnSnglOutboundDataChannelAxiReadBusAttrs_t ucdObCmnSnglOutboundDataChannelAxiReadBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_data_channel_axi_read_bus_attributes
    UcducdObCmnSnglOutboundDataChannelAxiWriteBusAttrs_t ucdObCmnSnglOutboundDataChannelAxiWriteBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_data_channel_axi_write_bus_attributes
    UcducdObCmnSnglOutboundControlChannelAxiReadBusAttrs_t ucdObCmnSnglOutboundControlChannelAxiReadBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_control_channel_axi_read_bus_attributes
    UcducdObCmnSnglOutboundControlChannelAxiWriteBusAttrs_t ucdObCmnSnglOutboundControlChannelAxiWriteBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_control_channel_axi_write_bus_attributes ///< 0x32c-0x330
    uint8_t rsvd330[16];                  //rsvd_330 ///< 0x330 - 0x340
    uint32_t ucdObCmnSnglOutboundQueueSoftError0ObQ310SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_0
    uint32_t ucdObCmnSnglOutboundQueueSoftError1ObQ6332SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_1
    uint32_t ucdObCmnSnglOutboundQueueSoftError2ObQ9564SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_2
    uint32_t ucdObCmnSnglOutboundQueueSoftError3ObQ12796SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_3
    UcducdObCmnSnglOutboundQueueSoftError4_t ucdObCmnSnglOutboundQueueSoftError4; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_4
} UcdCore0ObCmnRegisters_t;
static_assert(TYPE_OFFSET(UcdCore0ObCmnRegisters_t, ucdObCmnOslRegisters) == 0x70, "check register structure offset 0x70");
static_assert(TYPE_OFFSET(UcdCore0ObCmnRegisters_t, ucdObCmnCqRegisters) == 0x120, "check register structure offset 0x120");

/// @brief 0x1C0000
typedef struct
{
    UcdCore0ObCmnRegisters_t ucdCore0ObCmnRegisters; //ucd_core0_ob_cmn_registers
    uint8_t rsvd354[3244]; //rsvd_354 ///< 0x354 - 0x1000
    UcdCore0ObCmnOqRegisters_t ucdCore0ObCmnOqRegisters[132]; //ucd_core0_ob_cmn_oq_registers
    uint8_t rsvd3100[3840]; //rsvd_3100 ///< 0x3100 - 0x4000
    UcdCore1ObCmnRegisters_t ucdCore1ObCmnRegisters; //ucd_core1_ob_cmn_registers
    uint8_t rsvd4354[3244]; //rsvd_4354 ///< 0x4354 - 0x5000
    UcdCore1ObCmnOqRegisters_t ucdCore1ObCmnOqRegisters[132]; //ucd_core1_ob_cmn_oq_registers
    //uint8_t rsvd7100to8000[3840]; ///< 0x7100 - 0x8000
} UcdOutboundRegisters_t;
#if 1
static_assert(TYPE_OFFSET(UcdOutboundRegisters_t, ucdCore0ObCmnRegisters) == 0x0, "check register structure offset 0x0");
static_assert(TYPE_OFFSET(UcdOutboundRegisters_t, ucdCore0ObCmnOqRegisters) == 0x1000, "check register structure offset 0x1000");
static_assert(TYPE_OFFSET(UcdOutboundRegisters_t, ucdCore1ObCmnRegisters) == 0x4000, "check register structure offset 0x4000");
static_assert(TYPE_OFFSET(UcdOutboundRegisters_t, ucdCore1ObCmnOqRegisters) == 0x5000, "check register structure offset 0x5000");
#else
COMPILE_ASSERT(TYPE_OFFSET(UcdOutboundRegisters_t, ucdCore1ObCmnRegisters) == 0x4000, "check register structure offset 0x4000");
#endif

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
