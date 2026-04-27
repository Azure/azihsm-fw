// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hash_hs_sha.h"


/**
 * Calculate a hash for a buffer of data.  This will not take into account any previous context.
 *
 * @param sha The engine to use for hashing.
 * @param data The data to hash.
 * @param length Length of the data.
 * @param type The hash algorithm to use.
 * @param hash Output for the calculated.
 * @param hash_length Length of the output buffer.
 *
 * @return 0 if the hash was successfully calculated or an error code.
 */
static int hash_hs_sha_calculate_hash (const struct hash_engine_hs_sha *sha, const uint8_t *data,
	size_t length, enum hash_type type, uint8_t *hash, size_t hash_length)
{
	int status;

	if ((sha == NULL) || ((data == NULL) && (length != 0)) || (hash == NULL)) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	if (sha->state->context.active != HASH_ACTIVE_NONE) {
		return HASH_ENGINE_HASH_IN_PROGRESS;
	}

	status = hs_sha_finish_digest (sha->hw, data, length, length, NULL, type, hash, hash_length);
	if (ROT_IS_ERROR (status)) {
		if (status == HS_SHA_DIGEST_BUFFER_TOO_SMALL) {
			status = HASH_ENGINE_HASH_BUFFER_TOO_SMALL;
		}
	}
	else {
		status = 0;
	}

	return status;
}

#ifdef HASH_ENABLE_SHA1
int hash_hs_sha_calculate_sha1 (const struct hash_engine *engine, const uint8_t *data,
	size_t length, uint8_t *hash, size_t hash_length)
{
	return hash_hs_sha_calculate_hash ((const struct hash_engine_hs_sha*) engine, data, length,
		HASH_TYPE_SHA1, hash, hash_length);
}

int hash_hs_sha_start_sha1 (const struct hash_engine *engine)
{
	const struct hash_engine_hs_sha *sha = (const struct hash_engine_hs_sha*) engine;

	if (sha == NULL) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	if (sha->state->context.active != HASH_ACTIVE_NONE) {
		return HASH_ENGINE_HASH_IN_PROGRESS;
	}

	hs_sha_multi_update_start_sha1 (&sha->state->context);

	return 0;
}
#endif

int hash_hs_sha_calculate_sha256 (const struct hash_engine *engine, const uint8_t *data,
	size_t length, uint8_t *hash, size_t hash_length)
{
	return hash_hs_sha_calculate_hash ((const struct hash_engine_hs_sha*) engine, data, length,
		HASH_TYPE_SHA256, hash, hash_length);
}

int hash_hs_sha_start_sha256 (const struct hash_engine *engine)
{
	const struct hash_engine_hs_sha *sha = (const struct hash_engine_hs_sha*) engine;

	if (sha == NULL) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	if (sha->state->context.active != HASH_ACTIVE_NONE) {
		return HASH_ENGINE_HASH_IN_PROGRESS;
	}

	hs_sha_multi_update_start_sha256 (&sha->state->context);

	return 0;
}

#ifdef HASH_ENABLE_SHA384
int hash_hs_sha_calculate_sha384 (const struct hash_engine *engine, const uint8_t *data,
	size_t length, uint8_t *hash, size_t hash_length)
{
	return hash_hs_sha_calculate_hash ((const struct hash_engine_hs_sha*) engine, data, length,
		HASH_TYPE_SHA384, hash, hash_length);
}

int hash_hs_sha_start_sha384 (const struct hash_engine *engine)
{
	const struct hash_engine_hs_sha *sha = (const struct hash_engine_hs_sha*) engine;

	if (sha == NULL) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	if (sha->state->context.active != HASH_ACTIVE_NONE) {
		return HASH_ENGINE_HASH_IN_PROGRESS;
	}

	hs_sha_multi_update_start_sha384 (&sha->state->context);

	return 0;
}
#endif

#ifdef HASH_ENABLE_SHA512
int hash_hs_sha_calculate_sha512 (const struct hash_engine *engine, const uint8_t *data,
	size_t length, uint8_t *hash, size_t hash_length)
{
	return hash_hs_sha_calculate_hash ((const struct hash_engine_hs_sha*) engine, data, length,
		HASH_TYPE_SHA512, hash, hash_length);
}

int hash_hs_sha_start_sha512 (const struct hash_engine *engine)
{
	const struct hash_engine_hs_sha *sha = (const struct hash_engine_hs_sha*) engine;

	if (sha == NULL) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	if (sha->state->context.active != HASH_ACTIVE_NONE) {
		return HASH_ENGINE_HASH_IN_PROGRESS;
	}

	hs_sha_multi_update_start_sha512 (&sha->state->context);

	return 0;
}
#endif

enum hash_type hash_hs_sha_get_active_algorithm (const struct hash_engine *engine)
{
	const struct hash_engine_hs_sha *sha = (const struct hash_engine_hs_sha*) engine;

	if (sha == NULL) {
		return HASH_TYPE_INVALID;
	}

	return hash_get_type_from_active (sha->state->context.active);
}

int hash_hs_sha_update (const struct hash_engine *engine, const uint8_t *data, size_t length)
{
	const struct hash_engine_hs_sha *sha = (const struct hash_engine_hs_sha*) engine;

	if ((sha == NULL) || ((data == NULL) && (length != 0))) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	if (sha->state->context.active == HASH_ACTIVE_NONE) {
		return HASH_ENGINE_NO_ACTIVE_HASH;
	}

	return hs_sha_multi_update_update_digest (sha->hw, &sha->state->context, data, length);
}

int hash_hs_sha_get_hash (const struct hash_engine *engine, uint8_t *hash, size_t hash_length)
{
	const struct hash_engine_hs_sha *sha = (const struct hash_engine_hs_sha*) engine;
	int status;

	if ((sha == NULL) || (hash == NULL)) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	if (sha->state->context.active == HASH_ACTIVE_NONE) {
		return HASH_ENGINE_NO_ACTIVE_HASH;
	}

	status = hs_sha_multi_update_get_digest (sha->hw, &sha->state->context, hash, hash_length);
	if (ROT_IS_ERROR (status)) {
		if (status == HS_SHA_DIGEST_BUFFER_TOO_SMALL) {
			status = HASH_ENGINE_HASH_BUFFER_TOO_SMALL;
		}
	}
	else {
		status = 0;
	}

	return status;
}

void hash_hs_sha_cancel (const struct hash_engine *engine)
{
	const struct hash_engine_hs_sha *sha = (const struct hash_engine_hs_sha*) engine;

	if (sha) {
		hs_sha_multi_update_init (&sha->state->context);
	}
}

int hash_hs_sha_finish (const struct hash_engine *engine, uint8_t *hash, size_t hash_length)
{
	const struct hash_engine_hs_sha *sha = (const struct hash_engine_hs_sha*) engine;
	int status;

	if ((sha == NULL) || (hash == NULL)) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	if (sha->state->context.active == HASH_ACTIVE_NONE) {
		return HASH_ENGINE_NO_ACTIVE_HASH;
	}

	status = hs_sha_multi_update_finish_digest (sha->hw, &sha->state->context, hash, hash_length);
	if (ROT_IS_ERROR (status)) {
		if (status == HS_SHA_DIGEST_BUFFER_TOO_SMALL) {
			status = HASH_ENGINE_HASH_BUFFER_TOO_SMALL;
		}
	}
	else {
		status = 0;
	}

	return status;
}

/**
 * Initialize a HS-SHA hash engine.
 *
 * @param engine The hashing engine to initialize.
 * @param state Variable state for the hashing engine.  This must be uninitialized.
 * @param hw Driver for the specific HW block that will be used for hashing operations.  This driver
 * can be shared between multiple hash engine instances.
 *
 * @return 0 if the hash engine was successfully initialized or an error code.
 */
int hash_hs_sha_init (struct hash_engine_hs_sha *engine, struct hash_engine_hs_sha_state *state,
	const struct hs_sha *hw)
{
	if ((engine == NULL) || (state == NULL) || (hw == NULL)) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	memset (engine, 0, sizeof (struct hash_engine_hs_sha));

#ifdef HASH_ENABLE_SHA1
	engine->base.calculate_sha1 = hash_hs_sha_calculate_sha1;
	engine->base.start_sha1 = hash_hs_sha_start_sha1;
#endif
	engine->base.calculate_sha256 = hash_hs_sha_calculate_sha256;
	engine->base.start_sha256 = hash_hs_sha_start_sha256;
#ifdef HASH_ENABLE_SHA384
	engine->base.calculate_sha384 = hash_hs_sha_calculate_sha384;
	engine->base.start_sha384 = hash_hs_sha_start_sha384;
#endif
#ifdef HASH_ENABLE_SHA512
	engine->base.calculate_sha512 = hash_hs_sha_calculate_sha512;
	engine->base.start_sha512 = hash_hs_sha_start_sha512;
#endif
	engine->base.get_active_algorithm = hash_hs_sha_get_active_algorithm;
	engine->base.update = hash_hs_sha_update;
	engine->base.get_hash = hash_hs_sha_get_hash;
	engine->base.finish = hash_hs_sha_finish;
	engine->base.cancel = hash_hs_sha_cancel;

	engine->state = state;
	engine->hw = hw;

	return hash_hs_sha_init_state (engine);
}

/**
 * Initialize only the variable state for an HS-SHA hash engine.  The rest of the instance is
 * assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param engine The hash engine that contains the state to initialize.
 *
 * @return 0 if the driver state was successfully initialized or an error code.
 */
int hash_hs_sha_init_state (const struct hash_engine_hs_sha *engine)
{
	if ((engine == NULL) || (engine->state == NULL) || (engine->hw == NULL)) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	hs_sha_multi_update_init (&engine->state->context);

	return 0;
}

/**
 * Release the resources used by a HS-SHA hash engine.
 *
 * @param engine The hashing engine to release.
 */
void hash_hs_sha_release (const struct hash_engine_hs_sha *engine)
{
	if (engine) {
		/* Be sure any active hash operation is cleared. */
		hs_sha_multi_update_init (&engine->state->context);
	}
}
