// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "cmd_channel_i2c_dw_apb_slave_static.h"
#include "task_priority.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "freertos/freertos_utils.h"
#include "trap/hsp_trap.h"


/* Utilities */

/**
 * Gets the I2C slave command channel instance from the I2C handler.
 *
 * @return The associated command channel instance.
 */
static const struct cmd_channel_i2c_dw_apb_slave* cmd_channel_i2c_dw_apb_slave_from_i2c_handler (
	const struct i2c_dw_apb_handler *handler)
{
	return TO_DERIVED_TYPE (handler, const struct cmd_channel_i2c_dw_apb_slave, i2c_handler);
}

/**
 * Completes the currently pending Tx packet.
 *
 * @param i2c_chan The I2C slave command channel instance.
 * @param result The result of the transmission.
 *
 * @return NULL if no packet was queued or listener abandoned the transmission, else a valid packet.
 */
static const struct cmd_packet* cmd_channel_i2c_dw_apb_slave_get_tx_completion_packet (
	const struct cmd_channel_i2c_dw_apb_slave *i2c_chan, int result)
{
	struct cmd_channel_i2c_dw_apb_slave_state *state =
		(struct cmd_channel_i2c_dw_apb_slave_state*) i2c_chan->base.base.state;
	const struct cmd_packet *tx_packet = state->tx_pkt_active;

	if (tx_packet == NULL) {
		// No packet being transmitted
		return NULL;
	}

	state->tx_pkt_active = NULL;

	if (tx_packet != state->tx_pkt_pending) {
		// Original sender abandoned the send or TX abort occurred
		return NULL;
	}

	state->base.txn_result = result;
	state->tx_pkt_pending = NULL;

	return tx_packet;
}

/**
 * Completes a currently pending Tx packet and notifies the listener.
 *
 * @param i2c_chan The I2C slave command channel instance.
 * @param result The result of the transmission.
 */
static void cmd_channel_i2c_dw_apb_slave_tx_finished (
	const struct cmd_channel_i2c_dw_apb_slave *i2c_chan, int result)
{
	struct cmd_channel_i2c_dw_apb_slave_state *state =
		(struct cmd_channel_i2c_dw_apb_slave_state*) i2c_chan->base.base.state;
	const struct cmd_packet *tx_packet;

	tx_packet = cmd_channel_i2c_dw_apb_slave_get_tx_completion_packet (i2c_chan, result);
	if (tx_packet != NULL) {
		// Notify the sender
		cmd_channel_i2c_dw_apb_signal_txn_complete (&state->base);
	}
}

/**
 * Initializes the slave command channel variable state context that hasn't been initialized by the
 * base initializers.
 *
 * @param i2c The I2C slave command channel instance.
 * @param rx_buffers An array of cmd_packet buffers used for receiving packets.
 * @param rx_buf_count The number of buffers in rx_buffers.
 *
 * @return 0 if success, else an error code.
 */
static int cmd_channel_i2c_dw_apb_slave_init_state_local (
	const struct cmd_channel_i2c_dw_apb_slave *channel, struct cmd_packet *rx_buffers,
	size_t rx_buf_count)
{
	struct cmd_channel_i2c_dw_apb_slave_state *state =
		(struct cmd_channel_i2c_dw_apb_slave_state*) channel->base.base.state;
	int status;

	status = cmd_channel_i2c_dw_apb_slave_rx_init_state (&state->rx_state, rx_buffers,
		rx_buf_count);
	if (status != 0) {
		cmd_channel_i2c_dw_apb_release (&channel->base);
	}

	return status;
}

/* Interface Implementation */

void cmd_channel_i2c_dw_apb_slave_on_shutdown (const struct i2c_dw_apb_handler *handler)
{
	const struct cmd_channel_i2c_dw_apb_slave *i2c_chan =
		cmd_channel_i2c_dw_apb_slave_from_i2c_handler (handler);
	struct cmd_channel_i2c_dw_apb_slave_state *state =
		(struct cmd_channel_i2c_dw_apb_slave_state*) i2c_chan->base.base.state;
	const struct cmd_packet *tx_packet;

	cmd_channel_i2c_dw_apb_slave_rx_on_shutdown (&state->rx_state);

	tx_packet = cmd_channel_i2c_dw_apb_slave_get_tx_completion_packet (i2c_chan,
		CMD_CHANNEL_TX_FAILED);
	if (tx_packet != NULL) {
		cmd_channel_i2c_dw_apb_tx_on_shutdown (&state->base);
	}
}

size_t cmd_channel_i2c_dw_apb_slave_on_rx_pending (const struct i2c_dw_apb_handler *handler,
	uint8_t **data)
{
	const struct cmd_channel_i2c_dw_apb_slave *i2c_chan =
		cmd_channel_i2c_dw_apb_slave_from_i2c_handler (handler);
	struct cmd_channel_i2c_dw_apb_slave_state *state =
		(struct cmd_channel_i2c_dw_apb_slave_state*) i2c_chan->base.base.state;

	return cmd_channel_i2c_dw_apb_slave_rx_on_rx_pending (&i2c_chan->base, &state->rx_state, data);
}

void cmd_channel_i2c_dw_apb_slave_on_rx_discard (const struct i2c_dw_apb_handler *handler)
{
	const struct cmd_channel_i2c_dw_apb_slave *i2c_chan =
		cmd_channel_i2c_dw_apb_slave_from_i2c_handler (handler);
	struct cmd_channel_i2c_dw_apb_slave_state *state =
		(struct cmd_channel_i2c_dw_apb_slave_state*) i2c_chan->base.base.state;

	cmd_channel_i2c_dw_apb_slave_rx_on_discard (&state->rx_state);
}

void cmd_channel_i2c_dw_apb_slave_on_rx_data (const struct i2c_dw_apb_handler *handler,
	size_t count)
{
	const struct cmd_channel_i2c_dw_apb_slave *i2c_chan =
		cmd_channel_i2c_dw_apb_slave_from_i2c_handler (handler);
	struct cmd_channel_i2c_dw_apb_slave_state *state =
		(struct cmd_channel_i2c_dw_apb_slave_state*) i2c_chan->base.base.state;

	cmd_channel_i2c_dw_apb_slave_rx_on_data (&state->rx_state, count);
}

void cmd_channel_i2c_dw_apb_slave_on_rx_complete (const struct i2c_dw_apb_handler *handler)
{
	const struct cmd_channel_i2c_dw_apb_slave *i2c_chan =
		cmd_channel_i2c_dw_apb_slave_from_i2c_handler (handler);
	struct cmd_channel_i2c_dw_apb_slave_state *state =
		(struct cmd_channel_i2c_dw_apb_slave_state*) i2c_chan->base.base.state;

	cmd_channel_i2c_dw_apb_slave_rx_complete (&i2c_chan->base, &state->rx_state);
}

size_t cmd_channel_i2c_dw_apb_slave_on_read_request (const struct i2c_dw_apb_handler *handler,
	const uint8_t **data)
{
	const struct cmd_channel_i2c_dw_apb_slave *i2c_chan =
		cmd_channel_i2c_dw_apb_slave_from_i2c_handler (handler);
	struct cmd_channel_i2c_dw_apb_slave_state *state =
		(struct cmd_channel_i2c_dw_apb_slave_state*) i2c_chan->base.base.state;
	const struct cmd_packet *packet;

	packet = state->tx_pkt_pending;
	if (packet == NULL) {
		/* No packet is ready. HW stalls the bus while waiting for data to transmit. Wait for a
		 * sender to enable READ_REQ when a packet is queued up. */
		return 0;
	}

	state->tx_pkt_active = packet;
	*data = packet->data;

	return packet->pkt_size;
}

void cmd_channel_i2c_dw_apb_slave_on_tx_abort (const struct i2c_dw_apb_handler *handler)
{
	const struct cmd_channel_i2c_dw_apb_slave *i2c_chan =
		cmd_channel_i2c_dw_apb_slave_from_i2c_handler (handler);

	cmd_channel_i2c_dw_apb_slave_tx_finished (i2c_chan, CMD_CHANNEL_TX_FAILED);
}

void cmd_channel_i2c_dw_apb_slave_on_tx_depleted (const struct i2c_dw_apb_handler *handler)
{
	const struct cmd_channel_i2c_dw_apb_slave *i2c_chan =
		cmd_channel_i2c_dw_apb_slave_from_i2c_handler (handler);
	const struct i2c_dw_apb_slave *i2c_hw =
		(const struct i2c_dw_apb_slave*) i2c_chan->base.i2c_hw;

	// Only support transmission of a single packet.  Any additional data will be padded.
	i2c_dw_apb_slave_pad_tx_with_default_byte (i2c_hw);
}

void cmd_channel_i2c_dw_apb_slave_on_tx_complete (const struct i2c_dw_apb_handler *handler)
{
	const struct cmd_channel_i2c_dw_apb_slave *i2c_chan =
		cmd_channel_i2c_dw_apb_slave_from_i2c_handler (handler);

	cmd_channel_i2c_dw_apb_slave_tx_finished (i2c_chan, 0);
}

int cmd_channel_i2c_dw_apb_slave_receive_packet (const struct cmd_channel *channel,
	struct cmd_packet *packet, int timeout_ms)
{
	struct cmd_channel_i2c_dw_apb_slave_state *state;
	int status;

	if (channel == NULL) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	state = (struct cmd_channel_i2c_dw_apb_slave_state*) channel->state;

	status = cmd_channel_i2c_dw_apb_slave_rx_receive_packet (&state->rx_state, packet, timeout_ms);

	return status;
}

int cmd_channel_i2c_dw_apb_slave_send_packet (const struct cmd_channel *channel,
	const struct cmd_packet *packet)
{
	const struct cmd_channel_i2c_dw_apb_slave *i2c_chan =
		(const struct cmd_channel_i2c_dw_apb_slave*) channel;
	struct cmd_channel_i2c_dw_apb_slave_state *state;
	const struct i2c_dw_apb_slave *i2c_hw;
	const struct cmd_packet *pending;
	int status;
	int result;
	uintptr_t mstatus = 0;

	status = cmd_channel_validate_packet_for_send (packet);
	if (status != 0) {
		return status;
	}

	if (channel == NULL) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	state = (struct cmd_channel_i2c_dw_apb_slave_state*) channel->state;
	i2c_hw = (const struct i2c_dw_apb_slave*) i2c_chan->base.i2c_hw;

	status = i2c_dw_apb_lock_driver (&i2c_hw->i2c_base);
	if (status != 0) {
		return status;
	}

	// tx_pkt_pending should be NULL, otherwise in a bad state.
	result = CMD_CHANNEL_TX_FAILED;
	if ((state->tx_pkt_pending == NULL) && i2c_dw_apb_is_hw_enabled (&i2c_hw->i2c_base)) {
		// Disable interrupts and update state
		result = i2c_dw_apb_slave_begin_read_request_update (i2c_hw, &mstatus);
		if (result == 0) {
			state->base.txn_result = CMD_CHANNEL_TX_TIMEOUT;
			state->tx_pkt_pending = packet;
			hsp_trap_mstatus_write (mstatus);

			do {
				// Wait for the pending packet to be cleared or a timeout
				result = cmd_channel_i2c_dw_apb_wait_txn_complete (&state->base,
					CMD_CHANNEL_I2C_DW_APB_TX_TIMEOUT_MS);
				pending = state->tx_pkt_pending;
			} while ((pending != NULL) && (result == 0));

			/* The pending reference gets cleared when a transaction completes the packet. If it
			 * doesn't get cleared, it means the semaphore wait timed out. This will happen when
			 * the bus is extremely busy and repeated transmission attempts keep losing
			 * arbitration, the receiver isn't consuming the data in a timely manner, or the I2C
			 * driver is in a bugged state that isn't transmitting the data. */
			if (pending != NULL) {
				result = i2c_dw_apb_slave_begin_read_request_update (i2c_hw, &mstatus);

				if (result == 0) {
					// Check if an ISR fired and completed the request before interrupts were disabled
					pending = state->tx_pkt_pending;

					if (pending != NULL) {
						result = CMD_CHANNEL_TX_TIMEOUT;
						state->tx_pkt_pending = NULL;

						// Clears any remaining unsent data and pads the request out with default_byte
						i2c_dw_apb_slave_pad_tx_with_default_byte (i2c_hw);
					}

					hsp_trap_mstatus_write (mstatus);
				}
			}

			if (pending == NULL) {
				result = state->base.txn_result;
			}
		}
	}

	status = i2c_dw_apb_unlock_driver (&i2c_hw->i2c_base);
	if (result == 0) {
		result = status;
	}

	return result;
}

/* Public API */

/**
 * Initializes the slave DW APB I2C command channel driver and state context.
 *
 * @param channel The I2C slave command channel instance.
 * @param state The variable context for the driver.
 * @param i2c_hw The underlying I2C hardware for communication.
 * @param timeout_val_ms Timeout in milliseconds for packets.
 * @param channel_id Command channel ID assigned to this instance.
 * @param rx_buffers An array of cmd_packet buffers used for receiving packets.
 * @param rx_buf_count The number of buffers to allocate for the RX queue.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_slave_init (struct cmd_channel_i2c_dw_apb_slave *channel,
	struct cmd_channel_i2c_dw_apb_slave_state *state, const struct i2c_dw_apb_slave *i2c_hw,
	int channel_id, uint32_t timeout_val_ms, struct cmd_packet *rx_buffers, size_t rx_buf_count)
{
	int status;

	if ((channel == NULL) || (state == NULL) || (i2c_hw == NULL)) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	memset (channel, 0, sizeof (*channel));
	memset (state, 0, sizeof (*state));

	channel->i2c_handler.rx_handler.handler_base.on_shutdown =
		cmd_channel_i2c_dw_apb_slave_on_shutdown;
	channel->i2c_handler.rx_handler.handler_base.on_rx_discard =
		cmd_channel_i2c_dw_apb_slave_on_rx_discard;
	channel->i2c_handler.rx_handler.handler_base.on_rx_complete =
		cmd_channel_i2c_dw_apb_slave_on_rx_complete;
	channel->i2c_handler.rx_handler.handler_base.on_tx_abort =
		cmd_channel_i2c_dw_apb_slave_on_tx_abort;
	channel->i2c_handler.rx_handler.handler_base.on_tx_complete =
		cmd_channel_i2c_dw_apb_slave_on_tx_complete;
	channel->i2c_handler.rx_handler.on_rx_pending = cmd_channel_i2c_dw_apb_slave_on_rx_pending;
	channel->i2c_handler.rx_handler.on_rx_data = cmd_channel_i2c_dw_apb_slave_on_rx_data;
	channel->i2c_handler.on_read_request = cmd_channel_i2c_dw_apb_slave_on_read_request;
	channel->i2c_handler.on_tx_depleted = cmd_channel_i2c_dw_apb_slave_on_tx_depleted;

	channel->base.base.receive_packet = cmd_channel_i2c_dw_apb_slave_receive_packet;
	channel->base.base.send_packet = cmd_channel_i2c_dw_apb_slave_send_packet;

	status = cmd_channel_i2c_dw_apb_init (&channel->base, &state->base,	&i2c_hw->i2c_base,
		channel_id, timeout_val_ms);
	if (status != 0) {
		return status;
	}

	return cmd_channel_i2c_dw_apb_slave_init_state_local (channel, rx_buffers, rx_buf_count);
}

/**
 * Initializes the slave DW APB I2C command channel variable state context.
 *
 * @param i2c The I2C slave command channel instance.
 * @param rx_buffers An array of cmd_packet buffers used for receiving packets.
 * @param rx_buf_count The number of buffers in rx_buffers.
 *
 * @return 0 if success, else an error code.
 */
int cmd_channel_i2c_dw_apb_slave_init_state (const struct cmd_channel_i2c_dw_apb_slave *channel,
	struct cmd_packet *rx_buffers, size_t rx_buf_count)
{
	struct cmd_channel_i2c_dw_apb_slave_state *state;
	int status;

	if (channel == NULL) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	state = (struct cmd_channel_i2c_dw_apb_slave_state*) channel->base.base.state;
	if (state == NULL) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	memset (state, 0, sizeof (*state));

	status = cmd_channel_i2c_dw_apb_init_state (&channel->base);
	if (status != 0) {
		return status;
	}

	return cmd_channel_i2c_dw_apb_slave_init_state_local (channel, rx_buffers, rx_buf_count);
}

/**
 * Releases the resources held by the base command channel instance.
 *
 * @param channel The I2C slave command channel instance.
 */
void cmd_channel_i2c_dw_apb_slave_release (const struct cmd_channel_i2c_dw_apb_slave *channel)
{
	struct cmd_channel_i2c_dw_apb_slave_state *state;

	if (channel == NULL) {
		return;
	}

	state = (struct cmd_channel_i2c_dw_apb_slave_state*) channel->base.base.state;
	if (state != NULL) {
		cmd_channel_i2c_dw_apb_slave_rx_release (&state->rx_state);
	}

	cmd_channel_i2c_dw_apb_release (&channel->base);
}
