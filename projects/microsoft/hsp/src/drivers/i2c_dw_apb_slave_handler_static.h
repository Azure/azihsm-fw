// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef I2C_DW_APB_SLAVE_HANDLER_STATIC_H_
#define I2C_DW_APB_SLAVE_HANDLER_STATIC_H_

#include "drivers/i2c_dw_apb_handler_static.h"
#include "drivers/i2c_dw_apb_slave_handler.h"


/* Static initializer API. */

/**
 * Initialize a static instance of an I2C DW APB slave handler.
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
#define i2c_dw_apb_slave_handler_static_init(shutdown_func, rx_pending_func, rx_data_func, \
	rx_discard_func, rx_complete_func, read_req_func, tx_abort_func, tx_depleted_func, \
	tx_complete_func) { \
		.rx_handler = i2c_dw_apb_handler_slave_rx_static_init (shutdown_func, rx_pending_func, \
			rx_data_func, rx_discard_func, rx_complete_func, tx_abort_func, tx_complete_func), \
		.on_read_request = read_req_func, \
		.on_tx_depleted = tx_depleted_func, \
	}


#endif	/* I2C_DW_APB_SLAVE_HANDLER_STATIC_H_ */
