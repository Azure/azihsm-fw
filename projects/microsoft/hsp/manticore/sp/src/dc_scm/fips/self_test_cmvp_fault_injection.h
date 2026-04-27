// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SELF_TEST_CMVP_FAULT_INJECTION_H_
#define SELF_TEST_CMVP_FAULT_INJECTION_H_

#include "fips/self_test_interface.h"


/**
 * Support CMVP testing by providing a mechanism to inject faults into the periodic self-tests.
 */
struct self_test_cmvp_fault_injection {
	struct self_test_interface base_first;	/**< Base API for injecting faults into the first group of self-tests. */
	struct self_test_interface base_second;	/**< Base API for injecting faults into the second group of self-tests. */
	const struct hsp_rng_hw *rng;			/**< Driver for the HSP hardware DRBG. */
};


int self_test_cmvp_fault_injection_init (struct self_test_cmvp_fault_injection *cmvp,
	const struct hsp_rng_hw *rng);
void self_test_cmvp_fault_injection_release (const struct self_test_cmvp_fault_injection *cmvp);


#endif	/* SELF_TEST_CMVP_FAULT_INJECTION_H_ */
