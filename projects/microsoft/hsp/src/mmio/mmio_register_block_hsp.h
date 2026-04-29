// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MMIO_REGISTER_BLOCK_HSP_H_
#define MMIO_REGISTER_BLOCK_HSP_H_

#include "mmio/mmio_register_block.h"


/**
 * MMIO register block implementation for HSP local core
 */
struct mmio_register_block_hsp {
	struct mmio_register_block base;	/**< MMIO base interface */
	uint32_t *block_address;			/**< HSP register block address */
	size_t block_size;					/**< Register block size in bytes */
};


int mmio_register_block_hsp_init (struct mmio_register_block_hsp *register_block,
	uint32_t *block_address, size_t block_size);
void mmio_register_block_hsp_release (const struct mmio_register_block_hsp *register_block);


#endif	// MMIO_REGISTER_BLOCK_HSP_H_
