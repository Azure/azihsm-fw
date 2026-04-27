// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MMIO_REGISTER_BLOCK_SOC_H_
#define MMIO_REGISTER_BLOCK_SOC_H_

#include "common/sram_util.h"
#include "drivers/hsp_dmb.h"
#include "mmio/mmio_register_block.h"

/**
 * DMB mapped MMIO register block state
 */
struct mmio_register_block_soc_state {
	uint32_t *mapped_address;	/**< Runtime mapped register block address */
};

/**
 * MMIO register block implementation for DMB mapped register block
 */
struct mmio_register_block_soc {
	struct mmio_register_block base;				/**< MMIO base interface */
	const struct hsp_dmb *dmb;						/**< DMB object */
	struct soc_sram_block register_block;			/**< SOC based registers block */
	struct mmio_register_block_soc_state *state;	/**< Runtime state */
	uint8_t dmb_flags;								/**< DMB flag object */
};


int mmio_register_block_soc_init (struct mmio_register_block_soc *register_block,
	struct mmio_register_block_soc_state *state, const struct hsp_dmb *dmb, uint64_t soc_address,
	size_t block_size);
int mmio_register_block_dmb_soc_init_with_flags (struct mmio_register_block_soc *register_block,
	struct mmio_register_block_soc_state *state, const struct hsp_dmb *dmb, uint64_t soc_address,
	size_t block_size, uint8_t dmb_flags);
int mmio_register_block_soc_init_state (const struct mmio_register_block_soc *register_block);
void mmio_register_block_soc_release (const struct mmio_register_block_soc *register_block);


#endif	// MMIO_REGISTER_BLOCK_SOC_H_
