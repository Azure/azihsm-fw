// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "platform_api.h"
#include "common/buffer_util.h"
#include "firmware/key_manifest_rsa2k.h"
#include "firmware/key_manifest_rsa_cert_block.h"
#include "flash/flash_util.h"


/**
 * Get the offset in the certificate block for the root key hash table.
 *
 * @param x The cert block instance.
 */
#define	CERT_BLOCK_RKH_OFFSET(x)\
	(x->address + sizeof (struct key_manifest_rsa_cert_block_header) + x->table_length)


/**
 * Initialize an RSA key from a certificate.
 *
 * @param x509 The X.509 engine for the certificate.
 * @param cert The certificate that contains the key.
 * @param rsa RSA engine to use to initialize the key.
 * @param key The key to initialize.
 *
 * @return 0 if the key was successfully initialized or an error code.
 */
static int key_manifest_rsa_cert_block_init_public_key (const struct x509_engine *x509,
	struct x509_certificate *cert, const struct rsa_engine *rsa, struct rsa_public_key *key)
{
	uint8_t *der;
	size_t length;
	int status;

	status = x509->get_public_key (x509, cert, &der, &length);
	if (status != 0) {
		return status;
	}

	status = rsa->init_public_key (rsa, key, der, length);

	platform_free (der);

	return status;
}

/**
 * Verify that the certificate table in the certificate block is valid.
 *
 * @param manifest The manifest to validate.
 * @param root List of trusted root keys.
 * @param table The certificate chain to validate.
 * @param hash Hash engine to use for validation.
 *
 * @return 0 if the certificate table is valid or an error code.
 */
static int key_manifest_rsa_cert_block_verify_cert_table (
	const struct key_manifest_rsa_cert_block *manifest,
	const struct key_manifest_rsa_cert_block_root_keys *root, const uint8_t *table,
	const struct hash_engine *hash)
{
	const uint8_t *root_der;
	uint32_t root_length;
	struct x509_certificate root_cert;
	uint8_t root_key_exp[4];
	uint8_t root_key_hash[SHA256_HASH_LENGTH];
	const uint8_t *app_der;
	uint32_t app_length;
	struct x509_certificate app_cert;
	struct x509_ca_certs ca;
	size_t min_key_length;
	int status;
	int i;
	int j;

	/* Authenticate the certificate chain contained in the certificate table. */
	status = manifest->x509->init_ca_cert_store (manifest->x509, &ca);
	if (status != 0) {
		return status;
	}

	root_length = *((uint32_t*) table);
	root_der = table + 4;
	status = manifest->x509->add_root_ca (manifest->x509, &ca, root_der, root_length);
	if (status != 0) {
		goto exit_store;
	}

	app_length = *((uint32_t*) (table + 4 + root_length));
	app_der = table + 4 + root_length + 4;
	status = manifest->x509->load_certificate (manifest->x509, &app_cert, app_der, app_length);
	if (status != 0) {
		goto exit_store;
	}

	status = manifest->x509->authenticate (manifest->x509, &app_cert, &ca);
	if (status != 0) {
		goto exit_app;
	}

	/* Check that the public key for the root certificate is one of the trusted root keys. */
	status = manifest->x509->load_certificate (manifest->x509, &root_cert, root_der, root_length);
	if (status != 0) {
		goto exit_app;
	}

	root_key_exp[0] = manifest->root_key_data.exponent >> 24;
	root_key_exp[1] = manifest->root_key_data.exponent >> 16;
	root_key_exp[2] = manifest->root_key_data.exponent >> 8;
	root_key_exp[3] = manifest->root_key_data.exponent;
	i = 0;
	j = 4;
	while ((i < 3) && (root_key_exp[i] == 0)) {
		i++;
		j--;
	}

	status = hash->start_sha256 (hash);
	if (status != 0) {
		goto exit_root;
	}

	status = hash->update (hash, manifest->root_key_data.modulus,
		manifest->root_key_data.mod_length);
	if (status == 0) {
		status = hash->update (hash, &root_key_exp[i], j);
		if (status == 0) {
			status = hash->finish (hash, root_key_hash, sizeof (root_key_hash));
		}
	}
	if (status != 0) {
		hash->cancel (hash);
		goto exit_root;
	}

	i = 0;
	while ((i < 4) &&
		(buffer_compare (root_key_hash, root->key_hash[i], SHA256_HASH_LENGTH) != 0)) {
		bool unused = true;

		for (j = 0; j < SHA256_HASH_LENGTH; j++) {
			if (root->key_hash[i][j] != 0) {
				unused = false;
			}
		}

		if (!unused) {
			i++;
		}
		else {
			/* Stop looking for hash matches as soon as we encounter an unused root key slot. */
			i = 4;
		}
	}
	if (i == 4) {
		status = KEY_MANIFEST_BAD_ROOT_KEY;
		goto exit_root;
	}

	status = manifest->hw->is_root_key_trusted (manifest->hw, i);
	if (status != 1) {
		if (status == 0) {
			status = KEY_MANIFEST_UNTRUSTED_ROOT_KEY;
		}
		goto exit_root;
	}

	/* Check that the key lengths are supported. */
	status = manifest->hw->get_minimum_key_length (manifest->hw, &min_key_length);
	if (status != 0) {
		goto exit_root;
	}

	min_key_length /= 8;
	if ((manifest->root_key_data.mod_length < min_key_length) ||
		(manifest->image_key_data.mod_length < min_key_length)) {
		status = KEY_MANIFEST_WEAK_KEY;
	}

exit_root:
	manifest->x509->release_certificate (manifest->x509, &root_cert);
exit_app:
	manifest->x509->release_certificate (manifest->x509, &app_cert);
exit_store:
	manifest->x509->release_ca_cert_store (manifest->x509, &ca);

	return status;
}

static int key_manifest_rsa_cert_block_verify (const struct key_manifest *manifest,
	const struct hash_engine *hash)
{
	const struct key_manifest_rsa_cert_block *cert =
		(const struct key_manifest_rsa_cert_block*) manifest;
	struct key_manifest_rsa_cert_block_root_keys root;
	uint8_t *cert_table;
	int status;

	if ((cert == NULL) || (hash == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	status = cert->flash->read (cert->flash, CERT_BLOCK_RKH_OFFSET (cert), (uint8_t*) &root,
		sizeof (root));
	if (status != 0) {
		return status;
	}

	status = cert->hw->verify_root_key (cert->hw, (uint8_t*) &root, sizeof (root), hash);
	if (status != 0) {
		return status;
	}

	cert_table = platform_malloc (cert->table_length);
	if (cert_table == NULL) {
		return KEY_MANIFEST_NO_MEMORY;
	}

	status = cert->flash->read (cert->flash,
		cert->address + sizeof (struct key_manifest_rsa_cert_block_header), cert_table,
		cert->table_length);
	if (status != 0) {
		goto exit;
	}

	status = key_manifest_rsa_cert_block_verify_cert_table (cert, &root, cert_table, hash);

exit:
	platform_free (cert_table);

	return status;
}

static int key_manifest_rsa_cert_block_memory_verify (const struct key_manifest *manifest,
	const struct hash_engine *hash)
{
	const struct key_manifest_rsa_cert_block *cert =
		(const struct key_manifest_rsa_cert_block*) manifest;
	struct key_manifest_rsa_cert_block_root_keys *root;
	uint8_t *cert_table;
	int status;

	if ((cert == NULL) || (hash == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	root = (void*) CERT_BLOCK_RKH_OFFSET (cert);

	status = cert->hw->verify_root_key (cert->hw, (uint8_t*) root, sizeof (*root), hash);
	if (status != 0) {
		return status;
	}

	cert_table = (uint8_t*) (cert->address + sizeof (struct key_manifest_rsa_cert_block_header));

	return key_manifest_rsa_cert_block_verify_cert_table (cert, root, cert_table, hash);
}

static int key_manifest_rsa_cert_block_is_allowed (const struct key_manifest *manifest)
{
	const struct key_manifest_rsa_cert_block *cert =
		(const struct key_manifest_rsa_cert_block*) manifest;

	if (cert == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	return key_manifest_rsa2k_check_cert_id (cert->hw, cert->revocation_id);
}

static int key_manifest_rsa_cert_block_revokes_old_manifest (const struct key_manifest *manifest)
{
	const struct key_manifest_rsa_cert_block *cert =
		(const struct key_manifest_rsa_cert_block*) manifest;

	if (cert == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	return key_manifest_rsa2k_check_cert_revocation (cert->hw, cert->revocation_id);
}

static int key_manifest_rsa_cert_block_update_revocation (const struct key_manifest *manifest)
{
	const struct key_manifest_rsa_cert_block *cert =
		(const struct key_manifest_rsa_cert_block*) manifest;

	if (cert == NULL) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	return cert->hw->set_revocation (cert->hw, cert->revocation_id);
}

static const struct key_manifest_public_key* key_manifest_rsa_cert_block_get_root_key (
	const struct key_manifest *manifest)
{
	const struct key_manifest_rsa_cert_block *cert =
		(const struct key_manifest_rsa_cert_block*) manifest;

	if (cert != NULL) {
		return &cert->root_key;
	}
	else {
		return NULL;
	}
}

static const struct key_manifest_public_key* key_manifest_rsa_cert_block_get_app_key (
	const struct key_manifest *manifest)
{
	const struct key_manifest_rsa_cert_block *cert =
		(const struct key_manifest_rsa_cert_block*) manifest;

	if (cert != NULL) {
		return &cert->image_key;
	}
	else {
		return NULL;
	}
}

static const struct key_manifest_public_key* key_manifest_rsa_cert_block_get_manifest_key (
	const struct key_manifest *manifest)
{
	const struct key_manifest_rsa_cert_block *cert =
		(const struct key_manifest_rsa_cert_block*) manifest;

	if (cert != NULL) {
		return &cert->image_key;
	}
	else {
		return NULL;
	}
}

static int key_manifest_rsa_cert_block_get_hash (const struct key_manifest_rsa_cert_block *manifest,
	const struct hash_engine *hash, uint8_t *hash_out, size_t hash_length)
{
	size_t total_length;

	if ((manifest == NULL) || (hash == NULL) || (hash_out == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	total_length = sizeof (struct key_manifest_rsa_cert_block_header) + manifest->table_length +
		sizeof (struct key_manifest_rsa_cert_block_root_keys);

	return flash_hash_contents (manifest->flash, manifest->address, total_length, hash,
		HASH_TYPE_SHA256, hash_out, hash_length);
}

static int key_manifest_rsa_cert_block_memory_get_hash (
	const struct key_manifest_rsa_cert_block *manifest, const struct hash_engine *hash,
	uint8_t *hash_out, size_t hash_length)
{
	size_t total_length;

	if ((manifest == NULL) || (hash == NULL) || (hash_out == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	total_length = sizeof (struct key_manifest_rsa_cert_block_header) + manifest->table_length +
		sizeof (struct key_manifest_rsa_cert_block_root_keys);

	return hash->calculate_sha256 (hash, (const uint8_t*) manifest->address, total_length, hash_out,
		hash_length);
}

static int key_manifest_rsa_cert_block_get_root_keys (
	const struct key_manifest_rsa_cert_block *manifest,
	struct key_manifest_rsa_cert_block_root_keys *root_keys)
{
	if ((manifest == NULL) || (root_keys == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	return manifest->flash->read (manifest->flash, CERT_BLOCK_RKH_OFFSET (manifest),
		(uint8_t*) root_keys, sizeof (*root_keys));
}

static int key_manifest_rsa_cert_block_memory_get_root_keys (
	const struct key_manifest_rsa_cert_block *manifest,
	struct key_manifest_rsa_cert_block_root_keys *root_keys)
{
	if ((manifest == NULL) || (root_keys == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	memcpy (root_keys, (uint8_t*) CERT_BLOCK_RKH_OFFSET (manifest), sizeof (*root_keys));

	return 0;
}

/**
 * Load a certificate from the table.
 *
 * @param x509 X.509 engine to load the certificate with.
 * @param der The certificate data to load.
 * @param length The length of the certificate data.
 * @param cert The certificate to load.
 *
 * @return 0 if the certificate was loaded successfully or an error code.
 */
static int key_manifest_rsa_cert_block_init_certificate (const struct x509_engine *x509,
	const uint8_t *der, size_t length, struct x509_certificate *cert)
{
	int cert_check;
	int status;

	status = x509->load_certificate (x509, cert, der, length);
	if (status != 0) {
		return status;
	}

	cert_check = x509->get_certificate_version (x509, cert);
	if (cert_check != X509_CERT_VERSION_3) {
		status = ROT_IS_ERROR (cert_check) ? cert_check : KEY_MANIFEST_UNSUPPORTED_CERT;
		goto exit;
	}

	cert_check = x509->get_public_key_type (x509, cert);
	if (cert_check != X509_PUBLIC_KEY_RSA) {
		status = ROT_IS_ERROR (cert_check) ? cert_check : KEY_MANIFEST_UNSUPPORTED_KEY;
		goto exit;
	}

	cert_check = x509->get_public_key_length (x509, cert);
	if ((cert_check != 2048) && (cert_check != 3072) && (cert_check != 4096)) {
		status = ROT_IS_ERROR (cert_check) ? cert_check : KEY_MANIFEST_UNSUPPORTED_KEY;
	}

exit:
	if (status != 0) {
		x509->release_certificate (x509, cert);
	}

	return status;
}

/**
 * Initialize the keys from the certificate table.
 *
 * @param manifest The key manifest to initialize.
 * @param hw Interface to device hardware for certificate management.
 * @param x509 The X.509 engine to use for certificate operations.
 * @param rsa The RSA engine to use for key operations.
 * @param header The certificate block header.
 * @param table The certificate table.
 *
 * @param header Header for the certificate block.
 *
 * @return 0 if the manifest was successfully initialized or an error code.
 */
static int key_manifest_rsa_cert_block_init_keys (struct key_manifest_rsa_cert_block *manifest,
	struct cert_device_hw *hw, const struct x509_engine *x509, const struct rsa_engine *rsa,
	const struct key_manifest_rsa_cert_block_header *header, const uint8_t *table)
{
	const uint8_t *root_der;
	size_t root_length;
	struct x509_certificate root_cert;
	const uint8_t *image_der;
	size_t image_length;
	struct x509_certificate image_cert;
	uint8_t image_serial[X509_MAX_SERIAL_NUMBER];
	int status;

	/* Check that the length of the certificate table matches the total length of the certificates
	 * contained in the table.  Also confirm required alignment for individual certificates. */
	root_length = *((uint32_t*) table);
	root_der = table + 4;
	if ((root_length % 4) != 0) {
		return KEY_MANIFEST_INVALID_FORMAT;
	}

	image_length = *((uint32_t*) (table + 4 + root_length));
	image_der = table + 4 + root_length + 4;
	/* If the root cert and overall block are both aligned correctly, the image cert must also be
	 * correctly aligned.  No need to check it here. */

	if ((4 + root_length + 4 + image_length) != header->cert_table_length) {
		return KEY_MANIFEST_INVALID_FORMAT;
	}

	/* Check that the certificates and keys are supported. */
	status = key_manifest_rsa_cert_block_init_certificate (x509, root_der, root_length, &root_cert);
	if (status != 0) {
		return status;
	}

	status = key_manifest_rsa_cert_block_init_certificate (x509, image_der, image_length,
		&image_cert);
	if (status != 0) {
		goto exit_root;
	}

	/* Load persistent data from the certificates. */
	manifest->root_key.type = KEY_MANIFEST_RSA_KEY;
	manifest->root_key.key.rsa = &manifest->root_key_data;
	status = key_manifest_rsa_cert_block_init_public_key (x509, &root_cert, rsa,
		&manifest->root_key_data);
	if (status != 0) {
		goto exit_image;
	}

	manifest->image_key.type = KEY_MANIFEST_RSA_KEY;
	manifest->image_key.key.rsa = &manifest->image_key_data;
	status = key_manifest_rsa_cert_block_init_public_key (x509, &image_cert, rsa,
		&manifest->image_key_data);
	if (status != 0) {
		goto exit_image;
	}

	status = x509->get_serial_number (x509, &image_cert, image_serial, sizeof (image_serial));
	if (ROT_IS_ERROR (status)) {
		goto exit_image;
	}
	else if ((status < 4) || (image_serial[0] != 0x3c) || (image_serial[1] != 0xc3)) {
		status = KEY_MANIFEST_UNSUPPORTED_CERT;
		goto exit_image;
	}
	else {
		manifest->revocation_id = image_serial[2] | (image_serial[3] << 8);
		status = 0;
	}

	/* Base initialization. */
	manifest->base.is_allowed = key_manifest_rsa_cert_block_is_allowed;
	manifest->base.revokes_old_manifest = key_manifest_rsa_cert_block_revokes_old_manifest;
	manifest->base.update_revocation = key_manifest_rsa_cert_block_update_revocation;
	manifest->base.get_root_key = key_manifest_rsa_cert_block_get_root_key;
	manifest->base.get_app_key = key_manifest_rsa_cert_block_get_app_key;
	manifest->base.get_manifest_key = key_manifest_rsa_cert_block_get_manifest_key;

	manifest->hw = hw;
	manifest->x509 = x509;
	manifest->rsa = rsa;
	manifest->table_length = header->cert_table_length;

exit_image:
	x509->release_certificate (x509, &image_cert);
exit_root:
	x509->release_certificate (x509, &root_cert);

	return status;
}

/**
 * Verify the certificate block header on the manifest contains valid information.
 *
 * @param header The header to verify.
 *
 * @return 0 if the header is valid or an error code.
 */
static int key_manifest_rsa_cert_block_init_check_header (
	const struct key_manifest_rsa_cert_block_header *header)
{
	if ((header->signature != KEY_MANIFEST_RSA_CERT_BLOCK_SIGNATURE) ||
		(header->major_version != 1) || (header->minor_version != 0) ||
		(header->header_length != sizeof (struct key_manifest_rsa_cert_block_header)) ||
		(header->flags != 0) || (header->cert_count != 2) ||
		((header->cert_table_length % 4) != 0)) {
		return KEY_MANIFEST_INVALID_FORMAT;
	}

	return 0;
}

/**
 * Initialize a key manifest using an X.509 certificate chain.  The manifest is stored in flash.
 *
 * @param manifest The key manifest to initialize.
 * @param hw Interface to device hardware for certificate management.
 * @param flash The flash where the key manifest is stored.
 * @param base_addr Starting address of the key manifest on flash.
 * @param x509 The X.509 engine to use for certificate operations.
 * @param rsa The RSA engine to use for key operations.
 *
 * @return 0 if the key manifest was successfully initialized or an error code.
 */
int key_manifest_rsa_cert_block_init (struct key_manifest_rsa_cert_block *manifest,
	struct cert_device_hw *hw, const struct flash *flash, uint32_t base_addr,
	const struct x509_engine *x509, const struct rsa_engine *rsa)
{
	struct key_manifest_rsa_cert_block_header header;
	uint8_t *cert_table;
	int status;

	if ((manifest == NULL) || (hw == NULL) || (flash == NULL) || (x509 == NULL) || (rsa == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	memset (manifest, 0, sizeof (struct key_manifest_rsa_cert_block));

	status = flash->read (flash, base_addr, (uint8_t*) &header, sizeof (header));
	if (status != 0) {
		return status;
	}

	status = key_manifest_rsa_cert_block_init_check_header (&header);
	if (status != 0) {
		return status;
	}

	cert_table = platform_malloc (header.cert_table_length);
	if (cert_table == NULL) {
		return KEY_MANIFEST_NO_MEMORY;
	}

	status = flash->read (flash, base_addr + sizeof (header), cert_table, header.cert_table_length);
	if (status != 0) {
		goto exit;
	}

	manifest->base.verify = key_manifest_rsa_cert_block_verify;
	manifest->get_hash = key_manifest_rsa_cert_block_get_hash;
	manifest->get_root_keys = key_manifest_rsa_cert_block_get_root_keys;

	manifest->flash = flash;
	manifest->address = base_addr;

	status = key_manifest_rsa_cert_block_init_keys (manifest, hw, x509, rsa, &header, cert_table);

exit:
	platform_free (cert_table);

	return status;
}

/**
 * Initialize a key manifest using an X.509 certificate chain.  The manifest is stored in memory
 * which must remain valid for the entire lifetime of this interface.
 *
 * @param manifest The key manifest to initialize.
 * @param hw Interface to device hardware for certificate management.
 * @param cert The beginning of the certificate block stored in memory.
 * @param x509 The X.509 engine to use for certificate operations.
 * @param rsa The RSA engine to use for key operations.
 *
 * @return 0 if the key manifest was successfully initialized or an error code.
 */
int key_manifest_rsa_cert_block_init_from_memory (struct key_manifest_rsa_cert_block *manifest,
	struct cert_device_hw *hw, const uint8_t *cert, const struct x509_engine *x509,
	const struct rsa_engine *rsa)
{
	const struct key_manifest_rsa_cert_block_header *header;
	int status;

	if ((manifest == NULL) || (hw == NULL) || (cert == NULL) || (x509 == NULL) || (rsa == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	memset (manifest, 0, sizeof (struct key_manifest_rsa_cert_block));

	header = (const struct key_manifest_rsa_cert_block_header*) cert;
	status = key_manifest_rsa_cert_block_init_check_header (header);
	if (status != 0) {
		return status;
	}

	manifest->base.verify = key_manifest_rsa_cert_block_memory_verify;
	manifest->get_hash = key_manifest_rsa_cert_block_memory_get_hash;
	manifest->get_root_keys = key_manifest_rsa_cert_block_memory_get_root_keys;

	manifest->address = (uintptr_t) cert;

	return key_manifest_rsa_cert_block_init_keys (manifest, hw, x509, rsa, header,
		cert + sizeof (struct key_manifest_rsa_cert_block_header));
}

/**
 * Release the resources used for the key manifest.
 *
 * @param manifest The manifest to release.
 */
void key_manifest_rsa_cert_block_release (struct key_manifest_rsa_cert_block *manifest)
{

}

/**
 * Get the length of the entire certificate block.
 *
 * @param manifest The manifest to utilize.
 * @param length Output length of the certificate block
 *
 * @return 0 if the certificate block length is retrieved successfully or an error code.
 */
int key_manifest_rsa_cert_block_get_length (const struct key_manifest_rsa_cert_block *manifest,
	size_t *length)
{
	if ((manifest == NULL) || (length == NULL)) {
		return KEY_MANIFEST_INVALID_ARGUMENT;
	}

	*length = sizeof (struct key_manifest_rsa_cert_block_header) + manifest->table_length +
		sizeof (struct key_manifest_rsa_cert_block_root_keys);

	return 0;
}
