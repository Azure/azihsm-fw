// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_CHANNEL_I2C_DW_APB_H_
#define CMD_CHANNEL_I2C_DW_APB_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "platform_api.h"
#include "platform_config.h"
#include "queue.h"
#include "cmd_interface/cmd_channel.h"
#include "drivers/i2c_dw_apb_slave.h"


/**
 * Timeout for packet transmission.
 */
#define CMD_CHANNEL_I2C_DW_APB_TX_TIMEOUT_MS		5000

/**
 * Base variable context for a command channel I2C driver.
 */
struct cmd_channel_i2c_dw_apb_state {
	struct cmd_channel_state base;			/**< Variable state context for the channel. */
	platform_semaphore txn_sig_complete;	/**< Signal to notify a listener that a transaction is complete. */
	volatile int txn_result;				/**< The result of a transaction. */
};

/**
 * Variable context used for receiving packets in SLAVE mode.
 *
 * This has a dependency on FreeRTOS queue objects.
 */
struct cmd_channel_i2c_dw_apb_slave_rx_state {
	struct cmd_packet *pkt_current;	/**< Current buffer being used for receiving. */
	QueueHandle_t queue_available;	/**< Queue for available buffers to receive into. */
	QueueHandle_t queue_ready;		/**< Queue for buffers that have valid data. */
};

/**
 * Base driver instance for the a DW APB I2C command channel.
 */
struct cmd_channel_i2c_dw_apb {
	struct cmd_channel base;			/**< Command channel interface. */
	const struct i2c_dw_apb *i2c_hw;	/**< Underlying I2C hardware. */
	uint32_t timeout_ms;				/**< Timeout in milliseconds for packets. */
};


/* Internal functions for use by derived types. */

int cmd_channel_i2c_dw_apb_init (struct cmd_channel_i2c_dw_apb *channel,
	struct cmd_channel_i2c_dw_apb_state *state, const struct i2c_dw_apb *i2c_hw, int channel_id,
	uint32_t timeout_ms);
int cmd_channel_i2c_dw_apb_init_state (const struct cmd_channel_i2c_dw_apb *channel);
void cmd_channel_i2c_dw_apb_release (const struct cmd_channel_i2c_dw_apb *channel);

int cmd_channel_i2c_dw_apb_slave_rx_init_state (
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state, struct cmd_packet *rx_buffers,
	size_t rx_buf_count);
void cmd_channel_i2c_dw_apb_slave_rx_release (
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state);

int cmd_channel_i2c_dw_apb_reset_txn_complete (struct cmd_channel_i2c_dw_apb_state *state);
int cmd_channel_i2c_dw_apb_signal_txn_complete (struct cmd_channel_i2c_dw_apb_state *state);
int cmd_channel_i2c_dw_apb_wait_txn_complete (struct cmd_channel_i2c_dw_apb_state *state,
	uint32_t timeout_ms);

int cmd_channel_i2c_dw_apb_slave_rx_on_shutdown (
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state);
size_t cmd_channel_i2c_dw_apb_slave_rx_on_rx_pending (
	const struct cmd_channel_i2c_dw_apb *i2c_chan,
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state, uint8_t **data);
int cmd_channel_i2c_dw_apb_slave_rx_on_discard (
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state);
int cmd_channel_i2c_dw_apb_slave_rx_on_data (
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state, size_t count);
int cmd_channel_i2c_dw_apb_slave_rx_complete (const struct cmd_channel_i2c_dw_apb *i2c_chan,
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state);
int cmd_channel_i2c_dw_apb_slave_rx_receive_packet (
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state, struct cmd_packet *packet,
	unsigned timeout_ms);

int cmd_channel_i2c_dw_apb_tx_on_shutdown (struct cmd_channel_i2c_dw_apb_state *state);
int cmd_channel_i2c_dw_apb_master_tx_set_result (struct cmd_channel_i2c_dw_apb_state *state,
	int result);
int cmd_channel_i2c_dw_apb_begin_master_send_packet (const struct cmd_channel_i2c_dw_apb *i2c_chan,
	platform_clock *timeout);
int cmd_channel_i2c_dw_apb_try_master_send_packet (const struct cmd_channel_i2c_dw_apb *i2c_chan,
	const platform_clock *timeout, const struct cmd_packet *packet);
int cmd_channel_i2c_dw_apb_finish_master_send_packet (
	const struct cmd_channel_i2c_dw_apb *i2c_chan);


#endif	/* CMD_CHANNEL_I2C_DW_APB_H_ */
