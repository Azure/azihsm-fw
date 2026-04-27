// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef ATTESTATION_RESPONDER_OVERLAKE_H_
#define ATTESTATION_RESPONDER_OVERLAKE_H_

#include "attestation/attestation_responder.h"


/**
 * Attestation responder interface for use with the fTMP handler.
 */
struct attestation_responder_overlake {
	struct attestation_responder base;	/**< Base attestation responder instance. */
};


int attestation_responder_overlake_init (struct attestation_responder_overlake *attestation,
	const struct riot_key_manager *riot, const struct hash_engine *hash,
	const struct ecc_engine *ecc, const struct rng_engine *rng, struct pcr_store *store,
	struct aux_attestation *aux, uint16_t min_protocol_version, uint16_t max_protocol_version);
void attestation_responder_overlake_release (struct attestation_responder_overlake *attestation);


#endif	/* ATTESTATION_RESPONDER_OVERLAKE_H_ */
