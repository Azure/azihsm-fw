// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FUSE_CONTROLLER_EXT_POWER_H_
#define FUSE_CONTROLLER_EXT_POWER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "platform_api.h"
#include "drivers/ext_power_interface.h"
#include "drivers/fuse_controller_interface.h"


/**
 * Implementation of the fuse controller driver interface to execute commands against hardware blocks.
 */
struct fuse_controller_ext_power {
	struct fuse_controller_interface base;			/**< External power interface wrapper for fuse commands. */
	const struct fuse_controller_interface *fuses;	/**< Base driver interface for Fuse commands. */
	const struct ext_power_interface *power;		/**< External power control interface. */
};


int fuse_controller_ext_power_init (struct fuse_controller_ext_power *fuses_ext_pwr,
	const struct fuse_controller_interface *fuses, const struct ext_power_interface *power);
void fuse_controller_ext_power_release (struct fuse_controller_ext_power *fuses_ext_pwr);


#endif	/* FUSE_CONTROLLER_EXT_POWER_H_ */
