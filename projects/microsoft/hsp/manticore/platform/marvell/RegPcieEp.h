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
//! @brief PCIE_EP Registers
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
        uint32_t PCI_TYPE0_VENDOR_ID         :16;     ///<BIT [15:0] PCI_TYPE0_VENDOR_ID
        uint32_t PCI_TYPE0_DEVICE_ID         :16;     ///<BIT [31:16] PCI_TYPE0_DEVICE_ID
    } b;
} DeviceIdVendorId_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_TYPE0_IO_EN             :1;      ///<BIT [0] PCI_TYPE0_IO_EN
        uint32_t PCI_TYPE0_MEM_SPACE_EN      :1;      ///<BIT [1] PCI_TYPE0_MEM_SPACE_EN
        uint32_t PCI_TYPE0_BUS_MASTER_EN     :1;      ///<BIT [2] PCI_TYPE0_BUS_MASTER_EN
        uint32_t PCI_TYPE0_SPECIAL_CYCLE_OPERATION :1;      ///<BIT [3] PCI_TYPE0_SPECIAL_CYCLE_OPERATION
        uint32_t PCI_TYPE_MWI_ENABLE         :1;      ///<BIT [4] PCI_TYPE_MWI_ENABLE
        uint32_t PCI_TYPE_VGA_PALETTE_SNOOP  :1;      ///<BIT [5] PCI_TYPE_VGA_PALETTE_SNOOP
        uint32_t PCI_TYPE0_PARITY_ERR_EN     :1;      ///<BIT [6] PCI_TYPE0_PARITY_ERR_EN
        uint32_t PCI_TYPE_IDSEL_STEPPING     :1;      ///<BIT [7] PCI_TYPE_IDSEL_STEPPING
        uint32_t PCI_TYPE0_SERREN            :1;      ///<BIT [8] PCI_TYPE0_SERREN
        uint32_t RSVDP_9                     :1;      ///<BIT [9] RSVDP_9
        uint32_t PCI_TYPE0_INT_EN            :1;      ///<BIT [10] PCI_TYPE0_INT_EN
        uint32_t PCI_TYPE_RESERV             :5;      ///<BIT [15:11] PCI_TYPE_RESERV
        uint32_t RSVD_16                     :1;      ///<BIT [16] rsvd_16
        uint32_t RSVDP_17                    :2;      ///<BIT [18:17] RSVDP_17
        uint32_t INT_STATUS                  :1;      ///<BIT [19] INT_STATUS
        uint32_t CAP_LIST                    :1;      ///<BIT [20] CAP_LIST
        uint32_t FAST_66MHZ_CAP              :1;      ///<BIT [21] FAST_66MHZ_CAP
        uint32_t RSVDP_22                    :1;      ///<BIT [22] RSVDP_22
        uint32_t FAST_B2B_CAP                :1;      ///<BIT [23] FAST_B2B_CAP
        uint32_t MASTER_DPE                  :1;      ///<BIT [24] MASTER_DPE
        uint32_t DEV_SEL_TIMING              :2;      ///<BIT [26:25] DEV_SEL_TIMING
        uint32_t SIGNALED_TARGET_ABORT       :1;      ///<BIT [27] SIGNALED_TARGET_ABORT
        uint32_t RCVD_TARGET_ABORT           :1;      ///<BIT [28] RCVD_TARGET_ABORT
        uint32_t RCVD_MASTER_ABORT           :1;      ///<BIT [29] RCVD_MASTER_ABORT
        uint32_t SIGNALED_SYS_ERR            :1;      ///<BIT [30] SIGNALED_SYS_ERR
        uint32_t DETECTED_PARITY_ERR         :1;      ///<BIT [31] DETECTED_PARITY_ERR
    } b;
} StatusCommand_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t REVISION_ID                 :8;      ///<BIT [7:0] REVISION_ID
        uint32_t PROGRAM_INTERFACE           :8;      ///<BIT [15:8] PROGRAM_INTERFACE
        uint32_t SUBCLASS_CODE               :8;      ///<BIT [23:16] SUBCLASS_CODE
        uint32_t BASE_CLASS_CODE             :8;      ///<BIT [31:24] BASE_CLASS_CODE
    } b;
} ClassCodeRevisionId_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CACHE_LINE_SIZE             :8;      ///<BIT [7:0] CACHE_LINE_SIZE
        uint32_t LATENCY_MASTER_TIMER        :8;      ///<BIT [15:8] LATENCY_MASTER_TIMER
        uint32_t HEADER_TYPE                 :7;      ///<BIT [22:16] HEADER_TYPE
        uint32_t MULTI_FUNC                  :1;      ///<BIT [23] MULTI_FUNC
        uint32_t BIST                        :8;      ///<BIT [31:24] BIST
    } b;
} BistHeaderTypeLatencyCacheLineSize_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAR0_MEM_IO                 :1;      ///<BIT [0] BAR0_MEM_IO
        uint32_t BAR0_TYPE                   :2;      ///<BIT [2:1] BAR0_TYPE
        uint32_t BAR0_PREFETCH               :1;      ///<BIT [3] BAR0_PREFETCH
        uint32_t BAR0_START                  :28;     ///<BIT [31:4] BAR0_START
    } b;
} Bar0_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAR1_MEM_IO                 :1;      ///<BIT [0] BAR1_MEM_IO
        uint32_t BAR1_TYPE                   :2;      ///<BIT [2:1] BAR1_TYPE
        uint32_t BAR1_PREFETCH               :1;      ///<BIT [3] BAR1_PREFETCH
        uint32_t BAR1_START                  :28;     ///<BIT [31:4] BAR1_START
    } b;
} Bar1_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAR2_MEM_IO                 :1;      ///<BIT [0] BAR2_MEM_IO
        uint32_t BAR2_TYPE                   :2;      ///<BIT [2:1] BAR2_TYPE
        uint32_t BAR2_PREFETCH               :1;      ///<BIT [3] BAR2_PREFETCH
        uint32_t BAR2_START                  :28;     ///<BIT [31:4] BAR2_START
    } b;
} Bar2_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAR3_MEM_IO                 :1;      ///<BIT [0] BAR3_MEM_IO
        uint32_t BAR3_TYPE                   :2;      ///<BIT [2:1] BAR3_TYPE
        uint32_t BAR3_PREFETCH               :1;      ///<BIT [3] BAR3_PREFETCH
        uint32_t BAR3_START                  :28;     ///<BIT [31:4] BAR3_START
    } b;
} Bar3_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAR4_MEM_IO                 :1;      ///<BIT [0] BAR4_MEM_IO
        uint32_t BAR4_TYPE                   :2;      ///<BIT [2:1] BAR4_TYPE
        uint32_t BAR4_PREFETCH               :1;      ///<BIT [3] BAR4_PREFETCH
        uint32_t BAR4_START                  :28;     ///<BIT [31:4] BAR4_START
    } b;
} Bar4_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAR5_MEM_IO                 :1;      ///<BIT [0] BAR5_MEM_IO
        uint32_t BAR5_TYPE                   :2;      ///<BIT [2:1] BAR5_TYPE
        uint32_t BAR5_PREFETCH               :1;      ///<BIT [3] BAR5_PREFETCH
        uint32_t BAR5_START                  :28;     ///<BIT [31:4] BAR5_START
    } b;
} Bar5_t;

/// @brief 0x2C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SUBSYS_VENDOR_ID            :16;     ///<BIT [15:0] SUBSYS_VENDOR_ID
        uint32_t SUBSYS_DEV_ID               :16;     ///<BIT [31:16] SUBSYS_DEV_ID
    } b;
} SubsystemIdSubsystemVendorId_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ROM_BAR_ENABLE              :1;      ///<BIT [0] ROM_BAR_ENABLE
        uint32_t ROM_BAR_VALIDATION_STATUS   :3;      ///<BIT [3:1] ROM_BAR_VALIDATION_STATUS
        uint32_t ROM_BAR_VALIDATION_DETAILS  :4;      ///<BIT [7:4] ROM_BAR_VALIDATION_DETAILS
        uint32_t RSVDP_8                     :3;      ///<BIT [10:8] RSVDP_8
        uint32_t EXP_ROM_BASE_ADDRESS        :21;     ///<BIT [31:11] EXP_ROM_BASE_ADDRESS
    } b;
} ExpRomBaseAddr_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CAP_POINTER                 :8;      ///<BIT [7:0] CAP_POINTER
        uint32_t RSVDP_8                     :24;     ///<BIT [31:8] RSVDP_8
    } b;
} PciCapPtr_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INT_LINE                    :8;      ///<BIT [7:0] INT_LINE
        uint32_t INT_PIN                     :8;      ///<BIT [15:8] INT_PIN
        uint32_t RSVDP_16                    :16;     ///<BIT [31:16] RSVDP_16
    } b;
} MaxLatencyMinGrantIntrPinIntrLine_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PM_CAP_ID                   :8;      ///<BIT [7:0] PM_CAP_ID
        uint32_t PM_NEXT_POINTER             :8;      ///<BIT [15:8] PM_NEXT_POINTER
        uint32_t PM_SPEC_VER                 :3;      ///<BIT [18:16] PM_SPEC_VER
        uint32_t PME_CLK                     :1;      ///<BIT [19] PME_CLK
        uint32_t RSVD_20                     :1;      ///<BIT [20] rsvd_20
        uint32_t DSI                         :1;      ///<BIT [21] DSI
        uint32_t AUX_CURR                    :3;      ///<BIT [24:22] AUX_CURR
        uint32_t D1_SUPPORT                  :1;      ///<BIT [25] D1_SUPPORT
        uint32_t D2_SUPPORT                  :1;      ///<BIT [26] D2_SUPPORT
        uint32_t PME_SUPPORT                 :5;      ///<BIT [31:27] PME_SUPPORT
    } b;
} CapIdNxtPtr_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t POWER_STATE                 :2;      ///<BIT [1:0] POWER_STATE
        uint32_t RSVDP_2                     :1;      ///<BIT [2] RSVDP_2
        uint32_t NO_SOFT_RST                 :1;      ///<BIT [3] NO_SOFT_RST
        uint32_t RSVDP_4                     :4;      ///<BIT [7:4] RSVDP_4
        uint32_t PME_ENABLE                  :1;      ///<BIT [8] PME_ENABLE
        uint32_t DATA_SELECT                 :4;      ///<BIT [12:9] DATA_SELECT
        uint32_t DATA_SCALE                  :2;      ///<BIT [14:13] DATA_SCALE
        uint32_t PME_STATUS                  :1;      ///<BIT [15] PME_STATUS
        uint32_t RSVDP_16                    :6;      ///<BIT [21:16] RSVDP_16
        uint32_t B2_B3_SUPPORT               :1;      ///<BIT [22] B2_B3_SUPPORT
        uint32_t BUS_PWR_CLK_CON_EN          :1;      ///<BIT [23] BUS_PWR_CLK_CON_EN
        uint32_t DATA_REG_ADD_INFO           :8;      ///<BIT [31:24] DATA_REG_ADD_INFO
    } b;
} ConStatus_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSI_CAP_ID              :8;      ///<BIT [7:0] PCI_MSI_CAP_ID
        uint32_t PCI_MSI_CAP_NEXT_OFFSET     :8;      ///<BIT [15:8] PCI_MSI_CAP_NEXT_OFFSET
        uint32_t PCI_MSI_ENABLE              :1;      ///<BIT [16] PCI_MSI_ENABLE
        uint32_t PCI_MSI_MULTIPLE_MSG_CAP    :3;      ///<BIT [19:17] PCI_MSI_MULTIPLE_MSG_CAP
        uint32_t PCI_MSI_MULTIPLE_MSG_EN     :3;      ///<BIT [22:20] PCI_MSI_MULTIPLE_MSG_EN
        uint32_t PCI_MSI_64_BIT_ADDR_CAP     :1;      ///<BIT [23] PCI_MSI_64_BIT_ADDR_CAP
        uint32_t PCI_PVM_SUPPORT             :1;      ///<BIT [24] PCI_PVM_SUPPORT
        uint32_t PCI_MSI_EXT_DATA_CAP        :1;      ///<BIT [25] PCI_MSI_EXT_DATA_CAP
        uint32_t PCI_MSI_EXT_DATA_EN         :1;      ///<BIT [26] PCI_MSI_EXT_DATA_EN
        uint32_t RSVDP_27                    :5;      ///<BIT [31:27] RSVDP_27
    } b;
} PciMsiCapIdNextCtrl_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :2;      ///<BIT [1:0] RSVDP_0
        uint32_t PCI_MSI_CAP_OFF_04H         :30;     ///<BIT [31:2] PCI_MSI_CAP_OFF_04H
    } b;
} MsiCapOff04h_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSI_CAP_OFF_08H         :16;     ///<BIT [15:0] PCI_MSI_CAP_OFF_08H
        uint32_t PCI_MSI_CAP_OFF_0AH         :16;     ///<BIT [31:16] PCI_MSI_CAP_OFF_0AH
    } b;
} MsiCapOff08h_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSI_CAP_OFF_0CH         :16;     ///<BIT [15:0] PCI_MSI_CAP_OFF_0CH
        uint32_t PCI_MSI_CAP_OFF_0EH         :16;     ///<BIT [31:16] PCI_MSI_CAP_OFF_0EH
    } b;
} MsiCapOff0ch_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_ID                 :8;      ///<BIT [7:0] PCIE_CAP_ID
        uint32_t PCIE_CAP_NEXT_PTR           :8;      ///<BIT [15:8] PCIE_CAP_NEXT_PTR
        uint32_t PCIE_CAP_REG                :4;      ///<BIT [19:16] PCIE_CAP_REG
        uint32_t PCIE_DEV_PORT_TYPE          :4;      ///<BIT [23:20] PCIE_DEV_PORT_TYPE
        uint32_t PCIE_SLOT_IMP               :1;      ///<BIT [24] PCIE_SLOT_IMP
        uint32_t PCIE_INT_MSG_NUM            :5;      ///<BIT [29:25] PCIE_INT_MSG_NUM
        uint32_t RSVD                        :1;      ///<BIT [30] RSVD
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} PcieCapIdPcieNextCapPtrPcieCap_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_MAX_PAYLOAD_SIZE   :3;      ///<BIT [2:0] PCIE_CAP_MAX_PAYLOAD_SIZE
        uint32_t PCIE_CAP_PHANTOM_FUNC_SUPPORT :2;      ///<BIT [4:3] PCIE_CAP_PHANTOM_FUNC_SUPPORT
        uint32_t PCIE_CAP_EXT_TAG_SUPP       :1;      ///<BIT [5] PCIE_CAP_EXT_TAG_SUPP
        uint32_t PCIE_CAP_EP_L0S_ACCPT_LATENCY :3;      ///<BIT [8:6] PCIE_CAP_EP_L0S_ACCPT_LATENCY
        uint32_t PCIE_CAP_EP_L1_ACCPT_LATENCY :3;      ///<BIT [11:9] PCIE_CAP_EP_L1_ACCPT_LATENCY
        uint32_t RSVDP_12                    :3;      ///<BIT [14:12] RSVDP_12
        uint32_t PCIE_CAP_ROLE_BASED_ERR_REPORT :1;      ///<BIT [15] PCIE_CAP_ROLE_BASED_ERR_REPORT
        uint32_t RSVDP_16                    :1;      ///<BIT [16] RSVDP_16
        uint32_t RSVD_17                     :1;      ///<BIT [17] rsvd_17
        uint32_t PCIE_CAP_CAP_SLOT_PWR_LMT_VALUE :8;      ///<BIT [25:18] PCIE_CAP_CAP_SLOT_PWR_LMT_VALUE
        uint32_t PCIE_CAP_CAP_SLOT_PWR_LMT_SCALE :2;      ///<BIT [27:26] PCIE_CAP_CAP_SLOT_PWR_LMT_SCALE
        uint32_t PCIE_CAP_FLR_CAP            :1;      ///<BIT [28] PCIE_CAP_FLR_CAP
        uint32_t RSVD_29                     :1;      ///<BIT [29] rsvd_29
        uint32_t PCIE_CAP_TEE_IO_SUPPORTED   :1;      ///<BIT [30] PCIE_CAP_TEE_IO_SUPPORTED
        uint32_t RSVDP_31                    :1;      ///<BIT [31] RSVDP_31
    } b;
} DeviceCapabilities_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_CORR_ERR_REPORT_EN :1;      ///<BIT [0] PCIE_CAP_CORR_ERR_REPORT_EN
        uint32_t PCIE_CAP_NON_FATAL_ERR_REPORT_EN :1;      ///<BIT [1] PCIE_CAP_NON_FATAL_ERR_REPORT_EN
        uint32_t PCIE_CAP_FATAL_ERR_REPORT_EN :1;      ///<BIT [2] PCIE_CAP_FATAL_ERR_REPORT_EN
        uint32_t PCIE_CAP_UNSUPPORT_REQ_REP_EN :1;      ///<BIT [3] PCIE_CAP_UNSUPPORT_REQ_REP_EN
        uint32_t PCIE_CAP_EN_REL_ORDER       :1;      ///<BIT [4] PCIE_CAP_EN_REL_ORDER
        uint32_t PCIE_CAP_MAX_PAYLOAD_SIZE_CS :3;      ///<BIT [7:5] PCIE_CAP_MAX_PAYLOAD_SIZE_CS
        uint32_t PCIE_CAP_EXT_TAG_EN         :1;      ///<BIT [8] PCIE_CAP_EXT_TAG_EN
        uint32_t PCIE_CAP_PHANTOM_FUNC_EN    :1;      ///<BIT [9] PCIE_CAP_PHANTOM_FUNC_EN
        uint32_t PCIE_CAP_AUX_POWER_PM_EN    :1;      ///<BIT [10] PCIE_CAP_AUX_POWER_PM_EN
        uint32_t PCIE_CAP_EN_NO_SNOOP        :1;      ///<BIT [11] PCIE_CAP_EN_NO_SNOOP
        uint32_t PCIE_CAP_MAX_READ_REQ_SIZE  :3;      ///<BIT [14:12] PCIE_CAP_MAX_READ_REQ_SIZE
        uint32_t PCIE_CAP_INITIATE_FLR       :1;      ///<BIT [15] PCIE_CAP_INITIATE_FLR
        uint32_t PCIE_CAP_CORR_ERR_DETECTED  :1;      ///<BIT [16] PCIE_CAP_CORR_ERR_DETECTED
        uint32_t PCIE_CAP_NON_FATAL_ERR_DETECTED :1;      ///<BIT [17] PCIE_CAP_NON_FATAL_ERR_DETECTED
        uint32_t PCIE_CAP_FATAL_ERR_DETECTED :1;      ///<BIT [18] PCIE_CAP_FATAL_ERR_DETECTED
        uint32_t PCIE_CAP_UNSUPPORTED_REQ_DETECTED :1;      ///<BIT [19] PCIE_CAP_UNSUPPORTED_REQ_DETECTED
        uint32_t PCIE_CAP_AUX_POWER_DETECTED :1;      ///<BIT [20] PCIE_CAP_AUX_POWER_DETECTED
        uint32_t PCIE_CAP_TRANS_PENDING      :1;      ///<BIT [21] PCIE_CAP_TRANS_PENDING
        uint32_t RSVD_22                     :1;      ///<BIT [22] rsvd_22
        uint32_t RSVDP_23                    :9;      ///<BIT [31:23] RSVDP_23
    } b;
} DeviceControlDeviceStatus_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_MAX_LINK_SPEED     :4;      ///<BIT [3:0] PCIE_CAP_MAX_LINK_SPEED
        uint32_t PCIE_CAP_MAX_LINK_WIDTH     :6;      ///<BIT [9:4] PCIE_CAP_MAX_LINK_WIDTH
        uint32_t PCIE_CAP_ACTIVE_STATE_LINK_PM_SUPPORT :2;      ///<BIT [11:10] PCIE_CAP_ACTIVE_STATE_LINK_PM_SUPPORT
        uint32_t PCIE_CAP_L0S_EXIT_LATENCY   :3;      ///<BIT [14:12] PCIE_CAP_L0S_EXIT_LATENCY
        uint32_t PCIE_CAP_L1_EXIT_LATENCY    :3;      ///<BIT [17:15] PCIE_CAP_L1_EXIT_LATENCY
        uint32_t PCIE_CAP_CLOCK_POWER_MAN    :1;      ///<BIT [18] PCIE_CAP_CLOCK_POWER_MAN
        uint32_t PCIE_CAP_SURPRISE_DOWN_ERR_REP_CAP :1;      ///<BIT [19] PCIE_CAP_SURPRISE_DOWN_ERR_REP_CAP
        uint32_t PCIE_CAP_DLL_ACTIVE_REP_CAP :1;      ///<BIT [20] PCIE_CAP_DLL_ACTIVE_REP_CAP
        uint32_t PCIE_CAP_LINK_BW_NOT_CAP    :1;      ///<BIT [21] PCIE_CAP_LINK_BW_NOT_CAP
        uint32_t PCIE_CAP_ASPM_OPT_COMPLIANCE :1;      ///<BIT [22] PCIE_CAP_ASPM_OPT_COMPLIANCE
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t PCIE_CAP_PORT_NUM           :8;      ///<BIT [31:24] PCIE_CAP_PORT_NUM
    } b;
} LinkCapabilities_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_ACTIVE_STATE_LINK_PM_CONTROL :2;      ///<BIT [1:0] PCIE_CAP_ACTIVE_STATE_LINK_PM_CONTROL
        uint32_t RSVDP_2                     :1;      ///<BIT [2] RSVDP_2
        uint32_t PCIE_CAP_RCB                :1;      ///<BIT [3] PCIE_CAP_RCB
        uint32_t PCIE_CAP_LINK_DISABLE       :1;      ///<BIT [4] PCIE_CAP_LINK_DISABLE
        uint32_t PCIE_CAP_RETRAIN_LINK       :1;      ///<BIT [5] PCIE_CAP_RETRAIN_LINK
        uint32_t PCIE_CAP_COMMON_CLK_CONFIG  :1;      ///<BIT [6] PCIE_CAP_COMMON_CLK_CONFIG
        uint32_t PCIE_CAP_EXTENDED_SYNCH     :1;      ///<BIT [7] PCIE_CAP_EXTENDED_SYNCH
        uint32_t PCIE_CAP_EN_CLK_POWER_MAN   :1;      ///<BIT [8] PCIE_CAP_EN_CLK_POWER_MAN
        uint32_t PCIE_CAP_HW_AUTO_WIDTH_DISABLE :1;      ///<BIT [9] PCIE_CAP_HW_AUTO_WIDTH_DISABLE
        uint32_t PCIE_CAP_LINK_BW_MAN_INT_EN :1;      ///<BIT [10] PCIE_CAP_LINK_BW_MAN_INT_EN
        uint32_t PCIE_CAP_LINK_AUTO_BW_INT_EN :1;      ///<BIT [11] PCIE_CAP_LINK_AUTO_BW_INT_EN
        uint32_t RSVD_12_13                  :2;      ///<BIT [13:12] rsvd_12_13
        uint32_t PCIE_CAP_DRS_SIGNALING_CONTROL :2;      ///<BIT [15:14] PCIE_CAP_DRS_SIGNALING_CONTROL
        uint32_t PCIE_CAP_LINK_SPEED         :4;      ///<BIT [19:16] PCIE_CAP_LINK_SPEED
        uint32_t PCIE_CAP_NEGO_LINK_WIDTH    :6;      ///<BIT [25:20] PCIE_CAP_NEGO_LINK_WIDTH
        uint32_t RSVDP_26                    :1;      ///<BIT [26] RSVDP_26
        uint32_t PCIE_CAP_LINK_TRAINING      :1;      ///<BIT [27] PCIE_CAP_LINK_TRAINING
        uint32_t PCIE_CAP_SLOT_CLK_CONFIG    :1;      ///<BIT [28] PCIE_CAP_SLOT_CLK_CONFIG
        uint32_t PCIE_CAP_DLL_ACTIVE         :1;      ///<BIT [29] PCIE_CAP_DLL_ACTIVE
        uint32_t PCIE_CAP_LINK_BW_MAN_STATUS :1;      ///<BIT [30] PCIE_CAP_LINK_BW_MAN_STATUS
        uint32_t PCIE_CAP_LINK_AUTO_BW_STATUS :1;      ///<BIT [31] PCIE_CAP_LINK_AUTO_BW_STATUS
    } b;
} LinkControlLinkStatus_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_CPL_TIMEOUT_RANGE  :4;      ///<BIT [3:0] PCIE_CAP_CPL_TIMEOUT_RANGE
        uint32_t PCIE_CAP_CPL_TIMEOUT_DISABLE_SUPPORT :1;      ///<BIT [4] PCIE_CAP_CPL_TIMEOUT_DISABLE_SUPPORT
        uint32_t PCIE_CAP_ARI_FORWARD_SUPPORT :1;      ///<BIT [5] PCIE_CAP_ARI_FORWARD_SUPPORT
        uint32_t PCIE_CAP_ATOMIC_ROUTING_SUPP :1;      ///<BIT [6] PCIE_CAP_ATOMIC_ROUTING_SUPP
        uint32_t PCIE_CAP_32_ATOMIC_CPL_SUPP :1;      ///<BIT [7] PCIE_CAP_32_ATOMIC_CPL_SUPP
        uint32_t PCIE_CAP_64_ATOMIC_CPL_SUPP :1;      ///<BIT [8] PCIE_CAP_64_ATOMIC_CPL_SUPP
        uint32_t PCIE_CAP_128_CAS_CPL_SUPP   :1;      ///<BIT [9] PCIE_CAP_128_CAS_CPL_SUPP
        uint32_t PCIE_CAP_NO_RO_EN_PR2PR_PAR :1;      ///<BIT [10] PCIE_CAP_NO_RO_EN_PR2PR_PAR
        uint32_t PCIE_CAP_LTR_SUPP           :1;      ///<BIT [11] PCIE_CAP_LTR_SUPP
        uint32_t RSVD_12_13                  :2;      ///<BIT [13:12] rsvd_12_13
        uint32_t PCIE_CAP2_LN_SYS_CLS        :2;      ///<BIT [15:14] PCIE_CAP2_LN_SYS_CLS
        uint32_t PCIE_CAP2_10_BIT_TAG_COMP_SUPPORT :1;      ///<BIT [16] PCIE_CAP2_10_BIT_TAG_COMP_SUPPORT
        uint32_t PCIE_CAP2_10_BIT_TAG_REQ_SUPPORT :1;      ///<BIT [17] PCIE_CAP2_10_BIT_TAG_REQ_SUPPORT
        uint32_t RSVD_18_19                  :2;      ///<BIT [19:18] rsvd_18_19
        uint32_t PCIE_CAP2_CFG_EXTND_FMT_SUPPORT :1;      ///<BIT [20] PCIE_CAP2_CFG_EXTND_FMT_SUPPORT
        uint32_t PCIE_CAP2_CFG_END2END_TLP_PRFX_SUPPORT :1;      ///<BIT [21] PCIE_CAP2_CFG_END2END_TLP_PRFX_SUPPORT
        uint32_t PCIE_CAP2_CFG_MAX_END2END_TLP_PRFXS :2;      ///<BIT [23:22] PCIE_CAP2_CFG_MAX_END2END_TLP_PRFXS
        uint32_t RSVD_24_26                  :3;      ///<BIT [26:24] rsvd_24_26
        uint32_t RSVDP_27                    :1;      ///<BIT [27] RSVDP_27
        uint32_t PCIE_CAP_DMWR_CPL_SUPP      :1;      ///<BIT [28] PCIE_CAP_DMWR_CPL_SUPP
        uint32_t PCIE_CAP_DMWR_LEN_SUPP      :2;      ///<BIT [30:29] PCIE_CAP_DMWR_LEN_SUPP
        uint32_t PCIE_CAP_FRS_SUPPORTED      :1;      ///<BIT [31] PCIE_CAP_FRS_SUPPORTED
    } b;
} DeviceCapabilities2_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_CPL_TIMEOUT_VALUE  :4;      ///<BIT [3:0] PCIE_CAP_CPL_TIMEOUT_VALUE
        uint32_t PCIE_CAP_CPL_TIMEOUT_DISABLE :1;      ///<BIT [4] PCIE_CAP_CPL_TIMEOUT_DISABLE
        uint32_t PCIE_CAP_ARI_FORWARD_SUPPORT_CS :1;      ///<BIT [5] PCIE_CAP_ARI_FORWARD_SUPPORT_CS
        uint32_t RSVD_6_9                    :4;      ///<BIT [9:6] rsvd_6_9
        uint32_t PCIE_CAP_LTR_EN             :1;      ///<BIT [10] PCIE_CAP_LTR_EN
        uint32_t RSVD_11_14                  :4;      ///<BIT [14:11] rsvd_11_14
        uint32_t PCIE_CTRL2_CFG_END2END_TLP_PFX_BLCK :1;      ///<BIT [15] PCIE_CTRL2_CFG_END2END_TLP_PFX_BLCK
        uint32_t RSVDP_16                    :16;     ///<BIT [31:16] RSVDP_16
    } b;
} DeviceControl2DeviceStatus2_t;

/// @brief 0x2C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :1;      ///<BIT [0] RSVDP_0
        uint32_t PCIE_CAP_SUPPORT_LINK_SPEED_VECTOR :7;      ///<BIT [7:1] PCIE_CAP_SUPPORT_LINK_SPEED_VECTOR
        uint32_t PCIE_CAP_CROSS_LINK_SUPPORT :1;      ///<BIT [8] PCIE_CAP_CROSS_LINK_SUPPORT
        uint32_t PCIE_CAP_LWR_SKP_OS_GEN_SUP :7;      ///<BIT [15:9] PCIE_CAP_LWR_SKP_OS_GEN_SUP
        uint32_t PCIE_CAP_LWR_SKP_OS_RCV_SUP :7;      ///<BIT [22:16] PCIE_CAP_LWR_SKP_OS_RCV_SUP
        uint32_t PCIE_CAP_RETIMER_PRE_DET_SUPPORT :1;      ///<BIT [23] PCIE_CAP_RETIMER_PRE_DET_SUPPORT
        uint32_t PCIE_CAP_TWO_RETIMERS_PRE_DET_SUPPORT :1;      ///<BIT [24] PCIE_CAP_TWO_RETIMERS_PRE_DET_SUPPORT
        uint32_t RSVDP_25                    :6;      ///<BIT [30:25] RSVDP_25
        uint32_t DRS_SUPPORTED               :1;      ///<BIT [31] DRS_SUPPORTED
    } b;
} LinkCapabilities2_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_TARGET_LINK_SPEED  :4;      ///<BIT [3:0] PCIE_CAP_TARGET_LINK_SPEED
        uint32_t PCIE_CAP_ENTER_COMPLIANCE   :1;      ///<BIT [4] PCIE_CAP_ENTER_COMPLIANCE
        uint32_t PCIE_CAP_HW_AUTO_SPEED_DISABLE :1;      ///<BIT [5] PCIE_CAP_HW_AUTO_SPEED_DISABLE
        uint32_t PCIE_CAP_SEL_DEEMPHASIS     :1;      ///<BIT [6] PCIE_CAP_SEL_DEEMPHASIS
        uint32_t PCIE_CAP_TX_MARGIN          :3;      ///<BIT [9:7] PCIE_CAP_TX_MARGIN
        uint32_t PCIE_CAP_ENTER_MODIFIED_COMPLIANCE :1;      ///<BIT [10] PCIE_CAP_ENTER_MODIFIED_COMPLIANCE
        uint32_t PCIE_CAP_COMPLIANCE_SOS     :1;      ///<BIT [11] PCIE_CAP_COMPLIANCE_SOS
        uint32_t PCIE_CAP_COMPLIANCE_PRESET  :4;      ///<BIT [15:12] PCIE_CAP_COMPLIANCE_PRESET
        uint32_t PCIE_CAP_CURR_DEEMPHASIS    :1;      ///<BIT [16] PCIE_CAP_CURR_DEEMPHASIS
        uint32_t PCIE_CAP_EQ_CPL             :1;      ///<BIT [17] PCIE_CAP_EQ_CPL
        uint32_t PCIE_CAP_EQ_CPL_P1          :1;      ///<BIT [18] PCIE_CAP_EQ_CPL_P1
        uint32_t PCIE_CAP_EQ_CPL_P2          :1;      ///<BIT [19] PCIE_CAP_EQ_CPL_P2
        uint32_t PCIE_CAP_EQ_CPL_P3          :1;      ///<BIT [20] PCIE_CAP_EQ_CPL_P3
        uint32_t PCIE_CAP_LINK_EQ_REQ        :1;      ///<BIT [21] PCIE_CAP_LINK_EQ_REQ
        uint32_t PCIE_CAP_RETIMER_PRE_DET    :1;      ///<BIT [22] PCIE_CAP_RETIMER_PRE_DET
        uint32_t PCIE_CAP_TWO_RETIMERS_PRE_DET :1;      ///<BIT [23] PCIE_CAP_TWO_RETIMERS_PRE_DET
        uint32_t PCIE_CAP_CROSSLINK_RESOLUTION :2;      ///<BIT [25:24] PCIE_CAP_CROSSLINK_RESOLUTION
        uint32_t RSVD_26                     :1;      ///<BIT [26] rsvd_26
        uint32_t RSVDP_27                    :1;      ///<BIT [27] RSVDP_27
        uint32_t DOWNSTREAM_COMPO_PRESENCE   :3;      ///<BIT [30:28] DOWNSTREAM_COMPO_PRESENCE
        uint32_t DRS_MESSAGE_RECEIVED        :1;      ///<BIT [31] DRS_MESSAGE_RECEIVED
    } b;
} LinkControl2LinkStatus2_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSIX_CAP_ID             :8;      ///<BIT [7:0] PCI_MSIX_CAP_ID
        uint32_t PCI_MSIX_CAP_NEXT_OFFSET    :8;      ///<BIT [15:8] PCI_MSIX_CAP_NEXT_OFFSET
        uint32_t PCI_MSIX_TABLE_SIZE         :11;     ///<BIT [26:16] PCI_MSIX_TABLE_SIZE
        uint32_t RSVDP_27                    :3;      ///<BIT [29:27] RSVDP_27
        uint32_t PCI_MSIX_FUNCTION_MASK      :1;      ///<BIT [30] PCI_MSIX_FUNCTION_MASK
        uint32_t PCI_MSIX_ENABLE             :1;      ///<BIT [31] PCI_MSIX_ENABLE
    } b;
} PciMsixCapIdNextCtrl_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSIX_BIR                :3;      ///<BIT [2:0] PCI_MSIX_BIR
        uint32_t PCI_MSIX_TABLE_OFFSET       :29;     ///<BIT [31:3] PCI_MSIX_TABLE_OFFSET
    } b;
} MsixTableOffset_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSIX_PBA_BIR            :3;      ///<BIT [2:0] PCI_MSIX_PBA_BIR
        uint32_t PCI_MSIX_PBA_OFFSET         :29;     ///<BIT [31:3] PCI_MSIX_PBA_OFFSET
    } b;
} MsixPbaOffset_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VPD_PCIE_EXTENDED_CAP_ID    :8;      ///<BIT [7:0] VPD_PCIE_EXTENDED_CAP_ID
        uint32_t VPD_NEXT_OFFSET             :8;      ///<BIT [15:8] VPD_NEXT_OFFSET
        uint32_t VPD_ADDRESS                 :15;     ///<BIT [30:16] VPD_ADDRESS
        uint32_t VPD_FLAG                    :1;      ///<BIT [31] VPD_FLAG
    } b;
} VpdBase_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CAP_ID                      :16;     ///<BIT [15:0] CAP_ID
        uint32_t CAP_VERSION                 :4;      ///<BIT [19:16] CAP_VERSION
        uint32_t NEXT_OFFSET                 :12;     ///<BIT [31:20] NEXT_OFFSET
    } b;
} AerExtCapHdrOff_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :4;      ///<BIT [3:0] RSVDP_0
        uint32_t DL_PROTOCOL_ERR_STATUS      :1;      ///<BIT [4] DL_PROTOCOL_ERR_STATUS
        uint32_t SURPRISE_DOWN_ERR_STATUS    :1;      ///<BIT [5] SURPRISE_DOWN_ERR_STATUS
        uint32_t RSVDP_6                     :6;      ///<BIT [11:6] RSVDP_6
        uint32_t POIS_TLP_ERR_STATUS         :1;      ///<BIT [12] POIS_TLP_ERR_STATUS
        uint32_t FC_PROTOCOL_ERR_STATUS      :1;      ///<BIT [13] FC_PROTOCOL_ERR_STATUS
        uint32_t CMPLT_TIMEOUT_ERR_STATUS    :1;      ///<BIT [14] CMPLT_TIMEOUT_ERR_STATUS
        uint32_t CMPLT_ABORT_ERR_STATUS      :1;      ///<BIT [15] CMPLT_ABORT_ERR_STATUS
        uint32_t UNEXP_CMPLT_ERR_STATUS      :1;      ///<BIT [16] UNEXP_CMPLT_ERR_STATUS
        uint32_t REC_OVERFLOW_ERR_STATUS     :1;      ///<BIT [17] REC_OVERFLOW_ERR_STATUS
        uint32_t MALF_TLP_ERR_STATUS         :1;      ///<BIT [18] MALF_TLP_ERR_STATUS
        uint32_t ECRC_ERR_STATUS             :1;      ///<BIT [19] ECRC_ERR_STATUS
        uint32_t UNSUPPORTED_REQ_ERR_STATUS  :1;      ///<BIT [20] UNSUPPORTED_REQ_ERR_STATUS
        uint32_t RSVD_21                     :1;      ///<BIT [21] rsvd_21
        uint32_t INTERNAL_ERR_STATUS         :1;      ///<BIT [22] INTERNAL_ERR_STATUS
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t RSVD_24                     :1;      ///<BIT [24] rsvd_24
        uint32_t TLP_PRFX_BLOCKED_ERR_STATUS :1;      ///<BIT [25] TLP_PRFX_BLOCKED_ERR_STATUS
        uint32_t RSVD_26_27                  :2;      ///<BIT [27:26] rsvd_26_27
        uint32_t IDE_CHECK_FAILED_STATUS     :1;      ///<BIT [28] IDE_CHECK_FAILED_STATUS
        uint32_t MISROUTED_IDE_TLP_STATUS    :1;      ///<BIT [29] MISROUTED_IDE_TLP_STATUS
        uint32_t PCRC_CHECK_FAILED_STATUS    :1;      ///<BIT [30] PCRC_CHECK_FAILED_STATUS
        uint32_t RSVDP_31                    :1;      ///<BIT [31] RSVDP_31
    } b;
} UncorrErrStatusOff_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :4;      ///<BIT [3:0] RSVDP_0
        uint32_t DL_PROTOCOL_ERR_MASK        :1;      ///<BIT [4] DL_PROTOCOL_ERR_MASK
        uint32_t SURPRISE_DOWN_ERR_MASK      :1;      ///<BIT [5] SURPRISE_DOWN_ERR_MASK
        uint32_t RSVDP_6                     :6;      ///<BIT [11:6] RSVDP_6
        uint32_t POIS_TLP_ERR_MASK           :1;      ///<BIT [12] POIS_TLP_ERR_MASK
        uint32_t FC_PROTOCOL_ERR_MASK        :1;      ///<BIT [13] FC_PROTOCOL_ERR_MASK
        uint32_t CMPLT_TIMEOUT_ERR_MASK      :1;      ///<BIT [14] CMPLT_TIMEOUT_ERR_MASK
        uint32_t CMPLT_ABORT_ERR_MASK        :1;      ///<BIT [15] CMPLT_ABORT_ERR_MASK
        uint32_t UNEXP_CMPLT_ERR_MASK        :1;      ///<BIT [16] UNEXP_CMPLT_ERR_MASK
        uint32_t REC_OVERFLOW_ERR_MASK       :1;      ///<BIT [17] REC_OVERFLOW_ERR_MASK
        uint32_t MALF_TLP_ERR_MASK           :1;      ///<BIT [18] MALF_TLP_ERR_MASK
        uint32_t ECRC_ERR_MASK               :1;      ///<BIT [19] ECRC_ERR_MASK
        uint32_t UNSUPPORTED_REQ_ERR_MASK    :1;      ///<BIT [20] UNSUPPORTED_REQ_ERR_MASK
        uint32_t RSVD_21                     :1;      ///<BIT [21] rsvd_21
        uint32_t INTERNAL_ERR_MASK           :1;      ///<BIT [22] INTERNAL_ERR_MASK
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t ATOMIC_EGRESS_BLOCKED_ERR_MASK :1;      ///<BIT [24] ATOMIC_EGRESS_BLOCKED_ERR_MASK
        uint32_t TLP_PRFX_BLOCKED_ERR_MASK   :1;      ///<BIT [25] TLP_PRFX_BLOCKED_ERR_MASK
        uint32_t RSVDP_26                    :1;      ///<BIT [26] RSVDP_26
        uint32_t DMWR_EGRESS_BLOCKED_ERR_MASK :1;      ///<BIT [27] DMWR_EGRESS_BLOCKED_ERR_MASK
        uint32_t IDE_CHECK_FAILED_MASK       :1;      ///<BIT [28] IDE_CHECK_FAILED_MASK
        uint32_t MISROUTED_IDE_TLP_MASK      :1;      ///<BIT [29] MISROUTED_IDE_TLP_MASK
        uint32_t PCRC_CHECK_FAILED_MASK      :1;      ///<BIT [30] PCRC_CHECK_FAILED_MASK
        uint32_t RSVDP_31                    :1;      ///<BIT [31] RSVDP_31
    } b;
} UncorrErrMaskOff_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :4;      ///<BIT [3:0] RSVDP_0
        uint32_t DL_PROTOCOL_ERR_SEVERITY    :1;      ///<BIT [4] DL_PROTOCOL_ERR_SEVERITY
        uint32_t SURPRISE_DOWN_ERR_SVRITY    :1;      ///<BIT [5] SURPRISE_DOWN_ERR_SVRITY
        uint32_t RSVDP_6                     :6;      ///<BIT [11:6] RSVDP_6
        uint32_t POIS_TLP_ERR_SEVERITY       :1;      ///<BIT [12] POIS_TLP_ERR_SEVERITY
        uint32_t FC_PROTOCOL_ERR_SEVERITY    :1;      ///<BIT [13] FC_PROTOCOL_ERR_SEVERITY
        uint32_t CMPLT_TIMEOUT_ERR_SEVERITY  :1;      ///<BIT [14] CMPLT_TIMEOUT_ERR_SEVERITY
        uint32_t CMPLT_ABORT_ERR_SEVERITY    :1;      ///<BIT [15] CMPLT_ABORT_ERR_SEVERITY
        uint32_t UNEXP_CMPLT_ERR_SEVERITY    :1;      ///<BIT [16] UNEXP_CMPLT_ERR_SEVERITY
        uint32_t REC_OVERFLOW_ERR_SEVERITY   :1;      ///<BIT [17] REC_OVERFLOW_ERR_SEVERITY
        uint32_t MALF_TLP_ERR_SEVERITY       :1;      ///<BIT [18] MALF_TLP_ERR_SEVERITY
        uint32_t ECRC_ERR_SEVERITY           :1;      ///<BIT [19] ECRC_ERR_SEVERITY
        uint32_t UNSUPPORTED_REQ_ERR_SEVERITY :1;      ///<BIT [20] UNSUPPORTED_REQ_ERR_SEVERITY
        uint32_t RSVD_21                     :1;      ///<BIT [21] rsvd_21
        uint32_t INTERNAL_ERR_SEVERITY       :1;      ///<BIT [22] INTERNAL_ERR_SEVERITY
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t RSVD_24                     :1;      ///<BIT [24] rsvd_24
        uint32_t TLP_PRFX_BLOCKED_ERR_SEVERITY :1;      ///<BIT [25] TLP_PRFX_BLOCKED_ERR_SEVERITY
        uint32_t RSVD_26                     :1;      ///<BIT [26] rsvd_26
        uint32_t DMWR_EGRESS_BLOCKED_ERR_SEVERITY :1;      ///<BIT [27] DMWR_EGRESS_BLOCKED_ERR_SEVERITY
        uint32_t IDE_CHECK_FAILED_SEV        :1;      ///<BIT [28] IDE_CHECK_FAILED_SEV
        uint32_t MISROUTED_IDE_TLP_SEV       :1;      ///<BIT [29] MISROUTED_IDE_TLP_SEV
        uint32_t PCRC_CHECK_FAILED_SEV       :1;      ///<BIT [30] PCRC_CHECK_FAILED_SEV
        uint32_t RSVDP_31                    :1;      ///<BIT [31] RSVDP_31
    } b;
} UncorrErrSevOff_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_ERR_STATUS               :1;      ///<BIT [0] RX_ERR_STATUS
        uint32_t RSVDP_1                     :5;      ///<BIT [5:1] RSVDP_1
        uint32_t BAD_TLP_STATUS              :1;      ///<BIT [6] BAD_TLP_STATUS
        uint32_t BAD_DLLP_STATUS             :1;      ///<BIT [7] BAD_DLLP_STATUS
        uint32_t REPLAY_NO_ROLEOVER_STATUS   :1;      ///<BIT [8] REPLAY_NO_ROLEOVER_STATUS
        uint32_t RSVDP_9                     :3;      ///<BIT [11:9] RSVDP_9
        uint32_t RPL_TIMER_TIMEOUT_STATUS    :1;      ///<BIT [12] RPL_TIMER_TIMEOUT_STATUS
        uint32_t ADVISORY_NON_FATAL_ERR_STATUS :1;      ///<BIT [13] ADVISORY_NON_FATAL_ERR_STATUS
        uint32_t CORRECTED_INT_ERR_STATUS    :1;      ///<BIT [14] CORRECTED_INT_ERR_STATUS
        uint32_t HEADER_LOG_OVERFLOW_STATUS  :1;      ///<BIT [15] HEADER_LOG_OVERFLOW_STATUS
        uint32_t RSVDP_16                    :16;     ///<BIT [31:16] RSVDP_16
    } b;
} CorrErrStatusOff_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_ERR_MASK                 :1;      ///<BIT [0] RX_ERR_MASK
        uint32_t RSVDP_1                     :5;      ///<BIT [5:1] RSVDP_1
        uint32_t BAD_TLP_MASK                :1;      ///<BIT [6] BAD_TLP_MASK
        uint32_t BAD_DLLP_MASK               :1;      ///<BIT [7] BAD_DLLP_MASK
        uint32_t REPLAY_NO_ROLEOVER_MASK     :1;      ///<BIT [8] REPLAY_NO_ROLEOVER_MASK
        uint32_t RSVDP_9                     :3;      ///<BIT [11:9] RSVDP_9
        uint32_t RPL_TIMER_TIMEOUT_MASK      :1;      ///<BIT [12] RPL_TIMER_TIMEOUT_MASK
        uint32_t ADVISORY_NON_FATAL_ERR_MASK :1;      ///<BIT [13] ADVISORY_NON_FATAL_ERR_MASK
        uint32_t CORRECTED_INT_ERR_MASK      :1;      ///<BIT [14] CORRECTED_INT_ERR_MASK
        uint32_t HEADER_LOG_OVERFLOW_MASK    :1;      ///<BIT [15] HEADER_LOG_OVERFLOW_MASK
        uint32_t RSVDP_16                    :16;     ///<BIT [31:16] RSVDP_16
    } b;
} CorrErrMaskOff_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FIRST_ERR_POINTER           :5;      ///<BIT [4:0] FIRST_ERR_POINTER
        uint32_t ECRC_GEN_CAP                :1;      ///<BIT [5] ECRC_GEN_CAP
        uint32_t ECRC_GEN_EN                 :1;      ///<BIT [6] ECRC_GEN_EN
        uint32_t ECRC_CHECK_CAP              :1;      ///<BIT [7] ECRC_CHECK_CAP
        uint32_t ECRC_CHECK_EN               :1;      ///<BIT [8] ECRC_CHECK_EN
        uint32_t MULTIPLE_HEADER_CAP         :1;      ///<BIT [9] MULTIPLE_HEADER_CAP
        uint32_t MULTIPLE_HEADER_EN          :1;      ///<BIT [10] MULTIPLE_HEADER_EN
        uint32_t TLP_PRFX_LOG_PRESENT        :1;      ///<BIT [11] TLP_PRFX_LOG_PRESENT
        uint32_t CTO_PRFX_HDR_LOG_CAP        :1;      ///<BIT [12] CTO_PRFX_HDR_LOG_CAP
        uint32_t RSVD_13_23                  :11;     ///<BIT [23:13] rsvd_13_23
        uint32_t RSVDP_24                    :8;      ///<BIT [31:24] RSVDP_24
    } b;
} AdvErrCapCtrlOff_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FIRST_DWORD_FIRST_BYTE      :8;      ///<BIT [7:0] FIRST_DWORD_FIRST_BYTE
        uint32_t FIRST_DWORD_SECOND_BYTE     :8;      ///<BIT [15:8] FIRST_DWORD_SECOND_BYTE
        uint32_t FIRST_DWORD_THIRD_BYTE      :8;      ///<BIT [23:16] FIRST_DWORD_THIRD_BYTE
        uint32_t FIRST_DWORD_FOURTH_BYTE     :8;      ///<BIT [31:24] FIRST_DWORD_FOURTH_BYTE
    } b;
} HdrLog0Off_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SECOND_DWORD_FIRST_BYTE     :8;      ///<BIT [7:0] SECOND_DWORD_FIRST_BYTE
        uint32_t SECOND_DWORD_SECOND_BYTE    :8;      ///<BIT [15:8] SECOND_DWORD_SECOND_BYTE
        uint32_t SECOND_DWORD_THIRD_BYTE     :8;      ///<BIT [23:16] SECOND_DWORD_THIRD_BYTE
        uint32_t SECOND_DWORD_FOURTH_BYTE    :8;      ///<BIT [31:24] SECOND_DWORD_FOURTH_BYTE
    } b;
} HdrLog1Off_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t THIRD_DWORD_FIRST_BYTE      :8;      ///<BIT [7:0] THIRD_DWORD_FIRST_BYTE
        uint32_t THIRD_DWORD_SECOND_BYTE     :8;      ///<BIT [15:8] THIRD_DWORD_SECOND_BYTE
        uint32_t THIRD_DWORD_THIRD_BYTE      :8;      ///<BIT [23:16] THIRD_DWORD_THIRD_BYTE
        uint32_t THIRD_DWORD_FOURTH_BYTE     :8;      ///<BIT [31:24] THIRD_DWORD_FOURTH_BYTE
    } b;
} HdrLog2Off_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FOURTH_DWORD_FIRST_BYTE     :8;      ///<BIT [7:0] FOURTH_DWORD_FIRST_BYTE
        uint32_t FOURTH_DWORD_SECOND_BYTE    :8;      ///<BIT [15:8] FOURTH_DWORD_SECOND_BYTE
        uint32_t FOURTH_DWORD_THIRD_BYTE     :8;      ///<BIT [23:16] FOURTH_DWORD_THIRD_BYTE
        uint32_t FOURTH_DWORD_FOURTH_BYTE    :8;      ///<BIT [31:24] FOURTH_DWORD_FOURTH_BYTE
    } b;
} HdrLog3Off_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_TLP_PFX_LOG_1_FIRST_BYTE :8;      ///<BIT [7:0] CFG_TLP_PFX_LOG_1_FIRST_BYTE
        uint32_t CFG_TLP_PFX_LOG_1_SECOND_BYTE :8;      ///<BIT [15:8] CFG_TLP_PFX_LOG_1_SECOND_BYTE
        uint32_t CFG_TLP_PFX_LOG_1_THIRD_BYTE :8;      ///<BIT [23:16] CFG_TLP_PFX_LOG_1_THIRD_BYTE
        uint32_t CFG_TLP_PFX_LOG_1_FOURTH_BYTE :8;      ///<BIT [31:24] CFG_TLP_PFX_LOG_1_FOURTH_BYTE
    } b;
} TlpPrefixLog1Off_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_TLP_PFX_LOG_2_FIRST_BYTE :8;      ///<BIT [7:0] CFG_TLP_PFX_LOG_2_FIRST_BYTE
        uint32_t CFG_TLP_PFX_LOG_2_SECOND_BYTE :8;      ///<BIT [15:8] CFG_TLP_PFX_LOG_2_SECOND_BYTE
        uint32_t CFG_TLP_PFX_LOG_2_THIRD_BYTE :8;      ///<BIT [23:16] CFG_TLP_PFX_LOG_2_THIRD_BYTE
        uint32_t CFG_TLP_PFX_LOG_2_FOURTH_BYTE :8;      ///<BIT [31:24] CFG_TLP_PFX_LOG_2_FOURTH_BYTE
    } b;
} TlpPrefixLog2Off_t;

/// @brief 0x40
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_TLP_PFX_LOG_3_FIRST_BYTE :8;      ///<BIT [7:0] CFG_TLP_PFX_LOG_3_FIRST_BYTE
        uint32_t CFG_TLP_PFX_LOG_3_SECOND_BYTE :8;      ///<BIT [15:8] CFG_TLP_PFX_LOG_3_SECOND_BYTE
        uint32_t CFG_TLP_PFX_LOG_3_THIRD_BYTE :8;      ///<BIT [23:16] CFG_TLP_PFX_LOG_3_THIRD_BYTE
        uint32_t CFG_TLP_PFX_LOG_3_FOURTH_BYTE :8;      ///<BIT [31:24] CFG_TLP_PFX_LOG_3_FOURTH_BYTE
    } b;
} TlpPrefixLog3Off_t;

/// @brief 0x44
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_TLP_PFX_LOG_4_FIRST_BYTE :8;      ///<BIT [7:0] CFG_TLP_PFX_LOG_4_FIRST_BYTE
        uint32_t CFG_TLP_PFX_LOG_4_SECOND_BYTE :8;      ///<BIT [15:8] CFG_TLP_PFX_LOG_4_SECOND_BYTE
        uint32_t CFG_TLP_PFX_LOG_4_THIRD_BYTE :8;      ///<BIT [23:16] CFG_TLP_PFX_LOG_4_THIRD_BYTE
        uint32_t CFG_TLP_PFX_LOG_4_FOURTH_BYTE :8;      ///<BIT [31:24] CFG_TLP_PFX_LOG_4_FOURTH_BYTE
    } b;
} TlpPrefixLog4Off_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SN_PCIE_EXTENDED_CAP_ID     :16;     ///<BIT [15:0] SN_PCIE_EXTENDED_CAP_ID
        uint32_t SN_CAP_VERSION              :4;      ///<BIT [19:16] SN_CAP_VERSION
        uint32_t SN_NEXT_OFFSET              :12;     ///<BIT [31:20] SN_NEXT_OFFSET
    } b;
} SnBase_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PB_PCIE_EXTENDED_CAP_ID     :16;     ///<BIT [15:0] PB_PCIE_EXTENDED_CAP_ID
        uint32_t PB_CAP_VERSION              :4;      ///<BIT [19:16] PB_CAP_VERSION
        uint32_t PB_NEXT_OFFSET              :12;     ///<BIT [31:20] PB_NEXT_OFFSET
    } b;
} PbBase_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PB_DATA_SEL                 :8;      ///<BIT [7:0] PB_DATA_SEL
        uint32_t RSVDP_8                     :8;      ///<BIT [15:8] RSVDP_8
        uint32_t RSVD_16_24                  :9;      ///<BIT [24:16] rsvd_16_24
        uint32_t RSVDP_25                    :7;      ///<BIT [31:25] RSVDP_25
    } b;
} PbDataSelect_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PB_BASE_POWER               :8;      ///<BIT [7:0] PB_BASE_POWER
        uint32_t PB_DATA_SCALE               :2;      ///<BIT [9:8] PB_DATA_SCALE
        uint32_t PB_PM_SUB_STATE             :3;      ///<BIT [12:10] PB_PM_SUB_STATE
        uint32_t PB_PM_STATE                 :2;      ///<BIT [14:13] PB_PM_STATE
        uint32_t PB_TYPE                     :3;      ///<BIT [17:15] PB_TYPE
        uint32_t PB_POWER_RAIL_STATE         :3;      ///<BIT [20:18] PB_POWER_RAIL_STATE
        uint32_t PB_DATA_SCALE2              :1;      ///<BIT [21] PB_DATA_SCALE2
        uint32_t PB_CONNECTOR_NUM            :3;      ///<BIT [24:22] PB_CONNECTOR_NUM
        uint32_t PB_CONNECTOR_TYPE           :6;      ///<BIT [30:25] PB_CONNECTOR_TYPE
        uint32_t PB_EXT_PRESENT              :1;      ///<BIT [31] PB_EXT_PRESENT
    } b;
} DataPb_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PB_SYS_ALLOC                :1;      ///<BIT [0] PB_SYS_ALLOC
        uint32_t RSVD_1_31                   :31;     ///<BIT [31:1] rsvd_1_31
    } b;
} CapPb_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ARI_PCIE_EXTENDED_CAP_ID    :16;     ///<BIT [15:0] ARI_PCIE_EXTENDED_CAP_ID
        uint32_t ARI_CAP_VERSION             :4;      ///<BIT [19:16] ARI_CAP_VERSION
        uint32_t ARI_NEXT_OFFSET             :12;     ///<BIT [31:20] ARI_NEXT_OFFSET
    } b;
} AriBase_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ARI_MFVC_FUN_GRP_CAP        :1;      ///<BIT [0] ARI_MFVC_FUN_GRP_CAP
        uint32_t ARI_ACS_FUN_GRP_CAP         :1;      ///<BIT [1] ARI_ACS_FUN_GRP_CAP
        uint32_t RSVDP_2                     :6;      ///<BIT [7:2] RSVDP_2
        uint32_t ARI_NEXT_FUN_NUM            :8;      ///<BIT [15:8] ARI_NEXT_FUN_NUM
        uint32_t ARI_MFVC_FUN_GRP_EN         :1;      ///<BIT [16] ARI_MFVC_FUN_GRP_EN
        uint32_t ARI_ACS_FUN_GRP_EN          :1;      ///<BIT [17] ARI_ACS_FUN_GRP_EN
        uint32_t RSVDP_18                    :2;      ///<BIT [19:18] RSVDP_18
        uint32_t ARI_FUN_GRP                 :3;      ///<BIT [22:20] ARI_FUN_GRP
        uint32_t RSVDP_23                    :9;      ///<BIT [31:23] RSVDP_23
    } b;
} Cap_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EXTENDED_CAP_ID             :16;     ///<BIT [15:0] EXTENDED_CAP_ID
        uint32_t CAP_VERSION                 :4;      ///<BIT [19:16] CAP_VERSION
        uint32_t NEXT_OFFSET                 :12;     ///<BIT [31:20] NEXT_OFFSET
    } b;
} SpcieCapHeader_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PERFORM_EQ                  :1;      ///<BIT [0] PERFORM_EQ
        uint32_t EQ_REQ_INT_EN               :1;      ///<BIT [1] EQ_REQ_INT_EN
        uint32_t RSVDP_2                     :30;     ///<BIT [31:2] RSVDP_2
    } b;
} LinkControl3_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LANE_ERR_STATUS             :4;      ///<BIT [3:0] LANE_ERR_STATUS
        uint32_t RSVDP_LANE_ERR_STATUS       :28;     ///<BIT [31:4] RSVDP_LANE_ERR_STATUS
    } b;
} LaneErrStatus_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DSP_TX_PRESET0              :4;      ///<BIT [3:0] DSP_TX_PRESET0
        uint32_t DSP_RX_PRESET_HINT0         :3;      ///<BIT [6:4] DSP_RX_PRESET_HINT0
        uint32_t RSVDP_7                     :1;      ///<BIT [7] RSVDP_7
        uint32_t USP_TX_PRESET0              :4;      ///<BIT [11:8] USP_TX_PRESET0
        uint32_t USP_RX_PRESET_HINT0         :3;      ///<BIT [14:12] USP_RX_PRESET_HINT0
        uint32_t RSVDP_15                    :1;      ///<BIT [15] RSVDP_15
        uint32_t DSP_TX_PRESET1              :4;      ///<BIT [19:16] DSP_TX_PRESET1
        uint32_t DSP_RX_PRESET_HINT1         :3;      ///<BIT [22:20] DSP_RX_PRESET_HINT1
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t USP_TX_PRESET1              :4;      ///<BIT [27:24] USP_TX_PRESET1
        uint32_t USP_RX_PRESET_HINT1         :3;      ///<BIT [30:28] USP_RX_PRESET_HINT1
        uint32_t RSVDP_31                    :1;      ///<BIT [31] RSVDP_31
    } b;
} SpcieCapOff0ch_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DSP_TX_PRESET2              :4;      ///<BIT [3:0] DSP_TX_PRESET2
        uint32_t DSP_RX_PRESET_HINT2         :3;      ///<BIT [6:4] DSP_RX_PRESET_HINT2
        uint32_t RSVDP_7                     :1;      ///<BIT [7] RSVDP_7
        uint32_t USP_TX_PRESET2              :4;      ///<BIT [11:8] USP_TX_PRESET2
        uint32_t USP_RX_PRESET_HINT2         :3;      ///<BIT [14:12] USP_RX_PRESET_HINT2
        uint32_t RSVDP_15                    :1;      ///<BIT [15] RSVDP_15
        uint32_t DSP_TX_PRESET3              :4;      ///<BIT [19:16] DSP_TX_PRESET3
        uint32_t DSP_RX_PRESET_HINT3         :3;      ///<BIT [22:20] DSP_RX_PRESET_HINT3
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t USP_TX_PRESET3              :4;      ///<BIT [27:24] USP_TX_PRESET3
        uint32_t USP_RX_PRESET_HINT3         :3;      ///<BIT [30:28] USP_RX_PRESET_HINT3
        uint32_t RSVDP_31                    :1;      ///<BIT [31] RSVDP_31
    } b;
} SpcieCapOff10h_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EXTENDED_CAP_ID             :16;     ///<BIT [15:0] EXTENDED_CAP_ID
        uint32_t CAP_VERSION                 :4;      ///<BIT [19:16] CAP_VERSION
        uint32_t NEXT_OFFSET                 :12;     ///<BIT [31:20] NEXT_OFFSET
    } b;
} Pl16gExtCapHdr_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EQ_16G_CPL                  :1;      ///<BIT [0] EQ_16G_CPL
        uint32_t EQ_16G_CPL_P1               :1;      ///<BIT [1] EQ_16G_CPL_P1
        uint32_t EQ_16G_CPL_P2               :1;      ///<BIT [2] EQ_16G_CPL_P2
        uint32_t EQ_16G_CPL_P3               :1;      ///<BIT [3] EQ_16G_CPL_P3
        uint32_t LINK_EQ_16G_REQ             :1;      ///<BIT [4] LINK_EQ_16G_REQ
        uint32_t RSVDP_5                     :27;     ///<BIT [31:5] RSVDP_5
    } b;
} Pl16gStatus_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LC_DPAR_STATUS              :4;      ///<BIT [3:0] LC_DPAR_STATUS
        uint32_t RSVDP_LC_DPAR_STATUS        :28;     ///<BIT [31:4] RSVDP_LC_DPAR_STATUS
    } b;
} Pl16gLcDparStatus_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FIRST_RETIMER_DPAR_STATUS   :4;      ///<BIT [3:0] FIRST_RETIMER_DPAR_STATUS
        uint32_t RSVDP_FIRST_RETIMER_DPAR_STATUS :28;     ///<BIT [31:4] RSVDP_FIRST_RETIMER_DPAR_STATUS
    } b;
} Pl16gFirstRetimerDparStatus_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SECOND_RETIMER_DPAR_STATUS  :4;      ///<BIT [3:0] SECOND_RETIMER_DPAR_STATUS
        uint32_t RSVDP_SECOND_RETIMER_DPAR_STATUS :28;     ///<BIT [31:4] RSVDP_SECOND_RETIMER_DPAR_STATUS
    } b;
} Pl16gSecondRetimerDparStatus_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DSP_16G_TX_PRESET0          :4;      ///<BIT [3:0] DSP_16G_TX_PRESET0
        uint32_t USP_16G_TX_PRESET0          :4;      ///<BIT [7:4] USP_16G_TX_PRESET0
        uint32_t DSP_16G_TX_PRESET1          :4;      ///<BIT [11:8] DSP_16G_TX_PRESET1
        uint32_t USP_16G_TX_PRESET1          :4;      ///<BIT [15:12] USP_16G_TX_PRESET1
        uint32_t DSP_16G_TX_PRESET2          :4;      ///<BIT [19:16] DSP_16G_TX_PRESET2
        uint32_t USP_16G_TX_PRESET2          :4;      ///<BIT [23:20] USP_16G_TX_PRESET2
        uint32_t DSP_16G_TX_PRESET3          :4;      ///<BIT [27:24] DSP_16G_TX_PRESET3
        uint32_t USP_16G_TX_PRESET3          :4;      ///<BIT [31:28] USP_16G_TX_PRESET3
    } b;
} Pl16gCapOff20h_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EXTENDED_CAP_ID             :16;     ///<BIT [15:0] EXTENDED_CAP_ID
        uint32_t CAP_VERSION                 :4;      ///<BIT [19:16] CAP_VERSION
        uint32_t NEXT_OFFSET                 :12;     ///<BIT [31:20] NEXT_OFFSET
    } b;
} MarginExtCapHdr_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MARGINING_USES_DRIVER_SOFTWARE :1;      ///<BIT [0] MARGINING_USES_DRIVER_SOFTWARE
        uint32_t RSVDP_1                     :15;     ///<BIT [15:1] RSVDP_1
        uint32_t MARGINING_READY             :1;      ///<BIT [16] MARGINING_READY
        uint32_t MARGINING_SOFTWARE_READY    :1;      ///<BIT [17] MARGINING_SOFTWARE_READY
        uint32_t RSVDP_18                    :14;     ///<BIT [31:18] RSVDP_18
    } b;
} MarginPortCapabilitiesStatus_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RECEIVER_NUMBER             :3;      ///<BIT [2:0] RECEIVER_NUMBER
        uint32_t MARGIN_TYPE                 :3;      ///<BIT [5:3] MARGIN_TYPE
        uint32_t USAGE_MODEL                 :1;      ///<BIT [6] USAGE_MODEL
        uint32_t RSVDP_7                     :1;      ///<BIT [7] RSVDP_7
        uint32_t MARGIN_PAYLOAD              :8;      ///<BIT [15:8] MARGIN_PAYLOAD
        uint32_t RECEIVER_NUMBER_STATUS      :3;      ///<BIT [18:16] RECEIVER_NUMBER_STATUS
        uint32_t MARGIN_TYPE_STATUS          :3;      ///<BIT [21:19] MARGIN_TYPE_STATUS
        uint32_t USAGE_MODEL_STATUS          :1;      ///<BIT [22] USAGE_MODEL_STATUS
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t MARGIN_PAYLOAD_STATUS       :8;      ///<BIT [31:24] MARGIN_PAYLOAD_STATUS
    } b;
} MarginLaneCntrlStatus0_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RECEIVER_NUMBER             :3;      ///<BIT [2:0] RECEIVER_NUMBER
        uint32_t MARGIN_TYPE                 :3;      ///<BIT [5:3] MARGIN_TYPE
        uint32_t USAGE_MODEL                 :1;      ///<BIT [6] USAGE_MODEL
        uint32_t RSVDP_7                     :1;      ///<BIT [7] RSVDP_7
        uint32_t MARGIN_PAYLOAD              :8;      ///<BIT [15:8] MARGIN_PAYLOAD
        uint32_t RECEIVER_NUMBER_STATUS      :3;      ///<BIT [18:16] RECEIVER_NUMBER_STATUS
        uint32_t MARGIN_TYPE_STATUS          :3;      ///<BIT [21:19] MARGIN_TYPE_STATUS
        uint32_t USAGE_MODEL_STATUS          :1;      ///<BIT [22] USAGE_MODEL_STATUS
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t MARGIN_PAYLOAD_STATUS       :8;      ///<BIT [31:24] MARGIN_PAYLOAD_STATUS
    } b;
} MarginLaneCntrlStatus1_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RECEIVER_NUMBER             :3;      ///<BIT [2:0] RECEIVER_NUMBER
        uint32_t MARGIN_TYPE                 :3;      ///<BIT [5:3] MARGIN_TYPE
        uint32_t USAGE_MODEL                 :1;      ///<BIT [6] USAGE_MODEL
        uint32_t RSVDP_7                     :1;      ///<BIT [7] RSVDP_7
        uint32_t MARGIN_PAYLOAD              :8;      ///<BIT [15:8] MARGIN_PAYLOAD
        uint32_t RECEIVER_NUMBER_STATUS      :3;      ///<BIT [18:16] RECEIVER_NUMBER_STATUS
        uint32_t MARGIN_TYPE_STATUS          :3;      ///<BIT [21:19] MARGIN_TYPE_STATUS
        uint32_t USAGE_MODEL_STATUS          :1;      ///<BIT [22] USAGE_MODEL_STATUS
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t MARGIN_PAYLOAD_STATUS       :8;      ///<BIT [31:24] MARGIN_PAYLOAD_STATUS
    } b;
} MarginLaneCntrlStatus2_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RECEIVER_NUMBER             :3;      ///<BIT [2:0] RECEIVER_NUMBER
        uint32_t MARGIN_TYPE                 :3;      ///<BIT [5:3] MARGIN_TYPE
        uint32_t USAGE_MODEL                 :1;      ///<BIT [6] USAGE_MODEL
        uint32_t RSVDP_7                     :1;      ///<BIT [7] RSVDP_7
        uint32_t MARGIN_PAYLOAD              :8;      ///<BIT [15:8] MARGIN_PAYLOAD
        uint32_t RECEIVER_NUMBER_STATUS      :3;      ///<BIT [18:16] RECEIVER_NUMBER_STATUS
        uint32_t MARGIN_TYPE_STATUS          :3;      ///<BIT [21:19] MARGIN_TYPE_STATUS
        uint32_t USAGE_MODEL_STATUS          :1;      ///<BIT [22] USAGE_MODEL_STATUS
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t MARGIN_PAYLOAD_STATUS       :8;      ///<BIT [31:24] MARGIN_PAYLOAD_STATUS
    } b;
} MarginLaneCntrlStatus3_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EXTENDED_CAP_ID             :16;     ///<BIT [15:0] EXTENDED_CAP_ID
        uint32_t CAP_VERSION                 :4;      ///<BIT [19:16] CAP_VERSION
        uint32_t NEXT_OFFSET                 :12;     ///<BIT [31:20] NEXT_OFFSET
    } b;
} Pl32gExtCapHdr_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EQ_BYPASS_HIGHEST_RATE_SUPPORT :1;      ///<BIT [0] EQ_BYPASS_HIGHEST_RATE_SUPPORT
        uint32_t NO_EQ_NEEDED_SUPPORT        :1;      ///<BIT [1] NO_EQ_NEEDED_SUPPORT
        uint32_t RSVDP_2                     :6;      ///<BIT [7:2] RSVDP_2
        uint32_t MOD_TS_PCIE_SUPPORT         :1;      ///<BIT [8] MOD_TS_PCIE_SUPPORT
        uint32_t MOD_TS_TRAING_SET_MSG_SUPPORT :1;      ///<BIT [9] MOD_TS_TRAING_SET_MSG_SUPPORT
        uint32_t RSVD_10                     :1;      ///<BIT [10] rsvd_10
        uint32_t MOD_TS_RSVD_USAGE_MODE      :5;      ///<BIT [15:11] MOD_TS_RSVD_USAGE_MODE
        uint32_t RSVDP_16                    :16;     ///<BIT [31:16] RSVDP_16
    } b;
} Pl32gCapability_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EQ_BYPASS_HIGHEST_RATE_DISABLE :1;      ///<BIT [0] EQ_BYPASS_HIGHEST_RATE_DISABLE
        uint32_t NO_EQ_NEEDED_DISABLE        :1;      ///<BIT [1] NO_EQ_NEEDED_DISABLE
        uint32_t RSVDP_2                     :6;      ///<BIT [7:2] RSVDP_2
        uint32_t MOD_TS_USAGE_MODE_SELECT    :3;      ///<BIT [10:8] MOD_TS_USAGE_MODE_SELECT
        uint32_t RSVDP_11                    :21;     ///<BIT [31:11] RSVDP_11
    } b;
} Pl32gControl_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EQ_32G_CPL                  :1;      ///<BIT [0] EQ_32G_CPL
        uint32_t EQ_32G_CPL_P1               :1;      ///<BIT [1] EQ_32G_CPL_P1
        uint32_t EQ_32G_CPL_P2               :1;      ///<BIT [2] EQ_32G_CPL_P2
        uint32_t EQ_32G_CPL_P3               :1;      ///<BIT [3] EQ_32G_CPL_P3
        uint32_t LINK_EQ_32G_REQ             :1;      ///<BIT [4] LINK_EQ_32G_REQ
        uint32_t MOD_TS_RCVD                 :1;      ///<BIT [5] MOD_TS_RCVD
        uint32_t RX_ENH_LINK_BEHAVIOR_CTRL   :2;      ///<BIT [7:6] RX_ENH_LINK_BEHAVIOR_CTRL
        uint32_t TX_PRECODING_ON             :1;      ///<BIT [8] TX_PRECODING_ON
        uint32_t TX_PRECODE_REQ              :1;      ///<BIT [9] TX_PRECODE_REQ
        uint32_t NO_EQ_NEEDED_RCVD           :1;      ///<BIT [10] NO_EQ_NEEDED_RCVD
        uint32_t RSVDP_11                    :21;     ///<BIT [31:11] RSVDP_11
    } b;
} Pl32gStatus_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RCVD_MOD_TS_USAGE_MODE      :3;      ///<BIT [2:0] RCVD_MOD_TS_USAGE_MODE
        uint32_t RCVD_MOD_TS_INFO1           :13;     ///<BIT [15:3] RCVD_MOD_TS_INFO1
        uint32_t RCVD_MOD_TS_VENDER_ID       :16;     ///<BIT [31:16] RCVD_MOD_TS_VENDER_ID
    } b;
} Pl32gRcvdModTsData1_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RCVD_MOD_TS_INFO2           :24;     ///<BIT [23:0] RCVD_MOD_TS_INFO2
        uint32_t RCVD_ALT_PROTOCOL_NEGO_STATUS :2;      ///<BIT [25:24] RCVD_ALT_PROTOCOL_NEGO_STATUS
        uint32_t RSVDP_26                    :6;      ///<BIT [31:26] RSVDP_26
    } b;
} Pl32gRcvdModTsData2_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_MOD_TS_USAGE_MODE        :3;      ///<BIT [2:0] TX_MOD_TS_USAGE_MODE
        uint32_t TX_MOD_TS_INFO1             :13;     ///<BIT [15:3] TX_MOD_TS_INFO1
        uint32_t TX_MOD_TS_VENDER_ID         :16;     ///<BIT [31:16] TX_MOD_TS_VENDER_ID
    } b;
} Pl32gTxModTsData1_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_MOD_TS_INFO2             :24;     ///<BIT [23:0] TX_MOD_TS_INFO2
        uint32_t TX_ALT_PROTOCOL_NEGO_STATUS :2;      ///<BIT [25:24] TX_ALT_PROTOCOL_NEGO_STATUS
        uint32_t RSVDP_26                    :6;      ///<BIT [31:26] RSVDP_26
    } b;
} Pl32gTxModTsData2_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DSP_32G_TX_PRESET0          :4;      ///<BIT [3:0] DSP_32G_TX_PRESET0
        uint32_t USP_32G_TX_PRESET0          :4;      ///<BIT [7:4] USP_32G_TX_PRESET0
        uint32_t DSP_32G_TX_PRESET1          :4;      ///<BIT [11:8] DSP_32G_TX_PRESET1
        uint32_t USP_32G_TX_PRESET1          :4;      ///<BIT [15:12] USP_32G_TX_PRESET1
        uint32_t DSP_32G_TX_PRESET2          :4;      ///<BIT [19:16] DSP_32G_TX_PRESET2
        uint32_t USP_32G_TX_PRESET2          :4;      ///<BIT [23:20] USP_32G_TX_PRESET2
        uint32_t DSP_32G_TX_PRESET3          :4;      ///<BIT [27:24] DSP_32G_TX_PRESET3
        uint32_t USP_32G_TX_PRESET3          :4;      ///<BIT [31:28] USP_32G_TX_PRESET3
    } b;
} Pl32gCapOff20h_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SRIOV_PCIE_EXTENDED_CAP_ID  :16;     ///<BIT [15:0] SRIOV_PCIE_EXTENDED_CAP_ID
        uint32_t SRIOV_CAP_VERSION           :4;      ///<BIT [19:16] SRIOV_CAP_VERSION
        uint32_t SRIOV_NEXT_OFFSET           :12;     ///<BIT [31:20] SRIOV_NEXT_OFFSET
    } b;
} SriovBase_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SRIOV_VF_MIGRATION_CAP      :1;      ///<BIT [0] SRIOV_VF_MIGRATION_CAP
        uint32_t SRIOV_ARI_CAP_HIER_PRESERVED :1;      ///<BIT [1] SRIOV_ARI_CAP_HIER_PRESERVED
        uint32_t SRIOV_VF_10BIT_TAG_REQ_CAP  :1;      ///<BIT [2] SRIOV_VF_10BIT_TAG_REQ_CAP
        uint32_t RSVDP_3                     :18;     ///<BIT [20:3] RSVDP_3
        uint32_t SRIOV_VF_MIGRATION_INT_MSG_NUM :10;     ///<BIT [30:21] SRIOV_VF_MIGRATION_INT_MSG_NUM
        uint32_t RSVDP_31                    :1;      ///<BIT [31] RSVDP_31
    } b;
} Capabilities_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SRIOV_VF_ENABLE             :1;      ///<BIT [0] SRIOV_VF_ENABLE
        uint32_t SRIOV_VF_MIGRATION_ENABLE   :1;      ///<BIT [1] SRIOV_VF_MIGRATION_ENABLE
        uint32_t SRIOV_VF_MIGRATION_INT_ENABLE :1;      ///<BIT [2] SRIOV_VF_MIGRATION_INT_ENABLE
        uint32_t SRIOV_VF_MSE                :1;      ///<BIT [3] SRIOV_VF_MSE
        uint32_t SRIOV_ARI_CAPABLE_HIER      :1;      ///<BIT [4] SRIOV_ARI_CAPABLE_HIER
        uint32_t SRIOV_VF_10BIT_TAG_REQ_ENABLE :1;      ///<BIT [5] SRIOV_VF_10BIT_TAG_REQ_ENABLE
        uint32_t RSVDP_6                     :26;     ///<BIT [31:6] RSVDP_6
    } b;
} StatusControl_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SRIOV_INITIAL_VFS           :16;     ///<BIT [15:0] SRIOV_INITIAL_VFS
        uint32_t SRIOV_TOTAL_VFS             :16;     ///<BIT [31:16] SRIOV_TOTAL_VFS
    } b;
} TotalVfsInitialVfs_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SRIOV_NUM_VFS               :16;     ///<BIT [15:0] SRIOV_NUM_VFS
        uint32_t SRIOV_FDL                   :8;      ///<BIT [23:16] SRIOV_FDL
        uint32_t RSVDP_24                    :8;      ///<BIT [31:24] RSVDP_24
    } b;
} SriovNumVfs_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SRIOV_VF_OFFSET             :16;     ///<BIT [15:0] SRIOV_VF_OFFSET
        uint32_t SRIOV_VF_STRIDE             :16;     ///<BIT [31:16] SRIOV_VF_STRIDE
    } b;
} SriovVfOffsetPosition_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :16;     ///<BIT [15:0] RSVDP_0
        uint32_t SRIOV_VF_DEVICE_ID          :16;     ///<BIT [31:16] SRIOV_VF_DEVICE_ID
    } b;
} VfDeviceId_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :1;      ///<BIT [0] RSVDP_0
        uint32_t SRIOV_VF_BAR0_TYPE          :2;      ///<BIT [2:1] SRIOV_VF_BAR0_TYPE
        uint32_t SRIOV_VF_BAR0_PREFETCH      :1;      ///<BIT [3] SRIOV_VF_BAR0_PREFETCH
        uint32_t SRIOV_VF_BAR0_START         :28;     ///<BIT [31:4] SRIOV_VF_BAR0_START
    } b;
} SriovBar0_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_SRIOV_VF_BAR1         :1;      ///<BIT [0] RSVDP_SRIOV_VF_BAR1
        uint32_t SRIOV_VF_BAR1_TYPE          :2;      ///<BIT [2:1] SRIOV_VF_BAR1_TYPE
        uint32_t SRIOV_VF_BAR1_PREFETCH      :1;      ///<BIT [3] SRIOV_VF_BAR1_PREFETCH
        uint32_t SRIOV_VF_BAR1_START         :28;     ///<BIT [31:4] SRIOV_VF_BAR1_START
    } b;
} SriovBar1_t;

/// @brief 0x2C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :1;      ///<BIT [0] RSVDP_0
        uint32_t SRIOV_VF_BAR2_TYPE          :2;      ///<BIT [2:1] SRIOV_VF_BAR2_TYPE
        uint32_t SRIOV_VF_BAR2_PREFETCH      :1;      ///<BIT [3] SRIOV_VF_BAR2_PREFETCH
        uint32_t SRIOV_VF_BAR2_START         :28;     ///<BIT [31:4] SRIOV_VF_BAR2_START
    } b;
} SriovBar2_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_SRIOV_VF_BAR3         :1;      ///<BIT [0] RSVDP_SRIOV_VF_BAR3
        uint32_t SRIOV_VF_BAR3_TYPE          :2;      ///<BIT [2:1] SRIOV_VF_BAR3_TYPE
        uint32_t SRIOV_VF_BAR3_PREFETCH      :1;      ///<BIT [3] SRIOV_VF_BAR3_PREFETCH
        uint32_t SRIOV_VF_BAR3_START         :28;     ///<BIT [31:4] SRIOV_VF_BAR3_START
    } b;
} SriovBar3_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :1;      ///<BIT [0] RSVDP_0
        uint32_t SRIOV_VF_BAR4_TYPE          :2;      ///<BIT [2:1] SRIOV_VF_BAR4_TYPE
        uint32_t SRIOV_VF_BAR4_PREFETCH      :1;      ///<BIT [3] SRIOV_VF_BAR4_PREFETCH
        uint32_t SRIOV_VF_BAR4_START         :28;     ///<BIT [31:4] SRIOV_VF_BAR4_START
    } b;
} SriovBar4_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_SRIOV_VF_BAR5         :1;      ///<BIT [0] RSVDP_SRIOV_VF_BAR5
        uint32_t SRIOV_VF_BAR5_TYPE          :2;      ///<BIT [2:1] SRIOV_VF_BAR5_TYPE
        uint32_t SRIOV_VF_BAR5_PREFETCH      :1;      ///<BIT [3] SRIOV_VF_BAR5_PREFETCH
        uint32_t SRIOV_VF_BAR5_START         :28;     ///<BIT [31:4] SRIOV_VF_BAR5_START
    } b;
} SriovBar5_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SRIOV_VF_MIGRATION_STATE_BIR :3;      ///<BIT [2:0] SRIOV_VF_MIGRATION_STATE_BIR
        uint32_t SRIOV_VF_MIGRATION_STATE_OFFSET :29;     ///<BIT [31:3] SRIOV_VF_MIGRATION_STATE_OFFSET
    } b;
} VfMigrationStateArray_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CAP_ID                      :16;     ///<BIT [15:0] CAP_ID
        uint32_t CAP_VERSION                 :4;      ///<BIT [19:16] CAP_VERSION
        uint32_t NEXT_OFFSET                 :12;     ///<BIT [31:20] NEXT_OFFSET
    } b;
} LtrCapHdr_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MAX_SNOOP_LAT               :10;     ///<BIT [9:0] MAX_SNOOP_LAT
        uint32_t MAX_SNOOP_LAT_SCALE         :3;      ///<BIT [12:10] MAX_SNOOP_LAT_SCALE
        uint32_t RSVDP_13                    :3;      ///<BIT [15:13] RSVDP_13
        uint32_t MAX_NO_SNOOP_LAT            :10;     ///<BIT [25:16] MAX_NO_SNOOP_LAT
        uint32_t MAX_NO_SNOOP_LAT_SCALE      :3;      ///<BIT [28:26] MAX_NO_SNOOP_LAT_SCALE
        uint32_t RSVDP_29                    :3;      ///<BIT [31:29] RSVDP_29
    } b;
} LtrLatency_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EXTENDED_CAP_ID             :16;     ///<BIT [15:0] EXTENDED_CAP_ID
        uint32_t CAP_VERSION                 :4;      ///<BIT [19:16] CAP_VERSION
        uint32_t NEXT_OFFSET                 :12;     ///<BIT [31:20] NEXT_OFFSET
    } b;
} L1subCapHeader_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t L1_2_PCIPM_SUPPORT          :1;      ///<BIT [0] L1_2_PCIPM_SUPPORT
        uint32_t L1_1_PCIPM_SUPPORT          :1;      ///<BIT [1] L1_1_PCIPM_SUPPORT
        uint32_t L1_2_ASPM_SUPPORT           :1;      ///<BIT [2] L1_2_ASPM_SUPPORT
        uint32_t L1_1_ASPM_SUPPORT           :1;      ///<BIT [3] L1_1_ASPM_SUPPORT
        uint32_t L1_PMSUB_SUPPORT            :1;      ///<BIT [4] L1_PMSUB_SUPPORT
        uint32_t RSVDP_5                     :3;      ///<BIT [7:5] RSVDP_5
        uint32_t COMM_MODE_SUPPORT           :8;      ///<BIT [15:8] COMM_MODE_SUPPORT
        uint32_t PWR_ON_SCALE_SUPPORT        :2;      ///<BIT [17:16] PWR_ON_SCALE_SUPPORT
        uint32_t RSVDP_18                    :1;      ///<BIT [18] RSVDP_18
        uint32_t PWR_ON_VALUE_SUPPORT        :5;      ///<BIT [23:19] PWR_ON_VALUE_SUPPORT
        uint32_t RSVDP_24                    :8;      ///<BIT [31:24] RSVDP_24
    } b;
} L1subCapability_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t L1_2_PCIPM_EN               :1;      ///<BIT [0] L1_2_PCIPM_EN
        uint32_t L1_1_PCIPM_EN               :1;      ///<BIT [1] L1_1_PCIPM_EN
        uint32_t L1_2_ASPM_EN                :1;      ///<BIT [2] L1_2_ASPM_EN
        uint32_t L1_1_ASPM_EN                :1;      ///<BIT [3] L1_1_ASPM_EN
        uint32_t RSVDP_4                     :4;      ///<BIT [7:4] RSVDP_4
        uint32_t T_COMMON_MODE               :8;      ///<BIT [15:8] T_COMMON_MODE
        uint32_t L1_2_TH_VAL                 :10;     ///<BIT [25:16] L1_2_TH_VAL
        uint32_t RSVDP_26                    :3;      ///<BIT [28:26] RSVDP_26
        uint32_t L1_2_TH_SCA                 :3;      ///<BIT [31:29] L1_2_TH_SCA
    } b;
} L1subControl1_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t T_POWER_ON_SCALE            :2;      ///<BIT [1:0] T_POWER_ON_SCALE
        uint32_t RSVDP_2                     :1;      ///<BIT [2] RSVDP_2
        uint32_t T_POWER_ON_VALUE            :5;      ///<BIT [7:3] T_POWER_ON_VALUE
        uint32_t RSVDP_8                     :24;     ///<BIT [31:8] RSVDP_8
    } b;
} L1subControl2_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EXT_CAP_ID                  :16;     ///<BIT [15:0] EXT_CAP_ID
        uint32_t CAP_VERSION                 :4;      ///<BIT [19:16] CAP_VERSION
        uint32_t NEXT_OFFSET                 :12;     ///<BIT [31:20] NEXT_OFFSET
    } b;
} DpaExtCapHdr_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SUBSTATE_MAX                :5;      ///<BIT [4:0] SUBSTATE_MAX
        uint32_t RSVDP_5                     :3;      ///<BIT [7:5] RSVDP_5
        uint32_t TLUNIT                      :2;      ///<BIT [9:8] TLUNIT
        uint32_t RSVDP_10                    :2;      ///<BIT [11:10] RSVDP_10
        uint32_t PAS                         :2;      ///<BIT [13:12] PAS
        uint32_t RSVDP_14                    :2;      ///<BIT [15:14] RSVDP_14
        uint32_t XLCY0                       :8;      ///<BIT [23:16] XLCY0
        uint32_t XLCY1                       :8;      ///<BIT [31:24] XLCY1
    } b;
} DpaCap_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SUBSTATE_STATUS             :5;      ///<BIT [4:0] SUBSTATE_STATUS
        uint32_t RSVDP_5                     :3;      ///<BIT [7:5] RSVDP_5
        uint32_t SUBSTATE_CONTROL_EN         :1;      ///<BIT [8] SUBSTATE_CONTROL_EN
        uint32_t RSVDP_9                     :7;      ///<BIT [15:9] RSVDP_9
        uint32_t SUBSTATE_CONTROL            :5;      ///<BIT [20:16] SUBSTATE_CONTROL
        uint32_t RSVDP_21                    :11;     ///<BIT [31:21] RSVDP_21
    } b;
} DpaStatusCntrl_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PWR_ALLOC_VAL0              :8;      ///<BIT [7:0] PWR_ALLOC_VAL0
        uint32_t PWR_ALLOC_VAL1              :8;      ///<BIT [15:8] PWR_ALLOC_VAL1
        uint32_t PWR_ALLOC_VAL2              :8;      ///<BIT [23:16] PWR_ALLOC_VAL2
        uint32_t PWR_ALLOC_VAL3              :8;      ///<BIT [31:24] PWR_ALLOC_VAL3
    } b;
} DpaPwrAllocArray0_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PWR_ALLOC_VAL4              :8;      ///<BIT [7:0] PWR_ALLOC_VAL4
        uint32_t PWR_ALLOC_VAL5              :8;      ///<BIT [15:8] PWR_ALLOC_VAL5
        uint32_t PWR_ALLOC_VAL6              :8;      ///<BIT [23:16] PWR_ALLOC_VAL6
        uint32_t PWR_ALLOC_VAL7              :8;      ///<BIT [31:24] PWR_ALLOC_VAL7
    } b;
} DpaPwrAllocArray4_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PWR_ALLOC_VAL8              :8;      ///<BIT [7:0] PWR_ALLOC_VAL8
        uint32_t PWR_ALLOC_VAL9              :8;      ///<BIT [15:8] PWR_ALLOC_VAL9
        uint32_t PWR_ALLOC_VAL10             :8;      ///<BIT [23:16] PWR_ALLOC_VAL10
        uint32_t PWR_ALLOC_VAL11             :8;      ///<BIT [31:24] PWR_ALLOC_VAL11
    } b;
} DpaPwrAllocArray8_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PWR_ALLOC_VAL12             :8;      ///<BIT [7:0] PWR_ALLOC_VAL12
        uint32_t PWR_ALLOC_VAL13             :8;      ///<BIT [15:8] PWR_ALLOC_VAL13
        uint32_t PWR_ALLOC_VAL14             :8;      ///<BIT [23:16] PWR_ALLOC_VAL14
        uint32_t PWR_ALLOC_VAL15             :8;      ///<BIT [31:24] PWR_ALLOC_VAL15
    } b;
} DpaPwrAllocArray12_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PWR_ALLOC_VAL16             :8;      ///<BIT [7:0] PWR_ALLOC_VAL16
        uint32_t PWR_ALLOC_VAL17             :8;      ///<BIT [15:8] PWR_ALLOC_VAL17
        uint32_t PWR_ALLOC_VAL18             :8;      ///<BIT [23:16] PWR_ALLOC_VAL18
        uint32_t PWR_ALLOC_VAL19             :8;      ///<BIT [31:24] PWR_ALLOC_VAL19
    } b;
} DpaPwrAllocArray16_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PWR_ALLOC_VAL20             :8;      ///<BIT [7:0] PWR_ALLOC_VAL20
        uint32_t PWR_ALLOC_VAL21             :8;      ///<BIT [15:8] PWR_ALLOC_VAL21
        uint32_t PWR_ALLOC_VAL22             :8;      ///<BIT [23:16] PWR_ALLOC_VAL22
        uint32_t PWR_ALLOC_VAL23             :8;      ///<BIT [31:24] PWR_ALLOC_VAL23
    } b;
} DpaPwrAllocArray20_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PWR_ALLOC_VAL24             :8;      ///<BIT [7:0] PWR_ALLOC_VAL24
        uint32_t PWR_ALLOC_VAL25             :8;      ///<BIT [15:8] PWR_ALLOC_VAL25
        uint32_t PWR_ALLOC_VAL26             :8;      ///<BIT [23:16] PWR_ALLOC_VAL26
        uint32_t PWR_ALLOC_VAL27             :8;      ///<BIT [31:24] PWR_ALLOC_VAL27
    } b;
} DpaPwrAllocArray24_t;

/// @brief 0x2C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PWR_ALLOC_VAL28             :8;      ///<BIT [7:0] PWR_ALLOC_VAL28
        uint32_t PWR_ALLOC_VAL29             :8;      ///<BIT [15:8] PWR_ALLOC_VAL29
        uint32_t PWR_ALLOC_VAL30             :8;      ///<BIT [23:16] PWR_ALLOC_VAL30
        uint32_t PWR_ALLOC_VAL31             :8;      ///<BIT [31:24] PWR_ALLOC_VAL31
    } b;
} DpaPwrAllocArray28_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EXTENDED_CAP_ID             :16;     ///<BIT [15:0] EXTENDED_CAP_ID
        uint32_t CAP_VERSION                 :4;      ///<BIT [19:16] CAP_VERSION
        uint32_t NEXT_OFFSET                 :12;     ///<BIT [31:20] NEXT_OFFSET
    } b;
} RasDesCapHeader_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VSEC_ID                     :16;     ///<BIT [15:0] VSEC_ID
        uint32_t VSEC_REV                    :4;      ///<BIT [19:16] VSEC_REV
        uint32_t VSEC_LENGTH                 :12;     ///<BIT [31:20] VSEC_LENGTH
    } b;
} VendorSpecificHeader_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EVENT_COUNTER_CLEAR         :2;      ///<BIT [1:0] EVENT_COUNTER_CLEAR
        uint32_t EVENT_COUNTER_ENABLE        :3;      ///<BIT [4:2] EVENT_COUNTER_ENABLE
        uint32_t RSVDP_5                     :2;      ///<BIT [6:5] RSVDP_5
        uint32_t EVENT_COUNTER_STATUS        :1;      ///<BIT [7] EVENT_COUNTER_STATUS
        uint32_t EVENT_COUNTER_LANE_SELECT   :4;      ///<BIT [11:8] EVENT_COUNTER_LANE_SELECT
        uint32_t RSVDP_12                    :4;      ///<BIT [15:12] RSVDP_12
        uint32_t EVENT_COUNTER_EVENT_SELECT  :12;     ///<BIT [27:16] EVENT_COUNTER_EVENT_SELECT
        uint32_t RSVDP_28                    :4;      ///<BIT [31:28] RSVDP_28
    } b;
} EventCounterControl_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TIMER_START                 :1;      ///<BIT [0] TIMER_START
        uint32_t RSVDP_1                     :7;      ///<BIT [7:1] RSVDP_1
        uint32_t TIME_BASED_DURATION_SELECT  :8;      ///<BIT [15:8] TIME_BASED_DURATION_SELECT
        uint32_t RSVDP_16                    :8;      ///<BIT [23:16] RSVDP_16
        uint32_t TIME_BASED_REPORT_SELECT    :8;      ///<BIT [31:24] TIME_BASED_REPORT_SELECT
    } b;
} TimeBasedAnalysisControl_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_INJECTION0_ENABLE     :1;      ///<BIT [0] ERROR_INJECTION0_ENABLE
        uint32_t ERROR_INJECTION1_ENABLE     :1;      ///<BIT [1] ERROR_INJECTION1_ENABLE
        uint32_t ERROR_INJECTION2_ENABLE     :1;      ///<BIT [2] ERROR_INJECTION2_ENABLE
        uint32_t ERROR_INJECTION3_ENABLE     :1;      ///<BIT [3] ERROR_INJECTION3_ENABLE
        uint32_t ERROR_INJECTION4_ENABLE     :1;      ///<BIT [4] ERROR_INJECTION4_ENABLE
        uint32_t ERROR_INJECTION5_ENABLE     :1;      ///<BIT [5] ERROR_INJECTION5_ENABLE
        uint32_t ERROR_INJECTION6_ENABLE     :1;      ///<BIT [6] ERROR_INJECTION6_ENABLE
        uint32_t RSVDP_7                     :25;     ///<BIT [31:7] RSVDP_7
    } b;
} EinjEnable_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EINJ0_COUNT                 :8;      ///<BIT [7:0] EINJ0_COUNT
        uint32_t EINJ0_CRC_TYPE              :4;      ///<BIT [11:8] EINJ0_CRC_TYPE
        uint32_t RSVDP_12                    :4;      ///<BIT [15:12] RSVDP_12
        uint32_t RSVD_16_18                  :3;      ///<BIT [18:16] rsvd_16_18
        uint32_t RSVDP_19                    :1;      ///<BIT [19] RSVDP_19
        uint32_t RSVD_20_21                  :2;      ///<BIT [21:20] rsvd_20_21
        uint32_t RSVDP_22                    :2;      ///<BIT [23:22] RSVDP_22
        uint32_t RSVD_24                     :1;      ///<BIT [24] rsvd_24
        uint32_t RSVDP_25                    :7;      ///<BIT [31:25] RSVDP_25
    } b;
} Einj0Crc_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EINJ1_COUNT                 :8;      ///<BIT [7:0] EINJ1_COUNT
        uint32_t EINJ1_SEQNUM_TYPE           :1;      ///<BIT [8] EINJ1_SEQNUM_TYPE
        uint32_t RSVDP_9                     :7;      ///<BIT [15:9] RSVDP_9
        uint32_t EINJ1_BAD_SEQNUM            :13;     ///<BIT [28:16] EINJ1_BAD_SEQNUM
        uint32_t RSVDP_29                    :3;      ///<BIT [31:29] RSVDP_29
    } b;
} Einj1Seqnum_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EINJ2_COUNT                 :8;      ///<BIT [7:0] EINJ2_COUNT
        uint32_t EINJ2_DLLP_TYPE             :2;      ///<BIT [9:8] EINJ2_DLLP_TYPE
        uint32_t RSVDP_10                    :22;     ///<BIT [31:10] RSVDP_10
    } b;
} Einj2Dllp_t;

/// @brief 0x40
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EINJ3_COUNT                 :8;      ///<BIT [7:0] EINJ3_COUNT
        uint32_t EINJ3_SYMBOL_TYPE           :3;      ///<BIT [10:8] EINJ3_SYMBOL_TYPE
        uint32_t RSVDP_11                    :21;     ///<BIT [31:11] RSVDP_11
    } b;
} Einj3Symbol_t;

/// @brief 0x44
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EINJ4_COUNT                 :8;      ///<BIT [7:0] EINJ4_COUNT
        uint32_t EINJ4_UPDFC_TYPE            :3;      ///<BIT [10:8] EINJ4_UPDFC_TYPE
        uint32_t RSVDP_11                    :1;      ///<BIT [11] RSVDP_11
        uint32_t EINJ4_VC_NUMBER             :3;      ///<BIT [14:12] EINJ4_VC_NUMBER
        uint32_t RSVDP_15                    :1;      ///<BIT [15] RSVDP_15
        uint32_t EINJ4_BAD_UPDFC_VALUE       :13;     ///<BIT [28:16] EINJ4_BAD_UPDFC_VALUE
        uint32_t RSVDP_29                    :3;      ///<BIT [31:29] RSVDP_29
    } b;
} Einj4Fc_t;

/// @brief 0x48
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EINJ5_COUNT                 :8;      ///<BIT [7:0] EINJ5_COUNT
        uint32_t EINJ5_SPECIFIED_TLP         :1;      ///<BIT [8] EINJ5_SPECIFIED_TLP
        uint32_t RSVDP_9                     :23;     ///<BIT [31:9] RSVDP_9
    } b;
} Einj5SpTlp_t;

/// @brief 0x8C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EINJ6_COUNT                 :8;      ///<BIT [7:0] EINJ6_COUNT
        uint32_t EINJ6_INVERTED_CONTROL      :1;      ///<BIT [8] EINJ6_INVERTED_CONTROL
        uint32_t EINJ6_PACKET_TYPE           :3;      ///<BIT [11:9] EINJ6_PACKET_TYPE
        uint32_t RSVDP_12                    :20;     ///<BIT [31:12] RSVDP_12
    } b;
} Einj6Tlp_t;

/// @brief 0xA0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FORCE_DETECT_LANE           :16;     ///<BIT [15:0] FORCE_DETECT_LANE
        uint32_t FORCE_DETECT_LANE_EN        :1;      ///<BIT [16] FORCE_DETECT_LANE_EN
        uint32_t RSVDP_17                    :3;      ///<BIT [19:17] RSVDP_17
        uint32_t TX_EIOS_NUM                 :2;      ///<BIT [21:20] TX_EIOS_NUM
        uint32_t LOW_POWER_INTERVAL          :2;      ///<BIT [23:22] LOW_POWER_INTERVAL
        uint32_t RSVDP_24                    :8;      ///<BIT [31:24] RSVDP_24
    } b;
} SdControl1_t;

/// @brief 0xA4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HOLD_LTSSM                  :1;      ///<BIT [0] HOLD_LTSSM
        uint32_t RECOVERY_REQUEST            :1;      ///<BIT [1] RECOVERY_REQUEST
        uint32_t NOACK_FORCE_LINKDOWN        :1;      ///<BIT [2] NOACK_FORCE_LINKDOWN
        uint32_t RSVDP_3                     :5;      ///<BIT [7:3] RSVDP_3
        uint32_t DIRECT_RECIDLE_TO_CONFIG    :1;      ///<BIT [8] DIRECT_RECIDLE_TO_CONFIG
        uint32_t DIRECT_POLCOMP_TO_DETECT    :1;      ///<BIT [9] DIRECT_POLCOMP_TO_DETECT
        uint32_t DIRECT_LPBKSLV_TO_EXIT      :1;      ///<BIT [10] DIRECT_LPBKSLV_TO_EXIT
        uint32_t RSVDP_11                    :5;      ///<BIT [15:11] RSVDP_11
        uint32_t FRAMING_ERR_RECOVERY_DISABLE :1;      ///<BIT [16] FRAMING_ERR_RECOVERY_DISABLE
        uint32_t RSVDP_17                    :15;     ///<BIT [31:17] RSVDP_17
    } b;
} SdControl2_t;

/// @brief 0xB0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LANE_SELECT                 :4;      ///<BIT [3:0] LANE_SELECT
        uint32_t RSVDP_4                     :12;     ///<BIT [15:4] RSVDP_4
        uint32_t PIPE_RXPOLARITY             :1;      ///<BIT [16] PIPE_RXPOLARITY
        uint32_t PIPE_DETECT_LANE            :1;      ///<BIT [17] PIPE_DETECT_LANE
        uint32_t PIPE_RXVALID                :1;      ///<BIT [18] PIPE_RXVALID
        uint32_t PIPE_RXELECIDLE             :1;      ///<BIT [19] PIPE_RXELECIDLE
        uint32_t PIPE_TXELECIDLE             :1;      ///<BIT [20] PIPE_TXELECIDLE
        uint32_t RSVDP_21                    :3;      ///<BIT [23:21] RSVDP_21
        uint32_t DESKEW_POINTER              :8;      ///<BIT [31:24] DESKEW_POINTER
    } b;
} SdStatusL1lane_t;

/// @brief 0xB4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FRAMING_ERR_PTR             :7;      ///<BIT [6:0] FRAMING_ERR_PTR
        uint32_t FRAMING_ERR                 :1;      ///<BIT [7] FRAMING_ERR
        uint32_t PIPE_POWER_DOWN             :3;      ///<BIT [10:8] PIPE_POWER_DOWN
        uint32_t RSVDP_11                    :4;      ///<BIT [14:11] RSVDP_11
        uint32_t LANE_REVERSAL               :1;      ///<BIT [15] LANE_REVERSAL
        uint32_t LTSSM_VARIABLE              :16;     ///<BIT [31:16] LTSSM_VARIABLE
    } b;
} SdStatusL1ltssm_t;

/// @brief 0xB8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INTERNAL_PM_MSTATE          :5;      ///<BIT [4:0] INTERNAL_PM_MSTATE
        uint32_t RSVDP_5                     :3;      ///<BIT [7:5] RSVDP_5
        uint32_t INTERNAL_PM_SSTATE          :4;      ///<BIT [11:8] INTERNAL_PM_SSTATE
        uint32_t PME_RESEND_FLAG             :1;      ///<BIT [12] PME_RESEND_FLAG
        uint32_t L1SUB_STATE                 :3;      ///<BIT [15:13] L1SUB_STATE
        uint32_t LATCHED_NFTS                :8;      ///<BIT [23:16] LATCHED_NFTS
        uint32_t RSVDP_24                    :8;      ///<BIT [31:24] RSVDP_24
    } b;
} SdStatusPm_t;

/// @brief 0xBC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_TLP_SEQ_NO               :12;     ///<BIT [11:0] TX_TLP_SEQ_NO
        uint32_t RX_ACK_SEQ_NO               :12;     ///<BIT [23:12] RX_ACK_SEQ_NO
        uint32_t DLCMSM                      :2;      ///<BIT [25:24] DLCMSM
        uint32_t FC_INIT1                    :1;      ///<BIT [26] FC_INIT1
        uint32_t FC_INIT2                    :1;      ///<BIT [27] FC_INIT2
        uint32_t RSVDP_28                    :4;      ///<BIT [31:28] RSVDP_28
    } b;
} SdStatusL2_t;

/// @brief 0xC0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CREDIT_SEL_VC               :3;      ///<BIT [2:0] CREDIT_SEL_VC
        uint32_t CREDIT_SEL_CREDIT_TYPE      :1;      ///<BIT [3] CREDIT_SEL_CREDIT_TYPE
        uint32_t CREDIT_SEL_TLP_TYPE         :2;      ///<BIT [5:4] CREDIT_SEL_TLP_TYPE
        uint32_t CREDIT_SEL_HD               :1;      ///<BIT [6] CREDIT_SEL_HD
        uint32_t RSVDP_7                     :1;      ///<BIT [7] RSVDP_7
        uint32_t CREDIT_DATA0                :12;     ///<BIT [19:8] CREDIT_DATA0
        uint32_t CREDIT_DATA1                :12;     ///<BIT [31:20] CREDIT_DATA1
    } b;
} SdStatusL3fc_t;

/// @brief 0xC4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MFTLP_POINTER               :7;      ///<BIT [6:0] MFTLP_POINTER
        uint32_t MFTLP_STATUS                :1;      ///<BIT [7] MFTLP_STATUS
        uint32_t RSVDP_8                     :24;     ///<BIT [31:8] RSVDP_8
    } b;
} SdStatusL3_t;

/// @brief 0xD0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EQ_LANE_SEL                 :4;      ///<BIT [3:0] EQ_LANE_SEL
        uint32_t EQ_RATE_SEL                 :2;      ///<BIT [5:4] EQ_RATE_SEL
        uint32_t RSVDP_6                     :2;      ///<BIT [7:6] RSVDP_6
        uint32_t EXT_EQ_TIMEOUT              :2;      ///<BIT [9:8] EXT_EQ_TIMEOUT
        uint32_t RSVDP_10                    :6;      ///<BIT [15:10] RSVDP_10
        uint32_t EVAL_INTERVAL_TIME          :2;      ///<BIT [17:16] EVAL_INTERVAL_TIME
        uint32_t RSVDP_18                    :5;      ///<BIT [22:18] RSVDP_18
        uint32_t FOM_TARGET_ENABLE           :1;      ///<BIT [23] FOM_TARGET_ENABLE
        uint32_t FOM_TARGET                  :8;      ///<BIT [31:24] FOM_TARGET
    } b;
} SdEqControl1_t;

/// @brief 0xD4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FORCE_LOCAL_TX_PRE_CURSOR   :6;      ///<BIT [5:0] FORCE_LOCAL_TX_PRE_CURSOR
        uint32_t FORCE_LOCAL_TX_CURSOR       :6;      ///<BIT [11:6] FORCE_LOCAL_TX_CURSOR
        uint32_t FORCE_LOCAL_TX_POST_CURSOR  :6;      ///<BIT [17:12] FORCE_LOCAL_TX_POST_CURSOR
        uint32_t FORCE_LOCAL_RX_HINT_OR_FORCE_LOCAL_TX_2ND_PRE_CURSOR :6;      ///<BIT [23:18] FORCE_LOCAL_RX_HINT_OR_FORCE_LOCAL_TX_2ND_PRE_CURSOR
        uint32_t FORCE_LOCAL_TX_PRESET       :4;      ///<BIT [27:24] FORCE_LOCAL_TX_PRESET
        uint32_t FORCE_LOCAL_TX_COEF_ENABLE  :1;      ///<BIT [28] FORCE_LOCAL_TX_COEF_ENABLE
        uint32_t FORCE_LOCAL_RX_HINT_ENABLE  :1;      ///<BIT [29] FORCE_LOCAL_RX_HINT_ENABLE
        uint32_t FORCE_LOCAL_TX_PRESET_ENABLE :1;      ///<BIT [30] FORCE_LOCAL_TX_PRESET_ENABLE
        uint32_t RSVDP_31                    :1;      ///<BIT [31] RSVDP_31
    } b;
} SdEqControl2_t;

/// @brief 0xD8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FORCE_REMOTE_TX_PRE_CURSOR  :6;      ///<BIT [5:0] FORCE_REMOTE_TX_PRE_CURSOR
        uint32_t FORCE_REMOTE_TX_CURSOR      :6;      ///<BIT [11:6] FORCE_REMOTE_TX_CURSOR
        uint32_t FORCE_REMOTE_TX_POST_CURSOR :6;      ///<BIT [17:12] FORCE_REMOTE_TX_POST_CURSOR
        uint32_t FORCE_REMOTE_TX_2ND_PRE_CURSOR :6;      ///<BIT [23:18] FORCE_REMOTE_TX_2ND_PRE_CURSOR
        uint32_t RSVDP_24                    :4;      ///<BIT [27:24] RSVDP_24
        uint32_t FORCE_REMOTE_TX_COEF_ENABLE :1;      ///<BIT [28] FORCE_REMOTE_TX_COEF_ENABLE
        uint32_t RSVDP_29                    :3;      ///<BIT [31:29] RSVDP_29
    } b;
} SdEqControl3_t;

/// @brief 0xE0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EQ_SEQUENCE                 :1;      ///<BIT [0] EQ_SEQUENCE
        uint32_t EQ_CONVERGENCE_INFO         :2;      ///<BIT [2:1] EQ_CONVERGENCE_INFO
        uint32_t EQ_RULED_VIOLATION          :1;      ///<BIT [3] EQ_RULED_VIOLATION
        uint32_t EQ_RULEA_VIOLATION          :1;      ///<BIT [4] EQ_RULEA_VIOLATION
        uint32_t EQ_RULEB_VIOLATION          :1;      ///<BIT [5] EQ_RULEB_VIOLATION
        uint32_t EQ_RULEC_VIOLATION          :1;      ///<BIT [6] EQ_RULEC_VIOLATION
        uint32_t EQ_REJECT_EVENT             :1;      ///<BIT [7] EQ_REJECT_EVENT
        uint32_t RSVDP_8                     :18;     ///<BIT [25:8] RSVDP_8
        uint32_t EQ_REMOTE_2ND_PRE_CURSOR    :6;      ///<BIT [31:26] EQ_REMOTE_2ND_PRE_CURSOR
    } b;
} SdEqStatus1_t;

/// @brief 0xE4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EQ_LOCAL_PRE_CURSOR         :6;      ///<BIT [5:0] EQ_LOCAL_PRE_CURSOR
        uint32_t EQ_LOCAL_CURSOR             :6;      ///<BIT [11:6] EQ_LOCAL_CURSOR
        uint32_t EQ_LOCAL_POST_CURSOR        :6;      ///<BIT [17:12] EQ_LOCAL_POST_CURSOR
        uint32_t EQ_LOCAL_RX_HINT_OR_EQ_LOCAL_2ND_PRE_CURSOR :6;      ///<BIT [23:18] EQ_LOCAL_RX_HINT_OR_EQ_LOCAL_2ND_PRE_CURSOR
        uint32_t EQ_LOCAL_FOM_VALUE          :8;      ///<BIT [31:24] EQ_LOCAL_FOM_VALUE
    } b;
} SdEqStatus2_t;

/// @brief 0xE8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EQ_REMOTE_PRE_CURSOR        :6;      ///<BIT [5:0] EQ_REMOTE_PRE_CURSOR
        uint32_t EQ_REMOTE_CURSOR            :6;      ///<BIT [11:6] EQ_REMOTE_CURSOR
        uint32_t EQ_REMOTE_POST_CURSOR       :6;      ///<BIT [17:12] EQ_REMOTE_POST_CURSOR
        uint32_t EQ_REMOTE_LF                :6;      ///<BIT [23:18] EQ_REMOTE_LF
        uint32_t EQ_REMOTE_FS                :6;      ///<BIT [29:24] EQ_REMOTE_FS
        uint32_t RSVDP_30                    :2;      ///<BIT [31:30] RSVDP_30
    } b;
} SdEqStatus3_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ID                          :16;     ///<BIT [15:0] ID
        uint32_t CAP                         :4;      ///<BIT [19:16] CAP
        uint32_t NEXT_OFFSET                 :12;     ///<BIT [31:20] NEXT_OFFSET
    } b;
} RasdpExtCapHdrOff_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VSEC_ID                     :16;     ///<BIT [15:0] VSEC_ID
        uint32_t VSEC_REV                    :4;      ///<BIT [19:16] VSEC_REV
        uint32_t VSEC_LENGTH                 :12;     ///<BIT [31:20] VSEC_LENGTH
    } b;
} RasdpVendorSpecificHdrOff_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_PROT_DISABLE_TX       :1;      ///<BIT [0] ERROR_PROT_DISABLE_TX
        uint32_t ERROR_PROT_DISABLE_AXI_BRIDGE_MASTER :1;      ///<BIT [1] ERROR_PROT_DISABLE_AXI_BRIDGE_MASTER
        uint32_t ERROR_PROT_DISABLE_AXI_BRIDGE_OUTBOUND :1;      ///<BIT [2] ERROR_PROT_DISABLE_AXI_BRIDGE_OUTBOUND
        uint32_t ERROR_PROT_DISABLE_DMA_WRITE :1;      ///<BIT [3] ERROR_PROT_DISABLE_DMA_WRITE
        uint32_t ERROR_PROT_DISABLE_LAYER2_TX :1;      ///<BIT [4] ERROR_PROT_DISABLE_LAYER2_TX
        uint32_t ERROR_PROT_DISABLE_LAYER3_TX :1;      ///<BIT [5] ERROR_PROT_DISABLE_LAYER3_TX
        uint32_t ERROR_PROT_DISABLE_ADM_TX   :1;      ///<BIT [6] ERROR_PROT_DISABLE_ADM_TX
        uint32_t ERROR_PROT_DISABLE_CXS_TX   :1;      ///<BIT [7] ERROR_PROT_DISABLE_CXS_TX
        uint32_t ERROR_PROT_DISABLE_DTIM_TX  :1;      ///<BIT [8] ERROR_PROT_DISABLE_DTIM_TX
        uint32_t ERROR_PROT_DISABLE_CXL_TX   :1;      ///<BIT [9] ERROR_PROT_DISABLE_CXL_TX
        uint32_t RSVDP_10                    :6;      ///<BIT [15:10] RSVDP_10
        uint32_t ERROR_PROT_DISABLE_RX       :1;      ///<BIT [16] ERROR_PROT_DISABLE_RX
        uint32_t ERROR_PROT_DISABLE_AXI_BRIDGE_INBOUND_COMPLETION :1;      ///<BIT [17] ERROR_PROT_DISABLE_AXI_BRIDGE_INBOUND_COMPLETION
        uint32_t ERROR_PROT_DISABLE_AXI_BRIDGE_INBOUND_REQUEST :1;      ///<BIT [18] ERROR_PROT_DISABLE_AXI_BRIDGE_INBOUND_REQUEST
        uint32_t ERROR_PROT_DISABLE_DMA_READ :1;      ///<BIT [19] ERROR_PROT_DISABLE_DMA_READ
        uint32_t ERROR_PROT_DISABLE_LAYER2_RX :1;      ///<BIT [20] ERROR_PROT_DISABLE_LAYER2_RX
        uint32_t ERROR_PROT_DISABLE_LAYER3_RX :1;      ///<BIT [21] ERROR_PROT_DISABLE_LAYER3_RX
        uint32_t ERROR_PROT_DISABLE_ADM_RX   :1;      ///<BIT [22] ERROR_PROT_DISABLE_ADM_RX
        uint32_t ERROR_PROT_DISABLE_CXS_RX   :1;      ///<BIT [23] ERROR_PROT_DISABLE_CXS_RX
        uint32_t ERROR_PROT_DISABLE_LTIM     :1;      ///<BIT [24] ERROR_PROT_DISABLE_LTIM
        uint32_t RSVDP_25                    :7;      ///<BIT [31:25] RSVDP_25
    } b;
} RasdpErrorProtCtrlOff_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CORR_CLEAR_COUNTERS         :1;      ///<BIT [0] CORR_CLEAR_COUNTERS
        uint32_t RSVDP_1                     :3;      ///<BIT [3:1] RSVDP_1
        uint32_t CORR_EN_COUNTERS            :1;      ///<BIT [4] CORR_EN_COUNTERS
        uint32_t RSVDP_5                     :15;     ///<BIT [19:5] RSVDP_5
        uint32_t CORR_COUNTER_SELECTION_REGION :4;      ///<BIT [23:20] CORR_COUNTER_SELECTION_REGION
        uint32_t CORR_COUNTER_SELECTION      :8;      ///<BIT [31:24] CORR_COUNTER_SELECTION
    } b;
} RasdpCorrCounterCtrlOff_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CORR_COUNTER                :8;      ///<BIT [7:0] CORR_COUNTER
        uint32_t RSVDP_8                     :12;     ///<BIT [19:8] RSVDP_8
        uint32_t CORR_COUNTER_SELECTED_REGION :4;      ///<BIT [23:20] CORR_COUNTER_SELECTED_REGION
        uint32_t CORR_COUNTER_SELECTED       :8;      ///<BIT [31:24] CORR_COUNTER_SELECTED
    } b;
} RasdpCorrCountReportOff_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UNCORR_CLEAR_COUNTERS       :1;      ///<BIT [0] UNCORR_CLEAR_COUNTERS
        uint32_t RSVDP_1                     :3;      ///<BIT [3:1] RSVDP_1
        uint32_t UNCORR_EN_COUNTERS          :1;      ///<BIT [4] UNCORR_EN_COUNTERS
        uint32_t RSVDP_5                     :15;     ///<BIT [19:5] RSVDP_5
        uint32_t UNCORR_COUNTER_SELECTION_REGION :4;      ///<BIT [23:20] UNCORR_COUNTER_SELECTION_REGION
        uint32_t UNCORR_COUNTER_SELECTION    :8;      ///<BIT [31:24] UNCORR_COUNTER_SELECTION
    } b;
} RasdpUncorrCounterCtrlOff_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UNCORR_COUNTER              :8;      ///<BIT [7:0] UNCORR_COUNTER
        uint32_t RSVDP_8                     :12;     ///<BIT [19:8] RSVDP_8
        uint32_t UNCORR_COUNTER_SELECTED_REGION :4;      ///<BIT [23:20] UNCORR_COUNTER_SELECTED_REGION
        uint32_t UNCORR_COUNTER_SELECTED     :8;      ///<BIT [31:24] UNCORR_COUNTER_SELECTED
    } b;
} RasdpUncorrCountReportOff_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_INJ_EN                :1;      ///<BIT [0] ERROR_INJ_EN
        uint32_t RSVDP_1                     :3;      ///<BIT [3:1] RSVDP_1
        uint32_t ERROR_INJ_TYPE              :2;      ///<BIT [5:4] ERROR_INJ_TYPE
        uint32_t RSVDP_6                     :2;      ///<BIT [7:6] RSVDP_6
        uint32_t ERROR_INJ_COUNT             :8;      ///<BIT [15:8] ERROR_INJ_COUNT
        uint32_t ERROR_INJ_LOC               :8;      ///<BIT [23:16] ERROR_INJ_LOC
        uint32_t RSVDP_24                    :8;      ///<BIT [31:24] RSVDP_24
    } b;
} RasdpErrorInjCtrlOff_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :4;      ///<BIT [3:0] RSVDP_0
        uint32_t REG_FIRST_CORR_ERROR        :4;      ///<BIT [7:4] REG_FIRST_CORR_ERROR
        uint32_t LOC_FIRST_CORR_ERROR        :8;      ///<BIT [15:8] LOC_FIRST_CORR_ERROR
        uint32_t RSVDP_16                    :4;      ///<BIT [19:16] RSVDP_16
        uint32_t REG_LAST_CORR_ERROR         :4;      ///<BIT [23:20] REG_LAST_CORR_ERROR
        uint32_t LOC_LAST_CORR_ERROR         :8;      ///<BIT [31:24] LOC_LAST_CORR_ERROR
    } b;
} RasdpCorrErrorLocationOff_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :4;      ///<BIT [3:0] RSVDP_0
        uint32_t REG_FIRST_UNCORR_ERROR      :4;      ///<BIT [7:4] REG_FIRST_UNCORR_ERROR
        uint32_t LOC_FIRST_UNCORR_ERROR      :8;      ///<BIT [15:8] LOC_FIRST_UNCORR_ERROR
        uint32_t RSVDP_16                    :4;      ///<BIT [19:16] RSVDP_16
        uint32_t REG_LAST_UNCORR_ERROR       :4;      ///<BIT [23:20] REG_LAST_UNCORR_ERROR
        uint32_t LOC_LAST_UNCORR_ERROR       :8;      ///<BIT [31:24] LOC_LAST_UNCORR_ERROR
    } b;
} RasdpUncorrErrorLocationOff_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_MODE_EN               :1;      ///<BIT [0] ERROR_MODE_EN
        uint32_t AUTO_LINK_DOWN_EN           :1;      ///<BIT [1] AUTO_LINK_DOWN_EN
        uint32_t RSVDP_2                     :30;     ///<BIT [31:2] RSVDP_2
    } b;
} RasdpErrorModeEnOff_t;

/// @brief 0x2C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_MODE_CLEAR            :1;      ///<BIT [0] ERROR_MODE_CLEAR
        uint32_t RSVDP_1                     :31;     ///<BIT [31:1] RSVDP_1
    } b;
} RasdpErrorModeClearOff_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RAM_ADDR_CORR_ERROR         :27;     ///<BIT [26:0] RAM_ADDR_CORR_ERROR
        uint32_t RSVDP_27                    :1;      ///<BIT [27] RSVDP_27
        uint32_t RAM_INDEX_CORR_ERROR        :4;      ///<BIT [31:28] RAM_INDEX_CORR_ERROR
    } b;
} RasdpRamAddrCorrErrorOff_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RAM_ADDR_UNCORR_ERROR       :27;     ///<BIT [26:0] RAM_ADDR_UNCORR_ERROR
        uint32_t RSVDP_27                    :1;      ///<BIT [27] RSVDP_27
        uint32_t RAM_INDEX_UNCORR_ERROR      :4;      ///<BIT [31:28] RAM_INDEX_UNCORR_ERROR
    } b;
} RasdpRamAddrUncorrErrorOff_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DLINK_EXT_CAP_ID            :16;     ///<BIT [15:0] DLINK_EXT_CAP_ID
        uint32_t DLINK_CAP_VERSION           :4;      ///<BIT [19:16] DLINK_CAP_VERSION
        uint32_t DLINK_NEXT_OFFSET           :12;     ///<BIT [31:20] DLINK_NEXT_OFFSET
    } b;
} DataLinkFeatureExtHdrOff_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SCALED_FLOW_CNTL_SUPPORTED  :1;      ///<BIT [0] SCALED_FLOW_CNTL_SUPPORTED
        uint32_t LCL_IMMEDIATE_READINESS     :1;      ///<BIT [1] LCL_IMMEDIATE_READINESS
        uint32_t LCL_EXTENDED_VC_CNT         :3;      ///<BIT [4:2] LCL_EXTENDED_VC_CNT
        uint32_t LCL_L0P_EXIT_LTNCY          :3;      ///<BIT [7:5] LCL_L0P_EXIT_LTNCY
        uint32_t FUTURE_FEATURE_SUPPORTED    :15;     ///<BIT [22:8] FUTURE_FEATURE_SUPPORTED
        uint32_t RSVDP_23                    :8;      ///<BIT [30:23] RSVDP_23
        uint32_t DL_FEATURE_EXCHANGE_EN      :1;      ///<BIT [31] DL_FEATURE_EXCHANGE_EN
    } b;
} DataLinkFeatureCapOff_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t REMOTE_SCALED_FLOW_CNTL_SUPPORTED :1;      ///<BIT [0] REMOTE_SCALED_FLOW_CNTL_SUPPORTED
        uint32_t REMOTE_IMMEDIATE_READINESS  :1;      ///<BIT [1] REMOTE_IMMEDIATE_READINESS
        uint32_t REMOTE_EXTENDED_VC_CNT      :3;      ///<BIT [4:2] REMOTE_EXTENDED_VC_CNT
        uint32_t REMOTE_L0P_EXIT_LTNCY       :3;      ///<BIT [7:5] REMOTE_L0P_EXIT_LTNCY
        uint32_t REMOTE_RESERVED             :15;     ///<BIT [22:8] REMOTE_RESERVED
        uint32_t RSVDP_23                    :8;      ///<BIT [30:23] RSVDP_23
        uint32_t DATA_LINK_FEATURE_STATUS_VALID :1;      ///<BIT [31] DATA_LINK_FEATURE_STATUS_VALID
    } b;
} DataLinkFeatureStatusOff_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ROUND_TRIP_LATENCY_TIME_LIMIT :16;     ///<BIT [15:0] ROUND_TRIP_LATENCY_TIME_LIMIT
        uint32_t REPLAY_TIME_LIMIT           :16;     ///<BIT [31:16] REPLAY_TIME_LIMIT
    } b;
} AckLatencyTimerOff_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LINK_NUM                    :8;      ///<BIT [7:0] LINK_NUM
        uint32_t FORCED_LTSSM                :4;      ///<BIT [11:8] FORCED_LTSSM
        uint32_t RSVDP_12                    :3;      ///<BIT [14:12] RSVDP_12
        uint32_t FORCE_EN                    :1;      ///<BIT [15] FORCE_EN
        uint32_t LINK_STATE                  :6;      ///<BIT [21:16] LINK_STATE
        uint32_t SUPPORT_PART_LANES_RXEI_EXIT :1;      ///<BIT [22] SUPPORT_PART_LANES_RXEI_EXIT
        uint32_t DO_DESKEW_FOR_SRIS          :1;      ///<BIT [23] DO_DESKEW_FOR_SRIS
        uint32_t RSVDP_24                    :8;      ///<BIT [31:24] RSVDP_24
    } b;
} PortForceOff_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ACK_FREQ                    :8;      ///<BIT [7:0] ACK_FREQ
        uint32_t ACK_N_FTS                   :8;      ///<BIT [15:8] ACK_N_FTS
        uint32_t COMMON_CLK_N_FTS            :8;      ///<BIT [23:16] COMMON_CLK_N_FTS
        uint32_t L0S_ENTRANCE_LATENCY        :3;      ///<BIT [26:24] L0S_ENTRANCE_LATENCY
        uint32_t L1_ENTRANCE_LATENCY         :3;      ///<BIT [29:27] L1_ENTRANCE_LATENCY
        uint32_t ENTER_ASPM                  :1;      ///<BIT [30] ENTER_ASPM
        uint32_t ASPM_L1_TIMER_ENABLE        :1;      ///<BIT [31] ASPM_L1_TIMER_ENABLE
    } b;
} AckFAspmCtrlOff_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VENDOR_SPECIFIC_DLLP_REQ    :1;      ///<BIT [0] VENDOR_SPECIFIC_DLLP_REQ
        uint32_t SCRAMBLE_DISABLE            :1;      ///<BIT [1] SCRAMBLE_DISABLE
        uint32_t LOOPBACK_ENABLE             :1;      ///<BIT [2] LOOPBACK_ENABLE
        uint32_t RESET_ASSERT                :1;      ///<BIT [3] RESET_ASSERT
        uint32_t RSVDP_4                     :1;      ///<BIT [4] RSVDP_4
        uint32_t DLL_LINK_EN                 :1;      ///<BIT [5] DLL_LINK_EN
        uint32_t LINK_DISABLE                :1;      ///<BIT [6] LINK_DISABLE
        uint32_t FAST_LINK_MODE              :1;      ///<BIT [7] FAST_LINK_MODE
        uint32_t LINK_RATE                   :4;      ///<BIT [11:8] LINK_RATE
        uint32_t RSVDP_12                    :4;      ///<BIT [15:12] RSVDP_12
        uint32_t LINK_CAPABLE                :6;      ///<BIT [21:16] LINK_CAPABLE
        uint32_t RSVD_22_23                  :2;      ///<BIT [23:22] rsvd_22_23
        uint32_t BEACON_ENABLE               :1;      ///<BIT [24] BEACON_ENABLE
        uint32_t CORRUPT_LCRC_ENABLE         :1;      ///<BIT [25] CORRUPT_LCRC_ENABLE
        uint32_t EXTENDED_SYNCH              :1;      ///<BIT [26] EXTENDED_SYNCH
        uint32_t TRANSMIT_LANE_REVERSALE_ENABLE :1;      ///<BIT [27] TRANSMIT_LANE_REVERSALE_ENABLE
        uint32_t RSVDP_28                    :4;      ///<BIT [31:28] RSVDP_28
    } b;
} PortLinkCtrlOff_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INSERT_LANE_SKEW            :24;     ///<BIT [23:0] INSERT_LANE_SKEW
        uint32_t FLOW_CTRL_DISABLE           :1;      ///<BIT [24] FLOW_CTRL_DISABLE
        uint32_t ACK_NAK_DISABLE             :1;      ///<BIT [25] ACK_NAK_DISABLE
        uint32_t ELASTIC_BUFFER_MODE         :1;      ///<BIT [26] ELASTIC_BUFFER_MODE
        uint32_t IMPLEMENT_NUM_LANES         :4;      ///<BIT [30:27] IMPLEMENT_NUM_LANES
        uint32_t DISABLE_LANE_TO_LANE_DESKEW :1;      ///<BIT [31] DISABLE_LANE_TO_LANE_DESKEW
    } b;
} LaneSkewOff_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MAX_FUNC_NUM                :8;      ///<BIT [7:0] MAX_FUNC_NUM
        uint32_t RSVDP_8                     :6;      ///<BIT [13:8] RSVDP_8
        uint32_t TIMER_MOD_REPLAY_TIMER      :5;      ///<BIT [18:14] TIMER_MOD_REPLAY_TIMER
        uint32_t TIMER_MOD_ACK_NAK           :5;      ///<BIT [23:19] TIMER_MOD_ACK_NAK
        uint32_t UPDATE_FREQ_TIMER           :5;      ///<BIT [28:24] UPDATE_FREQ_TIMER
        uint32_t FAST_LINK_SCALING_FACTOR    :2;      ///<BIT [30:29] FAST_LINK_SCALING_FACTOR
        uint32_t RSVDP_31                    :1;      ///<BIT [31] RSVDP_31
    } b;
} TimerCtrlMaxFuncNumOff_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SKP_INT_VAL                 :11;     ///<BIT [10:0] SKP_INT_VAL
        uint32_t EIDLE_TIMER                 :4;      ///<BIT [14:11] EIDLE_TIMER
        uint32_t DISABLE_FC_WD_TIMER         :1;      ///<BIT [15] DISABLE_FC_WD_TIMER
        uint32_t MASK_RADM_1                 :16;     ///<BIT [31:16] MASK_RADM_1
    } b;
} SymbolTimerFilter1Off_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_P_DATA_FC_CREDIT         :16;     ///<BIT [15:0] TX_P_DATA_FC_CREDIT
        uint32_t TX_P_HEADER_FC_CREDIT       :12;     ///<BIT [27:16] TX_P_HEADER_FC_CREDIT
        uint32_t RSVDP_TX_P_FC_CREDIT_STATUS :4;      ///<BIT [31:28] RSVDP_TX_P_FC_CREDIT_STATUS
    } b;
} TxPFcCreditStatusOff_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_NP_DATA_FC_CREDIT        :16;     ///<BIT [15:0] TX_NP_DATA_FC_CREDIT
        uint32_t TX_NP_HEADER_FC_CREDIT      :12;     ///<BIT [27:16] TX_NP_HEADER_FC_CREDIT
        uint32_t RSVDP_TX_NP_FC_CREDIT_STATUS :4;      ///<BIT [31:28] RSVDP_TX_NP_FC_CREDIT_STATUS
    } b;
} TxNpFcCreditStatusOff_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_CPL_DATA_FC_CREDIT       :16;     ///<BIT [15:0] TX_CPL_DATA_FC_CREDIT
        uint32_t TX_CPL_HEADER_FC_CREDIT     :12;     ///<BIT [27:16] TX_CPL_HEADER_FC_CREDIT
        uint32_t RSVDP_TX_CPL_FC_CREDIT_STATUS :4;      ///<BIT [31:28] RSVDP_TX_CPL_FC_CREDIT_STATUS
    } b;
} TxCplFcCreditStatusOff_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_TLP_FC_CREDIT_NON_RETURN :1;      ///<BIT [0] RX_TLP_FC_CREDIT_NON_RETURN
        uint32_t TX_RETRY_BUFFER_NE          :1;      ///<BIT [1] TX_RETRY_BUFFER_NE
        uint32_t RX_QUEUE_NON_EMPTY          :1;      ///<BIT [2] RX_QUEUE_NON_EMPTY
        uint32_t RX_QUEUE_OVERFLOW           :1;      ///<BIT [3] RX_QUEUE_OVERFLOW
        uint32_t RSVDP_4                     :9;      ///<BIT [12:4] RSVDP_4
        uint32_t RX_SERIALIZATION_Q_NON_EMPTY :1;      ///<BIT [13] RX_SERIALIZATION_Q_NON_EMPTY
        uint32_t RSVD_14_15                  :2;      ///<BIT [15:14] rsvd_14_15
        uint32_t TIMER_MOD_FLOW_CONTROL      :13;     ///<BIT [28:16] TIMER_MOD_FLOW_CONTROL
        uint32_t RSVDP_29                    :2;      ///<BIT [30:29] RSVDP_29
        uint32_t TIMER_MOD_FLOW_CONTROL_EN   :1;      ///<BIT [31] TIMER_MOD_FLOW_CONTROL_EN
    } b;
} QueueStatusOff_t;

/// @brief 0x40
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t WRR_WEIGHT_VC_0             :8;      ///<BIT [7:0] WRR_WEIGHT_VC_0
        uint32_t WRR_WEIGHT_VC_1             :8;      ///<BIT [15:8] WRR_WEIGHT_VC_1
        uint32_t WRR_WEIGHT_VC_2             :8;      ///<BIT [23:16] WRR_WEIGHT_VC_2
        uint32_t WRR_WEIGHT_VC_3             :8;      ///<BIT [31:24] WRR_WEIGHT_VC_3
    } b;
} VcTxArbi1Off_t;

/// @brief 0x44
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t WRR_WEIGHT_VC_4             :8;      ///<BIT [7:0] WRR_WEIGHT_VC_4
        uint32_t WRR_WEIGHT_VC_5             :8;      ///<BIT [15:8] WRR_WEIGHT_VC_5
        uint32_t WRR_WEIGHT_VC_6             :8;      ///<BIT [23:16] WRR_WEIGHT_VC_6
        uint32_t WRR_WEIGHT_VC_7             :8;      ///<BIT [31:24] WRR_WEIGHT_VC_7
    } b;
} VcTxArbi2Off_t;

/// @brief 0x48
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VC0_P_DATA_CREDIT           :12;     ///<BIT [11:0] VC0_P_DATA_CREDIT
        uint32_t VC0_P_HEADER_CREDIT         :8;      ///<BIT [19:12] VC0_P_HEADER_CREDIT
        uint32_t RESERVED4                   :1;      ///<BIT [20] RESERVED4
        uint32_t VC0_P_TLP_Q_MODE            :3;      ///<BIT [23:21] VC0_P_TLP_Q_MODE
        uint32_t VC0_P_HDR_SCALE             :2;      ///<BIT [25:24] VC0_P_HDR_SCALE
        uint32_t VC0_P_DATA_SCALE            :2;      ///<BIT [27:26] VC0_P_DATA_SCALE
        uint32_t RESERVED5                   :2;      ///<BIT [29:28] RESERVED5
        uint32_t TLP_TYPE_ORDERING_VC0       :1;      ///<BIT [30] TLP_TYPE_ORDERING_VC0
        uint32_t VC_ORDERING_RX_Q            :1;      ///<BIT [31] VC_ORDERING_RX_Q
    } b;
} Vc0PRxQCtrlOff_t;

/// @brief 0x4C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VC0_NP_DATA_CREDIT          :12;     ///<BIT [11:0] VC0_NP_DATA_CREDIT
        uint32_t VC0_NP_HEADER_CREDIT        :8;      ///<BIT [19:12] VC0_NP_HEADER_CREDIT
        uint32_t RESERVED6                   :1;      ///<BIT [20] RESERVED6
        uint32_t VC0_NP_TLP_Q_MODE           :3;      ///<BIT [23:21] VC0_NP_TLP_Q_MODE
        uint32_t VC0_NP_HDR_SCALE            :2;      ///<BIT [25:24] VC0_NP_HDR_SCALE
        uint32_t VC0_NP_DATA_SCALE           :2;      ///<BIT [27:26] VC0_NP_DATA_SCALE
        uint32_t RESERVED7                   :4;      ///<BIT [31:28] RESERVED7
    } b;
} Vc0NpRxQCtrlOff_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VC0_CPL_DATA_CREDIT         :12;     ///<BIT [11:0] VC0_CPL_DATA_CREDIT
        uint32_t VC0_CPL_HEADER_CREDIT       :8;      ///<BIT [19:12] VC0_CPL_HEADER_CREDIT
        uint32_t RESERVED8                   :1;      ///<BIT [20] RESERVED8
        uint32_t VC0_CPL_TLP_Q_MODE          :3;      ///<BIT [23:21] VC0_CPL_TLP_Q_MODE
        uint32_t VC0_CPL_HDR_SCALE           :2;      ///<BIT [25:24] VC0_CPL_HDR_SCALE
        uint32_t VC0_CPL_DATA_SCALE          :2;      ///<BIT [27:26] VC0_CPL_DATA_SCALE
        uint32_t RESERVED9                   :4;      ///<BIT [31:28] RESERVED9
    } b;
} Vc0CplRxQCtrlOff_t;

/// @brief 0x10C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FAST_TRAINING_SEQ           :8;      ///<BIT [7:0] FAST_TRAINING_SEQ
        uint32_t NUM_OF_LANES                :5;      ///<BIT [12:8] NUM_OF_LANES
        uint32_t PRE_DET_LANE                :3;      ///<BIT [15:13] PRE_DET_LANE
        uint32_t AUTO_LANE_FLIP_CTRL_EN      :1;      ///<BIT [16] AUTO_LANE_FLIP_CTRL_EN
        uint32_t DIRECT_SPEED_CHANGE         :1;      ///<BIT [17] DIRECT_SPEED_CHANGE
        uint32_t CONFIG_PHY_TX_CHANGE        :1;      ///<BIT [18] CONFIG_PHY_TX_CHANGE
        uint32_t CONFIG_TX_COMP_RX           :1;      ///<BIT [19] CONFIG_TX_COMP_RX
        uint32_t SEL_DEEMPHASIS              :1;      ///<BIT [20] SEL_DEEMPHASIS
        uint32_t GEN1_EI_INFERENCE           :1;      ///<BIT [21] GEN1_EI_INFERENCE
        uint32_t SELECT_DEEMPH_VAR_MUX       :1;      ///<BIT [22] SELECT_DEEMPH_VAR_MUX
        uint32_t SELECTABLE_DEEMPH_BIT_MUX   :1;      ///<BIT [23] SELECTABLE_DEEMPH_BIT_MUX
        uint32_t LANE_UNDER_TEST             :4;      ///<BIT [27:24] LANE_UNDER_TEST
        uint32_t EQ_FOR_LOOPBACK             :1;      ///<BIT [28] EQ_FOR_LOOPBACK
        uint32_t TX_MOD_CMPL_PATTERN_FOR_LOOPBACK :1;      ///<BIT [29] TX_MOD_CMPL_PATTERN_FOR_LOOPBACK
        uint32_t FORCE_LANE_FLIP             :1;      ///<BIT [30] FORCE_LANE_FLIP
        uint32_t SUPPORT_MOD_TS              :1;      ///<BIT [31] SUPPORT_MOD_TS
    } b;
} Gen2CtrlOff_t;

/// @brief 0x11C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TARGET_MAP_PF               :6;      ///<BIT [5:0] TARGET_MAP_PF
        uint32_t TARGET_MAP_ROM              :1;      ///<BIT [6] TARGET_MAP_ROM
        uint32_t TARGET_MAP_VF               :6;      ///<BIT [12:7] TARGET_MAP_VF
        uint32_t TARGET_MAP_RESERVED_13_15   :3;      ///<BIT [15:13] TARGET_MAP_RESERVED_13_15
        uint32_t TARGET_MAP_INDEX            :5;      ///<BIT [20:16] TARGET_MAP_INDEX
        uint32_t TARGET_MAP_RESERVED_21_31   :11;     ///<BIT [31:21] TARGET_MAP_RESERVED_21_31
    } b;
} TrgtMapCtrlOff_t;

/// @brief 0x18C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RADM_CLK_GATING_EN          :1;      ///<BIT [0] RADM_CLK_GATING_EN
        uint32_t AXI_CLK_GATING_EN           :1;      ///<BIT [1] AXI_CLK_GATING_EN
        uint32_t RSVDP_2                     :30;     ///<BIT [31:2] RSVDP_2
    } b;
} ClockGatingCtrlOff_t;

/// @brief 0x190
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GEN3_ZRXDC_NONCOMPL         :1;      ///<BIT [0] GEN3_ZRXDC_NONCOMPL
        uint32_t NO_SEED_VALUE_CHANGE        :1;      ///<BIT [1] NO_SEED_VALUE_CHANGE
        uint32_t RSVDP_2                     :6;      ///<BIT [7:2] RSVDP_2
        uint32_t DISABLE_SCRAMBLER_GEN_3     :1;      ///<BIT [8] DISABLE_SCRAMBLER_GEN_3
        uint32_t EQ_PHASE_2_3                :1;      ///<BIT [9] EQ_PHASE_2_3
        uint32_t EQ_EIEOS_CNT                :1;      ///<BIT [10] EQ_EIEOS_CNT
        uint32_t EQ_REDO                     :1;      ///<BIT [11] EQ_REDO
        uint32_t RXEQ_PH01_EN                :1;      ///<BIT [12] RXEQ_PH01_EN
        uint32_t RXEQ_RGRDLESS_RXTS          :1;      ///<BIT [13] RXEQ_RGRDLESS_RXTS
        uint32_t RSVDP_14                    :2;      ///<BIT [15:14] RSVDP_14
        uint32_t GEN3_EQUALIZATION_DISABLE   :1;      ///<BIT [16] GEN3_EQUALIZATION_DISABLE
        uint32_t GEN3_DLLP_XMT_DELAY_DISABLE :1;      ///<BIT [17] GEN3_DLLP_XMT_DELAY_DISABLE
        uint32_t GEN3_DC_BALANCE_DISABLE     :1;      ///<BIT [18] GEN3_DC_BALANCE_DISABLE
        uint32_t RSVDP_19                    :2;      ///<BIT [20:19] RSVDP_19
        uint32_t AUTO_EQ_DISABLE             :1;      ///<BIT [21] AUTO_EQ_DISABLE
        uint32_t USP_SEND_8GT_EQ_TS2_DISABLE :1;      ///<BIT [22] USP_SEND_8GT_EQ_TS2_DISABLE
        uint32_t GEN3_EQ_INVREQ_EVAL_DIFF_DISABLE :1;      ///<BIT [23] GEN3_EQ_INVREQ_EVAL_DIFF_DISABLE
        uint32_t RATE_SHADOW_SEL             :2;      ///<BIT [25:24] RATE_SHADOW_SEL
        uint32_t RSVDP_26                    :6;      ///<BIT [31:26] RSVDP_26
    } b;
} Gen3RelatedOff_t;

/// @brief 0x1A8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GEN3_EQ_FB_MODE             :4;      ///<BIT [3:0] GEN3_EQ_FB_MODE
        uint32_t GEN3_EQ_PHASE23_EXIT_MODE   :1;      ///<BIT [4] GEN3_EQ_PHASE23_EXIT_MODE
        uint32_t GEN3_EQ_EVAL_2MS_DISABLE    :1;      ///<BIT [5] GEN3_EQ_EVAL_2MS_DISABLE
        uint32_t GEN3_LOWER_RATE_EQ_REDO_ENABLE :1;      ///<BIT [6] GEN3_LOWER_RATE_EQ_REDO_ENABLE
        uint32_t RSVDP_7                     :1;      ///<BIT [7] RSVDP_7
        uint32_t GEN3_EQ_PSET_REQ_VEC        :16;     ///<BIT [23:8] GEN3_EQ_PSET_REQ_VEC
        uint32_t GEN3_EQ_FOM_INC_INITIAL_EVAL :1;      ///<BIT [24] GEN3_EQ_FOM_INC_INITIAL_EVAL
        uint32_t GEN3_EQ_PSET_REQ_AS_COEF    :1;      ///<BIT [25] GEN3_EQ_PSET_REQ_AS_COEF
        uint32_t GEN3_REQ_SEND_CONSEC_EIEOS_FOR_PSET_MAP :1;      ///<BIT [26] GEN3_REQ_SEND_CONSEC_EIEOS_FOR_PSET_MAP
        uint32_t GEN3_EQ_REQ_NUM             :3;      ///<BIT [29:27] GEN3_EQ_REQ_NUM
        uint32_t GEN3_SUPPORT_FINITE_EQ_REQUEST :1;      ///<BIT [30] GEN3_SUPPORT_FINITE_EQ_REQUEST
        uint32_t RSVDP_31                    :1;      ///<BIT [31] RSVDP_31
    } b;
} Gen3EqControlOff_t;

/// @brief 0x1B4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t NP_PASS_P                   :8;      ///<BIT [7:0] NP_PASS_P
        uint32_t CPL_PASS_P                  :8;      ///<BIT [15:8] CPL_PASS_P
        uint32_t RSVDP_16                    :16;     ///<BIT [31:16] RSVDP_16
    } b;
} OrderRuleCtrlOff_t;

/// @brief 0x1B8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LPBK_RXVALID                :16;     ///<BIT [15:0] LPBK_RXVALID
        uint32_t RXSTATUS_LANE               :6;      ///<BIT [21:16] RXSTATUS_LANE
        uint32_t RSVDP_22                    :2;      ///<BIT [23:22] RSVDP_22
        uint32_t RXSTATUS_VALUE              :3;      ///<BIT [26:24] RXSTATUS_VALUE
        uint32_t RSVDP_27                    :4;      ///<BIT [30:27] RSVDP_27
        uint32_t PIPE_LOOPBACK               :1;      ///<BIT [31] PIPE_LOOPBACK
    } b;
} PipeLoopbackControlOff_t;

/// @brief 0x1BC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DBI_RO_WR_EN                :1;      ///<BIT [0] DBI_RO_WR_EN
        uint32_t DEFAULT_TARGET              :1;      ///<BIT [1] DEFAULT_TARGET
        uint32_t UR_CA_MASK_4_TRGT1          :1;      ///<BIT [2] UR_CA_MASK_4_TRGT1
        uint32_t SIMPLIFIED_REPLAY_TIMER     :1;      ///<BIT [3] SIMPLIFIED_REPLAY_TIMER
        uint32_t DISABLE_AUTO_LTR_CLR_MSG    :1;      ///<BIT [4] DISABLE_AUTO_LTR_CLR_MSG
        uint32_t ARI_DEVICE_NUMBER           :1;      ///<BIT [5] ARI_DEVICE_NUMBER
        uint32_t CPLQ_MNG_EN                 :1;      ///<BIT [6] CPLQ_MNG_EN
        uint32_t CFG_TLP_BYPASS_EN_REG       :1;      ///<BIT [7] CFG_TLP_BYPASS_EN_REG
        uint32_t CONFIG_LIMIT_REG            :10;     ///<BIT [17:8] CONFIG_LIMIT_REG
        uint32_t TARGET_ABOVE_CONFIG_LIMIT_REG :2;      ///<BIT [19:18] TARGET_ABOVE_CONFIG_LIMIT_REG
        uint32_t P2P_TRACK_CPL_TO_REG        :1;      ///<BIT [20] P2P_TRACK_CPL_TO_REG
        uint32_t P2P_ERR_RPT_CTRL            :1;      ///<BIT [21] P2P_ERR_RPT_CTRL
        uint32_t PORT_LOGIC_WR_DISABLE       :1;      ///<BIT [22] PORT_LOGIC_WR_DISABLE
        uint32_t RAS_REG_PF0_ONLY            :1;      ///<BIT [23] RAS_REG_PF0_ONLY
        uint32_t RASDES_REG_PF0_ONLY         :1;      ///<BIT [24] RASDES_REG_PF0_ONLY
        uint32_t ERR_INJ_WR_DISABLE          :1;      ///<BIT [25] ERR_INJ_WR_DISABLE
        uint32_t RSVD_26_29                  :4;      ///<BIT [29:26] rsvd_26_29
        uint32_t RSVDP_30                    :2;      ///<BIT [31:30] RSVDP_30
    } b;
} MiscControl1Off_t;

/// @brief 0x1C0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TARGET_LINK_WIDTH           :6;      ///<BIT [5:0] TARGET_LINK_WIDTH
        uint32_t DIRECT_LINK_WIDTH_CHANGE    :1;      ///<BIT [6] DIRECT_LINK_WIDTH_CHANGE
        uint32_t UPCONFIGURE_SUPPORT         :1;      ///<BIT [7] UPCONFIGURE_SUPPORT
        uint32_t RELIABILITY_LINK_WIDTH_CHANGE_ENABLE :1;      ///<BIT [8] RELIABILITY_LINK_WIDTH_CHANGE_ENABLE
        uint32_t RSVDP_9                     :23;     ///<BIT [31:9] RSVDP_9
    } b;
} MultiLaneControlOff_t;

/// @brief 0x1C4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RXSTANDBY_CONTROL           :7;      ///<BIT [6:0] RXSTANDBY_CONTROL
        uint32_t RSVDP_7                     :1;      ///<BIT [7] RSVDP_7
        uint32_t L1SUB_EXIT_MODE             :1;      ///<BIT [8] L1SUB_EXIT_MODE
        uint32_t L1_NOWAIT_P1                :1;      ///<BIT [9] L1_NOWAIT_P1
        uint32_t L1_CLK_SEL                  :1;      ///<BIT [10] L1_CLK_SEL
        uint32_t RSVD_11                     :1;      ///<BIT [11] rsvd_11
        uint32_t PHY_RST_TIMER               :18;     ///<BIT [29:12] PHY_RST_TIMER
        uint32_t PHY_PERST_ON_WARM_RESET     :1;      ///<BIT [30] PHY_PERST_ON_WARM_RESET
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} PhyInteropCtrlOff_t;

/// @brief 0x1C8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LOOK_UP_ID                  :31;     ///<BIT [30:0] LOOK_UP_ID
        uint32_t DELETE_EN                   :1;      ///<BIT [31] DELETE_EN
    } b;
} TrgtCplLutDeleteEntryOff_t;

/// @brief 0x1CC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AUTO_FLUSH_EN               :1;      ///<BIT [0] AUTO_FLUSH_EN
        uint32_t RSVDP_1                     :23;     ///<BIT [23:1] RSVDP_1
        uint32_t AUTO_FLUSH_TIMEOUT          :8;      ///<BIT [31:24] AUTO_FLUSH_TIMEOUT
    } b;
} LinkFlushControlOff_t;

/// @brief 0x1D0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AMBA_ERROR_RESPONSE_GLOBAL  :1;      ///<BIT [0] AMBA_ERROR_RESPONSE_GLOBAL
        uint32_t RSVDP_1                     :1;      ///<BIT [1] RSVDP_1
        uint32_t AMBA_ERROR_RESPONSE_VENDORID :1;      ///<BIT [2] AMBA_ERROR_RESPONSE_VENDORID
        uint32_t AMBA_ERROR_RESPONSE_CRS     :2;      ///<BIT [4:3] AMBA_ERROR_RESPONSE_CRS
        uint32_t RSVDP_5                     :5;      ///<BIT [9:5] RSVDP_5
        uint32_t AMBA_ERROR_RESPONSE_MAP     :6;      ///<BIT [15:10] AMBA_ERROR_RESPONSE_MAP
        uint32_t RSVDP_16                    :14;     ///<BIT [29:16] RSVDP_16
        uint32_t RSVD_30_31                  :2;      ///<BIT [31:30] rsvd_30_31
    } b;
} AmbaErrorResponseDefaultOff_t;

/// @brief 0x1D4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LINK_TIMEOUT_PERIOD_DEFAULT :8;      ///<BIT [7:0] LINK_TIMEOUT_PERIOD_DEFAULT
        uint32_t LINK_TIMEOUT_ENABLE_DEFAULT :1;      ///<BIT [8] LINK_TIMEOUT_ENABLE_DEFAULT
        uint32_t RSVDP_9                     :23;     ///<BIT [31:9] RSVDP_9
    } b;
} AmbaLinkTimeoutOff_t;

/// @brief 0x1D8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :1;      ///<BIT [0] RSVDP_0
        uint32_t AX_SNP_EN                   :1;      ///<BIT [1] AX_SNP_EN
        uint32_t RSVDP_2                     :1;      ///<BIT [2] RSVDP_2
        uint32_t AX_MSTR_ORDR_P_EVENT_SEL    :2;      ///<BIT [4:3] AX_MSTR_ORDR_P_EVENT_SEL
        uint32_t RSVDP_5                     :2;      ///<BIT [6:5] RSVDP_5
        uint32_t AX_MSTR_ZEROLREAD_FW        :1;      ///<BIT [7] AX_MSTR_ZEROLREAD_FW
        uint32_t RSVDP_8                     :24;     ///<BIT [31:8] RSVDP_8
    } b;
} AmbaOrderingCtrlOff_t;

/// @brief 0x1E0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_MEMTYPE_VALUE           :1;      ///<BIT [0] CFG_MEMTYPE_VALUE
        uint32_t RSVDP_1                     :1;      ///<BIT [1] RSVDP_1
        uint32_t CFG_MEMTYPE_BOUNDARY_LOW_ADDR :30;     ///<BIT [31:2] CFG_MEMTYPE_BOUNDARY_LOW_ADDR
    } b;
} CoherencyControl1Off_t;

/// @brief 0x1E8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_2                    :3;      ///<BIT [2:0] rsvd_0_2
        uint32_t CFG_MSTR_ARCACHE_MODE       :4;      ///<BIT [6:3] CFG_MSTR_ARCACHE_MODE
        uint32_t RSVD_7_10                   :4;      ///<BIT [10:7] rsvd_7_10
        uint32_t CFG_MSTR_AWCACHE_MODE       :4;      ///<BIT [14:11] CFG_MSTR_AWCACHE_MODE
        uint32_t RSVD_15_18                  :4;      ///<BIT [18:15] rsvd_15_18
        uint32_t CFG_MSTR_ARCACHE_VALUE      :4;      ///<BIT [22:19] CFG_MSTR_ARCACHE_VALUE
        uint32_t RSVD_23_26                  :4;      ///<BIT [26:23] rsvd_23_26
        uint32_t CFG_MSTR_AWCACHE_VALUE      :4;      ///<BIT [30:27] CFG_MSTR_AWCACHE_VALUE
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} CoherencyControl3Off_t;

/// @brief 0x1F0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_AXIMSTR_MSG_ADDR_LOW_RESERVED :12;     ///<BIT [11:0] CFG_AXIMSTR_MSG_ADDR_LOW_RESERVED
        uint32_t CFG_AXIMSTR_MSG_ADDR_LOW    :20;     ///<BIT [31:12] CFG_AXIMSTR_MSG_ADDR_LOW
    } b;
} AxiMstrMsgAddrLowOff_t;

/// @brief 0x41C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIPM_VDM_TRAFFIC_BLOCKED   :1;      ///<BIT [0] PCIPM_VDM_TRAFFIC_BLOCKED
        uint32_t PCIPM_NEW_TLP_CLIENT0_BLOCKED :1;      ///<BIT [1] PCIPM_NEW_TLP_CLIENT0_BLOCKED
        uint32_t PCIPM_NEW_TLP_CLIENT1_BLOCKED :1;      ///<BIT [2] PCIPM_NEW_TLP_CLIENT1_BLOCKED
        uint32_t PCIPM_NEW_TLP_CLIENT2_BLOCKED :1;      ///<BIT [3] PCIPM_NEW_TLP_CLIENT2_BLOCKED
        uint32_t PCIPM_RESERVED_4_7          :4;      ///<BIT [7:4] PCIPM_RESERVED_4_7
        uint32_t RSVDP_8                     :24;     ///<BIT [31:8] RSVDP_8
    } b;
} PcipmTrafficCtrlOff_t;

/// @brief 0x430
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SNOOP_LATENCY_VALUE         :10;     ///<BIT [9:0] SNOOP_LATENCY_VALUE
        uint32_t SNOOP_LATENCY_SCALE         :3;      ///<BIT [12:10] SNOOP_LATENCY_SCALE
        uint32_t RSVDP_13                    :2;      ///<BIT [14:13] RSVDP_13
        uint32_t SNOOP_LATENCY_REQUIRE       :1;      ///<BIT [15] SNOOP_LATENCY_REQUIRE
        uint32_t NO_SNOOP_LATENCY_VALUE      :10;     ///<BIT [25:16] NO_SNOOP_LATENCY_VALUE
        uint32_t NO_SNOOP_LATENCY_SCALE      :3;      ///<BIT [28:26] NO_SNOOP_LATENCY_SCALE
        uint32_t RSVDP_29                    :2;      ///<BIT [30:29] RSVDP_29
        uint32_t NO_SNOOP_LATENCY_REQUIRE    :1;      ///<BIT [31] NO_SNOOP_LATENCY_REQUIRE
    } b;
} PlLtrLatencyOff_t;

/// @brief 0x440
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AUX_CLK_FREQ                :10;     ///<BIT [9:0] AUX_CLK_FREQ
        uint32_t RSVDP_10                    :22;     ///<BIT [31:10] RSVDP_10
    } b;
} AuxClkFreqOff_t;

/// @brief 0x444
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t L1SUB_T_POWER_OFF           :2;      ///<BIT [1:0] L1SUB_T_POWER_OFF
        uint32_t L1SUB_T_L1_2                :4;      ///<BIT [5:2] L1SUB_T_L1_2
        uint32_t L1SUB_T_PCLKACK_LOW         :2;      ///<BIT [7:6] L1SUB_T_PCLKACK_LOW
        uint32_t L1SUB_LOW_POWER_CLOCK_SWITCH_MODE :1;      ///<BIT [8] L1SUB_LOW_POWER_CLOCK_SWITCH_MODE
        uint32_t L1SUB_T_PCLKACK_HIGH        :5;      ///<BIT [13:9] L1SUB_T_PCLKACK_HIGH
        uint32_t RSVDP_14                    :18;     ///<BIT [31:14] RSVDP_14
    } b;
} L1SubstatesOff_t;

/// @brief 0x448
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t POWERDOWN_FORCE             :1;      ///<BIT [0] POWERDOWN_FORCE
        uint32_t POWERDOWN_VMAIN_ACK         :1;      ///<BIT [1] POWERDOWN_VMAIN_ACK
        uint32_t RSVDP_2                     :2;      ///<BIT [3:2] RSVDP_2
        uint32_t POWERDOWN_MAC_POWERDOWN     :4;      ///<BIT [7:4] POWERDOWN_MAC_POWERDOWN
        uint32_t POWERDOWN_PHY_POWERDOWN     :4;      ///<BIT [11:8] POWERDOWN_PHY_POWERDOWN
        uint32_t RSVDP_12                    :20;     ///<BIT [31:12] RSVDP_12
    } b;
} PowerdownCtrlStatusOff_t;

/// @brief 0x44C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PMA_PIPE_RST_DELAY_TIMER    :6;      ///<BIT [5:0] PMA_PIPE_RST_DELAY_TIMER
        uint32_t RSVDP_6                     :2;      ///<BIT [7:6] RSVDP_6
        uint32_t DSP_PCIPM_L1_ENTER_DELAY    :4;      ///<BIT [11:8] DSP_PCIPM_L1_ENTER_DELAY
        uint32_t RSVDP_12                    :20;     ///<BIT [31:12] RSVDP_12
    } b;
} PhyInteropCtrl2Off_t;

/// @brief 0x480
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GEN4_MARGINING_NUM_TIMING_STEPS :6;      ///<BIT [5:0] GEN4_MARGINING_NUM_TIMING_STEPS
        uint32_t RSVDP_6                     :2;      ///<BIT [7:6] RSVDP_6
        uint32_t GEN4_MARGINING_MAX_TIMING_OFFSET :6;      ///<BIT [13:8] GEN4_MARGINING_MAX_TIMING_OFFSET
        uint32_t RSVDP_14                    :2;      ///<BIT [15:14] RSVDP_14
        uint32_t GEN4_MARGINING_NUM_VOLTAGE_STEPS :7;      ///<BIT [22:16] GEN4_MARGINING_NUM_VOLTAGE_STEPS
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t GEN4_MARGINING_MAX_VOLTAGE_OFFSET :6;      ///<BIT [29:24] GEN4_MARGINING_MAX_VOLTAGE_OFFSET
        uint32_t RSVDP_30                    :2;      ///<BIT [31:30] RSVDP_30
    } b;
} Gen4LaneMargining1Off_t;

/// @brief 0x484
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GEN4_MARGINING_SAMPLE_RATE_VOLTAGE :6;      ///<BIT [5:0] GEN4_MARGINING_SAMPLE_RATE_VOLTAGE
        uint32_t RSVDP_6                     :2;      ///<BIT [7:6] RSVDP_6
        uint32_t GEN4_MARGINING_SAMPLE_RATE_TIMING :6;      ///<BIT [13:8] GEN4_MARGINING_SAMPLE_RATE_TIMING
        uint32_t RSVDP_14                    :2;      ///<BIT [15:14] RSVDP_14
        uint32_t GEN4_MARGINING_MAXLANES     :5;      ///<BIT [20:16] GEN4_MARGINING_MAXLANES
        uint32_t RSVDP_21                    :3;      ///<BIT [23:21] RSVDP_21
        uint32_t GEN4_MARGINING_VOLTAGE_SUPPORTED :1;      ///<BIT [24] GEN4_MARGINING_VOLTAGE_SUPPORTED
        uint32_t GEN4_MARGINING_IND_UP_DOWN_VOLTAGE :1;      ///<BIT [25] GEN4_MARGINING_IND_UP_DOWN_VOLTAGE
        uint32_t GEN4_MARGINING_IND_LEFT_RIGHT_TIMING :1;      ///<BIT [26] GEN4_MARGINING_IND_LEFT_RIGHT_TIMING
        uint32_t GEN4_MARGINING_SAMPLE_REPORTING_METHOD :1;      ///<BIT [27] GEN4_MARGINING_SAMPLE_REPORTING_METHOD
        uint32_t GEN4_MARGINING_IND_ERROR_SAMPLER :1;      ///<BIT [28] GEN4_MARGINING_IND_ERROR_SAMPLER
        uint32_t RSVDP_29                    :2;      ///<BIT [30:29] RSVDP_29
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} Gen4LaneMargining2Off_t;

/// @brief 0x488
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GEN5_MARGINING_NUM_TIMING_STEPS :6;      ///<BIT [5:0] GEN5_MARGINING_NUM_TIMING_STEPS
        uint32_t RSVDP_6                     :2;      ///<BIT [7:6] RSVDP_6
        uint32_t GEN5_MARGINING_MAX_TIMING_OFFSET :6;      ///<BIT [13:8] GEN5_MARGINING_MAX_TIMING_OFFSET
        uint32_t RSVDP_14                    :2;      ///<BIT [15:14] RSVDP_14
        uint32_t GEN5_MARGINING_NUM_VOLTAGE_STEPS :7;      ///<BIT [22:16] GEN5_MARGINING_NUM_VOLTAGE_STEPS
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t GEN5_MARGINING_MAX_VOLTAGE_OFFSET :6;      ///<BIT [29:24] GEN5_MARGINING_MAX_VOLTAGE_OFFSET
        uint32_t RSVDP_30                    :2;      ///<BIT [31:30] RSVDP_30
    } b;
} Gen5LaneMargining1Off_t;

/// @brief 0x48C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GEN5_MARGINING_SAMPLE_RATE_VOLTAGE :6;      ///<BIT [5:0] GEN5_MARGINING_SAMPLE_RATE_VOLTAGE
        uint32_t RSVDP_6                     :2;      ///<BIT [7:6] RSVDP_6
        uint32_t GEN5_MARGINING_SAMPLE_RATE_TIMING :6;      ///<BIT [13:8] GEN5_MARGINING_SAMPLE_RATE_TIMING
        uint32_t RSVDP_14                    :2;      ///<BIT [15:14] RSVDP_14
        uint32_t GEN5_MARGINING_MAXLANES     :5;      ///<BIT [20:16] GEN5_MARGINING_MAXLANES
        uint32_t RSVDP_21                    :3;      ///<BIT [23:21] RSVDP_21
        uint32_t GEN5_MARGINING_VOLTAGE_SUPPORTED :1;      ///<BIT [24] GEN5_MARGINING_VOLTAGE_SUPPORTED
        uint32_t GEN5_MARGINING_IND_UP_DOWN_VOLTAGE :1;      ///<BIT [25] GEN5_MARGINING_IND_UP_DOWN_VOLTAGE
        uint32_t GEN5_MARGINING_IND_LEFT_RIGHT_TIMING :1;      ///<BIT [26] GEN5_MARGINING_IND_LEFT_RIGHT_TIMING
        uint32_t GEN5_MARGINING_SAMPLE_REPORTING_METHOD :1;      ///<BIT [27] GEN5_MARGINING_SAMPLE_REPORTING_METHOD
        uint32_t GEN5_MARGINING_IND_ERROR_SAMPLER :1;      ///<BIT [28] GEN5_MARGINING_IND_ERROR_SAMPLER
        uint32_t RSVDP_29                    :3;      ///<BIT [31:29] RSVDP_29
    } b;
} Gen5LaneMargining2Off_t;

/// @brief 0x490
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_MESSAGE_BUS_WRITE_BUFFER_DEPTH :4;      ///<BIT [3:0] RX_MESSAGE_BUS_WRITE_BUFFER_DEPTH
        uint32_t TX_MESSAGE_BUS_MIN_WRITE_BUFFER_DEPTH :4;      ///<BIT [7:4] TX_MESSAGE_BUS_MIN_WRITE_BUFFER_DEPTH
        uint32_t PIPE_GARBAGE_DATA_MODE      :1;      ///<BIT [8] PIPE_GARBAGE_DATA_MODE
        uint32_t RSVD_9_31                   :23;     ///<BIT [31:9] rsvd_9_31
    } b;
} PipeRelatedOff_t;

/// @brief 0x57C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DBI_FUNCTION_BANK_CTRL_REG  :1;      ///<BIT [0] DBI_FUNCTION_BANK_CTRL_REG
        uint32_t DBI_FUNCTION_BANK_CTRL_REG_RSVD :31;     ///<BIT [31:1] DBI_FUNCTION_BANK_CTRL_REG_RSVD
    } b;
} DbiFunctionBankCtrlOff_t;

/// @brief 0x58C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IDE_CTRL_DISABLE            :1;      ///<BIT [0] IDE_CTRL_DISABLE
        uint32_t EARLY_TDISP_TIMEOUT_DISABLE :1;      ///<BIT [1] EARLY_TDISP_TIMEOUT_DISABLE
        uint32_t RSVDP_2                     :30;     ///<BIT [31:2] RSVDP_2
    } b;
} IdeCtrlOff_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_TYPE0_VENDOR_ID         :16;     ///<BIT [15:0] PCI_TYPE0_VENDOR_ID
        uint32_t PCI_TYPE0_DEVICE_ID         :16;     ///<BIT [31:16] PCI_TYPE0_DEVICE_ID
    } b;
} VfDeviceIdVendorId_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_TYPE0_IO_EN             :1;      ///<BIT [0] PCI_TYPE0_IO_EN
        uint32_t PCI_TYPE0_MEM_SPACE_EN      :1;      ///<BIT [1] PCI_TYPE0_MEM_SPACE_EN
        uint32_t PCI_TYPE0_BUS_MASTER_EN     :1;      ///<BIT [2] PCI_TYPE0_BUS_MASTER_EN
        uint32_t PCI_TYPE0_SPECIAL_CYCLE_OPERATION :1;      ///<BIT [3] PCI_TYPE0_SPECIAL_CYCLE_OPERATION
        uint32_t PCI_TYPE_MWI_ENABLE         :1;      ///<BIT [4] PCI_TYPE_MWI_ENABLE
        uint32_t PCI_TYPE_VGA_PALETTE_SNOOP  :1;      ///<BIT [5] PCI_TYPE_VGA_PALETTE_SNOOP
        uint32_t PCI_TYPE0_PARITY_ERR_EN     :1;      ///<BIT [6] PCI_TYPE0_PARITY_ERR_EN
        uint32_t PCI_TYPE_IDSEL_STEPPING     :1;      ///<BIT [7] PCI_TYPE_IDSEL_STEPPING
        uint32_t PCI_TYPE0_SERREN            :1;      ///<BIT [8] PCI_TYPE0_SERREN
        uint32_t RSVDP_9                     :1;      ///<BIT [9] RSVDP_9
        uint32_t PCI_TYPE0_INT_EN            :1;      ///<BIT [10] PCI_TYPE0_INT_EN
        uint32_t PCI_TYPE_RESERV             :5;      ///<BIT [15:11] PCI_TYPE_RESERV
        uint32_t IMM_READINESS               :1;      ///<BIT [16] IMM_READINESS
        uint32_t RSVDP_17                    :2;      ///<BIT [18:17] RSVDP_17
        uint32_t INT_STATUS                  :1;      ///<BIT [19] INT_STATUS
        uint32_t CAP_LIST                    :1;      ///<BIT [20] CAP_LIST
        uint32_t FAST_66MHZ_CAP              :1;      ///<BIT [21] FAST_66MHZ_CAP
        uint32_t RSVDP_22                    :1;      ///<BIT [22] RSVDP_22
        uint32_t FAST_B2B_CAP                :1;      ///<BIT [23] FAST_B2B_CAP
        uint32_t MASTER_DPE                  :1;      ///<BIT [24] MASTER_DPE
        uint32_t DEV_SEL_TIMING              :2;      ///<BIT [26:25] DEV_SEL_TIMING
        uint32_t SIGNALED_TARGET_ABORT       :1;      ///<BIT [27] SIGNALED_TARGET_ABORT
        uint32_t RCVD_TARGET_ABORT           :1;      ///<BIT [28] RCVD_TARGET_ABORT
        uint32_t RCVD_MASTER_ABORT           :1;      ///<BIT [29] RCVD_MASTER_ABORT
        uint32_t SIGNALED_SYS_ERR            :1;      ///<BIT [30] SIGNALED_SYS_ERR
        uint32_t DETECTED_PARITY_ERR         :1;      ///<BIT [31] DETECTED_PARITY_ERR
    } b;
} VfStatusCommand_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t REVISION_ID                 :8;      ///<BIT [7:0] REVISION_ID
        uint32_t PROGRAM_INTERFACE           :8;      ///<BIT [15:8] PROGRAM_INTERFACE
        uint32_t SUBCLASS_CODE               :8;      ///<BIT [23:16] SUBCLASS_CODE
        uint32_t BASE_CLASS_CODE             :8;      ///<BIT [31:24] BASE_CLASS_CODE
    } b;
} VfClassCodeRevisionId_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CACHE_LINE_SIZE             :8;      ///<BIT [7:0] CACHE_LINE_SIZE
        uint32_t LATENCY_MASTER_TIMER        :8;      ///<BIT [15:8] LATENCY_MASTER_TIMER
        uint32_t HEADER_TYPE                 :7;      ///<BIT [22:16] HEADER_TYPE
        uint32_t MULTI_FUNC                  :1;      ///<BIT [23] MULTI_FUNC
        uint32_t BIST                        :8;      ///<BIT [31:24] BIST
    } b;
} VfBistHeaderTypeLatencyCacheLineSize_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAR0_MEM_IO                 :1;      ///<BIT [0] BAR0_MEM_IO
        uint32_t BAR0_TYPE                   :2;      ///<BIT [2:1] BAR0_TYPE
        uint32_t BAR0_PREFETCH               :1;      ///<BIT [3] BAR0_PREFETCH
        uint32_t BAR0_START                  :28;     ///<BIT [31:4] BAR0_START
    } b;
} VfBar0_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAR1_MEM_IO                 :1;      ///<BIT [0] BAR1_MEM_IO
        uint32_t BAR1_TYPE                   :2;      ///<BIT [2:1] BAR1_TYPE
        uint32_t BAR1_PREFETCH               :1;      ///<BIT [3] BAR1_PREFETCH
        uint32_t BAR1_START                  :28;     ///<BIT [31:4] BAR1_START
    } b;
} VfBar1_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAR2_MEM_IO                 :1;      ///<BIT [0] BAR2_MEM_IO
        uint32_t BAR2_TYPE                   :2;      ///<BIT [2:1] BAR2_TYPE
        uint32_t BAR2_PREFETCH               :1;      ///<BIT [3] BAR2_PREFETCH
        uint32_t BAR2_START                  :28;     ///<BIT [31:4] BAR2_START
    } b;
} VfBar2_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAR3_MEM_IO                 :1;      ///<BIT [0] BAR3_MEM_IO
        uint32_t BAR3_TYPE                   :2;      ///<BIT [2:1] BAR3_TYPE
        uint32_t BAR3_PREFETCH               :1;      ///<BIT [3] BAR3_PREFETCH
        uint32_t BAR3_START                  :28;     ///<BIT [31:4] BAR3_START
    } b;
} VfBar3_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAR4_MEM_IO                 :1;      ///<BIT [0] BAR4_MEM_IO
        uint32_t BAR4_TYPE                   :2;      ///<BIT [2:1] BAR4_TYPE
        uint32_t BAR4_PREFETCH               :1;      ///<BIT [3] BAR4_PREFETCH
        uint32_t BAR4_START                  :28;     ///<BIT [31:4] BAR4_START
    } b;
} VfBar4_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAR5_MEM_IO                 :1;      ///<BIT [0] BAR5_MEM_IO
        uint32_t BAR5_TYPE                   :2;      ///<BIT [2:1] BAR5_TYPE
        uint32_t BAR5_PREFETCH               :1;      ///<BIT [3] BAR5_PREFETCH
        uint32_t BAR5_START                  :28;     ///<BIT [31:4] BAR5_START
    } b;
} VfBar5_t;

/// @brief 0x2C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SUBSYS_VENDOR_ID            :16;     ///<BIT [15:0] SUBSYS_VENDOR_ID
        uint32_t SUBSYS_DEV_ID               :16;     ///<BIT [31:16] SUBSYS_DEV_ID
    } b;
} VfSubsystemIdSubsystemVendorId_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CAP_POINTER                 :8;      ///<BIT [7:0] CAP_POINTER
        uint32_t RSVDP_8                     :24;     ///<BIT [31:8] RSVDP_8
    } b;
} VfPciCapPtr_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INT_LINE                    :8;      ///<BIT [7:0] INT_LINE
        uint32_t INT_PIN                     :8;      ///<BIT [15:8] INT_PIN
        uint32_t RSVDP_16                    :16;     ///<BIT [31:16] RSVDP_16
    } b;
} VfMaxLatencyMinGrantIntrPinIntrLine_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSI_CAP_ID              :8;      ///<BIT [7:0] PCI_MSI_CAP_ID
        uint32_t PCI_MSI_CAP_NEXT_OFFSET     :8;      ///<BIT [15:8] PCI_MSI_CAP_NEXT_OFFSET
        uint32_t PCI_MSI_ENABLE              :1;      ///<BIT [16] PCI_MSI_ENABLE
        uint32_t PCI_MSI_MULTIPLE_MSG_CAP    :3;      ///<BIT [19:17] PCI_MSI_MULTIPLE_MSG_CAP
        uint32_t PCI_MSI_MULTIPLE_MSG_EN     :3;      ///<BIT [22:20] PCI_MSI_MULTIPLE_MSG_EN
        uint32_t PCI_MSI_64_BIT_ADDR_CAP     :1;      ///<BIT [23] PCI_MSI_64_BIT_ADDR_CAP
        uint32_t PCI_PVM_SUPPORT             :1;      ///<BIT [24] PCI_PVM_SUPPORT
        uint32_t PCI_MSI_EXT_DATA_CAP        :1;      ///<BIT [25] PCI_MSI_EXT_DATA_CAP
        uint32_t PCI_MSI_EXT_DATA_EN         :1;      ///<BIT [26] PCI_MSI_EXT_DATA_EN
        uint32_t RSVDP_27                    :5;      ///<BIT [31:27] RSVDP_27
    } b;
} VfPciMsiCapIdNextCtrl_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :2;      ///<BIT [1:0] RSVDP_0
        uint32_t PCI_MSI_CAP_OFF_04H         :30;     ///<BIT [31:2] PCI_MSI_CAP_OFF_04H
    } b;
} VfMsiCapOff04h_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSI_CAP_OFF_08H         :16;     ///<BIT [15:0] PCI_MSI_CAP_OFF_08H
        uint32_t PCI_MSI_CAP_OFF_0AH         :16;     ///<BIT [31:16] PCI_MSI_CAP_OFF_0AH
    } b;
} VfMsiCapOff08h_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSI_CAP_OFF_0CH         :16;     ///<BIT [15:0] PCI_MSI_CAP_OFF_0CH
        uint32_t PCI_MSI_CAP_OFF_0EH         :16;     ///<BIT [31:16] PCI_MSI_CAP_OFF_0EH
    } b;
} VfMsiCapOff0ch_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_ID                 :8;      ///<BIT [7:0] PCIE_CAP_ID
        uint32_t PCIE_CAP_NEXT_PTR           :8;      ///<BIT [15:8] PCIE_CAP_NEXT_PTR
        uint32_t PCIE_CAP_REG                :4;      ///<BIT [19:16] PCIE_CAP_REG
        uint32_t PCIE_DEV_PORT_TYPE          :4;      ///<BIT [23:20] PCIE_DEV_PORT_TYPE
        uint32_t PCIE_SLOT_IMP               :1;      ///<BIT [24] PCIE_SLOT_IMP
        uint32_t PCIE_INT_MSG_NUM            :5;      ///<BIT [29:25] PCIE_INT_MSG_NUM
        uint32_t RSVD                        :1;      ///<BIT [30] RSVD
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} VfPcieCapIdPcieNextCapPtrPcieCap_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_MAX_PAYLOAD_SIZE   :3;      ///<BIT [2:0] PCIE_CAP_MAX_PAYLOAD_SIZE
        uint32_t PCIE_CAP_PHANTOM_FUNC_SUPPORT :2;      ///<BIT [4:3] PCIE_CAP_PHANTOM_FUNC_SUPPORT
        uint32_t PCIE_CAP_EXT_TAG_SUPP       :1;      ///<BIT [5] PCIE_CAP_EXT_TAG_SUPP
        uint32_t PCIE_CAP_EP_L0S_ACCPT_LATENCY :3;      ///<BIT [8:6] PCIE_CAP_EP_L0S_ACCPT_LATENCY
        uint32_t PCIE_CAP_EP_L1_ACCPT_LATENCY :3;      ///<BIT [11:9] PCIE_CAP_EP_L1_ACCPT_LATENCY
        uint32_t RSVDP_12                    :3;      ///<BIT [14:12] RSVDP_12
        uint32_t PCIE_CAP_ROLE_BASED_ERR_REPORT :1;      ///<BIT [15] PCIE_CAP_ROLE_BASED_ERR_REPORT
        uint32_t RSVDP_16                    :1;      ///<BIT [16] RSVDP_16
        uint32_t RSVD_17                     :1;      ///<BIT [17] rsvd_17
        uint32_t PCIE_CAP_CAP_SLOT_PWR_LMT_VALUE :8;      ///<BIT [25:18] PCIE_CAP_CAP_SLOT_PWR_LMT_VALUE
        uint32_t PCIE_CAP_CAP_SLOT_PWR_LMT_SCALE :2;      ///<BIT [27:26] PCIE_CAP_CAP_SLOT_PWR_LMT_SCALE
        uint32_t PCIE_CAP_FLR_CAP            :1;      ///<BIT [28] PCIE_CAP_FLR_CAP
        uint32_t RSVD_29                     :1;      ///<BIT [29] rsvd_29
        uint32_t PCIE_CAP_TEE_IO_SUPPORTED   :1;      ///<BIT [30] PCIE_CAP_TEE_IO_SUPPORTED
        uint32_t RSVDP_31                    :1;      ///<BIT [31] RSVDP_31
    } b;
} VfDeviceCapabilities_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_CORR_ERR_REPORT_EN :1;      ///<BIT [0] PCIE_CAP_CORR_ERR_REPORT_EN
        uint32_t PCIE_CAP_NON_FATAL_ERR_REPORT_EN :1;      ///<BIT [1] PCIE_CAP_NON_FATAL_ERR_REPORT_EN
        uint32_t PCIE_CAP_FATAL_ERR_REPORT_EN :1;      ///<BIT [2] PCIE_CAP_FATAL_ERR_REPORT_EN
        uint32_t PCIE_CAP_UNSUPPORT_REQ_REP_EN :1;      ///<BIT [3] PCIE_CAP_UNSUPPORT_REQ_REP_EN
        uint32_t PCIE_CAP_EN_REL_ORDER       :1;      ///<BIT [4] PCIE_CAP_EN_REL_ORDER
        uint32_t PCIE_CAP_MAX_PAYLOAD_SIZE_CS :3;      ///<BIT [7:5] PCIE_CAP_MAX_PAYLOAD_SIZE_CS
        uint32_t PCIE_CAP_EXT_TAG_EN         :1;      ///<BIT [8] PCIE_CAP_EXT_TAG_EN
        uint32_t PCIE_CAP_PHANTOM_FUNC_EN    :1;      ///<BIT [9] PCIE_CAP_PHANTOM_FUNC_EN
        uint32_t PCIE_CAP_AUX_POWER_PM_EN    :1;      ///<BIT [10] PCIE_CAP_AUX_POWER_PM_EN
        uint32_t PCIE_CAP_EN_NO_SNOOP        :1;      ///<BIT [11] PCIE_CAP_EN_NO_SNOOP
        uint32_t PCIE_CAP_MAX_READ_REQ_SIZE  :3;      ///<BIT [14:12] PCIE_CAP_MAX_READ_REQ_SIZE
        uint32_t PCIE_CAP_INITIATE_FLR       :1;      ///<BIT [15] PCIE_CAP_INITIATE_FLR
        uint32_t PCIE_CAP_CORR_ERR_DETECTED  :1;      ///<BIT [16] PCIE_CAP_CORR_ERR_DETECTED
        uint32_t PCIE_CAP_NON_FATAL_ERR_DETECTED :1;      ///<BIT [17] PCIE_CAP_NON_FATAL_ERR_DETECTED
        uint32_t PCIE_CAP_FATAL_ERR_DETECTED :1;      ///<BIT [18] PCIE_CAP_FATAL_ERR_DETECTED
        uint32_t PCIE_CAP_UNSUPPORTED_REQ_DETECTED :1;      ///<BIT [19] PCIE_CAP_UNSUPPORTED_REQ_DETECTED
        uint32_t PCIE_CAP_AUX_POWER_DETECTED :1;      ///<BIT [20] PCIE_CAP_AUX_POWER_DETECTED
        uint32_t PCIE_CAP_TRANS_PENDING      :1;      ///<BIT [21] PCIE_CAP_TRANS_PENDING
        uint32_t RSVD_22                     :1;      ///<BIT [22] rsvd_22
        uint32_t RSVDP_23                    :9;      ///<BIT [31:23] RSVDP_23
    } b;
} VfDeviceControlDeviceStatus_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_MAX_LINK_SPEED     :4;      ///<BIT [3:0] PCIE_CAP_MAX_LINK_SPEED
        uint32_t PCIE_CAP_MAX_LINK_WIDTH     :6;      ///<BIT [9:4] PCIE_CAP_MAX_LINK_WIDTH
        uint32_t PCIE_CAP_ACTIVE_STATE_LINK_PM_SUPPORT :2;      ///<BIT [11:10] PCIE_CAP_ACTIVE_STATE_LINK_PM_SUPPORT
        uint32_t PCIE_CAP_L0S_EXIT_LATENCY   :3;      ///<BIT [14:12] PCIE_CAP_L0S_EXIT_LATENCY
        uint32_t PCIE_CAP_L1_EXIT_LATENCY    :3;      ///<BIT [17:15] PCIE_CAP_L1_EXIT_LATENCY
        uint32_t PCIE_CAP_CLOCK_POWER_MAN    :1;      ///<BIT [18] PCIE_CAP_CLOCK_POWER_MAN
        uint32_t PCIE_CAP_SURPRISE_DOWN_ERR_REP_CAP :1;      ///<BIT [19] PCIE_CAP_SURPRISE_DOWN_ERR_REP_CAP
        uint32_t PCIE_CAP_DLL_ACTIVE_REP_CAP :1;      ///<BIT [20] PCIE_CAP_DLL_ACTIVE_REP_CAP
        uint32_t PCIE_CAP_LINK_BW_NOT_CAP    :1;      ///<BIT [21] PCIE_CAP_LINK_BW_NOT_CAP
        uint32_t PCIE_CAP_ASPM_OPT_COMPLIANCE :1;      ///<BIT [22] PCIE_CAP_ASPM_OPT_COMPLIANCE
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t PCIE_CAP_PORT_NUM           :8;      ///<BIT [31:24] PCIE_CAP_PORT_NUM
    } b;
} VfLinkCapabilities_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_ACTIVE_STATE_LINK_PM_CONTROL :2;      ///<BIT [1:0] PCIE_CAP_ACTIVE_STATE_LINK_PM_CONTROL
        uint32_t RSVDP_2                     :1;      ///<BIT [2] RSVDP_2
        uint32_t PCIE_CAP_RCB                :1;      ///<BIT [3] PCIE_CAP_RCB
        uint32_t PCIE_CAP_LINK_DISABLE       :1;      ///<BIT [4] PCIE_CAP_LINK_DISABLE
        uint32_t PCIE_CAP_RETRAIN_LINK       :1;      ///<BIT [5] PCIE_CAP_RETRAIN_LINK
        uint32_t PCIE_CAP_COMMON_CLK_CONFIG  :1;      ///<BIT [6] PCIE_CAP_COMMON_CLK_CONFIG
        uint32_t PCIE_CAP_EXTENDED_SYNCH     :1;      ///<BIT [7] PCIE_CAP_EXTENDED_SYNCH
        uint32_t PCIE_CAP_EN_CLK_POWER_MAN   :1;      ///<BIT [8] PCIE_CAP_EN_CLK_POWER_MAN
        uint32_t PCIE_CAP_HW_AUTO_WIDTH_DISABLE :1;      ///<BIT [9] PCIE_CAP_HW_AUTO_WIDTH_DISABLE
        uint32_t PCIE_CAP_LINK_BW_MAN_INT_EN :1;      ///<BIT [10] PCIE_CAP_LINK_BW_MAN_INT_EN
        uint32_t PCIE_CAP_LINK_AUTO_BW_INT_EN :1;      ///<BIT [11] PCIE_CAP_LINK_AUTO_BW_INT_EN
        uint32_t RSVD_12_13                  :2;      ///<BIT [13:12] rsvd_12_13
        uint32_t PCIE_CAP_DRS_SIGNALING_CONTROL :2;      ///<BIT [15:14] PCIE_CAP_DRS_SIGNALING_CONTROL
        uint32_t PCIE_CAP_LINK_SPEED         :4;      ///<BIT [19:16] PCIE_CAP_LINK_SPEED
        uint32_t PCIE_CAP_NEGO_LINK_WIDTH    :6;      ///<BIT [25:20] PCIE_CAP_NEGO_LINK_WIDTH
        uint32_t RSVDP_26                    :1;      ///<BIT [26] RSVDP_26
        uint32_t PCIE_CAP_LINK_TRAINING      :1;      ///<BIT [27] PCIE_CAP_LINK_TRAINING
        uint32_t PCIE_CAP_SLOT_CLK_CONFIG    :1;      ///<BIT [28] PCIE_CAP_SLOT_CLK_CONFIG
        uint32_t PCIE_CAP_DLL_ACTIVE         :1;      ///<BIT [29] PCIE_CAP_DLL_ACTIVE
        uint32_t PCIE_CAP_LINK_BW_MAN_STATUS :1;      ///<BIT [30] PCIE_CAP_LINK_BW_MAN_STATUS
        uint32_t PCIE_CAP_LINK_AUTO_BW_STATUS :1;      ///<BIT [31] PCIE_CAP_LINK_AUTO_BW_STATUS
    } b;
} VfLinkControlLinkStatus_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_CPL_TIMEOUT_RANGE  :4;      ///<BIT [3:0] PCIE_CAP_CPL_TIMEOUT_RANGE
        uint32_t PCIE_CAP_CPL_TIMEOUT_DISABLE_SUPPORT :1;      ///<BIT [4] PCIE_CAP_CPL_TIMEOUT_DISABLE_SUPPORT
        uint32_t PCIE_CAP_ARI_FORWARD_SUPPORT :1;      ///<BIT [5] PCIE_CAP_ARI_FORWARD_SUPPORT
        uint32_t PCIE_CAP_ATOMIC_ROUTING_SUPP :1;      ///<BIT [6] PCIE_CAP_ATOMIC_ROUTING_SUPP
        uint32_t PCIE_CAP_32_ATOMIC_CPL_SUPP :1;      ///<BIT [7] PCIE_CAP_32_ATOMIC_CPL_SUPP
        uint32_t PCIE_CAP_64_ATOMIC_CPL_SUPP :1;      ///<BIT [8] PCIE_CAP_64_ATOMIC_CPL_SUPP
        uint32_t PCIE_CAP_128_CAS_CPL_SUPP   :1;      ///<BIT [9] PCIE_CAP_128_CAS_CPL_SUPP
        uint32_t PCIE_CAP_NO_RO_EN_PR2PR_PAR :1;      ///<BIT [10] PCIE_CAP_NO_RO_EN_PR2PR_PAR
        uint32_t PCIE_CAP_LTR_SUPP           :1;      ///<BIT [11] PCIE_CAP_LTR_SUPP
        uint32_t RSVD_12_13                  :2;      ///<BIT [13:12] rsvd_12_13
        uint32_t PCIE_CAP2_LN_SYS_CLS        :2;      ///<BIT [15:14] PCIE_CAP2_LN_SYS_CLS
        uint32_t PCIE_CAP2_10_BIT_TAG_COMP_SUPPORT :1;      ///<BIT [16] PCIE_CAP2_10_BIT_TAG_COMP_SUPPORT
        uint32_t PCIE_CAP2_10_BIT_TAG_REQ_SUPPORT :1;      ///<BIT [17] PCIE_CAP2_10_BIT_TAG_REQ_SUPPORT
        uint32_t RSVD_18_19                  :2;      ///<BIT [19:18] rsvd_18_19
        uint32_t PCIE_CAP2_CFG_EXTND_FMT_SUPPORT :1;      ///<BIT [20] PCIE_CAP2_CFG_EXTND_FMT_SUPPORT
        uint32_t PCIE_CAP2_CFG_END2END_TLP_PRFX_SUPPORT :1;      ///<BIT [21] PCIE_CAP2_CFG_END2END_TLP_PRFX_SUPPORT
        uint32_t PCIE_CAP2_CFG_MAX_END2END_TLP_PRFXS :2;      ///<BIT [23:22] PCIE_CAP2_CFG_MAX_END2END_TLP_PRFXS
        uint32_t RSVD_24_26                  :3;      ///<BIT [26:24] rsvd_24_26
        uint32_t RSVDP_27                    :1;      ///<BIT [27] RSVDP_27
        uint32_t PCIE_CAP_DMWR_CPL_SUPP      :1;      ///<BIT [28] PCIE_CAP_DMWR_CPL_SUPP
        uint32_t PCIE_CAP_DMWR_LEN_SUPP      :2;      ///<BIT [30:29] PCIE_CAP_DMWR_LEN_SUPP
        uint32_t PCIE_CAP_FRS_SUPPORTED      :1;      ///<BIT [31] PCIE_CAP_FRS_SUPPORTED
    } b;
} VfDeviceCapabilities2_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_CPL_TIMEOUT_VALUE  :4;      ///<BIT [3:0] PCIE_CAP_CPL_TIMEOUT_VALUE
        uint32_t PCIE_CAP_CPL_TIMEOUT_DISABLE :1;      ///<BIT [4] PCIE_CAP_CPL_TIMEOUT_DISABLE
        uint32_t PCIE_CAP_ARI_FORWARD_SUPPORT_CS :1;      ///<BIT [5] PCIE_CAP_ARI_FORWARD_SUPPORT_CS
        uint32_t RSVD_6_9                    :4;      ///<BIT [9:6] rsvd_6_9
        uint32_t PCIE_CAP_LTR_EN             :1;      ///<BIT [10] PCIE_CAP_LTR_EN
        uint32_t RSVD_11_14                  :4;      ///<BIT [14:11] rsvd_11_14
        uint32_t PCIE_CTRL2_CFG_END2END_TLP_PFX_BLCK :1;      ///<BIT [15] PCIE_CTRL2_CFG_END2END_TLP_PFX_BLCK
        uint32_t RSVDP_16                    :16;     ///<BIT [31:16] RSVDP_16
    } b;
} VfDeviceControl2DeviceStatus2_t;

/// @brief 0x2C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :1;      ///<BIT [0] RSVDP_0
        uint32_t PCIE_CAP_SUPPORT_LINK_SPEED_VECTOR :7;      ///<BIT [7:1] PCIE_CAP_SUPPORT_LINK_SPEED_VECTOR
        uint32_t PCIE_CAP_CROSS_LINK_SUPPORT :1;      ///<BIT [8] PCIE_CAP_CROSS_LINK_SUPPORT
        uint32_t PCIE_CAP_LWR_SKP_OS_GEN_SUP :7;      ///<BIT [15:9] PCIE_CAP_LWR_SKP_OS_GEN_SUP
        uint32_t PCIE_CAP_LWR_SKP_OS_RCV_SUP :7;      ///<BIT [22:16] PCIE_CAP_LWR_SKP_OS_RCV_SUP
        uint32_t PCIE_CAP_RETIMER_PRE_DET_SUPPORT :1;      ///<BIT [23] PCIE_CAP_RETIMER_PRE_DET_SUPPORT
        uint32_t PCIE_CAP_TWO_RETIMERS_PRE_DET_SUPPORT :1;      ///<BIT [24] PCIE_CAP_TWO_RETIMERS_PRE_DET_SUPPORT
        uint32_t RSVDP_25                    :6;      ///<BIT [30:25] RSVDP_25
        uint32_t DRS_SUPPORTED               :1;      ///<BIT [31] DRS_SUPPORTED
    } b;
} VfLinkCapabilities2_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_TARGET_LINK_SPEED  :4;      ///<BIT [3:0] PCIE_CAP_TARGET_LINK_SPEED
        uint32_t PCIE_CAP_ENTER_COMPLIANCE   :1;      ///<BIT [4] PCIE_CAP_ENTER_COMPLIANCE
        uint32_t PCIE_CAP_HW_AUTO_SPEED_DISABLE :1;      ///<BIT [5] PCIE_CAP_HW_AUTO_SPEED_DISABLE
        uint32_t PCIE_CAP_SEL_DEEMPHASIS     :1;      ///<BIT [6] PCIE_CAP_SEL_DEEMPHASIS
        uint32_t PCIE_CAP_TX_MARGIN          :3;      ///<BIT [9:7] PCIE_CAP_TX_MARGIN
        uint32_t PCIE_CAP_ENTER_MODIFIED_COMPLIANCE :1;      ///<BIT [10] PCIE_CAP_ENTER_MODIFIED_COMPLIANCE
        uint32_t PCIE_CAP_COMPLIANCE_SOS     :1;      ///<BIT [11] PCIE_CAP_COMPLIANCE_SOS
        uint32_t PCIE_CAP_COMPLIANCE_PRESET  :4;      ///<BIT [15:12] PCIE_CAP_COMPLIANCE_PRESET
        uint32_t PCIE_CAP_CURR_DEEMPHASIS    :1;      ///<BIT [16] PCIE_CAP_CURR_DEEMPHASIS
        uint32_t PCIE_CAP_EQ_CPL             :1;      ///<BIT [17] PCIE_CAP_EQ_CPL
        uint32_t PCIE_CAP_EQ_CPL_P1          :1;      ///<BIT [18] PCIE_CAP_EQ_CPL_P1
        uint32_t PCIE_CAP_EQ_CPL_P2          :1;      ///<BIT [19] PCIE_CAP_EQ_CPL_P2
        uint32_t PCIE_CAP_EQ_CPL_P3          :1;      ///<BIT [20] PCIE_CAP_EQ_CPL_P3
        uint32_t PCIE_CAP_LINK_EQ_REQ        :1;      ///<BIT [21] PCIE_CAP_LINK_EQ_REQ
        uint32_t PCIE_CAP_RETIMER_PRE_DET    :1;      ///<BIT [22] PCIE_CAP_RETIMER_PRE_DET
        uint32_t PCIE_CAP_TWO_RETIMERS_PRE_DET :1;      ///<BIT [23] PCIE_CAP_TWO_RETIMERS_PRE_DET
        uint32_t PCIE_CAP_CROSSLINK_RESOLUTION :2;      ///<BIT [25:24] PCIE_CAP_CROSSLINK_RESOLUTION
        uint32_t RSVD_26                     :1;      ///<BIT [26] rsvd_26
        uint32_t RSVDP_27                    :1;      ///<BIT [27] RSVDP_27
        uint32_t DOWNSTREAM_COMPO_PRESENCE   :3;      ///<BIT [30:28] DOWNSTREAM_COMPO_PRESENCE
        uint32_t DRS_MESSAGE_RECEIVED        :1;      ///<BIT [31] DRS_MESSAGE_RECEIVED
    } b;
} VfLinkControl2LinkStatus2_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSIX_CAP_ID             :8;      ///<BIT [7:0] PCI_MSIX_CAP_ID
        uint32_t PCI_MSIX_CAP_NEXT_OFFSET    :8;      ///<BIT [15:8] PCI_MSIX_CAP_NEXT_OFFSET
        uint32_t PCI_MSIX_TABLE_SIZE         :11;     ///<BIT [26:16] PCI_MSIX_TABLE_SIZE
        uint32_t RSVDP_27                    :3;      ///<BIT [29:27] RSVDP_27
        uint32_t PCI_MSIX_FUNCTION_MASK      :1;      ///<BIT [30] PCI_MSIX_FUNCTION_MASK
        uint32_t PCI_MSIX_ENABLE             :1;      ///<BIT [31] PCI_MSIX_ENABLE
    } b;
} VfPciMsixCapIdNextCtrl_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSIX_BIR                :3;      ///<BIT [2:0] PCI_MSIX_BIR
        uint32_t PCI_MSIX_TABLE_OFFSET       :29;     ///<BIT [31:3] PCI_MSIX_TABLE_OFFSET
    } b;
} VfMsixTableOffset_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSIX_PBA_BIR            :3;      ///<BIT [2:0] PCI_MSIX_PBA_BIR
        uint32_t PCI_MSIX_PBA_OFFSET         :29;     ///<BIT [31:3] PCI_MSIX_PBA_OFFSET
    } b;
} VfMsixPbaOffset_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CAP_ID                      :16;     ///<BIT [15:0] CAP_ID
        uint32_t CAP_VERSION                 :4;      ///<BIT [19:16] CAP_VERSION
        uint32_t NEXT_OFFSET                 :12;     ///<BIT [31:20] NEXT_OFFSET
    } b;
} VfAerExtCapHdrOff_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :4;      ///<BIT [3:0] RSVDP_0
        uint32_t DL_PROTOCOL_ERR_STATUS      :1;      ///<BIT [4] DL_PROTOCOL_ERR_STATUS
        uint32_t SURPRISE_DOWN_ERR_STATUS    :1;      ///<BIT [5] SURPRISE_DOWN_ERR_STATUS
        uint32_t RSVDP_6                     :6;      ///<BIT [11:6] RSVDP_6
        uint32_t POIS_TLP_ERR_STATUS         :1;      ///<BIT [12] POIS_TLP_ERR_STATUS
        uint32_t FC_PROTOCOL_ERR_STATUS      :1;      ///<BIT [13] FC_PROTOCOL_ERR_STATUS
        uint32_t CMPLT_TIMEOUT_ERR_STATUS    :1;      ///<BIT [14] CMPLT_TIMEOUT_ERR_STATUS
        uint32_t CMPLT_ABORT_ERR_STATUS      :1;      ///<BIT [15] CMPLT_ABORT_ERR_STATUS
        uint32_t UNEXP_CMPLT_ERR_STATUS      :1;      ///<BIT [16] UNEXP_CMPLT_ERR_STATUS
        uint32_t REC_OVERFLOW_ERR_STATUS     :1;      ///<BIT [17] REC_OVERFLOW_ERR_STATUS
        uint32_t MALF_TLP_ERR_STATUS         :1;      ///<BIT [18] MALF_TLP_ERR_STATUS
        uint32_t ECRC_ERR_STATUS             :1;      ///<BIT [19] ECRC_ERR_STATUS
        uint32_t UNSUPPORTED_REQ_ERR_STATUS  :1;      ///<BIT [20] UNSUPPORTED_REQ_ERR_STATUS
        uint32_t RSVD_21                     :1;      ///<BIT [21] rsvd_21
        uint32_t INTERNAL_ERR_STATUS         :1;      ///<BIT [22] INTERNAL_ERR_STATUS
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t RSVD_24                     :1;      ///<BIT [24] rsvd_24
        uint32_t TLP_PRFX_BLOCKED_ERR_STATUS :1;      ///<BIT [25] TLP_PRFX_BLOCKED_ERR_STATUS
        uint32_t RSVD_26_27                  :2;      ///<BIT [27:26] rsvd_26_27
        uint32_t IDE_CHECK_FAILED_STATUS     :1;      ///<BIT [28] IDE_CHECK_FAILED_STATUS
        uint32_t MISROUTED_IDE_TLP_STATUS    :1;      ///<BIT [29] MISROUTED_IDE_TLP_STATUS
        uint32_t PCRC_CHECK_FAILED_STATUS    :1;      ///<BIT [30] PCRC_CHECK_FAILED_STATUS
        uint32_t RSVDP_31                    :1;      ///<BIT [31] RSVDP_31
    } b;
} VfUncorrErrStatusOff_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :4;      ///<BIT [3:0] RSVDP_0
        uint32_t DL_PROTOCOL_ERR_MASK        :1;      ///<BIT [4] DL_PROTOCOL_ERR_MASK
        uint32_t SURPRISE_DOWN_ERR_MASK      :1;      ///<BIT [5] SURPRISE_DOWN_ERR_MASK
        uint32_t RSVDP_6                     :6;      ///<BIT [11:6] RSVDP_6
        uint32_t POIS_TLP_ERR_MASK           :1;      ///<BIT [12] POIS_TLP_ERR_MASK
        uint32_t FC_PROTOCOL_ERR_MASK        :1;      ///<BIT [13] FC_PROTOCOL_ERR_MASK
        uint32_t CMPLT_TIMEOUT_ERR_MASK      :1;      ///<BIT [14] CMPLT_TIMEOUT_ERR_MASK
        uint32_t CMPLT_ABORT_ERR_MASK        :1;      ///<BIT [15] CMPLT_ABORT_ERR_MASK
        uint32_t UNEXP_CMPLT_ERR_MASK        :1;      ///<BIT [16] UNEXP_CMPLT_ERR_MASK
        uint32_t REC_OVERFLOW_ERR_MASK       :1;      ///<BIT [17] REC_OVERFLOW_ERR_MASK
        uint32_t MALF_TLP_ERR_MASK           :1;      ///<BIT [18] MALF_TLP_ERR_MASK
        uint32_t ECRC_ERR_MASK               :1;      ///<BIT [19] ECRC_ERR_MASK
        uint32_t UNSUPPORTED_REQ_ERR_MASK    :1;      ///<BIT [20] UNSUPPORTED_REQ_ERR_MASK
        uint32_t RSVD_21                     :1;      ///<BIT [21] rsvd_21
        uint32_t INTERNAL_ERR_MASK           :1;      ///<BIT [22] INTERNAL_ERR_MASK
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t ATOMIC_EGRESS_BLOCKED_ERR_MASK :1;      ///<BIT [24] ATOMIC_EGRESS_BLOCKED_ERR_MASK
        uint32_t TLP_PRFX_BLOCKED_ERR_MASK   :1;      ///<BIT [25] TLP_PRFX_BLOCKED_ERR_MASK
        uint32_t RSVDP_26                    :1;      ///<BIT [26] RSVDP_26
        uint32_t DMWR_EGRESS_BLOCKED_ERR_MASK :1;      ///<BIT [27] DMWR_EGRESS_BLOCKED_ERR_MASK
        uint32_t IDE_CHECK_FAILED_MASK       :1;      ///<BIT [28] IDE_CHECK_FAILED_MASK
        uint32_t MISROUTED_IDE_TLP_MASK      :1;      ///<BIT [29] MISROUTED_IDE_TLP_MASK
        uint32_t PCRC_CHECK_FAILED_MASK      :1;      ///<BIT [30] PCRC_CHECK_FAILED_MASK
        uint32_t RSVDP_31                    :1;      ///<BIT [31] RSVDP_31
    } b;
} VfUncorrErrMaskOff_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVDP_0                     :4;      ///<BIT [3:0] RSVDP_0
        uint32_t DL_PROTOCOL_ERR_SEVERITY    :1;      ///<BIT [4] DL_PROTOCOL_ERR_SEVERITY
        uint32_t SURPRISE_DOWN_ERR_SVRITY    :1;      ///<BIT [5] SURPRISE_DOWN_ERR_SVRITY
        uint32_t RSVDP_6                     :6;      ///<BIT [11:6] RSVDP_6
        uint32_t POIS_TLP_ERR_SEVERITY       :1;      ///<BIT [12] POIS_TLP_ERR_SEVERITY
        uint32_t FC_PROTOCOL_ERR_SEVERITY    :1;      ///<BIT [13] FC_PROTOCOL_ERR_SEVERITY
        uint32_t CMPLT_TIMEOUT_ERR_SEVERITY  :1;      ///<BIT [14] CMPLT_TIMEOUT_ERR_SEVERITY
        uint32_t CMPLT_ABORT_ERR_SEVERITY    :1;      ///<BIT [15] CMPLT_ABORT_ERR_SEVERITY
        uint32_t UNEXP_CMPLT_ERR_SEVERITY    :1;      ///<BIT [16] UNEXP_CMPLT_ERR_SEVERITY
        uint32_t REC_OVERFLOW_ERR_SEVERITY   :1;      ///<BIT [17] REC_OVERFLOW_ERR_SEVERITY
        uint32_t MALF_TLP_ERR_SEVERITY       :1;      ///<BIT [18] MALF_TLP_ERR_SEVERITY
        uint32_t ECRC_ERR_SEVERITY           :1;      ///<BIT [19] ECRC_ERR_SEVERITY
        uint32_t UNSUPPORTED_REQ_ERR_SEVERITY :1;      ///<BIT [20] UNSUPPORTED_REQ_ERR_SEVERITY
        uint32_t RSVD_21                     :1;      ///<BIT [21] rsvd_21
        uint32_t INTERNAL_ERR_SEVERITY       :1;      ///<BIT [22] INTERNAL_ERR_SEVERITY
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t RSVD_24                     :1;      ///<BIT [24] rsvd_24
        uint32_t TLP_PRFX_BLOCKED_ERR_SEVERITY :1;      ///<BIT [25] TLP_PRFX_BLOCKED_ERR_SEVERITY
        uint32_t RSVD_26                     :1;      ///<BIT [26] rsvd_26
        uint32_t DMWR_EGRESS_BLOCKED_ERR_SEVERITY :1;      ///<BIT [27] DMWR_EGRESS_BLOCKED_ERR_SEVERITY
        uint32_t IDE_CHECK_FAILED_SEV        :1;      ///<BIT [28] IDE_CHECK_FAILED_SEV
        uint32_t MISROUTED_IDE_TLP_SEV       :1;      ///<BIT [29] MISROUTED_IDE_TLP_SEV
        uint32_t PCRC_CHECK_FAILED_SEV       :1;      ///<BIT [30] PCRC_CHECK_FAILED_SEV
        uint32_t RSVDP_31                    :1;      ///<BIT [31] RSVDP_31
    } b;
} VfUncorrErrSevOff_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_ERR_STATUS               :1;      ///<BIT [0] RX_ERR_STATUS
        uint32_t RSVDP_1                     :5;      ///<BIT [5:1] RSVDP_1
        uint32_t BAD_TLP_STATUS              :1;      ///<BIT [6] BAD_TLP_STATUS
        uint32_t BAD_DLLP_STATUS             :1;      ///<BIT [7] BAD_DLLP_STATUS
        uint32_t REPLAY_NO_ROLEOVER_STATUS   :1;      ///<BIT [8] REPLAY_NO_ROLEOVER_STATUS
        uint32_t RSVDP_9                     :3;      ///<BIT [11:9] RSVDP_9
        uint32_t RPL_TIMER_TIMEOUT_STATUS    :1;      ///<BIT [12] RPL_TIMER_TIMEOUT_STATUS
        uint32_t ADVISORY_NON_FATAL_ERR_STATUS :1;      ///<BIT [13] ADVISORY_NON_FATAL_ERR_STATUS
        uint32_t CORRECTED_INT_ERR_STATUS    :1;      ///<BIT [14] CORRECTED_INT_ERR_STATUS
        uint32_t HEADER_LOG_OVERFLOW_STATUS  :1;      ///<BIT [15] HEADER_LOG_OVERFLOW_STATUS
        uint32_t RSVDP_16                    :16;     ///<BIT [31:16] RSVDP_16
    } b;
} VfCorrErrStatusOff_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_ERR_MASK                 :1;      ///<BIT [0] RX_ERR_MASK
        uint32_t RSVDP_1                     :5;      ///<BIT [5:1] RSVDP_1
        uint32_t BAD_TLP_MASK                :1;      ///<BIT [6] BAD_TLP_MASK
        uint32_t BAD_DLLP_MASK               :1;      ///<BIT [7] BAD_DLLP_MASK
        uint32_t REPLAY_NO_ROLEOVER_MASK     :1;      ///<BIT [8] REPLAY_NO_ROLEOVER_MASK
        uint32_t RSVDP_9                     :3;      ///<BIT [11:9] RSVDP_9
        uint32_t RPL_TIMER_TIMEOUT_MASK      :1;      ///<BIT [12] RPL_TIMER_TIMEOUT_MASK
        uint32_t ADVISORY_NON_FATAL_ERR_MASK :1;      ///<BIT [13] ADVISORY_NON_FATAL_ERR_MASK
        uint32_t CORRECTED_INT_ERR_MASK      :1;      ///<BIT [14] CORRECTED_INT_ERR_MASK
        uint32_t HEADER_LOG_OVERFLOW_MASK    :1;      ///<BIT [15] HEADER_LOG_OVERFLOW_MASK
        uint32_t RSVDP_16                    :16;     ///<BIT [31:16] RSVDP_16
    } b;
} VfCorrErrMaskOff_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FIRST_ERR_POINTER           :5;      ///<BIT [4:0] FIRST_ERR_POINTER
        uint32_t ECRC_GEN_CAP                :1;      ///<BIT [5] ECRC_GEN_CAP
        uint32_t ECRC_GEN_EN                 :1;      ///<BIT [6] ECRC_GEN_EN
        uint32_t ECRC_CHECK_CAP              :1;      ///<BIT [7] ECRC_CHECK_CAP
        uint32_t ECRC_CHECK_EN               :1;      ///<BIT [8] ECRC_CHECK_EN
        uint32_t MULTIPLE_HEADER_CAP         :1;      ///<BIT [9] MULTIPLE_HEADER_CAP
        uint32_t MULTIPLE_HEADER_EN          :1;      ///<BIT [10] MULTIPLE_HEADER_EN
        uint32_t TLP_PRFX_LOG_PRESENT        :1;      ///<BIT [11] TLP_PRFX_LOG_PRESENT
        uint32_t CTO_PRFX_HDR_LOG_CAP        :1;      ///<BIT [12] CTO_PRFX_HDR_LOG_CAP
        uint32_t RSVD_13_23                  :11;     ///<BIT [23:13] rsvd_13_23
        uint32_t RSVDP_24                    :8;      ///<BIT [31:24] RSVDP_24
    } b;
} VfAdvErrCapCtrlOff_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FIRST_DWORD_FIRST_BYTE      :8;      ///<BIT [7:0] FIRST_DWORD_FIRST_BYTE
        uint32_t FIRST_DWORD_SECOND_BYTE     :8;      ///<BIT [15:8] FIRST_DWORD_SECOND_BYTE
        uint32_t FIRST_DWORD_THIRD_BYTE      :8;      ///<BIT [23:16] FIRST_DWORD_THIRD_BYTE
        uint32_t FIRST_DWORD_FOURTH_BYTE     :8;      ///<BIT [31:24] FIRST_DWORD_FOURTH_BYTE
    } b;
} VfHdrLog0Off_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SECOND_DWORD_FIRST_BYTE     :8;      ///<BIT [7:0] SECOND_DWORD_FIRST_BYTE
        uint32_t SECOND_DWORD_SECOND_BYTE    :8;      ///<BIT [15:8] SECOND_DWORD_SECOND_BYTE
        uint32_t SECOND_DWORD_THIRD_BYTE     :8;      ///<BIT [23:16] SECOND_DWORD_THIRD_BYTE
        uint32_t SECOND_DWORD_FOURTH_BYTE    :8;      ///<BIT [31:24] SECOND_DWORD_FOURTH_BYTE
    } b;
} VfHdrLog1Off_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t THIRD_DWORD_FIRST_BYTE      :8;      ///<BIT [7:0] THIRD_DWORD_FIRST_BYTE
        uint32_t THIRD_DWORD_SECOND_BYTE     :8;      ///<BIT [15:8] THIRD_DWORD_SECOND_BYTE
        uint32_t THIRD_DWORD_THIRD_BYTE      :8;      ///<BIT [23:16] THIRD_DWORD_THIRD_BYTE
        uint32_t THIRD_DWORD_FOURTH_BYTE     :8;      ///<BIT [31:24] THIRD_DWORD_FOURTH_BYTE
    } b;
} VfHdrLog2Off_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FOURTH_DWORD_FIRST_BYTE     :8;      ///<BIT [7:0] FOURTH_DWORD_FIRST_BYTE
        uint32_t FOURTH_DWORD_SECOND_BYTE    :8;      ///<BIT [15:8] FOURTH_DWORD_SECOND_BYTE
        uint32_t FOURTH_DWORD_THIRD_BYTE     :8;      ///<BIT [23:16] FOURTH_DWORD_THIRD_BYTE
        uint32_t FOURTH_DWORD_FOURTH_BYTE    :8;      ///<BIT [31:24] FOURTH_DWORD_FOURTH_BYTE
    } b;
} VfHdrLog3Off_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_TLP_PFX_LOG_1_FIRST_BYTE :8;      ///<BIT [7:0] CFG_TLP_PFX_LOG_1_FIRST_BYTE
        uint32_t CFG_TLP_PFX_LOG_1_SECOND_BYTE :8;      ///<BIT [15:8] CFG_TLP_PFX_LOG_1_SECOND_BYTE
        uint32_t CFG_TLP_PFX_LOG_1_THIRD_BYTE :8;      ///<BIT [23:16] CFG_TLP_PFX_LOG_1_THIRD_BYTE
        uint32_t CFG_TLP_PFX_LOG_1_FOURTH_BYTE :8;      ///<BIT [31:24] CFG_TLP_PFX_LOG_1_FOURTH_BYTE
    } b;
} VfTlpPrefixLog1Off_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_TLP_PFX_LOG_2_FIRST_BYTE :8;      ///<BIT [7:0] CFG_TLP_PFX_LOG_2_FIRST_BYTE
        uint32_t CFG_TLP_PFX_LOG_2_SECOND_BYTE :8;      ///<BIT [15:8] CFG_TLP_PFX_LOG_2_SECOND_BYTE
        uint32_t CFG_TLP_PFX_LOG_2_THIRD_BYTE :8;      ///<BIT [23:16] CFG_TLP_PFX_LOG_2_THIRD_BYTE
        uint32_t CFG_TLP_PFX_LOG_2_FOURTH_BYTE :8;      ///<BIT [31:24] CFG_TLP_PFX_LOG_2_FOURTH_BYTE
    } b;
} VfTlpPrefixLog2Off_t;

/// @brief 0x40
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_TLP_PFX_LOG_3_FIRST_BYTE :8;      ///<BIT [7:0] CFG_TLP_PFX_LOG_3_FIRST_BYTE
        uint32_t CFG_TLP_PFX_LOG_3_SECOND_BYTE :8;      ///<BIT [15:8] CFG_TLP_PFX_LOG_3_SECOND_BYTE
        uint32_t CFG_TLP_PFX_LOG_3_THIRD_BYTE :8;      ///<BIT [23:16] CFG_TLP_PFX_LOG_3_THIRD_BYTE
        uint32_t CFG_TLP_PFX_LOG_3_FOURTH_BYTE :8;      ///<BIT [31:24] CFG_TLP_PFX_LOG_3_FOURTH_BYTE
    } b;
} VfTlpPrefixLog3Off_t;

/// @brief 0x44
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_TLP_PFX_LOG_4_FIRST_BYTE :8;      ///<BIT [7:0] CFG_TLP_PFX_LOG_4_FIRST_BYTE
        uint32_t CFG_TLP_PFX_LOG_4_SECOND_BYTE :8;      ///<BIT [15:8] CFG_TLP_PFX_LOG_4_SECOND_BYTE
        uint32_t CFG_TLP_PFX_LOG_4_THIRD_BYTE :8;      ///<BIT [23:16] CFG_TLP_PFX_LOG_4_THIRD_BYTE
        uint32_t CFG_TLP_PFX_LOG_4_FOURTH_BYTE :8;      ///<BIT [31:24] CFG_TLP_PFX_LOG_4_FOURTH_BYTE
    } b;
} VfTlpPrefixLog4Off_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ARI_PCIE_EXTENDED_CAP_ID    :16;     ///<BIT [15:0] ARI_PCIE_EXTENDED_CAP_ID
        uint32_t ARI_CAP_VERSION             :4;      ///<BIT [19:16] ARI_CAP_VERSION
        uint32_t ARI_NEXT_OFFSET             :12;     ///<BIT [31:20] ARI_NEXT_OFFSET
    } b;
} VfAriBase_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ARI_MFVC_FUN_GRP_CAP        :1;      ///<BIT [0] ARI_MFVC_FUN_GRP_CAP
        uint32_t ARI_ACS_FUN_GRP_CAP         :1;      ///<BIT [1] ARI_ACS_FUN_GRP_CAP
        uint32_t RSVDP_2                     :6;      ///<BIT [7:2] RSVDP_2
        uint32_t ARI_NEXT_FUN_NUM            :8;      ///<BIT [15:8] ARI_NEXT_FUN_NUM
        uint32_t ARI_MFVC_FUN_GRP_EN         :1;      ///<BIT [16] ARI_MFVC_FUN_GRP_EN
        uint32_t ARI_ACS_FUN_GRP_EN          :1;      ///<BIT [17] ARI_ACS_FUN_GRP_EN
        uint32_t RSVDP_18                    :2;      ///<BIT [19:18] RSVDP_18
        uint32_t ARI_FUN_GRP                 :3;      ///<BIT [22:20] ARI_FUN_GRP
        uint32_t RSVDP_23                    :9;      ///<BIT [31:23] RSVDP_23
    } b;
} VfCap_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_TYPE0_BAR0_ENABLED      :1;      ///<BIT [0] PCI_TYPE0_BAR0_ENABLED
        uint32_t PCI_TYPE0_BAR0_MASK         :31;     ///<BIT [31:1] PCI_TYPE0_BAR0_MASK
    } b;
} Bar0Mask_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_TYPE0_BAR1_ENABLED      :1;      ///<BIT [0] PCI_TYPE0_BAR1_ENABLED
        uint32_t PCI_TYPE0_BAR1_MASK         :31;     ///<BIT [31:1] PCI_TYPE0_BAR1_MASK
    } b;
} Bar1Mask_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_TYPE0_BAR2_ENABLED      :1;      ///<BIT [0] PCI_TYPE0_BAR2_ENABLED
        uint32_t PCI_TYPE0_BAR2_MASK         :31;     ///<BIT [31:1] PCI_TYPE0_BAR2_MASK
    } b;
} Bar2Mask_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_TYPE0_BAR3_ENABLED      :1;      ///<BIT [0] PCI_TYPE0_BAR3_ENABLED
        uint32_t PCI_TYPE0_BAR3_MASK         :31;     ///<BIT [31:1] PCI_TYPE0_BAR3_MASK
    } b;
} Bar3Mask_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_TYPE0_BAR4_ENABLED      :1;      ///<BIT [0] PCI_TYPE0_BAR4_ENABLED
        uint32_t PCI_TYPE0_BAR4_MASK         :31;     ///<BIT [31:1] PCI_TYPE0_BAR4_MASK
    } b;
} Bar4Mask_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_TYPE0_BAR5_ENABLED      :1;      ///<BIT [0] PCI_TYPE0_BAR5_ENABLED
        uint32_t PCI_TYPE0_BAR5_MASK         :31;     ///<BIT [31:1] PCI_TYPE0_BAR5_MASK
    } b;
} Bar5Mask_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ROM_BAR_ENABLED             :1;      ///<BIT [0] ROM_BAR_ENABLED
        uint32_t ROM_MASK                    :31;     ///<BIT [31:1] ROM_MASK
    } b;
} ExpRomBarMask_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_CAP_MAX_LINK_SPEED     :4;      ///<BIT [3:0] PCIE_CAP_MAX_LINK_SPEED
        uint32_t PCIE_CAP_MAX_LINK_WIDTH     :6;      ///<BIT [9:4] PCIE_CAP_MAX_LINK_WIDTH
        uint32_t PCIE_CAP_ACTIVE_STATE_LINK_PM_SUPPORT :2;      ///<BIT [11:10] PCIE_CAP_ACTIVE_STATE_LINK_PM_SUPPORT
        uint32_t SHADOW_PCIE_CAP_L0S_EXIT_LATENCY :3;      ///<BIT [14:12] SHADOW_PCIE_CAP_L0S_EXIT_LATENCY
        uint32_t SHADOW_PCIE_CAP_L1_EXIT_LATENCY :3;      ///<BIT [17:15] SHADOW_PCIE_CAP_L1_EXIT_LATENCY
        uint32_t PCIE_CAP_CLOCK_POWER_MAN    :1;      ///<BIT [18] PCIE_CAP_CLOCK_POWER_MAN
        uint32_t PCIE_CAP_SURPRISE_DOWN_ERR_REP_CAP :1;      ///<BIT [19] PCIE_CAP_SURPRISE_DOWN_ERR_REP_CAP
        uint32_t PCIE_CAP_DLL_ACTIVE_REP_CAP :1;      ///<BIT [20] PCIE_CAP_DLL_ACTIVE_REP_CAP
        uint32_t PCIE_CAP_LINK_BW_NOT_CAP    :1;      ///<BIT [21] PCIE_CAP_LINK_BW_NOT_CAP
        uint32_t PCIE_CAP_ASPM_OPT_COMPLIANCE :1;      ///<BIT [22] PCIE_CAP_ASPM_OPT_COMPLIANCE
        uint32_t RSVDP_23                    :1;      ///<BIT [23] RSVDP_23
        uint32_t PCIE_CAP_PORT_NUM           :8;      ///<BIT [31:24] PCIE_CAP_PORT_NUM
    } b;
} ShadowLinkCapabilities_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSIX_RESERVED0          :16;     ///<BIT [15:0] PCI_MSIX_RESERVED0
        uint32_t PCI_MSIX_TABLE_SIZE         :11;     ///<BIT [26:16] PCI_MSIX_TABLE_SIZE
        uint32_t PCI_MSIX_RESERVED1          :5;      ///<BIT [31:27] PCI_MSIX_RESERVED1
    } b;
} ShadowPciMsixCapIdNextCtrl_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSIX_BIR                :3;      ///<BIT [2:0] PCI_MSIX_BIR
        uint32_t PCI_MSIX_TABLE_OFFSET       :29;     ///<BIT [31:3] PCI_MSIX_TABLE_OFFSET
    } b;
} ShadowMsixTableOffset_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_MSIX_PBA_BIR            :3;      ///<BIT [2:0] PCI_MSIX_PBA_BIR
        uint32_t PCI_MSIX_PBA_OFFSET         :29;     ///<BIT [31:3] PCI_MSIX_PBA_OFFSET
    } b;
} ShadowMsixPbaOffset_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SHADOW_SRIOV_VF_OFFSET      :16;     ///<BIT [15:0] SHADOW_SRIOV_VF_OFFSET
        uint32_t SHADOW_SRIOV_VF_STRIDE      :16;     ///<BIT [31:16] SHADOW_SRIOV_VF_STRIDE
    } b;
} ShadowSriovVfOffsetPosition_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_SRIOV_BAR0_ENABLED      :1;      ///<BIT [0] PCI_SRIOV_BAR0_ENABLED
        uint32_t PCI_SRIOV_BAR0_MASK         :31;     ///<BIT [31:1] PCI_SRIOV_BAR0_MASK
    } b;
} SriovBar0Mask_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_SRIOV_BAR1_ENABLED      :1;      ///<BIT [0] PCI_SRIOV_BAR1_ENABLED
        uint32_t PCI_SRIOV_BAR1_MASK         :31;     ///<BIT [31:1] PCI_SRIOV_BAR1_MASK
    } b;
} SriovBar1Mask_t;

/// @brief 0x2C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_SRIOV_BAR2_ENABLED      :1;      ///<BIT [0] PCI_SRIOV_BAR2_ENABLED
        uint32_t PCI_SRIOV_BAR2_MASK         :31;     ///<BIT [31:1] PCI_SRIOV_BAR2_MASK
    } b;
} SriovBar2Mask_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_SRIOV_BAR3_ENABLED      :1;      ///<BIT [0] PCI_SRIOV_BAR3_ENABLED
        uint32_t PCI_SRIOV_BAR3_MASK         :31;     ///<BIT [31:1] PCI_SRIOV_BAR3_MASK
    } b;
} SriovBar3Mask_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_SRIOV_BAR4_ENABLED      :1;      ///<BIT [0] PCI_SRIOV_BAR4_ENABLED
        uint32_t PCI_SRIOV_BAR4_MASK         :31;     ///<BIT [31:1] PCI_SRIOV_BAR4_MASK
    } b;
} SriovBar4Mask_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCI_SRIOV_BAR5_ENABLED      :1;      ///<BIT [0] PCI_SRIOV_BAR5_ENABLED
        uint32_t PCI_SRIOV_BAR5_MASK         :31;     ///<BIT [31:1] PCI_SRIOV_BAR5_MASK
    } b;
} SriovBar5Mask_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TYPE                        :5;      ///<BIT [4:0] TYPE
        uint32_t TC                          :3;      ///<BIT [7:5] TC
        uint32_t TD                          :1;      ///<BIT [8] TD
        uint32_t ATTR                        :2;      ///<BIT [10:9] ATTR
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t INCREASE_REGION_SIZE        :1;      ///<BIT [13] INCREASE_REGION_SIZE
        uint32_t RSVD_14_19                  :6;      ///<BIT [19:14] rsvd_14_19
        uint32_t CTRL_1_FUNC_NUM             :1;      ///<BIT [20] CTRL_1_FUNC_NUM
        uint32_t RSVD_21_31                  :11;     ///<BIT [31:21] rsvd_21_31
    } b;
} IatuRegionCtrl1OffOutbound0_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSG_CODE                    :8;      ///<BIT [7:0] MSG_CODE
        uint32_t TAG                         :8;      ///<BIT [15:8] TAG
        uint32_t TAG_SUBSTITUTE_EN           :1;      ///<BIT [16] TAG_SUBSTITUTE_EN
        uint32_t MSB2BITS_TAG                :2;      ///<BIT [18:17] MSB2BITS_TAG
        uint32_t FUNC_BYPASS                 :1;      ///<BIT [19] FUNC_BYPASS
        uint32_t SNP                         :1;      ///<BIT [20] SNP
        uint32_t TLP_HEADER_FIELDS_BYPASS    :1;      ///<BIT [21] TLP_HEADER_FIELDS_BYPASS
        uint32_t INHIBIT_PAYLOAD             :1;      ///<BIT [22] INHIBIT_PAYLOAD
        uint32_t HEADER_SUBSTITUTE_EN        :1;      ///<BIT [23] HEADER_SUBSTITUTE_EN
        uint32_t RSVD_24_27                  :4;      ///<BIT [27:24] rsvd_24_27
        uint32_t CFG_SHIFT_MODE              :1;      ///<BIT [28] CFG_SHIFT_MODE
        uint32_t INVERT_MODE                 :1;      ///<BIT [29] INVERT_MODE
        uint32_t RSVD_30                     :1;      ///<BIT [30] rsvd_30
        uint32_t REGION_EN                   :1;      ///<BIT [31] REGION_EN
    } b;
} IatuRegionCtrl2OffOutbound0_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_BASE_HW                 :12;     ///<BIT [11:0] LWR_BASE_HW
        uint32_t LWR_BASE_RW                 :20;     ///<BIT [31:12] LWR_BASE_RW
    } b;
} IatuLwrBaseAddrOffOutbound0_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CBUF_INCR                   :4;      ///<BIT [3:0] CBUF_INCR
        uint32_t LIMIT_ADDR_HW               :8;      ///<BIT [11:4] LIMIT_ADDR_HW
        uint32_t LIMIT_ADDR_RW               :20;     ///<BIT [31:12] LIMIT_ADDR_RW
    } b;
} IatuLimitAddrOffOutbound0_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VF_NUMBER                   :6;      ///<BIT [5:0] VF_NUMBER
        uint32_t RSVDP_VF_NUMBER             :25;     ///<BIT [30:6] RSVDP_VF_NUMBER
        uint32_t VF_ACTIVE                   :1;      ///<BIT [31] VF_ACTIVE
    } b;
} IatuRegionCtrl3OffOutbound0_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPPR_LIMIT_ADDR_RW          :5;      ///<BIT [4:0] UPPR_LIMIT_ADDR_RW
        uint32_t UPPR_LIMIT_ADDR_HW          :27;     ///<BIT [31:5] UPPR_LIMIT_ADDR_HW
    } b;
} IatuUpprLimitAddrOffOutbound0_t;

/// @brief 0x100
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TYPE                        :5;      ///<BIT [4:0] TYPE
        uint32_t TC                          :3;      ///<BIT [7:5] TC
        uint32_t TD                          :1;      ///<BIT [8] TD
        uint32_t ATTR                        :2;      ///<BIT [10:9] ATTR
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t INCREASE_REGION_SIZE        :1;      ///<BIT [13] INCREASE_REGION_SIZE
        uint32_t RSVD_14_19                  :6;      ///<BIT [19:14] rsvd_14_19
        uint32_t CTRL_1_FUNC_NUM             :1;      ///<BIT [20] CTRL_1_FUNC_NUM
        uint32_t RSVD_21_31                  :11;     ///<BIT [31:21] rsvd_21_31
    } b;
} IatuRegionCtrl1OffInbound0_t;

/// @brief 0x104
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSG_CODE                    :8;      ///<BIT [7:0] MSG_CODE
        uint32_t BAR_NUM                     :3;      ///<BIT [10:8] BAR_NUM
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t MSG_TYPE_MATCH_MODE         :1;      ///<BIT [13] MSG_TYPE_MATCH_MODE
        uint32_t TC_MATCH_EN                 :1;      ///<BIT [14] TC_MATCH_EN
        uint32_t TD_MATCH_EN                 :1;      ///<BIT [15] TD_MATCH_EN
        uint32_t ATTR_MATCH_EN               :1;      ///<BIT [16] ATTR_MATCH_EN
        uint32_t RSVD_17_18                  :2;      ///<BIT [18:17] rsvd_17_18
        uint32_t FUNC_NUM_MATCH_EN           :1;      ///<BIT [19] FUNC_NUM_MATCH_EN
        uint32_t VF_MATCH_EN                 :1;      ///<BIT [20] VF_MATCH_EN
        uint32_t MSG_CODE_MATCH_EN           :1;      ///<BIT [21] MSG_CODE_MATCH_EN
        uint32_t RSVD_22                     :1;      ///<BIT [22] rsvd_22
        uint32_t SINGLE_ADDR_LOC_TRANS_EN    :1;      ///<BIT [23] SINGLE_ADDR_LOC_TRANS_EN
        uint32_t RESPONSE_CODE               :2;      ///<BIT [25:24] RESPONSE_CODE
        uint32_t VFBAR_MATCH_MODE_EN         :1;      ///<BIT [26] VFBAR_MATCH_MODE_EN
        uint32_t FUZZY_TYPE_MATCH_CODE       :1;      ///<BIT [27] FUZZY_TYPE_MATCH_CODE
        uint32_t CFG_SHIFT_MODE              :1;      ///<BIT [28] CFG_SHIFT_MODE
        uint32_t INVERT_MODE                 :1;      ///<BIT [29] INVERT_MODE
        uint32_t MATCH_MODE                  :1;      ///<BIT [30] MATCH_MODE
        uint32_t REGION_EN                   :1;      ///<BIT [31] REGION_EN
    } b;
} IatuRegionCtrl2OffInbound0_t;

/// @brief 0x108
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_BASE_HW                 :12;     ///<BIT [11:0] LWR_BASE_HW
        uint32_t LWR_BASE_RW                 :20;     ///<BIT [31:12] LWR_BASE_RW
    } b;
} IatuLwrBaseAddrOffInbound0_t;

/// @brief 0x110
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CBUF_INCR                   :4;      ///<BIT [3:0] CBUF_INCR
        uint32_t LIMIT_ADDR_HW               :8;      ///<BIT [11:4] LIMIT_ADDR_HW
        uint32_t LIMIT_ADDR_RW               :20;     ///<BIT [31:12] LIMIT_ADDR_RW
    } b;
} IatuLimitAddrOffInbound0_t;

/// @brief 0x114
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_TARGET_HW               :12;     ///<BIT [11:0] LWR_TARGET_HW
        uint32_t LWR_TARGET_RW               :20;     ///<BIT [31:12] LWR_TARGET_RW
    } b;
} IatuLwrTargetAddrOffInbound0_t;

/// @brief 0x11C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VF_NUMBER                   :6;      ///<BIT [5:0] VF_NUMBER
        uint32_t RSVDP_VF_NUMBER             :25;     ///<BIT [30:6] RSVDP_VF_NUMBER
        uint32_t VF_ACTIVE                   :1;      ///<BIT [31] VF_ACTIVE
    } b;
} IatuRegionCtrl3OffInbound0_t;

/// @brief 0x120
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPPR_LIMIT_ADDR_RW          :5;      ///<BIT [4:0] UPPR_LIMIT_ADDR_RW
        uint32_t UPPR_LIMIT_ADDR_HW          :27;     ///<BIT [31:5] UPPR_LIMIT_ADDR_HW
    } b;
} IatuUpprLimitAddrOffInbound0_t;

/// @brief 0x300
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TYPE                        :5;      ///<BIT [4:0] TYPE
        uint32_t TC                          :3;      ///<BIT [7:5] TC
        uint32_t TD                          :1;      ///<BIT [8] TD
        uint32_t ATTR                        :2;      ///<BIT [10:9] ATTR
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t INCREASE_REGION_SIZE        :1;      ///<BIT [13] INCREASE_REGION_SIZE
        uint32_t RSVD_14_19                  :6;      ///<BIT [19:14] rsvd_14_19
        uint32_t CTRL_1_FUNC_NUM             :1;      ///<BIT [20] CTRL_1_FUNC_NUM
        uint32_t RSVD_21_31                  :11;     ///<BIT [31:21] rsvd_21_31
    } b;
} IatuRegionCtrl1OffInbound1_t;

/// @brief 0x304
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSG_CODE                    :8;      ///<BIT [7:0] MSG_CODE
        uint32_t BAR_NUM                     :3;      ///<BIT [10:8] BAR_NUM
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t MSG_TYPE_MATCH_MODE         :1;      ///<BIT [13] MSG_TYPE_MATCH_MODE
        uint32_t TC_MATCH_EN                 :1;      ///<BIT [14] TC_MATCH_EN
        uint32_t TD_MATCH_EN                 :1;      ///<BIT [15] TD_MATCH_EN
        uint32_t ATTR_MATCH_EN               :1;      ///<BIT [16] ATTR_MATCH_EN
        uint32_t RSVD_17_18                  :2;      ///<BIT [18:17] rsvd_17_18
        uint32_t FUNC_NUM_MATCH_EN           :1;      ///<BIT [19] FUNC_NUM_MATCH_EN
        uint32_t VF_MATCH_EN                 :1;      ///<BIT [20] VF_MATCH_EN
        uint32_t MSG_CODE_MATCH_EN           :1;      ///<BIT [21] MSG_CODE_MATCH_EN
        uint32_t RSVD_22                     :1;      ///<BIT [22] rsvd_22
        uint32_t SINGLE_ADDR_LOC_TRANS_EN    :1;      ///<BIT [23] SINGLE_ADDR_LOC_TRANS_EN
        uint32_t RESPONSE_CODE               :2;      ///<BIT [25:24] RESPONSE_CODE
        uint32_t VFBAR_MATCH_MODE_EN         :1;      ///<BIT [26] VFBAR_MATCH_MODE_EN
        uint32_t FUZZY_TYPE_MATCH_CODE       :1;      ///<BIT [27] FUZZY_TYPE_MATCH_CODE
        uint32_t CFG_SHIFT_MODE              :1;      ///<BIT [28] CFG_SHIFT_MODE
        uint32_t INVERT_MODE                 :1;      ///<BIT [29] INVERT_MODE
        uint32_t MATCH_MODE                  :1;      ///<BIT [30] MATCH_MODE
        uint32_t REGION_EN                   :1;      ///<BIT [31] REGION_EN
    } b;
} IatuRegionCtrl2OffInbound1_t;

/// @brief 0x308
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_BASE_HW                 :12;     ///<BIT [11:0] LWR_BASE_HW
        uint32_t LWR_BASE_RW                 :20;     ///<BIT [31:12] LWR_BASE_RW
    } b;
} IatuLwrBaseAddrOffInbound1_t;

/// @brief 0x310
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CBUF_INCR                   :4;      ///<BIT [3:0] CBUF_INCR
        uint32_t LIMIT_ADDR_HW               :8;      ///<BIT [11:4] LIMIT_ADDR_HW
        uint32_t LIMIT_ADDR_RW               :20;     ///<BIT [31:12] LIMIT_ADDR_RW
    } b;
} IatuLimitAddrOffInbound1_t;

/// @brief 0x314
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_TARGET_HW               :12;     ///<BIT [11:0] LWR_TARGET_HW
        uint32_t LWR_TARGET_RW               :20;     ///<BIT [31:12] LWR_TARGET_RW
    } b;
} IatuLwrTargetAddrOffInbound1_t;

/// @brief 0x31C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VF_NUMBER                   :6;      ///<BIT [5:0] VF_NUMBER
        uint32_t RSVDP_VF_NUMBER             :25;     ///<BIT [30:6] RSVDP_VF_NUMBER
        uint32_t VF_ACTIVE                   :1;      ///<BIT [31] VF_ACTIVE
    } b;
} IatuRegionCtrl3OffInbound1_t;

/// @brief 0x320
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPPR_LIMIT_ADDR_RW          :5;      ///<BIT [4:0] UPPR_LIMIT_ADDR_RW
        uint32_t UPPR_LIMIT_ADDR_HW          :27;     ///<BIT [31:5] UPPR_LIMIT_ADDR_HW
    } b;
} IatuUpprLimitAddrOffInbound1_t;

/// @brief 0x500
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TYPE                        :5;      ///<BIT [4:0] TYPE
        uint32_t TC                          :3;      ///<BIT [7:5] TC
        uint32_t TD                          :1;      ///<BIT [8] TD
        uint32_t ATTR                        :2;      ///<BIT [10:9] ATTR
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t INCREASE_REGION_SIZE        :1;      ///<BIT [13] INCREASE_REGION_SIZE
        uint32_t RSVD_14_19                  :6;      ///<BIT [19:14] rsvd_14_19
        uint32_t CTRL_1_FUNC_NUM             :1;      ///<BIT [20] CTRL_1_FUNC_NUM
        uint32_t RSVD_21_31                  :11;     ///<BIT [31:21] rsvd_21_31
    } b;
} IatuRegionCtrl1OffInbound2_t;

/// @brief 0x504
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSG_CODE                    :8;      ///<BIT [7:0] MSG_CODE
        uint32_t BAR_NUM                     :3;      ///<BIT [10:8] BAR_NUM
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t MSG_TYPE_MATCH_MODE         :1;      ///<BIT [13] MSG_TYPE_MATCH_MODE
        uint32_t TC_MATCH_EN                 :1;      ///<BIT [14] TC_MATCH_EN
        uint32_t TD_MATCH_EN                 :1;      ///<BIT [15] TD_MATCH_EN
        uint32_t ATTR_MATCH_EN               :1;      ///<BIT [16] ATTR_MATCH_EN
        uint32_t RSVD_17_18                  :2;      ///<BIT [18:17] rsvd_17_18
        uint32_t FUNC_NUM_MATCH_EN           :1;      ///<BIT [19] FUNC_NUM_MATCH_EN
        uint32_t VF_MATCH_EN                 :1;      ///<BIT [20] VF_MATCH_EN
        uint32_t MSG_CODE_MATCH_EN           :1;      ///<BIT [21] MSG_CODE_MATCH_EN
        uint32_t RSVD_22                     :1;      ///<BIT [22] rsvd_22
        uint32_t SINGLE_ADDR_LOC_TRANS_EN    :1;      ///<BIT [23] SINGLE_ADDR_LOC_TRANS_EN
        uint32_t RESPONSE_CODE               :2;      ///<BIT [25:24] RESPONSE_CODE
        uint32_t VFBAR_MATCH_MODE_EN         :1;      ///<BIT [26] VFBAR_MATCH_MODE_EN
        uint32_t FUZZY_TYPE_MATCH_CODE       :1;      ///<BIT [27] FUZZY_TYPE_MATCH_CODE
        uint32_t CFG_SHIFT_MODE              :1;      ///<BIT [28] CFG_SHIFT_MODE
        uint32_t INVERT_MODE                 :1;      ///<BIT [29] INVERT_MODE
        uint32_t MATCH_MODE                  :1;      ///<BIT [30] MATCH_MODE
        uint32_t REGION_EN                   :1;      ///<BIT [31] REGION_EN
    } b;
} IatuRegionCtrl2OffInbound2_t;

/// @brief 0x508
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_BASE_HW                 :12;     ///<BIT [11:0] LWR_BASE_HW
        uint32_t LWR_BASE_RW                 :20;     ///<BIT [31:12] LWR_BASE_RW
    } b;
} IatuLwrBaseAddrOffInbound2_t;

/// @brief 0x510
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CBUF_INCR                   :4;      ///<BIT [3:0] CBUF_INCR
        uint32_t LIMIT_ADDR_HW               :8;      ///<BIT [11:4] LIMIT_ADDR_HW
        uint32_t LIMIT_ADDR_RW               :20;     ///<BIT [31:12] LIMIT_ADDR_RW
    } b;
} IatuLimitAddrOffInbound2_t;

/// @brief 0x514
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_TARGET_HW               :12;     ///<BIT [11:0] LWR_TARGET_HW
        uint32_t LWR_TARGET_RW               :20;     ///<BIT [31:12] LWR_TARGET_RW
    } b;
} IatuLwrTargetAddrOffInbound2_t;

/// @brief 0x51C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VF_NUMBER                   :6;      ///<BIT [5:0] VF_NUMBER
        uint32_t RSVDP_VF_NUMBER             :25;     ///<BIT [30:6] RSVDP_VF_NUMBER
        uint32_t VF_ACTIVE                   :1;      ///<BIT [31] VF_ACTIVE
    } b;
} IatuRegionCtrl3OffInbound2_t;

/// @brief 0x520
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPPR_LIMIT_ADDR_RW          :5;      ///<BIT [4:0] UPPR_LIMIT_ADDR_RW
        uint32_t UPPR_LIMIT_ADDR_HW          :27;     ///<BIT [31:5] UPPR_LIMIT_ADDR_HW
    } b;
} IatuUpprLimitAddrOffInbound2_t;

/// @brief 0x700
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TYPE                        :5;      ///<BIT [4:0] TYPE
        uint32_t TC                          :3;      ///<BIT [7:5] TC
        uint32_t TD                          :1;      ///<BIT [8] TD
        uint32_t ATTR                        :2;      ///<BIT [10:9] ATTR
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t INCREASE_REGION_SIZE        :1;      ///<BIT [13] INCREASE_REGION_SIZE
        uint32_t RSVD_14_19                  :6;      ///<BIT [19:14] rsvd_14_19
        uint32_t CTRL_1_FUNC_NUM             :1;      ///<BIT [20] CTRL_1_FUNC_NUM
        uint32_t RSVD_21_31                  :11;     ///<BIT [31:21] rsvd_21_31
    } b;
} IatuRegionCtrl1OffInbound3_t;

/// @brief 0x704
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSG_CODE                    :8;      ///<BIT [7:0] MSG_CODE
        uint32_t BAR_NUM                     :3;      ///<BIT [10:8] BAR_NUM
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t MSG_TYPE_MATCH_MODE         :1;      ///<BIT [13] MSG_TYPE_MATCH_MODE
        uint32_t TC_MATCH_EN                 :1;      ///<BIT [14] TC_MATCH_EN
        uint32_t TD_MATCH_EN                 :1;      ///<BIT [15] TD_MATCH_EN
        uint32_t ATTR_MATCH_EN               :1;      ///<BIT [16] ATTR_MATCH_EN
        uint32_t RSVD_17_18                  :2;      ///<BIT [18:17] rsvd_17_18
        uint32_t FUNC_NUM_MATCH_EN           :1;      ///<BIT [19] FUNC_NUM_MATCH_EN
        uint32_t VF_MATCH_EN                 :1;      ///<BIT [20] VF_MATCH_EN
        uint32_t MSG_CODE_MATCH_EN           :1;      ///<BIT [21] MSG_CODE_MATCH_EN
        uint32_t RSVD_22                     :1;      ///<BIT [22] rsvd_22
        uint32_t SINGLE_ADDR_LOC_TRANS_EN    :1;      ///<BIT [23] SINGLE_ADDR_LOC_TRANS_EN
        uint32_t RESPONSE_CODE               :2;      ///<BIT [25:24] RESPONSE_CODE
        uint32_t VFBAR_MATCH_MODE_EN         :1;      ///<BIT [26] VFBAR_MATCH_MODE_EN
        uint32_t FUZZY_TYPE_MATCH_CODE       :1;      ///<BIT [27] FUZZY_TYPE_MATCH_CODE
        uint32_t CFG_SHIFT_MODE              :1;      ///<BIT [28] CFG_SHIFT_MODE
        uint32_t INVERT_MODE                 :1;      ///<BIT [29] INVERT_MODE
        uint32_t MATCH_MODE                  :1;      ///<BIT [30] MATCH_MODE
        uint32_t REGION_EN                   :1;      ///<BIT [31] REGION_EN
    } b;
} IatuRegionCtrl2OffInbound3_t;

/// @brief 0x708
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_BASE_HW                 :12;     ///<BIT [11:0] LWR_BASE_HW
        uint32_t LWR_BASE_RW                 :20;     ///<BIT [31:12] LWR_BASE_RW
    } b;
} IatuLwrBaseAddrOffInbound3_t;

/// @brief 0x710
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CBUF_INCR                   :4;      ///<BIT [3:0] CBUF_INCR
        uint32_t LIMIT_ADDR_HW               :8;      ///<BIT [11:4] LIMIT_ADDR_HW
        uint32_t LIMIT_ADDR_RW               :20;     ///<BIT [31:12] LIMIT_ADDR_RW
    } b;
} IatuLimitAddrOffInbound3_t;

/// @brief 0x714
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_TARGET_HW               :12;     ///<BIT [11:0] LWR_TARGET_HW
        uint32_t LWR_TARGET_RW               :20;     ///<BIT [31:12] LWR_TARGET_RW
    } b;
} IatuLwrTargetAddrOffInbound3_t;

/// @brief 0x71C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VF_NUMBER                   :6;      ///<BIT [5:0] VF_NUMBER
        uint32_t RSVDP_VF_NUMBER             :25;     ///<BIT [30:6] RSVDP_VF_NUMBER
        uint32_t VF_ACTIVE                   :1;      ///<BIT [31] VF_ACTIVE
    } b;
} IatuRegionCtrl3OffInbound3_t;

/// @brief 0x720
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPPR_LIMIT_ADDR_RW          :5;      ///<BIT [4:0] UPPR_LIMIT_ADDR_RW
        uint32_t UPPR_LIMIT_ADDR_HW          :27;     ///<BIT [31:5] UPPR_LIMIT_ADDR_HW
    } b;
} IatuUpprLimitAddrOffInbound3_t;

/// @brief 0x900
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TYPE                        :5;      ///<BIT [4:0] TYPE
        uint32_t TC                          :3;      ///<BIT [7:5] TC
        uint32_t TD                          :1;      ///<BIT [8] TD
        uint32_t ATTR                        :2;      ///<BIT [10:9] ATTR
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t INCREASE_REGION_SIZE        :1;      ///<BIT [13] INCREASE_REGION_SIZE
        uint32_t RSVD_14_19                  :6;      ///<BIT [19:14] rsvd_14_19
        uint32_t CTRL_1_FUNC_NUM             :1;      ///<BIT [20] CTRL_1_FUNC_NUM
        uint32_t RSVD_21_31                  :11;     ///<BIT [31:21] rsvd_21_31
    } b;
} IatuRegionCtrl1OffInbound4_t;

/// @brief 0x904
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSG_CODE                    :8;      ///<BIT [7:0] MSG_CODE
        uint32_t BAR_NUM                     :3;      ///<BIT [10:8] BAR_NUM
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t MSG_TYPE_MATCH_MODE         :1;      ///<BIT [13] MSG_TYPE_MATCH_MODE
        uint32_t TC_MATCH_EN                 :1;      ///<BIT [14] TC_MATCH_EN
        uint32_t TD_MATCH_EN                 :1;      ///<BIT [15] TD_MATCH_EN
        uint32_t ATTR_MATCH_EN               :1;      ///<BIT [16] ATTR_MATCH_EN
        uint32_t RSVD_17_18                  :2;      ///<BIT [18:17] rsvd_17_18
        uint32_t FUNC_NUM_MATCH_EN           :1;      ///<BIT [19] FUNC_NUM_MATCH_EN
        uint32_t VF_MATCH_EN                 :1;      ///<BIT [20] VF_MATCH_EN
        uint32_t MSG_CODE_MATCH_EN           :1;      ///<BIT [21] MSG_CODE_MATCH_EN
        uint32_t RSVD_22                     :1;      ///<BIT [22] rsvd_22
        uint32_t SINGLE_ADDR_LOC_TRANS_EN    :1;      ///<BIT [23] SINGLE_ADDR_LOC_TRANS_EN
        uint32_t RESPONSE_CODE               :2;      ///<BIT [25:24] RESPONSE_CODE
        uint32_t VFBAR_MATCH_MODE_EN         :1;      ///<BIT [26] VFBAR_MATCH_MODE_EN
        uint32_t FUZZY_TYPE_MATCH_CODE       :1;      ///<BIT [27] FUZZY_TYPE_MATCH_CODE
        uint32_t CFG_SHIFT_MODE              :1;      ///<BIT [28] CFG_SHIFT_MODE
        uint32_t INVERT_MODE                 :1;      ///<BIT [29] INVERT_MODE
        uint32_t MATCH_MODE                  :1;      ///<BIT [30] MATCH_MODE
        uint32_t REGION_EN                   :1;      ///<BIT [31] REGION_EN
    } b;
} IatuRegionCtrl2OffInbound4_t;

/// @brief 0x908
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_BASE_HW                 :12;     ///<BIT [11:0] LWR_BASE_HW
        uint32_t LWR_BASE_RW                 :20;     ///<BIT [31:12] LWR_BASE_RW
    } b;
} IatuLwrBaseAddrOffInbound4_t;

/// @brief 0x910
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CBUF_INCR                   :4;      ///<BIT [3:0] CBUF_INCR
        uint32_t LIMIT_ADDR_HW               :8;      ///<BIT [11:4] LIMIT_ADDR_HW
        uint32_t LIMIT_ADDR_RW               :20;     ///<BIT [31:12] LIMIT_ADDR_RW
    } b;
} IatuLimitAddrOffInbound4_t;

/// @brief 0x914
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_TARGET_HW               :12;     ///<BIT [11:0] LWR_TARGET_HW
        uint32_t LWR_TARGET_RW               :20;     ///<BIT [31:12] LWR_TARGET_RW
    } b;
} IatuLwrTargetAddrOffInbound4_t;

/// @brief 0x91C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VF_NUMBER                   :6;      ///<BIT [5:0] VF_NUMBER
        uint32_t RSVDP_VF_NUMBER             :25;     ///<BIT [30:6] RSVDP_VF_NUMBER
        uint32_t VF_ACTIVE                   :1;      ///<BIT [31] VF_ACTIVE
    } b;
} IatuRegionCtrl3OffInbound4_t;

/// @brief 0x920
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPPR_LIMIT_ADDR_RW          :5;      ///<BIT [4:0] UPPR_LIMIT_ADDR_RW
        uint32_t UPPR_LIMIT_ADDR_HW          :27;     ///<BIT [31:5] UPPR_LIMIT_ADDR_HW
    } b;
} IatuUpprLimitAddrOffInbound4_t;

/// @brief 0xB00
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TYPE                        :5;      ///<BIT [4:0] TYPE
        uint32_t TC                          :3;      ///<BIT [7:5] TC
        uint32_t TD                          :1;      ///<BIT [8] TD
        uint32_t ATTR                        :2;      ///<BIT [10:9] ATTR
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t INCREASE_REGION_SIZE        :1;      ///<BIT [13] INCREASE_REGION_SIZE
        uint32_t RSVD_14_19                  :6;      ///<BIT [19:14] rsvd_14_19
        uint32_t CTRL_1_FUNC_NUM             :1;      ///<BIT [20] CTRL_1_FUNC_NUM
        uint32_t RSVD_21_31                  :11;     ///<BIT [31:21] rsvd_21_31
    } b;
} IatuRegionCtrl1OffInbound5_t;

/// @brief 0xB04
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSG_CODE                    :8;      ///<BIT [7:0] MSG_CODE
        uint32_t BAR_NUM                     :3;      ///<BIT [10:8] BAR_NUM
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t MSG_TYPE_MATCH_MODE         :1;      ///<BIT [13] MSG_TYPE_MATCH_MODE
        uint32_t TC_MATCH_EN                 :1;      ///<BIT [14] TC_MATCH_EN
        uint32_t TD_MATCH_EN                 :1;      ///<BIT [15] TD_MATCH_EN
        uint32_t ATTR_MATCH_EN               :1;      ///<BIT [16] ATTR_MATCH_EN
        uint32_t RSVD_17_18                  :2;      ///<BIT [18:17] rsvd_17_18
        uint32_t FUNC_NUM_MATCH_EN           :1;      ///<BIT [19] FUNC_NUM_MATCH_EN
        uint32_t VF_MATCH_EN                 :1;      ///<BIT [20] VF_MATCH_EN
        uint32_t MSG_CODE_MATCH_EN           :1;      ///<BIT [21] MSG_CODE_MATCH_EN
        uint32_t RSVD_22                     :1;      ///<BIT [22] rsvd_22
        uint32_t SINGLE_ADDR_LOC_TRANS_EN    :1;      ///<BIT [23] SINGLE_ADDR_LOC_TRANS_EN
        uint32_t RESPONSE_CODE               :2;      ///<BIT [25:24] RESPONSE_CODE
        uint32_t VFBAR_MATCH_MODE_EN         :1;      ///<BIT [26] VFBAR_MATCH_MODE_EN
        uint32_t FUZZY_TYPE_MATCH_CODE       :1;      ///<BIT [27] FUZZY_TYPE_MATCH_CODE
        uint32_t CFG_SHIFT_MODE              :1;      ///<BIT [28] CFG_SHIFT_MODE
        uint32_t INVERT_MODE                 :1;      ///<BIT [29] INVERT_MODE
        uint32_t MATCH_MODE                  :1;      ///<BIT [30] MATCH_MODE
        uint32_t REGION_EN                   :1;      ///<BIT [31] REGION_EN
    } b;
} IatuRegionCtrl2OffInbound5_t;

/// @brief 0xB08
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_BASE_HW                 :12;     ///<BIT [11:0] LWR_BASE_HW
        uint32_t LWR_BASE_RW                 :20;     ///<BIT [31:12] LWR_BASE_RW
    } b;
} IatuLwrBaseAddrOffInbound5_t;

/// @brief 0xB10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CBUF_INCR                   :4;      ///<BIT [3:0] CBUF_INCR
        uint32_t LIMIT_ADDR_HW               :8;      ///<BIT [11:4] LIMIT_ADDR_HW
        uint32_t LIMIT_ADDR_RW               :20;     ///<BIT [31:12] LIMIT_ADDR_RW
    } b;
} IatuLimitAddrOffInbound5_t;

/// @brief 0xB14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_TARGET_HW               :12;     ///<BIT [11:0] LWR_TARGET_HW
        uint32_t LWR_TARGET_RW               :20;     ///<BIT [31:12] LWR_TARGET_RW
    } b;
} IatuLwrTargetAddrOffInbound5_t;

/// @brief 0xB1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VF_NUMBER                   :6;      ///<BIT [5:0] VF_NUMBER
        uint32_t RSVDP_VF_NUMBER             :25;     ///<BIT [30:6] RSVDP_VF_NUMBER
        uint32_t VF_ACTIVE                   :1;      ///<BIT [31] VF_ACTIVE
    } b;
} IatuRegionCtrl3OffInbound5_t;

/// @brief 0xB20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPPR_LIMIT_ADDR_RW          :5;      ///<BIT [4:0] UPPR_LIMIT_ADDR_RW
        uint32_t UPPR_LIMIT_ADDR_HW          :27;     ///<BIT [31:5] UPPR_LIMIT_ADDR_HW
    } b;
} IatuUpprLimitAddrOffInbound5_t;

/// @brief 0xD00
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TYPE                        :5;      ///<BIT [4:0] TYPE
        uint32_t TC                          :3;      ///<BIT [7:5] TC
        uint32_t TD                          :1;      ///<BIT [8] TD
        uint32_t ATTR                        :2;      ///<BIT [10:9] ATTR
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t INCREASE_REGION_SIZE        :1;      ///<BIT [13] INCREASE_REGION_SIZE
        uint32_t RSVD_14_19                  :6;      ///<BIT [19:14] rsvd_14_19
        uint32_t CTRL_1_FUNC_NUM             :1;      ///<BIT [20] CTRL_1_FUNC_NUM
        uint32_t RSVD_21_31                  :11;     ///<BIT [31:21] rsvd_21_31
    } b;
} IatuRegionCtrl1OffInbound6_t;

/// @brief 0xD04
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSG_CODE                    :8;      ///<BIT [7:0] MSG_CODE
        uint32_t BAR_NUM                     :3;      ///<BIT [10:8] BAR_NUM
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t MSG_TYPE_MATCH_MODE         :1;      ///<BIT [13] MSG_TYPE_MATCH_MODE
        uint32_t TC_MATCH_EN                 :1;      ///<BIT [14] TC_MATCH_EN
        uint32_t TD_MATCH_EN                 :1;      ///<BIT [15] TD_MATCH_EN
        uint32_t ATTR_MATCH_EN               :1;      ///<BIT [16] ATTR_MATCH_EN
        uint32_t RSVD_17_18                  :2;      ///<BIT [18:17] rsvd_17_18
        uint32_t FUNC_NUM_MATCH_EN           :1;      ///<BIT [19] FUNC_NUM_MATCH_EN
        uint32_t VF_MATCH_EN                 :1;      ///<BIT [20] VF_MATCH_EN
        uint32_t MSG_CODE_MATCH_EN           :1;      ///<BIT [21] MSG_CODE_MATCH_EN
        uint32_t RSVD_22                     :1;      ///<BIT [22] rsvd_22
        uint32_t SINGLE_ADDR_LOC_TRANS_EN    :1;      ///<BIT [23] SINGLE_ADDR_LOC_TRANS_EN
        uint32_t RESPONSE_CODE               :2;      ///<BIT [25:24] RESPONSE_CODE
        uint32_t VFBAR_MATCH_MODE_EN         :1;      ///<BIT [26] VFBAR_MATCH_MODE_EN
        uint32_t FUZZY_TYPE_MATCH_CODE       :1;      ///<BIT [27] FUZZY_TYPE_MATCH_CODE
        uint32_t CFG_SHIFT_MODE              :1;      ///<BIT [28] CFG_SHIFT_MODE
        uint32_t INVERT_MODE                 :1;      ///<BIT [29] INVERT_MODE
        uint32_t MATCH_MODE                  :1;      ///<BIT [30] MATCH_MODE
        uint32_t REGION_EN                   :1;      ///<BIT [31] REGION_EN
    } b;
} IatuRegionCtrl2OffInbound6_t;

/// @brief 0xD08
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_BASE_HW                 :12;     ///<BIT [11:0] LWR_BASE_HW
        uint32_t LWR_BASE_RW                 :20;     ///<BIT [31:12] LWR_BASE_RW
    } b;
} IatuLwrBaseAddrOffInbound6_t;

/// @brief 0xD10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CBUF_INCR                   :4;      ///<BIT [3:0] CBUF_INCR
        uint32_t LIMIT_ADDR_HW               :8;      ///<BIT [11:4] LIMIT_ADDR_HW
        uint32_t LIMIT_ADDR_RW               :20;     ///<BIT [31:12] LIMIT_ADDR_RW
    } b;
} IatuLimitAddrOffInbound6_t;

/// @brief 0xD14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_TARGET_HW               :12;     ///<BIT [11:0] LWR_TARGET_HW
        uint32_t LWR_TARGET_RW               :20;     ///<BIT [31:12] LWR_TARGET_RW
    } b;
} IatuLwrTargetAddrOffInbound6_t;

/// @brief 0xD1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VF_NUMBER                   :6;      ///<BIT [5:0] VF_NUMBER
        uint32_t RSVDP_VF_NUMBER             :25;     ///<BIT [30:6] RSVDP_VF_NUMBER
        uint32_t VF_ACTIVE                   :1;      ///<BIT [31] VF_ACTIVE
    } b;
} IatuRegionCtrl3OffInbound6_t;

/// @brief 0xD20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPPR_LIMIT_ADDR_RW          :5;      ///<BIT [4:0] UPPR_LIMIT_ADDR_RW
        uint32_t UPPR_LIMIT_ADDR_HW          :27;     ///<BIT [31:5] UPPR_LIMIT_ADDR_HW
    } b;
} IatuUpprLimitAddrOffInbound6_t;

/// @brief 0xF00
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TYPE                        :5;      ///<BIT [4:0] TYPE
        uint32_t TC                          :3;      ///<BIT [7:5] TC
        uint32_t TD                          :1;      ///<BIT [8] TD
        uint32_t ATTR                        :2;      ///<BIT [10:9] ATTR
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t INCREASE_REGION_SIZE        :1;      ///<BIT [13] INCREASE_REGION_SIZE
        uint32_t RSVD_14_19                  :6;      ///<BIT [19:14] rsvd_14_19
        uint32_t CTRL_1_FUNC_NUM             :1;      ///<BIT [20] CTRL_1_FUNC_NUM
        uint32_t RSVD_21_31                  :11;     ///<BIT [31:21] rsvd_21_31
    } b;
} IatuRegionCtrl1OffInbound7_t;

/// @brief 0xF04
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSG_CODE                    :8;      ///<BIT [7:0] MSG_CODE
        uint32_t BAR_NUM                     :3;      ///<BIT [10:8] BAR_NUM
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t MSG_TYPE_MATCH_MODE         :1;      ///<BIT [13] MSG_TYPE_MATCH_MODE
        uint32_t TC_MATCH_EN                 :1;      ///<BIT [14] TC_MATCH_EN
        uint32_t TD_MATCH_EN                 :1;      ///<BIT [15] TD_MATCH_EN
        uint32_t ATTR_MATCH_EN               :1;      ///<BIT [16] ATTR_MATCH_EN
        uint32_t RSVD_17_18                  :2;      ///<BIT [18:17] rsvd_17_18
        uint32_t FUNC_NUM_MATCH_EN           :1;      ///<BIT [19] FUNC_NUM_MATCH_EN
        uint32_t VF_MATCH_EN                 :1;      ///<BIT [20] VF_MATCH_EN
        uint32_t MSG_CODE_MATCH_EN           :1;      ///<BIT [21] MSG_CODE_MATCH_EN
        uint32_t RSVD_22                     :1;      ///<BIT [22] rsvd_22
        uint32_t SINGLE_ADDR_LOC_TRANS_EN    :1;      ///<BIT [23] SINGLE_ADDR_LOC_TRANS_EN
        uint32_t RESPONSE_CODE               :2;      ///<BIT [25:24] RESPONSE_CODE
        uint32_t VFBAR_MATCH_MODE_EN         :1;      ///<BIT [26] VFBAR_MATCH_MODE_EN
        uint32_t FUZZY_TYPE_MATCH_CODE       :1;      ///<BIT [27] FUZZY_TYPE_MATCH_CODE
        uint32_t CFG_SHIFT_MODE              :1;      ///<BIT [28] CFG_SHIFT_MODE
        uint32_t INVERT_MODE                 :1;      ///<BIT [29] INVERT_MODE
        uint32_t MATCH_MODE                  :1;      ///<BIT [30] MATCH_MODE
        uint32_t REGION_EN                   :1;      ///<BIT [31] REGION_EN
    } b;
} IatuRegionCtrl2OffInbound7_t;

/// @brief 0xF08
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_BASE_HW                 :12;     ///<BIT [11:0] LWR_BASE_HW
        uint32_t LWR_BASE_RW                 :20;     ///<BIT [31:12] LWR_BASE_RW
    } b;
} IatuLwrBaseAddrOffInbound7_t;

/// @brief 0xF10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CBUF_INCR                   :4;      ///<BIT [3:0] CBUF_INCR
        uint32_t LIMIT_ADDR_HW               :8;      ///<BIT [11:4] LIMIT_ADDR_HW
        uint32_t LIMIT_ADDR_RW               :20;     ///<BIT [31:12] LIMIT_ADDR_RW
    } b;
} IatuLimitAddrOffInbound7_t;

/// @brief 0xF14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_TARGET_HW               :12;     ///<BIT [11:0] LWR_TARGET_HW
        uint32_t LWR_TARGET_RW               :20;     ///<BIT [31:12] LWR_TARGET_RW
    } b;
} IatuLwrTargetAddrOffInbound7_t;

/// @brief 0xF1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VF_NUMBER                   :6;      ///<BIT [5:0] VF_NUMBER
        uint32_t RSVDP_VF_NUMBER             :25;     ///<BIT [30:6] RSVDP_VF_NUMBER
        uint32_t VF_ACTIVE                   :1;      ///<BIT [31] VF_ACTIVE
    } b;
} IatuRegionCtrl3OffInbound7_t;

/// @brief 0xF20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPPR_LIMIT_ADDR_RW          :5;      ///<BIT [4:0] UPPR_LIMIT_ADDR_RW
        uint32_t UPPR_LIMIT_ADDR_HW          :27;     ///<BIT [31:5] UPPR_LIMIT_ADDR_HW
    } b;
} IatuUpprLimitAddrOffInbound7_t;

/// @brief 0x1100
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TYPE                        :5;      ///<BIT [4:0] TYPE
        uint32_t TC                          :3;      ///<BIT [7:5] TC
        uint32_t TD                          :1;      ///<BIT [8] TD
        uint32_t ATTR                        :2;      ///<BIT [10:9] ATTR
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t INCREASE_REGION_SIZE        :1;      ///<BIT [13] INCREASE_REGION_SIZE
        uint32_t RSVD_14_19                  :6;      ///<BIT [19:14] rsvd_14_19
        uint32_t CTRL_1_FUNC_NUM             :1;      ///<BIT [20] CTRL_1_FUNC_NUM
        uint32_t RSVD_21_31                  :11;     ///<BIT [31:21] rsvd_21_31
    } b;
} IatuRegionCtrl1OffInbound8_t;

/// @brief 0x1104
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSG_CODE                    :8;      ///<BIT [7:0] MSG_CODE
        uint32_t BAR_NUM                     :3;      ///<BIT [10:8] BAR_NUM
        uint32_t RSVD_11_12                  :2;      ///<BIT [12:11] rsvd_11_12
        uint32_t MSG_TYPE_MATCH_MODE         :1;      ///<BIT [13] MSG_TYPE_MATCH_MODE
        uint32_t TC_MATCH_EN                 :1;      ///<BIT [14] TC_MATCH_EN
        uint32_t TD_MATCH_EN                 :1;      ///<BIT [15] TD_MATCH_EN
        uint32_t ATTR_MATCH_EN               :1;      ///<BIT [16] ATTR_MATCH_EN
        uint32_t RSVD_17_18                  :2;      ///<BIT [18:17] rsvd_17_18
        uint32_t FUNC_NUM_MATCH_EN           :1;      ///<BIT [19] FUNC_NUM_MATCH_EN
        uint32_t VF_MATCH_EN                 :1;      ///<BIT [20] VF_MATCH_EN
        uint32_t MSG_CODE_MATCH_EN           :1;      ///<BIT [21] MSG_CODE_MATCH_EN
        uint32_t RSVD_22                     :1;      ///<BIT [22] rsvd_22
        uint32_t SINGLE_ADDR_LOC_TRANS_EN    :1;      ///<BIT [23] SINGLE_ADDR_LOC_TRANS_EN
        uint32_t RESPONSE_CODE               :2;      ///<BIT [25:24] RESPONSE_CODE
        uint32_t VFBAR_MATCH_MODE_EN         :1;      ///<BIT [26] VFBAR_MATCH_MODE_EN
        uint32_t FUZZY_TYPE_MATCH_CODE       :1;      ///<BIT [27] FUZZY_TYPE_MATCH_CODE
        uint32_t CFG_SHIFT_MODE              :1;      ///<BIT [28] CFG_SHIFT_MODE
        uint32_t INVERT_MODE                 :1;      ///<BIT [29] INVERT_MODE
        uint32_t MATCH_MODE                  :1;      ///<BIT [30] MATCH_MODE
        uint32_t REGION_EN                   :1;      ///<BIT [31] REGION_EN
    } b;
} IatuRegionCtrl2OffInbound8_t;

/// @brief 0x1108
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_BASE_HW                 :12;     ///<BIT [11:0] LWR_BASE_HW
        uint32_t LWR_BASE_RW                 :20;     ///<BIT [31:12] LWR_BASE_RW
    } b;
} IatuLwrBaseAddrOffInbound8_t;

/// @brief 0x1110
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CBUF_INCR                   :4;      ///<BIT [3:0] CBUF_INCR
        uint32_t LIMIT_ADDR_HW               :8;      ///<BIT [11:4] LIMIT_ADDR_HW
        uint32_t LIMIT_ADDR_RW               :20;     ///<BIT [31:12] LIMIT_ADDR_RW
    } b;
} IatuLimitAddrOffInbound8_t;

/// @brief 0x1114
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LWR_TARGET_HW               :12;     ///<BIT [11:0] LWR_TARGET_HW
        uint32_t LWR_TARGET_RW               :20;     ///<BIT [31:12] LWR_TARGET_RW
    } b;
} IatuLwrTargetAddrOffInbound8_t;

/// @brief 0x111C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VF_NUMBER                   :6;      ///<BIT [5:0] VF_NUMBER
        uint32_t RSVDP_VF_NUMBER             :25;     ///<BIT [30:6] RSVDP_VF_NUMBER
        uint32_t VF_ACTIVE                   :1;      ///<BIT [31] VF_ACTIVE
    } b;
} IatuRegionCtrl3OffInbound8_t;

/// @brief 0x1120
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPPR_LIMIT_ADDR_RW          :5;      ///<BIT [4:0] UPPR_LIMIT_ADDR_RW
        uint32_t UPPR_LIMIT_ADDR_HW          :27;     ///<BIT [31:5] UPPR_LIMIT_ADDR_HW
    } b;
} IatuUpprLimitAddrOffInbound8_t;

/// @brief 0xC000
typedef struct
{
    IatuRegionCtrl1OffOutbound0_t iatuRegionCtrl1OffOutbound0; //IATU_REGION_CTRL_1_OFF_OUTBOUND_0
    IatuRegionCtrl2OffOutbound0_t iatuRegionCtrl2OffOutbound0; //IATU_REGION_CTRL_2_OFF_OUTBOUND_0
    IatuLwrBaseAddrOffOutbound0_t iatuLwrBaseAddrOffOutbound0; //IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0
    uint32_t iatuUpperBaseAddrOffOutbound0UpperBaseRw; //IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0
    IatuLimitAddrOffOutbound0_t iatuLimitAddrOffOutbound0; //IATU_LIMIT_ADDR_OFF_OUTBOUND_0
    uint32_t iatuLwrTargetAddrOffOutbound0LwrTargetRwOutbound; //IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0
    uint32_t iatuUpperTargetAddrOffOutbound0UpperTargetRw; //IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0
    IatuRegionCtrl3OffOutbound0_t iatuRegionCtrl3OffOutbound0; //IATU_REGION_CTRL_3_OFF_OUTBOUND_0
    IatuUpprLimitAddrOffOutbound0_t iatuUpprLimitAddrOffOutbound0; //IATU_UPPR_LIMIT_ADDR_OFF_OUTBOUND_0
    uint8_t rsvd24[220];                  //rsvd_24
    IatuRegionCtrl1OffInbound0_t iatuRegionCtrl1OffInbound0; //IATU_REGION_CTRL_1_OFF_INBOUND_0
    IatuRegionCtrl2OffInbound0_t iatuRegionCtrl2OffInbound0; //IATU_REGION_CTRL_2_OFF_INBOUND_0
    IatuLwrBaseAddrOffInbound0_t iatuLwrBaseAddrOffInbound0; //IATU_LWR_BASE_ADDR_OFF_INBOUND_0
    uint32_t iatuUpperBaseAddrOffInbound0UpperBaseRw; //IATU_UPPER_BASE_ADDR_OFF_INBOUND_0
    IatuLimitAddrOffInbound0_t iatuLimitAddrOffInbound0; //IATU_LIMIT_ADDR_OFF_INBOUND_0
    IatuLwrTargetAddrOffInbound0_t iatuLwrTargetAddrOffInbound0; //IATU_LWR_TARGET_ADDR_OFF_INBOUND_0
    uint8_t rsvd118[4];                   //rsvd_118
    IatuRegionCtrl3OffInbound0_t iatuRegionCtrl3OffInbound0; //IATU_REGION_CTRL_3_OFF_INBOUND_0
    IatuUpprLimitAddrOffInbound0_t iatuUpprLimitAddrOffInbound0; //IATU_UPPR_LIMIT_ADDR_OFF_INBOUND_0
    uint8_t rsvd124[476];                 //rsvd_124
    IatuRegionCtrl1OffInbound1_t iatuRegionCtrl1OffInbound1; //IATU_REGION_CTRL_1_OFF_INBOUND_1
    IatuRegionCtrl2OffInbound1_t iatuRegionCtrl2OffInbound1; //IATU_REGION_CTRL_2_OFF_INBOUND_1
    IatuLwrBaseAddrOffInbound1_t iatuLwrBaseAddrOffInbound1; //IATU_LWR_BASE_ADDR_OFF_INBOUND_1
    uint32_t iatuUpperBaseAddrOffInbound1UpperBaseRw; //IATU_UPPER_BASE_ADDR_OFF_INBOUND_1
    IatuLimitAddrOffInbound1_t iatuLimitAddrOffInbound1; //IATU_LIMIT_ADDR_OFF_INBOUND_1
    IatuLwrTargetAddrOffInbound1_t iatuLwrTargetAddrOffInbound1; //IATU_LWR_TARGET_ADDR_OFF_INBOUND_1
    uint8_t rsvd318[4];                   //rsvd_318
    IatuRegionCtrl3OffInbound1_t iatuRegionCtrl3OffInbound1; //IATU_REGION_CTRL_3_OFF_INBOUND_1
    IatuUpprLimitAddrOffInbound1_t iatuUpprLimitAddrOffInbound1; //IATU_UPPR_LIMIT_ADDR_OFF_INBOUND_1
    uint8_t rsvd324[476];                 //rsvd_324
    IatuRegionCtrl1OffInbound2_t iatuRegionCtrl1OffInbound2; //IATU_REGION_CTRL_1_OFF_INBOUND_2
    IatuRegionCtrl2OffInbound2_t iatuRegionCtrl2OffInbound2; //IATU_REGION_CTRL_2_OFF_INBOUND_2
    IatuLwrBaseAddrOffInbound2_t iatuLwrBaseAddrOffInbound2; //IATU_LWR_BASE_ADDR_OFF_INBOUND_2
    uint32_t iatuUpperBaseAddrOffInbound2UpperBaseRw; //IATU_UPPER_BASE_ADDR_OFF_INBOUND_2
    IatuLimitAddrOffInbound2_t iatuLimitAddrOffInbound2; //IATU_LIMIT_ADDR_OFF_INBOUND_2
    IatuLwrTargetAddrOffInbound2_t iatuLwrTargetAddrOffInbound2; //IATU_LWR_TARGET_ADDR_OFF_INBOUND_2
    uint8_t rsvd518[4];                   //rsvd_518
    IatuRegionCtrl3OffInbound2_t iatuRegionCtrl3OffInbound2; //IATU_REGION_CTRL_3_OFF_INBOUND_2
    IatuUpprLimitAddrOffInbound2_t iatuUpprLimitAddrOffInbound2; //IATU_UPPR_LIMIT_ADDR_OFF_INBOUND_2
    uint8_t rsvd524[476];                 //rsvd_524
    IatuRegionCtrl1OffInbound3_t iatuRegionCtrl1OffInbound3; //IATU_REGION_CTRL_1_OFF_INBOUND_3
    IatuRegionCtrl2OffInbound3_t iatuRegionCtrl2OffInbound3; //IATU_REGION_CTRL_2_OFF_INBOUND_3
    IatuLwrBaseAddrOffInbound3_t iatuLwrBaseAddrOffInbound3; //IATU_LWR_BASE_ADDR_OFF_INBOUND_3
    uint32_t iatuUpperBaseAddrOffInbound3UpperBaseRw; //IATU_UPPER_BASE_ADDR_OFF_INBOUND_3
    IatuLimitAddrOffInbound3_t iatuLimitAddrOffInbound3; //IATU_LIMIT_ADDR_OFF_INBOUND_3
    IatuLwrTargetAddrOffInbound3_t iatuLwrTargetAddrOffInbound3; //IATU_LWR_TARGET_ADDR_OFF_INBOUND_3
    uint8_t rsvd718[4];                   //rsvd_718
    IatuRegionCtrl3OffInbound3_t iatuRegionCtrl3OffInbound3; //IATU_REGION_CTRL_3_OFF_INBOUND_3
    IatuUpprLimitAddrOffInbound3_t iatuUpprLimitAddrOffInbound3; //IATU_UPPR_LIMIT_ADDR_OFF_INBOUND_3
    uint8_t rsvd724[476];                 //rsvd_724
    IatuRegionCtrl1OffInbound4_t iatuRegionCtrl1OffInbound4; //IATU_REGION_CTRL_1_OFF_INBOUND_4
    IatuRegionCtrl2OffInbound4_t iatuRegionCtrl2OffInbound4; //IATU_REGION_CTRL_2_OFF_INBOUND_4
    IatuLwrBaseAddrOffInbound4_t iatuLwrBaseAddrOffInbound4; //IATU_LWR_BASE_ADDR_OFF_INBOUND_4
    uint32_t iatuUpperBaseAddrOffInbound4UpperBaseRw; //IATU_UPPER_BASE_ADDR_OFF_INBOUND_4
    IatuLimitAddrOffInbound4_t iatuLimitAddrOffInbound4; //IATU_LIMIT_ADDR_OFF_INBOUND_4
    IatuLwrTargetAddrOffInbound4_t iatuLwrTargetAddrOffInbound4; //IATU_LWR_TARGET_ADDR_OFF_INBOUND_4
    uint8_t rsvd918[4];                   //rsvd_918
    IatuRegionCtrl3OffInbound4_t iatuRegionCtrl3OffInbound4; //IATU_REGION_CTRL_3_OFF_INBOUND_4
    IatuUpprLimitAddrOffInbound4_t iatuUpprLimitAddrOffInbound4; //IATU_UPPR_LIMIT_ADDR_OFF_INBOUND_4
    uint8_t rsvd924[476];                 //rsvd_924
    IatuRegionCtrl1OffInbound5_t iatuRegionCtrl1OffInbound5; //IATU_REGION_CTRL_1_OFF_INBOUND_5
    IatuRegionCtrl2OffInbound5_t iatuRegionCtrl2OffInbound5; //IATU_REGION_CTRL_2_OFF_INBOUND_5
    IatuLwrBaseAddrOffInbound5_t iatuLwrBaseAddrOffInbound5; //IATU_LWR_BASE_ADDR_OFF_INBOUND_5
    uint32_t iatuUpperBaseAddrOffInbound5UpperBaseRw; //IATU_UPPER_BASE_ADDR_OFF_INBOUND_5
    IatuLimitAddrOffInbound5_t iatuLimitAddrOffInbound5; //IATU_LIMIT_ADDR_OFF_INBOUND_5
    IatuLwrTargetAddrOffInbound5_t iatuLwrTargetAddrOffInbound5; //IATU_LWR_TARGET_ADDR_OFF_INBOUND_5
    uint8_t rsvdB18[4];                   //rsvd_b18
    IatuRegionCtrl3OffInbound5_t iatuRegionCtrl3OffInbound5; //IATU_REGION_CTRL_3_OFF_INBOUND_5
    IatuUpprLimitAddrOffInbound5_t iatuUpprLimitAddrOffInbound5; //IATU_UPPR_LIMIT_ADDR_OFF_INBOUND_5
    uint8_t rsvdB24[476];                 //rsvd_b24
    IatuRegionCtrl1OffInbound6_t iatuRegionCtrl1OffInbound6; //IATU_REGION_CTRL_1_OFF_INBOUND_6
    IatuRegionCtrl2OffInbound6_t iatuRegionCtrl2OffInbound6; //IATU_REGION_CTRL_2_OFF_INBOUND_6
    IatuLwrBaseAddrOffInbound6_t iatuLwrBaseAddrOffInbound6; //IATU_LWR_BASE_ADDR_OFF_INBOUND_6
    uint32_t iatuUpperBaseAddrOffInbound6UpperBaseRw; //IATU_UPPER_BASE_ADDR_OFF_INBOUND_6
    IatuLimitAddrOffInbound6_t iatuLimitAddrOffInbound6; //IATU_LIMIT_ADDR_OFF_INBOUND_6
    IatuLwrTargetAddrOffInbound6_t iatuLwrTargetAddrOffInbound6; //IATU_LWR_TARGET_ADDR_OFF_INBOUND_6
    uint8_t rsvdD18[4];                   //rsvd_d18
    IatuRegionCtrl3OffInbound6_t iatuRegionCtrl3OffInbound6; //IATU_REGION_CTRL_3_OFF_INBOUND_6
    IatuUpprLimitAddrOffInbound6_t iatuUpprLimitAddrOffInbound6; //IATU_UPPR_LIMIT_ADDR_OFF_INBOUND_6
    uint8_t rsvdD24[476];                 //rsvd_d24
    IatuRegionCtrl1OffInbound7_t iatuRegionCtrl1OffInbound7; //IATU_REGION_CTRL_1_OFF_INBOUND_7
    IatuRegionCtrl2OffInbound7_t iatuRegionCtrl2OffInbound7; //IATU_REGION_CTRL_2_OFF_INBOUND_7
    IatuLwrBaseAddrOffInbound7_t iatuLwrBaseAddrOffInbound7; //IATU_LWR_BASE_ADDR_OFF_INBOUND_7
    uint32_t iatuUpperBaseAddrOffInbound7UpperBaseRw; //IATU_UPPER_BASE_ADDR_OFF_INBOUND_7
    IatuLimitAddrOffInbound7_t iatuLimitAddrOffInbound7; //IATU_LIMIT_ADDR_OFF_INBOUND_7
    IatuLwrTargetAddrOffInbound7_t iatuLwrTargetAddrOffInbound7; //IATU_LWR_TARGET_ADDR_OFF_INBOUND_7
    uint8_t rsvdF18[4];                   //rsvd_f18
    IatuRegionCtrl3OffInbound7_t iatuRegionCtrl3OffInbound7; //IATU_REGION_CTRL_3_OFF_INBOUND_7
    IatuUpprLimitAddrOffInbound7_t iatuUpprLimitAddrOffInbound7; //IATU_UPPR_LIMIT_ADDR_OFF_INBOUND_7
    uint8_t rsvdF24[476];                 //rsvd_f24
    IatuRegionCtrl1OffInbound8_t iatuRegionCtrl1OffInbound8; //IATU_REGION_CTRL_1_OFF_INBOUND_8
    IatuRegionCtrl2OffInbound8_t iatuRegionCtrl2OffInbound8; //IATU_REGION_CTRL_2_OFF_INBOUND_8
    IatuLwrBaseAddrOffInbound8_t iatuLwrBaseAddrOffInbound8; //IATU_LWR_BASE_ADDR_OFF_INBOUND_8
    uint32_t iatuUpperBaseAddrOffInbound8UpperBaseRw; //IATU_UPPER_BASE_ADDR_OFF_INBOUND_8
    IatuLimitAddrOffInbound8_t iatuLimitAddrOffInbound8; //IATU_LIMIT_ADDR_OFF_INBOUND_8
    IatuLwrTargetAddrOffInbound8_t iatuLwrTargetAddrOffInbound8; //IATU_LWR_TARGET_ADDR_OFF_INBOUND_8
    uint8_t rsvd1118[4];                  //rsvd_1118
    IatuRegionCtrl3OffInbound8_t iatuRegionCtrl3OffInbound8; //IATU_REGION_CTRL_3_OFF_INBOUND_8
    IatuUpprLimitAddrOffInbound8_t iatuUpprLimitAddrOffInbound8; //IATU_UPPR_LIMIT_ADDR_OFF_INBOUND_8
} Pf0AtuCap_t;

/// @brief 0x41F8
typedef struct
{
    uint8_t rsvd0[20];                    //rsvd_0
    ShadowSriovVfOffsetPosition_t shadowSriovVfOffsetPosition; //SHADOW_SRIOV_VF_OFFSET_POSITION
    uint8_t rsvd18[12];                   //rsvd_18
    SriovBar0Mask_t sriovBar0Mask;        //SRIOV_BAR0_MASK_REG
    SriovBar1Mask_t sriovBar1Mask;        //SRIOV_BAR1_MASK_REG
    SriovBar2Mask_t sriovBar2Mask;        //SRIOV_BAR2_MASK_REG
    SriovBar3Mask_t sriovBar3Mask;        //SRIOV_BAR3_MASK_REG
    SriovBar4Mask_t sriovBar4Mask;        //SRIOV_BAR4_MASK_REG
    SriovBar5Mask_t sriovBar5Mask;        //SRIOV_BAR5_MASK_REG
} Pf0SriovCapDbi2_t;

/// @brief 0x40B0
typedef struct
{
    ShadowPciMsixCapIdNextCtrl_t shadowPciMsixCapIdNextCtrl; //SHADOW_PCI_MSIX_CAP_ID_NEXT_CTRL_REG
    ShadowMsixTableOffset_t shadowMsixTableOffset; //SHADOW_MSIX_TABLE_OFFSET_REG
    ShadowMsixPbaOffset_t shadowMsixPbaOffset; //SHADOW_MSIX_PBA_OFFSET_REG
} Pf0MsixCapDbi2_t;

/// @brief 0x4070
typedef struct
{
    uint8_t rsvd0[12];                    //rsvd_0
    ShadowLinkCapabilities_t shadowLinkCapabilities; //SHADOW_LINK_CAPABILITIES_REG
} Pf0PcieCapDbi2_t;

/// @brief 0x4000
typedef struct
{
    uint8_t rsvd0[16];                    //rsvd_0
    Bar0Mask_t bar0Mask;                  //BAR0_MASK_REG
    Bar1Mask_t bar1Mask;                  //BAR1_MASK_REG
    Bar2Mask_t bar2Mask;                  //BAR2_MASK_REG
    Bar3Mask_t bar3Mask;                  //BAR3_MASK_REG
    Bar4Mask_t bar4Mask;                  //BAR4_MASK_REG
    Bar5Mask_t bar5Mask;                  //BAR5_MASK_REG
    uint8_t rsvd28[8];                    //rsvd_28
    ExpRomBarMask_t expRomBarMask;        //EXP_ROM_BAR_MASK_REG
} Pf0Type0HdrDbi2_t;

/// @brief 0x2148
typedef struct
{
    VfAriBase_t vfAriBase;                //VF_ARI_BASE
    VfCap_t vfCap;                        //VF_CAP_REG
} Vf1Pf0AriCap_t;

/// @brief 0x2100
typedef struct
{
    VfAerExtCapHdrOff_t vfAerExtCapHdrOff; //VF_AER_EXT_CAP_HDR_OFF
    VfUncorrErrStatusOff_t vfUncorrErrStatusOff; //VF_UNCORR_ERR_STATUS_OFF
    VfUncorrErrMaskOff_t vfUncorrErrMaskOff; //VF_UNCORR_ERR_MASK_OFF
    VfUncorrErrSevOff_t vfUncorrErrSevOff; //VF_UNCORR_ERR_SEV_OFF
    VfCorrErrStatusOff_t vfCorrErrStatusOff; //VF_CORR_ERR_STATUS_OFF
    VfCorrErrMaskOff_t vfCorrErrMaskOff;  //VF_CORR_ERR_MASK_OFF
    VfAdvErrCapCtrlOff_t vfAdvErrCapCtrlOff; //VF_ADV_ERR_CAP_CTRL_OFF
    VfHdrLog0Off_t vfHdrLog0Off;          //VF_HDR_LOG_0_OFF
    VfHdrLog1Off_t vfHdrLog1Off;          //VF_HDR_LOG_1_OFF
    VfHdrLog2Off_t vfHdrLog2Off;          //VF_HDR_LOG_2_OFF
    VfHdrLog3Off_t vfHdrLog3Off;          //VF_HDR_LOG_3_OFF
    uint8_t rsvd2c[12];                   //rsvd_2c
    VfTlpPrefixLog1Off_t vfTlpPrefixLog1Off; //VF_TLP_PREFIX_LOG_1_OFF
    VfTlpPrefixLog2Off_t vfTlpPrefixLog2Off; //VF_TLP_PREFIX_LOG_2_OFF
    VfTlpPrefixLog3Off_t vfTlpPrefixLog3Off; //VF_TLP_PREFIX_LOG_3_OFF
    VfTlpPrefixLog4Off_t vfTlpPrefixLog4Off; //VF_TLP_PREFIX_LOG_4_OFF
} Vf1Pf0AerCap_t;

/// @brief 0x20B0
typedef struct
{
    VfPciMsixCapIdNextCtrl_t vfPciMsixCapIdNextCtrl; //VF_PCI_MSIX_CAP_ID_NEXT_CTRL_REG
    VfMsixTableOffset_t vfMsixTableOffset; //VF_MSIX_TABLE_OFFSET_REG
    VfMsixPbaOffset_t vfMsixPbaOffset;    //VF_MSIX_PBA_OFFSET_REG
} Vf1Pf0MsixCap_t;

/// @brief 0x2070
typedef struct
{
    VfPcieCapIdPcieNextCapPtrPcieCap_t vfPcieCapIdPcieNextCapPtrPcieCap; //VF_PCIE_CAP_ID_PCIE_NEXT_CAP_PTR_PCIE_CAP_REG
    VfDeviceCapabilities_t vfDeviceCapabilities; //VF_DEVICE_CAPABILITIES_REG
    VfDeviceControlDeviceStatus_t vfDeviceControlDeviceStatus; //VF_DEVICE_CONTROL_DEVICE_STATUS
    VfLinkCapabilities_t vfLinkCapabilities; //VF_LINK_CAPABILITIES_REG
    VfLinkControlLinkStatus_t vfLinkControlLinkStatus; //VF_LINK_CONTROL_LINK_STATUS_REG
    uint8_t rsvd14[16];                   //rsvd_14
    VfDeviceCapabilities2_t vfDeviceCapabilities2; //VF_DEVICE_CAPABILITIES2_REG
    VfDeviceControl2DeviceStatus2_t vfDeviceControl2DeviceStatus2; //VF_DEVICE_CONTROL2_DEVICE_STATUS2_REG
    VfLinkCapabilities2_t vfLinkCapabilities2; //VF_LINK_CAPABILITIES2_REG
    VfLinkControl2LinkStatus2_t vfLinkControl2LinkStatus2; //VF_LINK_CONTROL2_LINK_STATUS2_REG
} Vf1Pf0PcieCap_t;

/// @brief 0x2050
typedef struct
{
    VfPciMsiCapIdNextCtrl_t vfPciMsiCapIdNextCtrl; //VF_PCI_MSI_CAP_ID_NEXT_CTRL_REG
    VfMsiCapOff04h_t vfMsiCapOff04h;      //VF_MSI_CAP_OFF_04H_REG
    VfMsiCapOff08h_t vfMsiCapOff08h;      //VF_MSI_CAP_OFF_08H_REG
    VfMsiCapOff0ch_t vfMsiCapOff0ch;      //VF_MSI_CAP_OFF_0CH_REG
    uint32_t vfMsiCapOff10hPciMsiCapOff10h; //VF_MSI_CAP_OFF_10H_REG
    uint32_t vfMsiCapOff14hPciMsiCapOff14h; //VF_MSI_CAP_OFF_14H_REG
} Vf1Pf0MsiCap_t;

/// @brief 0x2000
typedef struct
{
    VfDeviceIdVendorId_t vfDeviceIdVendorId; //VF_DEVICE_ID_VENDOR_ID_REG
    VfStatusCommand_t vfStatusCommand;    //VF_STATUS_COMMAND_REG
    VfClassCodeRevisionId_t vfClassCodeRevisionId; //VF_CLASS_CODE_REVISION_ID
    VfBistHeaderTypeLatencyCacheLineSize_t vfBistHeaderTypeLatencyCacheLineSize; //VF_BIST_HEADER_TYPE_LATENCY_CACHE_LINE_SIZE_REG
    VfBar0_t vfBar0;                      //VF_BAR0_REG
    VfBar1_t vfBar1;                      //VF_BAR1_REG
    VfBar2_t vfBar2;                      //VF_BAR2_REG
    VfBar3_t vfBar3;                      //VF_BAR3_REG
    VfBar4_t vfBar4;                      //VF_BAR4_REG
    VfBar5_t vfBar5;                      //VF_BAR5_REG
    uint32_t vfCardbusCisPtrCardbusCisPointer; //VF_CARDBUS_CIS_PTR_REG
    VfSubsystemIdSubsystemVendorId_t vfSubsystemIdSubsystemVendorId; //VF_SUBSYSTEM_ID_SUBSYSTEM_VENDOR_ID_REG
    uint8_t rsvd30[4];                    //rsvd_30
    VfPciCapPtr_t vfPciCapPtr;            //VF_PCI_CAP_PTR_REG
    uint8_t rsvd38[4];                    //rsvd_38
    VfMaxLatencyMinGrantIntrPinIntrLine_t vfMaxLatencyMinGrantIntrPinIntrLine; //VF_MAX_LATENCY_MIN_GRANT_INTERRUPT_PIN_INTERRUPT_LINE_REG
} Vf1Pf0Type0Hdr_t;

/// @brief 0x700
typedef struct
{
    AckLatencyTimerOff_t ackLatencyTimerOff; //ACK_LATENCY_TIMER_OFF
    uint32_t vendorSpecDllpOffVendorSpecDllp; //VENDOR_SPEC_DLLP_OFF
    PortForceOff_t portForceOff;          //PORT_FORCE_OFF
    AckFAspmCtrlOff_t ackFAspmCtrlOff;    //ACK_F_ASPM_CTRL_OFF
    PortLinkCtrlOff_t portLinkCtrlOff;    //PORT_LINK_CTRL_OFF
    LaneSkewOff_t laneSkewOff;            //LANE_SKEW_OFF
    TimerCtrlMaxFuncNumOff_t timerCtrlMaxFuncNumOff; //TIMER_CTRL_MAX_FUNC_NUM_OFF
    SymbolTimerFilter1Off_t symbolTimerFilter1Off; //SYMBOL_TIMER_FILTER_1_OFF
    uint32_t filterMask2OffMaskRadm2;     //FILTER_MASK_2_OFF
    uint8_t rsvd24[4];                    //rsvd_24
    uint32_t plDebug0OffDebReg0;          //PL_DEBUG0_OFF
    uint32_t plDebug1OffDebReg1;          //PL_DEBUG1_OFF
    TxPFcCreditStatusOff_t txPFcCreditStatusOff; //TX_P_FC_CREDIT_STATUS_OFF
    TxNpFcCreditStatusOff_t txNpFcCreditStatusOff; //TX_NP_FC_CREDIT_STATUS_OFF
    TxCplFcCreditStatusOff_t txCplFcCreditStatusOff; //TX_CPL_FC_CREDIT_STATUS_OFF
    QueueStatusOff_t queueStatusOff;      //QUEUE_STATUS_OFF
    VcTxArbi1Off_t vcTxArbi1Off;          //VC_TX_ARBI_1_OFF
    VcTxArbi2Off_t vcTxArbi2Off;          //VC_TX_ARBI_2_OFF
    Vc0PRxQCtrlOff_t vc0PRxQCtrlOff;      //VC0_P_RX_Q_CTRL_OFF
    Vc0NpRxQCtrlOff_t vc0NpRxQCtrlOff;    //VC0_NP_RX_Q_CTRL_OFF
    Vc0CplRxQCtrlOff_t vc0CplRxQCtrlOff;  //VC0_CPL_RX_Q_CTRL_OFF
    uint8_t rsvd54[184];                  //rsvd_54
    Gen2CtrlOff_t gen2CtrlOff;            //GEN2_CTRL_OFF
    uint32_t phyStatusOffPhyStatus;       //PHY_STATUS_OFF
    uint32_t phyControlOffPhyControl;     //PHY_CONTROL_OFF
    uint8_t rsvd118[4];                   //rsvd_118
    TrgtMapCtrlOff_t trgtMapCtrlOff;      //TRGT_MAP_CTRL_OFF
    uint8_t rsvd120[108];                 //rsvd_120
    ClockGatingCtrlOff_t clockGatingCtrlOff; //CLOCK_GATING_CTRL_OFF
    Gen3RelatedOff_t gen3RelatedOff;      //GEN3_RELATED_OFF
    uint8_t rsvd194[20];                  //rsvd_194
    Gen3EqControlOff_t gen3EqControlOff;  //GEN3_EQ_CONTROL_OFF
    uint8_t rsvd1ac[8];                   //rsvd_1ac
    OrderRuleCtrlOff_t orderRuleCtrlOff;  //ORDER_RULE_CTRL_OFF
    PipeLoopbackControlOff_t pipeLoopbackControlOff; //PIPE_LOOPBACK_CONTROL_OFF
    MiscControl1Off_t miscControl1Off;    //MISC_CONTROL_1_OFF
    MultiLaneControlOff_t multiLaneControlOff; //MULTI_LANE_CONTROL_OFF
    PhyInteropCtrlOff_t phyInteropCtrlOff; //PHY_INTEROP_CTRL_OFF
    TrgtCplLutDeleteEntryOff_t trgtCplLutDeleteEntryOff; //TRGT_CPL_LUT_DELETE_ENTRY_OFF
    LinkFlushControlOff_t linkFlushControlOff; //LINK_FLUSH_CONTROL_OFF
    AmbaErrorResponseDefaultOff_t ambaErrorResponseDefaultOff; //AMBA_ERROR_RESPONSE_DEFAULT_OFF
    AmbaLinkTimeoutOff_t ambaLinkTimeoutOff; //AMBA_LINK_TIMEOUT_OFF
    AmbaOrderingCtrlOff_t ambaOrderingCtrlOff; //AMBA_ORDERING_CTRL_OFF
    uint8_t rsvd1dc[4];                   //rsvd_1dc
    CoherencyControl1Off_t coherencyControl1Off; //COHERENCY_CONTROL_1_OFF
    uint32_t coherencyControl2OffCfgMemtypeBoundaryHighAddr; //COHERENCY_CONTROL_2_OFF
    CoherencyControl3Off_t coherencyControl3Off; //COHERENCY_CONTROL_3_OFF
    uint8_t rsvd1ec[4];                   //rsvd_1ec
    AxiMstrMsgAddrLowOff_t axiMstrMsgAddrLowOff; //AXI_MSTR_MSG_ADDR_LOW_OFF
    uint32_t axiMstrMsgAddrHighOffCfgAximstrMsgAddrHigh; //AXI_MSTR_MSG_ADDR_HIGH_OFF
    uint32_t pcieVersionNumberOffVersionNumber; //PCIE_VERSION_NUMBER_OFF
    uint32_t pcieVersionTypeOffVersionType; //PCIE_VERSION_TYPE_OFF
    uint8_t rsvd200[540];                 //rsvd_200
    PcipmTrafficCtrlOff_t pcipmTrafficCtrlOff; //PCIPM_TRAFFIC_CTRL_OFF
    uint8_t rsvd420[16];                  //rsvd_420
    PlLtrLatencyOff_t plLtrLatencyOff;    //PL_LTR_LATENCY_OFF
    uint8_t rsvd434[12];                  //rsvd_434
    AuxClkFreqOff_t auxClkFreqOff;        //AUX_CLK_FREQ_OFF
    L1SubstatesOff_t l1SubstatesOff;      //L1_SUBSTATES_OFF
    PowerdownCtrlStatusOff_t powerdownCtrlStatusOff; //POWERDOWN_CTRL_STATUS_OFF
    PhyInteropCtrl2Off_t phyInteropCtrl2Off; //PHY_INTEROP_CTRL_2_OFF
    uint8_t rsvd450[48];                  //rsvd_450
    Gen4LaneMargining1Off_t gen4LaneMargining1Off; //GEN4_LANE_MARGINING_1_OFF
    Gen4LaneMargining2Off_t gen4LaneMargining2Off; //GEN4_LANE_MARGINING_2_OFF
    Gen5LaneMargining1Off_t gen5LaneMargining1Off; //GEN5_LANE_MARGINING_1_OFF
    Gen5LaneMargining2Off_t gen5LaneMargining2Off; //GEN5_LANE_MARGINING_2_OFF
    PipeRelatedOff_t pipeRelatedOff;      //PIPE_RELATED_OFF
    uint8_t rsvd494[232];                 //rsvd_494
    DbiFunctionBankCtrlOff_t dbiFunctionBankCtrlOff; //DBI_FUNCTION_BANK_CTRL_REG_OFF
    uint32_t utilityOffUtility;           //UTILITY_OFF
    uint8_t rsvd584[4];                   //rsvd_584
    uint32_t pmUtilityOffPmUtility;       //PM_UTILITY_OFF
    IdeCtrlOff_t ideCtrlOff;              //IDE_CTRL_OFF
} Pf0PortLogic_t;

/// @brief 0x3B8
typedef struct
{
    DataLinkFeatureExtHdrOff_t dataLinkFeatureExtHdrOff; //DATA_LINK_FEATURE_EXT_HDR_OFF
    DataLinkFeatureCapOff_t dataLinkFeatureCapOff; //DATA_LINK_FEATURE_CAP_OFF
    DataLinkFeatureStatusOff_t dataLinkFeatureStatusOff; //DATA_LINK_FEATURE_STATUS_OFF
} Pf0DlinkCap_t;

/// @brief 0x380
typedef struct
{
    RasdpExtCapHdrOff_t rasdpExtCapHdrOff; //RASDP_EXT_CAP_HDR_OFF
    RasdpVendorSpecificHdrOff_t rasdpVendorSpecificHdrOff; //RASDP_VENDOR_SPECIFIC_HDR_OFF
    RasdpErrorProtCtrlOff_t rasdpErrorProtCtrlOff; //RASDP_ERROR_PROT_CTRL_OFF
    RasdpCorrCounterCtrlOff_t rasdpCorrCounterCtrlOff; //RASDP_CORR_COUNTER_CTRL_OFF
    RasdpCorrCountReportOff_t rasdpCorrCountReportOff; //RASDP_CORR_COUNT_REPORT_OFF
    RasdpUncorrCounterCtrlOff_t rasdpUncorrCounterCtrlOff; //RASDP_UNCORR_COUNTER_CTRL_OFF
    RasdpUncorrCountReportOff_t rasdpUncorrCountReportOff; //RASDP_UNCORR_COUNT_REPORT_OFF
    RasdpErrorInjCtrlOff_t rasdpErrorInjCtrlOff; //RASDP_ERROR_INJ_CTRL_OFF
    RasdpCorrErrorLocationOff_t rasdpCorrErrorLocationOff; //RASDP_CORR_ERROR_LOCATION_OFF
    RasdpUncorrErrorLocationOff_t rasdpUncorrErrorLocationOff; //RASDP_UNCORR_ERROR_LOCATION_OFF
    RasdpErrorModeEnOff_t rasdpErrorModeEnOff; //RASDP_ERROR_MODE_EN_OFF
    RasdpErrorModeClearOff_t rasdpErrorModeClearOff; //RASDP_ERROR_MODE_CLEAR_OFF
    RasdpRamAddrCorrErrorOff_t rasdpRamAddrCorrErrorOff; //RASDP_RAM_ADDR_CORR_ERROR_OFF
    RasdpRamAddrUncorrErrorOff_t rasdpRamAddrUncorrErrorOff; //RASDP_RAM_ADDR_UNCORR_ERROR_OFF
} Pf0VsecrasCap_t;

/// @brief 0x280
typedef struct
{
    RasDesCapHeader_t rasDesCapHeader;    //RAS_DES_CAP_HEADER_REG
    VendorSpecificHeader_t vendorSpecificHeader; //VENDOR_SPECIFIC_HEADER_REG
    EventCounterControl_t eventCounterControl; //EVENT_COUNTER_CONTROL_REG
    uint32_t eventCounterData;            //EVENT_COUNTER_DATA_REG
    TimeBasedAnalysisControl_t timeBasedAnalysisControl; //TIME_BASED_ANALYSIS_CONTROL_REG
    uint32_t timeBasedAnalysisData;       //TIME_BASED_ANALYSIS_DATA_REG
    uint32_t timeBasedAnalysisData6332;   //TIME_BASED_ANALYSIS_DATA_63_32_REG
    uint8_t rsvd1c[20];                   //rsvd_1c
    EinjEnable_t einjEnable;              //EINJ_ENABLE_REG
    Einj0Crc_t einj0Crc;                  //EINJ0_CRC_REG
    Einj1Seqnum_t einj1Seqnum;            //EINJ1_SEQNUM_REG
    Einj2Dllp_t einj2Dllp;                //EINJ2_DLLP_REG
    Einj3Symbol_t einj3Symbol;            //EINJ3_SYMBOL_REG
    Einj4Fc_t einj4Fc;                    //EINJ4_FC_REG
    Einj5SpTlp_t einj5SpTlp;              //EINJ5_SP_TLP_REG
    uint32_t einj6ComparePointH0;         //EINJ6_COMPARE_POINT_H0_REG
    uint32_t einj6ComparePointH1;         //EINJ6_COMPARE_POINT_H1_REG
    uint32_t einj6ComparePointH2;         //EINJ6_COMPARE_POINT_H2_REG
    uint32_t einj6ComparePointH3;         //EINJ6_COMPARE_POINT_H3_REG
    uint32_t einj6CompareValueH0;         //EINJ6_COMPARE_VALUE_H0_REG
    uint32_t einj6CompareValueH1;         //EINJ6_COMPARE_VALUE_H1_REG
    uint32_t einj6CompareValueH2;         //EINJ6_COMPARE_VALUE_H2_REG
    uint32_t einj6CompareValueH3;         //EINJ6_COMPARE_VALUE_H3_REG
    uint32_t einj6ChangePointH0;          //EINJ6_CHANGE_POINT_H0_REG
    uint32_t einj6ChangePointH1;          //EINJ6_CHANGE_POINT_H1_REG
    uint32_t einj6ChangePointH2;          //EINJ6_CHANGE_POINT_H2_REG
    uint32_t einj6ChangePointH3;          //EINJ6_CHANGE_POINT_H3_REG
    uint32_t einj6ChangeValueH0;          //EINJ6_CHANGE_VALUE_H0_REG
    uint32_t einj6ChangeValueH1;          //EINJ6_CHANGE_VALUE_H1_REG
    uint32_t einj6ChangeValueH2;          //EINJ6_CHANGE_VALUE_H2_REG
    uint32_t einj6ChangeValueH3;          //EINJ6_CHANGE_VALUE_H3_REG
    Einj6Tlp_t einj6Tlp;                  //EINJ6_TLP_REG
    uint8_t rsvd90[16];                   //rsvd_90
    SdControl1_t sdControl1;              //SD_CONTROL1_REG
    SdControl2_t sdControl2;              //SD_CONTROL2_REG
    uint8_t rsvdA8[8];                    //rsvd_a8
    SdStatusL1lane_t sdStatusL1lane;      //SD_STATUS_L1LANE_REG
    SdStatusL1ltssm_t sdStatusL1ltssm;    //SD_STATUS_L1LTSSM_REG
    SdStatusPm_t sdStatusPm;              //SD_STATUS_PM_REG
    SdStatusL2_t sdStatusL2;              //SD_STATUS_L2_REG
    SdStatusL3fc_t sdStatusL3fc;          //SD_STATUS_L3FC_REG
    SdStatusL3_t sdStatusL3;              //SD_STATUS_L3_REG
    uint8_t rsvdC8[8];                    //rsvd_c8
    SdEqControl1_t sdEqControl1;          //SD_EQ_CONTROL1_REG
    SdEqControl2_t sdEqControl2;          //SD_EQ_CONTROL2_REG
    SdEqControl3_t sdEqControl3;          //SD_EQ_CONTROL3_REG
    uint8_t rsvdDc[4];                    //rsvd_dc
    SdEqStatus1_t sdEqStatus1;            //SD_EQ_STATUS1_REG
    SdEqStatus2_t sdEqStatus2;            //SD_EQ_STATUS2_REG
    SdEqStatus3_t sdEqStatus3;            //SD_EQ_STATUS3_REG
} Pf0RasDesCap_t;

/// @brief 0x250
typedef struct
{
    DpaExtCapHdr_t dpaExtCapHdr;          //DPA_EXT_CAP_HDR_REG
    DpaCap_t dpaCap;                      //DPA_CAP_REG
    uint32_t dpaLatIndX1Indicator1;       //DPA_LAT_IND_REG
    DpaStatusCntrl_t dpaStatusCntrl;      //DPA_STATUS_CNTRL_REG
    DpaPwrAllocArray0_t dpaPwrAllocArray0; //DPA_PWR_ALLOC_ARRAY0
    DpaPwrAllocArray4_t dpaPwrAllocArray4; //DPA_PWR_ALLOC_ARRAY4
    DpaPwrAllocArray8_t dpaPwrAllocArray8; //DPA_PWR_ALLOC_ARRAY8
    DpaPwrAllocArray12_t dpaPwrAllocArray12; //DPA_PWR_ALLOC_ARRAY12
    DpaPwrAllocArray16_t dpaPwrAllocArray16; //DPA_PWR_ALLOC_ARRAY16
    DpaPwrAllocArray20_t dpaPwrAllocArray20; //DPA_PWR_ALLOC_ARRAY20
    DpaPwrAllocArray24_t dpaPwrAllocArray24; //DPA_PWR_ALLOC_ARRAY24
    DpaPwrAllocArray28_t dpaPwrAllocArray28; //DPA_PWR_ALLOC_ARRAY28
} Pf0DpaCap_t;

/// @brief 0x240
typedef struct
{
    L1subCapHeader_t l1subCapHeader;      //L1SUB_CAP_HEADER_REG
    L1subCapability_t l1subCapability;    //L1SUB_CAPABILITY_REG
    L1subControl1_t l1subControl1;        //L1SUB_CONTROL1_REG
    L1subControl2_t l1subControl2;        //L1SUB_CONTROL2_REG
} Pf0L1subCap_t;

/// @brief 0x238
typedef struct
{
    LtrCapHdr_t ltrCapHdr;                //LTR_CAP_HDR_REG
    LtrLatency_t ltrLatency;              //LTR_LATENCY_REG
} Pf0LtrCap_t;

/// @brief 0x1F8
typedef struct
{
    SriovBase_t sriovBase;                //SRIOV_BASE_REG
    Capabilities_t capabilities;          //CAPABILITIES_REG
    StatusControl_t statusControl;        //STATUS_CONTROL_REG
    TotalVfsInitialVfs_t totalVfsInitialVfs; //TOTAL_VFS_INITIAL_VFS_REG
    SriovNumVfs_t sriovNumVfs;            //SRIOV_NUM_VFS
    SriovVfOffsetPosition_t sriovVfOffsetPosition; //SRIOV_VF_OFFSET_POSITION
    VfDeviceId_t vfDeviceId;              //VF_DEVICE_ID_REG
    uint32_t supPageSizesSriovSupPageSize; //SUP_PAGE_SIZES_REG
    uint32_t systemPageSize;              //SYSTEM_PAGE_SIZE_REG
    SriovBar0_t sriovBar0;                //SRIOV_BAR0_REG
    SriovBar1_t sriovBar1;                //SRIOV_BAR1_REG
    SriovBar2_t sriovBar2;                //SRIOV_BAR2_REG
    SriovBar3_t sriovBar3;                //SRIOV_BAR3_REG
    SriovBar4_t sriovBar4;                //SRIOV_BAR4_REG
    SriovBar5_t sriovBar5;                //SRIOV_BAR5_REG
    VfMigrationStateArray_t vfMigrationStateArray; //VF_MIGRATION_STATE_ARRAY_REG
} Pf0SriovCap_t;

/// @brief 0x1D4
typedef struct
{
    Pl32gExtCapHdr_t pl32gExtCapHdr;      //PL32G_EXT_CAP_HDR_REG
    Pl32gCapability_t pl32gCapability;    //PL32G_CAPABILITY_REG
    Pl32gControl_t pl32gControl;          //PL32G_CONTROL_REG
    Pl32gStatus_t pl32gStatus;            //PL32G_STATUS_REG
    Pl32gRcvdModTsData1_t pl32gRcvdModTsData1; //PL32G_RCVD_MOD_TS_DATA1_REG
    Pl32gRcvdModTsData2_t pl32gRcvdModTsData2; //PL32G_RCVD_MOD_TS_DATA2_REG
    Pl32gTxModTsData1_t pl32gTxModTsData1; //PL32G_TX_MOD_TS_DATA1_REG
    Pl32gTxModTsData2_t pl32gTxModTsData2; //PL32G_TX_MOD_TS_DATA2_REG
    Pl32gCapOff20h_t pl32gCapOff20h;      //PL32G_CAP_OFF_20H_REG
} Pf0Pl32gCap_t;

/// @brief 0x1BC
typedef struct
{
    MarginExtCapHdr_t marginExtCapHdr;    //MARGIN_EXT_CAP_HDR_REG
    MarginPortCapabilitiesStatus_t marginPortCapabilitiesStatus; //MARGIN_PORT_CAPABILITIES_STATUS_REG
    MarginLaneCntrlStatus0_t marginLaneCntrlStatus0; //MARGIN_LANE_CNTRL_STATUS0_REG
    MarginLaneCntrlStatus1_t marginLaneCntrlStatus1; //MARGIN_LANE_CNTRL_STATUS1_REG
    MarginLaneCntrlStatus2_t marginLaneCntrlStatus2; //MARGIN_LANE_CNTRL_STATUS2_REG
    MarginLaneCntrlStatus3_t marginLaneCntrlStatus3; //MARGIN_LANE_CNTRL_STATUS3_REG
} Pf0MarginCap_t;

/// @brief 0x198
typedef struct
{
    Pl16gExtCapHdr_t pl16gExtCapHdr;      //PL16G_EXT_CAP_HDR_REG
    uint32_t pl16gCapabilityRsvdp0;       //PL16G_CAPABILITY_REG
    uint32_t pl16gControlRsvdp0;          //PL16G_CONTROL_REG
    Pl16gStatus_t pl16gStatus;            //PL16G_STATUS_REG
    Pl16gLcDparStatus_t pl16gLcDparStatus; //PL16G_LC_DPAR_STATUS_REG
    Pl16gFirstRetimerDparStatus_t pl16gFirstRetimerDparStatus; //PL16G_FIRST_RETIMER_DPAR_STATUS_REG
    Pl16gSecondRetimerDparStatus_t pl16gSecondRetimerDparStatus; //PL16G_SECOND_RETIMER_DPAR_STATUS_REG
    uint8_t rsvd1c[4];                    //rsvd_1c
    Pl16gCapOff20h_t pl16gCapOff20h;      //PL16G_CAP_OFF_20H_REG
} Pf0Pl16gCap_t;

/// @brief 0x178
typedef struct
{
    SpcieCapHeader_t spcieCapHeader;      //SPCIE_CAP_HEADER_REG
    LinkControl3_t linkControl3;          //LINK_CONTROL3_REG
    LaneErrStatus_t laneErrStatus;        //LANE_ERR_STATUS_REG
    SpcieCapOff0ch_t spcieCapOff0ch;      //SPCIE_CAP_OFF_0CH_REG
    SpcieCapOff10h_t spcieCapOff10h;      //SPCIE_CAP_OFF_10H_REG
} Pf0SpcieCap_t;

/// @brief 0x168
typedef struct
{
    AriBase_t ariBase;                    //ARI_BASE
    Cap_t cap;                            //CAP_REG
} Pf0AriCap_t;

/// @brief 0x158
typedef struct
{
    PbBase_t pbBase;                      //PB_BASE
    PbDataSelect_t pbDataSelect;          //PB_DATA_SELECT
    DataPb_t dataPb;                      //DATA_REG_PB
    CapPb_t capPb;                        //CAP_REG_PB
} Pf0PbCap_t;

/// @brief 0x148
typedef struct
{
    SnBase_t snBase;                      //SN_BASE
    uint32_t serNumDw1SnSerNumReg1Dw;     //SER_NUM_REG_DW_1
    uint32_t serNumDw2SnSerNumReg2Dw;     //SER_NUM_REG_DW_2
} Pf0SnCap_t;

/// @brief 0x100
typedef struct
{
    AerExtCapHdrOff_t aerExtCapHdrOff;    //AER_EXT_CAP_HDR_OFF
    UncorrErrStatusOff_t uncorrErrStatusOff; //UNCORR_ERR_STATUS_OFF
    UncorrErrMaskOff_t uncorrErrMaskOff;  //UNCORR_ERR_MASK_OFF
    UncorrErrSevOff_t uncorrErrSevOff;    //UNCORR_ERR_SEV_OFF
    CorrErrStatusOff_t corrErrStatusOff;  //CORR_ERR_STATUS_OFF
    CorrErrMaskOff_t corrErrMaskOff;      //CORR_ERR_MASK_OFF
    AdvErrCapCtrlOff_t advErrCapCtrlOff;  //ADV_ERR_CAP_CTRL_OFF
    HdrLog0Off_t hdrLog0Off;              //HDR_LOG_0_OFF
    HdrLog1Off_t hdrLog1Off;              //HDR_LOG_1_OFF
    HdrLog2Off_t hdrLog2Off;              //HDR_LOG_2_OFF
    HdrLog3Off_t hdrLog3Off;              //HDR_LOG_3_OFF
    uint8_t rsvd2c[12];                   //rsvd_2c
    TlpPrefixLog1Off_t tlpPrefixLog1Off;  //TLP_PREFIX_LOG_1_OFF
    TlpPrefixLog2Off_t tlpPrefixLog2Off;  //TLP_PREFIX_LOG_2_OFF
    TlpPrefixLog3Off_t tlpPrefixLog3Off;  //TLP_PREFIX_LOG_3_OFF
    TlpPrefixLog4Off_t tlpPrefixLog4Off;  //TLP_PREFIX_LOG_4_OFF
} Pf0AerCap_t;

/// @brief 0xD0
typedef struct
{
    VpdBase_t vpdBase;                    //VPD_BASE
    uint32_t dataVpdData;                 //DATA_REG
} Pf0VpdCap_t;

/// @brief 0xB0
typedef struct
{
    PciMsixCapIdNextCtrl_t pciMsixCapIdNextCtrl; //PCI_MSIX_CAP_ID_NEXT_CTRL_REG
    MsixTableOffset_t msixTableOffset;    //MSIX_TABLE_OFFSET_REG
    MsixPbaOffset_t msixPbaOffset;        //MSIX_PBA_OFFSET_REG
} Pf0MsixCap_t;

/// @brief 0x70
typedef struct
{
    PcieCapIdPcieNextCapPtrPcieCap_t pcieCapIdPcieNextCapPtrPcieCap; //PCIE_CAP_ID_PCIE_NEXT_CAP_PTR_PCIE_CAP_REG
    DeviceCapabilities_t deviceCapabilities; //DEVICE_CAPABILITIES_REG
    DeviceControlDeviceStatus_t deviceControlDeviceStatus; //DEVICE_CONTROL_DEVICE_STATUS
    LinkCapabilities_t linkCapabilities;  //LINK_CAPABILITIES_REG
    LinkControlLinkStatus_t linkControlLinkStatus; //LINK_CONTROL_LINK_STATUS_REG
    uint8_t rsvd14[16];                   //rsvd_14
    DeviceCapabilities2_t deviceCapabilities2; //DEVICE_CAPABILITIES2_REG
    DeviceControl2DeviceStatus2_t deviceControl2DeviceStatus2; //DEVICE_CONTROL2_DEVICE_STATUS2_REG
    LinkCapabilities2_t linkCapabilities2; //LINK_CAPABILITIES2_REG
    LinkControl2LinkStatus2_t linkControl2LinkStatus2; //LINK_CONTROL2_LINK_STATUS2_REG
} Pf0PcieCap_t;

/// @brief 0x50
typedef struct
{
    PciMsiCapIdNextCtrl_t pciMsiCapIdNextCtrl; //PCI_MSI_CAP_ID_NEXT_CTRL_REG
    MsiCapOff04h_t msiCapOff04h;          //MSI_CAP_OFF_04H_REG
    MsiCapOff08h_t msiCapOff08h;          //MSI_CAP_OFF_08H_REG
    MsiCapOff0ch_t msiCapOff0ch;          //MSI_CAP_OFF_0CH_REG
    uint32_t msiCapOff10hPciMsiCapOff10h; //MSI_CAP_OFF_10H_REG
    uint32_t msiCapOff14hPciMsiCapOff14h; //MSI_CAP_OFF_14H_REG
} Pf0MsiCap_t;

/// @brief 0x40
typedef struct
{
    CapIdNxtPtr_t capIdNxtPtr;            //CAP_ID_NXT_PTR_REG
    ConStatus_t conStatus;                //CON_STATUS_REG
} Pf0PmCap_t;

/// @brief 0x0
typedef struct
{
    DeviceIdVendorId_t deviceIdVendorId;  //DEVICE_ID_VENDOR_ID_REG
    StatusCommand_t statusCommand;        //STATUS_COMMAND_REG
    ClassCodeRevisionId_t classCodeRevisionId; //CLASS_CODE_REVISION_ID
    BistHeaderTypeLatencyCacheLineSize_t bistHeaderTypeLatencyCacheLineSize; //BIST_HEADER_TYPE_LATENCY_CACHE_LINE_SIZE_REG
    Bar0_t bar0;                          //BAR0_REG
    Bar1_t bar1;                          //BAR1_REG
    Bar2_t bar2;                          //BAR2_REG
    Bar3_t bar3;                          //BAR3_REG
    Bar4_t bar4;                          //BAR4_REG
    Bar5_t bar5;                          //BAR5_REG
    uint32_t cardbusCisPtrCardbusCisPointer; //CARDBUS_CIS_PTR_REG
    SubsystemIdSubsystemVendorId_t subsystemIdSubsystemVendorId; //SUBSYSTEM_ID_SUBSYSTEM_VENDOR_ID_REG
    ExpRomBaseAddr_t expRomBaseAddr;      //EXP_ROM_BASE_ADDR_REG
    PciCapPtr_t pciCapPtr;                //PCI_CAP_PTR_REG
    uint8_t rsvd38[4];                    //rsvd_38
    MaxLatencyMinGrantIntrPinIntrLine_t maxLatencyMinGrantIntrPinIntrLine; //MAX_LATENCY_MIN_GRANT_INTERRUPT_PIN_INTERRUPT_LINE_REG
} Pf0Type0Hdr_t;

/// @brief 0x0
typedef struct
{
    Pf0Type0Hdr_t pf0Type0Hdr;            //PF0_TYPE0_HDR
    Pf0PmCap_t pf0PmCap;                  //PF0_PM_CAP
    uint8_t rsvd48[8];                    //rsvd_48
    Pf0MsiCap_t pf0MsiCap;                //PF0_MSI_CAP
    uint8_t rsvd68[8];                    //rsvd_68
    Pf0PcieCap_t pf0PcieCap;              //PF0_PCIE_CAP
    uint8_t rsvdA4[12];                   //rsvd_a4
    Pf0MsixCap_t pf0MsixCap;              //PF0_MSIX_CAP
    uint8_t rsvdBc[20];                   //rsvd_bc
    Pf0VpdCap_t pf0VpdCap;                //PF0_VPD_CAP
    uint8_t rsvdD8[40];                   //rsvd_d8
    Pf0AerCap_t pf0AerCap;                //PF0_AER_CAP
    Pf0SnCap_t pf0SnCap;                  //PF0_SN_CAP
    uint8_t rsvd154[4];                   //rsvd_154
    Pf0PbCap_t pf0PbCap;                  //PF0_PB_CAP
    Pf0AriCap_t pf0AriCap;                //PF0_ARI_CAP
    uint8_t rsvd170[8];                   //rsvd_170
    Pf0SpcieCap_t pf0SpcieCap;            //PF0_SPCIE_CAP
    uint8_t rsvd18c[12];                  //rsvd_18c
    Pf0Pl16gCap_t pf0Pl16gCap;            //PF0_PL16G_CAP
    Pf0MarginCap_t pf0MarginCap;          //PF0_MARGIN_CAP
    Pf0Pl32gCap_t pf0Pl32gCap;            //PF0_PL32G_CAP
    Pf0SriovCap_t pf0SriovCap;            //PF0_SRIOV_CAP
    Pf0LtrCap_t pf0LtrCap;                //PF0_LTR_CAP
    Pf0L1subCap_t pf0L1subCap;            //PF0_L1SUB_CAP
    Pf0DpaCap_t pf0DpaCap;                //PF0_DPA_CAP
    Pf0RasDesCap_t pf0RasDesCap;          //PF0_RAS_DES_CAP
    uint8_t rsvd36c[20];                  //rsvd_36c
    Pf0VsecrasCap_t pf0VsecrasCap;        //PF0_VSECRAS_CAP
    Pf0DlinkCap_t pf0DlinkCap;            //PF0_DLINK_CAP
    uint8_t rsvd3c4[828];                 //rsvd_3c4
    Pf0PortLogic_t pf0PortLogic;          //PF0_PORT_LOGIC
    uint8_t rsvdC90[4976];                //rsvd_c90
    Vf1Pf0Type0Hdr_t vf1Pf0Type0Hdr;      //VF1_PF0_TYPE0_HDR
    uint8_t rsvd2040[16];                 //rsvd_2040
    Vf1Pf0MsiCap_t vf1Pf0MsiCap;          //VF1_PF0_MSI_CAP
    uint8_t rsvd2068[8];                  //rsvd_2068
    Vf1Pf0PcieCap_t vf1Pf0PcieCap;        //VF1_PF0_PCIE_CAP
    uint8_t rsvd20a4[12];                 //rsvd_20a4
    Vf1Pf0MsixCap_t vf1Pf0MsixCap;        //VF1_PF0_MSIX_CAP
    uint8_t rsvd20bc[68];                 //rsvd_20bc
    Vf1Pf0AerCap_t vf1Pf0AerCap;          //VF1_PF0_AER_CAP
    Vf1Pf0AriCap_t vf1Pf0AriCap;          //VF1_PF0_ARI_CAP
    uint8_t rsvd2150[7856];               //rsvd_2150
    Pf0Type0HdrDbi2_t pf0Type0HdrDbi2;    //PF0_TYPE0_HDR_DBI2
    uint8_t rsvd4034[60];                 //rsvd_4034
    Pf0PcieCapDbi2_t pf0PcieCapDbi2;      //PF0_PCIE_CAP_DBI2
    uint8_t rsvd4080[48];                 //rsvd_4080
    Pf0MsixCapDbi2_t pf0MsixCapDbi2;      //PF0_MSIX_CAP_DBI2
    uint8_t rsvd40bc[316];                //rsvd_40bc
    Pf0SriovCapDbi2_t pf0SriovCapDbi2;    //PF0_SRIOV_CAP_DBI2
    uint8_t rsvd4234[32204];              //rsvd_4234
    Pf0AtuCap_t pf0AtuCap;                //PF0_ATU_CAP
} DwcPcieUsp_t;

typedef struct
{
    DwcPcieUsp_t dwcPcieUsp;                                                // 0x0 : DWC_PCIE_USP / 
} PcieEp_t;

COMPILE_ASSERT(offsetof(PcieEp_t,dwcPcieUsp)==0x0,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile PcieEp_t rPcieEp; ///< 0xB0180000
