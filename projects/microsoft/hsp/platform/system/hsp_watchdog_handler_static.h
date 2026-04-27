// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_WATCHDOG_HANDLER_STATIC_H_
#define HSP_WATCHDOG_HANDLER_STATIC_H_

#include "hsp_watchdog_handler.h"


/* Internal functions declared to allow for static initialization. */
void hsp_watchdog_handler_prepare (const struct periodic_task_handler *handler);
void hsp_watchdog_handler_prepare_refresh_only (const struct periodic_task_handler *handler);
const platform_clock* hsp_watchdog_handler_get_next_execution (
	const struct periodic_task_handler *handler);
void hsp_watchdog_handler_execute (const struct periodic_task_handler *handler);


/**
 * Constant initializer for the periodic handler API.
 */
#define	HSP_WATCHDOG_HANDLER_API_INIT	{ \
		.prepare = hsp_watchdog_handler_prepare, \
		.get_next_execution = hsp_watchdog_handler_get_next_execution, \
		.execute = hsp_watchdog_handler_execute, \
	}

/**
 * Constant initializer for the periodic handler API without starting the timer.
 */
#define	HSP_WATCHDOG_HANDLER_REFRESH_ONLY_API_INIT	{ \
		.prepare = hsp_watchdog_handler_prepare_refresh_only, \
		.get_next_execution = hsp_watchdog_handler_get_next_execution, \
		.execute = hsp_watchdog_handler_execute, \
	}


/**
 * Initialize a static handler for the HSP watchdog.  The handler will be responsible for starting
 * the watchdog timer.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the watchdog handler.
 * @param watchdog_ptr The watchdog that will be managed by the handler.
 * @param refresh_period_ms_arg The amount of time between calls to refresh the watchdog.
 * @param enable_sticky_error_arg Flag to indicate that sticky errors should be on watchdog
 * failures.
 */
#define	hsp_watchdog_handler_static_init(state_ptr, watchdog_ptr, refresh_period_ms_arg, \
	enable_sticky_error_arg) { \
		.base = HSP_WATCHDOG_HANDLER_API_INIT, \
		.state = state_ptr, \
		.watchdog = watchdog_ptr, \
		.refresh_period = refresh_period_ms_arg, \
		.enable_sticky_error = enable_sticky_error_arg, \
	}

/**
 * Initialize a static handler for the HSP watchdog.  The watchdog timer must be started externally
 * to the handler.  The handler execution will only refresh the timer.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the watchdog handler.
 * @param watchdog_ptr The watchdog that will be managed by the handler.
 * @param refresh_period_ms_arg The amount of time between calls to refresh the watchdog.
 */
#define	hsp_watchdog_handler_static_init_refresh_only(state_ptr, watchdog_ptr, \
	refresh_period_ms_arg) { \
		.base = HSP_WATCHDOG_HANDLER_REFRESH_ONLY_API_INIT, \
		.state = state_ptr, \
		.watchdog = watchdog_ptr, \
		.refresh_period = refresh_period_ms_arg, \
	}


#endif	/* HEARTBEAT_LED_HANDLER_STATIC_H_ */
