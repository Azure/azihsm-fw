// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hash_hs_sha_soc.h"

/**
 * Map the state for the SHA SoC engine.
 * @param sha The SHA engine to use for mapping.
 * @param state The state to map.
 *
 * @return 0 if the mapping was successful or an error code.
 */
static int hash_hs_sha_soc_map (const struct hash_engine_hs_sha_soc *sha,
	struct hs_sha_multi_update **state)
{
	if ((sha == NULL) || (state == NULL) || (sha->dmb == NULL)) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	return sha->dmb->map_soc_address (sha->dmb, sha->soc_state_address,
		sizeof (struct hs_sha_multi_update), HSP_DMB_ACCESS_WRITE, (void**) state);
}

/**
 * Unmap the state for the SHA SoC engine.
 * @param sha The SHA engine to use for unmapping.
 * @param state The state to unmap.
 */
static void hash_hs_sha_soc_unmap (const struct hash_engine_hs_sha_soc *sha,
	struct hs_sha_multi_update *state)
{
	if ((sha == NULL) || (state == NULL) || (sha->dmb == NULL)) {
		return;
	}

	sha->dmb->unmap_soc_address (sha->dmb, state);
}

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
static int hash_hs_sha_soc_calculate_hash (const struct hash_engine_hs_sha_soc *sha,
	const uint8_t *data, size_t length, enum hash_type type, uint8_t *hash, size_t hash_length)
{
	int status;
	struct hs_sha_multi_update *state;

	if ((sha == NULL) || ((data == NULL) && (length != 0)) || (hash == NULL)) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	status = hash_hs_sha_soc_map (sha, &state);
	if (status != 0) {
		return status;
	}

	if (state->active != HASH_ACTIVE_NONE) {
		status = HASH_ENGINE_HASH_IN_PROGRESS;
		goto exit;
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

exit:
	hash_hs_sha_soc_unmap (sha, state);

	return status;
}

#ifdef HASH_ENABLE_SHA1
int hash_hs_sha_soc_calculate_sha1 (const struct hash_engine *engine, const uint8_t *data,
	size_t length, uint8_t *hash, size_t hash_length)
{
	return hash_hs_sha_soc_calculate_hash ((const struct hash_engine_hs_sha_soc*) engine, data,
		length, HASH_TYPE_SHA1, hash, hash_length);
}

int hash_hs_sha_soc_start_sha1 (const struct hash_engine *engine)
{
	const struct hash_engine_hs_sha_soc *sha = (const struct hash_engine_hs_sha_soc*) engine;
	struct hs_sha_multi_update *state;
	int status;

	if (sha == NULL) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	status = hash_hs_sha_soc_map (sha, &state);
	if (status != 0) {
		return status;
	}

	if (state->active != HASH_ACTIVE_NONE) {
		status = HASH_ENGINE_HASH_IN_PROGRESS;
		goto exit;
	}

	hs_sha_multi_update_start_sha1 (state);

exit:
	hash_hs_sha_soc_unmap (sha, state);

	return status;
}
#endif

int hash_hs_sha_soc_calculate_sha256 (const struct hash_engine *engine, const uint8_t *data,
	size_t length, uint8_t *hash, size_t hash_length)
{
	return hash_hs_sha_soc_calculate_hash ((const struct hash_engine_hs_sha_soc*) engine, data,
		length, HASH_TYPE_SHA256, hash, hash_length);
}

int hash_hs_sha_soc_start_sha256 (const struct hash_engine *engine)
{
	const struct hash_engine_hs_sha_soc *sha = (const struct hash_engine_hs_sha_soc*) engine;
	struct hs_sha_multi_update *state;
	int status;

	if (sha == NULL) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	status = hash_hs_sha_soc_map (sha, &state);
	if (status != 0) {
		return status;
	}

	if (state->active != HASH_ACTIVE_NONE) {
		status = HASH_ENGINE_HASH_IN_PROGRESS;
		goto exit;
	}

	hs_sha_multi_update_start_sha256 (state);

exit:
	hash_hs_sha_soc_unmap (sha, state);

	return status;
}

#ifdef HASH_ENABLE_SHA384
int hash_hs_sha_soc_calculate_sha384 (const struct hash_engine *engine, const uint8_t *data,
	size_t length, uint8_t *hash, size_t hash_length)
{
	return hash_hs_sha_soc_calculate_hash ((const struct hash_engine_hs_sha_soc*) engine, data,
		length, HASH_TYPE_SHA384, hash, hash_length);
}

int hash_hs_sha_soc_start_sha384 (const struct hash_engine *engine)
{
	const struct hash_engine_hs_sha_soc *sha = (const struct hash_engine_hs_sha_soc*) engine;
	struct hs_sha_multi_update *state;
	int status;

	if (sha == NULL) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	status = hash_hs_sha_soc_map (sha, &state);
	if (status != 0) {
		return status;
	}

	if (state->active != HASH_ACTIVE_NONE) {
		status = HASH_ENGINE_HASH_IN_PROGRESS;
		goto exit;
	}

	hs_sha_multi_update_start_sha384 (state);

exit:
	hash_hs_sha_soc_unmap (sha, state);

	return status;
}
#endif

#ifdef HASH_ENABLE_SHA512
int hash_hs_sha_soc_calculate_sha512 (const struct hash_engine *engine, const uint8_t *data,
	size_t length, uint8_t *hash, size_t hash_length)
{
	return hash_hs_sha_soc_calculate_hash ((const struct hash_engine_hs_sha_soc*) engine, data,
		length, HASH_TYPE_SHA512, hash, hash_length);
}

int hash_hs_sha_soc_start_sha512 (const struct hash_engine *engine)
{
	const struct hash_engine_hs_sha_soc *sha = (const struct hash_engine_hs_sha_soc*) engine;
	struct hs_sha_multi_update *state;
	int status;

	if (sha == NULL) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	status = hash_hs_sha_soc_map (sha, &state);
	if (status != 0) {
		return status;
	}

	if (state->active != HASH_ACTIVE_NONE) {
		status = HASH_ENGINE_HASH_IN_PROGRESS;
		goto exit;
	}

	hs_sha_multi_update_start_sha512 (state);

exit:
	hash_hs_sha_soc_unmap (sha, state);

	return status;
}
#endif

enum hash_type hash_hs_sha_soc_get_active_algorithm (const struct hash_engine *engine)
{
	const struct hash_engine_hs_sha_soc *sha = (const struct hash_engine_hs_sha_soc*) engine;
	int status;
	struct hs_sha_multi_update *state;

	if (sha == NULL) {
		return HASH_TYPE_INVALID;
	}

	status = hash_hs_sha_soc_map (sha, &state);
	if (status != 0) {
		return HASH_TYPE_INVALID;
	}

	status = hash_get_type_from_active (state->active);

	hash_hs_sha_soc_unmap (sha, state);

	return status;
}

int hash_hs_sha_soc_update (const struct hash_engine *engine, const uint8_t *data, size_t length)
{
	const struct hash_engine_hs_sha_soc *sha = (const struct hash_engine_hs_sha_soc*) engine;
	int status;
	struct hs_sha_multi_update *state;

	if ((sha == NULL) || ((data == NULL) && (length != 0))) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	status = hash_hs_sha_soc_map (sha, &state);
	if (status != 0) {
		return status;
	}

	if (state->active == HASH_ACTIVE_NONE) {
		status = HASH_ENGINE_NO_ACTIVE_HASH;
		goto exit;
	}

	status = hs_sha_multi_update_update_digest (sha->hw, state, data, length);

exit:
	hash_hs_sha_soc_unmap (sha, state);

	return status;
}

int hash_hs_sha_soc_get_hash (const struct hash_engine *engine, uint8_t *hash, size_t hash_length)
{
	const struct hash_engine_hs_sha_soc *sha = (const struct hash_engine_hs_sha_soc*) engine;
	int status;
	struct hs_sha_multi_update *state;

	if ((sha == NULL) || (hash == NULL)) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	status = hash_hs_sha_soc_map (sha, &state);
	if (status != 0) {
		return status;
	}

	if (state->active == HASH_ACTIVE_NONE) {
		status = HASH_ENGINE_NO_ACTIVE_HASH;
		goto exit;
	}

	status = hs_sha_multi_update_get_digest (sha->hw, state, hash, hash_length);
	if (ROT_IS_ERROR (status)) {
		if (status == HS_SHA_DIGEST_BUFFER_TOO_SMALL) {
			status = HASH_ENGINE_HASH_BUFFER_TOO_SMALL;
		}
	}
	else {
		status = 0;
	}

exit:
	hash_hs_sha_soc_unmap (sha, state);

	return status;
}

void hash_hs_sha_soc_cancel (const struct hash_engine *engine)
{
	const struct hash_engine_hs_sha_soc *sha = (const struct hash_engine_hs_sha_soc*) engine;
	int status;
	struct hs_sha_multi_update *state;

	if (sha) {
		status = hash_hs_sha_soc_map (sha, &state);
		if (status != 0) {
			return;
		}

		hs_sha_multi_update_init (state);
		hash_hs_sha_soc_unmap (sha, state);
	}
}

int hash_hs_sha_soc_finish (const struct hash_engine *engine, uint8_t *hash, size_t hash_length)
{
	const struct hash_engine_hs_sha_soc *sha = (const struct hash_engine_hs_sha_soc*) engine;
	int status;
	struct hs_sha_multi_update *state;

	if ((sha == NULL) || (hash == NULL)) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	status = hash_hs_sha_soc_map (sha, &state);
	if (status != 0) {
		return status;
	}

	if (state->active == HASH_ACTIVE_NONE) {
		status = HASH_ENGINE_NO_ACTIVE_HASH;
		goto exit;
	}

	status = hs_sha_multi_update_finish_digest (sha->hw, state, hash, hash_length);
	if (ROT_IS_ERROR (status)) {
		if (status == HS_SHA_DIGEST_BUFFER_TOO_SMALL) {
			status = HASH_ENGINE_HASH_BUFFER_TOO_SMALL;
		}
	}
	else {
		status = 0;
	}

exit:
	hash_hs_sha_soc_unmap (sha, state);

	return status;
}

/**
 * Initialize a HS-SHA SOC hash engine.
 *
 * @param engine The hashing engine to initialize.
 * @param soc_state_address The address in SOC memory where the state for this engine will be stored.
 * @param dmb Driver for accessing the SOC memory.
 * @param hw Driver for the specific HW block that will be used for hashing operations.  This driver
 * can be shared between multiple hash engine instances.
 *
 * @return 0 if the hash engine was successfully initialized or an error code.
 */
int hash_hs_sha_soc_init (struct hash_engine_hs_sha_soc *engine, uint64_t soc_state_address,
	const struct hsp_dmb *dmb, const struct hs_sha *hw)
{
	if ((engine == NULL) || (dmb == NULL) || (hw == NULL)) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	memset (engine, 0, sizeof (struct hash_engine_hs_sha_soc));

#ifdef HASH_ENABLE_SHA1
	engine->base.calculate_sha1 = hash_hs_sha_soc_calculate_sha1;
	engine->base.start_sha1 = hash_hs_sha_soc_start_sha1;
#endif
	engine->base.calculate_sha256 = hash_hs_sha_soc_calculate_sha256;
	engine->base.start_sha256 = hash_hs_sha_soc_start_sha256;
#ifdef HASH_ENABLE_SHA384
	engine->base.calculate_sha384 = hash_hs_sha_soc_calculate_sha384;
	engine->base.start_sha384 = hash_hs_sha_soc_start_sha384;
#endif
#ifdef HASH_ENABLE_SHA512
	engine->base.calculate_sha512 = hash_hs_sha_soc_calculate_sha512;
	engine->base.start_sha512 = hash_hs_sha_soc_start_sha512;
#endif
	engine->base.get_active_algorithm = hash_hs_sha_soc_get_active_algorithm;
	engine->base.update = hash_hs_sha_soc_update;
	engine->base.get_hash = hash_hs_sha_soc_get_hash;
	engine->base.finish = hash_hs_sha_soc_finish;
	engine->base.cancel = hash_hs_sha_soc_cancel;

	engine->dmb = dmb;
	engine->hw = hw;
	engine->soc_state_address = soc_state_address;

	return hash_hs_sha_soc_init_state (engine);
}

/**
 * Initialize only the variable state for an HS-SHA SOC hash engine.  The rest of the instance is
 * assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param engine The hash engine that contains the state to initialize.
 *
 * @return 0 if the driver state was successfully initialized or an error code.
 */
int hash_hs_sha_soc_init_state (const struct hash_engine_hs_sha_soc *engine)
{
	int status;
	struct hs_sha_multi_update *state;

	if ((engine == NULL) || (engine->dmb == NULL) || (engine->hw == NULL)) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	status = hash_hs_sha_soc_map (engine, &state);
	if (status != 0) {
		return status;
	}

	hs_sha_multi_update_init (state);

	hash_hs_sha_soc_unmap (engine, state);

	return 0;
}

/**
 * Release the resources used by a HS-SHA hash engine.
 *
 * @param engine The hashing engine to release.
 */
int hash_hs_sha_soc_release (const struct hash_engine_hs_sha_soc *engine)
{
	int status;
	struct hs_sha_multi_update *state;

	if (engine == NULL) {
		return HASH_ENGINE_INVALID_ARGUMENT;
	}

	/* Be sure any active hash operation is cleared. */
	status = hash_hs_sha_soc_map (engine, &state);
	if (status != 0) {
		return status;
	}

	hs_sha_multi_update_init (state);
	hash_hs_sha_soc_unmap (engine, state);

	return 0;
}
