// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SELF_TEST_HSP_RNG_HW_STATIC_H_
#define SELF_TEST_HSP_RNG_HW_STATIC_H_

#include "self_test_hsp_rng_hw.h"


/* Internal functions declared to allow for static initialization. */
int self_test_hsp_rng_hw_run_self_test (const struct self_test_interface *self_test,
	struct debug_log_entry_info *error_info);


/**
 * Constant initializer for the self-test API.
 */
#define	SELF_TEST_HSP_RNG_HW_API_INIT  { \
		.run_self_test = self_test_hsp_rng_hw_run_self_test, \
	}


/**
 * Initialize a static instance for self-testing the HSP hardware DRBG.  This can be a constant
 * instance.
 *
 * There is no validation done on the arguments.
 *
 * @param rng_ptr Driver for the hardware DRBG that will be self-tested.
 */
#define	self_test_hsp_rng_hw_static_init(rng_ptr)	{ \
		.base = SELF_TEST_HSP_RNG_HW_API_INIT, \
		.rng = rng_ptr, \
	}


#endif	/* SELF_TEST_HSP_RNG_HW_STATIC_H_ */
