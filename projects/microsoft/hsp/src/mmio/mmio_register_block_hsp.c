// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "common/sram_util.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "mmio/mmio_register_block_hsp.h"
#include "mmio/mmio_util.h"


int mmio_register_block_hsp_map (const struct mmio_register_block *register_block)
{
	UNUSED (register_block);

	return 0;
}

void mmio_register_block_hsp_unmap (const struct mmio_register_block *register_block)
{
	UNUSED (register_block);
}

int mmio_register_block_hsp_read32 (const struct mmio_register_block *register_block,
	uintptr_t register_offset, uint32_t *dest)
{
	const struct mmio_register_block_hsp *block = TO_DERIVED_TYPE (register_block,
		const struct mmio_register_block_hsp, base);

	if ((register_block == NULL) || (dest == NULL)) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if ((register_offset & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_OFFSET;
	}

	if ((register_offset + sizeof (uint32_t)) > block->block_size) {
		return MMIO_REGISTER_OFFSET_OUT_OF_RANGE;
	}

	*dest = mmio_register_read32 (block->block_address + (register_offset / sizeof (uint32_t)));

	return 0;
}

int mmio_register_block_hsp_write32 (const struct mmio_register_block *register_block,
	uintptr_t register_offset, uint32_t value)
{
	const struct mmio_register_block_hsp *block = TO_DERIVED_TYPE (register_block,
		const struct mmio_register_block_hsp, base);

	if (register_block == NULL) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if ((register_offset & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_OFFSET;
	}

	if ((register_offset + sizeof (uint32_t)) > block->block_size) {
		return MMIO_REGISTER_OFFSET_OUT_OF_RANGE;
	}

	mmio_register_write32 (block->block_address + (register_offset / sizeof (uint32_t)), value);

	return 0;
}

int mmio_register_block_hsp_block_read32 (const struct mmio_register_block *register_block,
	uintptr_t block_offset, uint32_t *dest, size_t dwords_count)
{
	const struct mmio_register_block_hsp *block = TO_DERIVED_TYPE (register_block,
		const struct mmio_register_block_hsp, base);

	if ((register_block == NULL) || (dest == NULL) || (dwords_count == 0)) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if ((block_offset & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_OFFSET;
	}

	if (sram_is_range_inside_region (block->block_address, block->block_size,
		(((uint8_t*) block->block_address) + block_offset),
		(dwords_count * sizeof (uint32_t))) != 0) {
		return MMIO_REGISTER_OFFSET_OUT_OF_RANGE;
	}

	mmio_register_block_read32 (dest, block->block_address + (block_offset / sizeof (uint32_t)),
		dwords_count);

	return 0;
}

int mmio_register_block_hsp_block_write32 (const struct mmio_register_block *register_block,
	uintptr_t block_offset, const uint32_t *src, size_t dwords_count)
{
	const struct mmio_register_block_hsp *block = TO_DERIVED_TYPE (register_block,
		const struct mmio_register_block_hsp, base);

	if ((register_block == NULL) || (src == NULL) || (dwords_count == 0)) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if ((block_offset & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_OFFSET;
	}

	if (sram_is_range_inside_region (block->block_address, block->block_size,
		(((uint8_t*) block->block_address) + block_offset),
		(dwords_count * sizeof (uint32_t))) != 0) {
		return MMIO_REGISTER_OFFSET_OUT_OF_RANGE;
	}

	mmio_register_block_write32 (block->block_address + (block_offset / sizeof (uint32_t)), src,
		dwords_count);

	return 0;
}

int mmio_register_block_hsp_get_address_offset (const struct mmio_register_block *register_block,
	uint64_t address, uintptr_t *offset)
{
	const struct mmio_register_block_hsp *block = TO_DERIVED_TYPE (register_block,
		const struct mmio_register_block_hsp, base);

	if ((register_block == NULL) || (offset == NULL)) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if (sram_is_soc_range_inside_region ((uintptr_t) block->block_address, block->block_size,
		address, sizeof (uint32_t)) != 0) {
		return MMIO_REGISTER_ADDRESS_OUT_OF_RANGE;
	}

	if ((address & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_ADDRESS;
	}

	*offset = address - ((uintptr_t) block->block_address);

	return 0;
}

int mmio_register_block_hsp_read32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, uint32_t *dest)
{
	uintptr_t address_offset;
	int status;

	status = mmio_register_block_hsp_get_address_offset (register_block, physical_address,
		&address_offset);
	if (status != 0) {
		return status;
	}

	return mmio_register_block_hsp_read32 (register_block, address_offset, dest);
}

int mmio_register_block_hsp_write32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, uint32_t value)
{
	uintptr_t address_offset;
	int status;

	status = mmio_register_block_hsp_get_address_offset (register_block, physical_address,
		&address_offset);
	if (status != 0) {
		return status;
	}

	return mmio_register_block_hsp_write32 (register_block, address_offset, value);
}

int mmio_register_block_hsp_block_read32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, uint32_t *dest, size_t dwords_count)
{
	uintptr_t address_offset;
	int status;

	status = mmio_register_block_hsp_get_address_offset (register_block, physical_address,
		&address_offset);
	if (status != 0) {
		return status;
	}

	status = mmio_register_block_hsp_block_read32 (register_block, address_offset, dest,
		dwords_count);
	if (status == MMIO_REGISTER_OFFSET_OUT_OF_RANGE) {
		status = MMIO_REGISTER_ADDRESS_OUT_OF_RANGE;
	}

	return status;
}

int mmio_register_block_hsp_block_write32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, const uint32_t *src, size_t dwords_count)
{
	uintptr_t address_offset;
	int status;

	status = mmio_register_block_hsp_get_address_offset (register_block, physical_address,
		&address_offset);
	if (status != 0) {
		return status;
	}
	status = mmio_register_block_hsp_block_write32 (register_block, address_offset, src,
		dwords_count);
	if (status == MMIO_REGISTER_OFFSET_OUT_OF_RANGE) {
		status = MMIO_REGISTER_ADDRESS_OUT_OF_RANGE;
	}

	return status;
}

int mmio_register_block_hsp_get_physical_address (const struct mmio_register_block *register_block,
	uintptr_t offset, uint64_t *address)
{
	const struct mmio_register_block_hsp *block = TO_DERIVED_TYPE (register_block,
		const struct mmio_register_block_hsp, base);

	if ((register_block == NULL) || (address == NULL)) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if ((offset & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_OFFSET;
	}

	if (sram_is_range_inside_region (block->block_address, block->block_size,
		block->block_address + (offset / sizeof (uint32_t)), sizeof (uint32_t)) != 0) {
		return MMIO_REGISTER_OFFSET_OUT_OF_RANGE;
	}

	*address = ((uintptr_t) block->block_address) + offset;

	return 0;
}

/**
 * Initialize SP local register block
 *
 * @param register_block - instance to be initialized
 * @param soc_address - register block SOC address
 * @param block_size - register block size
 *
 * @return 0 if successful, error code otherwise
 */
int mmio_register_block_hsp_init (struct mmio_register_block_hsp *register_block,
	uint32_t *block_address, size_t block_size)
{
	if ((register_block == NULL) || (block_address == 0) ||	(block_size == 0)) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if ((((uintptr_t) block_address) & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_ADDRESS;
	}

	if ((block_size & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_SIZE;
	}

	memset (register_block, 0, sizeof (*register_block));

	register_block->base.map = mmio_register_block_hsp_map;
	register_block->base.unmap = mmio_register_block_hsp_unmap;
	register_block->base.read32 = mmio_register_block_hsp_read32;
	register_block->base.write32 = mmio_register_block_hsp_write32;
	register_block->base.block_read32 = mmio_register_block_hsp_block_read32;
	register_block->base.block_write32 = mmio_register_block_hsp_block_write32;

	register_block->base.read32_by_addr = mmio_register_block_hsp_read32_by_addr;
	register_block->base.write32_by_addr = mmio_register_block_hsp_write32_by_addr;
	register_block->base.block_read32_by_addr = mmio_register_block_hsp_block_read32_by_addr;
	register_block->base.block_write32_by_addr = mmio_register_block_hsp_block_write32_by_addr;
	register_block->base.get_physical_address = mmio_register_block_hsp_get_physical_address;
	register_block->base.get_address_offset = mmio_register_block_hsp_get_address_offset;

	register_block->block_address = block_address;
	register_block->block_size = block_size;

	return 0;
}

/**
 * Release register block resources
 *
 * @param register_block - register block instance
 */
void mmio_register_block_hsp_release (const struct mmio_register_block_hsp *register_block)
{
	UNUSED (register_block);
}
