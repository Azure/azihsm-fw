// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Marvell

//=============================================================================
//
//! @file  Heartbeat.h
//! @brief  System Heartbeat API & MACRO.
//!
//=============================================================================

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------
#include "M7MemMap.h"
#include "LoggingDebug.h"
#include "LoggingDebugCategory.h"
#include "FpCommon.h" // fps reg

/**
 *  @brief   Entry point function for FpsCpu2CheckHeartbeatFiber
 *
 *  @param   void* pObj - pointer to fpsCpu2 object
 *
 *  @return  None
 */
void CheckHeartbeatFiber(void* pObj);
