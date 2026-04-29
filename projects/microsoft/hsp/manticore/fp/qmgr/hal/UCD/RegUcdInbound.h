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
        uint32_t IB_UCD_RST                  : 1;     ///<BIT [31] ib_ucd_rst
    } b;
} UcducdIbCmnSnglInboundUcdCfg_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_UCD_ENBL                 : 1;     ///<BIT [0] ib_ucd_enbl
        uint32_t IB_UCD_PAUSE                : 1;     ///<BIT [1] ib_ucd_pause
        uint32_t RSVD_0                      : 30;    ///<BIT [31:2] rsvd_0
    } b;
} UcducdIbCmnSnglInboundUcdControl_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_UCD_ENBLD                : 1;     ///<BIT [0] ib_ucd_enbld
        uint32_t IB_UCD_PAUSED               : 1;     ///<BIT [1] ib_ucd_paused
        uint32_t IB_UCD_HLTD                 : 1;     ///<BIT [2] ib_ucd_hltd
        uint32_t RSVD_2                      : 5;     ///<BIT [7:3] rsvd_2
        uint32_t IB_UCD_PAUSED_RSN_TXN_SM    : 1;     ///<BIT [8] ib_ucd_paused_rsn_txn_sm
        uint32_t IB_UCD_PAUSED_RSN_CQ_FULL   : 1;     ///<BIT [9] ib_ucd_paused_rsn_cq_full
        uint32_t RSVD_1                      : 2;     ///<BIT [11:10] rsvd_1
        uint32_t IB_UCD_BUSY                 : 1;     ///<BIT [12] ib_ucd_busy
        uint32_t RSVD_0                      : 19;    ///<BIT [31:13] rsvd_0
    } b;
} UcducdIbCmnSnglInboundUcdStatus_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FP1_CMPLTN_Q_IRQ            : 1;     ///<BIT [0] fp1_cmpltn_q_irq
        uint32_t FP2_CMPLTN_Q_IRQ            : 1;     ///<BIT [1] fp2_cmpltn_q_irq
        uint32_t RR1_CMPLTN_Q_IRQ            : 1;     ///<BIT [2] rr1_cmpltn_q_irq
        uint32_t RR2_CMPLTN_Q_IRQ            : 1;     ///<BIT [3] rr2_cmpltn_q_irq
        uint32_t RR3_CMPLTN_Q_IRQ            : 1;     ///<BIT [4] rr3_cmpltn_q_irq
        uint32_t RSVD_0                      : 2;     ///<BIT [6:5] rsvd_0
        uint32_t IB_INTRNL_HW_ERR            : 1;     ///<BIT [7] ib_intrnl_hw_err
        uint32_t FP1_CMPLTN_Q_FULL_IRQ       : 1;     ///<BIT [8] fp1_cmpltn_q_full_irq
        uint32_t FP2_CMPLTN_Q_FULL_IRQ       : 1;     ///<BIT [9] fp2_cmpltn_q_full_irq
        uint32_t RR1_CMPLTN_Q_FULL_IRQ       : 1;     ///<BIT [10] rr1_cmpltn_q_full_irq
        uint32_t RR2_CMPLTN_Q_FULL_IRQ       : 1;     ///<BIT [11] rr2_cmpltn_q_full_irq
        uint32_t RR3_CMPLTN_Q_FULL_IRQ       : 1;     ///<BIT [12] rr3_cmpltn_q_full_irq
        uint32_t IB_Q_SOFT_ERR_IRQ           : 1;     ///<BIT [13] ib_q_soft_err_irq
        uint32_t RSVD_1                      : 3;     ///<BIT [16:14] rsvd_1
        uint32_t IB_DEST_FREE_LIST_0_EMPTY_IRQ : 1;     ///<BIT [17] ib_dest_free_list_0_empty_irq
        uint32_t IB_DEST_FREE_LIST_1_EMPTY_IRQ : 1;     ///<BIT [18] ib_dest_free_list_1_empty_irq
        uint32_t IB_DEST_FREE_LIST_2_EMPTY_IRQ : 1;     ///<BIT [19] ib_dest_free_list_2_empty_irq
        uint32_t IB_DEST_FREE_LIST_3_EMPTY_IRQ : 1;     ///<BIT [20] ib_dest_free_list_3_empty_irq
        uint32_t IB_DEST_FREE_LIST_4_EMPTY_IRQ : 1;     ///<BIT [21] ib_dest_free_list_4_empty_irq
        uint32_t IB_DEST_FREE_LIST_5_EMPTY_IRQ : 1;     ///<BIT [22] ib_dest_free_list_5_empty_irq
        uint32_t IB_DEST_FREE_LIST_0_OVRFLW_IRQ : 1;     ///<BIT [23] ib_dest_free_list_0_ovrflw_irq
        uint32_t IB_DEST_FREE_LIST_1_OVRFLW_IRQ : 1;     ///<BIT [24] ib_dest_free_list_1_ovrflw_irq
        uint32_t IB_DEST_FREE_LIST_2_OVRFLW_IRQ : 1;     ///<BIT [25] ib_dest_free_list_2_ovrflw_irq
        uint32_t IB_DEST_FREE_LIST_3_OVRFLW_IRQ : 1;     ///<BIT [26] ib_dest_free_list_3_ovrflw_irq
        uint32_t IB_DEST_FREE_LIST_4_OVRFLW_IRQ : 1;     ///<BIT [27] ib_dest_free_list_4_ovrflw_irq
        uint32_t IB_DEST_FREE_LIST_5_OVRFLW_IRQ : 1;     ///<BIT [28] ib_dest_free_list_5_ovrflw_irq
        uint32_t IB_CONTROL_PATH_ERR         : 1;     ///<BIT [29] ib_control_path_err
        uint32_t IB_INTRNL_MEM_PERR          : 1;     ///<BIT [30] ib_intrnl_mem_perr
        uint32_t IB_DATA_PATH_ERR            : 1;     ///<BIT [31] ib_data_path_err
    } b;
} UcducdIbCmnSnglInboundUcdIntrCause_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_UCD_SRAM_PERR            : 1;     ///<BIT [0] ib_ucd_sram_perr
        uint32_t RSVD_0                      : 31;    ///<BIT [31:1] rsvd_0
    } b;
} UcducdIbCmnSnglInboundUcdSramParityErrorCause_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_UCD_SRAM_PERR_EN         : 1;     ///<BIT [0] ib_ucd_sram_perr_en
        uint32_t RSVD_0                      : 31;    ///<BIT [31:1] rsvd_0
    } b;
} UcducdIbCmnSnglInboundUcdSramParityErrorEnable_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_DP_PAR                   : 1;     ///<BIT [0] ib_dp_par
        uint32_t IB_DP_PERR_EN               : 1;     ///<BIT [1] ib_dp_perr_en
        uint32_t IB_DP_FERR_EN               : 1;     ///<BIT [2] ib_dp_ferr_en
        uint32_t IB_DP_RD_TXN_ERR_DSBL       : 3;     ///<BIT [5:3] ib_dp_rd_txn_err_dsbl
        uint32_t IB_DP_WR_TXN_ERR_DSBL       : 2;     ///<BIT [7:6] ib_dp_wr_txn_err_dsbl
        uint32_t RSVD_0                      : 6;     ///<BIT [13:8] rsvd_0
        uint32_t IB_FRC_DP_PERR_CONT         : 1;     ///<BIT [14] ib_frc_dp_perr_cont
        uint32_t IB_FRC_DP_PERR_ONCE         : 1;     ///<BIT [15] ib_frc_dp_perr_once
        uint32_t IB_DP_PRTY_MASK             : 16;    ///<BIT [31:16] ib_dp_prty_mask
    } b;
} UcducdIbCmnSnglInboundUcdDataPathErrorControl_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_1                      : 1;     ///<BIT [0] rsvd_1
        uint32_t IB_DP_PERR                  : 1;     ///<BIT [1] ib_dp_perr
        uint32_t IB_DP_FERR                  : 1;     ///<BIT [2] ib_dp_ferr
        uint32_t IB_DP_RD_TXN_ERR            : 3;     ///<BIT [5:3] ib_dp_rd_txn_err
        uint32_t IB_DP_WR_TXN_ERR            : 2;     ///<BIT [7:6] ib_dp_wr_txn_err
        uint32_t RSVD_0                      : 16;    ///<BIT [23:8] rsvd_0
        uint32_t IB_ERR_PORT                 : 8;     ///<BIT [31:24] ib_err_port
    } b;
} UcducdIbCmnSnglInboundUcdDataPathErrorStatus_t;

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
} UcducdIbCmnSnglInboundQueueArbitrationCfgRegister_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_DEST_FREE_LIST_EN        : 1;     ///<BIT [0] ib_dest_free_list_en
        uint32_t RSVD_2                      : 7;     ///<BIT [7:1] rsvd_2
        uint32_t IB_DEST_FREE_LIST_SIZE      : 4;     ///<BIT [11:8] ib_dest_free_list_size
        uint32_t RSVD_1                      : 4;     ///<BIT [15:12] rsvd_1
        uint32_t IB_DEST_FREE_LIST_BFFR_LNGTH : 12;    ///<BIT [27:16] ib_dest_free_list_bffr_lngth
        uint32_t RSVD_0                      : 4;     ///<BIT [31:28] rsvd_0
    } b;
} UcducdIbCmnDflInboundDestinationFreeListCfg0_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_DEST_FREE_LIST_BFFR_IFC_SLCT : 8;     ///<BIT [7:0] ib_dest_free_list_bffr_ifc_slct
        uint32_t IB_DEST_FREE_LIST_LIST_IFC_SLCT : 8;     ///<BIT [15:8] ib_dest_free_list_list_ifc_slct
        uint32_t RSVD_0                      : 16;    ///<BIT [31:16] rsvd_0
    } b;
} UcducdIbCmnDflInboundDestinationFreeListCfg1_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      : 3;     ///<BIT [2:0] rsvd_0
        uint32_t IB_DEST_FREE_LIST_BASE_ADDR_LO : 29;    ///<BIT [31:3] ib_dest_free_list_base_addr_lo
    } b;
} UcducdIbCmnDflInboundDestinationFreeListBaseAddrLo_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_DEST_FREE_LIST_PI        : 16;    ///<BIT [15:0] ib_dest_free_list_pi
        uint32_t RSVD_0                      : 16;    ///<BIT [31:16] rsvd_0
    } b;
} UcducdIbCmnDflInboundDestinationFreeListPi_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_DEST_FREE_LIST_CI        : 16;    ///<BIT [15:0] ib_dest_free_list_ci
        uint32_t RSVD_0                      : 16;    ///<BIT [31:16] rsvd_0
    } b;
} UcducdIbCmnDflInboundDestinationFreeListCi_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_DEST_FREE_LIST_EMPTY     : 1;     ///<BIT [0] ib_dest_free_list_empty
        uint32_t IB_DEST_FREE_LIST_FULL      : 1;     ///<BIT [1] ib_dest_free_list_full
        uint32_t IB_DEST_FREE_LIST_OVRFLW    : 1;     ///<BIT [2] ib_dest_free_list_ovrflw
        uint32_t RSVD_1                      : 5;     ///<BIT [7:3] rsvd_1
        uint32_t IB_DEST_FREE_LIST_MEM_EMPTY : 1;     ///<BIT [8] ib_dest_free_list_mem_empty
        uint32_t IB_DEST_FREE_LIST_FIFO_EMPTY : 1;     ///<BIT [9] ib_dest_free_list_fifo_empty
        uint32_t RSVD_0                      : 22;    ///<BIT [31:10] rsvd_0
    } b;
} UcducdIbCmnDflInboundDestinationFreeListStatus_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q_EN                 : 1;     ///<BIT [0] cmpltn_q_en
        uint32_t CMPLTN_Q_SHDW_EN            : 1;     ///<BIT [1] cmpltn_q_shdw_en
        uint32_t RSVD_2                      : 6;     ///<BIT [7:2] rsvd_2
        uint32_t CMPLTN_Q_SIZE               : 4;     ///<BIT [11:8] cmpltn_q_size
        uint32_t RSVD_1                      : 4;     ///<BIT [15:12] rsvd_1
        uint32_t CMPLTN_Q_IFC_SLCT           : 8;     ///<BIT [23:16] cmpltn_q_ifc_slct
        uint32_t RSVD_0                      : 8;     ///<BIT [31:24] rsvd_0
    } b;
} UcducdIbCmnCqCompletionQueueCfgControl_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q_MAX_COAL_TIME      : 16;    ///<BIT [15:0] cmpltn_q_max_coal_time
        uint32_t CMPLTN_Q_MIN_COAL_TIME      : 16;    ///<BIT [31:16] cmpltn_q_min_coal_time
    } b;
} UcducdIbCmnCqCompletionQueueIntrCoalescing0_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q_INT_COAL_COUNT     : 16;    ///<BIT [15:0] cmpltn_q_int_coal_count
        uint32_t CMPLTN_Q_EN_INT_COAL        : 1;     ///<BIT [16] cmpltn_q_en_int_coal
        uint32_t RSVD_2                      : 1;     ///<BIT [17] rsvd_2
        uint32_t CMPLTN_Q_RESTART_WHEN_CI_UPDT : 1;     ///<BIT [18] cmpltn_q_restart_when_ci_updt
        uint32_t RSVD_1                      : 1;     ///<BIT [19] rsvd_1
        uint32_t RSVD_0                      : 12;    ///<BIT [31:20] rsvd_0
    } b;
} UcducdIbCmnCqCompletionQueueIntrCoalescing1_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      : 2;     ///<BIT [1:0] rsvd_0
        uint32_t CMPLTN_Q_PI_SHDW_BASE_ADDR_LO : 30;    ///<BIT [31:2] cmpltn_q_pi_shdw_base_addr_lo
    } b;
} UcducdIbCmnCqCompletionQueuePiShadowBaseAddrLo_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      : 4;     ///<BIT [3:0] rsvd_0
        uint32_t CMPLTN_Q_BASE_ADDR_LO       : 28;    ///<BIT [31:4] cmpltn_q_base_addr_lo
    } b;
} UcducdIbCmnCqCompletionQueueBaseAddrLo_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q_PI                 : 16;    ///<BIT [15:0] cmpltn_q_pi
        uint32_t RSVD_0                      : 16;    ///<BIT [31:16] rsvd_0
    } b;
} UcducdIbCmnCqCompletionQueuePi_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q_CI                 : 16;    ///<BIT [15:0] cmpltn_q_ci
        uint32_t RSVD_1                      : 14;    ///<BIT [29:16] rsvd_1
        uint32_t INTRPT_CLR                  : 1;     ///<BIT [30] intrpt_clr
        uint32_t RSVD_0                      : 1;     ///<BIT [31] rsvd_0
    } b;
} UcducdIbCmnCqCompletionQueueCi_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q_EMPTY              : 1;     ///<BIT [0] cmpltn_q_empty
        uint32_t CMPLTN_Q_FULL               : 1;     ///<BIT [1] cmpltn_q_full
        uint32_t RSVD_1                      : 6;     ///<BIT [7:2] rsvd_1
        uint32_t CMPLTN_Q_CS_LKHD_EMPTY      : 1;     ///<BIT [8] cmpltn_q_cs_lkhd_empty
        uint32_t CMPLTN_Q_CS_LKHD_FULL       : 1;     ///<BIT [9] cmpltn_q_cs_lkhd_full
        uint32_t CMPLTN_Q_CS_FULL            : 1;     ///<BIT [10] cmpltn_q_cs_full
        uint32_t RSVD_0                      : 5;     ///<BIT [15:11] rsvd_0
        uint32_t CMPLTN_Q_LKHD_PI            : 16;    ///<BIT [31:16] cmpltn_q_lkhd_pi
    } b;
} UcducdIbCmnCqCompletionQueueStatus_t;

/// @brief 0x300
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DATA_FIFO_MEM_RF2P_RTC      : 2;     ///<BIT [1:0] data_fifo_mem_rf2p_rtc
        uint32_t DATA_FIFO_MEM_RF2P_WTC      : 2;     ///<BIT [3:2] data_fifo_mem_rf2p_wtc
        uint32_t DFL_NOT_EMPTY_WAIT_TMR_SEL  : 3;     ///<BIT [6:4] dfl_not_empty_wait_tmr_sel
        uint32_t CQ_NOT_FULL_WAIT_TMR_SEL    : 3;     ///<BIT [9:7] cq_not_full_wait_tmr_sel
        uint32_t ENBL_DFL_EMPTY_PAUSE        : 1;     ///<BIT [10] enbl_dfl_empty_pause
        uint32_t ENBL_CQ_FULL_PAUSE          : 1;     ///<BIT [11] enbl_cq_full_pause
        uint32_t DSBL_CQ_CLR_CQ_UPDT         : 1;     ///<BIT [12] dsbl_cq_clr_cq_updt
        uint32_t ENBL_CQ_CLR_CQ_EMPTY        : 1;     ///<BIT [13] enbl_cq_clr_cq_empty
        uint32_t RSVD_0                      : 6;     ///<BIT [19:14] rsvd_0
        uint32_t DSBL_AXI_ERR_PROPAGATION    : 1;     ///<BIT [20] dsbl_axi_err_propagation
        uint32_t DP_DIAG_HALT                : 1;     ///<BIT [21] dp_diag_halt
        uint32_t ENBL_FSC_SM_SYNC            : 1;     ///<BIT [22] enbl_fsc_sm_sync
        uint32_t ENBL_Q_ACC_WHILE_DSBLD_ERR  : 1;     ///<BIT [23] enbl_q_acc_while_dsbld_err
        uint32_t DSBL_DBELL_COMPLIANCE_CHK_ERR : 3;     ///<BIT [26:24] dsbl_dbell_compliance_chk_err
        uint32_t RSVD_1                      : 4;     ///<BIT [30:27] rsvd_1
        uint32_t MISC_ERROR_STATUS_CLR       : 1;     ///<BIT [31] misc_error_status_clr
    } b;
} UcducdIbCmnSnglInboundMiscellaneousControl_t;

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
} UcducdIbCmnSnglInboundDiagnosticControl_t;

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
} UcducdIbCmnSnglInboundDataChannelAxiReadBusAttrs_t;

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
} UcducdIbCmnSnglInboundDataChannelAxiWriteBusAttrs_t;

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
} UcducdIbCmnSnglInboundControlChannelAxiReadBusAttrs_t;

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
} UcducdIbCmnSnglInboundControlChannelAxiWriteBusAttrs_t;

/// @brief 0x350
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_Q_131_128_SOFT_ERR       : 4;     ///<BIT [3:0] ib_q_131_128_soft_err
        uint32_t RSVD_0                      : 28;    ///<BIT [31:4] rsvd_0
    } b;
} UcducdIbCmnSnglInboundQueueSoftError4_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HF_CREDIT_COUNT             : 10;    ///<BIT [9:0] hf_credit_count
        uint32_t RSVD0                       : 22;    ///<BIT [31:10] rsvd0
    } b;
} UcducdIbHfCreditUcdInboundHfCreditCount_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IQ_EN                       : 1;     ///<BIT [0] iq_en
        uint32_t RSVD_1                      : 2;     ///<BIT [2:1] rsvd_1
        uint32_t IQ_RST                      : 1;     ///<BIT [3] iq_rst
        uint32_t IQ_PRIORITY                 : 3;     ///<BIT [6:4] iq_priority
        uint32_t IQ_PASS_THRU_MODE_EN        : 1;     ///<BIT [7] iq_pass_thru_mode_en
        uint32_t IQ_FREE_LIST_SLCT           : 3;     ///<BIT [10:8] iq_free_list_slct
        uint32_t IQ_INTRNL_MEM_BFR_EN        : 1;     ///<BIT [11] iq_intrnl_mem_bfr_en
        uint32_t RSVD_0                      : 2;     ///<BIT [13:12] rsvd_0
        uint32_t IQ_CREDIT_POLICY_EN         : 2;     ///<BIT [15:14] iq_credit_policy_en
        uint32_t IQ_NM_ELMNTS                : 16;    ///<BIT [31:16] iq_nm_elmnts
    } b;
} UcducdIbCmnIqInboundQueueCfg0_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IQ_HF_CREDIT_COUNTER_SLCT   : 6;     ///<BIT [5:0] iq_hf_credit_counter_slct
        uint32_t RSVD_0                      : 2;     ///<BIT [7:6] rsvd_0
        uint32_t IQ_IFC_SLCT                 : 8;     ///<BIT [15:8] iq_ifc_slct
        uint32_t IQ_HOST_LOGICAL_ID          : 16;    ///<BIT [31:16] iq_host_logical_id
    } b;
} UcducdIbCmnIqInboundQueueCfg1_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IQ_CREDIT_COUNT             : 8;     ///<BIT [7:0] iq_credit_count
        uint32_t RSVD_0                      : 24;    ///<BIT [31:8] rsvd_0
    } b;
} UcducdIbCmnIqInboundQueueCreditCount_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      : 2;     ///<BIT [1:0] rsvd_0
        uint32_t IQ_BASE_ADDR_LO             : 30;    ///<BIT [31:2] iq_base_addr_lo
    } b;
} UcducdIbCmnIqInboundQueueBaseAddressLow_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IQ_PRDCR_INDX               : 16;    ///<BIT [15:0] iq_prdcr_indx
        uint32_t RSVD_0                      : 16;    ///<BIT [31:16] rsvd_0
    } b;
} UcducdIbCmnIqInboundQueuePi_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IQ_CNSMR_INDX               : 16;    ///<BIT [15:0] iq_cnsmr_indx
        uint32_t RSVD_0                      : 16;    ///<BIT [31:16] rsvd_0
    } b;
} UcducdIbCmnIqInboundQueueCi_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IQ_EMPTY                    : 1;     ///<BIT [0] iq_empty
        uint32_t IQ_FULL                     : 1;     ///<BIT [1] iq_full
        uint32_t IQ_OVRFLW                   : 1;     ///<BIT [2] iq_ovrflw
        uint32_t RSVD_2                      : 2;     ///<BIT [4:3] rsvd_2
        uint32_t IQ_DBELL_WRITE_OUT_OF_RANGE_ERR : 1;     ///<BIT [5] iq_dbell_write_out_of_range_err
        uint32_t IQ_DBELL_WRITE_SAME_VALUE_ERR : 1;     ///<BIT [6] iq_dbell_write_same_value_err
        uint32_t IQ_DBELL_WRITE_ADD_CMD_TO_FULL_SQ_ERR : 1;     ///<BIT [7] iq_dbell_write_add_cmd_to_full_sq_err
        uint32_t RSVD_1                      : 2;     ///<BIT [9:8] rsvd_1
        uint32_t IQ_LKHD_EMPTY               : 1;     ///<BIT [10] iq_lkhd_empty
        uint32_t IQ_LKHD_FULL                : 1;     ///<BIT [11] iq_lkhd_full
        uint32_t IQ_BUSY                     : 1;     ///<BIT [12] iq_busy
        uint32_t RSVD_0                      : 3;     ///<BIT [15:13] rsvd_0
        uint32_t IQ_LKHD_CNSMR_INDX          : 16;    ///<BIT [31:16] iq_lkhd_cnsmr_indx
    } b;
} UcducdIbCmnIqInboundQueueStatus_t;

/// @brief 0x5000
typedef struct
{
    UcducdIbCmnIqInboundQueueCfg0_t ucdIbCmnIqInboundQueueCfg0; //ucd_ib_cmn_iq_reg_inbound_queue_configuration_0
    UcducdIbCmnIqInboundQueueCfg1_t ucdIbCmnIqInboundQueueCfg1; //ucd_ib_cmn_iq_reg_inbound_queue_configuration_1
    UcducdIbCmnIqInboundQueueCreditCount_t ucdIbCmnIqInboundQueueCreditCount; //ucd_ib_cmn_iq_reg_inbound_queue_credit_count
    uint8_t rsvdC[4]; //rsvd_c ///< 0xc - 0x10
    uint32_t ucdIbCmnIqInboundQueueCiShadowAddressLowRsvd0; //ucd_ib_cmn_iq_reg_inbound_queue_ci_shadow_address_low
    uint32_t ucdIbCmnIqInboundQueueCiShadowAddressHighRsvd0; //ucd_ib_cmn_iq_reg_inbound_queue_ci_shadow_address_high
    UcducdIbCmnIqInboundQueueBaseAddressLow_t ucdIbCmnIqInboundQueueBaseAddressLow; //ucd_ib_cmn_iq_reg_inbound_queue_base_address_low
    uint32_t ucdIbCmnIqInboundQueueBaseAddressHighIqBaseAddrHi; //ucd_ib_cmn_iq_reg_inbound_queue_base_address_high
    UcducdIbCmnIqInboundQueuePi_t ucdIbCmnIqInboundQueuePi; //ucd_ib_cmn_iq_reg_inbound_queue_pi
    UcducdIbCmnIqInboundQueueCi_t ucdIbCmnIqInboundQueueCi; //ucd_ib_cmn_iq_reg_inbound_queue_ci
    UcducdIbCmnIqInboundQueueStatus_t ucdIbCmnIqInboundQueueStatus; //ucd_ib_cmn_iq_reg_inbound_queue_status
    uint8_t endPadding[20];               //end_padding ///< 0x2c - 0x40
} UcdCore1IbCmnIqRegisters_t;
static_assert(TYPE_OFFSET(UcdCore1IbCmnIqRegisters_t, ucdIbCmnIqInboundQueueCiShadowAddressLowRsvd0) == 0x10, "check register structure offset 0x10");
static_assert(TYPE_OFFSET(UcdCore1IbCmnIqRegisters_t, ucdIbCmnIqInboundQueuePi) == 0x20, "check register structure offset 0x20");

/// @brief 0x400
typedef struct
{
    UcducdIbHfCreditUcdInboundHfCreditCount_t ucdIbHfCreditUcdInboundHfCreditCount; //ucd_ib_hf_credit_reg_ucd_inbound_hf_credit_count
} UcducdIbHfCreditRegisters_t;

/// @brief 0x120
typedef struct
{
    UcducdIbCmnCqCompletionQueueCfgControl_t ucdIbCmnCqCompletionQueueCfgControl; //ucd_ib_cmn_cq_reg_completion_queue_configuration_control
    uint8_t rsvd4[4];  //rsvd_4 ///< 0x4-0x8
    UcducdIbCmnCqCompletionQueueIntrCoalescing0_t ucdIbCmnCqCompletionQueueIntrCoalescing0; //ucd_ib_cmn_cq_reg_completion_queue_interrupt_coalescing_0
    UcducdIbCmnCqCompletionQueueIntrCoalescing1_t ucdIbCmnCqCompletionQueueIntrCoalescing1; //ucd_ib_cmn_cq_reg_completion_queue_interrupt_coalescing_1
    UcducdIbCmnCqCompletionQueuePiShadowBaseAddrLo_t ucdIbCmnCqCompletionQueuePiShadowBaseAddrLo; //ucd_ib_cmn_cq_reg_completion_queue_pi_shadow_base_addr_lo
    uint32_t ucdIbCmnCqCompletionQueuePiShadowBaseAddrHiCmpltnQPiShdwBaseAddrHi; //ucd_ib_cmn_cq_reg_completion_queue_pi_shadow_base_addr_hi
    UcducdIbCmnCqCompletionQueueBaseAddrLo_t ucdIbCmnCqCompletionQueueBaseAddrLo; //ucd_ib_cmn_cq_reg_completion_queue_base_addr_lo
    uint32_t ucdIbCmnCqCompletionQueueBaseAddrHiCmpltnQBaseAddrHi; //ucd_ib_cmn_cq_reg_completion_queue_base_addr_hi
    UcducdIbCmnCqCompletionQueuePi_t ucdIbCmnCqCompletionQueuePi; //ucd_ib_cmn_cq_reg_completion_queue_pi
    UcducdIbCmnCqCompletionQueueCi_t ucdIbCmnCqCompletionQueueCi; //ucd_ib_cmn_cq_reg_completion_queue_ci
    UcducdIbCmnCqCompletionQueueStatus_t ucdIbCmnCqCompletionQueueStatus; //ucd_ib_cmn_cq_reg_completion_queue_status
    uint8_t endPadding[4];                //end_padding
} UcducdIbCmnCqRegisters_t;
static_assert(TYPE_OFFSET(UcducdIbCmnCqRegisters_t, ucdIbCmnCqCompletionQueuePiShadowBaseAddrLo) == 0x10, "check register structure offset 0x10");
static_assert(TYPE_OFFSET(UcducdIbCmnCqRegisters_t, ucdIbCmnCqCompletionQueuePi) == 0x20, "check register structure offset 0x20");

/// @brief 0x60
typedef struct
{
    UcducdIbCmnDflInboundDestinationFreeListCfg0_t ucdIbCmnDflInboundDestinationFreeListCfg0; //ucd_ib_cmn_dfl_reg_inbound_destination_free_list_configuration_0
    UcducdIbCmnDflInboundDestinationFreeListCfg1_t ucdIbCmnDflInboundDestinationFreeListCfg1; //ucd_ib_cmn_dfl_reg_inbound_destination_free_list_configuration_1
    UcducdIbCmnDflInboundDestinationFreeListBaseAddrLo_t ucdIbCmnDflInboundDestinationFreeListBaseAddrLo; //ucd_ib_cmn_dfl_reg_inbound_destination_free_list_base_addr_lo
    uint32_t ucdIbCmnDflInboundDestinationFreeListBaseAddrHiIbDestFreeListBaseAddrHi; //ucd_ib_cmn_dfl_reg_inbound_destination_free_list_base_addr_hi
    UcducdIbCmnDflInboundDestinationFreeListPi_t ucdIbCmnDflInboundDestinationFreeListPi; //ucd_ib_cmn_dfl_reg_inbound_destination_free_list_pi
    UcducdIbCmnDflInboundDestinationFreeListCi_t ucdIbCmnDflInboundDestinationFreeListCi; //ucd_ib_cmn_dfl_reg_inbound_destination_free_list_ci
    UcducdIbCmnDflInboundDestinationFreeListStatus_t ucdIbCmnDflInboundDestinationFreeListStatus; //ucd_ib_cmn_dfl_reg_inbound_destination_free_list_status
    uint8_t endPadding[4];                //end_padding
} UcducdIbCmnDflRegisters_t;
static_assert(TYPE_OFFSET(UcducdIbCmnDflRegisters_t, ucdIbCmnDflInboundDestinationFreeListPi) == 0x10, "check register structure offset 0x10");

/// @brief 0x4000
typedef struct
{
    UcducdIbCmnSnglInboundUcdCfg_t ucdIbCmnSnglInboundUcdCfg; //ucd_ib_cmn_sngl_reg_inbound_ucd_configuration
    UcducdIbCmnSnglInboundUcdControl_t ucdIbCmnSnglInboundUcdControl; //ucd_ib_cmn_sngl_reg_inbound_ucd_control
    UcducdIbCmnSnglInboundUcdStatus_t ucdIbCmnSnglInboundUcdStatus; //ucd_ib_cmn_sngl_reg_inbound_ucd_status
    UcducdIbCmnSnglInboundUcdIntrCause_t ucdIbCmnSnglInboundUcdIntrCause; //ucd_ib_cmn_sngl_reg_inbound_ucd_interrupt_cause
    uint32_t ucdIbCmnSnglInboundUcdIntr0EnableSetIbUcdIrq0EnblSet; //ucd_ib_cmn_sngl_reg_inbound_ucd_interrupt_0_enable_set
    uint32_t ucdIbCmnSnglInboundUcdIntr0EnableClearIbUcdIrq0EnblClr; //ucd_ib_cmn_sngl_reg_inbound_ucd_interrupt_0_enable_clear
    uint32_t ucdIbCmnSnglInboundUcdIntr1EnableSetIbUcdIrq1EnblSet; //ucd_ib_cmn_sngl_reg_inbound_ucd_interrupt_1_enable_set
    uint32_t ucdIbCmnSnglInboundUcdIntr1EnableClearIbUcdIrq1EnblClr; //ucd_ib_cmn_sngl_reg_inbound_ucd_interrupt_1_enable_clear
    uint8_t rsvd20[16];                   //rsvd_20
    UcducdIbCmnSnglInboundUcdSramParityErrorCause_t ucdIbCmnSnglInboundUcdSramParityErrorCause; //ucd_ib_cmn_sngl_reg_inbound_ucd_sram_parity_error_cause
    UcducdIbCmnSnglInboundUcdSramParityErrorEnable_t ucdIbCmnSnglInboundUcdSramParityErrorEnable; //ucd_ib_cmn_sngl_reg_inbound_ucd_sram_parity_error_enable
    UcducdIbCmnSnglInboundUcdDataPathErrorControl_t ucdIbCmnSnglInboundUcdDataPathErrorControl; //ucd_ib_cmn_sngl_reg_inbound_ucd_data_path_error_control
    UcducdIbCmnSnglInboundUcdDataPathErrorStatus_t ucdIbCmnSnglInboundUcdDataPathErrorStatus; //ucd_ib_cmn_sngl_reg_inbound_ucd_data_path_error_status
    uint32_t ucdIbCmnSnglInboundUcdErrorAddressLowIbErrAddrLow; //ucd_ib_cmn_sngl_reg_inbound_ucd_error_address_low
    uint32_t ucdIbCmnSnglInboundUcdErrorAddressHighIbErrAddrHigh; //ucd_ib_cmn_sngl_reg_inbound_ucd_error_address_high
    uint8_t rsvd48[8]; //rsvd_48 ///< 0x48 - 0x50
    UcducdIbCmnSnglInboundQueueArbitrationCfgRegister_t ucdIbCmnSnglInboundQueueArbitrationCfgRegister; //ucd_ib_cmn_sngl_reg_inbound_queue_arbitration_configuration_register
    uint8_t rsvd54[12];  //rsvd_54 ///< 0x54 - 0x60
    UcducdIbCmnDflRegisters_t ucdIbCmnDflRegisters[6]; //ucd_ib_cmn_dfl_registers
    UcducdIbCmnCqRegisters_t ucdIbCmnCqRegisters[5]; //ucd_ib_cmn_cq_registers
    uint8_t rsvd210[240]; //rsvd_210 ///< 0x210 - 0x300
    UcducdIbCmnSnglInboundMiscellaneousControl_t ucdIbCmnSnglInboundMiscellaneousControl; //ucd_ib_cmn_sngl_reg_inbound_miscellaneous_control
    uint32_t ucdIbCmnSnglInboundControlPathErrorStatusCpErrorStatus; //ucd_ib_cmn_sngl_reg_inbound_control_path_error_status
    uint32_t ucdIbCmnSnglInboundDatapathHaltErrorStatusDpathHaltErrorStatus; //ucd_ib_cmn_sngl_reg_inbound_datapath_halt_error_status
    uint32_t ucdIbCmnSnglInboundDatapathHaltErrorMaskDpathHaltErrorMask; //ucd_ib_cmn_sngl_reg_inbound_datapath_halt_error_mask
    UcducdIbCmnSnglInboundDiagnosticControl_t ucdIbCmnSnglInboundDiagnosticControl; //ucd_ib_cmn_sngl_reg_inbound_diagnostic_control
    uint32_t ucdIbCmnSnglInboundDiagnosticReadPortDiagMiscStatus; //ucd_ib_cmn_sngl_reg_inbound_diagnostic_read_port
    uint8_t rsvd318[8];  //rsvd_318 ///< 0x318 - 0x320
    UcducdIbCmnSnglInboundDataChannelAxiReadBusAttrs_t ucdIbCmnSnglInboundDataChannelAxiReadBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_data_channel_axi_read_bus_attributes
    UcducdIbCmnSnglInboundDataChannelAxiWriteBusAttrs_t ucdIbCmnSnglInboundDataChannelAxiWriteBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_data_channel_axi_write_bus_attributes
    UcducdIbCmnSnglInboundControlChannelAxiReadBusAttrs_t ucdIbCmnSnglInboundControlChannelAxiReadBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_control_channel_axi_read_bus_attributes
    UcducdIbCmnSnglInboundControlChannelAxiWriteBusAttrs_t ucdIbCmnSnglInboundControlChannelAxiWriteBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_control_channel_axi_write_bus_attributes
    uint8_t rsvd330[16]; //rsvd_330 ///< 0x330 - 0x340
    uint32_t ucdIbCmnSnglInboundQueueSoftError0IbQ310SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_0
    uint32_t ucdIbCmnSnglInboundQueueSoftError1IbQ6332SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_1
    uint32_t ucdIbCmnSnglInboundQueueSoftError2IbQ9564SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_2
    uint32_t ucdIbCmnSnglInboundQueueSoftError3IbQ12796SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_3
    UcducdIbCmnSnglInboundQueueSoftError4_t ucdIbCmnSnglInboundQueueSoftError4; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_4
    uint8_t rsvd354[172];                 //rsvd_354 ///< 0x354 - 0x400
    UcducdIbHfCreditRegisters_t ucdIbHfCreditRegisters[64]; //ucd_ib_hf_credit_registers ///< 0x400 - 0x500
    uint8_t padding6[2816];  ///< 0x500 - 0x1000
} UcdCore1IbCmnRegisters_t;
static_assert(TYPE_OFFSET(UcdCore1IbCmnRegisters_t, ucdIbCmnDflRegisters) == 0x60, "check register structure offset 0x60");
static_assert(TYPE_OFFSET(UcdCore1IbCmnRegisters_t, ucdIbCmnCqRegisters) == 0x120, "check register structure offset 0x120");

/// @brief 0x1000
typedef struct
{
    UcducdIbCmnIqInboundQueueCfg0_t ucdIbCmnIqInboundQueueCfg0; //ucd_ib_cmn_iq_reg_inbound_queue_configuration_0
    UcducdIbCmnIqInboundQueueCfg1_t ucdIbCmnIqInboundQueueCfg1; //ucd_ib_cmn_iq_reg_inbound_queue_configuration_1
    UcducdIbCmnIqInboundQueueCreditCount_t ucdIbCmnIqInboundQueueCreditCount; //ucd_ib_cmn_iq_reg_inbound_queue_credit_count
    uint8_t rsvdC[4]; //rsvd_c ///< 0xc - 0x10
    uint32_t ucdIbCmnIqInboundQueueCiShadowAddressLowRsvd0; //ucd_ib_cmn_iq_reg_inbound_queue_ci_shadow_address_low
    uint32_t ucdIbCmnIqInboundQueueCiShadowAddressHighRsvd0; //ucd_ib_cmn_iq_reg_inbound_queue_ci_shadow_address_high
    UcducdIbCmnIqInboundQueueBaseAddressLow_t ucdIbCmnIqInboundQueueBaseAddressLow; //ucd_ib_cmn_iq_reg_inbound_queue_base_address_low
    uint32_t ucdIbCmnIqInboundQueueBaseAddressHighIqBaseAddrHi; //ucd_ib_cmn_iq_reg_inbound_queue_base_address_high
    UcducdIbCmnIqInboundQueuePi_t ucdIbCmnIqInboundQueuePi; //ucd_ib_cmn_iq_reg_inbound_queue_pi
    UcducdIbCmnIqInboundQueueCi_t ucdIbCmnIqInboundQueueCi; //ucd_ib_cmn_iq_reg_inbound_queue_ci
    UcducdIbCmnIqInboundQueueStatus_t ucdIbCmnIqInboundQueueStatus; //ucd_ib_cmn_iq_reg_inbound_queue_status
    uint8_t endPadding[20];               //end_padding ///< 0x2c - 0x40
} UcdCore0IbCmnIqRegisters_t;
static_assert(TYPE_OFFSET(UcdCore0IbCmnIqRegisters_t, ucdIbCmnIqInboundQueueCiShadowAddressLowRsvd0) == 0x10, "check register structure offset 0x10");
static_assert(TYPE_OFFSET(UcdCore0IbCmnIqRegisters_t, ucdIbCmnIqInboundQueuePi) == 0x20, "check register structure offset 0x20");

/// @brief 0x0
typedef struct
{
    UcducdIbCmnSnglInboundUcdCfg_t ucdIbCmnSnglInboundUcdCfg; //ucd_ib_cmn_sngl_reg_inbound_ucd_configuration
    UcducdIbCmnSnglInboundUcdControl_t ucdIbCmnSnglInboundUcdControl; //ucd_ib_cmn_sngl_reg_inbound_ucd_control
    UcducdIbCmnSnglInboundUcdStatus_t ucdIbCmnSnglInboundUcdStatus; //ucd_ib_cmn_sngl_reg_inbound_ucd_status
    UcducdIbCmnSnglInboundUcdIntrCause_t ucdIbCmnSnglInboundUcdIntrCause; //ucd_ib_cmn_sngl_reg_inbound_ucd_interrupt_cause
    uint32_t ucdIbCmnSnglInboundUcdIntr0EnableSetIbUcdIrq0EnblSet; //ucd_ib_cmn_sngl_reg_inbound_ucd_interrupt_0_enable_set
    uint32_t ucdIbCmnSnglInboundUcdIntr0EnableClearIbUcdIrq0EnblClr; //ucd_ib_cmn_sngl_reg_inbound_ucd_interrupt_0_enable_clear
    uint32_t ucdIbCmnSnglInboundUcdIntr1EnableSetIbUcdIrq1EnblSet; //ucd_ib_cmn_sngl_reg_inbound_ucd_interrupt_1_enable_set
    uint32_t ucdIbCmnSnglInboundUcdIntr1EnableClearIbUcdIrq1EnblClr; //ucd_ib_cmn_sngl_reg_inbound_ucd_interrupt_1_enable_clear
    uint8_t rsvd20[16];                   //rsvd_20
    UcducdIbCmnSnglInboundUcdSramParityErrorCause_t ucdIbCmnSnglInboundUcdSramParityErrorCause; //ucd_ib_cmn_sngl_reg_inbound_ucd_sram_parity_error_cause
    UcducdIbCmnSnglInboundUcdSramParityErrorEnable_t ucdIbCmnSnglInboundUcdSramParityErrorEnable; //ucd_ib_cmn_sngl_reg_inbound_ucd_sram_parity_error_enable
    UcducdIbCmnSnglInboundUcdDataPathErrorControl_t ucdIbCmnSnglInboundUcdDataPathErrorControl; //ucd_ib_cmn_sngl_reg_inbound_ucd_data_path_error_control
    UcducdIbCmnSnglInboundUcdDataPathErrorStatus_t ucdIbCmnSnglInboundUcdDataPathErrorStatus; //ucd_ib_cmn_sngl_reg_inbound_ucd_data_path_error_status
    uint32_t ucdIbCmnSnglInboundUcdErrorAddressLowIbErrAddrLow; //ucd_ib_cmn_sngl_reg_inbound_ucd_error_address_low
    uint32_t ucdIbCmnSnglInboundUcdErrorAddressHighIbErrAddrHigh; //ucd_ib_cmn_sngl_reg_inbound_ucd_error_address_high
    uint8_t rsvd48[8];  //rsvd_48 ///<
    UcducdIbCmnSnglInboundQueueArbitrationCfgRegister_t ucdIbCmnSnglInboundQueueArbitrationCfgRegister; //ucd_ib_cmn_sngl_reg_inbound_queue_arbitration_configuration_register
    uint8_t rsvd54[12];  //rsvd_54 ///<
    UcducdIbCmnDflRegisters_t ucdIbCmnDflRegisters[6]; //ucd_ib_cmn_dfl_registers
    UcducdIbCmnCqRegisters_t ucdIbCmnCqRegisters[5]; //ucd_ib_cmn_cq_registers
    uint8_t rsvd210[240];  //rsvd_210 ///< 0x210 - 0x300
    UcducdIbCmnSnglInboundMiscellaneousControl_t ucdIbCmnSnglInboundMiscellaneousControl; //ucd_ib_cmn_sngl_reg_inbound_miscellaneous_control
    uint32_t ucdIbCmnSnglInboundControlPathErrorStatusCpErrorStatus; //ucd_ib_cmn_sngl_reg_inbound_control_path_error_status
    uint32_t ucdIbCmnSnglInboundDatapathHaltErrorStatusDpathHaltErrorStatus; //ucd_ib_cmn_sngl_reg_inbound_datapath_halt_error_status
    uint32_t ucdIbCmnSnglInboundDatapathHaltErrorMaskDpathHaltErrorMask; //ucd_ib_cmn_sngl_reg_inbound_datapath_halt_error_mask
    UcducdIbCmnSnglInboundDiagnosticControl_t ucdIbCmnSnglInboundDiagnosticControl; //ucd_ib_cmn_sngl_reg_inbound_diagnostic_control
    uint32_t ucdIbCmnSnglInboundDiagnosticReadPortDiagMiscStatus; //ucd_ib_cmn_sngl_reg_inbound_diagnostic_read_port
    uint8_t rsvd318[8];  //rsvd_318 ///< 0x318 - 0x320
    UcducdIbCmnSnglInboundDataChannelAxiReadBusAttrs_t ucdIbCmnSnglInboundDataChannelAxiReadBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_data_channel_axi_read_bus_attributes
    UcducdIbCmnSnglInboundDataChannelAxiWriteBusAttrs_t ucdIbCmnSnglInboundDataChannelAxiWriteBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_data_channel_axi_write_bus_attributes
    UcducdIbCmnSnglInboundControlChannelAxiReadBusAttrs_t ucdIbCmnSnglInboundControlChannelAxiReadBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_control_channel_axi_read_bus_attributes
    UcducdIbCmnSnglInboundControlChannelAxiWriteBusAttrs_t ucdIbCmnSnglInboundControlChannelAxiWriteBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_control_channel_axi_write_bus_attributes
    uint8_t rsvd330[16]; //rsvd_330 ///< 0x330 - 0x340
    uint32_t ucdIbCmnSnglInboundQueueSoftError0IbQ310SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_0
    uint32_t ucdIbCmnSnglInboundQueueSoftError1IbQ6332SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_1
    uint32_t ucdIbCmnSnglInboundQueueSoftError2IbQ9564SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_2
    uint32_t ucdIbCmnSnglInboundQueueSoftError3IbQ12796SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_3
    UcducdIbCmnSnglInboundQueueSoftError4_t ucdIbCmnSnglInboundQueueSoftError4; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_4
    uint8_t rsvd354[172];  //rsvd_354 ///< 0x354 - 0x400
    UcducdIbHfCreditRegisters_t ucdIbHfCreditRegisters[64]; //ucd_ib_hf_credit_registers ///< 0x400 - 0x500
    uint8_t padding6[2816];  ///< 0x500 - 0x1000
} UcdCore0IbCmnRegisters_t;
static_assert(TYPE_OFFSET(UcdCore0IbCmnRegisters_t, ucdIbCmnDflRegisters) == 0x60, "check register structure offset 0x60");
static_assert(TYPE_OFFSET(UcdCore0IbCmnRegisters_t, ucdIbCmnCqRegisters) == 0x120, "check register structure offset 0x120");

/// @brief 0x180000
typedef struct
{
    UcdCore0IbCmnRegisters_t ucdCore0IbCmnRegisters; //ucd_core0_ib_cmn_registers ///< 0x0000 - 0x1000
    UcdCore0IbCmnIqRegisters_t ucdCore0IbCmnIqRegisters[132]; //ucd_core0_ib_cmn_iq_registers ///< 0x1000 - 0x3100
    uint8_t rsvd3100[3840];  //rsvd_3100 ///< 0x3100 - 0x4000
    UcdCore1IbCmnRegisters_t ucdCore1IbCmnRegisters; //ucd_core1_ib_cmn_registers ///< 0x4000 - 0x5000
    UcdCore1IbCmnIqRegisters_t ucdCore1IbCmnIqRegisters[132]; //ucd_core1_ib_cmn_iq_registers ///< 0x5000 - 0x7100
    //uint8_t rsvd7100to8000[3840]; ///< 0x7100 - 0x8000
} UcdInboundRegisters_t;

#if 1
static_assert(TYPE_OFFSET(UcdInboundRegisters_t, ucdCore0IbCmnRegisters) == 0x0, "check register structure offset 0x0");
static_assert(TYPE_OFFSET(UcdInboundRegisters_t, ucdCore0IbCmnIqRegisters) == 0x1000, "check register structure offset 0x1000");
static_assert(TYPE_OFFSET(UcdInboundRegisters_t, ucdCore1IbCmnRegisters) == 0x4000, "check register structure offset 0x4000");
static_assert(TYPE_OFFSET(UcdInboundRegisters_t, ucdCore1IbCmnIqRegisters) == 0x5000, "check register structure offset 0x5000");
#else
COMPILE_ASSERT(TYPE_OFFSET(UcdInboundRegisters_t, ucdCore1IbCmnRegisters) == 0x4000, "check register structure offset 0x4000");
#endif

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
