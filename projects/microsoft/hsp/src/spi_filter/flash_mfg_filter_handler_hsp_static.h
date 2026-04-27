// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FLASH_MFG_FILTER_HANDLER_HSP_STATIC_H_
#define FLASH_MFG_FILTER_HANDLER_HSP_STATIC_H_

#include "spi_filter/flash_mfg_filter_handler_hsp.h"


/* Internal functions declared to allow for static initialization. */
int flash_mfg_filter_handler_hsp_set_flash_manufacturer (
	const struct flash_mfg_filter_handler *handler, uint8_t vendor, uint16_t device);


/**
 * Constant initializer for the handler API.
 */
#define	FLASH_MFG_FILTER_HANDLER_HSP_API_INIT  { \
		.set_flash_manufacturer = flash_mfg_filter_handler_hsp_set_flash_manufacturer \
	}


/**
 * Initialize a static handler for configuring an HSP SPI filter based on flash device type.
 *
 * There is no validation done on the arguments.
 *
 * @param filter_ptr The HSP SPI filter that will be configured.
 */
#define	flash_mfg_filter_handler_hsp_static_init(filter_ptr)	{ \
		.base = FLASH_MFG_FILTER_HANDLER_HSP_API_INIT, \
		.filter = filter_ptr \
	}


#endif	/* FLASH_MFG_FILTER_HANDLER_HSP_STATIC_H_ */
