// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SELF_TEST_CMVP_FAULT_INJECTION_STATIC_H_
#define SELF_TEST_CMVP_FAULT_INJECTION_STATIC_H_

#include "self_test_cmvp_fault_injection.h"


/* Internal functions declared to allow for static initialization. */
int self_test_cmvp_fault_injection_run_self_test_first (const struct self_test_interface *self_test,
	struct debug_log_entry_info *error_info);
int self_test_cmvp_fault_injection_run_self_test_second (
	const struct self_test_interface *self_test, struct debug_log_entry_info *error_info);


/**
 * Constant initializer for the self-test API for injecting faults into the first group of tests.
 */
#define	SELF_TEST_CMVP_FAULT_INJECTION_FIRST_API_INIT  { \
		.run_self_test = self_test_cmvp_fault_injection_run_self_test_first, \
	}

/**
 * Constant initializer for the self-test API for injecting faults into the second group of tests.
 */
#define	SELF_TEST_CMVP_FAULT_INJECTION_SECOND_API_INIT  { \
		.run_self_test = self_test_cmvp_fault_injection_run_self_test_second, \
	}


/**
 * Initialize a static instance for injecting faults into periodic self-tests to support CMVP
 * testing.  This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param rng_ptr Driver for the hardware DRBG that will be self-tested.
 */
#define	self_test_cmvp_fault_injection_static_init(rng_ptr) { \
		.base_first = SELF_TEST_CMVP_FAULT_INJECTION_FIRST_API_INIT, \
		.base_second = SELF_TEST_CMVP_FAULT_INJECTION_SECOND_API_INIT, \
		.rng = rng_ptr, \
	}


#endif	/* SELF_TEST_CMVP_FAULT_INJECTION_STATIC_H_ */
