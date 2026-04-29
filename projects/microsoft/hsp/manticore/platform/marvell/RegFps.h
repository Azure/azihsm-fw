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
//! @brief FPS Registers
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
        uint32_t HWE2FP_WQ_00_UCD_IB_CQ0_EMPTY :1;      ///<BIT [0] hwe2fp_wq_00_ucd_ib_cq0_empty
        uint32_t HWE2FP_WQ_01_UCD_IB_CQ1_EMPTY :1;      ///<BIT [1] hwe2fp_wq_01_ucd_ib_cq1_empty
        uint32_t HWE2FP_WQ_02_UCD_OB_CQ0_EMPTY :1;      ///<BIT [2] hwe2fp_wq_02_ucd_ob_cq0_empty
        uint32_t HWE2FP_WQ_03_UCD_OB_CQ1_EMPTY :1;      ///<BIT [3] hwe2fp_wq_03_ucd_ob_cq1_empty
        uint32_t HWE2FP_WQ_04_CDMA_CQ_EMPTY  :1;      ///<BIT [4] hwe2fp_wq_04_cdma_cq_empty
        uint32_t HWE2FP_WQ_05_GDMA_CQ_EMPTY  :1;      ///<BIT [5] hwe2fp_wq_05_gdma_cq_empty
        uint32_t HWE2FP_WQ_06_CP2FP_CMD_SQ_EMPTY :1;      ///<BIT [6] hwe2fp_wq_06_cp2fp_cmd_sq_empty
        uint32_t HWE2FP_WQ_07_FP2CP_ERR_CQ_EMPTY :1;      ///<BIT [7] hwe2fp_wq_07_fp2cp_err_cq_empty
        uint32_t HWE2FP_WQ_08_EMPTY          :1;      ///<BIT [8] hwe2fp_wq_08_empty
        uint32_t HWE2FP_WQ_09_EMPTY          :1;      ///<BIT [9] hwe2fp_wq_09_empty
        uint32_t HWE2FP_WQ_10_EMPTY          :1;      ///<BIT [10] hwe2fp_wq_10_empty
        uint32_t HWE2FP_WQ_11_EMPTY          :1;      ///<BIT [11] hwe2fp_wq_11_empty
        uint32_t FP2HWE_WQ_00_FULL           :1;      ///<BIT [12] fp2hwe_wq_00_full
        uint32_t FP2HWE_WQ_01_FULL           :1;      ///<BIT [13] fp2hwe_wq_01_full
        uint32_t FP2HWE_WQ_02_FULL           :1;      ///<BIT [14] fp2hwe_wq_02_full
        uint32_t FP2HWE_WQ_03_FULL           :1;      ///<BIT [15] fp2hwe_wq_03_full
        uint32_t FP2HWE_WQ_04_CDMA_SQ_FULL   :1;      ///<BIT [16] fp2hwe_wq_04_cdma_sq_full
        uint32_t FP2HWE_WQ_05_GDMA_SQ_FULL   :1;      ///<BIT [17] fp2hwe_wq_05_gdma_sq_full
        uint32_t FP2HWE_WQ_06_CP2FP_CMD_CQ_FULL :1;      ///<BIT [18] fp2hwe_wq_06_cp2fp_cmd_cq_full
        uint32_t FP2HWE_WQ_07_FP2CP_ERR_SQ_FULL :1;      ///<BIT [19] fp2hwe_wq_07_fp2cp_err_sq_full
        uint32_t FP2HWE_WQ_08_FULL           :1;      ///<BIT [20] fp2hwe_wq_08_full
        uint32_t FP2HWE_WQ_09_FULL           :1;      ///<BIT [21] fp2hwe_wq_09_full
        uint32_t FP2HWE_WQ_10_FULL           :1;      ///<BIT [22] fp2hwe_wq_10_full
        uint32_t FP2HWE_WQ_11_FULL           :1;      ///<BIT [23] fp2hwe_wq_11_full
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_7_0   :1;      ///<BIT [24] cpux_to_cpuy_wq_empty_7_0
        uint32_t CPUX_TO_CPUY_WQ_FULL_7_0    :1;      ///<BIT [25] cpux_to_cpuy_wq_full_7_0
        uint32_t SLOT_ARRAY_EMPTY_131_0      :1;      ///<BIT [26] slot_array_empty_131_0
        uint32_t SLOT_ARRAY_FULL_131_0       :1;      ///<BIT [27] slot_array_full_131_0
        uint32_t RSVD0                       :1;      ///<BIT [28] rsvd0
        uint32_t RSVD1                       :1;      ///<BIT [29] rsvd1
        uint32_t FPS_FABRIC_PARITY_ERROR     :1;      ///<BIT [30] fps_fabric_parity_error
        uint32_t FPS_MEMORY_PROTECTION_ERROR :1;      ///<BIT [31] fps_memory_protection_error
    } b;
} FpsBank0EventStatus0_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HWE2FP_WQ_00_UCD_IB_CQ0_FULL :1;      ///<BIT [0] hwe2fp_wq_00_ucd_ib_cq0_full
        uint32_t HWE2FP_WQ_01_UCD_IB_CQ1_FULL :1;      ///<BIT [1] hwe2fp_wq_01_ucd_ib_cq1_full
        uint32_t HWE2FP_WQ_02_UCD_OB_CQ0_FULL :1;      ///<BIT [2] hwe2fp_wq_02_ucd_ob_cq0_full
        uint32_t HWE2FP_WQ_03_UCD_OB_CQ1_FULL :1;      ///<BIT [3] hwe2fp_wq_03_ucd_ob_cq1_full
        uint32_t HWE2FP_WQ_04_CDMA_CQ_FULL   :1;      ///<BIT [4] hwe2fp_wq_04_cdma_cq_full
        uint32_t HWE2FP_WQ_05_GDMA_CQ_FULL   :1;      ///<BIT [5] hwe2fp_wq_05_gdma_cq_full
        uint32_t HWE2FP_WQ_06_CP2FP_CMD_SQ_FULL :1;      ///<BIT [6] hwe2fp_wq_06_cp2fp_cmd_sq_full
        uint32_t HWE2FP_WQ_07_FP2CP_ERR_CQ_FULL :1;      ///<BIT [7] hwe2fp_wq_07_fp2cp_err_cq_full
        uint32_t HWE2FP_WQ_08_FULL           :1;      ///<BIT [8] hwe2fp_wq_08_full
        uint32_t HWE2FP_WQ_09_FULL           :1;      ///<BIT [9] hwe2fp_wq_09_full
        uint32_t HWE2FP_WQ_10_FULL           :1;      ///<BIT [10] hwe2fp_wq_10_full
        uint32_t HWE2FP_WQ_11_FULL           :1;      ///<BIT [11] hwe2fp_wq_11_full
        uint32_t FP2HWE_WQ_00_EMPTY          :1;      ///<BIT [12] fp2hwe_wq_00_empty
        uint32_t FP2HWE_WQ_01_EMPTY          :1;      ///<BIT [13] fp2hwe_wq_01_empty
        uint32_t FP2HWE_WQ_02_EMPTY          :1;      ///<BIT [14] fp2hwe_wq_02_empty
        uint32_t FP2HWE_WQ_03_EMPTY          :1;      ///<BIT [15] fp2hwe_wq_03_empty
        uint32_t FP2HWE_WQ_04_CDMA_SQ_EMPTY  :1;      ///<BIT [16] fp2hwe_wq_04_cdma_sq_empty
        uint32_t FP2HWE_WQ_05_GDMA_SQ_EMPTY  :1;      ///<BIT [17] fp2hwe_wq_05_gdma_sq_empty
        uint32_t FP2HWE_WQ_06_CP2FP_CMD_CQ_EMPTY :1;      ///<BIT [18] fp2hwe_wq_06_cp2fp_cmd_cq_empty
        uint32_t FP2HWE_WQ_07_FP2CP_ERR_SQ_EMPTY :1;      ///<BIT [19] fp2hwe_wq_07_fp2cp_err_sq_empty
        uint32_t FP2HWE_WQ_08_EMPTY          :1;      ///<BIT [20] fp2hwe_wq_08_empty
        uint32_t FP2HWE_WQ_09_EMPTY          :1;      ///<BIT [21] fp2hwe_wq_09_empty
        uint32_t FP2HWE_WQ_10_EMPTY          :1;      ///<BIT [22] fp2hwe_wq_10_empty
        uint32_t FP2HWE_WQ_11_EMPTY          :1;      ///<BIT [23] fp2hwe_wq_11_empty
        uint32_t RSVD                        :8;      ///<BIT [31:24] rsvd
    } b;
} FpsBank0EventStatus1_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_0     :1;      ///<BIT [0] cpux_to_cpuy_wq_empty_0
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_1     :1;      ///<BIT [1] cpux_to_cpuy_wq_empty_1
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_2     :1;      ///<BIT [2] cpux_to_cpuy_wq_empty_2
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_3     :1;      ///<BIT [3] cpux_to_cpuy_wq_empty_3
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_4     :1;      ///<BIT [4] cpux_to_cpuy_wq_empty_4
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_5     :1;      ///<BIT [5] cpux_to_cpuy_wq_empty_5
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_6     :1;      ///<BIT [6] cpux_to_cpuy_wq_empty_6
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_7     :1;      ///<BIT [7] cpux_to_cpuy_wq_empty_7
        uint32_t RSVD0                       :8;      ///<BIT [15:8] rsvd0
        uint32_t CPUX_TO_CPUY_WQ_FULL_0      :1;      ///<BIT [16] cpux_to_cpuy_wq_full_0
        uint32_t CPUX_TO_CPUY_WQ_FULL_1      :1;      ///<BIT [17] cpux_to_cpuy_wq_full_1
        uint32_t CPUX_TO_CPUY_WQ_FULL_2      :1;      ///<BIT [18] cpux_to_cpuy_wq_full_2
        uint32_t CPUX_TO_CPUY_WQ_FULL_3      :1;      ///<BIT [19] cpux_to_cpuy_wq_full_3
        uint32_t CPUX_TO_CPUY_WQ_FULL_4      :1;      ///<BIT [20] cpux_to_cpuy_wq_full_4
        uint32_t CPUX_TO_CPUY_WQ_FULL_5      :1;      ///<BIT [21] cpux_to_cpuy_wq_full_5
        uint32_t CPUX_TO_CPUY_WQ_FULL_6      :1;      ///<BIT [22] cpux_to_cpuy_wq_full_6
        uint32_t CPUX_TO_CPUY_WQ_FULL_7      :1;      ///<BIT [23] cpux_to_cpuy_wq_full_7
        uint32_t RSVD1                       :8;      ///<BIT [31:24] rsvd1
    } b;
} FpsBank0CpuxToCpuyWqStatus_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SLOT_ARRAY_GROUP_EMPTY_7_0  :1;      ///<BIT [0] slot_array_group_empty_7_0
        uint32_t SLOT_ARRAY_GROUP_EMPTY_15_8 :1;      ///<BIT [1] slot_array_group_empty_15_8
        uint32_t SLOT_ARRAY_GROUP_EMPTY_23_16 :1;      ///<BIT [2] slot_array_group_empty_23_16
        uint32_t SLOT_ARRAY_GROUP_EMPTY_31_24 :1;      ///<BIT [3] slot_array_group_empty_31_24
        uint32_t SLOT_ARRAY_GROUP_EMPTY_39_32 :1;      ///<BIT [4] slot_array_group_empty_39_32
        uint32_t SLOT_ARRAY_GROUP_EMPTY_47_40 :1;      ///<BIT [5] slot_array_group_empty_47_40
        uint32_t SLOT_ARRAY_GROUP_EMPTY_55_48 :1;      ///<BIT [6] slot_array_group_empty_55_48
        uint32_t SLOT_ARRAY_GROUP_EMPTY_63_56 :1;      ///<BIT [7] slot_array_group_empty_63_56
        uint32_t SLOT_ARRAY_GROUP_EMPTY_71_64 :1;      ///<BIT [8] slot_array_group_empty_71_64
        uint32_t SLOT_ARRAY_GROUP_EMPTY_79_72 :1;      ///<BIT [9] slot_array_group_empty_79_72
        uint32_t SLOT_ARRAY_GROUP_EMPTY_87_80 :1;      ///<BIT [10] slot_array_group_empty_87_80
        uint32_t SLOT_ARRAY_GROUP_EMPTY_95_88 :1;      ///<BIT [11] slot_array_group_empty_95_88
        uint32_t SLOT_ARRAY_GROUP_EMPTY_103_96 :1;      ///<BIT [12] slot_array_group_empty_103_96
        uint32_t SLOT_ARRAY_GROUP_EMPTY_111_104 :1;      ///<BIT [13] slot_array_group_empty_111_104
        uint32_t SLOT_ARRAY_GROUP_EMPTY_119_112 :1;      ///<BIT [14] slot_array_group_empty_119_112
        uint32_t SLOT_ARRAY_GROUP_EMPTY_127_120 :1;      ///<BIT [15] slot_array_group_empty_127_120
        uint32_t SLOT_ARRAY_GROUP_EMPTY_131_128 :1;      ///<BIT [16] slot_array_group_empty_131_128
        uint32_t RSVD                        :15;     ///<BIT [31:17] rsvd_0
    } b;
} FpsBank0SlotArrayGroupEmptyStatus_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SLOT_ARRAY_GROUP_FULL_7_0   :1;      ///<BIT [0] slot_array_group_full_7_0
        uint32_t SLOT_ARRAY_GROUP_FULL_15_8  :1;      ///<BIT [1] slot_array_group_full_15_8
        uint32_t SLOT_ARRAY_GROUP_FULL_23_16 :1;      ///<BIT [2] slot_array_group_full_23_16
        uint32_t SLOT_ARRAY_GROUP_FULL_31_24 :1;      ///<BIT [3] slot_array_group_full_31_24
        uint32_t SLOT_ARRAY_GROUP_FULL_39_32 :1;      ///<BIT [4] slot_array_group_full_39_32
        uint32_t SLOT_ARRAY_GROUP_FULL_47_40 :1;      ///<BIT [5] slot_array_group_full_47_40
        uint32_t SLOT_ARRAY_GROUP_FULL_55_48 :1;      ///<BIT [6] slot_array_group_full_55_48
        uint32_t SLOT_ARRAY_GROUP_FULL_63_56 :1;      ///<BIT [7] slot_array_group_full_63_56
        uint32_t SLOT_ARRAY_GROUP_FULL_71_64 :1;      ///<BIT [8] slot_array_group_full_71_64
        uint32_t SLOT_ARRAY_GROUP_FULL_79_72 :1;      ///<BIT [9] slot_array_group_full_79_72
        uint32_t SLOT_ARRAY_GROUP_FULL_87_80 :1;      ///<BIT [10] slot_array_group_full_87_80
        uint32_t SLOT_ARRAY_GROUP_FULL_95_88 :1;      ///<BIT [11] slot_array_group_full_95_88
        uint32_t SLOT_ARRAY_GROUP_FULL_103_96 :1;      ///<BIT [12] slot_array_group_full_103_96
        uint32_t SLOT_ARRAY_GROUP_FULL_111_104 :1;      ///<BIT [13] slot_array_group_full_111_104
        uint32_t SLOT_ARRAY_GROUP_FULL_119_112 :1;      ///<BIT [14] slot_array_group_full_119_112
        uint32_t SLOT_ARRAY_GROUP_FULL_127_120 :1;      ///<BIT [15] slot_array_group_full_127_120
        uint32_t SLOT_ARRAY_GROUP_FULL_131_128 :1;      ///<BIT [16] slot_array_group_full_131_128
        uint32_t RSVD                        :15;     ///<BIT [31:17] rsvd_0
    } b;
} FpsBank0SlotArrayGroupFullStatus_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SLOT_ARRAY_Q_EMPTY_131_128  :4;      ///<BIT [3:0] slot_array_q_empty_131_128
        uint32_t RSVD                        :28;     ///<BIT [31:4] rsvd_0
    } b;
} FpsBank0SlotArrayQueueEmptyStatus4_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SLOT_ARRAY_Q_FULL_131_128   :4;      ///<BIT [3:0] slot_array_q_full_131_128
        uint32_t RSVD                        :28;     ///<BIT [31:4] rsvd_0
    } b;
} FpsBank0SlotArrayQueueFullStatus4_t;

/// @brief 0x70
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GLUE_LOGIC_RESET            :1;      ///<BIT [0] glue_logic_reset
        uint32_t FABRIC_INTERFACE_RESET      :1;      ///<BIT [1] fabric_interface_reset
        uint32_t FPS_ERROR_CLR               :1;      ///<BIT [2] fps_error_clr
        uint32_t RSVD0                       :5;      ///<BIT [7:3] rsvd0
        uint32_t PARITY_GEN_ODD              :1;      ///<BIT [8] parity_gen_odd
        uint32_t PARITY_CHECK_ODD            :1;      ///<BIT [9] parity_check_odd
        uint32_t RSVD1                       :22;     ///<BIT [31:10] rsvd1
    } b;
} FpsBank0FpsControl_t;

/// @brief 0x74
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SYSTICK_TIMER_PULSE_THRESHOLD :6;      ///<BIT [5:0] systick_timer_pulse_threshold
        uint32_t CPU_ITCMERR_DISABLE         :1;      ///<BIT [6] cpu_itcmerr_disable
        uint32_t CPU_DTCMERR_DISABLE         :1;      ///<BIT [7] cpu_dtcmerr_disable
        uint32_t RSVD1                       :7;      ///<BIT [14:8] rsvd1
        uint32_t GEN_AXI_RESP_ERR_DISABLE    :1;      ///<BIT [15] gen_axi_resp_err_disable
        uint32_t MISC_CTL_SPARE_RW_BITS      :16;     ///<BIT [31:16] misc_ctl_spare_rw_bits
    } b;
} FpsBank0FpsCfg_t;

/// @brief 0xA4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FPS_EGRESS_SW_INTERRUPT_STATUS :8;      ///<BIT [7:0] fps_egress_sw_interrupt_status
        uint32_t FPS_EGRESS_SW_INTERRUPT_SET :8;      ///<BIT [15:8] fps_egress_sw_interrupt_set
        uint32_t FPS_EGRESS_SW_INTERRUPT_CLR :8;      ///<BIT [23:16] fps_egress_sw_interrupt_clr
        uint32_t RSVD                        :8;      ///<BIT [31:24] rsvd_0
    } b;
} FpsBank0FpsSwIntrStatus_t;

/// @brief 0xAC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FPS_EGRESS_SW_INTERRUPT     :8;      ///<BIT [7:0] fps_egress_sw_interrupt
        uint32_t FPS_FABRIC_PARITY_ERROR     :1;      ///<BIT [8] fps_fabric_parity_error
        uint32_t FPS_MEMORY_PROTECTION_ERROR :1;      ///<BIT [9] fps_memory_protection_error
        uint32_t RSVD                        :22;     ///<BIT [31:10] rsvd_0
    } b;
} FpsBank0FpsEgressIntrStatus_t;

/// @brief 0xB0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FPS_EGRESS_INTERRUPT_ENABLE :16;     ///<BIT [15:0] fps_egress_interrupt_enable
        uint32_t RSVD                        :16;     ///<BIT [31:16] rsvd_0
    } b;
} FpsBank0FpsEgressIntrEnable_t;

/// @brief 0xF0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HWE2FP_Q_CI_00_WR_STATUS    :1;      ///<BIT [0] hwe2fp_q_ci_00_wr_status
        uint32_t HWE2FP_Q_CI_01_WR_STATUS    :1;      ///<BIT [1] hwe2fp_q_ci_01_wr_status
        uint32_t HWE2FP_Q_CI_02_WR_STATUS    :1;      ///<BIT [2] hwe2fp_q_ci_02_wr_status
        uint32_t HWE2FP_Q_CI_03_WR_STATUS    :1;      ///<BIT [3] hwe2fp_q_ci_03_wr_status
        uint32_t HWE2FP_Q_CI_04_WR_STATUS    :1;      ///<BIT [4] hwe2fp_q_ci_04_wr_status
        uint32_t HWE2FP_Q_CI_05_WR_STATUS    :1;      ///<BIT [5] hwe2fp_q_ci_05_wr_status
        uint32_t HWE2FP_Q_CI_06_WR_STATUS    :1;      ///<BIT [6] hwe2fp_q_ci_06_wr_status
        uint32_t HWE2FP_Q_CI_07_WR_STATUS    :1;      ///<BIT [7] hwe2fp_q_ci_07_wr_status
        uint32_t HWE2FP_Q_CI_08_WR_STATUS    :1;      ///<BIT [8] hwe2fp_q_ci_08_wr_status
        uint32_t HWE2FP_Q_CI_09_WR_STATUS    :1;      ///<BIT [9] hwe2fp_q_ci_09_wr_status
        uint32_t HWE2FP_Q_CI_10_WR_STATUS    :1;      ///<BIT [10] hwe2fp_q_ci_10_wr_status
        uint32_t HWE2FP_Q_CI_11_WR_STATUS    :1;      ///<BIT [11] hwe2fp_q_ci_11_wr_status
        uint32_t FP2HWE_Q_PI_00_WR_STATUS    :1;      ///<BIT [12] fp2hwe_q_pi_00_wr_status
        uint32_t FP2HWE_Q_PI_01_WR_STATUS    :1;      ///<BIT [13] fp2hwe_q_pi_01_wr_status
        uint32_t FP2HWE_Q_PI_02_WR_STATUS    :1;      ///<BIT [14] fp2hwe_q_pi_02_wr_status
        uint32_t FP2HWE_Q_PI_03_WR_STATUS    :1;      ///<BIT [15] fp2hwe_q_pi_03_wr_status
        uint32_t FP2HWE_Q_PI_04_WR_STATUS    :1;      ///<BIT [16] fp2hwe_q_pi_04_wr_status
        uint32_t FP2HWE_Q_PI_05_WR_STATUS    :1;      ///<BIT [17] fp2hwe_q_pi_05_wr_status
        uint32_t FP2HWE_Q_PI_06_WR_STATUS    :1;      ///<BIT [18] fp2hwe_q_pi_06_wr_status
        uint32_t FP2HWE_Q_PI_07_WR_STATUS    :1;      ///<BIT [19] fp2hwe_q_pi_07_wr_status
        uint32_t FP2HWE_Q_PI_08_WR_STATUS    :1;      ///<BIT [20] fp2hwe_q_pi_08_wr_status
        uint32_t FP2HWE_Q_PI_09_WR_STATUS    :1;      ///<BIT [21] fp2hwe_q_pi_09_wr_status
        uint32_t FP2HWE_Q_PI_10_WR_STATUS    :1;      ///<BIT [22] fp2hwe_q_pi_10_wr_status
        uint32_t FP2HWE_Q_PI_11_WR_STATUS    :1;      ///<BIT [23] fp2hwe_q_pi_11_wr_status
        uint32_t SOC_REG_0_WR_STATUS         :1;      ///<BIT [24] soc_reg_0_wr_status
        uint32_t SOC_REG_1_WR_STATUS         :1;      ///<BIT [25] soc_reg_1_wr_status
        uint32_t SOC_REG_2_WR_STATUS         :1;      ///<BIT [26] soc_reg_2_wr_status
        uint32_t SOC_REG_3_WR_STATUS         :1;      ///<BIT [27] soc_reg_3_wr_status
        uint32_t SOC_REG_4_WR_STATUS         :1;      ///<BIT [28] soc_reg_4_wr_status
        uint32_t SOC_REG_5_WR_STATUS         :1;      ///<BIT [29] soc_reg_5_wr_status
        uint32_t SOC_REG_6_WR_STATUS         :1;      ///<BIT [30] soc_reg_6_wr_status
        uint32_t SOC_REG_7_WR_STATUS         :1;      ///<BIT [31] soc_reg_7_wr_status
    } b;
} FpsBank0IndirectRegisterWriteStatus_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HWE2FP_Q_PI_SHADOW          :11;     ///<BIT [10:0] hwe2fp_q_pi_shadow
        uint32_t RSVD                        :21;     ///<BIT [31:11] rsvd_0
    } b;
} FpsHwe2fpHwEngineToFpQPiShadow_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HWE2FP_Q_CI_INDIRECT_REG_DATA :11;     ///<BIT [10:0] hwe2fp_q_ci_indirect_reg_data
        uint32_t RSVD                        :21;     ///<BIT [31:11] rsvd_0
    } b;
} FpsHwe2fpHwEngineToFpQCiIndirectDataPort_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HWE2FP_Q_SIZE               :4;      ///<BIT [3:0] hwe2fp_q_size
        uint32_t RSVD                        :28;     ///<BIT [31:4] rsvd_0
    } b;
} FpsHwe2fpHwEngineToFpQSize_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t Q_EMPTY                     :1;      ///<BIT [0] q_empty
        uint32_t Q_FULL                      :1;      ///<BIT [1] q_full
        uint32_t RSVD_1                      :14;     ///<BIT [15:2] rsvd_1
        uint32_t Q_STORED_COUNT              :11;     ///<BIT [26:16] q_stored_count
        uint32_t RSVD                        :5;      ///<BIT [31:27] rsvd_0
    } b;
} FpsHwe2fpQueueStatus_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FP2HWE_Q_CI_SHADOW          :11;     ///<BIT [10:0] fp2hwe_q_ci_shadow
        uint32_t RSVD                        :21;     ///<BIT [31:11] rsvd_0
    } b;
} FpsFp2hweFpToHweQCiShadow_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FP2HWE_Q_PI_INDIRECT_REG_DATA :11;     ///<BIT [10:0] fp2hwe_q_pi_indirect_reg_data
        uint32_t RSVD                        :21;     ///<BIT [31:11] rsvd_0
    } b;
} FpsFp2hweFpToHweQPiIndirectDataPort_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FP2HWE_Q_SIZE               :4;      ///<BIT [3:0] fp2hwe_q_size
        uint32_t RSVD                        :28;     ///<BIT [31:4] rsvd_0
    } b;
} FpsFp2hweFpToHweQSize_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t Q_EMPTY                     :1;      ///<BIT [0] q_empty
        uint32_t Q_FULL                      :1;      ///<BIT [1] q_full
        uint32_t RSVD_1                      :14;     ///<BIT [15:2] rsvd_1
        uint32_t Q_STORED_COUNT              :11;     ///<BIT [26:16] q_stored_count
        uint32_t RSVD                        :5;      ///<BIT [31:27] rsvd_0
    } b;
} FpsFp2hweQueueStatus_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t Q_PI                        :11;     ///<BIT [10:0] q_pi
        uint32_t RSVD                        :21;     ///<BIT [31:11] rsvd_0
    } b;
} FpsCpuxToCpuyQueueProducerIndex_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t Q_CI                        :11;     ///<BIT [10:0] q_ci
        uint32_t RSVD                        :21;     ///<BIT [31:11] rsvd_0
    } b;
} FpsCpuxToCpuyQueueConsumerIndex_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t Q_SIZE                      :4;      ///<BIT [3:0] q_size
        uint32_t RSVD                        :28;     ///<BIT [31:4] rsvd_0
    } b;
} FpsCpuxToCpuyQueueSize_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t Q_EMPTY                     :1;      ///<BIT [0] q_empty
        uint32_t Q_FULL                      :1;      ///<BIT [1] q_full
        uint32_t RSVD_1                      :14;     ///<BIT [15:2] rsvd_1
        uint32_t Q_STORED_COUNT              :11;     ///<BIT [26:16] q_stored_count
        uint32_t RSVD                        :5;      ///<BIT [31:27] rsvd_0
    } b;
} FpsCpuxToCpuyQueueStatus_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD4                       :1;      ///<BIT [0] rsvd4
        uint32_t M7_CLKEN                    :1;      ///<BIT [1] m7_clken
        uint32_t M7_FCLKEN                   :1;      ///<BIT [2] m7_fclken
        uint32_t M7_HCLKEN                   :1;      ///<BIT [3] m7_hclken
        uint32_t M7_ETMCLKEN                 :1;      ///<BIT [4] m7_etmclken
        uint32_t M7_STCLKEN                  :1;      ///<BIT [5] m7_stclken
        uint32_t M7_CFGBIGEND                :1;      ///<BIT [6] m7_cfgbigend
        uint32_t M7_SLEEPHOLDREQN            :1;      ///<BIT [7] m7_sleepholdreqn
        uint32_t M7_WICENREQ                 :1;      ///<BIT [8] m7_wicenreq
        uint32_t M7_RXEV                     :1;      ///<BIT [9] m7_rxev
        uint32_t RSVD3                       :6;      ///<BIT [15:10] rsvd3
        uint32_t M7_CTLPPBLOCK               :4;      ///<BIT [19:16] m7_ctlppblock
        uint32_t RSVD2                       :4;      ///<BIT [23:20] rsvd2
        uint32_t M7_MSTR_PORT_READ_PERR_NMI_EN :1;      ///<BIT [24] m7_mstr_port_read_perr_nmi_en
        uint32_t M7_SLAVE_PORT_WRITE_PERR_NMI_EN :1;      ///<BIT [25] m7_slave_port_write_perr_nmi_en
        uint32_t M7_ITCM_PERR_NMI_EN         :1;      ///<BIT [26] m7_itcm_perr_nmi_en
        uint32_t M7_DTCM_PERR_NMI_EN         :1;      ///<BIT [27] m7_dtcm_perr_nmi_en
        uint32_t RSVD1                       :1;      ///<BIT [28] rsvd1
        uint32_t M7_DTCM_ADDR_RANGE_ERR_NMI_EN :1;      ///<BIT [29] m7_dtcm_addr_range_err_nmi_en
        uint32_t RSVD0                       :2;      ///<BIT [31:30] rsvd0
    } b;
} FpsCpuCpuCfg0_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t M7_CFGSTCALIB               :26;     ///<BIT [25:0] m7_cfgstcalib
        uint32_t RSVD                        :6;      ///<BIT [31:26] rsvd_0
    } b;
} FpsCpuCpuCfg1_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t M7_CPUWAIT                  :1;      ///<BIT [0] m7_cpuwait
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} FpsCpuCpuControl_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INTERNAL_SW_IRQ             :8;      ///<BIT [7:0] internal_sw_irq
        uint32_t CPU_INTERNAL_ERROR          :1;      ///<BIT [8] cpu_internal_error
        uint32_t RSVD1                       :1;      ///<BIT [9] rsvd1
        uint32_t CPU_MEM_SOFT_ERROR          :1;      ///<BIT [10] cpu_mem_soft_error
        uint32_t FABRIC_MEM_SOFT_ERROR       :1;      ///<BIT [11] fabric_mem_soft_error
        uint32_t RSVD                        :20;     ///<BIT [31:12] rsvd_0
    } b;
} FpsCpuCpuIntrCause_t;

/// @brief 0x54
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INTERRUPT_ENABLE            :12;     ///<BIT [11:0] interrupt_enable
        uint32_t RSVD                        :20;     ///<BIT [31:12] rsvd_0
    } b;
} FpsCpuCpuIntrEnable_t;

/// @brief 0x58
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CPU_INTERNAL_SW_INTERRUPT_STATUS :8;      ///<BIT [7:0] cpu_internal_sw_interrupt_status
        uint32_t CPU_INTERNAL_SW_INTERRUPT_SET :8;      ///<BIT [15:8] cpu_internal_sw_interrupt_set
        uint32_t CPU_INTERNAL_SW_INTERRUPT_CLR :8;      ///<BIT [23:16] cpu_internal_sw_interrupt_clr
        uint32_t RSVD                        :8;      ///<BIT [31:24] rsvd_0
    } b;
} FpsCpuCpuSwIntrStatus_t;

/// @brief 0x60
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CPU_ITCM_SD                 :1;      ///<BIT [0] cpu_itcm_sd
        uint32_t CPU_ITCM_DSLP               :1;      ///<BIT [1] cpu_itcm_dslp
        uint32_t CPU_ITCM_SLP                :1;      ///<BIT [2] cpu_itcm_slp
        uint32_t RSVD0                       :1;      ///<BIT [3] rsvd0
        uint32_t CPU_DTCM_SD                 :1;      ///<BIT [4] cpu_dtcm_sd
        uint32_t CPU_DTCM_DSLP               :1;      ///<BIT [5] cpu_dtcm_dslp
        uint32_t CPU_DTCM_SLP                :1;      ///<BIT [6] cpu_dtcm_slp
        uint32_t RSVD1                       :1;      ///<BIT [7] rsvd1
        uint32_t CPU_ITCM_RTC                :2;      ///<BIT [9:8] cpu_itcm_rtc
        uint32_t CPU_ITCM_WTC                :2;      ///<BIT [11:10] cpu_itcm_wtc
        uint32_t CPU_BANK0_D0TCM_RTC         :2;      ///<BIT [13:12] cpu_bank0_d0tcm_rtc
        uint32_t CPU_BANK0_D0TCM_WTC         :2;      ///<BIT [15:14] cpu_bank0_d0tcm_wtc
        uint32_t CPU_BANK0_D1TCM_RTC         :2;      ///<BIT [17:16] cpu_bank0_d1tcm_rtc
        uint32_t CPU_BANK0_D1TCM_WTC         :2;      ///<BIT [19:18] cpu_bank0_d1tcm_wtc
        uint32_t CPU_BANK1_D0TCM_RTC         :2;      ///<BIT [21:20] cpu_bank1_d0tcm_rtc
        uint32_t CPU_BANK1_D0TCM_WTC         :2;      ///<BIT [23:22] cpu_bank1_d0tcm_wtc
        uint32_t CPU_BANK1_D1TCM_RTC         :2;      ///<BIT [25:24] cpu_bank1_d1tcm_rtc
        uint32_t CPU_BANK1_D1TCM_WTC         :2;      ///<BIT [27:26] cpu_bank1_d1tcm_wtc
        uint32_t CPU_DTCM_ECC_PROTECTION_MODE_ENABLE :1;      ///<BIT [28] cpu_dtcm_ecc_protection_mode_enable
        uint32_t CPU_DTCM_PROTECTION_CHECK_ENABLE :1;      ///<BIT [29] cpu_dtcm_protection_check_enable
        uint32_t CPU_ITCM_ECC_PROTECTION_MODE_ENABLE :1;      ///<BIT [30] cpu_itcm_ecc_protection_mode_enable
        uint32_t CPU_ITCM_PROTECTION_CHECK_ENABLE :1;      ///<BIT [31] cpu_itcm_protection_check_enable
    } b;
} FpsCpuCpuMemoryControl_t;

/// @brief 0x64
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CPU_ITCM_PROTECTION_ERROR   :1;      ///<BIT [0] cpu_itcm_protection_error
        uint32_t CPU_D0TCM_PROTECTION_ERROR  :1;      ///<BIT [1] cpu_d0tcm_protection_error
        uint32_t CPU_D1TCM_PROTECTION_ERROR  :1;      ///<BIT [2] cpu_d1tcm_protection_error
        uint32_t RSVD_2                      :5;      ///<BIT [7:3] rsvd_2
        uint32_t CPU_ITCM_ECC_CORRECTABLE_ERROR :1;      ///<BIT [8] cpu_itcm_ecc_correctable_error
        uint32_t CPU_D1TCM_ECC_CORRECTABLE_ERROR :1;      ///<BIT [9] cpu_d1tcm_ecc_correctable_error
        uint32_t CPU_D0TCM_ECC_CORRECTABLE_ERROR :1;      ///<BIT [10] cpu_d0tcm_ecc_correctable_error
        uint32_t RSVD_1                      :5;      ///<BIT [15:11] rsvd_1
        uint32_t CPU_D0TCM_ADDR_OUT_OF_RANGE_ERROR :1;      ///<BIT [16] cpu_d0tcm_addr_out_of_range_error
        uint32_t CPU_D1TCM_ADDR_OUT_OF_RANGE_ERROR :1;      ///<BIT [17] cpu_d1tcm_addr_out_of_range_error
        uint32_t RSVD                        :14;     ///<BIT [31:18] rsvd_0
    } b;
} FpsCpuCpuMemoryErrorStatus_t;

/// @brief 0x6C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_STATUS_PORT_SELECT    :2;      ///<BIT [1:0] error_status_port_select
        uint32_t RSVD                        :30;     ///<BIT [31:2] rsvd
    } b;
} FpsCpuCpuCapturedErrorStatusControl_t;

/// @brief 0x70
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TCM_ADDRESS_ERROR_BITS      :18;     ///<BIT [17:0] tcm_address_error_bits
        uint32_t RSVD                        :14;     ///<BIT [31:18] rsvd
    } b;
} FpsCpuCpuCapturedAddressErrorStatus_t;

/// @brief 0x7C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TCM_PROTECTION_ERROR_BITS   :8;      ///<BIT [7:0] tcm_protection_error_bits
        uint32_t RSVD2                       :8;      ///<BIT [15:8] rsvd2
        uint32_t TCM_CPU_MASTER_ID           :4;      ///<BIT [19:16] tcm_cpu_master_id
        uint32_t RSVD0                       :12;     ///<BIT [31:20] rsvd0
    } b;
} FpsCpuCpuCapturedProtectionErrorStatus_t;

/// @brief 0x80
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TCM_ECC_CORRECTABLE_ERROR_COUNT :16;     ///<BIT [15:0] tcm_ecc_correctable_error_count
        uint32_t TCM_ECC_UNCORRECTABLE_ERROR_COUNT :16;     ///<BIT [31:16] tcm_ecc_uncorrectable_error_count
    } b;
} FpsCpuCpuTcmEccErrorCount_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HWE2FP_WQ_00_UCD_IB_CQ0_EMPTY :1;      ///<BIT [0] hwe2fp_wq_00_ucd_ib_cq0_empty
        uint32_t HWE2FP_WQ_01_UCD_IB_CQ1_EMPTY :1;      ///<BIT [1] hwe2fp_wq_01_ucd_ib_cq1_empty
        uint32_t HWE2FP_WQ_02_UCD_OB_CQ0_EMPTY :1;      ///<BIT [2] hwe2fp_wq_02_ucd_ob_cq0_empty
        uint32_t HWE2FP_WQ_03_UCD_OB_CQ1_EMPTY :1;      ///<BIT [3] hwe2fp_wq_03_ucd_ob_cq1_empty
        uint32_t HWE2FP_WQ_04_CDMA_CQ_EMPTY  :1;      ///<BIT [4] hwe2fp_wq_04_cdma_cq_empty
        uint32_t HWE2FP_WQ_05_GDMA_CQ_EMPTY  :1;      ///<BIT [5] hwe2fp_wq_05_gdma_cq_empty
        uint32_t HWE2FP_WQ_06_CP2FP_CMD_SQ_EMPTY :1;      ///<BIT [6] hwe2fp_wq_06_cp2fp_cmd_sq_empty
        uint32_t HWE2FP_WQ_07_FP2CP_ERR_CQ_EMPTY :1;      ///<BIT [7] hwe2fp_wq_07_fp2cp_err_cq_empty
        uint32_t HWE2FP_WQ_08_EMPTY          :1;      ///<BIT [8] hwe2fp_wq_08_empty
        uint32_t HWE2FP_WQ_09_EMPTY          :1;      ///<BIT [9] hwe2fp_wq_09_empty
        uint32_t HWE2FP_WQ_10_EMPTY          :1;      ///<BIT [10] hwe2fp_wq_10_empty
        uint32_t HWE2FP_WQ_11_EMPTY          :1;      ///<BIT [11] hwe2fp_wq_11_empty
        uint32_t FP2HWE_WQ_00_FULL           :1;      ///<BIT [12] fp2hwe_wq_00_full
        uint32_t FP2HWE_WQ_01_FULL           :1;      ///<BIT [13] fp2hwe_wq_01_full
        uint32_t FP2HWE_WQ_02_FULL           :1;      ///<BIT [14] fp2hwe_wq_02_full
        uint32_t FP2HWE_WQ_03_FULL           :1;      ///<BIT [15] fp2hwe_wq_03_full
        uint32_t FP2HWE_WQ_04_CDMA_SQ_FULL   :1;      ///<BIT [16] fp2hwe_wq_04_cdma_sq_full
        uint32_t FP2HWE_WQ_05_GDMA_SQ_FULL   :1;      ///<BIT [17] fp2hwe_wq_05_gdma_sq_full
        uint32_t FP2HWE_WQ_06_CP2FP_CMD_CQ_FULL :1;      ///<BIT [18] fp2hwe_wq_06_cp2fp_cmd_cq_full
        uint32_t FP2HWE_WQ_07_FP2CP_ERR_SQ_FULL :1;      ///<BIT [19] fp2hwe_wq_07_fp2cp_err_sq_full
        uint32_t FP2HWE_WQ_08_FULL           :1;      ///<BIT [20] fp2hwe_wq_08_full
        uint32_t FP2HWE_WQ_09_FULL           :1;      ///<BIT [21] fp2hwe_wq_09_full
        uint32_t FP2HWE_WQ_10_FULL           :1;      ///<BIT [22] fp2hwe_wq_10_full
        uint32_t FP2HWE_WQ_11_FULL           :1;      ///<BIT [23] fp2hwe_wq_11_full
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_7_0   :1;      ///<BIT [24] cpux_to_cpuy_wq_empty_7_0
        uint32_t CPUX_TO_CPUY_WQ_FULL_7_0    :1;      ///<BIT [25] cpux_to_cpuy_wq_full_7_0
        uint32_t SLOT_ARRAY_EMPTY_131_0      :1;      ///<BIT [26] slot_array_empty_131_0
        uint32_t SLOT_ARRAY_FULL_131_0       :1;      ///<BIT [27] slot_array_full_131_0
        uint32_t RSVD0                       :1;      ///<BIT [28] rsvd0
        uint32_t RSVD1                       :1;      ///<BIT [29] rsvd1
        uint32_t FPS_FABRIC_PARITY_ERROR     :1;      ///<BIT [30] fps_fabric_parity_error
        uint32_t FPS_MEMORY_PROTECTION_ERROR :1;      ///<BIT [31] fps_memory_protection_error
    } b;
} FpsBank1EventStatus0_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HWE2FP_WQ_00_UCD_IB_CQ0_FULL :1;      ///<BIT [0] hwe2fp_wq_00_ucd_ib_cq0_full
        uint32_t HWE2FP_WQ_01_UCD_IB_CQ1_FULL :1;      ///<BIT [1] hwe2fp_wq_01_ucd_ib_cq1_full
        uint32_t HWE2FP_WQ_02_UCD_OB_CQ0_FULL :1;      ///<BIT [2] hwe2fp_wq_02_ucd_ob_cq0_full
        uint32_t HWE2FP_WQ_03_UCD_OB_CQ1_FULL :1;      ///<BIT [3] hwe2fp_wq_03_ucd_ob_cq1_full
        uint32_t HWE2FP_WQ_04_CDMA_CQ_FULL   :1;      ///<BIT [4] hwe2fp_wq_04_cdma_cq_full
        uint32_t HWE2FP_WQ_05_GDMA_CQ_FULL   :1;      ///<BIT [5] hwe2fp_wq_05_gdma_cq_full
        uint32_t HWE2FP_WQ_06_CP2FP_CMD_SQ_FULL :1;      ///<BIT [6] hwe2fp_wq_06_cp2fp_cmd_sq_full
        uint32_t HWE2FP_WQ_07_FP2CP_ERR_CQ_FULL :1;      ///<BIT [7] hwe2fp_wq_07_fp2cp_err_cq_full
        uint32_t HWE2FP_WQ_08_FULL           :1;      ///<BIT [8] hwe2fp_wq_08_full
        uint32_t HWE2FP_WQ_09_FULL           :1;      ///<BIT [9] hwe2fp_wq_09_full
        uint32_t HWE2FP_WQ_10_FULL           :1;      ///<BIT [10] hwe2fp_wq_10_full
        uint32_t HWE2FP_WQ_11_FULL           :1;      ///<BIT [11] hwe2fp_wq_11_full
        uint32_t FP2HWE_WQ_00_EMPTY          :1;      ///<BIT [12] fp2hwe_wq_00_empty
        uint32_t FP2HWE_WQ_01_EMPTY          :1;      ///<BIT [13] fp2hwe_wq_01_empty
        uint32_t FP2HWE_WQ_02_EMPTY          :1;      ///<BIT [14] fp2hwe_wq_02_empty
        uint32_t FP2HWE_WQ_03_EMPTY          :1;      ///<BIT [15] fp2hwe_wq_03_empty
        uint32_t FP2HWE_WQ_04_CDMA_SQ_EMPTY  :1;      ///<BIT [16] fp2hwe_wq_04_cdma_sq_empty
        uint32_t FP2HWE_WQ_05_GDMA_SQ_EMPTY  :1;      ///<BIT [17] fp2hwe_wq_05_gdma_sq_empty
        uint32_t FP2HWE_WQ_06_CP2FP_CMD_CQ_EMPTY :1;      ///<BIT [18] fp2hwe_wq_06_cp2fp_cmd_cq_empty
        uint32_t FP2HWE_WQ_07_FP2CP_ERR_SQ_EMPTY :1;      ///<BIT [19] fp2hwe_wq_07_fp2cp_err_sq_empty
        uint32_t FP2HWE_WQ_08_EMPTY          :1;      ///<BIT [20] fp2hwe_wq_08_empty
        uint32_t FP2HWE_WQ_09_EMPTY          :1;      ///<BIT [21] fp2hwe_wq_09_empty
        uint32_t FP2HWE_WQ_10_EMPTY          :1;      ///<BIT [22] fp2hwe_wq_10_empty
        uint32_t FP2HWE_WQ_11_EMPTY          :1;      ///<BIT [23] fp2hwe_wq_11_empty
        uint32_t RSVD                        :8;      ///<BIT [31:24] rsvd
    } b;
} FpsBank1EventStatus1_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_0     :1;      ///<BIT [0] cpux_to_cpuy_wq_empty_0
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_1     :1;      ///<BIT [1] cpux_to_cpuy_wq_empty_1
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_2     :1;      ///<BIT [2] cpux_to_cpuy_wq_empty_2
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_3     :1;      ///<BIT [3] cpux_to_cpuy_wq_empty_3
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_4     :1;      ///<BIT [4] cpux_to_cpuy_wq_empty_4
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_5     :1;      ///<BIT [5] cpux_to_cpuy_wq_empty_5
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_6     :1;      ///<BIT [6] cpux_to_cpuy_wq_empty_6
        uint32_t CPUX_TO_CPUY_WQ_EMPTY_7     :1;      ///<BIT [7] cpux_to_cpuy_wq_empty_7
        uint32_t RSVD0                       :8;      ///<BIT [15:8] rsvd0
        uint32_t CPUX_TO_CPUY_WQ_FULL_0      :1;      ///<BIT [16] cpux_to_cpuy_wq_full_0
        uint32_t CPUX_TO_CPUY_WQ_FULL_1      :1;      ///<BIT [17] cpux_to_cpuy_wq_full_1
        uint32_t CPUX_TO_CPUY_WQ_FULL_2      :1;      ///<BIT [18] cpux_to_cpuy_wq_full_2
        uint32_t CPUX_TO_CPUY_WQ_FULL_3      :1;      ///<BIT [19] cpux_to_cpuy_wq_full_3
        uint32_t CPUX_TO_CPUY_WQ_FULL_4      :1;      ///<BIT [20] cpux_to_cpuy_wq_full_4
        uint32_t CPUX_TO_CPUY_WQ_FULL_5      :1;      ///<BIT [21] cpux_to_cpuy_wq_full_5
        uint32_t CPUX_TO_CPUY_WQ_FULL_6      :1;      ///<BIT [22] cpux_to_cpuy_wq_full_6
        uint32_t CPUX_TO_CPUY_WQ_FULL_7      :1;      ///<BIT [23] cpux_to_cpuy_wq_full_7
        uint32_t RSVD1                       :8;      ///<BIT [31:24] rsvd1
    } b;
} FpsBank1CpuxToCpuyWqStatus_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SLOT_ARRAY_GROUP_EMPTY_7_0  :1;      ///<BIT [0] slot_array_group_empty_7_0
        uint32_t SLOT_ARRAY_GROUP_EMPTY_15_8 :1;      ///<BIT [1] slot_array_group_empty_15_8
        uint32_t SLOT_ARRAY_GROUP_EMPTY_23_16 :1;      ///<BIT [2] slot_array_group_empty_23_16
        uint32_t SLOT_ARRAY_GROUP_EMPTY_31_24 :1;      ///<BIT [3] slot_array_group_empty_31_24
        uint32_t SLOT_ARRAY_GROUP_EMPTY_39_32 :1;      ///<BIT [4] slot_array_group_empty_39_32
        uint32_t SLOT_ARRAY_GROUP_EMPTY_47_40 :1;      ///<BIT [5] slot_array_group_empty_47_40
        uint32_t SLOT_ARRAY_GROUP_EMPTY_55_48 :1;      ///<BIT [6] slot_array_group_empty_55_48
        uint32_t SLOT_ARRAY_GROUP_EMPTY_63_56 :1;      ///<BIT [7] slot_array_group_empty_63_56
        uint32_t SLOT_ARRAY_GROUP_EMPTY_71_64 :1;      ///<BIT [8] slot_array_group_empty_71_64
        uint32_t SLOT_ARRAY_GROUP_EMPTY_79_72 :1;      ///<BIT [9] slot_array_group_empty_79_72
        uint32_t SLOT_ARRAY_GROUP_EMPTY_87_80 :1;      ///<BIT [10] slot_array_group_empty_87_80
        uint32_t SLOT_ARRAY_GROUP_EMPTY_95_88 :1;      ///<BIT [11] slot_array_group_empty_95_88
        uint32_t SLOT_ARRAY_GROUP_EMPTY_103_96 :1;      ///<BIT [12] slot_array_group_empty_103_96
        uint32_t SLOT_ARRAY_GROUP_EMPTY_111_104 :1;      ///<BIT [13] slot_array_group_empty_111_104
        uint32_t SLOT_ARRAY_GROUP_EMPTY_119_112 :1;      ///<BIT [14] slot_array_group_empty_119_112
        uint32_t SLOT_ARRAY_GROUP_EMPTY_127_120 :1;      ///<BIT [15] slot_array_group_empty_127_120
        uint32_t SLOT_ARRAY_GROUP_EMPTY_131_128 :1;      ///<BIT [16] slot_array_group_empty_131_128
        uint32_t RSVD                        :15;     ///<BIT [31:17] rsvd_0
    } b;
} FpsBank1SlotArrayGroupEmptyStatus_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SLOT_ARRAY_GROUP_FULL_7_0   :1;      ///<BIT [0] slot_array_group_full_7_0
        uint32_t SLOT_ARRAY_GROUP_FULL_15_8  :1;      ///<BIT [1] slot_array_group_full_15_8
        uint32_t SLOT_ARRAY_GROUP_FULL_23_16 :1;      ///<BIT [2] slot_array_group_full_23_16
        uint32_t SLOT_ARRAY_GROUP_FULL_31_24 :1;      ///<BIT [3] slot_array_group_full_31_24
        uint32_t SLOT_ARRAY_GROUP_FULL_39_32 :1;      ///<BIT [4] slot_array_group_full_39_32
        uint32_t SLOT_ARRAY_GROUP_FULL_47_40 :1;      ///<BIT [5] slot_array_group_full_47_40
        uint32_t SLOT_ARRAY_GROUP_FULL_55_48 :1;      ///<BIT [6] slot_array_group_full_55_48
        uint32_t SLOT_ARRAY_GROUP_FULL_63_56 :1;      ///<BIT [7] slot_array_group_full_63_56
        uint32_t SLOT_ARRAY_GROUP_FULL_71_64 :1;      ///<BIT [8] slot_array_group_full_71_64
        uint32_t SLOT_ARRAY_GROUP_FULL_79_72 :1;      ///<BIT [9] slot_array_group_full_79_72
        uint32_t SLOT_ARRAY_GROUP_FULL_87_80 :1;      ///<BIT [10] slot_array_group_full_87_80
        uint32_t SLOT_ARRAY_GROUP_FULL_95_88 :1;      ///<BIT [11] slot_array_group_full_95_88
        uint32_t SLOT_ARRAY_GROUP_FULL_103_96 :1;      ///<BIT [12] slot_array_group_full_103_96
        uint32_t SLOT_ARRAY_GROUP_FULL_111_104 :1;      ///<BIT [13] slot_array_group_full_111_104
        uint32_t SLOT_ARRAY_GROUP_FULL_119_112 :1;      ///<BIT [14] slot_array_group_full_119_112
        uint32_t SLOT_ARRAY_GROUP_FULL_127_120 :1;      ///<BIT [15] slot_array_group_full_127_120
        uint32_t SLOT_ARRAY_GROUP_FULL_131_128 :1;      ///<BIT [16] slot_array_group_full_131_128
        uint32_t RSVD                        :15;     ///<BIT [31:17] rsvd_0
    } b;
} FpsBank1SlotArrayGroupFullStatus_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SLOT_ARRAY_Q_EMPTY_131_128  :4;      ///<BIT [3:0] slot_array_q_empty_131_128
        uint32_t RSVD                        :28;     ///<BIT [31:4] rsvd_0
    } b;
} FpsBank1SlotArrayQueueEmptyStatus4_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SLOT_ARRAY_Q_FULL_131_128   :4;      ///<BIT [3:0] slot_array_q_full_131_128
        uint32_t RSVD                        :28;     ///<BIT [31:4] rsvd_0
    } b;
} FpsBank1SlotArrayQueueFullStatus4_t;

/// @brief 0x70
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PSRAM_SD                    :1;      ///<BIT [0] psram_sd
        uint32_t PSRAM_DSLP                  :1;      ///<BIT [1] psram_dslp
        uint32_t PSRAM_SLP                   :1;      ///<BIT [2] psram_slp
        uint32_t RSVD0                       :5;      ///<BIT [7:3] rsvd0
        uint32_t PSRAM_RTC                   :2;      ///<BIT [9:8] psram_rtc
        uint32_t PSRAM_WTC                   :2;      ///<BIT [11:10] psram_wtc
        uint32_t RSVD1                       :4;      ///<BIT [15:12] rsvd1
        uint32_t PSRAM_ECC_PROTECTION_MODE_ENABLE :1;      ///<BIT [16] psram_ecc_protection_mode_enable
        uint32_t PSRAM_PROTECTION_CHECK_ENABLE :1;      ///<BIT [17] psram_protection_check_enable
        uint32_t PSRAM_PROTECTION_WRITE_DISABLE :1;      ///<BIT [18] psram_protection_write_disable
        uint32_t RSVD2                       :5;      ///<BIT [23:19] rsvd2
        uint32_t PSRAM_INIT_EN               :1;      ///<BIT [24] psram_init_en
        uint32_t PSRAM_SCRUB_EN              :1;      ///<BIT [25] psram_scrub_en
        uint32_t RSVD3                       :6;      ///<BIT [31:26] rsvd3
    } b;
} FpsBank1Psram0MemoryControl_t;

/// @brief 0x80
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ECC_CORRECTABLE_ERROR_COUNT_THRESHOLD :3;      ///<BIT [2:0] ecc_correctable_error_count_threshold
        uint32_t ELEVATE_ECC_CORRECTABLE_ERROR2FATAL :1;      ///<BIT [3] elevate_ecc_correctable_error2fatal
        uint32_t RSVD0                       :1;      ///<BIT [4] rsvd0
        uint32_t CPU_ITCM_PROTECTION_WRITE_DISABLE :1;      ///<BIT [5] cpu_itcm_protection_write_disable
        uint32_t CPU_DTCM_PROTECTION_WRITE_DISABLE :1;      ///<BIT [6] cpu_dtcm_protection_write_disable
        uint32_t CPU_TCM_ECC_UNCORR_ERROR_CONT_RETRY_EN :1;      ///<BIT [7] cpu_tcm_ecc_uncorr_error_cont_retry_en
        uint32_t CPU_TCM_ECC_CORRECTABLE_ERROR_WRTBK_EN :1;      ///<BIT [8] cpu_tcm_ecc_correctable_error_wrtbk_en
        uint32_t PSRAM_ECC_CORRECTABLE_ERROR_WRTBK_EN :1;      ///<BIT [9] psram_ecc_correctable_error_wrtbk_en
        uint32_t RSVD1                       :6;      ///<BIT [15:10] rsvd1
        uint32_t PSRAM_MEM_INIT_PATTERN_SELECT :1;      ///<BIT [16] psram_mem_init_pattern_select
        uint32_t PSRAM_ECC_PARTIAL_WRITE_RMW_EN :1;      ///<BIT [17] psram_ecc_partial_write_rmw_en
        uint32_t RSVD2                       :14;     ///<BIT [31:18] rsvd2
    } b;
} FpsBank1FpsMemoryControl_t;

/// @brief 0x84
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CPU0_MEMORY_ERROR           :1;      ///<BIT [0] cpu0_memory_error
        uint32_t CPU1_MEMORY_ERROR           :1;      ///<BIT [1] cpu1_memory_error
        uint32_t CPU2_MEMORY_ERROR           :1;      ///<BIT [2] cpu2_memory_error
        uint32_t RSVD0                       :13;     ///<BIT [15:3] rsvd0
        uint32_t PSRAM0_PROTECTION_ERROR     :1;      ///<BIT [16] psram0_protection_error
        uint32_t RSVD1                       :7;      ///<BIT [23:17] rsvd1
        uint32_t PSRAM0_ECC_CORRECTABLE_ERROR :1;      ///<BIT [24] psram0_ecc_correctable_error
        uint32_t RSVD2                       :7;      ///<BIT [31:25] rsvd2
    } b;
} FpsBank1MemoryErrorStatus_t;

/// @brief 0x90
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FABRIC_PSRAM0_S_PORT_WR_PARITY_CHECK_ENABLE :1;      ///<BIT [0] fabric_psram0_s_port_wr_parity_check_enable
        uint32_t RSVD0                       :7;      ///<BIT [7:1] rsvd0
        uint32_t FABRIC_CPU0_M_PORT_PARITY_CHECK_ENABLE :1;      ///<BIT [8] fabric_cpu0_m_port_parity_check_enable
        uint32_t FABRIC_CPU1_M_PORT_PARITY_CHECK_ENABLE :1;      ///<BIT [9] fabric_cpu1_m_port_parity_check_enable
        uint32_t FABRIC_CPU2_M_PORT_PARITY_CHECK_ENABLE :1;      ///<BIT [10] fabric_cpu2_m_port_parity_check_enable
        uint32_t RSVD1                       :5;      ///<BIT [15:11] rsvd1
        uint32_t FABRIC_CPU0_S_PORT_PARITY_CHECK_ENABLE :1;      ///<BIT [16] fabric_cpu0_s_port_parity_check_enable
        uint32_t FABRIC_CPU1_S_PORT_PARITY_CHECK_ENABLE :1;      ///<BIT [17] fabric_cpu1_s_port_parity_check_enable
        uint32_t FABRIC_CPU2_S_PORT_PARITY_CHECK_ENABLE :1;      ///<BIT [18] fabric_cpu2_s_port_parity_check_enable
        uint32_t RSVD2                       :5;      ///<BIT [23:19] rsvd2
        uint32_t FABRIC_REG_M_PORT_PARITY_CHECK_ENABLE :1;      ///<BIT [24] fabric_reg_m_port_parity_check_enable
        uint32_t FABRIC_REG_S_PORT_PARITY_CHECK_ENABLE :1;      ///<BIT [25] fabric_reg_s_port_parity_check_enable
        uint32_t FABRIC_REG_M_PORT_RESPONSE_CHECK_ENABLE :1;      ///<BIT [26] fabric_reg_m_port_response_check_enable
        uint32_t RSVD3                       :1;      ///<BIT [27] rsvd3
        uint32_t FABRIC_MAIN_AXI_PORT_PARITY_CHECK_ENABLE :1;      ///<BIT [28] fabric_main_axi_port_parity_check_enable
        uint32_t RSVD4                       :1;      ///<BIT [29] rsvd4
        uint32_t FORCE_MF_AXI_SLV_RD_FWD_ERR :1;      ///<BIT [30] force_mf_axi_slv_rd_fwd_err
        uint32_t FORCE_MF_AXI_MST_WR_FWD_ERR :1;      ///<BIT [31] force_mf_axi_mst_wr_fwd_err
    } b;
} FpsBank1FabricErrorControl_t;

/// @brief 0x94
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD0                       :1;      ///<BIT [0] rsvd0
        uint32_t FABRIC_PSRAM0_S_PORT_WR_PARITY_ERROR :1;      ///<BIT [1] fabric_psram0_s_port_wr_parity_error
        uint32_t RSVD1                       :6;      ///<BIT [7:2] rsvd1
        uint32_t FABRIC_CPU0_M_PORT_PARITY_ERROR :1;      ///<BIT [8] fabric_cpu0_m_port_parity_error
        uint32_t FABRIC_CPU1_M_PORT_PARITY_ERROR :1;      ///<BIT [9] fabric_cpu1_m_port_parity_error
        uint32_t FABRIC_CPU2_M_PORT_PARITY_ERROR :1;      ///<BIT [10] fabric_cpu2_m_port_parity_error
        uint32_t RSVD2                       :5;      ///<BIT [15:11] rsvd2
        uint32_t FABRIC_CPU0_S_PORT_PARITY_ERROR :1;      ///<BIT [16] fabric_cpu0_s_port_parity_error
        uint32_t FABRIC_CPU1_S_PORT_PARITY_ERROR :1;      ///<BIT [17] fabric_cpu1_s_port_parity_error
        uint32_t FABRIC_CPU2_S_PORT_PARITY_ERROR :1;      ///<BIT [18] fabric_cpu2_s_port_parity_error
        uint32_t RSVD3                       :5;      ///<BIT [23:19] rsvd3
        uint32_t FABRIC_REG_M_PORT_PARITY_ERROR :1;      ///<BIT [24] fabric_reg_m_port_parity_error
        uint32_t FABRIC_REG_S_PORT_PARITY_ERROR :1;      ///<BIT [25] fabric_reg_s_port_parity_error
        uint32_t FABRIC_REG_M_PORT_RESPONSE_ERROR :1;      ///<BIT [26] fabric_reg_m_port_response_error
        uint32_t RSVD4                       :1;      ///<BIT [27] rsvd4
        uint32_t FABRIC_MAIN_AXI_PORT_PARITY_ERROR :1;      ///<BIT [28] fabric_main_axi_port_parity_error
        uint32_t RSVD5                       :3;      ///<BIT [31:29] rsvd5
    } b;
} FpsBank1FabricErrorStatus_t;

/// @brief 0xA0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ERROR_STATUS_PORT_SELECT    :6;      ///<BIT [5:0] error_status_port_select
        uint32_t RSVD0                       :26;     ///<BIT [31:6] rsvd0
    } b;
} FpsBank1FabricCapturedErrorStatusControl_t;

/// @brief 0xB0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PROTECTION_ERROR_BITS       :14;     ///<BIT [13:0] protection_error_bits
        uint32_t RSVD0                       :18;     ///<BIT [31:14] rsvd0
    } b;
} FpsBank1FabricCapturedProtectionErrorStatus_t;

/// @brief 0xB4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PSRAM_ECC_CORRECTABLE_ERROR_COUNT :16;     ///<BIT [15:0] psram_ecc_correctable_error_count
        uint32_t PSRAM_ECC_UNCORRECTABLE_ERROR_COUNT :16;     ///<BIT [31:16] psram_ecc_uncorrectable_error_count
    } b;
} FpsBank1PsramEccErrorCount_t;

/// @brief 0xC0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MF_AXI_MST_RD_PERR          :1;      ///<BIT [0] mf_axi_mst_rd_perr
        uint32_t MF_AXI_MST_WR_PERR          :1;      ///<BIT [1] mf_axi_mst_wr_perr
        uint32_t MF_AXI_SLV_RD_PERR          :1;      ///<BIT [2] mf_axi_slv_rd_perr
        uint32_t MF_AXI_SLV_WR_PERR          :1;      ///<BIT [3] mf_axi_slv_wr_perr
        uint32_t MF_AXI_MST_RD_FERR          :1;      ///<BIT [4] mf_axi_mst_rd_ferr
        uint32_t MF_AXI_SLV_WR_FERR          :1;      ///<BIT [5] mf_axi_slv_wr_ferr
        uint32_t MF_AXI_FERR_CHK_EN          :1;      ///<BIT [6] mf_axi_ferr_chk_en
        uint32_t MF_AXI_FERR2PERR_EN         :1;      ///<BIT [7] mf_axi_ferr2perr_en
        uint32_t MF_AXI_SLV_RD_FORCE_PERROR_ONCE :1;      ///<BIT [8] mf_axi_slv_rd_force_perror_once
        uint32_t MF_AXI_SLV_RD_FORCE_PERROR_CONT :1;      ///<BIT [9] mf_axi_slv_rd_force_perror_cont
        uint32_t MF_AXI_SLV_WR_FORCE_PERROR_ONCE :1;      ///<BIT [10] mf_axi_slv_wr_force_perror_once
        uint32_t MF_AXI_SLV_WR_FORCE_PERROR_CONT :1;      ///<BIT [11] mf_axi_slv_wr_force_perror_cont
        uint32_t MF_AXI_MST_RD_FORCE_PERROR_ONCE :1;      ///<BIT [12] mf_axi_mst_rd_force_perror_once
        uint32_t MF_AXI_MST_RD_FORCE_PERROR_CONT :1;      ///<BIT [13] mf_axi_mst_rd_force_perror_cont
        uint32_t MF_AXI_MST_WR_FORCE_PERROR_ONCE :1;      ///<BIT [14] mf_axi_mst_wr_force_perror_once
        uint32_t MF_AXI_MST_WR_FORCE_PERROR_CONT :1;      ///<BIT [15] mf_axi_mst_wr_force_perror_cont
        uint32_t MF_AXI_PERROR_MASK          :16;     ///<BIT [31:16] mf_axi_perror_mask
    } b;
} FpsBank1MainFabricAxiErrorStatusControl_t;

/// @brief 0xF0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HWE2FP_Q_CI_00_WR_STATUS    :1;      ///<BIT [0] hwe2fp_q_ci_00_wr_status
        uint32_t HWE2FP_Q_CI_01_WR_STATUS    :1;      ///<BIT [1] hwe2fp_q_ci_01_wr_status
        uint32_t HWE2FP_Q_CI_02_WR_STATUS    :1;      ///<BIT [2] hwe2fp_q_ci_02_wr_status
        uint32_t HWE2FP_Q_CI_03_WR_STATUS    :1;      ///<BIT [3] hwe2fp_q_ci_03_wr_status
        uint32_t HWE2FP_Q_CI_04_WR_STATUS    :1;      ///<BIT [4] hwe2fp_q_ci_04_wr_status
        uint32_t HWE2FP_Q_CI_05_WR_STATUS    :1;      ///<BIT [5] hwe2fp_q_ci_05_wr_status
        uint32_t HWE2FP_Q_CI_06_WR_STATUS    :1;      ///<BIT [6] hwe2fp_q_ci_06_wr_status
        uint32_t HWE2FP_Q_CI_07_WR_STATUS    :1;      ///<BIT [7] hwe2fp_q_ci_07_wr_status
        uint32_t HWE2FP_Q_CI_08_WR_STATUS    :1;      ///<BIT [8] hwe2fp_q_ci_08_wr_status
        uint32_t HWE2FP_Q_CI_09_WR_STATUS    :1;      ///<BIT [9] hwe2fp_q_ci_09_wr_status
        uint32_t HWE2FP_Q_CI_10_WR_STATUS    :1;      ///<BIT [10] hwe2fp_q_ci_10_wr_status
        uint32_t HWE2FP_Q_CI_11_WR_STATUS    :1;      ///<BIT [11] hwe2fp_q_ci_11_wr_status
        uint32_t FP2HWE_Q_PI_00_WR_STATUS    :1;      ///<BIT [12] fp2hwe_q_pi_00_wr_status
        uint32_t FP2HWE_Q_PI_01_WR_STATUS    :1;      ///<BIT [13] fp2hwe_q_pi_01_wr_status
        uint32_t FP2HWE_Q_PI_02_WR_STATUS    :1;      ///<BIT [14] fp2hwe_q_pi_02_wr_status
        uint32_t FP2HWE_Q_PI_03_WR_STATUS    :1;      ///<BIT [15] fp2hwe_q_pi_03_wr_status
        uint32_t FP2HWE_Q_PI_04_WR_STATUS    :1;      ///<BIT [16] fp2hwe_q_pi_04_wr_status
        uint32_t FP2HWE_Q_PI_05_WR_STATUS    :1;      ///<BIT [17] fp2hwe_q_pi_05_wr_status
        uint32_t FP2HWE_Q_PI_06_WR_STATUS    :1;      ///<BIT [18] fp2hwe_q_pi_06_wr_status
        uint32_t FP2HWE_Q_PI_07_WR_STATUS    :1;      ///<BIT [19] fp2hwe_q_pi_07_wr_status
        uint32_t FP2HWE_Q_PI_08_WR_STATUS    :1;      ///<BIT [20] fp2hwe_q_pi_08_wr_status
        uint32_t FP2HWE_Q_PI_09_WR_STATUS    :1;      ///<BIT [21] fp2hwe_q_pi_09_wr_status
        uint32_t FP2HWE_Q_PI_10_WR_STATUS    :1;      ///<BIT [22] fp2hwe_q_pi_10_wr_status
        uint32_t FP2HWE_Q_PI_11_WR_STATUS    :1;      ///<BIT [23] fp2hwe_q_pi_11_wr_status
        uint32_t SOC_REG_0_WR_STATUS         :1;      ///<BIT [24] soc_reg_0_wr_status
        uint32_t SOC_REG_1_WR_STATUS         :1;      ///<BIT [25] soc_reg_1_wr_status
        uint32_t SOC_REG_2_WR_STATUS         :1;      ///<BIT [26] soc_reg_2_wr_status
        uint32_t SOC_REG_3_WR_STATUS         :1;      ///<BIT [27] soc_reg_3_wr_status
        uint32_t SOC_REG_4_WR_STATUS         :1;      ///<BIT [28] soc_reg_4_wr_status
        uint32_t SOC_REG_5_WR_STATUS         :1;      ///<BIT [29] soc_reg_5_wr_status
        uint32_t SOC_REG_6_WR_STATUS         :1;      ///<BIT [30] soc_reg_6_wr_status
        uint32_t SOC_REG_7_WR_STATUS         :1;      ///<BIT [31] soc_reg_7_wr_status
    } b;
} FpsBank1IndirectRegisterWriteStatus_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t Q_PI                        :11;     ///<BIT [10:0] q_pi
        uint32_t RSVD                        :21;     ///<BIT [31:11] rsvd_0
    } b;
} FpsSlotArrayQueueProducerIndex_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t Q_CI                        :11;     ///<BIT [10:0] q_ci
        uint32_t RSVD                        :21;     ///<BIT [31:11] rsvd_0
    } b;
} FpsSlotArrayQueueConsumerIndex_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t Q_SIZE                      :4;      ///<BIT [3:0] q_size
        uint32_t RSVD                        :28;     ///<BIT [31:4] rsvd_0
    } b;
} FpsSlotArrayQueueSize_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t Q_EMPTY                     :1;      ///<BIT [0] q_empty
        uint32_t Q_FULL                      :1;      ///<BIT [1] q_full
        uint32_t RSVD_1                      :14;     ///<BIT [15:2] rsvd_1
        uint32_t Q_STORED_COUNT              :11;     ///<BIT [26:16] q_stored_count
        uint32_t RSVD                        :5;      ///<BIT [31:27] rsvd_0
    } b;
} FpsSlotArrayQueueStatus_t;

/// @brief 0x5000
typedef struct
{
    FpsSlotArrayQueueProducerIndex_t fpsSlotArrayQueueProducerIndex; //fps_slot_array_reg_queue_producer_index
    FpsSlotArrayQueueConsumerIndex_t fpsSlotArrayQueueConsumerIndex; //fps_slot_array_reg_queue_consumer_index
    FpsSlotArrayQueueSize_t fpsSlotArrayQueueSize; //fps_slot_array_reg_queue_size
    FpsSlotArrayQueueStatus_t fpsSlotArrayQueueStatus; //fps_slot_array_reg_queue_status
} FpsSlotArrayRegRegisters_t;

/// @brief 0x4000
typedef struct
{
    FpsBank1EventStatus0_t fpsBank1EventStatus0; //fps_bank1_reg_event_status_0
    FpsBank1EventStatus1_t fpsBank1EventStatus1; //fps_bank1_reg_event_status_1
    FpsBank1CpuxToCpuyWqStatus_t fpsBank1CpuxToCpuyWqStatus; //fps_bank1_reg_cpux_to_cpuy_wq_status
    uint8_t rsvdC[4];                     //rsvd_c
    FpsBank1SlotArrayGroupEmptyStatus_t fpsBank1SlotArrayGroupEmptyStatus; //fps_bank1_reg_slot_array_group_empty_status
    FpsBank1SlotArrayGroupFullStatus_t fpsBank1SlotArrayGroupFullStatus; //fps_bank1_reg_slot_array_group_full_status
    uint8_t rsvd18[8];                    //rsvd_18
    uint32_t fpsBank1SlotArrayQueueEmptyStatus0SlotArrayQEmpty310; //fps_bank1_reg_slot_array_queue_empty_status_0
    uint32_t fpsBank1SlotArrayQueueEmptyStatus1SlotArrayQEmpty6332; //fps_bank1_reg_slot_array_queue_empty_status_1
    uint32_t fpsBank1SlotArrayQueueEmptyStatus2SlotArrayQEmpty9564; //fps_bank1_reg_slot_array_queue_empty_status_2
    uint32_t fpsBank1SlotArrayQueueEmptyStatus3SlotArrayQEmpty12796; //fps_bank1_reg_slot_array_queue_empty_status_3
    FpsBank1SlotArrayQueueEmptyStatus4_t fpsBank1SlotArrayQueueEmptyStatus4; //fps_bank1_reg_slot_array_queue_empty_status_4
    uint8_t rsvd34[12];                   //rsvd_34
    uint32_t fpsBank1SlotArrayQueueFullStatus0SlotArrayQFull310; //fps_bank1_reg_slot_array_queue_full_status_0
    uint32_t fpsBank1SlotArrayQueueFullStatus1SlotArrayQFull6332; //fps_bank1_reg_slot_array_queue_full_status_1
    uint32_t fpsBank1SlotArrayQueueFullStatus2SlotArrayQFull9564; //fps_bank1_reg_slot_array_queue_full_status_2
    uint32_t fpsBank1SlotArrayQueueFullStatus3SlotArrayQFull12796; //fps_bank1_reg_slot_array_queue_full_status_3
    FpsBank1SlotArrayQueueFullStatus4_t fpsBank1SlotArrayQueueFullStatus4; //fps_bank1_reg_slot_array_queue_full_status_4
    uint8_t rsvd54[28];                   //rsvd_54
    FpsBank1Psram0MemoryControl_t fpsBank1Psram0MemoryControl; //fps_bank1_reg_psram0_memory_control
    uint8_t rsvd74[12];                   //rsvd_74
    FpsBank1FpsMemoryControl_t fpsBank1FpsMemoryControl; //fps_bank1_reg_fps_memory_control
    FpsBank1MemoryErrorStatus_t fpsBank1MemoryErrorStatus; //fps_bank1_reg_memory_error_status
    uint8_t rsvd88[8];                    //rsvd_88
    FpsBank1FabricErrorControl_t fpsBank1FabricErrorControl; //fps_bank1_reg_fabric_error_control
    FpsBank1FabricErrorStatus_t fpsBank1FabricErrorStatus; //fps_bank1_reg_fabric_error_status
    uint8_t rsvd98[8];                    //rsvd_98
    FpsBank1FabricCapturedErrorStatusControl_t fpsBank1FabricCapturedErrorStatusControl; //fps_bank1_reg_fabric_captured_error_status_control
    uint32_t fpsBank1FabricCapturedAddressErrorStatusAddressErrorBits; //fps_bank1_reg_fabric_captured_address_error_status
    uint32_t fpsBank1FabricCapturedDataErrorStatus0DataErrorBitsLwr32; //fps_bank1_reg_fabric_captured_data_error_status_0
    uint32_t fpsBank1FabricCapturedDataErrorStatus1DataErrorBitsUpr32; //fps_bank1_reg_fabric_captured_data_error_status_1
    FpsBank1FabricCapturedProtectionErrorStatus_t fpsBank1FabricCapturedProtectionErrorStatus; //fps_bank1_reg_fabric_captured_protection_error_status
    FpsBank1PsramEccErrorCount_t fpsBank1PsramEccErrorCount; //fps_bank1_reg_psram_ecc_error_count
    uint8_t rsvdB8[8];                    //rsvd_b8
    FpsBank1MainFabricAxiErrorStatusControl_t fpsBank1MainFabricAxiErrorStatusControl; //fps_bank1_reg_main_fabric_axi_error_status_control
    uint8_t rsvdC4[44];                   //rsvd_c4
    FpsBank1IndirectRegisterWriteStatus_t fpsBank1IndirectRegisterWriteStatus; //fps_bank1_reg_indirect_register_write_status
} FpsBank1RegRegisters_t;

/// @brief 0x2000
typedef struct
{
    FpsCpuCpuCfg0_t fpsCpuCpuCfg0;        //fps_cpu_reg_cpu_configuration_0
    FpsCpuCpuCfg1_t fpsCpuCpuCfg1;        //fps_cpu_reg_cpu_configuration_1
    FpsCpuCpuControl_t fpsCpuCpuControl;  //fps_cpu_reg_cpu_control
    uint32_t fpsCpuCpuIoStatusM7IoStatus; //fps_cpu_reg_cpu_io_status
    uint8_t rsvd10[64];                   //rsvd_10
    FpsCpuCpuIntrCause_t fpsCpuCpuIntrCause; //fps_cpu_reg_cpu_interrupt_cause
    FpsCpuCpuIntrEnable_t fpsCpuCpuIntrEnable; //fps_cpu_reg_cpu_interrupt_enable
    FpsCpuCpuSwIntrStatus_t fpsCpuCpuSwIntrStatus; //fps_cpu_reg_cpu_sw_interrupt_status
    uint8_t rsvd5c[4];                    //rsvd_5c
    FpsCpuCpuMemoryControl_t fpsCpuCpuMemoryControl; //fps_cpu_reg_cpu_memory_control
    FpsCpuCpuMemoryErrorStatus_t fpsCpuCpuMemoryErrorStatus; //fps_cpu_reg_cpu_memory_error_status
    uint8_t rsvd68[4];                    //rsvd_68
    FpsCpuCpuCapturedErrorStatusControl_t fpsCpuCpuCapturedErrorStatusControl; //fps_cpu_reg_cpu_captured_error_status_control
    FpsCpuCpuCapturedAddressErrorStatus_t fpsCpuCpuCapturedAddressErrorStatus; //fps_cpu_reg_cpu_captured_address_error_status
    uint32_t fpsCpuCpuCapturedDataErrorStatus0TcmDataErrorBitsLwr32; //fps_cpu_reg_cpu_captured_data_error_status_0
    uint32_t fpsCpuCpuCapturedDataErrorStatus1TcmDataErrorBitsUpr32; //fps_cpu_reg_cpu_captured_data_error_status_1
    FpsCpuCpuCapturedProtectionErrorStatus_t fpsCpuCpuCapturedProtectionErrorStatus; //fps_cpu_reg_cpu_captured_protection_error_status
    FpsCpuCpuTcmEccErrorCount_t fpsCpuCpuTcmEccErrorCount; //fps_cpu_reg_cpu_tcm_ecc_error_count
    uint8_t endPadding[124];              //end_padding
} FpsCpuRegRegisters_t;

/// @brief 0xD00
typedef struct
{
    uint32_t fpsSocFwdSocIndirectAddressPortSocIndirectRegAddr; //fps_soc_fwd_reg_soc_indirect_reg_address_port
    uint32_t fpsSocFwdSocIndirectDataPortSocIndirectRegData; //fps_soc_fwd_reg_soc_indirect_reg_data_port
} FpsSocFwdRegRegisters_t;

/// @brief 0xB00
typedef struct
{
    FpsCpuxToCpuyQueueProducerIndex_t fpsCpuxToCpuyQueueProducerIndex; //fps_cpux_to_cpuy_reg_queue_producer_index
    FpsCpuxToCpuyQueueConsumerIndex_t fpsCpuxToCpuyQueueConsumerIndex; //fps_cpux_to_cpuy_reg_queue_consumer_index
    FpsCpuxToCpuyQueueSize_t fpsCpuxToCpuyQueueSize; //fps_cpux_to_cpuy_reg_queue_size
    FpsCpuxToCpuyQueueStatus_t fpsCpuxToCpuyQueueStatus; //fps_cpux_to_cpuy_reg_queue_status
} FpsCpuxToCpuyRegRegisters_t;

/// @brief 0x800
typedef struct
{
    FpsFp2hweFpToHweQCiShadow_t fpsFp2hweFpToHweQCiShadow; //fps_fp2hwe_reg_fp_to_hwe_q_ci_shadow
    uint32_t fpsFp2hweFpToHweQPiIndirectAddressPortFp2hweQPiIndirectRegAddr; //fps_fp2hwe_reg_fp_to_hwe_q_pi_indirect_address_port
    FpsFp2hweFpToHweQPiIndirectDataPort_t fpsFp2hweFpToHweQPiIndirectDataPort; //fps_fp2hwe_reg_fp_to_hwe_q_pi_indirect_data_port
    uint8_t rsvdC[4];                     //rsvd_c
    FpsFp2hweFpToHweQSize_t fpsFp2hweFpToHweQSize; //fps_fp2hwe_reg_fp_to_hwe_q_size
    FpsFp2hweQueueStatus_t fpsFp2hweQueueStatus; //fps_fp2hwe_reg_queue_status
    uint8_t endPadding[8];                //end_padding
} FpsFp2hweRegRegisters_t;

/// @brief 0x500
typedef struct
{
    FpsHwe2fpHwEngineToFpQPiShadow_t fpsHwe2fpHwEngineToFpQPiShadow; //fps_hwe2fp_reg_hw_engine_to_fp_q_pi_shadow
    uint32_t fpsHwe2fpHwEngineToFpQCiIndirectAddressPortHwe2fpQCiIndirectRegAddr; //fps_hwe2fp_reg_hw_engine_to_fp_q_ci_indirect_address_port
    FpsHwe2fpHwEngineToFpQCiIndirectDataPort_t fpsHwe2fpHwEngineToFpQCiIndirectDataPort; //fps_hwe2fp_reg_hw_engine_to_fp_q_ci_indirect_data_port
    uint8_t rsvdC[4];                     //rsvd_c
    FpsHwe2fpHwEngineToFpQSize_t fpsHwe2fpHwEngineToFpQSize; //fps_hwe2fp_reg_hw_engine_to_fp_q_size
    FpsHwe2fpQueueStatus_t fpsHwe2fpQueueStatus; //fps_hwe2fp_reg_queue_status
    uint8_t endPadding[8];                //end_padding
} FpsHwe2fpRegRegister_t;

/// @brief 0x0
typedef struct
{
    FpsBank0EventStatus0_t fpsBank0EventStatus0; //fps_bank0_reg_event_status_0
    FpsBank0EventStatus1_t fpsBank0EventStatus1; //fps_bank0_reg_event_status_1
    FpsBank0CpuxToCpuyWqStatus_t fpsBank0CpuxToCpuyWqStatus; //fps_bank0_reg_cpux_to_cpuy_wq_status
    uint8_t rsvdC[4];                     //rsvd_c
    FpsBank0SlotArrayGroupEmptyStatus_t fpsBank0SlotArrayGroupEmptyStatus; //fps_bank0_reg_slot_array_group_empty_status
    FpsBank0SlotArrayGroupFullStatus_t fpsBank0SlotArrayGroupFullStatus; //fps_bank0_reg_slot_array_group_full_status
    uint8_t rsvd18[8];                    //rsvd_18
    uint32_t fpsBank0SlotArrayQueueEmptyStatus0SlotArrayQEmpty310; //fps_bank0_reg_slot_array_queue_empty_status_0
    uint32_t fpsBank0SlotArrayQueueEmptyStatus1SlotArrayQEmpty6332; //fps_bank0_reg_slot_array_queue_empty_status_1
    uint32_t fpsBank0SlotArrayQueueEmptyStatus2SlotArrayQEmpty9564; //fps_bank0_reg_slot_array_queue_empty_status_2
    uint32_t fpsBank0SlotArrayQueueEmptyStatus3SlotArrayQEmpty12796; //fps_bank0_reg_slot_array_queue_empty_status_3
    FpsBank0SlotArrayQueueEmptyStatus4_t fpsBank0SlotArrayQueueEmptyStatus4; //fps_bank0_reg_slot_array_queue_empty_status_4
    uint8_t rsvd34[12];                   //rsvd_34
    uint32_t fpsBank0SlotArrayQueueFullStatus0SlotArrayQFull310; //fps_bank0_reg_slot_array_queue_full_status_0
    uint32_t fpsBank0SlotArrayQueueFullStatus1SlotArrayQFull6332; //fps_bank0_reg_slot_array_queue_full_status_1
    uint32_t fpsBank0SlotArrayQueueFullStatus2SlotArrayQFull9564; //fps_bank0_reg_slot_array_queue_full_status_2
    uint32_t fpsBank0SlotArrayQueueFullStatus3SlotArrayQFull12796; //fps_bank0_reg_slot_array_queue_full_status_3
    FpsBank0SlotArrayQueueFullStatus4_t fpsBank0SlotArrayQueueFullStatus4; //fps_bank0_reg_slot_array_queue_full_status_4
    uint8_t rsvd54[28];                   //rsvd_54
    FpsBank0FpsControl_t fpsBank0FpsControl; //fps_bank0_reg_fps_control
    FpsBank0FpsCfg_t fpsBank0FpsCfg;      //fps_bank0_reg_fps_configuration
    uint8_t rsvd78[40];                   //rsvd_78
    uint32_t fpsBank0SystemIngressIntrStatusSysIngressIrq; //fps_bank0_reg_system_ingress_interrupt_status
    FpsBank0FpsSwIntrStatus_t fpsBank0FpsSwIntrStatus; //fps_bank0_reg_fps_sw_interrupt_status
    uint8_t rsvdA8[4];                    //rsvd_a8
    FpsBank0FpsEgressIntrStatus_t fpsBank0FpsEgressIntrStatus; //fps_bank0_reg_fps_egress_interrupt_status
    FpsBank0FpsEgressIntrEnable_t fpsBank0FpsEgressIntrEnable; //fps_bank0_reg_fps_egress_interrupt_enable
    uint8_t rsvdB4[60];                   //rsvd_b4
    FpsBank0IndirectRegisterWriteStatus_t fpsBank0IndirectRegisterWriteStatus; //fps_bank0_reg_indirect_register_write_status
    uint32_t fpsBank0IndirectRegisterWriteDisableIndirectRegWriteFwdDisable; //fps_bank0_reg_indirect_register_write_disable
    uint32_t fpsBank0FpsTraceportFpsTraceport; //fps_bank0_reg_fps_traceport
} FpsBank0RegRegisters_t;

typedef struct
{
    FpsBank0RegRegisters_t fpsBank0RegRegisters;                            // 0x0 : fps_bank0_reg_registers / 
    uint8_t rsvdFc[1028];                                                   // 0xFC : rsvd_fc / rsvd_fc
    FpsHwe2fpRegRegister_t fpsHwe2fpRegRegister[12];                        // 0x500 : fps_hwe2fp_reg_register / 
    uint8_t rsvd680[384];                                                   // 0x680 : rsvd_680 / rsvd_680
    FpsFp2hweRegRegisters_t fpsFp2hweRegRegisters[12];                      // 0x800 : fps_fp2hwe_reg_registers / 
    uint8_t rsvd980[384];                                                   // 0x980 : rsvd_980 / rsvd_980
    FpsCpuxToCpuyRegRegisters_t fpsCpuxToCpuyRegRegisters[8];               // 0xB00 : fps_cpux_to_cpuy_reg_registers / 
    uint8_t rsvdB80[384];                                                   // 0xB80 : rsvd_b80 / rsvd_b80
    FpsSocFwdRegRegisters_t fpsSocFwdRegRegisters[8];                       // 0xD00 : fps_soc_fwd_reg_registers / 
    uint8_t rsvdD40[4800];                                                  // 0xD40 : rsvd_d40 / rsvd_d40
    FpsCpuRegRegisters_t fpsCpuRegRegisters[3];                             // 0x2000 : fps_cpu_reg_registers / 
    uint8_t rsvd2300[7424];                                                 // 0x2300 : rsvd_2300 / rsvd_2300
    FpsBank1RegRegisters_t fpsBank1RegRegisters;                            // 0x4000 : fps_bank1_reg_registers / 
    uint8_t rsvd40f4[3852];                                                 // 0x40F4 : rsvd_40f4 / rsvd_40f4
    FpsSlotArrayRegRegisters_t fpsSlotArrayRegRegisters[132];               // 0x5000 : fps_slot_array_reg_registers / 
} Fps_t;

COMPILE_ASSERT(offsetof(Fps_t,fpsBank0RegRegisters)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(Fps_t,fpsHwe2fpRegRegister)==0x500,"check register structure offset");
COMPILE_ASSERT(offsetof(Fps_t,fpsFp2hweRegRegisters)==0x800,"check register structure offset");
COMPILE_ASSERT(offsetof(Fps_t,fpsCpuxToCpuyRegRegisters)==0xB00,"check register structure offset");
COMPILE_ASSERT(offsetof(Fps_t,fpsSocFwdRegRegisters)==0xD00,"check register structure offset");
COMPILE_ASSERT(offsetof(Fps_t,fpsCpuRegRegisters)==0x2000,"check register structure offset");
COMPILE_ASSERT(offsetof(Fps_t,fpsBank1RegRegisters)==0x4000,"check register structure offset");
COMPILE_ASSERT(offsetof(Fps_t,fpsSlotArrayRegRegisters)==0x5000,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile Fps_t rFps; ///< 0xA1E00000
