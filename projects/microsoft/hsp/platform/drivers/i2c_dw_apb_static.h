// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef I2C_DW_APB_STATIC_H_
#define I2C_DW_APB_STATIC_H_

#include "drivers/i2c_dw_apb.h"
#include "trap/hsp_interrupt_handler_static.h"


/* Internal functions declared to allow for static initialization. */

bool i2c_dw_apb_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param);


/* Static initializer API for derived types. */

/**
 * Initialize a static driver instance of a base I2C DW APB context.  This is not a top level API
 * and must only be called by a mode driver API.  This does not initialize the state context which
 * must be initialized by a top level API that calls i2c_dw_apb_init_state.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the I2C driver.
 * @param i2c_regs The underlying I2C HW register set for the driver.
 * @param caps_mask A i2c_dw_apb_capabilities mask that indicates what operation the implementation
 * supports.
 * @param handler_ptr A pointer to an I2C handler implementation.
 * @param shutdown_func Callback pointer for the on_shutdown event.
 * @param start_func Callback pointer for the on_start event.
 * @param rx_full_func Callback pointer for the on_rx_full event.
 * @param rx_over_func Callback pointer for the on_rx_overflow event.
 * @param read_req_func Callback pointer for the on_read_request event.
 * @param tx_timeout_func Callback pointer for the on_tx_timeout event.
 * @param tx_abort_func Callback pointer for the on_tx_abort event.
 * @param tx_depleted_func Callback pointer for the on_tx_depleted event.
 * @param restart_func Callback pointer for the on_restart event.
 * @param stop_func Callback pointer for the on_stop event.
 */
#define	i2c_dw_apb_static_init(state_ptr, i2c_regs, caps_mask, handler_ptr, shutdown_func, \
	start_func, rx_full_func, rx_over_func, read_req_func, tx_timeout_func, tx_abort_func, \
	tx_depleted_func, restart_func, stop_func) { \
		.isr_handler = hsp_interrupt_handler_static_init (i2c_dw_apb_handle_interrupt), \
		.on_shutdown = shutdown_func, \
		.on_start = start_func, \
		.on_rx_full = rx_full_func, \
		.on_rx_discard = rx_over_func, \
		.on_read_request = read_req_func, \
		.on_tx_timeout = tx_timeout_func, \
		.on_tx_abort = tx_abort_func, \
		.on_tx_depleted = tx_depleted_func, \
		.on_restart = restart_func, \
		.on_stop = stop_func, \
		.state = state_ptr, \
		.regs = i2c_regs, \
		.i2c_handler = handler_ptr, \
		.capabilities = caps_mask, \
	}


#endif	/* I2C_DW_APB_STATIC_H_ */
