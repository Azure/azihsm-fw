/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    regrw.h

Abstract:

    Function declarations and definitions to simplify
    register/memory access and memory operations.
    Ported and modified from Xbox HSP.

Author:

    Navin Pai (navinp)
    Timothy Prinz (tiprinz)

--*/

#pragma once

#include "dmb.h"

HSP_API
INLINE void HspWriteRegister32(pvuint32_t Register, uint32_t Value)
/*++

    Write value to the register

--*/
{
    *(pvuint32_t)Register = Value;
}


HSP_API
#ifdef __clang__
static INLINE uint32_t HspReadRegister32(pvuint32_t Register)
#else
// GCC doesn't like the static function used from a non-static inline function.
INLINE uint32_t HspReadRegister32(pvuint32_t Register)
#endif
/*++

    Read value from the register

--*/
{
    return *(pvuint32_t)Register;
}


HSP_API
static INLINE void HspRmwRegister32(pvuint32_t Register,
                                    uint8_t HighBit,
                                    uint8_t LowBit,
                                    uint32_t Value)
/*++

    Generic Read/Modify/Write a register, using bit range and value
    within the bit range, Basically, implement
    RegAddr[HighBit:LowBit] = Value.

--*/
{
    uint32_t regValue = HspReadRegister32(Register);
    uint32_t mask = 0xFFFFFFFF << (31 - HighBit) >> (31 - HighBit + LowBit)
                                                        << LowBit;
    uint32_t maskedValue = (Value << LowBit) & mask;

    regValue &= ~mask;
    regValue |= maskedValue;
    HspWriteRegister32(Register, regValue);
}


HSP_API
static INLINE void HspWriteGlobalRegister32(uint64_t GlobalAddress, uint32_t Data)
/*++

Description:

    Write to global address via dmb.

Arguments:

    GlobalAddress - Global address to write to

    Data - Data to write to global address

--*/
{
    uint32_t hspLocalAddr;

    if (SP_SUCCEEDED(HspDmbMap(GlobalAddress,
                               HspDmbSegmentWindowSize128MiB,
                               &hspLocalAddr)))
    {
        HspWriteRegister32((puint32_t)hspLocalAddr, Data);
        HspDmbUnmap(hspLocalAddr);
    }
}


HSP_API
static INLINE uint32_t HspReadGlobalRegister32(uint64_t GlobalAddress)
/*++

Description:

    Read from global address via dmb.

Arguments:

    GlobalAddress - Global address to read from

--*/
{
    uint32_t retVal = 0;
    uint32_t hspLocalAddr;

    if (SP_SUCCEEDED(HspDmbMap(GlobalAddress,
                               HspDmbSegmentWindowSize128MiB,
                               &hspLocalAddr)))
    {
        retVal = HspReadRegister32((puint32_t)hspLocalAddr);
        HspDmbUnmap(hspLocalAddr);
    }

    return retVal;
}
