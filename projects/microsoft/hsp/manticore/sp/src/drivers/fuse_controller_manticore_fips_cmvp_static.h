// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FUSE_CONTROLLER_MANTICORE_FIPS_CMVP_STATIC_H_
#define FUSE_CONTROLLER_MANTICORE_FIPS_CMVP_STATIC_H_

#include "fuse_controller_manticore_fips_static.h"


/* Internal functions declared to allow for static initialization. */
int fuse_controller_manticore_fips_cmvp_read_rng_calibration (
	const struct fuse_controller_interface *fuses,
	uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH]);


/**
 * Static initialization of the Fuse Controller driver API.
 */
#define	FUSE_CONTROLLER_MANTICORE_FIPS_CMVP_API_INIT	{ \
		FUSE_CONTROLLER_API_INIT \
		.read_rng_calibration = fuse_controller_manticore_fips_cmvp_read_rng_calibration, \
		.program_rng_calibration = fuse_controller_manticore_fips_program_rng_calibration, \
	}


/**
 * Initialize a static fuse controller driver instance that uses a hard-coded RNG calibration that
 * is FIPS approved for the RNG with support for CMVP negative testing.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr The variable context for the driver instance.
 * @param regs_ptr Base address of the hardware registers.
 */
#define	fuse_controller_manticore_fips_cmvp_static_init(state_ptr, regs_ptr)	{ \
		fuse_controller_static_init (FUSE_CONTROLLER_MANTICORE_FIPS_CMVP_API_INIT, state_ptr, \
			regs_ptr), \
	}


#endif	/* FUSE_CONTROLLER_MANTICORE_FIPS_CMVP_STATIC_H_ */
