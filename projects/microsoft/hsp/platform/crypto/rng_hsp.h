// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef RNG_HSP_H_
#define RNG_HSP_H_

#include "crypto/rng.h"
#include "drivers/hsp_rng_hw.h"


/**
 * Random number generator using HSP hardware.
 */
struct rng_engine_hsp {
	struct rng_engine base;			/**< Base RNG API instance. */
	const struct hsp_rng_hw *hw;	/**< Interface to the RNG hardware. */
};


int rng_hsp_init (struct rng_engine_hsp *rng, const struct hsp_rng_hw *hw);
void rng_hsp_release (const struct rng_engine_hsp *rng);


#endif	/* RNG_HSP_H_ */
