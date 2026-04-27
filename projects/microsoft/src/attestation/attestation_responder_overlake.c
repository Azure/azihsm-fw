// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "attestation_responder_overlake.h"


static int attestation_responder_overlake_get_certificate (
	struct attestation_responder *attestation, uint8_t slot_num, uint8_t cert_num,
	struct der_cert *cert)
{
	const struct riot_keys *keys;
	const struct der_cert *int_ca;
	const struct der_cert *aux_cert;
	int status = 0;

	if ((attestation == NULL) || (cert == NULL)) {
		return ATTESTATION_INVALID_ARGUMENT;
	}

	if (slot_num > ATTESTATION_AUX_SLOT_NUM) {
		return ATTESTATION_INVALID_SLOT_NUM;
	}

	int_ca = riot_key_manager_get_intermediate_ca (attestation->riot);
	keys = riot_key_manager_get_riot_keys (attestation->riot);

	memset (cert, 0, sizeof (struct der_cert));

	if (cert_num == 0) {
		if (int_ca == NULL) {
			status = ATTESTATION_CERT_NOT_AVAILABLE;
			goto exit;
		}

		cert->cert = int_ca->cert;
		cert->length = int_ca->length;
	}
	else if (cert_num == 1) {
		if ((keys->devid_cert == NULL) || (keys->devid_cert_length == 0)) {
			status = ATTESTATION_CERT_NOT_AVAILABLE;
			goto exit;
		}

		cert->cert = keys->devid_cert;
		cert->length = keys->devid_cert_length;
	}
	else if (cert_num == 2) {
		if (slot_num == ATTESTATION_RIOT_SLOT_NUM) {
			if ((keys->alias_cert == NULL) || (keys->alias_cert_length == 0)) {
				status = ATTESTATION_CERT_NOT_AVAILABLE;
				goto exit;
			}

			cert->cert = keys->alias_cert;
			cert->length = keys->alias_cert_length;
		}
		else if (slot_num == ATTESTATION_AUX_SLOT_NUM) {
			aux_cert = aux_attestation_get_certificate (attestation->aux);
			if (aux_cert == NULL) {
				status = ATTESTATION_CERT_NOT_AVAILABLE;
				goto exit;
			}

			cert->cert = aux_cert->cert;
			cert->length = aux_cert->length;
		}
	}
	else {
		status = ATTESTATION_INVALID_CERT_NUM;
	}

exit:
	riot_key_manager_release_riot_keys (attestation->riot, keys);

	return status;
}

/**
 * Initialize an attestation handler for Overlake fTPM requests.
 *
 * @param attestation The attestation handler to initialize.
 * @param riot Manager for RIoT keys.
 * @param hash Hash engine to use.
 * @param ecc ECC engine to use.
 * @param rng Random number generator to use.
 * @param store Manager for system PCRs.
 * @param aux Handler for auxiliary attestation requests.
 * @param min_protocol_version Minimum protocol version supported by the device.
 * @param max_protocol_version Maximum protocol version supported by the device.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int attestation_responder_overlake_init (struct attestation_responder_overlake *attestation,
	const struct riot_key_manager *riot, const struct hash_engine *hash,
	const struct ecc_engine *ecc, const struct rng_engine *rng, struct pcr_store *store,
	struct aux_attestation *aux, uint16_t min_protocol_version, uint16_t max_protocol_version)
{
	int status;

	if (attestation == NULL) {
		return ATTESTATION_INVALID_ARGUMENT;
	}

	status = attestation_responder_init (&attestation->base, riot, hash, ecc, rng, store, aux,
		min_protocol_version, max_protocol_version);
	if (status == 0) {
		attestation->base.get_certificate = attestation_responder_overlake_get_certificate;
	}

	return status;
}

/**
 * Release an Overlake fTPM attestation handler.
 *
 * @param attestation The handler to release.
 *
 */
void attestation_responder_overlake_release (struct attestation_responder_overlake *attestation)
{
	attestation_responder_release (&attestation->base);
}
