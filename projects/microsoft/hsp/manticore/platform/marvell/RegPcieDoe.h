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
//! @brief PCIE_DOE Registers
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


/// @brief 0x1000
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EXT_CAP_ID                  :16;     ///<BIT [15:0] ext_cap_id
        uint32_t EXT_CAP_VER                 :4;      ///<BIT [19:16] ext_cap_ver
        uint32_t RSVD_20_23                  :4;      ///<BIT [23:20] rsvd_20_23
        uint32_t NXT_CAP_OFFSET              :8;      ///<BIT [31:24] nxt_cap_offset
    } b;
} PcieExtentedCapabilityHeader_t;

/// @brief 0x1004
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MSI_SUPPORT                 :1;      ///<BIT [0] msi_support
        uint32_t MSI_NUM                     :11;     ///<BIT [11:1] msi_num
        uint32_t RSVD_12_13                  :2;      ///<BIT [13:12] rsvd_12_13
        uint32_t RESERVED                    :18;     ///<BIT [31:14] Reserved
    } b;
} DoeCapability_t;

/// @brief 0x1008
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DOE_ABORT                   :1;      ///<BIT [0] doe_abort
        uint32_t DOE_INT_EN                  :1;      ///<BIT [1] doe_int_en
        uint32_t RSVD_2_3                    :2;      ///<BIT [3:2] rsvd_2_3
        uint32_t RESERVED                    :27;     ///<BIT [30:4] Reserved
        uint32_t DOE_GO                      :1;      ///<BIT [31] doe_go
    } b;
} DoeControl_t;

/// @brief 0x100C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DOE_STATUS_BUSY             :1;      ///<BIT [0] doe_status_busy
        uint32_t DOE_STATUS_INT              :1;      ///<BIT [1] doe_status_int
        uint32_t DOE_STATUS_ERR              :1;      ///<BIT [2] doe_status_err
        uint32_t RSVD_3_4                    :2;      ///<BIT [4:3] rsvd_3_4
        uint32_t RESERVED                    :26;     ///<BIT [30:5] Reserved
        uint32_t DOE_STATUS_READY            :1;      ///<BIT [31] doe_status_ready
    } b;
} DoeStatus_t;

/// @brief 0x1018
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TO_THRESHOLD                :4;      ///<BIT [3:0] to_threshold
        uint32_t RSVD_4_7                    :4;      ///<BIT [7:4] rsvd_4_7
        uint32_t ABORT_BUSY_EN               :1;      ///<BIT [8] abort_busy_en
        uint32_t RX_RDY_BUSY_EN              :1;      ///<BIT [9] rx_rdy_busy_en
        uint32_t RX_ECC_ERR_BUSY_EN          :1;      ///<BIT [10] rx_ecc_err_busy_en
        uint32_t ECC_ERR_EN                  :1;      ///<BIT [11] ecc_err_en
        uint32_t ELBI_TO_ERR_EN              :1;      ///<BIT [12] elbi_to_err_en
        uint32_t ABORT_READY_EN              :1;      ///<BIT [13] abort_ready_en
        uint32_t TX_RDY_READY_EN             :1;      ///<BIT [14] tx_rdy_ready_en
        uint32_t TX_ECC_READY_EN             :1;      ///<BIT [15] tx_ecc_ready_en
        uint32_t RXFIFO_ECC_PSLVERR_EN       :1;      ///<BIT [16] rxfifo_ecc_pslverr_en
        uint32_t RXFIFO_ECC_INT_EN           :1;      ///<BIT [17] rxfifo_ecc_int_en
        uint32_t RX_UNDERFLOW_PSLVERR_EN     :1;      ///<BIT [18] rx_underflow_pslverr_en
        uint32_t RX_UNDERFLOW_INT_EN         :1;      ///<BIT [19] rx_underflow_int_en
        uint32_t TX_OVERFLOW_PSLVERR_EN      :1;      ///<BIT [20] tx_overflow_pslverr_en
        uint32_t TX_OVERFLOW_INT_EN          :1;      ///<BIT [21] tx_overflow_int_en
        uint32_t TX_ERR_HOLD_EBLI_EN         :1;      ///<BIT [22] tx_err_hold_ebli_en
        uint32_t ABORT_ERR_EN                :1;      ///<BIT [23] abort_err_en
        uint32_t GO_BUSY_EN                  :1;      ///<BIT [24] go_busy_en
        uint32_t TX_DONE_READY_EN            :1;      ///<BIT [25] tx_done_ready_en
        uint32_t ELBI_TO_ACK_EN              :1;      ///<BIT [26] elbi_to_ack_en
        uint32_t RESERVED                    :4;      ///<BIT [30:27] Reserved
        uint32_t LAST_DW                     :1;      ///<BIT [31] last_dw
    } b;
} DoeCfg1_t;

/// @brief 0x101C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DOE_RX_FIFO_WTSEL           :2;      ///<BIT [1:0] doe_rx_fifo_wtsel
        uint32_t DOE_RX_FIFO_RTSEL           :2;      ///<BIT [3:2] doe_rx_fifo_rtsel
        uint32_t DOE_RX_FIFO_MTSEL           :2;      ///<BIT [5:4] doe_rx_fifo_mtsel
        uint32_t RESERVED1                   :2;      ///<BIT [7:6] Reserved1
        uint32_t DOE_TX_FIFO_WTSEL           :2;      ///<BIT [9:8] doe_tx_fifo_wtsel
        uint32_t DOE_TX_FIFO_RTSEL           :2;      ///<BIT [11:10] doe_tx_fifo_rtsel
        uint32_t DOE_TX_FIFO_MTSEL           :2;      ///<BIT [13:12] doe_tx_fifo_mtsel
        uint32_t RSVD_14_15                  :2;      ///<BIT [15:14] rsvd_14_15
        uint32_t RX_FIFO_CNT                 :10;     ///<BIT [25:16] rx_fifo_cnt
        uint32_t RESERVED0                   :2;      ///<BIT [27:26] Reserved0
        uint32_t RX_ECC_EN                   :1;      ///<BIT [28] rx_ecc_en
        uint32_t TX_ECC_EN                   :1;      ///<BIT [29] tx_ecc_en
        uint32_t RXFIFO_RAN_EN               :1;      ///<BIT [30] rxfifo_ran_en
        uint32_t TXFIFO_RAN_EN               :1;      ///<BIT [31] txfifo_ran_en
    } b;
} DoeCfg2_t;

/// @brief 0x1020
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ELBI_TO_INT_EN              :1;      ///<BIT [0] elbi_to_int_en
        uint32_t RX_RDY_INT_EN               :1;      ///<BIT [1] rx_rdy_int_en
        uint32_t TX_RDY_INT_EN               :1;      ///<BIT [2] tx_rdy_int_en
        uint32_t DOE_GO_INT_EN               :1;      ///<BIT [3] doe_go_int_en
        uint32_t DOE_ABORT_INT_EN            :1;      ///<BIT [4] doe_abort_int_en
        uint32_t ECC_ERR_RXFIFO_INT_EN       :1;      ///<BIT [5] ecc_err_rxfifo_int_en
        uint32_t ECC_ERR_TXFIFO_INT_EN       :1;      ///<BIT [6] ecc_err_txfifo_int_en
        uint32_t TX_DONE_INT_EN              :1;      ///<BIT [7] tx_done_int_en
        uint32_t RXFIFO_UNDERFLOW_INT_EN     :1;      ///<BIT [8] rxfifo_underflow_int_en
        uint32_t TXFIFO_OVERFLOW_INT_EN      :1;      ///<BIT [9] txfifo_overflow_int_en
        uint32_t POISON_CFGWR_INT_EN         :1;      ///<BIT [10] poison_cfgwr_int_en
        uint32_t RESERVED1                   :21;     ///<BIT [31:11] Reserved1
    } b;
} DoeIntrEn_t;

/// @brief 0x1024
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ELBI_TO_INT                 :1;      ///<BIT [0] elbi_to_int
        uint32_t RX_RDY_INT                  :1;      ///<BIT [1] rx_rdy_int
        uint32_t TX_RDY_INT                  :1;      ///<BIT [2] tx_rdy_int
        uint32_t DOE_GO_INT                  :1;      ///<BIT [3] doe_go_int
        uint32_t DOE_ABORT_INT               :1;      ///<BIT [4] doe_abort_int
        uint32_t ECC_ERR_RXFIFO              :1;      ///<BIT [5] ecc_err_rxfifo
        uint32_t ECC_ERR_TXFIFO              :1;      ///<BIT [6] ecc_err_txfifo
        uint32_t TX_DONE_INT                 :1;      ///<BIT [7] tx_done_int
        uint32_t RX_UNDERFLOW_INT            :1;      ///<BIT [8] rx_underflow_int
        uint32_t TX_OVERFLOW_INT             :1;      ///<BIT [9] tx_overflow_int
        uint32_t POISON_CFGWR_INT            :1;      ///<BIT [10] poison_cfgwr_int
        uint32_t RESERVED1                   :21;     ///<BIT [31:11] Reserved1
    } b;
} DoeIntrStatus_t;

/// @brief 0x1040
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DOE_FW_RST_N                :1;      ///<BIT [0] doe_fw_rst_n
        uint32_t DOE_RST_DIS                 :1;      ///<BIT [1] doe_rst_dis
        uint32_t DOE_MON_SEL                 :2;      ///<BIT [3:2] doe_mon_sel
        uint32_t RESERVED1                   :4;      ///<BIT [7:4] Reserved1
        uint32_t ECC_OUT_INJ_RXFIFO          :2;      ///<BIT [9:8] ecc_out_inj_rxfifo
        uint32_t ECC_OUT_INJ_TXFIFO          :2;      ///<BIT [11:10] ecc_out_inj_txfifo
        uint32_t RESERVED0                   :20;     ///<BIT [31:12] Reserved0
    } b;
} DoeCfg3_t;

/// @brief 0x1044
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RX_FIFO_WR_PTR              :10;     ///<BIT [9:0] RX_FIFO_WR_PTR
        uint32_t RESERVED1                   :6;      ///<BIT [15:10] Reserved1
        uint32_t RX_FIFO_RD_PTR              :10;     ///<BIT [25:16] RX_FIFO_RD_PTR
        uint32_t RESERVED0                   :6;      ///<BIT [31:26] Reserved0
    } b;
} RxFifoPointer_t;

/// @brief 0x1048
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_FIFO_WR_PTR              :10;     ///<BIT [9:0] TX_FIFO_WR_PTR
        uint32_t RESERVED1                   :6;      ///<BIT [15:10] Reserved1
        uint32_t TX_FIFO_RD_PTR              :10;     ///<BIT [25:16] TX_FIFO_RD_PTR
        uint32_t RESERVED0                   :6;      ///<BIT [31:26] Reserved0
    } b;
} TxFifoPointer_t;

/// @brief 0x1050
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TX_UNDERFLOW                :1;      ///<BIT [0] TX_UNDERFLOW
        uint32_t TX_OVERFLOW                 :1;      ///<BIT [1] TX_OVERFLOW
        uint32_t TX_RDY                      :1;      ///<BIT [2] TX_RDY
        uint32_t TX_MTY                      :1;      ///<BIT [3] TX_MTY
        uint32_t TX_FULL                     :1;      ///<BIT [4] TX_FULL
        uint32_t RESERVED1                   :11;     ///<BIT [15:5] Reserved1
        uint32_t RX_UNDERFLOW                :1;      ///<BIT [16] RX_UNDERFLOW
        uint32_t RX_OVERFLOW                 :1;      ///<BIT [17] RX_OVERFLOW
        uint32_t RX_RDY                      :1;      ///<BIT [18] RX_RDY
        uint32_t RX_MTY                      :1;      ///<BIT [19] RX_MTY
        uint32_t RX_FULL                     :1;      ///<BIT [20] RX_FULL
        uint32_t RESERVED0                   :11;     ///<BIT [31:21] Reserved0
    } b;
} FifoSts_t;

typedef struct
{
    uint8_t rsvd0[4096];                                                    // 0x0 : rsvd_0 / rsvd_0
    PcieExtentedCapabilityHeader_t pcieExtentedCapabilityHeader;            // 0x1000 : PCIe_Extented_Capability_Header / 
    DoeCapability_t doeCapability;                                          // 0x1004 : DOE_Capability / 
    DoeControl_t doeControl;                                                // 0x1008 : DOE_Control / 
    DoeStatus_t doeStatus;                                                  // 0x100C : DOE_Status / 
    uint32_t doeWriteDataMailboxDoeWrMbReg;                                 // 0x1010 : DOE_Write_Data_Mailbox / 
    uint32_t doeReadDataMailboxDoeRdMbReg;                                  // 0x1014 : DOE_Read_Data_Mailbox / 
    DoeCfg1_t doeCfg1;                                                      // 0x1018 : DOE_Configuration_1 / 
    DoeCfg2_t doeCfg2;                                                      // 0x101C : DOE_Configuration_2 / 
    DoeIntrEn_t doeIntrEn;                                                  // 0x1020 : DOE_INTERRUPT_EN / 
    DoeIntrStatus_t doeIntrStatus;                                          // 0x1024 : DOE_INTERRUPT_STATUS / 
    uint32_t rxFifoCorrectableErrCnt;                                       // 0x1028 : RX_FIFO_CORRECTABLE_ERR_CNT / 
    uint32_t rxFifoUncorrectableErrCnt;                                     // 0x102C : RX_FIFO_UNCORRECTABLE_ERR_CNT / 
    uint32_t txFifoCorrectableErrCnt;                                       // 0x1030 : TX_FIFO_CORRECTABLE_ERR_CNT / 
    uint32_t txFifoUncorrectableErrCnt;                                     // 0x1034 : TX_FIFO_UNCORRECTABLE_ERR_CNT / 
    uint32_t rxFifoRdData;                                                  // 0x1038 : RX_FIFO_RD_DATA / 
    uint32_t txFifoWrData;                                                  // 0x103C : TX_FIFO_WR_DATA / 
    DoeCfg3_t doeCfg3;                                                      // 0x1040 : DOE_Configuration_3 / 
    RxFifoPointer_t rxFifoPointer;                                          // 0x1044 : RX_FIFO_POINTER / 
    TxFifoPointer_t txFifoPointer;                                          // 0x1048 : TX_FIFO_POINTER / 
    uint8_t rsvd104c[4];                                                    // 0x104C : rsvd_104c / rsvd_104c
    FifoSts_t fifoSts;                                                      // 0x1050 : FIFO_STS / 
} PcieDoe_t;

COMPILE_ASSERT(offsetof(PcieDoe_t,pcieExtentedCapabilityHeader)==0x1000,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,doeCapability)==0x1004,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,doeControl)==0x1008,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,doeStatus)==0x100C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,doeWriteDataMailboxDoeWrMbReg)==0x1010,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,doeReadDataMailboxDoeRdMbReg)==0x1014,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,doeCfg1)==0x1018,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,doeCfg2)==0x101C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,doeIntrEn)==0x1020,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,doeIntrStatus)==0x1024,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,rxFifoCorrectableErrCnt)==0x1028,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,rxFifoUncorrectableErrCnt)==0x102C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,txFifoCorrectableErrCnt)==0x1030,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,txFifoUncorrectableErrCnt)==0x1034,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,rxFifoRdData)==0x1038,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,txFifoWrData)==0x103C,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,doeCfg3)==0x1040,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,rxFifoPointer)==0x1044,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,txFifoPointer)==0x1048,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieDoe_t,fifoSts)==0x1050,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile PcieDoe_t rPcieDoe; ///< 0xB01D0000
