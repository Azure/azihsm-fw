// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FLASH_MFG_FILTER_HANDLER_CPLD_H_
#define FLASH_MFG_FILTER_HANDLER_CPLD_H_

#include "spi_filter/flash_mfg_filter_handler.h"
#include "spi_filter/spi_filter_cpld.h"


/**
 * Flash device type handler for the CPLD SPI filter.
 */
struct flash_mfg_filter_handler_cpld {
	struct flash_mfg_filter_handler base;	/**< The base handler instance. */
	struct spi_filter_cpld *filter;			/**< The SPI filter. */
};


int flash_mfg_filter_handler_cpld_init (struct flash_mfg_filter_handler_cpld *handler,
	struct spi_filter_cpld *filter);
void flash_mfg_filter_handler_cpld_release (struct flash_mfg_filter_handler_cpld *handler);


#endif	/* FLASH_MFG_FILTER_HANDLER_CPLD_H_ */
