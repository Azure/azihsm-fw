// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include "hsp_top.h"
#include "init_flash.h"
#include "init_system.h"
#include "common/array_size.h"
#include "drivers/spi_dwc_ssi_static.h"
#include "flash/flash_master_dwc_ssi_static.h"
#include "logging/init_logging.h"
#include "trap/hsp_interrupt.h"
#include "trap/hsp_interrupt_group_static.h"


/**
 * Variable context for the SPI master driver.
 */
static struct spi_dwc_ssi_state spi_master_context[2];

/**
 * Driver for the SPI masters connected to flash devices.
 */
static const struct spi_dwc_ssi spi_master[2] = {
	spi_dwc_ssi_static_init_interrupt (&spi_master_context[0],
		(struct DWC_ssi_AHB_Slave*) HSP_ADDR_MAP_CREG_SPI_SPI0_ADDRESS,
		(struct Ssi_regs*) HSP_ADDR_MAP_CREG_SPI_CREG_SSI_GROUP0_ADDRESS),
	spi_dwc_ssi_static_init_interrupt (&spi_master_context[1],
		(struct DWC_ssi_AHB_Slave*) HSP_ADDR_MAP_CREG_SPI_SPI2_ADDRESS,
		(struct Ssi_regs*) HSP_ADDR_MAP_CREG_SPI_CREG_SSI_GROUP2_ADDRESS)
};

/**
 * List of individual handlers to use for SPI interrupts.
 */
static const struct hsp_interrupt_handler *const spi_irq_group[] = {
	&spi_master[0].base, &spi_master[1].base
};

/**
 * Handler for the aggregated SPI interrupt from all SPI masters.
 */
static const struct hsp_interrupt_group spi_irq =
	hsp_interrupt_group_static_init (spi_irq_group, ARRAY_SIZE (spi_irq_group), false);

/**
 * Variable context for the flash connected to SPI0.
 */
static struct flash_master_dwc_ssi_state spi0_context[2];

/**
 * Device interface to the flash connected to SPI0.
 */
static const struct flash_master_dwc_ssi spi0[2] = {
	flash_master_dwc_ssi_static_init_with_capabilities (&spi0_context[0], &spi_master[0], 0,
		MANTICORE_INTERNAL_FLASH_CAPABILITIES),
	flash_master_dwc_ssi_static_init_with_capabilities (&spi0_context[1], &spi_master[0], 1,
		MANTICORE_EXTERNAL_FLASH_CAPABILITIES)
};

/**
 * Variable context for the flash connected to SPI2.
 */
static struct flash_master_dwc_ssi_state spi2_context[2];

/**
 * Device interface to the flash connected to SPI2.
 */
static const struct flash_master_dwc_ssi spi2[2] = {
	flash_master_dwc_ssi_static_init_with_capabilities (&spi2_context[0], &spi_master[1], 0,
		MANTICORE_EXTERNAL_FLASH_CAPABILITIES),
	flash_master_dwc_ssi_static_init_with_capabilities (&spi2_context[1], &spi_master[1], 1,
		MANTICORE_EXTERNAL_FLASH_CAPABILITIES)
};

/**
 * Variable context for the internal Manticore SPI flash device.
 */
static struct spi_flash_state flash_internal_context;

/**
 * Driver to communicate with internal Manticore SPI flash.
 */
const struct spi_flash flash_internal = spi_flash_static_init (SPI_FLASH_API_INIT,
	&flash_internal_context, &spi0[0].base);

/**
 * Variable context for the external Manticore SPI flash device.
 */
static struct spi_flash_state flash_external_context;

/**
 * Driver to communicate with external Manticore SPI flash.
 */
const struct spi_flash flash_external = spi_flash_static_init (SPI_FLASH_API_INIT,
	&flash_external_context, &spi0[1].base);

/**
 * Variable context for host flash connected to chip select 0.
 */
static struct spi_flash_state host_flash_cs0_context;

/**
 * Host flash device connected to chip select 0.
 */
const struct spi_flash host_flash_cs0 = spi_flash_static_init (SPI_FLASH_API_INIT,
	&host_flash_cs0_context, &spi2[0].base);

/**
 * Variable context for host flash connected to chip select 1.
 */
static struct spi_flash_state host_flash_cs1_context;

/**
 * Host flash device connected to chip select 1.
 */
const struct spi_flash host_flash_cs1 = spi_flash_static_init (SPI_FLASH_API_INIT,
	&host_flash_cs1_context, &spi2[1].base);

/**
 * Variable context for the host flash initialization manager.
 */
static struct host_flash_initialization_state host_flash_init_context;

/**
 * Initialization manager for host flash.
 */
const struct host_flash_initialization host_flash_init =
	host_flash_initialization_static_init (&host_flash_init_context, &host_flash_cs0,
	&host_flash_cs1, true, false);


/**
 * Determine the frequency that should be used for SPI transactions.
 *
 * @return The default SPI frequency.
 */
uint32_t get_default_spi_flash_frequency ()
{
	if (is_a0_bypass ()) {
		return MANTICORE_A0_BYPASS_FLASH_CLOCK_FREQUENCY_HZ;
	}
	else {
		return MANTICORE_FLASH_DEFAULT_CLOCK_FREQUENCY_HZ;
	}
}

/**
 * Determine the maximum frequency that can be used for SPI transactions.
 *
 * @return The maximum SPI frequency.
 */
uint32_t get_maximum_spi_flash_frequency ()
{
	if (is_a0_bypass ()) {
		return MANTICORE_A0_BYPASS_FLASH_CLOCK_FREQUENCY_HZ;
	}
	else {
		return MANTICORE_FLASH_CLOCK_FREQUENCY_HZ;
	}
}

/**
 * Initialize all SPI HW blocks and configure SPI interrupts.
 *
 * @return 0 if the SPI HW was successfully initialize or an error code.
 */
static int initialize_spi_hw ()
{
	size_t i;
	int status;

	for (i = 0; i < ARRAY_SIZE (spi_master); i++) {
		status = spi_dwc_ssi_init_state (&spi_master[i], HSP_CLOCK_FREQUENCY_HZ,
			get_default_spi_flash_frequency ());
		if (status != 0) {
			return status;
		}
	}

	status = hsp_interrupt_register (CREG_REGS_INT_HSP_IRQINTEN_SPI_INTEN_MSB, &spi_irq.base);
	if (status != 0) {
		return status;
	}

	status = hsp_interrupt_enable (CREG_REGS_INT_HSP_IRQINTEN_SPI_INTEN_MSB,
		HSP_INTERRUPT_IRQ_LEVEL_IRQ);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Initialize the interfaces to Manticore SPI flash, connected to SPI0.  CS0 is the in-package flash
 * device and CS1 is external.
 *
 * @return 0 if the flash was successfully initialized or an error code.
 */
int initialize_manticore_flash ()
{
	int status;

	status = initialize_spi_hw ();
	if (status != 0) {
		return status;
	}

	status = flash_master_dwc_ssi_init_state (&spi0[0]);
	if (status != 0) {
		return status;
	}

	status = spi0[0].base.set_spi_clock_frequency (&spi0[0].base,
		get_maximum_spi_flash_frequency ());
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	status = flash_master_dwc_ssi_init_state (&spi0[1]);
	if (status != 0) {
		return status;
	}

	status = spi0[1].base.set_spi_clock_frequency (&spi0[1].base,
		get_maximum_spi_flash_frequency ());
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	status = spi_flash_initialize_device_state (&flash_internal, true, false, SPI_FLASH_RESET_NONE,
		false);
	if (status != 0) {
		return status;
	}

	status = spi_flash_initialize_device_state (&flash_external, true, false, SPI_FLASH_RESET_NONE,
		false);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Log information about a specified SPI flash device.
 *
 * @param flash The SPI flash to log.
 * @param id ID for the device being logged.
 */
void log_flash_device_info (const struct spi_flash *flash, uint8_t id)
{
	uint8_t vendor = 0;
	uint16_t device = 0;
	uint32_t capacity = 0;

	spi_flash_get_device_id (flash, &vendor, &device);
	spi_flash_get_device_size (flash, &capacity);

	debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_INIT,
		INIT_LOGGING_FLASH_DEVICE, ((id << 24) | (vendor << 16) | device), capacity);
}

/**
 * Initialize access to the flash connected to an external host processor.
 *
 * @return 0 if flash access was successfully initialized or an error code.
 */
int initialize_host_flash_access ()
{
	int status;

	status = flash_master_dwc_ssi_init_state (&spi2[0]);
	if (status != 0) {
		return status;
	}

	status = flash_master_dwc_ssi_init_state (&spi2[1]);
	if (status != 0) {
		return status;
	}

	status = host_flash_initialization_init_state (&host_flash_init);
	if (status != 0) {
		return status;
	}

	if (reset_source == RESET_POR) {
		status = host_flash_initialization_initialize_flash (&host_flash_init);
		if (status == 0) {
			log_flash_device_info (&host_flash_cs0, INIT_FLASH_PORT1_CS0);
			log_flash_device_info (&host_flash_cs1, INIT_FLASH_PORT1_CS1);
		}
		else {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
				INIT_LOGGING_HOST_FLASH_INIT, 1, status);

			/* On flash initialization failures, allow the system to continue booting.  The
			 * initialization manager will be used to retry flash initialization before accessing
			 * the devices.  If initialization continues to fail, the host processor manager will
			 * handle that error. */
			status = 0;
		}
	}

	return status;
}

/**
 * Initialize the SPI clock frequency for host flash.  If a frequency higher than the maximum is
 * requested, the maximum SPI frequency will be set.
 *
 * @param spi_freq The SPI bus frequency to configure.
 *
 * @return 0 if the SPI frequency was set successfully or an error code.
 */
int initialize_host_spi_frequency (uint32_t spi_freq)
{
	uint32_t max = get_maximum_spi_flash_frequency ();
	int status;
	int i;

	if (spi_freq > max) {
		spi_freq = max;
	}

	for (i = 0; i < 2; i++) {
		status = spi2[i].base.set_spi_clock_frequency (&spi2[i].base, spi_freq);
		if (ROT_IS_ERROR (status)) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_INIT,
				INIT_LOGGING_HOST_SPI_FREQ, status, 1);

			return status;
		}
	}

	return 0;
}
