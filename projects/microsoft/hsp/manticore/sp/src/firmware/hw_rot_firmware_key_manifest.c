// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "hw_rot_firmware_key_manifest.h"
#include "asn1/ecc_der_util.h"
#include "common/buffer_util.h"
#include "common/unused.h"


/**
 * Determine the offset of the current redundant copy of the SVN.
 *
 * @param fuse Pointer to the fuse structure holding the SVN values.
 * @param x Index for the desired redundant copy.
 */
#define	HW_ROT_FIRMWARE_KEY_MANIFEST_SVN_OFFSET(fuse, x)    \
	((uint8_t*) &fuse->data[x].svn_fw_key_manifest - (uint8_t*) fuse)


int hw_rot_firmware_key_manifest_has_root_key (const struct hw_rot *rot)
{
	if (rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* The key manifest will always have a root key. */
	return 0;
}

int hw_rot_firmware_key_manifest_get_root_key_hash (const struct hw_rot *rot,
	const struct hash_engine *hash, enum hash_type type, uint8_t *digest, size_t length)
{
	UNUSED (type);
	UNUSED (length);

	if ((rot == NULL) || (hash == NULL) || (digest == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* No real need to support this feature. */
	return HW_ROT_UNSUPPORTED;
}

int hw_rot_firmware_key_manifest_verify_root_key (const struct hw_rot *rot, const uint8_t *root_key,
	size_t length, const struct hash_engine *hash)
{
	const struct hw_rot_firmware_key_manifest *key_rot =
		(const struct hw_rot_firmware_key_manifest*) rot;
	SP_ECDSA_P384_PUBLIC raw_key;
	int status;

	if ((key_rot == NULL) || (root_key == NULL) || (length == 0) || (hash == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	status = ecc_der_decode_public_key (root_key, length, raw_key.Parts.X.AsBytes,
		raw_key.Parts.Y.AsBytes, SP_MSG_384_SIZE);
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	status = buffer_compare (raw_key.AsBytes, key_rot->root_key->AsBytes,
		SP_ECDSA_P384_PUBLIC_KEY_SIZE);
	if (status != 0) {
		if (key_rot->secondary_key) {
			status = buffer_compare (raw_key.AsBytes, key_rot->secondary_key->AsBytes,
				SP_ECDSA_P384_PUBLIC_KEY_SIZE);
		}

		if (status != 0) {
			return HW_ROT_BAD_ROOT_KEY;
		}
	}

	/* TODO: Further check the root key against the ROM measurements log.  Calculate the expected
	 * PCR value and compare to the HW PCR.  This will prove the memory contents are good. */
	return 0;
}

int hw_rot_firmware_key_manifest_has_free_root_key_slots (const struct hw_rot *rot)
{
	if (rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* There are no root key slots for this RoT. */
	return HW_ROT_UNSUPPORTED;
}

int hw_rot_firmware_key_manifest_update_root_key (const struct hw_rot *rot, const uint8_t *root_key,
	size_t length, const struct hash_engine *hash)
{
	if ((rot == NULL) || (root_key == NULL) || (length == 0) || (hash == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* Root key cannot be updated. */
	return HW_ROT_UNSUPPORTED;
}

int hw_rot_firmware_key_manifest_get_svn (const struct hw_rot *rot, uint64_t *svn)
{
	const struct hw_rot_firmware_key_manifest *key_rot =
		(const struct hw_rot_firmware_key_manifest*) rot;
	struct hsp_fuses_sw1 *sw1;
	size_t i;

	if ((key_rot == NULL) || (svn == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	sw1 = (struct hsp_fuses_sw1*) key_rot->fuse_regs->SW1_fuse.SW1_fuse;
	*svn = 0;

	/* Read all redundant copies of the SVN data and set all bits. */
	for (i = 0; i < (sizeof (sw1->data) / sizeof (sw1->data[0])); i++) {
		*svn |= sw1->data[i].svn_fw_key_manifest;
	}

	return 0;
}

int hw_rot_firmware_key_manifest_get_svn_bits (const struct hw_rot *rot)
{
	if (rot) {
		return 64;
	}
	else {
		return 0;
	}
}

int hw_rot_firmware_key_manifest_update_svn (const struct hw_rot *rot, uint64_t svn)
{
	const struct hw_rot_firmware_key_manifest *key_rot =
		(const struct hw_rot_firmware_key_manifest*) rot;
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
			HSP_FUSES_ADDRESS (SW1) + HW_ROT_FIRMWARE_KEY_MANIFEST_SVN_OFFSET (sw1, i),
			(uint32_t*) &svn, 2);

		if (status != 0) {
			/* Latch successful program operations, or report the last failure. */
			status = prog_status;
		}
	}

	return status;
}

int hw_rot_firmware_key_manifest_refresh_svn (const struct hw_rot *rot, uint64_t svn)
{
	const struct hw_rot_firmware_key_manifest *key_rot =
		(const struct hw_rot_firmware_key_manifest*) rot;
	struct hsp_fuses_sw1 *sw1;
	size_t i;
	int status = 0;

	if (key_rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	sw1 = (struct hsp_fuses_sw1*) key_rot->fuse_regs->SW1_fuse.SW1_fuse;

	for (i = 0; i < (sizeof (sw1->data) / sizeof (sw1->data[0])); i++) {
		if (sw1->data[i].svn_fw_key_manifest != svn) {
			int prog_status = key_rot->fuses->program_sw_fuses (key_rot->fuses,
				HSP_FUSES_ADDRESS (SW1) + HW_ROT_FIRMWARE_KEY_MANIFEST_SVN_OFFSET (sw1, i),
				(uint32_t*) &svn, 2);

			if (prog_status != 0) {
				status = prog_status;
			}
		}
	}

	return status;
}

int hw_rot_firmware_key_manifest_has_active_tenancy (const struct hw_rot *rot)
{
	if (rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* Firmware key manifest does not support tenancy transfers. */
	return HW_ROT_UNSUPPORTED;
}

int hw_rot_firmware_key_manifest_get_tenancy_counter (const struct hw_rot *rot, uint8_t *counter,
	size_t length)
{
	UNUSED (length);

	if ((rot == NULL) || (counter == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* Firmware key manifest does not support tenancy transfers. */
	return HW_ROT_UNSUPPORTED;
}

int hw_rot_firmware_key_manifest_get_tenancy_grant_token (const struct hw_rot *rot,
	const uint8_t *tenant_key, size_t key_length, uint8_t *token, size_t length)
{
	UNUSED (key_length);
	UNUSED (length);

	if ((rot == NULL) || (tenant_key == NULL) || (token == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* Firmware key manifest does not support tenancy transfers. */
	return HW_ROT_UNSUPPORTED;
}

int hw_rot_firmware_key_manifest_tenancy_transfer (const struct hw_rot *rot, bool grant)
{
	UNUSED (grant);

	if (rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	/* Firmware key manifest does not support tenancy transfers. */
	return HW_ROT_UNSUPPORTED;
}

/**
 * Initialize the RoT for the firmware key manifest.
 *
 * @param rot The RoT to initialize.
 * @param fuses Driver interface to the HSP fuses where security information is stored.
 * @param fuse_regs Register interface to the fuse controller.  This contains cached information
 * for most fuse slots.
 * @param ccs Driver interface for accessing hardware PCR values.
 * @param root_key The root key for the firmware key manifest.
 * @param secondary_root_key Optional secondary root key for the firmware key manifest.
 *
 * @return 0 if the RoT was initialized successfully or an error code.
 */
int hw_rot_firmware_key_manifest_init (struct hw_rot_firmware_key_manifest *rot,
	const struct fuse_controller_interface *fuses, struct Gfc_regs *fuse_regs,
	const struct ccs_ksu_interface *ccs, const SP_ECDSA_P384_PUBLIC *root_key,
	const SP_ECDSA_P384_PUBLIC *secondary_root_key)
{
	if ((rot == NULL) || (fuses == NULL) || (fuse_regs == NULL) || (ccs == NULL) ||
		(root_key == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	memset (rot, 0, sizeof (struct hw_rot_firmware_key_manifest));

	rot->base.has_root_key = hw_rot_firmware_key_manifest_has_root_key;
	rot->base.get_root_key_hash = hw_rot_firmware_key_manifest_get_root_key_hash;
	rot->base.verify_root_key = hw_rot_firmware_key_manifest_verify_root_key;
	rot->base.has_free_root_key_slots = hw_rot_firmware_key_manifest_has_free_root_key_slots;
	rot->base.update_root_key = hw_rot_firmware_key_manifest_update_root_key;
	rot->base.get_svn = hw_rot_firmware_key_manifest_get_svn;
	rot->base.get_svn_bits = hw_rot_firmware_key_manifest_get_svn_bits;
	rot->base.update_svn = hw_rot_firmware_key_manifest_update_svn;
	rot->base.refresh_svn = hw_rot_firmware_key_manifest_refresh_svn;
	rot->base.has_active_tenancy = hw_rot_firmware_key_manifest_has_active_tenancy;
	rot->base.get_tenancy_counter = hw_rot_firmware_key_manifest_get_tenancy_counter;
	rot->base.get_tenancy_grant_token = hw_rot_firmware_key_manifest_get_tenancy_grant_token;
	rot->base.tenancy_transfer = hw_rot_firmware_key_manifest_tenancy_transfer;

	rot->fuses = fuses;
	rot->fuse_regs = fuse_regs;
	rot->root_key = root_key;
	rot->secondary_key = secondary_root_key;

	return 0;
}

/**
 * Release the resources used by the firmware key manifest RoT.
 *
 * @param rot The RoT instance to release.
 */
void hw_rot_firmware_key_manifest_release (const struct hw_rot_firmware_key_manifest *rot)
{
	UNUSED (rot);
}
