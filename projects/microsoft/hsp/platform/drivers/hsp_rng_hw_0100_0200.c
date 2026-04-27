// Copyright (c) Microsoft Corporation. All rights reserved.

/* There are some significant differences in functionality with older RNG versions that have a big
 * impact on driver handling.  This file isolates these differences, making the main driver code
 * cleaner. */

#if (defined HSP_RNG_HW_0100 || defined HSP_RNG_HW_0200)
#include <string.h>
#include "hsp_top.h"
#include "common/unused.h"
#include "drivers/hsp_rng_hw.h"


/**
 * Initialize the HSP HW random number generator for RNG versions 0100 and 0200.
 *
 * @param rng The driver to initialize.
 * @param state Variable context for the RNG driver.  This must be uninitialized.
 * @param regs Base address for the HSP RNG registers.
 * @param fuses Unused.
 * @param clk_div_min Unused.
 * @param clk_div_max Unused.
 *
 * @return 0 if the RNG driver was successfully initialized or an error code.
 */
int hsp_rng_hw_init (struct hsp_rng_hw *rng, struct hsp_rng_hw_state *state, struct Rng_regs *regs,
	const struct fuse_controller_interface *fuses, uint16_t clk_div_min, uint16_t clk_div_max)
{
	int status;

	UNUSED (fuses);
	UNUSED (clk_div_min);
	UNUSED (clk_div_max);

	status = hsp_rng_hw_init_common (rng, state, regs);
	if (status != 0) {
		return status;
	}

	return hsp_rng_hw_init_state (rng, false);
}

/**
 * Initialize only the variable state for an RNG driver for RNG versions 0100 and 0200.  The rest of
 * the driver is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param rng The RNG driver that contains the state to initialize.
 * @param calibrate Unused.
 *
 * @return 0 if the driver state was successfully initialized or an error code.
 */
int hsp_rng_hw_init_state (const struct hsp_rng_hw *rng, bool calibrate)
{
	UNUSED (calibrate);

	return hsp_rng_hw_init_state_common (rng);
}

/**
 * RNG versions 0100 and 0200 don't support calibration, so this call is a no-op.
 *
 * @param rng Unused
 *
 * @return 0 always.
 */
int hsp_rng_hw_recalibrate (const struct hsp_rng_hw *rng)
{
	UNUSED (rng);

	return 0;
}

#ifdef HSP_RNG_HW_0100
bool hsp_rng_hw_is_rng_busy (struct Rng_regs *regs)
{
	return (regs->status & RNG_REGS_STATUS_BUSY_FIELD_MASK);
}
#endif

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
#if defined (HSP_RNG_HW_0200)
	uint32_t rng_drbg_status = rng->regs->drbg_status;

	/* Keep polling until DRBG is not in Instantiate State (0x1) or Reseed State (0x6). */
	while ((rng_drbg_status == 0x1) || (rng_drbg_status == 0x6)) {
		rng_drbg_status = rng->regs->drbg_status;
	}
#else
	UNUSED (rng);
#endif
}


#endif	/* HSP_RNG_HW_0100 || HSP_RNG_HW_0200 */
