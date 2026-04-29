// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "firmware_loader_hsp_memory.h"
#include "common/sram_util.h"
#include "common/unused.h"


/**
 * Check an address range to see if it fits within a known block of SRAM.
 *
 * @param hsp The HSP loader containing the SRAM blocks to check against.
 * @param address The starting address of the range to check.
 * @param length The size of the range that needs to fit in an SRAM block.
 *
 * @return 0 if the address range fits in an SRAM block or an error code.
 */
static int firmware_loader_hsp_memory_check_address_range (
	const struct firmware_loader_hsp_memory *hsp, void *address, size_t length)
{
	size_t i;

	for (i = 0; i < hsp->blocks; i++) {
		int status = sram_data_fits_in_block (&hsp->sram[i], address, length);

		if (status == 0) {
			return 0;
		}
		else if (status > 0) {
			return FIRMWARE_LOADER_IMAGE_TOO_LARGE;
		}
	}

	return FIRMWARE_LOADER_INVALID_ADDR;
}

int firmware_loader_hsp_memory_is_address_valid (const struct firmware_loader *loader,
	uint64_t address, size_t length)
{
	const struct firmware_loader_hsp_memory *hsp =
		(const struct firmware_loader_hsp_memory*) loader;

	if (hsp == NULL) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	/* HSP is a 32-bit address space.  Anything outside of that is immediately invalid. */
	if ((address > 0xffffffffULL) || (address & 0x3U)) {
		return FIRMWARE_LOADER_INVALID_ADDR;
	}

	return firmware_loader_hsp_memory_check_address_range (hsp, (void*) (uintptr_t) address,
		length);
}

int firmware_loader_hsp_memory_map_address (const struct firmware_loader *loader, uint64_t phy_addr,
	size_t length, void **virt_addr)
{
	UNUSED (length);

	if ((loader == NULL) || (virt_addr == NULL)) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	if (phy_addr > 0xffffffffULL) {
		return FIRMWARE_LOADER_INVALID_ADDR;
	}

	/* No special mapping or bounds checking.  As long as the address fits in a 32-bit pointer, just
	 * return that value. */
	*virt_addr = (void*) ((uintptr_t) phy_addr);

	return 0;
}

void firmware_loader_hsp_memory_unmap_address (const struct firmware_loader *loader,
	void *virt_addr)
{
	UNUSED (loader);
	UNUSED (virt_addr);

	/* Nothing to do here. */
}

/**
 * Load an image to the target memory, decrypt the contents, and generate a digest of the loaded
 * data.  The image can be stored either in flash or another region of memory.
 *
 * @param hsp The handler to use for image loading.
 * @param flash Flash where the firmware image should be loaded from.  If this is null, the source
 * address represents to a pointer in memory where the image should be copied from.
 * @param src_addr Address on where the image is stored.
 * @param length Total length of the image to load.
 * @param dest_addr Address in memory where the image will be loaded to.
 * @param iv The IV to use for image decryption.  Null if the image is not encrypted.
 * @param iv_length Length of the encryption IV.
 * @param hash Hash engine to use for calculating the digest.
 * @param hash_algo The hash algorithm to use.  Ignored if the digest output is null.
 * @param digest Output for the image digest.  If this null, only update the current hash context
 * @param digest_length Length of the output buffer.  Ignored if the digest output is null.
 *
 * @return 0 if the image was loaded into memory successfully or an error code.
 */
static int firmware_loader_hsp_memory_load_to_target (const struct firmware_loader_hsp_memory *hsp,
	const struct flash *flash, uintptr_t src_addr, size_t length, uint8_t *dest_addr,
	const uint8_t *iv, size_t iv_length, const struct hash_engine *hash, enum hash_type hash_algo,
	uint8_t *digest, size_t digest_length)
{
	int status;

	if ((hsp == NULL) || (dest_addr == NULL)) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	if (iv && (iv_length != SP_MSG_128_SIZE)) {
		return FIRMWARE_LOADER_BAD_IV_LENGTH;
	}

	status = firmware_loader_hsp_memory_check_address_range (hsp, dest_addr, length);
	if (status != 0) {
		return status;
	}

	if (flash) {
		status = flash->read (flash, src_addr, dest_addr, length);
		if (status != 0) {
			return status;
		}
	}
	else {
		memmove (dest_addr, (uint8_t*) src_addr, length);
	}

	if (iv) {
		status = hsp->aes->decrypt (hsp->aes, HSP_AES_MODE_CBC, hsp->key_slot,
			(const SP_MSG_128*) iv, dest_addr, length, dest_addr, length, NULL);
		if (status != 0) {
			return status;
		}
	}

	if (hash) {
		if (digest) {
			status = hash_calculate (hash, hash_algo, dest_addr, length, digest, digest_length);
		}
		else {
			status = hash->update (hash, dest_addr, length);
		}
		if (ROT_IS_ERROR (status)) {
			return status;
		}
	}

	return 0;
}

int firmware_loader_hsp_memory_load_image (const struct firmware_loader *loader,
	const struct flash *flash, uint32_t src_addr, size_t length, uint8_t *dest_addr,
	const uint8_t *iv, size_t iv_length, const struct hash_engine *hash, enum hash_type hash_algo,
	uint8_t *digest, size_t digest_length)
{
	const struct firmware_loader_hsp_memory *hsp =
		(const struct firmware_loader_hsp_memory*) loader;

	if ((flash == NULL) || ((hash != NULL) && (digest == NULL))) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	return firmware_loader_hsp_memory_load_to_target (hsp, flash, src_addr, length, dest_addr, iv,
		iv_length, hash, hash_algo, digest, digest_length);
}

int firmware_loader_hsp_memory_load_image_update_digest (const struct firmware_loader *loader,
	const struct flash *flash, uint32_t src_addr, size_t length, uint8_t *dest_addr,
	const uint8_t *iv, size_t iv_length, const struct hash_engine *hash)
{
	const struct firmware_loader_hsp_memory *hsp =
		(const struct firmware_loader_hsp_memory*) loader;

	if ((flash == NULL) || (hash == NULL)) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	return firmware_loader_hsp_memory_load_to_target (hsp, flash, src_addr, length, dest_addr, iv,
		iv_length, hash, HASH_TYPE_SHA384, NULL, 0);
}

int firmware_loader_hsp_memory_copy_image (const struct firmware_loader *loader,
	const uint8_t *src_addr, size_t length, uint8_t *dest_addr, const uint8_t *iv, size_t iv_length,
	const struct hash_engine *hash, enum hash_type hash_algo, uint8_t *digest, size_t digest_length)
{
	const struct firmware_loader_hsp_memory *hsp =
		(const struct firmware_loader_hsp_memory*) loader;

	if ((src_addr == NULL) || ((hash != NULL) && (digest == NULL))) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	return firmware_loader_hsp_memory_load_to_target (hsp, NULL, (uintptr_t) src_addr, length,
		dest_addr, iv, iv_length, hash, hash_algo, digest, digest_length);
}

int firmware_loader_hsp_memory_copy_image_update_digest (const struct firmware_loader *loader,
	const uint8_t *src_addr, size_t length, uint8_t *dest_addr, const uint8_t *iv, size_t iv_length,
	const struct hash_engine *hash)
{
	const struct firmware_loader_hsp_memory *hsp =
		(const struct firmware_loader_hsp_memory*) loader;

	if ((src_addr == NULL) || (hash == NULL)) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	return firmware_loader_hsp_memory_load_to_target (hsp, NULL, (uintptr_t) src_addr, length,
		dest_addr, iv, iv_length, hash, HASH_TYPE_SHA384, NULL, 0);
}

/**
 * Initialize a handler for loading firmware images into memory locations in the HSP address space.
 *
 * @param loader Tho handler to initialize.
 * @param sram A list of address ranges than can be used to load the firmware image.  When
 * checking to see if an image can be loaded, each address range is inspected individually, even if
 * multiple blocks represent a single contiguous memory region.  This allows a single range of
 * addresses to be viewed as different blocks (e.g. IRAM vs. DRAM) from a firmware load perspective.
 * Firmware will not be allowed to load if it crosses between address ranges.
 * @param blocks The total number of address ranges that can be used for firmware.
 * @param aes The AES HW engine to use for decrypting encrypted images.
 * @param key_slot Index for the key slot to use for decryption.
 *
 * @return 0 if the image loader was initialized successfully or an error code.
 */
int firmware_loader_hsp_memory_init (struct firmware_loader_hsp_memory *loader,
	const struct sram_block *sram, size_t blocks, const struct hsp_aes *aes, uint8_t key_slot)
{
	if ((loader == NULL) || (sram == NULL) || (blocks == 0) || (aes == NULL)) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	memset (loader, 0, sizeof (struct firmware_loader_hsp_memory));

	loader->base.is_address_valid = firmware_loader_hsp_memory_is_address_valid;
	loader->base.map_address = firmware_loader_hsp_memory_map_address;
	loader->base.unmap_address = firmware_loader_hsp_memory_unmap_address;
	loader->base.load_image = firmware_loader_hsp_memory_load_image;
	loader->base.load_image_update_digest = firmware_loader_hsp_memory_load_image_update_digest;
	loader->base.copy_image = firmware_loader_hsp_memory_copy_image;
	loader->base.copy_image_update_digest = firmware_loader_hsp_memory_copy_image_update_digest;

	loader->sram = sram;
	loader->blocks = blocks;
	loader->aes = aes;
	loader->key_slot = key_slot;

	return 0;
}

/**
 * Release the resources used by an firmware loader for HSP memory.
 *
 * @param loader The firmware loader to release.
 */
void firmware_loader_hsp_memory_release (const struct firmware_loader_hsp_memory *loader)
{
	UNUSED (loader);
}
