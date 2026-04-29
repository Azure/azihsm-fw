/*++

    Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    postcode.h

Abstract:

    Functions to update current HSP status, whether informational status or
fatal error.

Author:

    Timothy Prinz (tiprinz)

--*/

#pragma once

//
// max is 256 cycle on 500mhz (2 ns per cycle)
// so max delay is 0.5us
//
#define HSP_STALL_MAXIMUM_BITS 8


#define HSP_SUSPEND_STATUS_IF_FAILED(x)   \
    {                                     \
        HSP_STATUS _status = (x);         \
        if (UNLIKELY(SP_FAILED(_status))) \
        {                                 \
            HspSuspend(_status);          \
        }                                 \
    }

HSP_API
void HspPostCodeInternal(HSP_STATUS Code, bool AddToChkpt);


HSP_API
static INLINE void HspPostCode(HSP_STATUS Code)
{
    HspPostCodeInternal(Code, false);
}


HSP_API
static INLINE void HspPostCodeChkpt(HSP_STATUS Code)
{
    HspPostCodeInternal(Code, true);
}

//
// Fatal status output function to suspend HSP
//
NORETURN
void HspSuspendInternal(HSP_STATUS Status, uint32_t Reason);


HSP_API
NORETURN
static INLINE void HspSuspend(HSP_STATUS Status)
{
    HspSuspendInternal(Status, 0);
}


HSP_API
NORETURN
static INLINE void HspSuspendWithReason(HSP_STATUS Status, uint32_t Reason)
{
    HspSuspendInternal(Status, Reason);
}


HSP_API
void HspWarmReset(HSP_STATUS Status);
