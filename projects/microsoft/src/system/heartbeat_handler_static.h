// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HEARTBEAT_HANDLER_STATIC_H_
#define HEARTBEAT_HANDLER_STATIC_H_

#include "heartbeat_handler.h"


/* Internal functions declared to allow for static initialization. */
void heartbeat_handler_prepare (const struct periodic_task_handler *handler);
const platform_clock* heartbeat_handler_get_next_execution (
	const struct periodic_task_handler *handler);
void heartbeat_handler_execute (const struct periodic_task_handler *handler);


/**
 * Constant initializer for the CPU health heartbeat handler API.
 */
#define	HEARTBEAT_HANDLER_API_INIT	{ \
		.prepare = heartbeat_handler_prepare, \
		.get_next_execution = heartbeat_handler_get_next_execution, \
		.execute = heartbeat_handler_execute, \
	}


/**
 * Initialize a static instance of a CPU health heartbeat handler.  This does not initialize the
 * handler state. This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the handler.
 * @param notifier_ptr The notifier used to transmit heartbeat notifications.
 * @param period_ms The amount of time between heartbeat notifications, in milliseconds.
 * @param timeout_seconds The timeout value, in seconds, included in heartbeat notifications.
 * @param core_id_arg The core identifier to report in heartbeat notifications.
 * @param health_status_arg The health status to report in heartbeat notifications.
 */
#define	heartbeat_handler_static_init(state_ptr, notifier_ptr, period_ms, \
	timeout_seconds, core_id_arg, health_status_arg) {  \
		.base = HEARTBEAT_HANDLER_API_INIT, \
		.state = state_ptr, \
		.notifier = notifier_ptr, \
		.period = period_ms, \
		.timeout_secs = timeout_seconds, \
		.core_id = core_id_arg, \
		.health_status = health_status_arg, \
	}


#endif	/* HEARTBEAT_HANDLER_STATIC_H_ */
