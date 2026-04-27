// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "hw_rot_firmware_package.h"
#include "common/unused.h"


/**
 * Determine the offset of the current redundant copy of the SVN.
 *
 * @param fuse Pointer to the fuse structure holding the SVN values.
 * @param x Index for the desired redundant copy.
 */
#define	HW_ROT_FIRMWARE_PACKAGE_SVN_OFFSET(fuse, x)\
	((uint8_t*) &fuse->data[x].svn_fw_package - (uint8_t*) fuse)


int hw_rot_firmware_package_has_root_key (const struct hw_rot *rot)
{
	if (rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* The firmware package will always have a root key. */
	return 0;
}

int hw_rot_firmware_package_get_root_key_hash (const struct hw_rot *rot,
	const struct hash_engine *hash, enum hash_type type, uint8_t *digest, size_t length)
{
	UNUSED (type);
	UNUSED (length);

	if ((rot == NULL) || (hash == NULL) || (digest == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* The root key is not known to the RoT. */
	return HW_ROT_UNSUPPORTED;
}

int hw_rot_firmware_package_verify_root_key (const struct hw_rot *rot, const uint8_t *root_key,
	size_t length, const struct hash_engine *hash)
{
	if ((rot == NULL) || (root_key == NULL) || (length == 0) || (hash == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* The root key is not known here, so it is always valid. */
	return 0;
}

int hw_rot_firmware_package_has_free_root_key_slots (const struct hw_rot *rot)
{
	if (rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* There are no root key slots for this RoT. */
	return HW_ROT_UNSUPPORTED;
}

int hw_rot_firmware_package_update_root_key (const struct hw_rot *rot, const uint8_t *root_key,
	size_t length, const struct hash_engine *hash)
{
	if ((rot == NULL) || (root_key == NULL) || (length == 0) || (hash == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* Root key cannot be updated. */
	return HW_ROT_UNSUPPORTED;
}

int hw_rot_firmware_package_get_svn (const struct hw_rot *rot, uint64_t *svn)
{
	const struct hw_rot_firmware_package *key_rot =
		(const struct hw_rot_firmware_package*) rot;
	struct hsp_fuses_sw1 *sw1;
	size_t i;

	if ((key_rot == NULL) || (svn == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	sw1 = (struct hsp_fuses_sw1*) key_rot->fuse_regs->SW1_fuse.SW1_fuse;
	*svn = 0;

	/* Read all redundant copies of the SVN data and set all bits. */
	for (i = 0; i < (sizeof (sw1->data) / sizeof (sw1->data[0])); i++) {
		*svn |= sw1->data[i].svn_fw_package;
	}

	return 0;
}

int hw_rot_firmware_package_get_svn_bits (const struct hw_rot *rot)
{
	if (rot) {
		return 64;
	}
	else {
		return 0;
	}
}

int hw_rot_firmware_package_update_svn (const struct hw_rot *rot, uint64_t svn)
{
	const struct hw_rot_firmware_package *key_rot =
		(const struct hw_rot_firmware_package*) rot;
	struct hsp_fuses_sw1 *sw1;
	size_t i;
	int status;

	if (key_rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	sw1 = (struct hsp_fuses_sw1*) key_rot->fuse_regs->SW1_fuse.SW1_fuse;

	/* Program all redundant copies of the SVN.  Do not fail on an error for a single copy. */
	status = -1;
	for (i = 0; i < (sizeof (sw1->data) / sizeof (sw1->data[0])); i++) {
		int prog_status = key_rot->fuses->program_sw_fuses (key_rot->fuses,
			HSP_FUSES_ADDRESS (SW1) + HW_ROT_FIRMWARE_PACKAGE_SVN_OFFSET (sw1, i), (uint32_t*) &svn,
			2);

		if (status != 0) {
			/* Latch successful program operations, or report the last failure. */
			status = prog_status;
		}
	}

	return status;
}

int hw_rot_firmware_package_refresh_svn (const struct hw_rot *rot, uint64_t svn)
{
	const struct hw_rot_firmware_package *key_rot =
		(const struct hw_rot_firmware_package*) rot;
	struct hsp_fuses_sw1 *sw1;
	size_t i;
	int status = 0;

	if (key_rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	sw1 = (struct hsp_fuses_sw1*) key_rot->fuse_regs->SW1_fuse.SW1_fuse;

	for (i = 0; i < (sizeof (sw1->data) / sizeof (sw1->data[0])); i++) {
		if (sw1->data[i].svn_fw_package != svn) {
			int prog_status = key_rot->fuses->program_sw_fuses (key_rot->fuses,
				HSP_FUSES_ADDRESS (SW1) + HW_ROT_FIRMWARE_PACKAGE_SVN_OFFSET (sw1, i),
				(uint32_t*) &svn, 2);

			if (prog_status != 0) {
				status = prog_status;
			}
		}
	}

	return status;
}

int hw_rot_firmware_package_has_active_tenancy (const struct hw_rot *rot)
{
	if (rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* Firmware package does not support tenancy transfers. */
	return HW_ROT_UNSUPPORTED;
}

int hw_rot_firmware_package_get_tenancy_counter (const struct hw_rot *rot, uint8_t *counter,
	size_t length)
{
	UNUSED (length);

	if ((rot == NULL) || (counter == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* Firmware package does not support tenancy transfers. */
	return HW_ROT_UNSUPPORTED;
}

int hw_rot_firmware_package_get_tenancy_grant_token (const struct hw_rot *rot,
	const uint8_t *tenant_key, size_t key_length, uint8_t *token, size_t length)
{
	UNUSED (key_length);
	UNUSED (length);

	if ((rot == NULL) || (tenant_key == NULL) || (token == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* Firmware package does not support tenancy transfers. */
	return HW_ROT_UNSUPPORTED;
}

int hw_rot_firmware_package_tenancy_transfer (const struct hw_rot *rot, bool grant)
{
	UNUSED (grant);

	if (rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* Firmware package does not support tenancy transfers. */
	return HW_ROT_UNSUPPORTED;
}

/**
 * Initialize the RoT for the main firmware package.
 *
 * @param rot The RoT to initialize.
 * @param fuses Driver interface to the HSP fuses where security information is stored.
 * @param fuse_regs Register interface to the fuse controller.  This contains cached information
 * for most fuse slots.
 *
 * @return 0 if the RoT was initialized successfully or an error code.
 */
int hw_rot_firmware_package_init (struct hw_rot_firmware_package *rot,
	const struct fuse_controller_interface *fuses, struct Gfc_regs *fuse_regs)
{
	if ((rot == NULL) || (fuses == NULL) || (fuse_regs == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	memset (rot, 0, sizeof (struct hw_rot_firmware_package));

	rot->base.has_root_key = hw_rot_firmware_package_has_root_key;
	rot->base.get_root_key_hash = hw_rot_firmware_package_get_root_key_hash;
	rot->base.verify_root_key = hw_rot_firmware_package_verify_root_key;
	rot->base.has_free_root_key_slots = hw_rot_firmware_package_has_free_root_key_slots;
	rot->base.update_root_key = hw_rot_firmware_package_update_root_key;
	rot->base.get_svn = hw_rot_firmware_package_get_svn;
	rot->base.get_svn_bits = hw_rot_firmware_package_get_svn_bits;
	rot->base.update_svn = hw_rot_firmware_package_update_svn;
	rot->base.refresh_svn = hw_rot_firmware_package_refresh_svn;
	rot->base.has_active_tenancy = hw_rot_firmware_package_has_active_tenancy;
	rot->base.get_tenancy_counter = hw_rot_firmware_package_get_tenancy_counter;
	rot->base.get_tenancy_grant_token = hw_rot_firmware_package_get_tenancy_grant_token;
	rot->base.tenancy_transfer = hw_rot_firmware_package_tenancy_transfer;

	rot->fuses = fuses;
	rot->fuse_regs = fuse_regs;

	return 0;
}

/**
 * Release the resources used by the main firmware package RoT.
 *
 * @param rot The RoT instance to release.
 */
void hw_rot_firmware_package_release (const struct hw_rot_firmware_package *rot)
{
	UNUSED (rot);
}
