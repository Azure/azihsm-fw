// Copyright (c) Microsoft Corporation. All rights reserved.

#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"


/**
 * Global singleton that must be provided by the integration for the HS-SHA hardware driver to use
 * with mbedtls calls.  The integration must ensure this is properly initialized.
 */
extern const struct hs_sha *const mbedtls_hs_sha;


#ifdef MBEDTLS_SHA1_ALT

void mbedtls_sha1_init (mbedtls_sha1_context *ctx)
{
	hs_sha_multi_update_init (ctx);
}

void mbedtls_sha1_free (mbedtls_sha1_context *ctx)
{
	hs_sha_multi_update_init (ctx);
}

void mbedtls_sha1_clone (mbedtls_sha1_context *dst, const mbedtls_sha1_context *src)
{
	*dst = *src;
}

int mbedtls_sha1_starts (mbedtls_sha1_context *ctx)
{
	if (ctx == NULL) {
		return MBEDTLS_ERR_SHA1_BAD_INPUT_DATA;
	}

	hs_sha_multi_update_start_sha1 (ctx);

	return 0;
}

int mbedtls_sha1_update (mbedtls_sha1_context *ctx, const unsigned char *input, size_t ilen)
{
	int status;

	if ((ctx == NULL) || ((input == NULL) && (ilen != 0))) {
		return MBEDTLS_ERR_SHA1_BAD_INPUT_DATA;
	}

	if (ctx->active == HASH_ACTIVE_NONE) {
		return MBEDTLS_ERR_SHA1_BAD_INPUT_DATA;
	}

	status = hs_sha_multi_update_update_digest (mbedtls_hs_sha, ctx, input, ilen);
	if (ROT_IS_ERROR (status)) {
		return -status;
	}

	return 0;
}

int mbedtls_sha1_finish (mbedtls_sha1_context *ctx, unsigned char output[20])
{
	int status;

	if ((ctx == NULL) || (output == NULL)) {
		return MBEDTLS_ERR_SHA1_BAD_INPUT_DATA;
	}

	if (ctx->active == HASH_ACTIVE_NONE) {
		return MBEDTLS_ERR_SHA1_BAD_INPUT_DATA;
	}

	status = hs_sha_multi_update_finish_digest (mbedtls_hs_sha, ctx, output, SHA1_HASH_LENGTH);
	if (ROT_IS_ERROR (status)) {
		return -status;
	}

	return 0;
}

#endif	/* MBEDTLS_SHA1_ALT */


#ifdef MBEDTLS_SHA256_ALT

void mbedtls_sha256_init (mbedtls_sha256_context *ctx)
{
	hs_sha_multi_update_init (ctx);
}

void mbedtls_sha256_free (mbedtls_sha256_context *ctx)
{
	hs_sha_multi_update_init (ctx);
}

void mbedtls_sha256_clone (mbedtls_sha256_context *dst, const mbedtls_sha256_context *src)
{
	*dst = *src;
}

int mbedtls_sha256_starts (mbedtls_sha256_context *ctx, int is224)
{
	if (ctx == NULL) {
		return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
	}

	/* Only SHA-256 is supported. */
	if (is224 != 0) {
		return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
	}

	hs_sha_multi_update_start_sha256 (ctx);

	return 0;
}

int mbedtls_sha256_update (mbedtls_sha256_context *ctx, const unsigned char *input, size_t ilen)
{
	int status;

	if ((ctx == NULL) || ((input == NULL) && (ilen != 0))) {
		return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
	}

	if (ctx->active == HASH_ACTIVE_NONE) {
		return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
	}

	status = hs_sha_multi_update_update_digest (mbedtls_hs_sha, ctx, input, ilen);
	if (ROT_IS_ERROR (status)) {
		return -status;
	}

	return 0;
}

int mbedtls_sha256_finish (mbedtls_sha256_context *ctx, unsigned char *output)
{
	int status;

	if ((ctx == NULL) || (output == NULL)) {
		return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
	}

	if (ctx->active == HASH_ACTIVE_NONE) {
		return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
	}

	status = hs_sha_multi_update_finish_digest (mbedtls_hs_sha, ctx, output, SHA256_HASH_LENGTH);
	if (ROT_IS_ERROR (status)) {
		return -status;
	}

	return 0;
}

#endif	/* MBEDTLS_SHA256_ALT */


#ifdef MBEDTLS_SHA512_ALT

void mbedtls_sha512_init (mbedtls_sha512_context *ctx)
{
	hs_sha_multi_update_init (ctx);
}

void mbedtls_sha512_free (mbedtls_sha512_context *ctx)
{
	hs_sha_multi_update_init (ctx);
}

void mbedtls_sha512_clone (mbedtls_sha512_context *dst, const mbedtls_sha512_context *src)
{
	*dst = *src;
}

int mbedtls_sha512_starts (mbedtls_sha512_context *ctx, int is384)
{
	if (ctx == NULL) {
		return MBEDTLS_ERR_SHA512_BAD_INPUT_DATA;
	}

#if defined (MBEDTLS_SHA384_C) && defined (MBEDTLS_SHA512_C)
	if ((is384 != 0) && (is384 != 1)) {
		return MBEDTLS_ERR_SHA512_BAD_INPUT_DATA;
	}
#elif defined (MBEDTLS_SHA512_C)
	if (is384 != 0) {
		return MBEDTLS_ERR_SHA512_BAD_INPUT_DATA;
	}
#else	/* defined MBEDTLS_SHA384_C only */
	if (is384 == 0) {
		return MBEDTLS_ERR_SHA512_BAD_INPUT_DATA;
	}
#endif

	if (is384 == 0) {
		hs_sha_multi_update_start_sha512 (ctx);
	}
	else {
		hs_sha_multi_update_start_sha384 (ctx);
	}

	return 0;
}

int mbedtls_sha512_update (mbedtls_sha512_context *ctx, const unsigned char *input, size_t ilen)
{
	int status;

	if ((ctx == NULL) || ((input == NULL) && (ilen != 0))) {
		return MBEDTLS_ERR_SHA512_BAD_INPUT_DATA;
	}

	if (ctx->active == HASH_ACTIVE_NONE) {
		return MBEDTLS_ERR_SHA512_BAD_INPUT_DATA;
	}

	status = hs_sha_multi_update_update_digest (mbedtls_hs_sha, ctx, input, ilen);
	if (ROT_IS_ERROR (status)) {
		return -status;
	}

	return 0;
}

int mbedtls_sha512_finish (mbedtls_sha512_context *ctx, unsigned char *output)
{
	int status;

	if ((ctx == NULL) || (output == NULL)) {
		return MBEDTLS_ERR_SHA512_BAD_INPUT_DATA;
	}

	if (ctx->active == HASH_ACTIVE_NONE) {
		return MBEDTLS_ERR_SHA512_BAD_INPUT_DATA;
	}

	status = hs_sha_multi_update_finish_digest (mbedtls_hs_sha, ctx, output,
		(ctx->active == HASH_TYPE_SHA384) ? SHA384_HASH_LENGTH : SHA512_HASH_LENGTH);
	if (ROT_IS_ERROR (status)) {
		return -status;
	}

	return 0;
}

#endif	/* MBEDTLS_SHA512_ALT */
