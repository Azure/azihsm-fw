// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_RNG_HW_STATIC_H_
#define HSP_RNG_HW_STATIC_H_

#include "drivers/hsp_rng_hw.h"


/**
 * Initialize a static RNG driver instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the RNG driver.
 * @param regs_ptr Base address for the HSP RNG registers.
 * @param fuses_ptr Interface to the HSP fuses where RNG calibration data is stored.
 * @param min_clk The minimum clock divider that should be allowed.  If the RNG calibration data
 * specifies a smaller value, the minimum will be applied instead.
 * @param max_clk The maximum clock divider that should be allowed.  If the RNG calibration data
 * specifies a larger value, the maximum will  be applied instead.  If there is no limit, use
 * HSP_RNG_HW_MAX_CLOCK_DIVIDER.
 */
#define	hsp_rng_hw_static_init(state_ptr, regs_ptr, fuses_ptr, min_clk, max_clk)	{ \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.fuses = fuses_ptr, \
		.clk_div_min = ((min_clk) & HSP_RNG_HW_MAX_CLOCK_DIVIDER), \
		.clk_div_max = ((max_clk) & HSP_RNG_HW_MAX_CLOCK_DIVIDER), \
	}

/**
 * Initialize a static RNG driver instance with no RNG calibration support.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the RNG driver.
 * @param regs_ptr Base address for the HSP RNG registers.
 */
#define	hsp_rng_hw_static_init_no_rng_calibration(state_ptr, regs_ptr)	{ \
		.state = state_ptr, \
		.regs = regs_ptr, \
	}


#endif	/* HSP_RNG_HW_STATIC_H_ */
