// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef I2C_DW_APB_SLAVE_HANDLER_H_
#define I2C_DW_APB_SLAVE_HANDLER_H_

#include "drivers/i2c_dw_apb_handler.h"


/**
 * Handler for the I2C slave driver implementation.
 */
struct i2c_dw_apb_slave_handler {
	struct i2c_dw_apb_handler_slave_rx rx_handler;	/**< I2C slave Rx event handler. */

	/**
	 * Callback when a MASTER is requesting data from this I2C instance.
	 *
	 * If a valid buffer is not returned by this callback, the driver will automatically stall the
	 * HW until TX ABORT, STOP, or RESTART interrupt happens.  It is up to the upper implementation
	 * layer to queue a buffer and enable the RD_REQ interrupt via begin_read_request_update().
	 *
	 * This is called in an ISR context and must be handled.
	 *
	 * @param i2c The calling I2C handler instance.
	 * @param data A pointer to return a pointer of data to transmit.
	 *
	 * @return The length of the buffer returned by data.  If 0 is returned or data is NULL, the HW
	 * will be stalled.  Otherwise, the data will be pushed into the FIFO.
	 */
	size_t (*on_read_request) (const struct i2c_dw_apb_handler *handler, const uint8_t **data);

	/**
	 * Callback when all the data in the current Tx buffer has been pushed to the FIFO.  This does
	 * not indicate that all the data in the FIFO has been received by the other endpoint.
	 *
	 * This is called in an ISR context and can be NULL if this event notification is not needed.
	 *
	 * @param i2c The calling I2C handler instance.
	 */
	void (*on_tx_depleted) (const struct i2c_dw_apb_handler *handler);
};


#endif	/* I2C_DW_APB_SLAVE_HANDLER_H_ */
