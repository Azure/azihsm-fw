// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "aes_hsp_hw_intf.h"
#include "aes_xts_hsp.h"
#include "common/buffer_util.h"
#include "common/unused.h"


int aes_xts_hsp_set_key (const struct aes_xts_engine *engine, const uint8_t *key, size_t length)
{
	const struct aes_xts_engine_hsp *hsp = (const struct aes_xts_engine_hsp*) engine;

	if ((hsp == NULL) || (key == NULL)) {
		return AES_XTS_ENGINE_INVALID_ARGUMENT;
	}

	switch (length) {
		case (128 / 8) * 2:
			break;

		case (256 / 8) * 2:
			/* HSP only supports AES-128 keys for XTS. */
			return AES_XTS_ENGINE_UNSUPPORTED_KEY_LENGTH;

		default:
			return AES_XTS_ENGINE_INVALID_KEY_LENGTH;
	}

	if (buffer_compare (key, &key[length / 2], length / 2) == 0) {
		/* The two AES keys must be different. */
		return AES_XTS_ENGINE_MATCHING_KEYS;
	}

	return aes_hsp_hw_intf_set_key (hsp->ccs, key, length, hsp->slot_id,
		CCS_KSU_ATTR_AES_128BIT_KEY_ALLOWED);
}

int aes_xts_hsp_clear_key (const struct aes_xts_engine *engine)
{
	const struct aes_xts_engine_hsp *hsp = (const struct aes_xts_engine_hsp*) engine;

	if (hsp == NULL) {
		return AES_XTS_ENGINE_INVALID_ARGUMENT;
	}

	return aes_hsp_hw_intf_clear_key (hsp->ccs, hsp->slot_id);
}

int aes_xts_hsp_encrypt_data (const struct aes_xts_engine *engine, const uint8_t *plaintext,
	size_t length, const uint8_t data_unit_id[16], uint8_t *ciphertext, size_t out_length)
{
	const struct aes_xts_engine_hsp *hsp = (const struct aes_xts_engine_hsp*) engine;

	if ((engine == NULL) || (plaintext == NULL) || (data_unit_id == NULL) || (ciphertext == NULL)) {
		return AES_XTS_ENGINE_INVALID_ARGUMENT;
	}

	return aes_hsp_hw_intf_encrypt (hsp->aes, hsp->ccs, HSP_AES_MODE_XTS, hsp->slot_id,
		CCS_KSU_ATTR_AES_128BIT_KEY_ALLOWED, AES_XTS_ENGINE_NO_KEY, plaintext, length,
		(SP_MSG_128*) data_unit_id, ciphertext, out_length, NULL);
}

int aes_xts_hsp_decrypt_data (const struct aes_xts_engine *engine, const uint8_t *ciphertext,
	size_t length, const uint8_t data_unit_id[16], uint8_t *plaintext, size_t out_length)
{
	const struct aes_xts_engine_hsp *hsp = (const struct aes_xts_engine_hsp*) engine;

	if ((engine == NULL) || (ciphertext == NULL) || (data_unit_id == NULL) || (plaintext == NULL)) {
		return AES_XTS_ENGINE_INVALID_ARGUMENT;
	}

	return aes_hsp_hw_intf_decrypt (hsp->aes, hsp->ccs, HSP_AES_MODE_XTS, hsp->slot_id,
		CCS_KSU_ATTR_AES_128BIT_KEY_ALLOWED, AES_XTS_ENGINE_NO_KEY, ciphertext, length,
		(SP_MSG_128*) data_unit_id, plaintext, out_length, NULL);
}

/**
 * Initialize an instance for running AES-XTS operations using the HSP AES hardware block.
 *
 * @param engine The AES-XTS engine to initialize.
 * @param aes Driver for the AES hardware to use.
 * @param ccs Driver for the KSU used for managing the AES key used for encryption.
 * @param slot_id KSU slot ID to use for the key.
 *
 * @return 0 if the AES-XTS engine was successfully initialized or an error code.
 */
int aes_xts_hsp_init (struct aes_xts_engine_hsp *engine, const struct hsp_aes *aes,
	const struct ccs_ksu_interface *ccs, uint8_t slot_id)
{
	if ((engine == NULL) || (aes == NULL) || (ccs == NULL)) {
		return AES_XTS_ENGINE_INVALID_ARGUMENT;
	}

	memset (engine, 0, sizeof (*engine));

	engine->base.set_key = aes_xts_hsp_set_key;
	engine->base.clear_key = aes_xts_hsp_clear_key;
	engine->base.encrypt_data = aes_xts_hsp_encrypt_data;
	engine->base.decrypt_data = aes_xts_hsp_decrypt_data;

	engine->aes = aes;
	engine->ccs = ccs;
	engine->slot_id = slot_id;

	return 0;
}

/**
 * Release the resources used by an HSP AES-XTS engine.
 *
 * @param engine The AES-XTS engine to release.
 */
void aes_xts_hsp_release (const struct aes_xts_engine_hsp *engine)
{
	UNUSED (engine);
}

/**
 * Derive a new key for XTS use from an existing KSU key.  The new key will be set for use by future
 * XTS operations.
 *
 * This derivation will be performed in a FIPS-complaint way, guaranteeing that XTS key1 and key2
 * will be different.  To achieve this, the AES keys will be present in FW memory for a brief period
 * of time to enable the validity check.
 *
 * @param engine The AES-XTS engine that will be assigned the new key.
 * @param src_key A KSU key slot that will be used to derive the XTS encryption key.  This must have
 * the KDFKeyAllowed attribute set.
 * @param context KDF context to use for XTS key derivation.
 *
 * @return 0 if the key was successfully generated or an error code.
 */
int aes_xts_hsp_derive_xts_key (const struct aes_xts_engine_hsp *engine, uint8_t src_key,
	const SP_MSG_384 *context)
{
	SP_MSG_384 key = {0};
	int status;

	if ((engine == NULL) || (context == NULL)) {
		return AES_XTS_ENGINE_INVALID_ARGUMENT;
	}

	/* Derive a key from the KSU into memory. */
	status = engine->ccs->derive_key (engine->ccs, src_key, context, engine->slot_id,
		CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED);
	if (status != 0) {
		return status;
	}

	do {
		status = engine->ccs->hmac (engine->ccs, engine->slot_id, context->AsBytes, SP_MSG_384_SIZE,
			&key, NULL);
		if (status != 0) {
			break;
		}

		/* Store the derived key into the KSU for use with the AES HW. */
		status = aes_xts_hsp_set_key (&engine->base, key.AsBytes, SP_MSG_256_SIZE);
		if (status == AES_XTS_ENGINE_MATCHING_KEYS) {
			/* The KDF has generated an invalid XTS key.  Run additional KDFs to get a valid key.
			 * The next KDF will use the invalid key as the source key. */
			status = engine->ccs->set_key (engine->ccs, &key, engine->slot_id,
				CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED);
			if (status == 0) {
				status = AES_XTS_ENGINE_MATCHING_KEYS;
			}
		}
	} while (status == AES_XTS_ENGINE_MATCHING_KEYS);

	/* Clear the AES key from FW memory. */
	buffer_zeroize (key.AsBytes, SP_MSG_384_SIZE);

	return status;
}
