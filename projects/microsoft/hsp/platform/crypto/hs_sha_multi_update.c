// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "hs_sha_multi_update.h"
#include "common/common_math.h"


/**
 * Initialize the context for a HS-SHA multi-update hash operation.  All data in the context will be
 * erased and will be ready to start a new operation.
 *
 * @param context The hash context to initialize.  If this is null, no operation will be performed.
 */
void hs_sha_multi_update_init (struct hs_sha_multi_update *context)
{
	if (context != NULL) {
		memset (context, 0, sizeof (*context));

		context->active = HASH_ACTIVE_NONE;
	}
}

/**
 * Start a multi-update SHA-1 operation.
 *
 * Do not call this function if there an active operation without first calling
 * hs_sha_multi_update_init().
 *
 * @param context The hash context to use for the operation.  This is cannot be null and is not
 * checked for validity.
 */
void hs_sha_multi_update_start_sha1 (struct hs_sha_multi_update *context)
{
	context->active = HASH_ACTIVE_SHA1;
	context->block_size = SHA1_BLOCK_SIZE;
}

/**
 * Start a multi-update SHA-256 operation.
 *
 * Do not call this function if there an active operation without first calling
 * hs_sha_multi_update_init().
 *
 * @param context The hash context to use for the operation.  This is cannot be null and is not
 * checked for validity.
 */
void hs_sha_multi_update_start_sha256 (struct hs_sha_multi_update *context)
{
	context->active = HASH_ACTIVE_SHA256;
	context->block_size = SHA256_BLOCK_SIZE;
}

/**
 * Start a multi-update SHA-384 operation.
 *
 * Do not call this function if there an active operation without first calling
 * hs_sha_multi_update_init().
 *
 * @param context The hash context to use for the operation.  This is cannot be null and is not
 * checked for validity.
 */
void hs_sha_multi_update_start_sha384 (struct hs_sha_multi_update *context)
{
	context->active = HASH_ACTIVE_SHA384;
	context->block_size = SHA384_BLOCK_SIZE;
}

/**
 * Start a multi-update SHA-512 operation.
 *
 * Do not call this function if there an active operation without first calling
 * hs_sha_multi_update_init().
 *
 * @param context The hash context to use for the operation.  This is cannot be null and is not
 * checked for validity.
 */
void hs_sha_multi_update_start_sha512 (struct hs_sha_multi_update *context)
{
	context->active = HASH_ACTIVE_SHA512;
	context->block_size = SHA512_BLOCK_SIZE;
}

/**
 * Update the current digest calculation with additional data.
 *
 * @param sha Driver for the HS-SHA used for digest calculation.
 * @param context The hash context to update.  This cannot be null and is not checked for validity.
 * @param data Data to add to the digest calculation.  If length is non-zero, this cannot be null
 * and is not checked for validity.
 * @param length Length of the data.
 *
 * @return 0 if the digest calculation was updated successfully or an error code.
 */
int hs_sha_multi_update_update_digest (const struct hs_sha *sha,
	struct hs_sha_multi_update *context, const uint8_t *data, size_t length)
{
	size_t block_bytes;
	int status;

	/* First, deal with any partial blocks that may have already been cached or if the input does
	 * not itself fill a whole block. */
	if ((context->byte_count != 0) || (length < context->block_size)) {
		block_bytes = min (length, context->block_size - context->byte_count);

		if (data != NULL) {
			memcpy (&context->block[context->byte_count], data, block_bytes);
		}
		context->byte_count += block_bytes;
		length -= block_bytes;
		data += block_bytes;

		/* If we now have a full block of data, hash it. */
		if (context->byte_count == context->block_size) {
			status = hs_sha_update_digest (sha, context->block, context->byte_count,
				(context->total_count != 0) ? &context->hw_context : NULL,
				(enum hash_type) context->active, &context->hw_context);
			if (status != 0) {
				return status;
			}

			context->total_count += context->byte_count;
			context->byte_count = 0;
		}
	}

	/* Next, hash a contiguous region of block-aligned data. */
	if (length >= context->block_size) {
		block_bytes = HS_SHA_BLOCK_ALIGN (length, context->block_size);

		status = hs_sha_update_digest (sha, data, block_bytes,
			(context->total_count != 0) ? &context->hw_context : NULL,
			(enum hash_type) context->active, &context->hw_context);
		if (status != 0) {
			return status;
		}

		context->total_count += block_bytes;
		length -= block_bytes;
		data += block_bytes;
	}

	/* Last, cache any leftover data that was not block aligned. */
	if (length != 0) {
		memcpy (context->block, data, length);
		context->byte_count = length;
	}

	return 0;
}

/**
 * Get the calculated digest, leaving the context open for additional updates.
 *
 * @param sha Driver for the HS-SHA used for digest calculation.
 * @param context The hash context for the digest.  This cannot be null and is not checked for
 * validity.
 * @param digest Output for the calculated digest.
 * @param digest_length Length of the digest buffer.
 *
 * @return Length of the calculated digest or an error code.  Use ROT_IS_ERROR to check the return
 * value.
 */
int hs_sha_multi_update_get_digest (const struct hs_sha *sha, struct hs_sha_multi_update *context,
	uint8_t *digest, size_t digest_length)
{
	size_t total_bytes;

	total_bytes = context->total_count + context->byte_count;

	return hs_sha_finish_digest (sha, context->block, context->byte_count, total_bytes,
		(context->total_count != 0) ? &context->hw_context : NULL, (enum hash_type) context->active,
		digest, digest_length);
}

/**
 * Get the calculated digest, finalizing the context so that it will not accept any more updates.
 *
 * Upon successful return from this call, the context will be reinitialized, ready to start a new
 * operation.
 *
 * @param sha Driver for the HS-SHA used for digest calculation.
 * @param context The hash context for the digest.  This cannot be null and is not checked for
 * validity.
 * @param digest Output for the calculated digest.
 * @param digest_length Length of the digest buffer.
 *
 * @return Length of the calculated digest or an error code.  Use ROT_IS_ERROR to check the return
 * value.
 */
int hs_sha_multi_update_finish_digest (const struct hs_sha *sha,
	struct hs_sha_multi_update *context, uint8_t *digest, size_t digest_length)
{
	int status;

	status = hs_sha_multi_update_get_digest (sha, context, digest, digest_length);
	if (!ROT_IS_ERROR (status)) {
		hs_sha_multi_update_init (context);
	}

	return status;
}
