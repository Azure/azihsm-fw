// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HS_SHA_MULTI_UPDATE_H_
#define HS_SHA_MULTI_UPDATE_H_

#include "drivers/hs_sha.h"


/**
 * Provides the algorithmic implementation for executing a multi-update hash operation against the
 * HS-SHA hardware.  This structure represents the context used for a single hash operation.
 *
 * For one-shot hash operations, the algorithm is completely implemented in hardware so does not
 * have any software component to it.
 */
struct hs_sha_multi_update {
	uint8_t block[SHA512_BLOCK_SIZE];	/**< Buffer for a single hash block of data. */
	size_t block_size;					/**< Block size for the current hash algorithm. */
	size_t byte_count;					/**< The amount of data in the block data buffer. */
	SP_MSG_512 hw_context;				/**< The current hashing context for the HS-SHA hardware. */
	size_t total_count;					/**< The total number of bytes hashed in the current context. */
	uint8_t active;						/**< The type of hash actively being calculated. */
};


void hs_sha_multi_update_init (struct hs_sha_multi_update *context);

void hs_sha_multi_update_start_sha1 (struct hs_sha_multi_update *context);
void hs_sha_multi_update_start_sha256 (struct hs_sha_multi_update *context);
void hs_sha_multi_update_start_sha384 (struct hs_sha_multi_update *context);
void hs_sha_multi_update_start_sha512 (struct hs_sha_multi_update *context);

int hs_sha_multi_update_update_digest (const struct hs_sha *sha,
	struct hs_sha_multi_update *context, const uint8_t *data, size_t length);
int hs_sha_multi_update_get_digest (const struct hs_sha *sha, struct hs_sha_multi_update *context,
	uint8_t *digest, size_t digest_length);
int hs_sha_multi_update_finish_digest (const struct hs_sha *sha,
	struct hs_sha_multi_update *context, uint8_t *digest, size_t digest_length);


#endif	/* HS_SHA_MULTI_UPDATE_H_ */
