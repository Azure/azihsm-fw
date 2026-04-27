// Copyright (c) Microsoft Corporation. All rights reserved.

#include <memory.h>
#include "hsp_mpu.h"
#include "common/type_cast.h"
#include "common/unused.h"


/**
 * Helper function which looks up an memory map entry based on the given memory region
 *
 * @param hsp_mpu - MPU instance
 * @param region_address - given memory region start address. Must be page aligned.
 * @param region_size - memory region size. Must be page aligned
 * @param found_entry - Output for found entry in case of success.
 *
 * @return 0 if entry is found, error otherwise.
 */
static int hsp_mpu_find_memory_map_entry (const struct hsp_mpu *hsp_mpu, const void *region_address,
	size_t region_size, const struct hsp_mpu_memory_map_entry **found_entry)
{
	uint32_t entry_index = 0;
	const struct hsp_mpu_memory_map_entry *entry;

	if ((hsp_mpu == NULL) || (region_address == NULL) || (region_size == 0) ||
		(found_entry == NULL)) {
		return MPU_INVALID_ARGUMENT;
	}

	if ((((uintptr_t) region_address) % hsp_mpu->page_size) != 0) {
		return MPU_UNALIGNED_ADDRESS;
	}

	if ((region_size % hsp_mpu->page_size) != 0) {
		return MPU_UNALIGNED_SIZE;
	}

	for (entry_index = 0; entry_index < hsp_mpu->entries_count; entry_index++) {
		entry = &hsp_mpu->memory_map_entries[entry_index];

		if (sram_data_fits_in_block (&entry->memory_region, region_address, region_size) == 0) {
			*found_entry = entry;

			return 0;
		}
	}

	return MPU_UNSUPPORTED_ADDRESS;
}

/**
 * Helper function to set page attributes for specific protection level (user/privileged)
 *
 * @param hsp_mpu - HSP MPU instance
 * @param register_offset - Registers block offset for given protection level
 * @param page_index - Memory page index
 * @param page_attributes - page attributes to be set
 *
 * @return 0 if successful, error code otherwise
 */
static int hsp_mpu_set_page_attributes (const struct hsp_mpu *hsp_mpu, size_t register_offset,
	size_t page_index, uint32_t page_attributes)
{
	int status;
	uint32_t register_index;
	uint32_t register_mask;
	uint32_t attribute_register;

	register_index = page_index / 8;
	register_mask = MPU_PAGE_ATTRIBUTE_ALL << ((page_index % 8) * 4);

	status = hsp_mpu->mpu_register_block->read32 (hsp_mpu->mpu_register_block,
		register_offset + (register_index * sizeof (uint32_t)), &attribute_register);
	if (status != 0) {
		return status;
	}

	/* HSP MPU registers are using the same set of attribute bits as MPU defined interface
	 * so we are using this mask without any modifications */
	attribute_register = (attribute_register & ~register_mask) |
		(page_attributes << ((page_index % 8) * 4));

	status = hsp_mpu->mpu_register_block->write32 (hsp_mpu->mpu_register_block,
		register_offset + (register_index * sizeof (uint32_t)), attribute_register);

	return status;
}

int hsp_mpu_get_page_size (const struct mpu_interface *mpu, size_t *page_size)
{
	const struct hsp_mpu *hsp_mpu = TO_DERIVED_TYPE (mpu, const struct hsp_mpu,	base);

	if ((hsp_mpu == NULL) || (page_size == NULL)) {
		return MPU_INVALID_ARGUMENT;
	}

	*page_size = hsp_mpu->page_size;

	return 0;
}

int hsp_mpu_set_region_attributes (const struct mpu_interface *mpu,	const void *region_address,
	size_t region_size, uint32_t protection_level, uint32_t page_attributes)
{
	const struct hsp_mpu *hsp_mpu = TO_DERIVED_TYPE (mpu, const struct hsp_mpu,	base);
	int status;
	const struct hsp_mpu_memory_map_entry *found_entry = NULL;
	uint32_t i;
	uint8_t *page_address;
	uint32_t page_index;

	if ((page_attributes & ~MPU_PAGE_ATTRIBUTE_ALL) != 0) {
		return MPU_INVALID_ARGUMENT;
	}

	if ((protection_level & ~MPU_PROTECTION_LEVEL_ALL) != 0) {
		return MPU_INVALID_ARGUMENT;
	}

	status = hsp_mpu_find_memory_map_entry (hsp_mpu, region_address, region_size, &found_entry);
	if (status != 0) {
		return status;
	}

	status = hsp_mpu->mpu_register_block->map (hsp_mpu->mpu_register_block);
	if (status != 0) {
		return status;
	}

	/* got through each page of the region */
	for (i = 0; i < (region_size / hsp_mpu->page_size); i++) {
		page_address = ((uint8_t*) region_address) + (i * hsp_mpu->page_size);
		page_index = (page_address - ((const uint8_t*) found_entry->memory_region.start)) /
			hsp_mpu->page_size;

		if ((protection_level & MPU_PROTECTION_LEVEL_USER) != 0) {
			status = hsp_mpu_set_page_attributes (hsp_mpu, found_entry->user_register_offset,
				page_index, page_attributes);
			if (status != 0) {
				goto exit;
			}
		}

		if ((protection_level & MPU_PROTECTION_LEVEL_PRIVILEGE) != 0) {
			status = hsp_mpu_set_page_attributes (hsp_mpu, found_entry->privileged_register_offset,
				page_index, page_attributes);
			if (status != 0) {
				goto exit;
			}
		}
	}

exit:
	hsp_mpu->mpu_register_block->unmap (hsp_mpu->mpu_register_block);

	return status;
}

int hsp_mpu_get_page_attributes (const struct mpu_interface *mpu, const void *address,
	uint32_t protection_level, uint32_t *page_attributes)
{
	const struct hsp_mpu *hsp_mpu = TO_DERIVED_TYPE (mpu, const struct hsp_mpu,	base);
	int status;
	const struct hsp_mpu_memory_map_entry *found_entry = NULL;
	uint32_t page_index;
	uint32_t register_index;
	uint32_t attribute_register;
	const void *page_address;
	size_t register_offset;

	if ((hsp_mpu == NULL) || (page_attributes == NULL)) {
		return MPU_INVALID_ARGUMENT;
	}

	if ((protection_level != MPU_PROTECTION_LEVEL_USER) &&
		(protection_level != MPU_PROTECTION_LEVEL_PRIVILEGE)) {
		return MPU_INVALID_ARGUMENT;
	}

	/* Align address to begining of the page */
	page_address = (const void*) (((uintptr_t) address) & ~(hsp_mpu->page_size - 1));

	status = hsp_mpu_find_memory_map_entry (hsp_mpu, page_address, hsp_mpu->page_size,
		&found_entry);
	if (status != 0) {
		return status;
	}

	page_index = (((uint8_t*) (page_address)) -
		((const uint8_t*) found_entry->memory_region.start)) / hsp_mpu->page_size;

	/* 8 pages per single register */
	register_index = page_index / 8;

	status = hsp_mpu->mpu_register_block->map (hsp_mpu->mpu_register_block);
	if (status != 0) {
		return status;
	}

	if (protection_level == MPU_PROTECTION_LEVEL_USER) {
		register_offset = found_entry->user_register_offset;
	}
	else {
		register_offset = found_entry->privileged_register_offset;
	}

	status = hsp_mpu->mpu_register_block->read32 (hsp_mpu->mpu_register_block,
		register_offset + (register_index * sizeof (uint32_t)), &attribute_register);

	hsp_mpu->mpu_register_block->unmap (hsp_mpu->mpu_register_block);

	if (status != 0) {
		return status;
	}

	*page_attributes = (attribute_register >> ((page_index % 8) * 4)) & MPU_PAGE_ATTRIBUTE_ALL;

	return 0;
}

/**
 * Initialize instance of HSP MPU object
 *
 * @param mpu - HSP MPU object to initialize
 * @param mpu_registers - MPU MMIO registers block
 * @param page_size - MPU supported page size
 * @param memory_entries - Array of memory map descriptors
 * @param entries_count - Number of memory regions
 */
int hsp_mpu_init (struct hsp_mpu *mpu, const struct mmio_register_block *mpu_registers,
	size_t page_size, const struct hsp_mpu_memory_map_entry *memory_entries, size_t entries_count)
{
	uint32_t i;

	if ((mpu == NULL) || (mpu_registers == NULL) || (memory_entries == NULL) ||
		(entries_count == 0) || (page_size == 0)) {
		return MPU_INVALID_ARGUMENT;
	}

	for (i = 0; i < entries_count; i++) {
		if ((((uintptr_t) memory_entries[i].memory_region.start) % page_size) != 0) {
			return MPU_UNALIGNED_ADDRESS;
		}

		if ((memory_entries[i].memory_region.length % page_size) != 0) {
			return MPU_UNALIGNED_SIZE;
		}
	}

	memset (mpu, 0, sizeof (*mpu));

	mpu->base.get_page_size = hsp_mpu_get_page_size;
	mpu->base.set_region_attributes = hsp_mpu_set_region_attributes;
	mpu->base.get_page_attributes = hsp_mpu_get_page_attributes;

	mpu->page_size = page_size;
	mpu->mpu_register_block = mpu_registers;
	mpu->memory_map_entries = memory_entries;
	mpu->entries_count = entries_count;

	return 0;
}

/**
 * Clean up HSP MPU object
 */
void hsp_mpu_release (const struct hsp_mpu *mpu)
{
	UNUSED (mpu);
}
