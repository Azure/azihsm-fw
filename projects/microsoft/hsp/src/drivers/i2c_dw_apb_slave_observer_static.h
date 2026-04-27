// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef I2C_DW_APB_SLAVE_OBSERVER_STATIC_H_
#define I2C_DW_APB_SLAVE_OBSERVER_STATIC_H_

#include "drivers/i2c_dw_apb_slave_handler_static.h"
#include "drivers/i2c_dw_apb_slave_observer.h"


/* Internal functions declared to allow for static initialization */

void i2c_dw_apb_slave_observer_on_shutdown (const struct i2c_dw_apb_handler *handler);
size_t i2c_dw_apb_slave_observer_on_rx_pending (const struct i2c_dw_apb_handler *handler,
	uint8_t **data);
void i2c_dw_apb_slave_observer_on_rx_data (const struct i2c_dw_apb_handler *handler, size_t count);
void i2c_dw_apb_slave_observer_on_rx_discard (const struct i2c_dw_apb_handler *handler);
void i2c_dw_apb_slave_observer_on_rx_complete (const struct i2c_dw_apb_handler *handler);
size_t i2c_dw_apb_slave_observer_on_read_request (const struct i2c_dw_apb_handler *handler,
	const uint8_t **data);
void i2c_dw_apb_slave_observer_on_tx_abort (const struct i2c_dw_apb_handler *handler);
void i2c_dw_apb_slave_observer_on_tx_depleted (const struct i2c_dw_apb_handler *handler);
void i2c_dw_apb_slave_observer_on_tx_complete (const struct i2c_dw_apb_handler *handler);


/* Static initializer API */

/**
 * Initialize a static instance of an I2C DW APB slave observer handler.
 *
 * There is no validation done on the arguments.
 *
 * @param shutdown_func Callback pointer to the on_shutdown event.
 * @param rx_pending_func Callback pointer to the on_rx_pending event.
 * @param rx_data_func Callback pointer to the on_rx_data event.
 * @param rx_discard_func Callback pointer to the on_rx_discard event.
 * @param rx_complete_func Callback pointer to the on_rx_complete event.
 * @param read_req_func Callback pointer to the on_read_request event.
 * @param tx_abort_func Callback pointer to the on_tx_abort event.
 * @param tx_depleted_func Callback pointer to the on_tx_depleted event.
 * @param tx_complete_func Callback pointer to the on_tx_complete event.
 */
#define i2c_dw_apb_slave_observer_handler_static_init(shutdown_func, rx_pending_func, \
	rx_data_func, rx_discard_func, rx_complete_func, read_req_func, tx_abort_func, \
	tx_depleted_func, tx_complete_func) { \
		.on_shutdown = shutdown_func, \
		.on_rx_pending = rx_pending_func, \
		.on_rx_data = rx_data_func, \
		.on_rx_discard = rx_discard_func, \
		.on_rx_complete = rx_complete_func, \
		.on_read_request = read_req_func, \
		.on_tx_abort = tx_abort_func, \
		.on_tx_depleted = tx_depleted_func, \
		.on_tx_complete = tx_complete_func, \
	}

/**
 * Initialize a static instance of an I2C DW APB slave observer.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the I2C driver.
 * @param i2c_regs The underlying I2C slave driver.
 * @param handler_ptr A pointer to an I2C observer handler implementation.
 */
#define	i2c_dw_apb_slave_observer_static_init(i2c_ptr, handler_ptr) { \
		.i2c_handler = i2c_dw_apb_slave_handler_static_init ( \
			i2c_dw_apb_slave_observer_on_shutdown, i2c_dw_apb_slave_observer_on_rx_pending, \
			i2c_dw_apb_slave_observer_on_rx_data, i2c_dw_apb_slave_observer_on_rx_discard, \
			i2c_dw_apb_slave_observer_on_rx_complete, i2c_dw_apb_slave_observer_on_read_request, \
			i2c_dw_apb_slave_observer_on_tx_abort, i2c_dw_apb_slave_observer_on_tx_depleted, \
			i2c_dw_apb_slave_observer_on_tx_complete), \
		.i2c_hw = i2c_ptr, \
		.handler = handler_ptr, \
	}


#endif	/* I2C_DW_APB_SLAVE_OBSERVER_STATIC_H_ */
