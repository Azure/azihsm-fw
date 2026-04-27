// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "fuse_controller_sw0_rng_calibration_ecc.h"
#include "hsp_top.h"
#include "drivers/hsp_fuses.h"

/**
 * Length of the total RNG calibration data to apply to the hardware.
 */
#define	FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH_TOTAL		20


#pragma pack(push, 1)
/**
 * Data to be used for RNG FIPS calibration.
 */
union fuse_controller_rng_calibration_ecc {
	uint8_t bytes[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH_TOTAL];	/**< Total raw bytes of calibration data. */
	uint32_t words[5];												/**< Fuse words used for the data.  Unused bits should be zero-padded. */
};

#pragma pack(pop)


/**
 * Bitwise OR the DEST data with the SRC data and copy the resultant data to the DEST
 * This is done for num_bytes
 *
 * There is no validation done on the arguments.
 *
 * @param dest The pointer to destination.
 * @param src The pointer to source.
 * @param num_bytes Number of total bytes.
 */
static void fuse_controller_sw0_rng_calibration_ecc_bitwise_or (uint8_t *dest, const uint8_t *src,
	size_t num_bytes)
{
	size_t i;

	for (i = 0; i < num_bytes; ++i) {
		dest[i] |= src[i];
	}
}

int fuse_controller_sw0_rng_calibration_read_rng_calibration_ecc (
	const struct fuse_controller_interface *fuses,
	uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH])
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;
	union fuse_controller_rng_calibration_ecc rng_prog;
	int i;

	if (fuses_hw == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	/* RNG calibration is stored in the first 160 bits of the SW0 fuse slot.  No need to read from
	 * the fuse array since this value was sensed during fuse initialization.  But the register is
	 * not byte addressable, so we need to cache the data in a word array. */
	for (i = 0; i < 5; i++) {
		rng_prog.words[i] = fuses_hw->regs->SW0_fuse.SW0_fuse[i];
	}

	/* Copy the first 40 bits */
	memcpy (rng_data, rng_prog.bytes, FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH);

	/* Bitwise OR and copy the rest 120 bits */
	for (i = 1; i < 4; i++) {
		fuse_controller_sw0_rng_calibration_ecc_bitwise_or (rng_data,
			&rng_prog.bytes[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH * i],
			FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH);
	}

	return 0;
}

int fuse_controller_sw0_rng_calibration_program_rng_calibration_ecc (
	const struct fuse_controller_interface *fuses,
	const uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH])
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;
	union fuse_controller_rng_calibration_ecc rng_prog = {0};
	int i;
	int status;

	if (fuses_hw == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	/* Make 4 copies of RNG data */
	for (i = 0; i < 4; i++) {
		memcpy (&rng_prog.bytes[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH * i], rng_data,
			FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH);
	}

	/* Write 4 copies of RNG data (160 bits) in 5 fuse words */
	for (i = 0; i < 5; i++) {
		status = fuse_controller_execute_command (fuses_hw, FUSE_CONTROLLER_CMD_PROGRAM_DATA,
			HSP_FUSES_ADDRESS (SW0) + (i * sizeof (uint32_t)), &rng_prog.words[i], 1);
		if (status != 0) {
			return status;
		}
	}

	return 0;
}

/**
 * Initialize a driver for interfacing with the HSP fuse controller. Multiple copies of RNG
 * calibration data is stored at the beginning of the SW0 fuse slot.
 *
 * @param fuses The fuse driver instance to initialize.
 * @param state The variable context for a fuse driver instance.  This must be uninitialized.
 * @param regs Base address for the GFC registers.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int fuse_controller_sw0_rng_calibration_ecc_init (struct fuse_controller *fuses,
	struct fuse_controller_state *state, struct Gfc_regs *regs)
{
	int status;

	status = fuse_controller_init (fuses, state, regs);
	if (status == 0) {
		fuses->base.read_rng_calibration =
			fuse_controller_sw0_rng_calibration_read_rng_calibration_ecc;
		fuses->base.program_rng_calibration =
			fuse_controller_sw0_rng_calibration_program_rng_calibration_ecc;
	}

	return status;
}
