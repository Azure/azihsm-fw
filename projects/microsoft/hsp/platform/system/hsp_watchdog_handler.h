// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_WATCHDOG_HANDLER_H_
#define HSP_WATCHDOG_HANDLER_H_

#include "system/hsp_watchdog.h"
#include "system/periodic_task.h"


/**
 * Variable context for the watchdog handler.
 */
struct hsp_watchdog_handler_state {
	platform_clock next;	/**< Time for the next execution. */
	bool next_valid;		/**< Flag indicating the next execution time is valid. */
};

/**
 * Periodic handler for toggling the DC-SCM heartbeat LED.
 */
struct hsp_watchdog_handler {
	struct periodic_task_handler base;			/**< Base handler API. */
	struct hsp_watchdog_handler_state *state;	/**< Variable context for the handler. */
	const struct hsp_watchdog *watchdog;		/**< Driver for the HSP GPIOs */
	uint32_t refresh_period;					/**< Time between watchdog refresh calls. */
	bool enable_sticky_error;					/**< The watchdog should trigger a sticky error. */
};


int hsp_watchdog_handler_init (struct hsp_watchdog_handler *handler,
	struct hsp_watchdog_handler_state *state, const struct hsp_watchdog *watchdog,
	uint32_t refresh_period_ms, bool enable_sticky_error);
int hsp_watchdog_handler_init_refresh_only (struct hsp_watchdog_handler *handler,
	struct hsp_watchdog_handler_state *state, const struct hsp_watchdog *watchdog,
	uint32_t refresh_period_ms);
void hsp_watchdog_handler_release (const struct hsp_watchdog_handler *handler);


/* This is effectively an extension of the watchdog driver and will use the same error codes. */


#endif	/* HSP_WATCHDOG_HANDLER_H_ */
