// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "flash_master_dwc_ssi.h"
#include "hsp_top.h"
#include "common/unused.h"


int flash_master_dwc_ssi_xfer (const struct flash_master *spi, const struct flash_xfer *xfer)
{
	const struct flash_master_dwc_ssi *flash = (const struct flash_master_dwc_ssi*) spi;

	if ((flash == NULL) || (xfer == NULL)) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	return spi_dwc_ssi_xfer (flash->spi, xfer, flash->chip_select, flash->state->spi_freq);
}

uint32_t flash_master_dwc_ssi_capabilities (const struct flash_master *spi)
{
	const struct flash_master_dwc_ssi *flash = (const struct flash_master_dwc_ssi*) spi;

	if (flash) {
		return flash->capabilities;
	}
	else {
#if DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_CLK_STRETCH_EN_WRITE_ACCESS == 1

		return FLASH_MASTER_DWC_SSI_FULL_CAPABILITIES;
#else

		/* If the hardware does not support clock stretching, only support Dual/Quad operation for
		 * read transfers. */
		return FLASH_MASTER_DWC_SSI_FULL_CAPABILITIES | FLASH_CAP_DUAL_RX_ONLY |
			   FLASH_CAP_QUAD_RX_ONLY;
#endif
	}
}

int flash_master_dwc_ssi_get_spi_clock_frequency (const struct flash_master *spi)
{
	const struct flash_master_dwc_ssi *flash = (const struct flash_master_dwc_ssi*) spi;

	if (flash == NULL) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	return flash->state->spi_freq;
}

int flash_master_dwc_ssi_set_spi_clock_frequency (const struct flash_master *spi, uint32_t freq)
{
	const struct flash_master_dwc_ssi *flash = (const struct flash_master_dwc_ssi*) spi;
	int spi_freq;

	if (flash == NULL) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	spi_freq = spi_dwc_ssi_get_supported_frequency (flash->spi, freq);
	if (ROT_IS_ERROR (spi_freq)) {
		return spi_freq;
	}

	flash->state->spi_freq = spi_freq;

	return spi_freq;
}

/**
 * Initialize the flash interface for a DesignWare SSI hardware block.
 *
 * @param flash The flash interface instance to initialize.
 * @param state Variable context for the flash interface.  This must be uninitialized.
 * @param spi The SPI master connected to the flash device.
 * @param chip_select Chip select number for the flash device on the SPI bus.
 *
 * @return 0 if the flash interface was initialized successfully or an error code.
 */
int flash_master_dwc_ssi_init (struct flash_master_dwc_ssi *flash,
	struct flash_master_dwc_ssi_state *state, const struct spi_dwc_ssi *spi, uint8_t chip_select)
{
	return flash_master_dwc_ssi_init_with_capabilities (flash, state, spi, chip_select,
		FLASH_MASTER_DWC_SSI_FULL_CAPABILITIES);
}

/**
 * Initialize the flash interface for a DesignWare SSI hardware block.  The capabilities advertised
 * by the flash master can be specified to control what modes of operation are available to the SPI
 * flash driver.
 *
 * @param flash The flash interface instance to initialize.
 * @param state Variable context for the flash interface.  This must be uninitialized.
 * @param spi The SPI master connected to the flash device.
 * @param chip_select Chip select number for the flash device on the SPI bus.
 * @param capabilities The bitmask of capabilities that should be advertised for this flash master.
 * These must be flags from the FLASH_CAP_* enum.  At least one of the address mode capability flags
 * needs to be set.
 *
 * @return 0 if the flash interface was initialized successfully or an error code.
 */
int flash_master_dwc_ssi_init_with_capabilities (struct flash_master_dwc_ssi *flash,
	struct flash_master_dwc_ssi_state *state, const struct spi_dwc_ssi *spi, uint8_t chip_select,
	uint32_t capabilities)
{
	if ((flash == NULL) || (spi == NULL)) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	memset (flash, 0, sizeof (struct flash_master_dwc_ssi));

	flash->base.xfer = flash_master_dwc_ssi_xfer;
	flash->base.capabilities = flash_master_dwc_ssi_capabilities;
	flash->base.get_spi_clock_frequency = flash_master_dwc_ssi_get_spi_clock_frequency;
	flash->base.set_spi_clock_frequency = flash_master_dwc_ssi_set_spi_clock_frequency;

	flash->state = state;
	flash->spi = spi;
	flash->chip_select = chip_select;
	flash->capabilities = (capabilities & FLASH_MASTER_DWC_SSI_FULL_CAPABILITIES);

#if DWC_SSI_AHB_SLAVE_SSIC_ADDRESS_BLOCK_SPI_CTRLR0_CLK_STRETCH_EN_WRITE_ACCESS == 0
	/* Only support Dual/Quad for read transfers without clock stretching. */
	if (flash->capabilities &
		(FLASH_CAP_DUAL_2_2_2 | FLASH_CAP_DUAL_1_2_2 | FLASH_CAP_DUAL_1_1_2)) {
		flash->capabilities |= FLASH_CAP_DUAL_RX_ONLY;
	}

	if (flash->capabilities &
		(FLASH_CAP_QUAD_4_4_4 | FLASH_CAP_QUAD_1_4_4 | FLASH_CAP_QUAD_1_1_4)) {
		flash->capabilities |= FLASH_CAP_QUAD_RX_ONLY;
	}
#endif

	return flash_master_dwc_ssi_init_state (flash);
}

/**
 * Initialize only the variable state of the SPI flash device instance.  The rest of the device
 * structure is assumed to have already been initialized.
 *
 * @param flash The flash interface that contains the state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int flash_master_dwc_ssi_init_state (const struct flash_master_dwc_ssi *flash)
{
	if ((flash == NULL) || (flash->state == NULL) || (flash->spi == NULL)) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	if ((flash->capabilities & (FLASH_CAP_3BYTE_ADDR | FLASH_CAP_4BYTE_ADDR)) == 0) {
		return FLASH_MASTER_INVALID_ARGUMENT;
	}

	memset (flash->state, 0, sizeof (struct flash_master_dwc_ssi_state));

	flash->state->spi_freq = spi_dwc_ssi_get_spi_frequency (flash->spi);

	return 0;
}

/**
 * Release the resources used by a DesignWare SSI flash interface.
 *
 * @param flash The flash interface instance to release.
 */
void flash_master_dwc_ssi_release (const struct flash_master_dwc_ssi *flash)
{
	UNUSED (flash);
}
