// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef RNG_HSP_STATIC_H_
#define RNG_HSP_STATIC_H_

#include "crypto/rng_hsp.h"


/* Internal functions declared to allow for static initialization. */
int rng_hsp_generate_random_buffer (const struct rng_engine *engine, size_t rand_len, uint8_t *buf);


/**
 * Constant initializer for the HSP RNG API.
 */
#define	RNG_HSP_API_INIT  { \
		.generate_random_buffer = rng_hsp_generate_random_buffer, \
	}


/**
 * Initialize a static instance of an RNG driver.
 *
 * There is no validation done on the arguments.
 *
 * @param hw_ptr Base address of the hardware registers.
 */
#define	rng_hsp_static_init(hw_ptr)	{ \
		.base = RNG_HSP_API_INIT, \
		.hw = hw_ptr, \
	}


#endif	/* RNG_HSP_STATIC_H_ */
