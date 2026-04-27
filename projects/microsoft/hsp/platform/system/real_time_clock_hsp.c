// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <string.h>
#include "hsp_top.h"
#include "platform_clock_hsp.h"
#include "logging/debug_log.h"
#include "logging/hsp_logging.h"
#include "system/real_time_clock_hsp.h"


/**
 * The timeout in milliseconds to delay sequential calls to set_time.
 */
#define REAL_TIME_CLOCK_HSP_COOLDOWN_DELAY_MS		1000

/**
 * The bitmask that indicates if the RTC HW is ready to have its time read.
 */
#define RTC_READ_READY_MASK		(\
	CREG_REGS_CREG_RTC_GROUP_RTC_READ_READY_READ_READY_RTCCLK_FIELD_MASK | \
	CREG_REGS_CREG_RTC_GROUP_RTC_READ_READY_READ_READY_SECCLK_FIELD_MASK)

/**
 * The bitmask that indicates if the RTC HW is ready to have its time set.
 */
#define RTC_WRITE_READY_MASK		(\
	CREG_REGS_CREG_RTC_GROUP_RTC_WRITE_READY_WRITE_READY_RTCCLK_FIELD_MASK |\
	CREG_REGS_CREG_RTC_GROUP_RTC_WRITE_READY_WRITE_READY_SECCLK_FIELD_MASK)


/**
 * Wait for the RTC HW to signal that it's ready for a read/write operation.
 *
 * @param rtc The real time clock instance.
 * @param reg The RTC_*_READY register
 * @param mask The bitmask value that reg will match when ready.
 *
 * @return 0 if successful, 1 if it timed out, or an error code.
 */
static int real_time_clock_hsp_wait_ready (const struct real_time_clock_hsp *rtc,
	volatile uint32_t *reg, uint32_t mask)
{
	int status = 1;
	platform_clock timeout;

	/* The RTC needs to initialize after POR before the register can be accessed.  Spin waiting for
	 * them to be ready.  This could take a significant amount of time, but only will get triggered
	 * once on each POR. */
	platform_init_timeout (rtc->ready_timeout_ms, &timeout);
	while (!platform_has_timeout_expired (&timeout)) {
		if (*reg == mask) {
			status = 0;
			break;
		}
	}

	return status;
}

/**
 * Read from the RTC HW register.
 *
 * @param rtc The HSP real time clock instance.
 * @param reg The RTC HW register address.
 * @param reg_value The output for the value read from the RTC HW register.
 *
 * @return Returns 0 if success, else an error code
 */
static int real_time_clock_hsp_read_reg (const struct real_time_clock_hsp *rtc,
	volatile uint32_t *reg, uint32_t *reg_value)
{
	/* Wait for two ready bits for rtcclk and secclk of RTC_READ_READY register to get set. */
	if (real_time_clock_hsp_wait_ready (rtc, &rtc->regs->RTC_READ_READY,
		RTC_READ_READY_MASK) != 0) {
		/* Timeout to wait for RTC_READ_READY register to become ready. */
		return REAL_TIME_CLOCK_READ_TIMEOUT;
	}

	/* Read the given register. */
	*reg_value = *reg;

	return 0;
}

/**
 * Write the value to the RTC HW register.
 *
 * @param rtc The HSP real time clock instance.
 * @param reg The RTC HW register address.
 * @param reg_value The value written to the RTC HW register.
 *
 * @return Returns 0 if success, else an error code
 */
static int real_time_clock_hsp_write_reg (const struct real_time_clock_hsp *rtc,
	volatile uint32_t *reg, uint32_t reg_value)
{
	/* Wait for two ready bits for rtcclk and secclk of RTC_WRITE_READY register to get set. */
	if (real_time_clock_hsp_wait_ready (rtc, &rtc->regs->RTC_WRITE_READY,
		RTC_WRITE_READY_MASK) != 0) {
		/* Timeout to wait for RTC_WRITE_READY register to become ready. */
		return REAL_TIME_CLOCK_WRITE_TIMEOUT;
	}

	/* Write the value to the given register. */
	*reg = reg_value;

	return 0;
}

int real_time_clock_hsp_get_time (const struct real_time_clock *rtc, uint64_t *msec)
{
	const struct real_time_clock_hsp *rtc_hsp = (const struct real_time_clock_hsp*) rtc;
	const uint32_t tick_rate = HSP_RTC_FREQUENCY_HZ;
	uint32_t seconds[2];
	uint32_t sub_seconds;

	if ((rtc_hsp == NULL) || (msec == NULL)) {
		return REAL_TIME_CLOCK_INVALID_ARGUMENT;
	}

	if (real_time_clock_hsp_wait_ready (rtc_hsp, &rtc_hsp->regs->RTC_READ_READY,
		RTC_READ_READY_MASK) != 0) {
		return REAL_TIME_CLOCK_READ_TIMEOUT;
	}

	/* The HSP accessible registers are synchronized with the RTC registers based on a 1Hz clock,
	 * and there is some indeterminism around the value that is read.  To account for this, the
	 * following sequence needs to be performed.
	 *
	 * 	1.  Read the SECONDS_COUNTER register, which represents the current time in seconds.
	 * 	2.  Read the COUNT_TO_SECOND register, which is used to generate the 1Hz clock.  On
	 * 		overflow of this counter, the SECONDS_COUNTER is incremented.
	 * 	3.  Read the SECONDS_COUNTER register again.  There is a corner case that can occur while
	 * 		the HSP register is being updated from the RTC domain.
	 * 	4.  If SECONDS_COUNTER from #1 and #3 are different, the current time is the second value.
	 * 	5.  If the two SECONDS_COUNTER values are the same, check the MSB of COUNT_TO_SECONDS:
	 * 			- MSB == 0 --> current time is SECONDS_COUNTER + 1
	 * 			- MSB == 1 --> current time is SECONDS_COUNTER
	 */
	seconds[0] = rtc_hsp->regs->SECONDS_COUNTER;
	sub_seconds = rtc_hsp->regs->COUNT_TO_SECOND;
	seconds[1] = rtc_hsp->regs->SECONDS_COUNTER;

	if (seconds[0] == seconds[1]) {
		if (!(sub_seconds & (1U << CREG_REGS_CREG_RTC_GROUP_COUNT_TO_SECOND_COUNT_TO_SECOND_MSB))) {
			seconds[1]++;
		}
	}

	*msec = ((uint64_t) seconds[1] * 1000) + CLOCK_TICKS_TO_MS (sub_seconds, tick_rate);

	return 0;
}

int real_time_clock_hsp_set_time (const struct real_time_clock *rtc, uint64_t msec)
{
	const struct real_time_clock_hsp *rtc_hsp = (const struct real_time_clock_hsp*) rtc;
	int status;

	if (rtc_hsp == NULL) {
		return REAL_TIME_CLOCK_INVALID_ARGUMENT;
	}

	msec /= 1000;

	/* NOTE: There is a 1 second uncertainty when the value will get applied to the RTC HW due to
	 * clock domain crossings. Add 2 seconds to input to help improve accuracy. */
	msec += 2;

	if (msec > UINT32_MAX) {
		return REAL_TIME_CLOCK_OUT_OF_RANGE;
	}

	platform_mutex_lock (&rtc_hsp->state->lock);

	if (rtc_hsp->state->cooldown_valid) {
		status = platform_has_timeout_expired (&rtc_hsp->state->set_time_cooldown);
		if (status != 1) {
			if (status == 0) {
				status = REAL_TIME_CLOCK_SET_BLOCKED;
			}

			goto unlock;
		}
	}
	else {
		// Cooldown initialization failed last set time call. Try to reinitialize
		status = platform_init_timeout (1000, &rtc_hsp->state->set_time_cooldown);
		if (status == 0) {
			// Initialization was successful, but play safe and block
			status = REAL_TIME_CLOCK_SET_BLOCKED;

			rtc_hsp->state->cooldown_valid = true;
		}

		goto unlock;
	}

	if (real_time_clock_hsp_wait_ready (rtc_hsp, &rtc_hsp->regs->RTC_WRITE_READY,
		RTC_WRITE_READY_MASK) != 0) {
		status = REAL_TIME_CLOCK_WRITE_TIMEOUT;
		goto unlock;
	}

	rtc_hsp->regs->COUNT_TO_SECOND = 0;
	rtc_hsp->regs->SECONDS_COUNTER = (uint32_t) msec;

	// Do not fail on initializing cooldown.
	status = platform_init_timeout (REAL_TIME_CLOCK_HSP_COOLDOWN_DELAY_MS,
		&rtc_hsp->state->set_time_cooldown);
	if (status == 0) {
		rtc_hsp->state->cooldown_valid = true;
	}
	else {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_RTC_COOLDOWN_INIT_ERROR, status, 0);

		rtc_hsp->state->cooldown_valid = false;
		status = 0;
	}

unlock:
	platform_mutex_unlock (&rtc_hsp->state->lock);

	return status;
}

int real_time_clock_hsp_get_intrusion_count (const struct real_time_clock_hsp *rtc,
	uint32_t *intrusion_count)
{
	if ((rtc == NULL) || (intrusion_count == NULL)) {
		return REAL_TIME_CLOCK_INVALID_ARGUMENT;
	}

	return real_time_clock_hsp_read_reg (rtc, &rtc->regs->TAMPER_COUNT, intrusion_count);
}

int real_time_clock_hsp_clear_intrusion_count (const struct real_time_clock_hsp *rtc)
{
	if (rtc == NULL) {
		return REAL_TIME_CLOCK_INVALID_ARGUMENT;
	}

	return real_time_clock_hsp_write_reg (rtc, &rtc->regs->TAMPER_COUNT, 0);
}

int real_time_clock_hsp_get_intrusion_state (const struct real_time_clock_hsp *rtc,
	uint32_t *intrusion_state)
{
	if ((rtc == NULL) || (intrusion_state == NULL)) {
		return REAL_TIME_CLOCK_INVALID_ARGUMENT;
	}

	return real_time_clock_hsp_read_reg (rtc, &rtc->regs->RTC_STS, intrusion_state);
}

int real_time_clock_hsp_get_intrusion_magic_number (const struct real_time_clock_hsp *rtc,
	uint32_t *intrusion_magic_number)
{
	if ((rtc == NULL) || (intrusion_magic_number == NULL)) {
		return REAL_TIME_CLOCK_INVALID_ARGUMENT;
	}

	return real_time_clock_hsp_read_reg (rtc, &rtc->regs->TAMPER_0, intrusion_magic_number);
}

int real_time_clock_hsp_set_intrusion_magic_number (const struct real_time_clock_hsp *rtc,
	uint32_t intrusion_magic_number)
{
	if (rtc == NULL) {
		return REAL_TIME_CLOCK_INVALID_ARGUMENT;
	}

	return real_time_clock_hsp_write_reg (rtc, &rtc->regs->TAMPER_0, intrusion_magic_number);
}

int real_time_clock_hsp_get_interrupt_status (const struct real_time_clock_hsp *rtc,
	uint32_t *int_status)
{
	if ((rtc == NULL) || (int_status == NULL)) {
		return REAL_TIME_CLOCK_INVALID_ARGUMENT;
	}

	return real_time_clock_hsp_read_reg (rtc, &rtc->regs->RTC_INTSTS, int_status);
}

int real_time_clock_hsp_clear_interrupt_status (const struct real_time_clock_hsp *rtc,
	uint32_t int_status)
{
	if (rtc == NULL) {
		return REAL_TIME_CLOCK_INVALID_ARGUMENT;
	}

	return real_time_clock_hsp_write_reg (rtc, &rtc->regs->RTC_INTSTS, int_status);
}

int real_time_clock_hsp_get_interrupt_enable (const struct real_time_clock_hsp *rtc,
	uint32_t *enable)
{
	if ((rtc == NULL) || (enable == NULL)) {
		return REAL_TIME_CLOCK_INVALID_ARGUMENT;
	}

	return real_time_clock_hsp_read_reg (rtc, &rtc->regs->RTC_INTEN, enable);
}

int real_time_clock_hsp_set_interrupt_enable (const struct real_time_clock_hsp *rtc,
	uint32_t enable)
{
	if (rtc == NULL) {
		return REAL_TIME_CLOCK_INVALID_ARGUMENT;
	}

	return real_time_clock_hsp_write_reg (rtc, &rtc->regs->RTC_INTEN, enable);
}

/**
 * Initialize an HSP real time clock instance that supports both get and set time.
 *
 * @param rtc The real time clock instance to initialize.
 * @param state The variable state context.
 * @param regs_ptr The RTC HW registers.
 * @param ready_timeout_ms The timeout in milliseconds to wait for the HW to be ready for access.
 * A value of 0 will immediately fail if the HW is not ready and a value of UINT32_MAX will block
 * indefinitely until the HW is ready.
 *
 * @return 0 if the instance was initialized successfully or an error code.
 */
int real_time_clock_hsp_init (struct real_time_clock_hsp *rtc,
	struct real_time_clock_hsp_state *state, struct Creg_regs_creg_rtc_group *regs,
	uint32_t ready_timeout_ms)
{
	if ((rtc == NULL) || (state == NULL)) {
		return REAL_TIME_CLOCK_INVALID_ARGUMENT;
	}

	memset (rtc, 0, sizeof (*rtc));

	rtc->base.get_time = real_time_clock_hsp_get_time;
	rtc->base.set_time = real_time_clock_hsp_set_time;
	rtc->get_intrusion_count = real_time_clock_hsp_get_intrusion_count;
	rtc->clear_intrusion_count = real_time_clock_hsp_clear_intrusion_count;
	rtc->get_intrusion_state = real_time_clock_hsp_get_intrusion_state;
	rtc->get_intrusion_magic_number = real_time_clock_hsp_get_intrusion_magic_number;
	rtc->set_intrusion_magic_number = real_time_clock_hsp_set_intrusion_magic_number;
	rtc->get_interrupt_status = real_time_clock_hsp_get_interrupt_status;
	rtc->clear_interrupt_status = real_time_clock_hsp_clear_interrupt_status;
	rtc->get_interrupt_enable = real_time_clock_hsp_get_interrupt_enable;
	rtc->set_interrupt_enable = real_time_clock_hsp_set_interrupt_enable;

	rtc->state = state;
	rtc->regs = regs;
	rtc->ready_timeout_ms = ready_timeout_ms;

	return real_time_clock_hsp_init_state (rtc);
}

/**
 * Initialize an HSP real time clock instance that supports only get time and not set time.
 *
 * @param rtc The real time clock instance to initialize.
 * @param regs_ptr The RTC HW registers.
 * @param ready_timeout_ms The timeout in milliseconds to wait for the HW to be ready for access.
 * A value of 0 will immediately fail if the HW is not ready and a value of UINT32_MAX will block
 * indefinitely until the HW is ready.
 *
 * @return 0 if the instance was initialized successfully or an error code.
 */
int real_time_clock_hsp_init_no_set_time (struct real_time_clock_hsp *rtc,
	struct Creg_regs_creg_rtc_group *regs, uint32_t ready_timeout_ms)
{
	if ((rtc == NULL) || (regs == NULL)) {
		return REAL_TIME_CLOCK_INVALID_ARGUMENT;
	}

	memset (rtc, 0, sizeof (*rtc));

	rtc->base.get_time = real_time_clock_hsp_get_time;
	rtc->base.set_time = real_time_clock_set_time_unsupported;
	rtc->get_intrusion_count = real_time_clock_hsp_get_intrusion_count;
	rtc->clear_intrusion_count = real_time_clock_hsp_clear_intrusion_count;
	rtc->get_intrusion_state = real_time_clock_hsp_get_intrusion_state;
	rtc->get_intrusion_magic_number = real_time_clock_hsp_get_intrusion_magic_number;
	rtc->set_intrusion_magic_number = real_time_clock_hsp_set_intrusion_magic_number;
	rtc->get_interrupt_status = real_time_clock_hsp_get_interrupt_status;
	rtc->clear_interrupt_status = real_time_clock_hsp_clear_interrupt_status;
	rtc->get_interrupt_enable = real_time_clock_hsp_get_interrupt_enable;
	rtc->set_interrupt_enable = real_time_clock_hsp_set_interrupt_enable;

	rtc->regs = regs;
	rtc->ready_timeout_ms = ready_timeout_ms;

	return 0;
}

/**
 * Initializes the HSP real time clock variable state context.
 *
 * @param rtc The HSP real time clock instance.
 *
 * @return Returns 0 if success, else an error code
 */
int real_time_clock_hsp_init_state (const struct real_time_clock_hsp *rtc)
{
	int status;

	if ((rtc == NULL) || (rtc->state == NULL)) {
		return REAL_TIME_CLOCK_INVALID_ARGUMENT;
	}

	status = platform_mutex_init (&rtc->state->lock);
	if (status != 0) {
		return status;
	}

	status = platform_init_timeout (1, &rtc->state->set_time_cooldown);
	if (status != 0) {
		platform_mutex_free (&rtc->state->lock);
	}

	rtc->state->cooldown_valid = true;

	return status;
}

/**
 * Release the resources used by an HSP real time clock instance.
 *
 * @param intf The real time clock instance to release.
 */
void real_time_clock_hsp_release (const struct real_time_clock_hsp *rtc)
{
	if ((rtc == NULL) || (rtc->state == NULL)) {
		return;
	}

	platform_mutex_free (&rtc->state->lock);
}
