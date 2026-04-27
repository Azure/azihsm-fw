// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#ifndef FP3CORE_SYSTEM_FIBER_M7FIBERDATATYPES_H_
#define FP3CORE_SYSTEM_FIBER_M7FIBERDATATYPES_H_
#pragma once
//=============================================================================
//!
//! @file   M7FiberDataTypes.h
//! @brief  M7 Fiber Data Types
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "LoggingDebug.h"
#include "M7FibersBitmap.h"

//-----------------------------------------------------------------------------
//  Public Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Public Data Type Definitions
//-----------------------------------------------------------------------------

struct M7FiberSchedulerContext_t;  // forward declaration

/// Fiber identifier
typedef uint16_t M7FiberId_t;
constexpr M7FiberId_t cM7InvalidFiberId = UINT16_MAX;

/**
 * Fiber execution function pointer.
 */
typedef void (*M7FiberFptr_t)(void);

/**
 * Fiber execution function pointer that uses context.
 */
typedef void (*M7FiberWithCtxFptr_t)(void*);

/// Fiber algorithm data.
typedef struct M7FiberAlgorithm_t
{
    struct
    {
        ///< Shadow from the actual executing bitmap.
        M7FibersBitmap    execBitmap;
    } weightedRoundRobin;
} M7FiberAlgorithm_t;

/// Fiber algorithm parameters
typedef union M7FiberAlgorithmParams_t
{
    ///< base
    uint16_t                all;
    struct
    {
        ///< fiber weight
        uint16_t            weight;
    } weightedRoundRobin;
} M7FiberAlgorithmParams_t;

/// Fiber states
typedef enum M7FiberState_t
{
    /// < Fiber is deactivated and will not be scheduled (default)
    cM7FiberStateDeactivated               = 0,
    /// < Fiber is active and will be scheduled
    cM7FiberStateActive,
    /// < Fiber is suspended
    cM7FiberStateSuspended,
} M7FiberState_t;

/// Fiber parameters
typedef struct M7Fiber_t
{
    /// < Fiber execution function pointer
    M7FiberFptr_t             fiberFptr;
    /// < Algorithm parameters
    M7FiberAlgorithmParams_t  algorithmParams;
    /// < Max allowed run time of current fiber
    uint32_t                Rsvd;
    /// < Pointer to object, which context shall be used when fiber is running
    void*                   pObject;
    /// < Fiber's Name
    const char*             pName;
    /// < Absolute working time of fiber
    //  (64-bit to avoid overflow during calculation)
    uint64_t                runTimeAbsolute;
    /// < Max run time of fiber
    uint32_t                maxRunTime;
    /// < Min run time of fiber
    uint32_t                minRunTime;
    /// < Number of fiber invocation
    uint32_t                numInvoke;
} M7Fiber_t;

typedef enum M7FiberPriority_t
{
    cM7FiberPriorityHigh = 0,
    cM7FiberPriorityLow,
    cM7NumFiberPriority
} M7FiberPriority_t;

/// Fiber scheduler context parameters
// lint -e669 -save Flexible array is not supported in C++.
// That's why array of 1 element is specified. In this case
// Lint suppression of 669 is needed.
struct M7FiberSchedulerContext_t
{
    // Assignment operator
    M7FiberSchedulerContext_t& operator= \
        (const M7FiberSchedulerContext_t& context);
    ///< number of fibers
    int                     numberOfFibers;
    ///< fiber currently being executed
    M7FiberId_t               current;
    ///< scheduler algorithm
    M7FiberAlgorithm_t        algorithm;
    ///< bits from execBitmap and waitBitmap form
    // a bits pairs [exec, wait], which define the fiber states:
    M7FibersBitmap            execBitmap;
    ///< [0, 0] - fiber deactivated (fiber isn't executed)
    ///< [0, 1] - fiber activated and in wait state (fiber isn't executed)
    ///< [1, X] - fiber active (fiber is executed)
    M7FibersBitmap            waitBitmap;
    /// < Working time of fiber scheduler
    // (64-bit to avoid overflow during time accumulation)
    uint64_t                currentStatTime;
    ///< flag indicates that one round passed for all fibers in scheduler
    bool                    roundPassedFlag;
    /// < flag indicates whether the current
    //  running fiber has had a modification to its
    /// < state (e.g. activate/deactivate, suspend, entry point update)
    bool                    currentFiberStateChanged;
    ///< list of fibers. note: must be last struct parameter.
    M7Fiber_t               fibers[M7FibersBitmap::_maxFiberCount];
};

// Assignment operator
inline M7FiberSchedulerContext_t&         \
    M7FiberSchedulerContext_t::operator = \
    (const M7FiberSchedulerContext_t& context)
{

    if (this != &context)
    {
        current         = context.current;
        execBitmap      = context.execBitmap;
        waitBitmap      = context.waitBitmap;
        currentStatTime = context.currentStatTime;
        roundPassedFlag = context.roundPassedFlag;
        fibers[0]       = context.fibers[0];
        numberOfFibers  = context.numberOfFibers;
        algorithm.weightedRoundRobin.execBitmap = \
            context.algorithm.weightedRoundRobin.execBitmap;
    }

    return *this;
}
// lint -restore

//-----------------------------------------------------------------------------
//  Public Interface Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Inline Functions
//-----------------------------------------------------------------------------

#endif  // FP3CORE_SYSTEM_FIBER_M7FIBERDATATYPES_H_
