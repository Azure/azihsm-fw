// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HASH_HS_SHA_H_
#define HASH_HS_SHA_H_

#include "hs_sha_multi_update.h"
#include "crypto/hash.h"
#include "drivers/hs_sha.h"


/**
 * Variable context for hashing with the HS-SHA hardware.
 */
struct hash_engine_hs_sha_state {
	struct hs_sha_multi_update context;	/**< Context for multi-update hash operations. */
};

/**
 * A context for calculating hashes using HS-SHA hardware.
 */
struct hash_engine_hs_sha {
	struct hash_engine base;				/**< Base hash API instance. */
	struct hash_engine_hs_sha_state *state;	/**< Variable state for the API instance. */
	const struct hs_sha *hw;				/**< HW driver for the HS-SHA block. */
};


int hash_hs_sha_init (struct hash_engine_hs_sha *engine, struct hash_engine_hs_sha_state *state,
	const struct hs_sha *hw);
int hash_hs_sha_init_state (const struct hash_engine_hs_sha *engine);
void hash_hs_sha_release (const struct hash_engine_hs_sha *engine);


#endif	/* HASH_HS_SHA_H_ */
