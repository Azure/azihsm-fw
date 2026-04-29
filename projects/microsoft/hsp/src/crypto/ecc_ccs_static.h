// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef ECC_CCS_STATIC_H_
#define ECC_CCS_STATIC_H_

#include "crypto/ecc_ccs.h"


/* Internal functions declared to allow for static initialization. */
int ecc_ccs_init_key_pair (const struct ecc_engine *engine, const uint8_t *key, size_t key_length,
	struct ecc_private_key *priv_key, struct ecc_public_key *pub_key);
int ecc_ccs_init_public_key (const struct ecc_engine *engine, const uint8_t *key, size_t key_length,
	struct ecc_public_key *pub_key);
int ecc_ccs_generate_derived_key_pair (const struct ecc_engine *engine, const uint8_t *priv,
	size_t key_length, struct ecc_private_key *priv_key, struct ecc_public_key *pub_key);
int ecc_ccs_generate_key_pair (const struct ecc_engine *engine, size_t key_length,
	struct ecc_private_key *priv_key, struct ecc_public_key *pub_key);
void ecc_ccs_release_key_pair (const struct ecc_engine *engine, struct ecc_private_key *priv_key,
	struct ecc_public_key *pub_key);
int ecc_ccs_get_signature_max_length (const struct ecc_engine *engine,
	const struct ecc_private_key *key);
int ecc_ccs_get_signature_max_verify_length (const struct ecc_engine *engine,
	const struct ecc_public_key *key);
int ecc_ccs_get_private_key_der (const struct ecc_engine *engine, const struct ecc_private_key *key,
	uint8_t **der, size_t *length);
int ecc_ccs_get_public_key_der (const struct ecc_engine *engine, const struct ecc_public_key *key,
	uint8_t **der, size_t *length);
int ecc_ccs_sign (const struct ecc_engine *engine, const struct ecc_private_key *key,
	const uint8_t *digest, size_t length, const struct rng_engine *rng, uint8_t *signature,
	size_t sig_length);
int ecc_ccs_verify (const struct ecc_engine *engine, const struct ecc_public_key *key,
	const uint8_t *digest, size_t length, const uint8_t *signature, size_t sig_length);
int ecc_ccs_get_shared_secret_max_length (const struct ecc_engine *engine,
	const struct ecc_private_key *key);
int ecc_ccs_compute_shared_secret (const struct ecc_engine *engine,
	const struct ecc_private_key *priv_key, const struct ecc_public_key *pub_key, uint8_t *secret,
	size_t length);


/**
 * Constant initializer for key generation APIs.
 */
#ifdef ECC_ENABLE_GENERATE_KEY_PAIR
#define	ECC_CCS_GENERATE_API    \
	.generate_derived_key_pair = ecc_ccs_generate_derived_key_pair, \
	.generate_key_pair = ecc_ccs_generate_key_pair,

#define	ECC_CCS_DER_API \
	.get_private_key_der = ecc_ccs_get_private_key_der, \
	.get_public_key_der = ecc_ccs_get_public_key_der,
#else
#define	ECC_CCS_GENERATE_API
#define	ECC_CCS_DER_API
#endif

/**
 * Constant initializer for ECDH APIs.
 */
#ifdef ECC_ENABLE_ECDH
#define	ECC_CCS_ECDH_API    \
	.get_shared_secret_max_length = ecc_ccs_get_shared_secret_max_length, \
	.compute_shared_secret = ecc_ccs_compute_shared_secret,
#else
#define	ECC_CCS_ECDH_API
#endif

/**
 * Constant initializer for the ECC API.
 */
#define	ECC_CCS_API_INIT  { \
		.init_key_pair = ecc_ccs_init_key_pair, \
		.init_public_key = ecc_ccs_init_public_key, \
		ECC_CCS_GENERATE_API \
		.release_key_pair = ecc_ccs_release_key_pair, \
		.get_signature_max_length = ecc_ccs_get_signature_max_length, \
		.get_signature_max_verify_length = ecc_ccs_get_signature_max_verify_length, \
		ECC_CCS_DER_API \
		.sign = ecc_ccs_sign, \
		.verify = ecc_ccs_verify, \
		ECC_CCS_ECDH_API \
	}


/**
 * Initialize a static instance for executing ECC operations using the HSP CCS.
 *
 * There is no validation done on the arguments.
 *
 * @param ccs_ptr The CCS driver that should be used for ECC private key operations.
 * @param pub_ptr The ECC engine that should be used for ECC public key operations.
 */
#define	ecc_ccs_static_init(ccs_ptr, pub_ptr)	{ \
		.base = ECC_CCS_API_INIT, \
		.ccs = ccs_ptr, \
		.pub = pub_ptr \
	}


#endif	/* ECC_CCS_STATIC_H_ */
