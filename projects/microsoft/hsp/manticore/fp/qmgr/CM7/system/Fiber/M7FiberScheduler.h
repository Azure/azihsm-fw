// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   M7FiberScheduler.h
//! @brief  M7 Fiber Scheduler header
//!
//=============================================================================
#ifndef FP3CORE_SYSTEM_FIBER_M7FIBERSCHEDULER_H_
#define FP3CORE_SYSTEM_FIBER_M7FIBERSCHEDULER_H_
#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "M7FiberDataTypes.h"
#include "M7FiberWeightedRoundRobinAlgorithm.h"
#include "M7Partition.h"
//-----------------------------------------------------------------------------
//  Public Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Public Data Type Definitions
//-----------------------------------------------------------------------------

typedef void (*OnM7FiberSchedulerInitializedFptr_t)(void);

/*
 * Pointer to fiber scheduler initialized callback that uses context
 */
typedef void (*OnM7FiberSchedulerInitializedWithCxtFptr_t)(void*);
typedef struct M7FiberParameters_t
{
    uint8_t     fiberWeight;         // Fiber weight
    uint32_t    Rsvd;  // Maximum threshold runtime of a fiber
} M7FiberParameters_t;
/// < Type for callback w/o any parameters
typedef void (*M7CallbackNoParam_t)();

class M7Fiber;

//-----------------------------------------------------------------------------
//  Public Interface Functions
//-----------------------------------------------------------------------------

/**
 * For Ever loop Servicing fibers.
 *
 */
void M7FiberScheduler_ServiceLoop(M7CoreId_t core_id);

/**
 * Registers a partition.
 *
 * @param[in ]  partition       partition
 * @param[in ]  numberOfFibers  number of fibers for this partition
 * @param[in ]  onInitialized   Initialization completed callback
 * @param[in ]  objPtr          pointer to partition object
 */
void M7FiberScheduler_RegisterCompGroup(M7CoreId_t core_id,                                \
                                        M7CompGroupId_t partition,                         \
                                        M7FiberId_t numberOfFibers,                        \
                                        OnM7FiberSchedulerInitializedFptr_t onInitialized, \
                                        void* objPtr = nullptr);

/**
 * Registers a fiber.
 *
 * @param[in ]  partition           partition
 * @param[in ]  fiberId             fiber ID
 * @param[in ]  name                Fiber's name
 * @param[in ]  fiberFptr           fiber execution function pointer
 * @param[in ]  activate            activate immediately or keep fiber dormant until activated
 * @param[in ]  fiberParameters     fiber parameters, including weight of the fiber and max threshold runtime of the fiber
 * @param[in ]  objPtr              pointer to object which context shall be used when fiber is running
 */
void M7FiberScheduler_RegisterFiber(M7CoreId_t core_id,                  \
                                    M7CompGroupId_t partition,           \
                                    M7FiberId_t fiberId,                 \
                                    const char* name,                    \
                                    M7FiberFptr_t fiberFptr,             \
                                    bool activate,                       \
                                    M7FiberParameters_t fiberParameters, \
                                    void* objPtr = nullptr);

/**
 * Registers a fiber with priority
 *
 * @param[in ]  partition           partition
 * @param[in ]  fiberId             fiber ID
 * @param[in ]  pName               Fiber's Name
 * @param[in ]  fiberFptr           fiber execution function pointer
 * @param[in ]  activate            activate immediately or keep fiber dormant until activated
 * @param[in ]  fiberParameters     fiber parameters, including weight of the fiber and max threshold runtime of the fiber
 * @param[in ]  fiberPriority       fiber's priority
 * @param[in ]  pObjPtr              pointer to object which context shall be used when fiber is running
 */
void M7FiberScheduler_RegisterFiber(M7CoreId_t core_id,                  \
                                    M7CompGroupId_t compGroup,           \
                                    M7FiberId_t fiberId,                 \
                                    const char name[],                   \
                                    M7FiberFptr_t fiberFptr,             \
                                    bool activate,                       \
                                    M7FiberParameters_t fiberParameters, \
                                    M7FiberPriority_t fiberPriority,     \
                                    void* pObjPtr = nullptr);

/**
 * Update the Entry point function for the Fiber
 *
 * @param[in ]  partition           partition
 * @param[in ]  fiberId             fiber ID
 * @param[in ]  fiberFptr           fiber execution function pointer
 * @param[in ]  objPtr              pointer to object which context shall be used when fiber is running
 * @param[in ]  fiberPriority       fiber's priority
 */
void M7FiberScheduler_UpdateFiberEntryFunction(M7CoreId_t core_id,              \
                                               M7CompGroupId_t compGroup,       \
                                               M7FiberId_t fiberId,             \
                                               M7FiberFptr_t fiberFptr,         \
                                               M7FiberPriority_t fiberPriority, \
                                               void* objPtr = nullptr);

/**
 * Sets the state of the fiber.
 *
 * @param[in ]  partition   partition
 * @param[in ]  fiberId     fiber ID
 * @param[in ]  state       the new fiber state
 * @param[in ]  fiberPriority    fiber's priority
 */
void M7FiberScheduler_SetState(M7CoreId_t core_id,        \
                               M7CompGroupId_t partition, \
                               M7FiberId_t fiberId,       \
                               M7FiberState_t state,      \
                               M7FiberPriority_t fiberPriority = cM7FiberPriorityLow);

/**
 * Returns the fiber state.
 *
 * @param[in ]  partition   partition
 * @param[in ]  fiberId     fiber ID
 * @param[in ]  fiberPriority  fiber's priority
 * @return                  the fiber state
 */
bool M7FiberScheduler_IsActive(M7CoreId_t core_id,        \
                               M7CompGroupId_t partition, \
                               M7FiberId_t fiberId,       \
                               M7FiberPriority_t fiberPriority = cM7FiberPriorityLow);

/**
 * Checks if an event is pending
 */
bool M7FiberScheduler_IsEventPending(M7CoreId_t core_id);

/**
 * Initialize partition's fiber.
 */
void M7FiberScheduler_Initialize(M7CoreId_t core_id);

/**
 * Returns amount of executed fibers in context
 *
 * @param[in ]  fiberPriority  fiber's priority
 * @return          amount of executed fibers
 */
uint32_t M7FiberScheduler_GetExecFibersCountInContext(M7CoreId_t core_id, \
                                                      M7FiberPriority_t fiberPriority = cM7FiberPriorityLow);

/**
 * Determines if scheduler is idle
 *
 * @param[in ]  fiberPriority  fiber's priority
 * @return true  - if scheduler is idle,
 *         false - otherwise
 */
bool M7FiberScheduler_IsIdle(M7CoreId_t core_id, \
                             M7FiberPriority_t fiberPriority = cM7FiberPriorityLow);

/**
 * This function is used when multiple queues is bound to one fiber.
 * It tries to subscribe to all queues if it successful fiber will be
 * switched in wait state.
 *
 * @param[in ]  queueList  pointer to queue list.
 *
 * @return      None
 */
void M7FiberScheduler_WaitForMultipleEvents(M7CoreId_t core_id, \
                                            M7FiberId_t curId,  \
                                            M7FiberPriority_t fiberPriority = cM7FiberPriorityLow);


/**
 * This function is used when multiple queues is bound to one fiber.
 * It tries to subscribe to all queues if it successful fiber will be
 * switched in wait state. This function will also update execbitmap of current schedule structure.
 *
 * @param[in ]  queueList  pointer to queue list.
 *
 * @return      None
 */

void M7FiberScheduler_WaitIncludeThisRound(M7CoreId_t core_id, \
                                           M7FiberId_t curId,  \
                                           M7FiberPriority_t fiberPriority = cM7FiberPriorityLow);

/**
 * Resume fiber from wait state.
 *
 * @param[in ]  compGroup   compGroup
 * @param[in ]  fiberId     fiber ID
 */
void M7FiberScheduler_Resume(M7CoreId_t core_id,        \
                             M7CompGroupId_t compGroup, \
                             M7FiberId_t fiberId,       \
                             M7FiberPriority_t fiberPriority = cM7FiberPriorityLow);


/**
 * Resume fiber from wait state. This function will also update execbitmap of current schedule structure.
 *
 * @param[in ]  compGroup   compGroup
 * @param[in ]  fiberId     fiber ID
 */
void M7FiberScheduler_ResumeIncludeThisRound(M7CoreId_t core_id, M7CompGroupId_t compGroup, \
                                             M7FiberId_t fiberId, M7FiberPriority_t fiberPriority);

/**
 * Prints status of the fibers which were bound(each queue holds subscriber Fiber ID).
 * This function is used for monitor command.
 *
 * @param       None
 *
 * @return      None
 */
void M7FiberScheduler_PrintQueueSubscriptionFiberStatuses(M7CoreId_t core_id, \
                                                          M7FiberPriority_t fiberPriority = cM7FiberPriorityLow);

//-----------------------------------------------------------------------------
//  Public Interface Functions
//-----------------------------------------------------------------------------

/**
 * Temporarily suspends the current fiber.
 * The fiber will temporarily not be called by the fiber scheduler and
 * will be resumed next round.
 *
 * @param[in ]  fiberPriority       fiber's priority
 */
__inline static void M7FiberScheduler_SuspendCurrentFiber(M7CoreId_t core_id, \
                                                          M7FiberPriority_t fiberPriority = cM7FiberPriorityLow);

/**
 * Returns the current fiber.
 *
 * @param[in ]      pContent    scheduler context
 * @return                      the current fiber.
 */
__inline static M7FiberId_t M7FiberScheduler_GetCurrent( \
    const M7FiberSchedulerContext_t* pContext);

//-----------------------------------------------------------------------------
//  Inline Functions
//-----------------------------------------------------------------------------

#endif  // FP3CORE_SYSTEM_FIBER_M7FIBERSCHEDULER_H_
