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
//! @brief PCIE_IDE_AES Registers
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


/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t KEY_DONE                    :1;      ///<BIT [0] KEY_DONE
        uint32_t RSVD_1_3                    :3;      ///<BIT [3:1] rsvd_1_3
        uint32_t CTX_IDX_ERR                 :1;      ///<BIT [4] CTX_IDX_ERR
        uint32_t RSVD_5_15                   :11;     ///<BIT [15:5] rsvd_5_15
        uint32_t REG_PAR_ERR                 :1;      ///<BIT [16] REG_PAR_ERR
        uint32_t FSM_PAR_ERR                 :1;      ///<BIT [17] FSM_PAR_ERR
        uint32_t RSVD_18_31                  :14;     ///<BIT [31:18] rsvd_18_31
    } b;
} IrqStat1_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t VERSION_NUM                 :16;     ///<BIT [15:0] VERSION_NUM
        uint32_t RESERVED_31_16              :16;     ///<BIT [31:16] RESERVED_31_16
    } b;
} PcieIdeAesCoreVerNum_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TYPE_NUM                    :8;      ///<BIT [7:0] TYPE_NUM
        uint32_t PKG_NUM                     :4;      ///<BIT [11:8] PKG_NUM
        uint32_t TYPE_ENUM                   :4;      ///<BIT [15:12] TYPE_ENUM
        uint32_t RESERVED_31_16              :16;     ///<BIT [31:16] RESERVED_31_16
    } b;
} PcieIdeAesCoreVerType_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t KEY_DONE                    :1;      ///<BIT [0] KEY_DONE
        uint32_t RSVD_1                      :1;      ///<BIT [1] rsvd_1
        uint32_t RESERVED_3_2                :2;      ///<BIT [3:2] RESERVED_3_2
        uint32_t CTX_IDX_ERR                 :1;      ///<BIT [4] CTX_IDX_ERR
        uint32_t RESERVED_15_5               :11;     ///<BIT [15:5] RESERVED_15_5
        uint32_t REG_PAR_ERR                 :1;      ///<BIT [16] REG_PAR_ERR
        uint32_t FSM_PAR_ERR                 :1;      ///<BIT [17] FSM_PAR_ERR
        uint32_t RESERVED_31_18              :13;     ///<BIT [30:18] RESERVED_31_18
        uint32_t OUTPUT                      :1;      ///<BIT [31] OUTPUT
    } b;
} PcieIdeAesIrqEn_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t KEY_DONE                    :1;      ///<BIT [0] KEY_DONE
        uint32_t RSVD_1_3                    :3;      ///<BIT [3:1] rsvd_1_3
        uint32_t CTX_IDX_ERR                 :2;      ///<BIT [5:4] CTX_IDX_ERR
        uint32_t RSVD_6_15                   :10;     ///<BIT [15:6] rsvd_6_15
        uint32_t REG_PAR_ERR                 :1;      ///<BIT [16] REG_PAR_ERR
        uint32_t FSM_PAR_ERR                 :2;      ///<BIT [18:17] FSM_PAR_ERR
        uint32_t RSVD_19_31                  :13;     ///<BIT [31:19] rsvd_19_31
    } b;
} IrqStat2_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CTX_IDX                     :4;      ///<BIT [3:0] CTX_IDX
        uint32_t RSVD_4_15                   :12;     ///<BIT [15:4] rsvd_4_15
        uint32_t KEY_SZ                      :1;      ///<BIT [16] KEY_SZ
        uint32_t RESERVED_17                 :1;      ///<BIT [17] RESERVED_17
        uint32_t ENCRYPT                     :1;      ///<BIT [18] ENCRYPT
        uint32_t RSVD_19                     :1;      ///<BIT [19] rsvd_19
        uint32_t RESERVED_31_20              :12;     ///<BIT [31:20] RESERVED_31_20
    } b;
} PcieIdeAesCtrl_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BUSY                        :1;      ///<BIT [0] BUSY
        uint32_t RSVD_1                      :1;      ///<BIT [1] rsvd_1
        uint32_t RESERVED_31_2               :30;     ///<BIT [31:2] RESERVED_31_2
    } b;
} PcieIdeAesStat_t;

/// @brief 0x1C0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DISABLE_KEY_S0              :1;      ///<BIT [0] DISABLE_KEY_S0
        uint32_t DISABLE_KEY_S1              :1;      ///<BIT [1] DISABLE_KEY_S1
        uint32_t RSVD_2_23                   :22;     ///<BIT [23:2] rsvd_2_23
        uint32_t RESERVED_31_24              :8;      ///<BIT [31:24] RESERVED_31_24
    } b;
} PcieIdeAesDisableStreamKeys_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMD_SOP                     :1;      ///<BIT [0] CMD_SOP
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesCmdSop_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMD_EOP                     :1;      ///<BIT [0] CMD_EOP
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesCmdEop_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMD_AAD                     :1;      ///<BIT [0] CMD_AAD
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesCmdAad_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMD_MSG                     :1;      ///<BIT [0] CMD_MSG
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesCmdMsg_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CTR_IV_IDX                  :2;      ///<BIT [1:0] CTR_IV_IDX
        uint32_t RSVD_2_31                   :30;     ///<BIT [31:2] rsvd_2_31
    } b;
} PcieIdeAesCtrIvIdx_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMD_AAD_SIZE                :6;      ///<BIT [5:0] CMD_AAD_SIZE
        uint32_t RSVD_6_31                   :26;     ///<BIT [31:6] rsvd_6_31
    } b;
} PcieIdeAesCmdAadSize_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMD_MSG_SIZE                :7;      ///<BIT [6:0] CMD_MSG_SIZE
        uint32_t RSVD_7_31                   :25;     ///<BIT [31:7] rsvd_7_31
    } b;
} PcieIdeAesCmdMsgSize_t;

/// @brief 0x2C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMD_CTX                     :4;      ///<BIT [3:0] CMD_CTX
        uint32_t RESERVED                    :28;     ///<BIT [31:4] RESERVED
    } b;
} PcieIdeAesCmdCtx_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMD_VALID                   :1;      ///<BIT [0] CMD_VALID
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesCmdValid_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CMD_READY                   :1;      ///<BIT [0] CMD_READY
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesCmdReady_t;

/// @brief 0x4C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TEST_MODE_CFG               :2;      ///<BIT [1:0] TEST_MODE_CFG
        uint32_t RESERVED_31_2               :30;     ///<BIT [31:2] RESERVED_31_2
    } b;
} PcieIdeAesTestModeCfg_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t IN_DATA_IDX                 :5;      ///<BIT [4:0] IN_DATA_IDX
        uint32_t RSVD_5_31                   :27;     ///<BIT [31:5] rsvd_5_31
    } b;
} PcieIdeAesInDataIdx_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DATA_VALID                  :1;      ///<BIT [0] DATA_VALID
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesDataValid_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DATA_READY                  :1;      ///<BIT [0] DATA_READY
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesDataReady_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OUT_READY                   :1;      ///<BIT [0] OUT_READY
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesOutReady_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OUT_VALID                   :1;      ///<BIT [0] OUT_VALID
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesOutValid_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OUT_SOP                     :1;      ///<BIT [0] OUT_SOP
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesOutSop_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OUT_EOP                     :1;      ///<BIT [0] OUT_EOP
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesOutEop_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OUT_AAD                     :1;      ///<BIT [0] OUT_AAD
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesOutAad_t;

/// @brief 0x18
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OUT_AAD_SIZE                :6;      ///<BIT [5:0] OUT_AAD_SIZE
        uint32_t RESERVED                    :26;     ///<BIT [31:6] RESERVED
    } b;
} PcieIdeAesOutAadSize_t;

/// @brief 0x1C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OUT_MSG                     :1;      ///<BIT [0] OUT_MSG
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesOutMsg_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OUT_MSG_SIZE                :7;      ///<BIT [6:0] OUT_MSG_SIZE
        uint32_t RESERVED                    :25;     ///<BIT [31:7] RESERVED
    } b;
} PcieIdeAesOutMsgSize_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OUT_DATA_IDX                :5;      ///<BIT [4:0] OUT_DATA_IDX
        uint32_t RSVD_5_31                   :27;     ///<BIT [31:5] rsvd_5_31
    } b;
} PcieIdeAesOutDataIdx_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t OUT_MAC_IDX                 :2;      ///<BIT [1:0] OUT_MAC_IDX
        uint32_t RESERVED_31_2               :30;     ///<BIT [31:2] RESERVED_31_2
    } b;
} PcieIdeAesOutMacIdx_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AES_SEL                     :1;      ///<BIT [0] AES_SEL
        uint32_t RESERVED_31_1               :31;     ///<BIT [31:1] RESERVED_31_1
    } b;
} PcieIdeAesAesSel_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BIST_START                  :1;      ///<BIT [0] BIST_START
        uint32_t BIST_TEST_MODE              :4;      ///<BIT [4:1] BIST_TEST_MODE
        uint32_t RESERVED_27_5               :27;     ///<BIT [31:5] RESERVED_27_5
    } b;
} BistCtrl_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BIST_DONE                   :1;      ///<BIT [0] BIST_DONE
        uint32_t BIST_FAIL                   :1;      ///<BIT [1] BIST_FAIL
        uint32_t BIST_ERR_TEST_TYPE_TX       :3;      ///<BIT [4:2] BIST_ERR_TEST_TYPE_TX
        uint32_t BIST_ERR_TEST_TYPE_RX1      :3;      ///<BIT [7:5] BIST_ERR_TEST_TYPE_RX1
        uint32_t BIST_ERR_TEST_TYPE_RX2      :3;      ///<BIT [10:8] BIST_ERR_TEST_TYPE_RX2
        uint32_t ERR_CORE                    :3;      ///<BIT [13:11] ERR_CORE
        uint32_t RESERVED_18_14              :18;     ///<BIT [31:14] RESERVED_18_14
    } b;
} BistStatus_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BIST_LOG_FAIL               :1;      ///<BIT [0] BIST_LOG_FAIL
        uint32_t BIST_LOG_TEST_TYPE_TX       :3;      ///<BIT [3:1] BIST_LOG_TEST_TYPE_TX
        uint32_t BIST_LOG_TEST_TYPE_RX1      :3;      ///<BIT [6:4] BIST_LOG_TEST_TYPE_RX1
        uint32_t BIST_LOG_TEST_TYPE_RX2      :3;      ///<BIT [9:7] BIST_LOG_TEST_TYPE_RX2
        uint32_t BIST_LOG_CORE               :3;      ///<BIT [12:10] BIST_LOG_CORE
        uint32_t RESERVED_19_13              :19;     ///<BIT [31:13] RESERVED_19_13
    } b;
} BistErrorLog_t;

/// @brief 0xE00
typedef struct
{
    BistCtrl_t bistCtrl;                  //BIST_CTRL
    BistStatus_t bistStatus;              //BIST_STATUS
    uint32_t bistTimestampTx031;          //BIST_TIMESTAMP_TX_0_31
    uint32_t bistTimestampTx3263;         //BIST_TIMESTAMP_TX_32_63
    uint32_t bistTimestampRx1031;         //BIST_TIMESTAMP_RX1_0_31
    uint32_t bistTimestampRx13263;        //BIST_TIMESTAMP_RX1_32_63
    uint32_t bistTimestampRx2031;         //BIST_TIMESTAMP_RX2_0_31
    uint32_t bistTimestampRx23263;        //BIST_TIMESTAMP_RX2_32_63
    BistErrorLog_t bistErrorLog;          //BIST_ERROR_LOG
    uint32_t bistLogTimestampTx031;       //BIST_LOG_TIMESTAMP_TX_0_31
    uint32_t bistLogTimestampTx3263;      //BIST_LOG_TIMESTAMP_TX_32_63
    uint32_t bistLogTimestampRx1031;      //BIST_LOG_TIMESTAMP_RX1_0_31
    uint32_t bistLogTimestampRx13263;     //BIST_LOG_TIMESTAMP_RX1_32_63
    uint32_t bistLogTimestampRx2031;      //BIST_LOG_TIMESTAMP_RX2_0_31
    uint32_t bistLogTimestampRx23263;     //BIST_LOG_TIMESTAMP_RX2_32_63
} IdBist_t;

/// @brief 0xDFC
typedef struct
{
    PcieIdeAesAesSel_t aesSel;            //AES_SEL
} IdAesRxFipsSel_t;

/// @brief 0xD00
typedef struct
{
    PcieIdeAesOutReady_t outReady;        //OUT_READY
    PcieIdeAesOutValid_t outValid;        //OUT_VALID
    PcieIdeAesOutSop_t outSop;            //OUT_SOP
    PcieIdeAesOutEop_t outEop;            //OUT_EOP
    uint8_t rsvd10[4];                    //rsvd_10
    PcieIdeAesOutAad_t outAad;            //OUT_AAD
    PcieIdeAesOutAadSize_t outAadSize;    //OUT_AAD_SIZE
    PcieIdeAesOutMsg_t outMsg;            //OUT_MSG
    PcieIdeAesOutMsgSize_t outMsgSize;    //OUT_MSG_SIZE
    PcieIdeAesOutDataIdx_t outDataIdx;    //OUT_DATA_IDX
    uint32_t outDataChunk;                //OUT_DATA_CHUNK
    uint8_t rsvd2c[4];                    //rsvd_2c
    PcieIdeAesOutMacIdx_t outMacIdx;      //OUT_MAC_IDX
    uint32_t outMacChunk;                 //OUT_MAC_CHUNK
} IdAesRxFipsOutput_t;

/// @brief 0xC50
typedef struct
{
    PcieIdeAesInDataIdx_t inDataIdx;      //IN_DATA_IDX
    uint32_t inData;                      //IN_DATA
    PcieIdeAesDataValid_t dataValid;      //DATA_VALID
    PcieIdeAesDataReady_t dataReady;      //DATA_READY
} IdAesRxFipsInput_t;

/// @brief 0xC00
typedef struct
{
    PcieIdeAesCmdSop_t cmdSop;            //CMD_SOP
    PcieIdeAesCmdEop_t cmdEop;            //CMD_EOP
    PcieIdeAesCmdAad_t cmdAad;            //CMD_AAD
    PcieIdeAesCmdMsg_t cmdMsg;            //CMD_MSG
    PcieIdeAesCtrIvIdx_t ctrIvIdx;        //CTR_IV_IDX
    uint32_t ctrIvChunk;                  //CTR_IV_CHUNK
    uint8_t rsvd18[12];                   //rsvd_18
    PcieIdeAesCmdAadSize_t cmdAadSize;    //CMD_AAD_SIZE
    PcieIdeAesCmdMsgSize_t cmdMsgSize;    //CMD_MSG_SIZE
    PcieIdeAesCmdCtx_t cmdCtx;            //CMD_CTX
    PcieIdeAesCmdValid_t cmdValid;        //CMD_VALID
    PcieIdeAesCmdReady_t cmdReady;        //CMD_READY
    uint8_t rsvd38[20];                   //rsvd_38
    PcieIdeAesTestModeCfg_t testModeCfg;  //TEST_MODE_CFG
} IdAesRxFipsCommand_t;

/// @brief 0x800
typedef struct
{
    PcieIdeAesCoreVerNum_t coreVerNum;    //CORE_VER_NUM
    PcieIdeAesCoreVerType_t coreVerType;  //CORE_VER_TYPE
    PcieIdeAesIrqEn_t irqEn;              //IRQ_EN
    IrqStat2_t irqStat2;                  //IRQ_STAT
    uint8_t rsvd10[4];                    //rsvd_10
    PcieIdeAesCtrl_t ctrl;                //CTRL
    PcieIdeAesStat_t stat;                //STAT
    uint8_t rsvd1c[4];                    //rsvd_1c
    uint32_t key0Key;                     //KEY_0
    uint32_t key1Key;                     //KEY_1
    uint32_t key2Key;                     //KEY_2
    uint32_t key3Key;                     //KEY_3
    uint32_t key4Key;                     //KEY_4
    uint32_t key5Key;                     //KEY_5
    uint32_t key6Key;                     //KEY_6
    uint32_t key7Key;                     //KEY_7
    uint32_t initialIvLsw;                //INITIAL_IV_LSW
    uint32_t initialIvMsw;                //INITIAL_IV_MSW
    uint8_t rsvd48[376];                  //rsvd_48
    PcieIdeAesDisableStreamKeys_t disableStreamKeys; //DISABLE_STREAM_KEYS
} IdAesRxHost_t;

/// @brief 0x5FC
typedef struct
{
    PcieIdeAesAesSel_t aesSel;            //AES_SEL
} IdAesTxFipsSel_t;

/// @brief 0x500
typedef struct
{
    PcieIdeAesOutReady_t outReady;        //OUT_READY
    PcieIdeAesOutValid_t outValid;        //OUT_VALID
    PcieIdeAesOutSop_t outSop;            //OUT_SOP
    PcieIdeAesOutEop_t outEop;            //OUT_EOP
    uint8_t rsvd10[4];                    //rsvd_10
    PcieIdeAesOutAad_t outAad;            //OUT_AAD
    PcieIdeAesOutAadSize_t outAadSize;    //OUT_AAD_SIZE
    PcieIdeAesOutMsg_t outMsg;            //OUT_MSG
    PcieIdeAesOutMsgSize_t outMsgSize;    //OUT_MSG_SIZE
    PcieIdeAesOutDataIdx_t outDataIdx;    //OUT_DATA_IDX
    uint32_t outDataChunk;                //OUT_DATA_CHUNK
    uint8_t rsvd2c[4];                    //rsvd_2c
    PcieIdeAesOutMacIdx_t outMacIdx;      //OUT_MAC_IDX
    uint32_t outMacChunk;                 //OUT_MAC_CHUNK
} IdAesTxFipsOutput_t;

/// @brief 0x450
typedef struct
{
    PcieIdeAesInDataIdx_t inDataIdx;      //IN_DATA_IDX
    uint32_t inData;                      //IN_DATA
    PcieIdeAesDataValid_t dataValid;      //DATA_VALID
    PcieIdeAesDataReady_t dataReady;      //DATA_READY
} IdAesTxFipsInput_t;

/// @brief 0x400
typedef struct
{
    PcieIdeAesCmdSop_t cmdSop;            //CMD_SOP
    PcieIdeAesCmdEop_t cmdEop;            //CMD_EOP
    PcieIdeAesCmdAad_t cmdAad;            //CMD_AAD
    PcieIdeAesCmdMsg_t cmdMsg;            //CMD_MSG
    PcieIdeAesCtrIvIdx_t ctrIvIdx;        //CTR_IV_IDX
    uint32_t ctrIvChunk;                  //CTR_IV_CHUNK
    uint8_t rsvd18[12];                   //rsvd_18
    PcieIdeAesCmdAadSize_t cmdAadSize;    //CMD_AAD_SIZE
    PcieIdeAesCmdMsgSize_t cmdMsgSize;    //CMD_MSG_SIZE
    PcieIdeAesCmdCtx_t cmdCtx;            //CMD_CTX
    PcieIdeAesCmdValid_t cmdValid;        //CMD_VALID
    PcieIdeAesCmdReady_t cmdReady;        //CMD_READY
    uint8_t rsvd38[20];                   //rsvd_38
    PcieIdeAesTestModeCfg_t testModeCfg;  //TEST_MODE_CFG
} IdAesTxFipsCommand_t;

/// @brief 0x0
typedef struct
{
    PcieIdeAesCoreVerNum_t coreVerNum;    //CORE_VER_NUM
    PcieIdeAesCoreVerType_t coreVerType;  //CORE_VER_TYPE
    PcieIdeAesIrqEn_t irqEn;              //IRQ_EN
    IrqStat1_t irqStat1;                  //IRQ_STAT
    uint8_t rsvd10[4];                    //rsvd_10
    PcieIdeAesCtrl_t ctrl;                //CTRL
    PcieIdeAesStat_t stat;                //STAT
    uint8_t rsvd1c[4];                    //rsvd_1c
    uint32_t key0Key;                     //KEY_0
    uint32_t key1Key;                     //KEY_1
    uint32_t key2Key;                     //KEY_2
    uint32_t key3Key;                     //KEY_3
    uint32_t key4Key;                     //KEY_4
    uint32_t key5Key;                     //KEY_5
    uint32_t key6Key;                     //KEY_6
    uint32_t key7Key;                     //KEY_7
    uint32_t initialIvLsw;                //INITIAL_IV_LSW
    uint32_t initialIvMsw;                //INITIAL_IV_MSW
    uint8_t rsvd48[376];                  //rsvd_48
    PcieIdeAesDisableStreamKeys_t disableStreamKeys; //DISABLE_STREAM_KEYS
} IdAesTxHost_t;

/// @brief 0x0
typedef struct
{
    IdAesTxHost_t idAesTxHost;            //id_aes_tx_host
    uint8_t rsvd1c4[572];                 //rsvd_1c4
    IdAesTxFipsCommand_t idAesTxFipsCommand; //id_aes_tx_fips_command
    IdAesTxFipsInput_t idAesTxFipsInput;  //id_aes_tx_fips_input
    uint8_t rsvd460[160];                 //rsvd_460
    IdAesTxFipsOutput_t idAesTxFipsOutput; //id_aes_tx_fips_output
    uint8_t rsvd538[196];                 //rsvd_538
    IdAesTxFipsSel_t idAesTxFipsSel;      //id_aes_tx_fips_sel
    uint8_t rsvd600[512];                 //rsvd_600
    IdAesRxHost_t idAesRxHost;            //id_aes_rx_host
    uint8_t rsvd9c4[572];                 //rsvd_9c4
    IdAesRxFipsCommand_t idAesRxFipsCommand; //id_aes_rx_fips_command
    IdAesRxFipsInput_t idAesRxFipsInput;  //id_aes_rx_fips_input
    uint8_t rsvdC60[160];                 //rsvd_c60
    IdAesRxFipsOutput_t idAesRxFipsOutput; //id_aes_rx_fips_output
    uint8_t rsvdD38[196];                 //rsvd_d38
    IdAesRxFipsSel_t idAesRxFipsSel;      //id_aes_rx_fips_sel
    IdBist_t idBist;                      //id_bist
} DwcPcieIdeUaesGcmpApb_t;

typedef struct
{
    DwcPcieIdeUaesGcmpApb_t dwcPcieIdeUaesGcmpApb;                          // 0x0 : DWC_pcie_ide_uaes_gcmp_apb / 
} PcieIdeAes_t;

COMPILE_ASSERT(offsetof(PcieIdeAes_t,dwcPcieIdeUaesGcmpApb)==0x0,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile PcieIdeAes_t rPcieIdeAes; ///< 0xB01F0000
