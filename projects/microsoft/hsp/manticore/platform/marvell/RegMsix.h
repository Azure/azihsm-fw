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
//! @brief MSIX Registers
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


/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TBL_IFC_SLCT                :8;      ///<BIT [7:0] tbl_ifc_slct
        uint32_t RSVD                        :24;     ///<BIT [31:8] rsvd_0
    } b;
} MsixtableInterfaceSelect_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RTC                         :2;      ///<BIT [1:0] rtc
        uint32_t WTC                         :2;      ///<BIT [3:2] wtc
        uint32_t PAR_GEN_DIS                 :1;      ///<BIT [4] par_gen_dis
        uint32_t PAR_CHK_DIS                 :1;      ///<BIT [5] par_chk_dis
        uint32_t PAR_ERR_INJ                 :1;      ///<BIT [6] par_err_inj
        uint32_t RSVD_3                      :1;      ///<BIT [7] rsvd_3
        uint32_t MASK_IRQ_TOGGLE_MODE        :1;      ///<BIT [8] mask_irq_toggle_mode
        uint32_t DSPL_PSA_READ               :1;      ///<BIT [9] dspl_psa_read
        uint32_t RSVD_2                      :6;      ///<BIT [15:10] rsvd_2
        uint32_t AWCACHE                     :4;      ///<BIT [19:16] awcache
        uint32_t AWLOCK                      :2;      ///<BIT [21:20] awlock
        uint32_t RSVD_1                      :2;      ///<BIT [23:22] rsvd_1
        uint32_t AWPROT                      :3;      ///<BIT [26:24] awprot
        uint32_t REMAP_COAL_MODE             :3;      ///<BIT [29:27] remap_coal_mode
        uint32_t SRAM_SD                     :1;      ///<BIT [30] sram_sd
        uint32_t SRAM_SLEEP                  :1;      ///<BIT [31] sram_sleep
    } b;
} MiscellaneousControl_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSI_X_EN_RISING_EDGE_0      :1;      ///<BIT [0] msi_x_en_rising_edge_0
        uint32_t MSI_X_EN_RISING_EDGE_1      :1;      ///<BIT [1] msi_x_en_rising_edge_1
        uint32_t MSI_X_EN_RISING_EDGE_2      :1;      ///<BIT [2] msi_x_en_rising_edge_2
        uint32_t MSI_X_EN_RISING_EDGE_3      :1;      ///<BIT [3] msi_x_en_rising_edge_3
        uint32_t MSI_X_EN_RISING_EDGE_4      :1;      ///<BIT [4] msi_x_en_rising_edge_4
        uint32_t MSI_X_EN_RISING_EDGE_5      :1;      ///<BIT [5] msi_x_en_rising_edge_5
        uint32_t MSI_X_EN_RISING_EDGE_6      :1;      ///<BIT [6] msi_x_en_rising_edge_6
        uint32_t MSI_X_EN_RISING_EDGE_7      :1;      ///<BIT [7] msi_x_en_rising_edge_7
        uint32_t MSI_X_EN_RISING_EDGE_8      :1;      ///<BIT [8] msi_x_en_rising_edge_8
        uint32_t MSI_X_EN_RISING_EDGE_9      :1;      ///<BIT [9] msi_x_en_rising_edge_9
        uint32_t MSI_X_EN_RISING_EDGE_10     :1;      ///<BIT [10] msi_x_en_rising_edge_10
        uint32_t MSI_X_EN_RISING_EDGE_11     :1;      ///<BIT [11] msi_x_en_rising_edge_11
        uint32_t MSI_X_EN_RISING_EDGE_12     :1;      ///<BIT [12] msi_x_en_rising_edge_12
        uint32_t MSI_X_EN_RISING_EDGE_13     :1;      ///<BIT [13] msi_x_en_rising_edge_13
        uint32_t MSI_X_EN_RISING_EDGE_14     :1;      ///<BIT [14] msi_x_en_rising_edge_14
        uint32_t MSI_X_EN_RISING_EDGE_15     :1;      ///<BIT [15] msi_x_en_rising_edge_15
        uint32_t MSI_X_EN_RISING_EDGE_16     :1;      ///<BIT [16] msi_x_en_rising_edge_16
        uint32_t MSI_EN_RISING_EDGE_0        :1;      ///<BIT [17] msi_en_rising_edge_0
        uint32_t MSI_EN_RISING_EDGE_1        :1;      ///<BIT [18] msi_en_rising_edge_1
        uint32_t MSI_EN_RISING_EDGE_2        :1;      ///<BIT [19] msi_en_rising_edge_2
        uint32_t MSI_EN_RISING_EDGE_3        :1;      ///<BIT [20] msi_en_rising_edge_3
        uint32_t MSI_EN_RISING_EDGE_4        :1;      ///<BIT [21] msi_en_rising_edge_4
        uint32_t MSI_EN_RISING_EDGE_5        :1;      ///<BIT [22] msi_en_rising_edge_5
        uint32_t MSI_EN_RISING_EDGE_6        :1;      ///<BIT [23] msi_en_rising_edge_6
        uint32_t MSI_EN_RISING_EDGE_7        :1;      ///<BIT [24] msi_en_rising_edge_7
        uint32_t MSI_EN_RISING_EDGE_8        :1;      ///<BIT [25] msi_en_rising_edge_8
        uint32_t RSVD                        :1;      ///<BIT [26] rsvd_0
        uint32_t MEM_PERR                    :1;      ///<BIT [27] mem_perr
        uint32_t MSIX_AXI_ERR                :1;      ///<BIT [28] msix_axi_err
        uint32_t AXI_DP_ERR_INJCTD           :1;      ///<BIT [29] axi_dp_err_injctd
        uint32_t AHB_SRAM_ERR_OCCRD          :1;      ///<BIT [30] ahb_sram_err_occrd
        uint32_t SRAM_PERR_INJCTD            :1;      ///<BIT [31] sram_perr_injctd
    } b;
} MsiMsiXIntrCause0_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSI_MSIX_BOTH_ENBLD_0       :1;      ///<BIT [0] msi_msix_both_enbld_0
        uint32_t MSI_MSIX_BOTH_ENBLD_1       :1;      ///<BIT [1] msi_msix_both_enbld_1
        uint32_t MSI_MSIX_BOTH_ENBLD_2       :1;      ///<BIT [2] msi_msix_both_enbld_2
        uint32_t MSI_MSIX_BOTH_ENBLD_3       :1;      ///<BIT [3] msi_msix_both_enbld_3
        uint32_t MSI_MSIX_BOTH_ENBLD_4       :1;      ///<BIT [4] msi_msix_both_enbld_4
        uint32_t MSI_MSIX_BOTH_ENBLD_5       :1;      ///<BIT [5] msi_msix_both_enbld_5
        uint32_t MSI_MSIX_BOTH_ENBLD_6       :1;      ///<BIT [6] msi_msix_both_enbld_6
        uint32_t MSI_MSIX_BOTH_ENBLD_7       :1;      ///<BIT [7] msi_msix_both_enbld_7
        uint32_t MSI_MSIX_BOTH_ENBLD_8       :1;      ///<BIT [8] msi_msix_both_enbld_8
        uint32_t MSI_MSIX_BOTH_ENBLD_9       :1;      ///<BIT [9] msi_msix_both_enbld_9
        uint32_t MSI_X_EN_FALLING_EDGE_0     :1;      ///<BIT [10] msi_x_en_falling_edge_0
        uint32_t MSI_X_EN_FALLING_EDGE_1     :1;      ///<BIT [11] msi_x_en_falling_edge_1
        uint32_t MSI_X_EN_FALLING_EDGE_2     :1;      ///<BIT [12] msi_x_en_falling_edge_2
        uint32_t MSI_X_EN_FALLING_EDGE_3     :1;      ///<BIT [13] msi_x_en_falling_edge_3
        uint32_t MSI_X_EN_FALLING_EDGE_4     :1;      ///<BIT [14] msi_x_en_falling_edge_4
        uint32_t MSI_X_EN_FALLING_EDGE_5     :1;      ///<BIT [15] msi_x_en_falling_edge_5
        uint32_t MSI_X_EN_FALLING_EDGE_6     :1;      ///<BIT [16] msi_x_en_falling_edge_6
        uint32_t MSI_X_EN_FALLING_EDGE_7     :1;      ///<BIT [17] msi_x_en_falling_edge_7
        uint32_t MSI_X_EN_FALLING_EDGE_8     :1;      ///<BIT [18] msi_x_en_falling_edge_8
        uint32_t MSI_X_EN_FALLING_EDGE_9     :1;      ///<BIT [19] msi_x_en_falling_edge_9
        uint32_t MSI_X_EN_FALLING_EDGE_10    :1;      ///<BIT [20] msi_x_en_falling_edge_10
        uint32_t MSI_X_EN_FALLING_EDGE_11    :1;      ///<BIT [21] msi_x_en_falling_edge_11
        uint32_t MSI_X_EN_FALLING_EDGE_12    :1;      ///<BIT [22] msi_x_en_falling_edge_12
        uint32_t MSI_X_EN_FALLING_EDGE_13    :1;      ///<BIT [23] msi_x_en_falling_edge_13
        uint32_t MSI_X_EN_FALLING_EDGE_14    :1;      ///<BIT [24] msi_x_en_falling_edge_14
        uint32_t MSI_X_EN_FALLING_EDGE_15    :1;      ///<BIT [25] msi_x_en_falling_edge_15
        uint32_t MSI_X_EN_FALLING_EDGE_16    :1;      ///<BIT [26] msi_x_en_falling_edge_16
        uint32_t DMA_PAUSED_0                :1;      ///<BIT [27] dma_paused_0
        uint32_t DMA_PAUSED_1                :1;      ///<BIT [28] dma_paused_1
        uint32_t DMA_PAUSED_2                :1;      ///<BIT [29] dma_paused_2
        uint32_t DMA_PAUSED_3                :1;      ///<BIT [30] dma_paused_3
        uint32_t DMA_PAUSED_4                :1;      ///<BIT [31] dma_paused_4
    } b;
} MsiMsiXIntrCause1_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DMA_PAUSED_5                :1;      ///<BIT [0] dma_paused_5
        uint32_t DMA_PAUSED_6                :1;      ///<BIT [1] dma_paused_6
        uint32_t DMA_PAUSED_7                :1;      ///<BIT [2] dma_paused_7
        uint32_t DMA_PAUSED_8                :1;      ///<BIT [3] dma_paused_8
        uint32_t DMA_PAUSED_9                :1;      ///<BIT [4] dma_paused_9
        uint32_t DMA_PAUSED_10               :1;      ///<BIT [5] dma_paused_10
        uint32_t DMA_PAUSED_11               :1;      ///<BIT [6] dma_paused_11
        uint32_t DMA_PAUSED_12               :1;      ///<BIT [7] dma_paused_12
        uint32_t DMA_PAUSED_13               :1;      ///<BIT [8] dma_paused_13
        uint32_t DMA_PAUSED_14               :1;      ///<BIT [9] dma_paused_14
        uint32_t DMA_PAUSED_15               :1;      ///<BIT [10] dma_paused_15
        uint32_t DMA_PAUSED_16               :1;      ///<BIT [11] dma_paused_16
        uint32_t MSIX_TBL_UPDT_OCCRD_0       :1;      ///<BIT [12] msix_tbl_updt_occrd_0
        uint32_t MSIX_TBL_UPDT_OCCRD_1       :1;      ///<BIT [13] msix_tbl_updt_occrd_1
        uint32_t MSIX_TBL_UPDT_OCCRD_2       :1;      ///<BIT [14] msix_tbl_updt_occrd_2
        uint32_t MSIX_TBL_UPDT_OCCRD_3       :1;      ///<BIT [15] msix_tbl_updt_occrd_3
        uint32_t MSIX_TBL_UPDT_OCCRD_4       :1;      ///<BIT [16] msix_tbl_updt_occrd_4
        uint32_t MSIX_TBL_UPDT_OCCRD_5       :1;      ///<BIT [17] msix_tbl_updt_occrd_5
        uint32_t MSIX_TBL_UPDT_OCCRD_6       :1;      ///<BIT [18] msix_tbl_updt_occrd_6
        uint32_t MSIX_TBL_UPDT_OCCRD_7       :1;      ///<BIT [19] msix_tbl_updt_occrd_7
        uint32_t MSIX_TBL_UPDT_OCCRD_8       :1;      ///<BIT [20] msix_tbl_updt_occrd_8
        uint32_t MSIX_TBL_UPDT_OCCRD_9       :1;      ///<BIT [21] msix_tbl_updt_occrd_9
        uint32_t MSIX_TBL_UPDT_OCCRD_10      :1;      ///<BIT [22] msix_tbl_updt_occrd_10
        uint32_t MSIX_TBL_UPDT_OCCRD_11      :1;      ///<BIT [23] msix_tbl_updt_occrd_11
        uint32_t MSIX_TBL_UPDT_OCCRD_12      :1;      ///<BIT [24] msix_tbl_updt_occrd_12
        uint32_t MSIX_TBL_UPDT_OCCRD_13      :1;      ///<BIT [25] msix_tbl_updt_occrd_13
        uint32_t MSIX_TBL_UPDT_OCCRD_14      :1;      ///<BIT [26] msix_tbl_updt_occrd_14
        uint32_t MSIX_TBL_UPDT_OCCRD_15      :1;      ///<BIT [27] msix_tbl_updt_occrd_15
        uint32_t MSIX_TBL_UPDT_OCCRD_16      :1;      ///<BIT [28] msix_tbl_updt_occrd_16
        uint32_t RSVD                        :3;      ///<BIT [31:29] rsvd_0
    } b;
} MsiMsiXIntrCause2_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSI_MSIX_BOTH_ENBLD_10      :1;      ///<BIT [0] msi_msix_both_enbld_10
        uint32_t MSI_MSIX_BOTH_ENBLD_11      :1;      ///<BIT [1] msi_msix_both_enbld_11
        uint32_t MSI_MSIX_BOTH_ENBLD_12      :1;      ///<BIT [2] msi_msix_both_enbld_12
        uint32_t MSI_MSIX_BOTH_ENBLD_13      :1;      ///<BIT [3] msi_msix_both_enbld_13
        uint32_t MSI_MSIX_BOTH_ENBLD_14      :1;      ///<BIT [4] msi_msix_both_enbld_14
        uint32_t MSI_MSIX_BOTH_ENBLD_15      :1;      ///<BIT [5] msi_msix_both_enbld_15
        uint32_t MSI_MSIX_BOTH_ENBLD_16      :1;      ///<BIT [6] msi_msix_both_enbld_16
        uint32_t MSI_EN_RISING_EDGE_9        :1;      ///<BIT [7] msi_en_rising_edge_9
        uint32_t MSI_EN_RISING_EDGE_10       :1;      ///<BIT [8] msi_en_rising_edge_10
        uint32_t MSI_EN_RISING_EDGE_11       :1;      ///<BIT [9] msi_en_rising_edge_11
        uint32_t MSI_EN_RISING_EDGE_12       :1;      ///<BIT [10] msi_en_rising_edge_12
        uint32_t MSI_EN_RISING_EDGE_13       :1;      ///<BIT [11] msi_en_rising_edge_13
        uint32_t MSI_EN_RISING_EDGE_14       :1;      ///<BIT [12] msi_en_rising_edge_14
        uint32_t MSI_EN_RISING_EDGE_15       :1;      ///<BIT [13] msi_en_rising_edge_15
        uint32_t MSI_EN_RISING_EDGE_16       :1;      ///<BIT [14] msi_en_rising_edge_16
        uint32_t MSI_EN_FALLING_EDGE_0       :1;      ///<BIT [15] msi_en_falling_edge_0
        uint32_t MSI_EN_FALLING_EDGE_1       :1;      ///<BIT [16] msi_en_falling_edge_1
        uint32_t MSI_EN_FALLING_EDGE_2       :1;      ///<BIT [17] msi_en_falling_edge_2
        uint32_t MSI_EN_FALLING_EDGE_3       :1;      ///<BIT [18] msi_en_falling_edge_3
        uint32_t MSI_EN_FALLING_EDGE_4       :1;      ///<BIT [19] msi_en_falling_edge_4
        uint32_t MSI_EN_FALLING_EDGE_5       :1;      ///<BIT [20] msi_en_falling_edge_5
        uint32_t MSI_EN_FALLING_EDGE_6       :1;      ///<BIT [21] msi_en_falling_edge_6
        uint32_t MSI_EN_FALLING_EDGE_7       :1;      ///<BIT [22] msi_en_falling_edge_7
        uint32_t MSI_EN_FALLING_EDGE_8       :1;      ///<BIT [23] msi_en_falling_edge_8
        uint32_t MSI_EN_FALLING_EDGE_9       :1;      ///<BIT [24] msi_en_falling_edge_9
        uint32_t MSI_EN_FALLING_EDGE_10      :1;      ///<BIT [25] msi_en_falling_edge_10
        uint32_t MSI_EN_FALLING_EDGE_11      :1;      ///<BIT [26] msi_en_falling_edge_11
        uint32_t MSI_EN_FALLING_EDGE_12      :1;      ///<BIT [27] msi_en_falling_edge_12
        uint32_t MSI_EN_FALLING_EDGE_13      :1;      ///<BIT [28] msi_en_falling_edge_13
        uint32_t MSI_EN_FALLING_EDGE_14      :1;      ///<BIT [29] msi_en_falling_edge_14
        uint32_t MSI_EN_FALLING_EDGE_15      :1;      ///<BIT [30] msi_en_falling_edge_15
        uint32_t MSI_EN_FALLING_EDGE_16      :1;      ///<BIT [31] msi_en_falling_edge_16
    } b;
} MsiMsiXIntrCause3_t;

/// @brief 0x40
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DP_PAR                      :1;      ///<BIT [0] dp_par
        uint32_t RSVD_2                      :7;      ///<BIT [7:1] rsvd_2
        uint32_t MEM_PERR_EN                 :1;      ///<BIT [8] mem_perr_en
        uint32_t AXI_ERR_EN                  :1;      ///<BIT [9] axi_err_en
        uint32_t RSVD_1                      :6;      ///<BIT [15:10] rsvd_1
        uint32_t FRC_DP_PERR_CONT            :1;      ///<BIT [16] frc_dp_perr_cont
        uint32_t FRC_DP_PERR_ONCE            :1;      ///<BIT [17] frc_dp_perr_once
        uint32_t DP_PRTY_MASK                :2;      ///<BIT [19:18] dp_prty_mask
        uint32_t RSVD                        :3;      ///<BIT [22:20] rsvd_0
        uint32_t FRC_SRAM_PERR_ONCE          :1;      ///<BIT [23] frc_sram_perr_once
        uint32_t SRAM_PRTY_MASK              :8;      ///<BIT [31:24] sram_prty_mask
    } b;
} MsiMsiXErrorControl_t;

/// @brief 0x44
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MEM_PERR_OCC                :1;      ///<BIT [0] mem_perr_occ
        uint32_t AXI_ERR                     :1;      ///<BIT [1] axi_err
        uint32_t DP_ERR_INJCTD               :1;      ///<BIT [2] dp_err_injctd
        uint32_t SRAM_ERR_INJCTD             :1;      ///<BIT [3] sram_err_injctd
        uint32_t RSVD                        :28;     ///<BIT [31:4] rsvd_0
    } b;
} MsiMsiXErrorStatus_t;

/// @brief 0x48
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AXI_BRESP                   :2;      ///<BIT [1:0] axi_bresp
        uint32_t RSVD_2                      :6;      ///<BIT [7:2] rsvd_2
        uint32_t AXI_BID                     :5;      ///<BIT [12:8] axi_bid
        uint32_t RSVD_1                      :3;      ///<BIT [15:13] rsvd_1
        uint32_t AXI_CMD_ID                  :5;      ///<BIT [20:16] axi_cmd_id
        uint32_t RSVD                        :3;      ///<BIT [23:21] rsvd_0
        uint32_t AXI_PORT_ID                 :8;      ///<BIT [31:24] axi_port_id
    } b;
} MsiMsiXAxiErrorInfo_t;

/// @brief 0x54
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MEM_PERR_ADDR               :16;     ///<BIT [15:0] mem_perr_addr
        uint32_t REQUESTOR_ID                :8;      ///<BIT [23:16] requestor_id
        uint32_t RSVD                        :8;      ///<BIT [31:24] rsvd_0
    } b;
} MsiMsiXInternalMemPerrAddr_t;

/// @brief 0x68
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EXPCTD_PRTY                 :8;      ///<BIT [7:0] expctd_prty
        uint32_t RSVD_1                      :8;      ///<BIT [15:8] rsvd_1
        uint32_t ACTUAL_PRTY                 :8;      ///<BIT [23:16] actual_prty
        uint32_t RSVD                        :8;      ///<BIT [31:24] rsvd_0
    } b;
} MsiMsiXInternalMemPerrParity_t;

/// @brief 0x6C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PERR_COUNT                  :8;      ///<BIT [7:0] perr_count
        uint32_t RSVD                        :24;     ///<BIT [31:8] rsvd_0
    } b;
} MsiMsiXNternalMemPerrCount_t;

/// @brief 0xB0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PF_MSG_EN                   :1;      ///<BIT [0] pf_msg_en
        uint32_t RSVD                        :29;     ///<BIT [29:1] rsvd_0
        uint32_t MSIX_SM_EN                  :1;      ///<BIT [30] msix_sm_en
        uint32_t MSIX_RST                    :1;      ///<BIT [31] msix_rst
    } b;
} StateMachineControl16_t;

/// @brief 0xF4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSI_X_PF_MSG_GEN_ENBLD      :1;      ///<BIT [0] msi_x_pf_msg_gen_enbld
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} MsixStateMachineStatus16_t;

/// @brief 0x13C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSI_PF_MSG_GEN_ENBLD        :1;      ///<BIT [0] msi_pf_msg_gen_enbld
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} MsiStateMachineStatus16_t;

/// @brief 0x180
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DMA_PAUSED_PF               :1;      ///<BIT [0] dma_paused_pf
        uint32_t RSVD_1                      :2;      ///<BIT [2:1] rsvd_1
        uint32_t AHB_SRAM_ERR                :2;      ///<BIT [4:3] ahb_sram_err
        uint32_t RSVD                        :27;     ///<BIT [31:5] rsvd_0
    } b;
} MsiMsiXDmaPaused16_t;

/// @brief 0x1C4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DMA_PAUSED_IRQ_EN_PF        :1;      ///<BIT [0] dma_paused_irq_en_pf
        uint32_t RSVD_1                      :7;      ///<BIT [7:1] rsvd_1
        uint32_t AHB_SRAM_ERR_IRQ_EN         :2;      ///<BIT [9:8] ahb_sram_err_irq_en
        uint32_t RSVD                        :22;     ///<BIT [31:10] rsvd_0
    } b;
} MsiMsiXDmaPausedIntrEnable16_t;

/// @brief 0x208
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSI_X_EN_FALLING_EDGE_PF    :1;      ///<BIT [0] msi_x_en_falling_edge_pf
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} MsiXEnableFallingEdgePf_t;

/// @brief 0x24C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSI_EN_FALLING_EDGE_PF      :1;      ///<BIT [0] msi_en_falling_edge_pf
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} MsiEnableFallingEdgePf_t;

/// @brief 0x290
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSI_X_EN_RISING_EDGE_PF     :1;      ///<BIT [0] msi_x_en_rising_edge_pf
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} MsiXEnableRisingEdgePf_t;

/// @brief 0x2D4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSI_EN_RISING_EDGE_PF       :1;      ///<BIT [0] msi_en_rising_edge_pf
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} MsiEnableRisingEdgePf_t;

/// @brief 0x318
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSIX_TBL_UPDTD_PF           :1;      ///<BIT [0] msix_tbl_updtd_pf
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} MsiXTableUpdatedPf_t;

/// @brief 0x35C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSI_MSIX_BOTH_ENBLD_PF      :1;      ///<BIT [0] msi_msix_both_enbld_pf
        uint32_t RSVD                        :31;     ///<BIT [31:1] rsvd_0
    } b;
} BothModesEnabledErrPf_t;

/// @brief 0x360
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ICU_TO_TBL_PF0_ENTRY_SLCT   :5;      ///<BIT [4:0] icu_to_tbl_pf0_entry_slct
        uint32_t RSVD                        :27;     ///<BIT [31:5] rsvd_0
    } b;
} MsiXIcuIntrRoutingControl_t;

/// @brief 0x364
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PCIE_PORT_SLCT_FOR_TBLS_63_0 :3;      ///<BIT [2:0] pcie_port_slct_for_tbls_63_0
        uint32_t RSVD_1                      :1;      ///<BIT [3] rsvd_1
        uint32_t PCIE_PORT_SLCT_FOR_TBL_PF0  :4;      ///<BIT [7:4] pcie_port_slct_for_tbl_pf0
        uint32_t RSVD                        :24;     ///<BIT [31:8] rsvd_0
    } b;
} PcieCapabilitiesStructureSelect_t;

/// @brief 0x368
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSIX_MONITOR_PORT_SLCT      :4;      ///<BIT [3:0] msix_monitor_port_slct
        uint32_t RSVD                        :28;     ///<BIT [31:4] rsvd_0
    } b;
} MsiXMonitorPortSelect_t;

/// @brief 0x36C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AWUSER                      :24;     ///<BIT [23:0] awuser
        uint32_t RSVD                        :8;      ///<BIT [31:24] rsvd_0
    } b;
} AwuserCtrl_t;

/// @brief 0x18000
typedef struct
{
    MiscellaneousControl_t miscellaneousControl; //miscellaneous_control
    MsiMsiXIntrCause0_t msiMsiXIntrCause0; //msi_msi_x_interrupt_cause_0
    MsiMsiXIntrCause1_t msiMsiXIntrCause1; //msi_msi_x_interrupt_cause_1
    MsiMsiXIntrCause2_t msiMsiXIntrCause2; //msi_msi_x_interrupt_cause_2
    MsiMsiXIntrCause3_t msiMsiXIntrCause3; //msi_msi_x_interrupt_cause_3
    uint32_t msixIntEnable0;              //msix_int_enable_0
    uint32_t msixIntEnable1;              //msix_int_enable_1
    uint32_t msixIntEnable2;              //msix_int_enable_2
    uint32_t msixIntEnable3;              //msix_int_enable_3
    uint8_t rsvd24[28];                   //rsvd_24
    MsiMsiXErrorControl_t msiMsiXErrorControl; //msi_msi_x_error_control
    MsiMsiXErrorStatus_t msiMsiXErrorStatus; //msi_msi_x_error_status
    MsiMsiXAxiErrorInfo_t msiMsiXAxiErrorInfo; //msi_msi_x_axi_error_info
    uint32_t msiMsiXAxiErrorAddressLowErrAddrLow; //msi_msi_x_axi_error_address_low
    uint32_t msiMsiXAxiErrorAddressHighErrAddrHigh; //msi_msi_x_axi_error_address_high
    MsiMsiXInternalMemPerrAddr_t msiMsiXInternalMemPerrAddr; //msi_msi_x_internal_mem_perr_addr
    uint32_t msiMsiXNternalPerrRdata0PerrRdata0; //msi_msi_x_nternal_perr_rdata_0
    uint32_t msiMsiXNternalMemPerrRdata1PerrRdata1; //msi_msi_x_nternal_mem_perr_rdata_1
    uint32_t msiMsiXNternalMemPerrRdata2PerrRdata2; //msi_msi_x_nternal_mem_perr_rdata_2
    uint32_t msiMsiXNternalMemPerrRdata3PerrRdata3; //msi_msi_x_nternal_mem_perr_rdata_3
    MsiMsiXInternalMemPerrParity_t msiMsiXInternalMemPerrParity; //msi_msi_x_internal_mem_perr_parity
    MsiMsiXNternalMemPerrCount_t msiMsiXNternalMemPerrCount; //msi_msi_x_nternal_mem_perr_count
    uint32_t stateMachineControl0VfMsgEn0; //state_machine_control_0
    uint32_t stateMachineControl1VfMsgEn1; //state_machine_control_1
    uint32_t stateMachineControl2VfMsgEn2; //state_machine_control_2
    uint32_t stateMachineControl3VfMsgEn3; //state_machine_control_3
    uint32_t stateMachineControl4VfMsgEn4; //state_machine_control_4
    uint32_t stateMachineControl5VfMsgEn5; //state_machine_control_5
    uint32_t stateMachineControl6VfMsgEn6; //state_machine_control_6
    uint32_t stateMachineControl7VfMsgEn7; //state_machine_control_7
    uint32_t stateMachineControl8VfMsgEn8; //state_machine_control_8
    uint32_t stateMachineControl9VfMsgEn9; //state_machine_control_9
    uint32_t stateMachineControl10VfMsgEn10; //state_machine_control_10
    uint32_t stateMachineControl11VfMsgEn11; //state_machine_control_11
    uint32_t stateMachineControl12VfMsgEn12; //state_machine_control_12
    uint32_t stateMachineControl13VfMsgEn13; //state_machine_control_13
    uint32_t stateMachineControl14VfMsgEn14; //state_machine_control_14
    uint32_t stateMachineControl15VfMsgEn15; //state_machine_control_15
    StateMachineControl16_t stateMachineControl16; //state_machine_control_16
    uint32_t msixStateMachineStatus0MsiXVfMsgGenEnbld0; //msix_state_machine_status_0
    uint32_t msixStateMachineStatus1MsiXVfMsgGenEnbld1; //msix_state_machine_status_1
    uint32_t msixStateMachineStatus2MsiXVfMsgGenEnbld2; //msix_state_machine_status_2
    uint32_t msixStateMachineStatus3MsiXVfMsgGenEnbld3; //msix_state_machine_status_3
    uint32_t msixStateMachineStatus4MsiXVfMsgGenEnbld4; //msix_state_machine_status_4
    uint32_t msixStateMachineStatus5MsiXVfMsgGenEnbld5; //msix_state_machine_status_5
    uint32_t msixStateMachineStatus6MsiXVfMsgGenEnbld6; //msix_state_machine_status_6
    uint32_t msixStateMachineStatus7MsiXVfMsgGenEnbld7; //msix_state_machine_status_7
    uint32_t msixStateMachineStatus8MsiXVfMsgGenEnbld8; //msix_state_machine_status_8
    uint32_t msixStateMachineStatus9MsiXVfMsgGenEnbld9; //msix_state_machine_status_9
    uint32_t msixStateMachineStatus10MsiXVfMsgGenEnbld10; //msix_state_machine_status_10
    uint32_t msixStateMachineStatus11MsiXVfMsgGenEnbld11; //msix_state_machine_status_11
    uint32_t msixStateMachineStatus12MsiXVfMsgGenEnbld12; //msix_state_machine_status_12
    uint32_t msixStateMachineStatus13MsiXVfMsgGenEnbld13; //msix_state_machine_status_13
    uint32_t msixStateMachineStatus14MsiXVfMsgGenEnbld14; //msix_state_machine_status_14
    uint32_t msixStateMachineStatus15MsiXVfMsgGenEnbld15; //msix_state_machine_status_15
    MsixStateMachineStatus16_t msixStateMachineStatus16; //msix_state_machine_status_16
    uint8_t rsvdF8[4];                    //rsvd_f8
    uint32_t msiStateMachineStatus0MsiVfMsgGenEnbld0; //msi_state_machine_status_0
    uint32_t msiStateMachineStatus1MsiVfMsgGenEnbld1; //msi_state_machine_status_1
    uint32_t msiStateMachineStatus2MsiVfMsgGenEnbld2; //msi_state_machine_status_2
    uint32_t msiStateMachineStatus3MsiVfMsgGenEnbld2; //msi_state_machine_status_3
    uint32_t msiStateMachineStatus4MsiVfMsgGenEnbld4; //msi_state_machine_status_4
    uint32_t msiStateMachineStatus5MsiVfMsgGenEnbld5; //msi_state_machine_status_5
    uint32_t msiStateMachineStatus6MsiVfMsgGenEnbld6; //msi_state_machine_status_6
    uint32_t msiStateMachineStatus7MsiVfMsgGenEnbld7; //msi_state_machine_status_7
    uint32_t msiStateMachineStatus8MsiVfMsgGenEnbld8; //msi_state_machine_status_8
    uint32_t msiStateMachineStatus9MsiVfMsgGenEnbld9; //msi_state_machine_status_9
    uint32_t msiStateMachineStatus10MsiVfMsgGenEnbld10; //msi_state_machine_status_10
    uint32_t msiStateMachineStatus11MsiVfMsgGenEnbld11; //msi_state_machine_status_11
    uint32_t msiStateMachineStatus12MsiVfMsgGenEnbld12; //msi_state_machine_status_12
    uint32_t msiStateMachineStatus13MsiVfMsgGenEnbld13; //msi_state_machine_status_13
    uint32_t msiStateMachineStatus14MsiVfMsgGenEnbld14; //msi_state_machine_status_14
    uint32_t msiStateMachineStatus15MsiVfMsgGenEnbld15; //msi_state_machine_status_15
    MsiStateMachineStatus16_t msiStateMachineStatus16; //msi_state_machine_status_16
    uint32_t msiMsiXDmaPaused0DmaPaused0; //msi_msi_x_dma_paused_0
    uint32_t msiMsiXDmaPaused1DmaPaused1; //msi_msi_x_dma_paused_1
    uint32_t msiMsiXDmaPaused2DmaPaused2; //msi_msi_x_dma_paused_2
    uint32_t msiMsiXDmaPaused3DmaPaused3; //msi_msi_x_dma_paused_3
    uint32_t msiMsiXDmaPaused4DmaPaused4; //msi_msi_x_dma_paused_4
    uint32_t msiMsiXDmaPaused5DmaPaused5; //msi_msi_x_dma_paused_5
    uint32_t msiMsiXDmaPaused6DmaPaused6; //msi_msi_x_dma_paused_6
    uint32_t msiMsiXDmaPaused7DmaPaused7; //msi_msi_x_dma_paused_7
    uint32_t msiMsiXDmaPaused8DmaPaused8; //msi_msi_x_dma_paused_8
    uint32_t msiMsiXDmaPaused9DmaPaused9; //msi_msi_x_dma_paused_9
    uint32_t msiMsiXDmaPaused10DmaPaused10; //msi_msi_x_dma_paused_10
    uint32_t msiMsiXDmaPaused11DmaPaused11; //msi_msi_x_dma_paused_11
    uint32_t msiMsiXDmaPaused12DmaPaused12; //msi_msi_x_dma_paused_12
    uint32_t msiMsiXDmaPaused13DmaPaused13; //msi_msi_x_dma_paused_13
    uint32_t msiMsiXDmaPaused14DmaPaused14; //msi_msi_x_dma_paused_14
    uint32_t msiMsiXDmaPaused15DmaPaused15; //msi_msi_x_dma_paused_15
    MsiMsiXDmaPaused16_t msiMsiXDmaPaused16; //msi_msi_x_dma_paused_16
    uint32_t msiMsiXDmaPausedIntrEnable0DmaPausedIrqEn0; //msi_msi_x_dma_paused_interrupt_enable_0
    uint32_t msiMsiXDmaPausedIntrEnable1DmaPausedIrqEn1; //msi_msi_x_dma_paused_interrupt_enable_1
    uint32_t msiMsiXDmaPausedIntrEnable2DmaPausedIrqEn2; //msi_msi_x_dma_paused_interrupt_enable_2
    uint32_t msiMsiXDmaPausedIntrEnable3DmaPausedIrqEn3; //msi_msi_x_dma_paused_interrupt_enable_3
    uint32_t msiMsiXDmaPausedIntrEnable4DmaPausedIrqEn4; //msi_msi_x_dma_paused_interrupt_enable_4
    uint32_t msiMsiXDmaPausedIntrEnable5DmaPausedIrqEn5; //msi_msi_x_dma_paused_interrupt_enable_5
    uint32_t msiMsiXDmaPausedIntrEnable6DmaPausedIrqEn6; //msi_msi_x_dma_paused_interrupt_enable_6
    uint32_t msiMsiXDmaPausedIntrEnable7DmaPausedIrqEn7; //msi_msi_x_dma_paused_interrupt_enable_7
    uint32_t msiMsiXDmaPausedIntrEnable8DmaPausedIrqEn8; //msi_msi_x_dma_paused_interrupt_enable_8
    uint32_t msiMsiXDmaPausedIntrEnable9DmaPausedIrqEn9; //msi_msi_x_dma_paused_interrupt_enable_9
    uint32_t msiMsiXDmaPausedIntrEnable10DmaPausedIrqEn10; //msi_msi_x_dma_paused_interrupt_enable_10
    uint32_t msiMsiXDmaPausedIntrEnable11DmaPausedIrqEn11; //msi_msi_x_dma_paused_interrupt_enable_11
    uint32_t msiMsiXDmaPausedIntrEnable12DmaPausedIrqEn12; //msi_msi_x_dma_paused_interrupt_enable_12
    uint32_t msiMsiXDmaPausedIntrEnable13DmaPausedIrqEn13; //msi_msi_x_dma_paused_interrupt_enable_13
    uint32_t msiMsiXDmaPausedIntrEnable14DmaPausedIrqEn14; //msi_msi_x_dma_paused_interrupt_enable_14
    uint32_t msiMsiXDmaPausedIntrEnable15DmaPausedIrqEn15; //msi_msi_x_dma_paused_interrupt_enable_15
    MsiMsiXDmaPausedIntrEnable16_t msiMsiXDmaPausedIntrEnable16; //msi_msi_x_dma_paused_interrupt_enable_16
    uint32_t msiXEnableFallingEdgeVf0MsiXEnFallingEdgeVf0; //msi_x_enable_falling_edge_vf_0
    uint32_t msiXEnableFallingEdgeVf1MsiXEnFallingEdgeVf1; //msi_x_enable_falling_edge_vf_1
    uint32_t msiXEnableFallingEdgeVf2MsiXEnFallingEdgeVf2; //msi_x_enable_falling_edge_vf_2
    uint32_t msiXEnableFallingEdgeVf3MsiXEnFallingEdgeVf3; //msi_x_enable_falling_edge_vf_3
    uint32_t msiXEnableFallingEdgeVf4MsiXEnFallingEdgeVf4; //msi_x_enable_falling_edge_vf_4
    uint32_t msiXEnableFallingEdgeVf5MsiXEnFallingEdgeVf5; //msi_x_enable_falling_edge_vf_5
    uint32_t msiXEnableFallingEdgeVf6MsiXEnFallingEdgeVf6; //msi_x_enable_falling_edge_vf_6
    uint32_t msiXEnableFallingEdgeVf7MsiXEnFallingEdgeVf7; //msi_x_enable_falling_edge_vf_7
    uint32_t msiXEnableFallingEdgeVf8MsiXEnFallingEdgeVf8; //msi_x_enable_falling_edge_vf_8
    uint32_t msiXEnableFallingEdgeVf9MsiXEnFallingEdgeVf9; //msi_x_enable_falling_edge_vf_9
    uint32_t msiXEnableFallingEdgeVf10MsiXEnFallingEdgeVf10; //msi_x_enable_falling_edge_vf_10
    uint32_t msiXEnableFallingEdgeVf11MsiXEnFallingEdgeVf11; //msi_x_enable_falling_edge_vf_11
    uint32_t msiXEnableFallingEdgeVf12MsiXEnFallingEdgeVf12; //msi_x_enable_falling_edge_vf_12
    uint32_t msiXEnableFallingEdgeVf13MsiXEnFallingEdgeVf13; //msi_x_enable_falling_edge_vf_13
    uint32_t msiXEnableFallingEdgeVf14MsiXEnFallingEdgeVf14; //msi_x_enable_falling_edge_vf_14
    uint32_t msiXEnableFallingEdgeVf15MsiXEnFallingEdgeVf15; //msi_x_enable_falling_edge_vf_15
    MsiXEnableFallingEdgePf_t msiXEnableFallingEdgePf; //msi_x_enable_falling_edge_pf
    uint32_t msiEnableFallingEdgeVf0MsiEnFallingEdgeVf0; //msi_enable_falling_edge_vf_0
    uint32_t msiEnableFallingEdgeVf1MsiEnFallingEdgeVf1; //msi_enable_falling_edge_vf_1
    uint32_t msiEnableFallingEdgeVf2MsiEnFallingEdgeVf2; //msi_enable_falling_edge_vf_2
    uint32_t msiEnableFallingEdgeVf3MsiEnFallingEdgeVf3; //msi_enable_falling_edge_vf_3
    uint32_t msiEnableFallingEdgeVf4MsiEnFallingEdgeVf4; //msi_enable_falling_edge_vf_4
    uint32_t msiEnableFallingEdgeVf5MsiEnFallingEdgeVf5; //msi_enable_falling_edge_vf_5
    uint32_t msiEnableFallingEdgeVf6MsiEnFallingEdgeVf6; //msi_enable_falling_edge_vf_6
    uint32_t msiEnableFallingEdgeVf7MsiEnFallingEdgeVf7; //msi_enable_falling_edge_vf_7
    uint32_t msiEnableFallingEdgeVf8MsiEnFallingEdgeVf8; //msi_enable_falling_edge_vf_8
    uint32_t msiEnableFallingEdgeVf9MsiEnFallingEdgeVf9; //msi_enable_falling_edge_vf_9
    uint32_t msiEnableFallingEdgeVf10MsiEnFallingEdgeVf10; //msi_enable_falling_edge_vf_10
    uint32_t msiEnableFallingEdgeVf11MsiEnFallingEdgeVf11; //msi_enable_falling_edge_vf_11
    uint32_t msiEnableFallingEdgeVf12MsiEnFallingEdgeVf12; //msi_enable_falling_edge_vf_12
    uint32_t msiEnableFallingEdgeVf13MsiEnFallingEdgeVf13; //msi_enable_falling_edge_vf_13
    uint32_t msiEnableFallingEdgeVf14MsiEnFallingEdgeVf14; //msi_enable_falling_edge_vf_14
    uint32_t msiEnableFallingEdgeVf15MsiEnFallingEdgeVf15; //msi_enable_falling_edge_vf_15
    MsiEnableFallingEdgePf_t msiEnableFallingEdgePf; //msi_enable_falling_edge_pf
    uint32_t msiXEnableRisingEdgeVf0MsiXEnRisingEdgeVf0; //msi_x_enable_rising_edge_vf_0
    uint32_t msiXEnableRisingEdgeVf1MsiXEnRisingEdgeVf1; //msi_x_enable_rising_edge_vf_1
    uint32_t msiXEnableRisingEdgeVf2MsiXEnRisingEdgeVf2; //msi_x_enable_rising_edge_vf_2
    uint32_t msiXEnableRisingEdgeVf3MsiXEnRisingEdgeVf3; //msi_x_enable_rising_edge_vf_3
    uint32_t msiXEnableRisingEdgeVf4MsiXEnRisingEdgeVf4; //msi_x_enable_rising_edge_vf_4
    uint32_t msiXEnableRisingEdgeVf5MsiXEnRisingEdgeVf5; //msi_x_enable_rising_edge_vf_5
    uint32_t msiXEnableRisingEdgeVf6MsiXEnRisingEdgeVf6; //msi_x_enable_rising_edge_vf_6
    uint32_t msiXEnableRisingEdgeVf7MsiXEnRisingEdgeVf7; //msi_x_enable_rising_edge_vf_7
    uint32_t msiXEnableRisingEdgeVf8MsiXEnRisingEdgeVf8; //msi_x_enable_rising_edge_vf_8
    uint32_t msiXEnableRisingEdgeVf9MsiXEnRisingEdgeVf9; //msi_x_enable_rising_edge_vf_9
    uint32_t msiXEnableRisingEdgeVf10MsiXEnRisingEdgeVf10; //msi_x_enable_rising_edge_vf_10
    uint32_t msiXEnableRisingEdgeVf11MsiXEnRisingEdgeVf11; //msi_x_enable_rising_edge_vf_11
    uint32_t msiXEnableRisingEdgeVf12MsiXEnRisingEdgeVf12; //msi_x_enable_rising_edge_vf_12
    uint32_t msiXEnableRisingEdgeVf13MsiXEnRisingEdgeVf13; //msi_x_enable_rising_edge_vf_13
    uint32_t msiXEnableRisingEdgeVf14MsiXEnRisingEdgeVf14; //msi_x_enable_rising_edge_vf_14
    uint32_t msiXEnableRisingEdgeVf15MsiXEnRisingEdgeVf15; //msi_x_enable_rising_edge_vf_15
    MsiXEnableRisingEdgePf_t msiXEnableRisingEdgePf; //msi_x_enable_rising_edge_pf
    uint32_t msiEnableRisingEdgeVf0MsiEnRisingEdgeVf0; //msi_enable_rising_edge_vf_0
    uint32_t msiEnableRisingEdgeVf1MsiEnRisingEdgeVf1; //msi_enable_rising_edge_vf_1
    uint32_t msiEnableRisingEdgeVf2MsiEnRisingEdgeVf2; //msi_enable_rising_edge_vf_2
    uint32_t msiEnableRisingEdgeVf3MsiEnRisingEdgeVf3; //msi_enable_rising_edge_vf_3
    uint32_t msiEnableRisingEdgeVf4MsiEnRisingEdgeVf4; //msi_enable_rising_edge_vf_4
    uint32_t msiEnableRisingEdgeVf5MsiEnRisingEdgeVf5; //msi_enable_rising_edge_vf_5
    uint32_t msiEnableRisingEdgeVf6MsiEnRisingEdgeVf6; //msi_enable_rising_edge_vf_6
    uint32_t msiEnableRisingEdgeVf7MsiEnRisingEdgeVf7; //msi_enable_rising_edge_vf_7
    uint32_t msiEnableRisingEdgeVf8MsiEnRisingEdgeVf8; //msi_enable_rising_edge_vf_8
    uint32_t msiEnableRisingEdgeVf9MsiEnRisingEdgeVf9; //msi_enable_rising_edge_vf_9
    uint32_t msiEnableRisingEdgeVf10MsiEnRisingEdgeVf10; //msi_enable_rising_edge_vf_10
    uint32_t msiEnableRisingEdgeVf11MsiEnRisingEdgeVf11; //msi_enable_rising_edge_vf_11
    uint32_t msiEnableRisingEdgeVf12MsiEnRisingEdgeVf12; //msi_enable_rising_edge_vf_12
    uint32_t msiEnableRisingEdgeVf13MsiEnRisingEdgeVf13; //msi_enable_rising_edge_vf_13
    uint32_t msiEnableRisingEdgeVf14MsiEnRisingEdgeVf14; //msi_enable_rising_edge_vf_14
    uint32_t msiEnableRisingEdgeVf15MsiEnRisingEdgeVf15; //msi_enable_rising_edge_vf_15
    MsiEnableRisingEdgePf_t msiEnableRisingEdgePf; //msi_enable_rising_edge_pf
    uint32_t msiXTableUpdatedVf0MsixTblUpdtdVf0; //msi_x_table_updated_vf_0
    uint32_t msiXTableUpdatedVf1MsixTblUpdtdVf1; //msi_x_table_updated_vf_1
    uint32_t msiXTableUpdatedVf2MsixTblUpdtdVf2; //msi_x_table_updated_vf_2
    uint32_t msiXTableUpdatedVf3MsixTblUpdtdVf3; //msi_x_table_updated_vf_3
    uint32_t msiXTableUpdatedVf4MsixTblUpdtdVf4; //msi_x_table_updated_vf_4
    uint32_t msiXTableUpdatedVf5MsixTblUpdtdVf5; //msi_x_table_updated_vf_5
    uint32_t msiXTableUpdatedVf6MsixTblUpdtdVf6; //msi_x_table_updated_vf_6
    uint32_t msiXTableUpdatedVf7MsixTblUpdtdVf7; //msi_x_table_updated_vf_7
    uint32_t msiXTableUpdatedVf8MsixTblUpdtdVf8; //msi_x_table_updated_vf_8
    uint32_t msiXTableUpdatedVf9MsixTblUpdtdVf9; //msi_x_table_updated_vf_9
    uint32_t msiXTableUpdatedVf10MsixTblUpdtdVf10; //msi_x_table_updated_vf_10
    uint32_t msiXTableUpdatedVf11MsixTblUpdtdVf11; //msi_x_table_updated_vf_11
    uint32_t msiXTableUpdatedVf12MsixTblUpdtdVf12; //msi_x_table_updated_vf_12
    uint32_t msiXTableUpdatedVf13MsixTblUpdtdVf13; //msi_x_table_updated_vf_13
    uint32_t msiXTableUpdatedVf14MsixTblUpdtdVf14; //msi_x_table_updated_vf_14
    uint32_t msiXTableUpdatedVf15MsixTblUpdtdVf15; //msi_x_table_updated_vf_15
    MsiXTableUpdatedPf_t msiXTableUpdatedPf; //msi_x_table_updated_pf
    uint32_t bothModesEnabledErrVf0MsiMsixBothEnbldVf0; //both_modes_enabled_err_vf_0
    uint32_t bothModesEnabledErrVf1MsiMsixBothEnbldVf1; //both_modes_enabled_err_vf_1
    uint32_t bothModesEnabledErrVf2MsiMsixBothEnbldVf2; //both_modes_enabled_err_vf_2
    uint32_t bothModesEnabledErrVf3MsiMsixBothEnbldVf2; //both_modes_enabled_err_vf_3
    uint32_t bothModesEnabledErrVf4MsiMsixBothEnbldVf4; //both_modes_enabled_err_vf_4
    uint32_t bothModesEnabledErrVf5MsiMsixBothEnbldVf5; //both_modes_enabled_err_vf_5
    uint32_t bothModesEnabledErrVf6MsiMsixBothEnbldVf6; //both_modes_enabled_err_vf_6
    uint32_t bothModesEnabledErrVf7MsiMsixBothEnbldVf7; //both_modes_enabled_err_vf_7
    uint32_t bothModesEnabledErrVf8MsiMsixBothEnbldVf8; //both_modes_enabled_err_vf_8
    uint32_t bothModesEnabledErrVf9MsiMsixBothEnbldVf9; //both_modes_enabled_err_vf_9
    uint32_t bothModesEnabledErrVf10MsiMsixBothEnbldVf10; //both_modes_enabled_err_vf_10
    uint32_t bothModesEnabledErrVf11MsiMsixBothEnbldVf11; //both_modes_enabled_err_vf_11
    uint32_t bothModesEnabledErrVf12MsiMsixBothEnbldVf12; //both_modes_enabled_err_vf_12
    uint32_t bothModesEnabledErrVf13MsiMsixBothEnbldVf13; //both_modes_enabled_err_vf_13
    uint32_t bothModesEnabledErrVf14MsiMsixBothEnbldVf14; //both_modes_enabled_err_vf_14
    uint32_t bothModesEnabledErrVf15MsiMsixBothEnbldVf15; //both_modes_enabled_err_vf_15
    BothModesEnabledErrPf_t bothModesEnabledErrPf; //both_modes_enabled_err_pf
    MsiXIcuIntrRoutingControl_t msiXIcuIntrRoutingControl; //msi_x_icu_interrupt_routing_control
    PcieCapabilitiesStructureSelect_t pcieCapabilitiesStructureSelect; //pcie_capabilities_structure_select
    MsiXMonitorPortSelect_t msiXMonitorPortSelect; //msi_x_monitor_port_select
    AwuserCtrl_t awuserCtrl;              //awuser_ctrl
} MsixCommonRegisters_t;

/// @brief 0x2000
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixPf0FuncRegisters_t;

/// @brief 0x1F80
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf63FuncRegisters_t;

/// @brief 0x1F00
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf62FuncRegisters_t;

/// @brief 0x1E80
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf61FuncRegisters_t;

/// @brief 0x1E00
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf60FuncRegisters_t;

/// @brief 0x1D80
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf59FuncRegisters_t;

/// @brief 0x1D00
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf58FuncRegisters_t;

/// @brief 0x1C80
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf57FuncRegisters_t;

/// @brief 0x1C00
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf56FuncRegisters_t;

/// @brief 0x1B80
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf55FuncRegisters_t;

/// @brief 0x1B00
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf54FuncRegisters_t;

/// @brief 0x1A80
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf53FuncRegisters_t;

/// @brief 0x1A00
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf52FuncRegisters_t;

/// @brief 0x1980
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf51FuncRegisters_t;

/// @brief 0x1900
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf50FuncRegisters_t;

/// @brief 0x1880
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf49FuncRegisters_t;

/// @brief 0x1800
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf48FuncRegisters_t;

/// @brief 0x1780
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf47FuncRegisters_t;

/// @brief 0x1700
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf46FuncRegisters_t;

/// @brief 0x1680
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf45FuncRegisters_t;

/// @brief 0x1600
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf44FuncRegisters_t;

/// @brief 0x1580
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf43FuncRegisters_t;

/// @brief 0x1500
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf42FuncRegisters_t;

/// @brief 0x1480
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf41FuncRegisters_t;

/// @brief 0x1400
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf40FuncRegisters_t;

/// @brief 0x1380
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf39FuncRegisters_t;

/// @brief 0x1300
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf38FuncRegisters_t;

/// @brief 0x1280
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf37FuncRegisters_t;

/// @brief 0x1200
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf36FuncRegisters_t;

/// @brief 0x1180
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf35FuncRegisters_t;

/// @brief 0x1100
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf34FuncRegisters_t;

/// @brief 0x1080
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf33FuncRegisters_t;

/// @brief 0x1000
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf32FuncRegisters_t;

/// @brief 0xF80
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf31FuncRegisters_t;

/// @brief 0xF00
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf30FuncRegisters_t;

/// @brief 0xE80
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf29FuncRegisters_t;

/// @brief 0xE00
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf28FuncRegisters_t;

/// @brief 0xD80
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf27FuncRegisters_t;

/// @brief 0xD00
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf26FuncRegisters_t;

/// @brief 0xC80
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf25FuncRegisters_t;

/// @brief 0xC00
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf24FuncRegisters_t;

/// @brief 0xB80
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf23FuncRegisters_t;

/// @brief 0xB00
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf22FuncRegisters_t;

/// @brief 0xA80
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf21FuncRegisters_t;

/// @brief 0xA00
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf20FuncRegisters_t;

/// @brief 0x980
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf19FuncRegisters_t;

/// @brief 0x900
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf18FuncRegisters_t;

/// @brief 0x880
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf17FuncRegisters_t;

/// @brief 0x800
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf16FuncRegisters_t;

/// @brief 0x780
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf15FuncRegisters_t;

/// @brief 0x700
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf14FuncRegisters_t;

/// @brief 0x680
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf13FuncRegisters_t;

/// @brief 0x600
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf12FuncRegisters_t;

/// @brief 0x580
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf11FuncRegisters_t;

/// @brief 0x500
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf10FuncRegisters_t;

/// @brief 0x480
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf9FuncRegisters_t;

/// @brief 0x400
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf8FuncRegisters_t;

/// @brief 0x380
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf7FuncRegisters_t;

/// @brief 0x300
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf6FuncRegisters_t;

/// @brief 0x280
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf5FuncRegisters_t;

/// @brief 0x200
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf4FuncRegisters_t;

/// @brief 0x180
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf3FuncRegisters_t;

/// @brief 0x100
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf2FuncRegisters_t;

/// @brief 0x80
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf1FuncRegisters_t;

/// @brief 0x0
typedef struct
{
    uint32_t pba;                         //pba
    uint32_t setPba;                      //set_pba
    uint32_t resetPba;                    //reset_pba
    uint32_t psa;                         //psa
    uint32_t setPsa;                      //set_psa
    uint32_t resetPsa;                    //reset_psa
    uint32_t maskBitShadow;               //mask_bit_shadow
    uint32_t msiXTableVfpfMsgdataUpdatedMsixTblVfpfMsgDataUpdated; //msi_x_table_vfpf_msgdata_updated
    uint8_t rsvd20[8];                    //rsvd_20
    MsixtableInterfaceSelect_t tableInterfaceSelect; //table_interface_select
} MsixVf0FuncRegisters_t;

typedef struct
{
    MsixVf0FuncRegisters_t msixVf0FuncRegisters;                            // 0x0 : msix_vf0_func_registers / 
    uint8_t rsvd2c[84];                                                     // 0x2C : rsvd_2c / rsvd_2c
    MsixVf1FuncRegisters_t msixVf1FuncRegisters;                            // 0x80 : msix_vf1_func_registers / 
    uint8_t rsvdAc[84];                                                     // 0xAC : rsvd_ac / rsvd_ac
    MsixVf2FuncRegisters_t msixVf2FuncRegisters;                            // 0x100 : msix_vf2_func_registers / 
    uint8_t rsvd12c[84];                                                    // 0x12C : rsvd_12c / rsvd_12c
    MsixVf3FuncRegisters_t msixVf3FuncRegisters;                            // 0x180 : msix_vf3_func_registers / 
    uint8_t rsvd1ac[84];                                                    // 0x1AC : rsvd_1ac / rsvd_1ac
    MsixVf4FuncRegisters_t msixVf4FuncRegisters;                            // 0x200 : msix_vf4_func_registers / 
    uint8_t rsvd22c[84];                                                    // 0x22C : rsvd_22c / rsvd_22c
    MsixVf5FuncRegisters_t msixVf5FuncRegisters;                            // 0x280 : msix_vf5_func_registers / 
    uint8_t rsvd2ac[84];                                                    // 0x2AC : rsvd_2ac / rsvd_2ac
    MsixVf6FuncRegisters_t msixVf6FuncRegisters;                            // 0x300 : msix_vf6_func_registers / 
    uint8_t rsvd32c[84];                                                    // 0x32C : rsvd_32c / rsvd_32c
    MsixVf7FuncRegisters_t msixVf7FuncRegisters;                            // 0x380 : msix_vf7_func_registers / 
    uint8_t rsvd3ac[84];                                                    // 0x3AC : rsvd_3ac / rsvd_3ac
    MsixVf8FuncRegisters_t msixVf8FuncRegisters;                            // 0x400 : msix_vf8_func_registers / 
    uint8_t rsvd42c[84];                                                    // 0x42C : rsvd_42c / rsvd_42c
    MsixVf9FuncRegisters_t msixVf9FuncRegisters;                            // 0x480 : msix_vf9_func_registers / 
    uint8_t rsvd4ac[84];                                                    // 0x4AC : rsvd_4ac / rsvd_4ac
    MsixVf10FuncRegisters_t msixVf10FuncRegisters;                          // 0x500 : msix_vf10_func_registers / 
    uint8_t rsvd52c[84];                                                    // 0x52C : rsvd_52c / rsvd_52c
    MsixVf11FuncRegisters_t msixVf11FuncRegisters;                          // 0x580 : msix_vf11_func_registers / 
    uint8_t rsvd5ac[84];                                                    // 0x5AC : rsvd_5ac / rsvd_5ac
    MsixVf12FuncRegisters_t msixVf12FuncRegisters;                          // 0x600 : msix_vf12_func_registers / 
    uint8_t rsvd62c[84];                                                    // 0x62C : rsvd_62c / rsvd_62c
    MsixVf13FuncRegisters_t msixVf13FuncRegisters;                          // 0x680 : msix_vf13_func_registers / 
    uint8_t rsvd6ac[84];                                                    // 0x6AC : rsvd_6ac / rsvd_6ac
    MsixVf14FuncRegisters_t msixVf14FuncRegisters;                          // 0x700 : msix_vf14_func_registers / 
    uint8_t rsvd72c[84];                                                    // 0x72C : rsvd_72c / rsvd_72c
    MsixVf15FuncRegisters_t msixVf15FuncRegisters;                          // 0x780 : msix_vf15_func_registers / 
    uint8_t rsvd7ac[84];                                                    // 0x7AC : rsvd_7ac / rsvd_7ac
    MsixVf16FuncRegisters_t msixVf16FuncRegisters;                          // 0x800 : msix_vf16_func_registers / 
    uint8_t rsvd82c[84];                                                    // 0x82C : rsvd_82c / rsvd_82c
    MsixVf17FuncRegisters_t msixVf17FuncRegisters;                          // 0x880 : msix_vf17_func_registers / 
    uint8_t rsvd8ac[84];                                                    // 0x8AC : rsvd_8ac / rsvd_8ac
    MsixVf18FuncRegisters_t msixVf18FuncRegisters;                          // 0x900 : msix_vf18_func_registers / 
    uint8_t rsvd92c[84];                                                    // 0x92C : rsvd_92c / rsvd_92c
    MsixVf19FuncRegisters_t msixVf19FuncRegisters;                          // 0x980 : msix_vf19_func_registers / 
    uint8_t rsvd9ac[84];                                                    // 0x9AC : rsvd_9ac / rsvd_9ac
    MsixVf20FuncRegisters_t msixVf20FuncRegisters;                          // 0xA00 : msix_vf20_func_registers / 
    uint8_t rsvdA2c[84];                                                    // 0xA2C : rsvd_a2c / rsvd_a2c
    MsixVf21FuncRegisters_t msixVf21FuncRegisters;                          // 0xA80 : msix_vf21_func_registers / 
    uint8_t rsvdAac[84];                                                    // 0xAAC : rsvd_aac / rsvd_aac
    MsixVf22FuncRegisters_t msixVf22FuncRegisters;                          // 0xB00 : msix_vf22_func_registers / 
    uint8_t rsvdB2c[84];                                                    // 0xB2C : rsvd_b2c / rsvd_b2c
    MsixVf23FuncRegisters_t msixVf23FuncRegisters;                          // 0xB80 : msix_vf23_func_registers / 
    uint8_t rsvdBac[84];                                                    // 0xBAC : rsvd_bac / rsvd_bac
    MsixVf24FuncRegisters_t msixVf24FuncRegisters;                          // 0xC00 : msix_vf24_func_registers / 
    uint8_t rsvdC2c[84];                                                    // 0xC2C : rsvd_c2c / rsvd_c2c
    MsixVf25FuncRegisters_t msixVf25FuncRegisters;                          // 0xC80 : msix_vf25_func_registers / 
    uint8_t rsvdCac[84];                                                    // 0xCAC : rsvd_cac / rsvd_cac
    MsixVf26FuncRegisters_t msixVf26FuncRegisters;                          // 0xD00 : msix_vf26_func_registers / 
    uint8_t rsvdD2c[84];                                                    // 0xD2C : rsvd_d2c / rsvd_d2c
    MsixVf27FuncRegisters_t msixVf27FuncRegisters;                          // 0xD80 : msix_vf27_func_registers / 
    uint8_t rsvdDac[84];                                                    // 0xDAC : rsvd_dac / rsvd_dac
    MsixVf28FuncRegisters_t msixVf28FuncRegisters;                          // 0xE00 : msix_vf28_func_registers / 
    uint8_t rsvdE2c[84];                                                    // 0xE2C : rsvd_e2c / rsvd_e2c
    MsixVf29FuncRegisters_t msixVf29FuncRegisters;                          // 0xE80 : msix_vf29_func_registers / 
    uint8_t rsvdEac[84];                                                    // 0xEAC : rsvd_eac / rsvd_eac
    MsixVf30FuncRegisters_t msixVf30FuncRegisters;                          // 0xF00 : msix_vf30_func_registers / 
    uint8_t rsvdF2c[84];                                                    // 0xF2C : rsvd_f2c / rsvd_f2c
    MsixVf31FuncRegisters_t msixVf31FuncRegisters;                          // 0xF80 : msix_vf31_func_registers / 
    uint8_t rsvdFac[84];                                                    // 0xFAC : rsvd_fac / rsvd_fac
    MsixVf32FuncRegisters_t msixVf32FuncRegisters;                          // 0x1000 : msix_vf32_func_registers / 
    uint8_t rsvd102c[84];                                                   // 0x102C : rsvd_102c / rsvd_102c
    MsixVf33FuncRegisters_t msixVf33FuncRegisters;                          // 0x1080 : msix_vf33_func_registers / 
    uint8_t rsvd10ac[84];                                                   // 0x10AC : rsvd_10ac / rsvd_10ac
    MsixVf34FuncRegisters_t msixVf34FuncRegisters;                          // 0x1100 : msix_vf34_func_registers / 
    uint8_t rsvd112c[84];                                                   // 0x112C : rsvd_112c / rsvd_112c
    MsixVf35FuncRegisters_t msixVf35FuncRegisters;                          // 0x1180 : msix_vf35_func_registers / 
    uint8_t rsvd11ac[84];                                                   // 0x11AC : rsvd_11ac / rsvd_11ac
    MsixVf36FuncRegisters_t msixVf36FuncRegisters;                          // 0x1200 : msix_vf36_func_registers / 
    uint8_t rsvd122c[84];                                                   // 0x122C : rsvd_122c / rsvd_122c
    MsixVf37FuncRegisters_t msixVf37FuncRegisters;                          // 0x1280 : msix_vf37_func_registers / 
    uint8_t rsvd12ac[84];                                                   // 0x12AC : rsvd_12ac / rsvd_12ac
    MsixVf38FuncRegisters_t msixVf38FuncRegisters;                          // 0x1300 : msix_vf38_func_registers / 
    uint8_t rsvd132c[84];                                                   // 0x132C : rsvd_132c / rsvd_132c
    MsixVf39FuncRegisters_t msixVf39FuncRegisters;                          // 0x1380 : msix_vf39_func_registers / 
    uint8_t rsvd13ac[84];                                                   // 0x13AC : rsvd_13ac / rsvd_13ac
    MsixVf40FuncRegisters_t msixVf40FuncRegisters;                          // 0x1400 : msix_vf40_func_registers / 
    uint8_t rsvd142c[84];                                                   // 0x142C : rsvd_142c / rsvd_142c
    MsixVf41FuncRegisters_t msixVf41FuncRegisters;                          // 0x1480 : msix_vf41_func_registers / 
    uint8_t rsvd14ac[84];                                                   // 0x14AC : rsvd_14ac / rsvd_14ac
    MsixVf42FuncRegisters_t msixVf42FuncRegisters;                          // 0x1500 : msix_vf42_func_registers / 
    uint8_t rsvd152c[84];                                                   // 0x152C : rsvd_152c / rsvd_152c
    MsixVf43FuncRegisters_t msixVf43FuncRegisters;                          // 0x1580 : msix_vf43_func_registers / 
    uint8_t rsvd15ac[84];                                                   // 0x15AC : rsvd_15ac / rsvd_15ac
    MsixVf44FuncRegisters_t msixVf44FuncRegisters;                          // 0x1600 : msix_vf44_func_registers / 
    uint8_t rsvd162c[84];                                                   // 0x162C : rsvd_162c / rsvd_162c
    MsixVf45FuncRegisters_t msixVf45FuncRegisters;                          // 0x1680 : msix_vf45_func_registers / 
    uint8_t rsvd16ac[84];                                                   // 0x16AC : rsvd_16ac / rsvd_16ac
    MsixVf46FuncRegisters_t msixVf46FuncRegisters;                          // 0x1700 : msix_vf46_func_registers / 
    uint8_t rsvd172c[84];                                                   // 0x172C : rsvd_172c / rsvd_172c
    MsixVf47FuncRegisters_t msixVf47FuncRegisters;                          // 0x1780 : msix_vf47_func_registers / 
    uint8_t rsvd17ac[84];                                                   // 0x17AC : rsvd_17ac / rsvd_17ac
    MsixVf48FuncRegisters_t msixVf48FuncRegisters;                          // 0x1800 : msix_vf48_func_registers / 
    uint8_t rsvd182c[84];                                                   // 0x182C : rsvd_182c / rsvd_182c
    MsixVf49FuncRegisters_t msixVf49FuncRegisters;                          // 0x1880 : msix_vf49_func_registers / 
    uint8_t rsvd18ac[84];                                                   // 0x18AC : rsvd_18ac / rsvd_18ac
    MsixVf50FuncRegisters_t msixVf50FuncRegisters;                          // 0x1900 : msix_vf50_func_registers / 
    uint8_t rsvd192c[84];                                                   // 0x192C : rsvd_192c / rsvd_192c
    MsixVf51FuncRegisters_t msixVf51FuncRegisters;                          // 0x1980 : msix_vf51_func_registers / 
    uint8_t rsvd19ac[84];                                                   // 0x19AC : rsvd_19ac / rsvd_19ac
    MsixVf52FuncRegisters_t msixVf52FuncRegisters;                          // 0x1A00 : msix_vf52_func_registers / 
    uint8_t rsvd1a2c[84];                                                   // 0x1A2C : rsvd_1a2c / rsvd_1a2c
    MsixVf53FuncRegisters_t msixVf53FuncRegisters;                          // 0x1A80 : msix_vf53_func_registers / 
    uint8_t rsvd1aac[84];                                                   // 0x1AAC : rsvd_1aac / rsvd_1aac
    MsixVf54FuncRegisters_t msixVf54FuncRegisters;                          // 0x1B00 : msix_vf54_func_registers / 
    uint8_t rsvd1b2c[84];                                                   // 0x1B2C : rsvd_1b2c / rsvd_1b2c
    MsixVf55FuncRegisters_t msixVf55FuncRegisters;                          // 0x1B80 : msix_vf55_func_registers / 
    uint8_t rsvd1bac[84];                                                   // 0x1BAC : rsvd_1bac / rsvd_1bac
    MsixVf56FuncRegisters_t msixVf56FuncRegisters;                          // 0x1C00 : msix_vf56_func_registers / 
    uint8_t rsvd1c2c[84];                                                   // 0x1C2C : rsvd_1c2c / rsvd_1c2c
    MsixVf57FuncRegisters_t msixVf57FuncRegisters;                          // 0x1C80 : msix_vf57_func_registers / 
    uint8_t rsvd1cac[84];                                                   // 0x1CAC : rsvd_1cac / rsvd_1cac
    MsixVf58FuncRegisters_t msixVf58FuncRegisters;                          // 0x1D00 : msix_vf58_func_registers / 
    uint8_t rsvd1d2c[84];                                                   // 0x1D2C : rsvd_1d2c / rsvd_1d2c
    MsixVf59FuncRegisters_t msixVf59FuncRegisters;                          // 0x1D80 : msix_vf59_func_registers / 
    uint8_t rsvd1dac[84];                                                   // 0x1DAC : rsvd_1dac / rsvd_1dac
    MsixVf60FuncRegisters_t msixVf60FuncRegisters;                          // 0x1E00 : msix_vf60_func_registers / 
    uint8_t rsvd1e2c[84];                                                   // 0x1E2C : rsvd_1e2c / rsvd_1e2c
    MsixVf61FuncRegisters_t msixVf61FuncRegisters;                          // 0x1E80 : msix_vf61_func_registers / 
    uint8_t rsvd1eac[84];                                                   // 0x1EAC : rsvd_1eac / rsvd_1eac
    MsixVf62FuncRegisters_t msixVf62FuncRegisters;                          // 0x1F00 : msix_vf62_func_registers / 
    uint8_t rsvd1f2c[84];                                                   // 0x1F2C : rsvd_1f2c / rsvd_1f2c
    MsixVf63FuncRegisters_t msixVf63FuncRegisters;                          // 0x1F80 : msix_vf63_func_registers / 
    uint8_t rsvd1fac[84];                                                   // 0x1FAC : rsvd_1fac / rsvd_1fac
    MsixPf0FuncRegisters_t msixPf0FuncRegisters;                            // 0x2000 : msix_pf0_func_registers / 
    uint8_t rsvd202c[90068];                                                // 0x202C : rsvd_202c / rsvd_202c
    MsixCommonRegisters_t msixCommonRegisters;                              // 0x18000 : msix_common_registers / 
} Msix_t;

COMPILE_ASSERT(offsetof(Msix_t,msixVf0FuncRegisters)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf1FuncRegisters)==0x80,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf2FuncRegisters)==0x100,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf3FuncRegisters)==0x180,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf4FuncRegisters)==0x200,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf5FuncRegisters)==0x280,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf6FuncRegisters)==0x300,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf7FuncRegisters)==0x380,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf8FuncRegisters)==0x400,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf9FuncRegisters)==0x480,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf10FuncRegisters)==0x500,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf11FuncRegisters)==0x580,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf12FuncRegisters)==0x600,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf13FuncRegisters)==0x680,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf14FuncRegisters)==0x700,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf15FuncRegisters)==0x780,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf16FuncRegisters)==0x800,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf17FuncRegisters)==0x880,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf18FuncRegisters)==0x900,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf19FuncRegisters)==0x980,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf20FuncRegisters)==0xA00,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf21FuncRegisters)==0xA80,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf22FuncRegisters)==0xB00,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf23FuncRegisters)==0xB80,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf24FuncRegisters)==0xC00,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf25FuncRegisters)==0xC80,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf26FuncRegisters)==0xD00,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf27FuncRegisters)==0xD80,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf28FuncRegisters)==0xE00,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf29FuncRegisters)==0xE80,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf30FuncRegisters)==0xF00,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf31FuncRegisters)==0xF80,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf32FuncRegisters)==0x1000,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf33FuncRegisters)==0x1080,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf34FuncRegisters)==0x1100,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf35FuncRegisters)==0x1180,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf36FuncRegisters)==0x1200,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf37FuncRegisters)==0x1280,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf38FuncRegisters)==0x1300,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf39FuncRegisters)==0x1380,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf40FuncRegisters)==0x1400,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf41FuncRegisters)==0x1480,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf42FuncRegisters)==0x1500,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf43FuncRegisters)==0x1580,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf44FuncRegisters)==0x1600,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf45FuncRegisters)==0x1680,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf46FuncRegisters)==0x1700,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf47FuncRegisters)==0x1780,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf48FuncRegisters)==0x1800,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf49FuncRegisters)==0x1880,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf50FuncRegisters)==0x1900,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf51FuncRegisters)==0x1980,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf52FuncRegisters)==0x1A00,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf53FuncRegisters)==0x1A80,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf54FuncRegisters)==0x1B00,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf55FuncRegisters)==0x1B80,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf56FuncRegisters)==0x1C00,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf57FuncRegisters)==0x1C80,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf58FuncRegisters)==0x1D00,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf59FuncRegisters)==0x1D80,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf60FuncRegisters)==0x1E00,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf61FuncRegisters)==0x1E80,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf62FuncRegisters)==0x1F00,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixVf63FuncRegisters)==0x1F80,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixPf0FuncRegisters)==0x2000,"check register structure offset");
COMPILE_ASSERT(offsetof(Msix_t,msixCommonRegisters)==0x18000,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile Msix_t rMsix; ///< 0xA1800000
