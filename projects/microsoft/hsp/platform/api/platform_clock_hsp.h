// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef PLATFORM_CLOCK_HSP_H_
#define PLATFORM_CLOCK_HSP_H_

#include <stdint.h>


/* Configurable RoT parameters.  Defaults can be overridden in platform_config.h. */
#include "platform_config.h"
#ifndef HSP_CLOCK_FREQUENCY_HZ
#error "Need to specify the frequency of the HSP clock."
#endif

#ifndef RISCV_RTC_FREQUENCY_HZ
#define	RISCV_RTC_FREQUENCY_HZ	(HSP_CLOCK_FREQUENCY_HZ / 32)
#endif

#ifndef HSP_RTC_FREQUENCY_HZ
#define	HSP_RTC_FREQUENCY_HZ    \
		(1U << CREG_REGS_CREG_RTC_GROUP_COUNT_TO_SECOND_COUNT_TO_SECOND_WIDTH)
#endif


/* The system time will just use the mtime ticks. */
typedef uint64_t platform_clock;


uint64_t platform_get_mtime ();
uint64_t platform_get_offset_mtime (uint32_t msec);


#endif	/* PLATFORM_CLOCK_HSP_H_ */
