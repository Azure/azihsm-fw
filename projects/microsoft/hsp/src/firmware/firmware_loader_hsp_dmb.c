// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "firmware_loader_hsp_dmb.h"
#include "common/sram_util.h"
#include "common/unused.h"


int firmware_loader_hsp_dmb_is_address_valid (const struct firmware_loader *loader,
	uint64_t address, size_t length)
{
	const struct firmware_loader_hsp_dmb *hsp = (const struct firmware_loader_hsp_dmb*) loader;
	size_t i;

	if (hsp == NULL) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	for (i = 0; i < hsp->blocks; i++) {
		int status = sram_data_fits_in_soc_block (&hsp->sram[i], address, length);

		if (status == 0) {
			return 0;
		}
		else if (status > 0) {
			return FIRMWARE_LOADER_IMAGE_TOO_LARGE;
		}
	}

	return FIRMWARE_LOADER_INVALID_ADDR;
}

int firmware_loader_hsp_dmb_map_address (const struct firmware_loader *loader, uint64_t phy_addr,
	size_t length, void **virt_addr)
{
	const struct firmware_loader_hsp_dmb *hsp = (const struct firmware_loader_hsp_dmb*) loader;
	int status;

	if ((hsp == NULL) || (length == 0) || (virt_addr == NULL)) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	status = firmware_loader_hsp_dmb_is_address_valid (loader, phy_addr, length);
	if (status != 0) {
		return status;
	}

	if (hsp->state->mapped.length != 0) {
		/* There is already a region mapped, so unmap it.  This implementation can only support one
		 * mapped region at a time. */
		hsp->dmb->unmap_soc_address (hsp->dmb, (void*) hsp->state->mapped.start);
	}

	status = hsp->dmb->map_soc_address (hsp->dmb, phy_addr, length, HSP_DMB_ACCESS_WRITE,
		virt_addr);
	if (status != 0) {
		hsp->state->mapped.length = 0;

		return status;
	}

	/* Track the memory region mapped by this loader instance. */
	hsp->state->mapped.start = *virt_addr;
	hsp->state->mapped.length = length;

	return 0;
}

void firmware_loader_hsp_dmb_unmap_address (const struct firmware_loader *loader, void *virt_addr)
{
	const struct firmware_loader_hsp_dmb *hsp = (const struct firmware_loader_hsp_dmb*) loader;

	if ((hsp == NULL) || (virt_addr == NULL)) {
		return;
	}

	if (sram_is_address_in_block (&hsp->state->mapped, virt_addr)) {
		hsp->dmb->unmap_soc_address (hsp->dmb, (void*) hsp->state->mapped.start);

		/* Mark the region as unmapped. */
		hsp->state->mapped.length = 0;
	}
}

int firmware_loader_hsp_dmb_load_image (const struct firmware_loader *loader,
	const struct flash *flash, uint32_t src_addr, size_t length, uint8_t *dest_addr,
	const uint8_t *iv, size_t iv_length, const struct hash_engine *hash, enum hash_type hash_algo,
	uint8_t *digest, size_t digest_length)
{
	const struct firmware_loader_hsp_dmb *dmb = (const struct firmware_loader_hsp_dmb*) loader;

	if (dmb == NULL) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	return dmb->fw_load.base.load_image (&dmb->fw_load.base, flash, src_addr, length, dest_addr, iv,
		iv_length, hash, hash_algo, digest, digest_length);
}

int firmware_loader_hsp_dmb_load_image_update_digest (const struct firmware_loader *loader,
	const struct flash *flash, uint32_t src_addr, size_t length, uint8_t *dest_addr,
	const uint8_t *iv, size_t iv_length, const struct hash_engine *hash)
{
	const struct firmware_loader_hsp_dmb *dmb = (const struct firmware_loader_hsp_dmb*) loader;

	if (dmb == NULL) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	return dmb->fw_load.base.load_image_update_digest (&dmb->fw_load.base, flash, src_addr, length,
		dest_addr, iv, iv_length, hash);
}

int firmware_loader_hsp_dmb_copy_image (const struct firmware_loader *loader,
	const uint8_t *src_addr, size_t length, uint8_t *dest_addr, const uint8_t *iv, size_t iv_length,
	const struct hash_engine *hash, enum hash_type hash_algo, uint8_t *digest, size_t digest_length)
{
	const struct firmware_loader_hsp_dmb *dmb = (const struct firmware_loader_hsp_dmb*) loader;

	if (dmb == NULL) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	return dmb->fw_load.base.copy_image (&dmb->fw_load.base, src_addr, length, dest_addr, iv,
		iv_length, hash, hash_algo, digest, digest_length);
}

int firmware_loader_hsp_dmb_copy_image_update_digest (const struct firmware_loader *loader,
	const uint8_t *src_addr, size_t length, uint8_t *dest_addr, const uint8_t *iv, size_t iv_length,
	const struct hash_engine *hash)
{
	const struct firmware_loader_hsp_dmb *dmb = (const struct firmware_loader_hsp_dmb*) loader;

	if (dmb == NULL) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	return dmb->fw_load.base.copy_image_update_digest (&dmb->fw_load.base, src_addr, length,
		dest_addr, iv, iv_length, hash);
}

/**
 * Initialize a handler for loading firmware images into memory locations in the HSP address space.
 *
 * @param loader Tho handler to initialize.
 * @param state Variable context for the firmware loader.  This must be uninitialized.
 * @param dmb Driver interface to the DMB hardware that will be used to load the firmware images
 * into memory.
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
int firmware_loader_hsp_dmb_init (struct firmware_loader_hsp_dmb *loader,
	struct firmware_loader_hsp_dmb_state *state, const struct hsp_dmb *dmb,
	const struct soc_sram_block *sram, size_t blocks, const struct hsp_aes *aes, uint8_t key_slot)
{
	if ((loader == NULL) || (state == NULL) || (dmb == NULL) || (sram == NULL) || (blocks == 0) ||
		(aes == NULL)) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	memset (loader, 0, sizeof (struct firmware_loader_hsp_dmb));

	loader->base.is_address_valid = firmware_loader_hsp_dmb_is_address_valid;
	loader->base.map_address = firmware_loader_hsp_dmb_map_address;
	loader->base.unmap_address = firmware_loader_hsp_dmb_unmap_address;
	loader->base.load_image = firmware_loader_hsp_dmb_load_image;
	loader->base.load_image_update_digest = firmware_loader_hsp_dmb_load_image_update_digest;
	loader->base.copy_image = firmware_loader_hsp_dmb_copy_image;
	loader->base.copy_image_update_digest = firmware_loader_hsp_dmb_copy_image_update_digest;

	loader->state = state;
	loader->dmb = dmb;
	loader->sram = sram;
	loader->blocks = blocks;
	loader->aes = aes;
	loader->key_slot = key_slot;

	/* This call can't fail in this context. */
	firmware_loader_hsp_dmb_init_state (loader);

	return firmware_loader_hsp_memory_init (&loader->fw_load, &state->mapped, 1, aes, key_slot);
}

/**
 * Initialize only the variable state for a firmware loader that uses the HSP DMB.  The rest of the
 * instance is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param loader The firmware loader instance that contains the state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int firmware_loader_hsp_dmb_init_state (const struct firmware_loader_hsp_dmb *loader)
{
	if ((loader == NULL) || (loader->state == NULL) || (loader->dmb == NULL) ||
		(loader->sram == NULL) || (loader->blocks == 0) || (loader->aes == NULL)) {
		return FIRMWARE_LOADER_INVALID_ARGUMENT;
	}

	memset (loader->state, 0, sizeof (struct firmware_loader_hsp_dmb_state));

	return 0;
}

/**
 * Release the resources used by an firmware loader using HSP DMB.
 *
 * @param loader The firmware loader to release.
 */
void firmware_loader_hsp_dmb_release (const struct firmware_loader_hsp_dmb *loader)
{
	if (loader) {
		firmware_loader_hsp_memory_release (&loader->fw_load);
	}
}
