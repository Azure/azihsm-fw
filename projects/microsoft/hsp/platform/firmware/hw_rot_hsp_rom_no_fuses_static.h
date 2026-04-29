// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HW_ROT_HSP_ROM_NO_FUSES_STATIC_H_
#define HW_ROT_HSP_ROM_NO_FUSES_STATIC_H_

#include "firmware/hw_rot_hsp_rom_no_fuses.h"
#include "firmware/hw_rot_hsp_rom_static.h"


/* Internal functions declared to allow for static initialization. */
int hw_rot_hsp_rom_no_fuses_has_free_root_key_slots (const struct hw_rot *rot);
int hw_rot_hsp_rom_no_fuses_update_root_key (const struct hw_rot *rot, const uint8_t *root_key,
	size_t length, const struct hash_engine *hash);
int hw_rot_hsp_rom_no_fuses_update_svn (const struct hw_rot *rot, uint64_t svn);
int hw_rot_hsp_rom_no_fuses_tenancy_transfer (const struct hw_rot *rot, bool grant);


/**
 * Constant initializer for the HSP ROM RoT API that doesn't read or write fuses.
 */
#define	HW_ROT_HSP_ROM_NO_FUSES_API_INIT  { \
		.has_root_key = hw_rot_hsp_rom_has_root_key, \
		.get_root_key_hash = hw_rot_hsp_rom_get_root_key_hash, \
		.verify_root_key = hw_rot_hsp_rom_verify_root_key, \
		.has_free_root_key_slots = hw_rot_hsp_rom_no_fuses_has_free_root_key_slots, \
		.update_root_key = hw_rot_hsp_rom_no_fuses_update_root_key, \
		.get_svn = hw_rot_hsp_rom_get_svn, \
		.get_svn_bits = hw_rot_hsp_rom_get_svn_bits, \
		.update_svn = hw_rot_hsp_rom_no_fuses_update_svn, \
		.refresh_svn = hw_rot_hsp_rom_no_fuses_update_svn, \
		.has_active_tenancy = hw_rot_hsp_rom_has_active_tenancy, \
		.get_tenancy_counter = hw_rot_hsp_rom_get_tenancy_counter, \
		.get_tenancy_grant_token = hw_rot_hsp_rom_get_tenancy_grant_token, \
		.tenancy_transfer = hw_rot_hsp_rom_no_fuses_tenancy_transfer \
	}


/**
 * Initialize a static instance of a HW RoT for run-time code to execute the same checks as ROM.
 *
 * Read/write access to the fuses is assumed to not be available, so all checks are done against
 * information already in memory or in the fuse cache registers.
 *
 * There is no validation done on the arguments.
 *
 * @param api The API implementation that should be used.
 * @param state_ptr Variable context for the RoT.
 * @param fuses_ptr Driver interface to the HSP fuses where security information is stored.  This
 * can be a constant instance.
 * @param fuse_reg_addr Register interface to the fuse controller.  This contains cached information
 * for most fuse slots.
 * @param ccs_ptr Driver interface for executing crypto operations with HW managed keys.  This con
 * be a constant instance.
 * @param mfg_root_key_ptr Optional hash of the root key that will be used for firmware verification
 * when there are no other keys programmed into fuses.  Set to null if there is no root key when no
 * key is programmed.
 * @param root_slot_count The number of SW ECC fuse slots that are used to store root keys.  The
 * first slot is assumed to be SW0_ECC.
 * @param tenancy_buffer Buffer to use for storage of the tenancy counter.
 */
#define	hw_rot_hsp_rom_no_fuses_static_init(state_ptr, fuses_ptr, fuse_reg_addr, ccs_ptr, \
	mfg_root_key_ptr, root_slot_count, tenancy_buffer)	{ \
		.base = hw_rot_hsp_rom_static_init (HW_ROT_HSP_ROM_NO_FUSES_API_INIT, state_ptr, \
			fuses_ptr, fuse_reg_addr, ccs_ptr, mfg_root_key_ptr, root_slot_count, tenancy_buffer, \
			0) \
	}


#endif	/* HW_ROT_HSP_ROM_NO_FUSES_STATIC_H_ */
