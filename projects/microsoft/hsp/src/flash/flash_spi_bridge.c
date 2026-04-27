// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <string.h>
#include "flash_spi_bridge.h"
#include "platform_api.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "flash/flash_master.h"
#include "mailbox/hsp_mailbox_register_bank.h"

/**
 * The SPI Bridge is an DesignWare Cores Synchronous Serial Interface (SSI) IP block.
 * The SPI Bridge converts SPI transactions into AHB transactions.
 *
 * Instruction (command), 8 bits.
 *
 *	7 6 5 4 3 2 1 0
 *  | | | | | | | +---- \                   000: 1    001: 4    010: 8   011: 16
 *	| | | | | | +------ |  Data length      100: 32   101: 64   110 and 111: reserved
 *	| | | | | +-------- /                   each frame size is 4 bytes
 *  | | | | |
 *  | | | | +---------- \ Read Wait cycles  if Transfer type 00, then 1, 2 or 3 wait cycles
 *  | | | +------------ / (if bit 7: 0)     if Transfer type 11, then 8, 16, 24 or 32 wait cycles
 *  | | |
 *  | | +-------------- \ Transfer type     00: Read/Write,        01: Read Request
 *  | +---------------- /                   10: Read/Write status, 11: Read/Write with dummy
 *  |
 *  +------------------ > Read/Write        0: Read, 1: Write
 *
 *	Address: 24-bit address.
 *
 *	Data is received or transmitted in frames of 32 bits.
 *
 * 	NOTE: Driver is configured to support the direct read approach.
 */


/**
 * DWC SSI SPI bridge command codes.
 */
enum {
	/* Write instructions */
	FLASH_SPI_BRIDGE_WRITE_1 = 0x80,			/**< Write operation to write 1 frame. */
	FLASH_SPI_BRIDGE_WRITE_4 = 0x81,			/**< Write operation to write 4 frames. */
	FLASH_SPI_BRIDGE_WRITE_8 = 0x82,			/**< Write operation to write 8 frames. */
	FLASH_SPI_BRIDGE_WRITE_16 = 0x83,			/**< Write operation to write 16 frames. */
	FLASH_SPI_BRIDGE_WRITE_32 = 0x84,			/**< Write operation to write 32 frames. */
	FLASH_SPI_BRIDGE_WRITE_64 = 0x85,			/**< Write operation to write 64 frames. */
	FLASH_SPI_BRIDGE_WRITE_RESERVED_1 = 0x86,	/**< Write operation, reserved. */
	FLASH_SPI_BRIDGE_WRITE_RESERVED_2 = 0x87,	/**< Write operation, reserved. */

	FLASH_SPI_BRIDGE_WRITESTATUS_0 = 0xc0,		/**< Write Data Status Instruction. */
	FLASH_SPI_BRIDGE_WRITESTATUS_1 = 0xc8,		/**< Write Data Status Instruction with 1 wait cycle. */
	FLASH_SPI_BRIDGE_WRITESTATUS_2 = 0xd0,		/**< Write Data Status Instruction with 2 wait cycles. */

	/**
	 * Read instructions:
	 *	- Direct read approach: READ instruction, address, wait cycles, data.
	 *	- 3 stage read approach:
	 *		1) READREQ instruction followed by address (4 bytes total).
	 *		2) READSTATUS instruction, wait on READY bit set.
	 *		3) READ instruction, wait cycles, data (NOTE: no address).
	 */

	/* These instructions are applicable for the three stage read approach. */
	FLASH_SPI_BRIDGE_READREQ_1 = 0x20,			/**< Read request, 1 frame. */
	FLASH_SPI_BRIDGE_READREQ_4 = 0x21,			/**< Read request, 4 frames. */
	FLASH_SPI_BRIDGE_READREQ_8 = 0x22,			/**< Read request, 8 frames. */
	FLASH_SPI_BRIDGE_READREQ_16 = 0x23,			/**< Read request, 16 frames. */
	FLASH_SPI_BRIDGE_READREQ_32 = 0x24,			/**< Read request, 32 frames. */
	FLASH_SPI_BRIDGE_READREQ_64 = 0x25,			/**< Read request, 64 frames. */

	/* These instructions are applicable for the three stage read approach. */
	FLASH_SPI_BRIDGE_READSTATUS_0 = 0x40,		/**< Read status, no wait cycles. */
	FLASH_SPI_BRIDGE_READSTATUS_1 = 0x48,		/**< Read status, with 1 wait cycle. */
	FLASH_SPI_BRIDGE_READSTATUS_2 = 0x50,		/**< Read status, with 2 wait cycles. */

	/* These instructions are applicable for the three stage read approach. */
	FLASH_SPI_BRIDGE_STAGE_READ_0_1 = 0x00,		/**< Read data, 1 frame, no wait cycles. */
	FLASH_SPI_BRIDGE_STAGE_READ_0_4 = 0x01,		/**< Read data, 4 frames, no wait cycles. */
	FLASH_SPI_BRIDGE_STAGE_READ_0_8 = 0x02,		/**< Read data, 8 frames, no wait cycles. */
	FLASH_SPI_BRIDGE_STAGE_READ_0_16 = 0x03,	/**< Read data, 16 frames, no wait cycles. */
	FLASH_SPI_BRIDGE_STAGE_READ_0_32 = 0x04,	/**< Read data, 32 frames, no wait cycles. */
	FLASH_SPI_BRIDGE_STAGE_READ_0_64 = 0x05,	/**< Read data, 64 frames, no wait cycles. */

	/* These instructions are applicable for three stage read approach */
	FLASH_SPI_BRIDGE_STAGE_READ_1_1 = 0x08,		/**< Read data, 1 frame, 1 wait cycle. */
	FLASH_SPI_BRIDGE_STAGE_READ_1_4 = 0x09,		/**< Read data, 4 frames, 1 wait cycle. */
	FLASH_SPI_BRIDGE_STAGE_READ_1_8 = 0x0a,		/**< Read data, 8 frames, 1 wait cycle. */
	FLASH_SPI_BRIDGE_STAGE_READ_1_16 = 0x0b,	/**< Read data, 16 frames, 1 wait cycle. */
	FLASH_SPI_BRIDGE_STAGE_READ_1_32 = 0x0c,	/**< Read data, 32 frames, 1 wait cycle. */
	FLASH_SPI_BRIDGE_STAGE_READ_1_64 = 0x0d,	/**< Read data, 64 frames, 1 wait cycle. */

	/* These instructions are applicable for the three stage read approach. */
	FLASH_SPI_BRIDGE_STAGE_READ_2_1 = 0x10,		/**< Read data, 1 frame, 2 wait cycles. */
	FLASH_SPI_BRIDGE_STAGE_READ_2_4 = 0x11,		/**< Read data, 4 frames, 2 wait cycles. */
	FLASH_SPI_BRIDGE_STAGE_READ_2_8 = 0x12,		/**< Read data, 8 frames, 2 wait cycles. */
	FLASH_SPI_BRIDGE_STAGE_READ_2_16 = 0x13,	/**< Read data, 16 frames, 2 wait cycles. */
	FLASH_SPI_BRIDGE_STAGE_READ_2_32 = 0x14,	/**< Read data, 32 frames, 2 wait cycles. */
	FLASH_SPI_BRIDGE_STAGE_READ_2_64 = 0x15,	/**< Read data, 64 frames, 2 wait cycles. */

	/* These instructions are applicable for the direct read approach. */
	FLASH_SPI_BRIDGE_DIRECT_READ_8_1 = 0x60,	/**< Read data, 1 frame, 8 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_8_4 = 0x61,	/**< Read data, 4 frames, 8 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_8_8 = 0x62,	/**< Read data, 8 frames, 8 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_8_16 = 0x63,	/**< Read data, 16 frames, 8 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_8_32 = 0x64,	/**< Read data, 32 frames, 8 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_8_64 = 0x65,	/**< Read data, 64 frames, 8 wait cycles. */

	/* These instructions are applicable for the direct read approach. */
	FLASH_SPI_BRIDGE_DIRECT_READ_16_1 = 0x68,	/**< Read data, 1 frame, 16 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_16_4 = 0x69,	/**< Read data, 4 frames, 16 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_16_8 = 0x6a,	/**< Read data, 8 frames, 16 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_16_16 = 0x6b,	/**< Read data, 16 frames, 16 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_16_32 = 0x6c,	/**< Read data, 32 frames, 16 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_16_64 = 0x6d,	/**< Read data, 64 frames, 16 wait cycles. */

	/* These instructions are applicable for the direct read approach. */
	FLASH_SPI_BRIDGE_DIRECT_READ_24_1 = 0x70,	/**< Read data, 1 frame, 24 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_24_4 = 0x71,	/**< Read data, 4 frames, 24 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_24_8 = 0x72,	/**< Read data, 8 frames, 24 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_24_16 = 0x73,	/**< Read data, 16 frames, 24 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_24_32 = 0x74,	/**< Read data, 32 frames, 24 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_24_64 = 0x75,	/**< Read data, 64 frames, 24 wait cycles. */

	/* These instructions are applicable for the direct read approach. */
	FLASH_SPI_BRIDGE_DIRECT_READ_32_1 = 0x78,	/**< Read data, 1 frame, 32 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_32_4 = 0x79,	/**< Read data, 4 frames, 32 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_32_8 = 0x7a,	/**< Read data, 8 frames, 32 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_32_16 = 0x7b,	/**< Read data, 16 frames, 32 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_32_32 = 0x7c,	/**< Read data, 32 frames, 32 wait cycles. */
	FLASH_SPI_BRIDGE_DIRECT_READ_32_64 = 0x7d,	/**< Read data, 64 frames, 32 wait cycles. */
};

/**
 * Status return codes.
 * Bit 7 is ready status.
 * Bit 6 is error status.
 * Bits 5..0 are reserved.
 */
enum {
	FLASH_SPI_BRIDGE_READY_MASK = 0x80,	/**< Bit 7. 0: not ready, 1: ready. */
	FLASH_SPI_BRIDGE_ERROR_MASK = 0x40,	/**< Bit 6. 0: no errors, 1: error. */
};

/**
 * SPI interface receives 24-bit address and sends to AHB interface.
 * Derived AHB word aligned address: SSIC_AHB_ADDR_OFFSET + {{SPI_ADDR},2'b00}.
 * Shift destination read/write address by 2, as the interface automatically word aligns.
 */
#define FLASH_SPI_BRIDGE_SPI_ADDR(addr) (addr >> 2)

/**
 * The SPI Bridge max frame size if 1 frame, 4 bytes total.
 */
#define FLASH_SPI_BRIDGE_MIN_FRAME_SIZE_BYTES 4

/**
 * The SPI bridge default read wait cycles.
 */
#define FLASH_SPI_BRIDGE_DEFAULT_DIRECT_READ_WAIT_CYCLES 24

/**
 * Calculate dummy bytes from direct read wait cycles for SPI bridge.
*/

#define FLASH_SPI_BRIDGE_GET_DUMMY_BYTES_FROM_WAIT_CYCLES_DIRECT_READ(wait_cycles,\
		divisor) (wait_cycles / divisor)


/* Forward declaration */
int flash_spi_bridge_read (const struct flash *flash, uint32_t address, uint8_t *data,
	size_t length);

/**
 * Choose SPI Bridge Direct Read command based on transaction length.
 *
 * @param length Transaction length in bytes.
 *
 * @return Command matching length, or 0 if the length is not supported.
 */
static uint8_t flash_spi_bridge_read_command_from_length (size_t length, uint32_t base_cmd)
{
	uint8_t cmd = 0;

	switch (length) {
		case 4:
			cmd = base_cmd;
			break;

		case 16:
			cmd = base_cmd + 1;
			break;

		case 32:
			cmd = base_cmd + 2;
			break;

		case 64:
			cmd = base_cmd + 3;
			break;

		case 128:
			cmd = base_cmd + 4;
			break;

		case 256:
			cmd = base_cmd + 5;
			break;
	}

	return cmd;
}

/**
 * Calculate how much data to read/write during one SPI Bridge transaction.
 *
 * @param length Data length in bytes.
 * @param max_length Maximum data length in bytes.
 *
 * @return One of the supported by commands data lengths.
 */
static size_t flash_spi_bridge_calculate_transaction_data_length (size_t length, size_t max_length)
{
	/* If length is greater then max supported length return max.
	 * Otherwise, choose smallest power of 2 size (except 8).
	 * NOTE: 8 is not supported by SPI Bridge commands. */
	if (length >= max_length) {
		return max_length;
	}

	/* Leave only one bit set - yields power of 2 size. */
	length = length & ~(length - 1);

	return (length == 8) ? FLASH_SPI_BRIDGE_MIN_FRAME_SIZE_BYTES : length;
}

/**
 * Reads the status information from the INSTS register to the supplied variables
 *
 * @param spi_bridge The spi bridge to use to read the status variables
 * @param address The address to be used to get the status variables
 * @param is_valid Optional pointer to variable in which to store the valid bit. Can be set to NULL if not needed.
 * @param is_err Optional pointer to variable in which to store the err bit. Can be set to NULL if not needed.
 * @param count Optional pointer to variable in which to store the FIFO count. Can be set to NULL if not needed.
 *
 * @return 0 if the read was successful or an error code.
 */
static int flash_spi_bridge_mailbox_get_status (const struct flash_spi_bridge *spi_bridge,
	const uint32_t address,	bool *is_valid, bool *is_err, uint32_t *count)
{
	uint32_t data;
	int status = flash_spi_bridge_read (&spi_bridge->base, address, (uint8_t*) &data,
		sizeof (data));

	if (status != 0) {
		return status;
	}

	if (is_valid != NULL) {
		*is_valid = HSP_MAILBOX_REGISTER_BANK_INSTS_FIFO_VALID_GET (data);
	}

	if (is_err != NULL) {
		*is_err = HSP_MAILBOX_REGISTER_BANK_INSTS_ERR_BIT_GET (data);
	}

	if (count != NULL) {
		*count = HSP_MAILBOX_REGISTER_BANK_INSTS_FIFO_CNT_GET (data);
	}

	return 0;
}

int flash_spi_bridge_read (const struct flash *flash, uint32_t address, uint8_t *data,
	size_t length)
{
	size_t remaining;
	size_t max_length;
	int status;
	const struct flash_spi_bridge *spi_bridge = (const struct flash_spi_bridge*) flash;

	if ((spi_bridge == NULL) || (data == NULL)) {
		return FLASH_INVALID_ARGUMENT;
	}

	/* The length must be a multiple of 4 (32 bits aligned for AHB bus access). */
	if ((length & 0x03) != 0) {
		return FLASH_READ_FAILED;
	}

	platform_mutex_lock (&spi_bridge->state->lock);

	max_length = spi_bridge->fifo_depth;
	remaining = length;
	status = 0;

	while ((status == 0) && remaining) {
		size_t read_len;
		uint8_t command;
		struct flash_xfer xfer;

		read_len = flash_spi_bridge_calculate_transaction_data_length (remaining, max_length);
		command = flash_spi_bridge_read_command_from_length (read_len,
			spi_bridge->state->read_base_command);

		FLASH_XFER_INIT_READ (xfer, command, FLASH_SPI_BRIDGE_SPI_ADDR (address),
			FLASH_SPI_BRIDGE_GET_DUMMY_BYTES_FROM_WAIT_CYCLES_DIRECT_READ (
			spi_bridge->state->read_wait_cycles, spi_bridge->mode.divisor), 0, data, read_len,
			spi_bridge->mode.dual_quad_mode);
		status = spi_bridge->spi->xfer (spi_bridge->spi, &xfer);
		if (status == 0) {
			remaining -= read_len;
			data += read_len;
			address += read_len;
		}
	}

	platform_mutex_unlock (&spi_bridge->state->lock);

	return status;
}

/**
 * Choose SPI Bridge Write command based on transaction length.
 * @param length Transaction length in bytes.
 *
 * @return Command matching length, or 0 if the length is not supported.
 */
static uint8_t flash_spi_bridge_write_command_from_length (size_t length)
{
	uint8_t cmd = 0;

	switch (length) {
		case 4:
			cmd = FLASH_SPI_BRIDGE_WRITE_1;
			break;

		case 16:
			cmd = FLASH_SPI_BRIDGE_WRITE_4;
			break;

		case 32:
			cmd = FLASH_SPI_BRIDGE_WRITE_8;
			break;

		case 64:
			cmd = FLASH_SPI_BRIDGE_WRITE_16;
			break;

		case 128:
			cmd = FLASH_SPI_BRIDGE_WRITE_32;
			break;

		case 256:
			cmd = FLASH_SPI_BRIDGE_WRITE_64;
			break;
	}

	return cmd;
}

int flash_spi_bridge_write (const struct flash *flash, uint32_t address, const uint8_t *data,
	size_t length)
{
	size_t remaining;
	size_t max_length;
	int status;
	const struct flash_spi_bridge *spi_bridge = (const struct flash_spi_bridge*) flash;

	if ((spi_bridge == NULL) || (data == NULL)) {
		return FLASH_INVALID_ARGUMENT;
	}

	/* The length must be a multiple of 4 (32 bits aligned for AHB bus access). */
	if ((length & 0x03) != 0) {
		return FLASH_WRITE_FAILED;
	}

	platform_mutex_lock (&spi_bridge->state->lock);

	max_length = spi_bridge->fifo_depth;
	remaining = length;
	status = 0;

	while ((status == 0) && remaining) {
		size_t write_len;
		uint8_t command;
		struct flash_xfer xfer;

		write_len = flash_spi_bridge_calculate_transaction_data_length (remaining, max_length);
		command = flash_spi_bridge_write_command_from_length (write_len);

		FLASH_XFER_INIT_WRITE (xfer, command, FLASH_SPI_BRIDGE_SPI_ADDR (address), 0,
			(uint8_t*) data, write_len,	spi_bridge->mode.dual_quad_mode | FLASH_FLAG_DATA_TX);
		status = spi_bridge->spi->xfer (spi_bridge->spi, &xfer);
		if (status != 0) {
			return status;
		}

		remaining -= write_len;
		data += write_len;
		address += write_len;
	}

	platform_mutex_unlock (&spi_bridge->state->lock);

	length = length - remaining;

	return length;
}

/**
 * Release the SPI Bridge interface.
 *
 * @param spi_bridge The SPI Bridge interface to release.
 */
void flash_spi_bridge_release (struct flash_spi_bridge *spi_bridge)
{
	if (spi_bridge) {
		platform_mutex_free (&spi_bridge->state->lock);
	}
}

int flash_spi_bridge_get_device_size (const struct flash *flash, uint32_t *bytes)
{
	const struct flash_spi_bridge *spi_bridge = (const struct flash_spi_bridge*) flash;

	if ((spi_bridge == NULL) || (bytes == NULL)) {
		return FLASH_INVALID_ARGUMENT;
	}

	*bytes = spi_bridge->device_size;

	return 0;
}

/* API handler for get_page_size, minimum_write_per_page, get_sector_size, and get_block_size when
 * statically initialized for read only access.
 * TODO: provide an implementation for all flash APIs if needed by run-time code. */
int flash_spi_bridge_get_size (const struct flash *flash, uint32_t *bytes)
{
	UNUSED (flash);
	UNUSED (bytes);

	return FLASH_PAGE_SIZE_FAILED;
}

/* API handler for sector_erase and block_erase when statically initialized for read only access.
 * TODO: provide an implementation for all flash APIs if needed by run-time code. */
int flash_spi_bridge_erase (const struct flash *flash, uint32_t addr)
{
	UNUSED (flash);
	UNUSED (addr);

	return FLASH_SECTOR_ERASE_FAILED;
}

/* API handler for chip_erase when statically initialized for read only access.
 * TODO: provide an implementation for all flash APIs if needed by run-time code. */
int flash_spi_bridge_chip_erase (const struct flash *flash)
{
	UNUSED (flash);

	return FLASH_CHIP_ERASE_FAILED;
}

int flash_spi_bridge_mailbox_send_fifo_push (const struct hsp_mailbox_interface *mailbox,
	uint32_t data)
{
	const struct flash_spi_bridge *spi_bridge = TO_DERIVED_TYPE (mailbox,
		const struct flash_spi_bridge, mbox_base);

	uint32_t address;
	int status;

	if (mailbox == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	address = spi_bridge->mbox_send_offset + HSP_MAILBOX_REGISTER_BANK_FIFO_PUSH_OFFSET;

	status = flash_spi_bridge_write (&spi_bridge->base, address, (uint8_t*) &data, sizeof (data));
	/* Check status for an error code */
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	return 0;
}

int flash_spi_bridge_mailbox_send_set_valid (const struct hsp_mailbox_interface *mailbox)
{
	const struct flash_spi_bridge *spi_bridge = TO_DERIVED_TYPE (mailbox,
		const struct flash_spi_bridge, mbox_base);
	uint32_t address;
	uint32_t data;
	int status;

	if (mailbox == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	address = spi_bridge->mbox_send_offset + HSP_MAILBOX_REGISTER_BANK_INSTS_OFFSET;
	data = HSP_MAILBOX_REGISTER_BANK_INSTS_FIFO_VALID_SET (1);

	status = flash_spi_bridge_write (&spi_bridge->base, address, (uint8_t*) &data, sizeof (data));
	/* Check status for an error code */
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	return 0;
}

int flash_spi_bridge_mailbox_send_set_err (const struct hsp_mailbox_interface *mailbox)
{
	const struct flash_spi_bridge *spi_bridge = TO_DERIVED_TYPE (mailbox,
		const struct flash_spi_bridge, mbox_base);
	uint32_t address;
	uint32_t data;
	int status;

	if (mailbox == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	address = spi_bridge->mbox_send_offset + HSP_MAILBOX_REGISTER_BANK_INSTS_OFFSET;
	data = HSP_MAILBOX_REGISTER_BANK_INSTS_ERR_BIT_SET (1);

	status = flash_spi_bridge_write (&spi_bridge->base, address, (uint8_t*) &data, sizeof (data));
	/* Check status for an error code */
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	return 0;
}

int flash_spi_bridge_mailbox_send_get_status (const struct hsp_mailbox_interface *mailbox,
	bool *is_valid, bool *is_err, uint32_t *count)
{
	const struct flash_spi_bridge *spi_bridge = TO_DERIVED_TYPE (mailbox,
		const struct flash_spi_bridge, mbox_base);
	uint32_t address;

	if (mailbox == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	address = spi_bridge->mbox_send_offset + HSP_MAILBOX_REGISTER_BANK_INSTS_OFFSET;

	return flash_spi_bridge_mailbox_get_status (spi_bridge, address, is_valid, is_err, count);
}

int flash_spi_bridge_mailbox_recv_fifo_pop (const struct hsp_mailbox_interface *mailbox,
	uint32_t *data)
{
	const struct flash_spi_bridge *spi_bridge = TO_DERIVED_TYPE (mailbox,
		const struct flash_spi_bridge, mbox_base);
	uint32_t address;

	if ((mailbox == NULL) || (data == NULL)) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	address = spi_bridge->mbox_recv_offset + HSP_MAILBOX_REGISTER_BANK_FIFO_POP_OFFSET;

	return flash_spi_bridge_read (&spi_bridge->base, address, (uint8_t*) data, sizeof (*data));
}

int flash_spi_bridge_mailbox_recv_get_status (const struct hsp_mailbox_interface *mailbox,
	bool *is_valid, bool *is_err, uint32_t *count)
{
	const struct flash_spi_bridge *spi_bridge = TO_DERIVED_TYPE (mailbox,
		const struct flash_spi_bridge, mbox_base);
	uint32_t address;

	if (mailbox == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	address = spi_bridge->mbox_recv_offset + HSP_MAILBOX_REGISTER_BANK_INSTS_OFFSET;

	return flash_spi_bridge_mailbox_get_status (spi_bridge, address, is_valid, is_err, count);
}

int flash_spi_bridge_mailbox_recv_clear_valid (const struct hsp_mailbox_interface *mailbox)
{
	const struct flash_spi_bridge *spi_bridge = TO_DERIVED_TYPE (mailbox,
		const struct flash_spi_bridge, mbox_base);
	uint32_t address;
	uint32_t data;
	int status;

	if (mailbox == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	address = spi_bridge->mbox_recv_offset + HSP_MAILBOX_REGISTER_BANK_INSTS_OFFSET;
	data = HSP_MAILBOX_REGISTER_BANK_INSTS_FIFO_VALID_SET (1);

	status = flash_spi_bridge_write (&spi_bridge->base, address, (uint8_t*) &data, sizeof (data));
	/* Check status for an error code */
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	return 0;
}

int flash_spi_bridge_mailbox_recv_clear_err (const struct hsp_mailbox_interface *mailbox)
{
	const struct flash_spi_bridge *spi_bridge = TO_DERIVED_TYPE (mailbox,
		const struct flash_spi_bridge, mbox_base);
	uint32_t address;
	uint32_t data;
	int status;

	if (mailbox == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	address = spi_bridge->mbox_recv_offset + HSP_MAILBOX_REGISTER_BANK_INSTS_OFFSET;
	data = HSP_MAILBOX_REGISTER_BANK_INSTS_ERR_BIT_SET (1);

	status = flash_spi_bridge_write (&spi_bridge->base, address, (uint8_t*) &data, sizeof (data));
	/* Check status for an error code */
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	return 0;
}

int flash_spi_bridge_mailbox_recv_enable_fifo_valid_interrupt (
	const struct hsp_mailbox_interface *mailbox, bool enabled)
{
	const struct flash_spi_bridge *spi_bridge = TO_DERIVED_TYPE (mailbox,
		const struct flash_spi_bridge, mbox_base);
	uint32_t address;
	uint32_t data;
	int status;

	if (mailbox == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	address = spi_bridge->mbox_recv_offset + HSP_MAILBOX_REGISTER_BANK_CTRL_OFFSET;

	status = flash_spi_bridge_read (&spi_bridge->base, address, (uint8_t*) &data, sizeof (data));
	if (status != 0) {
		return status;
	}

	data = HSP_MAILBOX_REGISTER_BANK_CTRL_FIFO_VALID_INT_EN_MODIFY (data, enabled);

	status = flash_spi_bridge_write (&spi_bridge->base, address, (uint8_t*) &data, sizeof (data));
	/* Check status for an error code */
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	return 0;
}

int flash_spi_bridge_mailbox_recv_enable_err_interrupt (const struct hsp_mailbox_interface *mailbox,
	bool enabled)
{
	const struct flash_spi_bridge *spi_bridge = TO_DERIVED_TYPE (mailbox,
		const struct flash_spi_bridge, mbox_base);
	uint32_t address;
	uint32_t data;
	int status;

	if (mailbox == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	address = spi_bridge->mbox_recv_offset + HSP_MAILBOX_REGISTER_BANK_CTRL_OFFSET;

	status = flash_spi_bridge_read (&spi_bridge->base, address, (uint8_t*) &data, sizeof (data));
	if (status != 0) {
		return status;
	}

	data = HSP_MAILBOX_REGISTER_BANK_CTRL_ERR_INT_EN_MODIFY (data, enabled);

	status = flash_spi_bridge_write (&spi_bridge->base, address, (uint8_t*) &data, sizeof (data));
	/* Check status for an error code */
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	return 0;
}

/**
 * Initialize only the variable state for a flash SPI bridge interface.
 * The rest of the interface is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param spi_bridge The flash interface that contains the state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 * NOTE: Configured to 24 direct read wait cycles by default. APIs provided
 * to support configurability.
 */
int flash_spi_bridge_init_state (const struct flash_spi_bridge *spi_bridge)
{
	if ((spi_bridge == NULL) || (spi_bridge->state == NULL) || (spi_bridge->spi == NULL)) {
		return FLASH_INVALID_ARGUMENT;
	}

	memset (spi_bridge->state, 0, sizeof (struct flash_spi_bridge_state));

	flash_spi_bridge_set_read_wait_cycles (spi_bridge,
		FLASH_SPI_BRIDGE_DEFAULT_DIRECT_READ_WAIT_CYCLES);

	return platform_mutex_init (&spi_bridge->state->lock);
}

/**
 * Initialize the SPI Bridge interface.
 *
 * @param spi_bridge The SPI Bridge interface to initialize.
 * @param state The variable context for a SPI Bridge driver instance.
 * @param spi The SPI flash master interface connected to the SPI Bridge.
 * @param mode Operating mode for the SPI bridge driver (Dual/Quad).
 * @param device_size The SPI Bridge device size.
 * @param fifo_depth The SPI Bridge FIFO depth.
 * @param send_offset The mailbox send offset within SPI bridge address space.
 * @param recv_offset The mailbox receive offset within SPI bridge address space.
 *
 * @return 0 if the SPI Bridge interface was initialized or an error code.
 */
int flash_spi_bridge_init (struct flash_spi_bridge *spi_bridge,
	struct flash_spi_bridge_state *state, const struct flash_master *spi, uint32_t mode,
	uint32_t device_size, uint32_t fifo_depth, uint32_t send_offset, uint32_t recv_offset)
{
	if ((spi_bridge == NULL) || (state == NULL) || (spi == NULL)) {
		return FLASH_INVALID_ARGUMENT;
	}

	if ((mode != FLASH_FLAG_DPI) && (mode != FLASH_FLAG_QPI)) {
		return FLASH_INVALID_ARGUMENT;
	}

	if ((fifo_depth & 0x03) != 0) {
		return FLASH_INVALID_ARGUMENT;
	}

	memset (spi_bridge, 0, sizeof (*spi_bridge));

	spi_bridge->base.get_device_size = flash_spi_bridge_get_device_size;
	spi_bridge->base.read = flash_spi_bridge_read;
	spi_bridge->base.get_page_size = flash_spi_bridge_get_size;
	spi_bridge->base.minimum_write_per_page = flash_spi_bridge_get_size;
	spi_bridge->base.write = flash_spi_bridge_write;
	spi_bridge->base.get_sector_size = flash_spi_bridge_get_size;
	spi_bridge->base.sector_erase = flash_spi_bridge_erase;
	spi_bridge->base.get_block_size = flash_spi_bridge_get_size;
	spi_bridge->base.block_erase = flash_spi_bridge_erase;
	spi_bridge->base.chip_erase = flash_spi_bridge_chip_erase;

	spi_bridge->mbox_base.send_fifo_push = flash_spi_bridge_mailbox_send_fifo_push;
	spi_bridge->mbox_base.send_set_valid = flash_spi_bridge_mailbox_send_set_valid;
	spi_bridge->mbox_base.send_set_err = flash_spi_bridge_mailbox_send_set_err;
	spi_bridge->mbox_base.send_get_status = flash_spi_bridge_mailbox_send_get_status;

	spi_bridge->mbox_base.recv_fifo_pop = flash_spi_bridge_mailbox_recv_fifo_pop;
	spi_bridge->mbox_base.recv_get_status = flash_spi_bridge_mailbox_recv_get_status;
	spi_bridge->mbox_base.recv_clear_valid = flash_spi_bridge_mailbox_recv_clear_valid;
	spi_bridge->mbox_base.recv_clear_err = flash_spi_bridge_mailbox_recv_clear_err;
	spi_bridge->mbox_base.recv_enable_fifo_valid_interrupt =
		flash_spi_bridge_mailbox_recv_enable_fifo_valid_interrupt;
	spi_bridge->mbox_base.recv_enable_err_interrupt =
		flash_spi_bridge_mailbox_recv_enable_err_interrupt;

	spi_bridge->spi = spi;
	spi_bridge->state = state;
	spi_bridge->mode.dual_quad_mode = mode;
	spi_bridge->mode.divisor = (mode == FLASH_FLAG_DPI) ? 4 : 2;
	spi_bridge->device_size = device_size;
	spi_bridge->fifo_depth = fifo_depth;

	spi_bridge->mbox_send_offset = send_offset;
	spi_bridge->mbox_recv_offset = recv_offset;

	return flash_spi_bridge_init_state (spi_bridge);
}

/**
 * Get the read wait cycles of the SPI bridge driver.
 *
 * @param spi_bridge The SPI bridge driver instance to query.
 *
 * @return The read wait cycles, or an error code. Use ROT_IS_ERROR to check the return
 * value.
 */
int flash_spi_bridge_get_read_wait_cycles (const struct flash_spi_bridge *spi_bridge)
{
	if (spi_bridge == NULL) {
		return FLASH_INVALID_ARGUMENT;
	}

	return spi_bridge->state->read_wait_cycles;
}

/**
 * Set the read wait cycles of the SPI bridge driver.
 *
 * @param spi_bridge The SPI bridge driver instance to set/modify the read wait cycles for.
 * @param read_wait_cycles The value used to program the read wait cycles for the SPI bridge.
 *
 * @return 0 if the read wait cycles was successfully set, or an error code.
 */
int flash_spi_bridge_set_read_wait_cycles (const struct flash_spi_bridge *spi_bridge,
	uint32_t read_wait_cycles)
{
	if (spi_bridge == NULL) {
		return FLASH_INVALID_ARGUMENT;
	}

	if ((read_wait_cycles != 8) && (read_wait_cycles != 16) && (read_wait_cycles != 24) &&
		(read_wait_cycles != 32)) {
		return FLASH_INVALID_ARGUMENT;
	}

	spi_bridge->state->read_wait_cycles = read_wait_cycles;

	switch (spi_bridge->state->read_wait_cycles) {
		case 8:
			spi_bridge->state->read_base_command = FLASH_SPI_BRIDGE_DIRECT_READ_8_1;
			break;

		case 16:
			spi_bridge->state->read_base_command = FLASH_SPI_BRIDGE_DIRECT_READ_16_1;
			break;

		case 24:
			spi_bridge->state->read_base_command = FLASH_SPI_BRIDGE_DIRECT_READ_24_1;
			break;

		case 32:
			spi_bridge->state->read_base_command = FLASH_SPI_BRIDGE_DIRECT_READ_32_1;
			break;
	}

	return 0;
}
