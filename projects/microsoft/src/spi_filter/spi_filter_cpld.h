// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SPI_FILTER_CPLD_H_
#define SPI_FILTER_CPLD_H_

#include <stdint.h>
#include "platform_api.h"
#include "i2c/i2c_master_interface.h"
#include "spi_filter/spi_filter_interface.h"


#define CPLD_MIN_FILTER_REGION							1
#define CPLD_NUM_FILTER_REGIONS							3
#define	CPLD_NUM_FILTER_REGIONS_EXTENDED				6

#define CPLD_FILTER_CTRL_ACV_CS_SHIFT					0
#define CPLD_FILTER_CTRL_ACV_CS_MASK					(1U << CPLD_FILTER_CTRL_ACV_CS_SHIFT)

#define CPLD_FILTER_CTRL_ADDR_BYTE_MODE_WE_SHIFT		1
#define CPLD_FILTER_CTRL_ADDR_BYTE_MODE_WE_MASK         \
		(1U << CPLD_FILTER_CTRL_ADDR_BYTE_MODE_WE_SHIFT)

#define CPLD_FILTER_CTRL_FIXED_ADDR_BYTE_MODE_SHIFT		2
#define CPLD_FILTER_CTRL_FIXED_ADDR_BYTE_MODE_MASK      \
		(1U << CPLD_FILTER_CTRL_FIXED_ADDR_BYTE_MODE_SHIFT)

#define CPLD_FILTER_CTRL_ADDR_SEL_SHIFT					3
#define CPLD_FILTER_CTRL_ADDR_SEL_MASK					(1U << CPLD_FILTER_CTRL_ADDR_SEL_SHIFT)

#define CPLD_FILTER_CTRL_ADDR_MODE_SHIFT				4
#define CPLD_FILTER_CTRL_ADDR_MODE_MASK					(1U << CPLD_FILTER_CTRL_ADDR_MODE_SHIFT)

#define CPLD_FILTER_CTRL_FLASH_DIRTY_SHIFT				5
#define CPLD_FILTER_CTRL_FLASH_DIRTY_MASK				(1U << CPLD_FILTER_CTRL_FLASH_DIRTY_SHIFT)

#define CPLD_FILTER_CTRL_ENABLE_SHIFT					6
#define CPLD_FILTER_CTRL_ENABLE_MASK					(1U << CPLD_FILTER_CTRL_ENABLE_SHIFT)

#define CPLD_FILTER_CTRL_RESET_ADDR_BYTE_MODE_SHIFT		7
#define CPLD_FILTER_CTRL_RESET_ADDR_BYTE_MODE_MASK      \
		(1U << CPLD_FILTER_CTRL_RESET_ADDR_BYTE_MODE_SHIFT)

#define CPLD_STATUS_FLASH_DIRTY_SHIFT					0
#define CPLD_STATUS_FLASH_DIRTY_MASK					(1U << CPLD_STATUS_FLASH_DIRTY_SHIFT)

#define CPLD_STATUS_ADDR_MODE_SHIFT						1
#define CPLD_STATUS_ADDR_MODE_MASK						(1U << CPLD_STATUS_ADDR_MODE_SHIFT)

#define CPLD_STATUS_WRITE_ENABLE_DETECTED_SHIFT			2
#define CPLD_STATUS_WRITE_ENABLE_DETECTED_MASK          \
		(1U << CPLD_STATUS_WRITE_ENABLE_DETECTED_SHIFT)

#define	CPLD_TEST_CTRL_BYPASS_P0_CS_SHIFT				0
#define	CPLD_TEST_CTRL_BYPASS_P0_CS_MASK				(1U << CPLD_TEST_CTRL_BYPASS_P0_CS_SHIFT)

#define CPLD_TEST_CTRL_BYPASS_P0_SHIFT					1
#define CPLD_TEST_CTRL_BYPASS_P0_MASK					(1U << CPLD_TEST_CTRL_BYPASS_P0_SHIFT)

#define	CPLD_TEST_CTRL_BYPASS_P1_CS_SHIFT				2
#define	CPLD_TEST_CTRL_BYPASS_P1_CS_MASK				(1U << CPLD_TEST_CTRL_BYPASS_P1_CS_SHIFT)

#define CPLD_TEST_CTRL_BYPASS_P1_SHIFT					3
#define CPLD_TEST_CTRL_BYPASS_P1_MASK					(1U << CPLD_TEST_CTRL_BYPASS_P1_SHIFT)

#define	CPLD_SINGLE_FLASH_MODE_SHIFT					0
#define	CPLD_SINGLE_FLASH_MODE_MASK						(1U << CPLD_SINGLE_FLASH_MODE_SHIFT)

#define	CPLD_SINGLE_FLASH_CS_SHIFT						1
#define	CPLD_SINGLE_FLASH_CS_MASK						(1U << CPLD_SINGLE_FLASH_CS_SHIFT)

#define	CPLD_SINGLE_FLASH_ALLOW_WRITE_SHIFT				2
#define	CPLD_SINGLE_FLASH_ALLOW_WRITE_MASK				(1U << CPLD_SINGLE_FLASH_ALLOW_WRITE_SHIFT)


#define	CPLD_INT_P0_HOST_CMD_MASK						(1U << 0)
#define	CPLD_INT_P1_HOST_CMD_MASK						(1U << 1)
#define	CPLD_INT_P0_DIRTY_MASK							(1U << 2)
#define	CPLD_INT_P1_DIRTY_MASK							(1U << 3)
#define	CPLD_INT_P0_BLOCK_MASK							(1U << 4)
#define	CPLD_INT_P1_BLOCK_MASK							(1U << 5)
#define	CPLD_INT_P0_ADDR_MODE_MASK						(1U << 6)
#define	CPLD_INT_P1_ADDR_MODE_MASK						(1U << 7)


/**
 * CPLD Register Map
 */
enum {
	CPLD_VERSION = 0,					/**< CPLD revision register */
	CPLD_FLASH_MFG_ID,					/**< CPLD flash manufacturing ID register */
	CPLD_P0_FILTER_CTRL,				/**< CPLD port 0 filter control register */
	CPLD_P1_FILTER_CTRL,				/**< CPLD port 1 filter control register */
	CPLD_SPI_INT,						/**< CPLD SPI interrupt register */
	CPLD_SPI_INT_MASK,					/**< CPLD SPI interrupt mask register */
	CPLD_P0_STATUS,						/**< CPLD port 0 status register */
	CPLD_P1_STATUS,						/**< CPLD port 1 status register */
	CPLD_TEST_CTRL,						/**< CPLD Test control register */
	CPLD_P0_FILTER_REGION_1_LOWER_MSB,	/**< CPLD port 0 address region 1 (lower MSBs) */
	CPLD_P0_FILTER_REGION_1_LOWER_LSB,	/**< CPLD port 0 address region 1 (lower LSBs) */
	CPLD_P0_FILTER_REGION_1_UPPER_MSB,	/**< CPLD port 0 address region 1 (upper MSBs) */
	CPLD_P0_FILTER_REGION_1_UPPER_LSB,	/**< CPLD port 0 address region 1 (upper LSBs) */
	CPLD_P0_FILTER_REGION_2_LOWER_MSB,	/**< CPLD port 0 address region 2 (lower MSBs) */
	CPLD_P0_FILTER_REGION_2_LOWER_LSB,	/**< CPLD port 0 address region 2 (lower LSBs) */
	CPLD_P0_FILTER_REGION_2_UPPER_MSB,	/**< CPLD port 0 address region 2 (upper MSBs) */
	CPLD_P0_FILTER_REGION_2_UPPER_LSB,	/**< CPLD port 0 address region 2 (upper LSBs) */
	CPLD_P0_FILTER_REGION_3_LOWER_MSB,	/**< CPLD port 0 address region 3 (lower MSBs) */
	CPLD_P0_FILTER_REGION_3_LOWER_LSB,	/**< CPLD port 0 address region 3 (lower LSBs) */
	CPLD_P0_FILTER_REGION_3_UPPER_MSB,	/**< CPLD port 0 address region 3 (upper MSBs) */
	CPLD_P0_FILTER_REGION_3_UPPER_LSB,	/**< CPLD port 0 address region 3 (upper LSBs) */
	CPLD_P1_FILTER_REGION_1_LOWER_MSB,	/**< CPLD port 1 address region 1 (lower MSBs) */
	CPLD_P1_FILTER_REGION_1_LOWER_LSB,	/**< CPLD port 1 address region 1 (lower LSBs) */
	CPLD_P1_FILTER_REGION_1_UPPER_MSB,	/**< CPLD port 1 address region 1 (upper MSBs) */
	CPLD_P1_FILTER_REGION_1_UPPER_LSB,	/**< CPLD port 1 address region 1 (upper LSBs) */
	CPLD_P1_FILTER_REGION_2_LOWER_MSB,	/**< CPLD port 1 address region 2 (lower MSBs) */
	CPLD_P1_FILTER_REGION_2_LOWER_LSB,	/**< CPLD port 1 address region 2 (lower LSBs) */
	CPLD_P1_FILTER_REGION_2_UPPER_MSB,	/**< CPLD port 1 address region 2 (upper MSBs) */
	CPLD_P1_FILTER_REGION_2_UPPER_LSB,	/**< CPLD port 1 address region 2 (upper LSBs) */
	CPLD_P1_FILTER_REGION_3_LOWER_MSB,	/**< CPLD port 1 address region 3 (lower MSBs) */
	CPLD_P1_FILTER_REGION_3_LOWER_LSB,	/**< CPLD port 1 address region 3 (lower LSBs) */
	CPLD_P1_FILTER_REGION_3_UPPER_MSB,	/**< CPLD port 1 address region 3 (upper MSBs) */
	CPLD_P1_FILTER_REGION_3_UPPER_LSB,	/**< CPLD port 1 address region 3 (upper LSBs) */
	CPLD_P0_BLOCKED_OPCODE,				/**< CPLD port 0 last blocked opcode. */
	CPLD_P1_BLOCKED_OPCODE,				/**< CPLD port 1 last blocked opcode. */
	CPLD_P0_MAX_ADDR_MASK_LOWER_LSB,	/**< CPLD port 0 max address mask (lower LSBs) */
	CPLD_P0_MAX_ADDR_MASK_UPPER_MSB,	/**< CPLD port 0 max address mask (upper MSBs) */
	CPLD_P1_MAX_ADDR_MASK_LOWER_LSB,	/**< CPLD port 1 max address mask (lower LSBs) */
	CPLD_P1_MAX_ADDR_MASK_UPPER_MSB,	/**< CPLD port 1 max address mask (upper MSBs) */
	CPLD_P0_FILTER_REGION_4_LOWER_MSB,	/**< CPLD port 0 address region 4 (lower MSBs) */
	CPLD_P0_FILTER_REGION_4_LOWER_LSB,	/**< CPLD port 0 address region 4 (lower LSBs) */
	CPLD_P0_FILTER_REGION_4_UPPER_MSB,	/**< CPLD port 0 address region 4 (upper MSBs) */
	CPLD_P0_FILTER_REGION_4_UPPER_LSB,	/**< CPLD port 0 address region 4 (upper LSBs) */
	CPLD_P0_FILTER_REGION_5_LOWER_MSB,	/**< CPLD port 0 address region 5 (lower MSBs) */
	CPLD_P0_FILTER_REGION_5_LOWER_LSB,	/**< CPLD port 0 address region 5 (lower LSBs) */
	CPLD_P0_FILTER_REGION_5_UPPER_MSB,	/**< CPLD port 0 address region 5 (upper MSBs) */
	CPLD_P0_FILTER_REGION_5_UPPER_LSB,	/**< CPLD port 0 address region 5 (upper LSBs) */
	CPLD_P0_FILTER_REGION_6_LOWER_MSB,	/**< CPLD port 0 address region 6 (lower MSBs) */
	CPLD_P0_FILTER_REGION_6_LOWER_LSB,	/**< CPLD port 0 address region 6 (lower LSBs) */
	CPLD_P0_FILTER_REGION_6_UPPER_MSB,	/**< CPLD port 0 address region 6 (upper MSBs) */
	CPLD_P0_FILTER_REGION_6_UPPER_LSB,	/**< CPLD port 0 address region 6 (upper LSBs) */
	CPLD_P1_FILTER_REGION_4_LOWER_MSB,	/**< CPLD port 1 address region 4 (lower MSBs) */
	CPLD_P1_FILTER_REGION_4_LOWER_LSB,	/**< CPLD port 1 address region 4 (lower LSBs) */
	CPLD_P1_FILTER_REGION_4_UPPER_MSB,	/**< CPLD port 1 address region 4 (upper MSBs) */
	CPLD_P1_FILTER_REGION_4_UPPER_LSB,	/**< CPLD port 1 address region 4 (upper LSBs) */
	CPLD_P1_FILTER_REGION_5_LOWER_MSB,	/**< CPLD port 1 address region 5 (lower MSBs) */
	CPLD_P1_FILTER_REGION_5_LOWER_LSB,	/**< CPLD port 1 address region 5 (lower LSBs) */
	CPLD_P1_FILTER_REGION_5_UPPER_MSB,	/**< CPLD port 1 address region 5 (upper MSBs) */
	CPLD_P1_FILTER_REGION_5_UPPER_LSB,	/**< CPLD port 1 address region 5 (upper LSBs) */
	CPLD_P1_FILTER_REGION_6_LOWER_MSB,	/**< CPLD port 1 address region 6 (lower MSBs) */
	CPLD_P1_FILTER_REGION_6_LOWER_LSB,	/**< CPLD port 1 address region 6 (lower LSBs) */
	CPLD_P1_FILTER_REGION_6_UPPER_MSB,	/**< CPLD port 1 address region 6 (upper MSBs) */
	CPLD_P1_FILTER_REGION_6_UPPER_LSB,	/**< CPLD port 1 address region 6 (upper LSBs) */
	CPLD_P0_SINGLE_FLASH_CTRL,			/**< CPLD port 0 single flash mode control register */
	CPLD_P1_SINGLE_FLASH_CTRL,			/**< CPLD port 1 single flash mode control register */
};

/**
 * CPLD Flash Ports
 */
typedef enum {
	CPLD_FLASH_PORT_0 = 0,	/**< CPLD flash port 0 */
	CPLD_FLASH_PORT_1,		/**< CPLD flash port 1 */
	NUM_CPLD_FLASH_PORTS,	/**< Number of CPLD flash ports */
} cpld_flash_port;

/**
 * CPLD Selection Modes
 */
typedef enum {
	CPLD_SEL_MODE_NORMAL = 0,	/**< Operation determined by Op Code commands */
	CPLD_SEL_MODE_OVERRIDE,		/**< Operation overridden by control register */
	NUM_CPLD_SEL_MODE,			/**< Number of CPLD selection modes */
} cpld_sel_mode;


/**
 * Control instance communicating with the CPLD that implements SPI filtering.
 */
struct spi_filter_cpld_control {
	struct i2c_master_interface *i2c;	/**< I2C device connected to the CPLD. */
	uint8_t slave_addr;					/**< The 7-bit I2C slave address of the cPLD. */
	platform_mutex lock;				/**< Synchronization for CPLD access. */
};

/**
 * A single SPI filter instance within the CPLD.
 */
struct spi_filter_cpld {
	struct spi_filter_interface base;			/**< SPI filter port instance */
	struct spi_filter_cpld_control *control;	/**< The CPLD that contains the SPI filter. */
	uint8_t port;								/**< Flash port number */
};


int spi_filter_cpld_control_init (struct spi_filter_cpld_control *cpld,
	struct i2c_master_interface *i2c, uint8_t slave_addr);
void spi_filter_cpld_control_deinit (struct spi_filter_cpld_control *cpld);

void spi_filter_cpld_block_register_access (struct spi_filter_cpld_control *cpld, bool block);

int spi_filter_cpld_get_version (struct spi_filter_cpld_control *cpld, uint8_t *version);
int spi_filter_cpld_get_int_status (struct spi_filter_cpld_control *cpld, uint8_t *int_status);
int spi_filter_cpld_get_int_mask (struct spi_filter_cpld_control *cpld, uint8_t *mask);
int spi_filter_cpld_set_int_mask (struct spi_filter_cpld_control *cpld, uint8_t mask);
int spi_filter_cpld_get_blocked_opcode (struct spi_filter_cpld_control *cpld, cpld_flash_port port,
	uint8_t *opcode);
int spi_filter_cpld_get_address_mode (struct spi_filter_cpld_control *cpld, cpld_flash_port port,
	spi_filter_address_mode *mode);

int spi_filter_cpld_init (struct spi_filter_cpld *filter, struct spi_filter_cpld_control *cpld,
	cpld_flash_port port);
void spi_filter_cpld_deinit (struct spi_filter_cpld *cpld);


#endif	/* SPI_FILTER_CPLD_H_ */
