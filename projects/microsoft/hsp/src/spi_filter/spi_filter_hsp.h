// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SPI_FILTER_HSP_H_
#define SPI_FILTER_HSP_H_

#include <stddef.h>
#include <stdint.h>
#include "platform_api.h"
#include "spi_filter/spi_filter_interface.h"


struct Creg_regs_spi_filter_regs;	/* Defined in HSP register definition. */

/**
 * Definition for a SPI flash opcode that should trigger some action in the SPI filter.
 */
union spi_filter_hsp_opcode {
	struct {
		uint8_t action;		/**< The action to take when the specified opcode is sent to flash.
								This is a combination of an target filter state and filter bit. */
		uint8_t opcode;		/**< The SPI flash command code to filter. */
		uint16_t reserved;	/**< Unused bits.  Set to 0. */
	};

	uint32_t reg_value;		/**< The value that will be written to the filter hardware register. */
};


// *INDENT-OFF*
/**
 * Filter states that can be used when defining an action to take for a SPI flash opcode.
 */
enum {
	/**
	 * Dual Flash:  Take no action.
	 * Single Flash:  Take no action.
	 */
	SPI_FILTER_HSP_FILTER_STATE_NULL = 0x0,

	/**
	 * Dual Flash:  Send the command to both flash devices.
	 * Single Flash:  Send the command to the flash device.
	 */
	SPI_FILTER_HSP_FILTER_STATE_BOTH = 0x0,

	/**
	 * Dual Flash:  Send the command to the active flash device (RO flash).
	 * Single Flash:  Send the command to the flash device.
	 */
	SPI_FILTER_HSP_FILTER_STATE_ACTIVE = 0x1,

	/**
	 * Dual Flash:  Block the command from being sent to either flash device.
	 * Single Flash:  Block the command from being sent to the flash device.
	 */
	SPI_FILTER_HSP_FILTER_STATE_BLOCK = 0x2,

	/**
	 * Dual Flash:  Send the command to the inactive flash device (RW flash).
	 * Single Flash:  Send the command to the flash device.
	 */
	SPI_FILTER_HSP_FILTER_STATE_INACTIVE = 0x3,

	/**
	 * A 1-1-1 SPI read command.  The address byte mode of the command is determined based on filter
	 * state.  This command will not be filtered on the 7th bit.
	 *
	 * Dual Flash:  The read command is sent to the active (RO) or inactive (RW) flash based on the
	 * 				defined filtered regions.
	 * Single Flash:  The read command is sent to the flash device.
	 */
	SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X1X1 = 0x4,

	/**
	 * A 1-1-2 SPI read command.  Behavior is the same as 1-1-1 read.
	 */
	SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X1X2 = 0x5,

	/**
	 * A 1-1-4 SPI read command.  Behavior is the same as 1-1-1 read.
	 */
	SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X1X4 = 0x6,

	/**
	 * A 1-2-2 SPI read command.  Behavior is the same as 1-1-1 read except that the address bytes
	 * are read in as dual SPI.
	 */
	SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X2X2 = 0x7,

	/**
	 * A 1-4-4 SPI read command.  Behavior is the same as 1-1-1 read except that the address bytes
	 * are read in as quad SPI.
	 */
	SPI_FILTER_HSP_FILTER_STATE_RDADDR_1X4X4 = 0x8,

	/**
	 * A 4-byte 1-1-1 SPI read command.  The address is always sent using 4 bytes.  This command
	 * will not be filtered on the 7th bit.
	 *
	 * Dual Flash:  The read command is sent to the active (RO) or inactive (RW) flash based on the
	 * 				defined filtered regions.
	 * Single Flash:  The read command is sent to the flash device.
	 */
	SPI_FILTER_HSP_FILTER_STATE_RDADDR4B_1X1X1 = 0x9,

	/**
	 * A 4-byte 1-1-2 SPI read command.  Behavior is the same as 4-byte 1-1-1 read.
	 */
	SPI_FILTER_HSP_FILTER_STATE_RDADDR4B_1X1X2 = 0xa,

	/**
	 * A 4-byte 1-1-4 SPI read command.  Behavior is the same as 4-byte 1-1-1 read.
	 */
	SPI_FILTER_HSP_FILTER_STATE_RDADDR4B_1X1X4 = 0xb,

	/**
	 * A 4-byte 1-2-2 SPI read command.  Behavior is the same as 4-byte 1-1-1 read except that the
	 * address bytes are read in as dual SPI.
	 */
	SPI_FILTER_HSP_FILTER_STATE_RDADDR4B_1X2X2 = 0xc,

	/**
	 * A 4-byte 1-4-4 SPI read command.  Behavior is the same as 4-byte 1-1-1 read except that the
	 * address bytes are read in as quad SPI.
	 */
	SPI_FILTER_HSP_FILTER_STATE_RDADDR4B_1X4X4 = 0xd,

	/**
	 * A 1-1-1 SPI page program command.  The address byte mode of the command is determined based
	 * on filter state.  The dirty interrupt is triggered based on Write Enable status and the
	 * defined filtered regions.  The Write Enable status is cleared.
	 *
	 * Dual Flash:  The write command is sent to the inactive (RW) flash.
	 * Single Flash:  The write command is sent to the flash device for address in the filtered
	 * 					regions.  Other addresses are blocked unless the filter is configured to
	 * 					allow single flash writes, in which case, the command will be sent to the
	 * 					flash device.
	 */
	SPI_FILTER_HSP_FILTER_STATE_WRADDR_1X1X1 = 0xe,

	/**
	 * A 1-1-2 SPI page program command.  Behavior is the same as 1-1-1 page program.
	 */
	SPI_FILTER_HSP_FILTER_STATE_WRADDR_1X1X2 = 0xf,

	/**
	 * A 1-1-4 SPI page program command.  Behavior is the same as 1-1-1 page program.
	 */
	SPI_FILTER_HSP_FILTER_STATE_WRADDR_1X1X4 = 0x10,

	/**
	 * A 1-2-2 SPI page program command.  Behavior is the same as 1-1-1 page program except that the
	 * address bytes are read in as dual SPI.
	 */
	SPI_FILTER_HSP_FILTER_STATE_WRADDR_1X2X2 = 0x11,

	/**
	 * A 1-4-4 SPI page program command.  Behavior is the same as 1-1-1 page program except that the
	 * address bytes are read in as quad SPI.
	 */
	SPI_FILTER_HSP_FILTER_STATE_WRADDR_1X4X4 = 0x12,

	/**
	 * A 4-byte 1-1-1 SPI page program command.  The address is always sent using 4 bytes.  The
	 * dirty interrupt is triggered based on Write Enable status and the defined filtered regions.
	 * The Write Enable status is cleared.
	 *
	 * Dual Flash:  The write command is sent to the inactive (RW) flash.
	 * Single Flash:  The write command is sent to the flash device for address in the filtered
	 * 					regions.  Other addresses are blocked unless the filter is configured to
	 * 					allow single flash writes, in which case, the command will be sent to the
	 * 					flash device.
	 */
	SPI_FILTER_HSP_FILTER_STATE_WRADDR4B_1X1X1 = 0x13,

	/**
	 * A 4-byte 1-1-2 SPI page program command.  Behavior is the same as 4-byte 1-1-1 page program.
	 */
	SPI_FILTER_HSP_FILTER_STATE_WRADDR4B_1X1X2 = 0x14,

	/**
	 * A 4-byte 1-1-4 SPI page program command.  Behavior is the same as 4-byte 1-1-1 page program.
	 */
	SPI_FILTER_HSP_FILTER_STATE_WRADDR4B_1X1X4 = 0x15,

	/**
	 * A 4-byte 1-2-2 SPI page program command.  Behavior is the same as 4-byte 1-1-1 page program
	 * except that the address bytes are read in as dual SPI.
	 */
	SPI_FILTER_HSP_FILTER_STATE_WRADDR4B_1X2X2 = 0x16,

	/**
	 * A 4-byte 1-4-4 SPI page program command.  Behavior is the same a 4-byte 1-1-1 page program
	 * except that the address bytes are read in as quad SPI.
	 */
	SPI_FILTER_HSP_FILTER_STATE_WRADDR4B_1X4X4 = 0x17,

	/**
	 * The host alert filter IRQ is asserted.
	 *
	 * Dual Flash:  The command is blocked from both flash devices.
	 * Single Flash:  The command is blocked from the flash device.
	 */
	SPI_FILTER_HSP_FILTER_STATE_HOSTALERTIRQ = 0x18,

	/**
	 * Updates the internal filter state to 3-byte address mode.  If Write Enable is required, the
	 * switch is contingent on on that being set.
	 *
	 * Dual Flash:  The command is sent to both flash devices.  If the filter is operating in
	 * 				permanent byte address mode, the command is blocked.
	 * Single Flash:  The command is sent to the flash device.  If the filter is operating in
	 * 					permanent byte address mode, the command is blocked.
	 */
	SPI_FILTER_HSP_FILTER_STATE_BYTEMODE3 = 0x19,

	/**
	 * Updates the internal filter state to 4-byte address mode.  If Write Enable is required, the
	 * switch is contingent on on that being set.
	 *
	 * Dual Flash:  The command is sent to both flash devices.  If the filter is operating in
	 * 				permanent byte address mode, the command is blocked.
	 * Single Flash:  The command is sent to the flash device.  If the filter is operating in
	 * 					permanent byte address mode, the command is blocked.
	 */
	SPI_FILTER_HSP_FILTER_STATE_BYTEMODE4 = 0x1a,

	/**
	 * Updates the internal state to understand flash soft reset has been enabled.  This state is
	 * cleared if the next opcode is not the RESET_DEV command.
	 *
	 * Dual Flash:  The command is sent to both flash devices.
	 * Single Flash:  The command is sent to the flash device.
	 */
	SPI_FILTER_HSP_FILTER_STATE_RESET_EN = 0x1b,

	/**
	 * Update the filter state in response to a flash device reset.  This only happens if RESET_EN
	 * was sent immediately prior to this command.
	 *
	 * Dual Flash:  The command is sent to both flash devices.
	 * Single Flash:  The command is sent to the flash device.
	 */
	SPI_FILTER_HSP_FILTER_STATE_RESET_DEV = 0x1c,

	/**
	 * Execute a chip erase of the entire flash device.  The dirty interrupt is asserted if Write
	 * Enable was also set.  This is always filtered after 7 bits.
	 *
	 * Dual Flash:  The command is sent to the inactive (RW) flash device.
	 * Single Flash:  The command is blocked from the flash device.  If the filter is configured to
	 * 					allow flash writes, the command is sent to the flash device.
	 */
	SPI_FILTER_HSP_FILTER_STATE_CHIP_ERASE = 0x1d,

	/**
	 * Update the Write Enable status tracking to disabled.
	 *
	 * Dual Flash:  The command is sent to the inactive (RW) flash device.  If Write Enable is
	 * 				required for address mode changes, the command is sent to both flash devices.
	 * Single Flash:  The command is sent to the flash device.
	 */
	SPI_FILTER_HSP_FILTER_STATE_WRITE_DIS = 0x1e,

	/**
	 * Update the Write Enable status tracking to enabled.
	 *
	 * Dual Flash:  The command is sent to the inactive (RW) flash device.  If Write Enable is
	 * 				required for address mode changes, the command is sent to both flash devices.
	 * Single Flash:  The command is sent to the flash device.
	 */
	SPI_FILTER_HSP_FILTER_STATE_WRITE_EN = 0x1f,

	/**
	 * A 1-1-0 SPI erase command.  Behavior is the same as 1-1-1 page program.
	 */
	SPI_FILTER_HSP_FILTER_STATE_ERASEADDR_1X1 = 0x20,

	/**
	 * A 1-2-0 SPI erase command.  Behavior is the same as 1-2-2 page program.
	 */
	SPI_FILTER_HSP_FILTER_STATE_ERASEADDR_1X2 = 0x21,

	/**
	 * A 1-4-0 SPI erase command.  Behavior is the same as 1-4-4 page program.
	 */
	SPI_FILTER_HSP_FILTER_STATE_ERASEADDR_1X4 = 0x22,

	/**
	 * A 4-byte 1-1-0 SPI erase command.  Behavior is the same as 4-byte 1-1-1 page program.
	 */
	SPI_FILTER_HSP_FILTER_STATE_ERASEADDR4B_1X1 = 0x23,

	/**
	 * A 4-byte 1-2-0 SPI erase command.  Behavior is the same as 4-byte 1-2-2 page program.
	 */
	SPI_FILTER_HSP_FILTER_STATE_ERASEADDR4B_1X2 = 0x24,

	/**
	 * A 4-byte 1-4-0 SPI erase command.  Behavior is the same as 4-byte 1-4-4 page program.
	 */
	SPI_FILTER_HSP_FILTER_STATE_ERASEADDR4B_1X4 = 0x25
};
// *INDENT-ON*

/**
 * Filter configuration indicating on which bit the opcode filter will take place.
 */
enum {
	SPI_FILTER_HSP_FILTER_BIT_7 = 0,			/**< Opcode filtering happens after 7 bits. */
	SPI_FILTER_HSP_FILTER_BIT_8 = (1u << 6),	/**< Opcode filtering happens after 8 bits. */
};


/**
 * Variable context for the SPI filter driver.
 */
struct spi_filter_hsp_state {
	uint32_t bypass_mode;	/**< Flag to track when bypass mode has been set. */
	platform_mutex lock;	/**< Lock for synchronizing HW access. */
};

/**
 * Driver for a single SPI filter contained with HSP.
 */
struct spi_filter_hsp {
	struct spi_filter_interface base;	/**< Base SPI filter API. */

	/**
	 * Read the interrupt status for the SPI filter.
	 *
	 * Reading the interrupts will clear any active interrupts.
	 *
	 * @param filter The SPI filter to query.
	 *
	 * @return Raw value from the interrupt status register.
	 */
	uint32_t (*get_interrupt_status) (const struct spi_filter_hsp *filter);

	/**
	 * Get the bit mask for the interrupts enabled for the SPI filter.
	 *
	 * @param filter The SPI filter to query.
	 *
	 * @return Raw value from from the interrupt enable register.
	 */
	uint32_t (*get_interrupt_enable) (const struct spi_filter_hsp *filter);

	/**
	 * Set the bit mask for the interrupts to enable for the SPI filter.
	 *
	 * @param filter The SPI filter to configure.
	 */
	void (*set_interrupt_enable) (const struct spi_filter_hsp *filter, uint32_t enable);

	/**
	 * Get the current value of the blocked SPI opcode register.  This corresponds to the opcode
	 * that generated a blocked opcode interrupt.
	 *
	 * Reading this register will allow additional blocked opcodes to be reported.
	 *
	 * @param filter The SPI filter to query.
	 *
	 * @return The opcode blocked by the SPI filter.
	 */
	uint8_t (*get_blocked_opcode) (const struct spi_filter_hsp *filter);

	/**
	 * Set the opcodes that should be processed by the filter.  Each opcode will identify the
	 * command code to look for, the bit at which filtering should happen, and the action that
	 * should be taken by the filter.
	 *
	 * Any opcode not defined for a specific action will be blocked.
	 *
	 * @param filter The SPI filter to configure.
	 * @param opcode_list A list of opcodes that should be processed by the filter.  If this is
	 * null, all configured opcodes will be cleared.
	 * @param count The number of opcodes in the list.  If this is 0, all configured opcodes will be
	 * cleared.
	 *
	 * @return 0 if all opcodes were successfully configured or an error code.
	 */
	int (*set_filtered_opcodes) (const struct spi_filter_hsp *filter,
		const union spi_filter_hsp_opcode *opcode_list, size_t count);

	struct spi_filter_hsp_state *state;		/**< Variable context for the driver. */
	struct Creg_regs_spi_filter_regs *regs;	/**< Register interface for the SPI filter. */
	uint8_t port;							/**< Port number for the SPI filter instance. */
};


int spi_filter_hsp_init (struct spi_filter_hsp *filter, struct spi_filter_hsp_state *state,
	struct Creg_regs_spi_filter_regs *regs, uint8_t port);
int spi_filter_hsp_init_state (const struct spi_filter_hsp *filter);
void spi_filter_hsp_release (const struct spi_filter_hsp *filter);


#endif	/* SPI_FILTER_HSP_H_ */
