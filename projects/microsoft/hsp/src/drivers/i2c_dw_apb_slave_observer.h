// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef I2C_DW_APB_SLAVE_OBSERVER_H_
#define I2C_DW_APB_SLAVE_OBSERVER_H_

#include "drivers/i2c_dw_apb_slave_handler.h"


/**
 * Event handler for the I2C DW APB slave observer.
 */
struct i2c_dw_apb_slave_observer_handler {
	/**
	 * Callback when the driver has successfully shutdown.
	 *
	 * This may be called in a USER context and can be NULL if this event notification is not
	 * needed.
	 *
	 * @param handler The calling observer handler instance.
	 */
	void (*on_shutdown) (const struct i2c_dw_apb_slave_observer_handler *handler);

	/**
	 * Callback when there is data in the Rx FIFO and there is no buffer available to write
	 * into.
	 *
	 * If a valid buffer isn't returned, the driver will discard the transaction.
	 *
	 * This is called from an ISR context and must be handled.
	 *
	 * @param handler The calling I2C observer instance.
	 * @param address The address of the HW being addressed.
	 * @param data A pointer to receive a pointer to an Rx buffer.
	 *
	 * @return The length of the buffer returned by data.  If 0 is returned or data is NULL, the
	 * current Rx transaction will be discarded.  Otherwise, the data in the FIFO will be popped
	 * into the returned buffer.
	 */
	size_t (*on_rx_pending) (const struct i2c_dw_apb_slave_observer_handler *handler,
		uint16_t address, uint8_t **data);

	/**
	 * Callback when the current Rx transaction is being discarded due to an error or cancelation.
	 *
	 * This is called in an ISR context and must be handled.
	 *
	 * @param handler The calling observer handler instance.
	 */
	void (*on_rx_discard) (const struct i2c_dw_apb_slave_observer_handler *handler);

	/**
	 * Callback when the driver is done with the queued Rx buffer.  This can happen when the buffer
	 * is filled up or when the transaction has ended.
	 *
	 * This is called from an ISR context and must be handled.
	 *
	 * @param handler The calling observer handler instance.
	 * @param count The count of valid data written to the buffer.
	 */
	void (*on_rx_data) (const struct i2c_dw_apb_slave_observer_handler *handler, size_t count);

	/**
	 * Callback when the current Rx transaction has completed.
	 *
	 * This is called in an ISR context and must be handled.
	 *
	 * @param handler The calling observer handler instance.
	 */
	void (*on_rx_complete) (const struct i2c_dw_apb_slave_observer_handler *handler);

	/**
	 * Callback when a MASTER is requesting data from this I2C instance.
	 *
	 * If a valid buffer is not returned by this callback, the driver will transmit default
	 * padding bytes.
	 *
	 * This is called in an ISR context and must be handled.
	 *
	 * @param i2c The calling I2C handler instance.
	 * @param address The address of the HW being addressed.
	 * @param data A pointer to return a pointer of data to transmit.
	 *
	 * @return The length of the buffer returned by data.  If 0 is returned or data is NULL, the
	 * driver will transmit default padding bytes.  Otherwise, the data will be pushed into the
	 * FIFO.
	 */
	size_t (*on_read_request) (const struct i2c_dw_apb_slave_observer_handler *handler,
		uint16_t address, const uint8_t **data);

	/**
	 * Callback when the current Tx transaction has been aborted.
	 *
	 * This is called in an ISR context and can be NULL if this event notification is not needed.
	 *
	 * @param handler The calling observer handler instance.
	 */
	void (*on_tx_abort) (const struct i2c_dw_apb_slave_observer_handler *handler);

	/**
	 * Callback when all the data in the current Tx buffer has been pushed to the FIFO.  This does
	 * not indicate that all the data in the FIFO has been received by the other endpoint.
	 *
	 * This is called in an ISR context and can be NULL if this event notification is not needed.
	 *
	 * @param i2c The calling observer handler instance.
	 */
	void (*on_tx_depleted) (const struct i2c_dw_apb_slave_observer_handler *handler);

	/**
	 * Callback when the current Tx transaction has completed.
	 *
	 * This is called in an ISR context and can be NULL if this event notification is not needed.
	 *
	 * @param handler The calling observer handler instace.
	 */
	void (*on_tx_complete) (const struct i2c_dw_apb_slave_observer_handler *handler);
};

struct i2c_dw_apb_slave;

/**
 * SLAVE mode event observer instance for a DW APB I2C instance.  This is meant for implementations
 * that do not need advanced HW control for transactions.
 */
struct i2c_dw_apb_slave_observer {
	struct i2c_dw_apb_slave_handler i2c_handler;				/**< I2C slave handler implementation. */
	const struct i2c_dw_apb_slave *i2c_hw;						/**< Underlying I2C driver instance. */
	const struct i2c_dw_apb_slave_observer_handler *handler;	/**< Event handler for the observer instance. */
};


int i2c_dw_apb_slave_observer_init (struct i2c_dw_apb_slave_observer *observer,
	const struct i2c_dw_apb_slave *i2c_hw, const struct i2c_dw_apb_slave_observer_handler *handler);
void i2c_dw_apb_slave_observer_release (const struct i2c_dw_apb_slave_observer *observer);


#endif	/* I2C_DW_APB_SLAVE_OBSERVER_H_ */
