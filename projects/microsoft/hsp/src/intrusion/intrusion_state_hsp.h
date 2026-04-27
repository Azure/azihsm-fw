// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INTRUSION_STATE_HSP_H_
#define INTRUSION_STATE_HSP_H_

#include "intrusion/intrusion_state.h"
#include "system/real_time_clock_hsp.h"


/**
 * Manage the HSP device's intrusion state. Intrusions are tracked as a means of improving
 * device security.
 */
struct intrusion_state_hsp {
	struct intrusion_state base;			/**< The intrusion base state. */
	const struct real_time_clock_hsp *rtc;	/**< The RTC generating the interrupts. */
};


int intrusion_state_hsp_init (struct intrusion_state_hsp *intrusion,
	const struct real_time_clock_hsp *rtc);
void intrusion_state_hsp_release (struct intrusion_state *intrusion);


#endif	/* INTRUSION_STATE_HSP_H_ */
