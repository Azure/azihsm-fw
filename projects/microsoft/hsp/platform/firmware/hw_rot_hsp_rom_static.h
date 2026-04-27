// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HW_ROT_HSP_ROM_STATIC_H_
#define HW_ROT_HSP_ROM_STATIC_H_

#include "firmware/hw_rot_hsp_rom.h"


/* Internal functions declared to allow for static initialization. */
int hw_rot_hsp_rom_has_root_key (const struct hw_rot *rot);
int hw_rot_hsp_rom_get_root_key_hash (const struct hw_rot *rot, const struct hash_engine *hash,
	enum hash_type type, uint8_t *digest, size_t length);
int hw_rot_hsp_rom_verify_root_key (const struct hw_rot *rot, const uint8_t *root_key,
	size_t length, const struct hash_engine *hash);
int hw_rot_hsp_rom_has_free_root_key_slots (const struct hw_rot *rot);
int hw_rot_hsp_rom_update_root_key (const struct hw_rot *rot, const uint8_t *root_key,
	size_t length, const struct hash_engine *hash);
int hw_rot_hsp_rom_get_svn (const struct hw_rot *rot, uint64_t *svn);
int hw_rot_hsp_rom_get_svn_bits (const struct hw_rot *rot);
int hw_rot_hsp_rom_update_svn (const struct hw_rot *rot, uint64_t svn);
int hw_rot_hsp_rom_refresh_svn (const struct hw_rot *rot, uint64_t svn);
int hw_rot_hsp_rom_has_active_tenancy (const struct hw_rot *rot);
int hw_rot_hsp_rom_get_tenancy_counter (const struct hw_rot *rot, uint8_t *counter, size_t length);
int hw_rot_hsp_rom_get_tenancy_grant_token (const struct hw_rot *rot, const uint8_t *tenant_key,
	size_t key_length, uint8_t *token, size_t length);
int hw_rot_hsp_rom_get_tenancy_grant_token_code_tracing (const struct hw_rot *rot,
	const uint8_t *tenant_key, size_t key_length, uint8_t *token, size_t length);
int hw_rot_hsp_rom_tenancy_transfer (const struct hw_rot *rot, bool grant);


/**
 * Constant initializer for the HSP ROM RoT API.
 */
#define	HW_ROT_HSP_ROM_API_INIT  { \
		.has_root_key = hw_rot_hsp_rom_has_root_key, \
		.get_root_key_hash = hw_rot_hsp_rom_get_root_key_hash, \
		.verify_root_key = hw_rot_hsp_rom_verify_root_key, \
		.has_free_root_key_slots = hw_rot_hsp_rom_has_free_root_key_slots, \
		.update_root_key = hw_rot_hsp_rom_update_root_key, \
		.get_svn = hw_rot_hsp_rom_get_svn, \
		.get_svn_bits = hw_rot_hsp_rom_get_svn_bits, \
		.update_svn = hw_rot_hsp_rom_update_svn, \
		.refresh_svn = hw_rot_hsp_rom_refresh_svn, \
		.has_active_tenancy = hw_rot_hsp_rom_has_active_tenancy, \
		.get_tenancy_counter = hw_rot_hsp_rom_get_tenancy_counter, \
		.get_tenancy_grant_token = hw_rot_hsp_rom_get_tenancy_grant_token, \
		.tenancy_transfer = hw_rot_hsp_rom_tenancy_transfer \
	}

/**
 * Constant initializer for the HSP ROM RoT API that include code path execution tracing.
 */
#define	HW_ROT_HSP_ROM_CODE_TRACING_API_INIT  { \
		.has_root_key = hw_rot_hsp_rom_has_root_key, \
		.get_root_key_hash = hw_rot_hsp_rom_get_root_key_hash, \
		.verify_root_key = hw_rot_hsp_rom_verify_root_key, \
		.has_free_root_key_slots = hw_rot_hsp_rom_has_free_root_key_slots, \
		.update_root_key = hw_rot_hsp_rom_update_root_key, \
		.get_svn = hw_rot_hsp_rom_get_svn, \
		.get_svn_bits = hw_rot_hsp_rom_get_svn_bits, \
		.update_svn = hw_rot_hsp_rom_update_svn, \
		.refresh_svn = hw_rot_hsp_rom_refresh_svn, \
		.has_active_tenancy = hw_rot_hsp_rom_has_active_tenancy, \
		.get_tenancy_counter = hw_rot_hsp_rom_get_tenancy_counter, \
		.get_tenancy_grant_token = hw_rot_hsp_rom_get_tenancy_grant_token_code_tracing, \
		.tenancy_transfer = hw_rot_hsp_rom_tenancy_transfer \
	}


/**
 * Initialize a static instance of a HW RoT for HSP ROM.
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
 * @param mfg_root_key_ptr Optional hash of the root key that should be used for firmware
 * verification when there are no other keys programmed into fuses.  If supplied, this will get used
 * in both Production and Secure states.  Setting this to null will disable root key operations
 * until a key is fused.
 * @param root_slot_count The number of SW ECC fuse slots that are used to store root keys.  The
 * first slot is assumed to be SW0_ECC.
 * @param tenancy_buffer Buffer to use for storage of the tenancy counter.
 * @param tenancy_fuse_addr Starting fuse address of the tenancy counter.
 */
#define	hw_rot_hsp_rom_static_init(api, state_ptr, fuses_ptr, fuse_reg_addr, ccs_ptr, \
	mfg_root_key_ptr, root_slot_count, tenancy_buffer, tenancy_fuse_addr)	{ \
		.base = api, \
		.state = state_ptr, \
		.fuses = fuses_ptr, \
		.fuse_regs = fuse_reg_addr, \
		.ccs = ccs_ptr, \
		.mfg_key = mfg_root_key_ptr, \
		.root_slots = root_slot_count, \
		.tenancy = tenancy_buffer, \
		.tenancy_addr = tenancy_fuse_addr \
	}


#endif	/* HW_ROT_HSP_ROM_STATIC_H_ */
