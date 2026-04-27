// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   M7FiberScheduler.cpp
//! @brief  M7 Fiber Scheduler Common code
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "M7MemMap.h"
#include "MemorySection.h"
MODULE_SECTION_FAST
    MODULE_OPTIMIZE_FOR_MAX_SPEED

#include "M7Fiber.h"
#include "M7FiberDataTypes.h"
#include "M7FiberScheduler.h"
#include "M7FiberSchedulerContext.h"
#include "M7Partition.h"
#include "cm7ikmcu.h"
#ifdef fps_cpu0Core
#include "FpsCpu0.h"
#elif defined (fps_cpu1Core)
#include "FpsCpu1.h"
#else
#include "FpsCpu2.h"
#endif //fps_cpu0Core
extern "C"
{
#include "vicommon.h"
}

//-----------------------------------------------------------------------------
//  Private Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Private Data Type Definitions
//-----------------------------------------------------------------------------

typedef struct M7CompGroupInformation_t
{
    M7FiberId_t                           fiberIdOffset;
    M7FiberId_t                           numberOfFibers;
    M7CoreId_t                            coreId;
    OnM7FiberSchedulerInitializedFptr_t   onInitialized;
    void*                               pCompGroupObject;
} M7CompGroupInformation_t;

/// Fiber Scheduler module variables.
//  Note that this is not a shared variable, so valid within each Cpu separately only
typedef struct M7FiberScheduler_t
{
    M7CompGroupInformation_t              compGroupInformation[cM7NumberOfCompGroups];
    M7FiberId_t                           totalNumberOfFibers;
    bool                                isLowPriorityFiberRunning;
} M7FiberScheduler_t;

//-----------------------------------------------------------------------------
//  Private Function Declarations
//-----------------------------------------------------------------------------

static void M7GetTotalNumberOfFibersAndFiberIdOffset(M7CoreId_t core_id);
static void M7CheckForIpcQueueEvent(void);
__inline static bool IsM7FiberRegistered(const M7FiberSchedulerContext_t* pContext, \
                                         M7FiberId_t fiberId);
__inline static bool IsM7FiberActive(const M7FiberSchedulerContext_t* pContext, \
                                     M7FiberId_t fiberId);
__inline static bool IsM7FiberInWaitState(const M7FiberSchedulerContext_t* pContext, \
                                          M7FiberId_t fiberId);
__inline static bool IsM7FiberOnExecute(const M7FiberSchedulerContext_t* pContext, \
                                        M7FiberId_t fiberId);
__inline static bool IsM7FiberInWait(const M7FiberSchedulerContext_t* pContext, \
                                     M7FiberId_t fiberId);
static void M7ActivateFiber(M7CoreId_t core_id, M7FiberId_t fiberId, \
                            M7FiberPriority_t fiberPriority = cM7FiberPriorityLow, bool resumeThisRound = false);
static void M7DeactivateFiber(M7CoreId_t core_id, M7FiberId_t fiberId, \
                              M7FiberPriority_t fiberPriority = cM7FiberPriorityLow);
static void M7SetFiberInWait(M7CoreId_t core_id, M7FiberId_t fiberId, \
                             M7FiberPriority_t fiberPriority = cM7FiberPriorityLow, bool waitThisRound = false);
__inline static void M7Run(M7CoreId_t core_id);
// no data is being processed by readers(the output fiber is disabled)
// extern void RegisterCommands_RegisterTestCommands(void);

//-----------------------------------------------------------------------------
//  Private Variable Definitions
//-----------------------------------------------------------------------------

///< CPU Fiber Scheduler module variables
static  M7FiberScheduler_t            m7fiberScheduler;
///< fiber scheduler context
M7FiberSchedulerContext_t*            _pM7FiberSchedulerContext[cM7NumFiberPriority];
#define DEF_M7FIBERSCHEDULER(id) (m7fiberScheduler)
#define DEF_M7FIBERSCHEDULER_CTX(id) (_pM7FiberSchedulerContext)

//-----------------------------------------------------------------------------
//  Public Function Definitions
//-----------------------------------------------------------------------------

// #if (_CPU_ != CPU_SHARED)

NORMAL_CODE
void M7FiberScheduler_RegisterCompGroup(M7CoreId_t core_id, M7CompGroupId_t compGroup,     \
                                        M7FiberId_t numberOfFibers,                        \
                                        OnM7FiberSchedulerInitializedFptr_t onInitialized, \
                                        void* objPtr)
{
    // Debug_Log(cLogFiberScheduler, cLogDebug, \
    //          ("compGroup %02d / fiber %d at core %d\n", compGroup, numberOfFibers, core_id));

    DEF_M7FIBERSCHEDULER(core_id).compGroupInformation[compGroup].numberOfFibers   = numberOfFibers;
    DEF_M7FIBERSCHEDULER(core_id).compGroupInformation[compGroup].coreId           =  core_id;
    DEF_M7FIBERSCHEDULER(core_id).compGroupInformation[compGroup].onInitialized    = onInitialized;
    DEF_M7FIBERSCHEDULER(core_id).compGroupInformation[compGroup].pCompGroupObject = objPtr;
}

NORMAL_CODE
void M7FiberScheduler_RegisterFiber(M7CoreId_t core_id, M7CompGroupId_t compGroup, \
                                    M7FiberId_t fiberId, const char name[],        \
                                    M7FiberFptr_t fiberFptr, bool activate,        \
                                    M7FiberParameters_t fiberParameters, void* objPtr)
{
    fiberId += DEF_M7FIBERSCHEDULER(core_id).compGroupInformation[compGroup].fiberIdOffset;

    M7Fiber_t defaultValue                                                                                             = {0};
    // Default initialization.
    DEF_M7FIBERSCHEDULER_CTX(core_id)[cM7FiberPriorityLow]->fibers[fiberId]                                            = defaultValue;
    DEF_M7FIBERSCHEDULER_CTX(core_id)[cM7FiberPriorityLow]->fibers[fiberId].pName                                      = name;
    DEF_M7FIBERSCHEDULER_CTX(core_id)[cM7FiberPriorityLow]->fibers[fiberId].fiberFptr                                  = fiberFptr;
    DEF_M7FIBERSCHEDULER_CTX(core_id)[cM7FiberPriorityLow]->fibers[fiberId].algorithmParams.weightedRoundRobin.weight  =
        fiberParameters.fiberWeight;
    DEF_M7FIBERSCHEDULER_CTX(core_id)[cM7FiberPriorityLow]->fibers[fiberId].pObject                                    = objPtr;

    if (activate)
    {
        M7ActivateFiber(core_id, fiberId, cM7FiberPriorityLow);
    }
    else
    {
        M7DeactivateFiber(core_id, fiberId, cM7FiberPriorityLow);
    }
}

NORMAL_CODE
void M7FiberScheduler_RegisterFiber(M7CoreId_t core_id, M7CompGroupId_t compGroup, \
                                    M7FiberId_t fiberId, const char pName[],       \
                                    M7FiberFptr_t fiberFptr, bool activate,        \
                                    M7FiberParameters_t fiberParameters,           \
                                    M7FiberPriority_t fiberPriority, void* pObjPtr)
{
    fiberId += DEF_M7FIBERSCHEDULER(core_id).compGroupInformation[compGroup].fiberIdOffset;

    M7Fiber_t defaultValue = { 0 };
    // Default initialization.
    DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority]->fibers[fiberId] = defaultValue;

    DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority]->fibers[fiberId].pName = pName;
    DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority]->fibers[fiberId].fiberFptr = fiberFptr;
    DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority]->fibers[fiberId].algorithmParams.weightedRoundRobin.weight =
        fiberParameters.fiberWeight;
    DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority]->fibers[fiberId].pObject = pObjPtr;

    if (activate)
    {
        M7ActivateFiber(core_id, fiberId, fiberPriority);
    }
    else
    {
        M7DeactivateFiber(core_id, fiberId, fiberPriority);
    }
}

void M7FiberScheduler_UpdateFiberEntryFunction(M7CoreId_t core_id, M7CompGroupId_t compGroup, \
                                               M7FiberId_t fiberId, M7FiberFptr_t fiberFptr,  \
                                               M7FiberPriority_t fiberPriority, void* pObjPtr)
{
    fiberId += DEF_M7FIBERSCHEDULER(core_id).compGroupInformation[compGroup].fiberIdOffset;

    DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority]->fibers[fiberId].fiberFptr = fiberFptr;
    DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority]->fibers[fiberId].pObject = pObjPtr;

    if (DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority]->current == fiberId)
    {
        DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority]->currentFiberStateChanged = true;
    }
}

void M7FiberScheduler_SetState(M7CoreId_t core_id, M7CompGroupId_t compGroup, \
                               M7FiberId_t fiberId, M7FiberState_t state,     \
                               M7FiberPriority_t fiberPriority)
{
    fiberId += DEF_M7FIBERSCHEDULER(core_id).compGroupInformation[compGroup].fiberIdOffset;

    switch (state)
    {
        case cM7FiberStateActive:
            if (!IsM7FiberActive(DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority], fiberId))
            {
                // activate only if fiber is deactivated
                M7ActivateFiber(core_id, fiberId, fiberPriority);
            }
            else
            {
                //    Debug_Log(cLogFiberScheduler, cLogDebug, \
                //              ("FiberScheduler_SetState: Fiber has been already activated! Fiber ID: %d\n", fiberId));
            }

            break;

        case cM7FiberStateDeactivated:
            M7DeactivateFiber(core_id, fiberId, fiberPriority);
            break;

        default:
            // HALT();
            break;
    }
}

bool M7FiberScheduler_IsActive(M7CoreId_t core_id, M7CompGroupId_t compGroup, \
                               M7FiberId_t fiberId, M7FiberPriority_t fiberPriority)
{
    fiberId += DEF_M7FIBERSCHEDULER(core_id).compGroupInformation[compGroup].fiberIdOffset;
    return IsM7FiberActive(DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority], fiberId);
}

NORMAL_CODE
void M7FiberScheduler_Initialize(M7CoreId_t core_id)
{
    M7GetTotalNumberOfFibersAndFiberIdOffset(core_id);

    // Start from high priority fiber
    DEF_M7FIBERSCHEDULER(core_id).isLowPriorityFiberRunning = false;

    DEF_M7FIBERSCHEDULER_CTX(core_id)[cM7FiberPriorityHigh] = M7FiberSchedulerContext_CreateInstance(core_id,
                                                                                                     DEF_M7FIBERSCHEDULER(core_id).totalNumberOfFibers, cM7FiberPriorityHigh);
    DEF_M7FIBERSCHEDULER_CTX(core_id)[cM7FiberPriorityLow] = M7FiberSchedulerContext_CreateInstance(core_id,
                                                                                                    DEF_M7FIBERSCHEDULER(core_id).totalNumberOfFibers, cM7FiberPriorityLow);

    // Notify the compGroups on this Cpu that we're initialized
    for (uint32_t compGroup = 0; compGroup < (uint32_t)cM7NumberOfCompGroups; compGroup++)
    {
        M7CompGroupInformation_t* pCompGroupInformation = &DEF_M7FIBERSCHEDULER(core_id).compGroupInformation[compGroup];

        if (pCompGroupInformation->coreId == (M7CoreId_t)core_id)
        {
            if (pCompGroupInformation->onInitialized != NULL)
            {
                if (nullptr == pCompGroupInformation->pCompGroupObject)
                {
                    pCompGroupInformation->onInitialized();
                }
                else
                {
                    auto pNotifyCompGroup = reinterpret_cast<OnM7FiberSchedulerInitializedWithCxtFptr_t>(pCompGroupInformation->onInitialized);
                    pNotifyCompGroup(pCompGroupInformation->pCompGroupObject);
                }
            }
        }
    }

}

uint32_t M7FiberScheduler_GetExecFibersCountInContext(M7CoreId_t core_id, M7FiberPriority_t fiberPriority)
{
    uint32_t execFibersCount = DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority]->execBitmap.GetFibersCount();

    return execFibersCount;
}

bool M7FiberScheduler_IsIdle(M7CoreId_t core_id, M7FiberPriority_t fiberPriority)
{
    // CPU can receive messages to activate fibers through CPU notification service
    // and cannot be considered as idle if there are any notification messages
    return (/*CpuNotificationService::IsEmpty() &&*/ (DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority]->execBitmap.GetFibersCount() ==
                                                      0));
}
#if 1
void M7FiberScheduler_WaitForMultipleEvents(M7CoreId_t core_id, M7FiberId_t curId,  M7FiberPriority_t fiberPriority)
{
    // Debug_Log(cLogFiberScheduler, cLogDebug, ("FiberScheduler_WaitForMultipleEvents: Goes to inactive pFiber=%X CPU%d\n",
    //          currentFiberId, core_id));

    // All queues in list are empty. Fiber can be switched in wait state.

    //M7SetFiberInWait(core_id, currentFiberId, fiberPriority);
    M7SetFiberInWait(core_id, curId, fiberPriority);
}
#endif

void M7FiberScheduler_WaitIncludeThisRound(M7CoreId_t core_id, M7FiberId_t curId,  M7FiberPriority_t fiberPriority)
{
    //M7FiberId_t currentFiberId = DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority]->current;
    M7SetFiberInWait(core_id, curId, fiberPriority,  true);
}

void M7FiberScheduler_Resume(M7CoreId_t core_id, M7CompGroupId_t compGroup, \
                             M7FiberId_t fiberId, M7FiberPriority_t fiberPriority)
{
    fiberId += DEF_M7FIBERSCHEDULER(core_id).compGroupInformation[compGroup].fiberIdOffset;

    // Activate the fiber only if it is in wait state
    if (IsM7FiberInWaitState(DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority], fiberId))
    {
        M7ActivateFiber(core_id, fiberId, fiberPriority);
    }
}

void M7FiberScheduler_ResumeIncludeThisRound(M7CoreId_t core_id, M7CompGroupId_t compGroup, \
                                             M7FiberId_t fiberId, M7FiberPriority_t fiberPriority)
{
    fiberId += DEF_M7FIBERSCHEDULER(core_id).compGroupInformation[compGroup].fiberIdOffset;

    // Activate the fiber only if it is in wait state
    if (IsM7FiberInWaitState(DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority], fiberId))
    {
        M7ActivateFiber(core_id, fiberId, fiberPriority, true);
    }
}

/**
 * Service loop of fiber scheduler.
 */
void M7FiberScheduler_ServiceLoop(M7CoreId_t core_id)
{
    do
    {
        M7Run(core_id);
    } while (true);
}

//-----------------------------------------------------------------------------
//  Private Function Definitions
//-----------------------------------------------------------------------------

/**
 * Get total number of fibers and fiber ID offset of each compGroup on the current Cpu
 *
 */
static void M7GetTotalNumberOfFibersAndFiberIdOffset(M7CoreId_t core_id)
{
    M7FiberId_t fiberOffset = 0;

    for (uint32_t compGroup = 0; compGroup < (uint32_t)cM7NumberOfCompGroups; compGroup++)
    {
        M7CompGroupInformation_t* pCompGroupInformation = &DEF_M7FIBERSCHEDULER(core_id).compGroupInformation[compGroup];

        if (pCompGroupInformation->coreId == (M7CoreId_t)core_id)
        {
            pCompGroupInformation->fiberIdOffset = fiberOffset;
            fiberOffset += pCompGroupInformation->numberOfFibers;
        }
    }

    DEF_M7FIBERSCHEDULER(core_id).totalNumberOfFibers = fiberOffset;
}

/**
 * Checks if a fiber has been registered.
 *
 * @param[in]       pContext    scheduler context
 * @param[in]       fiberId     ID of the fiber to be checked
 * @return                      True when the fiber has been registered, false otherwise.
 */
__inline static bool IsM7FiberRegistered(const M7FiberSchedulerContext_t* pContext, \
                                         M7FiberId_t fiberId)
{
    return (pContext->fibers[fiberId].fiberFptr != NULL);
}

/**
 * Checks if the fiber is active.
 *
 * @param[in]       pContext    scheduler context
 * @param[in]       fiberId     ID of the fiber to be checked
 * @return                      True if the fiber is active. False otherwise.
 */
__inline static bool IsM7FiberActive(const M7FiberSchedulerContext_t* pContext, \
                                     M7FiberId_t fiberId)
{
    return (pContext->execBitmap.IsFiberIdSet(fiberId) | pContext->waitBitmap.IsFiberIdSet(fiberId));
}

/**
 * Checks if the fiber is in wait state.
 *
 * @param[in]       pContext    scheduler context
 * @param[in]       fiberId     ID of the fiber to be checked
 * @return                      True if the fiber is in wait state. False otherwise.
 */
__inline static bool IsM7FiberInWaitState(const M7FiberSchedulerContext_t* pContext, \
                                          M7FiberId_t fiberId)
{
    return (pContext->waitBitmap.IsFiberIdSet(fiberId));
}

/**
 * Checks if the fiber is on execute.
 *
 * @param[in]       pContext    scheduler context
 * @param[in]       fiberId     ID of the fiber to be checked
 * @return                      True if the fiber is on execute. False otherwise.
 */
__inline static bool IsM7FiberOnExecute(const M7FiberSchedulerContext_t* pContext, \
                                        M7FiberId_t fiberId)
{
    return pContext->execBitmap.IsFiberIdSet(fiberId);
}

/**
 * Checks if the fiber is in wait state.
 *
 * @param[in]       pContext    scheduler context
 * @param[in]       fiberId     ID of the fiber to be checked
 * @return                      True if the fiber is in wait state. False otherwise.
 */
__inline static bool IsM7FiberInWait(const M7FiberSchedulerContext_t* pContext, \
                                     M7FiberId_t fiberId)
{
    // DEBUG_ASSERT_MSG(!(pContext->execBitmap.IsFiberIdSet(fiberId) && \
    //                    pContext->waitBitmap.IsFiberIdSet(fiberId)), ("IsFiberInWait: Undefined fiber state\n"));

    return pContext->waitBitmap.IsFiberIdSet(fiberId);
}

/**
 * Activates a fiber.
 * After activation the fiber scheduler will start scheduling the fiber.
 *
 * @param[in ]      fiberId         ID of the fiber to be activated
 * @param[in ]      fiberPriority   fiber's priority
 */
static void M7ActivateFiber(M7CoreId_t core_id, M7FiberId_t fiberId, M7FiberPriority_t fiberPriority, bool resumeThisRound)
{
    M7FiberSchedulerContext_t* const pContext = DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority];

    // Debug_Log(cLogFiberScheduler, cLogDebug, ("ActivateFiber(context=0x%x, fiberId=%d)\n", pContext, fiberId));

    if (pContext->execBitmap.IsEmpty())
    {
        // Debug_Log(cLogFiberScheduler, cLogDebug, ("ActivateFiber(context=0x%x): Wake Up!\n", pContext));
    }

//    CriticalSection_InstantiateVariables(cCriticalSectionSingleCoreIrq);

//    CriticalSection_Enter(cCriticalSectionSingleCoreIrq);

    const bool isEmpty = pContext->execBitmap.IsEmpty();

    // set execution bit and clear  wait bit: [1, 0]
    pContext->execBitmap.SetFiberId(fiberId);
    pContext->waitBitmap.ClearFiberId(fiberId);

    if (resumeThisRound)
    {
        pContext->algorithm.weightedRoundRobin.execBitmap.SetFiberId(fiberId);
    }

    if (isEmpty)
    {
        // This is the first fiber activated, so set up context
        M7FiberWeightedRoundRobinAlgorithm_Initialize(pContext);
        pContext->current = fiberId;
    }

//    CriticalSection_Leave(cCriticalSectionSingleCoreIrq);
}

/**
 * Deactivates a fiber.
 * The fiber will not longer be called by the fiber scheduler.
 *
 * @param[in ]      fiberId         ID of the fiber to be deactivated
 * @param[in ]      fiberPriority   fiber's priority
 */
static void M7DeactivateFiber(M7CoreId_t core_id, M7FiberId_t fiberId, \
                              M7FiberPriority_t fiberPriority)
{
    M7FiberSchedulerContext_t* const pContext = DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority];

//    CriticalSection_InstantiateVariables(cCriticalSectionSingleCoreIrq);

//    CriticalSection_Enter(cCriticalSectionSingleCoreIrq);

    // clear execution bit and wait bit: [0, 0]
    pContext->execBitmap.ClearFiberId(fiberId);
    pContext->waitBitmap.ClearFiberId(fiberId);
    pContext->algorithm.weightedRoundRobin.execBitmap.ClearFiberId(fiberId);

    if (pContext->current == fiberId)
    {
        pContext->currentFiberStateChanged = true;
    }

//    CriticalSection_Leave(cCriticalSectionSingleCoreIrq);
}

/**
 * Set a fiber in wait state.
 * The fiber will not longer be called by the fiber scheduler.
 *
 * @param[in ]      fiberId     ID of the fiber to be in wait state
 */
static void M7SetFiberInWait(M7CoreId_t core_id, M7FiberId_t fiberId, \
                             M7FiberPriority_t fiberPriority, bool waitThisRound)
{
    M7FiberSchedulerContext_t* const pContext = DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority];;

    // do assert if fiber isn't in wait or active state
    //DEBUG_ASSERT(IsM7FiberActive(pContext, fiberId) == true);

//    CriticalSection_InstantiateVariables(cCriticalSectionSingleCoreIrq);

//    CriticalSection_Enter(cCriticalSectionSingleCoreIrq);

    // clear execution bit and set wait bit: [0, 1]
    pContext->execBitmap.ClearFiberId(fiberId);
    pContext->waitBitmap.SetFiberId(fiberId);

    if (waitThisRound)
    {
        pContext->algorithm.weightedRoundRobin.execBitmap.ClearFiberId(fiberId);
    }

    if (pContext->current == fiberId)
    {
        pContext->currentFiberStateChanged = true;
    }

//    CriticalSection_Leave(cCriticalSectionSingleCoreIrq);
}

#if defined (fps_cpu0Core)
extern fpsCpu0 gFpsCpu0;
#elif defined (fps_cpu1Core)
extern fpsCpu1 gFpsCpu1;
#else
extern fpsCpu2 gFpsCpu2;
#endif
#ifdef IPC_SUPPORT
extern uint32_t gIrqFiber_need_resume;
static void M7checkFiberNeedResume()
{

    if (gIrqFiber_need_resume & (BIT(FP2FPMSG_FIBER)))
    {
        #if defined (fps_cpu0Core)
        gFpsCpu0.CheckFPMsgFiberNeedResume(&gFpsCpu0);
        #elif defined (fps_cpu1Core)
        gFpsCpu1.CheckFPMsgFiberNeedResume(&gFpsCpu1);
        #else
        gFpsCpu2.CheckFPMsgFiberNeedResume(&gFpsCpu2);
        #endif
        gIrqFiber_need_resume &= ~BIT(FP2FPMSG_FIBER);

    } // else do nothing

    #ifdef fps_cpu2Core
    if (gIrqFiber_need_resume & (BIT(CP2FPMSG_FIBER)))
    {
        gFpsCpu2.CheckCP2FPMsgFiberNeedResume(&gFpsCpu2);
        gIrqFiber_need_resume &= ~BIT(CP2FPMSG_FIBER);

    }
    if (gIrqFiber_need_resume & BIT(CDMA_FIBER))
    {
        // gFpsCpu2.CheckFPMsgFiberNeedResume(&gFpsCpu2);
        gFpsCpu2.FpsCpu2CheckCDMAFatalErrorIrq(&gFpsCpu2);
        gIrqFiber_need_resume &= ~BIT(CDMA_FIBER);
    }
    if (gIrqFiber_need_resume & (BIT(RESET_FIBER)))
    {
        gFpsCpu2.FpsCpu2CheckResetIrqCause();
        gIrqFiber_need_resume &= ~BIT(RESET_FIBER);
    }

    #endif
}
#endif

__inline static void M7Run(M7CoreId_t core_id)
{
    // Check if any fiber needs to be woken up
    // M7CheckForIpcQueueEvent();
    M7FiberSchedulerContext_t* pContext = DEF_M7FIBERSCHEDULER_CTX(core_id)[cM7FiberPriorityHigh];


    while (true)
    {


        bool isHighPriorityFiberUsed = (DEF_M7FIBERSCHEDULER_CTX(core_id)[cM7FiberPriorityHigh]->execBitmap.GetValue() != 0) ? true : false;

        // check if needs to switch to low priority context
        if (DEF_M7FIBERSCHEDULER(core_id).isLowPriorityFiberRunning || !isHighPriorityFiberUsed)
        {
            pContext = DEF_M7FIBERSCHEDULER_CTX(core_id)[cM7FiberPriorityLow];
        }

        // PriorityFiberDebugDump(pContext);

        #ifdef IPC_SUPPORT
        if (unlikely(gIrqFiber_need_resume))
        {
            uint32_t irqMasked = __get_PRIMASK();
            __disable_irq();
            M7checkFiberNeedResume();
            if (!irqMasked)
            {
                __enable_irq();
            }
        }
        #else //IPC_SUPPORT
        M7checkFiberNeedResume();
        #endif


        if (pContext->algorithm.weightedRoundRobin.execBitmap.IsFiberIdSet(pContext->current))
        {
            M7Fiber_t& curFiber = pContext->fibers[pContext->current];
            auto pObj = curFiber.pObject;
            auto pFiberCommon = curFiber.fiberFptr;
            auto weight = curFiber.algorithmParams.weightedRoundRobin.weight;
            // Set current running fiber to be unchanged. If fiber state is modified while running
            // this fiber, this variable is set to true, so that we can re-read all the parameters of
            // the fiber
            pContext->currentFiberStateChanged = false;

            // In case if pObj is equal to zero, fiber shall be executed without the context
            if (nullptr == pObj)
            {
                for (uint32_t iter = 0; iter < weight; iter++)
                {
                    pFiberCommon();

                    if (pContext->currentFiberStateChanged)
                    {
                        break;
                    }
                }
            }
            else  // Otherwise, pointer to object has to be forward as parameter
            {
                auto pFiber = reinterpret_cast<M7FiberWithCtxFptr_t>(pFiberCommon);

                for (uint32_t iter = 0; iter < weight; iter++)
                {
                    pFiber(pObj);

                    if (pContext->currentFiberStateChanged)
                    {
                        break;
                    }
                }
            }


        }
        // low priority one fiber passed, switch to high priofiry fiber round
        if (DEF_M7FIBERSCHEDULER(core_id).isLowPriorityFiberRunning && isHighPriorityFiberUsed)
        {
            DEF_M7FIBERSCHEDULER(core_id).isLowPriorityFiberRunning = false;
            pContext = DEF_M7FIBERSCHEDULER_CTX(core_id)[cM7FiberPriorityHigh];

            pContext->current = M7FiberWeightedRoundRobinAlgorithm_Next(pContext);
        }
        else      // keep fiber status in this round or check low priority fiber
        {
            M7FibersBitmap execBitmap = pContext->algorithm.weightedRoundRobin.execBitmap;
            execBitmap.ClearFiberId(pContext->current);

            if (execBitmap.IsEmpty())    // high priority fiber one round passed, switch to low priofiry fiber round
            {
                DEF_M7FIBERSCHEDULER(core_id).isLowPriorityFiberRunning = true;
                pContext = DEF_M7FIBERSCHEDULER_CTX(core_id)[cM7FiberPriorityLow];
            }

            pContext->current = M7FiberWeightedRoundRobinAlgorithm_Next(pContext);
        }

        if (pContext->roundPassedFlag)
        {
            // if scheduler passes one round for all execution fibers, then check for Ipc Queue events
            pContext->roundPassedFlag = false;
            // CheckForIpcQueueEvent(); // check isr or fp_register
        }
    }
}

/**
 * Temporarily suspends the current fiber.
 * The fiber will temporarily not be called by the fiber scheduler and
 * will be resumed next round.
 *
 * @param[in ]  fiberPriority       fiber's priority
 */
__inline static void M7FiberScheduler_SuspendCurrentFiber(M7CoreId_t core_id, \
                                                          M7FiberPriority_t fiberPriority)
{
// #if (_CPU_ != CPU_SHARED)
//    extern M7FiberSchedulerContext_t*     DEF_M7FIBERSCHEDULER_CTX(core_id)[cM7NumFiberPriority];
    M7FiberSchedulerContext_t*    pContext = DEF_M7FIBERSCHEDULER_CTX(core_id)[fiberPriority];

    // setting currentFiberStateChanged flag breaks current scheduller's round
    pContext->currentFiberStateChanged = true;
// #endif
}

/**
 * Returns the current fiber.
 *
 * @param[in ]      pContent    scheduler context
 * @return                      the current fiber.
 */
__inline static M7FiberId_t M7FiberScheduler_GetCurrent( \
    const M7FiberSchedulerContext_t* pContext)
{
    return pContext->current;
}

/**
 * Dumps the fiber scheduler context.
 *
 * @param[in ]  fiberPriority       fiber's priority
 */
