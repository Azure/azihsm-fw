// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Marvell

//=============================================================================
//!
//! @file   HeartBeat.cpp
//! @brief  The system HeartBeat API implements.
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------
#include "LoggingDebug.h"
#include "M7Fiber.h"
#include "FpCommon.h"
#include "MemIo.h"
#include "M7MemMap.h"
#if defined (CPU0)
#include "FpsCpu0/FpsCpu0.h"
extern fpsCpu0 gFpsCpu0;
#elif defined (CPU1)
#include "FpsCpu1/FpsCpu1.h"
extern fpsCpu1 gFpsCpu1;
#elif defined (CPU2)
#include "FpsCpu2/FpsCpu2.h"
extern fpsCpu2 gFpsCpu2;
#endif

typedef enum Heartbeat
{
    HeartbeatClear = 0x0,
    HeartbeatSet
}HeartbeatCounter;
//-----------------------------------------------------------------------------
//  Member Function Definitions
//-----------------------------------------------------------------------------
void CheckHeartbeatFiber(void* pObj)
{
    #if defined (CPU0)
    writel(HeartbeatSet, M7_FPS_CPU01_CRASH_DUMP_COUNTER);
    #elif defined (CPU1)
    writel(HeartbeatSet, M7_FPS_CPU12_CRASH_DUMP_COUNTER);
    #elif defined (CPU2)
    writel(HeartbeatSet, M7_FPS_CPU20_CRASH_DUMP_COUNTER);
    #endif
}
