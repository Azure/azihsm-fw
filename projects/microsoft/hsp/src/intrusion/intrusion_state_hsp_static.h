// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INTRUSION_STATE_HSP_STATIC_H_
#define INTRUSION_STATE_HSP_STATIC_H_

#include "intrusion/intrusion_state_hsp.h"


/* Internal functions declared to allow for static initialization. */
int intrusion_state_hsp_set (struct intrusion_state *intrusion);
int intrusion_state_hsp_clear (struct intrusion_state *intrusion);
int intrusion_state_hsp_check (struct intrusion_state *intrusion);
int intrusion_state_hsp_is_intrusion_active (struct intrusion_state *intrusion,	uint32_t *state);
int intrusion_state_hsp_get_intrusion_count (struct intrusion_state *intrusion,	uint32_t *count);

/**
 * Constant initializer for the intrusion state API.
 */
#define	INTRUSION_STATE_HSP_API_INIT  { \
		.clear = intrusion_state_hsp_clear, \
		.set = intrusion_state_hsp_set, \
		.check = intrusion_state_hsp_check, \
		.get_intrusion_count = intrusion_state_hsp_get_intrusion_count, \
		.is_active = intrusion_state_hsp_is_intrusion_active \
	}

/**
 * Initialize a static handler for HSP RTC interrupts.
 *
 * There is no validation done on the arguments.
 *
 * @param rtc_ptr The instance of HSP RTC driver.
 */
#define	intrusion_state_hsp_static_init(rtc_ptr) { \
		.base = INTRUSION_STATE_HSP_API_INIT, \
		.rtc = rtc_ptr, \
	}


#endif	/* INTRUSION_STATE_HSP_STATIC_H_ */
