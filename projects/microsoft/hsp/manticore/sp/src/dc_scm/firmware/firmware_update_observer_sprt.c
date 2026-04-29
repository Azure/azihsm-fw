// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "firmware_update_observer_sprt.h"
#include "sp_boot.h"
#include "common/unused.h"
#include "firmware/firmware_update.h"
#include "logging/manticore_logging.h"


void firmware_update_observer_sprt_on_update_applied (
	const struct firmware_update_observer *observer)
{
	const struct firmware_update_observer_sprt *sprt =
		(const struct firmware_update_observer_sprt*) observer;
	int status;

	status = sprt->impactful->is_update_not_impactful (sprt->impactful);
	if (status == 0) {
		impactless_update_applied ();

		debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_ALLOW_UPDATE_LOAD_DURING_RESET, 0, 0);
	}
	else {
		impactful_update_applied ();

		debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_BLOCK_UPDATE_LOAD_UNTIL_POR, status, 0);
	}
}

/**
 * Initialize a firmware update observer for SPRT impactless update state tracking.
 *
 * @param observer The observer to initialize.
 * @param impactful Interface to use for checking whether the update was impactful.
 *
 * @return 0 if the observer was initialized successfully or an error code.
 */
int firmware_update_observer_sprt_init (struct firmware_update_observer_sprt *observer,
	const struct impactful_update_interface *impactful)
{
	if ((observer == NULL) || (impactful == NULL)) {
		return FIRMWARE_UPDATE_INVALID_ARGUMENT;
	}

	memset (observer, 0, sizeof (*observer));

	observer->base.on_update_applied = firmware_update_observer_sprt_on_update_applied;

	observer->impactful = impactful;

	return 0;
}

/**
 * Release the resources used by an SPRT update state observer.
 *
 * @param observer The observer to release.
 */
void firmware_update_observer_sprt_release (const struct firmware_update_observer_sprt *observer)
{
	UNUSED (observer);
}
