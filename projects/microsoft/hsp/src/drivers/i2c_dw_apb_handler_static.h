// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef I2C_DW_APB_HANDLER_STATIC_H_
#define I2C_DW_APB_HANDLER_STATIC_H_

#include "drivers/i2c_dw_apb_handler.h"


/* Static initializer API. */

/**
 * Initialize a static instance of a base I2C DW APB handler.  This is not a top level API and must
 * only be called by a mode driver API.
 *
 * There is no validation done on the arguments.
 *
 * @param shutdown_func Callback pointer to the on_shutdown event.
 * @param rx_discard_func Callback pointer to the on_rx_discard event.
 * @param rx_complete_func Callback pointer to the on_rx_complete event.
 * @param tx_abort_func Callback pointer to the on_tx_abort event.
 * @param tx_complete_func Callback pointer to the on_tx_complete event.
 */
#define i2c_dw_apb_handler_static_init(shutdown_func, rx_discard_func, rx_complete_func, \
	tx_abort_func, tx_complete_func) { \
		.on_shutdown = shutdown_func, \
		.on_rx_discard = rx_discard_func, \
		.on_rx_complete = rx_complete_func, \
		.on_tx_abort = tx_abort_func, \
		.on_tx_complete = tx_complete_func, \
	}

/**
 * Initialize a static instance of a I2C DW APB slave Rx handler.  This is not a top level API and
 * must only be called by a mode driver API.
 *
 * There is no validation done on the arguments.
 *
 * @param shutdown_func Callback pointer to the on_shutdown event.
 * @param rx_pending_func Callback pointer to the on_rx_pending event.
 * @param rx_data_func Callback pointer to the on_rx_data event.
 * @param rx_discard_func Callback pointer to the on_rx_discard event.
 * @param rx_complete_func Callback pointer to the on_rx_complete event.
 * @param tx_abort_func Callback pointer to the on_tx_abort event.
 * @param tx_complete_func Callback pointer to the on_tx_complete event.
 */
#define i2c_dw_apb_handler_slave_rx_static_init(shutdown_func, rx_pending_func, rx_data_func, \
	rx_discard_func, rx_complete_func, tx_abort_func, tx_complete_func) { \
		.handler_base = i2c_dw_apb_handler_static_init (shutdown_func, rx_discard_func, \
			rx_complete_func, tx_abort_func, tx_complete_func), \
		.on_rx_pending = rx_pending_func, \
		.on_rx_data = rx_data_func, \
	}


#endif	/* I2C_DW_APB_HANDLER_STATIC_H_ */
