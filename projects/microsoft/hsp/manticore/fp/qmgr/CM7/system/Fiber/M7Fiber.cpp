// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   M7Fiber.cpp
//! @brief  M7 Fiber related interface
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "MemorySection.h"
MODULE_SECTION_NORMAL
#include "M7Partition.h"
#include "M7Fiber.h"

/**
 * This function serves for fiber initialization
 *
 * @param   CompGroupId_t fiberGroupId
 * @param   FiberParameters_t& fiberParams
 * @param   FiberId fiberId
 * @return  None
 */
void M7Fiber::Initialize(M7CoreId_t core_id,                     \
                         const M7CompGroupId_t fiberGroupId,     \
                         M7FiberId_t fiberId,                    \
                         const M7FiberParameters_t& fiberParams, \
                         M7FiberPriority_t fiberPriority)
{
    // Initialize all fields of fiber
    _groupId    = fiberGroupId;
    _parameters = fiberParams;
    _id         = fiberId;
    _priority   = fiberPriority;
    _core_id = core_id;
    _isInitialized = true;
}

/**
 * This function serves for fiber register
 *
 * @param   void* pObject pointer to instance of class to get context needed
 * @param   FiberWithCtxFptr_t pRunFiber
 * @param   name  Fiber's name
 * @param   bool activate
 */
void M7Fiber::Register(void* pObj, M7FiberWithCtxFptr_t pRunFiber, \
                       const char name[], bool activate)
{
    //ASSERT_MSG(_isInitialized, ("Fiber is not initialized!"));

    M7FiberScheduler_RegisterFiber(_core_id, _groupId, _id, name, \
                                   reinterpret_cast<M7FiberFptr_t>(pRunFiber),
                                   activate, _parameters, _priority, pObj);
}

/**
 * This function serves for fiber register
 *
 * @param   FiberFptr_t pRunFiber
 * @param   name  Fiber's name
 * @param   bool activate
 */
void M7Fiber::Register(M7FiberFptr_t pRunFiber, \
                       const char name[], bool activate)
{
    //ASSERT_MSG(_isInitialized, ("Fiber is not initialized!"));

    M7FiberScheduler_RegisterFiber(_core_id, _groupId, _id, name, \
                                   pRunFiber, activate, _parameters, _priority);
}

/**
 * This function serves to change the function pointer for the Fiber
 *
 * This is used by clients when they want to change the Entry point function for the Fiber
 *
 * @param   void* pObject pointer to instance of class to get context needed
 * @param   FiberFptr_t pRunFiber
 */
void M7Fiber::UpdateFiberEntryFunction(void* pObject, \
                                       M7FiberWithCtxFptr_t pRunFiber)
{
    //ASSERT_MSG(_isInitialized, ("Fiber is not initialized!"));

    // Update the Entry function
    M7FiberScheduler_UpdateFiberEntryFunction(_core_id, _groupId, _id,                    \
                                              reinterpret_cast<M7FiberFptr_t>(pRunFiber), \
                                              _priority, pObject);

    // Always resume the Fiber when updating the Entry function
    Resume();
}
MODULE_SECTION_FAST

/**
 * This function serves for setting fiber to active state
 *
 * @param   None
 * @return  None
 */
void M7Fiber::Activate()
{
    M7FiberScheduler_SetState(_core_id, _groupId, _id, \
                              cM7FiberStateActive, _priority);
}

/**
 * This function serves for setting fiber to deactivated state
 *
 * @param   None
 * @return  None
 */
void M7Fiber::Deactivate()
{
    M7FiberScheduler_SetState(_core_id, _groupId, _id, \
                              cM7FiberStateDeactivated, _priority);
}

/**
 * This function resume a fiber from wait state
 *
 */
void M7Fiber::Resume()
{
    M7FiberScheduler_Resume(_core_id, _groupId, _id, _priority);
}

/**
 * This function provide status of fiber (active/not active)
 *
 * @param   None
 * @return  bool status
 */
bool M7Fiber::IsActive()
{
    return M7FiberScheduler_IsActive(_core_id, _groupId, _id, _priority);
}

/**
 * This function set fiber in wait mode if wait functionality enabled
 *
 * @param   None
 * @return  None
 */
void M7Fiber::Wait()
{
    M7FiberScheduler_WaitForMultipleEvents(_core_id, _id);
    // M7FiberScheduler_WaitForMultipleEvents(_queuesList);
}

/**
 * This function set fiber in wait mode if wait functionality enabled, also wait the current schedule round
 *
 * @param   None
 * @return  None
 */
void M7Fiber::WaitCurrentRound()
{
    M7FiberScheduler_WaitIncludeThisRound(_core_id, _id);
    // M7FiberScheduler_WaitForMultipleEvents(_queuesList);
}
