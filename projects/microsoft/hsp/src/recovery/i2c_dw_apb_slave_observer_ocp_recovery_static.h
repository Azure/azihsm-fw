// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_STATIC_H_
#define I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_STATIC_H_

#include "drivers/i2c_dw_apb_slave_observer_static.h"
#include "recovery/i2c_dw_apb_slave_observer_ocp_recovery.h"


/* Internal functions declared to allow for static initialization. */
void i2c_dw_apb_slave_observer_ocp_recovery_on_shutdown (
	const struct i2c_dw_apb_slave_observer_handler *handler);
size_t i2c_dw_apb_slave_observer_ocp_recovery_on_rx_pending (
	const struct i2c_dw_apb_slave_observer_handler *handler, uint16_t address, uint8_t **data);
void i2c_dw_apb_slave_observer_ocp_recovery_on_rx_discard (
	const struct i2c_dw_apb_slave_observer_handler *handler);
void i2c_dw_apb_slave_observer_ocp_recovery_on_rx_data (
	const struct i2c_dw_apb_slave_observer_handler *handler, size_t count);
void i2c_dw_apb_slave_observer_ocp_recovery_on_rx_complete (
	const struct i2c_dw_apb_slave_observer_handler *handler);
size_t i2c_dw_apb_slave_observer_ocp_recovery_on_read_request (
	const struct i2c_dw_apb_slave_observer_handler *handler, uint16_t address,
	const uint8_t **data);
void i2c_dw_apb_slave_observer_ocp_recovery_on_tx_depleted (
	const struct i2c_dw_apb_slave_observer_handler *handler);
void i2c_dw_apb_slave_observer_ocp_recovery_on_tx_complete (
	const struct i2c_dw_apb_slave_observer_handler *handler);


/**
 * Initialize a static instance of an OCP recovery I2C handler.  This does not initialize the
 * handler state.  That will need to be initialized separately with
 * i2c_dw_apb_slave_observer_ocp_recovery_init_state.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the I2C handler.
 * @param smbus_ptr The SMBus handler for the recovery protocol.  This can be a constant instance.
 */
#define	i2c_dw_apb_slave_observer_ocp_recovery_static_init(state_ptr, smbus_ptr)	{ \
		.i2c_handler = i2c_dw_apb_slave_observer_handler_static_init ( \
			i2c_dw_apb_slave_observer_ocp_recovery_on_shutdown, \
			i2c_dw_apb_slave_observer_ocp_recovery_on_rx_pending, \
			i2c_dw_apb_slave_observer_ocp_recovery_on_rx_data, \
			i2c_dw_apb_slave_observer_ocp_recovery_on_rx_discard, \
			i2c_dw_apb_slave_observer_ocp_recovery_on_rx_complete, \
			i2c_dw_apb_slave_observer_ocp_recovery_on_read_request, NULL, \
			i2c_dw_apb_slave_observer_ocp_recovery_on_tx_depleted, \
			i2c_dw_apb_slave_observer_ocp_recovery_on_tx_complete), \
		.state = state_ptr, \
		.smbus = smbus_ptr \
	}


#endif	/* I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_STATIC_H_ */
