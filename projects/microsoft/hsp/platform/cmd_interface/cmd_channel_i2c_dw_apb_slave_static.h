// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_CHANNEL_I2C_DW_APB_SLAVE_STATIC_H_
#define CMD_CHANNEL_I2C_DW_APB_SLAVE_STATIC_H_

#include "cmd_channel_i2c_dw_apb_slave.h"
#include "cmd_channel_i2c_dw_apb_static.h"
#include "drivers/i2c_dw_apb_slave_handler_static.h"


/* Internal functions declared to allow for static initialization. */

void cmd_channel_i2c_dw_apb_slave_on_shutdown (const struct i2c_dw_apb_handler *handler);
size_t cmd_channel_i2c_dw_apb_slave_on_rx_pending (const struct i2c_dw_apb_handler *handler,
	uint8_t **data);
void cmd_channel_i2c_dw_apb_slave_on_rx_data (const struct i2c_dw_apb_handler *handler,
	size_t count);
void cmd_channel_i2c_dw_apb_slave_on_rx_discard (const struct i2c_dw_apb_handler *handler);
void cmd_channel_i2c_dw_apb_slave_on_rx_complete (const struct i2c_dw_apb_handler *handler);
size_t cmd_channel_i2c_dw_apb_slave_on_read_request (const struct i2c_dw_apb_handler *handler,
	const uint8_t **data);
void cmd_channel_i2c_dw_apb_slave_on_tx_abort (const struct i2c_dw_apb_handler *handler);
void cmd_channel_i2c_dw_apb_slave_on_tx_depleted (const struct i2c_dw_apb_handler *handler);
void cmd_channel_i2c_dw_apb_slave_on_tx_complete (const struct i2c_dw_apb_handler *handler);

int cmd_channel_i2c_dw_apb_slave_receive_packet (const struct cmd_channel *channel,
	struct cmd_packet *packet, int timeout_ms);
int cmd_channel_i2c_dw_apb_slave_send_packet (const struct cmd_channel *channel,
	const struct cmd_packet *packet);


/* Static initializer API. */

/**
 * Initialize a static instance of the I2C DW APB slave command channel.  This does not initialize
 * the channel state which needs to be initialized separately with
 * cmd_channel_i2c_dw_apb_slave_init_state.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the command channel.
 * @param i2c_ptr The underlying I2C HW for the instance.
 * @param chan_id The command channel ID for the instance.
 * @param timeout_ms The timeout in milliseconds for packet transmission.
 */
#define cmd_channel_i2c_dw_apb_slave_static_init(state_ptr, i2c_ptr, chan_id, timeout_val_ms) { \
		.base = cmd_channel_i2c_dw_apb_static_init (&(state_ptr)->base, &(i2c_ptr)->i2c_base, \
			cmd_channel_i2c_dw_apb_slave_receive_packet, \
			cmd_channel_i2c_dw_apb_slave_send_packet, chan_id, timeout_val_ms), \
		.i2c_handler = i2c_dw_apb_slave_handler_static_init ( \
			cmd_channel_i2c_dw_apb_slave_on_shutdown, cmd_channel_i2c_dw_apb_slave_on_rx_pending, \
			cmd_channel_i2c_dw_apb_slave_on_rx_data, cmd_channel_i2c_dw_apb_slave_on_rx_discard, \
			cmd_channel_i2c_dw_apb_slave_on_rx_complete, \
			cmd_channel_i2c_dw_apb_slave_on_read_request, \
			cmd_channel_i2c_dw_apb_slave_on_tx_abort, \
			cmd_channel_i2c_dw_apb_slave_on_tx_depleted, \
			cmd_channel_i2c_dw_apb_slave_on_tx_complete), \
	}


#endif	/* CMD_CHANNEL_I2C_DW_APB_SLAVE_STATIC_H_ */
