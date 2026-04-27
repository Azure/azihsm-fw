// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "fuse_controller_sw0_rng_calibration.h"
#include "hsp_top.h"
#include "drivers/hsp_fuses.h"


#pragma pack(push, 1)
/**
 * Data to be used for RNG FIPS calibration.
 */
union fuse_controller_rng_calibration {
	uint8_t bytes[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH];	/**< Raw bytes of calibration data. */
	uint32_t words[2];										/**< Fuse words used for the data.  Unused bits should be zero-padded. */
};

#pragma pack(pop)


int fuse_controller_sw0_rng_calibration_read_rng_calibration (
	const struct fuse_controller_interface *fuses,
	uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH])
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;
	union fuse_controller_rng_calibration rng_prog;

	if (fuses_hw == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	/* RNG calibration is stored in the first bits of the SW0 fuse slot.  No need to read from the
	 * fuse array since this value was sensed during fuse initialization.  But the register is not
	 * byte addressable, so we need to cache the data in a word array. */
	rng_prog.words[0] = fuses_hw->regs->SW0_fuse.SW0_fuse[0];
	rng_prog.words[1] = fuses_hw->regs->SW0_fuse.SW0_fuse[1];

	memcpy (rng_data, rng_prog.bytes, FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH);

	return 0;
}

int fuse_controller_sw0_rng_calibration_program_rng_calibration (
	const struct fuse_controller_interface *fuses,
	const uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH])
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;
	union fuse_controller_rng_calibration rng_prog = {0};
	int i;
	int status;

	if (fuses_hw == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	memcpy (&rng_prog.bytes, rng_data, FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH);

	for (i = 0; i < 2; i++) {
		status = fuse_controller_execute_command (fuses_hw, FUSE_CONTROLLER_CMD_PROGRAM_DATA,
			HSP_FUSES_ADDRESS (SW0) + (i * sizeof (uint32_t)), &rng_prog.words[i], 1);
		if (status != 0) {
			return status;
		}
	}

	return 0;
}

/**
 * Initialize a driver for interfacing with the HSP fuse controller.  A single copy of RNG
 * calibration data is stored at the beginning of the SW0 fuse slot.
 *
 * @param fuses The fuse driver instance to initialize.
 * @param state The variable context for a fuse driver instance.  This must be uninitialized.
 * @param regs Base address for the GFC registers.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int fuse_controller_sw0_rng_calibration_init (struct fuse_controller *fuses,
	struct fuse_controller_state *state, struct Gfc_regs *regs)
{
	int status;

	status = fuse_controller_init (fuses, state, regs);
	if (status == 0) {
		fuses->base.read_rng_calibration = fuse_controller_sw0_rng_calibration_read_rng_calibration;
		fuses->base.program_rng_calibration =
			fuse_controller_sw0_rng_calibration_program_rng_calibration;
	}

	return status;
}
