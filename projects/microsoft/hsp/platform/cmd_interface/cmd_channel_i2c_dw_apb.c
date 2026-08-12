// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "cmd_channel_i2c_dw_apb_static.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "drivers/i2c_dw_apb_error.h"
#include "freertos/freertos_utils.h"


/* Utilities */

/**
 * Allocates a FreeRTOS queue for pointers
 *
 * @param queue_ptr Pointer to the queue
 * @param count Count of elements the queue can hold
 *
 * @return 0 if queue was allocated, else error code
 */
static int cmd_channel_i2c_dw_apb_alloc_ptr_queue (QueueHandle_t *queue_ptr, size_t count)
{
	QueueHandle_t queue;

	queue = freertos_utils_ptr_queue_alloc (count);
	if (queue == NULL) {
		return CMD_CHANNEL_NO_MEMORY;
	}

	*queue_ptr = queue;

	return 0;
}

/**
 * Completes any currently queued buffer for a slave Rx state context.
 *
 * @param i2c_chan The command channel instance.
 * @param rx_state The slave Rx state context.
 * @param pkt_type The cmd_packet type to tag a valid buffer with.
 */
static void cmd_channel_i2c_dw_apb_slave_finish_rx_buffer (
	const struct cmd_channel_i2c_dw_apb *i2c_chan,
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state, uint8_t pkt_type)
{
	int status;
	QueueHandle_t rx_queue;
	struct cmd_packet *packet;

	packet = rx_state->pkt_current;
	if (packet == NULL) {
		return;
	}

	rx_state->pkt_current = NULL;

	if (packet->pkt_size > 0) {
		rx_queue = rx_state->queue_ready;

		packet->state = pkt_type;
		if (i2c_chan->timeout_ms != 0) {
			status = platform_init_timeout (i2c_chan->timeout_ms, &packet->pkt_timeout);
			packet->timeout_valid = (status == 0);
		}
	}
	else {
		rx_queue = rx_state->queue_available;
	}

	freertos_utils_valid_ptr_queue_push_isr (rx_queue, packet);
}

/**
 * Initializes the I2C DW APB command channel variable state context that hasn't been initialized
 * by the base initializers.
 *
 * @param i2c The I2C command channel instance.
 *
 * @return 0 if success, else an error code.
 */
static int cmd_channel_i2c_dw_apb_init_state_local (const struct cmd_channel_i2c_dw_apb *channel)
{
	struct cmd_channel_i2c_dw_apb_state *state =
		(struct cmd_channel_i2c_dw_apb_state*) channel->base.state;
	int status;

	state->txn_result = 0;

	status = platform_semaphore_init (&state->txn_sig_complete);
	if (status != 0) {
		cmd_channel_release (&channel->base);
	}

	return status;
}

/* Mode Implementation API */

/**
 * Initializes a base I2C DW APB command channel instance at runtime.  This does not initialize
 * the state context which must be initialized by a top level API call.
 *
 * @param channel The base command channel instance.
 * @param state The base command channel variable context.
 * @param i2c_hw The underlying I2C HW driver for the channel.
 * @param channel_id Channel ID for the command channel instance.
 * @param timeout_ms Packet timeout in milliseconds.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_init (struct cmd_channel_i2c_dw_apb *channel,
	struct cmd_channel_i2c_dw_apb_state *state, const struct i2c_dw_apb *i2c_hw, int channel_id,
	uint32_t timeout_ms)
{
	int status;

	if ((channel == NULL) || (state == NULL) || (i2c_hw == NULL)) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	channel->i2c_hw = i2c_hw;
	channel->timeout_ms = timeout_ms;

	status = cmd_channel_init (&channel->base, &state->base, channel_id);
	if (status != 0) {
		return status;
	}

	return cmd_channel_i2c_dw_apb_init_state_local (channel);
}

/**
 * Initialize only the variable state of the base I2C DW APB command channel instance.
 *
 * @param channel The base command channel instance.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_init_state (const struct cmd_channel_i2c_dw_apb *channel)
{
	int status;

	if (channel == NULL) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	status = cmd_channel_init_state (&channel->base);
	if (status != 0) {
		return status;
	}

	return cmd_channel_i2c_dw_apb_init_state_local (channel);
}

/**
 * Releases resources held by the base I2C DW APB and base command channel instance.
 *
 * @param channel The base I2C DW APB command channel.
 */
void cmd_channel_i2c_dw_apb_release (const struct cmd_channel_i2c_dw_apb *channel)
{
	struct cmd_channel_i2c_dw_apb_state *state;

	if (channel == NULL) {
		return;
	}

	state = (struct cmd_channel_i2c_dw_apb_state*) channel->base.state;

	cmd_channel_release (&channel->base);
	platform_semaphore_free (&state->txn_sig_complete);
}

/**
 * Initializes a slave Rx state context.
 *
 * @param rx_state The slave Rx state context.
 * @param rx_buffers An array of packet buffers to use for receiving data.
 * @param rx_buf_count The count of rx_buffers.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_slave_rx_init_state (
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state, struct cmd_packet *rx_buffers,
	size_t rx_buf_count)
{
	int status;
	size_t i;

	if ((rx_state == NULL) || (rx_buffers == NULL) || (rx_buf_count == 0)) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	status = cmd_channel_i2c_dw_apb_alloc_ptr_queue (&rx_state->queue_available, rx_buf_count);
	if (status != 0) {
		return status;
	}

	status = cmd_channel_i2c_dw_apb_alloc_ptr_queue (&rx_state->queue_ready, rx_buf_count);
	if (status != 0) {
		goto free_queue_available;
	}

	for (i = 0; i < rx_buf_count; ++i) {
		freertos_utils_valid_ptr_queue_push (rx_state->queue_available, rx_buffers + i, -1);
	}

	rx_state->pkt_current = NULL;

	return 0;

free_queue_available:
	vQueueDelete (rx_state->queue_available);

	return status;
}

/**
 * Releases resources held by the slave Rx state context.
 *
 * @param rx_state The slave Rx state context.
 */
void cmd_channel_i2c_dw_apb_slave_rx_release (
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state)
{
	if (rx_state == NULL) {
		return;
	}

	vQueueDelete (rx_state->queue_ready);
	vQueueDelete (rx_state->queue_available);
}

/**
 * Resets the signal used to notify that a listener that a transaction is complete.
 *
 * @param state The command channel state.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_reset_txn_complete (struct cmd_channel_i2c_dw_apb_state *state)
{
	return platform_semaphore_reset (&state->txn_sig_complete);
}

/**
 * Notifies a listener that a transaction is complete.
 *
 * This should only be called within an ISR context.
 *
 * @param state The command channel state.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_signal_txn_complete (struct cmd_channel_i2c_dw_apb_state *state)
{
	return platform_semaphore_post_from_isr (&state->txn_sig_complete);
}

/**
 * Waits for a transaction to be complete.
 *
 * @param state The command channel state.
 * @param timeout_ms The timeout in milliseconds to wait for a signal.
 *
 * @return 0 if successful, 1 if timedout, else an error code.
 */
int cmd_channel_i2c_dw_apb_wait_txn_complete (struct cmd_channel_i2c_dw_apb_state *state,
	uint32_t timeout_ms)
{
	return platform_semaphore_wait (&state->txn_sig_complete, timeout_ms);
}

/**
 * Initializes the slave Rx state when an I2C shutdown occurs.
 *
 * @param rx_state The slave Rx state context.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_slave_rx_on_shutdown (
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state)
{
	struct cmd_packet *packet;

	if (rx_state == NULL) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	packet = rx_state->pkt_current;
	if (packet != NULL) {
		rx_state->pkt_current = NULL;
		freertos_utils_valid_ptr_queue_push (rx_state->queue_available, packet, -1);
	}

	return 0;
}

/**
 * Queues a packet to begin receiving data into while in I2C SLAVE mode.
 *
 * @param i2c_chan The command channel instance.
 * @param rx_state The slave Rx state context.
 * @param data A pointer to return the packet buffer.
 *
 * @return The size of the returned buffer.
 */
size_t cmd_channel_i2c_dw_apb_slave_rx_on_rx_pending (const struct cmd_channel_i2c_dw_apb *i2c_chan,
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state, uint8_t **data)
{
	if ((i2c_chan == NULL) || (rx_state == NULL) || (data == NULL)) {
		return 0;
	}

	/* All head data bigger than a packet is marked overflow.  First on_rx_pending call will result
	 * in a NOOP because no packet has been queued to receive into.  But further calls will mark
	 * previous buffers as OVERFLOW.  The final on_rx_pending buffer will be marked VALID when
	 * on_rx_complete is called. */
	cmd_channel_i2c_dw_apb_slave_finish_rx_buffer (i2c_chan, rx_state, CMD_OVERFLOW_PACKET);

	rx_state->pkt_current =
		(struct cmd_packet*) freertos_utils_valid_ptr_queue_pop_isr (rx_state->queue_available);
	if (rx_state->pkt_current == NULL) {
		return 0;
	}

	rx_state->pkt_current->pkt_size = 0;
	rx_state->pkt_current->dest_addr = i2c_dw_apb_get_slave_address (i2c_chan->i2c_hw);
	rx_state->pkt_current->timeout_valid = false;

	*data = rx_state->pkt_current->data;

	return sizeof (rx_state->pkt_current->data);
}

/**
 * Discards any buffer currently being used to receive data into.
 *
 * This should only be called in an ISR context.
 *
 * @param rx_state The slave Rx state context.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_slave_rx_on_discard (
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state)
{
	struct cmd_packet *packet;

	if (rx_state == NULL) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	packet = rx_state->pkt_current;
	if (packet != NULL) {
		rx_state->pkt_current = NULL;
		freertos_utils_valid_ptr_queue_push_isr (rx_state->queue_available, packet);
	}

	return 0;
}

/**
 * Saves the length of data written into the current Rx buffer.
 *
 * @param rx_state The slave Rx state context.
 * @param count The count of bytes written to the packet buffer.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_slave_rx_on_data (
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state, size_t count)
{
	if (rx_state == NULL) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	if ((rx_state->pkt_current == NULL) || (count > sizeof (rx_state->pkt_current->data))) {
		return CMD_CHANNEL_RX_FAILED;
	}

	rx_state->pkt_current->pkt_size = count;

	return 0;
}

/**
 * Completes the current Rx buffer and marks it as valid.
 *
 * @param i2c_chan The command channel instance.
 * @param rx_state The slave Rx state context.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_slave_rx_complete (const struct cmd_channel_i2c_dw_apb *i2c_chan,
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state)
{
	if ((i2c_chan == NULL) || (rx_state == NULL)) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	cmd_channel_i2c_dw_apb_slave_finish_rx_buffer (i2c_chan, rx_state, CMD_VALID_PACKET);

	return 0;
}

/**
 * Attempts to consume a packet from the received packet queue.
 *
 * @param rx_state The slave Rx state context.
 * @param packet The packet to move the received data into.
 * @param timeout_ms The timeout in milliseconds to wait for a packet.
 *
 * @param 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_slave_rx_receive_packet (
	struct cmd_channel_i2c_dw_apb_slave_rx_state *rx_state, struct cmd_packet *packet,
	unsigned timeout_ms)
{
	struct cmd_packet *rx_packet;

	if ((rx_state == NULL) || (packet == NULL)) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	rx_packet = freertos_utils_valid_ptr_queue_pop (rx_state->queue_ready, timeout_ms);
	if (rx_packet == NULL) {
		return CMD_CHANNEL_RX_TIMEOUT;
	}

	memcpy (packet, rx_packet, sizeof (*packet));

	freertos_utils_valid_ptr_queue_push (rx_state->queue_available, rx_packet, -1);

	return 0;
}

/**
 * Notifies a listener that a shutdown has occurred during transmission.
 *
 * It is up to the top level driver to indicate to the listener of a shutdown/failure.
 *
 * @param state The command channel state context.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_tx_on_shutdown (struct cmd_channel_i2c_dw_apb_state *state)
{
	if (state == NULL) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	return platform_semaphore_post (&state->txn_sig_complete);
}

/**
 * Sets the result of a MASTER mode transmission.
 *
 * @param state The command channel state context.
 * @param result The result of the transaction.
 *
 * @return 0 if the transaction was completed, 1 if the transaction was already handled, else an
 * error code.
 */
int cmd_channel_i2c_dw_apb_master_tx_set_result (struct cmd_channel_i2c_dw_apb_state *state,
	int result)
{
	int tx_result;

	if (state == NULL) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	// State is initialized to TIMEOUT and remains until an ABORT or STOP occurs.
	tx_result = state->txn_result;
	if (tx_result == CMD_CHANNEL_TX_TIMEOUT) {
		state->txn_result = result;

		return 0;
	}

	if (tx_result == -1) {
		// This is during the shutdown during begin_master_transmit.
		state->txn_result = CMD_CHANNEL_TX_TIMEOUT;
	}

	return 1;
}

/**
 * Acquires the I2C driver lock and initializes the context for a MASTER mode transmission.
 *
 * If an error occurs, the lock is released.  Otherwise,
 * cmd_channel_i2c_dw_apb_finish_master_send_packet must be called to release the lock.
 *
 * @param i2c_chan The command channel instance.
 * @param timeout A pointer to initialize a timeout context for data transmission.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_begin_master_send_packet (const struct cmd_channel_i2c_dw_apb *i2c_chan,
	platform_clock *timeout)
{
	const struct i2c_dw_apb *i2c_hw;
	int status;
	int result;

	if ((i2c_chan == NULL) || (timeout == NULL)) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	i2c_hw = i2c_chan->i2c_hw;

	status = i2c_dw_apb_lock_driver (i2c_hw);
	if (status != 0) {
		return status;
	}

	result = platform_init_timeout (CMD_CHANNEL_I2C_DW_APB_TX_TIMEOUT_MS, timeout);
	if (result != 0) {
		i2c_dw_apb_unlock_driver (i2c_hw);
	}

	return result;
}

/**
 * Disables the I2C HW block, initializes the context for MASTER mode packet transmission, and then
 * enables the HW to transmit data.
 *
 * If an error occurs, the state of the HW is undefined.  It is up to the top level to reconfigure
 * the I2C HW.
 *
 * @param i2c_chan The command channel instance.
 * @param timeout The transaction timeout state.
 * @param packet The packet to transmit.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_try_master_send_packet (const struct cmd_channel_i2c_dw_apb *i2c_chan,
	const platform_clock *timeout, const struct cmd_packet *packet)
{
	struct cmd_channel_i2c_dw_apb_state *state;
	const struct i2c_dw_apb *i2c_hw;
	int status;
	int result;
	uint32_t timeout_remain;

	if ((i2c_chan == NULL) || (timeout == NULL)) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	status = cmd_channel_validate_packet_for_send (packet);
	if (status != 0) {
		return status;
	}

	state = (struct cmd_channel_i2c_dw_apb_state*) i2c_chan->base.state;
	i2c_hw = i2c_chan->i2c_hw;

	state->txn_result = -1;
	cmd_channel_i2c_dw_apb_reset_txn_complete (state);

	status = i2c_dw_apb_begin_master_transmit (i2c_hw, packet->dest_addr, packet->data,
		packet->pkt_size);
	if (status != 0) {
		return (status == I2C_DW_APB_BUS_BUSY) ? CMD_CHANNEL_TX_BUS_BUSY : status;
	}

	result = platform_get_timeout_remaining (timeout, &timeout_remain);
	if (result == 0) {
		result = cmd_channel_i2c_dw_apb_wait_txn_complete (state, timeout_remain);

		if (result == 0) {
			// Wait for HW to become inactive
			while (i2c_dw_apb_is_active (i2c_hw)) {
				result = platform_has_timeout_expired (timeout);

				if (result != 0) {
					result = CMD_CHANNEL_TX_TIMEOUT;
					break;
				}
			}
		}
		else {
			result = CMD_CHANNEL_TX_TIMEOUT;
		}
	}

	return result;
}

/**
 * Release the lock held by a previous successful call to
 * cmd_channel_i2c_dw_apb_begin_master_send_packet.
 *
 * @param i2c_chan The command channel instance.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_channel_i2c_dw_apb_finish_master_send_packet (
	const struct cmd_channel_i2c_dw_apb *i2c_chan)
{
	if (i2c_chan == NULL) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	return i2c_dw_apb_unlock_driver (i2c_chan->i2c_hw);
}
