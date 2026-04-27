// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FIRMWARE_LOADER_HSP_MEMORY_H_
#define FIRMWARE_LOADER_HSP_MEMORY_H_

#include "common/sram_util.h"
#include "drivers/hsp_aes.h"
#include "firmware/firmware_loader.h"


/**
 * Handler for loading firmware into memory locations within the HSP address space.
 */
struct firmware_loader_hsp_memory {
	struct firmware_loader base;	/**< The base firmware loader API. */
	const struct sram_block *sram;	/**< The list of address ranges where firmware can be loaded. */
	size_t blocks;					/**< Number of allowed address ranges. */
	const struct hsp_aes *aes;		/**< AES driver for decrypting images. */
	uint8_t key_slot;				/**< KSU key slot for the firmware encryption key. */
};


int firmware_loader_hsp_memory_init (struct firmware_loader_hsp_memory *loader,
	const struct sram_block *sram, size_t blocks, const struct hsp_aes *aes, uint8_t key_slot);
void firmware_loader_hsp_memory_release (const struct firmware_loader_hsp_memory *loader);


#endif	/* FIRMWARE_LOADER_HSP_MEMORY_H_ */
