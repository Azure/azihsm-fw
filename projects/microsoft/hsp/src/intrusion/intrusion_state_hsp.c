// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include <string.h>
#include "common/type_cast.h"
#include "common/unused.h"
#include "intrusion/intrusion_state_hsp.h"


int intrusion_state_hsp_set (struct intrusion_state *intrusion)
{
	const struct intrusion_state_hsp *intrusion_hsp = (const struct intrusion_state_hsp*) intrusion;

	if (intrusion == NULL) {
		return INTRUSION_STATE_INVALID_ARGUMENT;
	}

	/* Set the intrusion state to "intrusion detected" by clearing the
	 * intrusion magic number register. */
	return intrusion_hsp->rtc->set_intrusion_magic_number (intrusion_hsp->rtc, 0);
}

int intrusion_state_hsp_clear (struct intrusion_state *intrusion)
{
	const struct intrusion_state_hsp *intrusion_hsp = (const struct intrusion_state_hsp*) intrusion;
	int status;

	if (intrusion == NULL) {
		return INTRUSION_STATE_INVALID_ARGUMENT;
	}

	/* Clear the intrusion state to "no intrusion" by setting the
	  * intrusion magic number register to the expected number and
	  * clear intrusion count. */
	status = intrusion_hsp->rtc->set_intrusion_magic_number (intrusion_hsp->rtc,
		REAL_TIME_CLOCK_HSP_INTRUSION_MAGIC_NUMBER);
	if (status != 0) {
		return status;
	}

	status = intrusion_hsp->rtc->clear_intrusion_count (intrusion_hsp->rtc);

	return status;
}

int intrusion_state_hsp_check (struct intrusion_state *intrusion)
{
	const struct intrusion_state_hsp *intrusion_hsp = (const struct intrusion_state_hsp*) intrusion;
	int status;
	uint32_t intrusion_magic_number = 0;

	if (intrusion == NULL) {
		return INTRUSION_STATE_INVALID_ARGUMENT;
	}

	/* Check the HSP intrusion state. If "intrusion detected", return 1. */
	status = intrusion_hsp->rtc->get_intrusion_magic_number (intrusion_hsp->rtc,
		&intrusion_magic_number);
	if (status != 0) {
		return status;
	}

	/* "intrusion detected". */
	if (intrusion_magic_number != REAL_TIME_CLOCK_HSP_INTRUSION_MAGIC_NUMBER) {
		status = 1;
	}

	return status;
}

int intrusion_state_hsp_is_intrusion_active (struct intrusion_state *intrusion,	uint32_t *state)
{
	const struct intrusion_state_hsp *intrusion_hsp = (const struct intrusion_state_hsp*) intrusion;
	int status;

	if ((intrusion == NULL) || (state == NULL)) {
		return INTRUSION_STATE_INVALID_ARGUMENT;
	}

	status = intrusion_hsp->rtc->get_intrusion_state (intrusion_hsp->rtc, state);

	// Once the HW RTC_STS bit0 is set, no active intrusion; otherwise active intrusion.
	// We should reverst it for humanbeing logic.
	if (status == 0) {
		*state = (*state & 0x1) ? 0 : 1;
	}

	return status;
}

int intrusion_state_hsp_get_intrusion_count (struct intrusion_state *intrusion,	uint32_t *count)
{
	const struct intrusion_state_hsp *intrusion_hsp = (const struct intrusion_state_hsp*) intrusion;

	if ((intrusion == NULL) || (count == NULL)) {
		return INTRUSION_STATE_INVALID_ARGUMENT;
	}

	return intrusion_hsp->rtc->get_intrusion_count (intrusion_hsp->rtc, count);
}

/**
 * Initialize the HSP intrusion data structure.
 *
 * @param intrusion The HSP intrusion state instance being tracked.
 * @param rtc The HSP RTC driver.
 *
 * @return 0 if initialization successful or an error code.
 */
int intrusion_state_hsp_init (struct intrusion_state_hsp *intrusion,
	const struct real_time_clock_hsp *rtc)
{
	int status = 0;

	if ((intrusion == NULL) || (rtc == NULL)) {
		return INTRUSION_STATE_INVALID_ARGUMENT;
	}

	// Set all fields to a known value prior to assignment.
	memset (intrusion, 0, sizeof (struct intrusion_state_hsp));

	intrusion->base.clear = intrusion_state_hsp_clear;
	intrusion->base.set = intrusion_state_hsp_set;
	intrusion->base.check = intrusion_state_hsp_check;
	intrusion->base.is_active = intrusion_state_hsp_is_intrusion_active;
	intrusion->base.get_intrusion_count = intrusion_state_hsp_get_intrusion_count;

	intrusion->rtc = rtc;

	return status;
}

/**
 * Release any memory associated with the HSP intrusion state.
 *
 * @param intrusion The intrusion state instance being released.
 */
void intrusion_state_hsp_release (struct intrusion_state *intrusion)
{
	UNUSED (intrusion);
}
