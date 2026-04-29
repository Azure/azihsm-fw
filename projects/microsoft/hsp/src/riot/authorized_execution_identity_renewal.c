// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "authorized_execution_identity_renewal.h"
#include "common/common_math.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "riot/riot_logging.h"


/**
 * Execute identity renewal for either device key.
 *
 * @param renewal The execution context to use for identity renewal.
 * @param msg_success Log message to generate when renewal is successful.
 * @param msg_fail Log message to generate when renewal fails.
 * @param get Function to get the current renewal counter.
 * @param set Function to set the new renewal counter.
 *
 * @return 0 if the identity was successfully renewed or an error code.
 */
static int authorized_execution_identity_renewal_execute_common (
	const struct authorized_execution_identity_renewal *renewal, uint8_t msg_success,
	uint8_t msg_fail, int (*get) (const struct identity_renewal*, uint32_t*),
	int (*set) (const struct identity_renewal*, uint32_t))
{
	uint32_t renew_counter;
	int status;

	status = get (renewal->identity, &renew_counter);
	if (status != 0) {
		goto exit;
	}

	status = common_math_set_next_bit_in_array ((uint8_t*) &renew_counter, sizeof (renew_counter));
	if (status != 0) {
		/* This will only fail if the counter cannot be incremented. */
		status = IDENTITY_RENEWAL_NOT_POSSIBLE;
		goto exit;
	}

	status = set (renewal->identity, renew_counter);

exit:
	if (status == 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_RIOT, msg_success,
			renew_counter, 0);
	}
	else {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_RIOT, msg_fail,
			status, 0);
	}

	return status;
}

int authorized_execution_identity_renewal_execute_dme (const struct authorized_execution *execution,
	const uint8_t *data, size_t length, bool *reset_req)
{
	const struct authorized_execution_identity_renewal *renewal = TO_DERIVED_TYPE (execution,
		const struct authorized_execution_identity_renewal, base);

	UNUSED (data);
	UNUSED (length);
	UNUSED (reset_req);

	if (execution == NULL) {
		return AUTHORIZED_EXECUTION_INVALID_ARGUMENT;
	}

	return authorized_execution_identity_renewal_execute_common (renewal, RIOT_LOGGING_DME_REVOKED,
		RIOT_LOGGING_DME_REVOCATION_FAILED, renewal->identity->get_dme_renewal,
		renewal->identity->set_dme_renewal);
}

int authorized_execution_identity_renewal_execute_dice (
	const struct authorized_execution *execution, const uint8_t *data, size_t length,
	bool *reset_req)
{
	const struct authorized_execution_identity_renewal *renewal = TO_DERIVED_TYPE (execution,
		const struct authorized_execution_identity_renewal, base);

	UNUSED (data);
	UNUSED (length);
	UNUSED (reset_req);

	if (execution == NULL) {
		return AUTHORIZED_EXECUTION_INVALID_ARGUMENT;
	}

	return authorized_execution_identity_renewal_execute_common (renewal, RIOT_LOGGING_DICE_REVOKED,
		RIOT_LOGGING_DICE_REVOCATION_FAILED, renewal->identity->get_dice_renewal,
		renewal->identity->set_dice_renewal);
}

/**
 * Initialize an authorized execution context to renew the device's DME key.
 *
 * @param execution The execution context to initialize.
 * @param identity Interface to the hardware management of the DME renewal counter.
 *
 * @return 0 if the execution context was initialized successfully or an error code.
 */
int authorized_execution_identity_renewal_init_dme (
	struct authorized_execution_identity_renewal *execution,
	const struct identity_renewal *identity)
{
	if ((execution == NULL) || (identity == NULL)) {
		return AUTHORIZED_EXECUTION_INVALID_ARGUMENT;
	}

	memset (execution, 0, sizeof (*execution));

	execution->base.execute = authorized_execution_identity_renewal_execute_dme;
	execution->base.validate_data = authorized_execution_validate_data;
	execution->base.get_status_identifiers = authorized_execution_get_status_identifiers;

	execution->identity = identity;

	return 0;
}

/**
 * Initialize an authorized execution context to renew the DICE identity key.
 *
 * @param execution The execution context to initialize.
 * @param identity Interface to the hardware management of the DICE renewal counter.
 *
 * @return 0 if the execution context was initialized successfully or an error code.
 */
int authorized_execution_identity_renewal_init_dice (
	struct authorized_execution_identity_renewal *execution,
	const struct identity_renewal *identity)
{
	if ((execution == NULL) || (identity == NULL)) {
		return AUTHORIZED_EXECUTION_INVALID_ARGUMENT;
	}

	memset (execution, 0, sizeof (*execution));

	execution->base.execute = authorized_execution_identity_renewal_execute_dice;
	execution->base.validate_data = authorized_execution_validate_data;
	execution->base.get_status_identifiers = authorized_execution_get_status_identifiers;

	execution->identity = identity;

	return 0;
}

/**
 * Release the resources used to execute requests to renew device identity.
 *
 * @param execution The execution context to release.
 */
void authorized_execution_identity_renewal_release (
	const struct authorized_execution_identity_renewal *execution)
{
	UNUSED (execution);
}
