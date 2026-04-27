// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FLASH_SPI_BRIDGE_STATIC_H_
#define FLASH_SPI_BRIDGE_STATIC_H_

#include "flash/flash_spi_bridge.h"

/* Internal functions declared to allow for static initialization. */
int flash_spi_bridge_get_device_size (const struct flash *flash, uint32_t *bytes);
int flash_spi_bridge_get_size (const struct flash *flash, uint32_t *bytes);
int flash_spi_bridge_erase (const struct flash *flash, uint32_t addr);
int flash_spi_bridge_chip_erase (const struct flash *flash);
int flash_spi_bridge_read (const struct flash *flash, uint32_t address, uint8_t *data,
	size_t length);
int flash_spi_bridge_write (const struct flash *flash, uint32_t address, const uint8_t *data,
	size_t length);
int flash_spi_bridge_mailbox_send_fifo_push (const struct hsp_mailbox_interface *mailbox,
	uint32_t data);
int flash_spi_bridge_mailbox_send_set_valid (const struct hsp_mailbox_interface *mailbox);
int flash_spi_bridge_mailbox_send_set_err (const struct hsp_mailbox_interface *mailbox);
int flash_spi_bridge_mailbox_send_get_status (const struct hsp_mailbox_interface *mailbox,
	bool *is_valid, bool *is_err, uint32_t *count);
int flash_spi_bridge_mailbox_recv_fifo_pop (const struct hsp_mailbox_interface *mailbox,
	uint32_t *data);
int flash_spi_bridge_mailbox_recv_get_status (const struct hsp_mailbox_interface *mailbox,
	bool *is_valid, bool *is_err, uint32_t *count);
int flash_spi_bridge_mailbox_recv_clear_valid (const struct hsp_mailbox_interface *mailbox);
int flash_spi_bridge_mailbox_recv_clear_err (const struct hsp_mailbox_interface *mailbox);
int flash_spi_bridge_mailbox_recv_enable_fifo_valid_interrupt (
	const struct hsp_mailbox_interface *mailbox, bool enabled);
int flash_spi_bridge_mailbox_recv_enable_err_interrupt (const struct hsp_mailbox_interface *mailbox,
	bool enabled);

/**
 * Constant initializer for the SPI Bridge API.
 */
#define FLASH_SPI_BRIDGE_API_INIT \
	{ \
		.get_device_size = flash_spi_bridge_get_device_size, \
		.read = flash_spi_bridge_read, \
		.get_page_size = flash_spi_bridge_get_size, \
		.minimum_write_per_page = flash_spi_bridge_get_size, \
		.write = flash_spi_bridge_write, \
		.get_sector_size = flash_spi_bridge_get_size, \
		.sector_erase = flash_spi_bridge_erase, \
		.get_block_size = flash_spi_bridge_get_size, \
		.block_erase = flash_spi_bridge_erase, \
		.chip_erase = flash_spi_bridge_chip_erase, \
	}

/**
 * Constant initializer for the SPI Bridge mailbox API.
 */
#define FLASH_SPI_BRIDGE_MAILBOX_API_INIT \
	{ \
		.send_fifo_push = flash_spi_bridge_mailbox_send_fifo_push, \
		.send_set_valid = flash_spi_bridge_mailbox_send_set_valid, \
		.send_set_err = flash_spi_bridge_mailbox_send_set_err, \
		.send_get_status = flash_spi_bridge_mailbox_send_get_status, \
		.recv_fifo_pop = flash_spi_bridge_mailbox_recv_fifo_pop, \
		.recv_get_status = flash_spi_bridge_mailbox_recv_get_status, \
		.recv_clear_valid = flash_spi_bridge_mailbox_recv_clear_valid, \
		.recv_clear_err = flash_spi_bridge_mailbox_recv_clear_err, \
		.recv_enable_fifo_valid_interrupt = flash_spi_bridge_mailbox_recv_enable_fifo_valid_interrupt, \
		.recv_enable_err_interrupt = flash_spi_bridge_mailbox_recv_enable_err_interrupt, \
	}

/**
 * Initialize operating mode for the SPI bridge.
 */
#define FLASH_SPI_BRIDGE_MODE_INIT(mode) \
	{ \
		.dual_quad_mode = mode, \
		.divisor = (mode == FLASH_FLAG_DPI) ? 4 : 2, \
	}

/**
 * Initialize a static instance of a SPI Bridge device interface.
 *
 * There is no validation done on the arguments, with the exception of
 * fifo depth, and SPI mode.
 *
 * @param state_ptr Variable context for the SPI Bridge interface.
 * @param spi_ptr The SPI flash master interface connected to the SPI Bridge.
 * @param spi_mode Operating mode for the SPI bridge driver (Dual/Quad).
 * @param device_size The flash SPI Bridge device size.
 * @param depth The SPI Bridge FIFO depth.
 * @param send_offset The mailbox send offset within SPI bridge address space.
 * @param recv_offset The mailbox receive offset within SPI bridge address space.
 */
#define flash_spi_bridge_static_init(state_ptr, spi_ptr, spi_mode, size, depth, send_offset, recv_offset) \
	{ \
		.base = FLASH_SPI_BRIDGE_API_INIT, \
		.mbox_base = FLASH_SPI_BRIDGE_MAILBOX_API_INIT, \
		.mode = FLASH_SPI_BRIDGE_MODE_INIT(spi_mode), \
		.state = state_ptr, \
		.spi = spi_ptr, \
		.device_size = size, \
		.fifo_depth = depth, \
		.mbox_send_offset = send_offset, \
		.mbox_recv_offset = recv_offset, \
	}; \
	_Static_assert((depth & 0x3) == 0, "FIFO depth must be a multiple of 4"); \
	_Static_assert((spi_mode == FLASH_FLAG_DPI) || (spi_mode == FLASH_FLAG_QPI), "SPI mode must be 2-2-2 or 4-4-4"); \



#endif	/* FLASH_SPI_BRIDGE_STATIC_H_ */
