// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "sram_util.h"


/**
 * Check if an address is located within a specified address range.
 *
 * @param start The start address of the range.
 * @param length Length of the valid address range.
 * @param addr The address to check.
 *
 * @return true if the address is in the specified address, false if not.
 */
static bool sram_is_address_in_range (uint64_t start, uint64_t length, uint64_t addr)
{
	if (length != 0) {
		return ((addr >= start) && (addr <= (start + length - 1)));
	}
	else {
		return false;
	}
}

/**
 * Check if one address ranges falls completely within another specified address range.
 *
 * @param start The start address of the range that should contain the other.
 * @param range_length Length of the valid address range.
 * @param addr The starting address for the range that should be contained in the other.
 * @param check_length Length of the address range to check.
 *
 * @return 0 if the data fits in the SRAM block, 1 if it does not, or -1 if the address is outside
 * the address space of the block.
 */
static int sram_data_fits_in_range (uint64_t start, uint64_t range_length, uint64_t addr,
	uint64_t check_length)
{
	uint64_t end = (addr + check_length) - 1;

	if (sram_is_address_in_range (start, range_length, addr)) {
		if ((check_length == 0) ||
			((end >= addr) && sram_is_address_in_range (start, range_length, end))) {
			return 0;
		}
		else {
			return 1;
		}
	}
	else {
		return -1;
	}
}

/**
 * Check if an address is located within a specified block of SRAM.
 *
 * @param sram The block of SRAM to check against.
 * @param addr The address to check.
 *
 * @return true if the address is in the specified SRAM block, false if not.
 */
bool sram_is_address_in_block (const struct sram_block *sram, const void *addr)
{
	if (sram) {
		return sram_is_address_in_range ((uintptr_t) sram->start, sram->length, (uintptr_t) addr);
	}
	else {
		return false;
	}
}

/**
 * Check if an address range falls completely within a specified block of SRAM.
 *
 * @param sram The block of SRAM to check against.
 * @param addr The starting address for the range.
 * @param length Length of the address range.
 *
 * @return 0 if the data fits in the SRAM block, 1 if it does not, or -1 if the address is outside
 * the address space of the block.
 */
int sram_data_fits_in_block (const struct sram_block *sram, const void *addr, size_t length)
{
	if (sram) {
		return sram_data_fits_in_range ((uintptr_t) sram->start, sram->length, (uintptr_t) addr,
			length);
	}
	else {
		return -1;
	}
}

/**
 * Utility function to check if the address belong into the given address range.
 *
 * @param region_start - region start address
 * @param region_size - region size in bytes
 * @param address - memory address to be checked
 *
 * @return true if the address is in the specified address, false if not.
 */
bool sram_is_address_inside_region (const void *region_start, size_t region_size,
	const void *address)
{
	return sram_is_address_in_range ((uintptr_t) region_start, region_size, (uintptr_t) address);
}

/**
 * Utility function to check if the addresses belong into the given address range.
 *
 * @param region_start - region start address
 * @param region_size - region size in bytes
 * @param range_start - start range address to be checked
 * @param range_size - size of the range to be checked
 *
 * @return 0 if the data fits in the region block, 1 if it does not, or -1 if the address is outside
 * the address space of the block
 */
int sram_is_range_inside_region (const void *region_start, size_t region_size,
	const void *range_start, size_t range_size)
{
	return sram_data_fits_in_range ((uintptr_t) region_start, region_size, (uintptr_t) range_start,
		range_size);
}

/**
 * Check if an address is located within a specified block of SRAM.  This check supports the wider
 * addresses used external to the HSP in the overall SoC.
 *
 * @param sram The block of SRAM to check against.
 * @param addr The address to check.
 *
 * @return true if the address is in the specified SRAM block, false if not.
 */
bool sram_is_soc_address_in_block (const struct soc_sram_block *sram, uint64_t addr)
{
	if (sram) {
		return sram_is_address_in_range (sram->start, sram->length, addr);
	}
	else {
		return false;
	}
}

/**
 * Check if an address range falls completely within a specified block of SRAM.  This check supports
 * the wider addresses used external to the HSP in the overall SoC.
 *
 * @param sram The block of SRAM to check against.
 * @param addr The starting address for the range.
 * @param length Length of the address range.
 *
 * @return 0 if the data fits in the SRAM block, 1 if it does not, or -1 if the address is outside
 * the address space of the block.
 */
int sram_data_fits_in_soc_block (const struct soc_sram_block *sram, uint64_t addr, uint64_t length)
{
	if (sram) {
		return sram_data_fits_in_range (sram->start, sram->length, addr, length);
	}
	else {
		return -1;
	}
}

/**
 * Utility function to check if the SOC address belong into the given address region.
 *
 * @param region_start - region start address
 * @param region_size - region size in bytes
 * @param address - memory address to be checked
 *
 * @return true if the address is in the specified address, false if not.
 */
bool sram_is_soc_address_inside_region (uint64_t region_start, uint64_t region_size,
	uint64_t address)
{
	return sram_is_address_in_range (region_start, region_size, address);
}

/**
 * Utility function to check if the SOC addresses range belongs into the given address region.
 *
 * @param region_start - region start address
 * @param region_size - region size in bytes
 * @param range_start - start range address to be checked
 * @param range_size - size of the range to be checked
 *
 * @return 0 if the data fits in the region block, 1 if it does not, or -1 if the address is outside
 * the address space of the block
 */
int sram_is_soc_range_inside_region (uint64_t region_start, uint64_t region_size,
	uint64_t range_start, uint64_t range_size)
{
	return sram_data_fits_in_range (region_start, region_size, range_start, range_size);
}

/**
 * Erase a block of memory in the SoC address space by writing 0 to all bytes.
 *
 * @param dmb The DMB instance to use for mapping the SoC address.
 * @param address SoC address for the memory that should be erased.
 * @param length The number of bytes to erase.
 *
 * @return 0 if the SoC memory was erased successfully or if length is 0, or an error code.
 */
int sram_erase_soc_memory (const struct hsp_dmb *dmb, uint64_t address, uint64_t length)
{
	void *hsp_addr;
	int status;

	if (dmb == NULL) {
		return HSP_SRAM_INVALID_ARGUMENT;
	}

	if (length == 0) {
		return 0;
	}

	status = dmb->map_soc_address (dmb, address, length, HSP_DMB_ACCESS_WRITE, &hsp_addr);
	if (status != 0) {
		return status;
	}

	memset (hsp_addr, 0, length);

	dmb->unmap_soc_address (dmb, hsp_addr);

	return 0;
}

/**
 * Erase multiple blocks of memory in the SoC address space by writing 0 to all bytes in each block.
 *
 * @param dmb The DMB instance to use for mapping the SoC addresses.
 * @param sram A list of SoC memory blocks that should be erased.
 * @param count The number of memory blocks to erase.
 *
 * @return 0 if all SoC memory blocks were erased successfully or an error code.
 */
int sram_erase_soc_memory_blocks (const struct hsp_dmb *dmb, const struct soc_sram_block *sram,
	size_t count)
{
	size_t i;
	int status;

	if ((sram == NULL) && (count != 0)) {
		return HSP_SRAM_INVALID_ARGUMENT;
	}

	for (i = 0; i < count; i++) {
		status = sram_erase_soc_memory (dmb, sram[i].start, sram[i].length);
		if (status != 0) {
			return status;
		}
	}

	return 0;
}
