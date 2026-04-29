/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    timercommon.h

Abstract:

    Define common api shared between different timer implementations

Author:

    Peng Li (pengfeli)

--*/

#pragma once

#if defined(FLAVOR_FPGA)
#define TICKS_PER_1USEC 20             // FPGA is running at 20MHz
#define TICKS_PER_1MSEC (20 * 1000)    // Ticks per 1 milli secs
#else
#define TICKS_PER_1USEC RISCV_HSP_CLK_MHZ           // Ticks per 1 micro secs
#define TICKS_PER_1MSEC (TICKS_PER_1USEC * 1000)    // Ticks per 1 milli secs
#endif


void ClearTimer0Inst();

bool IsTimer0InstSet();

//
// Time related functions RiscvGetCpuTime (no interrupts)
//
#define EXPIRATION_TIME_INFINITE 0
#define MICROSECOND_CLK_KHZ      1000
#define MICROSECOND_RTC_CLK_RATIO \
    (1 + (((RISCV_RTC_CLK_KHZ << 16) - 1) / MICROSECOND_CLK_KHZ))
#define RTC_CLK_MICROSECOND_RATIO \
    (1 + (((MICROSECOND_CLK_KHZ << 16) - 1) / RISCV_RTC_CLK_KHZ))
#define MAX_TIMEOUT_CPUTIME (((uint64_t)-1) / MICROSECOND_RTC_CLK_RATIO)
#define MAX_TIMEOUT_USECS   (((uint64_t)-1) / RTC_CLK_MICROSECOND_RATIO)

INLINE
uint64_t HspMicrosecondsToCpuTimeTicks(uint64_t MicroSeconds)
/*++

Description:

    Computes how many RTC CLK periods is equivalent to microseconds

Arguments:

    MicroSeconds - The MicroSeconds value to convert

Returns:

    Converted time in RTC CLK units, or 0 if it's over the limit (numerical
    convertion)

--*/
{
    // check for overflow
    if (MicroSeconds > MAX_TIMEOUT_USECS)
    {
        return 0;
    }

    // convert from MicroSeconds to CPU Time (HSP RTC CLK)
    uint64_t const duration = (MicroSeconds * MICROSECOND_RTC_CLK_RATIO) >> 16;

    return duration;
}

HSP_API
INLINE
bool HspIsTimeExpired(uint64_t ExpirationTime)
/*++

Description:

    Test if current CPU Time is greater than ExpirationTime.

Arguments:

    ExpirationTime - Point in time based on HSP_RTC_CLK.

Returns:

    true if the the timeout expires, false if it doesn't.
    If ExpirationTime is EXPIRATION_TIME_INFINITE, it always returns false.

--*/
{
    // ExpirationTime == 0 (EXPIRATION_TIME_INFINITE) means no expiration time
    return (ExpirationTime && ExpirationTime <= RiscvGetCpuTime());
}

HSP_API
INLINE
uint64_t HspGetCpuTimeInFutureUs(uint32_t MicroSeconds)
/*++

Description:

    Gets current CPU time + duration in microseconds.

Returns:

    Gets time in the future in CPU Time ticks, or 0 if MicroSeconds is invalid.

--*/
{
    uint64_t const durationTicks = HspMicrosecondsToCpuTimeTicks(MicroSeconds);
    if (durationTicks)
    {
        return RiscvGetCpuTime() + durationTicks;
    }

    return 0;
}

HSP_API
INLINE
uint64_t HspGetCpuTimeInFutureMs(uint32_t MiliSeconds)
/*++

Description:

    Gets current CPU time + duration in miliseconds.

Returns:

    Gets time in the future in CPU Time ticks, or 0 if MiliSeeconds is invalid.

--*/
{
    uint64_t const durationTicks = HspMicrosecondsToCpuTimeTicks(
        (uint64_t)MiliSeconds * 1000ULL);
    if (durationTicks)
    {
        return RiscvGetCpuTime() + durationTicks;
    }

    return 0;
}
