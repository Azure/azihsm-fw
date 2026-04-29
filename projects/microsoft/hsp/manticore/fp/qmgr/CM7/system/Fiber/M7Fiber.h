// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file  Fiber.h
//! @brief Header file for Fiber class
//!
//=============================================================================
#ifndef FP3CORE_SYSTEM_FIBER_M7FIBER_H_
#define FP3CORE_SYSTEM_FIBER_M7FIBER_H_
#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include <stddef.h>
#include "M7FiberScheduler.h"

/**
 * Fiber class
 *
 * Fiber is a proxy class that helps to implement fiber functionality for other classes
 * consider object oriented design of them
 *
 */
class M7Fiber {
public:
    M7Fiber() = default;

    /**
     * This function serves for fiber initialization
     *
     * @param   ComponentGroupId_t fiberGroupId
     * @param   FiberParameters_t& fiberParameters
     * @param   FiberId fiberId
     * @param   FiberPriority Fiber's priority
     * @return  None
     */
    void Initialize(M7CoreId_t core_id,                         \
                    M7CompGroupId_t fiberGroupId,               \
                    M7FiberId_t fiberId,                        \
                    const M7FiberParameters_t& fiberParameters, \
                    M7FiberPriority_t fiberPriority = cM7FiberPriorityLow);

    /// register fiber with context
    void Register(void* pObj, M7FiberWithCtxFptr_t pRunFiber, \
                  const char name[], bool activate = true);

    /// register fiber with no context
    void Register(M7FiberFptr_t pRunFiber, \
                  const char name[], bool activate = true);

    /// Update the entry function for the fiber
    void UpdateFiberEntryFunction(void* pObject, \
                                  M7FiberWithCtxFptr_t pRunFiber);

    /// setting fiber to active state
    void Activate();

    /// setting fiber to deactivated state
    void Deactivate();

    /// resume the fiber from wait state
    void Resume();

    /// provide status of fiber (active/not active)
    bool IsActive();

    /// returns Component Group ID of Fiber
    inline M7CompGroupId_t GetGroupId();

    /// returns ID of Fiber
    inline M7FiberId_t GetFiberId();

    /// set fiber in wait mode
    void Wait();

    void WaitCurrentRound();

private:
    /// < Flag to avoid usage of uninitialized fiber
    bool               _isInitialized;
    M7CompGroupId_t      _groupId;         /// < Group ID which fiber belongs
    M7FiberPriority_t    _priority;        /// < Fiber priority
    M7FiberParameters_t  _parameters;      /// < Parameters of fiber
    M7FiberId_t          _id;              /// < Fiber id
    M7CoreId_t          _core_id;          /// < core id
    /// < List of queues to be waked up on
    //  M7IpcMsgQueue**      _queuesList;
};

/// returns Component Group ID of Fiber
inline M7CompGroupId_t M7Fiber::GetGroupId()
{
    return _groupId;
}

/// returns ID of Fiber
inline M7FiberId_t M7Fiber::GetFiberId()
{
    return _id;
}
#endif  // FP3CORE_SYSTEM_FIBER_M7FIBER_H_
