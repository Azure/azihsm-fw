// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "fuse_controller_no_rng_calibration.h"
#include "common/unused.h"


int fuse_controller_no_rng_calibration_read_rng_calibration (
	const struct fuse_controller_interface *fuses,
	uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH])
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;

	if (fuses_hw == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	/* Return an empty calibration, which represents no valid calibration data. */
	memcpy (rng_data, 0, FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH);

	return 0;
}

int fuse_controller_no_rng_calibration_program_rng_calibration (
	const struct fuse_controller_interface *fuses,
	const uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH])
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;

	UNUSED (rng_data);

	if (fuses_hw == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	return FUSE_CONTROLLER_UNSUPPORTED;
}

/**
 * Initialize a driver for interfacing with the HSP fuse controller.  No RNG calibration data is
 * stored in the fuses.
 *
 * @param fuses The fuse driver instance to initialize.
 * @param state The variable context for a fuse driver instance.  This must be uninitialized.
 * @param regs Base address for the GFC registers.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int fuse_controller_no_rng_calibration_init (struct fuse_controller *fuses,
	struct fuse_controller_state *state, struct Gfc_regs *regs)
{
	int status;

	status = fuse_controller_init (fuses, state, regs);
	if (status == 0) {
		fuses->base.read_rng_calibration = fuse_controller_no_rng_calibration_read_rng_calibration;
		fuses->base.program_rng_calibration =
			fuse_controller_no_rng_calibration_program_rng_calibration;
	}

	return status;
}
