// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HW_ROT_FIRMWARE_PACKAGE_STATIC_H_
#define HW_ROT_FIRMWARE_PACKAGE_STATIC_H_

#include  "firmware/hw_rot_firmware_package.h"


/* Internal functions declared to allow for static initialization. */
int hw_rot_firmware_package_has_root_key (const struct hw_rot *rot);
int hw_rot_firmware_package_get_root_key_hash (const struct hw_rot *rot,
	const struct hash_engine *hash, enum hash_type type, uint8_t *digest, size_t length);
int hw_rot_firmware_package_verify_root_key (const struct hw_rot *rot, const uint8_t *root_key,
	size_t length, const struct hash_engine *hash);
int hw_rot_firmware_package_has_free_root_key_slots (const struct hw_rot *rot);
int hw_rot_firmware_package_update_root_key (const struct hw_rot *rot, const uint8_t *root_key,
	size_t length, const struct hash_engine *hash);
int hw_rot_firmware_package_get_svn (const struct hw_rot *rot, uint64_t *svn);
int hw_rot_firmware_package_get_svn_bits (const struct hw_rot *rot);
int hw_rot_firmware_package_update_svn (const struct hw_rot *rot, uint64_t svn);
int hw_rot_firmware_package_refresh_svn (const struct hw_rot *rot, uint64_t svn);
int hw_rot_firmware_package_has_active_tenancy (const struct hw_rot *rot);
int hw_rot_firmware_package_get_tenancy_counter (const struct hw_rot *rot, uint8_t *counter,
	size_t length);
int hw_rot_firmware_package_get_tenancy_grant_token (const struct hw_rot *rot,
	const uint8_t *tenant_key, size_t key_length, uint8_t *token, size_t length);
int hw_rot_firmware_package_tenancy_transfer (const struct hw_rot *rot, bool grant);


/**
 * Constant initializer for the firmware package RoT API.
 */
#define	HW_ROT_FIRMWARE_PACKAGE_API_INIT  { \
		.has_root_key = hw_rot_firmware_package_has_root_key, \
		.get_root_key_hash = hw_rot_firmware_package_get_root_key_hash, \
		.verify_root_key = hw_rot_firmware_package_verify_root_key, \
		.has_free_root_key_slots = hw_rot_firmware_package_has_free_root_key_slots, \
		.update_root_key = hw_rot_firmware_package_update_root_key, \
		.get_svn = hw_rot_firmware_package_get_svn, \
		.get_svn_bits = hw_rot_firmware_package_get_svn_bits, \
		.update_svn = hw_rot_firmware_package_update_svn, \
		.refresh_svn = hw_rot_firmware_package_refresh_svn, \
		.has_active_tenancy = hw_rot_firmware_package_has_active_tenancy, \
		.get_tenancy_counter = hw_rot_firmware_package_get_tenancy_counter, \
		.get_tenancy_grant_token = hw_rot_firmware_package_get_tenancy_grant_token, \
		.tenancy_transfer = hw_rot_firmware_package_tenancy_transfer \
	}


/**
 * Initialize a static instance of a HW RoT for the main firmware package.
 *
 * There is no validation done on the arguments.
 *
 * @param fuses_ptr Driver interface to the HSP fuses where security information is stored.  This
 * can be a constant instance.
 * @param fuse_reg_addr Register interface to the fuse controller.  This contains cached information
 * for most fuse slots.
 */
#define	hw_rot_firmware_package_static_init(fuses_ptr, fuse_reg_addr)	{ \
		.base = HW_ROT_FIRMWARE_PACKAGE_API_INIT, \
		.fuses = fuses_ptr, \
		.fuse_regs = fuse_reg_addr, \
	}


#endif	/* HW_ROT_FIRMWARE_PACKAGE_STATIC_H_ */
