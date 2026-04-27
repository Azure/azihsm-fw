// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_fw_util.h"
#include "key_manifest_hsp_rom_dual_root.h"
#include "common/buffer_util.h"
#include "common/unused.h"
#include "logging/hsp_logging.h"


/**
 * Populate the base key_manifest_hsp_rom instance using the incoming
 * key_manifest_hsp_rom_dual_root_state instance. This is necessary because the layout in memory
 * of the single root instance does not match the memory of the dual root instance, so relevant
 * fields must be copied individually.
 *
 * @param state The dual root manifest instance, which also includes the single root instance.
 */
static void key_manifest_hsp_rom_dual_root_populate_base (
	struct key_manifest_hsp_rom_dual_root_state *state)
{
	/* Copy relevant transfer dual root fields into base instance. */
	state->base.manifest.transfer.marker = state->manifest.transfer.marker;

	if (state->manifest.transfer.marker == KEY_MANIFEST_HSP_ROM_TENANCY_MANIFEST_MARKER) {
		/* Grant manifests match identically in both single and dual root instances,
		 * so a direct copy works here. */
		memcpy (&state->base.manifest.transfer.grant, &state->manifest.transfer.grant,
			sizeof (state->base.manifest.transfer.grant));
	}
	else {
		/* Dual root ownership transfer manifests have an additional key and signature compared
		 * to the single root instance. Therefore, a single memcpy will not suffice. Manually copy
		 * the dual root fields into their single root fields in the base instance. */
		memcpy (&state->base.manifest.transfer.owner.owner_key,
			&state->manifest.transfer.owner.authenticity_key,
			sizeof (state->base.manifest.transfer.owner.owner_key));
		memcpy (&state->base.manifest.transfer.owner.signature,
			&state->manifest.transfer.owner.authenticity_signature,
			sizeof (state->base.manifest.transfer.owner.signature));
		memcpy (&state->base.manifest.transfer.owner.header_signed,
			&state->manifest.transfer.owner.header_signed,
			sizeof (state->base.manifest.transfer.owner.header_signed));
		memcpy (&state->base.manifest.transfer.owner.data.new_owner_key,
			&state->manifest.transfer.owner.data.new_authenticity_key,
			sizeof (state->base.manifest.transfer.owner.data.new_owner_key));
	}

	/* Like transfer manifests, dual root ownership manifests have an additional key and signature
	 * compared to the single root instance. Therefore, a single memcpy will not suffice. Manually
	 * copy the dual root fields into their single root fields in the base instance. */
	state->base.manifest.keys.marker = state->manifest.keys.marker;
	memcpy (&state->base.manifest.keys.owner_key, &state->manifest.keys.authenticity_key,
		sizeof (state->base.manifest.keys.owner_key));
	memcpy (&state->base.manifest.keys.signature, &state->manifest.keys.authenticity_signature,
		sizeof (state->base.manifest.keys.signature));
	memcpy (&state->base.manifest.keys.header_signed, &state->manifest.keys.header_signed,
		sizeof (state->base.manifest.keys.header_signed));
	memcpy (&state->base.manifest.keys.data.signing_key,
		&state->manifest.keys.data.image_authenticity_key,
		sizeof (state->base.manifest.keys.data.signing_key));
	memcpy (&state->base.manifest.keys.data.secondary_key,
		&state->manifest.keys.data.image_authority_key,
		sizeof (state->base.manifest.keys.data.secondary_key));
}

int key_manifest_hsp_rom_dual_root_validate_owner_key_buffer (
	const struct key_manifest_hsp_rom *manifest, const struct hash_engine *hash,
	uint8_t **key_buffer_out, size_t *key_buffer_out_length)
{
	int status;
	const struct key_manifest_hsp_rom_dual_root *rom =
		(const struct key_manifest_hsp_rom_dual_root*) manifest;
	uint8_t unset_key[SP_ECDSA_P384_PUBLIC_KEY_SIZE] = {0};
	bool previous_authority_key_populated = false;

	if (buffer_compare (rom->state->manifest.transfer.owner.authority_key.AsBytes, unset_key,
		SP_ECDSA_P384_PUBLIC_KEY_SIZE) != 0) {
		previous_authority_key_populated = true;
	}

	/* Validate the owner transfer manifest with authenticity key/signature. */
	status = key_manifest_hsp_rom_check_manifest_signature (&rom->base, hash,
		(uint8_t*) &rom->state->manifest.transfer.owner.header_signed,
		sizeof (rom->state->manifest.transfer.owner.header_signed),
		&rom->state->manifest.transfer.owner.authenticity_signature,
		&rom->state->manifest.transfer.owner.authenticity_key,
		(uint8_t*) &rom->state->manifest.transfer.owner.data,
		sizeof (rom->state->manifest.transfer.owner.data),
		rom->state->manifest.transfer.owner.header_signed.digest.AsBytes);
	if (status != 0) {
		return status;
	}

	/* Validate the owner transfer manifest with the authority key/signature if they are populated. */
	if (previous_authority_key_populated) {
		status = key_manifest_hsp_rom_check_manifest_signature (&rom->base, hash,
			(uint8_t*) &rom->state->manifest.transfer.owner.header_signed,
			sizeof (rom->state->manifest.transfer.owner.header_signed),
			&rom->state->manifest.transfer.owner.authority_signature,
			&rom->state->manifest.transfer.owner.authority_key,
			(uint8_t*) &rom->state->manifest.transfer.owner.data,
			sizeof (rom->state->manifest.transfer.owner.data),
			rom->state->manifest.transfer.owner.header_signed.digest.AsBytes);
		if (status != 0) {
			return status;
		}
	}

	/* To be a valid ownership transfer, the authenticity key on the key manifest must
	 * match the new authenticity key in the ownership transfer manifest. Same goes for
	 * the authority keys. */
	status = buffer_compare (rom->state->manifest.keys.authenticity_key.AsBytes,
		rom->state->manifest.transfer.owner.data.new_authenticity_key.AsBytes,
		(2 * SP_ECDSA_P384_PUBLIC_KEY_SIZE));
	if (status != 0) {
		return status;
	}

	*key_buffer_out = rom->state->manifest.transfer.owner.authenticity_key.AsBytes;
	*key_buffer_out_length = (2 * SP_ECDSA_P384_PUBLIC_KEY_SIZE);

	return status;
}

int key_manifest_hsp_rom_dual_root_verify (const struct key_manifest *manifest,
	const struct hash_engine *hash)
{
	const struct key_manifest_hsp_rom_dual_root *rom =
		(const struct key_manifest_hsp_rom_dual_root*) manifest;
	int status;

	if ((rom == NULL) || (hash == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	/* Validate the manifest using the authenticity key/signature. */
	status = key_manifest_hsp_rom_check_manifest_signature (&rom->base, hash,
		(uint8_t*) &rom->state->manifest.keys.header_signed,
		sizeof (rom->state->manifest.keys.header_signed),
		&rom->state->manifest.keys.authenticity_signature,
		&rom->state->manifest.keys.authenticity_key, (uint8_t*) &rom->state->manifest.keys.data,
		sizeof (rom->state->manifest.keys.data),
		rom->state->manifest.keys.header_signed.digest.AsBytes);
	if (status != 0) {
		return status;
	}

	/* Validate the manifest using the authority key/signature if they are populated. */
	if (rom->get_root_authority_key (rom) != NULL) {
		status = key_manifest_hsp_rom_check_manifest_signature (&rom->base, hash,
			(uint8_t*) &rom->state->manifest.keys.header_signed,
			sizeof (rom->state->manifest.keys.header_signed),
			&rom->state->manifest.keys.authority_signature,
			&rom->state->manifest.keys.authority_key, (uint8_t*) &rom->state->manifest.keys.data,
			sizeof (rom->state->manifest.keys.data),
			rom->state->manifest.keys.header_signed.digest.AsBytes);
		if (status != 0) {
			return status;
		}
	}

	status = rom->base.rot->has_root_key (rom->base.rot);
	if (status != 0) {
		return key_manifest_hsp_rom_verify_transfer_manifest_no_root_key (&rom->base, hash, status,
			KEY_MANIFEST_HSP_ROM_DUAL_ROOT_OWNERSHIP_MANIFEST_MARKER);
	}

	/* Note that both root keys are verified in a single block. */
	status = key_manifest_hsp_rom_verify_transfer_manifest_with_root_key (&rom->base, hash,
		(const uint8_t*) &rom->state->manifest.keys.authenticity_key,
		(2 * SP_ECDSA_P384_PUBLIC_KEY_SIZE),
		KEY_MANIFEST_HSP_ROM_DUAL_ROOT_OWNERSHIP_MANIFEST_MARKER);

	return (status == HW_ROT_BAD_ROOT_KEY) ? KEY_MANIFEST_BAD_ROOT_KEY : status;
}

const struct key_manifest_public_key* key_manifest_hsp_rom_dual_root_get_authority_key (
	const struct key_manifest_hsp_rom_dual_root *rom)
{
	if (rom) {
		if (rom->state->authority_key_populated) {
			return &rom->authority_key;
		}
		else {
			return NULL;
		}
	}
	else {
		return NULL;
	}
}

/**
 * Determine the total key manifest size based on the type of transfer manifest is present.
 *
 * @param transfer_marker The marker value for the transfer manifest.
 *
 * @return Total size of the key manifest or an KEY_MANIFEST_INVALID_FORMAT if the marker is
 * unknown.
 */
static int key_manifest_hsp_rom_dual_root_manifest_size (uint32_t transfer_marker)
{
	int length = sizeof (transfer_marker) + sizeof (struct key_manifest_hsp_rom_dual_root_keys);

	switch (transfer_marker) {
		case KEY_MANIFEST_HSP_ROM_TENANCY_MANIFEST_MARKER:
			length += sizeof (struct key_manifest_hsp_rom_grant);
			break;

		case KEY_MANIFEST_HSP_ROM_DUAL_ROOT_OWNERSHIP_MANIFEST_MARKER:
			length += sizeof (struct key_manifest_hsp_rom_dual_root_ownership);
			break;

		case KEY_MANIFEST_HSP_ROM_NULL_MANIFEST_MARKER:
			/* A null manifest has no additional length. */
			break;

		default:
			return KEY_MANIFEST_INVALID_FORMAT;
	}

	return length;
}

size_t key_manifest_hsp_rom_dual_root_get_total_size (const struct key_manifest_hsp_rom *rom)
{
	const struct key_manifest_hsp_rom_dual_root *manifest =
		(const struct key_manifest_hsp_rom_dual_root*) rom;

	if (manifest) {
		return key_manifest_hsp_rom_dual_root_manifest_size (
			manifest->state->manifest.transfer.marker);
	}
	else {
		return 0;
	}
}

int key_manifest_hsp_rom_dual_root_update_root_key (const struct key_manifest_hsp_rom *rom,
	const struct hash_engine *hash)
{
	const struct key_manifest_hsp_rom_dual_root *rom_dual_root =
		(const struct key_manifest_hsp_rom_dual_root*) rom;

	if ((rom_dual_root == NULL) || (hash == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	return hsp_key_manifest_hsp_rom_execute_ownership_transfer (&rom_dual_root->base, hash,
		(const uint8_t*) &rom_dual_root->state->manifest.keys.authenticity_key,
		(2 * SP_ECDSA_P384_PUBLIC_KEY_SIZE));
}

/**
 * Verify the basic format and construction of manifest data loaded into memory.
 *
 * @param manifest The manifest instance to check.
 *
 * @return 0 if the manifest is constructed correctly or an error code.
 */
static int key_manifest_hsp_rom_dual_root_check_manifest_format (
	const struct key_manifest_hsp_rom_dual_root *manifest)
{
	SP_ECDSA_P384_PUBLIC unset_key = {0};

	if (manifest->state->manifest.keys.marker !=
		KEY_MANIFEST_HSP_ROM_DUAL_ROOT_KEY_MANIFEST_MARKER) {
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

		if ((manifest->state->manifest.keys.header_signed.valid_keys != 1) &&
			(manifest->state->manifest.keys.header_signed.valid_keys != 2)) {
			return KEY_MANIFEST_INVALID_FORMAT;
		}

		/* There are 3 keys with an active grant manifest. The key in the grant manifest
		 * is the FW Authenticity key. The Image Authority key from the tenant manifest is the
		 * Image Authority key. The Image Authenticity Key (Grant Key) from the Tenant manifest
		 * is the Tenancy Grant Key. */
		hsp_fw_load_public_key (&manifest->state->manifest.transfer.grant.data.tenant_key,
			&manifest->state->base.fw_key);
		hsp_fw_load_public_key (&manifest->state->manifest.keys.data.image_authority_key,
			&manifest->state->base.secondary_key);
		hsp_fw_load_public_key (&manifest->state->manifest.keys.data.image_authenticity_key,
			&manifest->state->base.tenancy_grant_key);
	}
	else {
		if (manifest->state->manifest.transfer.marker ==
			KEY_MANIFEST_HSP_ROM_DUAL_ROOT_OWNERSHIP_MANIFEST_MARKER) {
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
		hsp_fw_load_public_key (&manifest->state->manifest.keys.data.image_authenticity_key,
			&manifest->state->base.fw_key);
		hsp_fw_load_public_key (&manifest->state->manifest.keys.data.image_authority_key,
			&manifest->state->base.secondary_key);
	}

	hsp_fw_load_public_key (&manifest->state->manifest.keys.authenticity_key,
		&manifest->state->base.root_key);

	hsp_fw_load_public_key (&manifest->state->manifest.keys.authority_key,
		&manifest->state->authority_key);

	key_manifest_hsp_rom_dual_root_populate_base (manifest->state);

	if (buffer_compare (manifest->state->manifest.keys.authority_key.AsBytes, unset_key.AsBytes,
		SP_ECDSA_P384_PUBLIC_KEY_SIZE) != 0) {
		manifest->state->authority_key_populated = true;
	}

	/* The number of Root Keys and FW Keys must match. */
	if ((manifest->state->authority_key_populated &&
		(manifest->state->manifest.keys.header_signed.valid_keys != 2)) ||
		(!manifest->state->authority_key_populated &&
		(manifest->state->manifest.keys.header_signed.valid_keys != 1))) {
		return KEY_MANIFEST_INVALID_FORMAT;
	}

	return 0;
}

int key_manifest_hsp_rom_dual_root_init_state (const struct key_manifest_hsp_rom *rom,
	const struct flash *flash, uint32_t base_addr)
{
	const struct key_manifest_hsp_rom_dual_root *manifest =
		(const struct key_manifest_hsp_rom_dual_root*) rom;
	size_t offset;
	int status;

	if ((manifest == NULL) || (flash == NULL) || (manifest->state == NULL) ||
		(manifest->base.rot == NULL) || (manifest->base.pka == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	memset (manifest->state, 0, sizeof (struct key_manifest_hsp_rom_dual_root_state));

	status = flash->read (flash, base_addr, (uint8_t*) &manifest->state->manifest.transfer.marker,
		sizeof (manifest->state->manifest.transfer.marker));
	if (status != 0) {
		return status;
	}

	offset = sizeof (manifest->state->manifest.transfer.marker);

	switch (manifest->state->manifest.transfer.marker) {
		case KEY_MANIFEST_HSP_ROM_NULL_MANIFEST_MARKER:
			/* No additional manifest data. */
			break;

		case KEY_MANIFEST_HSP_ROM_TENANCY_MANIFEST_MARKER:
			status = flash->read (flash, base_addr + offset,
				(uint8_t*) &manifest->state->manifest.transfer.grant,
				sizeof (manifest->state->manifest.transfer.grant));
			offset += sizeof (manifest->state->manifest.transfer.grant);
			break;

		case KEY_MANIFEST_HSP_ROM_DUAL_ROOT_OWNERSHIP_MANIFEST_MARKER:
			status = flash->read (flash, base_addr + offset,
				(uint8_t*) &manifest->state->manifest.transfer.owner,
				sizeof (manifest->state->manifest.transfer.owner));
			offset += sizeof (manifest->state->manifest.transfer.owner);
			break;

		default:
			status = KEY_MANIFEST_INVALID_FORMAT;
			break;
	}
	if (status != 0) {
		return status;
	}

	status = flash->read (flash, base_addr + offset, (uint8_t*) &manifest->state->manifest.keys,
		sizeof (manifest->state->manifest.keys));
	if (status != 0) {
		return status;
	}

	return key_manifest_hsp_rom_dual_root_check_manifest_format (manifest);
}

int key_manifest_hsp_rom_dual_root_init_state_from_memory (const struct key_manifest_hsp_rom *rom,
	const uint8_t *base_addr)
{
	const struct key_manifest_hsp_rom_dual_root *manifest =
		(const struct key_manifest_hsp_rom_dual_root*) rom;
	uint32_t offset;

	if ((manifest == NULL) || (base_addr == NULL) || (manifest->state == NULL) ||
		(manifest->base.rot == NULL) || (manifest->base.pka == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	memset (manifest->state, 0, sizeof (struct key_manifest_hsp_rom_dual_root_state));

	memcpy (&manifest->state->manifest.transfer.marker, base_addr,
		sizeof (manifest->state->manifest.transfer.marker));
	offset = sizeof (manifest->state->manifest.transfer.marker);

	switch (manifest->state->manifest.transfer.marker) {
		case KEY_MANIFEST_HSP_ROM_NULL_MANIFEST_MARKER:
			/* No additional manifest data. */
			break;

		case KEY_MANIFEST_HSP_ROM_TENANCY_MANIFEST_MARKER:
			memcpy (&manifest->state->manifest.transfer.grant, &base_addr[offset],
				sizeof (manifest->state->manifest.transfer.grant));
			offset += sizeof (manifest->state->manifest.transfer.grant);
			break;

		case KEY_MANIFEST_HSP_ROM_DUAL_ROOT_OWNERSHIP_MANIFEST_MARKER:
			memcpy (&manifest->state->manifest.transfer.owner, &base_addr[offset],
				sizeof (manifest->state->manifest.transfer.owner));
			offset += sizeof (manifest->state->manifest.transfer.owner);
			break;

		default:
			return KEY_MANIFEST_INVALID_FORMAT;
			break;
	}

	memcpy (&manifest->state->manifest.keys, &base_addr[offset],
		sizeof (manifest->state->manifest.keys));

	return key_manifest_hsp_rom_dual_root_check_manifest_format (manifest);
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
int key_manifest_hsp_rom_dual_root_init (struct key_manifest_hsp_rom_dual_root *manifest,
	struct key_manifest_hsp_rom_dual_root_state *state, const struct hw_rot *rot,
	const struct flash *flash, uint32_t base_addr, const struct ecc_hw *pka)
{
	int status;

	status = key_manifest_hsp_rom_dual_root_init_api (manifest, state, rot, pka);
	if (status != 0) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	return manifest->base.init_state (&manifest->base, flash, base_addr);
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
int key_manifest_hsp_rom_dual_root_init_from_memory (
	struct key_manifest_hsp_rom_dual_root *manifest,
	struct key_manifest_hsp_rom_dual_root_state *state, const struct hw_rot *rot,
	const uint8_t *base_addr, const struct ecc_hw *pka)
{
	int status;

	status = key_manifest_hsp_rom_dual_root_init_api (manifest, state, rot, pka);
	if (status != 0) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	return manifest->base.init_state_from_memory (&manifest->base, base_addr);
}

/**
 * Initialize the API and static contents of the dual root key manifest structure. No manifest
 * information will be loaded and the variable context for the manifest will remain uninitialized.
 * The result of the call is the same as static initialization, except parameter validation is
 * performed.
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
int key_manifest_hsp_rom_dual_root_init_api (struct key_manifest_hsp_rom_dual_root *manifest,
	struct key_manifest_hsp_rom_dual_root_state *state, const struct hw_rot *rot,
	const struct ecc_hw *pka)
{
	int status;

	if ((manifest == NULL) || (state == NULL) || (rot == NULL) || (pka == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	memset (manifest, 0, sizeof (struct key_manifest_hsp_rom_dual_root));

	/* Initialize base key manifest and single root key manifest definitions. */
	status = key_manifest_hsp_rom_init_api (&manifest->base, &state->base, rot, pka);
	if (status != 0) {
		return status;
	}

	/* Override the necessary base or single root key manifest definitions for dual root. */
	manifest->base.base.verify = key_manifest_hsp_rom_dual_root_verify;
	manifest->base.init_state = key_manifest_hsp_rom_dual_root_init_state;
	manifest->base.init_state_from_memory = key_manifest_hsp_rom_dual_root_init_state_from_memory;
	manifest->base.get_total_size = key_manifest_hsp_rom_dual_root_get_total_size;
	manifest->base.update_root_key = key_manifest_hsp_rom_dual_root_update_root_key;
	manifest->base.authenticate_ownership_transfer_root_key_buffer =
		key_manifest_hsp_rom_dual_root_validate_owner_key_buffer;

	/* ROM dual root key manifest definitions. */
	manifest->state = state;
	manifest->get_root_authority_key = key_manifest_hsp_rom_dual_root_get_authority_key;

	manifest->authority_key.type = KEY_MANIFEST_ECC_KEY;
	manifest->authority_key.key.ecc = &state->authority_key;

	return 0;
}

/**
 * Release the resources used by an HSP ROM key manifest.
 *
 * @param manifest The manifest instance to release.
 */
void key_manifest_hsp_rom_dual_root_release (const struct key_manifest_hsp_rom_dual_root *manifest)
{
	UNUSED (manifest);
}

/**
 * Determine the length of an HSP ROM dual root key manifest on flash.  This is determined without
 * needing to initialize a manifest instance, so no data will be permanently loaded into memory.  It
 * will also not be validated in any way.
 *
 * @param flash The flash device that contains the key manifest.
 * @param base_addr Address of the key manifest data.
 *
 * @return The total length of the key manifest or an error code.  Use ROT_IS_ERROR to check the
 * return value.
 */
int key_manifest_hsp_rom_dual_root_get_size_on_flash (const struct flash *flash, uint32_t base_addr)
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

	return key_manifest_hsp_rom_dual_root_manifest_size (marker);
}
