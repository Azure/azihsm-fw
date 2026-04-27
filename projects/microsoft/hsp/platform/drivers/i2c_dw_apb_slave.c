// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <string.h>
#include "platform_config.h"
#include "common/unused.h"
#include "drivers/i2c_dw_apb_error.h"
#include "drivers/i2c_dw_apb_slave_static.h"
#include "trap/hsp_trap.h"


/**
 * State machine for an I2C SLAVE mode operation.
 */
enum i2c_dw_apb_slave_txn_state {
	I2C_DW_APB_SLAVE_TXN_STATE_IDLE,		/**< No operation is in progress. */
	I2C_DW_APB_SLAVE_TXN_STATE_ACTIVE,		/**< START condition has occurred but no operation started. */
	I2C_DW_APB_SLAVE_TXN_STATE_RX,			/**< Driver is receiving data. */
	I2C_DW_APB_SLAVE_TXN_STATE_RX_DISCARD,	/**< Discarding the current Rx transaction. */
	I2C_DW_APB_SLAVE_TXN_STATE_TX_WAIT,		/**< Driver is stalled waiting for data to transmit. */
	I2C_DW_APB_SLAVE_TXN_STATE_TX,			/**< Data is being currently being transmitted. */
	I2C_DW_APB_SLAVE_TXN_STATE_TX_PAD,		/**< Read requests will be serviced with the default Tx padding bytes until the transaction ends. */
};


/* Utilities */

/**
 * Calls the I2C handler callbacks based on the transaction state and then initiailizes the state.
 *
 * @param i2c The I2C slave driver instance.
 * @param txn_state The transaction state to initialize the driver state.
 */
static void i2c_dw_apb_slave_txn_complete (const struct i2c_dw_apb_slave *i2c, unsigned txn_state)
{
	struct i2c_dw_apb_slave_state *state = (struct i2c_dw_apb_slave_state*) i2c->i2c_base.state;

	switch (state->txn_state) {
		case I2C_DW_APB_SLAVE_TXN_STATE_RX:
			i2c_dw_apb_slave_rx_call_rx_data (&i2c->i2c_base);
			i2c_dw_apb_call_rx_complete (&i2c->i2c_base);
			break;

		case I2C_DW_APB_SLAVE_TXN_STATE_TX:
		case I2C_DW_APB_SLAVE_TXN_STATE_TX_PAD:
			i2c_dw_apb_try_call_tx_complete (&i2c->i2c_base);
			break;
	}

	state->txn_state = txn_state;
}

/* Driver Mode Implementation */

void i2c_dw_apb_slave_on_shutdown (const struct i2c_dw_apb *i2c_hw)
{
	struct i2c_dw_apb_slave_state *state = (struct i2c_dw_apb_slave_state*) i2c_hw->state;

	i2c_dw_apb_call_on_shutdown (i2c_hw);
	state->txn_state = I2C_DW_APB_SLAVE_TXN_STATE_IDLE;
}

uint32_t i2c_dw_apb_slave_on_start (const struct i2c_dw_apb *i2c_hw)
{
	struct i2c_dw_apb_slave_state *state = (struct i2c_dw_apb_slave_state*) i2c_hw->state;

	state->txn_state = I2C_DW_APB_SLAVE_TXN_STATE_ACTIVE;

	return i2c_dw_apb_slave_start_intr_mask ();
}

void i2c_dw_apb_slave_on_rx_full (const struct i2c_dw_apb *i2c_hw)
{
	struct i2c_dw_apb_slave_state *state = (struct i2c_dw_apb_slave_state*) i2c_hw->state;

	switch (state->txn_state) {
		case I2C_DW_APB_SLAVE_TXN_STATE_ACTIVE:
			state->txn_state = I2C_DW_APB_SLAVE_TXN_STATE_RX;
			i2c_dw_apb_slave_rx_call_rx_pending (i2c_hw);
			break;

		case I2C_DW_APB_SLAVE_TXN_STATE_RX:
			i2c_dw_apb_slave_rx_call_rx_data (i2c_hw);
			i2c_dw_apb_slave_rx_call_rx_pending (i2c_hw);
			break;

		case I2C_DW_APB_SLAVE_TXN_STATE_TX:
		case I2C_DW_APB_SLAVE_TXN_STATE_TX_PAD:
		case I2C_DW_APB_SLAVE_TXN_STATE_TX_WAIT:
			// Need to complete the Tx transaction before servicing the pending Rx
			return;

		default:
			i2c_dw_apb_discard_rx (i2c_hw);
			break;
	}
}

void i2c_dw_apb_slave_on_rx_discard (const struct i2c_dw_apb *i2c_hw)
{
	struct i2c_dw_apb_slave_state *state = (struct i2c_dw_apb_slave_state*) i2c_hw->state;

	i2c_dw_apb_call_rx_discard (i2c_hw);
	state->txn_state = I2C_DW_APB_SLAVE_TXN_STATE_RX_DISCARD;
}

void i2c_dw_apb_slave_on_read_request (const struct i2c_dw_apb *i2c_hw)
{
	struct i2c_dw_apb_slave_state *state = (struct i2c_dw_apb_slave_state*) i2c_hw->state;
	const struct i2c_dw_apb_slave_handler *handler =
		(const struct i2c_dw_apb_slave_handler*) i2c_hw->i2c_handler;
	const uint8_t *data = NULL;
	size_t len = 0;
	uint32_t txn_state;

	switch (state->txn_state) {
		case I2C_DW_APB_SLAVE_TXN_STATE_ACTIVE:
		case I2C_DW_APB_SLAVE_TXN_STATE_TX_WAIT:
		case I2C_DW_APB_SLAVE_TXN_STATE_TX:
			txn_state = I2C_DW_APB_SLAVE_TXN_STATE_TX_WAIT;

			len = handler->on_read_request (&handler->rx_handler.handler_base, &data);
			if (len > 0) {
				i2c_dw_apb_set_tx_data (i2c_hw, data, len);
				txn_state = I2C_DW_APB_SLAVE_TXN_STATE_TX;
			}

			state->txn_state = txn_state;

			return;

		case I2C_DW_APB_SLAVE_TXN_STATE_RX:
		case I2C_DW_APB_SLAVE_TXN_STATE_RX_DISCARD:
			// Need to complete the Rx transaction before servicing pending Tx
			return;

		default:
			i2c_dw_apb_transmit_default_padding_byte (i2c_hw);
			break;
	}
}

void i2c_dw_apb_slave_on_tx_timeout (const struct i2c_dw_apb *i2c_hw)
{
	struct i2c_dw_apb_slave_state *state = (struct i2c_dw_apb_slave_state*) i2c_hw->state;

	state->txn_state = I2C_DW_APB_SLAVE_TXN_STATE_TX_PAD;
}

void i2c_dw_apb_slave_on_tx_abort (const struct i2c_dw_apb *i2c_hw)
{
	struct i2c_dw_apb_slave_state *state = (struct i2c_dw_apb_slave_state*) i2c_hw->state;

	i2c_dw_apb_try_call_tx_abort (i2c_hw);
	state->txn_state = I2C_DW_APB_SLAVE_TXN_STATE_TX;
}

void i2c_dw_apb_slave_on_tx_depleted (const struct i2c_dw_apb *i2c_hw)
{
	const struct i2c_dw_apb_slave_handler *handler =
		(const struct i2c_dw_apb_slave_handler*) i2c_hw->i2c_handler;

	if (handler->on_tx_depleted) {
		handler->on_tx_depleted (&handler->rx_handler.handler_base);
	}
}

uint32_t i2c_dw_apb_slave_on_restart (const struct i2c_dw_apb *i2c_hw)
{
	const struct i2c_dw_apb_slave *i2c = (const struct i2c_dw_apb_slave*) i2c_hw;

	i2c_dw_apb_slave_txn_complete (i2c, I2C_DW_APB_SLAVE_TXN_STATE_ACTIVE);

	return i2c_dw_apb_slave_start_intr_mask ();
}

void i2c_dw_apb_slave_on_stop (const struct i2c_dw_apb *i2c_hw)
{
	const struct i2c_dw_apb_slave *i2c = (const struct i2c_dw_apb_slave*) i2c_hw;

	i2c_dw_apb_slave_txn_complete (i2c, I2C_DW_APB_SLAVE_TXN_STATE_IDLE);
}

/* API */

/**
 * Initialize an I2C slave driver instance.  The I2C block will be configured, but it will remain
 * disabled.
 *
 * @param i2c The I2C slave driver instance.
 * @param state The variable context for the I2C driver.  This must not already be initialized.
 * @param regs Base address for the I2C HW registers.
 * @param handler I2C event handler for the device.
 * @param address The slave address to use for the interface.
 * @param mode The highest speed mode the slave is expected to support.  The I2C slave doesn't drive
 * the clock, but it uses this information to configure the appropriate spike filter.
 * @param ic_clk Frequency of the clock for the I2C block, in Hz.
 * @param scl_timeout_ms The timeout in milliseconds for the HW for the SCL stuck at low interrupt.
 * If 0 is specified, the timeout will be set to the default value.
 *
 * @return 0 if the driver was successfully initialized, else an error code.
 */
int i2c_dw_apb_slave_init (struct i2c_dw_apb_slave *i2c, struct i2c_dw_apb_slave_state *state,
	struct Creg_regs_DW_apb_i2c_APB_Slave *regs, const struct i2c_dw_apb_slave_handler *handler,
	uint16_t slave_address, enum i2c_dw_apb_speed mode, uint32_t ic_clk, unsigned scl_timeout_ms)
{
	int status;

	if ((i2c == NULL) || (state == NULL) || (handler == NULL)) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	memset (i2c, 0, sizeof (*i2c));

	i2c->i2c_base.on_shutdown = i2c_dw_apb_slave_on_shutdown;
	i2c->i2c_base.on_start = i2c_dw_apb_slave_on_start;
	i2c->i2c_base.on_rx_discard = i2c_dw_apb_slave_on_rx_discard;
	i2c->i2c_base.on_rx_full = i2c_dw_apb_slave_on_rx_full;
	i2c->i2c_base.on_read_request = i2c_dw_apb_slave_on_read_request;
	i2c->i2c_base.on_tx_timeout = i2c_dw_apb_slave_on_tx_timeout;
	i2c->i2c_base.on_tx_abort = i2c_dw_apb_slave_on_tx_abort;
	i2c->i2c_base.on_tx_depleted = i2c_dw_apb_slave_on_tx_depleted;
	i2c->i2c_base.on_restart = i2c_dw_apb_slave_on_restart;
	i2c->i2c_base.on_stop = i2c_dw_apb_slave_on_stop;

	status = i2c_dw_apb_init (&i2c->i2c_base, &state->state_base, regs,
		&handler->rx_handler.handler_base, I2C_DW_APB_CAPABILITIES_SLAVE_MODE);
	if (status != 0) {
		return status;
	}

	status = i2c_dw_apb_slave_init_state (i2c);
	if (status != 0) {
		return status;
	}

	status = i2c_dw_apb_slave_init_hw (i2c, slave_address, mode, ic_clk, scl_timeout_ms);
	if (status != 0) {
		i2c_dw_apb_slave_release (i2c);
	}

	return status;
}

/**
 * Initialize only the variable state of the I2C slave driver instance.  The rest of the driver
 * structure is assumed to have already been initialized.
 *
 * @param i2c The I2C slave driver instance.
 *
 * @return 0 if the state was successfully initialized, else an error code.
 */
int i2c_dw_apb_slave_init_state (const struct i2c_dw_apb_slave *i2c)
{
	struct i2c_dw_apb_slave_state *state;

	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	state = (struct i2c_dw_apb_slave_state*) i2c->i2c_base.state;
	if (state == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	memset (state, 0, sizeof (*state));

	return i2c_dw_apb_init_state (&i2c->i2c_base);
}

/**
 * Initializes the I2C DW APB HW block for SLAVE mode operation.  This leaves the HW disabled.
 *
 * @param i2c The I2C slave driver instance.
 * @param slave_address The address to configure the I2C SLAVE hW block.
 * @param mode The highest speed mode the slave is expected to support.  The I2C slave doesn't drive
 * the clock, but it uses this information to configure the appropriate spike filter.
 * @param ic_clk Frequency of the clock for the I2C block, in Hz.
 * @param scl_timeout_ms The timeout in milliseconds for the HW for the SCL stuck at low interrupt.
 * If 0 is specified, the timeout will be set to the default value.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_slave_init_hw (const struct i2c_dw_apb_slave *i2c, uint16_t slave_address,
	enum i2c_dw_apb_speed mode,	uint32_t ic_clk, unsigned scl_timeout_ms)
{
	int status;

	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	status = i2c_dw_apb_init_hw (&i2c->i2c_base, mode, ic_clk);
	if (status != 0) {
		return status;
	}

	status = i2c_dw_apb_set_slave_address (&i2c->i2c_base, slave_address);
	if (status != 0) {
		return status;
	}

	status = i2c_dw_apb_set_scl_timeout (&i2c->i2c_base, ic_clk, scl_timeout_ms);
	if (status != 0) {
		return status;
	}

	status = i2c_dw_apb_configure_slave_mode (&i2c->i2c_base);

	return status;
}

/**
 * Release the resources used for an I2C slave driver.
 *
 * @param i2c The I2C slave driver instance.
 */
void i2c_dw_apb_slave_release (const struct i2c_dw_apb_slave *i2c)
{
	if (i2c) {
		i2c_dw_apb_release (&i2c->i2c_base);
	}
}

/**
 * Disables global interrupts and configures the driver to service a stalled read request.  To
 * resume servicing I2C interrupts and handle the read request, write the critical value to mstatus.
 *
 * If there is an error, the previous value of mstatus is restored and critical context exited.
 *
 * @param i2c The I2C slave driver context.
 * @param critical A pointer to store the critical section state context.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_slave_begin_read_request_update (const struct i2c_dw_apb_slave *i2c,
	uintptr_t *critical)
{
	struct i2c_dw_apb_slave_state *state;
	int status;

	if ((i2c == NULL) || (critical == NULL)) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	state = (struct i2c_dw_apb_slave_state*) i2c->i2c_base.state;

	status = 0;
	*critical = hsp_trap_mstatus_mie_disable ();
	if (state->txn_state == I2C_DW_APB_SLAVE_TXN_STATE_TX_WAIT) {
		status = i2c_dw_apb_try_read_request (&i2c->i2c_base);
		if (status != 0) {
			hsp_trap_mstatus_write (*critical);
		}
	}

	return status;
}

/**
 * Configures the driver to discard the current Rx transaction.
 *
 * This should only be called in an ISR safe context.
 *
 * @param i2c The I2C slave driver instance.
 * @param data A pointer to return the discard buffer to.
 *
 * @return The length of the returned discard buffer.
 */
size_t i2c_dw_apb_slave_discard_rx (const struct i2c_dw_apb_slave *i2c, uint8_t **data)
{
	struct i2c_dw_apb_slave_state *state;

	if (i2c == NULL) {
		return 0;
	}

	state = (struct i2c_dw_apb_slave_state*) i2c->i2c_base.state;
	if (state->txn_state != I2C_DW_APB_SLAVE_TXN_STATE_RX) {
		return 0;
	}

	state->txn_state = I2C_DW_APB_SLAVE_TXN_STATE_RX_DISCARD;

	return i2c_dw_apb_rx_discard_buffer (data);
}

/**
 * Begins transmitting default padding bytes to respond to a read request.
 *
 * This should only be called in an ISR safe context.
 *
 * @param i2c The I2C slave driver instance.
 * @param data A pointer to return the padding buffer to.
 *
 * @return THe length of the returned padding buffer.
 */
size_t i2c_dw_apb_slave_transmit_default_pad_bytes (const struct i2c_dw_apb_slave *i2c,
	const uint8_t **data)
{
	struct i2c_dw_apb_slave_state *state;

	if (i2c == NULL) {
		return 0;
	}

	state = (struct i2c_dw_apb_slave_state*) i2c->i2c_base.state;
	if (state->txn_state != I2C_DW_APB_SLAVE_TXN_STATE_TX) {
		return 0;
	}

	state->txn_state = I2C_DW_APB_SLAVE_TXN_STATE_TX_PAD;

	return i2c_dw_apb_tx_pad_buffer (data);
}

/**
 * Preemptively configures the driver to transmit default padding bytes for future read requests.
 *
 * This should only be called in an ISR safe context.
 *
 * @param i2c The I2C slave driver instance.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_slave_pad_tx_with_default_byte (const struct i2c_dw_apb_slave *i2c)
{
	struct i2c_dw_apb_slave_state *state;

	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	state = (struct i2c_dw_apb_slave_state*) i2c->i2c_base.state;
	if (state->txn_state == I2C_DW_APB_SLAVE_TXN_STATE_TX) {
		state->txn_state = I2C_DW_APB_SLAVE_TXN_STATE_TX_PAD;
	}

	return 0;
}
