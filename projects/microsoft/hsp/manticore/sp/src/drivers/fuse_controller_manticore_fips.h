// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FUSE_CONTROLLER_MANTICORE_FIPS_H_
#define FUSE_CONTROLLER_MANTICORE_FIPS_H_

#include "drivers/fuse_controller.h"


int fuse_controller_manticore_fips_init (struct fuse_controller *fuses,
	struct fuse_controller_state *state, struct Gfc_regs *regs);

/* Internal functions for use by derived types. */
int fuse_controller_manticore_fips_read_rng_calibration (
	const struct fuse_controller_interface *fuses,
	uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH]);
int fuse_controller_manticore_fips_program_rng_calibration (
	const struct fuse_controller_interface *fuses,
	const uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH]);


#endif	/* FUSE_CONTROLLER_MANTICORE_FIPS_H_ */
