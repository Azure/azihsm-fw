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
//! @brief UCD Registers
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
        uint32_t CAP_LO_MQES                 :16;     ///<BIT [15:0] cap_lo_mqes
        uint32_t CAP_LO_CAP_LO_CQR           :1;      ///<BIT [16] cap_lo_cap_lo_cqr
        uint32_t CAP_LO_AMS                  :2;      ///<BIT [18:17] cap_lo_ams
        uint32_t CAP_LO_RSVD_RW_0            :5;      ///<BIT [23:19] cap_lo_rsvd_rw_0
        uint32_t CAP_LO_TO                   :8;      ///<BIT [31:24] cap_lo_to
    } b;
} UcducdNvmeControllerCapabilitiesLo_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CAP_HI_DSTRD                :4;      ///<BIT [3:0] cap_hi_dstrd
        uint32_t CAP_HI_NSSRS                :1;      ///<BIT [4] cap_hi_nssrs
        uint32_t CAP_HI_CSS                  :8;      ///<BIT [12:5] cap_hi_css
        uint32_t CAP_HI_RSVD_RW_1            :3;      ///<BIT [15:13] cap_hi_rsvd_rw_1
        uint32_t CAP_HI_MPSMIN               :4;      ///<BIT [19:16] cap_hi_mpsmin
        uint32_t CAP_HI_MPSMAX               :4;      ///<BIT [23:20] cap_hi_mpsmax
        uint32_t CAP_HI_RSVD_RW_0            :8;      ///<BIT [31:24] cap_hi_rsvd_rw_0
    } b;
} UcducdNvmeControllerCapabilitiesHi_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VS_RSVD_RW_0                :8;      ///<BIT [7:0] vs_rsvd_rw_0
        uint32_t VS_MNR                      :8;      ///<BIT [15:8] vs_mnr
        uint32_t VS_MJR                      :16;     ///<BIT [31:16] vs_mjr
    } b;
} UcducdNvmeVersion_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INTMS_IVMS                  :4;      ///<BIT [3:0] intms_ivms
        uint32_t INTMS_RSVD                  :28;     ///<BIT [31:4] intms_rsvd
    } b;
} UcducdNvmeIntrMaskSet_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INTMC_IVMC                  :4;      ///<BIT [3:0] intmc_ivmc
        uint32_t INTMC_RSVD                  :28;     ///<BIT [31:4] intmc_rsvd
    } b;
} UcducdNvmeIntrMaskClear_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CC_EN                       :1;      ///<BIT [0] cc_en
        uint32_t CC_RSVD_RW_1                :3;      ///<BIT [3:1] cc_rsvd_rw_1
        uint32_t CC_CSS                      :3;      ///<BIT [6:4] cc_css
        uint32_t CC_MPS                      :4;      ///<BIT [10:7] cc_mps
        uint32_t CC_AMS                      :3;      ///<BIT [13:11] cc_ams
        uint32_t CC_SHN                      :2;      ///<BIT [15:14] cc_shn
        uint32_t CC_IOSQES                   :4;      ///<BIT [19:16] cc_iosqes
        uint32_t CC_IOCQES                   :4;      ///<BIT [23:20] cc_iocqes
        uint32_t CC_RSVD_RW_0                :8;      ///<BIT [31:24] cc_rsvd_rw_0
    } b;
} UcducdNvmeControllerCfg_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CSTS_RDY                    :1;      ///<BIT [0] csts_rdy
        uint32_t CSTS_CFS                    :1;      ///<BIT [1] csts_cfs
        uint32_t CSTS_SHST                   :2;      ///<BIT [3:2] csts_shst
        uint32_t CSTS_NSSRO                  :1;      ///<BIT [4] csts_nssro
        uint32_t CSTS_RSVD_RW_0              :3;      ///<BIT [7:5] csts_rsvd_rw_0
        uint32_t CSTS_RSVD                   :24;     ///<BIT [31:8] csts_rsvd
    } b;
} UcducdNvmeControllerStatus_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AQA_ASQS                    :12;     ///<BIT [11:0] aqa_asqs
        uint32_t AQA_RSVD                    :4;      ///<BIT [15:12] aqa_rsvd
        uint32_t AQA_ACQS                    :12;     ///<BIT [27:16] aqa_acqs
        uint32_t AQA_RSVD1                   :4;      ///<BIT [31:28] aqa_rsvd1
    } b;
} UcducdNvmeAdminQueueAttrs_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ASQB_LO_RSVD                :6;      ///<BIT [5:0] asqb_lo_rsvd
        uint32_t ASQB_LO_RSVD_RW             :6;      ///<BIT [11:6] asqb_lo_rsvd_rw
        uint32_t ASQB_LO                     :20;     ///<BIT [31:12] asqb_lo
    } b;
} UcducdNvmeAdminSubmissionQueueBaseAddressLo_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ACQB_LO_RSVD                :6;      ///<BIT [5:0] acqb_lo_rsvd
        uint32_t ACQB_LO_RSVD_RW             :6;      ///<BIT [11:6] acqb_lo_rsvd_rw
        uint32_t ACQB_LO                     :20;     ///<BIT [31:12] acqb_lo
    } b;
} UcducdNvmeAdminCompletionQueueBaseAddressLo_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMBLOC_BIR                  :3;      ///<BIT [2:0] cmbloc_bir
        uint32_t CMBLOC_RSVD                 :9;      ///<BIT [11:3] cmbloc_rsvd
        uint32_t CMBLOC_OFST                 :20;     ///<BIT [31:12] cmbloc_ofst
    } b;
} UcducdNvmeControllerMemoryBufferLocation_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMBSZ_SQS                   :1;      ///<BIT [0] cmbsz_sqs
        uint32_t CMBSZ_CQS                   :1;      ///<BIT [1] cmbsz_cqs
        uint32_t CMBSZ_LISTS                 :1;      ///<BIT [2] cmbsz_lists
        uint32_t CMBSZ_RDS                   :1;      ///<BIT [3] cmbsz_rds
        uint32_t CMBSZ_WDS                   :1;      ///<BIT [4] cmbsz_wds
        uint32_t CMBSZ_RSVD                  :3;      ///<BIT [7:5] cmbsz_rsvd
        uint32_t CMBSZ_SZU                   :4;      ///<BIT [11:8] cmbsz_szu
        uint32_t CMBSZ_SZ                    :20;     ///<BIT [31:12] cmbsz_sz
    } b;
} UcducdNvmeControllerMemoryBufferSize_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IS_INTA_PENDING             :1;      ///<BIT [0] is_inta_pending
        uint32_t IS_INTA_MASK                :1;      ///<BIT [1] is_inta_mask
        uint32_t IS_INTA_SRC_PENDING         :1;      ///<BIT [2] is_inta_src_pending
        uint32_t IS_INTB_PENDING             :1;      ///<BIT [3] is_intb_pending
        uint32_t IS_INTB_MASK                :1;      ///<BIT [4] is_intb_mask
        uint32_t IS_INTB_SRC_PENDING         :1;      ///<BIT [5] is_intb_src_pending
        uint32_t IS_INTC_PENDING             :1;      ///<BIT [6] is_intc_pending
        uint32_t IS_INTC_MASK                :1;      ///<BIT [7] is_intc_mask
        uint32_t IS_INTC_SRC_PENDING         :1;      ///<BIT [8] is_intc_src_pending
        uint32_t IS_INTD_PENDING             :1;      ///<BIT [9] is_intd_pending
        uint32_t IS_INTD_MASK                :1;      ///<BIT [10] is_intd_mask
        uint32_t IS_INTD_SRC_PENDING         :1;      ///<BIT [11] is_intd_src_pending
        uint32_t IS_RSVD                     :20;     ///<BIT [31:12] is_rsvd
    } b;
} UcducdNvmeIntrStatus_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_1                      :8;      ///<BIT [7:0] rsvd_1
        uint32_t NVME_CC_EN_PF_UPDTD         :1;      ///<BIT [8] nvme_cc_en_pf_updtd
        uint32_t NVME_CC_EN_VF_UPDTD         :1;      ///<BIT [9] nvme_cc_en_vf_updtd
        uint32_t NVME_CC_SHN_PF_UPDTD        :1;      ///<BIT [10] nvme_cc_shn_pf_updtd
        uint32_t NVME_CC_SHN_VF_UPDTD        :1;      ///<BIT [11] nvme_cc_shn_vf_updtd
        uint32_t NVME_CNTRLR_RST_PF_RCVD     :1;      ///<BIT [12] nvme_cntrlr_rst_pf_rcvd
        uint32_t NVME_CNTRLR_RST_VF_RCVD     :1;      ///<BIT [13] nvme_cntrlr_rst_vf_rcvd
        uint32_t RSVD                        :9;      ///<BIT [22:14] rsvd_0
        uint32_t HOST_DOORBELL_ACCESS_ERROR  :1;      ///<BIT [23] host_doorbell_access_error
        uint32_t RSVD_2                      :5;      ///<BIT [28:24] rsvd_2
        uint32_t COMMON_AXI_MONITOR_ERROR    :1;      ///<BIT [29] common_axi_monitor_error
        uint32_t COMMON_AXI_SLV_PARITY_ERROR :1;      ///<BIT [30] common_axi_slv_parity_error
        uint32_t COMMON_AXI_MSTR_PARITY_ERROR :1;      ///<BIT [31] common_axi_mstr_parity_error
    } b;
} UcdGenCmnSnglCommonUcdIntrCause_t;

/// @brief 0x80
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HIU_NVME_REGSTR_SET_RST_65  :1;      ///<BIT [0] hiu_nvme_regstr_set_rst_65
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} UcdGenCmnSnglNvmeRegisterSetResetPf_t;

/// @brief 0x180
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HIU_NVME_ADMIN_REGSTRS_RST_65 :1;      ///<BIT [0] hiu_nvme_admin_regstrs_rst_65
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} UcdGenCmnSnglNvmeAdminResetPf_t;

/// @brief 0x280
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HIU_NVME_CC_EN_UPDTD_65     :1;      ///<BIT [0] hiu_nvme_cc_en_updtd_65
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} UcdGenCmnSnglNvmeControllerCfgEnFieldUpdatedPf_t;

/// @brief 0x380
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HIU_NVME_CC_SHN_UPDTD_65    :1;      ///<BIT [0] hiu_nvme_cc_shn_updtd_65
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} UcdGenCmnSnglNvmeControllerCfgShnFieldUpdatedPf_t;

/// @brief 0x480
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HIU_NVME_RESET_RCVD_65      :1;      ///<BIT [0] hiu_nvme_reset_rcvd_65
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} UcdGenCmnSnglNvmeSubsystemResetReceivedPf_t;

/// @brief 0xFA8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DBL_MGR_SM_EN               :1;      ///<BIT [0] dbl_mgr_sm_en
        uint32_t RSVD_2                      :3;      ///<BIT [3:1] rsvd_2
        uint32_t DBL_RD_FWD_MODE             :2;      ///<BIT [5:4] dbl_rd_fwd_mode
        uint32_t DBL_MAX_OUTSTND_RSP_CNT_SEL :2;      ///<BIT [7:6] dbl_max_outstnd_rsp_cnt_sel
        uint32_t DBL_WIDTH                   :1;      ///<BIT [8] dbl_width
        uint32_t DBL_HOST_WR_ACC_DSBL        :1;      ///<BIT [9] dbl_host_wr_acc_dsbl
        uint32_t DBL_HOST_RD_ACC_DSBL        :1;      ///<BIT [10] dbl_host_rd_acc_dsbl
        uint32_t DBL_PSTHRU_AXI_WR_STRETCH_WD_TMR_DSBL :1;      ///<BIT [11] dbl_psthru_axi_wr_stretch_wd_tmr_dsbl
        uint32_t RSVD_1                      :4;      ///<BIT [15:12] rsvd_1
        uint32_t DBL_RD_NO_MATCH_ERROR       :1;      ///<BIT [16] dbl_rd_no_match_error
        uint32_t DBL_WR_NO_MATCH_ERROR       :1;      ///<BIT [17] dbl_wr_no_match_error
        uint32_t DBL_RD_BOTH_IQ_OQ_MATCH_ERROR :1;      ///<BIT [18] dbl_rd_both_iq_oq_match_error
        uint32_t DBL_WR_BOTH_IQ_OQ_MATCH_ERROR :1;      ///<BIT [19] dbl_wr_both_iq_oq_match_error
        uint32_t DBL_RD_INVALID_ADDR_MATCH_ERROR :1;      ///<BIT [20] dbl_rd_invalid_addr_match_error
        uint32_t DBL_WR_INVALID_ADDR_MATCH_ERROR :1;      ///<BIT [21] dbl_wr_invalid_addr_match_error
        uint32_t RSVD                        :9;      ///<BIT [30:22] rsvd_0
        uint32_t ADMINQ_BA_11_6_WRTBL        :1;      ///<BIT [31] adminq_ba_11_6_wrtbl
    } b;
} UcdGenCmnSnglDoorbellManagerStateMachineControl_t;

/// @brief 0xFAC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMN_AXI_MSTR_FWD_CHK_EN     :1;      ///<BIT [0] cmn_axi_mstr_fwd_chk_en
        uint32_t CMN_AXI_MSTR_PARITY_CHK_EN  :1;      ///<BIT [1] cmn_axi_mstr_parity_chk_en
        uint32_t CMN_AXI_MSTR_ODD_PARITY_EN  :1;      ///<BIT [2] cmn_axi_mstr_odd_parity_en
        uint32_t CMN_AXI_MSTR_FWD_ERROR      :1;      ///<BIT [3] cmn_axi_mstr_fwd_error
        uint32_t CMN_AXI_MSTR_PARITY_ERROR   :1;      ///<BIT [4] cmn_axi_mstr_parity_error
        uint32_t RSVD                        :1;      ///<BIT [5] rsvd_0
        uint32_t CMN_AXI_MONITOR_ERROR_CHK_EN :1;      ///<BIT [6] cmn_axi_monitor_error_chk_en
        uint32_t CMN_AXI_MONITOR_ERROR_CLR   :1;      ///<BIT [7] cmn_axi_monitor_error_clr
        uint32_t CMN_AXI_MSTR_CPTRD_ERROR_STATUS :20;     ///<BIT [27:8] cmn_axi_mstr_cptrd_error_status
        uint32_t UCD_CMPST_ERROR_STATUS      :4;      ///<BIT [31:28] ucd_cmpst_error_status
    } b;
} UcdGenCmnSnglCommonUcdErrorControlAndStatus_t;

/// @brief 0xFB0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMN_AXI_IB_DC_MAX_RD_REQ_CNT :6;      ///<BIT [5:0] cmn_axi_ib_dc_max_rd_req_cnt
        uint32_t CMN_AXI_IB_CC_MAX_RD_REQ_CNT :6;      ///<BIT [11:6] cmn_axi_ib_cc_max_rd_req_cnt
        uint32_t CMN_AXI_OB_DC_MAX_RD_REQ_CNT :6;      ///<BIT [17:12] cmn_axi_ob_dc_max_rd_req_cnt
        uint32_t CMN_AXI_OB_CC_MAX_RD_REQ_CNT :6;      ///<BIT [23:18] cmn_axi_ob_cc_max_rd_req_cnt
        uint32_t CMN_AXI_MSTR_MAX_RD_REQ_CNT :8;      ///<BIT [31:24] cmn_axi_mstr_max_rd_req_cnt
    } b;
} UcdGenCmnSnglCommonAxiMaxReadReqCount_t;

/// @brief 0xFB4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMN_AXI_IB_DC_MAX_WR_REQ_CNT :6;      ///<BIT [5:0] cmn_axi_ib_dc_max_wr_req_cnt
        uint32_t CMN_AXI_IB_CC_MAX_WR_REQ_CNT :6;      ///<BIT [11:6] cmn_axi_ib_cc_max_wr_req_cnt
        uint32_t CMN_AXI_OB_DC_MAX_WR_REQ_CNT :6;      ///<BIT [17:12] cmn_axi_ob_dc_max_wr_req_cnt
        uint32_t CMN_AXI_OB_CC_MAX_WR_REQ_CNT :6;      ///<BIT [23:18] cmn_axi_ob_cc_max_wr_req_cnt
        uint32_t CMN_AXI_MSTR_MAX_WR_REQ_CNT :8;      ///<BIT [31:24] cmn_axi_mstr_max_wr_req_cnt
    } b;
} UcdGenCmnSnglCommonAxiMaxWriteReqCount_t;

/// @brief 0xFB8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMN_AXI_IB_DC_MAX_WR_REQ_CNT_REACHED :1;      ///<BIT [0] cmn_axi_ib_dc_max_wr_req_cnt_reached
        uint32_t CMN_AXI_IB_CC_MAX_WR_REQ_CNT_REACHED :1;      ///<BIT [1] cmn_axi_ib_cc_max_wr_req_cnt_reached
        uint32_t CMN_AXI_OB_DC_MAX_WR_REQ_CNT_REACHED :1;      ///<BIT [2] cmn_axi_ob_dc_max_wr_req_cnt_reached
        uint32_t CMN_AXI_OB_CC_MAX_WR_REQ_CNT_REACHED :1;      ///<BIT [3] cmn_axi_ob_cc_max_wr_req_cnt_reached
        uint32_t CMN_AXI_IB_DC_MAX_RD_REQ_CNT_REACHED :1;      ///<BIT [4] cmn_axi_ib_dc_max_rd_req_cnt_reached
        uint32_t CMN_AXI_IB_CC_MAX_RD_REQ_CNT_REACHED :1;      ///<BIT [5] cmn_axi_ib_cc_max_rd_req_cnt_reached
        uint32_t CMN_AXI_OB_DC_MAX_RD_REQ_CNT_REACHED :1;      ///<BIT [6] cmn_axi_ob_dc_max_rd_req_cnt_reached
        uint32_t CMN_AXI_OB_CC_MAX_RD_REQ_CNT_REACHED :1;      ///<BIT [7] cmn_axi_ob_cc_max_rd_req_cnt_reached
        uint32_t CMN_AXI_MSTR_MAX_WR_REQ_CNT_REACHED :1;      ///<BIT [8] cmn_axi_mstr_max_wr_req_cnt_reached
        uint32_t CMN_AXI_MSTR_MAX_RD_REQ_CNT_REACHED :1;      ///<BIT [9] cmn_axi_mstr_max_rd_req_cnt_reached
        uint32_t RSVD                        :22;     ///<BIT [31:10] rsvd_0
    } b;
} UcdGenCmnSnglCommonAxiRequestStatus_t;

/// @brief 0xFC0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMN_MSIX_VECTOR_COALESCING_MODE :2;      ///<BIT [1:0] cmn_msix_vector_coalescing_mode
        uint32_t RSVD_2                      :2;      ///<BIT [3:2] rsvd_2
        uint32_t CMN_AXI_SLV_PARITY_CHK_EN   :1;      ///<BIT [4] cmn_axi_slv_parity_chk_en
        uint32_t CMN_AXI_SLV_ODD_PARITY_EN   :1;      ///<BIT [5] cmn_axi_slv_odd_parity_en
        uint32_t CMN_AXI_SLV_ERROR           :1;      ///<BIT [6] cmn_axi_slv_error
        uint32_t RSVD_1                      :9;      ///<BIT [15:7] rsvd_1
        uint32_t CMN_AXI_SLV_CPTRD_ERROR_INFO :12;     ///<BIT [27:16] cmn_axi_slv_cptrd_error_info
        uint32_t RSVD                        :1;      ///<BIT [28] rsvd_0
        uint32_t CMN_DPATH_HALT_ERROR_MASK   :3;      ///<BIT [31:29] cmn_dpath_halt_error_mask
    } b;
} UcdGenCmnSnglUcdTopCfg_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LOGICAL_ADDR_OF_IQ_PRODUCER_INDX :20;     ///<BIT [19:0] logical_addr_of_iq_producer_indx
        uint32_t RSVD                        :11;     ///<BIT [30:20] rsvd_0
        uint32_t LOGICAL_ADDR_INVALID        :1;      ///<BIT [31] logical_addr_invalid
    } b;
} UcdGenCmnIbLgc2physIqLogicalToPhysicalAssignment_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LOGICAL_ADDR_OF_OQ_CONSUMER_INDX :20;     ///<BIT [19:0] logical_addr_of_oq_consumer_indx
        uint32_t RSVD                        :11;     ///<BIT [30:20] rsvd_0
        uint32_t LOGICAL_ADDR_INVALID        :1;      ///<BIT [31] logical_addr_invalid
    } b;
} UcdGenCmnObLgc2physOqLogicalToPhysicalAssignment_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD                        :31;     ///<BIT [30:0] rsvd_0
        uint32_t IB_UCD_RST                  :1;      ///<BIT [31] ib_ucd_rst
    } b;
} UcducdIbCmnSnglInboundUcdCfg_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_UCD_ENBL                 :1;      ///<BIT [0] ib_ucd_enbl
        uint32_t IB_UCD_PAUSE                :1;      ///<BIT [1] ib_ucd_pause
        uint32_t RSVD                        :30;     ///<BIT [31:2] rsvd_0
    } b;
} UcducdIbCmnSnglInboundUcdControl_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_UCD_ENBLD                :1;      ///<BIT [0] ib_ucd_enbld
        uint32_t IB_UCD_PAUSED               :1;      ///<BIT [1] ib_ucd_paused
        uint32_t IB_UCD_HLTD                 :1;      ///<BIT [2] ib_ucd_hltd
        uint32_t RSVD_2                      :5;      ///<BIT [7:3] rsvd_2
        uint32_t IB_UCD_PAUSED_RSN_TXN_SM    :1;      ///<BIT [8] ib_ucd_paused_rsn_txn_sm
        uint32_t IB_UCD_PAUSED_RSN_CQ_FULL   :1;      ///<BIT [9] ib_ucd_paused_rsn_cq_full
        uint32_t RSVD_1                      :2;      ///<BIT [11:10] rsvd_1
        uint32_t IB_UCD_BUSY                 :1;      ///<BIT [12] ib_ucd_busy
        uint32_t RSVD                        :19;     ///<BIT [31:13] rsvd_0
    } b;
} UcducdIbCmnSnglInboundUcdStatus_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FP1_CMPLTN_Q_IRQ            :1;      ///<BIT [0] fp1_cmpltn_q_irq
        uint32_t FP2_CMPLTN_Q_IRQ            :1;      ///<BIT [1] fp2_cmpltn_q_irq
        uint32_t RR1_CMPLTN_Q_IRQ            :1;      ///<BIT [2] rr1_cmpltn_q_irq
        uint32_t RR2_CMPLTN_Q_IRQ            :1;      ///<BIT [3] rr2_cmpltn_q_irq
        uint32_t RR3_CMPLTN_Q_IRQ            :1;      ///<BIT [4] rr3_cmpltn_q_irq
        uint32_t RSVD                        :2;      ///<BIT [6:5] rsvd_0
        uint32_t IB_INTRNL_HW_ERR            :1;      ///<BIT [7] ib_intrnl_hw_err
        uint32_t FP1_CMPLTN_Q_FULL_IRQ       :1;      ///<BIT [8] fp1_cmpltn_q_full_irq
        uint32_t FP2_CMPLTN_Q_FULL_IRQ       :1;      ///<BIT [9] fp2_cmpltn_q_full_irq
        uint32_t RR1_CMPLTN_Q_FULL_IRQ       :1;      ///<BIT [10] rr1_cmpltn_q_full_irq
        uint32_t RR2_CMPLTN_Q_FULL_IRQ       :1;      ///<BIT [11] rr2_cmpltn_q_full_irq
        uint32_t RR3_CMPLTN_Q_FULL_IRQ       :1;      ///<BIT [12] rr3_cmpltn_q_full_irq
        uint32_t IB_Q_SOFT_ERR_IRQ           :1;      ///<BIT [13] ib_q_soft_err_irq
        uint32_t RSVD_1                      :3;      ///<BIT [16:14] rsvd_1
        uint32_t IB_DEST_FREE_LIST_0_EMPTY_IRQ :1;      ///<BIT [17] ib_dest_free_list_0_empty_irq
        uint32_t IB_DEST_FREE_LIST_1_EMPTY_IRQ :1;      ///<BIT [18] ib_dest_free_list_1_empty_irq
        uint32_t IB_DEST_FREE_LIST_2_EMPTY_IRQ :1;      ///<BIT [19] ib_dest_free_list_2_empty_irq
        uint32_t IB_DEST_FREE_LIST_3_EMPTY_IRQ :1;      ///<BIT [20] ib_dest_free_list_3_empty_irq
        uint32_t IB_DEST_FREE_LIST_4_EMPTY_IRQ :1;      ///<BIT [21] ib_dest_free_list_4_empty_irq
        uint32_t IB_DEST_FREE_LIST_5_EMPTY_IRQ :1;      ///<BIT [22] ib_dest_free_list_5_empty_irq
        uint32_t IB_DEST_FREE_LIST_0_OVRFLW_IRQ :1;      ///<BIT [23] ib_dest_free_list_0_ovrflw_irq
        uint32_t IB_DEST_FREE_LIST_1_OVRFLW_IRQ :1;      ///<BIT [24] ib_dest_free_list_1_ovrflw_irq
        uint32_t IB_DEST_FREE_LIST_2_OVRFLW_IRQ :1;      ///<BIT [25] ib_dest_free_list_2_ovrflw_irq
        uint32_t IB_DEST_FREE_LIST_3_OVRFLW_IRQ :1;      ///<BIT [26] ib_dest_free_list_3_ovrflw_irq
        uint32_t IB_DEST_FREE_LIST_4_OVRFLW_IRQ :1;      ///<BIT [27] ib_dest_free_list_4_ovrflw_irq
        uint32_t IB_DEST_FREE_LIST_5_OVRFLW_IRQ :1;      ///<BIT [28] ib_dest_free_list_5_ovrflw_irq
        uint32_t IB_CONTROL_PATH_ERR         :1;      ///<BIT [29] ib_control_path_err
        uint32_t IB_INTRNL_MEM_PERR          :1;      ///<BIT [30] ib_intrnl_mem_perr
        uint32_t IB_DATA_PATH_ERR            :1;      ///<BIT [31] ib_data_path_err
    } b;
} UcducdIbCmnSnglInboundUcdIntrCause_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_UCD_SRAM_PERR            :1;      ///<BIT [0] ib_ucd_sram_perr
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} UcducdIbCmnSnglInboundUcdSramParityErrorCause_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_UCD_SRAM_PERR_EN         :1;      ///<BIT [0] ib_ucd_sram_perr_en
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} UcducdIbCmnSnglInboundUcdSramParityErrorEnable_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_DP_PAR                   :1;      ///<BIT [0] ib_dp_par
        uint32_t IB_DP_PERR_EN               :1;      ///<BIT [1] ib_dp_perr_en
        uint32_t IB_DP_FERR_EN               :1;      ///<BIT [2] ib_dp_ferr_en
        uint32_t IB_DP_RD_TXN_ERR_DSBL       :3;      ///<BIT [5:3] ib_dp_rd_txn_err_dsbl
        uint32_t IB_DP_WR_TXN_ERR_DSBL       :2;      ///<BIT [7:6] ib_dp_wr_txn_err_dsbl
        uint32_t RSVD                        :6;      ///<BIT [13:8] rsvd_0
        uint32_t IB_FRC_DP_PERR_CONT         :1;      ///<BIT [14] ib_frc_dp_perr_cont
        uint32_t IB_FRC_DP_PERR_ONCE         :1;      ///<BIT [15] ib_frc_dp_perr_once
        uint32_t IB_DP_PRTY_MASK             :16;     ///<BIT [31:16] ib_dp_prty_mask
    } b;
} UcducdIbCmnSnglInboundUcdDataPathErrorControl_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_1                      :1;      ///<BIT [0] rsvd_1
        uint32_t IB_DP_PERR                  :1;      ///<BIT [1] ib_dp_perr
        uint32_t IB_DP_FERR                  :1;      ///<BIT [2] ib_dp_ferr
        uint32_t IB_DP_RD_TXN_ERR            :3;      ///<BIT [5:3] ib_dp_rd_txn_err
        uint32_t IB_DP_WR_TXN_ERR            :2;      ///<BIT [7:6] ib_dp_wr_txn_err
        uint32_t RSVD                        :16;     ///<BIT [23:8] rsvd_0
        uint32_t IB_ERR_PORT                 :8;      ///<BIT [31:24] ib_err_port
    } b;
} UcducdIbCmnSnglInboundUcdDataPathErrorStatus_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ARB_BURST                   :3;      ///<BIT [2:0] arb_burst
        uint32_t RSVD                        :5;      ///<BIT [7:3] rsvd_0
        uint32_t RR_3_WEIGHT                 :8;      ///<BIT [15:8] rr_3_weight
        uint32_t RR_2_WEIGHT                 :8;      ///<BIT [23:16] rr_2_weight
        uint32_t RR_1_WEIGHT                 :8;      ///<BIT [31:24] rr_1_weight
    } b;
} UcducdIbCmnSnglInboundQueueArbitrationCfgRegister_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_DEST_FREE_LIST_EN        :1;      ///<BIT [0] ib_dest_free_list_en
        uint32_t RSVD_2                      :7;      ///<BIT [7:1] rsvd_2
        uint32_t IB_DEST_FREE_LIST_SIZE      :4;      ///<BIT [11:8] ib_dest_free_list_size
        uint32_t RSVD_1                      :4;      ///<BIT [15:12] rsvd_1
        uint32_t IB_DEST_FREE_LIST_BFFR_LNGTH :12;     ///<BIT [27:16] ib_dest_free_list_bffr_lngth
        uint32_t RSVD                        :4;      ///<BIT [31:28] rsvd_0
    } b;
} UcducdIbCmnDflInboundDestinationFreeListCfg0_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_DEST_FREE_LIST_BFFR_IFC_SLCT :8;      ///<BIT [7:0] ib_dest_free_list_bffr_ifc_slct
        uint32_t IB_DEST_FREE_LIST_LIST_IFC_SLCT :8;      ///<BIT [15:8] ib_dest_free_list_list_ifc_slct
        uint32_t RSVD                        :16;     ///<BIT [31:16] rsvd_0
    } b;
} UcducdIbCmnDflInboundDestinationFreeListCfg1_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD                        :3;      ///<BIT [2:0] rsvd_0
        uint32_t IB_DEST_FREE_LIST_BASE_ADDR_LO :29;     ///<BIT [31:3] ib_dest_free_list_base_addr_lo
    } b;
} UcducdIbCmnDflInboundDestinationFreeListBaseAddrLo_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_DEST_FREE_LIST_PI        :16;     ///<BIT [15:0] ib_dest_free_list_pi
        uint32_t RSVD                        :16;     ///<BIT [31:16] rsvd_0
    } b;
} UcducdIbCmnDflInboundDestinationFreeListPi_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_DEST_FREE_LIST_CI        :16;     ///<BIT [15:0] ib_dest_free_list_ci
        uint32_t RSVD                        :16;     ///<BIT [31:16] rsvd_0
    } b;
} UcducdIbCmnDflInboundDestinationFreeListCi_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_DEST_FREE_LIST_EMPTY     :1;      ///<BIT [0] ib_dest_free_list_empty
        uint32_t IB_DEST_FREE_LIST_FULL      :1;      ///<BIT [1] ib_dest_free_list_full
        uint32_t IB_DEST_FREE_LIST_OVRFLW    :1;      ///<BIT [2] ib_dest_free_list_ovrflw
        uint32_t RSVD_1                      :5;      ///<BIT [7:3] rsvd_1
        uint32_t IB_DEST_FREE_LIST_MEM_EMPTY :1;      ///<BIT [8] ib_dest_free_list_mem_empty
        uint32_t IB_DEST_FREE_LIST_FIFO_EMPTY :1;      ///<BIT [9] ib_dest_free_list_fifo_empty
        uint32_t RSVD                        :22;     ///<BIT [31:10] rsvd_0
    } b;
} UcducdIbCmnDflInboundDestinationFreeListStatus_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q_EN                 :1;      ///<BIT [0] cmpltn_q_en
        uint32_t CMPLTN_Q_SHDW_EN            :1;      ///<BIT [1] cmpltn_q_shdw_en
        uint32_t RSVD_2                      :6;      ///<BIT [7:2] rsvd_2
        uint32_t CMPLTN_Q_SIZE               :4;      ///<BIT [11:8] cmpltn_q_size
        uint32_t RSVD_1                      :4;      ///<BIT [15:12] rsvd_1
        uint32_t CMPLTN_Q_IFC_SLCT           :8;      ///<BIT [23:16] cmpltn_q_ifc_slct
        uint32_t RSVD                        :8;      ///<BIT [31:24] rsvd_0
    } b;
} UcducdIbCmnCqCompletionQueueCfgControl_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q_MAX_COAL_TIME      :16;     ///<BIT [15:0] cmpltn_q_max_coal_time
        uint32_t CMPLTN_Q_MIN_COAL_TIME      :16;     ///<BIT [31:16] cmpltn_q_min_coal_time
    } b;
} UcducdIbCmnCqCompletionQueueIntrCoalescing0_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q_INT_COAL_COUNT     :16;     ///<BIT [15:0] cmpltn_q_int_coal_count
        uint32_t CMPLTN_Q_EN_INT_COAL        :1;      ///<BIT [16] cmpltn_q_en_int_coal
        uint32_t RSVD_2                      :1;      ///<BIT [17] rsvd_2
        uint32_t CMPLTN_Q_RESTART_WHEN_CI_UPDT :1;      ///<BIT [18] cmpltn_q_restart_when_ci_updt
        uint32_t RSVD_1                      :1;      ///<BIT [19] rsvd_1
        uint32_t RSVD                        :12;     ///<BIT [31:20] rsvd_0
    } b;
} UcducdIbCmnCqCompletionQueueIntrCoalescing1_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD                        :2;      ///<BIT [1:0] rsvd_0
        uint32_t CMPLTN_Q_PI_SHDW_BASE_ADDR_LO :30;     ///<BIT [31:2] cmpltn_q_pi_shdw_base_addr_lo
    } b;
} UcducdIbCmnCqCompletionQueuePiShadowBaseAddrLo_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD                        :4;      ///<BIT [3:0] rsvd_0
        uint32_t CMPLTN_Q_BASE_ADDR_LO       :28;     ///<BIT [31:4] cmpltn_q_base_addr_lo
    } b;
} UcducdIbCmnCqCompletionQueueBaseAddrLo_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q_PI                 :16;     ///<BIT [15:0] cmpltn_q_pi
        uint32_t RSVD                        :16;     ///<BIT [31:16] rsvd_0
    } b;
} UcducdIbCmnCqCompletionQueuePi_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q_CI                 :16;     ///<BIT [15:0] cmpltn_q_ci
        uint32_t RSVD_1                      :14;     ///<BIT [29:16] rsvd_1
        uint32_t INTRPT_CLR                  :1;      ///<BIT [30] intrpt_clr
        uint32_t RSVD                        :1;      ///<BIT [31] rsvd_0
    } b;
} UcducdIbCmnCqCompletionQueueCi_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMPLTN_Q_EMPTY              :1;      ///<BIT [0] cmpltn_q_empty
        uint32_t CMPLTN_Q_FULL               :1;      ///<BIT [1] cmpltn_q_full
        uint32_t RSVD_1                      :6;      ///<BIT [7:2] rsvd_1
        uint32_t CMPLTN_Q_CS_LKHD_EMPTY      :1;      ///<BIT [8] cmpltn_q_cs_lkhd_empty
        uint32_t CMPLTN_Q_CS_LKHD_FULL       :1;      ///<BIT [9] cmpltn_q_cs_lkhd_full
        uint32_t CMPLTN_Q_CS_FULL            :1;      ///<BIT [10] cmpltn_q_cs_full
        uint32_t RSVD                        :5;      ///<BIT [15:11] rsvd_0
        uint32_t CMPLTN_Q_LKHD_PI            :16;     ///<BIT [31:16] cmpltn_q_lkhd_pi
    } b;
} UcducdIbCmnCqCompletionQueueStatus_t;

/// @brief 0x300
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DATA_FIFO_MEM_RF2P_RTC      :2;      ///<BIT [1:0] data_fifo_mem_rf2p_rtc
        uint32_t DATA_FIFO_MEM_RF2P_WTC      :2;      ///<BIT [3:2] data_fifo_mem_rf2p_wtc
        uint32_t DFL_NOT_EMPTY_WAIT_TMR_SEL  :3;      ///<BIT [6:4] dfl_not_empty_wait_tmr_sel
        uint32_t CQ_NOT_FULL_WAIT_TMR_SEL    :3;      ///<BIT [9:7] cq_not_full_wait_tmr_sel
        uint32_t ENBL_DFL_EMPTY_PAUSE        :1;      ///<BIT [10] enbl_dfl_empty_pause
        uint32_t ENBL_CQ_FULL_PAUSE          :1;      ///<BIT [11] enbl_cq_full_pause
        uint32_t DSBL_CQ_CLR_CQ_UPDT         :1;      ///<BIT [12] dsbl_cq_clr_cq_updt
        uint32_t ENBL_CQ_CLR_CQ_EMPTY        :1;      ///<BIT [13] enbl_cq_clr_cq_empty
        uint32_t RSVD                        :6;      ///<BIT [19:14] rsvd_0
        uint32_t DSBL_AXI_ERR_PROPAGATION    :1;      ///<BIT [20] dsbl_axi_err_propagation
        uint32_t DP_DIAG_HALT                :1;      ///<BIT [21] dp_diag_halt
        uint32_t ENBL_FSC_SM_SYNC            :1;      ///<BIT [22] enbl_fsc_sm_sync
        uint32_t ENBL_Q_ACC_WHILE_DSBLD_ERR  :1;      ///<BIT [23] enbl_q_acc_while_dsbld_err
        uint32_t DSBL_DBELL_COMPLIANCE_CHK_ERR :3;      ///<BIT [26:24] dsbl_dbell_compliance_chk_err
        uint32_t RSVD_1                      :4;      ///<BIT [30:27] rsvd_1
        uint32_t MISC_ERROR_STATUS_CLR       :1;      ///<BIT [31] misc_error_status_clr
    } b;
} UcducdIbCmnSnglInboundMiscellaneousControl_t;

/// @brief 0x310
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DIAG_READ_GROUP_SEL         :8;      ///<BIT [7:0] diag_read_group_sel
        uint32_t DIAG_FIFO_RD_PTR            :8;      ///<BIT [15:8] diag_fifo_rd_ptr
        uint32_t DIAG_HW_CTL                 :8;      ///<BIT [23:16] diag_hw_ctl
        uint32_t RSVD                        :8;      ///<BIT [31:24] rsvd_0
    } b;
} UcducdIbCmnSnglInboundDiagnosticControl_t;

/// @brief 0x320
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DC_AXI_ARUSER_INFO          :4;      ///<BIT [3:0] dc_axi_aruser_info
        uint32_t RSVD_2                      :12;     ///<BIT [15:4] rsvd_2
        uint32_t DC_AXI_ARCACHE              :4;      ///<BIT [19:16] dc_axi_arcache
        uint32_t RSVD_1                      :4;      ///<BIT [23:20] rsvd_1
        uint32_t DC_AXI_MAX_RD_BURST_BYTE_CNT_SEL :2;      ///<BIT [25:24] dc_axi_max_rd_burst_byte_cnt_sel
        uint32_t RSVD                        :6;      ///<BIT [31:26] rsvd_0
    } b;
} UcducdIbCmnSnglInboundDataChannelAxiReadBusAttrs_t;

/// @brief 0x324
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DC_AXI_AWUSER_INFO          :5;      ///<BIT [4:0] dc_axi_awuser_info
        uint32_t RSVD_2                      :11;     ///<BIT [15:5] rsvd_2
        uint32_t DC_AXI_AWCACHE              :4;      ///<BIT [19:16] dc_axi_awcache
        uint32_t RSVD_1                      :4;      ///<BIT [23:20] rsvd_1
        uint32_t DC_AXI_MAX_WR_BURST_BYTE_CNT_SEL :2;      ///<BIT [25:24] dc_axi_max_wr_burst_byte_cnt_sel
        uint32_t RSVD                        :5;      ///<BIT [30:26] rsvd_0
        uint32_t DC_AXI_AWCACHE0_AUTO_GEN_DISABLE :1;      ///<BIT [31] dc_axi_awcache0_auto_gen_disable
    } b;
} UcducdIbCmnSnglInboundDataChannelAxiWriteBusAttrs_t;

/// @brief 0x328
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CC_AXI_ARUSER_INFO          :4;      ///<BIT [3:0] cc_axi_aruser_info
        uint32_t RSVD_1                      :12;     ///<BIT [15:4] rsvd_1
        uint32_t CC_AXI_ARCACHE              :4;      ///<BIT [19:16] cc_axi_arcache
        uint32_t RSVD                        :12;     ///<BIT [31:20] rsvd_0
    } b;
} UcducdIbCmnSnglInboundControlChannelAxiReadBusAttrs_t;

/// @brief 0x32C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CC_AXI_AWUSER_INFO          :5;      ///<BIT [4:0] cc_axi_awuser_info
        uint32_t RSVD_1                      :11;     ///<BIT [15:5] rsvd_1
        uint32_t CC_AXI_AWCACHE              :4;      ///<BIT [19:16] cc_axi_awcache
        uint32_t RSVD                        :12;     ///<BIT [31:20] rsvd_0
    } b;
} UcducdIbCmnSnglInboundControlChannelAxiWriteBusAttrs_t;

/// @brief 0x350
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IB_Q_131_128_SOFT_ERR       :4;      ///<BIT [3:0] ib_q_131_128_soft_err
        uint32_t RSVD                        :28;     ///<BIT [31:4] rsvd_0
    } b;
} UcducdIbCmnSnglInboundQueueSoftError4_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HF_CREDIT_COUNT             :10;     ///<BIT [9:0] hf_credit_count
        uint32_t RSVD0                       :22;     ///<BIT [31:10] rsvd0
    } b;
} UcducdIbHfCreditUcdInboundHfCreditCount_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IQ_EN                       :1;      ///<BIT [0] iq_en
        uint32_t RSVD_1                      :2;      ///<BIT [2:1] rsvd_1
        uint32_t IQ_RST                      :1;      ///<BIT [3] iq_rst
        uint32_t IQ_PRIORITY                 :3;      ///<BIT [6:4] iq_priority
        uint32_t IQ_PASS_THRU_MODE_EN        :1;      ///<BIT [7] iq_pass_thru_mode_en
        uint32_t IQ_FREE_LIST_SLCT           :3;      ///<BIT [10:8] iq_free_list_slct
        uint32_t IQ_INTRNL_MEM_BFR_EN        :1;      ///<BIT [11] iq_intrnl_mem_bfr_en
        uint32_t RSVD                        :2;      ///<BIT [13:12] rsvd_0
        uint32_t IQ_CREDIT_POLICY_EN         :2;      ///<BIT [15:14] iq_credit_policy_en
        uint32_t IQ_NM_ELMNTS                :16;     ///<BIT [31:16] iq_nm_elmnts
    } b;
} UcducdIbCmnIqInboundQueueCfg0_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IQ_HF_CREDIT_COUNTER_SLCT   :6;      ///<BIT [5:0] iq_hf_credit_counter_slct
        uint32_t RSVD                        :2;      ///<BIT [7:6] rsvd_0
        uint32_t IQ_IFC_SLCT                 :8;      ///<BIT [15:8] iq_ifc_slct
        uint32_t IQ_HOST_LOGICAL_ID          :16;     ///<BIT [31:16] iq_host_logical_id
    } b;
} UcducdIbCmnIqInboundQueueCfg1_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IQ_CREDIT_COUNT             :8;      ///<BIT [7:0] iq_credit_count
        uint32_t RSVD                        :24;     ///<BIT [31:8] rsvd_0
    } b;
} UcducdIbCmnIqInboundQueueCreditCount_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD                        :2;      ///<BIT [1:0] rsvd_0
        uint32_t IQ_BASE_ADDR_LO             :30;     ///<BIT [31:2] iq_base_addr_lo
    } b;
} UcducdIbCmnIqInboundQueueBaseAddressLow_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IQ_PRDCR_INDX               :16;     ///<BIT [15:0] iq_prdcr_indx
        uint32_t RSVD                        :16;     ///<BIT [31:16] rsvd_0
    } b;
} UcducdIbCmnIqInboundQueuePi_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IQ_CNSMR_INDX               :16;     ///<BIT [15:0] iq_cnsmr_indx
        uint32_t RSVD                        :16;     ///<BIT [31:16] rsvd_0
    } b;
} UcducdIbCmnIqInboundQueueCi_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IQ_EMPTY                    :1;      ///<BIT [0] iq_empty
        uint32_t IQ_FULL                     :1;      ///<BIT [1] iq_full
        uint32_t IQ_OVRFLW                   :1;      ///<BIT [2] iq_ovrflw
        uint32_t RSVD_2                      :2;      ///<BIT [4:3] rsvd_2
        uint32_t IQ_DBELL_WRITE_OUT_OF_RANGE_ERR :1;      ///<BIT [5] iq_dbell_write_out_of_range_err
        uint32_t IQ_DBELL_WRITE_SAME_VALUE_ERR :1;      ///<BIT [6] iq_dbell_write_same_value_err
        uint32_t IQ_DBELL_WRITE_ADD_CMD_TO_FULL_SQ_ERR :1;      ///<BIT [7] iq_dbell_write_add_cmd_to_full_sq_err
        uint32_t RSVD_1                      :2;      ///<BIT [9:8] rsvd_1
        uint32_t IQ_LKHD_EMPTY               :1;      ///<BIT [10] iq_lkhd_empty
        uint32_t IQ_LKHD_FULL                :1;      ///<BIT [11] iq_lkhd_full
        uint32_t IQ_BUSY                     :1;      ///<BIT [12] iq_busy
        uint32_t RSVD                        :3;      ///<BIT [15:13] rsvd_0
        uint32_t IQ_LKHD_CNSMR_INDX          :16;     ///<BIT [31:16] iq_lkhd_cnsmr_indx
    } b;
} UcducdIbCmnIqInboundQueueStatus_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD                        :31;     ///<BIT [30:0] rsvd_0
        uint32_t OB_UCD_RST                  :1;      ///<BIT [31] ob_ucd_rst
    } b;
} UcducdObCmnSnglOutboundUcdCfg_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_UCD_ENBL                 :1;      ///<BIT [0] ob_ucd_enbl
        uint32_t OB_UCD_PAUSE                :1;      ///<BIT [1] ob_ucd_pause
        uint32_t RSVD                        :14;     ///<BIT [15:2] rsvd_0
        uint32_t OB_IRQ_SRVC_WAIT_TIME       :16;     ///<BIT [31:16] ob_irq_srvc_wait_time
    } b;
} UcducdObCmnSnglOutboundUcdControl_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_UCD_ENBLD                :1;      ///<BIT [0] ob_ucd_enbld
        uint32_t OB_UCD_PAUSED               :1;      ///<BIT [1] ob_ucd_paused
        uint32_t OB_UCD_HALTED               :1;      ///<BIT [2] ob_ucd_halted
        uint32_t RSVD_1                      :5;      ///<BIT [7:3] rsvd_1
        uint32_t OB_UCD_PAUSED_RSN_TXN_SM    :1;      ///<BIT [8] ob_ucd_paused_rsn_txn_sm
        uint32_t OB_UCD_PAUSED_RSN_CQ_FULL   :1;      ///<BIT [9] ob_ucd_paused_rsn_cq_full
        uint32_t RSVD                        :22;     ///<BIT [31:10] rsvd_0
    } b;
} UcducdObCmnSnglOutboundUcdStatus_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_FP1_CMPLTN_Q_IRQ         :1;      ///<BIT [0] ob_fp1_cmpltn_q_irq
        uint32_t OB_FP2_CMPLTN_Q_IRQ         :1;      ///<BIT [1] ob_fp2_cmpltn_q_irq
        uint32_t OB_RR1_CMPLTN_Q_IRQ         :1;      ///<BIT [2] ob_rr1_cmpltn_q_irq
        uint32_t OB_RR2_CMPLTN_Q_IRQ         :1;      ///<BIT [3] ob_rr2_cmpltn_q_irq
        uint32_t OB_RR3_CMPLTN_Q_IRQ         :1;      ///<BIT [4] ob_rr3_cmpltn_q_irq
        uint32_t RSVD_2                      :2;      ///<BIT [6:5] rsvd_2
        uint32_t OB_INTRNL_HW_ERR            :1;      ///<BIT [7] ob_intrnl_hw_err
        uint32_t OB_Q_SOFT_ERR_IRQ           :1;      ///<BIT [8] ob_q_soft_err_irq
        uint32_t RSVD_1                      :3;      ///<BIT [11:9] rsvd_1
        uint32_t OB_FP1_SRC_LIST_OVRFLW_IRQ  :1;      ///<BIT [12] ob_fp1_src_list_ovrflw_irq
        uint32_t OB_FP2_SRC_LIST_OVRFLW_IRQ  :1;      ///<BIT [13] ob_fp2_src_list_ovrflw_irq
        uint32_t OB_RR1_SRC_LIST_OVRFLW_IRQ  :1;      ///<BIT [14] ob_rr1_src_list_ovrflw_irq
        uint32_t OB_RR2_SRC_LIST_OVRFLW_IRQ  :1;      ///<BIT [15] ob_rr2_src_list_ovrflw_irq
        uint32_t OB_RR3_SRC_LIST_OVRFLW_IRQ  :1;      ///<BIT [16] ob_rr3_src_list_ovrflw_irq
        uint32_t OB_FP1_CMPLTN_Q_FULL_IRQ    :1;      ///<BIT [17] ob_fp1_cmpltn_q_full_irq
        uint32_t OB_FP2_CMPLTN_Q_FULL_IRQ    :1;      ///<BIT [18] ob_fp2_cmpltn_q_full_irq
        uint32_t OB_RR1_CMPLTN_Q_FULL_IRQ    :1;      ///<BIT [19] ob_rr1_cmpltn_q_full_irq
        uint32_t OB_RR2_CMPLTN_Q_FULL_IRQ    :1;      ///<BIT [20] ob_rr2_cmpltn_q_full_irq
        uint32_t OB_RR3_CMPLTN_Q_FULL_IRQ    :1;      ///<BIT [21] ob_rr3_cmpltn_q_full_irq
        uint32_t RSVD                        :7;      ///<BIT [28:22] rsvd_0
        uint32_t OB_CONTROL_PATH_ERR         :1;      ///<BIT [29] ob_control_path_err
        uint32_t OB_INTRNL_MEM_PERR          :1;      ///<BIT [30] ob_intrnl_mem_perr
        uint32_t OB_DATA_PATH_ERR            :1;      ///<BIT [31] ob_data_path_err
    } b;
} UcducdObCmnSnglOutboundUcdIntrCause_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_UCD_SRAM_PERR            :1;      ///<BIT [0] ob_ucd_sram_perr
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} UcducdObCmnSnglOutboundUcdSramParityErrorCause_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_UCD_SRAM_PERR_EN         :1;      ///<BIT [0] ob_ucd_sram_perr_en
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} UcducdObCmnSnglOutboundUcdSramParityErrorEnable_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_DP_PAR                   :1;      ///<BIT [0] ob_dp_par
        uint32_t OB_DP_PERR_EN               :1;      ///<BIT [1] ob_dp_perr_en
        uint32_t OB_DP_FERR_EN               :1;      ///<BIT [2] ob_dp_ferr_en
        uint32_t OB_DP_RD_TXN_ERR_DSBL       :3;      ///<BIT [5:3] ob_dp_rd_txn_err_dsbl
        uint32_t OB_DP_WR_TXN_ERR_DSBL       :2;      ///<BIT [7:6] ob_dp_wr_txn_err_dsbl
        uint32_t RSVD                        :6;      ///<BIT [13:8] rsvd_0
        uint32_t OB_FRC_DP_PERR_CONT         :1;      ///<BIT [14] ob_frc_dp_perr_cont
        uint32_t OB_FRC_DP_PERR_ONCE         :1;      ///<BIT [15] ob_frc_dp_perr_once
        uint32_t OB_DP_PRTY_MASK             :16;     ///<BIT [31:16] ob_dp_prty_mask
    } b;
} UcducdObCmnSnglOutboundUcdDataPathErrorControl_t;

/// @brief 0x40
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_1                      :1;      ///<BIT [0] rsvd_1
        uint32_t OB_DP_PERR                  :1;      ///<BIT [1] ob_dp_perr
        uint32_t OB_DP_FERR                  :1;      ///<BIT [2] ob_dp_ferr
        uint32_t OB_DP_RD_TXN_ERR            :3;      ///<BIT [5:3] ob_dp_rd_txn_err
        uint32_t OB_DP_WR_TXN_ERR            :2;      ///<BIT [7:6] ob_dp_wr_txn_err
        uint32_t RSVD                        :16;     ///<BIT [23:8] rsvd_0
        uint32_t OB_ERR_PORT                 :8;      ///<BIT [31:24] ob_err_port
    } b;
} UcducdObCmnSnglOutboundUcdDataPathErrorStatus_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ARB_BURST                   :3;      ///<BIT [2:0] arb_burst
        uint32_t RSVD                        :5;      ///<BIT [7:3] rsvd_0
        uint32_t RR_3_WEIGHT                 :8;      ///<BIT [15:8] rr_3_weight
        uint32_t RR_2_WEIGHT                 :8;      ///<BIT [23:16] rr_2_weight
        uint32_t RR_1_WEIGHT                 :8;      ///<BIT [31:24] rr_1_weight
    } b;
} UcducdObCmnSnglOutboundQueueArbitrationCfgRegister_t;

/// @brief 0x60
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_QUEUE_ELMNT_LNGTH_0      :16;     ///<BIT [15:0] ob_queue_elmnt_lngth_0
        uint32_t OB_QUEUE_ELMNT_LNGTH_1      :16;     ///<BIT [31:16] ob_queue_elmnt_lngth_1
    } b;
} UcducdObCmnSnglOutboundSizeSelect0_t;

/// @brief 0x64
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_QUEUE_ELMNT_LNGTH_2      :16;     ///<BIT [15:0] ob_queue_elmnt_lngth_2
        uint32_t OB_QUEUE_ELMNT_LNGTH_3      :16;     ///<BIT [31:16] ob_queue_elmnt_lngth_3
    } b;
} UcducdObCmnSnglOutboundSizeSelect1_t;

/// @brief 0x68
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_QUEUE_ELMNT_LNGTH_4      :16;     ///<BIT [15:0] ob_queue_elmnt_lngth_4
        uint32_t OB_QUEUE_ELMNT_LNGTH_5      :16;     ///<BIT [31:16] ob_queue_elmnt_lngth_5
    } b;
} UcducdObCmnSnglOutboundSizeSelect2_t;

/// @brief 0x6C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_QUEUE_ELMNT_LNGTH_6      :16;     ///<BIT [15:0] ob_queue_elmnt_lngth_6
        uint32_t OB_QUEUE_ELMNT_LNGTH_7      :16;     ///<BIT [31:16] ob_queue_elmnt_lngth_7
    } b;
} UcducdObCmnSnglOutboundSizeSelect3_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_SRC_LIST_EN              :1;      ///<BIT [0] ob_src_list_en
        uint32_t RSVD_2                      :7;      ///<BIT [7:1] rsvd_2
        uint32_t OB_SRC_LIST_SIZE            :4;      ///<BIT [11:8] ob_src_list_size
        uint32_t RSVD_1                      :4;      ///<BIT [15:12] rsvd_1
        uint32_t OB_SRC_LIST_IFC_SLCT        :8;      ///<BIT [23:16] ob_src_list_ifc_slct
        uint32_t RSVD                        :8;      ///<BIT [31:24] rsvd_0
    } b;
} UcducdObCmnOslOutboundSourceListCfg0_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD                        :5;      ///<BIT [4:0] rsvd_0
        uint32_t OB_SRC_LIST_BASE_ADDR_LO    :27;     ///<BIT [31:5] ob_src_list_base_addr_lo
    } b;
} UcducdObCmnOslOutboundSourceListBaseAddrLo_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_SRC_LIST_PI              :16;     ///<BIT [15:0] ob_src_list_pi
        uint32_t RSVD                        :16;     ///<BIT [31:16] rsvd_0
    } b;
} UcducdObCmnOslOutboundSourceListPi_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_SRC_LIST_CI              :16;     ///<BIT [15:0] ob_src_list_ci
        uint32_t RSVD                        :16;     ///<BIT [31:16] rsvd_0
    } b;
} UcducdObCmnOslOutboundSourceListCi_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_SRC_LIST_EMPTY           :1;      ///<BIT [0] ob_src_list_empty
        uint32_t OB_SRC_LIST_FULL            :1;      ///<BIT [1] ob_src_list_full
        uint32_t OB_SRC_LIST_OVRFLW          :1;      ///<BIT [2] ob_src_list_ovrflw
        uint32_t RSVD_1                      :6;      ///<BIT [8:3] rsvd_1
        uint32_t OB_SRC_LIST_FIFO_EMPTY      :1;      ///<BIT [9] ob_src_list_fifo_empty
        uint32_t RSVD                        :22;     ///<BIT [31:10] rsvd_0
    } b;
} UcducdObCmnOslOutboundSourceListStatus_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_CMPLTN_Q_EN              :1;      ///<BIT [0] ob_cmpltn_q_en
        uint32_t OB_CMPLTN_Q_SHDW_EN         :1;      ///<BIT [1] ob_cmpltn_q_shdw_en
        uint32_t RSVD_2                      :6;      ///<BIT [7:2] rsvd_2
        uint32_t OB_CMPLTN_Q_SIZE            :4;      ///<BIT [11:8] ob_cmpltn_q_size
        uint32_t RSVD_1                      :4;      ///<BIT [15:12] rsvd_1
        uint32_t OB_CMPLTN_Q_IFC_SLCT        :8;      ///<BIT [23:16] ob_cmpltn_q_ifc_slct
        uint32_t RSVD                        :8;      ///<BIT [31:24] rsvd_0
    } b;
} UcducdObCmnCqOutboundCompletionQueueCfgControl_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_CMPLTN_Q_MAX_COAL_TIME   :16;     ///<BIT [15:0] ob_cmpltn_q_max_coal_time
        uint32_t OB_CMPLTN_Q_MIN_COAL_TIME   :16;     ///<BIT [31:16] ob_cmpltn_q_min_coal_time
    } b;
} UcducdObCmnCqOutboundCompletionQueueIntrCoalescing0_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_CMPLTN_Q_INT_COAL_COUNT  :16;     ///<BIT [15:0] ob_cmpltn_q_int_coal_count
        uint32_t OB_CMPLTN_Q_EN_INT_COAL     :1;      ///<BIT [16] ob_cmpltn_q_en_int_coal
        uint32_t RSVD_1                      :1;      ///<BIT [17] rsvd_1
        uint32_t OB_CMPLTN_Q_RESTART_WHEN_CI_UPDT :1;      ///<BIT [18] ob_cmpltn_q_restart_when_ci_updt
        uint32_t RSVD                        :13;     ///<BIT [31:19] rsvd_0
    } b;
} UcducdObCmnCqOutboundCompletionQueueIntrCoalescing1_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD                        :2;      ///<BIT [1:0] rsvd_0
        uint32_t OB_CMPLTN_Q_PI_SHDW_BASE_ADDR_LO :30;     ///<BIT [31:2] ob_cmpltn_q_pi_shdw_base_addr_lo
    } b;
} UcducdObCmnCqOutboundCompletionQueuePiShadowBaseAddrLo_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD                        :5;      ///<BIT [4:0] rsvd_0
        uint32_t OB_CMPLTN_Q_BASE_ADDR_LO    :27;     ///<BIT [31:5] ob_cmpltn_q_base_addr_lo
    } b;
} UcducdObCmnCqOutboundCompletionQueueBaseAddrLo_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_CMPLTN_Q_PI              :16;     ///<BIT [15:0] ob_cmpltn_q_pi
        uint32_t RSVD                        :16;     ///<BIT [31:16] rsvd_0
    } b;
} UcducdObCmnCqOutboundCompletionQueuePi_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_CMPLTN_Q_CI              :16;     ///<BIT [15:0] ob_cmpltn_q_ci
        uint32_t RSVD_1                      :14;     ///<BIT [29:16] rsvd_1
        uint32_t OB_CMPLTN_Q_INTRPT_CLR      :1;      ///<BIT [30] ob_cmpltn_q_intrpt_clr
        uint32_t RSVD                        :1;      ///<BIT [31] rsvd_0
    } b;
} UcducdObCmnCqOutboundCompletionQueueCi_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_CMPLTN_Q_EMPTY           :1;      ///<BIT [0] ob_cmpltn_q_empty
        uint32_t OB_CMPLTN_Q_FULL            :1;      ///<BIT [1] ob_cmpltn_q_full
        uint32_t RSVD_1                      :6;      ///<BIT [7:2] rsvd_1
        uint32_t OB_CMPLTN_Q_CS_LKHD_EMPTY   :1;      ///<BIT [8] ob_cmpltn_q_cs_lkhd_empty
        uint32_t OB_CMPLTN_Q_CS_LKHD_FULL    :1;      ///<BIT [9] ob_cmpltn_q_cs_lkhd_full
        uint32_t OB_CMPLTN_Q_CS_FULL         :1;      ///<BIT [10] ob_cmpltn_q_cs_full
        uint32_t RSVD                        :5;      ///<BIT [15:11] rsvd_0
        uint32_t OB_CMPLTN_Q_LKHD_PI         :16;     ///<BIT [31:16] ob_cmpltn_q_lkhd_pi
    } b;
} UcducdObCmnCqOutboundCompletionQueueStatus_t;

/// @brief 0x300
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DATA_FIFO_MEM_RF2P_RTC      :2;      ///<BIT [1:0] data_fifo_mem_rf2p_rtc
        uint32_t DATA_FIFO_MEM_RF2P_WTC      :2;      ///<BIT [3:2] data_fifo_mem_rf2p_wtc
        uint32_t OQ_NOT_FULL_WAIT_TMR_SEL    :3;      ///<BIT [6:4] oq_not_full_wait_tmr_sel
        uint32_t CQ_NOT_FULL_WAIT_TMR_SEL    :3;      ///<BIT [9:7] cq_not_full_wait_tmr_sel
        uint32_t RSVD                        :1;      ///<BIT [10] rsvd_0
        uint32_t ENBL_CQ_FULL_PAUSE          :1;      ///<BIT [11] enbl_cq_full_pause
        uint32_t DSBL_CQ_CLR_CQ_UPDT         :1;      ///<BIT [12] dsbl_cq_clr_cq_updt
        uint32_t ENBL_CQ_CLR_CQ_EMPTY        :1;      ///<BIT [13] enbl_cq_clr_cq_empty
        uint32_t RSVD_1                      :2;      ///<BIT [15:14] rsvd_1
        uint32_t DSBL_ELMNT_SKIP_OQ_FULL     :1;      ///<BIT [16] dsbl_elmnt_skip_oq_full
        uint32_t DSBL_OQ_IRQ_CLR_CI_UPDT     :1;      ///<BIT [17] dsbl_oq_irq_clr_ci_updt
        uint32_t DSBL_OQ_IRQ_CLR_ON_MASK_ENBL :1;      ///<BIT [18] dsbl_oq_irq_clr_on_mask_enbl
        uint32_t DSBL_OQ_IRQ_CLR_ON_EMPTY    :1;      ///<BIT [19] dsbl_oq_irq_clr_on_empty
        uint32_t DSBL_AXI_ERR_PROPAGATION    :1;      ///<BIT [20] dsbl_axi_err_propagation
        uint32_t DP_DIAG_HALT                :1;      ///<BIT [21] dp_diag_halt
        uint32_t ENBL_FSC_SM_SYNC            :1;      ///<BIT [22] enbl_fsc_sm_sync
        uint32_t ENBL_Q_ACC_WHILE_DSBLD_ERR  :1;      ///<BIT [23] enbl_q_acc_while_dsbld_err
        uint32_t DSBL_DBELL_COMPLIANCE_CHK_ERR :3;      ///<BIT [26:24] dsbl_dbell_compliance_chk_err
        uint32_t RSVD_2                      :4;      ///<BIT [30:27] rsvd_2
        uint32_t MISC_ERROR_STATUS_CLR       :1;      ///<BIT [31] misc_error_status_clr
    } b;
} UcducdObCmnSnglOutboundMiscellaneousControl_t;

/// @brief 0x310
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DIAG_READ_GROUP_SEL         :8;      ///<BIT [7:0] diag_read_group_sel
        uint32_t DIAG_FIFO_RD_PTR            :8;      ///<BIT [15:8] diag_fifo_rd_ptr
        uint32_t DIAG_HW_CTL                 :8;      ///<BIT [23:16] diag_hw_ctl
        uint32_t RSVD                        :8;      ///<BIT [31:24] rsvd_0
    } b;
} UcducdObCmnSnglOutboundDiagnosticControl_t;

/// @brief 0x320
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DC_AXI_ARUSER_INFO          :4;      ///<BIT [3:0] dc_axi_aruser_info
        uint32_t RSVD_2                      :12;     ///<BIT [15:4] rsvd_2
        uint32_t DC_AXI_ARCACHE              :4;      ///<BIT [19:16] dc_axi_arcache
        uint32_t RSVD_1                      :4;      ///<BIT [23:20] rsvd_1
        uint32_t DC_AXI_MAX_RD_BURST_BYTE_CNT_SEL :2;      ///<BIT [25:24] dc_axi_max_rd_burst_byte_cnt_sel
        uint32_t RSVD                        :6;      ///<BIT [31:26] rsvd_0
    } b;
} UcducdObCmnSnglOutboundDataChannelAxiReadBusAttrs_t;

/// @brief 0x324
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DC_AXI_AWUSER_INFO          :5;      ///<BIT [4:0] dc_axi_awuser_info
        uint32_t RSVD_2                      :11;     ///<BIT [15:5] rsvd_2
        uint32_t DC_AXI_AWCACHE              :4;      ///<BIT [19:16] dc_axi_awcache
        uint32_t RSVD_1                      :4;      ///<BIT [23:20] rsvd_1
        uint32_t DC_AXI_MAX_WR_BURST_BYTE_CNT_SEL :2;      ///<BIT [25:24] dc_axi_max_wr_burst_byte_cnt_sel
        uint32_t RSVD                        :5;      ///<BIT [30:26] rsvd_0
        uint32_t DC_AXI_AWCACHE0_AUTO_GEN_DISABLE :1;      ///<BIT [31] dc_axi_awcache0_auto_gen_disable
    } b;
} UcducdObCmnSnglOutboundDataChannelAxiWriteBusAttrs_t;

/// @brief 0x328
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CC_AXI_ARUSER_INFO          :4;      ///<BIT [3:0] cc_axi_aruser_info
        uint32_t RSVD_1                      :12;     ///<BIT [15:4] rsvd_1
        uint32_t CC_AXI_ARCACHE              :4;      ///<BIT [19:16] cc_axi_arcache
        uint32_t RSVD                        :12;     ///<BIT [31:20] rsvd_0
    } b;
} UcducdObCmnSnglOutboundControlChannelAxiReadBusAttrs_t;

/// @brief 0x32C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CC_AXI_AWUSER_INFO          :5;      ///<BIT [4:0] cc_axi_awuser_info
        uint32_t RSVD_1                      :11;     ///<BIT [15:5] rsvd_1
        uint32_t CC_AXI_AWCACHE              :4;      ///<BIT [19:16] cc_axi_awcache
        uint32_t RSVD                        :12;     ///<BIT [31:20] rsvd_0
    } b;
} UcducdObCmnSnglOutboundControlChannelAxiWriteBusAttrs_t;

/// @brief 0x350
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_Q_131_128_SOFT_ERR       :4;      ///<BIT [3:0] ob_q_131_128_soft_err
        uint32_t RSVD                        :28;     ///<BIT [31:4] rsvd_0
    } b;
} UcducdObCmnSnglOutboundQueueSoftError4_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OQ_EN                       :1;      ///<BIT [0] oq_en
        uint32_t RSVD_1                      :1;      ///<BIT [1] rsvd_1
        uint32_t OQ_PHS_BIT_EN               :1;      ///<BIT [2] oq_phs_bit_en
        uint32_t OQ_RST                      :1;      ///<BIT [3] oq_rst
        uint32_t OQ_ELMNT_SZ                 :3;      ///<BIT [6:4] oq_elmnt_sz
        uint32_t OQ_PASS_THRU_MODE_EN        :1;      ///<BIT [7] oq_pass_thru_mode_en
        uint32_t OQ_IQ_CI_UPDT_EN            :1;      ///<BIT [8] oq_iq_ci_updt_en
        uint32_t OQ_OFFLINE                  :1;      ///<BIT [9] oq_offline
        uint32_t OQ_IQ_ID_UPDT_EN            :1;      ///<BIT [10] oq_iq_id_updt_en
        uint32_t RSVD                        :5;      ///<BIT [15:11] rsvd_0
        uint32_t OQ_NM_ELMNTS                :16;     ///<BIT [31:16] oq_nm_elmnts
    } b;
} UcducdObCmnOqOutboundQueueCfg0_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_1                      :8;      ///<BIT [7:0] rsvd_1
        uint32_t OQ_IFC_SLCT                 :8;      ///<BIT [15:8] oq_ifc_slct
        uint32_t RSVD                        :16;     ///<BIT [31:16] rsvd_0
    } b;
} UcducdObCmnOqOutboundQueueCfg1_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OQ_MAX_COAL_TIME            :16;     ///<BIT [15:0] oq_max_coal_time
        uint32_t OQ_MIN_COAL_TIME            :16;     ///<BIT [31:16] oq_min_coal_time
    } b;
} UcducdObCmnOqOutboundQueueIntrCfg0_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OQ_INT_COAL_COUNT           :16;     ///<BIT [15:0] oq_int_coal_count
        uint32_t OQ_EN_INT_COAL              :1;      ///<BIT [16] oq_en_int_coal
        uint32_t OQ_WAIT_FOR_REARM           :1;      ///<BIT [17] oq_wait_for_rearm
        uint32_t OQ_ENBL_CI_WRT_REARM        :1;      ///<BIT [18] oq_enbl_ci_wrt_rearm
        uint32_t OQ_ENBL_CI_BIT_31_RESTRT_COALESC_TMR :1;      ///<BIT [19] oq_enbl_ci_bit_31_restrt_coalesc_tmr
        uint32_t RSVD                        :12;     ///<BIT [31:20] rsvd_0
    } b;
} UcducdObCmnOqOutboundQueueIntrCfg1_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OQ_MSI_X_VCTR_SLCT          :5;      ///<BIT [4:0] oq_msi_x_vctr_slct
        uint32_t RSVD_1                      :3;      ///<BIT [7:5] rsvd_1
        uint32_t OQ_MSI_X_TBL_SLCT           :8;      ///<BIT [15:8] oq_msi_x_tbl_slct
        uint32_t RSVD_2                      :14;     ///<BIT [29:16] rsvd_2
        uint32_t OQ_EN_GEN_MSI_X             :1;      ///<BIT [30] oq_en_gen_msi_x
        uint32_t OQ_EN_EXTRNL_TMR_RSTRT      :1;      ///<BIT [31] oq_en_extrnl_tmr_rstrt
    } b;
} UcducdObCmnOqOutboundQueueIntrCfg2_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD                        :2;      ///<BIT [1:0] rsvd_0
        uint32_t OQ_BASE_ADDR_LO             :30;     ///<BIT [31:2] oq_base_addr_lo
    } b;
} UcducdObCmnOqOutboundQueueBaseAddrLo_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OQ_PRDCR_INDX               :16;     ///<BIT [15:0] oq_prdcr_indx
        uint32_t RSVD                        :15;     ///<BIT [30:16] rsvd_0
        uint32_t OQ_PHASE                    :1;      ///<BIT [31] oq_phase
    } b;
} UcducdObCmnOqOutboundQueuePi_t;

/// @brief 0x2C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OQ_CNSMR_INDX               :16;     ///<BIT [15:0] oq_cnsmr_indx
        uint32_t RSVD                        :14;     ///<BIT [29:16] rsvd_0
        uint32_t OQ_INTRPT_CLR               :1;      ///<BIT [30] oq_intrpt_clr
        uint32_t OQ_RESTRT_COAL_TMR          :1;      ///<BIT [31] oq_restrt_coal_tmr
    } b;
} UcducdObCmnOqOutboundQueueCi_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OQ_EMPTY                    :1;      ///<BIT [0] oq_empty
        uint32_t OQ_FULL                     :1;      ///<BIT [1] oq_full
        uint32_t OQ_OVRFLW                   :1;      ///<BIT [2] oq_ovrflw
        uint32_t OQ_ELMNT_SKIPPED            :1;      ///<BIT [3] oq_elmnt_skipped
        uint32_t OQ_STALLED                  :1;      ///<BIT [4] oq_stalled
        uint32_t OQ_DBELL_WRITE_OUT_OF_RANGE_ERR :1;      ///<BIT [5] oq_dbell_write_out_of_range_err
        uint32_t OQ_DBELL_WRITE_SAME_VALUE_ERR :1;      ///<BIT [6] oq_dbell_write_same_value_err
        uint32_t OQ_DBELL_WRITE_RMV_FROM_EMPTY_CQ_ERR :1;      ///<BIT [7] oq_dbell_write_rmv_from_empty_cq_err
        uint32_t OQ_PI_IRQ                   :1;      ///<BIT [8] oq_pi_irq
        uint32_t RSVD_1                      :1;      ///<BIT [9] rsvd_1
        uint32_t OQ_LKHD_EMPTY               :1;      ///<BIT [10] oq_lkhd_empty
        uint32_t OQ_LKHD_FULL                :1;      ///<BIT [11] oq_lkhd_full
        uint32_t RSVD                        :4;      ///<BIT [15:12] rsvd_0
        uint32_t OQ_LKHD_PRDCR_INDX          :16;     ///<BIT [31:16] oq_lkhd_prdcr_indx
    } b;
} UcducdObCmnOqOutboundQueueStatus_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CAP_LO_MQES                 :16;     ///<BIT [15:0] cap_lo_mqes
        uint32_t CAP_LO_CAP_LO_CQR           :1;      ///<BIT [16] cap_lo_cap_lo_cqr
        uint32_t CAP_LO_AMS                  :2;      ///<BIT [18:17] cap_lo_ams
        uint32_t CAP_LO_RSVD_RW_0            :5;      ///<BIT [23:19] cap_lo_rsvd_rw_0
        uint32_t CAP_LO_TO                   :8;      ///<BIT [31:24] cap_lo_to
    } b;
} UcducdNvmeHostControllerCapabilitiesLo_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CAP_HI_DSTRD                :4;      ///<BIT [3:0] cap_hi_dstrd
        uint32_t CAP_HI_NSSRS                :1;      ///<BIT [4] cap_hi_nssrs
        uint32_t CAP_HI_CSS                  :8;      ///<BIT [12:5] cap_hi_css
        uint32_t CAP_HI_RSVD_RW_1            :3;      ///<BIT [15:13] cap_hi_rsvd_rw_1
        uint32_t CAP_HI_MPSMIN               :4;      ///<BIT [19:16] cap_hi_mpsmin
        uint32_t CAP_HI_MPSMAX               :4;      ///<BIT [23:20] cap_hi_mpsmax
        uint32_t CAP_HI_RSVD_RW_0            :8;      ///<BIT [31:24] cap_hi_rsvd_rw_0
    } b;
} UcducdNvmeHostControllerCapabilitiesHi_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VS_RSVD_RW_0                :8;      ///<BIT [7:0] vs_rsvd_rw_0
        uint32_t VS_MNR                      :8;      ///<BIT [15:8] vs_mnr
        uint32_t VS_MJR                      :16;     ///<BIT [31:16] vs_mjr
    } b;
} UcducdNvmeHostVersion_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INTMS_IVMS                  :4;      ///<BIT [3:0] intms_ivms
        uint32_t INTMS_RSVD                  :28;     ///<BIT [31:4] intms_rsvd
    } b;
} UcducdNvmeHostIntrMaskSet_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INTMC_IVMC                  :4;      ///<BIT [3:0] intmc_ivmc
        uint32_t INTMC_RSVD                  :28;     ///<BIT [31:4] intmc_rsvd
    } b;
} UcducdNvmeHostIntrMaskClear_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CC_EN                       :1;      ///<BIT [0] cc_en
        uint32_t CC_RSVD_RW_1                :3;      ///<BIT [3:1] cc_rsvd_rw_1
        uint32_t CC_CSS                      :3;      ///<BIT [6:4] cc_css
        uint32_t CC_MPS                      :4;      ///<BIT [10:7] cc_mps
        uint32_t CC_AMS                      :3;      ///<BIT [13:11] cc_ams
        uint32_t CC_SHN                      :2;      ///<BIT [15:14] cc_shn
        uint32_t CC_IOSQES                   :4;      ///<BIT [19:16] cc_iosqes
        uint32_t CC_IOCQES                   :4;      ///<BIT [23:20] cc_iocqes
        uint32_t CC_RSVD_RW_0                :8;      ///<BIT [31:24] cc_rsvd_rw_0
    } b;
} UcducdNvmeHostControllerCfg_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CSTS_RDY                    :1;      ///<BIT [0] csts_rdy
        uint32_t CSTS_CFS                    :1;      ///<BIT [1] csts_cfs
        uint32_t CSTS_SHST                   :2;      ///<BIT [3:2] csts_shst
        uint32_t CSTS_NSSRO                  :1;      ///<BIT [4] csts_nssro
        uint32_t CSTS_RSVD_RW_0              :3;      ///<BIT [7:5] csts_rsvd_rw_0
        uint32_t CSTS_RSVD                   :24;     ///<BIT [31:8] csts_rsvd
    } b;
} UcducdNvmeHostControllerStatus_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AQA_ASQS                    :12;     ///<BIT [11:0] aqa_asqs
        uint32_t AQA_RSVD                    :4;      ///<BIT [15:12] aqa_rsvd
        uint32_t AQA_ACQS                    :12;     ///<BIT [27:16] aqa_acqs
        uint32_t AQA_RSVD1                   :4;      ///<BIT [31:28] aqa_rsvd1
    } b;
} UcducdNvmeHostAdminQueueAttrs_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ASQB_LO_RSVD                :6;      ///<BIT [5:0] asqb_lo_rsvd
        uint32_t ASQB_LO_RSVD_RW             :6;      ///<BIT [11:6] asqb_lo_rsvd_rw
        uint32_t ASQB_LO                     :20;     ///<BIT [31:12] asqb_lo
    } b;
} UcducdNvmeHostAdminSubmissionQueueBaseAddressLo_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ACQB_LO_RSVD                :6;      ///<BIT [5:0] acqb_lo_rsvd
        uint32_t ACQB_LO_RSVD_RW             :6;      ///<BIT [11:6] acqb_lo_rsvd_rw
        uint32_t ACQB_LO                     :20;     ///<BIT [31:12] acqb_lo
    } b;
} UcducdNvmeHostAdminCompletionQueueBaseAddressLo_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMBLOC_BIR                  :3;      ///<BIT [2:0] cmbloc_bir
        uint32_t CMBLOC_RSVD                 :9;      ///<BIT [11:3] cmbloc_rsvd
        uint32_t CMBLOC_OFST                 :20;     ///<BIT [31:12] cmbloc_ofst
    } b;
} UcducdNvmeHostControllerMemoryBufferLocation_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMBSZ_SQS                   :1;      ///<BIT [0] cmbsz_sqs
        uint32_t CMBSZ_CQS                   :1;      ///<BIT [1] cmbsz_cqs
        uint32_t CMBSZ_LISTS                 :1;      ///<BIT [2] cmbsz_lists
        uint32_t CMBSZ_RDS                   :1;      ///<BIT [3] cmbsz_rds
        uint32_t CMBSZ_WDS                   :1;      ///<BIT [4] cmbsz_wds
        uint32_t CMBSZ_RSVD                  :3;      ///<BIT [7:5] cmbsz_rsvd
        uint32_t CMBSZ_SZU                   :4;      ///<BIT [11:8] cmbsz_szu
        uint32_t CMBSZ_SZ                    :20;     ///<BIT [31:12] cmbsz_sz
    } b;
} UcducdNvmeHostControllerMemoryBufferSize_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IS_INTA_PENDING             :1;      ///<BIT [0] is_inta_pending
        uint32_t IS_INTA_MASK                :1;      ///<BIT [1] is_inta_mask
        uint32_t IS_INTA_SRC_PENDING         :1;      ///<BIT [2] is_inta_src_pending
        uint32_t IS_INTB_PENDING             :1;      ///<BIT [3] is_intb_pending
        uint32_t IS_INTB_MASK                :1;      ///<BIT [4] is_intb_mask
        uint32_t IS_INTB_SRC_PENDING         :1;      ///<BIT [5] is_intb_src_pending
        uint32_t IS_INTC_PENDING             :1;      ///<BIT [6] is_intc_pending
        uint32_t IS_INTC_MASK                :1;      ///<BIT [7] is_intc_mask
        uint32_t IS_INTC_SRC_PENDING         :1;      ///<BIT [8] is_intc_src_pending
        uint32_t IS_INTD_PENDING             :1;      ///<BIT [9] is_intd_pending
        uint32_t IS_INTD_MASK                :1;      ///<BIT [10] is_intd_mask
        uint32_t IS_INTD_SRC_PENDING         :1;      ///<BIT [11] is_intd_src_pending
        uint32_t IS_RSVD                     :20;     ///<BIT [31:12] is_rsvd
    } b;
} UcducdNvmeHostIntrStatus_t;

/// @brief 0x300000
typedef struct
{
    UcducdNvmeHostControllerCapabilitiesLo_t ucdNvmeHostControllerCapabilitiesLo; //ucd_nvme_host_reg_controller_capabilities_lo
    UcducdNvmeHostControllerCapabilitiesHi_t ucdNvmeHostControllerCapabilitiesHi; //ucd_nvme_host_reg_controller_capabilities_hi
    UcducdNvmeHostVersion_t ucdNvmeHostVersion; //ucd_nvme_host_reg_version
    UcducdNvmeHostIntrMaskSet_t ucdNvmeHostIntrMaskSet; //ucd_nvme_host_reg_interrupt_mask_set
    UcducdNvmeHostIntrMaskClear_t ucdNvmeHostIntrMaskClear; //ucd_nvme_host_reg_interrupt_mask_clear
    UcducdNvmeHostControllerCfg_t ucdNvmeHostControllerCfg; //ucd_nvme_host_reg_controller_configuration
    uint32_t ucdNvmeHostReserved1Rsvd1Rsvd; //ucd_nvme_host_reg_reserved_1
    UcducdNvmeHostControllerStatus_t ucdNvmeHostControllerStatus; //ucd_nvme_host_reg_controller_status
    uint32_t ucdNvmeHostNvmSubsystemResetNssrNssrc; //ucd_nvme_host_reg_nvm_subsystem_reset
    UcducdNvmeHostAdminQueueAttrs_t ucdNvmeHostAdminQueueAttrs; //ucd_nvme_host_reg_admin_queue_attributes
    UcducdNvmeHostAdminSubmissionQueueBaseAddressLo_t ucdNvmeHostAdminSubmissionQueueBaseAddressLo; //ucd_nvme_host_reg_admin_submission_queue_base_address_lo
    uint32_t ucdNvmeHostAdminSubmissionQueueBaseAddressHiAsqbHi; //ucd_nvme_host_reg_admin_submission_queue_base_address_hi
    UcducdNvmeHostAdminCompletionQueueBaseAddressLo_t ucdNvmeHostAdminCompletionQueueBaseAddressLo; //ucd_nvme_host_reg_admin_completion_queue_base_address_lo
    uint32_t ucdNvmeHostAdminCompletionQueueBaseAddressHiAcqbHi; //ucd_nvme_host_reg_admin_completion_queue_base_address_hi
    UcducdNvmeHostControllerMemoryBufferLocation_t ucdNvmeHostControllerMemoryBufferLocation; //ucd_nvme_host_reg_controller_memory_buffer_location
    UcducdNvmeHostControllerMemoryBufferSize_t ucdNvmeHostControllerMemoryBufferSize; //ucd_nvme_host_reg_controller_memory_buffer_size
    uint32_t ucdNvmeHostReserved2Rsvd2Rsvd; //ucd_nvme_host_reg_reserved_2
    uint8_t rsvd44[12];                   //rsvd_44
    UcducdNvmeHostIntrStatus_t ucdNvmeHostIntrStatus; //ucd_nvme_host_reg_interrupt_status
    uint8_t endPadding[16300];            //end_padding
} UcdHstVfNvmeControllerRegisters_t;

/// @brief 0x200000
typedef struct
{
    UcducdNvmeHostControllerCapabilitiesLo_t ucdNvmeHostControllerCapabilitiesLo; //ucd_nvme_host_reg_controller_capabilities_lo
    UcducdNvmeHostControllerCapabilitiesHi_t ucdNvmeHostControllerCapabilitiesHi; //ucd_nvme_host_reg_controller_capabilities_hi
    UcducdNvmeHostVersion_t ucdNvmeHostVersion; //ucd_nvme_host_reg_version
    UcducdNvmeHostIntrMaskSet_t ucdNvmeHostIntrMaskSet; //ucd_nvme_host_reg_interrupt_mask_set
    UcducdNvmeHostIntrMaskClear_t ucdNvmeHostIntrMaskClear; //ucd_nvme_host_reg_interrupt_mask_clear
    UcducdNvmeHostControllerCfg_t ucdNvmeHostControllerCfg; //ucd_nvme_host_reg_controller_configuration
    uint32_t ucdNvmeHostReserved1Rsvd1Rsvd; //ucd_nvme_host_reg_reserved_1
    UcducdNvmeHostControllerStatus_t ucdNvmeHostControllerStatus; //ucd_nvme_host_reg_controller_status
    uint32_t ucdNvmeHostNvmSubsystemResetNssrNssrc; //ucd_nvme_host_reg_nvm_subsystem_reset
    UcducdNvmeHostAdminQueueAttrs_t ucdNvmeHostAdminQueueAttrs; //ucd_nvme_host_reg_admin_queue_attributes
    UcducdNvmeHostAdminSubmissionQueueBaseAddressLo_t ucdNvmeHostAdminSubmissionQueueBaseAddressLo; //ucd_nvme_host_reg_admin_submission_queue_base_address_lo
    uint32_t ucdNvmeHostAdminSubmissionQueueBaseAddressHiAsqbHi; //ucd_nvme_host_reg_admin_submission_queue_base_address_hi
    UcducdNvmeHostAdminCompletionQueueBaseAddressLo_t ucdNvmeHostAdminCompletionQueueBaseAddressLo; //ucd_nvme_host_reg_admin_completion_queue_base_address_lo
    uint32_t ucdNvmeHostAdminCompletionQueueBaseAddressHiAcqbHi; //ucd_nvme_host_reg_admin_completion_queue_base_address_hi
    UcducdNvmeHostControllerMemoryBufferLocation_t ucdNvmeHostControllerMemoryBufferLocation; //ucd_nvme_host_reg_controller_memory_buffer_location
    UcducdNvmeHostControllerMemoryBufferSize_t ucdNvmeHostControllerMemoryBufferSize; //ucd_nvme_host_reg_controller_memory_buffer_size
    uint32_t ucdNvmeHostReserved2Rsvd2Rsvd; //ucd_nvme_host_reg_reserved_2
    uint8_t rsvd44[12];                   //rsvd_44
    UcducdNvmeHostIntrStatus_t ucdNvmeHostIntrStatus; //ucd_nvme_host_reg_interrupt_status
} UcdHstPfNvmeControllerRegisters_t;

/// @brief 0x5000
typedef struct
{
    UcducdObCmnOqOutboundQueueCfg0_t ucdObCmnOqOutboundQueueCfg0; //ucd_ob_cmn_oq_reg_outbound_queue_configuration_0
    UcducdObCmnOqOutboundQueueCfg1_t ucdObCmnOqOutboundQueueCfg1; //ucd_ob_cmn_oq_reg_outbound_queue_configuration_1
    UcducdObCmnOqOutboundQueueIntrCfg0_t ucdObCmnOqOutboundQueueIntrCfg0; //ucd_ob_cmn_oq_reg_outbound_queue_interrupt_configuration_0
    UcducdObCmnOqOutboundQueueIntrCfg1_t ucdObCmnOqOutboundQueueIntrCfg1; //ucd_ob_cmn_oq_reg_outbound_queue_interrupt_configuration_1
    UcducdObCmnOqOutboundQueueIntrCfg2_t ucdObCmnOqOutboundQueueIntrCfg2; //ucd_ob_cmn_oq_reg_outbound_queue_interrupt_configuration_2
    uint8_t rsvd14[4];                    //rsvd_14
    uint32_t ucdObCmnOqOutboundQueuePiShadowBaseAddrLoRsvd; //ucd_ob_cmn_oq_reg_outbound_queue_pi_shadow_base_addr_lo
    uint32_t ucdObCmnOqOutboundQueuePiShadowBaseAddrHiRsvd; //ucd_ob_cmn_oq_reg_outbound_queue_pi_shadow_base_addr_hi
    UcducdObCmnOqOutboundQueueBaseAddrLo_t ucdObCmnOqOutboundQueueBaseAddrLo; //ucd_ob_cmn_oq_reg_outbound_queue_base_addr_lo
    uint32_t ucdObCmnOqOutboundQueueBaseAddrHiOqBaseAddrHi; //ucd_ob_cmn_oq_reg_outbound_queue_base_addr_hi
    UcducdObCmnOqOutboundQueuePi_t ucdObCmnOqOutboundQueuePi; //ucd_ob_cmn_oq_reg_outbound_queue_pi
    UcducdObCmnOqOutboundQueueCi_t ucdObCmnOqOutboundQueueCi; //ucd_ob_cmn_oq_reg_outbound_queue_ci
    UcducdObCmnOqOutboundQueueStatus_t ucdObCmnOqOutboundQueueStatus; //ucd_ob_cmn_oq_reg_outbound_queue_status
    uint8_t endPadding[12];               //end_padding
} UcdCore1ObCmnOqRegisters_t;

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

/// @brief 0x70
typedef struct
{
    UcducdObCmnOslOutboundSourceListCfg0_t ucdObCmnOslOutboundSourceListCfg0; //ucd_ob_cmn_osl_reg_outbound_source_list_configuration_0
    uint8_t rsvd4[4];                     //rsvd_4
    UcducdObCmnOslOutboundSourceListBaseAddrLo_t ucdObCmnOslOutboundSourceListBaseAddrLo; //ucd_ob_cmn_osl_reg_outbound_source_list_base_addr_lo
    uint32_t ucdObCmnOslOutboundSourceListBaseAddrHiObSrcListBaseAddrHi; //ucd_ob_cmn_osl_reg_outbound_source_list_base_addr_hi
    UcducdObCmnOslOutboundSourceListPi_t ucdObCmnOslOutboundSourceListPi; //ucd_ob_cmn_osl_reg_outbound_source_list_pi
    UcducdObCmnOslOutboundSourceListCi_t ucdObCmnOslOutboundSourceListCi; //ucd_ob_cmn_osl_reg_outbound_source_list_ci
    UcducdObCmnOslOutboundSourceListStatus_t ucdObCmnOslOutboundSourceListStatus; //ucd_ob_cmn_osl_reg_outbound_source_list_status
    uint8_t endPadding[4];                //end_padding
} UcducdObCmnOslRegisters_t;

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
    uint8_t rsvd3c[4];                    //rsvd_3c
    UcducdObCmnSnglOutboundUcdDataPathErrorStatus_t ucdObCmnSnglOutboundUcdDataPathErrorStatus; //ucd_ob_cmn_sngl_reg_outbound_ucd_data_path_error_status
    uint8_t rsvd44[4];                    //rsvd_44
    uint32_t ucdObCmnSnglOutboundUcdErrorAddressLowObErrAddrLow; //ucd_ob_cmn_sngl_reg_outbound_ucd_error_address_low
    uint32_t ucdObCmnSnglOutboundUcdErrorAddressHighObErrAddrHigh; //ucd_ob_cmn_sngl_reg_outbound_ucd_error_address_high
    UcducdObCmnSnglOutboundQueueArbitrationCfgRegister_t ucdObCmnSnglOutboundQueueArbitrationCfgRegister; //ucd_ob_cmn_sngl_reg_outbound_queue_arbitration_configuration_register
    uint8_t rsvd54[12];                   //rsvd_54
    UcducdObCmnSnglOutboundSizeSelect0_t ucdObCmnSnglOutboundSizeSelect0; //ucd_ob_cmn_sngl_reg_outbound_size_select_0
    UcducdObCmnSnglOutboundSizeSelect1_t ucdObCmnSnglOutboundSizeSelect1; //ucd_ob_cmn_sngl_reg_outbound_size_select_1
    UcducdObCmnSnglOutboundSizeSelect2_t ucdObCmnSnglOutboundSizeSelect2; //ucd_ob_cmn_sngl_reg_outbound_size_select_2
    UcducdObCmnSnglOutboundSizeSelect3_t ucdObCmnSnglOutboundSizeSelect3; //ucd_ob_cmn_sngl_reg_outbound_size_select_3
    UcducdObCmnOslRegisters_t ucdObCmnOslRegisters[5]; //ucd_ob_cmn_osl_registers
    uint8_t rsvd110[16];                  //rsvd_110
    UcducdObCmnCqRegisters_t ucdObCmnCqRegisters[5]; //ucd_ob_cmn_cq_registers
    uint8_t rsvd210[240];                 //rsvd_210
    UcducdObCmnSnglOutboundMiscellaneousControl_t ucdObCmnSnglOutboundMiscellaneousControl; //ucd_ob_cmn_sngl_reg_outbound_miscellaneous_control
    uint32_t ucdObCmnSnglOutboundControlPathErrorStatusCpErrorStatus; //ucd_ob_cmn_sngl_reg_outbound_control_path_error_status
    uint32_t ucdObCmnSnglOutboundDatapathHaltErrorStatusDpathHaltErrorStatus; //ucd_ob_cmn_sngl_reg_outbound_datapath_halt_error_status
    uint32_t ucdObCmnSnglOutboundDatapathHaltErrorMaskDpathHaltErrorMask; //ucd_ob_cmn_sngl_reg_outbound_datapath_halt_error_mask
    UcducdObCmnSnglOutboundDiagnosticControl_t ucdObCmnSnglOutboundDiagnosticControl; //ucd_ob_cmn_sngl_reg_outbound_diagnostic_control
    uint32_t ucdObCmnSnglOutboundDiagnosticReadPortDiagMiscStatus; //ucd_ob_cmn_sngl_reg_outbound_diagnostic_read_port
    uint8_t rsvd318[8];                   //rsvd_318
    UcducdObCmnSnglOutboundDataChannelAxiReadBusAttrs_t ucdObCmnSnglOutboundDataChannelAxiReadBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_data_channel_axi_read_bus_attributes
    UcducdObCmnSnglOutboundDataChannelAxiWriteBusAttrs_t ucdObCmnSnglOutboundDataChannelAxiWriteBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_data_channel_axi_write_bus_attributes
    UcducdObCmnSnglOutboundControlChannelAxiReadBusAttrs_t ucdObCmnSnglOutboundControlChannelAxiReadBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_control_channel_axi_read_bus_attributes
    UcducdObCmnSnglOutboundControlChannelAxiWriteBusAttrs_t ucdObCmnSnglOutboundControlChannelAxiWriteBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_control_channel_axi_write_bus_attributes
    uint8_t rsvd330[16];                  //rsvd_330
    uint32_t ucdObCmnSnglOutboundQueueSoftError0ObQ310SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_0
    uint32_t ucdObCmnSnglOutboundQueueSoftError1ObQ6332SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_1
    uint32_t ucdObCmnSnglOutboundQueueSoftError2ObQ9564SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_2
    uint32_t ucdObCmnSnglOutboundQueueSoftError3ObQ12796SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_3
    UcducdObCmnSnglOutboundQueueSoftError4_t ucdObCmnSnglOutboundQueueSoftError4; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_4
} UcdCore1ObCmnRegisters_t;

/// @brief 0x1000
typedef struct
{
    UcducdObCmnOqOutboundQueueCfg0_t ucdObCmnOqOutboundQueueCfg0; //ucd_ob_cmn_oq_reg_outbound_queue_configuration_0
    UcducdObCmnOqOutboundQueueCfg1_t ucdObCmnOqOutboundQueueCfg1; //ucd_ob_cmn_oq_reg_outbound_queue_configuration_1
    UcducdObCmnOqOutboundQueueIntrCfg0_t ucdObCmnOqOutboundQueueIntrCfg0; //ucd_ob_cmn_oq_reg_outbound_queue_interrupt_configuration_0
    UcducdObCmnOqOutboundQueueIntrCfg1_t ucdObCmnOqOutboundQueueIntrCfg1; //ucd_ob_cmn_oq_reg_outbound_queue_interrupt_configuration_1
    UcducdObCmnOqOutboundQueueIntrCfg2_t ucdObCmnOqOutboundQueueIntrCfg2; //ucd_ob_cmn_oq_reg_outbound_queue_interrupt_configuration_2
    uint8_t rsvd14[4];                    //rsvd_14
    uint32_t ucdObCmnOqOutboundQueuePiShadowBaseAddrLoRsvd; //ucd_ob_cmn_oq_reg_outbound_queue_pi_shadow_base_addr_lo
    uint32_t ucdObCmnOqOutboundQueuePiShadowBaseAddrHiRsvd; //ucd_ob_cmn_oq_reg_outbound_queue_pi_shadow_base_addr_hi
    UcducdObCmnOqOutboundQueueBaseAddrLo_t ucdObCmnOqOutboundQueueBaseAddrLo; //ucd_ob_cmn_oq_reg_outbound_queue_base_addr_lo
    uint32_t ucdObCmnOqOutboundQueueBaseAddrHiOqBaseAddrHi; //ucd_ob_cmn_oq_reg_outbound_queue_base_addr_hi
    UcducdObCmnOqOutboundQueuePi_t ucdObCmnOqOutboundQueuePi; //ucd_ob_cmn_oq_reg_outbound_queue_pi
    UcducdObCmnOqOutboundQueueCi_t ucdObCmnOqOutboundQueueCi; //ucd_ob_cmn_oq_reg_outbound_queue_ci
    UcducdObCmnOqOutboundQueueStatus_t ucdObCmnOqOutboundQueueStatus; //ucd_ob_cmn_oq_reg_outbound_queue_status
    uint8_t endPadding[12];               //end_padding
} UcdCore0ObCmnOqRegisters_t;

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
    uint8_t rsvd3c[4];                    //rsvd_3c
    UcducdObCmnSnglOutboundUcdDataPathErrorStatus_t ucdObCmnSnglOutboundUcdDataPathErrorStatus; //ucd_ob_cmn_sngl_reg_outbound_ucd_data_path_error_status
    uint8_t rsvd44[4];                    //rsvd_44
    uint32_t ucdObCmnSnglOutboundUcdErrorAddressLowObErrAddrLow; //ucd_ob_cmn_sngl_reg_outbound_ucd_error_address_low
    uint32_t ucdObCmnSnglOutboundUcdErrorAddressHighObErrAddrHigh; //ucd_ob_cmn_sngl_reg_outbound_ucd_error_address_high
    UcducdObCmnSnglOutboundQueueArbitrationCfgRegister_t ucdObCmnSnglOutboundQueueArbitrationCfgRegister; //ucd_ob_cmn_sngl_reg_outbound_queue_arbitration_configuration_register
    uint8_t rsvd54[12];                   //rsvd_54
    UcducdObCmnSnglOutboundSizeSelect0_t ucdObCmnSnglOutboundSizeSelect0; //ucd_ob_cmn_sngl_reg_outbound_size_select_0
    UcducdObCmnSnglOutboundSizeSelect1_t ucdObCmnSnglOutboundSizeSelect1; //ucd_ob_cmn_sngl_reg_outbound_size_select_1
    UcducdObCmnSnglOutboundSizeSelect2_t ucdObCmnSnglOutboundSizeSelect2; //ucd_ob_cmn_sngl_reg_outbound_size_select_2
    UcducdObCmnSnglOutboundSizeSelect3_t ucdObCmnSnglOutboundSizeSelect3; //ucd_ob_cmn_sngl_reg_outbound_size_select_3
    UcducdObCmnOslRegisters_t ucdObCmnOslRegisters[5]; //ucd_ob_cmn_osl_registers
    uint8_t rsvd110[16];                  //rsvd_110
    UcducdObCmnCqRegisters_t ucdObCmnCqRegisters[5]; //ucd_ob_cmn_cq_registers
    uint8_t rsvd210[240];                 //rsvd_210
    UcducdObCmnSnglOutboundMiscellaneousControl_t ucdObCmnSnglOutboundMiscellaneousControl; //ucd_ob_cmn_sngl_reg_outbound_miscellaneous_control
    uint32_t ucdObCmnSnglOutboundControlPathErrorStatusCpErrorStatus; //ucd_ob_cmn_sngl_reg_outbound_control_path_error_status
    uint32_t ucdObCmnSnglOutboundDatapathHaltErrorStatusDpathHaltErrorStatus; //ucd_ob_cmn_sngl_reg_outbound_datapath_halt_error_status
    uint32_t ucdObCmnSnglOutboundDatapathHaltErrorMaskDpathHaltErrorMask; //ucd_ob_cmn_sngl_reg_outbound_datapath_halt_error_mask
    UcducdObCmnSnglOutboundDiagnosticControl_t ucdObCmnSnglOutboundDiagnosticControl; //ucd_ob_cmn_sngl_reg_outbound_diagnostic_control
    uint32_t ucdObCmnSnglOutboundDiagnosticReadPortDiagMiscStatus; //ucd_ob_cmn_sngl_reg_outbound_diagnostic_read_port
    uint8_t rsvd318[8];                   //rsvd_318
    UcducdObCmnSnglOutboundDataChannelAxiReadBusAttrs_t ucdObCmnSnglOutboundDataChannelAxiReadBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_data_channel_axi_read_bus_attributes
    UcducdObCmnSnglOutboundDataChannelAxiWriteBusAttrs_t ucdObCmnSnglOutboundDataChannelAxiWriteBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_data_channel_axi_write_bus_attributes
    UcducdObCmnSnglOutboundControlChannelAxiReadBusAttrs_t ucdObCmnSnglOutboundControlChannelAxiReadBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_control_channel_axi_read_bus_attributes
    UcducdObCmnSnglOutboundControlChannelAxiWriteBusAttrs_t ucdObCmnSnglOutboundControlChannelAxiWriteBusAttrs; //ucd_ob_cmn_sngl_reg_outbound_control_channel_axi_write_bus_attributes
    uint8_t rsvd330[16];                  //rsvd_330
    uint32_t ucdObCmnSnglOutboundQueueSoftError0ObQ310SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_0
    uint32_t ucdObCmnSnglOutboundQueueSoftError1ObQ6332SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_1
    uint32_t ucdObCmnSnglOutboundQueueSoftError2ObQ9564SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_2
    uint32_t ucdObCmnSnglOutboundQueueSoftError3ObQ12796SoftErr; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_3
    UcducdObCmnSnglOutboundQueueSoftError4_t ucdObCmnSnglOutboundQueueSoftError4; //ucd_ob_cmn_sngl_reg_outbound_queue_soft_error_4
} UcdCore0ObCmnRegisters_t;

/// @brief 0x1C0000
typedef struct
{
    UcdCore0ObCmnRegisters_t ucdCore0ObCmnRegisters; //ucd_core0_ob_cmn_registers
    uint8_t rsvd354[3244];                //rsvd_354
    UcdCore0ObCmnOqRegisters_t ucdCore0ObCmnOqRegisters[132]; //ucd_core0_ob_cmn_oq_registers
    uint8_t rsvd3100[3840];               //rsvd_3100
    UcdCore1ObCmnRegisters_t ucdCore1ObCmnRegisters; //ucd_core1_ob_cmn_registers
    uint8_t rsvd4354[3244];               //rsvd_4354
    UcdCore1ObCmnOqRegisters_t ucdCore1ObCmnOqRegisters[132]; //ucd_core1_ob_cmn_oq_registers
} UcdOutboundRegisters_t;

/// @brief 0x5000
typedef struct
{
    UcducdIbCmnIqInboundQueueCfg0_t ucdIbCmnIqInboundQueueCfg0; //ucd_ib_cmn_iq_reg_inbound_queue_configuration_0
    UcducdIbCmnIqInboundQueueCfg1_t ucdIbCmnIqInboundQueueCfg1; //ucd_ib_cmn_iq_reg_inbound_queue_configuration_1
    UcducdIbCmnIqInboundQueueCreditCount_t ucdIbCmnIqInboundQueueCreditCount; //ucd_ib_cmn_iq_reg_inbound_queue_credit_count
    uint8_t rsvdC[4];                     //rsvd_c
    uint32_t ucdIbCmnIqInboundQueueCiShadowAddressLowRsvd; //ucd_ib_cmn_iq_reg_inbound_queue_ci_shadow_address_low
    uint32_t ucdIbCmnIqInboundQueueCiShadowAddressHighRsvd; //ucd_ib_cmn_iq_reg_inbound_queue_ci_shadow_address_high
    UcducdIbCmnIqInboundQueueBaseAddressLow_t ucdIbCmnIqInboundQueueBaseAddressLow; //ucd_ib_cmn_iq_reg_inbound_queue_base_address_low
    uint32_t ucdIbCmnIqInboundQueueBaseAddressHighIqBaseAddrHi; //ucd_ib_cmn_iq_reg_inbound_queue_base_address_high
    UcducdIbCmnIqInboundQueuePi_t ucdIbCmnIqInboundQueuePi; //ucd_ib_cmn_iq_reg_inbound_queue_pi
    UcducdIbCmnIqInboundQueueCi_t ucdIbCmnIqInboundQueueCi; //ucd_ib_cmn_iq_reg_inbound_queue_ci
    UcducdIbCmnIqInboundQueueStatus_t ucdIbCmnIqInboundQueueStatus; //ucd_ib_cmn_iq_reg_inbound_queue_status
    uint8_t endPadding[20];               //end_padding
} UcdCore1IbCmnIqRegisters_t;

/// @brief 0x400
typedef struct
{
    UcducdIbHfCreditUcdInboundHfCreditCount_t ucdIbHfCreditUcdInboundHfCreditCount; //ucd_ib_hf_credit_reg_ucd_inbound_hf_credit_count
} UcducdIbHfCreditRegisters_t;

/// @brief 0x120
typedef struct
{
    UcducdIbCmnCqCompletionQueueCfgControl_t ucdIbCmnCqCompletionQueueCfgControl; //ucd_ib_cmn_cq_reg_completion_queue_configuration_control
    uint8_t rsvd4[4];                     //rsvd_4
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
    uint8_t rsvd48[8];                    //rsvd_48
    UcducdIbCmnSnglInboundQueueArbitrationCfgRegister_t ucdIbCmnSnglInboundQueueArbitrationCfgRegister; //ucd_ib_cmn_sngl_reg_inbound_queue_arbitration_configuration_register
    uint8_t rsvd54[12];                   //rsvd_54
    UcducdIbCmnDflRegisters_t ucdIbCmnDflRegisters[6]; //ucd_ib_cmn_dfl_registers
    UcducdIbCmnCqRegisters_t ucdIbCmnCqRegisters[5]; //ucd_ib_cmn_cq_registers
    uint8_t rsvd210[240];                 //rsvd_210
    UcducdIbCmnSnglInboundMiscellaneousControl_t ucdIbCmnSnglInboundMiscellaneousControl; //ucd_ib_cmn_sngl_reg_inbound_miscellaneous_control
    uint32_t ucdIbCmnSnglInboundControlPathErrorStatusCpErrorStatus; //ucd_ib_cmn_sngl_reg_inbound_control_path_error_status
    uint32_t ucdIbCmnSnglInboundDatapathHaltErrorStatusDpathHaltErrorStatus; //ucd_ib_cmn_sngl_reg_inbound_datapath_halt_error_status
    uint32_t ucdIbCmnSnglInboundDatapathHaltErrorMaskDpathHaltErrorMask; //ucd_ib_cmn_sngl_reg_inbound_datapath_halt_error_mask
    UcducdIbCmnSnglInboundDiagnosticControl_t ucdIbCmnSnglInboundDiagnosticControl; //ucd_ib_cmn_sngl_reg_inbound_diagnostic_control
    uint32_t ucdIbCmnSnglInboundDiagnosticReadPortDiagMiscStatus; //ucd_ib_cmn_sngl_reg_inbound_diagnostic_read_port
    uint8_t rsvd318[8];                   //rsvd_318
    UcducdIbCmnSnglInboundDataChannelAxiReadBusAttrs_t ucdIbCmnSnglInboundDataChannelAxiReadBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_data_channel_axi_read_bus_attributes
    UcducdIbCmnSnglInboundDataChannelAxiWriteBusAttrs_t ucdIbCmnSnglInboundDataChannelAxiWriteBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_data_channel_axi_write_bus_attributes
    UcducdIbCmnSnglInboundControlChannelAxiReadBusAttrs_t ucdIbCmnSnglInboundControlChannelAxiReadBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_control_channel_axi_read_bus_attributes
    UcducdIbCmnSnglInboundControlChannelAxiWriteBusAttrs_t ucdIbCmnSnglInboundControlChannelAxiWriteBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_control_channel_axi_write_bus_attributes
    uint8_t rsvd330[16];                  //rsvd_330
    uint32_t ucdIbCmnSnglInboundQueueSoftError0IbQ310SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_0
    uint32_t ucdIbCmnSnglInboundQueueSoftError1IbQ6332SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_1
    uint32_t ucdIbCmnSnglInboundQueueSoftError2IbQ9564SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_2
    uint32_t ucdIbCmnSnglInboundQueueSoftError3IbQ12796SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_3
    UcducdIbCmnSnglInboundQueueSoftError4_t ucdIbCmnSnglInboundQueueSoftError4; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_4
    uint8_t rsvd354[172];                 //rsvd_354
    UcducdIbHfCreditRegisters_t ucdIbHfCreditRegisters[64]; //ucd_ib_hf_credit_registers
} UcdCore1IbCmnRegisters_t;

/// @brief 0x1000
typedef struct
{
    UcducdIbCmnIqInboundQueueCfg0_t ucdIbCmnIqInboundQueueCfg0; //ucd_ib_cmn_iq_reg_inbound_queue_configuration_0
    UcducdIbCmnIqInboundQueueCfg1_t ucdIbCmnIqInboundQueueCfg1; //ucd_ib_cmn_iq_reg_inbound_queue_configuration_1
    UcducdIbCmnIqInboundQueueCreditCount_t ucdIbCmnIqInboundQueueCreditCount; //ucd_ib_cmn_iq_reg_inbound_queue_credit_count
    uint8_t rsvdC[4];                     //rsvd_c
    uint32_t ucdIbCmnIqInboundQueueCiShadowAddressLowRsvd; //ucd_ib_cmn_iq_reg_inbound_queue_ci_shadow_address_low
    uint32_t ucdIbCmnIqInboundQueueCiShadowAddressHighRsvd; //ucd_ib_cmn_iq_reg_inbound_queue_ci_shadow_address_high
    UcducdIbCmnIqInboundQueueBaseAddressLow_t ucdIbCmnIqInboundQueueBaseAddressLow; //ucd_ib_cmn_iq_reg_inbound_queue_base_address_low
    uint32_t ucdIbCmnIqInboundQueueBaseAddressHighIqBaseAddrHi; //ucd_ib_cmn_iq_reg_inbound_queue_base_address_high
    UcducdIbCmnIqInboundQueuePi_t ucdIbCmnIqInboundQueuePi; //ucd_ib_cmn_iq_reg_inbound_queue_pi
    UcducdIbCmnIqInboundQueueCi_t ucdIbCmnIqInboundQueueCi; //ucd_ib_cmn_iq_reg_inbound_queue_ci
    UcducdIbCmnIqInboundQueueStatus_t ucdIbCmnIqInboundQueueStatus; //ucd_ib_cmn_iq_reg_inbound_queue_status
    uint8_t endPadding[20];               //end_padding
} UcdCore0IbCmnIqRegisters_t;

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
    uint8_t rsvd48[8];                    //rsvd_48
    UcducdIbCmnSnglInboundQueueArbitrationCfgRegister_t ucdIbCmnSnglInboundQueueArbitrationCfgRegister; //ucd_ib_cmn_sngl_reg_inbound_queue_arbitration_configuration_register
    uint8_t rsvd54[12];                   //rsvd_54
    UcducdIbCmnDflRegisters_t ucdIbCmnDflRegisters[6]; //ucd_ib_cmn_dfl_registers
    UcducdIbCmnCqRegisters_t ucdIbCmnCqRegisters[5]; //ucd_ib_cmn_cq_registers
    uint8_t rsvd210[240];                 //rsvd_210
    UcducdIbCmnSnglInboundMiscellaneousControl_t ucdIbCmnSnglInboundMiscellaneousControl; //ucd_ib_cmn_sngl_reg_inbound_miscellaneous_control
    uint32_t ucdIbCmnSnglInboundControlPathErrorStatusCpErrorStatus; //ucd_ib_cmn_sngl_reg_inbound_control_path_error_status
    uint32_t ucdIbCmnSnglInboundDatapathHaltErrorStatusDpathHaltErrorStatus; //ucd_ib_cmn_sngl_reg_inbound_datapath_halt_error_status
    uint32_t ucdIbCmnSnglInboundDatapathHaltErrorMaskDpathHaltErrorMask; //ucd_ib_cmn_sngl_reg_inbound_datapath_halt_error_mask
    UcducdIbCmnSnglInboundDiagnosticControl_t ucdIbCmnSnglInboundDiagnosticControl; //ucd_ib_cmn_sngl_reg_inbound_diagnostic_control
    uint32_t ucdIbCmnSnglInboundDiagnosticReadPortDiagMiscStatus; //ucd_ib_cmn_sngl_reg_inbound_diagnostic_read_port
    uint8_t rsvd318[8];                   //rsvd_318
    UcducdIbCmnSnglInboundDataChannelAxiReadBusAttrs_t ucdIbCmnSnglInboundDataChannelAxiReadBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_data_channel_axi_read_bus_attributes
    UcducdIbCmnSnglInboundDataChannelAxiWriteBusAttrs_t ucdIbCmnSnglInboundDataChannelAxiWriteBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_data_channel_axi_write_bus_attributes
    UcducdIbCmnSnglInboundControlChannelAxiReadBusAttrs_t ucdIbCmnSnglInboundControlChannelAxiReadBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_control_channel_axi_read_bus_attributes
    UcducdIbCmnSnglInboundControlChannelAxiWriteBusAttrs_t ucdIbCmnSnglInboundControlChannelAxiWriteBusAttrs; //ucd_ib_cmn_sngl_reg_inbound_control_channel_axi_write_bus_attributes
    uint8_t rsvd330[16];                  //rsvd_330
    uint32_t ucdIbCmnSnglInboundQueueSoftError0IbQ310SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_0
    uint32_t ucdIbCmnSnglInboundQueueSoftError1IbQ6332SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_1
    uint32_t ucdIbCmnSnglInboundQueueSoftError2IbQ9564SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_2
    uint32_t ucdIbCmnSnglInboundQueueSoftError3IbQ12796SoftErr; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_3
    UcducdIbCmnSnglInboundQueueSoftError4_t ucdIbCmnSnglInboundQueueSoftError4; //ucd_ib_cmn_sngl_reg_inbound_queue_soft_error_4
    uint8_t rsvd354[172];                 //rsvd_354
    UcducdIbHfCreditRegisters_t ucdIbHfCreditRegisters[64]; //ucd_ib_hf_credit_registers
} UcdCore0IbCmnRegisters_t;

/// @brief 0x180000
typedef struct
{
    UcdCore0IbCmnRegisters_t ucdCore0IbCmnRegisters; //ucd_core0_ib_cmn_registers
    uint8_t rsvd500[2816];                //rsvd_500
    UcdCore0IbCmnIqRegisters_t ucdCore0IbCmnIqRegisters[132]; //ucd_core0_ib_cmn_iq_registers
    uint8_t rsvd3100[3840];               //rsvd_3100
    UcdCore1IbCmnRegisters_t ucdCore1IbCmnRegisters; //ucd_core1_ib_cmn_registers
    uint8_t rsvd4500[2816];               //rsvd_4500
    UcdCore1IbCmnIqRegisters_t ucdCore1IbCmnIqRegisters[132]; //ucd_core1_ib_cmn_iq_registers
} UcdInboundRegisters_t;

/// @brief 0x102000
typedef struct
{
    UcdGenCmnObLgc2physOqLogicalToPhysicalAssignment_t ucdGenCmnObLgc2physOqLogicalToPhysicalAssignment; //ucd_gen_cmn_ob_lgc2phys_reg_oq_logical_to_physical_assignment
} UcdGenCmnObLgc2physRegisters_t;

/// @brief 0x101000
typedef struct
{
    UcdGenCmnIbLgc2physIqLogicalToPhysicalAssignment_t ucdGenCmnIbLgc2physIqLogicalToPhysicalAssignment; //ucd_gen_cmn_ib_lgc2phys_reg_iq_logical_to_physical_assignment
} UcdGenCmnIbLgc2physRegisters_t;

/// @brief 0x100000
typedef struct
{
    uint8_t rsvd0[16];                    //rsvd_0
    UcdGenCmnSnglCommonUcdIntrCause_t ucdGenCmnSnglCommonUcdIntrCause; //ucd_gen_cmn_sngl_reg_common_ucd_interrupt_cause
    uint32_t ucdGenCmnSnglCommonUcdIntr0EnableCmnUcdIrq0Enbl; //ucd_gen_cmn_sngl_reg_common_ucd_interrupt_0_enable
    uint32_t ucdGenCmnSnglCommonUcdIntr1EnableCmnUcdIrq1Enbl; //ucd_gen_cmn_sngl_reg_common_ucd_interrupt_1_enable
    uint8_t rsvd1c[100];                  //rsvd_1c
    UcdGenCmnSnglNvmeRegisterSetResetPf_t ucdGenCmnSnglNvmeRegisterSetResetPf; //ucd_gen_cmn_sngl_reg_nvme_register_set_reset_pf
    uint32_t ucdGenCmnSnglNvmeRegisterSetReset0VfHiuNvmeRgstrSetRst310; //ucd_gen_cmn_sngl_reg_nvme_register_set_reset_0_vf
    uint32_t ucdGenCmnSnglNvmeRegisterSetReset1VfHiuNvmeRgstrSetRst6332; //ucd_gen_cmn_sngl_reg_nvme_register_set_reset_1_vf
    uint8_t rsvd8c[244];                  //rsvd_8c
    UcdGenCmnSnglNvmeAdminResetPf_t ucdGenCmnSnglNvmeAdminResetPf; //ucd_gen_cmn_sngl_reg_nvme_admin_registers_reset_pf
    uint32_t ucdGenCmnSnglNvmeAdminReset0VfHiuNvmeAdminRgstrsRst310; //ucd_gen_cmn_sngl_reg_nvme_admin_registers_reset_0_vf
    uint32_t ucdGenCmnSnglNvmeAdminReset1VfHiuNvmeAdminRgstrsRst6332; //ucd_gen_cmn_sngl_reg_nvme_admin_registers_reset_1_vf
    uint8_t rsvd18c[244];                 //rsvd_18c
    UcdGenCmnSnglNvmeControllerCfgEnFieldUpdatedPf_t ucdGenCmnSnglNvmeControllerCfgEnFieldUpdatedPf; //ucd_gen_cmn_sngl_reg_nvme_controller_configuration_en_field_updated_pf
    uint32_t ucdGenCmnSnglNvmeControllerCfgEnFieldUpdated0VfHiuNvmeCcEnUpdtd310; //ucd_gen_cmn_sngl_reg_nvme_controller_configuration_en_field_updated_0_vf
    uint32_t ucdGenCmnSnglNvmeControllerCfgEnFieldUpdated1VfHiuNvmeCcEnUpdtd6332; //ucd_gen_cmn_sngl_reg_nvme_controller_configuration_en_field_updated_1_vf
    uint8_t rsvd28c[244];                 //rsvd_28c
    UcdGenCmnSnglNvmeControllerCfgShnFieldUpdatedPf_t ucdGenCmnSnglNvmeControllerCfgShnFieldUpdatedPf; //ucd_gen_cmn_sngl_reg_nvme_controller_configuration_shn_field_updated_pf
    uint32_t ucdGenCmnSnglNvmeControllerCfgShnFieldUpdated0VfHiuNvmeCcShnUpdtd310; //ucd_gen_cmn_sngl_reg_nvme_controller_configuration_shn_field_updated_0_vf
    uint32_t ucdGenCmnSnglNvmeControllerCfgShnFieldUpdated1VfHiuNvmeCcShnUpdtd6332; //ucd_gen_cmn_sngl_reg_nvme_controller_configuration_shn_field_updated_1_vf
    uint8_t rsvd38c[244];                 //rsvd_38c
    UcdGenCmnSnglNvmeSubsystemResetReceivedPf_t ucdGenCmnSnglNvmeSubsystemResetReceivedPf; //ucd_gen_cmn_sngl_reg_nvme_subsystem_reset_received_pf
    uint32_t ucdGenCmnSnglNvmeSubsystemResetReceived0VfHiuNvmeResetRcvd310; //ucd_gen_cmn_sngl_reg_nvme_subsystem_reset_received_0_vf
    uint32_t ucdGenCmnSnglNvmeSubsystemResetReceived1VfHiuNvmeResetRcvd6332; //ucd_gen_cmn_sngl_reg_nvme_subsystem_reset_received_1_vf
    uint8_t rsvd48c[2844];                //rsvd_48c
    UcdGenCmnSnglDoorbellManagerStateMachineControl_t ucdGenCmnSnglDoorbellManagerStateMachineControl; //ucd_gen_cmn_sngl_reg_doorbell_manager_state_machine_control
    UcdGenCmnSnglCommonUcdErrorControlAndStatus_t ucdGenCmnSnglCommonUcdErrorControlAndStatus; //ucd_gen_cmn_sngl_reg_common_ucd_error_control_and_status
    UcdGenCmnSnglCommonAxiMaxReadReqCount_t ucdGenCmnSnglCommonAxiMaxReadReqCount; //ucd_gen_cmn_sngl_reg_common_axi_max_read_req_count
    UcdGenCmnSnglCommonAxiMaxWriteReqCount_t ucdGenCmnSnglCommonAxiMaxWriteReqCount; //ucd_gen_cmn_sngl_reg_common_axi_max_write_req_count
    UcdGenCmnSnglCommonAxiRequestStatus_t ucdGenCmnSnglCommonAxiRequestStatus; //ucd_gen_cmn_sngl_reg_common_axi_request_status
    uint32_t ucdGenCmnSnglUcdTopTraceportStatusUcdTopTraceportStatus; //ucd_gen_cmn_sngl_reg_ucd_top_traceport_status
    UcdGenCmnSnglUcdTopCfg_t ucdGenCmnSnglUcdTopCfg; //ucd_gen_cmn_sngl_reg_ucd_top_configuration
} UcdGenCmnRegisters_t;

/// @brief 0x8000
typedef struct
{
    UcducdNvmeControllerCapabilitiesLo_t ucdNvmeControllerCapabilitiesLo; //ucd_nvme_reg_controller_capabilities_lo
    UcducdNvmeControllerCapabilitiesHi_t ucdNvmeControllerCapabilitiesHi; //ucd_nvme_reg_controller_capabilities_hi
    UcducdNvmeVersion_t ucdNvmeVersion;   //ucd_nvme_reg_version
    UcducdNvmeIntrMaskSet_t ucdNvmeIntrMaskSet; //ucd_nvme_reg_interrupt_mask_set
    UcducdNvmeIntrMaskClear_t ucdNvmeIntrMaskClear; //ucd_nvme_reg_interrupt_mask_clear
    UcducdNvmeControllerCfg_t ucdNvmeControllerCfg; //ucd_nvme_reg_controller_configuration
    uint32_t ucdNvmeReserved1Rsvd1Rsvd;   //ucd_nvme_reg_reserved_1
    UcducdNvmeControllerStatus_t ucdNvmeControllerStatus; //ucd_nvme_reg_controller_status
    uint32_t ucdNvmeNvmSubsystemResetNssrNssrc; //ucd_nvme_reg_nvm_subsystem_reset
    UcducdNvmeAdminQueueAttrs_t ucdNvmeAdminQueueAttrs; //ucd_nvme_reg_admin_queue_attributes
    UcducdNvmeAdminSubmissionQueueBaseAddressLo_t ucdNvmeAdminSubmissionQueueBaseAddressLo; //ucd_nvme_reg_admin_submission_queue_base_address_lo
    uint32_t ucdNvmeAdminSubmissionQueueBaseAddressHiAsqbHi; //ucd_nvme_reg_admin_submission_queue_base_address_hi
    UcducdNvmeAdminCompletionQueueBaseAddressLo_t ucdNvmeAdminCompletionQueueBaseAddressLo; //ucd_nvme_reg_admin_completion_queue_base_address_lo
    uint32_t ucdNvmeAdminCompletionQueueBaseAddressHiAcqbHi; //ucd_nvme_reg_admin_completion_queue_base_address_hi
    UcducdNvmeControllerMemoryBufferLocation_t ucdNvmeControllerMemoryBufferLocation; //ucd_nvme_reg_controller_memory_buffer_location
    UcducdNvmeControllerMemoryBufferSize_t ucdNvmeControllerMemoryBufferSize; //ucd_nvme_reg_controller_memory_buffer_size
    uint32_t ucdNvmeReserved2Rsvd2Rsvd;   //ucd_nvme_reg_reserved_2
    uint8_t rsvd44[12];                   //rsvd_44
    UcducdNvmeIntrStatus_t ucdNvmeIntrStatus; //ucd_nvme_reg_interrupt_status
    uint8_t endPadding[940];              //end_padding
} UcdCpuVfNvmeControllerRegisters_t;

/// @brief 0x0
typedef struct
{
    UcducdNvmeControllerCapabilitiesLo_t ucdNvmeControllerCapabilitiesLo; //ucd_nvme_reg_controller_capabilities_lo
    UcducdNvmeControllerCapabilitiesHi_t ucdNvmeControllerCapabilitiesHi; //ucd_nvme_reg_controller_capabilities_hi
    UcducdNvmeVersion_t ucdNvmeVersion;   //ucd_nvme_reg_version
    UcducdNvmeIntrMaskSet_t ucdNvmeIntrMaskSet; //ucd_nvme_reg_interrupt_mask_set
    UcducdNvmeIntrMaskClear_t ucdNvmeIntrMaskClear; //ucd_nvme_reg_interrupt_mask_clear
    UcducdNvmeControllerCfg_t ucdNvmeControllerCfg; //ucd_nvme_reg_controller_configuration
    uint32_t ucdNvmeReserved1Rsvd1Rsvd;   //ucd_nvme_reg_reserved_1
    UcducdNvmeControllerStatus_t ucdNvmeControllerStatus; //ucd_nvme_reg_controller_status
    uint32_t ucdNvmeNvmSubsystemResetNssrNssrc; //ucd_nvme_reg_nvm_subsystem_reset
    UcducdNvmeAdminQueueAttrs_t ucdNvmeAdminQueueAttrs; //ucd_nvme_reg_admin_queue_attributes
    UcducdNvmeAdminSubmissionQueueBaseAddressLo_t ucdNvmeAdminSubmissionQueueBaseAddressLo; //ucd_nvme_reg_admin_submission_queue_base_address_lo
    uint32_t ucdNvmeAdminSubmissionQueueBaseAddressHiAsqbHi; //ucd_nvme_reg_admin_submission_queue_base_address_hi
    UcducdNvmeAdminCompletionQueueBaseAddressLo_t ucdNvmeAdminCompletionQueueBaseAddressLo; //ucd_nvme_reg_admin_completion_queue_base_address_lo
    uint32_t ucdNvmeAdminCompletionQueueBaseAddressHiAcqbHi; //ucd_nvme_reg_admin_completion_queue_base_address_hi
    UcducdNvmeControllerMemoryBufferLocation_t ucdNvmeControllerMemoryBufferLocation; //ucd_nvme_reg_controller_memory_buffer_location
    UcducdNvmeControllerMemoryBufferSize_t ucdNvmeControllerMemoryBufferSize; //ucd_nvme_reg_controller_memory_buffer_size
    uint32_t ucdNvmeReserved2Rsvd2Rsvd;   //ucd_nvme_reg_reserved_2
    uint8_t rsvd44[12];                   //rsvd_44
    UcducdNvmeIntrStatus_t ucdNvmeIntrStatus; //ucd_nvme_reg_interrupt_status
} UcdCpuPfNvmeControllerRegisters_t;

typedef struct
{
    UcdCpuPfNvmeControllerRegisters_t ucdCpuPfNvmeControllerRegisters;      // 0x0 : ucd_cpu_pf_nvme_controller_registers / 
    uint8_t rsvd54[32684];                                                  // 0x54 : rsvd_54 / rsvd_54
    UcdCpuVfNvmeControllerRegisters_t ucdCpuVfNvmeControllerRegisters[64];  // 0x8000 : ucd_cpu_vf_nvme_controller_registers / 
    uint8_t rsvd18000[950272];                                              // 0x18000 : rsvd_18000 / rsvd_18000
    UcdGenCmnRegisters_t ucdGenCmnRegisters;                                // 0x100000 : ucd_gen_cmn_registers / 
    uint8_t rsvd100fc4[60];                                                 // 0x100FC4 : rsvd_100fc4 / rsvd_100fc4
    UcdGenCmnIbLgc2physRegisters_t ucdGenCmnIbLgc2physRegisters[264];       // 0x101000 : ucd_gen_cmn_ib_lgc2phys_registers / 
    uint8_t rsvd101420[3040];                                               // 0x101420 : rsvd_101420 / rsvd_101420
    UcdGenCmnObLgc2physRegisters_t ucdGenCmnObLgc2physRegisters[264];       // 0x102000 : ucd_gen_cmn_ob_lgc2phys_registers / 
    uint8_t rsvd102420[515040];                                             // 0x102420 : rsvd_102420 / rsvd_102420
    UcdInboundRegisters_t ucdInboundRegisters;                              // 0x180000 : ucd_inbound_registers / 
    uint8_t rsvd187100[233216];                                             // 0x187100 : rsvd_187100 / rsvd_187100
    UcdOutboundRegisters_t ucdOutboundRegisters;                            // 0x1C0000 : ucd_outbound_registers / 
    uint8_t rsvd1c7100[233216];                                             // 0x1C7100 : rsvd_1c7100 / rsvd_1c7100
    UcdHstPfNvmeControllerRegisters_t ucdHstPfNvmeControllerRegisters;      // 0x200000 : ucd_hst_pf_nvme_controller_registers / 
    uint8_t rsvd200054[1048492];                                            // 0x200054 : rsvd_200054 / rsvd_200054
    UcdHstVfNvmeControllerRegisters_t ucdHstVfNvmeControllerRegisters[64];  // 0x300000 : ucd_hst_vf_nvme_controller_registers / 
} Ucd_t;

COMPILE_ASSERT(offsetof(Ucd_t,ucdCpuPfNvmeControllerRegisters)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t,ucdCpuVfNvmeControllerRegisters)==0x8000,"check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t,ucdGenCmnRegisters)==0x100000,"check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t,ucdGenCmnIbLgc2physRegisters)==0x101000,"check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t,ucdGenCmnObLgc2physRegisters)==0x102000,"check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t,ucdInboundRegisters)==0x180000,"check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t,ucdOutboundRegisters)==0x1C0000,"check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t,ucdHstPfNvmeControllerRegisters)==0x200000,"check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t,ucdHstVfNvmeControllerRegisters)==0x300000,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile Ucd_t rUcd; ///< 0xA1100000
