// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   M7FiberSchedulerContext.cpp
//! @brief  M7 Fiber Scheduler Common code
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "M7MemMap.h"
#include "MemorySection.h"
MODULE_SECTION_NORMAL
    MODULE_OPTIMIZE_FOR_MAX_SPEED

#include "M7FiberSchedulerContext.h"
#include "M7FiberWeightedRoundRobinAlgorithm.h"
#include "MemIo.h"
M7FiberSchedulerContext_t gM7FiberCtx[cM7NumFiberPriority];

//-----------------------------------------------------------------------------
//  Inline function(s)
//-----------------------------------------------------------------------------

M7FiberSchedulerContext_t* M7FiberSchedulerContext_CreateInstance( \
    M7CoreId_t core_id, int numberOfFibers, M7FiberPriority_t priority)
{

//  M7FiberSchedulerContext_t* pContext = (M7FiberSchedulerContext_t*)((uint8_t *)FCP_DTCM_MMIO_START + FCP_DTCM_MMIO_SIZE);  // M7MEM_AllocOnlyDtcm((sizeof(M7FiberSchedulerContext_t) + (sizeof(M7Fiber_t) * (uint32_t)M7FibersBitmap::_maxFiberCount)), 4);
    M7FiberSchedulerContext_t* pContext = &gM7FiberCtx[priority];

    MEM_CLR(pContext, sizeof(M7FiberSchedulerContext_t));

    pContext->numberOfFibers  = numberOfFibers;


    M7FiberWeightedRoundRobinAlgorithm_Initialize(pContext);

    return pContext;
}
