// Copyright (c) Microsoft Corporation. All rights reserved.

#include "mbedtls_ecc_pka.h"
#include "common/buffer_util.h"
#include "crypto/ecc.h"


/**
 * Generate a random ECC key using PKA.
 *
 * @param grp The curve for the new key.
 * @param d Output for the private key.
 * @param Q Output for the public key.
 *
 * @return 0 if the key was generated successfully or a negative error code.
 */
int mbedtls_ecc_pka_genkey (mbedtls_ecp_group *grp, mbedtls_mpi *d, mbedtls_ecp_point *Q)
{
	struct ecc_raw_private_key priv_key = {0};
	struct ecc_point_public_key pub_key = {0};
	int status;

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

	status = mbedtls_ecc_pka->base.generate_ecc_key_pair (&mbedtls_ecc_pka->base,
		priv_key.key_length, priv_key.d, &pub_key);
	if (status != 0) {
		status = -status;
		goto exit;
	}

	status = mbedtls_mpi_read_binary (d, priv_key.d, priv_key.key_length);
	if (status != 0) {
		goto exit;
	}

	status = mbedtls_mpi_read_binary (&Q->MBEDTLS_PRIVATE (X), pub_key.x, pub_key.key_length);
	if (status != 0) {
		goto exit;
	}

	status = mbedtls_mpi_read_binary (&Q->MBEDTLS_PRIVATE (Y), pub_key.y, pub_key.key_length);
	if (status != 0) {
		goto exit;
	}

exit:
	buffer_zeroize (&priv_key, sizeof (priv_key));
	buffer_zeroize (&pub_key, sizeof (pub_key));

	return status;
}
