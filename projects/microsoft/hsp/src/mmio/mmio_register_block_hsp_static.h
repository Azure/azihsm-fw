// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MMIO_REGISTER_BLOCK_HSP_STATIC_H_
#define MMIO_REGISTER_BLOCK_HSP_STATIC_H_

#include "mmio_register_block_hsp.h"


/* Internal function declarations to allow for static initialization. */
int mmio_register_block_hsp_map (const struct mmio_register_block *register_block);
void mmio_register_block_hsp_unmap (const struct mmio_register_block *register_block);
int mmio_register_block_hsp_read32 (const struct mmio_register_block *register_block,
	uintptr_t register_offset, uint32_t *dest);
int mmio_register_block_hsp_write32 (const struct mmio_register_block *register_block,
	uintptr_t register_offset, uint32_t value);
int mmio_register_block_hsp_block_read32 (const struct mmio_register_block *register_block,
	uintptr_t block_offset, uint32_t *dest, size_t dwords_count);
int mmio_register_block_hsp_block_write32 (const struct mmio_register_block *register_block,
	uintptr_t block_offset, const uint32_t *src, size_t dwords_count);
int mmio_register_block_hsp_read32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, uint32_t *dest);
int mmio_register_block_hsp_write32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, uint32_t value);
int mmio_register_block_hsp_block_read32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, uint32_t *dest, size_t dwords_count);
int mmio_register_block_hsp_block_write32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, const uint32_t *src, size_t dwords_count);
int mmio_register_block_hsp_get_physical_address (const struct mmio_register_block *register_block,
	uintptr_t offset, uint64_t *address);
int mmio_register_block_hsp_get_address_offset (const struct mmio_register_block *register_block,
	uint64_t address, uintptr_t *offset);

/**
 * Constant initializer for SP local MMIO register block
 */
#define MMIO_REGISTER_BLOCK_HSP_API_INIT { \
	.map = mmio_register_block_hsp_map, \
	.unmap = mmio_register_block_hsp_unmap, \
	.read32 = mmio_register_block_hsp_read32, \
	.write32 = mmio_register_block_hsp_write32, \
	.block_read32 = mmio_register_block_hsp_block_read32, \
	.block_write32 = mmio_register_block_hsp_block_write32, \
	.read32_by_addr = mmio_register_block_hsp_read32_by_addr, \
	.write32_by_addr = mmio_register_block_hsp_write32_by_addr, \
	.block_read32_by_addr = mmio_register_block_hsp_block_read32_by_addr, \
	.block_write32_by_addr = mmio_register_block_hsp_block_write32_by_addr, \
	.get_physical_address = mmio_register_block_hsp_get_physical_address, \
	.get_address_offset = mmio_register_block_hsp_get_address_offset, \
}

/**
 * SP local MMIO register block Static Initialization.
 *
 * There is no validation done on the arguments.
 *
 * @param block_address_arg - HSP local registers block address
 * @param size_arg - Registers block size
 *
 */
#define mmio_register_block_hsp_static_init(block_address_arg, size_arg) { \
	.base = MMIO_REGISTER_BLOCK_HSP_API_INIT, \
	.block_address = block_address_arg, \
	.block_size = size_arg, \
}


#endif	// MMIO_REGISTER_BLOCK_HSP_STATIC_H_
