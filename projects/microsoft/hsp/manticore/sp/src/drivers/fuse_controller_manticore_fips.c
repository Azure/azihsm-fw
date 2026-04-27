// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "fuse_controller_manticore_fips.h"
#include "common/unused.h"


/**
 * The FIPS approved RNG calibration to use with Manticore.  The calibration uses the following
 * values.  Each field is encoded as 10 bits.
 * - clk_div: 0x60 (bits 0:9)
 * - repcnt_cutoff: 0x33 (bits 10:19)
 * - apt_cutoff: 0x322 (bits 20:29)
 * - chisq_cutoff: 0x10d (bits 30:39)
 */
static const uint8_t FUSE_CONTROLLER_MANTICORE_FIPS_RNG_CALIBRATION[
	FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH] = {
	0x60, 0xcc, 0x20, 0x72, 0x43
};


int fuse_controller_manticore_fips_read_rng_calibration (
	const struct fuse_controller_interface *fuses,
	uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH])
{
	if (fuses == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	memcpy (rng_data, FUSE_CONTROLLER_MANTICORE_FIPS_RNG_CALIBRATION,
		FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH);

	return 0;
}

int fuse_controller_manticore_fips_program_rng_calibration (
	const struct fuse_controller_interface *fuses,
	const uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH])
{
	if (fuses == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	UNUSED (rng_data);

	/* RNG calibration cannot be changed. */
	return FUSE_CONTROLLER_UNSUPPORTED;
}

/**
 * Initialize a driver for interfacing with the HSP fuse controller.  RNG calibration data is hard-
 * coded to use the FIPS approved values.
 *
 * @param fuses The fuse driver instance to initialize.
 * @param state The variable context for a fuse driver instance.  This must be uninitialized.
 * @param regs Base address for the GFC registers.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int fuse_controller_manticore_fips_init (struct fuse_controller *fuses,
	struct fuse_controller_state *state, struct Gfc_regs *regs)
{
	int status;

	status = fuse_controller_init (fuses, state, regs);
	if (status == 0) {
		fuses->base.read_rng_calibration = fuse_controller_manticore_fips_read_rng_calibration;
		fuses->base.program_rng_calibration =
			fuse_controller_manticore_fips_program_rng_calibration;
	}

	return status;
}
