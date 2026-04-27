// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FLASH_USER_MODE_STATIC_H_
#define FLASH_USER_MODE_STATIC_H_

#include "flash/flash_user_mode.h"


/* Internal functions declared to allow for static initialization. */
int flash_user_mode_get_device_size (const struct flash *flash, uint32_t *bytes);
int flash_user_mode_read (const struct flash *flash, uint32_t address, uint8_t *data,
	size_t length);
int flash_user_mode_get_page_size (const struct flash *flash, uint32_t *bytes);
int flash_user_mode_minimum_write_per_page (const struct flash *flash, uint32_t *bytes);
int flash_user_mode_write (const struct flash *flash, uint32_t address, const uint8_t *data,
	size_t length);
int flash_user_mode_get_sector_size (const struct flash *flash, uint32_t *bytes);
int flash_user_mode_sector_erase (const struct flash *flash, uint32_t sector_addr);
int flash_user_mode_get_block_size (const struct flash *flash, uint32_t *bytes);
int flash_user_mode_block_erase (const struct flash *flash, uint32_t block_addr);
int flash_user_mode_chip_erase (const struct flash *flash);


/**
 * Constant initializer for the flash API.
 */
#define	FLASH_USER_MODE_API_INIT  { \
		.get_device_size = flash_user_mode_get_device_size, \
		.read = flash_user_mode_read, \
		.get_page_size = flash_user_mode_get_page_size, \
		.minimum_write_per_page = flash_user_mode_minimum_write_per_page, \
		.write = flash_user_mode_write, \
		.get_sector_size = flash_user_mode_get_sector_size, \
		.sector_erase = flash_user_mode_sector_erase, \
		.get_block_size = flash_user_mode_get_block_size, \
		.block_erase = flash_user_mode_block_erase, \
		.chip_erase = flash_user_mode_chip_erase \
	}


/**
 * Initialize a static instance of a flash device wrapped for user mode read access.
 *
 * There is no validation done on the arguments.
 *
 * @param flash_ptr The flash device that should be accessed.
 * @param pmp_ptr The list of PMP address configuration to use with user mode.
 * @param pmp_count Number of PMP address regions.
 */
#define	flash_user_mode_static_init(flash_ptr, pmp_ptr, pmp_count)	{ \
		.base = FLASH_USER_MODE_API_INIT, \
		.device = flash_ptr, \
		.pmp_config = pmp_ptr, \
		.pmp_regions = pmp_count \
	}


#endif	/* FLASH_USER_MODE_STATIC_H_ */
