// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HW_ROT_HSP_ROM_NO_FUSES_H_
#define HW_ROT_HSP_ROM_NO_FUSES_H_

#include <stdbool.h>
#include "firmware/hw_rot_hsp_rom.h"

/**
 * RoT implementation for run-time firmware to execute the same checks as ROM.  Raw fuse access is
 * not permitted in this context and everything is read from memory or cached fuse registers.
 */
struct hw_rot_hsp_rom_no_fuses {
	struct hw_rot_hsp_rom base;	/**< Base RoT instance. */
};


int hw_rot_hsp_rom_no_fuses_init (struct hw_rot_hsp_rom_no_fuses *rot,
	struct hw_rot_hsp_rom_state *state, const struct fuse_controller_interface *fuses,
	struct Gfc_regs *fuse_regs, const struct ccs_ksu_interface *ccs, const SP_MSG_384 *mfg_root_key,
	uint8_t root_slots, struct hw_rot_hsp_rom_tenancy_buffer *tenancy_buffer,
	const SP_ECDSA_P384_PUBLIC *root_key, const struct hash_engine *hash,
	const union hw_rot_hsp_rom_tenancy_counter *tenancy_counter);
int hw_rot_hsp_rom_no_fuses_init_state (const struct hw_rot_hsp_rom_no_fuses *rot,
	const SP_ECDSA_P384_PUBLIC *root_key, const struct hash_engine *hash,
	const union hw_rot_hsp_rom_tenancy_counter *tenancy_counter);

void hw_rot_hsp_rom_no_fuses_release (const struct hw_rot_hsp_rom_no_fuses *rot);


#endif	/* HW_ROT_HSP_ROM_NO_FUSES_H_ */
