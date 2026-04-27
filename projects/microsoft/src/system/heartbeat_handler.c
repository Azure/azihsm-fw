// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <string.h>
#include "heartbeat_handler.h"
#include "common/unused.h"
#include "msft_protocol/msft_base_commands.h"
#include "msft_protocol/msft_protocol_logging.h"


/**
 * Prepare the next valid execution by initializing the timeout.
 *
 * @param handler Handler for periodic heartbeat notifications.
 */
static void heartbeat_handler_prepare_internal (
	const struct heartbeat_handler *heartbeat)
{
	if (platform_init_timeout (heartbeat->period, &heartbeat->state->next) == 0) {
		heartbeat->state->next_valid = true;
	}
	else {
		heartbeat->state->next_valid = false;
	}
}

void heartbeat_handler_prepare (const struct periodic_task_handler *handler)
{
	const struct heartbeat_handler *heartbeat =
		(const struct heartbeat_handler*) handler;

	heartbeat_handler_prepare_internal (heartbeat);
}

const platform_clock* heartbeat_handler_get_next_execution (
	const struct periodic_task_handler *handler)
{
	const struct heartbeat_handler *heartbeat =
		(const struct heartbeat_handler*) handler;

	if (heartbeat->state->next_valid) {
		return &heartbeat->state->next;
	}
	else {
		return NULL;
	}
}

void heartbeat_handler_execute (const struct periodic_task_handler *handler)
{
	const struct heartbeat_handler *heartbeat =
		(const struct heartbeat_handler*) handler;
	uint8_t payload[sizeof (struct msft_base_heartbeat_request)];
	size_t payload_len;

	/* In the current implementation, the build request will always succeed hence skip error checking. */
	payload_len = msft_base_build_heartbeat_request (heartbeat->timeout_secs, heartbeat->core_id,
		heartbeat->health_status, payload, sizeof (payload));

	heartbeat->notifier->send_notification_request (heartbeat->notifier, payload, payload_len);

	heartbeat_handler_prepare_internal (heartbeat);
}

/**
 * Initialize a handler for heartbeat notifications.
 *
 * @param handler The heartbeat handler to initialize.
 * @param state Variable context for the handler. This must be uninitialized.
 * @param notifier Notifier to use for transmitting heartbeat notifications.
 * @param period_ms The amount of time between heartbeat notifications, in milliseconds.
 * @param timeout_secs The timeout value, in seconds, included in heartbeat notifications.
 * @param core_id The core identifier to report in heartbeat notifications.
 * @param health_status The health status to report in heartbeat notifications.
 *
 * @return 0 if the handler was successfully initialized or an error code.
 */
int heartbeat_handler_init (struct heartbeat_handler *handler,
	struct heartbeat_handler_state *state, const struct mctp_notifier_interface *mctp_notifier,
	uint32_t period_ms, uint16_t timeout_secs, uint16_t core_id, uint16_t health_status)
{
	if ((handler == NULL) || (state == NULL) || (mctp_notifier == NULL) || (period_ms == 0)) {
		return PERIODIC_TASK_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (*handler));

	handler->base.prepare = heartbeat_handler_prepare;
	handler->base.get_next_execution = heartbeat_handler_get_next_execution;
	handler->base.execute = heartbeat_handler_execute;

	handler->state = state;
	handler->notifier = mctp_notifier;
	handler->period = period_ms;
	handler->timeout_secs = timeout_secs;
	handler->core_id = core_id;
	handler->health_status = health_status;

	return heartbeat_handler_init_state (handler);
}

/**
 * Initialize only the variable state for a  heartbeat handler.  The rest of the handler
 * is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param handler The  heartbeat handler that contains the state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int heartbeat_handler_init_state (const struct heartbeat_handler *handler)
{
	if ((handler == NULL) || (handler->state == NULL) || (handler->notifier == NULL) ||
		(handler->period == 0)) {
		return PERIODIC_TASK_INVALID_ARGUMENT;
	}

	memset (handler->state, 0, sizeof (struct heartbeat_handler_state));

	return 0;
}

/**
 * Release the resources used by a  heartbeat handler.
 *
 * @param handler The handler to release.
 */
void heartbeat_handler_release (const struct heartbeat_handler *handler)
{
	UNUSED (handler);
}
