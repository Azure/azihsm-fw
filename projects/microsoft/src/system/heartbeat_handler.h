// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HEARTBEAT_HANDLER_H_
#define HEARTBEAT_HANDLER_H_

#include <stdbool.h>
#include <stdint.h>
#include "mctp/mctp_notifier_interface.h"
#include "system/periodic_task.h"


/**
 * Variable context for the heartbeat handler.
 */
struct heartbeat_handler_state {
	platform_clock next;	/**< Time at which the next execution should run. */
	bool next_valid;		/**< Indicate if the next timeout has been initialized. */
};

/**
 * Handler for periodic heartbeat notifications.
 *
 * This handler only supports sending heartbeat notifications for a single CPU.
 *
 */
struct heartbeat_handler {
	struct periodic_task_handler base;				/**< Base interface for task integration. */
	struct heartbeat_handler_state *state;			/**< Variable context for the handler. */
	const struct mctp_notifier_interface *notifier;	/**< MCTP notifier instance. */
	uint32_t period;								/**< Time between heartbeat notifications, in ms. */
	uint16_t timeout_secs;							/**< Timeout value, in seconds */
	uint16_t core_id;								/**< Core identifier in the heartbeat payload. */
	uint16_t health_status;							/**< Health status in the heartbeat payload. */
};


int heartbeat_handler_init (struct heartbeat_handler *handler,
	struct heartbeat_handler_state *state, const struct mctp_notifier_interface *mctp_notifier,
	uint32_t period_ms, uint16_t timeout_secs, uint16_t core_id, uint16_t health_status);
int heartbeat_handler_init_state (const struct heartbeat_handler *handler);
void heartbeat_handler_release (const struct heartbeat_handler *handler);


#endif	/* HEARTBEAT_HANDLER_H_ */
