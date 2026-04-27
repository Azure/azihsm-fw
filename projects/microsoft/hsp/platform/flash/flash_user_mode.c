// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "flash_user_mode.h"
#include "common/unused.h"


/**
 * Request context to be executed in user mode.
 */
struct flash_user_mode_request {
	const struct flash *flash;	/**< The flash device executing the request. */
	uint32_t address;			/**< Address to read from. */
	uint8_t *data;				/**< Output buffer for the data. */
	size_t length;				/**< Amount of data to read. */
};


int flash_user_mode_get_device_size (const struct flash *flash, uint32_t *bytes)
{
	const struct flash_user_mode *user_flash = (const struct flash_user_mode*) flash;

	if (user_flash == NULL) {
		return FLASH_INVALID_ARGUMENT;
	}

	return user_flash->device->get_device_size (user_flash->device, bytes);
}

/**
 * Callback to execute the SPI flash read operation in user mode.
 *
 * @param context The request context to execute.
 * @param unused Unused argument.
 *
 * @return 0 if the request was successful or an error code.  The return in HSP_STATUS to match the
 * callback signature, but it really returns standard error codes.  The return value is not
 * interpreted by the code that switches execution contexts.
 */
static HSP_STATUS flash_user_mode_read_callback (uintptr_t context, uintptr_t unused)
{
	struct flash_user_mode_request *request = (struct flash_user_mode_request*) context;

	UNUSED (unused);

	return request->flash->read (request->flash, request->address, request->data, request->length);
}

int flash_user_mode_read (const struct flash *flash, uint32_t address, uint8_t *data, size_t length)
{
	const struct flash_user_mode *user_flash = (const struct flash_user_mode*) flash;
	struct flash_user_mode_request request;

	if ((user_flash == NULL) || (data == NULL)) {
		return FLASH_INVALID_ARGUMENT;
	}

	request.flash = user_flash->device;
	request.address = address;
	request.data = data;
	request.length = length;

	return ExecuteInUserMode (user_flash->pmp_config, user_flash->pmp_regions,
		flash_user_mode_read_callback, (uintptr_t) &request, 0);
}

int flash_user_mode_get_page_size (const struct flash *flash, uint32_t *bytes)
{
	const struct flash_user_mode *user_flash = (const struct flash_user_mode*) flash;

	if (user_flash == NULL) {
		return FLASH_INVALID_ARGUMENT;
	}

	return user_flash->device->get_page_size (user_flash->device, bytes);
}

int flash_user_mode_minimum_write_per_page (const struct flash *flash, uint32_t *bytes)
{
	const struct flash_user_mode *user_flash = (const struct flash_user_mode*) flash;

	if (user_flash == NULL) {
		return FLASH_INVALID_ARGUMENT;
	}

	return user_flash->device->minimum_write_per_page (user_flash->device, bytes);
}

int flash_user_mode_write (const struct flash *flash, uint32_t address, const uint8_t *data,
	size_t length)
{
	const struct flash_user_mode *user_flash = (const struct flash_user_mode*) flash;

	if (user_flash == NULL) {
		return FLASH_INVALID_ARGUMENT;
	}

	return user_flash->device->write (user_flash->device, address, data, length);
}

int flash_user_mode_get_sector_size (const struct flash *flash, uint32_t *bytes)
{
	const struct flash_user_mode *user_flash = (const struct flash_user_mode*) flash;

	if (user_flash == NULL) {
		return FLASH_INVALID_ARGUMENT;
	}

	return user_flash->device->get_sector_size (user_flash->device, bytes);
}

int flash_user_mode_sector_erase (const struct flash *flash, uint32_t sector_addr)
{
	const struct flash_user_mode *user_flash = (const struct flash_user_mode*) flash;

	if (user_flash == NULL) {
		return FLASH_INVALID_ARGUMENT;
	}

	return user_flash->device->sector_erase (user_flash->device, sector_addr);
}

int flash_user_mode_get_block_size (const struct flash *flash, uint32_t *bytes)
{
	const struct flash_user_mode *user_flash = (const struct flash_user_mode*) flash;

	if (user_flash == NULL) {
		return FLASH_INVALID_ARGUMENT;
	}

	return user_flash->device->get_block_size (user_flash->device, bytes);
}

int flash_user_mode_block_erase (const struct flash *flash, uint32_t block_addr)
{
	const struct flash_user_mode *user_flash = (const struct flash_user_mode*) flash;

	if (user_flash == NULL) {
		return FLASH_INVALID_ARGUMENT;
	}

	return user_flash->device->block_erase (user_flash->device, block_addr);
}

int flash_user_mode_chip_erase (const struct flash *flash)
{
	const struct flash_user_mode *user_flash = (const struct flash_user_mode*) flash;

	if (user_flash == NULL) {
		return FLASH_INVALID_ARGUMENT;
	}

	return user_flash->device->chip_erase (user_flash->device);
}

/**
 * Initialize a flash device API that into switches RISC-V user mode and conigures PMP when reading
 * data into memory.
 *
 * This wrapper should only be used by machine mode code.
 *
 * @param flash The flash interface to initialize.
 * @param device The flash device that should be accessed.
 * @param pmp_config A list of PMP settings to apply when switching to user mode.  User mode must
 * have read access to the machine stack and the flash instances for this to work.
 * @param pmp_regions The number of PMP addresses in the list.
 *
 * @return 0 if the flash interface was successfully initialized or an error code.
 */
int flash_user_mode_init (struct flash_user_mode *flash, const struct flash *device,
	const RiscvPmpSetting *pmp_config, size_t pmp_regions)
{
	if ((flash == NULL) || (device == NULL) || (pmp_config == NULL) || (pmp_regions == 0)) {
		return FLASH_INVALID_ARGUMENT;
	}

	memset (flash, 0, sizeof (struct flash_user_mode));

	flash->base.get_device_size = flash_user_mode_get_device_size;
	flash->base.read = flash_user_mode_read;
	flash->base.get_page_size = flash_user_mode_get_page_size;
	flash->base.minimum_write_per_page = flash_user_mode_minimum_write_per_page;
	flash->base.write = flash_user_mode_write;
	flash->base.get_sector_size = flash_user_mode_get_sector_size;
	flash->base.sector_erase = flash_user_mode_sector_erase;
	flash->base.get_block_size = flash_user_mode_get_block_size;
	flash->base.block_erase = flash_user_mode_block_erase;
	flash->base.chip_erase = flash_user_mode_chip_erase;

	flash->device = device;
	flash->pmp_config = pmp_config;
	flash->pmp_regions = pmp_regions;

	return 0;
}

/**
 * Release the resources use by a user mode flash device wrapper.
 *
 * @param flash The flash interface to release.
 */
void flash_user_mode_release (const struct flash_user_mode *flash)
{
	UNUSED (flash);
}
