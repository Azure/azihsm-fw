// Copyright (c) Microsoft Corporation. All rights reserved.

#include <memory.h>
#include "common/type_cast.h"
#include "common/unused.h"
#include "mmio/mmio_logging.h"
#include "mmio/mmio_register_block_soc.h"
#include "mmio/mmio_util.h"

int mmio_register_block_soc_map (const struct mmio_register_block *register_block)
{
	const struct mmio_register_block_soc *block = TO_DERIVED_TYPE (register_block,
		const struct mmio_register_block_soc, base);

	if (register_block == NULL) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if (block->state->mapped_address != NULL) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MMIO,
			MMIO_LOGGING_ALREADY_MAPPED, (uint32_t) block->register_block.start,
			((uint32_t) (block->register_block.start >> 32)));

		return 0;
	}

	return block->dmb->map_soc_address (block->dmb, block->register_block.start,
		block->register_block.length, block->dmb_flags, (void**) &block->state->mapped_address);
}

void mmio_register_block_soc_unmap (const struct mmio_register_block *register_block)
{
	const struct mmio_register_block_soc *block = TO_DERIVED_TYPE (register_block,
		const struct mmio_register_block_soc, base);

	if (register_block == NULL) {
		return;
	}

	if (block->state->mapped_address == NULL) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MMIO,
			MMIO_LOGGING_NOT_MAPPED, (uint32_t) block->register_block.start,
			((uint32_t) (block->register_block.start >> 32)));

		return;
	}

	block->dmb->unmap_soc_address (block->dmb, block->state->mapped_address);

	block->state->mapped_address = NULL;
}

int mmio_register_block_soc_read32 (const struct mmio_register_block *register_block,
	uintptr_t register_offset, uint32_t *dest)
{
	const struct mmio_register_block_soc *block = TO_DERIVED_TYPE (register_block,
		const struct mmio_register_block_soc, base);

	if ((register_block == NULL) || (dest == NULL)) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if (sram_data_fits_in_soc_block (&block->register_block,
		block->register_block.start + register_offset, sizeof (uint32_t))) {
		return MMIO_REGISTER_OFFSET_OUT_OF_RANGE;
	}

	if ((register_offset & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_OFFSET;
	}

	if (block->state->mapped_address == NULL) {
		return MMIO_REGISTER_NOT_MAPPED;
	}

	*dest = mmio_register_read32 (block->state->mapped_address +
		(register_offset / sizeof (uint32_t)));

	return 0;
}

int mmio_register_block_soc_write32 (const struct mmio_register_block *register_block,
	uintptr_t register_offset, uint32_t value)
{
	const struct mmio_register_block_soc *block = TO_DERIVED_TYPE (register_block,
		const struct mmio_register_block_soc, base);

	if (register_block == NULL) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if (sram_data_fits_in_soc_block (&block->register_block,
		block->register_block.start + register_offset, sizeof (uint32_t))) {
		return MMIO_REGISTER_OFFSET_OUT_OF_RANGE;
	}

	if ((register_offset & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_OFFSET;
	}

	if (block->state->mapped_address == NULL) {
		return MMIO_REGISTER_NOT_MAPPED;
	}

	mmio_register_write32 (block->state->mapped_address + (register_offset / sizeof (uint32_t)),
		value);

	return 0;
}

int mmio_register_block_soc_block_read32 (const struct mmio_register_block *register_block,
	uintptr_t block_offset, uint32_t *dest, size_t dwords_count)
{
	const struct mmio_register_block_soc *block = TO_DERIVED_TYPE (register_block,
		const struct mmio_register_block_soc, base);

	if ((register_block == NULL) || (dest == NULL) || (dwords_count == 0)) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if (sram_data_fits_in_soc_block (&block->register_block,
		block->register_block.start + block_offset, (dwords_count * sizeof (uint32_t))) != 0) {
		return MMIO_REGISTER_OFFSET_OUT_OF_RANGE;
	}

	if ((block_offset & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_OFFSET;
	}

	if (block->state->mapped_address == NULL) {
		return MMIO_REGISTER_NOT_MAPPED;
	}

	mmio_register_block_read32 (dest,
		block->state->mapped_address + (block_offset / sizeof (uint32_t)), dwords_count);

	return 0;
}

int mmio_register_block_soc_block_write32 (const struct mmio_register_block *register_block,
	uintptr_t block_offset, const uint32_t *src, size_t dwords_count)
{
	const struct mmio_register_block_soc *block = TO_DERIVED_TYPE (register_block,
		const struct mmio_register_block_soc, base);

	if ((register_block == NULL) || (src == NULL) || (dwords_count == 0)) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if (sram_data_fits_in_soc_block (&block->register_block,
		block->register_block.start + block_offset, (dwords_count * sizeof (uint32_t))) != 0) {
		return MMIO_REGISTER_OFFSET_OUT_OF_RANGE;
	}

	if ((block_offset & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_OFFSET;
	}

	if (block->state->mapped_address == NULL) {
		return MMIO_REGISTER_NOT_MAPPED;
	}

	mmio_register_block_write32 (block->state->mapped_address + (block_offset / sizeof (uint32_t)),
		src, dwords_count);

	return 0;
}

int mmio_register_block_soc_get_address_offset (const struct mmio_register_block *register_block,
	uint64_t address, uintptr_t *offset)
{
	const struct mmio_register_block_soc *block = TO_DERIVED_TYPE (register_block,
		const struct mmio_register_block_soc, base);

	if ((register_block == NULL) || (offset == NULL)) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if (sram_data_fits_in_soc_block (&block->register_block, address, sizeof (uint32_t))) {
		return MMIO_REGISTER_ADDRESS_OUT_OF_RANGE;
	}

	if ((address & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_ADDRESS;
	}

	*offset = address - block->register_block.start;

	return 0;
}

int mmio_register_block_soc_read32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, uint32_t *dest)
{
	uintptr_t address_offset;
	int status;

	status = mmio_register_block_soc_get_address_offset (register_block, physical_address,
		&address_offset);
	if (status != 0) {
		return status;
	}

	return mmio_register_block_soc_read32 (register_block, address_offset, dest);
}

int mmio_register_block_soc_write32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, uint32_t value)
{
	uintptr_t address_offset;
	int status;

	status = mmio_register_block_soc_get_address_offset (register_block, physical_address,
		&address_offset);
	if (status != 0) {
		return status;
	}

	return mmio_register_block_soc_write32 (register_block, address_offset, value);
}

int mmio_register_block_soc_block_read32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, uint32_t *dest, size_t dwords_count)
{
	uintptr_t address_offset;
	int status;

	status = mmio_register_block_soc_get_address_offset (register_block, physical_address,
		&address_offset);
	if (status != 0) {
		return status;
	}

	status = mmio_register_block_soc_block_read32 (register_block, address_offset, dest,
		dwords_count);
	if (status == MMIO_REGISTER_OFFSET_OUT_OF_RANGE) {
		status = MMIO_REGISTER_ADDRESS_OUT_OF_RANGE;
	}

	return status;
}

int mmio_register_block_soc_block_write32_by_addr (const struct mmio_register_block *register_block,
	uint64_t physical_address, const uint32_t *src, size_t dwords_count)
{
	uintptr_t address_offset;
	int status;

	status = mmio_register_block_soc_get_address_offset (register_block, physical_address,
		&address_offset);
	if (status != 0) {
		return status;
	}

	status = mmio_register_block_soc_block_write32 (register_block, address_offset, src,
		dwords_count);
	if (status == MMIO_REGISTER_OFFSET_OUT_OF_RANGE) {
		status = MMIO_REGISTER_ADDRESS_OUT_OF_RANGE;
	}

	return status;
}

int mmio_register_block_soc_get_physical_address (const struct mmio_register_block *register_block,
	uintptr_t offset, uint64_t *address)
{
	const struct mmio_register_block_soc *block = TO_DERIVED_TYPE (register_block,
		const struct mmio_register_block_soc, base);

	if ((register_block == NULL) || (address == NULL)) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if (sram_data_fits_in_soc_block (&block->register_block, block->register_block.start + offset,
		sizeof (uint32_t))) {
		return MMIO_REGISTER_OFFSET_OUT_OF_RANGE;
	}

	if ((offset & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_OFFSET;
	}

	*address = block->register_block.start + offset;

	return 0;
}

/**
 * Initialize SOC based register block
 *
 * @param register_block - instance to be initialized
 * @param state - runtime state object
 * @param dmb - DMB object
 * @param soc_address - register block SOC address
 * @param block_size - register block size
 */
int mmio_register_block_soc_init (struct mmio_register_block_soc *register_block,
	struct mmio_register_block_soc_state *state, const struct hsp_dmb *dmb, uint64_t soc_address,
	size_t block_size)
{
	return mmio_register_block_dmb_soc_init_with_flags (register_block, state, dmb, soc_address,
		block_size, HSP_DMB_ACCESS_WRITE);
}

/**
 * Initialize SOC based register block
 *
 * @param register_block - instance to be initialized
 * @param state - runtime state object
 * @param dmb - DMB object
 * @param soc_address - register block SOC address
 * @param block_size - register block size
 * @param dmb_flags - DMB flag object
 */
int mmio_register_block_dmb_soc_init_with_flags (struct mmio_register_block_soc *register_block,
	struct mmio_register_block_soc_state *state, const struct hsp_dmb *dmb, uint64_t soc_address,
	size_t block_size, uint8_t dmb_flags)
{
	if (register_block == NULL) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	memset (register_block, 0, sizeof (*register_block));

	register_block->base.map = mmio_register_block_soc_map;
	register_block->base.unmap = mmio_register_block_soc_unmap;
	register_block->base.read32 = mmio_register_block_soc_read32;
	register_block->base.write32 = mmio_register_block_soc_write32;
	register_block->base.block_read32 = mmio_register_block_soc_block_read32;
	register_block->base.block_write32 = mmio_register_block_soc_block_write32;

	register_block->base.read32_by_addr = mmio_register_block_soc_read32_by_addr;
	register_block->base.write32_by_addr = mmio_register_block_soc_write32_by_addr;
	register_block->base.block_read32_by_addr = mmio_register_block_soc_block_read32_by_addr;
	register_block->base.block_write32_by_addr = mmio_register_block_soc_block_write32_by_addr;
	register_block->base.get_physical_address = mmio_register_block_soc_get_physical_address;
	register_block->base.get_address_offset = mmio_register_block_soc_get_address_offset;

	register_block->state = state;
	register_block->dmb = dmb;
	register_block->register_block.start = soc_address;
	register_block->register_block.length = block_size;
	register_block->dmb_flags = dmb_flags;

	return mmio_register_block_soc_init_state (register_block);
}

/**
 * Initialize SOC register block state
 *
 * @param register_block - register block instance
 */
int mmio_register_block_soc_init_state (const struct mmio_register_block_soc *register_block)
{
	if ((register_block == NULL) || (register_block->state == NULL) ||
		(register_block->dmb == NULL) || (register_block->register_block.start == 0) ||
		(register_block->register_block.length == 0)) {
		return MMIO_REGISTER_INVALID_ARGUMENT;
	}

	if ((register_block->register_block.start & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_ADDRESS;
	}

	if ((register_block->register_block.length & 0x3) != 0) {
		return MMIO_REGISTER_UNALIGNED_SIZE;
	}

	memset (register_block->state, 0, sizeof (*register_block->state));

	return 0;
}

/**
 * Release register block resources
 *
 * @param register_block - register block instance
 */
void mmio_register_block_soc_release (const struct mmio_register_block_soc *register_block)
{
	UNUSED (register_block);
}
