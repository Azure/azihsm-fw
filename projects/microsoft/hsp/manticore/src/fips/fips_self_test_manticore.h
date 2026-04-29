// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FIPS_SELF_TEST_MANTICORE_H_
#define FIPS_SELF_TEST_MANTICORE_H_

#include "cmd_interface/cmd_background.h"
#include "crypto/rng.h"
#include "fips/fips_self_test_interface.h"


/**
 * The amount of time to report for an on-demand self-test execution.  These self-tests are executed
 * via a graceful reset of the device, which should only take approximately 1 second to complete.
 * This value provides some buffer on top of this in case there is any failure that triggers
 * additional device resets.
 *
 * This value also accounts for the 5 second wait that exists before any device reset.
 */
#define	FIPS_SELF_TEST_MANTICORE_EXECUTION_TIME		((5 + 3) * 1000)

/**
 * Value that will be stored as the execution ID when no self-test has been schedule to execute.
 */
#define	FIPS_SELF_TEST_MANTICORE_NO_SELF_TEST		0


/**
 * Manticore implementation for FIPS self-test execution.
 */
struct fips_self_test_manticore {
	struct fips_self_test_interface base;		/**< Base self-test API. */
	const struct rng_engine *rng;				/**< RNG for create self-test execution IDs. */
	const struct cmd_background *background;	/**< Background command handler for device resets. */
	volatile uint32_t *execution_id;			/**< Execution ID for on-demand self-tests. */
};


int fips_self_test_manticore_init (struct fips_self_test_manticore *fips,
	const struct rng_engine *rng, const struct cmd_background *background,
	volatile uint32_t *execution_id);
void fips_self_test_manticore_release (const struct fips_self_test_manticore *fips);

void fips_self_test_manticore_on_demand_done (const struct fips_self_test_manticore *fips);
bool fips_self_test_manticore_is_on_demand_test_active (volatile uint32_t *execution_id);


#endif	/* FIPS_SELF_TEST_MANTICORE_H_ */
