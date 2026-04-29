// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef REAL_TIME_CLOCK_HSP_STATIC_H_
#define REAL_TIME_CLOCK_HSP_STATIC_H_

#include <stddef.h>
#include "system/real_time_clock_hsp.h"
#include "system/real_time_clock_static.h"


/* Internal functions declared to allow for static initialization. */

int real_time_clock_hsp_get_time (const struct real_time_clock *rtc, uint64_t *msec);
int real_time_clock_hsp_set_time (const struct real_time_clock *rtc, uint64_t msec);
int real_time_clock_hsp_get_intrusion_count (const struct real_time_clock_hsp *rtc,
	uint32_t *intrusion_count);
int real_time_clock_hsp_clear_intrusion_count (const struct real_time_clock_hsp *rtc);
int real_time_clock_hsp_get_intrusion_state (const struct real_time_clock_hsp *rtc,
	uint32_t *intrusion_state);
int real_time_clock_hsp_get_intrusion_magic_number (const struct real_time_clock_hsp *rtc,
	uint32_t *intrusion_state);
int real_time_clock_hsp_set_intrusion_magic_number (const struct real_time_clock_hsp *rtc,
	uint32_t intrusion_state);
int real_time_clock_hsp_get_interrupt_status (const struct real_time_clock_hsp *rtc,
	uint32_t *int_status);
int real_time_clock_hsp_clear_interrupt_status (const struct real_time_clock_hsp *rtc,
	uint32_t int_status);
int real_time_clock_hsp_get_interrupt_enable (const struct real_time_clock_hsp *rtc,
	uint32_t *enable);
int real_time_clock_hsp_set_interrupt_enable (const struct real_time_clock_hsp *rtc,
	uint32_t enable);


/* Static initializer API. */

#define REAL_TIME_CLOCK_HSP_STATIC_METHODS_INIT \
		.get_intrusion_count = real_time_clock_hsp_get_intrusion_count, \
		.clear_intrusion_count = real_time_clock_hsp_clear_intrusion_count, \
		.get_intrusion_state = real_time_clock_hsp_get_intrusion_state, \
		.get_intrusion_magic_number = real_time_clock_hsp_get_intrusion_magic_number, \
		.set_intrusion_magic_number = real_time_clock_hsp_set_intrusion_magic_number, \
		.get_interrupt_status = real_time_clock_hsp_get_interrupt_status, \
		.clear_interrupt_status = real_time_clock_hsp_clear_interrupt_status, \
		.get_interrupt_enable = real_time_clock_hsp_get_interrupt_enable, \
		.set_interrupt_enable = real_time_clock_hsp_set_interrupt_enable \

/**
 * Initializes the API for a static instance of an HSP real time clock that supports
 * get intrusion count, get intrusion state, and both get time and set time.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr The variable state context.
 * @param regs_ptr The RTC HW registers.
 * @param ms_ready_timeout The timeout in milliseconds to wait for the HW to be ready for access.
 * A value of 0 will immediately fail if the HW is not ready and a value of UINT32_MAX will block
 * indefinitely until the HW is ready.
 */
#define real_time_clock_hsp_static_init(state_ptr, regs_ptr, ms_ready_timeout) { \
		.base = real_time_clock_static_init (real_time_clock_hsp_get_time, \
			real_time_clock_hsp_set_time), \
		REAL_TIME_CLOCK_HSP_STATIC_METHODS_INIT, \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.ready_timeout_ms = ms_ready_timeout, \
	}

/**
 * Initializes the API for a static instance of an HSP real time clock that supports
 * intrustion get count, get state, get time but not set time.
 *
 * There is no validation done on the arguments.
 *
 * @param regs_ptr The RTC HW registers.
 * @param ms_ready_timeout The timeout in milliseconds to wait for the HW to be ready for access.
 * A value of 0 will immediately fail if the HW is not ready and a value of UINT32_MAX will block
 * indefinitely until the HW is ready.
 */
#define real_time_clock_hsp_static_init_no_set_time(regs_ptr, ms_ready_timeout) { \
		.base = real_time_clock_static_init (real_time_clock_hsp_get_time, \
			real_time_clock_set_time_unsupported), \
		REAL_TIME_CLOCK_HSP_STATIC_METHODS_INIT, \
		.state = NULL, \
		.regs = regs_ptr, \
		.ready_timeout_ms = ms_ready_timeout, \
	}


#endif	/* REAL_TIME_CLOCK_HSP_STATIC_H_ */
