// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef KEY_MANIFEST_HSP_ROM_DUAL_ROOT_STATIC_H_
#define KEY_MANIFEST_HSP_ROM_DUAL_ROOT_STATIC_H_

#include "firmware/key_manifest_hsp_rom_dual_root.h"
#include "firmware/key_manifest_hsp_rom_static.h"


/* Internal functions declared to allow for static initialization. */
int key_manifest_hsp_rom_dual_root_verify (const struct key_manifest *manifest,
	const struct hash_engine *hash);
int key_manifest_hsp_rom_dual_root_update_root_key (const struct key_manifest_hsp_rom *rom,
	const struct hash_engine *hash);
int key_manifest_hsp_rom_dual_root_init_state_from_memory (const struct key_manifest_hsp_rom *rom,
	const uint8_t *base_addr);
int key_manifest_hsp_rom_dual_root_validate_owner_key_buffer (
	const struct key_manifest_hsp_rom *manifest, const struct hash_engine *hash,
	uint8_t **key_buffer_out, size_t *key_buffer_out_length);
size_t key_manifest_hsp_rom_dual_root_get_total_size (const struct key_manifest_hsp_rom *rom);
const struct key_manifest_public_key* key_manifest_hsp_rom_dual_root_get_authority_key (
	const struct key_manifest_hsp_rom_dual_root *rom);


int key_manifest_hsp_rom_dual_root_init_state (const struct key_manifest_hsp_rom *rom,
	const struct flash *flash, uint32_t base_addr);


/**
 * Constant initializer for the key manifest base API.
 */
#define	KEY_MANIFEST_HSP_ROM_DUAL_ROOT_BASE_API_INIT  { \
		.verify = key_manifest_hsp_rom_dual_root_verify, \
		.is_allowed = key_manifest_hsp_rom_is_allowed, \
		.revokes_old_manifest = key_manifest_hsp_rom_revokes_old_manifest, \
		.update_revocation = key_manifest_hsp_rom_update_revocation, \
		.get_root_key = key_manifest_hsp_rom_get_root_key, \
		.get_app_key = key_manifest_hsp_rom_get_app_key, \
		.get_manifest_key = key_manifest_hsp_rom_get_manifest_key \
	}

/**
 * Constant initializer for the key manifest single root API.
 */
#define	KEY_MANIFEST_HSP_ROM_DUAL_ROOT_SINGLE_ROOT_API_INIT(state_ptr, rot_ptr, pka_ptr)  { \
		.base = KEY_MANIFEST_HSP_ROM_DUAL_ROOT_BASE_API_INIT, \
		.init_state = key_manifest_hsp_rom_dual_root_init_state, \
		.init_state_from_memory = key_manifest_hsp_rom_dual_root_init_state_from_memory, \
		.get_total_size = key_manifest_hsp_rom_dual_root_get_total_size, \
		.get_svn = key_manifest_hsp_rom_get_svn, \
		.get_secondary_key = key_manifest_hsp_rom_get_secondary_key, \
		.get_tenancy_grant_key = key_manifest_hsp_rom_get_tenancy_grant_key, \
		.is_ownership_transfer = key_manifest_hsp_rom_is_ownership_transfer, \
		.update_root_key = key_manifest_hsp_rom_dual_root_update_root_key, \
		.is_tenancy_transfer = key_manifest_hsp_rom_is_tenancy_transfer, \
		.is_tenancy_grant = key_manifest_hsp_rom_is_tenancy_grant, \
		.update_tenancy_counter = key_manifest_hsp_rom_update_tenancy_counter, \
		.authenticate_ownership_transfer_root_key_buffer = \
			key_manifest_hsp_rom_dual_root_validate_owner_key_buffer, \
		.state = state_ptr.base, \
		.rot = rot_ptr, \
		.pka = pka_ptr, \
		.root_key = { \
			.type = KEY_MANIFEST_ECC_KEY, \
			.key = { \
				.ecc = &(state_ptr)->base.root_key \
			}\
		}, \
		.fw_key = { \
			.type = KEY_MANIFEST_ECC_KEY, \
			.key = {\
				.ecc = &(state_ptr)->base.fw_key \
			} \
		}, \
		.secondary_key = { \
			.type = KEY_MANIFEST_ECC_KEY, \
			.key = { \
				.ecc = &(state_ptr)->base.secondary_key \
			} \
		}, \
		.tenancy_grant_key = { \
			.type = KEY_MANIFEST_ECC_KEY, \
			.key = { \
				.ecc = &(state_ptr)->base.tenancy_grant_key \
			}\
		}, \
	}

/**
 * Initialize a static instance of a key manifest parser for HSP ROM to use for 1SP validation.
 * This does not initialize the manifest state.  That will need to be initialized separately with
 * key_manifest_hsp_rom_init_state or key_manifest_hsp_rom_init_state_from_memory.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the manifest.
 * @param rot_ptr Interface to the HW RoT state.  This can be a constant instance.
 * @param pka_ptr Interface to the PKA engine for signature verification.  This can be a constant
 * instance.
 */
#define	key_manifest_hsp_rom_dual_root_static_init(state_ptr, rot_ptr, pka_ptr)	{ \
		.base = KEY_MANIFEST_HSP_ROM_DUAL_ROOT_SINGLE_ROOT_API_INIT(state_ptr, rot_ptr, pka_ptr), \
		.state = state_ptr, \
		.get_root_authority_key = key_manifest_hsp_rom_dual_root_get_authority_key, \
		.authority_key = { \
			.type = KEY_MANIFEST_ECC_KEY, \
			.key = { \
				.ecc = &(state_ptr)->authority_key \
			}\
		}, \
	}


#endif	/* KEY_MANIFEST_HSP_ROM_DUAL_ROOT_STATIC_H_ */
