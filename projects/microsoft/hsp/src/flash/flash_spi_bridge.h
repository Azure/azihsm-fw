// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FLASH_SPI_BRIDGE_H_
#define FLASH_SPI_BRIDGE_H_

#include "platform_api.h"
#include "flash/flash.h"
#include "flash/flash_master.h"
#include "mailbox/hsp_mailbox_interface.h"

/**
 * Variable context for a SPI Bridge driver instance.
 */
struct flash_spi_bridge_state {
	platform_mutex lock;		/**< Synchronization lock for accessing the SPI Bridge. */
	uint32_t read_wait_cycles;	/**< Read wait cycles. */
	uint32_t read_base_command;	/**< Base read command. */
};

/**
 * SPI Bridge operating mode.
 */
struct flash_spi_bridge_mode {
	uint32_t dual_quad_mode;	/**< Dual/Quad mode for the SPI Bridge (4-4-4 or 2-2-2).*/
	uint32_t divisor;			/**< Dummy byte divisor for the mode. */
};

/**
 * Flash SPI bridge instance that inherits both the functionality of a SPI flash device as
 * well as a HSP mailbox interface. The HSP mailbox interface will utilize the SPI flash interface
 * as its underlying read/write mechanism.
 */
struct flash_spi_bridge {
	struct flash base;						/**< Base flash instance. */
	struct hsp_mailbox_interface mbox_base;	/**< Base mailbox instance. */
	struct flash_spi_bridge_state *state;	/**< Variable context for the flash instance. */
	const struct flash_master *spi;			/**< The SPI master connected to the flash device. */
	struct flash_spi_bridge_mode mode;		/**< Operating mode of the SPI bridge. */
	uint32_t device_size;					/**< The total capacity of the flash device. */
	uint32_t fifo_depth;					/**< The fifo depth in bytes. */
	uint32_t mbox_send_offset;				/**< The address offset of the outgoing mailbox registers. */
	uint32_t mbox_recv_offset;				/**< The address offset of the incoming mailbox registers. */
};


int flash_spi_bridge_init (struct flash_spi_bridge *spi_bridge,
	struct flash_spi_bridge_state *state, const struct flash_master *spi, uint32_t mode,
	uint32_t device_size, uint32_t fifo_depth, uint32_t send_offset, uint32_t recv_offset);
int flash_spi_bridge_init_state (const struct flash_spi_bridge *spi_bridge);
void flash_spi_bridge_release (struct flash_spi_bridge *flash);

int flash_spi_bridge_get_read_wait_cycles (const struct flash_spi_bridge *spi_bridge);
int flash_spi_bridge_set_read_wait_cycles (const struct flash_spi_bridge *spi_bridge,
	uint32_t read_wait_cycles);


#endif	/* FLASH_SPI_BRIDGE_H_*/
