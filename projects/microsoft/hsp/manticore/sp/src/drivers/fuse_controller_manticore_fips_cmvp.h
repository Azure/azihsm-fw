// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FUSE_CONTROLLER_MANTICORE_FIPS_CMVP_H_
#define FUSE_CONTROLLER_MANTICORE_FIPS_CMVP_H_

#include "drivers/fuse_controller.h"


int fuse_controller_manticore_fips_cmvp_init (struct fuse_controller *fuses,
	struct fuse_controller_state *state, struct Gfc_regs *regs);


#endif	/* FUSE_CONTROLLER_MANTICORE_FIPS_CMVP_H_ */
