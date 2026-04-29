// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @brief UCD Registers
//!
//=============================================================================

// Generated with Dullahan v2.1.2.db897eb

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "SysTypes.h"
#include "assert.h"

//-----------------------------------------------------------------------------
//  Public Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Public Data Type Definitions
//-----------------------------------------------------------------------------

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t LOGICAL_ADDR_OF_OQ_CONSUMER_INDX : 20;    ///<BIT [19:0] logical_addr_of_oq_consumer_indx
        uint32_t RSVD_0                      : 11;    ///<BIT [30:20] rsvd_0
        uint32_t LOGICAL_ADDR_INVALID        : 1;     ///<BIT [31] logical_addr_invalid
    } b;
} UcdGenCmnObLgc2physOqLogicalToPhysicalAssignment_t;

/// @brief 0x102000
typedef struct
{
    UcdGenCmnObLgc2physOqLogicalToPhysicalAssignment_t ucdGenCmnObLgc2physOqLogicalToPhysicalAssignment; //ucd_gen_cmn_ob_lgc2phys_reg_oq_logical_to_physical_assignment
} UcdGenCmnObLgc2physRegisters_t; ///< 0x0 - 0x420

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
