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

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_1                      : 8;     ///<BIT [7:0] rsvd_1
        uint32_t NVME_CC_EN_PF_UPDTD         : 1;     ///<BIT [8] nvme_cc_en_pf_updtd
        uint32_t NVME_CC_EN_VF_UPDTD         : 1;     ///<BIT [9] nvme_cc_en_vf_updtd
        uint32_t NVME_CC_SHN_PF_UPDTD        : 1;     ///<BIT [10] nvme_cc_shn_pf_updtd
        uint32_t NVME_CC_SHN_VF_UPDTD        : 1;     ///<BIT [11] nvme_cc_shn_vf_updtd
        uint32_t NVME_CNTRLR_RST_PF_RCVD     : 1;     ///<BIT [12] nvme_cntrlr_rst_pf_rcvd
        uint32_t NVME_CNTRLR_RST_VF_RCVD     : 1;     ///<BIT [13] nvme_cntrlr_rst_vf_rcvd
        uint32_t RSVD_0                      : 9;     ///<BIT [22:14] rsvd_0
        uint32_t HOST_DOORBELL_ACCESS_ERROR  : 1;     ///<BIT [23] host_doorbell_access_error
        uint32_t RSVD_2                      : 5;     ///<BIT [28:24] rsvd_2
        uint32_t COMMON_AXI_MONITOR_ERROR    : 1;     ///<BIT [29] common_axi_monitor_error
        uint32_t COMMON_AXI_SLV_PARITY_ERROR : 1;     ///<BIT [30] common_axi_slv_parity_error
        uint32_t COMMON_AXI_MSTR_PARITY_ERROR : 1;     ///<BIT [31] common_axi_mstr_parity_error
    } b;
} UcdGenCmnSnglCommonUcdIntrCause_t;

/// @brief 0x80
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HIU_NVME_REGSTR_SET_RST_65  : 1;     ///<BIT [0] hiu_nvme_regstr_set_rst_65
        uint32_t RSVD_0                      : 31;    ///<BIT [31:1] rsvd_0
    } b;
} UcdGenCmnSnglNvmeRegisterSetResetPf_t;

/// @brief 0x180
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HIU_NVME_ADMIN_REGSTRS_RST_65 : 1;     ///<BIT [0] hiu_nvme_admin_regstrs_rst_65
        uint32_t RSVD_0                      : 31;    ///<BIT [31:1] rsvd_0
    } b;
} UcdGenCmnSnglNvmeAdminResetPf_t;

/// @brief 0x280
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HIU_NVME_CC_EN_UPDTD_65     : 1;     ///<BIT [0] hiu_nvme_cc_en_updtd_65
        uint32_t RSVD_0                      : 31;    ///<BIT [31:1] rsvd_0
    } b;
} UcdGenCmnSnglNvmeControllerCfgEnFieldUpdatedPf_t;

/// @brief 0x380
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HIU_NVME_CC_SHN_UPDTD_65    : 1;     ///<BIT [0] hiu_nvme_cc_shn_updtd_65
        uint32_t RSVD_0                      : 31;    ///<BIT [31:1] rsvd_0
    } b;
} UcdGenCmnSnglNvmeControllerCfgShnFieldUpdatedPf_t;

/// @brief 0x480
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HIU_NVME_RESET_RCVD_65      : 1;     ///<BIT [0] hiu_nvme_reset_rcvd_65
        uint32_t RSVD_0                      : 31;    ///<BIT [31:1] rsvd_0
    } b;
} UcdGenCmnSnglNvmeSubsystemResetReceivedPf_t;

/// @brief 0xFA8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DBL_MGR_SM_EN               : 1;     ///<BIT [0] dbl_mgr_sm_en
        uint32_t RSVD_2                      : 3;     ///<BIT [3:1] rsvd_2
        uint32_t DBL_RD_FWD_MODE             : 2;     ///<BIT [5:4] dbl_rd_fwd_mode
        uint32_t DBL_MAX_OUTSTND_RSP_CNT_SEL : 2;     ///<BIT [7:6] dbl_max_outstnd_rsp_cnt_sel
        uint32_t DBL_WIDTH                   : 1;     ///<BIT [8] dbl_width
        uint32_t DBL_HOST_WR_ACC_DSBL        : 1;     ///<BIT [9] dbl_host_wr_acc_dsbl
        uint32_t DBL_HOST_RD_ACC_DSBL        : 1;     ///<BIT [10] dbl_host_rd_acc_dsbl
        uint32_t DBL_PSTHRU_AXI_WR_STRETCH_WD_TMR_DSBL : 1;     ///<BIT [11] dbl_psthru_axi_wr_stretch_wd_tmr_dsbl
        uint32_t RSVD_1                      : 4;     ///<BIT [15:12] rsvd_1
        uint32_t DBL_RD_NO_MATCH_ERROR       : 1;     ///<BIT [16] dbl_rd_no_match_error
        uint32_t DBL_WR_NO_MATCH_ERROR       : 1;     ///<BIT [17] dbl_wr_no_match_error
        uint32_t DBL_RD_BOTH_IQ_OQ_MATCH_ERROR : 1;     ///<BIT [18] dbl_rd_both_iq_oq_match_error
        uint32_t DBL_WR_BOTH_IQ_OQ_MATCH_ERROR : 1;     ///<BIT [19] dbl_wr_both_iq_oq_match_error
        uint32_t DBL_RD_INVALID_ADDR_MATCH_ERROR : 1;     ///<BIT [20] dbl_rd_invalid_addr_match_error
        uint32_t DBL_WR_INVALID_ADDR_MATCH_ERROR : 1;     ///<BIT [21] dbl_wr_invalid_addr_match_error
        uint32_t RSVD_0                      : 9;     ///<BIT [30:22] rsvd_0
        uint32_t ADMINQ_BA_11_6_WRTBL        : 1;     ///<BIT [31] adminq_ba_11_6_wrtbl
    } b;
} UcdGenCmnSnglDoorbellManagerStateMachineControl_t;

/// @brief 0xFAC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMN_AXI_MSTR_FWD_CHK_EN     : 1;     ///<BIT [0] cmn_axi_mstr_fwd_chk_en
        uint32_t CMN_AXI_MSTR_PARITY_CHK_EN  : 1;     ///<BIT [1] cmn_axi_mstr_parity_chk_en
        uint32_t CMN_AXI_MSTR_ODD_PARITY_EN  : 1;     ///<BIT [2] cmn_axi_mstr_odd_parity_en
        uint32_t CMN_AXI_MSTR_FWD_ERROR      : 1;     ///<BIT [3] cmn_axi_mstr_fwd_error
        uint32_t CMN_AXI_MSTR_PARITY_ERROR   : 1;     ///<BIT [4] cmn_axi_mstr_parity_error
        uint32_t RSVD_0                      : 1;     ///<BIT [5] rsvd_0
        uint32_t CMN_AXI_MONITOR_ERROR_CHK_EN : 1;     ///<BIT [6] cmn_axi_monitor_error_chk_en
        uint32_t CMN_AXI_MONITOR_ERROR_CLR   : 1;     ///<BIT [7] cmn_axi_monitor_error_clr
        uint32_t CMN_AXI_MSTR_CPTRD_ERROR_STATUS : 20;    ///<BIT [27:8] cmn_axi_mstr_cptrd_error_status
        uint32_t UCD_CMPST_ERROR_STATUS      : 4;     ///<BIT [31:28] ucd_cmpst_error_status
    } b;
} UcdGenCmnSnglCommonUcdErrorControlAndStatus_t;

/// @brief 0xFB0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMN_AXI_IB_DC_MAX_RD_REQ_CNT : 6;     ///<BIT [5:0] cmn_axi_ib_dc_max_rd_req_cnt
        uint32_t CMN_AXI_IB_CC_MAX_RD_REQ_CNT : 6;     ///<BIT [11:6] cmn_axi_ib_cc_max_rd_req_cnt
        uint32_t CMN_AXI_OB_DC_MAX_RD_REQ_CNT : 6;     ///<BIT [17:12] cmn_axi_ob_dc_max_rd_req_cnt
        uint32_t CMN_AXI_OB_CC_MAX_RD_REQ_CNT : 6;     ///<BIT [23:18] cmn_axi_ob_cc_max_rd_req_cnt
        uint32_t CMN_AXI_MSTR_MAX_RD_REQ_CNT : 8;     ///<BIT [31:24] cmn_axi_mstr_max_rd_req_cnt
    } b;
} UcdGenCmnSnglCommonAxiMaxReadReqCount_t;

/// @brief 0xFB4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMN_AXI_IB_DC_MAX_WR_REQ_CNT : 6;     ///<BIT [5:0] cmn_axi_ib_dc_max_wr_req_cnt
        uint32_t CMN_AXI_IB_CC_MAX_WR_REQ_CNT : 6;     ///<BIT [11:6] cmn_axi_ib_cc_max_wr_req_cnt
        uint32_t CMN_AXI_OB_DC_MAX_WR_REQ_CNT : 6;     ///<BIT [17:12] cmn_axi_ob_dc_max_wr_req_cnt
        uint32_t CMN_AXI_OB_CC_MAX_WR_REQ_CNT : 6;     ///<BIT [23:18] cmn_axi_ob_cc_max_wr_req_cnt
        uint32_t CMN_AXI_MSTR_MAX_WR_REQ_CNT : 8;     ///<BIT [31:24] cmn_axi_mstr_max_wr_req_cnt
    } b;
} UcdGenCmnSnglCommonAxiMaxWriteReqCount_t;

/// @brief 0xFB8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMN_AXI_IB_DC_MAX_WR_REQ_CNT_REACHED : 1;     ///<BIT [0] cmn_axi_ib_dc_max_wr_req_cnt_reached
        uint32_t CMN_AXI_IB_CC_MAX_WR_REQ_CNT_REACHED : 1;     ///<BIT [1] cmn_axi_ib_cc_max_wr_req_cnt_reached
        uint32_t CMN_AXI_OB_DC_MAX_WR_REQ_CNT_REACHED : 1;     ///<BIT [2] cmn_axi_ob_dc_max_wr_req_cnt_reached
        uint32_t CMN_AXI_OB_CC_MAX_WR_REQ_CNT_REACHED : 1;     ///<BIT [3] cmn_axi_ob_cc_max_wr_req_cnt_reached
        uint32_t CMN_AXI_IB_DC_MAX_RD_REQ_CNT_REACHED : 1;     ///<BIT [4] cmn_axi_ib_dc_max_rd_req_cnt_reached
        uint32_t CMN_AXI_IB_CC_MAX_RD_REQ_CNT_REACHED : 1;     ///<BIT [5] cmn_axi_ib_cc_max_rd_req_cnt_reached
        uint32_t CMN_AXI_OB_DC_MAX_RD_REQ_CNT_REACHED : 1;     ///<BIT [6] cmn_axi_ob_dc_max_rd_req_cnt_reached
        uint32_t CMN_AXI_OB_CC_MAX_RD_REQ_CNT_REACHED : 1;     ///<BIT [7] cmn_axi_ob_cc_max_rd_req_cnt_reached
        uint32_t CMN_AXI_MSTR_MAX_WR_REQ_CNT_REACHED : 1;     ///<BIT [8] cmn_axi_mstr_max_wr_req_cnt_reached
        uint32_t CMN_AXI_MSTR_MAX_RD_REQ_CNT_REACHED : 1;     ///<BIT [9] cmn_axi_mstr_max_rd_req_cnt_reached
        uint32_t RSVD_0                      : 22;    ///<BIT [31:10] rsvd_0
    } b;
} UcdGenCmnSnglCommonAxiRequestStatus_t;

/// @brief 0xFC0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMN_MSIX_VECTOR_COALESCING_MODE : 2;     ///<BIT [1:0] cmn_msix_vector_coalescing_mode
        uint32_t RSVD_2                      : 2;     ///<BIT [3:2] rsvd_2
        uint32_t CMN_AXI_SLV_PARITY_CHK_EN   : 1;     ///<BIT [4] cmn_axi_slv_parity_chk_en
        uint32_t CMN_AXI_SLV_ODD_PARITY_EN   : 1;     ///<BIT [5] cmn_axi_slv_odd_parity_en
        uint32_t CMN_AXI_SLV_ERROR           : 1;     ///<BIT [6] cmn_axi_slv_error
        uint32_t RSVD_1                      : 9;     ///<BIT [15:7] rsvd_1
        uint32_t CMN_AXI_SLV_CPTRD_ERROR_INFO : 12;    ///<BIT [27:16] cmn_axi_slv_cptrd_error_info
        uint32_t RSVD_0                      : 1;     ///<BIT [28] rsvd_0
        uint32_t CMN_DPATH_HALT_ERROR_MASK   : 3;     ///<BIT [31:29] cmn_dpath_halt_error_mask
    } b;
} UcdGenCmnSnglUcdTopCfg_t;

/// @brief 0x100000
typedef struct
{
    uint8_t rsvd0[16];  //rsvd_0 ///< 0x0 - 0x10
    UcdGenCmnSnglCommonUcdIntrCause_t ucdGenCmnSnglCommonUcdIntrCause; //ucd_gen_cmn_sngl_reg_common_ucd_interrupt_cause
    uint32_t ucdGenCmnSnglCommonUcdIntr0EnableCmnUcdIrq0Enbl; //ucd_gen_cmn_sngl_reg_common_ucd_interrupt_0_enable
    uint32_t ucdGenCmnSnglCommonUcdIntr1EnableCmnUcdIrq1Enbl; //ucd_gen_cmn_sngl_reg_common_ucd_interrupt_1_enable
    uint8_t rsvd1c[100];  //rsvd_1c ///< 0x1c - 0x80
    UcdGenCmnSnglNvmeRegisterSetResetPf_t ucdGenCmnSnglNvmeRegisterSetResetPf; //ucd_gen_cmn_sngl_reg_nvme_register_set_reset_pf
    uint32_t ucdGenCmnSnglNvmeRegisterSetReset0VfHiuNvmeRgstrSetRst310; //ucd_gen_cmn_sngl_reg_nvme_register_set_reset_0_vf
    uint32_t ucdGenCmnSnglNvmeRegisterSetReset1VfHiuNvmeRgstrSetRst6332; //ucd_gen_cmn_sngl_reg_nvme_register_set_reset_1_vf
    uint8_t rsvd8c[244];  //rsvd_8c ///< 0x8c - 0x180
    UcdGenCmnSnglNvmeAdminResetPf_t ucdGenCmnSnglNvmeAdminResetPf; //ucd_gen_cmn_sngl_reg_nvme_admin_registers_reset_pf
    uint32_t ucdGenCmnSnglNvmeAdminReset0VfHiuNvmeAdminRgstrsRst310; //ucd_gen_cmn_sngl_reg_nvme_admin_registers_reset_0_vf
    uint32_t ucdGenCmnSnglNvmeAdminReset1VfHiuNvmeAdminRgstrsRst6332; //ucd_gen_cmn_sngl_reg_nvme_admin_registers_reset_1_vf
    uint8_t rsvd18c[244]; //rsvd_18c  ///< 0x18c - 0x280
    UcdGenCmnSnglNvmeControllerCfgEnFieldUpdatedPf_t ucdGenCmnSnglNvmeControllerCfgEnFieldUpdatedPf; //ucd_gen_cmn_sngl_reg_nvme_controller_configuration_en_field_updated_pf
    uint32_t ucdGenCmnSnglNvmeControllerCfgEnFieldUpdated0VfHiuNvmeCcEnUpdtd310; //ucd_gen_cmn_sngl_reg_nvme_controller_configuration_en_field_updated_0_vf
    uint32_t ucdGenCmnSnglNvmeControllerCfgEnFieldUpdated1VfHiuNvmeCcEnUpdtd6332; //ucd_gen_cmn_sngl_reg_nvme_controller_configuration_en_field_updated_1_vf
    uint8_t rsvd28c[244];  //rsvd_28c ///< 0x28c - 0x380
    UcdGenCmnSnglNvmeControllerCfgShnFieldUpdatedPf_t ucdGenCmnSnglNvmeControllerCfgShnFieldUpdatedPf; //ucd_gen_cmn_sngl_reg_nvme_controller_configuration_shn_field_updated_pf
    uint32_t ucdGenCmnSnglNvmeControllerCfgShnFieldUpdated0VfHiuNvmeCcShnUpdtd310; //ucd_gen_cmn_sngl_reg_nvme_controller_configuration_shn_field_updated_0_vf
    uint32_t ucdGenCmnSnglNvmeControllerCfgShnFieldUpdated1VfHiuNvmeCcShnUpdtd6332; //ucd_gen_cmn_sngl_reg_nvme_controller_configuration_shn_field_updated_1_vf
    uint8_t rsvd38c[244];  //rsvd_38c ///< 0x38c - 0x480
    UcdGenCmnSnglNvmeSubsystemResetReceivedPf_t ucdGenCmnSnglNvmeSubsystemResetReceivedPf; //ucd_gen_cmn_sngl_reg_nvme_subsystem_reset_received_pf
    uint32_t ucdGenCmnSnglNvmeSubsystemResetReceived0VfHiuNvmeResetRcvd310; //ucd_gen_cmn_sngl_reg_nvme_subsystem_reset_received_0_vf
    uint32_t ucdGenCmnSnglNvmeSubsystemResetReceived1VfHiuNvmeResetRcvd6332; //ucd_gen_cmn_sngl_reg_nvme_subsystem_reset_received_1_vf
    uint8_t rsvd48c[2844];  //rsvd_48c ///< 0x48c - 0xfa8
    UcdGenCmnSnglDoorbellManagerStateMachineControl_t ucdGenCmnSnglDoorbellManagerStateMachineControl; //ucd_gen_cmn_sngl_reg_doorbell_manager_state_machine_control
    UcdGenCmnSnglCommonUcdErrorControlAndStatus_t ucdGenCmnSnglCommonUcdErrorControlAndStatus; //ucd_gen_cmn_sngl_reg_common_ucd_error_control_and_status
    UcdGenCmnSnglCommonAxiMaxReadReqCount_t ucdGenCmnSnglCommonAxiMaxReadReqCount; //ucd_gen_cmn_sngl_reg_common_axi_max_read_req_count
    UcdGenCmnSnglCommonAxiMaxWriteReqCount_t ucdGenCmnSnglCommonAxiMaxWriteReqCount; //ucd_gen_cmn_sngl_reg_common_axi_max_write_req_count
    UcdGenCmnSnglCommonAxiRequestStatus_t ucdGenCmnSnglCommonAxiRequestStatus; //ucd_gen_cmn_sngl_reg_common_axi_request_status
    uint32_t ucdGenCmnSnglUcdTopTraceportStatusUcdTopTraceportStatus; //ucd_gen_cmn_sngl_reg_ucd_top_traceport_status
    UcdGenCmnSnglUcdTopCfg_t ucdGenCmnSnglUcdTopCfg; //ucd_gen_cmn_sngl_reg_ucd_top_configuration ///< 0xfc0 - 0xfc4
} UcdGenCmnRegisters_t;
static_assert(TYPE_OFFSET(UcdGenCmnRegisters_t, ucdGenCmnSnglNvmeControllerCfgEnFieldUpdatedPf) == 0x280, "check register structure offset 0x280");
static_assert(TYPE_OFFSET(UcdGenCmnRegisters_t, ucdGenCmnSnglNvmeControllerCfgShnFieldUpdatedPf) == 0x380, "check register structure offset 0x380");
static_assert(TYPE_OFFSET(UcdGenCmnRegisters_t, ucdGenCmnSnglNvmeSubsystemResetReceivedPf) == 0x480, "check register structure offset 0x480");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
