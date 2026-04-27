// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "fuse_controller_manticore_fips.h"
#include "fuse_controller_manticore_fips_cmvp.h"
#include "common/unused.h"
#include "dc_scm/sp_boot.h"
#include "fips/cmvp_test_case.h"


/**
 * The RNG calibration to use to trigger REPCNT faults.
 * - clk_div: 0x60 (bits 0:9)
 * - repcnt_cutoff: 0x08 (bits 10:19)
 * - apt_cutoff: 0x322 (bits 20:29)
 * - chisq_cutoff: 0x10d (bits 30:39)
 */
static const uint8_t FUSE_CONTROLLER_MANTICORE_FIPS_CMVP_REPCNT_FAULT[
	FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH] = {
	0x60, 0x20, 0x20, 0x72, 0x43
};

/**
 * The RNG calibration to use to trigger APT faults.
 * - clk_div: 0x60 (bits 0:9)
 * - repcnt_cutoff: 0x33 (bits 10:19)
 * - apt_cutoff: 0x0a (bits 20:29)
 * - chisq_cutoff: 0x10d (bits 30:39)
 */
static const uint8_t FUSE_CONTROLLER_MANTICORE_FIPS_CMVP_APT_FAULT[
	FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH] = {
	0x60, 0xcc, 0xa0, 0x40, 0x43
};


int fuse_controller_manticore_fips_cmvp_read_rng_calibration (
	const struct fuse_controller_interface *fuses,
	uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH])
{
	enum cmvp_test_case_cast_algorithm algo = CMVP_TEST_CASE_ALGORITHM_NONE;

	if (fuses == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	if ((cmvp_test != 0) &&
		(cmvp_test_case_get_test_type (cmvp_test) == CMVP_TEST_CASE_CAST_NEGATIVE_TEST) &&
		(cmvp_test_case_get_cast_type (cmvp_test) == CMVP_TEST_CASE_CAST_TYPE_PRE_OPERATIONAL)) {
		algo = cmvp_test_case_get_cast_algorithm (cmvp_test);
	}

	switch (algo) {
		case CMVP_TEST_CASE_ALGORITHM_RNG_RCT_HEALTH:
			memcpy (rng_data, FUSE_CONTROLLER_MANTICORE_FIPS_CMVP_REPCNT_FAULT,
				FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH);
			break;

		case CMVP_TEST_CASE_ALGORITHM_RNG_APT_HEALTH:
			memcpy (rng_data, FUSE_CONTROLLER_MANTICORE_FIPS_CMVP_APT_FAULT,
				FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH);
			break;

		default:
			return fuse_controller_manticore_fips_read_rng_calibration (fuses, rng_data);
	}

	return 0;
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
int fuse_controller_manticore_fips_cmvp_init (struct fuse_controller *fuses,
	struct fuse_controller_state *state, struct Gfc_regs *regs)
{
	int status;

	status = fuse_controller_init (fuses, state, regs);
	if (status == 0) {
		fuses->base.read_rng_calibration = fuse_controller_manticore_fips_cmvp_read_rng_calibration;
		fuses->base.program_rng_calibration =
			fuse_controller_manticore_fips_program_rng_calibration;
	}

	return status;
}
