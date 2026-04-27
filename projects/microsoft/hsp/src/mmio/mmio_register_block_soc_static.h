// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MMIO_REGISTER_BLOCK_SOC_STATIC_H_
#define MMIO_REGISTER_BLOCK_SOC_STATIC_H_

#include "mmio_register_block_soc.h"


/* Internal function declarations to allow for static initialization. */
int mmio_register_block_soc_map (const struct mmio_register_block *register_block);
void mmio_register_block_soc_unmap (const struct mmio_register_block *register_block);
int mmio_register_block_soc_read32 (const struct mmio_register_block *register_block,
	uintptr_t register_offset, uint32_t *dest);
int mmio_register_block_soc_write32 (const struct mmio_register_block *register_block,
	uintptr_t register_offset, uint32_t value);
int mmio_register_block_soc_block_read32 (const struct mmio_register_block *register_block,
	uintptr_t block_offset, uint32_t *dest, size_t dwords_count);
int mmio_register_block_soc_block_write32 (const struct mmio_register_block *register_block,
	uintptr_t block_offset, const uint32_t *src, size_t dwords_count);
int mmio_register_block_soc_read32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, uint32_t *dest);
int mmio_register_block_soc_write32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, uint32_t value);
int mmio_register_block_soc_block_read32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, uint32_t *dest, size_t dwords_count);
int mmio_register_block_soc_block_write32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, const uint32_t *src, size_t dwords_count);
int mmio_register_block_soc_get_physical_address (const struct mmio_register_block *register_block,
	uintptr_t offset, uint64_t *address);
int mmio_register_block_soc_get_address_offset (const struct mmio_register_block *register_block,
	uint64_t address, uintptr_t *offset);


/**
 * Constant initializer for SOC MMIO register block
 */
#define MMIO_REGISTER_BLOCK_SOC_API_INIT { \
	.map = mmio_register_block_soc_map, \
	.unmap = mmio_register_block_soc_unmap, \
	.read32 = mmio_register_block_soc_read32, \
	.write32 = mmio_register_block_soc_write32, \
	.block_read32 = mmio_register_block_soc_block_read32, \
	.block_write32 = mmio_register_block_soc_block_write32, \
	.read32_by_addr = mmio_register_block_soc_read32_by_addr, \
	.write32_by_addr = mmio_register_block_soc_write32_by_addr, \
	.block_read32_by_addr = mmio_register_block_soc_block_read32_by_addr, \
	.block_write32_by_addr = mmio_register_block_soc_block_write32_by_addr, \
	.get_physical_address = mmio_register_block_soc_get_physical_address, \
	.get_address_offset = mmio_register_block_soc_get_address_offset, \
}

/**
 * SOC MMIO register block Static Initialization.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr - MMIO register block state object to keep runtime state
 * @param dmb_ptr - DMB object for memory mapping
 * @param soc_addr - MMIO registers block SOC address
 * @param size - MMIO registers block size
 */
#define mmio_register_block_soc_static_init(state_ptr, dmb_ptr, soc_addr, size_arg) { \
	.base = MMIO_REGISTER_BLOCK_SOC_API_INIT, \
	.dmb = dmb_ptr, \
	.register_block = { \
		.start = soc_addr, \
		.length = size_arg, \
	}, \
	.state = state_ptr, \
	.dmb_flags = HSP_DMB_ACCESS_WRITE, \
}

/**
 * SOC MMIO register block Static Initialization.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr - MMIO register block state object to keep runtime state
 * @param dmb_ptr - DMB object for memory mapping
 * @param soc_addr - MMIO registers block SOC address
 * @param size - MMIO registers block size
 * @param dmb_flags_parm - DMB flag object
 */
#define mmio_register_block_dmb_soc_static_init_with_flags(state_ptr, dmb_ptr, soc_addr, size_arg, \
		dmb_flags_parm) {                                                                                            \
	.base = MMIO_REGISTER_BLOCK_SOC_API_INIT, \
	.dmb = dmb_ptr, \
	.register_block = { \
		.start = soc_addr, \
		.length = size_arg, \
	}, \
	.state = state_ptr, \
	.dmb_flags = dmb_flags_parm, \
}


#endif	// MMIO_REGISTER_BLOCK_SOC_STATIC_H_
