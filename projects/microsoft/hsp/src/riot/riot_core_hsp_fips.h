// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef RIOT_CORE_HSP_FIPS_H_
#define RIOT_CORE_HSP_FIPS_H_

#include "asn1/base64.h"
#include "asn1/x509.h"
#include "asn1/x509_extension_builder.h"
#include "crypto/ecc_hw.h"
#include "crypto/hash.h"
#include "drivers/ccs_ksu_interface.h"
#include "riot/riot_core_hsp.h"

/**
 * Variable context for a FIPS compliant DICE layer 0 on HSP.
 */
struct riot_core_hsp_fips_state {
	struct riot_core_hsp_state base;	/**< Variable context for the base type. */
};

/**
 * FIPS compliant DICE layer 0 implementation using HSP with keys and CDI stored in CCS.  The CCS
 * implementation must also be FIPS compliant.
 */
struct riot_core_hsp_fips {
	struct riot_core_hsp base;				/**< Base API instance. */
	struct riot_core_hsp_fips_state *state;	/**< Variable context for DICE. */
	const struct ecc_hw *ecc;				/**< Driver for the PKA to use for key verification. */
	const struct hash_engine *hash;			/**< Hash engine to use for key verification. */
};


int riot_core_hsp_fips_init (struct riot_core_hsp_fips *hsp, struct riot_core_hsp_fips_state *state,
	struct ccs_ksu_interface *ccs, const struct ecc_hw *ecc, const struct hash_engine *hash,
	const struct base64_engine *base64, const struct x509_engine *x509, uint8_t cdi_slot,
	uint8_t device_id_slot, uint8_t alias_slot,
	const struct x509_extension_builder *const *device_id_ext, size_t device_id_ext_count,
	uint8_t device_id_pathlen, const struct x509_extension_builder *const *alias_ext,
	size_t alias_ext_count);
int riot_core_hsp_fips_init_alias_ca (struct riot_core_hsp_fips *hsp,
	struct riot_core_hsp_fips_state *state, struct ccs_ksu_interface *ccs, const struct ecc_hw *ecc,
	const struct hash_engine *hash, const struct base64_engine *base64,
	const struct x509_engine *x509, uint8_t cdi_slot, uint8_t device_id_slot, uint8_t alias_slot,
	const struct x509_extension_builder *const *device_id_ext, size_t device_id_ext_count,
	uint8_t device_id_pathlen, const struct x509_extension_builder *const *alias_ext,
	size_t alias_ext_count, uint8_t alias_pathlen);
int riot_core_hsp_fips_init_state (const struct riot_core_hsp_fips *hsp);
int riot_core_hsp_fips_release (const struct riot_core_hsp_fips *hsp);


#endif	/* RIOT_CORE_HSP_FIPS_H_ */
