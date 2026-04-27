// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef I2C_DW_APB_MULTIMASTER_STATIC_H_
#define I2C_DW_APB_MULTIMASTER_STATIC_H_

#include "drivers/i2c_dw_apb_multimaster.h"
#include "drivers/i2c_dw_apb_static.h"


/* Internal functions declared to allow for static initialization. */

void i2c_dw_apb_multimaster_on_shutdown (const struct i2c_dw_apb *i2c_hw);
uint32_t i2c_dw_apb_multimaster_on_start (const struct i2c_dw_apb *i2c_hw);
void i2c_dw_apb_multimaster_on_rx_full (const struct i2c_dw_apb *i2c_hw);
void i2c_dw_apb_multimaster_on_rx_discard (const struct i2c_dw_apb *i2c_hw);
void i2c_dw_apb_multimaster_on_read_request (const struct i2c_dw_apb *i2c_hw);
void i2c_dw_apb_multimaster_on_tx_abort (const struct i2c_dw_apb *i2c_hw);
uint32_t i2c_dw_apb_multimaster_on_restart (const struct i2c_dw_apb *i2c_hw);
void i2c_dw_apb_multimaster_on_stop (const struct i2c_dw_apb *i2c_hw);

/* Static initializer API. */

/**
 * Initialize a static instance of an I2C DW APB multimaster driver.  This does not initialize the
 * driver state which must be initialized separately with i2c_slave_dw_apb_init_state.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the I2C driver.
 * @param i2c_regs The underlying I2C HW register set for the driver.
 * @param handler_ptr A pointer to an I2C handler implementation.
 */
#define	i2c_dw_apb_multimaster_static_init(state_ptr, i2c_regs, handler_ptr) { \
		.i2c_base = i2c_dw_apb_static_init (&(state_ptr)->state_base, i2c_regs, \
			I2C_DW_APB_CAPABILITIES_MULTIMASTER_MODE, &(handler_ptr)->rx_handler.handler_base, \
			i2c_dw_apb_multimaster_on_shutdown, i2c_dw_apb_multimaster_on_start, \
			i2c_dw_apb_multimaster_on_rx_full, i2c_dw_apb_multimaster_on_rx_discard, \
			i2c_dw_apb_multimaster_on_read_request, NULL, i2c_dw_apb_multimaster_on_tx_abort, \
			NULL, i2c_dw_apb_multimaster_on_restart, i2c_dw_apb_multimaster_on_stop), \
	}


#endif	/* I2C_DW_APB_MULTIMASTER_STATIC_H_ */
