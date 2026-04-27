// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   M7Partition.h
//! @brief  M7 Partition definitions for Fiber Scheduler.
//!
//=============================================================================
#ifndef FP3CORE_SYSTEM_BOOT_M7PARTITION_H_
#define FP3CORE_SYSTEM_BOOT_M7PARTITION_H_
#pragma once
#define IPC_SUPPORT

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Public Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Public Data Type Definitions
//-----------------------------------------------------------------------------

/// Firmware Partition Ids
typedef enum
{
    cM7CompGroupIo = 0,
    cM7CompGroupLog,
    cM7NumberOfCompGroups
} M7CompGroupId_t;

/// @brief  Core ID types
typedef enum M7CoreId_t
{
    cM7Core0 = 0,                     /// < Core 0
    cM7Core1,                         /// < Core 1
    cM7Core2,                         /// < Core 2
    cM7NumberOfCores                  /// < Number of cores
} M7CoreId_t;


//-----------------------------------------------------------------------------
//  Public Interface Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Inline Functions
//-----------------------------------------------------------------------------

#endif  // FP3CORE_SYSTEM_BOOT_M7PARTITION_H_
