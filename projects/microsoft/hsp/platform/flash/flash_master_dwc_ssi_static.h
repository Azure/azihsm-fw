// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FLASH_MASTER_DWC_SSI_STATIC_H_
#define FLASH_MASTER_DWC_SSI_STATIC_H_

#include "hsp_top.h"
#include "flash/flash_master_dwc_ssi.h"


/* Internal functions declared to allow for static initialization. */
int flash_master_dwc_ssi_xfer (const struct flash_master *spi, const struct flash_xfer *xfer);
uint32_t flash_master_dwc_ssi_capabilities (const struct flash_master *spi);
int flash_master_dwc_ssi_get_spi_clock_frequency (const struct flash_master *spi);
int flash_master_dwc_ssi_set_spi_clock_frequency (const struct flash_master *spi, uint32_t freq);


/**
 * Constant initializer for the firmware loader API.
 */
#define	FLASH_MASTER_DWC_SSI_API_INIT  { \
		.xfer = flash_master_dwc_ssi_xfer, \
		.capabilities = flash_master_dwc_ssi_capabilities, \
		.get_spi_clock_frequency = flash_master_dwc_ssi_get_spi_clock_frequency, \
		.set_spi_clock_frequency = flash_master_dwc_ssi_set_spi_clock_frequency, \
	}

/**
 * Constant initializer for the device capabilities.  Only allow Dual/Quad transfers when clock
 * stretching is not supported in hardware.
 */
#if DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_CLK_STRETCH_EN_WRITE_ACCESS == 1
#define	FLASH_MASTER_DWC_SSI_CAPABILITIES_INIT(x)   \
	(x & FLASH_MASTER_DWC_SSI_FULL_CAPABILITIES)
#else
#define	FLASH_MASTER_DWC_SSI_CAPABILITIES_INIT(x)   \
	((x & FLASH_MASTER_DWC_SSI_FULL_CAPABILITIES) | FLASH_CAP_DUAL_RX_ONLY | FLASH_CAP_QUAD_RX_ONLY)
#endif


/**
 * Initialize a static instance for a single flash device interface.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the flash interface.
 * @param spi_ptr The SPI master connected to the flash device.
 * @param chip_sel Chip select number for the flash device on the SPI bus.
 */
#define	flash_master_dwc_ssi_static_init(state_ptr, spi_ptr, chip_sel)	{ \
		.base = FLASH_MASTER_DWC_SSI_API_INIT, \
		.state = state_ptr, \
		.spi = spi_ptr, \
		.chip_select = chip_sel, \
		.capabilities = \
			FLASH_MASTER_DWC_SSI_CAPABILITIES_INIT (FLASH_MASTER_DWC_SSI_FULL_CAPABILITIES) \
	}

/**
 * Initialize a static instance for a single flash device interface and set the advertised
 * capabilities of the flash master.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the flash interface.
 * @param spi_ptr The SPI master connected to the flash device.
 * @param chip_sel Chip select number for the flash device on the SPI bus.
 * @param caps The bitmask of capabilities that should be advertised for this flash master.  These
 * must be flags from the FLASH_CAP_* enum.  At least one of the address mode capability flags needs
 * to be set.
 */
#define	flash_master_dwc_ssi_static_init_with_capabilities(state_ptr, spi_ptr, chip_sel, caps)	{ \
		.base = FLASH_MASTER_DWC_SSI_API_INIT, \
		.state = state_ptr, \
		.spi = spi_ptr, \
		.chip_select = chip_sel, \
		.capabilities = FLASH_MASTER_DWC_SSI_CAPABILITIES_INIT (caps) \
	}


#endif	/* FLASH_MASTER_DWC_SSI_STATIC_H_ */
