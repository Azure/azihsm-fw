// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_CHANNEL_I2C_DW_APB_STATIC_H_
#define CMD_CHANNEL_I2C_DW_APB_STATIC_H_

#include "cmd_channel_i2c_dw_apb.h"
#include "cmd_interface/cmd_channel_static.h"


/* Static initializer API for derived types. */

/**
 * Initialize a static instance of a base I2C DW APB command channel context.  This is not a top
 * level API and must only be called by an implementation API.  This does not initialize the state
 * context which must be initialized by a top level API that calls
 * cmd_channel_i2c_dw_apb_init_state.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the command channel.
 * @param i2c_ptr The underlying I2C HW for the instance.
 * @param recv_func The command channel packet receive handler.
 * @param send_func The command channel packet send handler.
 * @param chan_id The command channel ID for the instance.
 * @param timeout_ms The timeout in milliseconds for packet transmission.
 */
#define cmd_channel_i2c_dw_apb_static_init(state_ptr, i2c_ptr, recv_func, send_func, chan_id, \
	timeout_val_ms) { \
		.base = cmd_channel_static_init (&(state_ptr)->base, recv_func, send_func, chan_id), \
		.i2c_hw = i2c_ptr, \
		.timeout_ms = timeout_val_ms, \
	}


#endif	/* CMD_CHANNEL_I2C_DW_APB_STATIC_H_ */
