// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "intrusion_manager_msft.h"
#include "common/type_cast.h"
#include "intrusion/intrusion_logging.h"
#include "msft_protocol/rot_commands.h"


/**
 * Send an intrusion event notification via the MCTP notifier.
 *
 * @param manager_msft The intrusion manager msft handler.
 * @param event The intrusion event.
 */
static void intrusion_manager_msft_send_notification (struct intrusion_manager_msft *manager_msft,
	uint8_t event)
{
	uint8_t payload[sizeof (struct rot_intrusion_event_request)];
	size_t payload_len = sizeof (struct rot_intrusion_event_request);
	int status;

	rot_build_intrusion_event_request (event, payload, payload_len);

	status = manager_msft->mctp_notifier->send_notification_request (manager_msft->mctp_notifier,
		payload, payload_len);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INTRUSION,
			INTRUSION_LOGGING_INTRUSION_NOTIFICATION_FAILED, status, 0);
	}
}

static int intrusion_manager_msft_handle_intrusion (struct intrusion_manager *manager)
{
	/* TODO: manager_msft is required to cast to const pointer, and
	 * intrusion_manager and intrusion_state should accept const instances as arguments */
	struct intrusion_manager_msft *manager_msft = TO_DERIVED_TYPE (manager,
		struct intrusion_manager_msft, base);
	int status;

	if (manager == NULL) {
		return INTRUSION_MANAGER_INVALID_ARGUMENT;
	}

	status = intrusion_manager_handle_intrusion (&manager_msft->base);

	if (status == 0) {
		intrusion_manager_msft_send_notification (manager_msft, ROT_INTRUSION_DETECTED);
	}

	return status;
}

static int intrusion_manager_msft_reset_intrusion (struct intrusion_manager *manager)
{
	struct intrusion_manager_msft *manager_msft = TO_DERIVED_TYPE (manager,
		struct intrusion_manager_msft, base);
	int status;

	if (manager == NULL) {
		return INTRUSION_MANAGER_INVALID_ARGUMENT;
	}

	status = intrusion_manager_reset_intrusion (&manager_msft->base);

	if (status == 0) {
		intrusion_manager_msft_send_notification (manager_msft, ROT_INTRUSION_STATE_RESET);
	}

	return status;
}

/**
 * Initialize a manager for handling intrusion event and notifying
 * the registered intrusion event listeners.
 *
 * @param manager The msft intrusion manager to initialize.
 * @param state The handler for persisting intrusion state.
 * @param hash Hash engine to use for PCR updates.
 * @param pcr The PCR manager that will be used to report intrusion state.
 * @param measurement The measurement ID for the intrusion state.
 * @param mctp_notifier The instance of MCTP notifier.
 *
 * @return 0 if the intrusion manager was successfully initialized or an error code.
 */
int intrusion_manager_msft_init (struct intrusion_manager_msft *manager,
	const struct intrusion_state *state, const struct hash_engine *hash, struct pcr_store *pcr,
	uint16_t measurement, const struct mctp_notifier_interface *mctp_notifier)
{
	int status;

	if ((manager == NULL) || (state == NULL) || (hash == NULL) || (pcr == NULL) ||
		(mctp_notifier == NULL)) {
		return INTRUSION_MANAGER_INVALID_ARGUMENT;
	}

	status = intrusion_manager_init (&manager->base, (struct intrusion_state*) state, hash, pcr,
		measurement);

	if (status == 0) {
		manager->base.handle_intrusion = intrusion_manager_msft_handle_intrusion;
		manager->base.reset_intrusion = intrusion_manager_msft_reset_intrusion;
		manager->mctp_notifier = mctp_notifier;
	}

	return status;
}

/**
 * Release the resources used by an intrusion manager.
 *
 * @param manager The intrusion manager to release.
 */
void intrusion_manager_msft_release (struct intrusion_manager_msft *manager)
{
	intrusion_manager_release (&manager->base);
}
