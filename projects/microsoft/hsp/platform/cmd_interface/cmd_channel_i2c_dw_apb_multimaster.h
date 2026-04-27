// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_CHANNEL_I2C_DW_APB_MULTIMASTER_H_
#define CMD_CHANNEL_I2C_DW_APB_MULTIMASTER_H_

#include "cmd_channel_i2c_dw_apb.h"
#include "drivers/i2c_dw_apb_multimaster.h"
#include "system/system_observer.h"


/**
 * Variable context for DW APB I2C multi-master command channel.
 */
struct cmd_channel_i2c_dw_apb_multimaster_state {
	struct cmd_channel_i2c_dw_apb_state base;				/**< Base state instance. */
	struct cmd_channel_i2c_dw_apb_slave_rx_state rx_state;	/**< Context for receiving packets in SLAVE mode. */
};

/**
 * Driver instance for the DW APB I2C multi-master command channel.
 */
struct cmd_channel_i2c_dw_apb_multimaster {
	struct cmd_channel_i2c_dw_apb base;					/**< Base command channel instance. */
	struct system_observer base_observer;				/**< Base observer for system notifications. */
	struct i2c_dw_apb_multimaster_handler i2c_handler;	/**< I2C handler implementation. */
};


int cmd_channel_i2c_dw_apb_multimaster_init (struct cmd_channel_i2c_dw_apb_multimaster *channel,
	struct cmd_channel_i2c_dw_apb_multimaster_state *state,
	const struct i2c_dw_apb_multimaster *i2c_hw, int channel_id, uint32_t timeout_val_ms,
	struct cmd_packet *rx_buffers, size_t rx_buf_count);
int cmd_channel_i2c_dw_apb_multimaster_init_state (
	const struct cmd_channel_i2c_dw_apb_multimaster *channel, struct cmd_packet *rx_buffers,
	size_t rx_buf_count);
void cmd_channel_i2c_dw_apb_multimaster_release (
	const struct cmd_channel_i2c_dw_apb_multimaster *channel);


#endif	/* CMD_CHANNEL_I2C_DW_APB_MULTIMASTER_H_ */
