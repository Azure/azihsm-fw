// SPDX-License-Identifier: MIT
// Copyright (c) 2022-2026 Marvell

//=============================================================================
//!
//! @brief TCON Registers
//!
//=============================================================================

// Generated with Dullahan v2.4.2.cfa8763

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "SysTypes.h"

//-----------------------------------------------------------------------------
//  Public Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Public Data Type Definitions
//-----------------------------------------------------------------------------


/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t WATCHDOG_ENABLE             : 1;     ///<BIT [0] watchdog_enable
        uint32_t RSVD                        : 31;    ///<BIT [31:1] rsvd
    } b;
} WatchdogCtrl_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t WAKEUP_ENABLE               : 2;     ///<BIT [1:0] wakeup_enable
        uint32_t RSVD2                       : 6;     ///<BIT [7:2] rsvd2
        uint32_t WKINTR_LEVEL_EN             : 2;     ///<BIT [9:8] wkintr_level_en
        uint32_t RSVD1                       : 6;     ///<BIT [15:10] rsvd1
        uint32_t WKINTR_RPT_EN               : 2;     ///<BIT [17:16] wkintr_rpt_en
        uint32_t RSVD0                       : 14;    ///<BIT [31:18] rsvd0
    } b;
} WakeupCtrl_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TIMESTAMP_CLEAR             : 8;     ///<BIT [7:0] timestamp_clear
        uint32_t RSVD                        : 22;    ///<BIT [29:8] rsvd
        uint32_t TCON_SRESET                 : 1;     ///<BIT [30] tcon_sreset
        uint32_t PERR_MASK                   : 1;     ///<BIT [31] perr_mask
    } b;
} TimestampCtl_t;

/// @brief 0xA0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DEBUG_BUS                   : 28;    ///<BIT [27:0] debug_bus
        uint32_t RSVD                        : 2;     ///<BIT [29:28] rsvd
        uint32_t DEBUG_BUS_SEL               : 2;     ///<BIT [31:30] debug_bus_sel
    } b;
} Debug_t;

typedef struct
{
    uint32_t timerLoTconTimerLo;                                            // 0x0 : timer_lo /
    uint32_t timerHiTconTimerHi;                                            // 0x4 : timer_hi /
    WatchdogCtrl_t watchdogCtrl;                                            // 0x8 : watchdog_ctrl /
    uint32_t watchdogCnt;                                                   // 0xC : watchdog_cnt /
    WakeupCtrl_t wakeupCtrl;                                                // 0x10 : wakeup_ctrl /
    uint32_t wakeup0Cnt;                                                    // 0x14 : wakeup0_cnt /
    uint32_t wakeup1Cnt;                                                    // 0x18 : wakeup1_cnt /
    uint8_t rsvd1c[32];                                                     // 0x1C : rsvd_1c / rsvd_1c
    TimestampCtl_t timestampCtl;                                            // 0x3C : timestamp_ctl /
    uint32_t timestamp0CycleCnt;                                            // 0x40 : timestamp0_cycle_cnt /
    uint32_t timestamp0Lo;                                                  // 0x44 : timestamp0_lo /
    uint32_t timestamp0Hi;                                                  // 0x48 : timestamp0_hi /
    uint32_t timestamp1CycleCnt;                                            // 0x4C : timestamp1_cycle_cnt /
    uint32_t timestamp1Lo;                                                  // 0x50 : timestamp1_lo /
    uint32_t timestamp1Hi;                                                  // 0x54 : timestamp1_hi /
    uint32_t timestamp2CycleCnt;                                            // 0x58 : timestamp2_cycle_cnt /
    uint32_t timestamp2Lo;                                                  // 0x5C : timestamp2_lo /
    uint32_t timestamp2Hi;                                                  // 0x60 : timestamp2_hi /
    uint32_t timestamp3CycleCnt;                                            // 0x64 : timestamp3_cycle_cnt /
    uint32_t timestamp3Lo;                                                  // 0x68 : timestamp3_lo /
    uint32_t timestamp3Hi;                                                  // 0x6C : timestamp3_hi /
    uint32_t timestamp4CycleCnt;                                            // 0x70 : timestamp4_cycle_cnt /
    uint32_t timestamp4Lo;                                                  // 0x74 : timestamp4_lo /
    uint32_t timestamp4Hi;                                                  // 0x78 : timestamp4_hi /
    uint32_t timestamp5CycleCnt;                                            // 0x7C : timestamp5_cycle_cnt /
    uint32_t timestamp5Lo;                                                  // 0x80 : timestamp5_lo /
    uint32_t timestamp5Hi;                                                  // 0x84 : timestamp5_hi /
    uint32_t timestamp6CycleCnt;                                            // 0x88 : timestamp6_cycle_cnt /
    uint32_t timestamp6Lo;                                                  // 0x8C : timestamp6_lo /
    uint32_t timestamp6Hi;                                                  // 0x90 : timestamp6_hi /
    uint32_t timestamp7CycleCnt;                                            // 0x94 : timestamp7_cycle_cnt /
    uint32_t timestamp7Lo;                                                  // 0x98 : timestamp7_lo /
    uint32_t timestamp7Hi;                                                  // 0x9C : timestamp7_hi /
    Debug_t debug;                                                          // 0xA0 : debug /
} Tcon_t;

COMPILE_ASSERT(offsetof(Tcon_t, timerLoTconTimerLo) == 0x0, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timerHiTconTimerHi) == 0x4, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, watchdogCtrl) == 0x8, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, watchdogCnt) == 0xC, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, wakeupCtrl) == 0x10, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, wakeup0Cnt) == 0x14, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, wakeup1Cnt) == 0x18, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestampCtl) == 0x3C, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp0CycleCnt) == 0x40, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp0Lo) == 0x44, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp0Hi) == 0x48, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp1CycleCnt) == 0x4C, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp1Lo) == 0x50, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp1Hi) == 0x54, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp2CycleCnt) == 0x58, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp2Lo) == 0x5C, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp2Hi) == 0x60, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp3CycleCnt) == 0x64, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp3Lo) == 0x68, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp3Hi) == 0x6C, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp4CycleCnt) == 0x70, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp4Lo) == 0x74, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp4Hi) == 0x78, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp5CycleCnt) == 0x7C, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp5Lo) == 0x80, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp5Hi) == 0x84, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp6CycleCnt) == 0x88, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp6Lo) == 0x8C, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp6Hi) == 0x90, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp7CycleCnt) == 0x94, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp7Lo) == 0x98, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, timestamp7Hi) == 0x9C, "check register structure offset");
COMPILE_ASSERT(offsetof(Tcon_t, debug) == 0xA0, "check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------

//extern volatile Tcon_t rTcon; ///< 0xB0005000
