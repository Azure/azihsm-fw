// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HW_ROT_FIRMWARE_PACKAGE_H_
#define HW_ROT_FIRMWARE_PACKAGE_H_

#include "drivers/fuse_controller_interface.h"
#include "drivers/hsp_fuses.h"
#include "firmware/hw_rot.h"


/**
 * Root-of-Trust for verifying the run-time firmware package against the current hardware state.
 */
struct hw_rot_firmware_package {
	struct hw_rot base;								/**< Base RoT interface. */
	const struct fuse_controller_interface *fuses;	/**< Interface to the HSP fuses. */
	struct Gfc_regs *fuse_regs;						/**< Register interface for the HSP fuses. */
};


int hw_rot_firmware_package_init (struct hw_rot_firmware_package *rot,
	const struct fuse_controller_interface *fuses, struct Gfc_regs *fuse_regs);
void hw_rot_firmware_package_release (const struct hw_rot_firmware_package *rot);


#endif	/* HW_ROT_FIRMWARE_PACKAGE_H_ */
