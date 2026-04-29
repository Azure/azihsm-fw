// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MBEDTLS_ECC_PKA_H_
#define MBEDTLS_ECC_PKA_H_

#include "crypto/ecc_hw_pka.h"
#include "mbedtls/ecp.h"


/**
 * Global singleton that must be provided by the integration for the PKA hardware driver to use
 * with mbedtls calls.  The integration must ensure this is properly initialized.
 */
extern const struct ecc_hw_pka *const mbedtls_ecc_pka;


int mbedtls_ecc_pka_genkey (mbedtls_ecp_group *grp, mbedtls_mpi *d, mbedtls_ecp_point *Q);


#endif	/* MBEDTLS_ECC_PKA_H_ */
