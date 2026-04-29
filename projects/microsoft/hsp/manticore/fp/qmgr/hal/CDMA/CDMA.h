// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#ifndef _CDMA_H_
#define _CDMA_H_
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "RegCdma.h"
#define NEW_CMD         BIT(0)
#define CONTINUE_CMD    BIT(1)
#define LAST_CMD        BIT(2)
#define NEW_LAST_CMD        (NEW_CMD | LAST_CMD)
#define CONTINUE_LAST_CMD   (CONTINUE_CMD | LAST_CMD)

#define CDMA_CMD_SLOT_ERR_STS_REG_ID_0 0

#define CDMA_CMD_SLOT_ERR_CHECK_EN_REG_ID_0_MASK 0xffffffff

#define CDMA_REG_MAX_DESCR_ELMNT_COUNT_PER_CHUNK 4

#define FPS_CDMA_QUEUE_DEPTH 0x40UL
#define FPS_CDMA_QUEUE_DEPTH_MASK (FPS_CDMA_QUEUE_DEPTH - 1)
typedef struct CdmaCqCmdDescr_t
{
    union
    {
        uint32_t dw0;
        struct
        {
            uint32_t CmdId     : 10;
            uint32_t reserved0 : 1;
            uint32_t CmdStatus : 5;
            uint32_t reserved1 : 4;
            uint32_t CmdDflIdx : 10;
            uint32_t DflNum    : 2;
        } Dw0;
    };

    union
    {
        uint32_t dw1;
        struct
        {
            uint16_t CmdChunk;
            uint8_t CmdSlot;
            uint8_t UcdIqId;
        } Dw1;
    };
} CdmaCqCmdDescr_t;

typedef struct CdmaSqCmdDescr_t
{
    union
    {
        uint32_t dw0;
        struct
        {
            uint32_t CmdId    : 10;
            uint32_t CmdState : 3;
            uint32_t CmdOpcode : 3;
            uint32_t Reserved : 4;
            uint32_t CmdDflIdx : 10;
            uint32_t DflNum   : 2;
        } Dw0;
    };

    union
    {
        uint32_t dw1;
        struct
        {
            uint32_t DataProcessType        : 3;
            uint32_t DataDescrType          : 1;   // sg or prp
            uint32_t DataContinuation       : 1;
            uint32_t XtsDataUnitLength      : 2; // This value is provided in DW16 of the Host Command+Metadata structure in CIPHER[7:6] and the FW will move it into the CDMA Command.
            uint32_t DataXfrAttrReserved    : 1;
            uint32_t ChunkByteCnt           : 13;
            uint32_t Reserved               : 3;
            uint32_t UcdIqId                : 8;
        } Dw1;
    };

    union
    {
        uint32_t dw2;
        struct
        {
            uint32_t LocalKey1Idx   : 10;
            uint32_t LocalKey2Idx   : 10;
            uint32_t Reserved       : 12;
        } Dw2;
    };

    union
    {
        uint32_t dw3;
        struct
        {
            uint8_t SrcDescrIfSel;
            uint8_t SrcDataIfSel;
            uint8_t DstDescrIfSel;
            uint8_t DstDataIfSel;
        } Dw3;
    };
} CdmaSqCmdDescr_t;

static_assert(sizeof(CdmaSqCmdDescr_t) == 16, "Size of CdmaSqCmdDescr_t should be 16 bytes");
static_assert(sizeof(CdmaCqCmdDescr_t) == 8, "Size of CdmaCqCmdDescr_t should be 8 bytes");

#define CDMA_SQE_DW0_DFL_BUF_SHIFT 0x14UL//20
#define CDMA_SQE_DW0_CMD_STATE_SHIFT 0xaUL//10
#define CDMA_SQE_DW0_CDMA_LIST_NUM_SHIFT 30
#define CDMA_SQE_DW0_OPCODE_SHIFT 0xdUL//13
#define CDMA_SQE_DW1_IBQ_SHIFT 0x18UL//24
#define CDMA_SQE_DW1_CHUNK_BCNT_SHIFT 8
#define CDMA_SQE_DW1_XTS_DATA_UINT_LENGTH_SHIFT 5
#define CDMA_SQE_DW1_CONTINUE_SHIFT 0x4UL
#define CDMA_SQE_DW1_DATA_DESC_TYPE_SHIFT 3
#define CDMA_SQE_DW2_KEY1_INDEX_SHIFT 0
#define CDMA_SQE_DW2_KEY2_INDEX_SHIFT 10
#define CDMA_SQE_DW3_SDATA_ISEL_SHIFT 8
#define CDMA_SQE_DW3_DDESC_ISEL_SHIFT 16
#define CDMA_SQE_DW3_DDATA_ISEL_SHIFT 24
#define CDMA_DUMMY_PORT_ADDR (0xA0B00000ULL)

#define CDMA_STS_BCP_ERR_HALT (BIT(0))
#define CDMA_STS_CDMA_ACTIVE (BIT(1))
#define CDMA_HALT_ENABLE_FATAL_ERROR (BIT(9))
#define CDMA_INT_CAUSE_FATAL_ERROR (BIT(9))
#define CDMA_INT_CAUSE_KV_MEM_CORR_EXCEED_THRESHOLD_ERR (BIT(16))
#define CDMA_INT_CAUSE_KV_MEM_UNCORRECTABLE_ECC_ERR (BIT(17))

#define CDMA_INTERRUPT_0 (0)
#define CDMA_INTERRUPT_1 (1)

#define CDMA_DIAGNOSTIC_CMD_SLOT_STATUS_CPUID_MASK (0x3ff)

#define CDMA_IDLE_CMD_CPU_ID  0x3ffUL

#define CDMA_LIST_1  0x1UL
typedef enum Cdma_Opcode_t
{
    CDMA_OPCODE_DATA_TRANSFER = 0,
    CDMA_OPCODE_SKIP,
    CDMA_OPCODE_ABORT,
    CDMA_OPCODE_IDLE,
    CDMA_OPCODE_RESERVED = 7
}Cdma_Opcode_t;

typedef enum CdmaStatSetAction_t
{
    cActionResumeCdma = 0x0,
    cActionQoSLatencyTimer = 0x1,
    cActionKeyCorrErrThreshold = 0x2,
} CdmaStatSetAction_t;


#ifdef __cplusplus
}
#endif


#endif
