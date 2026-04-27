// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FUSE_CONTROLLER_SW0_RNG_CALIBRATION_ECC_H_
#define FUSE_CONTROLLER_SW0_RNG_CALIBRATION_ECC_H_

#include "drivers/fuse_controller.h"


int fuse_controller_sw0_rng_calibration_ecc_init (struct fuse_controller *fuses,
	struct fuse_controller_state *state, struct Gfc_regs *regs);


#endif	/* FUSE_CONTROLLER_SW0_RNG_CALIBRATION_ECC_H_ */
