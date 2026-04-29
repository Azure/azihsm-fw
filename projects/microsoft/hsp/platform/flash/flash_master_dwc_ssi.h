// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FLASH_MASTER_DWC_SSI_H_
#define FLASH_MASTER_DWC_SSI_H_

#include <stdint.h>
#include "drivers/spi_dwc_ssi.h"
#include "flash/flash_master.h"


/**
 * Full capabilities of the flash master.
 */
#define	FLASH_MASTER_DWC_SSI_FULL_CAPABILITIES  \
	(FLASH_CAP_DUAL_2_2_2 | FLASH_CAP_DUAL_1_2_2 | FLASH_CAP_DUAL_1_1_2 | \
		FLASH_CAP_QUAD_4_4_4 | FLASH_CAP_QUAD_1_4_4 | FLASH_CAP_QUAD_1_1_4 | \
		FLASH_CAP_3BYTE_ADDR | FLASH_CAP_4BYTE_ADDR)


/**
 * Variable context for the flash device instance.
 */
struct flash_master_dwc_ssi_state {
	uint32_t spi_freq;	/**< Frequency to use for SPI transactions with this device. */
};

/**
 * Interface to a single flash device using the Synopsys DesignWare Cores Synchronous Serial
 * Interface as the SPI master.
 */
struct flash_master_dwc_ssi {
	struct flash_master base;					/**< Base flash master instance. */
	struct flash_master_dwc_ssi_state *state;	/**< Variable context for the device instance. */
	const struct spi_dwc_ssi *spi;				/**< SPI master driver to use for this device. */
	uint8_t chip_select;						/**< Chip select on the SPI bus for the target flash device. */
	uint32_t capabilities;						/**< The flash capabilities to advertise. */
};


int flash_master_dwc_ssi_init (struct flash_master_dwc_ssi *flash,
	struct flash_master_dwc_ssi_state *state, const struct spi_dwc_ssi *spi, uint8_t chip_select);
int flash_master_dwc_ssi_init_with_capabilities (struct flash_master_dwc_ssi *flash,
	struct flash_master_dwc_ssi_state *state, const struct spi_dwc_ssi *spi, uint8_t chip_select,
	uint32_t capabilities);
int flash_master_dwc_ssi_init_state (const struct flash_master_dwc_ssi *flash);
void flash_master_dwc_ssi_release (const struct flash_master_dwc_ssi *flash);


#endif	/* FLASH_MASTER_DWC_SSI_H_ */
