// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef I2C_DW_APB_MULTIMASTER_HANDLER_H_
#define I2C_DW_APB_MULTIMASTER_HANDLER_H_

#include "drivers/i2c_dw_apb_handler.h"


/**
 * Handler for the I2C multi-master driver implementation.
 */
struct i2c_dw_apb_multimaster_handler {
	struct i2c_dw_apb_handler_slave_rx rx_handler;	/**< I2C SLAVE Rx event handler. */
};


#endif	/* I2C_DW_APB_MULTIMASTER_HANDLER_H_ */
