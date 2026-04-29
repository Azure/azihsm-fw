// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include "platform_api.h"
#include "common/unused.h"


void platform_msleep (uint32_t msec)
{
	uint64_t done;

	done = platform_get_offset_mtime (msec);
	while (platform_get_mtime () < done) {
	}
}

int platform_timer_create (platform_timer *timer, timer_callback callback, void *context)
{
	if ((timer == NULL) || (callback == NULL)) {
		return PLATFORM_TIMER_ERROR (PLATFORM_INVALID_ARGUMENT);
	}

	UNUSED (context);

	return PLATFORM_TIMER_ERROR (PLATFORM_FAILURE);
}

int platform_timer_arm_one_shot (platform_timer *timer, uint32_t ms_timeout)
{
	if ((timer == NULL) || (ms_timeout == 0)) {
		return PLATFORM_TIMER_ERROR (PLATFORM_INVALID_ARGUMENT);
	}

	return PLATFORM_TIMER_ERROR (PLATFORM_FAILURE);
}

int platform_timer_disarm (platform_timer *timer)
{
	if (timer == NULL) {
		return PLATFORM_TIMER_ERROR (PLATFORM_INVALID_ARGUMENT);
	}

	return PLATFORM_TIMER_ERROR (PLATFORM_FAILURE);
}

void platform_timer_delete (platform_timer *timer)
{
	UNUSED (timer);
}
