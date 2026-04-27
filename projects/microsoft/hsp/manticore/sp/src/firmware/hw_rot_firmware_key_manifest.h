// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HW_ROT_FIRMWARE_KEY_MANIFEST_H_
#define HW_ROT_FIRMWARE_KEY_MANIFEST_H_

#include "drivers/ccs_ksu_interface.h"
#include "drivers/fuse_controller_interface.h"
#include "drivers/hsp_fuses.h"
#include "firmware/hw_rot.h"
#include "splibs/inc/spcryptotypes.h"


/**
 * Root-of-Trust for verifying the firmware key manifest against the current hardware state.
 */
struct hw_rot_firmware_key_manifest {
	struct hw_rot base;								/**< Base RoT interface. */
	const struct fuse_controller_interface *fuses;	/**< Interface to the HSP fuses. */
	struct Gfc_regs *fuse_regs;						/**< Register interface for the HSP fuses. */
	const struct ccs_ksu_interface *ccs;			/**< Interface to the HW PCRs. */
	const SP_ECDSA_P384_PUBLIC *root_key;			/**< Root key for the firmware key manifest. */
	const SP_ECDSA_P384_PUBLIC *secondary_key;		/**< Secondary root key for the firmware key manifest. */
};


int hw_rot_firmware_key_manifest_init (struct hw_rot_firmware_key_manifest *rot,
	const struct fuse_controller_interface *fuses, struct Gfc_regs *fuse_regs,
	const struct ccs_ksu_interface *ccs, const SP_ECDSA_P384_PUBLIC *root_key,
	const SP_ECDSA_P384_PUBLIC *secondary_root_key);
void hw_rot_firmware_key_manifest_release (const struct hw_rot_firmware_key_manifest *rot);


#endif	/* HW_ROT_FIRMWARE_KEY_MANIFEST_H_ */
