// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "aes_cbc_hsp.h"
#include "aes_hsp_hw_intf.h"
#include "common/buffer_util.h"
#include "common/unused.h"


int aes_cbc_hsp_set_key (const struct aes_cbc_engine *engine, const uint8_t *key, size_t length)
{
	const struct aes_cbc_engine_hsp *hsp = (const struct aes_cbc_engine_hsp*) engine;

	if ((engine == NULL) || (key == NULL)) {
		return AES_CBC_ENGINE_INVALID_ARGUMENT;
	}

	switch (length) {
		case (128 / 8):
		case (192 / 8):
			return AES_CBC_ENGINE_UNSUPPORTED_KEY_LENGTH;

		case AES_CBC_256_KEY_LENGTH:
			break;

		default:
			return AES_CBC_ENGINE_INVALID_KEY_LENGTH;
	}

	return aes_hsp_hw_intf_set_key (hsp->ccs, key, length, hsp->slot_id, 0);
}

int aes_cbc_hsp_clear_key (const struct aes_cbc_engine *engine)
{
	const struct aes_cbc_engine_hsp *hsp = (const struct aes_cbc_engine_hsp*) engine;

	if (hsp == NULL) {
		return AES_CBC_ENGINE_INVALID_ARGUMENT;
	}

	return aes_hsp_hw_intf_clear_key (hsp->ccs, hsp->slot_id);
}

int aes_cbc_hsp_encrypt_data (const struct aes_cbc_engine *engine, const uint8_t *plaintext,
	size_t length, const uint8_t iv[AES_CBC_BLOCK_SIZE], uint8_t *ciphertext, size_t out_length,
	uint8_t out_iv[AES_CBC_BLOCK_SIZE])
{
	const struct aes_cbc_engine_hsp *hsp = (const struct aes_cbc_engine_hsp*) engine;

	if ((engine == NULL) || (plaintext == NULL) || (iv == NULL) || (ciphertext == NULL)) {
		return AES_CBC_ENGINE_INVALID_ARGUMENT;
	}

	return aes_hsp_hw_intf_encrypt (hsp->aes, hsp->ccs, HSP_AES_MODE_CBC, hsp->slot_id, 0,
		AES_CBC_ENGINE_NO_KEY, plaintext, length, (SP_MSG_128*) iv, ciphertext, out_length,
		(SP_MSG_128*) out_iv);
}

int aes_cbc_hsp_decrypt_data (const struct aes_cbc_engine *engine, const uint8_t *ciphertext,
	size_t length, const uint8_t iv[AES_CBC_BLOCK_SIZE], uint8_t *plaintext, size_t out_length,
	uint8_t out_iv[AES_CBC_BLOCK_SIZE])
{
	const struct aes_cbc_engine_hsp *hsp = (const struct aes_cbc_engine_hsp*) engine;

	if ((engine == NULL) || (ciphertext == NULL) || (iv == NULL) || (plaintext == NULL)) {
		return AES_CBC_ENGINE_INVALID_ARGUMENT;
	}

	return aes_hsp_hw_intf_decrypt (hsp->aes, hsp->ccs, HSP_AES_MODE_CBC, hsp->slot_id, 0,
		AES_CBC_ENGINE_NO_KEY, ciphertext, length, (SP_MSG_128*) iv, plaintext, out_length,
		(SP_MSG_128*) out_iv);
}

/**
 * Initialize an instance for running AES-CBC operations using the HSP AES hardware block.
 *
 * @param engine The AES-CBC engine to initialize.
 * @param aes Driver for the AES hardware to use.
 * @param ccs Driver for the KSU used for managing the AES key used for encryption.
 * @param slot_id KSU slot ID to use for the key.
 *
 * @return 0 if the AES-CBC engine was successfully initialized or an error code.
 */
int aes_cbc_hsp_init (struct aes_cbc_engine_hsp *engine, const struct hsp_aes *aes,
	const struct ccs_ksu_interface *ccs, uint8_t slot_id)
{
	if ((engine == NULL) || (aes == NULL) || (ccs == NULL)) {
		return AES_CBC_ENGINE_INVALID_ARGUMENT;
	}

	memset (engine, 0, sizeof (*engine));

	engine->base.set_key = aes_cbc_hsp_set_key;
	engine->base.clear_key = aes_cbc_hsp_clear_key;
	engine->base.encrypt_data = aes_cbc_hsp_encrypt_data;
	engine->base.decrypt_data = aes_cbc_hsp_decrypt_data;

	engine->aes = aes;
	engine->ccs = ccs;
	engine->slot_id = slot_id;

	return 0;
}

/**
 * Release the resources used by an HSP AES-CBC engine.
 *
 * @param engine The AES-CBC engine to release.
 */
void aes_cbc_hsp_release (const struct aes_cbc_engine_hsp *engine)
{
	UNUSED (engine);
}
