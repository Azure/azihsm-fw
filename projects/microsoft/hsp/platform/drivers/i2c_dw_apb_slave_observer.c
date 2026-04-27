// Copyright (c) Microsoft Corporation. All rights reserved.

#include "common/unused.h"
#include "drivers/i2c_dw_apb_error.h"
#include "drivers/i2c_dw_apb_slave.h"
#include "drivers/i2c_dw_apb_slave_observer_static.h"


/**
 * Gets the slave address for the underlying I2C HW driver.
 *
 * @param observer The observer instance.
 *
 * @return 0xFFFF if the there was an error, else a valid address.
 */
static uint16_t i2c_dw_apb_slave_observer_get_address (
	const struct i2c_dw_apb_slave_observer *observer)
{
	int address;

	address = i2c_dw_apb_get_slave_address (&observer->i2c_hw->i2c_base);
	if (ROT_IS_ERROR (address)) {
		// Shouldn't happen...
		return 0xFFFF;
	}

	return address;
}

void i2c_dw_apb_slave_observer_on_shutdown (const struct i2c_dw_apb_handler *handler)
{
	const struct i2c_dw_apb_slave_observer *observer =
		(const struct i2c_dw_apb_slave_observer*) handler;

	if (observer->handler->on_shutdown) {
		observer->handler->on_shutdown (observer->handler);
	}
}

size_t i2c_dw_apb_slave_observer_on_rx_pending (const struct i2c_dw_apb_handler *handler,
	uint8_t **data)
{
	const struct i2c_dw_apb_slave_observer *observer =
		(const struct i2c_dw_apb_slave_observer*) handler;
	uint16_t address;

	address = i2c_dw_apb_slave_observer_get_address (observer);

	return observer->handler->on_rx_pending (observer->handler, address, data);
}

void i2c_dw_apb_slave_observer_on_rx_data (const struct i2c_dw_apb_handler *handler, size_t count)
{
	const struct i2c_dw_apb_slave_observer *observer =
		(const struct i2c_dw_apb_slave_observer*) handler;

	observer->handler->on_rx_data (observer->handler, count);
}

void i2c_dw_apb_slave_observer_on_rx_discard (const struct i2c_dw_apb_handler *handler)
{
	const struct i2c_dw_apb_slave_observer *observer =
		(const struct i2c_dw_apb_slave_observer*) handler;

	observer->handler->on_rx_discard (observer->handler);
}

void i2c_dw_apb_slave_observer_on_rx_complete (const struct i2c_dw_apb_handler *handler)
{
	const struct i2c_dw_apb_slave_observer *observer =
		(const struct i2c_dw_apb_slave_observer*) handler;

	observer->handler->on_rx_complete (observer->handler);
}

size_t i2c_dw_apb_slave_observer_on_read_request (const struct i2c_dw_apb_handler *handler,
	const uint8_t **data)
{
	const struct i2c_dw_apb_slave_observer *observer =
		(const struct i2c_dw_apb_slave_observer*) handler;
	size_t len;
	uint16_t address;

	address = i2c_dw_apb_slave_observer_get_address (observer);
	len = observer->handler->on_read_request (observer->handler, address, data);
	if ((len == 0) || (*data == NULL)) {
		len = i2c_dw_apb_slave_transmit_default_pad_bytes (observer->i2c_hw, data);
	}

	return len;
}

void i2c_dw_apb_slave_observer_on_tx_abort (const struct i2c_dw_apb_handler *handler)
{
	const struct i2c_dw_apb_slave_observer *observer =
		(const struct i2c_dw_apb_slave_observer*) handler;

	if (observer->handler->on_tx_abort) {
		observer->handler->on_tx_abort (observer->handler);
	}
}

void i2c_dw_apb_slave_observer_on_tx_depleted (const struct i2c_dw_apb_handler *handler)
{
	const struct i2c_dw_apb_slave_observer *observer =
		(const struct i2c_dw_apb_slave_observer*) handler;

	if (observer->handler->on_tx_depleted) {
		observer->handler->on_tx_depleted (observer->handler);
	}
}

void i2c_dw_apb_slave_observer_on_tx_complete (const struct i2c_dw_apb_handler *handler)
{
	const struct i2c_dw_apb_slave_observer *observer =
		(const struct i2c_dw_apb_slave_observer*) handler;

	if (handler->on_tx_complete) {
		observer->handler->on_tx_complete (observer->handler);
	}
}

/**
 * Initializes an I2C DW APB slave observer instance.
 *
 * @param observer The I2C observer instance.
 * @param i2c_hw The underlying I2C DW APB slave driver.
 * @param handler The observer handler.
 *
 * @return 0 if successful, else an error code.
 */
int i2c_dw_apb_slave_observer_init (struct i2c_dw_apb_slave_observer *observer,
	const struct i2c_dw_apb_slave *i2c_hw, const struct i2c_dw_apb_slave_observer_handler *handler)
{
	if ((observer == NULL) || (i2c_hw == NULL) || (handler == NULL)) {
		return I2C_DW_APB_INVALID_ARGUMENT;
	}

	if ((handler->on_rx_pending == NULL) || (handler->on_rx_discard == NULL) ||
		(handler->on_rx_data == NULL) || (handler->on_rx_complete == NULL) ||
		(handler->on_read_request == NULL)) {
		return I2C_DW_APB_MISSING_HANDLERS;
	}

	observer->i2c_handler.rx_handler.handler_base.on_shutdown =
		i2c_dw_apb_slave_observer_on_shutdown;
	observer->i2c_handler.rx_handler.handler_base.on_rx_discard =
		i2c_dw_apb_slave_observer_on_rx_discard;
	observer->i2c_handler.rx_handler.handler_base.on_rx_complete =
		i2c_dw_apb_slave_observer_on_rx_complete;
	observer->i2c_handler.rx_handler.handler_base.on_tx_abort =
		i2c_dw_apb_slave_observer_on_tx_abort;
	observer->i2c_handler.rx_handler.handler_base.on_tx_complete =
		i2c_dw_apb_slave_observer_on_tx_complete;
	observer->i2c_handler.rx_handler.on_rx_pending = i2c_dw_apb_slave_observer_on_rx_pending;
	observer->i2c_handler.rx_handler.on_rx_data = i2c_dw_apb_slave_observer_on_rx_data;
	observer->i2c_handler.on_read_request = i2c_dw_apb_slave_observer_on_read_request;
	observer->i2c_handler.on_tx_depleted = i2c_dw_apb_slave_observer_on_tx_depleted;

	observer->i2c_hw = i2c_hw;
	observer->handler = handler;

	return 0;
}

/**
 * Releases resources held by the I2C DW APB slave observer instance.
 *
 * @param observer The I2C observer instance.
 */
void i2c_dw_apb_slave_observer_release (const struct i2c_dw_apb_slave_observer *observer)
{
	UNUSED (observer);
}
