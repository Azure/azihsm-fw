// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef I2C_DW_APB_HANDLER_H_
#define I2C_DW_APB_HANDLER_H_

#include <stddef.h>
#include <stdint.h>


/**
 * Base handler for the upper implementation layer for common events.
 */
struct i2c_dw_apb_handler {
	/**
	 * Callback when the driver has successfully shutdown.
	 *
	 * This may be called in a USER context and can be NULL if this event notification is not
	 * needed.
	 *
	 * @param handler The calling I2C handler instance.
	 */
	void (*on_shutdown) (const struct i2c_dw_apb_handler *handler);

	/**
	 * Callback when the current Rx transaction is being discarded due to an error or cancellation.
	 *
	 * This is called in an ISR context and must be handled.
	 *
	 * @param handler The calling I2C handler instance.
	 */
	void (*on_rx_discard) (const struct i2c_dw_apb_handler *handler);

	/**
	 * Callback when the current Rx transaction has completed.
	 *
	 * This is called in an ISR context and must be handled.
	 *
	 * @param handler The calling I2C handler instance.
	 */
	void (*on_rx_complete) (const struct i2c_dw_apb_handler *handler);

	/**
	 * Callback when the current Tx transaction has been aborted.
	 *
	 * This is called in an ISR context and can be NULL if this event notification is not needed.
	 *
	 * @param handler The calling I2C handler instance.
	 */
	void (*on_tx_abort) (const struct i2c_dw_apb_handler *handler);

	/**
	 * Callback when the current Tx transaction has completed.
	 *
	 * This is called in an ISR context and can be NULL if this event notification is not needed.
	 *
	 * @param handler The calling I2C handler instance.
	 */
	void (*on_tx_complete) (const struct i2c_dw_apb_handler *handler);
};

/**
 * Event handler for SLAVE mode Rx operations.  These are common to SLAVE and MULTI-MASTER mode
 * drivers.
 */
struct i2c_dw_apb_handler_slave_rx {
	struct i2c_dw_apb_handler handler_base;	/**< Base I2C handler. */

	/**
	 * Callback when there is data in the Rx FIFO and there is no buffer available to write
	 * into.
	 *
	 * If a valid buffer isn't returned, the driver will discard the transaction.
	 *
	 * This is called from an ISR context and must be handled.
	 *
	 * @param handler The calling I2C handler instance.
	 * @param data A pointer to receive a pointer to an Rx buffer.
	 *
	 * @return The length of the buffer returned by data.  If 0 is returned or data is NULL, the
	 * current Rx transaction will be discarded.  Otherwise, the data in the FIFO will be popped
	 * into the returned buffer.
	 */
	size_t (*on_rx_pending) (const struct i2c_dw_apb_handler *handler, uint8_t **data);

	/**
	 * Callback when the driver is done with the queued Rx buffer.  This can happen when the buffer
	 * is filled up or when the transaction has ended.
	 *
	 * This is called from an ISR context and must be handled.
	 *
	 * @param handler The calling I2C handler instance.
	 * @param count The count of valid data written to the buffer.
	 */
	void (*on_rx_data) (const struct i2c_dw_apb_handler *handler, size_t count);
};


#endif	/* I2C_DW_APB_HANDLER_H_ */
