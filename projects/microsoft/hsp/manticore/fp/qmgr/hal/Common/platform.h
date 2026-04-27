// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#pragma once

#include "RegUcd.h"


/**
 * Platform Config
 */
#define LIONMS_B0
#define SUPPORT_FPS_REGISTER
#ifdef SUPPORT_FPS_REGISTER
#define FPS_REG_IBCQ_HS_IDX (cHwe2FpWq00UcdIbCq0)
#define FPS_REG_OBCQ_HS_IDX (cHwe2FpWq02UcdObCq0)
#define CDMA_CMD_COUNT
#endif

/**
 * LionMS Silicon Config
 */
#define DISABLE_INDIRECT_REG_WRITE
#define DISABLE_CDMA_SQ_PI_INDIRECT_REG_WRITE
#define QOS_LATENCY_ERROR_HANDLING
//#define DISABLE_IO_LOG   // disable IO log and INTEGRATE_TIMESTAMP_TO_FPSCPU
//#define SUPPORT_VF65_QB65_INTERNAL_DEBUG  // for internal debug. map IBQ 0 and 1 to CA 128 and 129

/**
 * LionMS Global Config
 */
#ifndef DISABLE_IO_LOG
// NOTE: Enabling INTEGRATE_TIMESTAMP_TO_FPSCPU floods the FP logs
//       with "logging timestamp" related logs. This causes all the
//       other logs to be overwritten. As such this flag should be
//       enabled only when required.

//#define INTEGRATE_TIMESTAMP_TO_FPSCPU
#endif
// NOTE: Enabling ENABLE_TIMESTAMP_LOGGING floods the FP logs
//       with "logging timestamp" related logs. This causes all the
//       other logs to be overwritten. As such this flag should be
//       enabled only when required.

//#define ENABLE_TIMESTAMP_LOGGING

// #define SUPPORT_TELEMETRY
#define READ_CDMA_REG_ERR_STS_WITHOUT_CQE
#define SUPPORT_CFG_CDMA_REG_MAX_ELEMT_CNT
#define SUPPORT_UPDATE_TIMESTAMP
#define SUPPORT_UPDATE_TIMESTAMP_IPC  //If not define, using message update timestamp
// #define SUPPORT_CDMA_RESET_MSG
//#define READ_CDMA_REG_ERR_STS_WITHOUT_CQE
//#define SUPPORT_MSGERROR_INJECTION
#define SUPPORT_HOL_HANDLING
#ifdef SUPPORT_CDMA_ERROR_INJECTION
#define SUPPORT_CDMA_ERROR_INJECTION_DETECT_FW_BEHAVIOR
#endif
#define WEIGHT_ROUND_ROBIN // Enable Round Robin in FP1 to prevent low queue starvation

#ifdef SUPPORT_UCD_ERROR_INJECTION
//#define SUPPORT_UCD_INJ_INV_ADDR
//#define SUPPORT_UCD_ERR_INJ_DEBUG_LOG
#endif

#ifdef QOS_LATENCY_ERROR_HANDLING
#define QOS_LATENCY_GLOBAL_UNIQUE
//#define QOS_LATENCY_TEST   ///< PRINT_TIMESTAMP_LOGGING
#endif

#ifndef NEW_AES_KEY_VALIDATION_SUPPORT
#define NEW_VF_QUEUE_MSG_STRUCTURE
#endif

/**
 * Macro definition
 */
#define MAX_PF_NUM 0x1U
#define MAX_VF_NUM 0x40U //64
#define MAX_SUPPORT_FUNC_NUM (MAX_VF_NUM + MAX_PF_NUM)
#define UCD_FP_IO_Q_NUM 0x82U //130
#define PF_ID 0x40

#define UCD_CORE_NUM 0x2U // 2
#define UCD_CORE_QUEUE_NUM 0x5U // 5
#define MAX_UCD_Q_NUM 0x108U //264
#define UCD_Q_NUM 0x80U //128
#define UCD_FP_IO_Q_SLOT_NUM 0x40U //64

#define UCD_DFL_Q_SIZE         0x200UL //512  // depth
#define UCD_DFL_1_Q_SIZE       0x20UL  //32

#define VFID_INV 0xFFU
#define ABORT_NOT_SUBMIT 0xFF

#define GLOBAL_SYNC_COUNTER_CLOCK 62500  //khz
#define ARM_SYSTICK_CLOCK 750000  //khz
#define SYSTICK_TIMER_VALUE 10
#define GLOBAL_TICK_TO_SYSTICK(gclock, sclock, gtick, threshold) ((uint32_t)(((uint64_t)sclock * (uint64_t)gtick) / (gclock * (1 + threshold))))

#define MAX_WEIGHT_EXP 11

/**
 * UCD config
 */
#define UCD_CORE_0 0x0U
#define UCD_CORE_1 0x1U

#define UCD_QUEUE_0 0x0U
#define UCD_QUEUE_1 0x1U

/**
 *  UCD related
 */
typedef enum PlatformRegName_t
{
    cUcdRegBarCpu,
    cUcdRegVfBarCpu,
    cUcdRegCmn,
    cUcdRegIqLgc2phys,
    cUcdRegOqLgc2phys,
    cUcdRegIb,
    cUcdRegIbIq,
    cUcdRegOb,
    cUcdRegObOq,
    cUcdRegBarHost,
    cUcdRegVfBarHost,
} PlatformRegName_t;

#ifdef LIONPERF_SUPPORT
/**
 *  @brief  Use UCD register name and core id to get corresponding UCD register address.
 *
 *  @param[in]  name      UCD register name.
 *  @param[in]  CoreId     Core Id.
 *                                  For core id based register (cUcdRegIb, cUcdRegIbIq, cUcdRegOb, and cUcdRegObOq), input core id 0 or 1.
 *                                  For not core id based register (otherwise), input core id 0xff.
 *  @return       uint32_t   UCD register address.
 *
 */

uint32_t GetUcdRegCoreBase(enum PlatformRegName_t name, uint8_t CoreId);
#endif
