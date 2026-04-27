// Copyright (c) Microsoft Corporation. All rights reserved.

#include "mbedtls_ecc_pka.h"
#include "common/buffer_util.h"
#include "common/unused.h"
#include "crypto/ecc.h"
#include "crypto/ecc_hw_pka.h"
#include "mbedtls/ecdsa.h"


#ifdef MBEDTLS_ECDSA_VERIFY_ALT

int mbedtls_ecdsa_verify (mbedtls_ecp_group *grp, const unsigned char *buf, size_t blen,
	const mbedtls_ecp_point *Q, const mbedtls_mpi *r, const mbedtls_mpi *s)
{
	struct ecc_point_public_key pub_key = {0};
	struct ecc_ecdsa_signature signature = {0};
	int status;

	/* Only support P-256, P-384, and P-521 curves. */
	switch (grp->id) {
		case MBEDTLS_ECP_DP_SECP256R1:
			pub_key.key_length = ECC_KEY_LENGTH_256;
			signature.length = ECC_KEY_LENGTH_256;
			break;

#if ECC_MAX_KEY_LENGTH >= ECC_KEY_LENGTH_384
		case MBEDTLS_ECP_DP_SECP384R1:
			pub_key.key_length = ECC_KEY_LENGTH_384;
			signature.length = ECC_KEY_LENGTH_384;
			break;
#endif

#if ECC_MAX_KEY_LENGTH >= ECC_KEY_LENGTH_521
		case MBEDTLS_ECP_DP_SECP521R1:
			pub_key.key_length = ECC_KEY_LENGTH_521;
			signature.length = ECC_KEY_LENGTH_521;
			break;
#endif

		default:
			return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
	}

	/* Must use affine coordinates */
	if (mbedtls_mpi_cmp_int (&Q->MBEDTLS_PRIVATE (Z), 1) != 0) {
		return MBEDTLS_ERR_ECP_INVALID_KEY;
	}

	status = mbedtls_mpi_write_binary (&Q->MBEDTLS_PRIVATE (X), pub_key.x, pub_key.key_length);
	if (status != 0) {
		goto exit;
	}

	status = mbedtls_mpi_write_binary (&Q->MBEDTLS_PRIVATE (Y), pub_key.y, pub_key.key_length);
	if (status != 0) {
		goto exit;
	}

	status = mbedtls_mpi_write_binary (r, signature.r, signature.length);
	if (status != 0) {
		goto exit;
	}

	status = mbedtls_mpi_write_binary (s, signature.s, signature.length);
	if (status != 0) {
		goto exit;
	}

	status = mbedtls_ecc_pka->base.ecdsa_verify (&mbedtls_ecc_pka->base, &pub_key, &signature, buf,
		blen);
	if (status != 0) {
		if (status == ECC_HW_ECDSA_BAD_SIGNATURE) {
			status = MBEDTLS_ERR_ECP_VERIFY_FAILED;
		}
		else {
			status = -status;
		}
	}

exit:
	buffer_zeroize (&pub_key, sizeof (pub_key));
	buffer_zeroize (&signature, sizeof (signature));

	return status;
}

#endif	/* MBEDTLS_ECDSA_VERIFY_ALT */


#ifdef MBEDTLS_ECDSA_SIGN_ALT

int mbedtls_ecdsa_sign (mbedtls_ecp_group *grp, mbedtls_mpi *r, mbedtls_mpi *s,
	const mbedtls_mpi *d, const unsigned char *buf, size_t blen,
	int (*f_rng) (void*, unsigned char*, size_t), void *p_rng)
{
	struct ecc_raw_private_key priv_key = {0};
	struct ecc_ecdsa_signature signature = {0};
	int status;

	/* Do not use the RNG provided to the call.  The HW RNG will be used by PKA for the signing
	 * operation. */
	UNUSED (f_rng);
	UNUSED (p_rng);

	/* Only support P-256, P-384, and P-521 curves. */
	switch (grp->id) {
		case MBEDTLS_ECP_DP_SECP256R1:
			priv_key.key_length = ECC_KEY_LENGTH_256;
			break;

#if ECC_MAX_KEY_LENGTH >= ECC_KEY_LENGTH_384
		case MBEDTLS_ECP_DP_SECP384R1:
			priv_key.key_length = ECC_KEY_LENGTH_384;
			break;
#endif

#if ECC_MAX_KEY_LENGTH >= ECC_KEY_LENGTH_521
		case MBEDTLS_ECP_DP_SECP521R1:
			priv_key.key_length = ECC_KEY_LENGTH_521;
			break;
#endif

		default:
			return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
	}

	status = mbedtls_mpi_write_binary (d, priv_key.d, priv_key.key_length);
	if (status != 0) {
		goto exit;
	}

	status = mbedtls_ecc_pka->base.ecdsa_sign (&mbedtls_ecc_pka->base, priv_key.d,
		priv_key.key_length, buf, blen, NULL, &signature);
	if (status != 0) {
		status = -status;
		goto exit;
	}

	status = mbedtls_mpi_read_binary (r, signature.r, signature.length);
	if (status != 0) {
		goto exit;
	}

	status = mbedtls_mpi_read_binary (s, signature.s, signature.length);

exit:
	buffer_zeroize (&priv_key, sizeof (priv_key));
	buffer_zeroize (&signature, sizeof (signature));

	return status;
}

#endif	/* MBEDTLS_ECDSA_SIGN_ALT */


#ifdef MBEDTLS_ECDSA_GENKEY_ALT

int mbedtls_ecdsa_genkey (mbedtls_ecdsa_context *ctx, mbedtls_ecp_group_id gid,
	int (*f_rng) (void*, unsigned char*, size_t), void *p_rng)
{
	int status;

	/* Do not use the RNG provided to the call.  The HW RNG will be used by PKA for key
	 * generation. */
	UNUSED (f_rng);
	UNUSED (p_rng);

	status = mbedtls_ecp_group_load (&ctx->MBEDTLS_PRIVATE (grp), gid);
	if (status != 0) {
		return status;
	}

	return mbedtls_ecc_pka_genkey (&ctx->MBEDTLS_PRIVATE (grp), &ctx->MBEDTLS_PRIVATE (d),
		&ctx->MBEDTLS_PRIVATE (Q));
}

#endif	/* MBEDTLS_ECDSA_GENKEY_ALT */
