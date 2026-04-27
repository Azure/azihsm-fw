// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common/unused.h"
#include "flash/flash_common.h"
#include "spi_filter/flash_mfg_filter_handler_cpld.h"


static int flash_mfg_filter_handler_cpld_set_flash_manufacturer (
	const struct flash_mfg_filter_handler *handler, uint8_t vendor, uint16_t device)
{
	const struct flash_mfg_filter_handler_cpld *cpld =
		(const struct flash_mfg_filter_handler_cpld*) handler;
	uint8_t filter_id;

	if (cpld == NULL) {
		return MFG_FILTER_HANDLER_INVALID_ARGUMENT;
	}

	switch (vendor) {
		case FLASH_ID_MACRONIX:
			switch (FLASH_ID_DEVICE_SERIES (device)) {
				case FLASH_ID_MX25L:
					filter_id = SPI_FILTER_MFG_MACRONIX;
					break;

				default:
					return MFG_FILTER_HANDLER_UNSUPPORTED_DEVICE;
			}
			break;

		case FLASH_ID_WINBOND:
			switch (FLASH_ID_DEVICE_SERIES (device)) {
				case FLASH_ID_W25Q:
				case FLASH_ID_W25Q_DTR:
					filter_id = SPI_FILTER_MFG_WINBOND;
					break;

				default:
					return MFG_FILTER_HANDLER_UNSUPPORTED_DEVICE;
			}
			break;

		case FLASH_ID_MICRON:
			switch (FLASH_ID_DEVICE_SERIES (device)) {
				case FLASH_ID_MT25QL:
					filter_id = SPI_FILTER_MFG_MICRON;
					break;

				default:
					return MFG_FILTER_HANDLER_UNSUPPORTED_DEVICE;
			}
			break;

		default:
			return MFG_FILTER_HANDLER_UNSUPPORTED_VENDOR;
	}

	return cpld->filter->base.set_mfg_id (&cpld->filter->base, filter_id);
}

/**
 * Initialize a flash manufacturer handler for a CPLD SPI filter.
 *
 * @param handler The handler to initialize.
 * @param filter The SPI filter to configure from the handler.
 *
 * @return 0 if the handler was successfully initialized or an error code.
 */
int flash_mfg_filter_handler_cpld_init (struct flash_mfg_filter_handler_cpld *handler,
	struct spi_filter_cpld *filter)
{
	if ((handler == NULL) || (filter == NULL)) {
		return MFG_FILTER_HANDLER_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (struct flash_mfg_filter_handler_cpld));

	handler->base.set_flash_manufacturer = flash_mfg_filter_handler_cpld_set_flash_manufacturer;

	handler->filter = filter;

	return 0;
}

/**
 * Release the resources used by a flash manufacturer handler for a CPLD SPI filter.
 *
 * @param handler The handler to release.
 */
void flash_mfg_filter_handler_cpld_release (struct flash_mfg_filter_handler_cpld *handler)
{
	UNUSED (handler);
}
