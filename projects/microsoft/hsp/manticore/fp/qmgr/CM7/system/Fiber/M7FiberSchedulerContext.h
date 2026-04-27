// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   M7FiberSchedulerContext.h
//! @brief  M7 Fiber Scheduler Common code
//!
//=============================================================================
#ifndef FP3CORE_SYSTEM_FIBER_M7FIBERSCHEDULERCONTEXT_H_
#define FP3CORE_SYSTEM_FIBER_M7FIBERSCHEDULERCONTEXT_H_
#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "M7FiberDataTypes.h"
#include "M7Partition.h"

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
 * Creates an instance of a fiber context.
 *
 * @param[in ]  numberOfFibers  number of supported fibers
 * @return                      the created fiber context instance.
 */
M7FiberSchedulerContext_t* M7FiberSchedulerContext_CreateInstance( \
    M7CoreId_t core_id, int numberOfFibers, M7FiberPriority_t priority);

#endif  // FP3CORE_SYSTEM_FIBER_M7FIBERSCHEDULERCONTEXT_H_
