// Copyright (c) Microsoft Corporation. All rights reserved.

#include "mbedtls_ecc_pka.h"
#include "common/buffer_util.h"
#include "common/unused.h"
#include "crypto/ecc.h"
#include "mbedtls/ecdsa.h"


#ifdef MBEDTLS_ECDH_GEN_PUBLIC_ALT

int mbedtls_ecdh_gen_public (mbedtls_ecp_group *grp, mbedtls_mpi *d, mbedtls_ecp_point *Q,
	int (*f_rng) (void*, unsigned char*, size_t), void *p_rng)
{
	/* Do not use the RNG provided to the call.  The HW RNG will be used by PKA for key
	 * generation. */
	UNUSED (f_rng);
	UNUSED (p_rng);

	return mbedtls_ecc_pka_genkey (grp, d, Q);
}

#endif	/* MBEDTLS_ECDH_GEN_PUBLIC_ALT */


#ifdef MBEDTLS_ECDH_COMPUTE_SHARED_ALT

int mbedtls_ecdh_compute_shared (mbedtls_ecp_group *grp, mbedtls_mpi *z, const mbedtls_ecp_point *Q,
	const mbedtls_mpi *d, int (*f_rng) (void*, unsigned char*, size_t), void *p_rng)
{
	struct ecc_raw_private_key priv_key = {0};
	struct ecc_point_public_key pub_key = {0};
	uint8_t secret[ECC_MAX_KEY_LENGTH] = {0};
	int status;

	/* The RNG is not needed for this operation. */
	UNUSED (f_rng);
	UNUSED (p_rng);

	/* Only support P-256, P-384, and P-521 curves. */
	switch (grp->id) {
		case MBEDTLS_ECP_DP_SECP256R1:
			priv_key.key_length = ECC_KEY_LENGTH_256;
			pub_key.key_length = ECC_KEY_LENGTH_256;
			break;

#if ECC_MAX_KEY_LENGTH >= ECC_KEY_LENGTH_384
		case MBEDTLS_ECP_DP_SECP384R1:
			priv_key.key_length = ECC_KEY_LENGTH_384;
			pub_key.key_length = ECC_KEY_LENGTH_384;
			break;
#endif

#if ECC_MAX_KEY_LENGTH >= ECC_KEY_LENGTH_521
		case MBEDTLS_ECP_DP_SECP521R1:
			priv_key.key_length = ECC_KEY_LENGTH_521;
			pub_key.key_length = ECC_KEY_LENGTH_521;
			break;
#endif

		default:
			return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
	}

	status = mbedtls_mpi_write_binary (d, priv_key.d, priv_key.key_length);
	if (status != 0) {
		goto exit;
	}

	status = mbedtls_mpi_write_binary (&Q->MBEDTLS_PRIVATE (X), pub_key.x, pub_key.key_length);
	if (status != 0) {
		goto exit;
	}

	status = mbedtls_mpi_write_binary (&Q->MBEDTLS_PRIVATE (Y), pub_key.y, pub_key.key_length);
	if (status != 0) {
		goto exit;
	}

	status = mbedtls_ecc_pka->base.ecdh_compute (&mbedtls_ecc_pka->base, priv_key.d,
		priv_key.key_length, &pub_key, secret, priv_key.key_length);
	if (status != 0) {
		status = -status;
		goto exit;
	}

	status = mbedtls_mpi_read_binary (z, secret, priv_key.key_length);

exit:
	buffer_zeroize (&priv_key, sizeof (priv_key));
	buffer_zeroize (&pub_key, sizeof (pub_key));
	buffer_zeroize (secret, sizeof (secret));

	return status;
}

#endif	/* MBEDTLS_ECDH_COMPUTE_SHARED_ALT */
