// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "platform_config.h"
#include "common/unused.h"
#include "drivers/i2c_dw_apb_error.h"
#include "drivers/i2c_dw_apb_static.h"
#include "trap/hsp_trap.h"


/**
 * State to track the cycle of I2C transaction.
 */
enum i2c_dw_apb_irq_state {
	I2C_DW_APB_IRQ_STATE_IDLE,		/**< No interrupt operation took place. */
	I2C_DW_APB_IRQ_STATE_BUSY,		/**< An operation took place within the current transaction. */
	I2C_DW_APB_IRQ_STATE_COMPLETE,	/**< STOP condition detected that ended the transaction. */
};

/**
 * State to track data being received.
 */
enum i2c_dw_apb_rx_state {
	I2C_DW_APB_RX_STATE_NO_DATA,	/**< There is no data in the Rx FIFO. */
	I2C_DW_APB_RX_STATE_NO_BUFFER,	/**< There is data in the Rx FIFO but no buffer is available to receive into. */
	I2C_DW_APB_RX_STATE_FILLED,		/**< There is data in the Rx FIFO but the buffer was maxed out. */
};

/**
 * State to track the data being transmitted.
 */
enum i2c_dw_apb_tx_state {
	I2C_DW_APB_TX_STATE_NO_DATA,		/**< There is no data ready to send. */
	I2C_DW_APB_TX_STATE_TRANSMITTING,	/**< Tx FIFO is filled but there is more data to send. */
	I2C_DW_APB_TX_STATE_DEPLETED,		/**< All contents of the buffer was pushed to the Tx FIFO. */
};


/**
 * Friendly interrupt bit specifier.
 */
#define I2C_DW_APB_INTR(intr) \
	CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_INTR_MASK_M_ ## intr ## _FIELD_MASK

/**
 * Initial HW interrupt mask to detect START condition.
 */
#define I2C_DW_APB_INTR_IDLE \
	I2C_DW_APB_INTR (START_DET)

/**
 * Interrupt mask to detect the end of a current transaction.
 */
#define I2C_DW_APB_INTR_END ( \
	I2C_DW_APB_INTR (RESTART_DET) | I2C_DW_APB_INTR (STOP_DET))

/**
 * Interrupt mask when a SLAVE mode transaction has started.
 */
#define I2C_DW_APB_INTR_SLAVE_START ( \
	I2C_DW_APB_INTR (RX_FULL) | I2C_DW_APB_INTR (RD_REQ) | I2C_DW_APB_INTR_END)

/**
 * Interrupt mask to handle reading data out of the Rx FIFO.
 */
#define I2C_DW_APB_INTR_RX_HANDLER ( \
	I2C_DW_APB_INTR (RX_FULL) | I2C_DW_APB_INTR_END)

/**
 * Interrupt mask to handle a valid currently executing Rx transaction.
 */
#define I2C_DW_APB_INTR_RX_ACTIVE ( \
	I2C_DW_APB_INTR (RX_OVER) | I2C_DW_APB_INTR_RX_HANDLER)

/**
 * Interrupt mask to handle pushing data into the Tx FIFO.
 */
#define I2C_DW_APB_INTR_TX_HANDLER ( \
	I2C_DW_APB_INTR (TX_EMPTY) | I2C_DW_APB_INTR_END)

/**
 * Interrupt mask to handle timeouts when servicing read requests.
 */
#define I2C_DW_APB_INTR_SCL_STUCK ( \
	I2C_DW_APB_INTR (RD_REQ) | I2C_DW_APB_INTR_TX_HANDLER)

/**
 * Interrupt mask to monitor data transmission status.
 */
#define I2C_DW_APB_INTR_TX_MONITOR ( \
	I2C_DW_APB_INTR (TX_ABRT) | I2C_DW_APB_INTR_END)

/**
 * Interrupt mask to stall the bus when no data is immediately ready to service a read request.
 */
#define I2C_DW_APB_INTR_TX_STALL ( \
	I2C_DW_APB_INTR (SCL_STUCK_AT_LOW) | I2C_DW_APB_INTR_TX_MONITOR)

/**
 * Interrupt mask to retry a stalled read request.
 */
#define I2C_DW_APB_INTR_TRY_RD_REQ ( \
	I2C_DW_APB_INTR (RD_REQ) | I2C_DW_APB_INTR_END)

/**
 * Interrupt mask to handle an active data transmission.
 */
#define I2C_DW_APB_INTR_TX_ACTIVE ( \
	I2C_DW_APB_INTR (TX_EMPTY) | I2C_DW_APB_INTR_TX_MONITOR)

/**
 * Interrupt mask to monitor for additional read requests after one has already been serviced.
 */
#define I2C_DW_APB_INTR_SLAVE_TX_IDLE ( \
	I2C_DW_APB_INTR (RD_REQ) | I2C_DW_APB_INTR_TX_MONITOR)

/**
 * Friendly interrupt request bit specifier.
 */
#define I2C_DW_APB_IRQ(irq) \
	CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_RAW_INTR_STAT_ ## irq ## _FIELD_MASK

/**
 * Interrupt requests that signal the end of a transaction.
 */
#define I2C_DW_APB_IRQ_TXN_END ( \
	I2C_DW_APB_IRQ (STOP_DET) | I2C_DW_APB_IRQ (RESTART_DET))

/**
 * Friendly HW status bit specifier.
 */
#define I2C_DW_APB_STATUS(field) \
	CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_STATUS_ ## field ## _FIELD_MASK

/**
 * Friendly CMD bit specifier.
 */
#define I2C_DW_APB_CMD(field) \
	CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_DATA_CMD_ ## field ## _FIELD_MASK

/**
 * The STOP bit of the CMD register.
 */
#define I2C_DW_APB_CMD_STOP \
	I2C_DW_APB_CMD (RSVD_STOP)

/**
 * Friendly HW enable bit specifier.
 */
#define I2C_DW_APB_ENABLE(field) \
	CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_ENABLE_ ## field ## _FIELD_MASK

/**
 * Friendly HW control bit specifier.
 */
#define I2C_DW_APB_CON(field) \
	CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_CON_ ## field ## _FIELD_MASK

/**
 * Control mask that configures the HW for MASTER mode operations.
 */
#define I2C_DW_APB_CON_MASTER_MODE ( \
	I2C_DW_APB_CON (IC_SLAVE_DISABLE) | I2C_DW_APB_CON (MASTER_MODE))

/**
 * Friendly HW enabled status bit specifier.
 */
#define I2C_DW_APB_ENABLED(field) \
	CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_ENABLE_STATUS_ ## field ## _FIELD_MASK

/**
 * Convert a time value of a specified interval to a count of clocks.
 *
 * @param tm The time interval.
 * @param clk The ic_clk frequency in Hz.
 * @param div The interval division.
 */
#define I2C_DW_APB_TIME_TO_IC_CLK(tm, clk, div) \
	((((tm) * (uint64_t) (clk)) + ((div) - 1)) / (div))

/**
 * Convert a time value in nanoseconds to a count of clocks.
 *
 * @param ns The time in nanoseconds.
 * @param clk The ic_clk frequency in Hz.
 */
#define	I2C_DW_APB_NS_TO_IC_CLK(ns, clk)    \
	I2C_DW_APB_TIME_TO_IC_CLK (ns, clk, 1000000000ULL)

/**
 * Convert a time value in milliseconds to a count of clocks.
 *
 * @param ms The time in milliseconds.
 * @param clk The ic_clk frequency in Hz.
 */
#define I2C_DW_APB_MS_TO_IC_CLK(ms, clk)    \
	I2C_DW_APB_TIME_TO_IC_CLK (ms, clk, 1000ULL)

/**
 * Default padding data used for transmissions.
 */
static const uint8_t I2C_DW_APB_TX_DEFAULT_PADDING_BYTE = 0xff;

/**
 * A global buffer used to discard an Rx transaction.
 */
static uint8_t rx_discard_buffer;


/* Utilities */

/**
 * Writes the clock cycles calculated from one of the I2C_DW_APB_*_TO_IC_CLK macros to a register,
 * ensuring that it's within the register bounds.
 *
 * @param clocks_reg The register to write to.
 * @param clocks The number of clock ticks to assign to the register.
 *
 * @return 0 if success, else an error code.
 */
static int i2c_dw_apb_set_clocks_counter (volatile uint32_t *clocks_reg, uint64_t clocks)
{
	if (clocks > UINT32_MAX) {
		return I2C_DW_APB_INVALID_CLOCK_VALUE;
	}

	*clocks_reg = (uint32_t) clocks;

	return 0;
}

/**
 * Validate parameters for setting an I/O buffer.
 *
 * @param i2c The I2C driver instance.
 * @param data A pointer to a byte buffer.
 * @param len The length of the buffer.
 *
 * @return 0 if successful, else an error code.
 */
static int i2c_dw_apb_validate_buffer (const struct i2c_dw_apb *i2c, const uint8_t *data,
	size_t len)
{
	if ((i2c == NULL) || ((data == NULL) && (len > 0))) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	return 0;
}

/**
 * Validates the range of an I2C address.
 *
 * @param address The I2C address.
 *
 * @return 0 if successful, else an error code.
 */
static int i2c_dw_apb_validate_address (uint16_t address)
{
	/* Do not currently support 10-bit address */
	if (address > 0x7F) {
		return I2C_DW_APB_INVALID_ADDRESS;
	}

	return 0;
}

/**
 * Gets the currently configured slave address for the HW.
 *
 * @param i2c The I2C driver instance.
 *
 * @return The slave address of the device.
 */
static uint16_t i2c_dw_apb_read_slave_address (const struct i2c_dw_apb *i2c)
{
	return (uint16_t) CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_SAR_IC_SAR_GET (
		i2c->regs->DW_apb_i2c_addr_block1.IC_SAR);
}

/**
 * Gets the address configured for the MASTER mode operation.
 *
 * @param i2c The I2C driver instance.
 *
 * @return The address of the target device.
 */
static uint16_t i2c_dw_apb_read_target_address (const struct i2c_dw_apb *i2c)
{
	return (uint16_t) CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_TAR_IC_TAR_GET (
		i2c->regs->DW_apb_i2c_addr_block1.IC_TAR);
}

/**
 * Read a register to clear an I2C interrupt.
 *
 * @param clr_reg The register that should be read.
 *
 * @return The value of the register.
 */
static uint32_t i2c_dw_apb_clr_interrupt (volatile uint32_t *clr_reg)
{
	return *clr_reg;
}

/**
 * Updates the interrupt mask and clears a specified interrupt.
 *
 * @param i2c The I2C driver instance.
 * @param intr_mask The interrupt mask to apply.
 * @param clr_reg The interrupt to clear.
 */
static void i2c_dw_apb_update_clr_interrupt (const struct i2c_dw_apb *i2c, uint32_t intr_mask,
	volatile uint32_t *clr_reg)
{
	i2c->regs->DW_apb_i2c_addr_block1.IC_INTR_MASK = intr_mask;
	i2c_dw_apb_clr_interrupt (clr_reg);
}

/**
 * Updates the interrupt mask and reads the interrupt status.
 *
 * @param i2c The I2C driver instance.
 * @param intr_mask The interrupt mask to apply.
 *
 * @return The updated interrupt status.
 */
static uint32_t i2c_dw_apb_update_intr_state (const struct i2c_dw_apb *i2c, uint32_t intr_mask)
{
	i2c->regs->DW_apb_i2c_addr_block1.IC_INTR_MASK = intr_mask;

	return i2c->regs->DW_apb_i2c_addr_block1.IC_INTR_STAT;
}

/**
 * Resets the state of the I/O buffers.
 *
 * @param state The I2C driver state.
 */
static void i2c_dw_apb_rst_txn_buffers (struct i2c_dw_apb_state *state)
{
	// Don't clear rx_pos.  Driver will need it to complete the current Rx buffer.
	state->rx_len = 0;
	state->rx_buffer = NULL;
	state->tx_len = 0;
	state->tx_data = NULL;
}

/**
 * Enables the I2C HW and begins processing interrupts.
 *
 * @param i2c The I2C driver instance.
 */
static void i2c_dw_apb_enable_hw (const struct i2c_dw_apb *i2c)
{
	i2c->regs->DW_apb_i2c_addr_block1.IC_ENABLE |= I2C_DW_APB_ENABLE (ENABLE);
}

/**
 * Disables the HW block and initializes the internal driver state.
 *
 * This does not call the driver on_shutdown callback.
 *
 * @param i2c The I2C driver instance.
 *
 * @return 0 if successful, else an error code.
 */
static int i2c_dw_apb_disable_hw (const struct i2c_dw_apb *i2c)
{
	int status;
	platform_clock timeout;

	/* Disable the I2C hardware.  This can be done at any point, but the driver must wait for the
	 * HW state machine to gracefully exit before proceeding. */
	i2c->regs->DW_apb_i2c_addr_block1.IC_ENABLE &= ~I2C_DW_APB_ENABLE (ENABLE);

	/* Wait for the HW to report that the block has been disabled and that all interrupts have been
	 * serviced. */
	status = platform_init_timeout (1000, &timeout);
	if (status != 0) {
		return status;
	}

	while (i2c->regs->DW_apb_i2c_addr_block1.IC_ENABLE_STATUS & I2C_DW_APB_ENABLED (IC_EN)) {
		if (platform_has_timeout_expired (&timeout)) {
			return I2C_DW_APB_HW_ERROR;
		}
	}

	i2c->regs->DW_apb_i2c_addr_block1.IC_INTR_MASK = 0;

	i2c->state->rx_pos = 0;
	i2c_dw_apb_rst_txn_buffers (i2c->state);

	return 0;
}

/**
 * Configures the I2C HW for a MASTER mode operation.
 *
 * @param i2c The I2C driver instance.
 */
static void i2c_dw_apb_set_master_mode (const struct i2c_dw_apb *i2c)
{
	i2c->regs->DW_apb_i2c_addr_block1.IC_CON |= I2C_DW_APB_CON_MASTER_MODE;
}

/**
 * Sets the target address for a MASTER mode operations.
 *
 * @param i2c The I2C driver instance.
 * @param address The target address for the operation.
 *
 * @return 0 if success, else an error code.
 */
static int i2c_dw_apb_set_target_address (const struct i2c_dw_apb *i2c, uint16_t address)
{
	int status;

	status = i2c_dw_apb_validate_address (address);
	if (status != 0) {
		return status;
	}

	i2c->regs->DW_apb_i2c_addr_block1.IC_TAR =
		CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_TAR_IC_TAR_MODIFY (
		i2c->regs->DW_apb_i2c_addr_block1.IC_TAR, address);

	return 0;
}

/**
 * Prevents the MASTER HW from initiating a transmission.
 *
 * This should only be set if Tx FIFO is empty and the HW is shutdown.
 *
 * @param i2c The I2C driver instance.
 */
static void i2c_dw_apb_block_master_tx (const struct i2c_dw_apb *i2c)
{
	i2c->regs->DW_apb_i2c_addr_block1.IC_ENABLE |= I2C_DW_APB_ENABLE (TX_CMD_BLOCK);
}

/**
 * Release the hold on MASTER HW and allow it to begin processing the Tx FIFO.
 *
 * @param i2c The I2C driver instance.
 */
static void i2c_dw_apb_unblock_master_tx (const struct i2c_dw_apb *i2c)
{
	i2c->regs->DW_apb_i2c_addr_block1.IC_ENABLE &= ~I2C_DW_APB_ENABLE (TX_CMD_BLOCK);
}

/**
 * Moves data from the Rx FIFO into the Rx buffer if available.
 *
 * @param i2c The I2C driver instance.
 *
 * @return An i2c_dw_apb_rx_state value.
 */
static int i2c_dw_apb_fill_rx_buf (const struct i2c_dw_apb *i2c)
{
	struct i2c_dw_apb_state *state = i2c->state;
	int rx_state;

	rx_state = I2C_DW_APB_RX_STATE_NO_BUFFER;
	while (i2c->regs->DW_apb_i2c_addr_block1.IC_STATUS & I2C_DW_APB_STATUS (RFNE)) {
		if (state->rx_pos >= state->rx_len) {
			return rx_state;
		}

		state->rx_buffer[state->rx_pos] =
			CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_DATA_CMD_DAT_GET (
			i2c->regs->DW_apb_i2c_addr_block1.IC_DATA_CMD);

		// Only increment the state if we are not discarding the transaction.
		if (state->rx_buffer != &rx_discard_buffer) {
			++state->rx_pos;
		}

		// Only returns FILLED if more data in FIFO and buffer is maxed.
		rx_state = I2C_DW_APB_RX_STATE_FILLED;
	}

	// No more data available, Rx buffer could be partially filled.
	return I2C_DW_APB_RX_STATE_NO_DATA;
}

/**
 * Fills the Tx FIFO with the data in the Tx buffer if available.
 *
 * @param i2c The I2C driver instance.
 *
 * @return An i2c_dw_apb_tx_state value.
 */
static int i2c_dw_apb_fill_tx_fifo (const struct i2c_dw_apb *i2c)
{
	struct i2c_dw_apb_state *state = i2c->state;
	uint32_t cmd;
	int mode;

	if (state->tx_len == 0) {
		return I2C_DW_APB_TX_STATE_NO_DATA;
	}

	/* If the device is sending data, fill the Tx FIFO. */
	mode = i2c_dw_apb_get_mode (i2c);
	while (i2c->regs->DW_apb_i2c_addr_block1.IC_STATUS & I2C_DW_APB_STATUS (TFNF)) {
		cmd = *state->tx_data;

		if (((--state->tx_len) == 0) && (mode == I2C_DW_APB_MODE_MASTER)) {
			cmd |= I2C_DW_APB_CMD_STOP;
		}

		i2c->regs->DW_apb_i2c_addr_block1.IC_DATA_CMD = cmd;

		if (state->tx_len == 0) {
			return I2C_DW_APB_TX_STATE_DEPLETED;
		}

		++state->tx_data;
	}

	return I2C_DW_APB_TX_STATE_TRANSMITTING;
}

/**
 * Begins discarding the incoming Rx data and calls the Rx overflow callback.  This happens when
 * RX_OVER interrupt occurs or when no Rx buffer is available to receive into.
 *
 * @param i2c The I2C driver instance.
 */
static void i2c_dw_apb_drop_rx (const struct i2c_dw_apb *i2c)
{
	i2c_dw_apb_discard_rx (i2c);
	i2c->on_rx_discard (i2c);
}

/**
 * Polls and processes the I2C interrupts.
 *
 * @param i2c The I2C driver instance.
 *
 * @return An i2c_dw_apb_irq_state value.
 */
static int i2c_dw_apb_poll_interrupts (const struct i2c_dw_apb *i2c)
{
	int irq_state = I2C_DW_APB_IRQ_STATE_IDLE;
	uint32_t irq_status;
	uint32_t intr_mask;
	int txn_state;
	bool tx_pending;

	irq_status = i2c->regs->DW_apb_i2c_addr_block1.IC_INTR_STAT;

	if (irq_status & I2C_DW_APB_IRQ (START_DET)) {
		i2c_dw_apb_clr_interrupt (&i2c->regs->DW_apb_i2c_addr_block1.IC_CLR_START_DET);

		intr_mask = i2c->on_start (i2c);

		irq_state = I2C_DW_APB_IRQ_STATE_BUSY;
		irq_status = i2c_dw_apb_update_intr_state (i2c, intr_mask);
	}

	if (irq_status & I2C_DW_APB_IRQ (RX_OVER)) {
		i2c_dw_apb_drop_rx (i2c);

		i2c_dw_apb_update_clr_interrupt (i2c, I2C_DW_APB_INTR_RX_HANDLER,
			&i2c->regs->DW_apb_i2c_addr_block1.IC_CLR_RX_OVER);

		irq_state = I2C_DW_APB_IRQ_STATE_BUSY;
		irq_status = i2c->regs->DW_apb_i2c_addr_block1.IC_INTR_STAT;
	}

	if (irq_status & I2C_DW_APB_IRQ (RX_FULL)) {
		intr_mask = I2C_DW_APB_INTR_RX_ACTIVE;

		do {
			txn_state = i2c_dw_apb_fill_rx_buf (i2c);
			if (txn_state == I2C_DW_APB_RX_STATE_NO_BUFFER) {
				// Notify driver to queue an Rx buffer
				i2c->on_rx_full (i2c);

				txn_state = i2c_dw_apb_fill_rx_buf (i2c);
				if (txn_state == I2C_DW_APB_RX_STATE_NO_BUFFER) {
					// No buffer was queued, drop the transaction.
					i2c_dw_apb_drop_rx (i2c);
					txn_state = I2C_DW_APB_RX_STATE_FILLED;
					intr_mask = I2C_DW_APB_INTR_RX_HANDLER;
				}
			}
		} while (txn_state == I2C_DW_APB_RX_STATE_FILLED);

		irq_state = I2C_DW_APB_IRQ_STATE_BUSY;
		irq_status = i2c_dw_apb_update_intr_state (i2c, intr_mask);
	}

	if (irq_status & I2C_DW_APB_IRQ (TX_ABRT)) {
		/* Tx Abort flushes extra data from the Tx FIFO.  Any pending data not in the FIFO should
		 * also be cleared. */
		i2c->state->tx_len = 0;

		/* Cache the source of the Tx Abort so it can be inspected later.  This register gets
		 * automatically cleared when the interrupt is cleared. */
		i2c->state->tx_abort_src = i2c->regs->DW_apb_i2c_addr_block1.IC_TX_ABRT_SOURCE;

		i2c->on_tx_abort (i2c);

		i2c_dw_apb_update_clr_interrupt (i2c, I2C_DW_APB_INTR_END,
			&i2c->regs->DW_apb_i2c_addr_block1.IC_CLR_TX_ABRT);

		irq_state = I2C_DW_APB_IRQ_STATE_BUSY;
		irq_status = i2c->regs->DW_apb_i2c_addr_block1.IC_INTR_STAT;
	}

	if (irq_status & I2C_DW_APB_IRQ (SCL_STUCK_AT_LOW)) {
		if (i2c->on_tx_timeout) {
			i2c->on_tx_timeout (i2c);
		}

		i2c_dw_apb_update_clr_interrupt (i2c, I2C_DW_APB_INTR_SCL_STUCK,
			&i2c->regs->DW_apb_i2c_addr_block1.IC_CLR_SCL_STUCK_DET);

		// Address operations that caused timeout
		irq_state = I2C_DW_APB_IRQ_STATE_BUSY;
		i2c_dw_apb_transmit_default_padding_byte (i2c);
		irq_status = i2c->regs->DW_apb_i2c_addr_block1.IC_INTR_STAT;
	}

	if (irq_status & I2C_DW_APB_IRQ (RD_REQ)) {
		intr_mask = I2C_DW_APB_INTR_TX_ACTIVE;

		tx_pending = i2c_dw_apb_is_tx_data_pending (i2c);
		if (!tx_pending) {
			i2c->on_read_request (i2c);

			tx_pending = i2c_dw_apb_is_tx_data_pending (i2c);
			if (!tx_pending) {
				// No data queued. It is now up to the upper layers to enable RD_REQ
				intr_mask = I2C_DW_APB_INTR_TX_STALL;
			}
		}

		if (tx_pending) {
			i2c_dw_apb_clr_interrupt (&i2c->regs->DW_apb_i2c_addr_block1.IC_CLR_RD_REQ);
		}

		irq_state = I2C_DW_APB_IRQ_STATE_BUSY;
		irq_status = i2c_dw_apb_update_intr_state (i2c, intr_mask);
	}

	if (irq_status & I2C_DW_APB_IRQ (TX_EMPTY)) {
		intr_mask = 0;
		txn_state = i2c_dw_apb_fill_tx_fifo (i2c);
		switch (txn_state) {
			case I2C_DW_APB_TX_STATE_TRANSMITTING:
				intr_mask = I2C_DW_APB_INTR_TX_ACTIVE;
				break;

			case I2C_DW_APB_TX_STATE_DEPLETED:
				if (i2c->on_tx_depleted) {
					i2c->on_tx_depleted (i2c);
				}

			// fall through

			default:
				// No more data
				switch (i2c_dw_apb_get_mode (i2c)) {
					case I2C_DW_APB_MODE_SLAVE:
						intr_mask = I2C_DW_APB_INTR_SLAVE_TX_IDLE;
						break;

					case I2C_DW_APB_MODE_MASTER:
						intr_mask = I2C_DW_APB_INTR_TX_MONITOR;
						break;
				}
				break;
		}

		irq_state = I2C_DW_APB_IRQ_STATE_BUSY;
		irq_status = i2c_dw_apb_update_intr_state (i2c, intr_mask);
	}

	if (irq_status & I2C_DW_APB_IRQ_TXN_END) {
		i2c_dw_apb_rst_txn_buffers (i2c->state);

		if (irq_status & I2C_DW_APB_IRQ (RESTART_DET)) {
			i2c_dw_apb_clr_interrupt (&i2c->regs->DW_apb_i2c_addr_block1.IC_CLR_RESTART_DET);

			intr_mask = i2c->on_restart (i2c);

			irq_state = I2C_DW_APB_IRQ_STATE_BUSY;
		}
		else {
			i2c_dw_apb_clr_interrupt (&i2c->regs->DW_apb_i2c_addr_block1.IC_CLR_STOP_DET);

			i2c->on_stop (i2c);

			intr_mask = I2C_DW_APB_INTR_IDLE;
			irq_state = I2C_DW_APB_IRQ_STATE_COMPLETE;
		}

		i2c->regs->DW_apb_i2c_addr_block1.IC_INTR_MASK = intr_mask;

		if (i2c->state->rx_pos >= i2c->state->rx_len) {
			// Driver did not preemptively queue Rx buffer.
			i2c->state->rx_pos = 0;
		}
	}

	if ((irq_state == I2C_DW_APB_IRQ_STATE_IDLE) && !i2c_dw_apb_is_hw_enabled (i2c) &&
		(i2c->regs->DW_apb_i2c_addr_block1.IC_INTR_MASK != 0)) {
		/* HW is in the middle of disabling. Clear interrupt mask to prevent any race conditions
		 * with the HW controlled interrupts (TX_EMPTY, RX_FULL, etc). */
		i2c->regs->DW_apb_i2c_addr_block1.IC_INTR_MASK = 0;
		irq_state = I2C_DW_APB_IRQ_STATE_BUSY;
	}

	return irq_state;
}

/* Interface Implementation */

bool i2c_dw_apb_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param)
{
	const struct i2c_dw_apb *i2c = (const struct i2c_dw_apb*) handler;
	int irq_state;

	UNUSED (param);

	irq_state = i2c_dw_apb_poll_interrupts (i2c);

	return irq_state != I2C_DW_APB_IRQ_STATE_IDLE;
}

/* Public Status API */

/**
 * Gets the global Rx discard buffer for The I2C driver instance.
 *
 * @param data A pointer to store the pointer to the global discard buffer.
 *
 * @return The length of the returned discard buffer.
 */
size_t i2c_dw_apb_rx_discard_buffer (uint8_t **data)
{
	if (data == NULL) {
		return 0;
	}

	*data = &rx_discard_buffer;

	return 1;
}

/**
 * Gets the global default padding buffer.
 *
 * @param data A pointer to store the pointer to the global default padding buffer.
 *
 * @return The length of the returned padding buffer.
 */
size_t i2c_dw_apb_tx_pad_buffer (const uint8_t **data)
{
	if (data == NULL) {
		return 0;
	}

	*data = &I2C_DW_APB_TX_DEFAULT_PADDING_BYTE;

	return 1;
}

/**
 * Gets the depth of the Tx FIFO buffer.
 *
 * @param i2c The I2C driver instance.
 *
 * @return The Tx FIFO depth or an error code.
 */
int i2c_dw_apb_get_tx_fifo_depth (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	return (int)
		   CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_COMP_PARAM_1_TX_BUFFER_DEPTH_GET
			   (i2c->regs->DW_apb_i2c_addr_block1.IC_COMP_PARAM_1);
}

/**
 * Gets the currently configured operation mode for the I2C HW.
 *
 * @param i2c The I2C driver instance.
 *
 * @return An i2c_dw_apb_mode value if successful, else an error code.
 */
int i2c_dw_apb_get_mode (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	switch (i2c->regs->DW_apb_i2c_addr_block1.IC_CON & I2C_DW_APB_CON_MASTER_MODE) {
		case 0:
			return I2C_DW_APB_MODE_SLAVE;

		case I2C_DW_APB_CON_MASTER_MODE:
			return I2C_DW_APB_MODE_MASTER;
	}

	return I2C_DW_APB_INVALID_MODE;
}

/**
 * Gets a flag indicating if the HW is enabled.
 *
 * If this returns false, it does not necessarily mean the HW is fully disabled and inactive.
 * Refer to the ENABLED to DISABLED sequence.
 *
 * @param i2c The I2C driver instance.
 *
 * @return true if HW is enabled, else false.
 */
bool i2c_dw_apb_is_hw_enabled (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return false;
	}

	return !!(i2c->regs->DW_apb_i2c_addr_block1.IC_ENABLE & I2C_DW_APB_ENABLE (ENABLE));
}

/**
 * Gets a flag indicating if MASTER or SLAVE HW is in an active state.
 *
 * @param i2c The I2C driver instance.
 *
 * @return true if HW is active, else false.
 */
bool i2c_dw_apb_is_active (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return false;
	}

	return !!(i2c->regs->DW_apb_i2c_addr_block1.IC_STATUS & I2C_DW_APB_STATUS (ACTIVITY));
}

/**
 * Gets a flag indicating if the TX FIFO has data in it.
 *
 * @param i2c The I2C driver instance.
 *
 * @return true if FIFO has data in it, else false.
 */
bool i2c_dw_apb_tx_fifo_not_empty (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return false;
	}

	return !(i2c->regs->DW_apb_i2c_addr_block1.IC_STATUS & I2C_DW_APB_STATUS (TFE));
}

/**
 * Gets a flag indicating if Tx data is still pending to be sent.
 *
 * @param i2c The I2C driver instance.
 *
 * @return true if Tx data is pending to be sent, else false.
 */
bool i2c_dw_apb_is_tx_data_pending (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return false;
	}

	return (i2c->state->tx_len > 0);
}

/**
 * Gets the configured slave address for the I2C HW.
 *
 * @param i2c The I2C driver instance.
 *
 * @return The slave address if successful, else an error code.
 */
int i2c_dw_apb_get_slave_address (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	if (!(i2c->capabilities & I2C_DW_APB_CAPABILITIES_SLAVE_MODE)) {
		return I2C_DW_APB_NOT_SUPPORTED;
	}

	return i2c_dw_apb_read_slave_address (i2c);
}

/**
 * Gets the current target address for the MASTER mode operation.
 *
 * @param i2c The I2C driver instance.
 *
 * @return The target address if successful, else an error code.
 */
int i2c_dw_apb_get_target_address (const struct i2c_dw_apb *i2c)
{
	int status;

	status = i2c_dw_apb_get_mode (i2c);

	// TAR only initialized when performing a MASTER transaction
	switch (status) {
		case I2C_DW_APB_MODE_MASTER:
			status = i2c_dw_apb_read_target_address (i2c);
			break;

		case I2C_DW_APB_MODE_SLAVE:
			status = I2C_DW_APB_INVALID_MODE;
			break;
	}

	return status;
}

/**
 * Gets the address of the device being addressed based on the currently configured HW mode.
 *
 * @param i2c The I2C driver instance.
 *
 * @return The slave address if the HW is in SLAVE mode, the target address if the device is
 * performing a MASTER operation, else an error code.
 */
int i2c_dw_apb_get_address_for_mode (const struct i2c_dw_apb *i2c)
{
	int status;

	status = i2c_dw_apb_get_mode (i2c);
	switch (status) {
		case I2C_DW_APB_MODE_SLAVE:
			status = i2c_dw_apb_read_slave_address (i2c);
			break;

		case I2C_DW_APB_MODE_MASTER:
			status = i2c_dw_apb_read_target_address (i2c);
			break;
	}

	return status;
}

/* Public Control API */

/**
 * Acquires the mutex lock for the I2C driver instance.
 *
 * @param i2c The I2C driver instance.
 *
 * @return 0 for success, else an error code.
 */
int i2c_dw_apb_lock_driver (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	return platform_mutex_lock (&i2c->state->lock);
}

/**
 * Releases the mutex lock for the I2C driver instance.
 *
 * @param i2c The I2C driver instance.
 *
 * @return 0 for success, else an error code.
 */
int i2c_dw_apb_unlock_driver (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	return platform_mutex_unlock (&i2c->state->lock);
}

/**
 * Abort the current transmit operation while operating in master mode.  This call will trigger the
 * abort, but the abort will not be completed until the TX_ABRT interrupt is triggered.
 *
 * This call will have no effect in slave mode.
 *
 * @param i2c The I2C driver instance.
 *
 * @return 0 if the abort was initiated or an error code.
 */
int i2c_dw_apb_abort_tx (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	/* To avoid race conditions, just set the abort bit without running any checks.  If the
	 * controller is in slave mode or the HW is disabled, the HW will do nothing.  Calling abort
	 * when no transmission is active should not have negative consequences. */
	i2c->regs->DW_apb_i2c_addr_block1.IC_ENABLE |= I2C_DW_APB_ENABLE (ABORT);

	return 0;
}

/**
 * Abort the current transmit operation while operating in master mode and wait for the abort to
 * complete.  This call will poll on register status directly rather than waiting for interrupts,
 * allowing this to work within the context of another interrupt.
 *
 * This call will have no effect in slave mode.
 *
 * @param i2c The I2C driver instance.
 *
 * @return 0 if the abort was successful or an error code.
 */
int i2c_dw_apb_abort_tx_and_wait (const struct i2c_dw_apb *i2c)
{
	int status;
	platform_clock timeout;

	status = i2c_dw_apb_abort_tx (i2c);
	if (status != 0) {
		return status;
	}

	/* The abort bit is self-clearing when the abort has been completed.  If no abort is done, such
	 * as when in slave mode, the bit is never set.  Spin until the abort bit is clear. */
	status = platform_init_timeout (1000, &timeout);
	if (status != 0) {
		return status;
	}

	while (i2c->regs->DW_apb_i2c_addr_block1.IC_ENABLE & I2C_DW_APB_ENABLE (ABORT)) {
		if (platform_has_timeout_expired (&timeout)) {
			return I2C_DW_APB_HW_ERROR;
		}
	}

	return 0;
}

/**
 * Indicate the source for the last Tx Abort.
 *
 * @param i2c The I2C driver to query.
 *
 * @return The source of the last Tx Abort as would be reported by IC_TX_ABRT_SOURCE.
 */
uint32_t i2c_dw_apb_get_last_tx_abort_source (const struct i2c_dw_apb *i2c)
{
	if (i2c != NULL) {
		return i2c->state->tx_abort_src;
	}
	else {
		return 0;
	}
}

/**
 * Shutdown the I2C HW block and ensures the driver state is initialized.
 *
 * @param i2c The I2C driver instance.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_shutdown_hw (const struct i2c_dw_apb *i2c)
{
	int status;

	status = i2c_dw_apb_disable_hw (i2c);
	if (status != 0) {
		return status;
	}

	i2c->on_shutdown (i2c);

	return 0;
}

/**
 * Tries to handle a full I2C transaction.
 *
 * @param i2c The I2C driver instance.
 *
 * @return 0 if a transaction was handled, 1 if no events were handled, else an error code.
 */
int i2c_dw_apb_try_handle_transaction (const struct i2c_dw_apb *i2c)
{
	int irq_state;

	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	irq_state = i2c_dw_apb_poll_interrupts (i2c);
	if (irq_state == I2C_DW_APB_IRQ_STATE_IDLE) {
		return 1;
	}

	// Transaction started, finish it
	while (irq_state != I2C_DW_APB_IRQ_STATE_COMPLETE) {
		irq_state = i2c_dw_apb_poll_interrupts (i2c);
	}

	return 0;
}

/**
 * Handle a single transaction on the I2C bus.  This call will block until a remote device sends a
 * request and will return when that transaction has been completed.
 *
 * @param i2c The I2C driver instance.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_handle_transaction (const struct i2c_dw_apb *i2c)
{
	int status;

	do {
		status = i2c_dw_apb_try_handle_transaction (i2c);
	} while (status == 1);

	return status;
}

/**
 * Configures the I2C HW block for SLAVE mode operation and enables it.  This will execute the
 * shutdown sequence if MASTER mode is active.
 *
 * This only makes sense for SLAVE and MULTI-MASTER mode drivers.
 *
 * @param i2c The I2C driver instance.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_enable_slave_mode (const struct i2c_dw_apb *i2c)
{
	int status;

	if (i2c_dw_apb_is_hw_enabled (i2c) && (i2c_dw_apb_get_mode (i2c) == I2C_DW_APB_MODE_SLAVE)) {
		// SLAVE mode already active.
		return 0;
	}

	/* Always make sure the hardware is properly disabled before switching modes. */
	status = i2c_dw_apb_shutdown_hw (i2c);
	if (status != 0) {
		return status;
	}

	status = i2c_dw_apb_configure_slave_mode (i2c);
	if (status != 0) {
		return status;
	}

	i2c_dw_apb_enable_hw (i2c);

	return 0;
}

/**
 * Disables the I2C HW block, initializes the state for MASTER transmission, and then enables the
 * HW.
 *
 * This only makes sense for MULTI-MASTER and MASTER mode drivers.
 *
 * @param i2c The I2C driver instance.
 * @param target_address The address of the target I2C slave device to transmit data to.
 * @param data The data buffer to transmit.
 * @param len The length of the data to transmit.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_begin_master_transmit (const struct i2c_dw_apb *i2c, uint8_t target_address,
	const uint8_t *data, size_t len)
{
	int status;

	if ((data == NULL) || (len == 0)) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	if (!(i2c->capabilities & I2C_DW_APB_CAPABILITIES_MASTER_MODE)) {
		return I2C_DW_APB_NOT_SUPPORTED;
	}

	status = i2c_dw_apb_shutdown_hw (i2c);
	if (status != 0) {
		return status;
	}

	status = i2c_dw_apb_set_target_address (i2c, target_address);
	if (status != 0) {
		return status;
	}

	i2c_dw_apb_set_master_mode (i2c);

	i2c->state->tx_len = len;
	i2c->state->tx_data = data;

	// Prevent the transaction from starting until we have fully queued data
	i2c_dw_apb_block_master_tx (i2c);
	i2c_dw_apb_enable_hw (i2c);

	i2c_dw_apb_fill_tx_fifo (i2c);

	/* Configure interrupts and unblock to begin transmission. START will enable TX_EMPTY to
	 * continue filling the FIFO. */
	i2c->regs->DW_apb_i2c_addr_block1.IC_INTR_MASK = I2C_DW_APB_INTR_IDLE;
	i2c_dw_apb_unblock_master_tx (i2c);

	return 0;
}

/* Mode Driver Control API
 * These are not top level calls and instead, to be used by the mode implementation layer. */

/**
 * Gets the interrupt mask for starting SLAVE mode transactions.
 *
 * @return The SLAVE mode START interrupt mask.
 */
uint32_t i2c_dw_apb_slave_start_intr_mask ()
{
	return I2C_DW_APB_INTR_SLAVE_START;
}

/**
 * Gets the interrupt mask for handling a transmission operation.
 *
 * @return The transmission operation interrupt mask.
 */
uint32_t i2c_dw_apb_tx_intr_mask ()
{
	return I2C_DW_APB_INTR_TX_ACTIVE;
}

/**
 * Initialize the base I2C DW APB driver instance at runtime.  This does not initialize the HW or
 * the state context which must be initialized by the respective top layer API calls.
 *
 * @param i2c The base I2C driver instance.
 * @param state The base I2C variable context.  This must not already be initialized.
 * @param regs The I2C HW block registers.
 * @param handler The I2C implementation handler.
 * @param capabilities The i2c_dw_apb_capabilities mask.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_init (struct i2c_dw_apb *i2c, struct i2c_dw_apb_state *state,
	struct Creg_regs_DW_apb_i2c_APB_Slave *regs, const struct i2c_dw_apb_handler *handler,
	int capabilities)
{
	if ((i2c == NULL) || (state == NULL) || (regs == NULL)) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	i2c->isr_handler.handle_interrupt = i2c_dw_apb_handle_interrupt;
	i2c->state = state;
	i2c->regs = regs;
	i2c->i2c_handler = handler;
	i2c->capabilities = capabilities;

	return 0;
}

/**
 * Initialize only the variable state of the I2C base driver instance.
 *
 * @param i2c The base I2C driver instance.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int i2c_dw_apb_init_state (const struct i2c_dw_apb *i2c)
{
	if ((i2c == NULL) || (i2c->state == NULL)) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	return platform_mutex_init (&i2c->state->lock);
}

/**
 * Disables the I2C HW block and configures the HW.
 *
 * @param i2c The I2C driver instance.
 * @param mode The highest speed mode the slave is expected to support.  The I2C slave doesn't
 * drive the clock, but it uses this information to configure the appropriate spike filter.
 * @param ic_clk Frequency of the clock for the I2C block, in Hz.
 *
 * @return 0 if the hardware was successfully configured, else an error code.
 */
int i2c_dw_apb_init_hw (const struct i2c_dw_apb *i2c, enum i2c_dw_apb_speed mode, uint32_t ic_clk)
{
	uint32_t fifo_depth;
	uint32_t clks;
	int status;

	if ((i2c == NULL) || (i2c->state == NULL) || (i2c->regs == NULL)) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	if (!(i2c->capabilities & (I2C_DW_APB_CAPABILITIES_MODE_MASK))) {
		return I2C_DW_APB_INVALID_CAPS_MASK;
	}

	if ((i2c->on_shutdown == NULL) || (i2c->on_start == NULL) || (i2c->on_rx_full == NULL) ||
		(i2c->on_rx_discard == NULL) || (i2c->on_read_request == NULL) ||
		(i2c->on_tx_abort == NULL) || (i2c->on_restart == NULL) || (i2c->on_stop == NULL)) {
		return I2C_DW_APB_MISSING_HANDLERS;
	}

	status = i2c_dw_apb_disable_hw (i2c);
	if (status != 0) {
		return status;
	}

	i2c->regs->DW_apb_i2c_addr_block1.IC_CON =
		CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_CON_SPEED_SET (mode) |
		CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_CON_IC_RESTART_EN_FIELD_MASK |
		CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_CON_STOP_DET_IFADDRESSED_FIELD_MASK
		|
		CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_CON_STOP_DET_IF_MASTER_ACTIVE_FIELD_MASK;

	/* Determine the appropriate SDA setup time based on the HW clock.  The timing is configured for
	 * compatibility, and uses 250ns that is required for standard mode devices. */
	clks = I2C_DW_APB_NS_TO_IC_CLK (250, ic_clk);
	clks += 1;
	if (clks < 2) {
		clks = 2;	// Minimum value of 2 is required.
	}

	i2c->regs->DW_apb_i2c_addr_block1.IC_SDA_SETUP = clks;

	/* Configure the standard/fast mode spike suppression to 50ns. */
	clks = I2C_DW_APB_NS_TO_IC_CLK (50, ic_clk);
	if (clks < 1) {
		clks = 1;	// Minimum value of 1 is required.
	}

	i2c->regs->DW_apb_i2c_addr_block1.IC_FS_SPKLEN = clks;

	/* Use a minimum hold time allowed by the HW.  The I2C specification indicates a 300ns hold time
	 * must be used to account for slow fall times of the SCL with standard (100kHz) and fast
	 * (400kHz) modes.  However, using a 250ns setup time with a 300ns hold time would mean fast
	 * plus mode (1MHz) operation would not be possible.  The SMBus spec simply states the hold time
	 * needs to be long enough to account for the undefined range of the falling clock edge.  Assume
	 * we don't need to deal with slow fall times and use the minimum required by the I2C HW, which
	 * will be 50ns plus 7 ic_clks. */
	i2c->regs->DW_apb_i2c_addr_block1.IC_SDA_HOLD = clks + 7;

	/* If the HW block was designed with the IC_EMPTYFIFO_HOLD_MASTER_EN feature disabled, the HW
	 * will generate a STOP condition when the Tx FIFO is depleted.  Set the TX_EMPTY interrupt
	 * threshold to immediately trigger once the FIFO has an available byte.
	 *
	 * TODO: Evaluate once the system becomes more stable and see if we can lower the thresholds. */
	fifo_depth = i2c_dw_apb_get_tx_fifo_depth (i2c);
	i2c->regs->DW_apb_i2c_addr_block1.IC_TX_TL =
		CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_TX_TL_TX_TL_MODIFY (
		i2c->regs->DW_apb_i2c_addr_block1.IC_TX_TL, fifo_depth - 1);

	i2c->regs->DW_apb_i2c_addr_block1.IC_RX_TL =
		CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_RX_TL_RX_TL_MODIFY (
		i2c->regs->DW_apb_i2c_addr_block1.IC_RX_TL, 0);

	i2c->regs->DW_apb_i2c_addr_block1.IC_SCL_STUCK_AT_LOW_TIMEOUT = 0;

	return 0;
}

/**
 * Disables the I2C HW and releases resources held by the base I2C driver instance.
 *
 * @param i2c The I2C driver instance.
 */
void i2c_dw_apb_release (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return;
	}

	i2c_dw_apb_shutdown_hw (i2c);
	platform_mutex_free (&i2c->state->lock);
}

/**
 * Sets the slave address for the I2C HW block.
 *
 * This only makes sense in SLAVE or MULTI-MASTER mode drivers.
 *
 * @param i2c The I2C driver instance.
 * @param address The slave address to configure.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_set_slave_address (const struct i2c_dw_apb *i2c, uint8_t address)
{
	int status;

	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	status = i2c_dw_apb_validate_address (address);
	if (status != 0) {
		return status;
	}

	if (!(i2c->capabilities & I2C_DW_APB_CAPABILITIES_SLAVE_MODE)) {
		return I2C_DW_APB_NOT_SUPPORTED;
	}

	i2c->regs->DW_apb_i2c_addr_block1.IC_SAR =
		CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_SAR_IC_SAR_MODIFY (
		i2c->regs->DW_apb_i2c_addr_block1.IC_SAR, address);

	return 0;
}

/**
 * Configures the I2C HW block for SLAVE mode operation.
 *
 * @param i2c The I2C driver instance.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_configure_slave_mode (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	if (!(i2c->capabilities & I2C_DW_APB_CAPABILITIES_SLAVE_MODE)) {
		return I2C_DW_APB_NOT_SUPPORTED;
	}

	i2c->regs->DW_apb_i2c_addr_block1.IC_CON &= ~I2C_DW_APB_CON_MASTER_MODE;
	i2c->regs->DW_apb_i2c_addr_block1.IC_INTR_MASK = I2C_DW_APB_INTR_IDLE;

	return 0;
}

/**
 * Sets the timeout value for when the HW block triggers the SCL_STUCK_AT_LOW interrupt.
 *
 * This only makes sense for SLAVE mode drivers.
 *
 * @param i2c The I2C driver instance.
 * @param ic_clk Frequency of the clock for the I2C block, in Hz.
 * @param timeout_ms Timeout value in milliseconds. If 0 is specified, the timeout will be set to
 * the default value.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_set_scl_timeout (const struct i2c_dw_apb *i2c, uint32_t ic_clk, unsigned timeout_ms)
{
	int status;

	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	if ((i2c->capabilities & I2C_DW_APB_CAPABILITIES_MODE_MASK) !=
		I2C_DW_APB_CAPABILITIES_SLAVE_MODE) {
		return I2C_DW_APB_NOT_SUPPORTED;
	}

	if (timeout_ms == 0) {
		i2c->regs->DW_apb_i2c_addr_block1.IC_SCL_STUCK_AT_LOW_TIMEOUT =
			CREG_REGS_DW_APB_I2C_APB_SLAVE_DW_APB_I2C_ADDR_BLOCK1_IC_SCL_STUCK_AT_LOW_TIMEOUT_RESET_VALUE;

		return 0;
	}

	status =
		i2c_dw_apb_set_clocks_counter (
		&i2c->regs->DW_apb_i2c_addr_block1.IC_SCL_STUCK_AT_LOW_TIMEOUT,
		I2C_DW_APB_MS_TO_IC_CLK (timeout_ms, ic_clk));

	return status;
}

/**
 * Assigns the interrupt mask to service a stalled read request event.
 *
 * This should only be called in an I2C ISR safe context and only makes sense for a SLAVE mode
 * driver.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_try_read_request (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	if ((i2c->capabilities & I2C_DW_APB_CAPABILITIES_MODE_MASK) !=
		I2C_DW_APB_CAPABILITIES_SLAVE_MODE) {
		return I2C_DW_APB_NOT_SUPPORTED;
	}

	i2c->regs->DW_apb_i2c_addr_block1.IC_INTR_MASK = I2C_DW_APB_INTR_TRY_RD_REQ;

	return 0;
}

/**
 * Initialize the Tx context with the provided data buffer.
 *
 * This should only be called in an ISR safe context.
 *
 * @param i2c The I2C driver instance.
 * @param data A pointer to the bytes to transmit.
 * @param len The length of the data buffer.
 *
 * @return 0 if successful, else an error code
 */
int i2c_dw_apb_set_tx_data (const struct i2c_dw_apb *i2c, const uint8_t *data, size_t len)
{
	int status;

	status = i2c_dw_apb_validate_buffer (i2c, data, len);
	if (status != 0) {
		return status;
	}

	i2c->state->tx_len = len;
	i2c->state->tx_data = data;

	return 0;
}

/**
 * Initialize the Tx context with the global default padding data buffer.
 *
 * This should only be called in an ISR safe context.
 *
 * @param i2c The I2C driver instance.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_transmit_default_padding_byte (const struct i2c_dw_apb *i2c)
{
	return i2c_dw_apb_set_tx_data (i2c, &I2C_DW_APB_TX_DEFAULT_PADDING_BYTE, 1);
}

/**
 * Initialize the Rx context with the provided buffer.
 *
 * This should only be called in an ISR safe context.
 *
 * @param i2c The I2C driver instance.
 * @param buffer The byte buffer to receive data into.
 * @param len The length of the buffer.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_set_rx_buffer (const struct i2c_dw_apb *i2c, uint8_t *buffer, size_t len)
{
	int status;

	status = i2c_dw_apb_validate_buffer (i2c, buffer, len);
	if (status != 0) {
		return status;
	}

	if (buffer == &rx_discard_buffer) {
		len = 1;
	}
	else if (i2c->state->rx_buffer == &rx_discard_buffer) {
		// Cannot override internal discard
		return I2C_DW_APB_DISCARDING_RX;
	}

	i2c->state->rx_pos = 0;
	i2c->state->rx_len = len;
	i2c->state->rx_buffer = buffer;

	return 0;
}

/**
 * Initialize the Rx context with the global discard buffer.
 *
 * This should only be called in an ISR safe context.
 *
 * @param i2c The I2C driver context.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_discard_rx (const struct i2c_dw_apb *i2c)
{
	return i2c_dw_apb_set_rx_buffer (i2c, &rx_discard_buffer, 1);
}

/* Mode Driver Handler API */

/**
 * Calls the I2C handler shutdown event callback.
 *
 * @param i2c The I2C driver instance.
 */
void i2c_dw_apb_call_on_shutdown (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return;
	}

	i2c->i2c_handler->on_shutdown (i2c->i2c_handler);
}

/**
 * Calls the I2C handler Rx discard event callback.
 *
 * This should only be called in an ISR safe context.
 *
 * @param i2c The I2C driver instance.
 */
void i2c_dw_apb_call_rx_discard (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return;
	}

	i2c->i2c_handler->on_rx_discard (i2c->i2c_handler);
}

/**
 * Calls the I2C handler Rx complete event callback.
 *
 * This should only be called in an ISR safe context.
 *
 * @param i2c The I2C driver instance.
 */
void i2c_dw_apb_call_rx_complete (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return;
	}

	i2c->i2c_handler->on_rx_complete (i2c->i2c_handler);
}

/**
 * Calls the I2C handler Tx discard event callback if it is not NULL.
 *
 * This should only be called in an ISR safe context.
 *
 * @param i2c The I2C driver instance.
 */
void i2c_dw_apb_try_call_tx_abort (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return;
	}

	if (i2c->i2c_handler->on_tx_abort) {
		i2c->i2c_handler->on_tx_abort (i2c->i2c_handler);
	}
}

/**
 * Calls the I2C handler Tx complete event callback if it is not NULL.
 *
 * This should only be called in an ISR safe context.
 *
 * @param i2c The I2C driver instance.
 */
void i2c_dw_apb_try_call_tx_complete (const struct i2c_dw_apb *i2c)
{
	if (i2c == NULL) {
		return;
	}

	if (i2c->i2c_handler->on_tx_complete) {
		i2c->i2c_handler->on_tx_complete (i2c->i2c_handler);
	}
}

/**
 * Calls the I2C slave Rx handler Rx pending and initializes the Rx context with the returned
 * parameters.
 *
 * This should only be called in an ISR safe context for SLAVE or MULTI-MASTER drivers.
 *
 * @param i2c The I2C driver instance.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_slave_rx_call_rx_pending (const struct i2c_dw_apb *i2c)
{
	const struct i2c_dw_apb_handler_slave_rx *handler;
	uint8_t *data = NULL;
	size_t len;
	int status;

	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	handler = (const struct i2c_dw_apb_handler_slave_rx*) i2c->i2c_handler;
	len = handler->on_rx_pending (&handler->handler_base, &data);
	status = i2c_dw_apb_set_rx_buffer (i2c, data, len);

	return status;
}

/**
 * Calls the I2C slave Rx handler Rx data event callback.
 *
 * This should only be called in an ISR safe context for SLAVE or MULTI-MASTER drivers.
 *
 * @param i2c The I2C driver instance.
 */
void i2c_dw_apb_slave_rx_call_rx_data (const struct i2c_dw_apb *i2c)
{
	const struct i2c_dw_apb_handler_slave_rx *handler;

	if (i2c == NULL) {
		return;
	}

	handler = (const struct i2c_dw_apb_handler_slave_rx*) i2c->i2c_handler;
	handler->on_rx_data (&handler->handler_base, i2c->state->rx_pos);
}
