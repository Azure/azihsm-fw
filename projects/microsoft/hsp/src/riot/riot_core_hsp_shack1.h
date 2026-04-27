// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef RIOT_CORE_HSP_SHACK1_H_
#define RIOT_CORE_HSP_SHACK1_H_

#include "riot_core_hsp.h"
#include "crypto/hash.h"


/**
 * Derived type of HSP DICE layer 0 implementation for use with SHACK1 HSP.
 */
struct riot_core_hsp_shack1 {
	struct riot_core_hsp riot;		/**< Base HSP RIoT core instance. */
	const struct hash_engine *hash;	/**< Hash engine to use for DICE. */
};


int riot_core_hsp_shack1_init (struct riot_core_hsp_shack1 *hsp, struct riot_core_hsp_state *state,
	struct ccs_ksu_interface *ccs, const struct base64_engine *base64,
	const struct x509_engine *x509, const struct hash_engine *hash, uint8_t cdi_slot,
	uint8_t device_id_slot, uint8_t alias_slot,
	const struct x509_extension_builder *const *device_id_ext, size_t device_id_ext_count,
	const struct x509_extension_builder *const *alias_ext, size_t alias_ext_count);
int riot_core_hsp_shack1_init_state (const struct riot_core_hsp_shack1 *hsp);
int riot_core_hsp_shack1_release (const struct riot_core_hsp_shack1 *hsp);


#endif	/* RIOT_CORE_HSP_SHACK1_H_ */
