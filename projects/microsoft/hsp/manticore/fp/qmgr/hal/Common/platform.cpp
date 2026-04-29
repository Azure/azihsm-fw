// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   Platform.cpp
//! @brief  Platform
//=============================================================================
#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

// #include "platform.h"
// #include "../../FP3Core/ldscripts/M7MemMap.h"

//-----------------------------------------------------------------------------
//  Static Member Variable Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Public Data Type Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Interface Function Definitions
//-----------------------------------------------------------------------------

/**
 *  @brief  Use UCD register name and core id to get corresponding UCD register address.
 *
 *  @param[in]  name      UCD register name.
 *  @param[in]  CoreId     Core Id.
 *                                  For core id based register (cUcdRegIb, cUcdRegIbIq, cUcdRegOb, and cUcdRegObOq), input core id 0 or 1.
 *                                  For not core id based register (otherwise), input core id 0xff.
 *  @return       uint32_t   UCD register address.
 *
 */

#ifdef LIONPERF_SUPPORT
uint32_t GetUcdRegCoreBase(enum PlatformRegName_t name, uint8_t CoreId)
{
    switch (name)
    {
        case cUcdRegBarCpu:
            return ((uint32_t)UCD_CPU_CREG_PF_ADDR_START);
            break;
        case cUcdRegVfBarCpu:
            return ((uint32_t)UCD_CPU_CREG_VF_BASE_ADDR_START);
            break;
        case cUcdRegCmn:
            return ((uint32_t)(UCD_CMN_REGS_ADDR));
            break;
        case cUcdRegIqLgc2phys:
            return ((uint32_t)(UCD_CMN_IB_LGC2PHYS_REGS_ADDR));
            break;
        case cUcdRegOqLgc2phys:
            return ((uint32_t)(UCD_CMN_OB_LGC2PHYS_REGS_ADDR));
            break;
        case cUcdRegIb:
            return ((uint32_t)(UCD_IB_REGS_ADDR) + (uint32_t)(CoreId * UCD_IBOB_CORE_OFFSET));
            break;
        case cUcdRegIbIq:
            return ((uint32_t)(UCD_IB_REGS_ADDR + UCD_IBOB_IQOQ_OFFSET) + (uint32_t)(CoreId * UCD_IBOB_CORE_OFFSET));
            break;
        case cUcdRegOb:
            return ((uint32_t)(UCD_OB_REGS_ADDR) + (uint32_t)(CoreId * UCD_IBOB_CORE_OFFSET));
            break;
        case cUcdRegObOq:
            return ((uint32_t)(UCD_OB_REGS_ADDR + UCD_IBOB_IQOQ_OFFSET) + (uint32_t)(CoreId * UCD_IBOB_CORE_OFFSET));
            break;
        case cUcdRegBarHost:
            return ((uint32_t)UCD_HOST_CREG_PF_ADDR_START);
            break;
        case cUcdRegVfBarHost:
            return ((uint32_t)UCD_HOST_CREG_VF_ADDR_START);
            break;
    }
    return 0;
}
#endif
