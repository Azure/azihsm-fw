// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FUSE_CONTROLLER_HW_H_
#define FUSE_CONTROLLER_HW_H_

#include <stdint.h>
#include "drivers/fuse_controller.h"


/* These are fuse controller functions that are not abstracted to the HW-independent layer. */

int fuse_controller_set_program_pulse_counter (const struct fuse_controller *fuses,
	uint32_t hsp_clk);


#endif	/* FUSE_CONTROLLER_HW_H_ */
