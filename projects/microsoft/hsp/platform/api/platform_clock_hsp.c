// Copyright (c) Microsoft Corporation. All rights reserved.

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include "hsp_top.h"
#include "platform_api.h"
#include "platform_clock_hsp.h"
#include "common/clock_utils.h"
#include "splibs/hsprt/riscvcpu.h"
#include "splibs/inc/spstatus.h"
#include "splibs/inc/sptypes.h"


/**
 * Convert milliseconds to mtime counter ticks.  Always round the result up, ensuring that no less
 * than the requested time is represented in ticks.
 */
#define	RISCV_MS_TO_MTIME(x)		((((uint64_t) (x) * RISCV_RTC_FREQUENCY_HZ) + 999) / 1000)


/**
 * Calculates the elapsed time in milliseconds between a start and stop MTIME stamp.
 *
 * @param end A pointer to the current MTIME stamp.
 * @param start A pointer to the first MTIME stamp.
 *
 * @return The amount of milliseconds that has elapsed.
 */
static uint64_t mtime_elapsed_ms (const uint64_t *end, const uint64_t *start)
{
	const uint32_t tick_rate = RISCV_RTC_FREQUENCY_HZ;
	uint64_t elapsed = *end - *start;

	return CLOCK_TICKS_TO_MS (elapsed, tick_rate);
}

/**
 * Get the current value from the mtime RTC register in the RISC-V.
 *
 * @return The current RTC count.
 */
uint64_t platform_get_mtime ()
{
	volatile uint32_t *const mtime_lsb = (volatile uint32_t*) (RISCV_CLIC_HART0_MTIME_OFFSET);
	volatile uint32_t *const mtime_msb = (volatile uint32_t*) (RISCV_CLIC_HART0_MTIMEH_OFFSET);
	uint32_t lsb;
	uint32_t msb;

	do {
		msb = *mtime_msb;
		lsb = *mtime_lsb;
	} while (msb != *mtime_msb);	// Handle overflow scenario of the lower 32 bits

	return ((uint64_t) msb << 32) | lsb;
}

/**
 * Get the value of mtime a specified number of milliseconds in the future.
 *
 * @param msec The number of milliseconds to add to the current mtime value.
 *
 * @return The future mtime value.
 */
uint64_t platform_get_offset_mtime (uint32_t msec)
{
	uint64_t mtime;

	mtime = RISCV_MS_TO_MTIME (msec);
	mtime += platform_get_mtime ();

	return mtime;
}

int platform_init_timeout (uint32_t msec, platform_clock *timeout)
{
	if (timeout == NULL) {
		return PLATFORM_TIMEOUT_ERROR (PLATFORM_INVALID_ARGUMENT);
	}

	*timeout = platform_get_offset_mtime (msec);

	return 0;
}

int platform_increase_timeout (uint32_t msec, platform_clock *timeout)
{
	if (timeout == NULL) {
		return PLATFORM_TIMEOUT_ERROR (PLATFORM_INVALID_ARGUMENT);
	}

	*timeout += RISCV_MS_TO_MTIME (msec);

	return 0;
}

int platform_has_timeout_expired (const platform_clock *timeout)
{
	uint64_t mtime;

	if (timeout == NULL) {
		return PLATFORM_TIMEOUT_ERROR (PLATFORM_INVALID_ARGUMENT);
	}

	mtime = platform_get_mtime ();
	if (*timeout > mtime) {
		return 0;
	}

	return 1;
}

int platform_get_timeout_remaining (const platform_clock *timeout, uint32_t *msec)
{
	uint64_t elapsed;

	if ((timeout == NULL) || (msec == NULL)) {
		return PLATFORM_TIMEOUT_ERROR (PLATFORM_INVALID_ARGUMENT);
	}

	elapsed = platform_get_mtime ();
	if (*timeout <= elapsed) {
		*msec = 0;
	}
	else {
		elapsed = mtime_elapsed_ms (timeout, &elapsed);
		if (elapsed > UINT32_MAX) {
			return PLATFORM_TIMEOUT_ERROR (PLATFORM_FAILURE);
		}

		*msec = (uint32_t) elapsed;
	}

	return 0;
}

int platform_init_current_tick (platform_clock *currtime)
{
	if (currtime == NULL) {
		return PLATFORM_TIMEOUT_ERROR (PLATFORM_INVALID_ARGUMENT);
	}

	*currtime = platform_get_mtime ();

	return 0;
}

uint32_t platform_get_duration (const platform_clock *start, const platform_clock *end)
{
	uint64_t elapsed;

	if ((end == NULL) || (start == NULL)) {
		return 0;
	}

	elapsed = mtime_elapsed_ms (end, start);
	if (elapsed > UINT32_MAX) {
		return UINT32_MAX;
	}

	/* TODO:  May want to define a different function for calculating durations that may be less
	 * than 1ms (e.g. platform_get_us_duration). */
	return (uint32_t) elapsed;
}
