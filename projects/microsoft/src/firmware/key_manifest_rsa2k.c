// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdlib.h>
#include <string.h>
#include "platform_api.h"
#include "firmware/key_manifest_rsa2k.h"


static int key_manifest_rsa2k_verify (const struct key_manifest *manifest,
	const struct hash_engine *hash)
{
	const struct key_manifest_rsa2k *cert = (const struct key_manifest_rsa2k*) manifest;
	uint8_t digest[SHA256_HASH_LENGTH];
	int status;

	if ((cert == NULL) || (hash == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	status = cert->hw->verify_root_key (cert->hw, (const uint8_t*) &cert->cert->root_key,
		sizeof (cert->cert->root_key), hash);
	if (status != 0) {
		return status;
	}

	status = hash->calculate_sha256 (hash, (uint8_t*) cert->cert,
		(sizeof (struct key_certificate) - sizeof (cert->cert->signature)), digest,
		sizeof (digest));
	if (status != 0) {
		return status;
	}

	return cert->rsa->sig_verify (cert->rsa, &cert->root_key_data,
		(uint8_t*) &cert->cert->signature, sizeof (cert->cert->signature), HASH_TYPE_SHA256, digest,
		sizeof (digest));
}

/**
 * Check a certificate ID against the hardware revocation state to see if the certificate is allowed
 * to be used.
 *
 * @param hw The hardware containing the revocation information.
 * @param cert_id The certificate ID to check.
 *
 * @return 0 if the certificate ID is not allowed to run, 1 if it is, or an error code.
 */
int key_manifest_rsa2k_check_cert_id (struct cert_device_hw *hw, uint32_t cert_id)
{
	uint32_t revocation;
	int status;

	status = hw->get_revocation (hw, &revocation);
	if (status != 0) {
		return status;
	}

	if ((cert_id == revocation) || (cert_id == ((revocation + 1) | revocation))) {
		return 1;
	}
	else {
		return 0;
	}
}

static int key_manifest_rsa2k_is_allowed (const struct key_manifest *manifest)
{
	const struct key_manifest_rsa2k *cert = (const struct key_manifest_rsa2k*) manifest;

	if (cert == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	return key_manifest_rsa2k_check_cert_id (cert->hw, cert->cert->cert_id);
}

/**
 * Check a certificate ID against the hardware revocation state to see if the certificate changes
 * the revocation state.
 *
 * @param hw The hardware containing the revocation information.
 * @param cert_id The certificate ID to check.
 *
 * @return 1 if the certificate ID updates the revocation state, 0 if not, or an error code.
 */
int key_manifest_rsa2k_check_cert_revocation (struct cert_device_hw *hw, uint32_t cert_id)
{
	uint32_t revocation;
	int status;

	status = hw->get_revocation (hw, &revocation);
	if (status != 0) {
		return status;
	}

	if (cert_id > revocation) {
		return 1;
	}
	else {
		return 0;
	}
}

static int key_manifest_rsa2k_revokes_old_manifest (const struct key_manifest *manifest)
{
	const struct key_manifest_rsa2k *cert = (const struct key_manifest_rsa2k*) manifest;

	if (cert == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	return key_manifest_rsa2k_check_cert_revocation (cert->hw, cert->cert->cert_id);
}

static int key_manifest_rsa2k_update_revocation (const struct key_manifest *manifest)
{
	const struct key_manifest_rsa2k *cert = (const struct key_manifest_rsa2k*) manifest;

	if (cert == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	return cert->hw->set_revocation (cert->hw, cert->cert->cert_id);
}

static const struct key_manifest_public_key* key_manifest_rsa2k_get_root_key (
	const struct key_manifest *manifest)
{
	const struct key_manifest_rsa2k *cert = (const struct key_manifest_rsa2k*) manifest;

	if (cert != NULL) {
		return &cert->root_key;
	}
	else {
		return NULL;
	}
}

static const struct key_manifest_public_key* key_manifest_rsa2k_get_app_key (
	const struct key_manifest *manifest)
{
	const struct key_manifest_rsa2k *cert = (const struct key_manifest_rsa2k*) manifest;

	if (cert != NULL) {
		return &cert->app_key;
	}
	else {
		return NULL;
	}
}

static const struct key_manifest_public_key* key_manifest_rsa2k_get_manifest_key (
	const struct key_manifest *manifest)
{
	const struct key_manifest_rsa2k *cert = (const struct key_manifest_rsa2k*) manifest;

	if (cert != NULL) {
		return &cert->pfm_key;
	}
	else {
		return NULL;
	}
}

/**
 * Load a public key from the certificate.
 *
 * @param dest The destination container for the public key.
 * @param src The key in the certificate to load.
 */
static void key_manifest_rsa2k_load_key (struct rsa_public_key *dest,
	const struct cert_public_key *src)
{
	memcpy (&dest->modulus, &src->modulus, sizeof (src->modulus));
	dest->mod_length = sizeof (src->modulus);
	dest->exponent = ((src->exponent >> 24) & 0xff) | ((src->exponent >> 8) & 0xff00) |
		((src->exponent << 8) & 0xff0000) | ((src->exponent << 24) & 0xff000000);
}

/**
 * Initialize the key manifest from certificate data contained in the manifest instance.
 *
 * @param manifest The manifest instance to load.
 * @param hw The platform hardware API for certificate operations.
 * @param rsa The RSA engine for verification.
 *
 * @return 0 if the key manifest was successfully initialized or an error code.
 */
static int key_manifest_rsa2k_init_from_cert (struct key_manifest_rsa2k *manifest,
	struct cert_device_hw *hw, const struct rsa_engine *rsa)
{
	if ((manifest->cert->version != KEY_MANIFEST_VERSION) ||
		(manifest->cert->marker != KEY_MANIFEST_MARKER)) {
		return KEY_MANIFEST_INVALID_FORMAT;
	}

	manifest->base.verify = key_manifest_rsa2k_verify;
	manifest->base.is_allowed = key_manifest_rsa2k_is_allowed;
	manifest->base.revokes_old_manifest = key_manifest_rsa2k_revokes_old_manifest;
	manifest->base.update_revocation = key_manifest_rsa2k_update_revocation;
	manifest->base.get_root_key = key_manifest_rsa2k_get_root_key;
	manifest->base.get_app_key = key_manifest_rsa2k_get_app_key;
	manifest->base.get_manifest_key = key_manifest_rsa2k_get_manifest_key;

	manifest->hw = hw;
	manifest->rsa = rsa;

	key_manifest_rsa2k_load_key (&manifest->root_key_data, &manifest->cert->root_key);
	manifest->root_key.type = KEY_MANIFEST_RSA_KEY;
	manifest->root_key.key.rsa = &manifest->root_key_data;

	key_manifest_rsa2k_load_key (&manifest->app_key_data, &manifest->cert->app_key);
	manifest->app_key.type = KEY_MANIFEST_RSA_KEY;
	manifest->app_key.key.rsa = &manifest->app_key_data;

	key_manifest_rsa2k_load_key (&manifest->pfm_key_data, &manifest->cert->pfm_key);
	manifest->pfm_key.type = KEY_MANIFEST_RSA_KEY;
	manifest->pfm_key.key.rsa = &manifest->pfm_key_data;

	return 0;
}

/**
 * Initialize a common key manifest instance.  The manifest will be loaded from flash and stored
 * in memory.
 *
 * @param manifest The manifest instance to load.
 * @param hw The platform hardware API for certificate operations.
 * @param flash The flash device that contains the key manifest.
 * @param base_addr The start address of the manifest in flash.
 * @param rsa The RSA engine to use for manifest verification.
 *
 * @return 0 if the key manifest was successfully initialized or an error code.
 */
int key_manifest_rsa2k_init (struct key_manifest_rsa2k *manifest, struct cert_device_hw *hw,
	const struct flash *flash, uint32_t base_addr, const struct rsa_engine *rsa)
{
	int status;

	if ((manifest == NULL) || (hw == NULL) || (flash == NULL) || (rsa == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	memset (manifest, 0, sizeof (struct key_manifest_rsa2k));

	manifest->cert = platform_malloc (sizeof (struct key_certificate));
	if (manifest->cert == NULL) {
		return KEY_MANIFEST_NO_MEMORY;
	}

	manifest->cert_alloc = true;

	status = flash->read (flash, base_addr, (uint8_t*) manifest->cert,
		sizeof (struct key_certificate));
	if (status != 0) {
		goto error;
	}

	status = key_manifest_rsa2k_init_from_cert (manifest, hw, rsa);
	if (status != 0) {
		goto error;
	}

	return 0;

error:
	platform_free ((void*) manifest->cert);

	return status;
}

/**
 * Initialize a common key manifest instance.  The manifest data is already in memory, and is
 * expected to be valid at that memory location while this interface is active.
 *
 * @param manifest The manifest instance to load.
 * @param hw The platform hardware API for certificate operations.
 * @param cert The certificate in memory that contains the manifest data.
 * @param rsa The RSA engine to use for manifest verification.
 *
 * @return 0 if the key manifest was successfully initialized or an error code.
 */
int key_manifest_rsa2k_init_from_memory (struct key_manifest_rsa2k *manifest,
	struct cert_device_hw *hw, const struct key_certificate *cert, const struct rsa_engine *rsa)
{
	if ((manifest == NULL) || (hw == NULL) || (cert == NULL) || (rsa == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	memset (manifest, 0, sizeof (struct key_manifest_rsa2k));

	manifest->cert = cert;
	manifest->cert_alloc = false;

	return key_manifest_rsa2k_init_from_cert (manifest, hw, rsa);
}

/**
 * Release the resources used by a key manifest.
 *
 * @param manifest The manifest to release.
 */
void key_manifest_rsa2k_release (struct key_manifest_rsa2k *manifest)
{
	if (manifest && manifest->cert_alloc) {
		platform_free ((void*) manifest->cert);
	}
}
