// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "hsp_watchdog.h"
#include "common/unused.h"


/**
 * Initialize a timer to be used as a hardware watchdog.  The timer will not be started.
 *
 * @param watchdog The watchdog to initialize.
 * @param creg The CREG register interface (struct Creg_regs).
 * @param timer_offset Offset within CREG for the timer registers to use as a watchdog
 * (struct Creg_regs_tmr).
 * @param fatal_err_offset Offset within CREG for fatal error registers
 * (struct Creg_regs_fatal_err_log).
 * @param hsp_clk Frequency for the HSP clock used to run the timer.
 * @param timeout_us The amount of time before a HW reset is triggered, in microseconds.
 *
 * @return 0 if the watchdog was successfully initialized or an error code.
 */
int hsp_watchdog_init (struct hsp_watchdog *watchdog, const struct mmio_register_block *creg,
	size_t timer_offset, size_t fatal_err_offset, uint32_t hsp_clk, uint32_t timeout_us)
{
	if (watchdog == NULL) {
		return HSP_WATCHDOG_INVALID_ARGUMENT;
	}

	memset (watchdog, 0, sizeof (*watchdog));

	watchdog->creg = creg;
	watchdog->timer_regs = timer_offset;
	watchdog->fatal_err_regs = fatal_err_offset;

	return hsp_watchdog_init_timer (watchdog, hsp_clk, timeout_us);
}

/**
 * Initialize only the timer to be used as a hardware watchdog.  The timer will not be
 * started.  The watchdog instance is expected to have already been initialized.
 *
 * @param watchdog The watchdog instance containing the timer to initialize.
 * @param hsp_clk Frequency for the HSP clock used to run the timer.
 * @param timeout_us The amount of time before a HW reset is triggered, in microseconds.
 *
 * @return 0 if the timer state was successfully initialized or an error code.
 */
int hsp_watchdog_init_timer (const struct hsp_watchdog *watchdog, uint32_t hsp_clk,
	uint32_t timeout_us)
{
	uint64_t timeout_ticks;
	int status;

	if ((watchdog == NULL) || (watchdog->creg == NULL) ||
		(watchdog->timer_regs == watchdog->fatal_err_regs)) {
		return HSP_WATCHDOG_INVALID_ARGUMENT;
	}

	/* The timeout needs to be divided in half, since it takes two timer expirations to trigger a
	 * HW reset. */
	timeout_ticks = ((uint64_t) timeout_us * hsp_clk) / 2000000ULL;
	if ((timeout_ticks == 0) || (timeout_ticks > 0xffffffff)) {
		return HSP_WATCHDOG_INVALID_TIMEOUT;
	}

	status = watchdog->creg->map (watchdog->creg);
	if (status != 0) {
		return status;
	}

	status = watchdog->creg->write32 (watchdog->creg,
		watchdog->timer_regs + CREG_REGS_CREG_TIMER_CREG_TMR_CTRL0_OFFSET,
		CREG_REGS_CREG_TIMER_CREG_TMR_CTRL0_DEC_FIELD_MASK);
	if (status != 0) {
		goto exit;
	}

	status = watchdog->creg->write32 (watchdog->creg,
		watchdog->timer_regs + CREG_REGS_CREG_TIMER_CREG_TMR_CTRL1_OFFSET,
		CREG_REGS_CREG_TIMER_CREG_TMR_CTRL1_TSLICE_FIELD_MASK);
	if (status != 0) {
		goto exit;
	}

	status = watchdog->creg->write32 (watchdog->creg,
		watchdog->timer_regs + CREG_REGS_CREG_TIMER_CREG_TMR_AUTOINC_OFFSET, 0);
	if (status != 0) {
		goto exit;
	}

	status = watchdog->creg->write32 (watchdog->creg,
		watchdog->timer_regs + CREG_REGS_CREG_TIMER_CREG_TMR_OCMP0_OFFSET, timeout_ticks);

exit:
	watchdog->creg->unmap (watchdog->creg);

	return status;
}

/**
 * Release the resources used for the watchdog.  The timer will not be stopped.
 *
 * @param watchdog The watchdog to release.
 */
void hsp_watchdog_release (const struct hsp_watchdog *watchdog)
{
	UNUSED (watchdog);
}

/**
 * Start the watchdog timer.  Interrupt and fatal error will be enabled for the timer.  Enabling the
 * top-level interrupt and registering a handler for this event are the responsibility of other
 * system components.
 *
 * The interrupt will trigger half-way through the timeout period.  A hardware reset will trigger at
 * the full timeout.
 *
 * @param watchdog The watchdog to start.
 * @param sticky_en Flag to indicate if the HW reset should be configured as sticky.
 *
 * @return 0 if the timer was started successfully or an error code.
 */
int hsp_watchdog_start (const struct hsp_watchdog *watchdog, bool sticky_en)
{
	uint32_t ctrl0 = CREG_REGS_CREG_TIMER_CREG_TMR_CTRL0_START_FIELD_MASK |
		CREG_REGS_CREG_TIMER_CREG_TMR_CTRL0_CLEAR_FIELD_MASK |
		CREG_REGS_CREG_TIMER_CREG_TMR_CTRL0_DEC_FIELD_MASK |
		CREG_REGS_CREG_TIMER_CREG_TMR_CTRL0_INT_EN_FIELD_MASK |
		CREG_REGS_CREG_TIMER_CREG_TMR_CTRL0_ERR_EN_FIELD_MASK;
	int status;

	if (watchdog == NULL) {
		return HSP_WATCHDOG_INVALID_ARGUMENT;
	}

	if (sticky_en) {
		ctrl0 |= CREG_REGS_CREG_TIMER_CREG_TMR_CTRL0_STICKY_ERR_EN_FIELD_MASK;
	}

	status = watchdog->creg->map (watchdog->creg);
	if (status != 0) {
		return status;
	}

	/* Make sure the interrupt status is clear before starting the timer. */
	status = watchdog->creg->write32 (watchdog->creg,
		watchdog->timer_regs + CREG_REGS_CREG_TIMER_CREG_TMR_INTSTS_OFFSET,
		CREG_REGS_CREG_TIMER_CREG_TMR_INTSTS_TIMER_INT_FIELD_MASK);
	if (status != 0) {
		goto exit;
	}

	status = watchdog->creg->write32 (watchdog->creg,
		watchdog->timer_regs + CREG_REGS_CREG_TIMER_CREG_TMR_CTRL0_OFFSET, ctrl0);
	if (status != 0) {
		goto exit;
	}

	/* Make sure timer fatal errors are enabled. */
	status = mmio_register_block_set_bit (watchdog->creg,
		watchdog->fatal_err_regs + CREG_REGS_STICKY_REGS_HSP_FATAL_ERR_INTEN_OFFSET,
		CREG_REGS_STICKY_REGS_HSP_FATAL_ERR_INTEN_CREG_TIMER_ERR_INTEN_LSB);

exit:
	watchdog->creg->unmap (watchdog->creg);

	return status;
}

/**
 * Reset the watchdog timer to the maximum timeout.  It will continue to run.
 *
 * @param watchdog The watchdog to reset.
 *
 * @return 0 if the timer was reset successfully or an error code.
 */
int hsp_watchdog_refresh (const struct hsp_watchdog *watchdog)
{
	int status;

	if (watchdog == NULL) {
		return HSP_WATCHDOG_INVALID_ARGUMENT;
	}

	status = watchdog->creg->map (watchdog->creg);
	if (status != 0) {
		return status;
	}

	/* Clear and Dec bits are next to other in the CTRL0 register, and both are write only.  A
	 * read/modify/write of this register needs to set both bits, or else the watchdog stops
	 * decrementing and no longer functions correctly. */
	status = mmio_register_block_write_bits (watchdog->creg,
		watchdog->timer_regs + CREG_REGS_CREG_TIMER_CREG_TMR_CTRL0_OFFSET,
		CREG_REGS_CREG_TIMER_CREG_TMR_CTRL0_CLEAR_LSB, 2, 0x3);
	if (status != 0) {
		goto exit;
	}

	/* If the interrupt is not clear when the next time slice expires, it will trigger a fatal
	 * error.  This status is cleared by writing a 1. */
	status = watchdog->creg->write32 (watchdog->creg,
		watchdog->timer_regs + CREG_REGS_CREG_TIMER_CREG_TMR_INTSTS_OFFSET,
		CREG_REGS_CREG_TIMER_CREG_TMR_INTSTS_TIMER_INT_FIELD_MASK);

exit:
	watchdog->creg->unmap (watchdog->creg);

	return status;
}

/**
 * Stop the watchdog timer.
 *
 * @param watchdog The watchdog to stop.
 *
 * @return 0 if the timer was stopped successfully or an error code.
 */
int hsp_watchdog_stop (const struct hsp_watchdog *watchdog)
{
	int status;

	if (watchdog == NULL) {
		return HSP_WATCHDOG_INVALID_ARGUMENT;
	}

	status = watchdog->creg->map (watchdog->creg);
	if (status != 0) {
		return status;
	}

	/* Clearing CTRL0 will stop the counter and disable the interrupts. */
	status = watchdog->creg->write32 (watchdog->creg,
		watchdog->timer_regs + CREG_REGS_CREG_TIMER_CREG_TMR_CTRL0_OFFSET, 0);

	watchdog->creg->unmap (watchdog->creg);

	return status;
}
