// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HASH_HS_SHA_SOC_H_
#define HASH_HS_SHA_SOC_H_

#include "hash_hs_sha.h"
#include "crypto/hash.h"
#include "drivers/hsp_dmb.h"

/**
 * A component for calculating hashes using HS-SHA hardware, while hash operation state data
 * is kept in SOC memory (outside HSP).
 */
struct hash_engine_hs_sha_soc {
	struct hash_engine base;	/**< Base hash API instance. */
	uint64_t soc_state_address;	/**< Address of the SOC stored state. */
	const struct hsp_dmb *dmb;	/**< DMB driver for accessing the SOC memory. */
	const struct hs_sha *hw;	/**< HW driver for the HS-SHA block. */
};


int hash_hs_sha_soc_init (struct hash_engine_hs_sha_soc *engine, uint64_t soc_state_address,
	const struct hsp_dmb *dmb, const struct hs_sha *hw);
int hash_hs_sha_soc_init_state (const struct hash_engine_hs_sha_soc *engine);
int hash_hs_sha_soc_release (const struct hash_engine_hs_sha_soc *engine);


#endif	/* HASH_HS_SHA_SOC_H_ */
