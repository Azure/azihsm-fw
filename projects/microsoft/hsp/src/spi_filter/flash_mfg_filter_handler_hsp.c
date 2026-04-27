// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "flash_mfg_filter_handler_hsp.h"
#include "macronix_opcodes.h"
#include "micron_opcodes.h"
#include "winbond_opcodes.h"
#include "common/unused.h"
#include "flash/flash_common.h"


int flash_mfg_filter_handler_hsp_set_flash_manufacturer (
	const struct flash_mfg_filter_handler *handler, uint8_t vendor, uint16_t device)
{
	const struct flash_mfg_filter_handler_hsp *hsp =
		(const struct flash_mfg_filter_handler_hsp*) handler;
	const union spi_filter_hsp_opcode *opcodes;
	size_t count;

	if (hsp == NULL) {
		return MFG_FILTER_HANDLER_INVALID_ARGUMENT;
	}

	switch (vendor) {
		case FLASH_ID_MACRONIX:
			switch (FLASH_ID_DEVICE_SERIES (device)) {
				case FLASH_ID_MX25L:
					if (FLASH_ID_DEVICE_CAPACITY (device) < 0x19) {
						opcodes = macronix_opcodes_mx25l128;
						count = macronix_opcodes_mx25l128_count;
					}
					else {
						opcodes = macronix_opcodes_mx25l256;
						count = macronix_opcodes_mx25l256_count;
					}
					break;

				case FLASH_ID_MX25U:
					/* MX25U devices use the same opcodes as MX25L, but a different value for
					 * memory density. */
					if (FLASH_ID_DEVICE_CAPACITY (device) < 0x39) {
						opcodes = macronix_opcodes_mx25l128;
						count = macronix_opcodes_mx25l128_count;
					}
					else {
						opcodes = macronix_opcodes_mx25l256;
						count = macronix_opcodes_mx25l256_count;
					}
					break;

				default:
					return MFG_FILTER_HANDLER_UNSUPPORTED_DEVICE;
			}
			break;

		case FLASH_ID_WINBOND:
			switch (FLASH_ID_DEVICE_SERIES (device)) {
				case FLASH_ID_W25Q:
				case FLASH_ID_W25Q_DTR:
					if (FLASH_ID_DEVICE_CAPACITY (device) < 0x19) {
						opcodes = winbond_opcodes_w25q128;
						count = winbond_opcodes_w25q128_count;
					}
					else {
						opcodes = winbond_opcodes_w25q256;
						count = winbond_opcodes_w25q256_count;
					}
					break;

				default:
					return MFG_FILTER_HANDLER_UNSUPPORTED_DEVICE;
			}
			break;

		case FLASH_ID_MICRON:
			switch (FLASH_ID_DEVICE_SERIES (device)) {
				case FLASH_ID_MT25QL:
				case FLASH_ID_MT25QU:
					if (FLASH_ID_DEVICE_CAPACITY (device) < 0x19) {
						opcodes = micron_opcodes_mt25q128;
						count = micron_opcodes_mt25q128_count;
					}
					else if (FLASH_ID_DEVICE_CAPACITY (device) < 0x21) {
						opcodes = micron_opcodes_mt25q256;
						count = micron_opcodes_mt25q256_count;
					}
					else {
						opcodes = micron_opcodes_mt25q01g;
						count = micron_opcodes_mt25q01g_count;
					}
					break;

				default:
					return MFG_FILTER_HANDLER_UNSUPPORTED_DEVICE;
			}
			break;

		default:
			return MFG_FILTER_HANDLER_UNSUPPORTED_VENDOR;
	}

	return hsp->filter->set_filtered_opcodes (hsp->filter, opcodes, count);
}

/**
 * Initialize a handler for configuring an HSP SPI filter based on flash device type.
 *
 * @param handler The handler to initialize.
 * @param filter The SPI filter to configure.
 *
 * @return 0 if the handler was successfully initialized or an error code.
 */
int flash_mfg_filter_handler_hsp_init (struct flash_mfg_filter_handler_hsp *handler,
	const struct spi_filter_hsp *filter)
{
	if ((handler == NULL) || (filter == NULL)) {
		return MFG_FILTER_HANDLER_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (struct flash_mfg_filter_handler_hsp));

	handler->base.set_flash_manufacturer = flash_mfg_filter_handler_hsp_set_flash_manufacturer;

	handler->filter = filter;

	return 0;
}

/**
 * Release a HSP SPI filter device type configuration handler.
 *
 * @param handler The handler to release.
 */
void flash_mfg_filter_handler_hsp_release (const struct flash_mfg_filter_handler_hsp *handler)
{
	UNUSED (handler);
}
