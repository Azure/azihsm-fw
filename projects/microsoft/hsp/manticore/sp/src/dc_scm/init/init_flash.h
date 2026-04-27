// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_FLASH_H_
#define INIT_FLASH_H_

#include <stdint.h>
#include "platform_config.h"
#include "flash/spi_flash_static.h"
#include "host_fw/host_flash_initialization_static.h"


/**
 * Frequency to use when communicating with the SPI flash devices.
 */
#ifdef BUILD_FOR_SIMULATION
/* Increase the clock frequency to reduce simulation time. */
#define	MANTICORE_FLASH_CLOCK_FREQUENCY_HZ				25000000
#define	MANTICORE_A0_BYPASS_FLASH_CLOCK_FREQUENCY_HZ	25000000

#elif defined BUILD_FOR_FPGA
/* The MPS3 FPGA clock is much slower, so reduce the SPI clock, too.  Use the same divider value as
 * the production scenario. */
#define	MANTICORE_FLASH_CLOCK_FREQUENCY_HZ				555556
#define	MANTICORE_A0_BYPASS_FLASH_CLOCK_FREQUENCY_HZ	555556

#elif defined BUILD_FOR_HAPS
/* The HAPS FPGA runs even slower than MPS3. */
#define	MANTICORE_FLASH_CLOCK_FREQUENCY_HZ				83334
#define	MANTICORE_A0_BYPASS_FLASH_CLOCK_FREQUENCY_HZ	83334

#else
/* The clock frequency used on the actual device. */
#define	MANTICORE_FLASH_CLOCK_FREQUENCY_HZ				46875000

/**
 * Account for the slower system clock when A0 Bypass is asserted by running the flash slower.
 */
#define	MANTICORE_A0_BYPASS_FLASH_CLOCK_FREQUENCY_HZ	833334

/**
 * Default frequency used for compatibility before opting to set a higher frequency.
 */
#define	MANTICORE_FLASH_DEFAULT_CLOCK_FREQUENCY_HZ		12500000
#endif

/**
 * Flash capabilities for the internal flash device.
 */
#define	MANTICORE_INTERNAL_FLASH_CAPABILITIES           \
	(FLASH_CAP_DUAL_1_1_2 | FLASH_CAP_QUAD_1_1_4 | FLASH_CAP_3BYTE_ADDR | FLASH_CAP_4BYTE_ADDR)

/**
 * Flash capabilities for the external flash device.
 *
 * TODO: Explore the reasons why 1-2-2 and 1-4-4 modes don't work against the external Winbond
 * flash.  Lack of PU on the I/O lines during mode byte?
 */
#define	MANTICORE_EXTERNAL_FLASH_CAPABILITIES           \
	(FLASH_CAP_DUAL_1_1_2 | FLASH_CAP_QUAD_1_1_4 | FLASH_CAP_3BYTE_ADDR | FLASH_CAP_4BYTE_ADDR)


extern const struct spi_flash flash_internal;
extern const struct spi_flash flash_external;

extern const struct spi_flash host_flash_cs0;
extern const struct spi_flash host_flash_cs1;
extern const struct host_flash_initialization host_flash_init;


uint32_t get_default_spi_flash_frequency ();
uint32_t get_maximum_spi_flash_frequency ();

int initialize_manticore_flash ();

void log_flash_device_info (const struct spi_flash *flash, uint8_t id);

int initialize_host_flash_access ();
int initialize_host_spi_frequency (uint32_t spi_freq);


#endif	/* INIT_FLASH_H_ */
