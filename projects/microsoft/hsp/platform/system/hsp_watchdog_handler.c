// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_watchdog_handler.h"
#include "common/unused.h"
#include "system/system_logging.h"


/**
 * Set the time for the next execution of the watchdog refresh.
 *
 * @param wdt The watchdog handler to update.
 */
static void hsp_watchdog_handler_init_next_execution (const struct hsp_watchdog_handler *wdt)
{
	int status;

	status = platform_init_timeout (wdt->refresh_period, &wdt->state->next);
	wdt->state->next_valid = (status == 0);
}

void hsp_watchdog_handler_prepare (const struct periodic_task_handler *handler)
{
	const struct hsp_watchdog_handler *wdt = (const struct hsp_watchdog_handler*) handler;
	int status;

	status = hsp_watchdog_start (wdt->watchdog, wdt->enable_sticky_error);
	if (status != 0) {
		/* The only option here is to create a log message if the watchdog could not be started.
		 * The periodic task framework doesn't allow for a preparation failure, but a failure here
		 * is highly unlikely. */
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_SYSTEM,
			SYSTEM_LOGGING_START_WATCHDOG_FAIL, status, 0);
	}

	hsp_watchdog_handler_init_next_execution (wdt);
}

void hsp_watchdog_handler_prepare_refresh_only (const struct periodic_task_handler *handler)
{
	const struct hsp_watchdog_handler *wdt = (const struct hsp_watchdog_handler*) handler;

	/* Immediately refresh the watchdog. */
	wdt->state->next_valid = false;
}

const platform_clock* hsp_watchdog_handler_get_next_execution (
	const struct periodic_task_handler *handler)
{
	const struct hsp_watchdog_handler *wdt = (const struct hsp_watchdog_handler*) handler;

	if (wdt->state->next_valid) {
		return &wdt->state->next;
	}
	else {
		/* If the next execution time is not valid, immediately refresh the watchdog. */
		return NULL;
	}
}

void hsp_watchdog_handler_execute (const struct periodic_task_handler *handler)
{
	const struct hsp_watchdog_handler *wdt = (const struct hsp_watchdog_handler*) handler;
	int status;

	status = hsp_watchdog_refresh (wdt->watchdog);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_SYSTEM,
			SYSTEM_LOGGING_REFRESH_WATCHDOG_FAIL, status, 0);
	}

	hsp_watchdog_handler_init_next_execution (wdt);
}

/**
 * Initialize a periodic handler for the HSP watchdog.  The handler will be responsible for starting
 * the watchdog timer.
 *
 * @param handler The handler to initialize.
 * @param state Variable context for the watchdog handler.
 * @param watchdog The watchdog that will be managed by the handler.
 * @param refresh_period_ms The amount of time between calls to refresh the watchdog.
 * @param enable_sticky_error Flag to indicate that sticky errors should be on watchdog failures.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int hsp_watchdog_handler_init (struct hsp_watchdog_handler *handler,
	struct hsp_watchdog_handler_state *state, const struct hsp_watchdog *watchdog,
	uint32_t refresh_period_ms, bool enable_sticky_error)
{
	if ((handler == NULL) || (state == NULL) || (watchdog == NULL)) {
		return HSP_WATCHDOG_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (*handler));

	handler->base.prepare = hsp_watchdog_handler_prepare;
	handler->base.get_next_execution = hsp_watchdog_handler_get_next_execution;
	handler->base.execute = hsp_watchdog_handler_execute;

	handler->state = state;
	handler->watchdog = watchdog;
	handler->refresh_period = refresh_period_ms;
	handler->enable_sticky_error = enable_sticky_error;

	return 0;
}

/**
 * Initialize a periodic handler for the HSP watchdog.  The watchdog timer must be started
 * externally to the handler.  The handler execution will only refresh the timer.
 *
 * @param handler The handler to initialize.
 * @param state Variable context for the watchdog handler.
 * @param watchdog The watchdog that will be managed by the handler.
 * @param refresh_period_ms The amount of time between calls to refresh the watchdog.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int hsp_watchdog_handler_init_refresh_only (struct hsp_watchdog_handler *handler,
	struct hsp_watchdog_handler_state *state, const struct hsp_watchdog *watchdog,
	uint32_t refresh_period_ms)
{
	if ((handler == NULL) || (state == NULL) || (watchdog == NULL)) {
		return HSP_WATCHDOG_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (*handler));

	handler->base.prepare = hsp_watchdog_handler_prepare_refresh_only;
	handler->base.get_next_execution = hsp_watchdog_handler_get_next_execution;
	handler->base.execute = hsp_watchdog_handler_execute;

	handler->state = state;
	handler->watchdog = watchdog;
	handler->refresh_period = refresh_period_ms;

	return 0;
}

/**
 * Release the resources used by a handler for an HSP watchdog.  The watchdog timer will not be
 * stopped.
 *
 * @param handler The handler to release.
 */
void hsp_watchdog_handler_release (const struct hsp_watchdog_handler *handler)
{
	UNUSED (handler);
}
