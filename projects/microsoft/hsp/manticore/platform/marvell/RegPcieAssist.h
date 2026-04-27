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
//! @brief PCIE_ASSIST Registers
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
        uint32_t SMLH_LTSSM_STATE            :6;      ///<BIT [5:0] SMLH_LTSSM_STATE
        uint32_t SMLH_LINK_UP                :1;      ///<BIT [6] SMLH_LINK_UP
        uint32_t RDLH_LINK_UP                :1;      ///<BIT [7] RDLH_LINK_UP
        uint32_t PM_XTLH_BLOCK_TLP           :1;      ///<BIT [8] PM_XTLH_BLOCK_TLP
        uint32_t RESERVED_5                  :1;      ///<BIT [9] reserved_5
        uint32_t PM_LINKST_IN_L0S            :1;      ///<BIT [10] PM_LINKST_IN_L0S
        uint32_t PM_CUR_STATE                :3;      ///<BIT [13:11] PM_CUR_STATE
        uint32_t RESERVED_4                  :1;      ///<BIT [14] reserved_4
        uint32_t RSVD_15                     :1;      ///<BIT [15] rsvd_15
        uint32_t APP_PF_REQ_RETRY_EN         :1;      ///<BIT [16] APP_PF_REQ_RETRY_EN
        uint32_t APP_SRIS_MODE               :1;      ///<BIT [17] APP_SRIS_MODE
        uint32_t APP_REQ_RETRY_EN            :1;      ///<BIT [18] APP_REQ_RETRY_EN
        uint32_t APP_REQ_EXIT_L1             :1;      ///<BIT [19] APP_REQ_EXIT_L1
        uint32_t APP_REQ_ENTR_L1             :1;      ///<BIT [20] APP_REQ_ENTR_L1
        uint32_t APP_READY_ENTR_L23          :1;      ///<BIT [21] APP_READY_ENTR_L23
        uint32_t APP_LTSSM_ENABLE            :1;      ///<BIT [22] APP_LTSSM_ENABLE
        uint32_t APP_LTR_MSG_REQ             :1;      ///<BIT [23] APP_LTR_MSG_REQ
        uint32_t APP_LTR_MSG_GRANT           :1;      ///<BIT [24] APP_LTR_MSG_GRANT
        uint32_t RESERVED_2                  :1;      ///<BIT [25] reserved_2
        uint32_t RSVD_26                     :1;      ///<BIT [26] rsvd_26
        uint32_t RESERVED_1                  :2;      ///<BIT [28:27] reserved_1
        uint32_t RESERVED_0                  :1;      ///<BIT [29] reserved_0
        uint32_t RSVD_30                     :1;      ///<BIT [30] rsvd_30
        uint32_t LTR_EN                      :1;      ///<BIT [31] LTR_EN
    } b;
} PcieCoreGlobal0_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0                      :1;      ///<BIT [0] rsvd_0
        uint32_t RESERVED_2                  :2;      ///<BIT [2:1] reserved_2
        uint32_t WAKE                        :1;      ///<BIT [3] WAKE
        uint32_t SYS_AUX_PWR_DET             :1;      ///<BIT [4] SYS_AUX_PWR_DET
        uint32_t PCIE_LTSSM_IN_L0            :1;      ///<BIT [5] PCIE_LTSSM_IN_L0
        uint32_t PCIE_LTSSM_IN_I0S           :1;      ///<BIT [6] PCIE_LTSSM_IN_I0S
        uint32_t PCIE_LTSSM_IN_L1            :1;      ///<BIT [7] PCIE_LTSSM_IN_L1
        uint32_t PCIE_LTSSM_IN_L2            :1;      ///<BIT [8] PCIE_LTSSM_IN_L2
        uint32_t PCIE_LTSSM_IN_L1_1          :1;      ///<BIT [9] PCIE_LTSSM_IN_L1_1
        uint32_t PCIE_LTSSM_IN_L1_2          :1;      ///<BIT [10] PCIE_LTSSM_IN_L1_2
        uint32_t APP_XFER_PENDING_FW         :1;      ///<BIT [11] APP_XFER_PENDING_FW
        uint32_t APP_XFER_PENDING_EN         :1;      ///<BIT [12] APP_XFER_PENDING_EN
        uint32_t RX_LANE_FLIP_EN             :1;      ///<BIT [13] RX_LANE_FLIP_EN
        uint32_t TX_LANE_FLIP_EN             :1;      ///<BIT [14] TX_LANE_FLIP_EN
        uint32_t RSVD_15                     :1;      ///<BIT [15] rsvd_15
        uint32_t RESERVED_1                  :10;     ///<BIT [25:16] reserved_1
        uint32_t CLR_LTSSM_EN_HR             :1;      ///<BIT [26] CLR_LTSSM_EN_HR
        uint32_t AXI_PAR_INJ_ERR             :1;      ///<BIT [27] AXI_PAR_INJ_ERR
        uint32_t AXI_PAR_EN                  :1;      ///<BIT [28] AXI_PAR_EN
        uint32_t RESERVED_0                  :1;      ///<BIT [29] reserved_0
        uint32_t AUTO_L1_EXIT_MSI_EN         :1;      ///<BIT [30] AUTO_L1_EXIT_MSI_EN
        uint32_t AUTO_L1_EXIT_EN             :1;      ///<BIT [31] AUTO_L1_EXIT_EN
    } b;
} PcieCoreGlobal1_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FORCE_PREADY_DBI_EN         :1;      ///<BIT [0] FORCE_PREADY_DBI_EN
        uint32_t RESERVED_9                  :3;      ///<BIT [3:1] reserved_9
        uint32_t CFG_VF_EN                   :1;      ///<BIT [4] CFG_VF_EN
        uint32_t RESERVED_8                  :3;      ///<BIT [7:5] reserved_8
        uint32_t RESERVED_7                  :2;      ///<BIT [9:8] reserved_7
        uint32_t CFG_RCB                     :1;      ///<BIT [10] CFG_RCB
        uint32_t CFG_PM_NO_SOFT_RST          :1;      ///<BIT [11] CFG_PM_NO_SOFT_RST
        uint32_t CFG_MEM_SPACE_EN            :1;      ///<BIT [12] CFG_MEM_SPACE_EN
        uint32_t RESERVED_6                  :1;      ///<BIT [13] reserved_6
        uint32_t CFG_MAX_RD_REQ_SIZE         :3;      ///<BIT [16:14] CFG_MAX_RD_REQ_SIZE
        uint32_t RESERVED_5                  :1;      ///<BIT [17] reserved_5
        uint32_t CFG_MAX_PAYLOAD_SIZE        :3;      ///<BIT [20:18] CFG_MAX_PAYLOAD_SIZE
        uint32_t RESERVED_4                  :1;      ///<BIT [21] reserved_4
        uint32_t CFG_LTR_M_EN                :1;      ///<BIT [22] CFG_LTR_M_EN
        uint32_t RESERVED_3                  :1;      ///<BIT [23] reserved_3
        uint32_t CFG_L1SUB_EN                :1;      ///<BIT [24] CFG_L1SUB_EN
        uint32_t RESERVED_2                  :5;      ///<BIT [29:25] reserved_2
        uint32_t RESERVED_1                  :1;      ///<BIT [30] Reserved_1
        uint32_t RESERVED_0                  :1;      ///<BIT [31] Reserved_0
    } b;
} PcieCoreCfg_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IDE_SW_RESET                :1;      ///<BIT [0] IDE_SW_RESET
        uint32_t IDE_TEST_EN                 :1;      ///<BIT [1] IDE_TEST_EN
        uint32_t IDE_AES_MEM_ZERO_INIT       :1;      ///<BIT [2] IDE_AES_MEM_ZERO_INIT
        uint32_t IDE_AES_TX_MEM_ZERO_DONE    :1;      ///<BIT [3] IDE_AES_TX_MEM_ZERO_DONE
        uint32_t IDE_AES_RX_MEM_ZERO_DONE    :2;      ///<BIT [5:4] IDE_AES_RX_MEM_ZERO_DONE
        uint32_t IDE_TX_REKEY_REQ_STAT       :2;      ///<BIT [7:6] IDE_TX_REKEY_REQ_STAT
        uint32_t IDE_RX_REKEY_REQ_STAT       :2;      ///<BIT [9:8] IDE_RX_REKEY_REQ_STAT
        uint32_t RESERVED_0                  :22;     ///<BIT [31:10] reserved_0
    } b;
} IdeTestCtrl_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IDE_LINK_STREAM_STATUS      :4;      ///<BIT [3:0] IDE_LINK_STREAM_STATUS
        uint32_t IDE_SELECTIVE_STREAM_STATUS :4;      ///<BIT [7:4] IDE_SELECTIVE_STREAM_STATUS
        uint32_t RESERVED_0                  :24;     ///<BIT [31:8] reserved_0
    } b;
} IdeStreamStatus_t;

/// @brief 0x68
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_0                  :1;      ///<BIT [0] reserved_0
        uint32_t CFG_PBUS_NUM                :8;      ///<BIT [8:1] CFG_PBUS_NUM
        uint32_t CFG_PBUS_DEV_NUM            :5;      ///<BIT [13:9] CFG_PBUS_DEV_NUM
        uint32_t RESERVED_1                  :2;      ///<BIT [15:14] reserved_1
        uint32_t CFG_NUM_VF                  :16;     ///<BIT [31:16] CFG_NUM_VF
    } b;
} PcieCoreGeneralCfg_t;

/// @brief 0x74
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_0                  :16;     ///<BIT [15:0] reserved_0
        uint32_t CXPL_DEBUG_INFO_EI          :16;     ///<BIT [31:16] CXPL_DEBUG_INFO_EI
    } b;
} CxplDebugInfoEi_t;

/// @brief 0x80
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RADM_TIMEOUT_CPL_LEN        :12;     ///<BIT [11:0] RADM_TIMEOUT_CPL_LEN
        uint32_t RADM_TIMEOUT_CPL_ATTR       :2;      ///<BIT [13:12] RADM_TIMEOUT_CPL_ATTR
        uint32_t RADM_QOVERFLOW              :1;      ///<BIT [14] RADM_QOVERFLOW
        uint32_t RADM_Q_NOT_EMPTY            :1;      ///<BIT [15] RADM_Q_NOT_EMPTY
        uint32_t RADM_MSG_REQ_ID             :16;     ///<BIT [31:16] RADM_MSG_REQ_ID
    } b;
} PcieCoreRadmGeneral0_t;

/// @brief 0x84
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RADM_TIMEOUT_CPL_TAG        :10;     ///<BIT [9:0] RADM_TIMEOUT_CPL_TAG
        uint32_t RADM_TIMEOUT_VFUNC_NUM      :6;      ///<BIT [15:10] RADM_TIMEOUT_VFUNC_NUM
        uint32_t RADM_CPL_TIMEOUT            :1;      ///<BIT [16] RADM_CPL_TIMEOUT
        uint32_t RADM_VENDOR_MSG             :2;      ///<BIT [18:17] RADM_VENDOR_MSG
        uint32_t RADM_TIMEOUT_VFUNC_ACTIVE   :1;      ///<BIT [19] RADM_TIMEOUT_VFUNC_ACTIVE
        uint32_t RADMX_CMPOSER_LOOKUP_ERR    :1;      ///<BIT [20] RADMX_CMPOSER_LOOKUP_ERR
        uint32_t RADM_TIMEOUT_CPL_TC         :3;      ///<BIT [23:21] RADM_TIMEOUT_CPL_TC
        uint32_t RADM_TIMEOUT_FUNC_NUM       :1;      ///<BIT [24] RADM_TIMEOUT_FUNC_NUM
        uint32_t RESERVED_0                  :7;      ///<BIT [31:25] Reserved_0
    } b;
} PcieCoreRadmGeneral1_t;

/// @brief 0x88
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_0                  :10;     ///<BIT [9:0] reserved_0
        uint32_t TRGT_LOOKUP_ID              :10;     ///<BIT [19:10] TRGT_LOOKUP_ID
        uint32_t TRGT_TIMEOUT_LOOKUP_ID      :10;     ///<BIT [29:20] TRGT_TIMEOUT_LOOKUP_ID
        uint32_t TRGT_LOOKUP_EMPTY           :1;      ///<BIT [30] TRGT_LOOKUP_EMPTY
        uint32_t TRGT_CPL_TIMEOUT            :1;      ///<BIT [31] TRGT_CPL_TIMEOUT
    } b;
} PcieCoreTargetGeneral0_t;

/// @brief 0x8C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TRGT_TIMEOUT_CPL_FUNC_NUM   :1;      ///<BIT [0] TRGT_TIMEOUT_CPL_FUNC_NUM
        uint32_t RESERVED_4                  :3;      ///<BIT [3:1] reserved_4
        uint32_t TRGT_TIMEOUT_CPL_VFUNC_NUM  :6;      ///<BIT [9:4] TRGT_TIMEOUT_CPL_VFUNC_NUM
        uint32_t TRGT_TIMEOUT_CPL_VFUNC_ACTIVE :1;      ///<BIT [10] TRGT_TIMEOUT_CPL_VFUNC_ACTIVE
        uint32_t TRGT_TIMEOUT_CPL_TC         :3;      ///<BIT [13:11] TRGT_TIMEOUT_CPL_TC
        uint32_t RESERVED_3                  :1;      ///<BIT [14] reserved_3
        uint32_t TRGT_TIMEOUT_CPL_LEN        :12;     ///<BIT [26:15] TRGT_TIMEOUT_CPL_LEN
        uint32_t RESERVED_2                  :1;      ///<BIT [27] reserved_2
        uint32_t RESERVED_1                  :1;      ///<BIT [28] Reserved_1
        uint32_t RESERVED_0                  :1;      ///<BIT [29] reserved_0
        uint32_t TRGT_TIMEOUT_CPL_ATTR       :2;      ///<BIT [31:30] TRGT_TIMEOUT_CPL_ATTR
    } b;
} PcieCoreTargetGeneral1_t;

/// @brief 0xA0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_2                  :3;      ///<BIT [2:0] Reserved_2
        uint32_t VEN_MSG_FUNC_NUM            :1;      ///<BIT [3] VEN_MSG_FUNC_NUM
        uint32_t RESERVED_3                  :4;      ///<BIT [7:4] reserved_3
        uint32_t VEN_MSG_REQ                 :1;      ///<BIT [8] VEN_MSG_REQ
        uint32_t VEN_MSG_LEN                 :10;     ///<BIT [18:9] VEN_MSG_LEN
        uint32_t RESERVED_1                  :1;      ///<BIT [19] reserved_1
        uint32_t RESERVED_0                  :1;      ///<BIT [20] Reserved_0
        uint32_t VEN_MSG_FMT                 :2;      ///<BIT [22:21] VEN_MSG_FMT
        uint32_t VEN_MSG_EP                  :1;      ///<BIT [23] VEN_MSG_EP
        uint32_t VEN_MSG_CODE                :8;      ///<BIT [31:24] VEN_MSG_CODE
    } b;
} PcieCoreVendorMessageGeneral0_t;

/// @brief 0xA4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VEN_MSG_TAG                 :10;     ///<BIT [9:0] VEN_MSG_TAG
        uint32_t RESERVED_2                  :1;      ///<BIT [10] Reserved_2
        uint32_t VEN_MSG_VFUNC_NUM           :6;      ///<BIT [16:11] VEN_MSG_VFUNC_NUM
        uint32_t RESERVED_1                  :2;      ///<BIT [18:17] Reserved_1
        uint32_t VEN_MSG_ATTR                :2;      ///<BIT [20:19] VEN_MSG_ATTR
        uint32_t RESERVED_0                  :1;      ///<BIT [21] Reserved_0
        uint32_t VEN_MSG_VFUNC_ACTIVE        :1;      ///<BIT [22] VEN_MSG_VFUNC_ACTIVE
        uint32_t VEN_MSG_TYPE                :5;      ///<BIT [27:23] VEN_MSG_TYPE
        uint32_t VEN_MSG_TD                  :1;      ///<BIT [28] VEN_MSG_TD
        uint32_t VEN_MSG_TC                  :3;      ///<BIT [31:29] VEN_MSG_TC
    } b;
} PcieCoreVendorMessageGeneral1_t;

/// @brief 0xA8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_4                  :13;     ///<BIT [12:0] reserved_4
        uint32_t CFG_LINK_EQ_REQ_INT_STATUS  :1;      ///<BIT [13] CFG_LINK_EQ_REQ_INT_STATUS
        uint32_t RESERVED_3                  :1;      ///<BIT [14] reserved_3
        uint32_t RESERVED_2                  :1;      ///<BIT [15] Reserved_2
        uint32_t RESERVED_1                  :14;     ///<BIT [29:16] reserved_1
        uint32_t RESERVED_0                  :2;      ///<BIT [31:30] reserved_0
    } b;
} PcieCoreGlobal2_t;

/// @brief 0xAC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HOT_RST_LINK_DIS_TRI_EN     :1;      ///<BIT [0] HOT_RST_LINK_DIS_TRI_EN
        uint32_t OVRD_PSND_CPL_DIS           :1;      ///<BIT [1] OVRD_PSND_CPL_DIS
        uint32_t RESERVED_0                  :30;     ///<BIT [31:2] reserved_0
    } b;
} PcieCoreGlobal3_t;

/// @brief 0xD4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FLR_PF_INT                  :1;      ///<BIT [0] FLR_PF_INT
        uint32_t CFG_LINK_EQ_REQ_INT_STATUS  :1;      ///<BIT [1] CFG_LINK_EQ_REQ_INT_STATUS
        uint32_t CFG_BUS_MASTER_EN_INT       :1;      ///<BIT [2] CFG_BUS_MASTER_EN_INT
        uint32_t LINK_REQ_RST_NOT            :1;      ///<BIT [3] LINK_REQ_RST_NOT
        uint32_t RADM_PM_TURNOFF             :1;      ///<BIT [4] RADM_PM_TURNOFF
        uint32_t D3HOT_EXIT                  :1;      ///<BIT [5] D3HOT_EXIT
        uint32_t D3HOT_ENTER                 :1;      ///<BIT [6] D3HOT_ENTER
        uint32_t VPD_INT                     :1;      ///<BIT [7] VPD_INT
        uint32_t ERROR_INT                   :1;      ///<BIT [8] ERROR_INT
        uint32_t FLR_VF_INT                  :1;      ///<BIT [9] FLR_VF_INT
        uint32_t DPA_SUBSTATE_UPDATE_INT     :1;      ///<BIT [10] DPA_SUBSTATE_UPDATE_INT
        uint32_t SMLH_REQ_RST_NOT            :1;      ///<BIT [11] SMLH_REQ_RST_NOT
        uint32_t LTSSM_HR_DB_DET             :1;      ///<BIT [12] LTSSM_HR_DB_DET
        uint32_t RESERVED_1                  :1;      ///<BIT [13] reserved_1
        uint32_t RADM_VENDOR_MSG             :1;      ///<BIT [14] RADM_VENDOR_MSG
        uint32_t PM_L1_ENTRY_STARTED         :1;      ///<BIT [15] PM_L1_ENTRY_STARTED
        uint32_t RESERVED_0                  :3;      ///<BIT [18:16] reserved_0
        uint32_t RDLH_LINK_UP                :1;      ///<BIT [19] RDLH_LINK_UP
        uint32_t RDLH_LINK_DOWN              :1;      ///<BIT [20] RDLH_LINK_DOWN
        uint32_t TDISP_INT                   :1;      ///<BIT [21] TDISP_INT
        uint32_t ERROR_INT2                  :1;      ///<BIT [22] ERROR_INT2
        uint32_t IDE_WA_DET                  :1;      ///<BIT [23] IDE_WA_DET
        uint32_t PWR_BUDGET                  :1;      ///<BIT [24] PWR_BUDGET
        uint32_t LTSSM_L2_EXIT               :1;      ///<BIT [25] LTSSM_L2_EXIT
        uint32_t RSVD_26_29                  :4;      ///<BIT [29:26] rsvd_26_29
        uint32_t LTSSM_L1_EXIT               :1;      ///<BIT [30] LTSSM_L1_EXIT
        uint32_t LTSSM_L1_ENTER              :1;      ///<BIT [31] LTSSM_L1_ENTER
    } b;
} PcieCoreIntStatus_t;

/// @brief 0xD8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FLR_PF_ACTIVE_EN            :1;      ///<BIT [0] FLR_PF_ACTIVE_EN
        uint32_t CFG_LINK_EQ_REQ_INT_STATUS_EN :1;      ///<BIT [1] CFG_LINK_EQ_REQ_INT_STATUS_EN
        uint32_t CFG_BUS_MASTER_EN_INT_EN    :1;      ///<BIT [2] CFG_BUS_MASTER_EN_INT_EN
        uint32_t LINK_REQ_RST_NOT_EN         :1;      ///<BIT [3] LINK_REQ_RST_NOT_EN
        uint32_t RADM_PM_TURNOFF_EN          :1;      ///<BIT [4] RADM_PM_TURNOFF_EN
        uint32_t D3HOT_EXIT_EN               :1;      ///<BIT [5] D3HOT_EXIT_EN
        uint32_t D3HOT_ENTER_EN              :1;      ///<BIT [6] D3HOT_ENTER_EN
        uint32_t VPD_EN                      :1;      ///<BIT [7] VPD_EN
        uint32_t ERROR_EN                    :1;      ///<BIT [8] ERROR_EN
        uint32_t FLR_VF_EN                   :1;      ///<BIT [9] FLR_VF_EN
        uint32_t DPA_SUBSTATE_UPDATE_INT_EN  :1;      ///<BIT [10] DPA_SUBSTATE_UPDATE_INT_EN
        uint32_t SMLH_REQ_RST_NOT_EN         :1;      ///<BIT [11] SMLH_REQ_RST_NOT_EN
        uint32_t LTSSM_HR_DB_DET_EN          :1;      ///<BIT [12] LTSSM_HR_DB_DET_EN
        uint32_t RESERVED_1                  :1;      ///<BIT [13] reserved_1
        uint32_t RADM_VENDOR_MSG_EN          :1;      ///<BIT [14] RADM_VENDOR_MSG_EN
        uint32_t PM_L1_ENTRY_STARTED_EN      :1;      ///<BIT [15] PM_L1_ENTRY_STARTED_EN
        uint32_t RESERVED_0                  :3;      ///<BIT [18:16] reserved_0
        uint32_t RDLH_LINK_UP_EN             :1;      ///<BIT [19] RDLH_LINK_UP_EN
        uint32_t RDLH_LINK_DOWN_EN           :1;      ///<BIT [20] RDLH_LINK_DOWN_EN
        uint32_t TDISP_INT_EN                :1;      ///<BIT [21] TDISP_INT_EN
        uint32_t ERROR2_EN                   :1;      ///<BIT [22] ERROR2_EN
        uint32_t IDE_WA_DET_EN               :1;      ///<BIT [23] IDE_WA_DET_EN
        uint32_t PWR_BUDGET_EN               :1;      ///<BIT [24] PWR_BUDGET_EN
        uint32_t LTSSM_L2_EXIT_EN            :1;      ///<BIT [25] LTSSM_L2_EXIT_EN
        uint32_t RSVD_26_29                  :4;      ///<BIT [29:26] rsvd_26_29
        uint32_t LTSSM_L1_EXIT_EN            :1;      ///<BIT [30] LTSSM_L1_EXIT_EN
        uint32_t LTSSM_L1_ENTER_EN           :1;      ///<BIT [31] LTSSM_L1_ENTER_EN
    } b;
} PcieCoreIntEnable_t;

/// @brief 0xDC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LTR_FW_REQ                  :1;      ///<BIT [0] LTR_FW_REQ
        uint32_t LTR_FW_VECTOR_SEL           :2;      ///<BIT [2:1] LTR_FW_VECTOR_SEL
        uint32_t RESERVED_1                  :24;     ///<BIT [26:3] reserved_1
        uint32_t LTR_LIMIT_TO_MAX            :1;      ///<BIT [27] LTR_LIMIT_TO_MAX
        uint32_t RESERVED_0                  :2;      ///<BIT [29:28] reserved_0
        uint32_t RSVD_30                     :1;      ///<BIT [30] rsvd_30
        uint32_t LTR_EN_EN                   :1;      ///<BIT [31] LTR_EN_EN
    } b;
} PcieCoreLtrMessageControl_t;

/// @brief 0xF8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RCVD_WREQ_POISONED_CNT      :8;      ///<BIT [7:0] RCVD_WREQ_POISONED_CNT
        uint32_t RCVD_CPL_POISONED_CNT       :8;      ///<BIT [15:8] RCVD_CPL_POISONED_CNT
        uint32_t MLF_TLP_ERR_CNT             :8;      ///<BIT [23:16] MLF_TLP_ERR_CNT
        uint32_t ECRC_ERR_CNT                :8;      ///<BIT [31:24] ECRC_ERR_CNT
    } b;
} PcieCoreErrorCount0_t;

/// @brief 0xFC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RCVD_REQ_UR_CNT             :8;      ///<BIT [7:0] RCVD_REQ_UR_CNT
        uint32_t RCVD_REQ_CA_CNT             :8;      ///<BIT [15:8] RCVD_REQ_CA_CNT
        uint32_t RCVD_CPL_CA_CNT             :8;      ///<BIT [23:16] RCVD_CPL_CA_CNT
        uint32_t RCVD_CPL_UR_CNT             :8;      ///<BIT [31:24] RCVD_CPL_UR_CNT
    } b;
} PcieCoreErrorCount1_t;

/// @brief 0x100
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UNEXP_CPL_ERR_CNT           :8;      ///<BIT [7:0] UNEXP_CPL_ERR_CNT
        uint32_t QOVERFLOW_CNT               :8;      ///<BIT [15:8] QOVERFLOW_CNT
        uint32_t CPL_TIMEOUT_CNT             :8;      ///<BIT [23:16] CPL_TIMEOUT_CNT
        uint32_t DLLP_PROT_ERR_CNT           :8;      ///<BIT [31:24] DLLP_PROT_ERR_CNT
    } b;
} PcieCoreErrorCount2_t;

/// @brief 0x110
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t APP_DRS_READY               :1;      ///<BIT [0] APP_DRS_READY
        uint32_t RESERVED_1                  :1;      ///<BIT [1] Reserved_1
        uint32_t RESERVED_0                  :30;     ///<BIT [31:2] reserved_0
    } b;
} PcieCoreDrsFrsReady_t;

/// @brief 0x138
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PWR_BUDGET_SEL              :8;      ///<BIT [7:0] PWR_BUDGET_SEL
        uint32_t PWR_BUDGET_FNUM             :1;      ///<BIT [8] PWR_BUDGET_FNUM
        uint32_t RESERVED_0                  :23;     ///<BIT [31:9] reserved_0
    } b;
} PowerBudgetControl_t;

/// @brief 0x160
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ONEP_RBUF_WTSEL             :2;      ///<BIT [1:0] ONEP_RBUF_WTSEL
        uint32_t ONEP_RBUF_RTSEL             :2;      ///<BIT [3:2] ONEP_RBUF_RTSEL
        uint32_t TWOP_SOTBUF_WTC             :2;      ///<BIT [5:4] TWOP_SOTBUF_WTC
        uint32_t TWOP_SOTBUF_RTC             :2;      ///<BIT [7:6] TWOP_SOTBUF_RTC
        uint32_t TWOP_SOTBUF_KP              :3;      ///<BIT [10:8] TWOP_SOTBUF_KP
        uint32_t RADM_QBUFFER_HDR_WTSEL      :2;      ///<BIT [12:11] RADM_QBUFFER_HDR_WTSEL
        uint32_t RADM_QBUFFER_HDR_RTSEL      :2;      ///<BIT [14:13] RADM_QBUFFER_HDR_RTSEL
        uint32_t RADM_QBUFFER_HDR_MTSEL      :2;      ///<BIT [16:15] RADM_QBUFFER_HDR_MTSEL
        uint32_t RADM_QBUFFER_DATA_WTSEL     :2;      ///<BIT [18:17] RADM_QBUFFER_DATA_WTSEL
        uint32_t RADM_QBUFFER_DATA_RTSEL     :2;      ///<BIT [20:19] RADM_QBUFFER_DATA_RTSEL
        uint32_t RADM_QBUFFER_DATA_MTSEL     :2;      ///<BIT [22:21] RADM_QBUFFER_DATA_MTSEL
        uint32_t RASDES_EC_WTC               :2;      ///<BIT [24:23] RASDES_EC_WTC
        uint32_t RASDES_EC_RTC               :2;      ///<BIT [26:25] RASDES_EC_RTC
        uint32_t RASDES_EC_KP                :3;      ///<BIT [29:27] RASDES_EC_KP
        uint32_t RESERVED_0                  :2;      ///<BIT [31:30] reserved_0
    } b;
} MemoryRtcWtc1_t;

/// @brief 0x164
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RASDES_TBA_WTC              :2;      ///<BIT [1:0] RASDES_TBA_WTC
        uint32_t RASDES_TBA_RTC              :2;      ///<BIT [3:2] RASDES_TBA_RTC
        uint32_t RASDES_TBA_KP               :3;      ///<BIT [6:4] RASDES_TBA_KP
        uint32_t MCPL_SB_RAM_WTC             :2;      ///<BIT [8:7] MCPL_SB_RAM_WTC
        uint32_t MCPL_SB_RAM_RTC             :2;      ///<BIT [10:9] MCPL_SB_RAM_RTC
        uint32_t MCPL_SB_RAM_KP              :3;      ///<BIT [13:11] MCPL_SB_RAM_KP
        uint32_t MCPL_A2C_CDC_RAM_WTC        :2;      ///<BIT [15:14] MCPL_A2C_CDC_RAM_WTC
        uint32_t MCPL_A2C_CDC_RAM_RTC        :2;      ///<BIT [17:16] MCPL_A2C_CDC_RAM_RTC
        uint32_t MCPL_A2C_CDC_RAM_KP         :3;      ///<BIT [20:18] MCPL_A2C_CDC_RAM_KP
        uint32_t WREQ_C2A_CDC_WTC            :2;      ///<BIT [22:21] WREQ_C2A_CDC_WTC
        uint32_t WREQ_C2A_CDC_RTC            :2;      ///<BIT [24:23] WREQ_C2A_CDC_RTC
        uint32_t WREQ_C2A_CDC_KP             :3;      ///<BIT [27:25] WREQ_C2A_CDC_KP
        uint32_t RESERVED_0                  :4;      ///<BIT [31:28] reserved_0
    } b;
} MemoryRtcWtc2_t;

/// @brief 0x168
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RREQ_C2A_CDC_WTC            :2;      ///<BIT [1:0] RREQ_C2A_CDC_WTC
        uint32_t RREQ_C2A_CDC_RTC            :2;      ///<BIT [3:2] RREQ_C2A_CDC_RTC
        uint32_t RREQ_C2A_CDC_KP             :3;      ///<BIT [6:4] RREQ_C2A_CDC_KP
        uint32_t RREQ_ORDR_WTC               :2;      ///<BIT [8:7] RREQ_ORDR_WTC
        uint32_t RREQ_ORDR_RTC               :2;      ///<BIT [10:9] RREQ_ORDR_RTC
        uint32_t RREQ_ORDR_KP                :3;      ///<BIT [13:11] RREQ_ORDR_KP
        uint32_t SLV_NPW_SAB_WTC             :2;      ///<BIT [15:14] SLV_NPW_SAB_WTC
        uint32_t SLV_NPW_SAB_RTC             :2;      ///<BIT [17:16] SLV_NPW_SAB_RTC
        uint32_t SLV_NPW_SAB_KP              :3;      ///<BIT [20:18] SLV_NPW_SAB_KP
        uint32_t OB_PDCMP_HDR_WTC            :2;      ///<BIT [22:21] OB_PDCMP_HDR_WTC
        uint32_t OB_PDCMP_HDR_RTC            :2;      ///<BIT [24:23] OB_PDCMP_HDR_RTC
        uint32_t OB_PDCMP_HDR_KP             :3;      ///<BIT [27:25] OB_PDCMP_HDR_KP
        uint32_t RESERVED_0                  :4;      ///<BIT [31:28] reserved_0
    } b;
} MemoryRtcWtc3_t;

/// @brief 0x16C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VFUNC_NUM                   :6;      ///<BIT [5:0] VFUNC_NUM
        uint32_t RESERVED_0                  :26;     ///<BIT [31:6] reserved_0
    } b;
} FunctionSelection_t;

/// @brief 0x200
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAD_DLLP_SATUS              :1;      ///<BIT [0] BAD_DLLP_SATUS
        uint32_t BAD_TLP_SATUS               :1;      ///<BIT [1] BAD_TLP_SATUS
        uint32_t RCV_ERR_SATUS               :1;      ///<BIT [2] RCV_ERR_SATUS
        uint32_t REPLAY_TIMEOUT_SATUS        :1;      ///<BIT [3] REPLAY_TIMEOUT_SATUS
        uint32_t REPLAY_ROLLOVER_SATUS       :1;      ///<BIT [4] REPLAY_ROLLOVER_SATUS
        uint32_t FC_PROT_SATUS               :1;      ///<BIT [5] FC_PROT_SATUS
        uint32_t DLLP_PROT_SATUS             :1;      ///<BIT [6] DLLP_PROT_SATUS
        uint32_t CPL_TIMEOUT_SATUS           :1;      ///<BIT [7] CPL_TIMEOUT_SATUS
        uint32_t QOVERFLOW_SATUS             :1;      ///<BIT [8] QOVERFLOW_SATUS
        uint32_t UNEXP_CPL_SATUS             :1;      ///<BIT [9] UNEXP_CPL_SATUS
        uint32_t CPL_UR_SATUS                :1;      ///<BIT [10] CPL_UR_SATUS
        uint32_t CPL_CA_SATUS                :1;      ///<BIT [11] CPL_CA_SATUS
        uint32_t RCVD_REQ_CA_SATUS           :1;      ///<BIT [12] RCVD_REQ_CA_SATUS
        uint32_t RCVD_REQ_UR_SATUS           :1;      ///<BIT [13] RCVD_REQ_UR_SATUS
        uint32_t ECRC_SATUS                  :1;      ///<BIT [14] ECRC_SATUS
        uint32_t MALFORMED_TLP_SATUS         :1;      ///<BIT [15] MALFORMED_TLP_SATUS
        uint32_t CPL_POISONED_SATUS          :1;      ///<BIT [16] CPL_POISONED_SATUS
        uint32_t WREQ_POISONED_SATUS         :1;      ///<BIT [17] WREQ_POISONED_SATUS
        uint32_t SLV_DP_ERR_SATUS            :1;      ///<BIT [18] SLV_DP_ERR_SATUS
        uint32_t MSTR_DP_ERR_SATUS           :1;      ///<BIT [19] MSTR_DP_ERR_SATUS
        uint32_t APP_PARITY_ERR_SATUS        :3;      ///<BIT [22:20] APP_PARITY_ERR_SATUS
        uint32_t RESERVED_0                  :9;      ///<BIT [31:23] reserved_0
    } b;
} ErrorStatus_t;

/// @brief 0x204
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAD_DLLP_EN                 :1;      ///<BIT [0] BAD_DLLP_EN
        uint32_t BAD_TLP_EN                  :1;      ///<BIT [1] BAD_TLP_EN
        uint32_t RCV_ERR_EN                  :1;      ///<BIT [2] RCV_ERR_EN
        uint32_t REPLAY_TIMEOUT_EN           :1;      ///<BIT [3] REPLAY_TIMEOUT_EN
        uint32_t REPLAY_ROLLOVER_EN          :1;      ///<BIT [4] REPLAY_ROLLOVER_EN
        uint32_t FC_PROT_EN                  :1;      ///<BIT [5] FC_PROT_EN
        uint32_t DLLP_PROT_EN                :1;      ///<BIT [6] DLLP_PROT_EN
        uint32_t CPL_TIMEOUT_EN              :1;      ///<BIT [7] CPL_TIMEOUT_EN
        uint32_t QOVERFLOW_EN                :1;      ///<BIT [8] QOVERFLOW_EN
        uint32_t UNEXP_CPL_EN                :1;      ///<BIT [9] UNEXP_CPL_EN
        uint32_t CPL_UR_EN                   :1;      ///<BIT [10] CPL_UR_EN
        uint32_t CPL_CA_EN                   :1;      ///<BIT [11] CPL_CA_EN
        uint32_t RCVD_REQ_CA_EN              :1;      ///<BIT [12] RCVD_REQ_CA_EN
        uint32_t RCVD_REQ_UR_EN              :1;      ///<BIT [13] RCVD_REQ_UR_EN
        uint32_t ECRC_EN                     :1;      ///<BIT [14] ECRC_EN
        uint32_t MALFORMED_TLP_EN            :1;      ///<BIT [15] MALFORMED_TLP_EN
        uint32_t CPL_POISONED_EN             :1;      ///<BIT [16] CPL_POISONED_EN
        uint32_t WREQ_POISONED_EN            :1;      ///<BIT [17] WREQ_POISONED_EN
        uint32_t SLV_DP_ERR_EN               :1;      ///<BIT [18] SLV_DP_ERR_EN
        uint32_t MSTR_DP_ERR_EN              :1;      ///<BIT [19] MSTR_DP_ERR_EN
        uint32_t APP_PARITY_ERR_EN           :3;      ///<BIT [22:20] APP_PARITY_ERR_EN
        uint32_t RESERVED_0                  :9;      ///<BIT [31:23] reserved_0
    } b;
} ErrorStatusIntEn_t;

/// @brief 0x238
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IDE_CKEY_TX_WTC             :2;      ///<BIT [1:0] IDE_CKEY_TX_WTC
        uint32_t IDE_CKEY_TX_RTC             :2;      ///<BIT [3:2] IDE_CKEY_TX_RTC
        uint32_t IDE_CKEY_TX_KP              :3;      ///<BIT [6:4] IDE_CKEY_TX_KP
        uint32_t IDE_HKEY_TX_WTC             :2;      ///<BIT [8:7] IDE_HKEY_TX_WTC
        uint32_t IDE_HKEY_TX_RTC             :2;      ///<BIT [10:9] IDE_HKEY_TX_RTC
        uint32_t IDE_HKEY_TX_KP              :3;      ///<BIT [13:11] IDE_HKEY_TX_KP
        uint32_t IDE_CKEY_RX_WTC             :2;      ///<BIT [15:14] IDE_CKEY_RX_WTC
        uint32_t IDE_CKEY_RX_RTC             :2;      ///<BIT [17:16] IDE_CKEY_RX_RTC
        uint32_t IDE_CKEY_RX_KP              :3;      ///<BIT [20:18] IDE_CKEY_RX_KP
        uint32_t IDE_HKEY_RX_WTC             :2;      ///<BIT [22:21] IDE_HKEY_RX_WTC
        uint32_t IDE_HKEY_RX_RTC             :2;      ///<BIT [24:23] IDE_HKEY_RX_RTC
        uint32_t IDE_HKEY_RX_KP              :3;      ///<BIT [27:25] IDE_HKEY_RX_KP
        uint32_t RESERVED_0                  :4;      ///<BIT [31:28] reserved_0
    } b;
} IdeSramRtcWtc1_t;

/// @brief 0x23C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IDE_CKEY_RX2_WTC            :2;      ///<BIT [1:0] IDE_CKEY_RX2_WTC
        uint32_t IDE_CKEY_RX2_RTC            :2;      ///<BIT [3:2] IDE_CKEY_RX2_RTC
        uint32_t IDE_CKEY_RX2_KP             :3;      ///<BIT [6:4] IDE_CKEY_RX2_KP
        uint32_t IDE_HKEY_RX2_WTC            :2;      ///<BIT [8:7] IDE_HKEY_RX2_WTC
        uint32_t IDE_HKEY_RX2_RTC            :2;      ///<BIT [10:9] IDE_HKEY_RX2_RTC
        uint32_t IDE_HKEY_RX2_KP             :3;      ///<BIT [13:11] IDE_HKEY_RX2_KP
        uint32_t WREQ_PTRK_DATA_WTSEL        :2;      ///<BIT [15:14] WREQ_PTRK_DATA_WTSEL
        uint32_t WREQ_PTRK_DATA_RTSEL        :2;      ///<BIT [17:16] WREQ_PTRK_DATA_RTSEL
        uint32_t WREQ_PTRK_DATA_MTSEL        :2;      ///<BIT [19:18] WREQ_PTRK_DATA_MTSEL
        uint32_t WREQ_PTRK_HDR_WTSEL         :2;      ///<BIT [21:20] WREQ_PTRK_HDR_WTSEL
        uint32_t WREQ_PTRK_HDR_RTSEL         :2;      ///<BIT [23:22] WREQ_PTRK_HDR_RTSEL
        uint32_t WREQ_PTRK_HDR_MTSEL         :2;      ///<BIT [25:24] WREQ_PTRK_HDR_MTSEL
        uint32_t RESERVED_0                  :6;      ///<BIT [31:26] reserved_0
    } b;
} IdeSramRtcWtc2_t;

/// @brief 0x240
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_RCVR_ERR_STS            :1;      ///<BIT [0] CFG_RCVR_ERR_STS
        uint32_t CFG_BAD_TLP_ERR_STS         :1;      ///<BIT [1] CFG_BAD_TLP_ERR_STS
        uint32_t CFG_BAD_DLLP_ERR_STS        :1;      ///<BIT [2] CFG_BAD_DLLP_ERR_STS
        uint32_t CFG_REPLAY_TIMER_TIMEOUT_ERR_STS :1;      ///<BIT [3] CFG_REPLAY_TIMER_TIMEOUT_ERR_STS
        uint32_t CFG_REPLAY_NUMBER_ROLLOVER_ERR_STS :1;      ///<BIT [4] CFG_REPLAY_NUMBER_ROLLOVER_ERR_STS
        uint32_t CFG_CORRECTED_INTERNAL_ERR_STS :1;      ///<BIT [5] CFG_CORRECTED_INTERNAL_ERR_STS
        uint32_t CFG_ECRC_ERR_STS            :1;      ///<BIT [6] CFG_ECRC_ERR_STS
        uint32_t CFG_DL_PROTOCOL_ERR_STS     :1;      ///<BIT [7] CFG_DL_PROTOCOL_ERR_STS
        uint32_t CFG_SURPRISE_DOWN_ER_STS    :1;      ///<BIT [8] CFG_SURPRISE_DOWN_ER_STS
        uint32_t CFG_MLF_TLP_ERR_STS         :1;      ///<BIT [9] CFG_MLF_TLP_ERR_STS
        uint32_t CFG_FC_PROTOCOL_ERR_STS     :1;      ///<BIT [10] CFG_FC_PROTOCOL_ERR_STS
        uint32_t CFG_RCVR_OVERFLOW_ERR_STS   :1;      ///<BIT [11] CFG_RCVR_OVERFLOW_ERR_STS
        uint32_t CFG_UNCOR_INTERNAL_ERR_STS  :1;      ///<BIT [12] CFG_UNCOR_INTERNAL_ERR_STS
        uint32_t RESERVED0                   :19;     ///<BIT [31:13] reserved0
    } b;
} ErrorStatus2_t;

/// @brief 0x244
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_RCVR_ERR_STS_EN         :1;      ///<BIT [0] CFG_RCVR_ERR_STS_EN
        uint32_t CFG_BAD_TLP_ERR_STS_EN      :1;      ///<BIT [1] CFG_BAD_TLP_ERR_STS_EN
        uint32_t CFG_BAD_DLLP_ERR_STS_EN     :1;      ///<BIT [2] CFG_BAD_DLLP_ERR_STS_EN
        uint32_t CFG_REPLAY_TIMER_TIMEOUT_ERR_STS_EN :1;      ///<BIT [3] CFG_REPLAY_TIMER_TIMEOUT_ERR_STS_EN
        uint32_t CFG_REPLAY_NUMBER_ROLLOVER_ERR_STS_EN :1;      ///<BIT [4] CFG_REPLAY_NUMBER_ROLLOVER_ERR_STS_EN
        uint32_t CFG_CORRECTED_INTERNAL_ERR_STS_EN :1;      ///<BIT [5] CFG_CORRECTED_INTERNAL_ERR_STS_EN
        uint32_t CFG_ECRC_ERR_STS_EN         :1;      ///<BIT [6] CFG_ECRC_ERR_STS_EN
        uint32_t CFG_DL_PROTOCOL_ERR_STS_EN  :1;      ///<BIT [7] CFG_DL_PROTOCOL_ERR_STS_EN
        uint32_t CFG_SURPRISE_DOWN_ER_STS_EN :1;      ///<BIT [8] CFG_SURPRISE_DOWN_ER_STS_EN
        uint32_t CFG_MLF_TLP_ERR_STS_EN      :1;      ///<BIT [9] CFG_MLF_TLP_ERR_STS_EN
        uint32_t CFG_FC_PROTOCOL_ERR_STS_EN  :1;      ///<BIT [10] CFG_FC_PROTOCOL_ERR_STS_EN
        uint32_t CFG_RCVR_OVERFLOW_ERR_STS_EN :1;      ///<BIT [11] CFG_RCVR_OVERFLOW_ERR_STS_EN
        uint32_t CFG_UNCOR_INTERNAL_ERR_STS_EN :1;      ///<BIT [12] CFG_UNCOR_INTERNAL_ERR_STS_EN
        uint32_t RESERVED0                   :19;     ///<BIT [31:13] reserved0
    } b;
} ErrorStatus2IntEn_t;

/// @brief 0x300
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_VF_MSIX_TABLE_OFFSET    :29;     ///<BIT [28:0] CFG_VF_MSIX_TABLE_OFFSET
        uint32_t CFG_VF_MSIX_TABLE_BIR       :3;      ///<BIT [31:29] CFG_VF_MSIX_TABLE_BIR
    } b;
} CfgVfMsixTblStatus_t;

/// @brief 0x304
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_VF_MSIX_PBA_OFFSET      :29;     ///<BIT [28:0] CFG_VF_MSIX_PBA_OFFSET
        uint32_t CFG_VF_MSIX_PBA_BIR         :3;      ///<BIT [31:29] CFG_VF_MSIX_PBA_BIR
    } b;
} CfgVfMsixPbaStatus_t;

/// @brief 0x308
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_1                  :10;     ///<BIT [9:0] Reserved_1
        uint32_t CFG_MSI_EXT_DATA_EN_PF0     :1;      ///<BIT [10] cfg_msi_ext_data_en_pf0
        uint32_t CFG_VF_MSIX_TABLE_SIZE_PF0  :11;     ///<BIT [21:11] cfg_vf_msix_table_size_pf0
        uint32_t CFG_HP_SLOT_CTRL_ACCESS_PF0 :1;      ///<BIT [22] cfg_hp_slot_ctrl_access_pf0
        uint32_t CFG_DLL_STATE_CHGED_EN_PF0  :1;      ///<BIT [23] cfg_dll_state_chged_en_pf0
        uint32_t CFG_CMD_CPLED_INT_EN_PF0    :1;      ///<BIT [24] cfg_cmd_cpled_int_en_pf0
        uint32_t CFG_HP_INT_EN_PF0           :1;      ///<BIT [25] cfg_hp_int_en_pf0
        uint32_t CFG_PRE_DET_CHGED_EN_PF0    :1;      ///<BIT [26] cfg_pre_det_chged_en_pf0
        uint32_t CFG_MRL_SENSOR_CHGED_EN_PF0 :1;      ///<BIT [27] cfg_mrl_sensor_chged_en_pf0
        uint32_t CFG_PWR_FAULT_DET_EN_PF0    :1;      ///<BIT [28] cfg_pwr_fault_det_en_pf0
        uint32_t CFG_ATTEN_BUTTON_PRESSED_EN_PF0 :1;      ///<BIT [29] cfg_atten_button_pressed_en_pf0
        uint32_t CFG_DISABLE_LTR_CLR_MSG_PF0 :1;      ///<BIT [30] cfg_disable_ltr_clr_msg_pf0
        uint32_t RESERVED_0                  :1;      ///<BIT [31] Reserved_0
    } b;
} PcieCfg1_t;

/// @brief 0x30C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t APP_L1SUB_DISABLE           :1;      ///<BIT [0] APP_L1SUB_DISABLE
        uint32_t APP_L1_PWR_OFF_EN           :1;      ///<BIT [1] APP_L1_PWR_OFF_EN
        uint32_t APP_DBI_RO_WR_DISABLE       :1;      ///<BIT [2] APP_DBI_RO_WR_DISABLE
        uint32_t APP_RAS_DES_SD_HOLD_LTSSM   :1;      ///<BIT [3] APP_RAS_DES_SD_HOLD_LTSSM
        uint32_t APP_MARGINING_READY         :1;      ///<BIT [4] APP_MARGINING_READY
        uint32_t APP_MARGINING_SOFTWARE_READY :1;      ///<BIT [5] APP_MARGINING_SOFTWARE_READY
        uint32_t APP_RAS_DES_TBA_CTRL        :2;      ///<BIT [7:6] APP_RAS_DES_TBA_CTRL
        uint32_t RESERVED_1                  :8;      ///<BIT [15:8] Reserved_1
        uint32_t PM_L1SUB_STATE              :3;      ///<BIT [18:16] PM_L1SUB_STATE
        uint32_t PM_MASTER_STATE             :5;      ///<BIT [23:19] PM_MASTER_STATE
        uint32_t PM_SLAVE_STATE              :5;      ///<BIT [28:24] PM_SLAVE_STATE
        uint32_t RESERVED_0                  :3;      ///<BIT [31:29] Reserved_0
    } b;
} MiscStatus2_t;

/// @brief 0x314
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OB_PDCMP_DATA_WTC           :2;      ///<BIT [1:0] OB_PDCMP_DATA_WTC
        uint32_t OB_PDCMP_DATA_RTC           :2;      ///<BIT [3:2] OB_PDCMP_DATA_RTC
        uint32_t OB_PDCMP_DATA_KP            :3;      ///<BIT [6:4] OB_PDCMP_DATA_KP
        uint32_t OB_NPDCMP_WTC               :2;      ///<BIT [8:7] OB_NPDCMP_WTC
        uint32_t OB_NPDCMP_RTC               :2;      ///<BIT [10:9] OB_NPDCMP_RTC
        uint32_t OB_NPDCMP_KP                :3;      ///<BIT [13:11] OB_NPDCMP_KP
        uint32_t OB_CPL_C2A_CDC_WTC          :2;      ///<BIT [15:14] OB_CPL_C2A_CDC_WTC
        uint32_t OB_CPL_C2A_CDC_RTC          :2;      ///<BIT [17:16] OB_CPL_C2A_CDC_RTC
        uint32_t OB_CPL_C2A_CDC_KP           :3;      ///<BIT [20:18] OB_CPL_C2A_CDC_KP
        uint32_t OB_CCMP_DATA_WTC            :2;      ///<BIT [22:21] OB_CCMP_DATA_WTC
        uint32_t OB_CCMP_DATA_RTC            :2;      ///<BIT [24:23] OB_CCMP_DATA_RTC
        uint32_t OB_CCMP_DATA_KP             :3;      ///<BIT [27:25] OB_CCMP_DATA_KP
        uint32_t RESERVED_0                  :3;      ///<BIT [30:28] Reserved_0
        uint32_t MEM_CG_CTRL_DIS             :1;      ///<BIT [31] mem_cg_ctrl_dis
    } b;
} SramCtrl_t;

/// @brief 0x320
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FLR_PF_ACTIVE               :1;      ///<BIT [0] FLR_PF_ACTIVE
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} FlrPfActive_t;

/// @brief 0x324
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FLR_PF_ACTIVE_INT_STATUS    :1;      ///<BIT [0] FLR_PF_ACTIVE_INT_STATUS
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} FlrPfActiveIntStatus_t;

/// @brief 0x328
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FLR_PF_ACTIVE_INT_EN        :1;      ///<BIT [0] FLR_PF_ACTIVE_INT_EN
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} FlrPfActiveIntEn_t;

/// @brief 0x32C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FLR_PF_DONE                 :1;      ///<BIT [0] FLR_PF_DONE
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} FlrPfDone_t;

/// @brief 0x330
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PM_XMT_PME                  :1;      ///<BIT [0] PM_XMT_PME
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} PmXmtPme_t;

/// @brief 0x334
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OUTBAND_PWRUP_CMD           :1;      ///<BIT [0] OUTBAND_PWRUP_CMD
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} OutbandPwrupCmd_t;

/// @brief 0x338
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LTR_MSG_FUNC_NUM            :1;      ///<BIT [0] LTR_MSG_FUNC_NUM
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} LtrMsgFuncNum_t;

/// @brief 0x33C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PM_STATUS                   :1;      ///<BIT [0] PM_STATUS
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} PmStatus_t;

/// @brief 0x344
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DPA_SUBSTATE_UPDATE_INT_STATUS :1;      ///<BIT [0] DPA_SUBSTATE_UPDATE_INT_STATUS
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} DpaSubstateUpdateIntStatus_t;

/// @brief 0x348
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DPA_SUBSTATE_UPDATE_INT_EN  :1;      ///<BIT [0] DPA_SUBSTATE_UPDATE_INT_EN
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} DpaSubstateUpdateIntEn_t;

/// @brief 0x34C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PF_FRS_READY                :1;      ///<BIT [0] PF_FRS_READY
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} PfFrsReady_t;

/// @brief 0x350
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PF_FRS_GRANT                :1;      ///<BIT [0] PF_FRS_GRANT
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} PfFrsGrant_t;

/// @brief 0x354
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_PWR_BUDGET_SEL_INT_STATUS :1;      ///<BIT [0] CFG_PWR_BUDGET_SEL_INT_STATUS
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} CfgPwrBudgetSelIntStatus_t;

/// @brief 0x358
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_PWR_BUDGET_SEL_INT_EN   :1;      ///<BIT [0] CFG_PWR_BUDGET_SEL_INT_EN
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} CfgPwrBudgetSelIntEn_t;

/// @brief 0x35C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_VPD_INT_STATUS          :1;      ///<BIT [0] CFG_VPD_INT_STATUS
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} CfgVpdIntStatus_t;

/// @brief 0x360
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_VPD_INT_EN              :1;      ///<BIT [0] CFG_VPD_INT_EN
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} CfgVpdIntEn_t;

/// @brief 0x364
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_BUS_MASTER_EN           :1;      ///<BIT [0] CFG_BUS_MASTER_EN
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} CfgBusMasterEn_t;

/// @brief 0x368
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_BUS_MASTER_EN_INT_STATUS :1;      ///<BIT [0] CFG_BUS_MASTER_EN_INT_STATUS
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} CfgBusMasterEnIntStatus_t;

/// @brief 0x36C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CFG_BUS_MASTER_EN_INT_EN    :1;      ///<BIT [0] CFG_BUS_MASTER_EN_INT_EN
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} CfgBusMasterEnIntEn_t;

/// @brief 0x370
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t D3_HOT_ENTER_INT_STATUS     :1;      ///<BIT [0] D3_HOT_ENTER_INT_STATUS
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} D3HotEnterIntStatus_t;

/// @brief 0x374
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t D3_HOT_ENTER_INT_EN         :1;      ///<BIT [0] D3_HOT_ENTER_INT_EN
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} D3HotEnterIntEn_t;

/// @brief 0x378
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t D3_HOT_EXIT_INT_STATUS      :1;      ///<BIT [0] D3_HOT_EXIT_INT_STATUS
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} D3HotExitIntStatus_t;

/// @brief 0x37C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t D3_HOT_EXIT_INT_EN          :1;      ///<BIT [0] D3_HOT_EXIT_INT_EN
        uint32_t RESERVED_0                  :31;     ///<BIT [31:1] Reserved_0
    } b;
} D3HotExitIntEn_t;

typedef struct
{
    PcieCoreGlobal0_t pcieCoreGlobal0;                                      // 0x0 : PCIe_Core_Global_0 / 
    PcieCoreGlobal1_t pcieCoreGlobal1;                                      // 0x4 : PCIe_Core_Global_1 / 
    uint32_t appLtrLatency;                                                 // 0x8 : APP_LTR_LATENCY / 
    uint32_t appLtrMsgLatency;                                              // 0xC : APP_LTR_MSG_LATENCY / 
    PcieCoreCfg_t pcieCoreCfg;                                              // 0x10 : PCI_Express_Core_Configure / 
    uint32_t appVfReqRetryEn1;                                              // 0x14 : APP_VF_REQ_RETRY_EN_1 / 
    uint32_t appVfReqRetryEn2;                                              // 0x18 : APP_VF_REQ_RETRY_EN_2 / 
    IdeTestCtrl_t ideTestCtrl;                                              // 0x1C : IDE_TEST_CTRL / 
    IdeStreamStatus_t ideStreamStatus;                                      // 0x20 : IDE_STREAM_STATUS / 
    uint8_t rsvd24[64];                                                     // 0x24 : rsvd_24 / rsvd_24
    uint32_t cfgLtrMaxLatency;                                              // 0x64 : CFG_LTR_MAX_LATENCY / 
    PcieCoreGeneralCfg_t pcieCoreGeneralCfg;                                // 0x68 : PCIe_Core_General_Configuration / 
    uint32_t cxplDebugInfo6332CxplDebugInfo1;                               // 0x6C : CXPL_DEBUG_INFO_63_32 / 
    uint32_t cxplDebugInfo310CxplDebugInfo0;                                // 0x70 : CXPL_DEBUG_INFO_31_0 / 
    CxplDebugInfoEi_t cxplDebugInfoEi;                                      // 0x74 : CXPL_DEBUG_INFO_EI / 
    uint32_t radmMsgPayload6332RadmMsgPayload1;                             // 0x78 : RADM_MSG_PAYLOAD_63_32 / 
    uint32_t radmMsgPayload310RadmMsgPayload0;                              // 0x7C : RADM_MSG_PAYLOAD_31_0 / 
    PcieCoreRadmGeneral0_t pcieCoreRadmGeneral0;                            // 0x80 : PCIe_Core_RADM_General_0 / 
    PcieCoreRadmGeneral1_t pcieCoreRadmGeneral1;                            // 0x84 : PCIe_Core_RADM_General_1 / 
    PcieCoreTargetGeneral0_t pcieCoreTargetGeneral0;                        // 0x88 : PCIe_Core_Target_General_0 / 
    PcieCoreTargetGeneral1_t pcieCoreTargetGeneral1;                        // 0x8C : PCIe_Core_Target_General_1 / 
    uint32_t venMsgData6332VenMsgData1;                                     // 0x90 : VEN_MSG_DATA_63_32 / 
    uint8_t rsvd94[8];                                                      // 0x94 : rsvd_94 / rsvd_94
    uint32_t venMsgData310VenMsgData0;                                      // 0x9C : VEN_MSG_DATA_31_0 / 
    PcieCoreVendorMessageGeneral0_t pcieCoreVendorMessageGeneral0;          // 0xA0 : PCIe_Core_Vendor_Message_General_0 / 
    PcieCoreVendorMessageGeneral1_t pcieCoreVendorMessageGeneral1;          // 0xA4 : PCIe_Core_Vendor_Message_General_1 / 
    PcieCoreGlobal2_t pcieCoreGlobal2;                                      // 0xA8 : PCI_Express_Core_Global_2 / 
    PcieCoreGlobal3_t pcieCoreGlobal3;                                      // 0xAC : PCI_Express_Core_Global_3 / 
    uint8_t rsvdB0[4];                                                      // 0xB0 : rsvd_b0 / rsvd_b0
    uint32_t flrVfIntEn1;                                                   // 0xB4 : FLR_VF_INT_EN_1 / 
    uint32_t flrVfDone1;                                                    // 0xB8 : FLR_VF_DONE_1 / 
    uint32_t flrVfActiveStatus1;                                            // 0xBC : FLR_VF_ACTIVE_STATUS_1 / 
    uint8_t rsvdC0[20];                                                     // 0xC0 : rsvd_c0 / rsvd_c0
    PcieCoreIntStatus_t pcieCoreIntStatus;                                  // 0xD4 : PCIe_Core_Int_Status / 
    PcieCoreIntEnable_t pcieCoreIntEnable;                                  // 0xD8 : PCIe_Core_Int_Enable / 
    PcieCoreLtrMessageControl_t pcieCoreLtrMessageControl;                  // 0xDC : PCIe_Core_LTR_Message_Control / 
    uint32_t ltrEnVal;                                                      // 0xE0 : LTR_EN_VAL / 
    uint8_t rsvdE4[4];                                                      // 0xE4 : rsvd_e4 / rsvd_e4
    uint32_t ltrFw0Val;                                                     // 0xE8 : LTR_FW0_VAL / 
    uint32_t ltrFw1Val;                                                     // 0xEC : LTR_FW1_VAL / 
    uint32_t ltrFw2Val;                                                     // 0xF0 : LTR_FW2_VAL / 
    uint32_t ltrFw3Val;                                                     // 0xF4 : LTR_FW3_VAL / 
    PcieCoreErrorCount0_t pcieCoreErrorCount0;                              // 0xF8 : PCIe_Core_Error_Count_0 / 
    PcieCoreErrorCount1_t pcieCoreErrorCount1;                              // 0xFC : PCIe_Core_Error_Count_1 / 
    PcieCoreErrorCount2_t pcieCoreErrorCount2;                              // 0x100 : PCIe_Core_Error_Count_2 / 
    uint8_t rsvd104[12];                                                    // 0x104 : rsvd_104 / rsvd_104
    PcieCoreDrsFrsReady_t pcieCoreDrsFrsReady;                              // 0x110 : PCIe_Core_DRS_FRS_Ready / 
    uint32_t flrVfInt1;                                                     // 0x114 : FLR_VF_INT_1 / 
    uint32_t flrVfInt2;                                                     // 0x118 : FLR_VF_INT_2 / 
    uint32_t flrVfIntEn2;                                                   // 0x11C : FLR_VF_INT_EN_2 / 
    uint32_t flrVfDone2;                                                    // 0x120 : FLR_VF_DONE_2 / 
    uint32_t flrVfActiveStatus2;                                            // 0x124 : FLR_VF_ACTIVE_STATUS_2 / 
    uint32_t frsVfReady1;                                                   // 0x128 : FRS_VF_READY_1 / 
    uint32_t frsVfReady2;                                                   // 0x12C : FRS_VF_READY_2 / 
    uint8_t rsvd130[8];                                                     // 0x130 : rsvd_130 / rsvd_130
    PowerBudgetControl_t powerBudgetControl;                                // 0x138 : Power_Budget_Control / 
    uint32_t pwrBudgetData0;                                                // 0x13C : PWR_BUDGET_DATA_0 / 
    uint32_t pwrBudgetData1;                                                // 0x140 : PWR_BUDGET_DATA_1 / 
    uint32_t pwrBudgetData2;                                                // 0x144 : PWR_BUDGET_DATA_2 / 
    uint32_t pwrBudgetData3;                                                // 0x148 : PWR_BUDGET_DATA_3 / 
    uint32_t pwrBudgetData4;                                                // 0x14C : PWR_BUDGET_DATA_4 / 
    uint32_t pwrBudgetData5;                                                // 0x150 : PWR_BUDGET_DATA_5 / 
    uint32_t pwrBudgetData6;                                                // 0x154 : PWR_BUDGET_DATA_6 / 
    uint32_t pwrBudgetData7;                                                // 0x158 : PWR_BUDGET_DATA_7 / 
    uint32_t pwrBudgetDataFw;                                               // 0x15C : PWR_BUDGET_DATA_FW / 
    MemoryRtcWtc1_t memoryRtcWtc1;                                          // 0x160 : Memory_RTC_WTC_Reg_1 / 
    MemoryRtcWtc2_t memoryRtcWtc2;                                          // 0x164 : Memory_RTC_WTC_Reg_2 / 
    MemoryRtcWtc3_t memoryRtcWtc3;                                          // 0x168 : Memory_RTC_WTC_Reg_3 / 
    FunctionSelection_t functionSelection;                                  // 0x16C : Function_Selection / 
    uint8_t rsvd170[144];                                                   // 0x170 : rsvd_170 / rsvd_170
    ErrorStatus_t errorStatus;                                              // 0x200 : Error_Status / 
    ErrorStatusIntEn_t errorStatusIntEn;                                    // 0x204 : Error_Status_Int_En / 
    uint8_t rsvd208[48];                                                    // 0x208 : rsvd_208 / rsvd_208
    IdeSramRtcWtc1_t ideSramRtcWtc1;                                        // 0x238 : IDE_SRAM_RTC_WTC_Reg_1 / 
    IdeSramRtcWtc2_t ideSramRtcWtc2;                                        // 0x23C : IDE_SRAM_RTC_WTC_Reg_2 / 
    ErrorStatus2_t errorStatus2;                                            // 0x240 : ERROR_STATUS_2 / 
    ErrorStatus2IntEn_t errorStatus2IntEn;                                  // 0x244 : ERROR_STATUS_2_INT_EN / 
    uint8_t rsvd248[184];                                                   // 0x248 : rsvd_248 / rsvd_248
    CfgVfMsixTblStatus_t cfgVfMsixTblStatus;                                // 0x300 : CFG_VF_MSIX_TBL_STATUS / 
    CfgVfMsixPbaStatus_t cfgVfMsixPbaStatus;                                // 0x304 : CFG_VF_MSIX_PBA_STATUS / 
    PcieCfg1_t pcieCfg1;                                                    // 0x308 : PCIE_CFG_1 / 
    MiscStatus2_t miscStatus2;                                              // 0x30C : MISC_STATUS_2 / 
    uint8_t rsvd310[4];                                                     // 0x310 : rsvd_310 / rsvd_310
    SramCtrl_t sramCtrl;                                                    // 0x314 : SRAM_CTRL / 
    uint8_t rsvd318[8];                                                     // 0x318 : rsvd_318 / rsvd_318
    FlrPfActive_t flrPfActive;                                              // 0x320 : FLR_PF_ACTIVE / 
    FlrPfActiveIntStatus_t flrPfActiveIntStatus;                            // 0x324 : FLR_PF_ACTIVE_INT_STATUS / 
    FlrPfActiveIntEn_t flrPfActiveIntEn;                                    // 0x328 : FLR_PF_ACTIVE_INT_EN / 
    FlrPfDone_t flrPfDone;                                                  // 0x32C : FLR_PF_DONE / 
    PmXmtPme_t pmXmtPme;                                                    // 0x330 : PM_XMT_PME / 
    OutbandPwrupCmd_t outbandPwrupCmd;                                      // 0x334 : OUTBAND_PWRUP_CMD / 
    LtrMsgFuncNum_t ltrMsgFuncNum;                                          // 0x338 : LTR_MSG_FUNC_NUM / 
    PmStatus_t pmStatus;                                                    // 0x33C : PM_STATUS / 
    uint8_t rsvd340[4];                                                     // 0x340 : rsvd_340 / rsvd_340
    DpaSubstateUpdateIntStatus_t dpaSubstateUpdateIntStatus;                // 0x344 : DPA_SUBSTATE_UPDATE_INT_STATUS / 
    DpaSubstateUpdateIntEn_t dpaSubstateUpdateIntEn;                        // 0x348 : DPA_SUBSTATE_UPDATE_INT_EN / 
    PfFrsReady_t pfFrsReady;                                                // 0x34C : PF_FRS_READY / 
    PfFrsGrant_t pfFrsGrant;                                                // 0x350 : PF_FRS_GRANT / 
    CfgPwrBudgetSelIntStatus_t cfgPwrBudgetSelIntStatus;                    // 0x354 : CFG_PWR_BUDGET_SEL_INT_STATUS / 
    CfgPwrBudgetSelIntEn_t cfgPwrBudgetSelIntEn;                            // 0x358 : CFG_PWR_BUDGET_SEL_INT_EN / 
    CfgVpdIntStatus_t cfgVpdIntStatus;                                      // 0x35C : CFG_VPD_INT_STATUS / 
    CfgVpdIntEn_t cfgVpdIntEn;                                              // 0x360 : CFG_VPD_INT_EN / 
    CfgBusMasterEn_t cfgBusMasterEn;                                        // 0x364 : CFG_BUS_MASTER_EN / 
    CfgBusMasterEnIntStatus_t cfgBusMasterEnIntStatus;                      // 0x368 : CFG_BUS_MASTER_EN_INT_STATUS / 
    CfgBusMasterEnIntEn_t cfgBusMasterEnIntEn;                              // 0x36C : CFG_BUS_MASTER_EN_INT_EN / 
    D3HotEnterIntStatus_t d3HotEnterIntStatus;                              // 0x370 : D3_HOT_ENTER_INT_STATUS / 
    D3HotEnterIntEn_t d3HotEnterIntEn;                                      // 0x374 : D3_HOT_ENTER_INT_EN / 
    D3HotExitIntStatus_t d3HotExitIntStatus;                                // 0x378 : D3_HOT_EXIT_INT_STATUS / 
    D3HotExitIntEn_t d3HotExitIntEn;                                        // 0x37C : D3_HOT_EXIT_INT_EN / 
    uint8_t rsvd380[160];                                                   // 0x380 : rsvd_380 / rsvd_380
    uint32_t cfgBar0Limit310CfgBar0Limit0;                                  // 0x420 : CFG_BAR0_LIMIT_31_0 / 
    uint32_t cfgBar0Limit6332CfgBar0Limit1;                                 // 0x424 : CFG_BAR0_LIMIT_63_32 / 
    uint32_t cfgBar0Start310CfgBar0Start0;                                  // 0x428 : CFG_BAR0_START_31_0 / 
    uint32_t cfgBar0Start6332CfgBar0Start1;                                 // 0x42C : CFG_BAR0_START_63_32 / 
    uint32_t cfgBar1Limit;                                                  // 0x430 : CFG_BAR1_LIMIT / 
    uint32_t cfgBar1Start;                                                  // 0x434 : CFG_BAR1_START / 
    uint32_t cfgBar2Limit310CfgBar2Limit0;                                  // 0x438 : CFG_BAR2_LIMIT_31_0 / 
    uint32_t cfgBar2Limit6332CfgBar2Limit1;                                 // 0x43C : CFG_BAR2_LIMIT_63_32 / 
    uint32_t cfgBar2Start310CfgBar2Start0;                                  // 0x440 : CFG_BAR2_START_31_0 / 
    uint32_t cfgBar2Start6332CfgBar2Start1;                                 // 0x444 : CFG_BAR2_START_63_32 / 
    uint32_t cfgBar3Limit;                                                  // 0x448 : CFG_BAR3_LIMIT / 
    uint32_t cfgBar3Start;                                                  // 0x44C : CFG_BAR3_START / 
    uint32_t cfgBar4Limit310CfgBar4Limit0;                                  // 0x450 : CFG_BAR4_LIMIT_31_0 / 
    uint32_t cfgBar4Limit6332CfgBar4Limit1;                                 // 0x454 : CFG_BAR4_LIMIT_63_32 / 
    uint32_t cfgBar4Start310CfgBar4Start0;                                  // 0x458 : CFG_BAR4_START_31_0 / 
    uint32_t cfgBar4Start6332CfgBar4Start1;                                 // 0x45C : CFG_BAR4_START_63_32 / 
    uint32_t cfgBar5Limit;                                                  // 0x460 : CFG_BAR5_LIMIT / 
    uint32_t cfgBar5Start;                                                  // 0x464 : CFG_BAR5_START / 
    uint32_t cfgExpRomLimit;                                                // 0x468 : CFG_EXP_ROM_LIMIT / 
    uint32_t cfgExpRomStart;                                                // 0x46C : CFG_EXP_ROM_START / 
} PcieAssist_t;

COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreGlobal0)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreGlobal1)==0x4,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,appLtrLatency)==0x8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,appLtrMsgLatency)==0xC,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreCfg)==0x10,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,appVfReqRetryEn1)==0x14,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,appVfReqRetryEn2)==0x18,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,ideTestCtrl)==0x1C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,ideStreamStatus)==0x20,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgLtrMaxLatency)==0x64,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreGeneralCfg)==0x68,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cxplDebugInfo6332CxplDebugInfo1)==0x6C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cxplDebugInfo310CxplDebugInfo0)==0x70,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cxplDebugInfoEi)==0x74,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,radmMsgPayload6332RadmMsgPayload1)==0x78,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,radmMsgPayload310RadmMsgPayload0)==0x7C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreRadmGeneral0)==0x80,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreRadmGeneral1)==0x84,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreTargetGeneral0)==0x88,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreTargetGeneral1)==0x8C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,venMsgData6332VenMsgData1)==0x90,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,venMsgData310VenMsgData0)==0x9C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreVendorMessageGeneral0)==0xA0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreVendorMessageGeneral1)==0xA4,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreGlobal2)==0xA8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreGlobal3)==0xAC,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,flrVfIntEn1)==0xB4,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,flrVfDone1)==0xB8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,flrVfActiveStatus1)==0xBC,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreIntStatus)==0xD4,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreIntEnable)==0xD8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreLtrMessageControl)==0xDC,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,ltrEnVal)==0xE0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,ltrFw0Val)==0xE8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,ltrFw1Val)==0xEC,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,ltrFw2Val)==0xF0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,ltrFw3Val)==0xF4,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreErrorCount0)==0xF8,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreErrorCount1)==0xFC,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreErrorCount2)==0x100,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCoreDrsFrsReady)==0x110,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,flrVfInt1)==0x114,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,flrVfInt2)==0x118,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,flrVfIntEn2)==0x11C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,flrVfDone2)==0x120,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,flrVfActiveStatus2)==0x124,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,frsVfReady1)==0x128,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,frsVfReady2)==0x12C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,powerBudgetControl)==0x138,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pwrBudgetData0)==0x13C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pwrBudgetData1)==0x140,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pwrBudgetData2)==0x144,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pwrBudgetData3)==0x148,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pwrBudgetData4)==0x14C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pwrBudgetData5)==0x150,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pwrBudgetData6)==0x154,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pwrBudgetData7)==0x158,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pwrBudgetDataFw)==0x15C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,memoryRtcWtc1)==0x160,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,memoryRtcWtc2)==0x164,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,memoryRtcWtc3)==0x168,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,functionSelection)==0x16C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,errorStatus)==0x200,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,errorStatusIntEn)==0x204,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,ideSramRtcWtc1)==0x238,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,ideSramRtcWtc2)==0x23C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,errorStatus2)==0x240,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,errorStatus2IntEn)==0x244,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgVfMsixTblStatus)==0x300,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgVfMsixPbaStatus)==0x304,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pcieCfg1)==0x308,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,miscStatus2)==0x30C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,sramCtrl)==0x314,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,flrPfActive)==0x320,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,flrPfActiveIntStatus)==0x324,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,flrPfActiveIntEn)==0x328,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,flrPfDone)==0x32C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pmXmtPme)==0x330,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,outbandPwrupCmd)==0x334,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,ltrMsgFuncNum)==0x338,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pmStatus)==0x33C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,dpaSubstateUpdateIntStatus)==0x344,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,dpaSubstateUpdateIntEn)==0x348,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pfFrsReady)==0x34C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,pfFrsGrant)==0x350,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgPwrBudgetSelIntStatus)==0x354,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgPwrBudgetSelIntEn)==0x358,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgVpdIntStatus)==0x35C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgVpdIntEn)==0x360,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBusMasterEn)==0x364,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBusMasterEnIntStatus)==0x368,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBusMasterEnIntEn)==0x36C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,d3HotEnterIntStatus)==0x370,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,d3HotEnterIntEn)==0x374,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,d3HotExitIntStatus)==0x378,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,d3HotExitIntEn)==0x37C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar0Limit310CfgBar0Limit0)==0x420,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar0Limit6332CfgBar0Limit1)==0x424,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar0Start310CfgBar0Start0)==0x428,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar0Start6332CfgBar0Start1)==0x42C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar1Limit)==0x430,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar1Start)==0x434,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar2Limit310CfgBar2Limit0)==0x438,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar2Limit6332CfgBar2Limit1)==0x43C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar2Start310CfgBar2Start0)==0x440,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar2Start6332CfgBar2Start1)==0x444,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar3Limit)==0x448,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar3Start)==0x44C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar4Limit310CfgBar4Limit0)==0x450,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar4Limit6332CfgBar4Limit1)==0x454,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar4Start310CfgBar4Start0)==0x458,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar4Start6332CfgBar4Start1)==0x45C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar5Limit)==0x460,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgBar5Start)==0x464,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgExpRomLimit)==0x468,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieAssist_t,cfgExpRomStart)==0x46C,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile PcieAssist_t rPcieAssist; ///< 0xB01C0000
