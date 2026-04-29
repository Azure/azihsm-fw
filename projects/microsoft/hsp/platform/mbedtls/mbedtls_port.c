// Copyright (c) Microsoft Corporation. All rights reserved.

#include "common/unused.h"
#include "drivers/hsp_rng_hw.h"
#include "mbedtls/entropy.h"


/**
 * RNG hardware driver to use for seeding mbedTLS software DRBGs.  This must be provided by the
 * final image that integrates mbedTLS.
 */
extern const struct hsp_rng_hw *const mbedtls_entropy;


/* mbedTLS hardware entropy callback function. */
int mbedtls_hardware_poll (void *data, unsigned char *output, size_t len, size_t *out_len)
{
	int status;

	UNUSED (data);

	if ((output == NULL) || (out_len == NULL)) {
		return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
	}

	status = hsp_rng_hw_get_random_buffer (mbedtls_entropy, output, len);
	if (status != 0) {
		return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
	}

	*out_len = len;

	return 0;
}
