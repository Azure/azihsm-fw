// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_I2C_H_
#define INIT_I2C_H_

#include "fips/cmd_channel_error_state_static.h"
#include "manifest/pcd/pcd.h"

extern struct device_manager device_manager;
extern const struct cmd_channel_error_state fips_i2c;


int initialize_i2c_interface (const struct pcd *active_pcd);
int start_i2c_interface ();


#endif	/* INIT_I2C_H_ */
