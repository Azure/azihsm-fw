// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common/unused.h"
#include "crypto/rng_hsp.h"


int rng_hsp_generate_random_buffer (const struct rng_engine *engine, size_t rand_len, uint8_t *buf)
{
	const struct rng_engine_hsp *hsp = (const struct rng_engine_hsp*) engine;

	if ((hsp == NULL) || (buf == NULL)) {
		return RNG_ENGINE_INVALID_ARGUMENT;
	}

	return hsp_rng_hw_get_random_buffer (hsp->hw, buf, rand_len);
}

/**
 * Initialize an interface to the HSP HW random number generator.
 *
 * @param rng The RNG instance to initialize.
 * @param hw Interface to the RNG hardware to use.
 *
 * @return 0 if the RNG was successfully initialized or an error code.
 */
int rng_hsp_init (struct rng_engine_hsp *rng, const struct hsp_rng_hw *hw)
{
	if ((rng == NULL) || (hw == NULL)) {
		return RNG_ENGINE_INVALID_ARGUMENT;
	}

	memset (rng, 0, sizeof (struct rng_engine_hsp));

	rng->base.generate_random_buffer = rng_hsp_generate_random_buffer;

	rng->hw = hw;

	return 0;
}

/**
 * Release the resources used for an HSP RNG instance.
 *
 * @param rng The RNG instance to release.
 */
void rng_hsp_release (const struct rng_engine_hsp *rng)
{
	UNUSED (rng);
}
