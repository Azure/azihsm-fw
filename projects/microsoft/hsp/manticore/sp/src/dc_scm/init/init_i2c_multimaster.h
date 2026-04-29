// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_I2C_MULTIMASTER_H_
#define INIT_I2C_MULTIMASTER_H_

#include "cmd_interface/cmd_channel_i2c_dw_apb_multimaster_static.h"
#include "drivers/i2c_dw_apb_multimaster_static.h"


extern const struct i2c_dw_apb_multimaster i2c_hw;
extern const struct cmd_channel_i2c_dw_apb_multimaster system_i2c;


int initialize_i2c_driver (uint8_t i2c_slave_addr, struct cmd_packet *system_rx_buffers,
	size_t system_rx_buffers_size);


#endif	/* INIT_I2C_MULTIMASTER_H_ */
