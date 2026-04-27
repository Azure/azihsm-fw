// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_H_
#define I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_H_

#include <stdbool.h>
#include "drivers/i2c_dw_apb_slave_observer.h"
#include "recovery/ocp_recovery_smbus.h"


/**
 * Variable context for the I2C slave event handler.
 */
struct i2c_dw_apb_slave_observer_ocp_recovery_state {
	volatile unsigned txn_state;	/**< Internal I2C transaction state. */
	uint8_t rx_data;				/**< Local buffer to receive I2C data into. */
};

/**
 * OCP recovery handler for I2C slave events.
 */
struct i2c_dw_apb_slave_observer_ocp_recovery {
	struct i2c_dw_apb_slave_observer_handler i2c_handler;		/**< Base handler API. */
	struct i2c_dw_apb_slave_observer_ocp_recovery_state *state;	/**< Variable context for the handler. */
	const struct ocp_recovery_smbus *smbus;						/**< OCP recovery SMBus handler. */
};


int i2c_dw_apb_slave_observer_ocp_recovery_init (
	struct i2c_dw_apb_slave_observer_ocp_recovery *recovery,
	struct i2c_dw_apb_slave_observer_ocp_recovery_state *state,
	const struct ocp_recovery_smbus *smbus);
int i2c_dw_apb_slave_observer_ocp_recovery_init_state (
	const struct i2c_dw_apb_slave_observer_ocp_recovery *recovery);
void i2c_dw_apb_slave_observer_ocp_recovery_release (
	const struct i2c_dw_apb_slave_observer_ocp_recovery *recovery);


#endif	/* I2C_DW_APB_SLAVE_OBSERVER_OCP_RECOVERY_H_ */
