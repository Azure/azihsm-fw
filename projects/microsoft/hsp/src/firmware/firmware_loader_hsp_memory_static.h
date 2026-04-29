// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FIRMWARE_LOADER_HSP_MEMORY_STATIC_H_
#define FIRMWARE_LOADER_HSP_MEMORY_STATIC_H_

#include "firmware/firmware_loader_hsp_memory.h"


/* Internal functions declared to allow for static initialization. */
int firmware_loader_hsp_memory_is_address_valid (const struct firmware_loader *loader,
	uint64_t address, size_t length);
int firmware_loader_hsp_memory_map_address (const struct firmware_loader *loader, uint64_t phy_addr,
	size_t length, void **virt_addr);
void firmware_loader_hsp_memory_unmap_address (const struct firmware_loader *loader,
	void *virt_addr);
int firmware_loader_hsp_memory_load_image (const struct firmware_loader *loader,
	const struct flash *flash, uint32_t src_addr, size_t length, uint8_t *dest_addr,
	const uint8_t *iv, size_t iv_length, const struct hash_engine *hash, enum hash_type hash_algo,
	uint8_t *digest, size_t digest_length);
int firmware_loader_hsp_memory_load_image_update_digest (const struct firmware_loader *loader,
	const struct flash *flash, uint32_t src_addr, size_t length, uint8_t *dest_addr,
	const uint8_t *iv, size_t iv_length, const struct hash_engine *hash);
int firmware_loader_hsp_memory_copy_image (const struct firmware_loader *loader,
	const uint8_t *src_addr, size_t length, uint8_t *dest_addr, const uint8_t *iv, size_t iv_length,
	const struct hash_engine *hash, enum hash_type hash_algo, uint8_t *digest,
	size_t digest_length);
int firmware_loader_hsp_memory_copy_image_update_digest (const struct firmware_loader *loader,
	const uint8_t *src_addr, size_t length, uint8_t *dest_addr, const uint8_t *iv, size_t iv_length,
	const struct hash_engine *hash);


/**
 * Constant initializer for the firmware loader API.
 */
#define	FIRMWARE_LOADER_HSP_MEMORY_API_INIT  { \
		.is_address_valid = firmware_loader_hsp_memory_is_address_valid, \
		.map_address = firmware_loader_hsp_memory_map_address, \
		.unmap_address = firmware_loader_hsp_memory_unmap_address, \
		.load_image = firmware_loader_hsp_memory_load_image, \
		.load_image_update_digest = firmware_loader_hsp_memory_load_image_update_digest, \
		.copy_image = firmware_loader_hsp_memory_copy_image, \
		.copy_image_update_digest = firmware_loader_hsp_memory_copy_image_update_digest \
	}


/**
 * Initialize a static instance of a handler for loading firmware images into memory in the HSP
 * address space.
 *
 * There is no validation done on the arguments.
 *
 * @param sram_list A list of address ranges than can be used to load the firmware image.  When
 * checking to see if an image can be loaded, each address range is inspected individually, even if
 * multiple blocks represent a single contiguous memory region.  This allows a single range of
 * addresses to be viewed as different blocks (e.g. IRAM vs. DRAM) from a firmware load perspective.
 * Firmware will not be allowed to load if it crosses between address ranges.  This can be a
 * constant instance.
 * @param block_count The total number of address ranges that can be used for firmware.
 * @param aes_ptr The AES HW engine to use for decrypting encrypted images.  This can be a constant
 * instance.
 * @param aes_key Index for the key slot to use for decryption.
 */
#define	firmware_loader_hsp_memory_static_init(sram_list, block_count, aes_ptr, aes_key)	{ \
		.base = FIRMWARE_LOADER_HSP_MEMORY_API_INIT, \
		.sram = sram_list, \
		.blocks = block_count, \
		.aes = aes_ptr, \
		.key_slot = aes_key \
	}


#endif	/* FIRMWARE_LOADER_HSP_MEMORY_STATIC_H_ */
