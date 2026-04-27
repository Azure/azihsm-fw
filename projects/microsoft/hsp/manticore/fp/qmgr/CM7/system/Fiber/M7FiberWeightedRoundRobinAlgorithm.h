// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#ifndef FP3CORE_SYSTEM_FIBER_M7FIBERWEIGHTEDROUNDROBINALGORITHM_H_
#define FP3CORE_SYSTEM_FIBER_M7FIBERWEIGHTEDROUNDROBINALGORITHM_H_
#pragma once
//=============================================================================
//
//! @file   FiberWeightedRoundRobinAlgorithm.h
//! @brief  Fiber Weighted Round Robin Algorithm
//!
//=============================================================================

#include "M7FiberDataTypes.h"
#ifdef __cplusplus
extern "C"
{
#endif
#include "Mathematics.h"
#ifdef __cplusplus
}
#endif
//#include "Debug.h"

//-----------------------------------------------------------------------------
//  constant definitions:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Exported variable reference
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Interface Functions
//-----------------------------------------------------------------------------

/**
 * Installs the scheduler algorithm.
 *
 * @param[in|out]   pContext    scheduler context
 */
__inline static void M7FiberWeightedRoundRobinAlgorithm_Initialize( \
    M7FiberSchedulerContext_t* pContext);

/**
 * Returns the ID of the next fiber to be executed.
 *
 * @param[in|out]   pContext    scheduler context
 * @return                      the next fiber to be executed
 */
__inline static M7FiberId_t M7FiberWeightedRoundRobinAlgorithm_Next( \
    M7FiberSchedulerContext_t* pContext);

//-----------------------------------------------------------------------------
//  Inline function(s)
//-----------------------------------------------------------------------------

__inline static void M7FiberWeightedRoundRobinAlgorithm_Initialize( \
    M7FiberSchedulerContext_t* pContext)
{
    pContext->algorithm.weightedRoundRobin.execBitmap = pContext->execBitmap;
}

__inline static M7FiberId_t M7FiberWeightedRoundRobinAlgorithm_Next( \
    M7FiberSchedulerContext_t* pContext)
{
    M7FiberId_t current       = pContext->current;
    //M7Fiber_t*  pCurrentFiber = &(pContext->fibers[current]);
    M7FiberId_t next;

    // Disable the current fiber in the execBitmap.
    M7FibersBitmap execBitmap = \
        pContext->algorithm.weightedRoundRobin.execBitmap;
    execBitmap.ClearFiberId(current);

    if (execBitmap.IsEmpty())
    {
        execBitmap = pContext->execBitmap;

        // set this flag, when one round passed
        pContext->roundPassedFlag = true;
    }

    pContext->algorithm.weightedRoundRobin.execBitmap = execBitmap;

    next = execBitmap.NextFiberId();
    #if defined (fps_cpu0Core)
    //DebugLogLvDbgInfo (cLogCPU0Common, cLogDebug, ("M7FiberWeightedRoundRobinAlgorithm_Initialize (context=0x%x)\n", pContext), "32");
    //DebugLogLvDbgInfo (cLogCPU0Common, cLogDebug, ("M7FiberWeightedRoundRobinAlgorithm_Initialize (bitmap:0x%x)\n", pContext->execBitmap.GetValue()), "32");
    //DebugLogLvDbgInfo (cLogCPU0Common, cLogDebug, ("M7FiberWeightedRoundRobinAlgorithm_Initialize (shadowBitmap:0x%x)\n", pContext->algorithm.weightedRoundRobin.execBitmap.GetValue()), "32");
    //DebugLogLvDbgInfo (cLogCPU0Common, cLogDebug, ("M7FiberWeightedRoundRobinAlgorithm_Initialize (next:%d)\n", next), "32");
    #elif defined (fps_cpu1Core)
    //DebugLogLvDbgInfo (cLogCPU1Common, cLogDebug, ("M7FiberWeightedRoundRobinAlgorithm_Initialize (context=0x%x)\n", pContext), "32");
    //DebugLogLvDbgInfo (cLogCPU1Common, cLogDebug, ("M7FiberWeightedRoundRobinAlgorithm_Initialize (bitmap:0x%x)\n", pContext->execBitmap.GetValue()), "32");
    //DebugLogLvDbgInfo (cLogCPU1Common, cLogDebug, ("M7FiberWeightedRoundRobinAlgorithm_Initialize (shadowBitmap:0x%x)\n", pContext->algorithm.weightedRoundRobin.execBitmap.GetValue()), "32");
    //DebugLogLvDbgInfo (cLogCPU1Common, cLogDebug, ("M7FiberWeightedRoundRobinAlgorithm_Initialize (next:%d)\n", next), "32");
    #elif defined (fps_cpu2Core)
    //DebugLogLvDbgInfo (cLogCPU2Common, cLogDebug, ("M7FiberWeightedRoundRobinAlgorithm_Initialize (context=0x%x)\n", pContext), "32");
    //DebugLogLvDbgInfo (cLogCPU2Common, cLogDebug, ("M7FiberWeightedRoundRobinAlgorithm_Initialize (bitmap:0x%x)\n", pContext->execBitmap.GetValue()), "32");
    //DebugLogLvDbgInfo (cLogCPU2Common, cLogDebug, ("M7FiberWeightedRoundRobinAlgorithm_Initialize (shadowBitmap:0x%x)\n", pContext->algorithm.weightedRoundRobin.execBitmap.GetValue()), "32");
    //DebugLogLvDbgInfo (cLogCPU2Common, cLogDebug, ("M7FiberWeightedRoundRobinAlgorithm_Initialize (next:%d)\n", next), "32");
    #endif
    return next;
}
#endif  // FP3CORE_SYSTEM_FIBER_M7FIBERWEIGHTEDROUNDROBINALGORITHM_H_
