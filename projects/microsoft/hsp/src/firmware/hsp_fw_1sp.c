// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_fw_1sp.h"
#include "platform_api.h"
#include "common/buffer_util.h"
#include "common/common_math.h"
#include "common/unused.h"
#include "crypto/ecc.h"
#include "firmware/hsp_fw_util.h"
#include "flash/flash_util.h"


/**
 * Verify that the 1SP image header is structure correctly.
 *
 * @param fw The image context to check.
 *
 * @return 0 if the 1SP image header is formatted correctly or an error code.
 */
static int hsp_fw_1sp_check_image_header (const struct hsp_fw_1sp *fw)
{
	if (fw->state->header.marker != HSP_FW_1SP_MARKER) {
		return HSP_FW_1SP_BAD_IMAGE_MARKER;
	}

	if (fw->state->header.header_signed.length == 0) {
		return HSP_FW_1SP_IMAGE_NO_DATA;
	}

	if (fw->state->header.header_signed.length & 0xf) {
		return HSP_FW_1SP_IMAGE_NOT_BLOCK_ALIGNED;
	}

	if (fw->loader) {
		return fw->loader->is_address_valid (fw->loader, fw->state->header.header_signed.load_addr,
			fw->state->header.header_signed.length);
	}
	else {
		return 0;
	}
}

/**
 * Initialize the handler for a 1SP firmware image stored on flash.  Load the 1SP firmware image
 * header from flash and parse it for proper structure.  The header will not be validated.
 *
 * @param fw An image context that will be initialized.
 * @param state Variable context for the 1SP image handler.
 * @param loader Handler for loading the image into SP memory.  If this is null, loading the image
 * for execution will not be supported.
 * @param flash The flash device that contains the image.
 * @param base_addr Base address on flash where the image is stored.
 *
 * @return 0 if the 1SP header was loaded successfully or an error code.
 */
int hsp_fw_1sp_init (struct hsp_fw_1sp *fw, struct hsp_fw_1sp_state *state,
	const struct firmware_loader *loader, const struct flash *flash, uint32_t base_addr)
{
	int status;

	status = hsp_fw_1sp_init_api (fw, state, loader, flash);
	if (status != 0) {
		return status;
	}

	return hsp_fw_1sp_init_state (fw, NULL, base_addr);
}

/**
 * Initialize the handler for a 1SP firmware image stored in memory.  Copy the 1SP firmware image
 * header from memory and parse it for proper structure.  The header will not be validated.
 *
 * @param fw An image context that will be initialized.
 * @param state Variable context for the 1SP image handler.
 * @param loader Handler for loading the image into SP memory.  If this is null, loading the image
 * for execution will not be supported.
 * @param base_addr Base address in memory where the image is stored.
 *
 * @return 0 if the 1SP header was loaded successfully or an error code.
 */
int hsp_fw_1sp_init_from_memory (struct hsp_fw_1sp *fw, struct hsp_fw_1sp_state *state,
	const struct firmware_loader *loader, const uint8_t *base_addr)
{
	int status;

	status = hsp_fw_1sp_init_api (fw, state, loader, NULL);
	if (status != 0) {
		return status;
	}

	return hsp_fw_1sp_init_state_from_memory (fw, base_addr);
}

/**
 * Initialize the static contents for handling a 1SP firmware image.  No image information will be
 * read and the variable context will remain uninitialized.  The result of this call is the same as
 * static initialization, except there is some parameter validation done.
 *
 * A 1SP instance that has only had the API initialized does not need to be released.
 *
 * @param fw An image context that will be initialized.
 * @param state Variable context for the 1SP image handler.
 * @param loader Handler for loading the image into SP memory.  If this is null, loading the image
 * for execution will not be supported.
 * @param flash The flash device that contains the image.  This can be null if the image will not be
 * loaded from flash.
 *
 * @return 0 if the 1SP handler was initialized successfully or HSP_FW_1SP_INVALID_ARGUMENT if a
 * parameter is null.
 */
int hsp_fw_1sp_init_api (struct hsp_fw_1sp *fw, struct hsp_fw_1sp_state *state,
	const struct firmware_loader *loader, const struct flash *flash)
{
	if ((fw == NULL) || (state == NULL)) {
		return HSP_FW_1SP_INVALID_ARGUMENT;
	}

	memset (fw, 0, sizeof (struct hsp_fw_1sp));

	fw->state = state;
	fw->loader = loader;
	fw->flash = flash;

	return 0;
}

/**
 * Initialize only the variable state for a 1SP firmware image stored on flash.  Load the 1SP
 * firmware image header from flash and parse it for proper structure.  The header will not be
 * validated.
 *
 * This would generally be used with a statically initialized image instance, but it can also be
 * used to reinitialize an instance that was previously released with hsp_fw_1sp_release to load new
 * image data.
 *
 * @param fw The image that contains the state to initialize.
 * @param flash An optional flash device to use for loading the header data.  If this is not
 * provided, the flash provided during initialization for loading the image data will be used.
 * @param base_addr Base address on flash where the image is stored.
 *
 * @return 0 if the 1SP header was loaded successfully or an error code.
 */
int hsp_fw_1sp_init_state (const struct hsp_fw_1sp *fw, const struct flash *flash,
	uint32_t base_addr)
{
	int status;

	if ((fw == NULL) || (fw->state == NULL) || (fw->flash == NULL)) {
		return HSP_FW_1SP_INVALID_ARGUMENT;
	}

	memset (fw->state, 0, sizeof (struct hsp_fw_1sp_state));

	/* TODO: This actually doesn't work the way it's supposed to.  It only allows reading the header
	 * without providing flash at the beginning.  This flash argument can't be persisted because the
	 * 1SP instance is constant.  Just remove this parameter as an option and force the flash to be
	 * provided only during init.
	 *
	 * Alternative, take a flash argument to the verify_full_image function and load_image functions
	 * too.  Might start to get too complicated at that point, though, and not clear of the benefit
	 * it would provide. */
	if (flash == NULL) {
		flash = fw->flash;
	}

	status = flash->read (flash, base_addr, (uint8_t*) &fw->state->header,
		sizeof (fw->state->header));
	if (status != 0) {
		return status;
	}

	fw->state->base_addr = base_addr;
	fw->state->on_flash = true;

	return hsp_fw_1sp_check_image_header (fw);
}

/**
 * Initialize only the variable state for a 1SP firmware image stored in memory.  Copy the 1SP
 * firmware image header from memory and parse it for proper structure.  The header will not be
 * validated.
 *
 * This would generally be used with a statically initialized image instance, but it can also be
 * used to reinitialize an instance that was previously released with hsp_fw_1sp_release to load new
 * image data.
 *
 * @param fw The image that contains the state to initialize.
 * @param base_addr Base address in memory where the image is stored.
 *
 * @return 0 if the 1SP header was loaded successfully or an error code.
 */
int hsp_fw_1sp_init_state_from_memory (const struct hsp_fw_1sp *fw, const uint8_t *base_addr)
{
	if ((fw == NULL) || (base_addr == NULL) || (fw->state == NULL)) {
		return HSP_FW_1SP_INVALID_ARGUMENT;
	}

	memset (fw->state, 0, sizeof (struct hsp_fw_1sp_state));

	memcpy (&fw->state->header, base_addr, sizeof (fw->state->header));
	fw->state->base_addr = (uintptr_t) base_addr;

	return hsp_fw_1sp_check_image_header (fw);
}

/**
 * Release the resources used for a 1SP image handler.
 *
 * @param fw The image context to release.
 */
void hsp_fw_1sp_release (const struct hsp_fw_1sp *fw)
{
	UNUSED (fw);
}

/**
 * Get the total length of the wrapped 1SP image data.  This represents the amount of space the
 * image consumes in storage.
 *
 * @param fw The 1SP image to query.
 *
 * @return The total size of the 1SP firmware image, including all headers and firmware data.
 */
size_t hsp_fw_1sp_get_total_length (const struct hsp_fw_1sp *fw)
{
	if (fw) {
		return sizeof (fw->state->header) + fw->state->header.header_signed.length;
	}
	else {
		return 0;
	}
}

/**
 * Get the total length of the wrapped 1SP image data stored on flash.  No firmware image instance
 * is required, so no data will be persisted in memory.  The image will not be validated, but the
 * header will be structurally verified.
 *
 * @param flash The flash that contains the 1SP image.
 * @param base_addr Address of the 1SP image data.
 *
 * @return Length of the 1SP firmware image on flash or an error code.
 */
int hsp_fw_1sp_get_size_on_flash (const struct flash *flash, uint32_t base_addr)
{
	struct hsp_fw_1sp_state state;
	struct hsp_fw_1sp fw;
	int status;

	status = hsp_fw_1sp_init (&fw, &state, NULL, flash, base_addr);
	if (status != 0) {
		return status;
	}

	status = hsp_fw_1sp_get_total_length (&fw);
	hsp_fw_1sp_release (&fw);

	return status;
}

/**
 * Verify the signed header on the 1SP firmware image.
 *
 * @param fw The 1SP image header to verify.
 * @param hash A hash engine to use for header verification.
 * @param pka The HSP PKA engine to use to check the header signature.
 * @param fw_key Public key to use for signature verification.
 * @param secondary_key Optional secondary public key to use for signature verification.  Set this
 * to null if there is no secondary public key.  If a secondary key is provided, both signatures
 * must be valid to pass verification.
 * @param expected_svn The svn value that should be reported by the header.
 *
 * @return 0 if the signed header is valid or an error code.
 */
int hsp_fw_1sp_verify_signed_header (const struct hsp_fw_1sp *fw, const struct hash_engine *hash,
	const struct ecc_hw *pka, const struct ecc_point_public_key *fw_key,
	const struct ecc_point_public_key *secondary_key, uint32_t expected_svn)
{
	int status;

	if ((fw == NULL) || (hash == NULL) || (pka == NULL) || (fw_key == NULL)) {
		return HSP_FW_1SP_INVALID_ARGUMENT;
	}

	/* If the header SVN doesn't match our expected value, there is no need to check anything
	 * else. */
	if (fw->state->header.header_signed.svn != expected_svn) {
		return HSP_FW_1SP_SVN_MISMATCH;
	}

	status = hsp_fw_verify_signed_image (pka, hash, (uint8_t*) &fw->state->header.header_signed,
		sizeof (fw->state->header.header_signed), &fw->state->header.signature, fw_key, NULL, 0,
		NULL, -1);

	if ((status == 0) && secondary_key) {
		status = hsp_fw_verify_signed_image (pka, hash, (uint8_t*) &fw->state->header.header_signed,
			sizeof (fw->state->header.header_signed), &fw->state->header.secondary_signature,
			secondary_key, NULL, 0, NULL, -1);
	}

	return status;
}

/**
 * Verify the 1SP firmware image while it is not yet in target memory.  The signed header will first
 * be validated, followed by the image binary.  This verification can be run regardless of where the
 * image is stored (e.g. flash vs. staging memory).  In all cases, the image will not be permanently
 * loaded into memory for execution.
 *
 * This verification is only possible on plaintext images.
 *
 * @param fw The 1SP image header to verify.
 * @param hash A hash engine to use for header verification.
 * @param pka The HSP PKA engine to use to check the header signature.
 * @param fw_key Public key to use for signature verification.
 * @param secondary_key Optional secondary public key to use for signature verification.  Set this
 * to null if there is no secondary public key.  If a secondary key is provided, both signatures
 * must be valid to pass verification.
 * @param expected_svn The svn value that should be reported by the header.
 *
 * @return 0 if the image on flash is valid or an error code.
 */
int hsp_fw_1sp_verify_full_image (const struct hsp_fw_1sp *fw, const struct hash_engine *hash,
	const struct ecc_hw *pka, const struct ecc_point_public_key *fw_key,
	const struct ecc_point_public_key *secondary_key, uint32_t expected_svn)
{
	SP_MSG_384 digest;
	int status;

	if ((fw == NULL) || (hash == NULL) || (pka == NULL) || (fw_key == NULL)) {
		return HSP_FW_1SP_INVALID_ARGUMENT;
	}

	if (fw->state->header.flags & HSP_FW_1SP_ENCRYPTED_FLAG) {
		return HSP_FW_1SP_IMAGE_ENCRYPTION_NOT_SUPPORTED;
	}

	status = hsp_fw_1sp_verify_signed_header (fw, hash, pka, fw_key, secondary_key, expected_svn);
	if (status != 0) {
		return status;
	}

	if (fw->state->on_flash) {
		status = flash_hash_contents (fw->flash, fw->state->base_addr + sizeof (fw->state->header),
			fw->state->header.header_signed.length, hash, HASH_TYPE_SHA384, digest.AsBytes,
			SP_MSG_384_SIZE);
	}
	else {
		status = hash->calculate_sha384 (hash,
			(uint8_t*) (fw->state->base_addr + sizeof (fw->state->header)),
			fw->state->header.header_signed.length, digest.AsBytes, SP_MSG_384_SIZE);
	}
	if (status != 0) {
		return status;
	}

	status = buffer_compare (digest.AsBytes, fw->state->header.header_signed.digest.AsBytes,
		SP_MSG_384_SIZE);
	if (status != 0) {
		return HSP_FW_1SP_IMAGE_VERIFY_FAILED;
	}

	return 0;
}

/**
 * Load the 1SP firmware image from flash into memory and calculate the digest of the loaded data.
 *
 * @param fw The 1SP image to load.
 * @param hash A hash engine to use for image verification.
 * @param digest Output for the image digest.
 *
 * @return 0 if the image was successfully loaded or an error code.
 */
static int hsp_fw_1sp_load_image_from_flash (const struct hsp_fw_1sp *fw,
	const struct hash_engine *hash, SP_MSG_384 *digest)
{
	int status;

	/* If the image is encrypted, pass the IV from the header to the loader to indicate that image
	 * needs to be decrypted. */
	if (fw->state->header.flags & HSP_FW_1SP_ENCRYPTED_FLAG) {
		status = fw->loader->load_image (fw->loader, fw->flash,
			fw->state->base_addr + sizeof (fw->state->header),
			fw->state->header.header_signed.length,
			(uint8_t*) ((uintptr_t) fw->state->header.header_signed.load_addr),
			fw->state->header.iv, sizeof (fw->state->header.iv), hash, HASH_TYPE_SHA384,
			digest->AsBytes, SP_MSG_384_SIZE);
	}
	else {
		status = fw->loader->load_image (fw->loader, fw->flash,
			fw->state->base_addr + sizeof (fw->state->header),
			fw->state->header.header_signed.length,
			(uint8_t*) ((uintptr_t) fw->state->header.header_signed.load_addr), NULL, 0, hash,
			HASH_TYPE_SHA384, digest->AsBytes, SP_MSG_384_SIZE);
	}

	return status;
}

/**
 * Copy the 1SP firmware image from memory and calculate the digest of the copied data.
 *
 * @param fw The 1SP image to load.
 * @param hash A hash engine to use for image verification.
 * @param digest Output for the image digest.
 *
 * @return 0 if the image was successfully loaded or an error code.
 */
static int hsp_fw_1sp_copy_image_from_memory (const struct hsp_fw_1sp *fw,
	const struct hash_engine *hash, SP_MSG_384 *digest)
{
	int status = 0;

	/* If the image is encrypted, pass the IV from the header to the loader to indicate that image
	 * needs to be decrypted. */
	if (fw->state->header.flags & HSP_FW_1SP_ENCRYPTED_FLAG) {
		status = fw->loader->copy_image (fw->loader,
			(uint8_t*) (fw->state->base_addr + sizeof (fw->state->header)),
			fw->state->header.header_signed.length,
			(uint8_t*) ((uintptr_t) fw->state->header.header_signed.load_addr),
			fw->state->header.iv, sizeof (fw->state->header.iv), hash, HASH_TYPE_SHA384,
			digest->AsBytes, SP_MSG_384_SIZE);
	}
	else {
		status = fw->loader->copy_image (fw->loader,
			(uint8_t*) (fw->state->base_addr + sizeof (fw->state->header)),
			fw->state->header.header_signed.length,
			(uint8_t*) ((uintptr_t) fw->state->header.header_signed.load_addr), NULL, 0, hash,
			HASH_TYPE_SHA384, digest->AsBytes, SP_MSG_384_SIZE);
	}

	return status;
}

/**
 * Load the 1SP firmware image into memory for execution and verify it against the digest in the
 * header.  The header will not be validated as part of this operation.  The header must have
 * previously been verified as a separate action.
 *
 * @param fw The 1SP image to load.
 * @param hash A hash engine to use for image verification.
 *
 * @return 0 if the image was successfully loaded and validated or an error code.
 */
int hsp_fw_1sp_load_image (const struct hsp_fw_1sp *fw, const struct hash_engine *hash)
{
	SP_MSG_384 digest;
	int status;

	if ((fw == NULL) || (hash == NULL)) {
		return HSP_FW_1SP_INVALID_ARGUMENT;
	}

	if (fw->loader == NULL) {
		return HSP_FW_1SP_LOAD_IMAGE_NOT_SUPPORTED;
	}

	if (fw->state->on_flash) {
		status = hsp_fw_1sp_load_image_from_flash (fw, hash, &digest);
	}
	else {
		status = hsp_fw_1sp_copy_image_from_memory (fw, hash, &digest);
	}
	if (status != 0) {
		return status;
	}

	status = buffer_compare (digest.AsBytes, fw->state->header.header_signed.digest.AsBytes,
		SP_MSG_384_SIZE);
	if (status != 0) {
		return HSP_FW_1SP_IMAGE_VERIFY_FAILED;
	}

	return 0;
}

/**
 * Encrypt the 1SP firmware image that is stored in flash.
 *
 * @param fw The 1SP image to encrypt.
 * @param aes Interface to the AES driver to use for encryption.
 * @param key KSU key index to use for encryption.
 * @param iv The IV to use when encrypting the image.
 *
 * @return 0 if the image was successfully encrypted or an error code.
 */
static int hsp_fw_1sp_encrypt_image_in_flash (const struct hsp_fw_1sp *fw,
	const struct hsp_aes *aes, uint8_t key, const SP_MSG_128 *iv)
{
	uint32_t bytes;
	uint32_t offset;
	uint32_t addr;
	uint8_t *sector;
	struct hsp_fw_1sp_header *header;
	size_t hdr_length;
	size_t fw_length;
	size_t enc_length;
	size_t cache_length;
	SP_MSG_128 cache_block;
	SP_MSG_128 next_iv;
	int status;

	status = fw->flash->get_sector_size (fw->flash, &bytes);
	if (status != 0) {
		return status;
	}

	offset = FLASH_REGION_OFFSET (fw->state->base_addr, bytes);
	addr = fw->state->base_addr - offset;
	hdr_length = sizeof (fw->state->header);
	fw_length = fw->state->header.header_signed.length;
	cache_length = 0;

	/* Regardless of the sector size, make sure we can always deal with the header in a single
	 * operation, which greatly simplifies image handling.  It comes at the expense of extra memory
	 * being allocated, but this would really only happen in scenarios where the flash sector size
	 * is small. */
	while (bytes < (offset + hdr_length)) {
		bytes <<= 1;
	}

	sector = platform_malloc (bytes);
	if (sector == NULL) {
		return HSP_FW_1SP_NO_MEMORY;
	}

	while (fw_length != 0) {
		status = fw->flash->read (fw->flash, addr, sector, bytes);
		if (status != 0) {
			goto exit;
		}

		/* For the first sector, include the image header and update it to indicate an encrypted
		 * image.  Use the cached header for the image, since that is what was used for any previous
		 * validity checks or validation.*/
		if (hdr_length) {
			header = (struct hsp_fw_1sp_header*) &sector[offset];
			memcpy (header, &fw->state->header, hdr_length);

			memcpy (header->iv, iv->AsBytes, sizeof (header->iv));
			header->flags |= HSP_FW_1SP_ENCRYPTED_FLAG;
		}

		/* If there is any cached data from the previous sector, deal with that before the rest of
		 * the sector data. */
		if (cache_length) {
			/* Complete the partial AES block and encrypt it. */
			offset = SP_MSG_128_SIZE - cache_length;
			memcpy (&cache_block.AsBytes[cache_length], sector, offset);

			status = aes->encrypt (aes, HSP_AES_MODE_CBC, key, iv, cache_block.AsBytes,
				SP_MSG_128_SIZE, cache_block.AsBytes, SP_MSG_128_SIZE, &next_iv);
			if (status != 0) {
				goto exit;
			}

			/* Only the data from the previous sector data can be written now, since that sector was
			 * previously erased.  The current sector data needs to updated with the partial block
			 * of encrypted data, which will get written to flash with the rest of the sector
			 * data once that is ready. */
			status = flash_write_and_verify (fw->flash, addr - cache_length, cache_block.AsBytes,
				cache_length);
			if (status != 0) {
				goto exit;
			}

			memcpy (sector, &cache_block.AsBytes[cache_length], offset);

			fw_length -= SP_MSG_128_SIZE;
			cache_length = 0;
		}

		enc_length = min (fw_length, (bytes - offset - hdr_length));
		cache_length = enc_length & (SP_MSG_128_SIZE - 1);

		if (cache_length != 0) {
			/* The encryption required for this sector is not aligned to the AES block length.  The
			 * unaligned data needs to be cached until the next sector data is read. */
			memcpy (cache_block.AsBytes, &sector[bytes - cache_length], cache_length);
			enc_length -= cache_length;
		}

		status = aes->encrypt (aes, HSP_AES_MODE_CBC, key, iv, &sector[offset + hdr_length],
			enc_length, &sector[offset + hdr_length], enc_length, &next_iv);
		if (status != 0) {
			goto exit;
		}

		status = flash_sector_program_and_verify (fw->flash, addr, sector, bytes - cache_length);
		if (status != 0) {
			goto exit;
		}

		/* Update the cached header data once it has been committed to flash. */
		if (hdr_length) {
			memcpy (&fw->state->header, header, hdr_length);
		}

		addr += bytes;
		fw_length -= enc_length;
		offset = 0;
		hdr_length = 0;
		iv = &next_iv;
	}

exit:
	platform_free (sector);

	return status;
}

/**
 * Encrypt the 1SP firmware image that is stored in memory.
 *
 * @param fw The 1SP image to encrypt.
 * @param aes Interface to the AES driver to use for encryption.
 * @param key KSU key index to use for encryption.
 * @param iv The IV to use when encrypting the image.
 *
 * @return 0 if the image was successfully encrypted or an error code.
 */
static int hsp_fw_1sp_encrypt_image_in_memory (const struct hsp_fw_1sp *fw,
	const struct hsp_aes *aes, uint8_t key, const SP_MSG_128 *iv)
{
	struct hsp_fw_1sp_header *header = (struct hsp_fw_1sp_header*) fw->state->base_addr;
	uint8_t *img_data = (uint8_t*) (fw->state->base_addr + sizeof (fw->state->header));
	int status;

	status = aes->encrypt (aes, HSP_AES_MODE_CBC, key, iv, img_data,
		fw->state->header.header_signed.length, img_data, fw->state->header.header_signed.length,
		NULL);
	if (status != 0) {
		return status;
	}

	/* Update both the cached header and the header in memory with the encrypted state. */
	memcpy (fw->state->header.iv, iv->AsBytes, sizeof (fw->state->header.iv));
	fw->state->header.flags |= HSP_FW_1SP_ENCRYPTED_FLAG;

	memcpy (header, &fw->state->header, sizeof (fw->state->header));

	return 0;
}

/**
 * Encrypt the 1SP firmware image.  The original image data will be replaced with an encrypted
 * version.  If the operation fails or is interrupted, the original image will no longer be valid.
 *
 * No validation is done on the image or header as part of this operation.
 *
 * @param fw The 1SP image to encrypt.
 * @param aes Interface to the AES driver to use for encryption.
 * @param key KSU key index to use for encryption.
 * @param iv The IV to use when encrypting the image.
 *
 * @return 0 if the image was successfully encrypted or an error code.
 */
int hsp_fw_1sp_encrypt_image (const struct hsp_fw_1sp *fw, const struct hsp_aes *aes, uint8_t key,
	const SP_MSG_128 *iv)
{
	int status;

	if ((fw == NULL) || (aes == NULL) || (iv == NULL)) {
		return HSP_FW_1SP_INVALID_ARGUMENT;
	}

	if (fw->state->on_flash) {
		status = hsp_fw_1sp_encrypt_image_in_flash (fw, aes, key, iv);
	}
	else {
		status = hsp_fw_1sp_encrypt_image_in_memory (fw, aes, key, iv);
	}

	return status;
}

/**
 * Verify the 1SP image against the digest in the signed header.  Nothing new will be loaded into
 * memory and the signed header will not be verified.
 *
 * @param fw The 1SP to verify.
 * @param hash Hash engine to use for verification.
 *
 * @return 0 if the image in memory is valid or an error code.
 */
int hsp_fw_1sp_verify_image_in_memory (const struct hsp_fw_1sp *fw, const struct hash_engine *hash)
{
	SP_MSG_384 digest = {0};
	int status = 0;

	if ((fw == NULL) || (hash == NULL)) {
		return HSP_FW_1SP_INVALID_ARGUMENT;
	}

	status = hash->calculate_sha384 (hash,
		(uint8_t*) (uintptr_t) fw->state->header.header_signed.load_addr,
		fw->state->header.header_signed.length, digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto exit;
	}

	if (buffer_compare (fw->state->header.header_signed.digest.AsBytes, digest.AsBytes,
		SP_MSG_384_SIZE) != 0) {
		status = HSP_FW_1SP_IMAGE_VERIFY_FAILED;
	}

exit:
	buffer_zeroize (digest.AsBytes, SP_MSG_384_SIZE);

	return status;
}
