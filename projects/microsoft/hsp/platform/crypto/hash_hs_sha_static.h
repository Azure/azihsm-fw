// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HASH_HS_SHA_STATIC_H_
#define HASH_HS_SHA_STATIC_H_

#include "crypto/hash_hs_sha.h"


/* Internal functions declared to allow for static initialization. */
int hash_hs_sha_calculate_sha1 (const struct hash_engine *engine, const uint8_t *data,
	size_t length, uint8_t *hash, size_t hash_length);
int hash_hs_sha_start_sha1 (const struct hash_engine *engine);
int hash_hs_sha_calculate_sha256 (const struct hash_engine *engine, const uint8_t *data,
	size_t length, uint8_t *hash, size_t hash_length);
int hash_hs_sha_start_sha256 (const struct hash_engine *engine);
int hash_hs_sha_calculate_sha384 (const struct hash_engine *engine, const uint8_t *data,
	size_t length, uint8_t *hash, size_t hash_length);
int hash_hs_sha_start_sha384 (const struct hash_engine *engine);
int hash_hs_sha_calculate_sha512 (const struct hash_engine *engine, const uint8_t *data,
	size_t length, uint8_t *hash, size_t hash_length);
int hash_hs_sha_start_sha512 (const struct hash_engine *engine);
int hash_hs_sha_update (const struct hash_engine *engine, const uint8_t *data, size_t length);
int hash_hs_sha_get_hash (const struct hash_engine *engine, uint8_t *hash, size_t hash_length);
int hash_hs_sha_finish (const struct hash_engine *engine, uint8_t *hash, size_t hash_length);
void hash_hs_sha_cancel (const struct hash_engine *engine);
enum hash_type hash_hs_sha_get_active_algorithm (const struct hash_engine *engine);


/**
 * Constant initializer for SHA1 API functions.
 */
#ifdef HASH_ENABLE_SHA1
#define	HASH_HS_SHA_SHA1_API    \
	.calculate_sha1 = hash_hs_sha_calculate_sha1, \
	.start_sha1 = hash_hs_sha_start_sha1,
#else
#define	HASH_HS_SHA_SHA1_API
#endif

/**
 * Constant initializer for SHA256 API functions.
 */
#define	HASH_HS_SHA_SHA256_API  \
	.calculate_sha256 = hash_hs_sha_calculate_sha256, \
	.start_sha256 = hash_hs_sha_start_sha256,

/**
 * Constant initializer for SHA384 API functions.
 */
#ifdef HASH_ENABLE_SHA384
#define	HASH_HS_SHA_SHA384_API  \
	.calculate_sha384 = hash_hs_sha_calculate_sha384, \
	.start_sha384 = hash_hs_sha_start_sha384,
#else
#define	HASH_HS_SHA_SHA384_API
#endif

/**
 * Constant initializer for SHA512 API functions.
 */
#ifdef HASH_ENABLE_SHA512
#define	HASH_HS_SHA_SHA512_API  \
	.calculate_sha512 = hash_hs_sha_calculate_sha512, \
	.start_sha512 = hash_hs_sha_start_sha512,
#else
#define	HASH_HS_SHA_SHA512_API
#endif

/**
 * Constant initializer for the hash API.
 */
#define	HASH_HS_SHA_API_INIT	{ \
		HASH_HS_SHA_SHA1_API \
		HASH_HS_SHA_SHA256_API \
		HASH_HS_SHA_SHA384_API \
		HASH_HS_SHA_SHA512_API \
		.get_active_algorithm = hash_hs_sha_get_active_algorithm, \
		.update = hash_hs_sha_update, \
		.get_hash = hash_hs_sha_get_hash, \
		.finish = hash_hs_sha_finish, \
		.cancel = hash_hs_sha_cancel \
	}


/**
 * Initialize a static HS-SHA hash engine instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable state for the hashing engine.
 * @param hw_ptr Driver for the specific HW block that will be used for hashing operations.  This
 * driver can be shared between multiple hash engine instances.
 */
#define	hash_hs_sha_static_init(state_ptr, hw_ptr)	{ \
		.base = HASH_HS_SHA_API_INIT, \
		.state = state_ptr, \
		.hw = hw_ptr, \
	}


#endif	/* HASH_HS_SHA_STATIC_H_ */
