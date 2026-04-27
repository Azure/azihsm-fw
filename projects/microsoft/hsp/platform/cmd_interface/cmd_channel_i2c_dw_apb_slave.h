// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_CHANNEL_I2C_DW_APB_SLAVE_H_
#define CMD_CHANNEL_I2C_DW_APB_SLAVE_H_

#include "cmd_channel_i2c_dw_apb.h"
#include "drivers/i2c_dw_apb_slave.h"


/**
 * Variable context for DW APB I2C slave command channel.
 */
struct cmd_channel_i2c_dw_apb_slave_state {
	struct cmd_channel_i2c_dw_apb_state base;				/**< Base state instance. */
	struct cmd_channel_i2c_dw_apb_slave_rx_state rx_state;	/**< Context for receiving packets in SLAVE mode. */
	const struct cmd_packet *volatile tx_pkt_pending;		/**< Packet pending for transmission. */
	const struct cmd_packet *tx_pkt_active;					/**< Current packet being transmitted. */
};

/**
 * Driver instance for the DW APB I2C slave command channel.
 */
struct cmd_channel_i2c_dw_apb_slave {
	struct cmd_channel_i2c_dw_apb base;				/**< Base command channel instance. */
	struct i2c_dw_apb_slave_handler i2c_handler;	/**< I2C handler implementation. */
};


int cmd_channel_i2c_dw_apb_slave_init (struct cmd_channel_i2c_dw_apb_slave *i2c,
	struct cmd_channel_i2c_dw_apb_slave_state *state, const struct i2c_dw_apb_slave *i2c_hw,
	int channel_id, uint32_t timeout_val_ms, struct cmd_packet *rx_buffers, size_t rx_buf_count);
int cmd_channel_i2c_dw_apb_slave_init_state (const struct cmd_channel_i2c_dw_apb_slave *i2c,
	struct cmd_packet *rx_buffers, size_t rx_buf_count);
void cmd_channel_i2c_dw_apb_slave_release (const struct cmd_channel_i2c_dw_apb_slave *i2c);


#endif	/* CMD_CHANNEL_I2C_DW_APB_SLAVE_H_ */
