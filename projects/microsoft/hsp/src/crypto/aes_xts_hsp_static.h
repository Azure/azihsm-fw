// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef AES_XTS_HSP_STATIC_H_
#define AES_XTS_HSP_STATIC_H_

#include "aes_xts_hsp.h"


/* Internal functions declared to allow for static initialization. */
int aes_xts_hsp_set_key (const struct aes_xts_engine *engine, const uint8_t *key, size_t length);
int aes_xts_hsp_clear_key (const struct aes_xts_engine *engine);
int aes_xts_hsp_encrypt_data (const struct aes_xts_engine *engine, const uint8_t *plaintext,
	size_t length, const uint8_t data_unit_id[16], uint8_t *ciphertext, size_t out_length);
int aes_xts_hsp_decrypt_data (const struct aes_xts_engine *engine, const uint8_t *ciphertext,
	size_t length, const uint8_t data_unit_id[16], uint8_t *plaintext, size_t out_length);


/**
 * Constant initializer for the AES-XTS API.
 */
#define	AES_XTS_HSP_API_INIT	{ \
		.set_key = aes_xts_hsp_set_key, \
		.clear_key = aes_xts_hsp_clear_key, \
		.encrypt_data = aes_xts_hsp_encrypt_data, \
		.decrypt_data = aes_xts_hsp_decrypt_data, \
	}


/**
 * Initialize a static instance for running AES-XTS operations using the HSP AES hardware block.
 *
 * There is no validation done on the arguments.
 *
 * @param aes_ptr Driver for the AES hardware to use.
 * @param ccs_ptr Driver for the KSU used for managing the AES key used for encryption.
 * @param slot_id_arg KSU slot ID to use for the key.
 */
#define	aes_xts_hsp_static_init(aes_ptr, ccs_ptr, slot_id_arg)	{ \
		.base = AES_XTS_HSP_API_INIT, \
		.aes = aes_ptr, \
		.ccs = ccs_ptr, \
		.slot_id = slot_id_arg, \
	}


#endif	/* AES_XTS_HSP_STATIC_H_ */
