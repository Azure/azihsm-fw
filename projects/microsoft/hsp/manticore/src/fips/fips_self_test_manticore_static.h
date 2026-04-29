// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FIPS_SELF_TEST_MANTICORE_STATIC_H_
#define FIPS_SELF_TEST_MANTICORE_STATIC_H_

#include "fips_self_test_manticore.h"


/* Internal functions declared to allow for static initialization. */
int fips_self_test_manticore_execute_on_demand_self_test (
	const struct fips_self_test_interface *fips, uint32_t *execution_id, uint16_t *execution_time);
int fips_self_test_manticore_get_on_demand_self_test_result (
	const struct fips_self_test_interface *fips, uint32_t execution_id, uint32_t *result);
int fips_self_test_manticore_clear_on_demand_self_test_result (
	const struct fips_self_test_interface *fips, uint32_t execution_id);


/**
 * Constant initializer for the FIPS self-test base API.
 */
#define	FIPS_SELF_TEST_MANTICORE_API_INIT  { \
    .execute_on_demand_self_test = fips_self_test_manticore_execute_on_demand_self_test, \
	.get_on_demand_self_test_result = fips_self_test_manticore_get_on_demand_self_test_result, \
	.clear_on_demand_self_test_result = fips_self_test_manticore_clear_on_demand_self_test_result, \
}


/**
 * Initialize a static handler for FIPS self-tests on Manticore.
 *
 * There is no validation done on the arguments.
 *
 * @param rng_ptr RNG to use for generating on-demand self-test execution IDs.
 * @param background_ptr The background command handler that can trigger device resets.
 * @param execution_id_ptr Non-volatile storage for the on-demand self-test execution ID.
 */
#define	fips_self_test_manticore_static_init(rng_ptr, background_ptr, execution_id_ptr)	{ \
		.base = FIPS_SELF_TEST_MANTICORE_API_INIT, \
		.rng = rng_ptr, \
		.background = background_ptr, \
		.execution_id = execution_id_ptr, \
	}


#endif	/* FIPS_SELF_TEST_MANTICORE_STATIC_H_ */
