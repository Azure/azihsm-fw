// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "key_manifest_hsp_firmware.h"
#include "platform_api.h"
#include "common/buffer_util.h"
#include "crypto/ecdsa.h"
#include "logging/hsp_logging.h"


int key_manifest_hsp_firmware_verify (const struct key_manifest *manifest,
	const struct hash_engine *hash)
{
	const struct key_manifest_hsp_firmware *fw = (const struct key_manifest_hsp_firmware*) manifest;
	const struct security_policy *policy;
	uint8_t digest[SHA384_HASH_LENGTH] = {0};
	int status;

	if ((manifest == NULL) || (hash == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	/* Use the security policy to determine what kind of verification is needed for the key
	 * manifest.  Skip signature verification of the signed header if the security policy doesn't
	 * require enforcing firmware signing.  Integrity of the manifest data is still checked against
	 * the header digest. */
	policy = security_manager_get_security_policy (fw->security);

	if (security_policy_enforce_firmware_signing (policy)) {
		/* Root key verification against the HW RoT is deferred to the external entity utilizing
		 * this manifest.  Not all workflows require this type of verification, so don't enforce it
		 * here. */
		status = signature_verification_verify_message (fw->ecdsa, hash, HASH_TYPE_SHA384,
			(uint8_t*) &fw->keys->header_signed, sizeof (fw->keys->header_signed),
			fw->root_key.key.ecc_der_ref.der, fw->root_key.key.ecc_der_ref.length,
			fw->keys->signature, sizeof (fw->keys->signature));
		if (status != 0) {
			return status;
		}

		if (fw->secondary_key.key.ecc_der_ref.der != NULL) {
			status = signature_verification_verify_message (fw->ecdsa, hash, HASH_TYPE_SHA384,
				(uint8_t*) &fw->keys->header_signed, sizeof (fw->keys->header_signed),
				fw->secondary_key.key.ecc_der_ref.der, fw->root_key.key.ecc_der_ref.length,
				fw->keys->secondary_sig, sizeof (fw->keys->secondary_sig));
			if (status != 0) {
				return status;
			}
		}
	}

	status = hash->calculate_sha384 (hash, (uint8_t*) fw->keys->fw_key, sizeof (fw->keys->fw_key),
		digest, sizeof (digest));
	if (status != 0) {
		return status;
	}

	status = buffer_compare (digest, fw->keys->header_signed.digest, SHA384_HASH_LENGTH);
	if (status != 0) {
		status = KEY_MANIFEST_VERIFY_FAILED;
	}

	buffer_zeroize (digest, sizeof (digest));

	return status;
}

int key_manifest_hsp_firmware_is_allowed (const struct key_manifest *manifest)
{
	const struct key_manifest_hsp_firmware *fw = (const struct key_manifest_hsp_firmware*) manifest;
	const struct security_policy *policy;
	uint64_t svn;
	int status;

	if (manifest == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	policy = security_manager_get_security_policy (fw->security);
	if (!security_policy_enforce_anti_rollback (policy)) {
		/* The manifest is always allowed when anti-rollback is not enforced. */
		return 1;
	}

	status = fw->rot->get_svn (fw->rot, &svn);
	if (status != 0) {
		return status;
	}

	return (fw->keys->header_signed.svn >= svn);
}

int key_manifest_hsp_firmware_revokes_old_manifest (const struct key_manifest *manifest)
{
	const struct key_manifest_hsp_firmware *fw = (const struct key_manifest_hsp_firmware*) manifest;
	const struct security_policy *policy;
	uint64_t svn;
	int status;

	if (manifest == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	policy = security_manager_get_security_policy (fw->security);
	if (!security_policy_enforce_anti_rollback (policy)) {
		/* The manifest does not revoke anything when anti-rollback is not enforced. */
		return 0;
	}

	status = fw->rot->get_svn (fw->rot, &svn);
	if (status != 0) {
		return status;
	}

	return (fw->keys->header_signed.svn > svn);
}

int key_manifest_hsp_firmware_update_revocation (const struct key_manifest *manifest)
{
	const struct key_manifest_hsp_firmware *fw = (const struct key_manifest_hsp_firmware*) manifest;
	const struct security_policy *policy;
	uint64_t svn;
	int status;

	if (manifest == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	policy = security_manager_get_security_policy (fw->security);
	if (!security_policy_enforce_anti_rollback (policy)) {
		/* SVN update is a no-op when anti-rollback is not enforced. */
		return 0;
	}

	status = fw->rot->get_svn (fw->rot, &svn);
	if (status != 0) {
		return status;
	}

	if (fw->keys->header_signed.svn < svn) {
		return KEY_MANIFEST_REVOKED;
	}

	if (svn != fw->keys->header_signed.svn) {
		/* Only update when the SVN in the manifest is different. */
		status = fw->rot->update_svn (fw->rot, fw->keys->header_signed.svn);
	}
	else {
		/* If the SVN is the same, give the RoT an opportunity to correct any storage errors. */
		status = fw->rot->refresh_svn (fw->rot, fw->keys->header_signed.svn);
		if (status != 0) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_HSP,
				HSP_LOGGING_SVN_REDUNDANCY_DEGRADED, status, 0);

			status = 0;
		}
	}

	return status;
}

const struct key_manifest_public_key* key_manifest_hsp_firmware_get_root_key (
	const struct key_manifest *manifest)
{
	const struct key_manifest_hsp_firmware *fw = (const struct key_manifest_hsp_firmware*) manifest;

	if (fw != NULL) {
		return &fw->root_key;
	}
	else {
		return NULL;
	}
}

const struct key_manifest_public_key* key_manifest_hsp_firmware_get_app_key (
	const struct key_manifest *manifest)
{
	const struct key_manifest_hsp_firmware *fw = (const struct key_manifest_hsp_firmware*) manifest;

	return key_manifest_hsp_firmware_get_public_key (fw, 0);
}

const struct key_manifest_public_key* key_manifest_hsp_firmware_get_manifest_key (
	const struct key_manifest *manifest)
{
	const struct key_manifest_hsp_firmware *fw = (const struct key_manifest_hsp_firmware*) manifest;

	return key_manifest_hsp_firmware_get_public_key (fw, 1);
}

/**
 * Initialize an HSP firmware key manifest instance from data stored on flash.
 *
 * @param manifest The manifest instance to initialize.
 * @param state Variable context for the manifest.  This must not already be initialized.
 * @param key_data Buffer to hold the manifest data that is read from flash.  If this is null, the
 * buffer wil be dynamically allocated.
 * @param rot Interface to the HW RoT state for the firmware manifest.
 * @param security Manager for the device security policy to use during manifest verification.
 * @param ecdsa Signature verification context for verifying ECDSA signatures on the manifest.
 * @param root_key DER encoded public key to use for manifest verification.
 * @param der_length Length of the DER encoded public key.
 * @param secondary_key Optional DER encoded public key to use for verification of the secondary
 * manifest signature.  If this key is provided, the manifest must have a valid secondary signature.
 * @param secondary_length Length of the DER encoded secondary public key.
 * @param flash Flash device where the key manifest is stored.
 * @param base_addr Starting address for the key manifest on flash.
 *
 * @return 0 if the manifest was initialized successfully or an error code.
 */
int key_manifest_hsp_firmware_init (struct key_manifest_hsp_firmware *manifest,
	struct key_manifest_hsp_firmware_manifest *key_data, const struct hw_rot *rot,
	const struct security_manager *security, const struct signature_verification *ecdsa,
	const uint8_t *root_key, size_t der_length, const uint8_t *secondary_key,
	size_t secondary_length, const struct flash *flash, uint32_t base_addr)
{
	int status;

	if (flash == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	status = key_manifest_hsp_firmware_init_api (manifest, key_data, rot, security, ecdsa, root_key,
		der_length, secondary_key, secondary_length);
	if (status != 0) {
		return status;
	}

	status = key_manifest_hsp_firmware_init_keys (manifest, flash, base_addr);
	if ((status != 0) && !manifest->static_keys) {
		platform_free ((void*) manifest->keys);
	}

	return status;
}

/**
 * Initialize an HSP firmware key manifest instance from data stored in memory.
 *
 * @param manifest The manifest instance to initialize.
 * @param state Variable context for the manifest.  This must not already be initialized.
 * @param key_data Buffer containing the manifest data.
 * @param rot Interface to the HW RoT state for the firmware manifest.
 * @param security Manager for the device security policy to use during manifest verification.
 * @param ecdsa Signature verification context for verifying ECDSA signatures on the manifest.
 * @param root_key DER encoded public key to use for manifest verification.
 * @param der_length Length of the DER encoded public key.
 * @param secondary_key Optional DER encoded public key to use for verification of the secondary
 * manifest signature.  If this key is provided, the manifest must have a valid secondary signature.
 * @param secondary_length Length of the DER encoded secondary public key.
 *
 * @return 0 if the manifest was initialized successfully or an error code.
 */
int key_manifest_hsp_firmware_init_from_memory (struct key_manifest_hsp_firmware *manifest,
	const struct key_manifest_hsp_firmware_manifest *key_data, const struct hw_rot *rot,
	const struct security_manager *security, const struct signature_verification *ecdsa,
	const uint8_t *root_key, size_t der_length, const uint8_t *secondary_key,
	size_t secondary_length)
{
	int status;

	if (key_data == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	status = key_manifest_hsp_firmware_init_api (manifest,
		(struct key_manifest_hsp_firmware_manifest*) key_data, rot, security, ecdsa, root_key,
		der_length, secondary_key, secondary_length);
	if (status != 0) {
		return status;
	}

	return key_manifest_hsp_firmware_init_keys_from_memory (manifest);
}

/**
 * Initialize the API and static contents of the key manifest structure.  No manifest information
 * will be loaded.  The result of the call is the same as static initialization, except parameter
 * validation is performed and it allows for dynamic key buffer allocation.
 *
 * @param manifest The manifest instance to initialize.
 * @param key_data Buffer for the key manifest data.  If this is null, the buffer wil be dynamically
 * allocated.
 * @param rot Interface to the HW RoT state for the firmware manifest.
 * @param security Manager for the device security policy to use during manifest verification.
 * @param ecdsa Signature verification context for verifying ECDSA signatures on the manifest.
 * @param root_key DER encoded public key to use for manifest verification.
 * @param der_length Length of the DER encoded public key.
 * @param secondary_key Optional DER encoded public key to use for verification of the secondary
 * manifest signature.  If this key is provided, the manifest must have a valid secondary signature.
 * @param secondary_length Length of the DER encoded secondary public key.
 *
 * @return 0 if the manifest was initialized successfully or an error code.
 */
int key_manifest_hsp_firmware_init_api (struct key_manifest_hsp_firmware *manifest,
	struct key_manifest_hsp_firmware_manifest *key_data, const struct hw_rot *rot,
	const struct security_manager *security, const struct signature_verification *ecdsa,
	const uint8_t *root_key, size_t der_length, const uint8_t *secondary_key,
	size_t secondary_length)
{
	int i;

	if ((manifest == NULL) || (rot == NULL) || (security == NULL) || (ecdsa == NULL) ||
		(root_key == NULL) || (der_length == 0)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	if ((secondary_key != NULL) && (secondary_length == 0)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	memset (manifest, 0, sizeof (struct key_manifest_hsp_firmware));

	if (key_data == NULL) {
		manifest->keys = platform_malloc (sizeof (struct key_manifest_hsp_firmware_manifest));
		if (manifest->keys == NULL) {
			return KEY_MANIFEST_NO_MEMORY;
		}
	}
	else {
		manifest->keys = key_data;
		manifest->static_keys = true;
	}

	manifest->rot = rot;
	manifest->security = security;
	manifest->ecdsa = ecdsa;
	manifest->root_key.type = KEY_MANIFEST_ECC_DER_REF_KEY;
	manifest->root_key.key.ecc_der_ref.der = root_key;
	manifest->root_key.key.ecc_der_ref.length = der_length;
	if (secondary_key != NULL) {
		manifest->secondary_key.type = KEY_MANIFEST_ECC_DER_REF_KEY;
		manifest->secondary_key.key.ecc_der_ref.der = secondary_key;
		manifest->secondary_key.key.ecc_der_ref.length = secondary_length;
	}

	for (i = 0; i < KEY_MANIFEST_HSP_FIRMWARE_KEY_SLOTS; i++) {
		manifest->public_key[i].type = KEY_MANIFEST_ECC_DER_REF_KEY;
		manifest->public_key[i].key.ecc_der_ref.der = key_data->fw_key[i];
		manifest->public_key[i].key.ecc_der_ref.length = ECC_DER_P384_PUBLIC_LENGTH;
	}

	manifest->base.verify = key_manifest_hsp_firmware_verify;
	manifest->base.is_allowed = key_manifest_hsp_firmware_is_allowed;
	manifest->base.revokes_old_manifest = key_manifest_hsp_firmware_revokes_old_manifest;
	manifest->base.update_revocation = key_manifest_hsp_firmware_update_revocation;
	manifest->base.get_root_key = key_manifest_hsp_firmware_get_root_key;
	manifest->base.get_app_key = key_manifest_hsp_firmware_get_app_key;
	manifest->base.get_manifest_key = key_manifest_hsp_firmware_get_manifest_key;

	return 0;
}

/**
 * Check the manifest headers for validity.
 *
 * @param manifest The manifest to check.
 *
 * @return 0 if the key manifest headers represent valid data or an error code.
 */
static int key_manifest_hsp_firmware_check_headers (
	const struct key_manifest_hsp_firmware *manifest)
{
	if (manifest->keys->marker != KEY_MANIFEST_HSP_FIRMWARE_MANIFEST_MARKER) {
		return KEY_MANIFEST_INVALID_FORMAT;
	}

	if (manifest->keys->header_signed.length !=
		sizeof (((struct key_manifest_hsp_firmware_manifest*) 0)->fw_key)) {
		return KEY_MANIFEST_INVALID_FORMAT;
	}

	return 0;
}

/**
 * Initialize only the manifest keys of an HSP firmware key manifest instance from data stored on
 * flash.  The rest of the manifest structure is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized manifest instance, but this can also
 * be called multiple times to load different manifests without calling
 * key_manifest_hsp_firmware_release.  All manifests loaded in this way must use the same root key
 * for verification.
 *
 * @param manifest The manifest instance containing the state to initialize.
 * @param flash Flash device where the key manifest is stored.
 * @param base_addr Starting address for the key manifest on flash.
 *
 * @return 0 if the manifest state was successfully initialized or an error code.
 */
int key_manifest_hsp_firmware_init_keys (const struct key_manifest_hsp_firmware *manifest,
	const struct flash *flash, uint32_t base_addr)
{
	int status;

	if ((manifest == NULL) || (flash == NULL) || (manifest->keys == NULL) ||
		(manifest->rot == NULL) || (manifest->security == NULL) || (manifest->ecdsa == NULL) ||
		(manifest->root_key.key.ecc_der_ref.der == NULL) ||
		(manifest->root_key.key.ecc_der_ref.length == 0)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	if ((manifest->secondary_key.key.ecc_der_ref.der != NULL) &&
		(manifest->secondary_key.key.ecc_der_ref.length == 0)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	status = flash->read (flash, base_addr, (uint8_t*) manifest->keys,
		sizeof (struct key_manifest_hsp_firmware_manifest));
	if (status != 0) {
		return status;
	}

	return key_manifest_hsp_firmware_check_headers (manifest);
}

/**
 * Initialize only the manifest keys of an HSP firmware key manifest instance from data stored in
 * memory.  The rest of the manifest structure is assumed to have already been initialized.
 *
 * It is expected that the buffer for the key manifest instance already contains the manifest data.
 *
 * This would generally be used with a statically initialized manifest instance, but this can also
 * be called multiple times to load different manifests without calling
 * key_manifest_hsp_firmware_release.  All manifests loaded in this way must use the same root key
 * for verification.
 *
 * @param manifest The manifest instance containing the state to initialize.
 *
 * @return 0 if the manifest state was successfully initialized or an error code.
 */
int key_manifest_hsp_firmware_init_keys_from_memory (
	const struct key_manifest_hsp_firmware *manifest)
{
	if ((manifest == NULL) || (manifest->keys == NULL) || (manifest->rot == NULL) ||
		(manifest->security == NULL) || (manifest->ecdsa == NULL) ||
		(manifest->root_key.key.ecc_der_ref.der == NULL) ||
		(manifest->root_key.key.ecc_der_ref.length == 0)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	if ((manifest->secondary_key.key.ecc_der_ref.der != NULL) &&
		(manifest->secondary_key.key.ecc_der_ref.length == 0)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	return key_manifest_hsp_firmware_check_headers (manifest);
}

/**
 * Release the resources used by an HSP firmware key manifest.
 *
 * @param manifest The manifest instance to release.
 */
void key_manifest_hsp_firmware_release (const struct key_manifest_hsp_firmware *manifest)
{
	if (manifest && !manifest->static_keys) {
		platform_free ((void*) manifest->keys);
	}
}

/**
 * Get the SVN value of an HSP firmware key manifest.
 *
 * @param rom The key manifest to query.
 *
 * @return The SVN value from the key manifest.  If the manifest is null, 0 will be returned.
 */
uint64_t key_manifest_hsp_firmware_get_svn (const struct key_manifest_hsp_firmware *manifest)
{
	if (manifest != NULL) {
		return manifest->keys->header_signed.svn;
	}
	else {
		return 0;
	}
}

/**
 * Determine if the specified key slot contains a valid public key.
 *
 * @param manifest The key manifest to query.
 * @param key_index Index of the public key to check.
 *
 * @return true if the key slot has a valid key or false if not.
 */
static bool key_manifest_hsp_firmware_is_public_key_valid (
	const struct key_manifest_hsp_firmware *manifest, int key_index)
{
	if ((key_index < 0) || (key_index >= KEY_MANIFEST_HSP_FIRMWARE_KEY_SLOTS)) {
		return false;
	}

	/* A DER encoded key must start with the ASN.1 sequence identifier.  If this value is not
	 * present, there is no valid key in the slot. */
	return (manifest->public_key[key_index].key.ecc_der_ref.der[0] == 0x30);
}

/**
 * Get a DER encoded public key contained in the HSP firmware key manifest.
 *
 * @param manifest The key manifest to query.
 * @param key_index Index of the public key to get.
 *
 * @return The public key or null if there is an error or no key present at the specified index.
 * The memory for this key is managed by the manifest instance.
 */
const struct key_manifest_public_key* key_manifest_hsp_firmware_get_public_key (
	const struct key_manifest_hsp_firmware *manifest, int key_index)
{
	if (manifest == NULL) {
		return NULL;
	}

	if (key_manifest_hsp_firmware_is_public_key_valid (manifest, key_index)) {
		return &manifest->public_key[key_index];
	}
	else {
		return NULL;
	}
}

/**
 * Check that the HSP firmware key manifest contains valid public keys at a group of indicies.  If
 * any specified key slot is empty, the check will fail.
 *
 * @param manifest The key manifest to query.
 * @param key_mask Bit mask of key slots that must have a valid public key.  Each bit position
 * represents a key slot that will be checked when set.
 *
 * @return 0 if all specified key slots have valid public keys or an error code.
 */
int key_manifest_hsp_firmware_check_public_keys (const struct key_manifest_hsp_firmware *manifest,
	uint16_t key_mask)
{
	uint16_t check = 0x01;
	int i;

	if (manifest == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	if (key_mask & ~((1U << KEY_MANIFEST_HSP_FIRMWARE_KEY_SLOTS) - 1)) {
		/* There are bits set beyond the supported number of key slots. */
		return KEY_MANIFEST_UNSUPPORTED_KEY;
	}

	for (i = 0; i < KEY_MANIFEST_HSP_FIRMWARE_KEY_SLOTS; i++, check <<= 1) {
		if ((key_mask & check) && !key_manifest_hsp_firmware_is_public_key_valid (manifest, i)) {
			return KEY_MANIFEST_MISSING_KEY;
		}
	}

	return 0;
}
