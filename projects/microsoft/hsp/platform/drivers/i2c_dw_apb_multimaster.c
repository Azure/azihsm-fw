// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <string.h>
#include "platform_config.h"
#include "common/unused.h"
#include "drivers/i2c_dw_apb_error.h"
#include "drivers/i2c_dw_apb_multimaster_static.h"


/**
 * Calls the I2C handler callbacks based on the current HW mode.
 *
 * @param i2c The I2C slave driver instance.
 * @param txn_state The transaction state to initialize the driver state.
 */
static void i2c_dw_apb_multimaster_txn_complete (const struct i2c_dw_apb *i2c_hw)
{
	switch (i2c_dw_apb_get_mode (i2c_hw)) {
		case I2C_DW_APB_MODE_SLAVE:
			i2c_dw_apb_slave_rx_call_rx_data (i2c_hw);
			i2c_dw_apb_call_rx_complete (i2c_hw);
			break;

		case I2C_DW_APB_MODE_MASTER:
			i2c_dw_apb_try_call_tx_complete (i2c_hw);
			break;
	}
}

/* Driver Mode Implementation */

void i2c_dw_apb_multimaster_on_shutdown (const struct i2c_dw_apb *i2c_hw)
{
	i2c_dw_apb_call_on_shutdown (i2c_hw);
}

uint32_t i2c_dw_apb_multimaster_on_start (const struct i2c_dw_apb *i2c_hw)
{
	uint32_t intr_mask = 0;

	switch (i2c_dw_apb_get_mode (i2c_hw)) {
		case I2C_DW_APB_MODE_SLAVE:
			intr_mask = i2c_dw_apb_slave_start_intr_mask ();
			break;

		case I2C_DW_APB_MODE_MASTER:
			intr_mask = i2c_dw_apb_tx_intr_mask ();
			break;
	}

	return intr_mask;
}

void i2c_dw_apb_multimaster_on_rx_full (const struct i2c_dw_apb *i2c_hw)
{
	i2c_dw_apb_slave_rx_call_rx_data (i2c_hw);
	i2c_dw_apb_slave_rx_call_rx_pending (i2c_hw);
}

void i2c_dw_apb_multimaster_on_rx_discard (const struct i2c_dw_apb *i2c_hw)
{
	i2c_dw_apb_call_rx_discard (i2c_hw);
}

void i2c_dw_apb_multimaster_on_read_request (const struct i2c_dw_apb *i2c_hw)
{
	// Shouldn't be getting this request for a multi-master device. Send dummy byte.
	i2c_dw_apb_transmit_default_padding_byte (i2c_hw);
}

void i2c_dw_apb_multimaster_on_tx_abort (const struct i2c_dw_apb *i2c_hw)
{
	if (i2c_dw_apb_get_mode (i2c_hw) == I2C_DW_APB_MODE_MASTER) {
		i2c_dw_apb_try_call_tx_abort (i2c_hw);
	}
}

uint32_t i2c_dw_apb_multimaster_on_restart (const struct i2c_dw_apb *i2c_hw)
{
	uint32_t intr_mask = 0;

	i2c_dw_apb_multimaster_txn_complete (i2c_hw);

	// RESTART would only be useful in SLAVE mode Rx
	if (i2c_dw_apb_get_mode (i2c_hw) == I2C_DW_APB_MODE_SLAVE) {
		intr_mask = i2c_dw_apb_slave_start_intr_mask ();
	}

	return intr_mask;
}

void i2c_dw_apb_multimaster_on_stop (const struct i2c_dw_apb *i2c_hw)
{
	i2c_dw_apb_multimaster_txn_complete (i2c_hw);
}

/* API */

/**
 * Initialize an I2C multi-master driver instance.  The I2C block will be configured, but it will
 * remain disabled.
 *
 * @param i2c The I2C multi-master driver instance.
 * @param state The variable context for the I2C driver.  This must not already be initialized.
 * @param regs Base address for the I2C HW registers.
 * @param handler I2C event handler for the device.
 * @param address The slave address to use for the interface.
 * @param mode The highest speed mode the device is expected to support.  The I2C slave doesn't drive
 * the clock, but it uses this information to configure the appropriate spike filter.
 * @param ic_clk Frequency of the clock for the I2C block, in Hz.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int i2c_dw_apb_multimaster_init (struct i2c_dw_apb_multimaster *i2c,
	struct i2c_dw_apb_multimaster_state *state, struct Creg_regs_DW_apb_i2c_APB_Slave *regs,
	const struct i2c_dw_apb_multimaster_handler *handler, uint8_t slave_address,
	enum i2c_dw_apb_speed mode, uint32_t ic_clk)
{
	int status;

	if ((i2c == NULL) || (state == NULL) || (handler == NULL)) {
		return I2C_DW_APB_INVALID_MODE;
	}

	memset (i2c, 0, sizeof (*i2c));

	i2c->i2c_base.on_shutdown = i2c_dw_apb_multimaster_on_shutdown;
	i2c->i2c_base.on_start = i2c_dw_apb_multimaster_on_start;
	i2c->i2c_base.on_rx_full = i2c_dw_apb_multimaster_on_rx_full;
	i2c->i2c_base.on_rx_discard = i2c_dw_apb_multimaster_on_rx_discard;
	i2c->i2c_base.on_read_request = i2c_dw_apb_multimaster_on_read_request;
	i2c->i2c_base.on_tx_abort = i2c_dw_apb_multimaster_on_tx_abort;
	i2c->i2c_base.on_restart = i2c_dw_apb_multimaster_on_restart;
	i2c->i2c_base.on_stop = i2c_dw_apb_multimaster_on_stop;

	status = i2c_dw_apb_init (&i2c->i2c_base, &state->state_base, regs,
		&handler->rx_handler.handler_base, I2C_DW_APB_CAPABILITIES_MULTIMASTER_MODE);
	if (status != 0) {
		return status;
	}

	status = i2c_dw_apb_multimaster_init_state (i2c);
	if (status != 0) {
		return status;
	}

	status = i2c_dw_apb_multimaster_init_hw (i2c, slave_address, mode, ic_clk);
	if (status != 0) {
		i2c_dw_apb_multimaster_release (i2c);
	}

	return status;
}

/**
 * Initialize only the variable state of the I2C multi-master driver instance.
 *
 * @param i2c The I2C multi-master driver instance.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int i2c_dw_apb_multimaster_init_state (const struct i2c_dw_apb_multimaster *i2c)
{
	struct i2c_dw_apb_multimaster_state *state;

	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	state = (struct i2c_dw_apb_multimaster_state*) i2c->i2c_base.state;
	if (state == NULL) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	memset (state, 0, sizeof (*state));

	return i2c_dw_apb_init_state (&i2c->i2c_base);
}

/**
 * Initializes the I2C DW APB HW block for multi-master mode operation.  This leaves the HW
 * disabled.
 *
 * @param i2c The I2C slave driver instance.
 * @param slave_address The address to configure the I2C SLAVE hW block.
 * @param mode The highest speed mode the slave is expected to support.  The I2C slave doesn't drive
 * the clock, but it uses this information to configure the appropriate spike filter.
 * @param ic_clk Frequency of the clock for the I2C block, in Hz.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_multimaster_init_hw (const struct i2c_dw_apb_multimaster *i2c, uint8_t slave_address,
	enum i2c_dw_apb_speed mode, uint32_t ic_clk)
{
	int status;

	if (i2c == NULL) {
		return I2C_DW_APB_INVALID_MODE;
	}

	status = i2c_dw_apb_init_hw (&i2c->i2c_base, mode, ic_clk);
	if (status != 0) {
		return status;
	}

	status = i2c_dw_apb_set_slave_address (&i2c->i2c_base, slave_address);
	if (status != 0) {
		return status;
	}

	status = i2c_dw_apb_configure_slave_mode (&i2c->i2c_base);

	return status;
}

/**
 * Release the resources used for an I2C multi-master driver.
 *
 * @param i2c The I2C multi-master driver instance.
 */
void i2c_dw_apb_multimaster_release (const struct i2c_dw_apb_multimaster *i2c)
{
	if (i2c) {
		i2c_dw_apb_release (&i2c->i2c_base);
	}
}
