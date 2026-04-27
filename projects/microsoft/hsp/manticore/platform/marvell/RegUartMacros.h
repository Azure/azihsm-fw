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
//! @brief UART Macros
//!
//=============================================================================

// Generated with

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "SysTypes.h"
#include "RegUart.h"

#define UART_BASE_ADDRESS	0xB0009000

/* -------- Receiver_buffer address 0xB0009000 -------- */
#define UART_RECEIVER_BUFFER_REG_ADDRESS	0xB0009000
#define Get_Uart_receiverBuffer()                   (Uart_Ptr->receiverBuffer.all)
#define Get_Uart_receiverBuffer_UartRcvrBfr()       (Uart_Ptr->receiverBuffer.b.UART_RCVR_BFR)
#define Get_Uart_receiverBuffer_OvrunErrMirrUart()  (Uart_Ptr->receiverBuffer.b.OVRUN_ERR_MIRR_UART)
#define Get_Uart_receiverBuffer_ParityErrDetUart()  (Uart_Ptr->receiverBuffer.b.PARITY_ERR_DET_UART)
#define Get_Uart_receiverBuffer_FrameErrDetUart()   (Uart_Ptr->receiverBuffer.b.FRAME_ERR_DET_UART)
#define Get_Uart_receiverBuffer_BreakDetUart()      (Uart_Ptr->receiverBuffer.b.BREAK_DET_UART)

#define UART_RECEIVER_BUFFER_UART_RCVR_BFR_MASK        0xff
#define UART_RECEIVER_BUFFER_OVRUN_ERR_MIRR_UART_MASK  0x1000
#define UART_RECEIVER_BUFFER_PARITY_ERR_DET_UART_MASK  0x2000
#define UART_RECEIVER_BUFFER_FRAME_ERR_DET_UART_MASK   0x4000
#define UART_RECEIVER_BUFFER_BREAK_DET_UART_MASK       0x8000
#define UART_RECEIVER_BUFFER_UART_RCVR_BFR_POS         0x0
#define UART_RECEIVER_BUFFER_OVRUN_ERR_MIRR_UART_POS   0xc
#define UART_RECEIVER_BUFFER_PARITY_ERR_DET_UART_POS   0xd
#define UART_RECEIVER_BUFFER_FRAME_ERR_DET_UART_POS    0xe
#define UART_RECEIVER_BUFFER_BREAK_DET_UART_POS        0xf


/* -------- Transmitter_Holding address 0xB0009004 -------- */
#define UART_TRANSMITTER_HOLDING_REG_ADDRESS	0xB0009004
#define Set_Uart_transmitterHolding(value)                   (Uart_Ptr->transmitterHolding.all=value)
#define Get_Uart_transmitterHolding()                        (Uart_Ptr->transmitterHolding.all)
#define Set_Uart_transmitterHolding_UartTransHldUart(value)  (Uart_Ptr->transmitterHolding.b.UART_TRANS_HLD_UART=value)

#define UART_TRANSMITTER_HOLDING_UART_TRANS_HLD_UART_MASK  0xff
#define UART_TRANSMITTER_HOLDING_UART_TRANS_HLD_UART_POS   0x0


/* -------- Control address 0xB0009008 -------- */
#define UART_CONTROL_REG_ADDRESS	0xB0009008
#define Set_Uart_control(value)                       (Uart_Ptr->control.all=value)
#define Get_Uart_control()                            (Uart_Ptr->control.all)
#define Set_Uart_control_RtsnThresholdSel(value)      (Uart_Ptr->control.b.RTSN_THRESHOLD_SEL=value)
#define Get_Uart_control_RtsnThresholdSel()           (Uart_Ptr->control.b.RTSN_THRESHOLD_SEL)
#define Set_Uart_control_TxHfullSelUart(value)        (Uart_Ptr->control.b.TX_HFULL_SEL_UART=value)
#define Get_Uart_control_TxHfullSelUart()             (Uart_Ptr->control.b.TX_HFULL_SEL_UART)
#define Set_Uart_control_RxHfullSelUart(value)        (Uart_Ptr->control.b.RX_HFULL_SEL_UART=value)
#define Get_Uart_control_RxHfullSelUart()             (Uart_Ptr->control.b.RX_HFULL_SEL_UART)
#define Set_Uart_control_OvrunErrIntEnUart(value)     (Uart_Ptr->control.b.OVRUN_ERR_INT_EN_UART=value)
#define Get_Uart_control_OvrunErrIntEnUart()          (Uart_Ptr->control.b.OVRUN_ERR_INT_EN_UART)
#define Set_Uart_control_ParityErrIntEnUart(value)    (Uart_Ptr->control.b.PARITY_ERR_INT_EN_UART=value)
#define Get_Uart_control_ParityErrIntEnUart()         (Uart_Ptr->control.b.PARITY_ERR_INT_EN_UART)
#define Set_Uart_control_FrameErrIntEnUart(value)     (Uart_Ptr->control.b.FRAME_ERR_INT_EN_UART=value)
#define Get_Uart_control_FrameErrIntEnUart()          (Uart_Ptr->control.b.FRAME_ERR_INT_EN_UART)
#define Set_Uart_control_BrkDetIntEnUart(value)       (Uart_Ptr->control.b.BRK_DET_INT_EN_UART=value)
#define Get_Uart_control_BrkDetIntEnUart()            (Uart_Ptr->control.b.BRK_DET_INT_EN_UART)
#define Set_Uart_control_RxRdyIntEnUart(value)        (Uart_Ptr->control.b.RX_RDY_INT_EN_UART=value)
#define Get_Uart_control_RxRdyIntEnUart()             (Uart_Ptr->control.b.RX_RDY_INT_EN_UART)
#define Set_Uart_control_TxRdyIntEnUart(value)        (Uart_Ptr->control.b.TX_RDY_INT_EN_UART=value)
#define Get_Uart_control_TxRdyIntEnUart()             (Uart_Ptr->control.b.TX_RDY_INT_EN_UART)
#define Set_Uart_control_TxEmptyIntEnUart(value)      (Uart_Ptr->control.b.TX_EMPTY_INT_EN_UART=value)
#define Get_Uart_control_TxEmptyIntEnUart()           (Uart_Ptr->control.b.TX_EMPTY_INT_EN_UART)
#define Set_Uart_control_RxfifoHfullIntEnUart(value)  (Uart_Ptr->control.b.RXFIFO_HFULL_INT_EN_UART=value)
#define Get_Uart_control_RxfifoHfullIntEnUart()       (Uart_Ptr->control.b.RXFIFO_HFULL_INT_EN_UART)
#define Set_Uart_control_TxfifoHfullIntEnUart(value)  (Uart_Ptr->control.b.TXFIFO_HFULL_INT_EN_UART=value)
#define Get_Uart_control_TxfifoHfullIntEnUart()       (Uart_Ptr->control.b.TXFIFO_HFULL_INT_EN_UART)
#define Set_Uart_control_TwoStopBitsUart(value)       (Uart_Ptr->control.b.TWO_STOP_BITS_UART=value)
#define Get_Uart_control_TwoStopBitsUart()            (Uart_Ptr->control.b.TWO_STOP_BITS_UART)
#define Set_Uart_control_ParityEnUart(value)          (Uart_Ptr->control.b.PARITY_EN_UART=value)
#define Get_Uart_control_ParityEnUart()               (Uart_Ptr->control.b.PARITY_EN_UART)
#define Set_Uart_control_SendBreakSeqUart(value)      (Uart_Ptr->control.b.SEND_BREAK_SEQ_UART=value)
#define Get_Uart_control_SendBreakSeqUart()           (Uart_Ptr->control.b.SEND_BREAK_SEQ_UART)
#define Set_Uart_control_LoopbackRwEnUart(value)      (Uart_Ptr->control.b.LOOPBACK_RW_EN_UART=value)
#define Get_Uart_control_LoopbackRwEnUart()           (Uart_Ptr->control.b.LOOPBACK_RW_EN_UART)
#define Set_Uart_control_SetMirrorEnUart(value)       (Uart_Ptr->control.b.SET_MIRROR_EN_UART=value)
#define Get_Uart_control_SetMirrorEnUart()            (Uart_Ptr->control.b.SET_MIRROR_EN_UART)
#define Set_Uart_control_RxfifoRstUart(value)         (Uart_Ptr->control.b.RXFIFO_RST_UART=value)
#define Get_Uart_control_RxfifoRstUart()              (Uart_Ptr->control.b.RXFIFO_RST_UART)
#define Set_Uart_control_TxfifoRstUart(value)         (Uart_Ptr->control.b.TXFIFO_RST_UART=value)
#define Get_Uart_control_TxfifoRstUart()              (Uart_Ptr->control.b.TXFIFO_RST_UART)
#define Set_Uart_control_UartSreset(value)            (Uart_Ptr->control.b.UART_SRESET=value)
#define Get_Uart_control_UartSreset()                 (Uart_Ptr->control.b.UART_SRESET)
#define Set_Uart_control_PerrMask(value)              (Uart_Ptr->control.b.PERR_MASK=value)
#define Get_Uart_control_PerrMask()                   (Uart_Ptr->control.b.PERR_MASK)

#define UART_CONTROL_RTSN_THRESHOLD_SEL_MASK        0x300000
#define UART_CONTROL_TX_HFULL_SEL_UART_MASK         0xc0000
#define UART_CONTROL_RX_HFULL_SEL_UART_MASK         0x30000
#define UART_CONTROL_OVRUN_ERR_INT_EN_UART_MASK     0x1
#define UART_CONTROL_PARITY_ERR_INT_EN_UART_MASK    0x2
#define UART_CONTROL_FRAME_ERR_INT_EN_UART_MASK     0x4
#define UART_CONTROL_BRK_DET_INT_EN_UART_MASK       0x8
#define UART_CONTROL_RX_RDY_INT_EN_UART_MASK        0x10
#define UART_CONTROL_TX_RDY_INT_EN_UART_MASK        0x20
#define UART_CONTROL_TX_EMPTY_INT_EN_UART_MASK      0x40
#define UART_CONTROL_RXFIFO_HFULL_INT_EN_UART_MASK  0x80
#define UART_CONTROL_TXFIFO_HFULL_INT_EN_UART_MASK  0x100
#define UART_CONTROL_TWO_STOP_BITS_UART_MASK        0x200
#define UART_CONTROL_PARITY_EN_UART_MASK            0x400
#define UART_CONTROL_SEND_BREAK_SEQ_UART_MASK       0x800
#define UART_CONTROL_LOOPBACK_RW_EN_UART_MASK       0x1000
#define UART_CONTROL_SET_MIRROR_EN_UART_MASK        0x2000
#define UART_CONTROL_RXFIFO_RST_UART_MASK           0x4000
#define UART_CONTROL_TXFIFO_RST_UART_MASK           0x8000
#define UART_CONTROL_UART_SRESET_MASK               0x40000000
#define UART_CONTROL_PERR_MASK_MASK                 0x80000000
#define UART_CONTROL_RTSN_THRESHOLD_SEL_POS         0x14
#define UART_CONTROL_TX_HFULL_SEL_UART_POS          0x12
#define UART_CONTROL_RX_HFULL_SEL_UART_POS          0x10
#define UART_CONTROL_OVRUN_ERR_INT_EN_UART_POS      0x0
#define UART_CONTROL_PARITY_ERR_INT_EN_UART_POS     0x1
#define UART_CONTROL_FRAME_ERR_INT_EN_UART_POS      0x2
#define UART_CONTROL_BRK_DET_INT_EN_UART_POS        0x3
#define UART_CONTROL_RX_RDY_INT_EN_UART_POS         0x4
#define UART_CONTROL_TX_RDY_INT_EN_UART_POS         0x5
#define UART_CONTROL_TX_EMPTY_INT_EN_UART_POS       0x6
#define UART_CONTROL_RXFIFO_HFULL_INT_EN_UART_POS   0x7
#define UART_CONTROL_TXFIFO_HFULL_INT_EN_UART_POS   0x8
#define UART_CONTROL_TWO_STOP_BITS_UART_POS         0x9
#define UART_CONTROL_PARITY_EN_UART_POS             0xa
#define UART_CONTROL_SEND_BREAK_SEQ_UART_POS        0xb
#define UART_CONTROL_LOOPBACK_RW_EN_UART_POS        0xc
#define UART_CONTROL_SET_MIRROR_EN_UART_POS         0xd
#define UART_CONTROL_RXFIFO_RST_UART_POS            0xe
#define UART_CONTROL_TXFIFO_RST_UART_POS            0xf
#define UART_CONTROL_UART_SRESET_POS                0x1e
#define UART_CONTROL_PERR_MASK_POS                  0x1f


/* -------- Status address 0xB000900C -------- */
#define UART_STATUS_REG_ADDRESS	0xB000900C
#define Set_Uart_status(value)                  (Uart_Ptr->status.all=value)
#define Get_Uart_status()                       (Uart_Ptr->status.all)
#define Get_Uart_status_OvrunErrUart()          (Uart_Ptr->status.b.OVRUN_ERR_UART)
#define Get_Uart_status_ParityErrUart()         (Uart_Ptr->status.b.PARITY_ERR_UART)
#define Get_Uart_status_FrameErrUart()          (Uart_Ptr->status.b.FRAME_ERR_UART)
#define Get_Uart_status_BrkDetUart()            (Uart_Ptr->status.b.BRK_DET_UART)
#define Get_Uart_status_RxReadyUart()           (Uart_Ptr->status.b.RX_READY_UART)
#define Get_Uart_status_TxReadyUart()           (Uart_Ptr->status.b.TX_READY_UART)
#define Get_Uart_status_TxEmptyUart()           (Uart_Ptr->status.b.TX_EMPTY_UART)
#define Get_Uart_status_RxfifoHfullUart()       (Uart_Ptr->status.b.RXFIFO_HFULL_UART)
#define Get_Uart_status_RxfifoFullUart()        (Uart_Ptr->status.b.RXFIFO_FULL_UART)
#define Set_Uart_status_RxToggleUart(value)     (Uart_Ptr->status.b.RX_TOGGLE_UART=value)
#define Get_Uart_status_RxToggleUart()          (Uart_Ptr->status.b.RX_TOGGLE_UART)
#define Get_Uart_status_TxfifoHfullUart()       (Uart_Ptr->status.b.TXFIFO_HFULL_UART)
#define Get_Uart_status_TxfifoFullUart()        (Uart_Ptr->status.b.TXFIFO_FULL_UART)
#define Get_Uart_status_RxfifoEmptyUart()       (Uart_Ptr->status.b.RXFIFO_EMPTY_UART)
#define Get_Uart_status_TxfifoEmptyUart()       (Uart_Ptr->status.b.TXFIFO_EMPTY_UART)
#define Get_Uart_status_UartRtsnThresholdMet()  (Uart_Ptr->status.b.UART_RTSN_THRESHOLD_MET)
#define Get_Uart_status_UartRstnO()             (Uart_Ptr->status.b.UART_RSTN_O)
#define Get_Uart_status_UartCtsnI()             (Uart_Ptr->status.b.UART_CTSN_I)

#define UART_STATUS_OVRUN_ERR_UART_MASK           0x1
#define UART_STATUS_PARITY_ERR_UART_MASK          0x2
#define UART_STATUS_FRAME_ERR_UART_MASK           0x4
#define UART_STATUS_BRK_DET_UART_MASK             0x8
#define UART_STATUS_RX_READY_UART_MASK            0x10
#define UART_STATUS_TX_READY_UART_MASK            0x20
#define UART_STATUS_TX_EMPTY_UART_MASK            0x40
#define UART_STATUS_RXFIFO_HFULL_UART_MASK        0x80
#define UART_STATUS_RXFIFO_FULL_UART_MASK         0x100
#define UART_STATUS_RX_TOGGLE_UART_MASK           0x200
#define UART_STATUS_TXFIFO_HFULL_UART_MASK        0x400
#define UART_STATUS_TXFIFO_FULL_UART_MASK         0x800
#define UART_STATUS_RXFIFO_EMPTY_UART_MASK        0x1000
#define UART_STATUS_TXFIFO_EMPTY_UART_MASK        0x2000
#define UART_STATUS_UART_RTSN_THRESHOLD_MET_MASK  0x40000
#define UART_STATUS_UART_RSTN_O_MASK              0x200000
#define UART_STATUS_UART_CTSN_I_MASK              0x400000
#define UART_STATUS_OVRUN_ERR_UART_POS            0x0
#define UART_STATUS_PARITY_ERR_UART_POS           0x1
#define UART_STATUS_FRAME_ERR_UART_POS            0x2
#define UART_STATUS_BRK_DET_UART_POS              0x3
#define UART_STATUS_RX_READY_UART_POS             0x4
#define UART_STATUS_TX_READY_UART_POS             0x5
#define UART_STATUS_TX_EMPTY_UART_POS             0x6
#define UART_STATUS_RXFIFO_HFULL_UART_POS         0x7
#define UART_STATUS_RXFIFO_FULL_UART_POS          0x8
#define UART_STATUS_RX_TOGGLE_UART_POS            0x9
#define UART_STATUS_TXFIFO_HFULL_UART_POS         0xa
#define UART_STATUS_TXFIFO_FULL_UART_POS          0xb
#define UART_STATUS_RXFIFO_EMPTY_UART_POS         0xc
#define UART_STATUS_TXFIFO_EMPTY_UART_POS         0xd
#define UART_STATUS_UART_RTSN_THRESHOLD_MET_POS   0x12
#define UART_STATUS_UART_RSTN_O_POS               0x15
#define UART_STATUS_UART_CTSN_I_POS               0x16


/* -------- Programmable_Oversampling_Stack_1 address 0xB0009010 -------- */
#define UART_PRGRMMBL_OVERSAMPLING_STACK_1_REG_ADDRESS	0xB0009010
#define Set_Uart_prgrmmblOversamplingStack1(value)                  (Uart_Ptr->prgrmmblOversamplingStack1.all=value)
#define Get_Uart_prgrmmblOversamplingStack1()                       (Uart_Ptr->prgrmmblOversamplingStack1.all)
#define Set_Uart_prgrmmblOversamplingStack1_PossrEnt2Uart(value)    (Uart_Ptr->prgrmmblOversamplingStack1.b.POSSR_ENT_2_UART=value)
#define Get_Uart_prgrmmblOversamplingStack1_PossrEnt2Uart()         (Uart_Ptr->prgrmmblOversamplingStack1.b.POSSR_ENT_2_UART)
#define Set_Uart_prgrmmblOversamplingStack1_PossrEnt1Uart(value)    (Uart_Ptr->prgrmmblOversamplingStack1.b.POSSR_ENT_1_UART=value)
#define Get_Uart_prgrmmblOversamplingStack1_PossrEnt1Uart()         (Uart_Ptr->prgrmmblOversamplingStack1.b.POSSR_ENT_1_UART)
#define Set_Uart_prgrmmblOversamplingStack1_BaudRateDivUart(value)  (Uart_Ptr->prgrmmblOversamplingStack1.b.BAUD_RATE_DIV_UART=value)
#define Get_Uart_prgrmmblOversamplingStack1_BaudRateDivUart()       (Uart_Ptr->prgrmmblOversamplingStack1.b.BAUD_RATE_DIV_UART)

#define UART_PRGRMMBL_OVERSAMPLING_STACK_1_POSSR_ENT_2_UART_MASK  0x3f000000
#define UART_PRGRMMBL_OVERSAMPLING_STACK_1_POSSR_ENT_1_UART_MASK  0x3f0000
#define UART_PRGRMMBL_OVERSAMPLING_STACK_1_BAUD_RATE_DIV_UART_MASK 0x3ff
#define UART_PRGRMMBL_OVERSAMPLING_STACK_1_POSSR_ENT_2_UART_POS   0x18
#define UART_PRGRMMBL_OVERSAMPLING_STACK_1_POSSR_ENT_1_UART_POS   0x10
#define UART_PRGRMMBL_OVERSAMPLING_STACK_1_BAUD_RATE_DIV_UART_POS 0x0


/* -------- Programmable_Oversampling_Stack_2 address 0xB0009020 -------- */
#define UART_PRGRMMBL_OVERSAMPLING_STACK_2_REG_ADDRESS	0xB0009020
#define Set_Uart_prgrmmblOversamplingStack2(value)                (Uart_Ptr->prgrmmblOversamplingStack2.all=value)
#define Get_Uart_prgrmmblOversamplingStack2()                     (Uart_Ptr->prgrmmblOversamplingStack2.all)
#define Set_Uart_prgrmmblOversamplingStack2_PossrEnt4Uart(value)  (Uart_Ptr->prgrmmblOversamplingStack2.b.POSSR_ENT_4_UART=value)
#define Get_Uart_prgrmmblOversamplingStack2_PossrEnt4Uart()       (Uart_Ptr->prgrmmblOversamplingStack2.b.POSSR_ENT_4_UART)
#define Set_Uart_prgrmmblOversamplingStack2_PossrEnt3Uart(value)  (Uart_Ptr->prgrmmblOversamplingStack2.b.POSSR_ENT_3_UART=value)
#define Get_Uart_prgrmmblOversamplingStack2_PossrEnt3Uart()       (Uart_Ptr->prgrmmblOversamplingStack2.b.POSSR_ENT_3_UART)

#define UART_PRGRMMBL_OVERSAMPLING_STACK_2_POSSR_ENT_4_UART_MASK  0x3f00
#define UART_PRGRMMBL_OVERSAMPLING_STACK_2_POSSR_ENT_3_UART_MASK  0x3f
#define UART_PRGRMMBL_OVERSAMPLING_STACK_2_POSSR_ENT_4_UART_POS   0x8
#define UART_PRGRMMBL_OVERSAMPLING_STACK_2_POSSR_ENT_3_UART_POS   0x0


/* -------- Control_2 address 0xB0009038 -------- */
#define UART_CONTROL_2_REG_ADDRESS	0xB0009038
#define Set_Uart_control2(value)                       (Uart_Ptr->control2.all=value)
#define Get_Uart_control2()                            (Uart_Ptr->control2.all)
#define Set_Uart_control2_RxToggleIntEnUart(value)     (Uart_Ptr->control2.b.RX_TOGGLE_INT_EN_UART=value)
#define Get_Uart_control2_RxToggleIntEnUart()          (Uart_Ptr->control2.b.RX_TOGGLE_INT_EN_UART)
#define Set_Uart_control2_TxfifoFullIntEnUart(value)   (Uart_Ptr->control2.b.TXFIFO_FULL_INT_EN_UART=value)
#define Get_Uart_control2_TxfifoFullIntEnUart()        (Uart_Ptr->control2.b.TXFIFO_FULL_INT_EN_UART)
#define Set_Uart_control2_RxfifoFullIntEnUart(value)   (Uart_Ptr->control2.b.RXFIFO_FULL_INT_EN_UART=value)
#define Get_Uart_control2_RxfifoFullIntEnUart()        (Uart_Ptr->control2.b.RXFIFO_FULL_INT_EN_UART)
#define Set_Uart_control2_TxfifoEmptyIntEnUart(value)  (Uart_Ptr->control2.b.TXFIFO_EMPTY_INT_EN_UART=value)
#define Get_Uart_control2_TxfifoEmptyIntEnUart()       (Uart_Ptr->control2.b.TXFIFO_EMPTY_INT_EN_UART)
#define Set_Uart_control2_RxfifoEmptyIntEnUart(value)  (Uart_Ptr->control2.b.RXFIFO_EMPTY_INT_EN_UART=value)
#define Get_Uart_control2_RxfifoEmptyIntEnUart()       (Uart_Ptr->control2.b.RXFIFO_EMPTY_INT_EN_UART)
#define Set_Uart_control2_RtsnThresholdIntrEn(value)   (Uart_Ptr->control2.b.RTSN_THRESHOLD_INTR_EN=value)
#define Get_Uart_control2_RtsnThresholdIntrEn()        (Uart_Ptr->control2.b.RTSN_THRESHOLD_INTR_EN)
#define Set_Uart_control2_UartRtsnIntrEn(value)        (Uart_Ptr->control2.b.UART_RTSN_INTR_EN=value)
#define Get_Uart_control2_UartRtsnIntrEn()             (Uart_Ptr->control2.b.UART_RTSN_INTR_EN)
#define Set_Uart_control2_UartCtsnIntrEn(value)        (Uart_Ptr->control2.b.UART_CTSN_INTR_EN=value)
#define Get_Uart_control2_UartCtsnIntrEn()             (Uart_Ptr->control2.b.UART_CTSN_INTR_EN)
#define Set_Uart_control2_UartCtsnBlock(value)         (Uart_Ptr->control2.b.UART_CTSN_BLOCK=value)
#define Get_Uart_control2_UartCtsnBlock()              (Uart_Ptr->control2.b.UART_CTSN_BLOCK)
#define Set_Uart_control2_UartRtsnBlock(value)         (Uart_Ptr->control2.b.UART_RTSN_BLOCK=value)
#define Get_Uart_control2_UartRtsnBlock()              (Uart_Ptr->control2.b.UART_RTSN_BLOCK)

#define UART_CONTROL_2_RX_TOGGLE_INT_EN_UART_MASK     0x1
#define UART_CONTROL_2_TXFIFO_FULL_INT_EN_UART_MASK   0x2
#define UART_CONTROL_2_RXFIFO_FULL_INT_EN_UART_MASK   0x4
#define UART_CONTROL_2_TXFIFO_EMPTY_INT_EN_UART_MASK  0x8
#define UART_CONTROL_2_RXFIFO_EMPTY_INT_EN_UART_MASK  0x10
#define UART_CONTROL_2_RTSN_THRESHOLD_INTR_EN_MASK    0x200
#define UART_CONTROL_2_UART_RTSN_INTR_EN_MASK         0x1000
#define UART_CONTROL_2_UART_CTSN_INTR_EN_MASK         0x2000
#define UART_CONTROL_2_UART_CTSN_BLOCK_MASK           0x4000
#define UART_CONTROL_2_UART_RTSN_BLOCK_MASK           0x10000
#define UART_CONTROL_2_RX_TOGGLE_INT_EN_UART_POS      0x0
#define UART_CONTROL_2_TXFIFO_FULL_INT_EN_UART_POS    0x1
#define UART_CONTROL_2_RXFIFO_FULL_INT_EN_UART_POS    0x2
#define UART_CONTROL_2_TXFIFO_EMPTY_INT_EN_UART_POS   0x3
#define UART_CONTROL_2_RXFIFO_EMPTY_INT_EN_UART_POS   0x4
#define UART_CONTROL_2_RTSN_THRESHOLD_INTR_EN_POS     0x9
#define UART_CONTROL_2_UART_RTSN_INTR_EN_POS          0xc
#define UART_CONTROL_2_UART_CTSN_INTR_EN_POS          0xd
#define UART_CONTROL_2_UART_CTSN_BLOCK_POS            0xe
#define UART_CONTROL_2_UART_RTSN_BLOCK_POS            0x10


/* -------- Status_2 address 0xB000903C -------- */
#define UART_STATUS_2_REG_ADDRESS	0xB000903C
#define Get_Uart_status2()                (Uart_Ptr->status2.all)
#define Get_Uart_status2_TxfifoByteCnt()  (Uart_Ptr->status2.b.TXFIFO_BYTE_CNT)
#define Get_Uart_status2_RxfifoByteCnt()  (Uart_Ptr->status2.b.RXFIFO_BYTE_CNT)

#define UART_STATUS_2_TXFIFO_BYTE_CNT_MASK  0xff0000
#define UART_STATUS_2_RXFIFO_BYTE_CNT_MASK  0xff
#define UART_STATUS_2_TXFIFO_BYTE_CNT_POS   0x10
#define UART_STATUS_2_RXFIFO_BYTE_CNT_POS   0x0


