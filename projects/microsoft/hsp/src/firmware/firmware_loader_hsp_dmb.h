// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FIRMWARE_LOADER_HSP_DMB_H_
#define FIRMWARE_LOADER_HSP_DMB_H_

#include "common/sram_util.h"
#include "drivers/hsp_aes.h"
#include "drivers/hsp_dmb.h"
#include "firmware/firmware_loader_hsp_memory.h"


/**
 * Variable context for the DMB firmware loader.
 */
struct firmware_loader_hsp_dmb_state {
	struct sram_block mapped;	/**< Block of data current mapped by the loader. */
};

/**
 * Handler for loading firmware into memory locations external to the HSP through the DMB.
 */
struct firmware_loader_hsp_dmb {
	struct firmware_loader base;					/**< The base firmware loader API. */
	struct firmware_loader_hsp_dmb_state *state;	/**< Variable context for the loader. */
	const struct hsp_dmb *dmb;						/**< Driver interface to the DMB. */
	const struct soc_sram_block *sram;				/**< The list of address ranges where firmware can be loaded. */
	size_t blocks;									/**< Number of allowed address ranges. */
	const struct hsp_aes *aes;						/**< AES driver for decrypting images. */
	uint8_t key_slot;								/**< KSU key slot for the firmware encryption key. */
	struct firmware_loader_hsp_memory fw_load;		/**< Handler that will actually do to image loading. */
};


int firmware_loader_hsp_dmb_init (struct firmware_loader_hsp_dmb *loader,
	struct firmware_loader_hsp_dmb_state *state, const struct hsp_dmb *dmb,
	const struct soc_sram_block *sram, size_t blocks, const struct hsp_aes *aes, uint8_t key_slot);
int firmware_loader_hsp_dmb_init_state (const struct firmware_loader_hsp_dmb *loader);
void firmware_loader_hsp_dmb_release (const struct firmware_loader_hsp_dmb *loader);


#endif	/* FIRMWARE_LOADER_HSP_DMB_H_ */
