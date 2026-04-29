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
        uint32_t CAP_LO_MQES                 : 16;    ///<BIT [15:0] cap_lo_mqes
        uint32_t CAP_LO_CAP_LO_CQR           : 1;     ///<BIT [16] cap_lo_cap_lo_cqr
        uint32_t CAP_LO_AMS                  : 2;     ///<BIT [18:17] cap_lo_ams
        uint32_t CAP_LO_RSVD_RW_0            : 5;     ///<BIT [23:19] cap_lo_rsvd_rw_0
        uint32_t CAP_LO_TO                   : 8;     ///<BIT [31:24] cap_lo_to
    } b;
} UcducdNvmeControllerCapabilitiesLo_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CAP_HI_DSTRD                : 4;     ///<BIT [3:0] cap_hi_dstrd
        uint32_t CAP_HI_NSSRS                : 1;     ///<BIT [4] cap_hi_nssrs
        uint32_t CAP_HI_CSS                  : 8;     ///<BIT [12:5] cap_hi_css
        uint32_t CAP_HI_RSVD_RW_1            : 3;     ///<BIT [15:13] cap_hi_rsvd_rw_1
        uint32_t CAP_HI_MPSMIN               : 4;     ///<BIT [19:16] cap_hi_mpsmin
        uint32_t CAP_HI_MPSMAX               : 4;     ///<BIT [23:20] cap_hi_mpsmax
        uint32_t CAP_HI_RSVD_RW_0            : 8;     ///<BIT [31:24] cap_hi_rsvd_rw_0
    } b;
} UcducdNvmeControllerCapabilitiesHi_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VS_RSVD_RW_0                : 8;     ///<BIT [7:0] vs_rsvd_rw_0
        uint32_t VS_MNR                      : 8;     ///<BIT [15:8] vs_mnr
        uint32_t VS_MJR                      : 16;    ///<BIT [31:16] vs_mjr
    } b;
} UcducdNvmeVersion_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INTMS_IVMS                  : 4;     ///<BIT [3:0] intms_ivms
        uint32_t INTMS_RSVD                  : 28;    ///<BIT [31:4] intms_rsvd
    } b;
} UcducdNvmeIntrMaskSet_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INTMC_IVMC                  : 4;     ///<BIT [3:0] intmc_ivmc
        uint32_t INTMC_RSVD                  : 28;    ///<BIT [31:4] intmc_rsvd
    } b;
} UcducdNvmeIntrMaskClear_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CC_EN                       : 1;     ///<BIT [0] cc_en
        uint32_t CC_RSVD_RW_1                : 3;     ///<BIT [3:1] cc_rsvd_rw_1
        uint32_t CC_CSS                      : 3;     ///<BIT [6:4] cc_css
        uint32_t CC_MPS                      : 4;     ///<BIT [10:7] cc_mps
        uint32_t CC_AMS                      : 3;     ///<BIT [13:11] cc_ams
        uint32_t CC_SHN                      : 2;     ///<BIT [15:14] cc_shn
        uint32_t CC_IOSQES                   : 4;     ///<BIT [19:16] cc_iosqes
        uint32_t CC_IOCQES                   : 4;     ///<BIT [23:20] cc_iocqes
        uint32_t CC_RSVD_RW_0                : 8;     ///<BIT [31:24] cc_rsvd_rw_0
    } b;
} UcducdNvmeControllerCfg_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CSTS_RDY                    : 1;     ///<BIT [0] csts_rdy
        uint32_t CSTS_CFS                    : 1;     ///<BIT [1] csts_cfs
        uint32_t CSTS_SHST                   : 2;     ///<BIT [3:2] csts_shst
        uint32_t CSTS_NSSRO                  : 1;     ///<BIT [4] csts_nssro
        uint32_t CSTS_RSVD_RW_0              : 3;     ///<BIT [7:5] csts_rsvd_rw_0
        uint32_t CSTS_RSVD                   : 24;    ///<BIT [31:8] csts_rsvd
    } b;
} UcducdNvmeControllerStatus_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AQA_ASQS                    : 12;    ///<BIT [11:0] aqa_asqs
        uint32_t AQA_RSVD                    : 4;     ///<BIT [15:12] aqa_rsvd
        uint32_t AQA_ACQS                    : 12;    ///<BIT [27:16] aqa_acqs
        uint32_t AQA_RSVD1                   : 4;     ///<BIT [31:28] aqa_rsvd1
    } b;
} UcducdNvmeAdminQueueAttrs_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ASQB_LO_RSVD                : 6;     ///<BIT [5:0] asqb_lo_rsvd
        uint32_t ASQB_LO_RSVD_RW             : 6;     ///<BIT [11:6] asqb_lo_rsvd_rw
        uint32_t ASQB_LO                     : 20;    ///<BIT [31:12] asqb_lo
    } b;
} UcducdNvmeAdminSubmissionQueueBaseAddressLo_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ACQB_LO_RSVD                : 6;     ///<BIT [5:0] acqb_lo_rsvd
        uint32_t ACQB_LO_RSVD_RW             : 6;     ///<BIT [11:6] acqb_lo_rsvd_rw
        uint32_t ACQB_LO                     : 20;    ///<BIT [31:12] acqb_lo
    } b;
} UcducdNvmeAdminCompletionQueueBaseAddressLo_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMBLOC_BIR                  : 3;     ///<BIT [2:0] cmbloc_bir
        uint32_t CMBLOC_RSVD                 : 9;     ///<BIT [11:3] cmbloc_rsvd
        uint32_t CMBLOC_OFST                 : 20;    ///<BIT [31:12] cmbloc_ofst
    } b;
} UcducdNvmeControllerMemoryBufferLocation_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMBSZ_SQS                   : 1;     ///<BIT [0] cmbsz_sqs
        uint32_t CMBSZ_CQS                   : 1;     ///<BIT [1] cmbsz_cqs
        uint32_t CMBSZ_LISTS                 : 1;     ///<BIT [2] cmbsz_lists
        uint32_t CMBSZ_RDS                   : 1;     ///<BIT [3] cmbsz_rds
        uint32_t CMBSZ_WDS                   : 1;     ///<BIT [4] cmbsz_wds
        uint32_t CMBSZ_RSVD                  : 3;     ///<BIT [7:5] cmbsz_rsvd
        uint32_t CMBSZ_SZU                   : 4;     ///<BIT [11:8] cmbsz_szu
        uint32_t CMBSZ_SZ                    : 20;    ///<BIT [31:12] cmbsz_sz
    } b;
} UcducdNvmeControllerMemoryBufferSize_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IS_INTA_PENDING             : 1;     ///<BIT [0] is_inta_pending
        uint32_t IS_INTA_MASK                : 1;     ///<BIT [1] is_inta_mask
        uint32_t IS_INTA_SRC_PENDING         : 1;     ///<BIT [2] is_inta_src_pending
        uint32_t IS_INTB_PENDING             : 1;     ///<BIT [3] is_intb_pending
        uint32_t IS_INTB_MASK                : 1;     ///<BIT [4] is_intb_mask
        uint32_t IS_INTB_SRC_PENDING         : 1;     ///<BIT [5] is_intb_src_pending
        uint32_t IS_INTC_PENDING             : 1;     ///<BIT [6] is_intc_pending
        uint32_t IS_INTC_MASK                : 1;     ///<BIT [7] is_intc_mask
        uint32_t IS_INTC_SRC_PENDING         : 1;     ///<BIT [8] is_intc_src_pending
        uint32_t IS_INTD_PENDING             : 1;     ///<BIT [9] is_intd_pending
        uint32_t IS_INTD_MASK                : 1;     ///<BIT [10] is_intd_mask
        uint32_t IS_INTD_SRC_PENDING         : 1;     ///<BIT [11] is_intd_src_pending
        uint32_t IS_RSVD                     : 20;    ///<BIT [31:12] is_rsvd
    } b;
} UcducdNvmeIntrStatus_t;

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
    uint32_t ucdNvmeReserved2Rsvd2Rsvd;   //ucd_nvme_reg_reserved_2 ///< 0x40 - 0x44
    uint8_t rsvd44[12]; //rsvd_44  ///< 0x44 - 0x50
    UcducdNvmeIntrStatus_t ucdNvmeIntrStatus; //ucd_nvme_reg_interrupt_status ///< 0x50 - 0x54
} UcdCpuPfNvmeControllerRegisters_t;
static_assert(TYPE_OFFSET(UcdCpuPfNvmeControllerRegisters_t, ucdNvmeControllerCfg) == 0x14, "check register structure offset 0x14");
static_assert(TYPE_OFFSET(UcdCpuPfNvmeControllerRegisters_t, ucdNvmeControllerStatus) == 0x1c, "check register structure offset 0x1c");

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
    uint8_t rsvd44[12];  //rsvd_44 ///< 0x44 - 0x50
    UcducdNvmeIntrStatus_t ucdNvmeIntrStatus; //ucd_nvme_reg_interrupt_status ///< 0x50 - 0x54
    uint8_t endPadding[940];              //end_padding ///< 0x54 - 0x400
} UcdCpuVfNvmeControllerRegisters_t;
static_assert(TYPE_OFFSET(UcdCpuVfNvmeControllerRegisters_t, ucdNvmeControllerCfg) == 0x14, "check register structure offset 0x14");
static_assert(TYPE_OFFSET(UcdCpuVfNvmeControllerRegisters_t, ucdNvmeControllerStatus) == 0x1c, "check register structure offset 0x1c");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
