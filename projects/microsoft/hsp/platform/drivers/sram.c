// Copyright (c) Microsoft Corporation. All rights reserved.

#include "hsp_top.h"
#include "sram.h"


/**
 * Defines the block of HSP shared SRAM.
 */
static const struct sram_block SRAM_SHARED_RAM = {
	.start = (void*) HSP_ADDR_MAP_SHAREDRAM_ADDRESS,
	.length = sizeof (struct Sharedram_SHAREDRAM)
};


/**
 * Check if an address is located in HSP shared SRAM.
 *
 * @param addr The address to check.
 *
 * @return true if the address is in the shared SRAM, false if not.
 */
bool sram_is_shared_address (const void *addr)
{
	return sram_is_address_in_block (&SRAM_SHARED_RAM, addr);
}

/**
 * Check if a data buffer is entirely contained within HSP shared SRAM.
 *
 * @param addr The starting address of the buffer to check.
 * @param length Length of the buffer.
 *
 * @return true if the buffer is in shared SRAM, false if not.
 */
bool sram_is_buffer_in_shared_sram (const void *addr, size_t length)
{
	return (sram_data_fits_in_block (&SRAM_SHARED_RAM, addr, length) == 0);
}

/**
 * Defines the block of SP instruction SRAM.
 */
static const struct sram_block SRAM_SP_IRAM = {
	.start = (void*) HSP_ADDR_MAP_SP_IRAM_ADDRESS,
	.length = HSP_ADDR_MAP_SP_IRAM_SIZE
};


/**
 * Check if an address is located in SP instruction SRAM.
 *
 * @param addr The address to check.
 *
 * @return true if the address is in the instruction SRAM, false if not.
 */
bool sram_is_instruction_ram_address (const void *addr)
{
	return sram_is_address_in_block (&SRAM_SP_IRAM, addr);
}

/**
 * Check if a data buffer is entirely contained within HSP instruction SRAM.
 *
 * @param addr The starting address of the buffer to check.
 * @param length Length of the buffer.
 *
 * @return true if the buffer is in the instruction SRAM, false if not.
 */
bool sram_is_buffer_in_instruction_sram (const void *addr, size_t length)
{
	return (sram_data_fits_in_block (&SRAM_SP_IRAM, addr, length) == 0);
}

/**
 * Defines the block of SP data SRAM.
 */
static const struct sram_block SRAM_SP_DRAM = {
	.start = (void*) HSP_ADDR_MAP_SP_DRAM_ADDRESS,
	.length = HSP_ADDR_MAP_SP_DRAM_SIZE
};


/**
 * Check if an address is located in SP data SRAM.
 *
 * @param addr The address to check.
 *
 * @return true if the address is in the data SRAM, false if not.
 */
bool sram_is_data_ram_address (const void *addr)
{
	return sram_is_address_in_block (&SRAM_SP_DRAM, addr);
}

/**
 * Check if a data buffer is entirely contained within HSP data SRAM.
 *
 * @param addr The starting address of the buffer to check.
 * @param length Length of the buffer.
 *
 * @return true if the buffer is in the data SRAM, false if not.
 */
bool sram_is_buffer_in_data_sram (const void *addr, size_t length)
{
	return (sram_data_fits_in_block (&SRAM_SP_DRAM, addr, length) == 0);
}
