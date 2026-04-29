// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FIPS_SELF_TEST_INTERFACE_H_
#define FIPS_SELF_TEST_INTERFACE_H_

#include <stdint.h>
#include "status/msft_module_id.h"


/**
 * Defines the interface to execute self-tests necessary to satisfy FIPS requirements.
 */
struct fips_self_test_interface {
	/**
	 * Schedule execution of on-demand self-tests of all cryptographic implementations being used.
	 * The self-test will complete asynchronously from this call.
	 *
	 * @param fips The self-test handler to use for test execution.
	 * @param execution_id Output for an identifier assigned to the test execution.  It's not
	 * required that multiple test executions be able to run in parallel, but the execution must be
	 * mapped to a unique identifier.
	 * @param execution_time Output for the expected amount of time needed to complete the test.
	 * After this time expires it's possible the test may not be done, but the module needs to be
	 * able to respond to requests for the result.
	 *
	 * @return 0 if the self-test execution was scheduled successfully or an error code.
	 */
	int (*execute_on_demand_self_test) (const struct fips_self_test_interface *fips,
		uint32_t *execution_id, uint16_t *execution_time);

	/**
	 * Get the result of a previously scheduled on-demand self-test.
	 *
	 * @param fips The self-test handler to query for the result.
	 * @param execution_id The identifier for the self-test execution being requested.
	 * @param result Output for self-test execution result.
	 *
	 * @param 0 if the self-test result was retrieved successfully or an error code.
	 * FIPS_SELF_TEST_UNKNOWN_ID will be returned if the ID does not match any current self-test
	 * execution.  FIPS_SELF_TEST_RESULT_NOT_READY will be returned when the test execution has not
	 * yet completed.
	 */
	int (*get_on_demand_self_test_result) (const struct fips_self_test_interface *fips,
		uint32_t execution_id, uint32_t *result);

	/**
	 * Clear the result of a completed on-demand self-test.  Once the result is cleared, the
	 * execution context will be freed and the identifier will not map to any self-test execution.
	 * Subsequent requests using the same execution ID will return an error.
	 *
	 * The result can only be cleared after the self-test has finished executing.
	 *
	 * @param fips The self-test handler to update.
	 * @param execution_id The identifier for the self-test execution to clear.
	 *
	 * @return 0 if the self-test result was cleared successfully or an error code.
	 */
	int (*clear_on_demand_self_test_result) (const struct fips_self_test_interface *fips,
		uint32_t execution_id);
};


#define	FIPS_SELF_TEST_ERROR(code)		ROT_ERROR (MSFT_MODULE_FIPS_SELF_TEST, code)

/**
 * Error codes that can be generated during CMVP test handling.
 */
enum {
	FIPS_SELF_TEST_INVALID_ARGUMENT = FIPS_SELF_TEST_ERROR (0x00),		/**< Input parameter is null or not valid. */
	FIPS_SELF_TEST_NO_MEMORY = FIPS_SELF_TEST_ERROR (0x01),				/**< Memory allocation failed. */
	FIPS_SELF_TEST_EXE_ON_DEMAND_FAILED = FIPS_SELF_TEST_ERROR (0x02),	/**< Failed to schedule on-demand self-tests. */
	FIPS_SELF_TEST_GET_ON_DEMAND_FAILED = FIPS_SELF_TEST_ERROR (0x03),	/**< Failed to get the on-demand self-test result. */
	FIPS_SELF_TEST_CLR_ON_DEMAND_FAILED = FIPS_SELF_TEST_ERROR (0x04),	/**< Failed to clear the on-demand self-test result. */
	FIPS_SELF_TEST_UNKNOWN_ID = FIPS_SELF_TEST_ERROR (0x05),			/**< The execution ID does not match any active self-test. */
	FIPS_SELF_TEST_RESULT_NOT_READY = FIPS_SELF_TEST_ERROR (0x06),		/**< The self-test is still in progress. */
};


#endif	/* FIPS_SELF_TEST_INTERFACE_H_ */
