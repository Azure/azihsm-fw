// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "self_test_hsp_rng_hw.h"
#include "common/unused.h"
#include "drivers/kat/hsp_rng_hw_kat.h"
#include "fips/fips_logging.h"


int self_test_hsp_rng_hw_run_self_test (const struct self_test_interface *self_test,
	struct debug_log_entry_info *error_info)
{
	const struct self_test_hsp_rng_hw *hw_test = (const struct self_test_hsp_rng_hw*) self_test;
	int status;

	if ((self_test == NULL) || (error_info == NULL)) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_FIPS,
			FIPS_LOGGING_SELF_TEST_NOT_EXECUTED, HSP_RNG_HW_INVALID_ARGUMENT, 0);

		return HSP_RNG_HW_INVALID_ARGUMENT;
	}

	status = hsp_rng_hw_kat_run_self_test (hw_test->rng);
	if (status != 0) {
		error_info->severity = DEBUG_LOG_SEVERITY_ERROR;
		error_info->component = DEBUG_LOG_COMPONENT_FIPS;
		error_info->msg_index = FIPS_LOGGING_DRBG_KAT_FAILED;
		error_info->arg1 = 0;
		error_info->arg2 = status;
		error_info->format = 1;
	}

	return status;
}

/**
 * Initialize a handler for self-testing the HSP hardware DRBG.
 *
 * @param self_test The self-test handler to initialize.
 * @param rng Driver for the hardware DRBG that will be self-tested.
 *
 * @return 0 if the self-test handler was initialized successfully or an error code.
 */
int self_test_hsp_rng_hw_init (struct self_test_hsp_rng_hw *self_test, const struct hsp_rng_hw *rng)
{
	if ((self_test == NULL) || (rng == NULL)) {
		return HSP_RNG_HW_INVALID_ARGUMENT;
	}

	memset (self_test, 0, sizeof (*self_test));

	self_test->base.run_self_test = self_test_hsp_rng_hw_run_self_test;

	self_test->rng = rng;

	return 0;
}

/**
 * Release the resources used for self-testing the HSP hardware DRBG.
 *
 * @param self_test The self-test handler to release.
 */
void self_test_hsp_rng_hw_release (const struct self_test_hsp_rng_hw *self_test)
{
	UNUSED (self_test);
}
