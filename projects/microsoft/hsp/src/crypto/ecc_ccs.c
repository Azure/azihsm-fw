// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "ecc_ccs.h"
#include "platform_api.h"
#include "asn1/asn1_util.h"
#include "asn1/ecc_der_util.h"
#include "common/unused.h"


int ecc_ccs_init_key_pair (const struct ecc_engine *engine, const uint8_t *key, size_t key_length,
	struct ecc_private_key *priv_key, struct ecc_public_key *pub_key)
{
	const struct ecc_engine_ccs *ecc = (const struct ecc_engine_ccs*) engine;
	uint64_t slot_num;
	union ccs_ksu_ecc_public_key raw_pub;
	uint32_t attributes;
	uint8_t pub_der[ECC_DER_P384_PUBLIC_LENGTH];
	int status;

	if ((ecc == NULL) || (key == NULL) || (key_length == 0)) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	status = asn1_decode_integer (key, key_length, &slot_num);
	if (status == 0) {
		if (slot_num > 0xff) {
			/* The decoded slot number is too large for the CCS API. */
			return ECC_ENGINE_KEY_PAIR_FAILED;
		}

		if (priv_key) {
			status = ecc->ccs->is_key_slot_valid (ecc->ccs, slot_num);
			if (status != 0) {
				return status;
			}

			priv_key->context = (void*) (uintptr_t) slot_num;
		}

		if (pub_key) {
			status = ecc->ccs->get_ecc_public_key (ecc->ccs, slot_num, &raw_pub.p384, &attributes);
			if (status != 0) {
				return status;
			}

			/* DER encoding will never fail. */
			if (attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
				status = ecc_der_encode_public_key (raw_pub.p384.Parts.X.AsBytes,
					raw_pub.p384.Parts.Y.AsBytes, SP_MSG_384_SIZE, pub_der, sizeof (pub_der));
			}
			else {
				status = ecc_der_encode_public_key (raw_pub.p256.Parts.X.AsBytes,
					raw_pub.p256.Parts.Y.AsBytes, SP_MSG_256_SIZE, pub_der, sizeof (pub_der));
			}

			status = ecc->pub->init_public_key (ecc->pub, pub_der, status, pub_key);
		}
	}
	else {
		/* If the key is not an integer indicating a KSU key slot, treat it as a DER encoded private
		 * key. */
		if (priv_key || !pub_key) {
			/* TODO: Support loading DER encoded private keys into the KSU.  This would require a
			 * way to find an available slot in CCS and a way to know when to clear the key from the
			 * KSU.
			 *
			 * The flow would be:
			 * 1. Call CCS driver to get an available KSU slot.
			 * 2. Decode the DER encoded key (reference ecc_ecc_hw implementation).
			 * 3. Call set_key to put the private key into the KSU slot.
			 * 4. If the caller also wants the public key, initialize the public key context in the
			 *    same way as if a DER encoded slot was provided (i.e. get_ecc_public_key,
			 *    init_public_key).
			 * 5. On any error, the KSU slot needs to be cleared and made available again. */
			status = ECC_ENGINE_UNSUPPORTED_OPERATION;
		}
		else {
			status = ecc->pub->init_key_pair (ecc->pub, key, key_length, NULL, pub_key);
		}
	}

	return status;
}

int ecc_ccs_init_public_key (const struct ecc_engine *engine, const uint8_t *key, size_t key_length,
	struct ecc_public_key *pub_key)
{
	const struct ecc_engine_ccs *ecc = (const struct ecc_engine_ccs*) engine;

	if (ecc == NULL) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	return ecc->pub->init_public_key (ecc->pub, key, key_length, pub_key);
}

#ifdef ECC_ENABLE_GENERATE_KEY_PAIR
int ecc_ccs_generate_derived_key_pair (const struct ecc_engine *engine, const uint8_t *priv,
	size_t key_length, struct ecc_private_key *priv_key, struct ecc_public_key *pub_key)
{
	if ((engine == NULL) || (priv == NULL) || (key_length == 0)) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	switch (key_length) {
		case ECC_KEY_LENGTH_256:
#if ECC_MAX_KEY_LENGTH >= ECC_KEY_LENGTH_384
		case ECC_KEY_LENGTH_384:
#endif
			break;

		default:
			/* CCS cannot support 521-bit keys. */
			return ECC_ENGINE_UNSUPPORTED_KEY_LENGTH;
	}

	if (!priv_key && !pub_key) {
		/* Don't really need to do anything. */
		return 0;
	}

	/* TODO: Support deriving ECC from a known seed into the KSU.  This would require a way to find
	 * an available slot in CCS and a way to know when to clear the key from the KSU.
	 *
	 * The flow would be:
	 * 1. Call CSS driver to get an available KSU slot.
	 * 2. Call set_key to put the key seed into the KSU slot, with the KDF attribute set.
	 * 3. Call derive_ecc_key to generate the ECC key and put it into the same KSU slot.
	 * 4. If the caller also wants the public key, initialize the public key context in the
	 *    same way as if the key was already in the KSU (i.e. get_ecc_public_key, init_public_key).
	 * 5. On any error, the KSU slot needs to be cleared and made available again. */

	return ECC_ENGINE_UNSUPPORTED_OPERATION;
}

int ecc_ccs_generate_key_pair (const struct ecc_engine *engine, size_t key_length,
	struct ecc_private_key *priv_key, struct ecc_public_key *pub_key)
{
	if (engine == NULL) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	switch (key_length) {
		case ECC_KEY_LENGTH_256:
#if ECC_MAX_KEY_LENGTH >= ECC_KEY_LENGTH_384
		case ECC_KEY_LENGTH_384:
#endif
			break;

		default:
			/* CCS cannot support 521-bit keys. */
			return ECC_ENGINE_UNSUPPORTED_KEY_LENGTH;
	}

	if (!priv_key && !pub_key) {
		/* Don't really need to do anything. */
		return 0;
	}

	/* TODO: Support generating random ECC keys into the KSU.  This would require a way to find
	 * an available slot in CCS and a way to know when to clear the key from the KSU.
	 *
	 * The flow would be:
	 * 1. Call CSS driver to get an available KSU slot.
	 * 3. Call generate_random_ecc_key to generate the ECC key.
	 * 4. If the caller also wants the public key, initialize the public key context in the
	 *    same way as if the key was already in the KSU (i.e. get_ecc_public_key, init_public_key).
	 * 5. On any error, the KSU slot needs to be cleared and made available again. */

	return ECC_ENGINE_UNSUPPORTED_OPERATION;
}
#endif

void ecc_ccs_release_key_pair (const struct ecc_engine *engine, struct ecc_private_key *priv_key,
	struct ecc_public_key *pub_key)
{
	const struct ecc_engine_ccs *ecc = (const struct ecc_engine_ccs*) engine;

	if (priv_key) {
		priv_key->context = NULL;
	}

	if (pub_key) {
		ecc->pub->release_key_pair (ecc->pub, NULL, pub_key);
	}
}

int ecc_ccs_get_signature_max_length (const struct ecc_engine *engine,
	const struct ecc_private_key *key)
{
	const struct ecc_engine_ccs *ecc = (const struct ecc_engine_ccs*) engine;
	uint32_t attributes;
	int status;

	if ((ecc == NULL) || (key == NULL)) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	status = ecc->ccs->get_key_attributes (ecc->ccs, (uintptr_t) key->context, &attributes);
	if (status != 0) {
		return status;
	}

	if (!(attributes & CCS_KSU_ATTR_KEY_SIZE_384)) {
		return ECC_DER_P256_ECDSA_MAX_LENGTH;
	}
#if ECC_MAX_KEY_LENGTH >= ECC_KEY_LENGTH_384
	else {
		return ECC_DER_P384_ECDSA_MAX_LENGTH;
	}
#endif

	return ECC_ENGINE_UNSUPPORTED_KEY_LENGTH;
}

int ecc_ccs_get_signature_max_verify_length (const struct ecc_engine *engine,
	const struct ecc_public_key *key)
{
	const struct ecc_engine_ccs *ecc = (const struct ecc_engine_ccs*) engine;

	if ((ecc == NULL) || (key == NULL)) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	return ecc->pub->get_signature_max_verify_length (ecc->pub, key);
}

#ifdef ECC_ENABLE_GENERATE_KEY_PAIR
int ecc_ccs_get_private_key_der (const struct ecc_engine *engine, const struct ecc_private_key *key,
	uint8_t **der, size_t *length)
{
	if (der == NULL) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	*der = NULL;
	if ((engine == NULL) || (key == NULL) || (length == NULL)) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	if ((uintptr_t) key->context > 0xff) {
		/* The context does not represent a key slot number. */
		return ECC_ENGINE_NOT_PRIVATE_KEY;
	}

	/* CCS slot number is a one byte value.  A single byte will never encode to more than 4 bytes as
	 * an ASN.1 INTEGER. */
	*der = platform_malloc (4);
	if (*der == NULL) {
		return ECC_ENGINE_NO_MEMORY;
	}

	/* This call won't fail since the buffer is valid and large enough for any byte value. */
	*length = asn1_encode_integer ((uint8_t) (uintptr_t) key->context, *der, 4);

	return 0;
}

int ecc_ccs_get_public_key_der (const struct ecc_engine *engine, const struct ecc_public_key *key,
	uint8_t **der, size_t *length)
{
	const struct ecc_engine_ccs *ecc = (const struct ecc_engine_ccs*) engine;

	if (der == NULL) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	*der = NULL;
	if ((ecc == NULL) || (key == NULL) || (length == NULL)) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	return ecc->pub->get_public_key_der (ecc->pub, key, der, length);
}
#endif

int ecc_ccs_sign (const struct ecc_engine *engine, const struct ecc_private_key *key,
	const uint8_t *digest, size_t length, const struct rng_engine *rng, uint8_t *signature,
	size_t sig_length)
{
	const struct ecc_engine_ccs *ecc = (const struct ecc_engine_ccs*) engine;
	union ccs_ksu_ecc_signature raw_sig;
	uint32_t attributes;
	SP_MSG_384 buffer;
	SP_MSG_384 *sign_digest = (SP_MSG_384*) digest;
	int status;

	if ((ecc == NULL) || (key == NULL) || (digest == NULL) || (signature == NULL) ||
		(length == 0)) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	if ((uintptr_t) key->context > 0xff) {
		/* The context does not represent a key slot number. */
		return ECC_ENGINE_NOT_PRIVATE_KEY;
	}

	switch (length) {
		case SHA256_HASH_LENGTH:
		case SHA384_HASH_LENGTH:
			break;

		default:
			return ECC_ENGINE_UNSUPPORTED_HASH_TYPE;
	}

	status = ecc->ccs->get_key_attributes (ecc->ccs, (uintptr_t) key->context, &attributes);
	if (status != 0) {
		return status;
	}

	if (attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		if (length == SHA256_HASH_LENGTH) {
			/* The provided digest isn't long enough for the key, so it needs to be copied. */
			memset (buffer.AsBytes, 0, SP_MSG_384_SIZE);
			memcpy (buffer.AsBytes, digest, SHA256_HASH_LENGTH);
			sign_digest = &buffer;
		}
	}
	else {
		if (length != SHA256_HASH_LENGTH) {
			/* ECC-256 keys can only sign SHA256 digests.*/
			return ECC_ENGINE_INCOMPATIBLE_DIGEST;
		}
	}

	status = ecc->ccs->ecc_sign (ecc->ccs, (uintptr_t) key->context, sign_digest, rng,
		&raw_sig.p384, &attributes);
	if (status != 0) {
		return status;
	}

	if (attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		status = ecc_der_encode_ecdsa_signature (raw_sig.p384.Parts.R.AsBytes,
			raw_sig.p384.Parts.S.AsBytes, SP_MSG_384_SIZE, signature, sig_length);
	}
	else {
		status = ecc_der_encode_ecdsa_signature (raw_sig.p256.Parts.R.AsBytes,
			raw_sig.p256.Parts.S.AsBytes, SP_MSG_256_SIZE, signature, sig_length);
	}

	if (status == ECC_DER_UTIL_SMALL_DER_BUFFER) {
		status = ECC_ENGINE_SIG_BUFFER_TOO_SMALL;
	}

	return status;
}

int ecc_ccs_verify (const struct ecc_engine *engine, const struct ecc_public_key *key,
	const uint8_t *digest, size_t length, const uint8_t *signature, size_t sig_length)
{
	const struct ecc_engine_ccs *ecc = (const struct ecc_engine_ccs*) engine;

	if (ecc == NULL) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	return ecc->pub->verify (ecc->pub, key, digest, length, signature, sig_length);
}

#ifdef ECC_ENABLE_ECDH
int ecc_ccs_get_shared_secret_max_length (const struct ecc_engine *engine,
	const struct ecc_private_key *key)
{
	const struct ecc_engine_ccs *ecc = (const struct ecc_engine_ccs*) engine;
	uint32_t attributes;
	int status;

	if ((ecc == NULL) || (key == NULL)) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	status = ecc->ccs->get_key_attributes (ecc->ccs, (uintptr_t) key->context, &attributes);
	if (status != 0) {
		return status;
	}

	if (!(attributes & CCS_KSU_ATTR_KEY_SIZE_384)) {
		return ECC_KEY_LENGTH_256;
	}
#if ECC_MAX_KEY_LENGTH >= ECC_KEY_LENGTH_384
	else {
		return ECC_KEY_LENGTH_384;
	}
#endif

	return ECC_ENGINE_UNSUPPORTED_KEY_LENGTH;
}

int ecc_ccs_compute_shared_secret (const struct ecc_engine *engine,
	const struct ecc_private_key *priv_key, const struct ecc_public_key *pub_key, uint8_t *secret,
	size_t length)
{
	if ((engine == NULL) || (priv_key == NULL) || (pub_key == NULL) || (secret == NULL)) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	UNUSED (length);

	/* CCS does not support raw ECDH operations, which means this ECC implementation cannot be used
	 * by anything that relies on ECDH. */
	return ECC_ENGINE_UNSUPPORTED_OPERATION;
}
#endif

/**
 * Initialize an instance for executing ECC operations using the HSP CCS.
 *
 * @param engine The ECC context to initialize.
 * @param ccs The CCS driver that should be used for ECC private key operations.
 * @param ecc_pub The ECC engine that should be used for ECC public key operations.
 *
 * @return 0 if initialization was successful or an error code.
 */
int ecc_ccs_init (struct ecc_engine_ccs *engine, const struct ccs_ksu_interface *ccs,
	const struct ecc_engine *ecc_pub)
{
	if ((engine == NULL) || (ccs == NULL) || (ecc_pub == NULL)) {
		return ECC_ENGINE_INVALID_ARGUMENT;
	}

	memset (engine, 0, sizeof (struct ecc_engine_ccs));

	engine->base.init_key_pair = ecc_ccs_init_key_pair;
	engine->base.init_public_key = ecc_ccs_init_public_key;
#ifdef ECC_ENABLE_GENERATE_KEY_PAIR
	engine->base.generate_derived_key_pair = ecc_ccs_generate_derived_key_pair;
	engine->base.generate_key_pair = ecc_ccs_generate_key_pair;
#endif
	engine->base.release_key_pair = ecc_ccs_release_key_pair;
	engine->base.get_signature_max_length = ecc_ccs_get_signature_max_length;
	engine->base.get_signature_max_verify_length = ecc_ccs_get_signature_max_verify_length;
#ifdef ECC_ENABLE_GENERATE_KEY_PAIR
	engine->base.get_private_key_der = ecc_ccs_get_private_key_der;
	engine->base.get_public_key_der = ecc_ccs_get_public_key_der;
#endif
	engine->base.sign = ecc_ccs_sign;
	engine->base.verify = ecc_ccs_verify;
#ifdef ECC_ENABLE_ECDH
	engine->base.get_shared_secret_max_length = ecc_ccs_get_shared_secret_max_length;
	engine->base.compute_shared_secret = ecc_ccs_compute_shared_secret;
#endif

	engine->ccs = ccs;
	engine->pub = ecc_pub;

	return 0;
}

/**
 * Release a hardware accelerated ECC instance.  The interface to the hardware will not be released.
 *
 * @param engine The ECC context to release.
 */
void ecc_ccs_release (const struct ecc_engine_ccs *engine)
{
	UNUSED (engine);
}
