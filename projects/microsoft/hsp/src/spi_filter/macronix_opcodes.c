// Copyright (c) Microsoft Corporation. All rights reserved.

#include "spi_filter_hsp.h"
#include "common/array_size.h"


/**
 * SPI filter opcode configuration for Macronix MX25L devices with a capacity of no more than
 * 128Mbit.  These devices only support 3-byte address mode.
 */
const union spi_filter_hsp_opcode macronix_opcodes_mx25l128[] = {
	{
		.opcode = 0x02,	// 1-1-1 Page Program
		.action = SPI_FILTER_HSP_FILTER_STATE_WRADDR_1X1X1 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x03,	// 1-1-1 Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X1X1 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x04,	// Write Disable
		.action = SPI_FILTER_HSP_FILTER_STATE_WRITE_DIS | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x05,	// Read Status Register
		.action = SPI_FILTER_HSP_FILTER_STATE_INACTIVE | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x06,	// Write Enable
		.action = SPI_FILTER_HSP_FILTER_STATE_WRITE_EN | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x0b,	// 1-1-1 Fast Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X1X1 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x15,	// Read Configuration Register
		.action = SPI_FILTER_HSP_FILTER_STATE_ACTIVE | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x20,	// 4kB Erase
		.action = SPI_FILTER_HSP_FILTER_STATE_ERASEADDR_1X1 | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x38,	// 1-4-4 Page Program
		.action = SPI_FILTER_HSP_FILTER_STATE_WRADDR_1X4X4 | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x3b,	// 1-1-2 Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X1X2 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x52,	// 32kB Erase
		.action = SPI_FILTER_HSP_FILTER_STATE_ERASEADDR_1X1 | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x5a,	// Read SFDP REgister
		.action = SPI_FILTER_HSP_FILTER_STATE_ACTIVE | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x60,	// Chip Erase
		.action = SPI_FILTER_HSP_FILTER_STATE_CHIP_ERASE | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x66,	// Reset Enable
		.action = SPI_FILTER_HSP_FILTER_STATE_RESET_EN | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x6b,	// 1-1-4 Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X1X4 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x98,	// Gang Block Unlock
		.action = SPI_FILTER_HSP_FILTER_STATE_BOTH | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x99,	// Reset Device
		.action = SPI_FILTER_HSP_FILTER_STATE_RESET_DEV | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x9f,	// Read ID
		.action = SPI_FILTER_HSP_FILTER_STATE_ACTIVE | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0xbb,	// 1-2-2 Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X2X2 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0xc7,	// Chip Erase
		.action = SPI_FILTER_HSP_FILTER_STATE_CHIP_ERASE | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0xd8,	// 64kB Erase
		.action = SPI_FILTER_HSP_FILTER_STATE_ERASEADDR_1X1 | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0xeb,	// 1-4-4 Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X4X4 | SPI_FILTER_HSP_FILTER_BIT_8
	}
};

/**
 * The number of filtered opcodes for Macronix MX25L devices with a capacity of no more than
 * 128Mbit.
 */
const size_t macronix_opcodes_mx25l128_count = ARRAY_SIZE (macronix_opcodes_mx25l128);

/**
 * SPI filter opcode configuration for Macronix MX25L devices with a capacity of at least 256Mbit.
 * These devices support 3 and 4-byte address mode.
 */
const union spi_filter_hsp_opcode macronix_opcodes_mx25l256[] = {
	{
		.opcode = 0x02,	// 1-1-1 Page Program
		.action = SPI_FILTER_HSP_FILTER_STATE_WRADDR_1X1X1 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x03,	// 1-1-1 Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X1X1 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x04,	// Write Disable
		.action = SPI_FILTER_HSP_FILTER_STATE_WRITE_DIS | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x05,	// Read Status Register
		.action = SPI_FILTER_HSP_FILTER_STATE_INACTIVE | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x06,	// Write Enable
		.action = SPI_FILTER_HSP_FILTER_STATE_WRITE_EN | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x0b,	// 1-1-1 Fast Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X1X1 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x0c,	// 1-1-1 4B Fast Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR4B_1X1X1 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x12,	// 1-1-1 4B Page Program
		.action = SPI_FILTER_HSP_FILTER_STATE_WRADDR4B_1X1X1 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x13,	// 1-1-1 4B Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR4B_1X1X1 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x15,	// Read Configuration Register
		.action = SPI_FILTER_HSP_FILTER_STATE_ACTIVE | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x20,	// 4kB Erase
		.action = SPI_FILTER_HSP_FILTER_STATE_ERASEADDR_1X1 | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x21,	// 4kB 4B Erase
		.action = SPI_FILTER_HSP_FILTER_STATE_ERASEADDR4B_1X1 | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x38,	// 1-4-4 Page Program
		.action = SPI_FILTER_HSP_FILTER_STATE_WRADDR_1X4X4 | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x3b,	// 1-1-2 Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X1X2 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x3c,	// 1-1-2 4B Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR4B_1X1X2 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x3e,	// 1-4-4 4B Page Program
		.action = SPI_FILTER_HSP_FILTER_STATE_WRADDR4B_1X4X4 | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x52,	// 32kB Erase
		.action = SPI_FILTER_HSP_FILTER_STATE_ERASEADDR_1X1 | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x5a,	// Read SFDP REgister
		.action = SPI_FILTER_HSP_FILTER_STATE_ACTIVE | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x5c,	// 32kB 4B Erase
		.action = SPI_FILTER_HSP_FILTER_STATE_ERASEADDR4B_1X1 | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x60,	// Chip Erase
		.action = SPI_FILTER_HSP_FILTER_STATE_CHIP_ERASE | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x66,	// Reset Enable
		.action = SPI_FILTER_HSP_FILTER_STATE_RESET_EN | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x6b,	// 1-1-4 Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X1X4 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x6c,	// 1-1-4 4B Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR4B_1X1X4 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0x98,	// Gang Block Unlock
		.action = SPI_FILTER_HSP_FILTER_STATE_BOTH | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x99,	// Reset Device
		.action = SPI_FILTER_HSP_FILTER_STATE_RESET_DEV | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0x9f,	// Read ID
		.action = SPI_FILTER_HSP_FILTER_STATE_ACTIVE | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0xb7,	// Enter 4B Address Mode
		.action = SPI_FILTER_HSP_FILTER_STATE_BYTEMODE4 | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0xbb,	// 1-2-2 Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X2X2 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0xbc,	// 1-2-2 4B Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR4B_1X2X2 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0xc7,	// Chip Erase
		.action = SPI_FILTER_HSP_FILTER_STATE_CHIP_ERASE | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0xc8,	// Read Extended Address Register
		.action = SPI_FILTER_HSP_FILTER_STATE_ACTIVE | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0xd8,	// 64kB Erase
		.action = SPI_FILTER_HSP_FILTER_STATE_ERASEADDR_1X1 | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0xdc,	// 64kB 4B Erase
		.action = SPI_FILTER_HSP_FILTER_STATE_ERASEADDR4B_1X1 | SPI_FILTER_HSP_FILTER_BIT_7
	},
	{
		.opcode = 0xe9,	// Enter 3B Address Mode
		.action = SPI_FILTER_HSP_FILTER_STATE_BYTEMODE3 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0xeb,	// 1-4-4 Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X4X4 | SPI_FILTER_HSP_FILTER_BIT_8
	},
	{
		.opcode = 0xec,	// 1-4-4 4B Read
		.action = SPI_FILTER_HSP_FILTER_STATE_RDADDR4B_1X4X4 | SPI_FILTER_HSP_FILTER_BIT_8
	}
};

/**
 * The number of filtered opcodes for Macronix MX25L devices with a capacity of at least 256Mbit.
 */
const size_t macronix_opcodes_mx25l256_count = ARRAY_SIZE (macronix_opcodes_mx25l256);
