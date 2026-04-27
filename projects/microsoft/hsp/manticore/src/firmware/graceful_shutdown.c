// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "graceful_shutdown.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "ipc/ipc_message.h"
#include "logging/log_flush_handler.h"

#ifdef MANTICORE_ROT_RESET_CRASH
#include "platform_io_api.h"
#include "dc_scm/init/init_crashdump.h"
#include "dc_scm/sp_boot.h"
#endif


int graceful_shutdown_get_allowed_drain_time (const struct graceful_shutdown_control *handler,
	uint32_t *drain_time_ms)
{
	const struct graceful_shutdown *shutdown = TO_DERIVED_TYPE (handler,
		const struct graceful_shutdown, base_ctrl);

	if ((handler == NULL) || (drain_time_ms == NULL)) {
		return GRACEFUL_SHUTDOWN_INVALID_ARGUMENT;
	}

	*drain_time_ms = shutdown->state->drain_time_ms;

	return 0;
}

int graceful_shutdown_set_allowed_drain_time (const struct graceful_shutdown_control *handler,
	uint32_t drain_time_ms)
{
	const struct graceful_shutdown *shutdown = TO_DERIVED_TYPE (handler,
		const struct graceful_shutdown, base_ctrl);

	if (handler == NULL) {
		return GRACEFUL_SHUTDOWN_INVALID_ARGUMENT;
	}

	if (drain_time_ms > GRACEFUL_SHUTDOWN_MAX_ALLOWED_DRAIN_TIME) {
		return GRACEFUL_SHUTDOWN_OUT_OF_RANGE;
	}

	if (drain_time_ms == 0) {
		shutdown->state->drain_time_ms = shutdown->default_drain_time;
	}
	else {
		shutdown->state->drain_time_ms = drain_time_ms;
	}

	return 0;
}

int graceful_shutdown_get_uuid (const struct cmd_device *device, uint8_t *buffer, size_t buf_len)
{
	const struct graceful_shutdown *shutdown = TO_DERIVED_TYPE (device,
		const struct graceful_shutdown, base_device);

	if (device == NULL) {
		return GRACEFUL_SHUTDOWN_INVALID_ARGUMENT;
	}

	return shutdown->device->get_uuid (shutdown->device, buffer, buf_len);
}

int graceful_shutdown_reset (const struct cmd_device *device)
{
	const struct graceful_shutdown *shutdown = TO_DERIVED_TYPE (device,
		const struct graceful_shutdown, base_device);
	struct ipc_message ipc_msg = {0};
	struct ipc_message_shutdown_request *request = (struct ipc_message_shutdown_request*) &ipc_msg;
	int status;

	if (device == NULL) {
		return GRACEFUL_SHUTDOWN_INVALID_ARGUMENT;
	}

#ifdef MANTICORE_ROT_RESET_CRASH
	UNUSED (shutdown);
	UNUSED (request);

	status = trigger_crashdump_soc ();
	platform_printf (
		"TESTTEST: Fault-injection: Triggered crash interrupt to ARM cores: %x" NEWLINE, status);

#ifndef MANTICORE_ROT_RESET_INT_ONLY
	platform_printf ("TESTTEST: Fault-injection: Crash Reset" NEWLINE);

	/* Best-effort attempt to flush prior to the reset. Failures are ignored. */
	log_flush_handler_immediate_flush (shutdown->log_flush);

	/* Reset SPRT, 1SP will make warm boot. */
	boot_error_reset (sw_regs);
#endif	// MANTICORE_ROT_RESET_INT_ONLY

	/* We should never get here. */
	status = CMD_DEVICE_RESET_FAILED;
#else	// MANTICORE_ROT_RESET_CRASH
	request->header.opcode = IPC_MESSAGE_OPCODE_SHUTDOWN_REQUEST;
	request->header.data_length = sizeof (request->payload);
	request->payload.drain_time = shutdown->state->drain_time_ms;

	/* Message timeout needs to account for worst-case drain time plus additional processing. */
	status = shutdown->ipc->send_and_receive (shutdown->ipc, &ipc_msg,
		shutdown->state->drain_time_ms + shutdown->ipc_timeout);
	if (status != 0) {
		return status;
	}

	/* Check the response status to see if the request was successful. */
	switch (ipc_msg.header.status) {
		case 0:
			/* Success */
			break;

		case IPC_MESSAGE_SHUTDOWN_REQUEST_ERROR:
			return GRACEFUL_SHUTDOWN_IPC_ERROR_RESP;

		case IPC_MESSAGE_SHUTDOWN_REQUEST_TIMEOUT:
			return GRACEFUL_SHUTDOWN_IPC_TIMEOUT_RESP;

		case IPC_MESSAGE_SHUTDOWN_REQUEST_FAILURE:
			return GRACEFUL_SHUTDOWN_IPC_FAILURE_RESP;

		default:
			/* The response indicates an error, but with an unknown reason. */
			return GRACEFUL_SHUTDOWN_IPC_RESP_UNKNOWN;
	}

	/* Best-effort attempt to flush prior to the reset. Failures are ignored. */
	log_flush_handler_immediate_flush (shutdown->log_flush);

	*shutdown->indicator = GRACEFUL_SHUTDOWN_INDICATOR_MAGIC_NUMBER;

	status = shutdown->device->reset (shutdown->device);
	if (status != 0) {
		/* Clear the shutdown indicator, since the reset failed. */
		*shutdown->indicator = 0;
	}
#endif	// MANTICORE_ROT_RESET_CRASH

	return status;
}

int graceful_shutdown_get_reset_counter (const struct cmd_device *device, uint8_t type,
	uint8_t port, uint16_t *counter)
{
	const struct graceful_shutdown *shutdown = TO_DERIVED_TYPE (device,
		const struct graceful_shutdown, base_device);

	if (device == NULL) {
		return GRACEFUL_SHUTDOWN_INVALID_ARGUMENT;
	}

	return shutdown->device->get_reset_counter (shutdown->device, type, port, counter);
}

#ifdef CMD_ENABLE_HEAP_STATS
int graceful_shutdown_get_heap_stats (const struct cmd_device *device,
	struct cmd_device_heap_stats *heap)
{
	const struct graceful_shutdown *shutdown = TO_DERIVED_TYPE (device,
		const struct graceful_shutdown, base_device);

	if (device == NULL) {
		return GRACEFUL_SHUTDOWN_INVALID_ARGUMENT;
	}

	return shutdown->device->get_heap_stats (shutdown->device, heap);
}
#endif

/**
 * Initialize a handler for coordinating a graceful shutdown of CP and FP cores during device reset.
 *
 * @param shutdown The handler to initialize.
 * @param state Variable context for the handler.  This must be uninitialized.
 * @param device Underlying handler for device commands.  This will be used to actually trigger the
 * reset once the shutdown has completed.
 * @param ipc Channel to use for sending IPC requests to the CP Admin core.
 * @param log_flush Handler for flushing log data.
 * @param shutdown_indicator Reset tolerant storage for the magic number indicating a graceful
 * shutdown and reset.
 * @param shutdown_timeout_ms The timeout to use when sending shutdown IPC messages, in
 * milliseconds.  This will be added to the configured drain time to determine how long to wait.
 * @param drain_time_ms The initial value to configure for the allowed drain time.  This will also
 * serve as the default value to use, if requested.
 *
 * @return 0 if the shutdown handler was successfully initialized or an error code.
 */
int graceful_shutdown_init (struct graceful_shutdown *shutdown,
	struct graceful_shutdown_state *state, const struct cmd_device *device,
	const struct ipc_channel *ipc, const struct log_flush_handler *log_flush,
	volatile uint32_t *shutdown_indicator, uint32_t shutdown_timeout_ms, uint32_t drain_time_ms)
{
	if (shutdown == NULL) {
		return GRACEFUL_SHUTDOWN_INVALID_ARGUMENT;
	}

	memset (shutdown, 0, sizeof (*shutdown));

	shutdown->base_ctrl.get_allowed_drain_time = graceful_shutdown_get_allowed_drain_time;
	shutdown->base_ctrl.set_allowed_drain_time = graceful_shutdown_set_allowed_drain_time;

	shutdown->base_device.get_uuid = graceful_shutdown_get_uuid;
	shutdown->base_device.reset = graceful_shutdown_reset;
	shutdown->base_device.get_reset_counter = graceful_shutdown_get_reset_counter;
#ifdef CMD_ENABLE_HEAP_STATS
	shutdown->base_device.get_heap_stats = graceful_shutdown_get_heap_stats;
#endif

	shutdown->state = state;
	shutdown->device = device;
	shutdown->ipc = ipc;
	shutdown->indicator = shutdown_indicator;
	shutdown->ipc_timeout = shutdown_timeout_ms;
	shutdown->log_flush = log_flush;
	shutdown->default_drain_time = drain_time_ms;

	return graceful_shutdown_init_state (shutdown);
}

/**
 * Initialize only the variable state of the graceful shutdown handler.  The rest of the instance
 * is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param shutdown The handler that contains the state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int graceful_shutdown_init_state (const struct graceful_shutdown *shutdown)
{
	if ((shutdown == NULL) || (shutdown->state == NULL) || (shutdown->device == NULL) ||
		(shutdown->ipc == NULL) || (shutdown->indicator == NULL) || (shutdown->log_flush == NULL)) {
		return GRACEFUL_SHUTDOWN_INVALID_ARGUMENT;
	}

	memset (shutdown->state, 0, sizeof (*shutdown->state));

	shutdown->state->drain_time_ms = shutdown->default_drain_time;

	return 0;
}

/**
 * Release the resources used for an graceful shutdown handler.
 *
 * @param shutdown The handler to release.
 */
void graceful_shutdown_release (const struct graceful_shutdown *shutdown)
{
	UNUSED (shutdown);
}

/**
 * Send an IPC message to request a status change in the remote cores.
 *
 * @param shutdown The handler sending the IPC.
 * @param requested_status The requested status to send.
 * @param resp_timeout_ms The amount of time to wait for a response, in milliseconds.
 *
 * @return 0 if the IPC was successful or an error code.
 */
static int graceful_shutdown_send_ipc_status_change (const struct graceful_shutdown *shutdown,
	uint8_t requested_status, uint32_t resp_timeout_ms)
{
	struct ipc_message ipc_msg = {0};
	struct ipc_message_status_change *request = (struct ipc_message_status_change*) &ipc_msg;
	int status;

	request->header.opcode = IPC_MESSAGE_OPCODE_STATUS_CHANGE;
	request->header.data_length = sizeof (request->payload);
	request->payload.requested_status = requested_status;

	status = shutdown->ipc->send_and_receive (shutdown->ipc, &ipc_msg, resp_timeout_ms);
	if (status != 0) {
		return status;
	}

	/* Check if the request was successful. */
	if (request->header.status != 0) {
		return GRACEFUL_SHUTDOWN_IPC_ERROR_RESP;
	}

	return 0;
}

/**
 * Resume normal operation following a reset after a graceful shutdown.
 *
 * @param shutdown The handler to use for resuming operations.
 * @param resp_timeout_ms The amount of time to wait for a response to IPC messages, in
 * milliseconds.
 *
 * @return 0 if normal operation was resumed successfully or an error code.
 */
int graceful_shutdown_resume_normal_operation (const struct graceful_shutdown *shutdown,
	uint32_t resp_timeout_ms)
{
	int status;

	if (shutdown == NULL) {
		return GRACEFUL_SHUTDOWN_INVALID_ARGUMENT;
	}

	/* These messages are always required, even during cold boot and other scenarios where the other
	 * cores were not gracefully shutdown. */
	status = graceful_shutdown_send_ipc_status_change (shutdown,
		IPC_MESSAGE_STATUS_CHANGE_PREPARE_RELEASE, resp_timeout_ms);
	if (status != 0) {
		return status;
	}

	/* As soon as the RELEASE IPC is sent, graceful resets are not possible since the SoC cores
	 * could have altered their saved state.  This is true even if the IPC fails. */
	*shutdown->indicator = 0;

	status = graceful_shutdown_send_ipc_status_change (shutdown, IPC_MESSAGE_STATUS_CHANGE_RELEASE,
		resp_timeout_ms);
	if (status != 0) {
		return status;
	}

	return 0;
}
