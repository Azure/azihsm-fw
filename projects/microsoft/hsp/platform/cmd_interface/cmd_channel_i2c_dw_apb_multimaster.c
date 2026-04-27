// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "cmd_channel_i2c_dw_apb_multimaster.h"
#include "hsp_top.h"
#include "task_priority.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "freertos/freertos_utils.h"
#include "logging/hsp_logging.h"


/**
 * The delay to allow the I2C HW to process any Rx operations before trying a MASTER Tx again.
 */
#define CMD_CHANNEL_I2C_DW_APB_TX_RETRY_DELAY_MS		50


/* Utilities */

/**
 * Gets the I2C multi-master command channel instance from the I2C handler.
 *
 * @return The associated command channel instance.
 */
static const struct cmd_channel_i2c_dw_apb_multimaster*
cmd_channel_i2c_dw_apb_multimaster_from_i2c_handler (const struct i2c_dw_apb_handler *handler)
{
	return TO_DERIVED_TYPE (handler, const struct cmd_channel_i2c_dw_apb_multimaster, i2c_handler);
}

/**
 * Completes the currently pending Tx packet.
 *
 * @param i2c_chan The I2C slave command channel instance.
 * @param result The result of the transmission.
 *
 * @return NULL if no packet was queued or listener abandoned the transmission, else a valid packet.
 */
static void cmd_channel_i2c_dw_apb_multimaster_tx_finished (
	const struct cmd_channel_i2c_dw_apb_multimaster *i2c_chan, int result)
{
	struct cmd_channel_i2c_dw_apb_multimaster_state *state =
		(struct cmd_channel_i2c_dw_apb_multimaster_state*) i2c_chan->base.base.state;

	if (cmd_channel_i2c_dw_apb_master_tx_set_result (&state->base, result) == 0) {
		cmd_channel_i2c_dw_apb_signal_txn_complete (&state->base);
	}
}

/**
 * Initializes the multi-master command channel variable state context that hasn't been initialized
 * by the base initializers.
 *
 * @param i2c The I2C multi-master command channel instance.
 * @param rx_buffers An array of cmd_packet buffers used for receiving packets.
 * @param rx_buf_count The number of buffers in rx_buffers.
 *
 * @return 0 if success, else an error code.
 */
static int cmd_channel_i2c_dw_apb_multimaster_init_state_local (
	const struct cmd_channel_i2c_dw_apb_multimaster *channel, struct cmd_packet *rx_buffers,
	size_t rx_buf_count)
{
	struct cmd_channel_i2c_dw_apb_multimaster_state *state =
		(struct cmd_channel_i2c_dw_apb_multimaster_state*) channel->base.base.state;
	int status;

	status = cmd_channel_i2c_dw_apb_slave_rx_init_state (&state->rx_state, rx_buffers,
		rx_buf_count);
	if (status != 0) {
		cmd_channel_i2c_dw_apb_release (&channel->base);
	}

	return status;
}

/* Interface Implementation */

void cmd_channel_i2c_dw_apb_multimaster_on_shutdown (const struct i2c_dw_apb_handler *handler)
{
	const struct cmd_channel_i2c_dw_apb_multimaster *i2c_chan =
		cmd_channel_i2c_dw_apb_multimaster_from_i2c_handler (handler);
	struct cmd_channel_i2c_dw_apb_multimaster_state *state =
		(struct cmd_channel_i2c_dw_apb_multimaster_state*) i2c_chan->base.base.state;

	cmd_channel_i2c_dw_apb_slave_rx_on_shutdown (&state->rx_state);

	if (cmd_channel_i2c_dw_apb_master_tx_set_result (&state->base, CMD_CHANNEL_TX_FAILED) == 0) {
		cmd_channel_i2c_dw_apb_tx_on_shutdown (&state->base);
	}
}

size_t cmd_channel_i2c_dw_apb_multimaster_on_rx_pending (const struct i2c_dw_apb_handler *handler,
	uint8_t **data)
{
	const struct cmd_channel_i2c_dw_apb_multimaster *i2c_chan =
		cmd_channel_i2c_dw_apb_multimaster_from_i2c_handler (handler);
	struct cmd_channel_i2c_dw_apb_multimaster_state *state =
		(struct cmd_channel_i2c_dw_apb_multimaster_state*) i2c_chan->base.base.state;

	return cmd_channel_i2c_dw_apb_slave_rx_on_rx_pending (&i2c_chan->base, &state->rx_state, data);
}

void cmd_channel_i2c_dw_apb_multimaster_on_rx_discard (const struct i2c_dw_apb_handler *handler)
{
	const struct cmd_channel_i2c_dw_apb_multimaster *i2c_chan =
		cmd_channel_i2c_dw_apb_multimaster_from_i2c_handler (handler);
	struct cmd_channel_i2c_dw_apb_multimaster_state *state =
		(struct cmd_channel_i2c_dw_apb_multimaster_state*) i2c_chan->base.base.state;

	cmd_channel_i2c_dw_apb_slave_rx_on_discard (&state->rx_state);
}

void cmd_channel_i2c_dw_apb_multimaster_on_rx_data (const struct i2c_dw_apb_handler *handler,
	size_t count)
{
	const struct cmd_channel_i2c_dw_apb_multimaster *i2c_chan =
		cmd_channel_i2c_dw_apb_multimaster_from_i2c_handler (handler);
	struct cmd_channel_i2c_dw_apb_multimaster_state *state =
		(struct cmd_channel_i2c_dw_apb_multimaster_state*) i2c_chan->base.base.state;

	cmd_channel_i2c_dw_apb_slave_rx_on_data (&state->rx_state, count);
}

void cmd_channel_i2c_dw_apb_multimaster_on_rx_complete (const struct i2c_dw_apb_handler *handler)
{
	const struct cmd_channel_i2c_dw_apb_multimaster *i2c_chan =
		cmd_channel_i2c_dw_apb_multimaster_from_i2c_handler (handler);
	struct cmd_channel_i2c_dw_apb_multimaster_state *state =
		(struct cmd_channel_i2c_dw_apb_multimaster_state*) i2c_chan->base.base.state;

	cmd_channel_i2c_dw_apb_slave_rx_complete (&i2c_chan->base, &state->rx_state);
}

void cmd_channel_i2c_dw_apb_multimaster_on_tx_abort (const struct i2c_dw_apb_handler *handler)
{
	const struct cmd_channel_i2c_dw_apb_multimaster *i2c_chan =
		cmd_channel_i2c_dw_apb_multimaster_from_i2c_handler (handler);

	cmd_channel_i2c_dw_apb_multimaster_tx_finished (i2c_chan, CMD_CHANNEL_TX_ABORTED);
}

void cmd_channel_i2c_dw_apb_multimaster_on_tx_complete (const struct i2c_dw_apb_handler *handler)
{
	const struct cmd_channel_i2c_dw_apb_multimaster *i2c_chan =
		cmd_channel_i2c_dw_apb_multimaster_from_i2c_handler (handler);

	cmd_channel_i2c_dw_apb_multimaster_tx_finished (i2c_chan, 0);
}

int cmd_channel_i2c_dw_apb_multimaster_receive_packet (const struct cmd_channel *channel,
	struct cmd_packet *packet, int timeout_ms)
{
	struct cmd_channel_i2c_dw_apb_multimaster_state *state;

	if (channel == NULL) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	state = (struct cmd_channel_i2c_dw_apb_multimaster_state*) channel->state;

	return cmd_channel_i2c_dw_apb_slave_rx_receive_packet (&state->rx_state, packet, timeout_ms);
}

int cmd_channel_i2c_dw_apb_multimaster_send_packet (const struct cmd_channel *channel,
	const struct cmd_packet *packet)
{
	const struct cmd_channel_i2c_dw_apb_multimaster *i2c_chan =
		(const struct cmd_channel_i2c_dw_apb_multimaster*) channel;
	struct cmd_channel_i2c_dw_apb_multimaster_state *state;
	int status;
	int result;
	platform_clock timeout;
	int addr_nack = 3;

	if ((channel == NULL) || (packet == NULL)) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	state = (struct cmd_channel_i2c_dw_apb_multimaster_state*) channel->state;

	status = cmd_channel_i2c_dw_apb_begin_master_send_packet (&i2c_chan->base, &timeout);
	if (status != 0) {
		return status;
	}

	for (;;) {
		result = cmd_channel_i2c_dw_apb_try_master_send_packet (&i2c_chan->base, &timeout, packet);

		status = i2c_dw_apb_enable_slave_mode (i2c_chan->base.i2c_hw);
		if (result == 0) {
			result = status;
		}

		if (result != 0) {
			break;
		}

		result = state->base.txn_result;
		if (result == CMD_CHANNEL_TX_ABORTED) {
			uint32_t abort_source = i2c_dw_apb_get_last_tx_abort_source (i2c_chan->base.i2c_hw);

			if ((abort_source & I2C_DW_APB_TX_ABRT_SOURCE (ABRT_7B_ADDR_NOACK)) ||
				(abort_source & I2C_DW_APB_TX_ABRT_SOURCE (ABRT_10ADDR1_NOACK)) ||
				(abort_source & I2C_DW_APB_TX_ABRT_SOURCE (ABRT_10ADDR2_NOACK))) {
				/* The target device did not ACK the address.  Retry a few times to see if the
				 * device becomes available.  If it does not, fail the transaction. */
				if (--addr_nack <= 0) {
					result = CMD_CHANNEL_UNKNOWN_TARGET;
				}
				else {
					result = CMD_CHANNEL_TX_FAILED;
				}
			}
			else if (!(abort_source & I2C_DW_APB_TX_ABRT_SOURCE (ABRT_USER_ABRT))) {
				/* If the abort was triggered internally by FW, fail the transaction by leaving the
				 * result unchanged.  Otherwise, treat this as a transient bus condition and
				 * retry. */
				debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_HSP,
					HSP_LOGGING_I2C_TX_ABORT, cmd_channel_get_id (&i2c_chan->base.base),
					abort_source);

				result = CMD_CHANNEL_TX_FAILED;
			}
		}

		if (result != CMD_CHANNEL_TX_FAILED) {
			// Success or fatal error
			break;
		}

		// MASTER lost arbitration.  Delay to process any SLAVE Rx and then retry

		platform_msleep (CMD_CHANNEL_I2C_DW_APB_TX_RETRY_DELAY_MS);

		result = platform_has_timeout_expired (&timeout);
		if (result != 0) {
			result = CMD_CHANNEL_TX_TIMEOUT;
			break;
		}
	}

	status = cmd_channel_i2c_dw_apb_finish_master_send_packet (&i2c_chan->base);
	if (result == 0) {
		result = status;
	}

	return result;
}

void cmd_channel_i2c_dw_apb_multimaster_on_shutdown_system (struct system_observer *observer)
{
	const struct cmd_channel_i2c_dw_apb_multimaster *i2c_chan = TO_DERIVED_TYPE (observer,
		const struct cmd_channel_i2c_dw_apb_multimaster, base_observer);
	int status;

	/* Abort any active transfer and shutdown the I2C controller. */
	i2c_dw_apb_abort_tx (i2c_chan->base.i2c_hw);

	i2c_dw_apb_lock_driver (i2c_chan->base.i2c_hw);

	status = i2c_dw_apb_shutdown_hw (i2c_chan->base.i2c_hw);
	if (status != 0) {
		/* If the hardware failed to shutdown, log the error.  Locking the driver should be enough
		 * to prevent new messages from going out.  Shutdown just adds cleaner handling for received
		 * messages. */
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_I2C_SHUTDOWN_FAILED, status, 0);
	}
}

void cmd_channel_i2c_dw_apb_multimaster_on_shutdown_failed (struct system_observer *observer)
{
	const struct cmd_channel_i2c_dw_apb_multimaster *i2c_chan = TO_DERIVED_TYPE (observer,
		const struct cmd_channel_i2c_dw_apb_multimaster, base_observer);
	int status;

	status = i2c_dw_apb_enable_slave_mode (i2c_chan->base.i2c_hw);
	if (status != 0) {
		/* If this fails (which is unlikely, if not impossible), there is nothing that can really be
		 * done except log the failure for future reference.  Of course, if the I2C isn't working,
		 * the log won't be available until after the device resets. */
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_I2C_RESUME_FAILED, status, 0);
	}

	i2c_dw_apb_unlock_driver (i2c_chan->base.i2c_hw);
}

/* Public API */

/**
 * Initializes the multi-master DW APB I2C command channel driver and state context.
 *
 * @param channel The I2C multi-master command channel instance.
 * @param state The variable context for the driver.
 * @param i2c_hw The underlying I2C hardware for communication.
 * @param timeout_val_ms Timeout in milliseconds for packets.
 * @param channel_id Command channel ID assigned to this instance.
 * @param rx_buffers An array of cmd_packet buffers used for receiving packets.
 * @param rx_buf_count The number of buffers to allocate for the RX queue.
 *
 * @return Returns 0 if success, else an error code
 */
int cmd_channel_i2c_dw_apb_multimaster_init (struct cmd_channel_i2c_dw_apb_multimaster *channel,
	struct cmd_channel_i2c_dw_apb_multimaster_state *state,
	const struct i2c_dw_apb_multimaster *i2c_hw, int channel_id, uint32_t timeout_val_ms,
	struct cmd_packet *rx_buffers, size_t rx_buf_count)
{
	int status;

	if ((channel == NULL) || (state == NULL)) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	memset (channel, 0, sizeof (*channel));
	memset (state, 0, sizeof (*state));

	channel->i2c_handler.rx_handler.handler_base.on_shutdown =
		cmd_channel_i2c_dw_apb_multimaster_on_shutdown;
	channel->i2c_handler.rx_handler.handler_base.on_rx_discard =
		cmd_channel_i2c_dw_apb_multimaster_on_rx_discard;
	channel->i2c_handler.rx_handler.handler_base.on_rx_complete =
		cmd_channel_i2c_dw_apb_multimaster_on_rx_complete;
	channel->i2c_handler.rx_handler.handler_base.on_tx_abort =
		cmd_channel_i2c_dw_apb_multimaster_on_tx_abort;
	channel->i2c_handler.rx_handler.handler_base.on_tx_complete =
		cmd_channel_i2c_dw_apb_multimaster_on_tx_complete;
	channel->i2c_handler.rx_handler.on_rx_pending =
		cmd_channel_i2c_dw_apb_multimaster_on_rx_pending;
	channel->i2c_handler.rx_handler.on_rx_data =
		cmd_channel_i2c_dw_apb_multimaster_on_rx_data;

	channel->base.base.receive_packet = cmd_channel_i2c_dw_apb_multimaster_receive_packet;
	channel->base.base.send_packet = cmd_channel_i2c_dw_apb_multimaster_send_packet;

	channel->base_observer.on_shutdown = cmd_channel_i2c_dw_apb_multimaster_on_shutdown_system;
	channel->base_observer.on_shutdown_failed =
		cmd_channel_i2c_dw_apb_multimaster_on_shutdown_failed;

	status = cmd_channel_i2c_dw_apb_init (&channel->base, &state->base,	&i2c_hw->i2c_base,
		channel_id, timeout_val_ms);
	if (status != 0) {
		return status;
	}

	return cmd_channel_i2c_dw_apb_multimaster_init_state_local (channel, rx_buffers, rx_buf_count);
}

/**
 * Initializes the multi-master DW APB I2C command channel variable state context.
 *
 * @param channel The I2C multi-master command channel instance.
 * @param rx_buffers An array of cmd_packet buffers used for receiving packets.
 * @param rx_buf_count The number of buffers in rx_buffers.
 *
 * @return Returns 0 if success, else an error code
 */
int cmd_channel_i2c_dw_apb_multimaster_init_state (
	const struct cmd_channel_i2c_dw_apb_multimaster *channel, struct cmd_packet *rx_buffers,
	size_t rx_buf_count)
{
	struct cmd_channel_i2c_dw_apb_multimaster_state *state;
	int status;

	if (channel == NULL) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	state = (struct cmd_channel_i2c_dw_apb_multimaster_state*) channel->base.base.state;
	if (state == NULL) {
		return CMD_CHANNEL_INVALID_ARGUMENT;
	}

	memset (state, 0, sizeof (*state));

	status = cmd_channel_i2c_dw_apb_init_state (&channel->base);
	if (status != 0) {
		return status;
	}

	return cmd_channel_i2c_dw_apb_multimaster_init_state_local (channel, rx_buffers, rx_buf_count);
}

/**
 * Releases the resources held by the multi-master command channel instance.
 *
 * @param channel The command channel instance.
 */
void cmd_channel_i2c_dw_apb_multimaster_release (
	const struct cmd_channel_i2c_dw_apb_multimaster *channel)
{
	struct cmd_channel_i2c_dw_apb_multimaster_state *state;

	if (channel == NULL) {
		return;
	}

	state = (struct cmd_channel_i2c_dw_apb_multimaster_state*) channel->base.base.state;
	if (state != NULL) {
		cmd_channel_i2c_dw_apb_slave_rx_release (&state->rx_state);
	}

	cmd_channel_i2c_dw_apb_release (&channel->base);
}
