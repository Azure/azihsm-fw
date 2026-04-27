// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "common/buffer_util.h"
#include "common/common_math.h"
#include "common/unused.h"
#include "drivers/hsp_rng_hw.h"
#include "splibs/inc/spcryptotypes.h"
#include "splibs/inc/sptypes.h"


/**
 * Defines the list of faults to check for when reading random data.  Different versions of the RNG
 * hardware support different types of fault reporting.
 */
#ifdef HSP_RNG_HW_0100
/* Fault mask for RNG v0100 */
#define	HSP_RNG_HW_FAULT_MASK	(RNG_REGS_STATUS_ERROR_FAULT_FIELD_MASK)

#elif defined (HSP_RNG_HW_0200)
/* Fault mask for RNG v0200 */
#define	HSP_RNG_HW_FAULT_MASK   \
	(RNG_REGS_STATUS_DRBG_FAULT_ERROR_FIELD_MASK | RNG_REGS_STATUS_FAULT_ERROR_FIELD_MASK)

#else
/* Fault mask for RNG v0300 and later. */
#define	HSP_RNG_HW_FAULT_MASK	(RNG_REGS_STATUS_REPCNT_FAULT_ERROR_FIELD_MASK | \
	RNG_REGS_STATUS_APT_FAULT_ERROR_FIELD_MASK | RNG_REGS_STATUS_CHISQ_FAULT_ERROR_FIELD_MASK | \
	RNG_REGS_STATUS_RBG_FAULT_ERROR_FIELD_MASK | RNG_REGS_STATUS_DRBG_FAULT_ERROR_FIELD_MASK)
#endif


#pragma pack(push, 1)
/**
 * Structure of the RNG calibration data.
 */
union rng_hsp_calibration {
	struct {
		uint8_t clk_div;									/**< Override for the default RNG clock divider value. */
		uint32_t cutoff;									/**< MSB of clock divider and cutoff values. */
	} parsed;												/**< Parsed calibration bytes. */
	uint8_t raw[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH];	/**< Raw calibration bytes. */
};

#pragma pack(pop)

/**
 * Override for the default MSB of the RNG clock divider.
 */
#define	rng_hsp_calibration_get_clk_div_msb(x)			((x)->parsed.cutoff & 0x3)

/**
 * Set override value for the default MSB of the RNG clock divider.
 */
#define	rng_hsp_calibration_set_clk_div_msb(x, val)     \
	(x)->parsed.cutoff = (((x)->parsed.cutoff & ~0x3) | ((val) & 0x3))

/**
 * Override for the default repetition count entry health test value.
 */
#define	rng_hsp_calibration_get_repcnt_cutoff(x)		(((x)->parsed.cutoff & (0x3ffU << 2)) >> 2)

/**
 * Set override value for the default repetition count entry health test value.
 */
#define	rng_hsp_calibration_set_repcnt_cutoff(x, val)   \
	(x)->parsed.cutoff = (((x)->parsed.cutoff & ~(0x3ffU << 2)) | (((val) & 0x3ffU) << 2))

/**
 * Override for the default adaptive proportion health test value.
 */
#define	rng_hsp_calibration_get_apt_cutoff(x)           \
	(((x)->parsed.cutoff & (0x3ffU << 12)) >> 12)

/**
 * Set override value for the default adaptive proportion health test value.
 */
#define	rng_hsp_calibration_set_apt_cutoff(x, val)      \
	(x)->parsed.cutoff = (((x)->parsed.cutoff & ~(0x3ffU << 12)) | (((val) & 0x3ffU) << 12))

/**
 * Override for the default chi square entropy health test value.
 */
#define	rng_hsp_calibration_get_chisq_cutoff(x)         \
	(((x)->parsed.cutoff & (0x3ffU << 22)) >> 22)

/**
 * Set override value for the default chi square entropy health test value.
 */
#define	rng_hsp_calibration_set_chisq_cutoff(x, val)    \
	(x)->parsed.cutoff = (((x)->parsed.cutoff & ~(0x3ffU << 22)) | (((val) & 0x3ffU) << 22))

/**
 * Get the LSB for an RNG clock divider value.
 *
 * @param clk_div The clock divider value.
 */
#define	hsp_rng_hw_get_clk_div_lsb(clk_div)				((clk_div) & 0xff)

/**
 * Get the MSB for an RNG clock divider value.
 *
 * @param clk_div The clock divider value.
 */
#define	hsp_rng_hw_get_clk_div_msb(clk_div)				(((clk_div) >> 8) & 0x3)

/**
 * Get a clock divider value from the MSB and LSB.
 *
 * @param msb MSB of the clock divider.
 * @param lsb LSB of the clock divider.
 */
#define	hsp_rng_hw_get_clk_div(msb, lsb)				(((msb) << 8) | (lsb))


/**
 * Determine if the RNG hardware is busy generating random data.  The rn_data FIFO should not be
 * accessed while the hardware is busy.
 *
 * This function is compile-time abstracted since different versions of the RNG have different
 * mechanisms for detecting this condition.
 *
 * @param regs Base register address for the RNG hardware to query.
 *
 * @return true if the RNG hardware is busy or false if random data is available in the FIFO.
 */
bool hsp_rng_hw_is_rng_busy (struct Rng_regs *regs);

/**
 * Common HSP HW random number generator initialization function.
 *
 * @param rng The driver to initialize.
 * @param state Variable context for the RNG driver.  This must be uninitialized.
 * @param regs Base address for the HSP RNG registers.
 *
 * @return 0 if the RNG driver was successfully initialized or an error code.
 */
int hsp_rng_hw_init_common (struct hsp_rng_hw *rng, struct hsp_rng_hw_state *state,
	struct Rng_regs *regs)
{
	if ((rng == NULL) || (state == NULL) || (regs == NULL)) {
		return HSP_RNG_HW_INVALID_ARGUMENT;
	}

	memset (rng, 0, sizeof (struct hsp_rng_hw));

	rng->state = state;
	rng->regs = regs;

	return 0;
}

/**
 * Common variable state initialization function for RNG driver.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param rng The RNG driver that contains the state to initialize.
 *
 * @return 0 if the driver state was successfully initialized or an error code.
 */
int hsp_rng_hw_init_state_common (const struct hsp_rng_hw *rng)
{
	if ((rng == NULL) || (rng->state == NULL) || (rng->regs == NULL)) {
		return HSP_RNG_HW_INVALID_ARGUMENT;
	}

	memset (rng->state, 0, sizeof (struct hsp_rng_hw_state));

#ifndef HSP_RNG_HW_0100
	/* Reduce the generation interval to 1 block of random data.  This changes the granularity of
	 * random data reads to 512 bits from the default of 1024 bits, improving efficiency when
	 * reading small amounts of random data. */
	rng->regs->generate_interval = 1;
#endif

	return platform_mutex_init (&rng->state->lock);
}

#if (!defined HSP_RNG_HW_0100 && !defined HSP_RNG_HW_0200)
/**
 * Initialize the HSP HW random number generator.  As part of initialization, the RNG calibration
 * will be read from fuses and applied.
 *
 * @param rng The driver to initialize.
 * @param state Variable context for the RNG driver.  This must be uninitialized.
 * @param regs Base address for the HSP RNG registers.
 * @param fuses Interface to the HSP fuses where RNG calibration data is stored.
 * @param clk_div_min The minimum clock divider that should be allowed.  If the RNG calibration data
 * specifies a smaller value, the minimum will be applied instead.
 * @param clk_div_max The maximum clock divider that should be allowed.  If the RNG calibration data
 * specifies a larger value, the maximum will  be applied instead.  If there is no limit, use
 * HSP_RNG_HW_MAX_CLOCK_DIVIDER.
 *
 * @return 0 if the RNG driver was successfully initialized or an error code.
 */
int hsp_rng_hw_init (struct hsp_rng_hw *rng, struct hsp_rng_hw_state *state, struct Rng_regs *regs,
	const struct fuse_controller_interface *fuses, uint16_t clk_div_min, uint16_t clk_div_max)
{
	int status;

	if (fuses == NULL) {
		return HSP_RNG_HW_INVALID_ARGUMENT;
	}

	status = hsp_rng_hw_init_common (rng, state, regs);
	if (status != 0) {
		return status;
	}

	rng->fuses = fuses;
	rng->clk_div_min = (clk_div_min & HSP_RNG_HW_MAX_CLOCK_DIVIDER);
	rng->clk_div_max = (clk_div_max & HSP_RNG_HW_MAX_CLOCK_DIVIDER);

	return hsp_rng_hw_init_state (rng, true);
}

/**
 * Apply calibration data to the RNG, but only if the RNG values are different from the calibration
 * values.  If any values are updated, the RNG will be reset.
 *
 * @param rng The RNG to calibrate.
 * @param calibration Calibration data to apply to the RNG.
 * @param force Force the calibration to be applied.
 */
static void hsp_rng_hw_apply_calibration_data (const struct hsp_rng_hw *rng,
	const union rng_hsp_calibration *calibration, bool force)
{
	uint32_t ctrl;
	uint32_t clk_div_msb = rng_hsp_calibration_get_clk_div_msb (calibration);
	uint32_t repcnt_cutoff = rng_hsp_calibration_get_repcnt_cutoff (calibration);
	uint32_t apt_cutoff = rng_hsp_calibration_get_apt_cutoff (calibration);
	uint32_t chisq_cutoff = rng_hsp_calibration_get_chisq_cutoff (calibration);
	bool apply_cal = false;

	if (force || (RNG_REGS_CTRL_CLK_DIV_GET (rng->regs->ctrl) != calibration->parsed.clk_div) ||
		(rng->regs->clk_div_msb != clk_div_msb) || (rng->regs->repcnt_cutoff != repcnt_cutoff) ||
		(rng->regs->apt_cutoff != apt_cutoff) || (rng->regs->chisq_cutoff != chisq_cutoff)) {
		apply_cal = true;
	}

	if (apply_cal) {
		/* Calibration needs to be applied, so be sure the RNG is disabled. */
		rng->regs->ctrl &= ~RNG_REGS_CTRL_ENABLE_FIELD_MASK;

		/* Reset the RNG with the new calibration values. */
		ctrl = rng->regs->ctrl;
		ctrl |= RNG_REGS_CTRL_ENABLE_FIELD_MASK;

		/* Apply the calibration values. */
		ctrl = RNG_REGS_CTRL_CLK_DIV_MODIFY (ctrl, calibration->parsed.clk_div);
		rng->regs->clk_div_msb = clk_div_msb;
		rng->regs->repcnt_cutoff = repcnt_cutoff;
		rng->regs->apt_cutoff = apt_cutoff;
		rng->regs->chisq_cutoff = chisq_cutoff;

		rng->regs->ctrl = ctrl;
	}
}

/**
 * Initialize only the variable state for a RNG driver.  The rest of the driver is assumed to have
 * already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param rng The RNG driver that contains the state to initialize.
 * @param calibrate Flag indicating if RNG calibration should be read from fuses and applied or not.
 *
 * @return 0 if the driver state was successfully initialized or an error code.
 */
int hsp_rng_hw_init_state (const struct hsp_rng_hw *rng, bool calibrate)
{
	union rng_hsp_calibration calibration;
	bool apply_cal = false;
	int status;

	status = hsp_rng_hw_init_state_common (rng);
	if (status != 0) {
		return status;
	}

	if (rng->fuses == NULL) {
		return HSP_RNG_HW_INVALID_ARGUMENT;
	}

	if (calibrate) {
		status = hsp_rng_hw_get_calibration_data (rng, calibration.raw, &apply_cal);
		if (status != 0) {
			return status;
		}
	}

	if (apply_cal) {
		hsp_rng_hw_apply_calibration_data (rng, &calibration, true);
	}
	else {
#if defined BUILD_FOR_SIMULATION && defined SIMULATION_ADJUST_RNG_CLOCK
		/* If there is no other calibration being applied, just increase the RNG clock for
		 * simulation. */
		rng->regs->ctrl = RNG_REGS_CTRL_CLK_DIV_MODIFY (rng->regs->ctrl, 1);
#endif

		/* Just start the RNG without setting any calibration. */
		rng->regs->ctrl |= RNG_REGS_CTRL_ENABLE_FIELD_MASK;
	}

	return 0;
}
#endif	/* !RNG_HSP_HW_0100 && !RNG_HSP_HW_0200 */

/**
 * Release a HSP RNG driver instance.
 *
 * @param rng The RNG driver to release.
 */
void hsp_rng_hw_release (const struct hsp_rng_hw *rng)
{
	if (rng) {
		platform_mutex_free (&rng->state->lock);
	}
}

#if (!defined HSP_RNG_HW_0100 && !defined HSP_RNG_HW_0200)
/**
 * Read the current calibration data from HSP fuses and apply any imposed limits on the values.
 *
 * @param rng The RNG driver being queried.
 * @param data Output for the calibration data.
 * @param data_valid Optional output indicating if the data represents valid calibration information
 * that should be applied to the RNG.
 *
 * @return 0 if the calibration data was successfully read or an error code.
 */
int hsp_rng_hw_get_calibration_data (const struct hsp_rng_hw *rng,
	uint8_t calibration[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH], bool *data_valid)
{
	bool valid = false;
	int i = 1;
	int status;

	if ((rng == NULL) || (calibration == NULL)) {
		return HSP_RNG_HW_INVALID_ARGUMENT;
	}

	if (rng->fuses == NULL) {
		return HSP_RNG_HW_CALIBRATION_NOT_SUPPORTED;
	}

	status = rng->fuses->read_rng_calibration (rng->fuses, calibration);
	if (status != 0) {
		return status;
	}

	/* Only apply the calibration if the data is valid.  Invalid calibration data is any data that
	 * is all 0s or all 1s. */
	while (!valid && (i < FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH)) {
		if ((calibration[i] != calibration[i - 1]) ||
			((calibration[i] != 0) && (calibration[i] != 0xff))) {
			valid = true;
		}

		i++;
	}

	/* If there is valid calibration data, validate that the clock divider falls within the allowed
	 * range. */
	if (valid) {
		union rng_hsp_calibration *check = (union rng_hsp_calibration*) calibration;
		uint32_t clk_div = hsp_rng_hw_get_clk_div (rng_hsp_calibration_get_clk_div_msb (check),
			check->parsed.clk_div);

		if (clk_div < rng->clk_div_min) {
			check->parsed.clk_div = hsp_rng_hw_get_clk_div_lsb (rng->clk_div_min);
			rng_hsp_calibration_set_clk_div_msb (check,
				hsp_rng_hw_get_clk_div_msb (rng->clk_div_min));
		}
		else if (clk_div > rng->clk_div_max) {
			check->parsed.clk_div = hsp_rng_hw_get_clk_div_lsb (rng->clk_div_max);
			rng_hsp_calibration_set_clk_div_msb (check,
				hsp_rng_hw_get_clk_div_msb (rng->clk_div_max));
		}
	}

	if (data_valid) {
		*data_valid = valid;
	}

	return 0;
}

/**
 * Reapply the calibration values to the the RNG.  If there is no difference between the current
 * settings and calibration values, nothing will be done with the RNG.
 *
 * IF the calibration is applied, the RNG will be reset.
 *
 * @param rng The RNG to calibrate.
 *
 * @return 0 if the calibration was successful or an error code.
 */
int hsp_rng_hw_recalibrate (const struct hsp_rng_hw *rng)
{
	union rng_hsp_calibration calibration;
	bool apply_cal = false;
	int status;

	if (rng == NULL) {
		return HSP_RNG_HW_INVALID_ARGUMENT;
	}

	status = hsp_rng_hw_get_calibration_data (rng, calibration.raw, &apply_cal);
	if (status != 0) {
		return status;
	}

#if defined BUILD_FOR_SIMULATION && defined SIMULATION_ADJUST_RNG_CLOCK
	if (!apply_cal) {
		/* Increase the RNG clock for simulation. */
		calibration.parsed.clk_div = 1;

		/* Make sure the rest of the values are kept the same. */
		calibration.parsed.cutoff = 0;
		rng_hsp_calibration_set_clk_div_msb (&calibration, rng->regs->clk_div_msb);
		rng_hsp_calibration_set_repcnt_cutoff (&calibration, rng->regs->repcnt_cutoff);
		rng_hsp_calibration_set_apt_cutoff (&calibration, rng->regs->apt_cutoff);
		rng_hsp_calibration_set_chisq_cutoff (&calibration, rng->regs->chisq_cutoff);

		apply_cal = true;
	}
#endif

	if (apply_cal) {
		hsp_rng_hw_apply_calibration_data (rng, &calibration, false);
	}

	return 0;
}
#endif	/* !RNG_HSP_HW_0100 && !RNG_HSP_HW_0200 */

#ifndef HSP_RNG_HW_0100
bool hsp_rng_hw_is_rng_busy (struct Rng_regs *regs)
{
	/* When reseed_cnt != 0, the RNG data has been generated. */
	return (regs->reseed_cnt == 0);
}
#endif

/**
 * Wait until the RNG has random data ready to read.  Also monitor the RNG for faults and execute
 * recovery if faults occur.
 *
 * @param regs Base register address for the RNG hardware to query.
 */
static void hsp_rng_hw_wait_for_rn_data (struct Rng_regs *regs)
{
	uint32_t rng_status;
	bool busy = false;

	do {
		if (busy) {
			/* Wait a bit before checking the RNG again when it's busy. */
			platform_msleep (10);
		}

		rng_status = regs->status;
		if (rng_status & HSP_RNG_HW_FAULT_MASK) {
			/* There has been a fault.  Reset the RNG to clear it. */
			regs->ctrl &= ~RNG_REGS_CTRL_ENABLE_FIELD_MASK;
			platform_msleep (1);
			regs->ctrl |= RNG_REGS_CTRL_ENABLE_FIELD_MASK;
		}

		busy = true;
	} while (hsp_rng_hw_is_rng_busy (regs));
}

/**
 * Read a full block of random data from the output FIFO.
 *
 * @param rng The RNG to read data from.
 * @param data Output for the random data.
 */
static void hsp_rng_hw_read_random_data (const struct hsp_rng_hw *rng, SP_MSG_512 *data)
{
	uint32_t count = 1;
	int i;

#ifndef HSP_RNG_HW_0100
	/* The full block needs to be read from the FIFO for any amount of random data.  The
	 * generate_interval register indicates the number of 512-bit chunks that need to be read. */
	count = rng->regs->generate_interval;

	/* The random output is always expected to only be a single 512-bit chunk.  If something
	 * happened and this is no longer true, adjust the interval so that future accesses of random
	 * data are more efficient. */
	if (count != 1) {
		rng->regs->generate_interval = 1;
	}
#endif

	while (count-- > 0) {
		/* Read random data one word at a time to ensure faults or reseed events are handled
		 * correctly. */
		for (i = 0; i < IN_DWORDS (SP_MSG_512_SIZE); i++) {
			hsp_rng_hw_wait_for_rn_data (rng->regs);
			data->AsUINT32s[i] = *((uint32_t*) &rng->regs->rn_data);
		}
	}
}

/**
 * Get a random word of data from the hardware RNG.
 *
 * @param rng The RNG to read data from.
 * @param data Output for the random word of data.
 *
 * @return 0 if random data was successfully read or an error code.
 */
int hsp_rng_hw_get_random_word (const struct hsp_rng_hw *rng, uint32_t *data)
{
	return hsp_rng_hw_get_random_buffer (rng, (uint8_t*) data, sizeof (*data));
}

/**
 * Fill a buffer with random data from the hardware RNG.
 *
 * @param rng The RNG to read data from.
 * @param data Output for the random data.
 * @param length The number of bytes to read into the output buffer.
 *
 * @return 0 if the buffer was successfully filled with random data or an error code.
 */
int hsp_rng_hw_get_random_buffer (const struct hsp_rng_hw *rng, uint8_t *data, size_t length)
{
	size_t copy_len;
	SP_MSG_512 rn_data;

	if ((rng == NULL) || (data == NULL)) {
		return HSP_RNG_HW_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&rng->state->lock);

	/* Discard the first block of data, ensuring any previous context left by HW reads has been
	 * flushed. */
	hsp_rng_hw_read_random_data (rng, &rn_data);

	while (length > 0) {
		hsp_rng_hw_read_random_data (rng, &rn_data);

		copy_len = buffer_copy (rn_data.AsBytes, SP_MSG_512_SIZE, NULL, &length, data);
		data += copy_len;
	}

	/* Discard the last block block of data, ensuring no leakage of data from this context. */
	hsp_rng_hw_read_random_data (rng, &rn_data);

	platform_mutex_unlock (&rng->state->lock);

	return 0;
}

/**
 * Mark the RNG as being used by another HW block, such as CCS.  This prevents RNG access from FW
 * while being used for other HW purposes.
 *
 * This will block until the RNG is available to use.
 *
 * This must be followed by a call to hsp_rng_hw_mark_as_available for the RNG to be used again by
 * FW.
 *
 * @param rng The RNG that will be used by HW.
 */
void hsp_rng_hw_mark_as_in_use (const struct hsp_rng_hw *rng)
{
	if (rng) {
		platform_mutex_lock (&rng->state->lock);
	}
}

/**
 * Mark the RNG as no longer being used by another HW block.
 *
 * @param rng The RNG that is now available for use.
 */
void hsp_rng_hw_mark_as_available (const struct hsp_rng_hw *rng)
{
	if (rng) {
		platform_mutex_unlock (&rng->state->lock);
	}
}

/**
 * Ensure that the any RNG reseed operation has completed.  If a reseed is ongoing when random data
 * is requested, the bus transaction could time out.
 *
 * @param rng Driver for the RNG hardware to query.
 */
void hsp_rng_hw_wait_for_reseed (const struct hsp_rng_hw *rng)
{
	if (rng) {
		hsp_rng_hw_wait_for_rn_data (rng->regs);
	}
}

#if (!defined HSP_RNG_HW_0100 && !defined HSP_RNG_HW_0200)
/**
 * Ensure that the DRBG is in the right state to read entropy data when switching to FW mode.
 *
 * There is a risk of reading the same entropy data if the DRBG is in an incorrect state while RNG
 * switches to FW_MODE from Mission Mode and back due to FIFO underflow.  This may also risk
 * affecting the chi-sq test.
 *
 * @param rng Driver for the RNG hardware to query.
 */
void hsp_rng_hw_wait_for_entropy_read_done (const struct hsp_rng_hw *rng)
{
	while (rng->regs->status & RNG_REGS_STATUS_ENTROPY_FIFO_READ_FIELD_MASK) {
	}
}
#endif
