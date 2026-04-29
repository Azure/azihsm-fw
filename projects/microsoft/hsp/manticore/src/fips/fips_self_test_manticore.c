// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <string.h>
#include "fips_self_test_manticore.h"
#include "common/unused.h"
#include "logging/manticore_logging.h"


/**
 * Flag in the on-demand execution ID that indicates when the self-test has completed.
 */
#define	FIPS_SELF_TEST_MANTICORE_DONE_FLAG		(1U << 31)


int fips_self_test_manticore_execute_on_demand_self_test (
	const struct fips_self_test_interface *fips, uint32_t *execution_id, uint16_t *execution_time)
{
	const struct fips_self_test_manticore *manticore =
		(const struct fips_self_test_manticore*) fips;
	uint32_t id;
	int status;
	int id_retry = 16;

	if ((fips == NULL) || (execution_id == NULL) || (execution_time == NULL)) {
		return FIPS_SELF_TEST_INVALID_ARGUMENT;
	}

	/* This operation does not check to see if there is any active self-test in progress already.
	 * This situation is highly unlikely (if not impossible) for two reasons:
	 * 1. The device will only be responding to commands if all self-tests have passed.
	 * 2. Any additional commands that come after the first request for a self-test will fail when
	 * requesting a reboot due to the task being busy.
	 *
	 * The only realistic scenario is getting another self-test request when the previous result
	 * has not been cleared.  In this case, we want to trigger another self-test.  We should not
	 * block this operation if the external requester does not clean up after the last call. */

	do {
		status = manticore->rng->generate_random_buffer (manticore->rng, sizeof (*execution_id),
			(uint8_t*) &id);
		if (status != 0) {
			return status;
		}

		/* Make sure the flag used to track self-test execution is cleared. */
		id &= ~FIPS_SELF_TEST_MANTICORE_DONE_FLAG;

		/* Do not allow the random ID to be the same as the value used to indicate no scheduled
		 * self-test execution. */
	} while ((id == FIPS_SELF_TEST_MANTICORE_NO_SELF_TEST) && (--id_retry > 0));

	if (id == FIPS_SELF_TEST_MANTICORE_NO_SELF_TEST) {
		return FIPS_SELF_TEST_EXE_ON_DEMAND_FAILED;
	}

	status = manticore->background->reboot_device (manticore->background);
	if (status != 0) {
		return status;
	}

	debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
		MANTICORE_LOGGING_EXECUTE_ON_DEMAND_SELF_TESTS, id, 0);

	*manticore->execution_id = id;

	*execution_id = id;
	*execution_time = FIPS_SELF_TEST_MANTICORE_EXECUTION_TIME;

	return 0;
}

int fips_self_test_manticore_get_on_demand_self_test_result (
	const struct fips_self_test_interface *fips, uint32_t execution_id, uint32_t *result)
{
	const struct fips_self_test_manticore *manticore =
		(const struct fips_self_test_manticore*) fips;

	if ((fips == NULL) || (result == NULL)) {
		return FIPS_SELF_TEST_INVALID_ARGUMENT;
	}

	if ((*manticore->execution_id == FIPS_SELF_TEST_MANTICORE_NO_SELF_TEST) ||
		((*manticore->execution_id & ~FIPS_SELF_TEST_MANTICORE_DONE_FLAG) != execution_id)) {
		return FIPS_SELF_TEST_UNKNOWN_ID;
	}

	if (!(*manticore->execution_id & FIPS_SELF_TEST_MANTICORE_DONE_FLAG)) {
		return FIPS_SELF_TEST_RESULT_NOT_READY;
	}

	/* It's not possible for the self-test to report as completed and have failed.  Any self-test
	 * execution failure would trigger automatic device resets.  Initialization would only fully
	 * complete if all self-tests are successful. */
	*result = 0;

	return 0;
}

int fips_self_test_manticore_clear_on_demand_self_test_result (
	const struct fips_self_test_interface *fips, uint32_t execution_id)
{
	const struct fips_self_test_manticore *manticore =
		(const struct fips_self_test_manticore*) fips;

	if (fips == NULL) {
		return FIPS_SELF_TEST_INVALID_ARGUMENT;
	}

	if ((*manticore->execution_id == FIPS_SELF_TEST_MANTICORE_NO_SELF_TEST) ||
		((*manticore->execution_id & ~FIPS_SELF_TEST_MANTICORE_DONE_FLAG) != execution_id)) {
		return FIPS_SELF_TEST_UNKNOWN_ID;
	}

	if (!(*manticore->execution_id & FIPS_SELF_TEST_MANTICORE_DONE_FLAG)) {
		return FIPS_SELF_TEST_RESULT_NOT_READY;
	}

	*manticore->execution_id = FIPS_SELF_TEST_MANTICORE_NO_SELF_TEST;

	return 0;
}

/**
 * Initialize a handler for Manticore FIPS self-tests.
 *
 * @param fips The FIPS self-test handler to initialize.
 * @param rng RNG to use for generating on-demand self-test execution IDs.
 * @param background The background command handler that can trigger device resets.
 * @param execution_id Non-volatile storage for the on-demand self-test execution ID.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int fips_self_test_manticore_init (struct fips_self_test_manticore *fips,
	const struct rng_engine *rng, const struct cmd_background *background,
	volatile uint32_t *execution_id)
{
	if ((fips == NULL) || (rng == NULL) || (background == NULL) || (execution_id == NULL)) {
		return FIPS_SELF_TEST_INVALID_ARGUMENT;
	}

	memset (fips, 0, sizeof (*fips));

	fips->base.execute_on_demand_self_test = fips_self_test_manticore_execute_on_demand_self_test;
	fips->base.get_on_demand_self_test_result =
		fips_self_test_manticore_get_on_demand_self_test_result;
	fips->base.clear_on_demand_self_test_result =
		fips_self_test_manticore_clear_on_demand_self_test_result;

	fips->rng = rng;
	fips->background = background;
	fips->execution_id = execution_id;

	return 0;
}

/**
 * Release the resources used for Manticore FIPS self-tests.
 *
 * @param fips The FIPS self-test handler to release.
 */
void fips_self_test_manticore_release (const struct fips_self_test_manticore *fips)
{
	UNUSED (fips);
}

/**
 * Mark an existing on-demand self-test as having finished execution.  The execution ID will remain
 * valid until cleared by the user, but additional self-tests will not be triggered.
 *
 * If there is no active self-test or the argument is null, this call will do nothing.
 *
 * @param fips The FIPS self-test handler to use for the update.
 */
void fips_self_test_manticore_on_demand_done (const struct fips_self_test_manticore *fips)
{
	if ((fips != NULL) && (*fips->execution_id != FIPS_SELF_TEST_MANTICORE_NO_SELF_TEST)) {
		*fips->execution_id |= FIPS_SELF_TEST_MANTICORE_DONE_FLAG;
	}
}

/**
 * Check the FIPS on-demand execution ID to determine if there is currently an on-demand self-test
 * being executed.
 *
 * This operates directly on the execution ID state tracking rather than using a full instance so
 * that it can be easily used in 1SP without requiring additional dependencies.
 *
 * @param execution_id The execution ID for on-demand self-test executions.
 *
 * @return true if there is an on-demand self-test executing.
 */
bool fips_self_test_manticore_is_on_demand_test_active (volatile uint32_t *execution_id)
{
	return ((execution_id != NULL) && (*execution_id != FIPS_SELF_TEST_MANTICORE_NO_SELF_TEST) &&
		!(*execution_id & FIPS_SELF_TEST_MANTICORE_DONE_FLAG));
}
