// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#pragma once
#pragma pack(push, enterCommonh)
#pragma pack(1)

#include "List.h"
#include "UCD.h"
#include "CDMA.h"
#include "RegFps.h"

#define QUEUE_FULL_EXTRA_BIT(pi, ci, mask_lsb, mask_msb) \
    (((pi & mask_lsb) != (ci & mask_lsb)) ? 0 : ((pi & mask_msb) != (ci & mask_msb)))
#define QUEUE_EMPTY_EXTRA_BIT(pi, ci)   (pi == ci)

#define QUEUE_ELEMENT_NUMBER(pi, ci, mask) ((pi - ci) & mask)
#define QUEUE_OVER_THRESHOLD(pi, ci, mask, threshold) \
    (QUEUE_ELEMENT_NUMBER(pi, ci, mask) > threshold)

#define QUEUE_INC(pi, mask) ((pi + 1) & mask)
#define QUEUE_FULL(pi, ci, mask)   (((pi + 1) & mask) == ci)
#define QUEUE_EMPTY(pi, ci)   (pi == ci)

#define M7_IO_QUEUE_DEPTH 0x200
#define M7_IO_QUEUE_DEPTH_MASK 0x1FF
#define M7_IO_QUEUE_1_DEPTH_MASK 0x1F
/// < (0x1L << SHIFT) = Depth
#define M7_IO_QUEUE_SHIFT 9
/// < (0x1L << SHIFT) = Depth*2 // for new queue design
#define M7_IO_QUEUE_SHIFT_NEW 10
#define M7_IO_QUEUE2_DEPTH 0x20
#define M7_IO_QUEUE2_DEPTH_MASK 0x1F

#define M7_QUEUE_INC(index, mask) ((index + 1) & mask)
#define M7_QUEUE_FULL(pi, ci, mask) (M7_QUEUE_INC(pi, mask) == ci)


typedef enum CPUStatus_t
{
    CPStatusInit                   = 0x0,
    CPStatusInitDone,
    CPStatusStart,
} CPUStatus_t;

typedef struct _bit64
{
    union
    {
        uint64_t qw;
        uint32_t dw[2];
        uint16_t w[4];
        uint8_t  b[8];
    };
} bit64;

typedef struct
{
    uint8_t queueBlockIndex;
    uint8_t vfId;
    uint16_t remainCeIdx;
    uint32_t remainLen;
} QueueBlockInfo_t;
//COMPILE_ASSERT(sizeof(QueueBlockInfo_t) == M7_SHARE_QB_INFO_STRUCT_SIZE, "check QueueBlockInfo_t structure size");

typedef struct
{
    uint8_t vfId;
    uint8_t queueBlk65BitMap;
    uint8_t qosPenaltyPeriod;
    uint8_t reserve1;
    uint64_t queueBlkBitMap;
    uint32_t credit;
} VFNodeInfo_t;
//COMPILE_ASSERT(sizeof(VFNodeInfo_t) == M7_SHARE_VF_INFO_STRUCT_SIZE, "check VFNodeInfo_t structure size");

static inline void DMB(void)
{
    #ifndef TDD
    __asm volatile ("dmb");
    #endif
}

#pragma pack(pop, enterCommonh)
