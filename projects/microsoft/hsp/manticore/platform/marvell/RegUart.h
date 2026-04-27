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
//! @brief UART Registers
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
        uint32_t UART_RCVR_BFR               :8;      ///<BIT [7:0] uart_rcvr_bfr
        uint32_t RSVD1                       :4;      ///<BIT [11:8] rsvd1
        uint32_t OVRUN_ERR_MIRR_UART         :1;      ///<BIT [12] ovrun_err_mirr_uart
        uint32_t PARITY_ERR_DET_UART         :1;      ///<BIT [13] parity_err_det_uart
        uint32_t FRAME_ERR_DET_UART          :1;      ///<BIT [14] frame_err_det_uart
        uint32_t BREAK_DET_UART              :1;      ///<BIT [15] break_det_uart
        uint32_t RSVD0                       :16;     ///<BIT [31:16] rsvd0
    } b;
} ReceiverBuffer_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UART_TRANS_HLD_UART         :8;      ///<BIT [7:0] uart_trans_hld_uart
        uint32_t RSVD                        :24;     ///<BIT [31:8] rsvd
    } b;
} TransmitterHolding_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OVRUN_ERR_INT_EN_UART       :1;      ///<BIT [0] ovrun_err_int_en_uart
        uint32_t PARITY_ERR_INT_EN_UART      :1;      ///<BIT [1] parity_err_int_en_uart
        uint32_t FRAME_ERR_INT_EN_UART       :1;      ///<BIT [2] frame_err_int_en_uart
        uint32_t BRK_DET_INT_EN_UART         :1;      ///<BIT [3] brk_det_int_en_uart
        uint32_t RX_RDY_INT_EN_UART          :1;      ///<BIT [4] rx_rdy_int_en_uart
        uint32_t TX_RDY_INT_EN_UART          :1;      ///<BIT [5] tx_rdy_int_en_uart
        uint32_t TX_EMPTY_INT_EN_UART        :1;      ///<BIT [6] tx_empty_int_en_uart
        uint32_t RXFIFO_HFULL_INT_EN_UART    :1;      ///<BIT [7] rxfifo_hfull_int_en_uart
        uint32_t TXFIFO_HFULL_INT_EN_UART    :1;      ///<BIT [8] txfifo_hfull_int_en_uart
        uint32_t TWO_STOP_BITS_UART          :1;      ///<BIT [9] two_stop_bits_uart
        uint32_t PARITY_EN_UART              :1;      ///<BIT [10] parity_en_uart
        uint32_t SEND_BREAK_SEQ_UART         :1;      ///<BIT [11] send_break_seq_uart
        uint32_t LOOPBACK_RW_EN_UART         :1;      ///<BIT [12] loopback_rw_en_uart
        uint32_t SET_MIRROR_EN_UART          :1;      ///<BIT [13] set_mirror_en_uart
        uint32_t RXFIFO_RST_UART             :1;      ///<BIT [14] rxfifo_rst_uart
        uint32_t TXFIFO_RST_UART             :1;      ///<BIT [15] txfifo_rst_uart
        uint32_t RX_HFULL_SEL_UART           :2;      ///<BIT [17:16] rx_hfull_sel_uart
        uint32_t TX_HFULL_SEL_UART           :2;      ///<BIT [19:18] tx_hfull_sel_uart
        uint32_t RTSN_THRESHOLD_SEL          :2;      ///<BIT [21:20] rtsn_threshold_sel
        uint32_t RSVD_22_29                  :8;      ///<BIT [29:22] rsvd_22_29
        uint32_t UART_SRESET                 :1;      ///<BIT [30] uart_sreset
        uint32_t PERR_MASK                   :1;      ///<BIT [31] perr_mask
    } b;
} UartControl_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OVRUN_ERR_UART              :1;      ///<BIT [0] ovrun_err_uart
        uint32_t PARITY_ERR_UART             :1;      ///<BIT [1] parity_err_uart
        uint32_t FRAME_ERR_UART              :1;      ///<BIT [2] frame_err_uart
        uint32_t BRK_DET_UART                :1;      ///<BIT [3] brk_det_uart
        uint32_t RX_READY_UART               :1;      ///<BIT [4] rx_ready_uart
        uint32_t TX_READY_UART               :1;      ///<BIT [5] tx_ready_uart
        uint32_t TX_EMPTY_UART               :1;      ///<BIT [6] tx_empty_uart
        uint32_t RXFIFO_HFULL_UART           :1;      ///<BIT [7] rxfifo_hfull_uart
        uint32_t RXFIFO_FULL_UART            :1;      ///<BIT [8] rxfifo_full_uart
        uint32_t RX_TOGGLE_UART              :1;      ///<BIT [9] rx_toggle_uart
        uint32_t TXFIFO_HFULL_UART           :1;      ///<BIT [10] txfifo_hfull_uart
        uint32_t TXFIFO_FULL_UART            :1;      ///<BIT [11] txfifo_full_uart
        uint32_t RXFIFO_EMPTY_UART           :1;      ///<BIT [12] rxfifo_empty_uart
        uint32_t TXFIFO_EMPTY_UART           :1;      ///<BIT [13] txfifo_empty_uart
        uint32_t RSVD_14_17                  :4;      ///<BIT [17:14] rsvd_14_17
        uint32_t UART_RTSN_THRESHOLD_MET     :1;      ///<BIT [18] uart_rtsn_threshold_met
        uint32_t RSVD_19_20                  :2;      ///<BIT [20:19] rsvd_19_20
        uint32_t UART_RSTN_O                 :1;      ///<BIT [21] uart_rstn_o
        uint32_t UART_CTSN_I                 :1;      ///<BIT [22] uart_ctsn_i
        uint32_t RSVD_23_31                  :9;      ///<BIT [31:23] rsvd_23_31
    } b;
} UartStatus_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BAUD_RATE_DIV_UART          :10;     ///<BIT [9:0] baud_rate_div_uart
        uint32_t RSVD2                       :6;      ///<BIT [15:10] rsvd2
        uint32_t POSSR_ENT_1_UART            :6;      ///<BIT [21:16] possr_ent_1_uart
        uint32_t RSVD1                       :2;      ///<BIT [23:22] rsvd1
        uint32_t POSSR_ENT_2_UART            :6;      ///<BIT [29:24] possr_ent_2_uart
        uint32_t RSVD0                       :2;      ///<BIT [31:30] rsvd0
    } b;
} PrgrmmblOversamplingStack1_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t POSSR_ENT_3_UART            :6;      ///<BIT [5:0] possr_ent_3_uart
        uint32_t RSVD1                       :2;      ///<BIT [7:6] rsvd1
        uint32_t POSSR_ENT_4_UART            :6;      ///<BIT [13:8] possr_ent_4_uart
        uint32_t RSVD0                       :18;     ///<BIT [31:14] rsvd0
    } b;
} PrgrmmblOversamplingStack2_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_TOGGLE_INT_EN_UART       :1;      ///<BIT [0] rx_toggle_int_en_uart
        uint32_t TXFIFO_FULL_INT_EN_UART     :1;      ///<BIT [1] txfifo_full_int_en_uart
        uint32_t RXFIFO_FULL_INT_EN_UART     :1;      ///<BIT [2] rxfifo_full_int_en_uart
        uint32_t TXFIFO_EMPTY_INT_EN_UART    :1;      ///<BIT [3] txfifo_empty_int_en_uart
        uint32_t RXFIFO_EMPTY_INT_EN_UART    :1;      ///<BIT [4] rxfifo_empty_int_en_uart
        uint32_t RSVD_5_8                    :4;      ///<BIT [8:5] rsvd_5_8
        uint32_t RTSN_THRESHOLD_INTR_EN      :1;      ///<BIT [9] rtsn_threshold_intr_en
        uint32_t RSVD_10_11                  :2;      ///<BIT [11:10] rsvd_10_11
        uint32_t UART_RTSN_INTR_EN           :1;      ///<BIT [12] uart_rtsn_intr_en
        uint32_t UART_CTSN_INTR_EN           :1;      ///<BIT [13] uart_ctsn_intr_en
        uint32_t UART_CTSN_BLOCK             :1;      ///<BIT [14] uart_ctsn_block
        uint32_t RSVD_15                     :1;      ///<BIT [15] rsvd_15
        uint32_t UART_RTSN_BLOCK             :1;      ///<BIT [16] uart_rtsn_block
        uint32_t RSVD_17_31                  :15;     ///<BIT [31:17] rsvd_17_31
    } b;
} Control2_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RXFIFO_BYTE_CNT             :8;      ///<BIT [7:0] rxfifo_byte_cnt
        uint32_t RSVD1                       :8;      ///<BIT [15:8] rsvd1
        uint32_t TXFIFO_BYTE_CNT             :8;      ///<BIT [23:16] txfifo_byte_cnt
        uint32_t RSVD0                       :8;      ///<BIT [31:24] rsvd0
    } b;
} Status2_t;

typedef struct
{
    ReceiverBuffer_t receiverBuffer;                                        // 0x0 : Receiver_buffer / 
    TransmitterHolding_t transmitterHolding;                                // 0x4 : Transmitter_Holding / 
    UartControl_t control;                                                  // 0x8 : Control / 
    UartStatus_t status;                                                    // 0xC : Status / 
    PrgrmmblOversamplingStack1_t prgrmmblOversamplingStack1;                // 0x10 : Programmable_Oversampling_Stack_1 / 
    uint8_t rsvd14[12];                                                     // 0x14 : rsvd_14 / rsvd_14
    PrgrmmblOversamplingStack2_t prgrmmblOversamplingStack2;                // 0x20 : Programmable_Oversampling_Stack_2 / 
    uint8_t rsvd24[20];                                                     // 0x24 : rsvd_24 / rsvd_24
    Control2_t control2;                                                    // 0x38 : Control_2 / 
    Status2_t status2;                                                      // 0x3C : Status_2 / 
} Uart_t;

COMPILE_ASSERT(offsetof(Uart_t,receiverBuffer)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(Uart_t,transmitterHolding)==0x4,"check register structure offset");
COMPILE_ASSERT(offsetof(Uart_t,control)==0x8,"check register structure offset");
COMPILE_ASSERT(offsetof(Uart_t,status)==0xC,"check register structure offset");
COMPILE_ASSERT(offsetof(Uart_t,prgrmmblOversamplingStack1)==0x10,"check register structure offset");
COMPILE_ASSERT(offsetof(Uart_t,prgrmmblOversamplingStack2)==0x20,"check register structure offset");
COMPILE_ASSERT(offsetof(Uart_t,control2)==0x38,"check register structure offset");
COMPILE_ASSERT(offsetof(Uart_t,status2)==0x3C,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile Uart_t rUart; ///< 0xB0009000
