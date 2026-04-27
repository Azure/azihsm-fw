// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_fw_util.h"
#include "key_manifest_hsp_rom.h"
#include "common/buffer_util.h"
#include "common/unused.h"
#include "logging/hsp_logging.h"


/**
 * State values indicating the transfers supported by the manifest.
 */
enum {
	KEY_MANIFEST_HSP_ROM_TRANSFER_NONE = 0,			/**< No transfer is allowed. */
	KEY_MANIFEST_HSP_ROM_TRANSFER_TENANCY_REVOKE,	/**< An active tenancy will be revoked. */
	KEY_MANIFEST_HSP_ROM_TRANSFER_TENANCY_GRANT,	/**< Grant an active tenancy. */
	KEY_MANIFEST_HSP_ROM_TRANSFER_OWNERSHIP,		/**< Transfer ownership to a new root key. */
	KEY_MANIFEST_HSP_ROM_TRANSFER_NONE_TENANCY,		/**< The manifest is authorized for an active tenancy. */
};


/**
 * Verify integrity of the manifest contents through signature and hash validation.
 *
 * @param rom The ROM manifest instance to use for verification.
 * @param hash A hash engine to use for manifest verification.
 * @param signed_data The signed header for the manifest.
 * @param signed_length Length of the signed header.
 * @param signature Signature to verify the signed header.
 * @param key Public key to use for signature verification.
 * @param hashed_data The manifest data to verify.
 * @param hashed_length Length of the manifest data.
 * @param expected The expected digest for the data.
 *
 * @return 0 if the data is valid or an error code.
 */
int key_manifest_hsp_rom_check_manifest_signature (const struct key_manifest_hsp_rom *rom,
	const struct hash_engine *hash, const uint8_t *signed_data, size_t signed_length,
	const SP_ECDSA_P384_SIGNATURE *signature, const SP_ECDSA_P384_PUBLIC *key,
	const uint8_t *hashed_data, size_t hashed_length, const uint8_t *expected)
{
	struct ecc_point_public_key tmp_key;

	hsp_fw_load_public_key (key, &tmp_key);

	return hsp_fw_verify_signed_image (rom->pka, hash, signed_data, signed_length, signature,
		&tmp_key, hashed_data, hashed_length, expected, KEY_MANIFEST_VERIFY_FAILED);
}

/**
 * Check the SVN value of the manifest against the current RoT SVN state.  The manifest SVN is
 * validated against the following conditions:
 *    - If the SVN in the manifest is lower than the current RoT SVN state, the manifest has
 *      been revoked.
 *    - If the SVN is equal to the current RoT SVN state, the manifest is good.
 *    - If the key manifest is an owner manifest whose SVN is the next valid state, the manifest
 *      is good.
 *    - If the key manifest is a tenant manifest whose SVN is the next valid state, the manifest
 *      is not valid.  Key manifest revocation cannot be executed by tenants.
 *    - If the SVN in the manifest is a higher version than the next valid SVN state, the
 *      manifest is not valid.  This prevents a single FW image from consuming all the SVN bits
 *      in one shot.
 *
 * @param rom The ROM manifest to check.
 * @param rot_svn Optional output for the current RoT SVN value.
 *
 * @return 0 if the manifest is valid or an error code.
 */
int key_manifest_hsp_rom_check_manifest_svn (const struct key_manifest_hsp_rom *rom,
	uint64_t *rot_svn)
{
	uint64_t svn;
	int status;

	status = rom->rot->get_svn (rom->rot, &svn);
	if (status != 0) {
		return status;
	}

	if (rot_svn) {
		*rot_svn = svn;
	}

	if (rom->state->manifest.keys.header_signed.svn != svn) {
		if ((rom->state->manifest.keys.header_signed.type ==
				KEY_MANIFEST_HSP_ROM_KEY_MANIFEST_OWNER) &&
				(rom->state->manifest.keys.header_signed.svn == ((svn << 1) | 1))) {
			return 0;
		}

		if (rom->state->manifest.keys.header_signed.svn < svn) {
			return KEY_MANIFEST_REVOKED;
		}
		else {
			return KEY_MANIFEST_ID_TOO_HIGH;
		}
	}

	return 0;
}

int key_manifest_hsp_rom_validate_owner_key_buffer (const struct key_manifest_hsp_rom *manifest,
	const struct hash_engine *hash, uint8_t **key_buffer_out, size_t *key_buffer_out_length)
{
	int status;

	status = key_manifest_hsp_rom_check_manifest_signature (manifest, hash,
		(uint8_t*) &manifest->state->manifest.transfer.owner.header_signed,
		sizeof (manifest->state->manifest.transfer.owner.header_signed),
		&manifest->state->manifest.transfer.owner.signature,
		&manifest->state->manifest.transfer.owner.owner_key,
		(uint8_t*) &manifest->state->manifest.transfer.owner.data,
		sizeof (manifest->state->manifest.transfer.owner.data),
		manifest->state->manifest.transfer.owner.header_signed.digest.AsBytes);
	if (status != 0) {
		return status;
	}

	/* To be a valid ownership transfer, the root key on the key manifest must match the
	 * endorsed key in the ownership transfer manifest. */
	status = buffer_compare (manifest->state->manifest.keys.owner_key.AsBytes,
		manifest->state->manifest.transfer.owner.data.new_owner_key.AsBytes,
		SP_ECDSA_P384_PUBLIC_KEY_SIZE);
	if (status != 0) {
		return status;
	}

	*key_buffer_out = manifest->state->manifest.transfer.owner.owner_key.AsBytes;
	*key_buffer_out_length = SP_ECDSA_P384_PUBLIC_KEY_SIZE;

	return status;
}

/**
 * Check if the current manifest indicates an ownership transfer to a new root key.  If so,
 * authenticate the transfer and confirm the device can support it.
 *
 * @param rom The ROM manifest to check for ownership transfer.
 * @param hash A hash engine to use for manifest verification.
 * @param has_root_key Flag indicating whether the device currently has a root key or not.
 * @param expected_manifest_marker The manifest marker expected for ownership transfer manifests
 *
 * @return 0 if the manifest is valid or an error code.
 */
static int key_manifest_hsp_rom_validate_ownership_transfer (const struct key_manifest_hsp_rom *rom,
	const struct hash_engine *hash, bool has_root_key, uint32_t expected_manifest_marker)
{
	int status = -1;
	uint8_t *key_buffer_for_verification = NULL;
	size_t key_buffer_verification_length = 0;

	if (rom->state->manifest.transfer.marker == expected_manifest_marker) {
		status = rom->authenticate_ownership_transfer_root_key_buffer (rom, hash,
			&key_buffer_for_verification, &key_buffer_verification_length);
		if (status != 0) {
			goto fail;
		}

		if (!has_root_key) {
			/* The RoT SVN will be 0 until a root key is configured.  Ensure this image matches the
			 * RoT SVN or else it will be rejected after configuring the root key. */
			if (rom->state->manifest.keys.header_signed.svn != 0) {
				status = KEY_MANIFEST_ID_TOO_HIGH;
				goto fail;
			}
		}
		else {
			/* Make sure there are root key slots available and the endorsing key matches the
			 * current root key. */
			status = rom->rot->has_free_root_key_slots (rom->rot);
			if (status != 0) {
				goto fail;
			}

			status = rom->rot->verify_root_key (rom->rot, key_buffer_for_verification,
				key_buffer_verification_length, hash);
			if (status != 0) {
				goto fail;
			}
		}

		rom->state->transfer = KEY_MANIFEST_HSP_ROM_TRANSFER_OWNERSHIP;

fail:
		if (status != 0) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_HSP,
				HSP_LOGGING_INVALID_OWNERSHIP_TRANSFER, status, 0);
		}
	}

	/* When there is an error authenticating the ownership transfer, the result depends on whether
	 * there is a trusted root key or not.  When there is, return the error code indicating the root
	 * key in the manifest is not valid (this was the error during top-level verification that
	 * originally caused the ownership transfer check).  When there is not, return success to
	 * indicate a valid manifest without ownership transfer. */
	return ((status != 0) && has_root_key) ? HW_ROT_BAD_ROOT_KEY : 0;
}

/**
 * Determine the secure boot flow that should be used with the manifest and if there are any
 * tenancy transfers that need to be executed.
 *
 * @param rom The ROM manifest to inspect for secure boot information.
 * @param hash A hash engine to use for manifest verification.
 * @param has_root_key Flag indicating whether the device currently has a root key or not.
 *
 * @return 0 if the manifest is valid or an error code.
 */
static int key_manifest_hsp_rom_validate_tenancy_transfer (const struct key_manifest_hsp_rom *rom,
	const struct hash_engine *hash, bool has_root_key)
{
	int status;

	if (rom->state->manifest.keys.header_signed.type == KEY_MANIFEST_HSP_ROM_KEY_MANIFEST_TENANT) {
		status = key_manifest_hsp_rom_check_manifest_signature (rom, hash,
			(uint8_t*) &rom->state->manifest.transfer.grant.header_signed,
			sizeof (rom->state->manifest.transfer.grant.header_signed),
			&rom->state->manifest.transfer.grant.signature,
			&rom->state->manifest.keys.data.signing_key,
			(uint8_t*) &rom->state->manifest.transfer.grant.data,
			sizeof (rom->state->manifest.transfer.grant.data),
			rom->state->manifest.transfer.grant.header_signed.digest.AsBytes);
		if (status != 0) {
			return status;
		}

		if (has_root_key) {
			uint8_t token[SHA384_HASH_LENGTH];

			status = rom->rot->get_tenancy_grant_token (rom->rot,
				rom->state->manifest.transfer.grant.data.tenant_key.AsBytes,
				SP_ECDSA_P384_PUBLIC_KEY_SIZE, token, SHA384_HASH_LENGTH);
			if (ROT_IS_ERROR (status)) {
				return status;
			}

			status = buffer_compare (token,
				rom->state->manifest.transfer.grant.data.grant_token.AsBytes, SHA384_HASH_LENGTH);
			if (status != 0) {
				return KEY_MANIFEST_VERIFY_FAILED;
			}

			/* The grant manifest is valid.  If there is no active tenancy, indicate that this
			 * manifest triggers an tenancy grant transfer. */
			status = rom->rot->has_active_tenancy (rom->rot);
			if (status == 0) {
				rom->state->transfer = KEY_MANIFEST_HSP_ROM_TRANSFER_TENANCY_GRANT;
			}
			else if (status == 1) {
				rom->state->transfer = KEY_MANIFEST_HSP_ROM_TRANSFER_NONE_TENANCY;
			}
			else {
				return status;
			}
		}
		else {
			/* The grant manifest is valid, but there is no root key, making tenancy transfers not
			 * possible. */
			rom->state->transfer = KEY_MANIFEST_HSP_ROM_TRANSFER_NONE_TENANCY;
		}
	}
	else {
		/* This is a valid owner manifest which triggers a tenancy revocation transfer for any
		 * active tenancy. */
		status = rom->rot->has_active_tenancy (rom->rot);
		if (status == 1) {
			rom->state->transfer = KEY_MANIFEST_HSP_ROM_TRANSFER_TENANCY_REVOKE;
		}
		else if (status != 0) {
			return status;
		}
	}

	return 0;
}

/**
 * Validate transfer manifests without a root key already stored in memory.
 *
 * @param rom The ROM manifest without root key or with unsupported key.
 * @param hash A hash engine to use for manifest verification.
 * @param status HW_ROT status indicating the state of the root key.
 * @param expected_owership_transfer_marker Expected marker for the ownership transfer manifest
 *
 * @return 0 if the manifest is valid or an error code.
 */
int key_manifest_hsp_rom_verify_transfer_manifest_no_root_key (
	const struct key_manifest_hsp_rom *rom, const struct hash_engine *hash, int status,
	uint32_t expected_owership_transfer_marker)
{
	if (status == HW_ROT_UNSUPPORTED) {
		if (rom->state->manifest.keys.header_signed.type ==
			KEY_MANIFEST_HSP_ROM_KEY_MANIFEST_OWNER) {
			/* HW-backed security is not enabled yet, so skip all additional checks. */
			return 0;
		}
		else {
			/* For tenant manifests, we need to validate the grant manifest to get the firmware
			 * signing key. */
			return key_manifest_hsp_rom_validate_tenancy_transfer (rom, hash, false);
		}
	}
	else if (status == HW_ROT_NO_ROOT_KEY) {
		/* There is no root key enabled.  Both owner and tenant manifests are allowed in this
		 * case, but the workflow is different for each.  Owner manifests provide the
		 * possibility to configure the root key, while tenant manifests do not. */
		if (rom->state->manifest.keys.header_signed.type ==
			KEY_MANIFEST_HSP_ROM_KEY_MANIFEST_OWNER) {
			return key_manifest_hsp_rom_validate_ownership_transfer (rom, hash, false,
				expected_owership_transfer_marker);
		}
		else {
			return key_manifest_hsp_rom_validate_tenancy_transfer (rom, hash, false);
		}
	}
	else {
		return status;
	}
}

/**
 * Validate transfer manifests with a root key already stored in memory.
 *
 * @param rom The ROM manifest with root key present.
 * @param hash A hash engine to use for manifest verification.
 * @param key_buffer Root key buffer to verify
 * @param key_buffer_length Length of root key buffer
 * @param expected_owership_transfer_marker Expected marker for the ownership transfer manifest
 *
 * @return 0 if the manifest is valid or an error code.
 */
int key_manifest_hsp_rom_verify_transfer_manifest_with_root_key (
	const struct key_manifest_hsp_rom *rom, const struct hash_engine *hash,
	const uint8_t *key_buffer, size_t key_buffer_length, uint32_t expected_owership_transfer_marker)
{
	int status;

	status = key_manifest_hsp_rom_check_manifest_svn (rom, NULL);
	if (status != 0) {
		return status;
	}

	status = rom->rot->verify_root_key (rom->rot, key_buffer, key_buffer_length, hash);
	if (status == 0) {
		/* The manifest is signed by the trusted root key.  Check for any tenancy transfers. */
		status = key_manifest_hsp_rom_validate_tenancy_transfer (rom, hash, true);
	}
	else if ((status == HW_ROT_BAD_ROOT_KEY) &&
		(rom->state->manifest.keys.header_signed.type == KEY_MANIFEST_HSP_ROM_KEY_MANIFEST_OWNER)) {
		/* The manifest is not signed by the trusted root key.  This could either be that the
		 * manifest is simply not trusted or because it is initiating an ownership transfer.  We
		 * need to validate the transfer manifest to see if an ownership transfer needs to take
		 * place. */
		status = rom->rot->has_active_tenancy (rom->rot);
		if (status == 0) {
			status = key_manifest_hsp_rom_validate_ownership_transfer (rom, hash, true,
				expected_owership_transfer_marker);
		}
		else if (status == 1) {
			/* There is an active tenancy, so we cannot execute an ownership transfer. */
			status = HW_ROT_BAD_ROOT_KEY;
		}
	}

	return status;
}

int key_manifest_hsp_rom_verify (const struct key_manifest *manifest,
	const struct hash_engine *hash)
{
	const struct key_manifest_hsp_rom *rom = (const struct key_manifest_hsp_rom*) manifest;
	int status;

	if ((rom == NULL) || (hash == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	status = key_manifest_hsp_rom_check_manifest_signature (rom, hash,
		(uint8_t*) &rom->state->manifest.keys.header_signed,
		sizeof (rom->state->manifest.keys.header_signed), &rom->state->manifest.keys.signature,
		&rom->state->manifest.keys.owner_key, (uint8_t*) &rom->state->manifest.keys.data,
		sizeof (rom->state->manifest.keys.data),
		rom->state->manifest.keys.header_signed.digest.AsBytes);
	if (status != 0) {
		return status;
	}

	status = rom->rot->has_root_key (rom->rot);
	if (status != 0) {
		return key_manifest_hsp_rom_verify_transfer_manifest_no_root_key (rom, hash, status,
			KEY_MANIFEST_HSP_ROM_OWNERSHIP_MANIFEST_MARKER);
	}

	status = key_manifest_hsp_rom_verify_transfer_manifest_with_root_key (rom, hash,
		rom->state->manifest.keys.owner_key.AsBytes, SP_ECDSA_P384_PUBLIC_KEY_SIZE,
		KEY_MANIFEST_HSP_ROM_OWNERSHIP_MANIFEST_MARKER);

	return (status == HW_ROT_BAD_ROOT_KEY) ? KEY_MANIFEST_BAD_ROOT_KEY : status;
}

int key_manifest_hsp_rom_is_allowed (const struct key_manifest *manifest)
{
	const struct key_manifest_hsp_rom *rom = (const struct key_manifest_hsp_rom*) manifest;
	int status;

	if (rom == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	status = rom->rot->has_root_key (rom->rot);
	if (status != 0) {
		if ((status == HW_ROT_UNSUPPORTED) || (status == HW_ROT_NO_ROOT_KEY)) {
			/* There is no root key, so the SVN is ignored.  This means all manifests are allowed. */
			return 1;
		}
		else {
			return status;
		}
	}

	status = key_manifest_hsp_rom_check_manifest_svn (rom, NULL);
	if (status == 0) {
		return 1;
	}
	else if ((status == KEY_MANIFEST_REVOKED) || (status == KEY_MANIFEST_ID_TOO_HIGH)) {
		return 0;
	}
	else {
		return status;
	}
}

int key_manifest_hsp_rom_revokes_old_manifest (const struct key_manifest *manifest)
{
	const struct key_manifest_hsp_rom *rom = (const struct key_manifest_hsp_rom*) manifest;
	uint64_t svn;
	int status;

	if (rom == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	status = rom->rot->has_root_key (rom->rot);
	if (status != 0) {
		if ((status == HW_ROT_UNSUPPORTED) || (status == HW_ROT_NO_ROOT_KEY)) {
			/* There is no root key, so no revocation is possible. */
			return 0;
		}
		else {
			return status;
		}
	}

	status = rom->rot->get_svn (rom->rot, &svn);
	if (status != 0) {
		return status;
	}

	return (rom->state->manifest.keys.header_signed.svn > svn);
}

int key_manifest_hsp_rom_update_revocation (const struct key_manifest *manifest)
{
	const struct key_manifest_hsp_rom *rom = (const struct key_manifest_hsp_rom*) manifest;
	uint64_t svn;
	int status;

	if (rom == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	status = rom->rot->has_root_key (rom->rot);
	if (status != 0) {
		if ((status == HW_ROT_UNSUPPORTED) || (status == HW_ROT_NO_ROOT_KEY)) {
			/* There is no root key, so no update is necessary. */
			return 0;
		}
		else {
			return status;
		}
	}

	/* Check that the new SVN is valid */
	status = key_manifest_hsp_rom_check_manifest_svn (rom, &svn);
	if (status != 0) {
		return status;
	}

	if (svn != rom->state->manifest.keys.header_signed.svn) {
		/* Only update when the SVN in the manifest is different. */
		status = rom->rot->update_svn (rom->rot, rom->state->manifest.keys.header_signed.svn);
	}
	else {
		/* If the SVN is the same, give the RoT an opportunity to correct any storage errors. */
		status = rom->rot->refresh_svn (rom->rot, rom->state->manifest.keys.header_signed.svn);
		if (status != 0) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_HSP,
				HSP_LOGGING_SVN_REDUNDANCY_DEGRADED, status, 0);

			status = 0;
		}
	}

	return status;
}

const struct key_manifest_public_key* key_manifest_hsp_rom_get_root_key (
	const struct key_manifest *manifest)
{
	const struct key_manifest_hsp_rom *rom = (const struct key_manifest_hsp_rom*) manifest;

	if (rom) {
		return &rom->root_key;
	}
	else {
		return NULL;
	}
}

const struct key_manifest_public_key* key_manifest_hsp_rom_get_app_key (
	const struct key_manifest *manifest)
{
	const struct key_manifest_hsp_rom *rom = (const struct key_manifest_hsp_rom*) manifest;

	if (rom) {
		return &rom->fw_key;
	}
	else {
		return NULL;
	}
}

const struct key_manifest_public_key* key_manifest_hsp_rom_get_manifest_key (
	const struct key_manifest *manifest)
{
	return key_manifest_hsp_rom_get_app_key (manifest);
}

/**
 * Determine the total key manifest size based on the type of transfer manifest is present.
 *
 * @param transfer_marker The marker value for the transfer manifest.
 *
 * @return Total size of the key manifest or an KEY_MANIFEST_INVALID_FORMAT if the marker is
 * unknown.
 */
static int key_manifest_hsp_rom_manifest_size (uint32_t transfer_marker)
{
	int length = sizeof (transfer_marker) + sizeof (struct key_manifest_hsp_rom_keys);

	switch (transfer_marker) {
		case KEY_MANIFEST_HSP_ROM_TENANCY_MANIFEST_MARKER:
			length += sizeof (struct key_manifest_hsp_rom_grant);
			break;

		case KEY_MANIFEST_HSP_ROM_OWNERSHIP_MANIFEST_MARKER:
			length += sizeof (struct key_manifest_hsp_rom_ownership);
			break;

		case KEY_MANIFEST_HSP_ROM_NULL_MANIFEST_MARKER:
			/* A null manifest has no additional length. */
			break;

		default:
			return KEY_MANIFEST_INVALID_FORMAT;
	}

	return length;
}

size_t key_manifest_hsp_rom_get_total_size (const struct key_manifest_hsp_rom *rom)
{
	if (rom) {
		return key_manifest_hsp_rom_manifest_size (rom->state->manifest.transfer.marker);
	}
	else {
		return 0;
	}
}

uint32_t key_manifest_hsp_rom_get_svn (const struct key_manifest_hsp_rom *rom)
{
	if (rom) {
		return rom->state->manifest.keys.header_signed.svn;
	}
	else {
		return 0;
	}
}

const struct key_manifest_public_key* key_manifest_hsp_rom_get_secondary_key (
	const struct key_manifest_hsp_rom *rom)
{
	if (rom) {
		if (rom->state->manifest.keys.header_signed.valid_keys == 2) {
			return &rom->secondary_key;
		}
		else {
			return NULL;
		}
	}
	else {
		return NULL;
	}
}

const struct key_manifest_public_key* key_manifest_hsp_rom_get_tenancy_grant_key (
	const struct key_manifest_hsp_rom *rom)
{
	if (rom) {
		if (rom->state->manifest.keys.header_signed.type ==
			KEY_MANIFEST_HSP_ROM_KEY_MANIFEST_TENANT) {
			return &rom->tenancy_grant_key;
		}
		else {
			return NULL;
		}
	}
	else {
		return NULL;
	}
}

bool key_manifest_hsp_rom_is_ownership_transfer (const struct key_manifest_hsp_rom *rom)
{
	if (rom) {
		return (rom->state->transfer == KEY_MANIFEST_HSP_ROM_TRANSFER_OWNERSHIP);
	}
	else {
		return false;
	}
}

/**
 * Execute an onwership transfer to a new root key.
 *
 * @param rom The ownership transfer manifest with root key to update.
 * @param hash A hash engine to use for manifest verification.
 * @param key_buffer Buffer with the root key to store.
 * @param key_buffer_length Length of root key buffer.
 *
 * @return 0 if the ownership transfer executed successfully. Error code otherwise.
 */
int hsp_key_manifest_hsp_rom_execute_ownership_transfer (const struct key_manifest_hsp_rom *rom,
	const struct hash_engine *hash, const uint8_t *key_buffer, size_t key_buffer_length)
{
	int status;

	status = rom->rot->has_root_key (rom->rot);
	if ((status != 0) && (status != HW_ROT_NO_ROOT_KEY)) {
		return status;
	}

	if (rom->is_ownership_transfer (rom)) {
		status = rom->rot->update_root_key (rom->rot, key_buffer, key_buffer_length, hash);
	}
	else {
		status = 0;
	}

	return status;
}

int key_manifest_hsp_rom_update_root_key (const struct key_manifest_hsp_rom *rom,
	const struct hash_engine *hash)
{
	if ((rom == NULL) || (hash == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	return hsp_key_manifest_hsp_rom_execute_ownership_transfer (rom, hash,
		rom->state->manifest.keys.owner_key.AsBytes, SP_ECDSA_P384_PUBLIC_KEY_SIZE);
}

bool key_manifest_hsp_rom_is_tenancy_transfer (const struct key_manifest_hsp_rom *rom)
{
	if (rom) {
		return ((rom->state->transfer == KEY_MANIFEST_HSP_ROM_TRANSFER_TENANCY_GRANT) ||
			(rom->state->transfer == KEY_MANIFEST_HSP_ROM_TRANSFER_TENANCY_REVOKE));
	}
	else {
		return false;
	}
}

bool key_manifest_hsp_rom_is_tenancy_grant (const struct key_manifest_hsp_rom *rom)
{
	if (rom) {
		return ((rom->state->transfer == KEY_MANIFEST_HSP_ROM_TRANSFER_TENANCY_GRANT) ||
			(rom->state->transfer == KEY_MANIFEST_HSP_ROM_TRANSFER_NONE_TENANCY));
	}
	else {
		return false;
	}
}

int key_manifest_hsp_rom_update_tenancy_counter (const struct key_manifest_hsp_rom *rom)
{
	int status;

	if (rom == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	status = rom->rot->has_root_key (rom->rot);
	if (status != 0) {
		return status;
	}

	if (rom->is_tenancy_transfer (rom)) {
		status = rom->rot->tenancy_transfer (rom->rot,
			(rom->state->transfer == KEY_MANIFEST_HSP_ROM_TRANSFER_TENANCY_GRANT));
	}

	return status;
}

/**
 * Verify the basic format and construction of manifest data loaded into memory.
 *
 * @param manifest The manifest instance to check.
 *
 * @return 0 if the manifest is constructed correctly or an error code.
 */
static int key_manifest_hsp_rom_check_manifest_format (const struct key_manifest_hsp_rom *manifest)
{
	if (manifest->state->manifest.keys.marker != KEY_MANIFEST_HSP_ROM_KEY_MANIFEST_MARKER) {
		return KEY_MANIFEST_INVALID_FORMAT;
	}

	if (manifest->state->manifest.keys.header_signed.length !=
		sizeof (manifest->state->manifest.keys.data)) {
		return KEY_MANIFEST_INVALID_FORMAT;
	}

	/* A tenancy grant manifest can only be paired with a tenant key manifest.  Null and ownership
	 * transfer manifests can only be paired with an owner key manifest.  Any other combination or
	 * type indicator for the key manifest is invalid. */
	if (manifest->state->manifest.transfer.marker == KEY_MANIFEST_HSP_ROM_TENANCY_MANIFEST_MARKER) {
		/* Only accept tenancy grant manifests. */
		if (manifest->state->manifest.transfer.grant.header_signed.type != 0) {
			return KEY_MANIFEST_INVALID_FORMAT;
		}

		if (manifest->state->manifest.transfer.grant.header_signed.length !=
			sizeof (manifest->state->manifest.transfer.grant.data)) {
			return KEY_MANIFEST_INVALID_FORMAT;
		}

		if (manifest->state->manifest.keys.header_signed.type !=
			KEY_MANIFEST_HSP_ROM_KEY_MANIFEST_TENANT) {
			return KEY_MANIFEST_INVALID_FORMAT;
		}

		if (manifest->state->manifest.keys.header_signed.valid_keys != 1) {
			return KEY_MANIFEST_INVALID_FORMAT;
		}

		/* Secondary Key left null for tenancy grant manifests. */
		hsp_fw_load_public_key (&manifest->state->manifest.transfer.grant.data.tenant_key,
			&manifest->state->fw_key);
		hsp_fw_load_public_key (&manifest->state->manifest.keys.data.signing_key,
			&manifest->state->tenancy_grant_key);
	}
	else {
		if (manifest->state->manifest.transfer.marker ==
			KEY_MANIFEST_HSP_ROM_OWNERSHIP_MANIFEST_MARKER) {
			if (manifest->state->manifest.transfer.owner.header_signed.length !=
				sizeof (manifest->state->manifest.transfer.owner.data)) {
				return KEY_MANIFEST_INVALID_FORMAT;
			}
		}

		if (manifest->state->manifest.keys.header_signed.type !=
			KEY_MANIFEST_HSP_ROM_KEY_MANIFEST_OWNER) {
			return KEY_MANIFEST_INVALID_FORMAT;
		}

		if ((manifest->state->manifest.keys.header_signed.valid_keys != 1) &&
			(manifest->state->manifest.keys.header_signed.valid_keys != 2)) {
			return KEY_MANIFEST_INVALID_FORMAT;
		}

		/* Tenancy Grant Key left null for owner manifests. */
		hsp_fw_load_public_key (&manifest->state->manifest.keys.data.signing_key,
			&manifest->state->fw_key);
		hsp_fw_load_public_key (&manifest->state->manifest.keys.data.secondary_key,
			&manifest->state->secondary_key);
	}

	hsp_fw_load_public_key (&manifest->state->manifest.keys.owner_key, &manifest->state->root_key);

	return 0;
}

int key_manifest_hsp_rom_init_state (const struct key_manifest_hsp_rom *rom,
	const struct flash *flash, uint32_t base_addr)
{
	size_t offset;
	int status;

	if ((rom == NULL) || (flash == NULL) || (rom->state == NULL) ||
		(rom->rot == NULL) || (rom->pka == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	memset (rom->state, 0, sizeof (struct key_manifest_hsp_rom_state));

	status = flash->read (flash, base_addr, (uint8_t*) &rom->state->manifest.transfer.marker,
		sizeof (rom->state->manifest.transfer.marker));
	if (status != 0) {
		return status;
	}

	offset = sizeof (rom->state->manifest.transfer.marker);

	switch (rom->state->manifest.transfer.marker) {
		case KEY_MANIFEST_HSP_ROM_NULL_MANIFEST_MARKER:
			/* No additional manifest data. */
			break;

		case KEY_MANIFEST_HSP_ROM_TENANCY_MANIFEST_MARKER:
			status = flash->read (flash, base_addr + offset,
				(uint8_t*) &rom->state->manifest.transfer.grant,
				sizeof (rom->state->manifest.transfer.grant));
			offset += sizeof (rom->state->manifest.transfer.grant);
			break;

		case KEY_MANIFEST_HSP_ROM_OWNERSHIP_MANIFEST_MARKER:
			status = flash->read (flash, base_addr + offset,
				(uint8_t*) &rom->state->manifest.transfer.owner,
				sizeof (rom->state->manifest.transfer.owner));
			offset += sizeof (rom->state->manifest.transfer.owner);
			break;

		default:
			status = KEY_MANIFEST_INVALID_FORMAT;
			break;
	}
	if (status != 0) {
		return status;
	}

	status = flash->read (flash, base_addr + offset, (uint8_t*) &rom->state->manifest.keys,
		sizeof (rom->state->manifest.keys));
	if (status != 0) {
		return status;
	}

	return key_manifest_hsp_rom_check_manifest_format (rom);
}

int key_manifest_hsp_rom_init_state_from_memory (const struct key_manifest_hsp_rom *rom,
	const uint8_t *base_addr)
{
	uint32_t offset;

	if ((rom == NULL) || (base_addr == NULL) || (rom->state == NULL) ||
		(rom->rot == NULL) || (rom->pka == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	memset (rom->state, 0, sizeof (struct key_manifest_hsp_rom_state));

	memcpy (&rom->state->manifest.transfer.marker, base_addr,
		sizeof (rom->state->manifest.transfer.marker));
	offset = sizeof (rom->state->manifest.transfer.marker);

	switch (rom->state->manifest.transfer.marker) {
		case KEY_MANIFEST_HSP_ROM_NULL_MANIFEST_MARKER:
			/* No additional manifest data. */
			break;

		case KEY_MANIFEST_HSP_ROM_TENANCY_MANIFEST_MARKER:
			memcpy (&rom->state->manifest.transfer.grant, &base_addr[offset],
				sizeof (rom->state->manifest.transfer.grant));
			offset += sizeof (rom->state->manifest.transfer.grant);
			break;

		case KEY_MANIFEST_HSP_ROM_OWNERSHIP_MANIFEST_MARKER:
			memcpy (&rom->state->manifest.transfer.owner, &base_addr[offset],
				sizeof (rom->state->manifest.transfer.owner));
			offset += sizeof (rom->state->manifest.transfer.owner);
			break;

		default:
			return KEY_MANIFEST_INVALID_FORMAT;
			break;
	}

	memcpy (&rom->state->manifest.keys, &base_addr[offset], sizeof (rom->state->manifest.keys));

	return key_manifest_hsp_rom_check_manifest_format (rom);
}

/**
 * Initialize an HSP ROM key manifest instance from data stored on flash.
 *
 * @param manifest The manifest instance to initialize.
 * @param state Variable context for the manifest.  This must not already be initialized.
 * @param rot Interface to the HW RoT state.
 * @param flash Flash device where the key manifest is stored.
 * @param base_addr Starting address for ROM manifests on flash.
 * @param pka Interface to the PKA engine for signature verification.
 *
 * @return 0 if the manifest was initialized successfully or an error code.
 */
int key_manifest_hsp_rom_init (struct key_manifest_hsp_rom *manifest,
	struct key_manifest_hsp_rom_state *state, const struct hw_rot *rot, const struct flash *flash,
	uint32_t base_addr, const struct ecc_hw *pka)
{
	int status;

	status = key_manifest_hsp_rom_init_api (manifest, state, rot, pka);
	if (status != 0) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	return manifest->init_state (manifest, flash, base_addr);
}

/**
 * Initialize an HSP ROM key manifest instance from data stored in memory.
 *
 * @param manifest The manifest instance to initialize.
 * @param state Variable context for the manifest.  This must not already be initialized.
 * @param rot Interface to the HW RoT state.
 * @param base_addr Starting address for ROM manifests in memory.
 * @param pka Interface to the PKA engine for signature verification.
 *
 * @return 0 if the manifest was initialized successfully or an error code.
 */
int key_manifest_hsp_rom_init_from_memory (struct key_manifest_hsp_rom *manifest,
	struct key_manifest_hsp_rom_state *state, const struct hw_rot *rot, const uint8_t *base_addr,
	const struct ecc_hw *pka)
{
	int status;

	status = key_manifest_hsp_rom_init_api (manifest, state, rot, pka);
	if (status != 0) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	return manifest->init_state_from_memory (manifest, base_addr);
}

/**
 * Initialize the API and static contents of the key manifest structure.  No manifest information
 * will be loaded and the variable context for the manifest will remain uninitialized.  The result
 * of the call is the same as static initialization, except parameter validation is performed.
 *
 * A manifest instance that has only had the API initialized does not need to be released.
 *
 * @param manifest The manifest instance to initialize.
 * @param state Variable context for the manifest.
 * @param rot Interface to the HW RoT state.
 * @param pka Interface to the PKA engine for signature verification.
 *
 * @return 0 if the manifest API was successfully initialized or KEY_MANIFEST_INVALID_ARGUMENT if
 * there are null parameters.
 */
int key_manifest_hsp_rom_init_api (struct key_manifest_hsp_rom *manifest,
	struct key_manifest_hsp_rom_state *state, const struct hw_rot *rot, const struct ecc_hw *pka)
{
	if ((manifest == NULL) || (state == NULL) || (rot == NULL) || (pka == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	memset (manifest, 0, sizeof (struct key_manifest_hsp_rom));

	manifest->base.verify = key_manifest_hsp_rom_verify;
	manifest->base.is_allowed = key_manifest_hsp_rom_is_allowed;
	manifest->base.revokes_old_manifest = key_manifest_hsp_rom_revokes_old_manifest;
	manifest->base.update_revocation = key_manifest_hsp_rom_update_revocation;
	manifest->base.get_root_key = key_manifest_hsp_rom_get_root_key;
	manifest->base.get_app_key = key_manifest_hsp_rom_get_app_key;
	manifest->base.get_manifest_key = key_manifest_hsp_rom_get_manifest_key;

	manifest->init_state = key_manifest_hsp_rom_init_state;
	manifest->init_state_from_memory = key_manifest_hsp_rom_init_state_from_memory;
	manifest->get_total_size = key_manifest_hsp_rom_get_total_size;
	manifest->get_svn = key_manifest_hsp_rom_get_svn;
	manifest->get_secondary_key = key_manifest_hsp_rom_get_secondary_key;
	manifest->get_tenancy_grant_key = key_manifest_hsp_rom_get_tenancy_grant_key;
	manifest->is_ownership_transfer = key_manifest_hsp_rom_is_ownership_transfer;
	manifest->update_root_key = key_manifest_hsp_rom_update_root_key;
	manifest->is_tenancy_transfer = key_manifest_hsp_rom_is_tenancy_transfer;
	manifest->is_tenancy_grant = key_manifest_hsp_rom_is_tenancy_grant;
	manifest->update_tenancy_counter = key_manifest_hsp_rom_update_tenancy_counter;
	manifest->authenticate_ownership_transfer_root_key_buffer =
		key_manifest_hsp_rom_validate_owner_key_buffer;

	manifest->state = state;
	manifest->rot = rot;
	manifest->pka = pka;

	manifest->root_key.type = KEY_MANIFEST_ECC_KEY;
	manifest->root_key.key.ecc = &state->root_key;
	manifest->fw_key.type = KEY_MANIFEST_ECC_KEY;
	manifest->fw_key.key.ecc = &state->fw_key;
	manifest->secondary_key.type = KEY_MANIFEST_ECC_KEY;
	manifest->secondary_key.key.ecc = &state->secondary_key;
	manifest->tenancy_grant_key.type = KEY_MANIFEST_ECC_KEY;
	manifest->tenancy_grant_key.key.ecc = &state->tenancy_grant_key;

	return 0;
}

/**
 * Release the resources used by an HSP ROM key manifest.
 *
 * @param manifest The manifest instance to release.
 */
void key_manifest_hsp_rom_release (const struct key_manifest_hsp_rom *manifest)
{
	UNUSED (manifest);
}

/**
 * Determine the length of an HSP ROM key manifest on flash.  This is determined without needing to
 * initialize a manifest instance, so no data will be permanently loaded into memory.  It will also
 * not be validated in any way.
 *
 * @param flash The flash device that contains the key manifest.
 * @param base_addr Address of the key manifest data.
 *
 * @return The total length of the key manifest or an error code.  Use ROT_IS_ERROR to check the
 * return value.
 */
int key_manifest_hsp_rom_get_size_on_flash (const struct flash *flash, uint32_t base_addr)
{
	uint32_t marker;
	int status;

	if (flash == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	status = flash->read (flash, base_addr, (uint8_t*) &marker, sizeof (marker));
	if (status != 0) {
		return status;
	}

	return key_manifest_hsp_rom_manifest_size (marker);
}
