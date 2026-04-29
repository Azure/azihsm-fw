// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SELF_TEST_HSP_RNG_HW_H_
#define SELF_TEST_HSP_RNG_HW_H_

#include "drivers/hsp_rng_hw.h"
#include "fips/self_test_interface.h"


/**
 * Self-test for the HSP hardware DRBG.
 */
struct self_test_hsp_rng_hw {
	struct self_test_interface base;	/**< Base self-test API. */
	const struct hsp_rng_hw *rng;		/**< Driver for the HSP hardware DRBG. */
};


int self_test_hsp_rng_hw_init (struct self_test_hsp_rng_hw *self_test,
	const struct hsp_rng_hw *rng);
void self_test_hsp_rng_hw_release (const struct self_test_hsp_rng_hw *self_test);


#endif	/* SELF_TEST_HSP_RNG_HW_H_ */
