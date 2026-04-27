// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FLASH_MFG_FILTER_HANDLER_HSP_H_
#define FLASH_MFG_FILTER_HANDLER_HSP_H_

#include "spi_filter/flash_mfg_filter_handler.h"
#include "spi_filter/spi_filter_hsp.h"


/**
 * Flash device type handler for the HSP SPI filter.
 */
struct flash_mfg_filter_handler_hsp {
	struct flash_mfg_filter_handler base;	/**< Base API for filter configuration. */
	const struct spi_filter_hsp *filter;	/**< The SPI filter that will be configured. */
};


int flash_mfg_filter_handler_hsp_init (struct flash_mfg_filter_handler_hsp *handler,
	const struct spi_filter_hsp *filter);
void flash_mfg_filter_handler_hsp_release (const struct flash_mfg_filter_handler_hsp *handler);


#endif	/* FLASH_MFG_FILTER_HANDLER_HSP_H_ */
