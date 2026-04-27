// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MBEDTLS_HS_SHA_H_
#define MBEDTLS_HS_SHA_H_

#include "crypto/hs_sha_multi_update.h"
#include "mbedtls/build_info.h"


#ifdef MBEDTLS_SHA1_ALT

/* Provide the structure definition needed for the mbedtls SHA-1 APIs. */
typedef struct hs_sha_multi_update mbedtls_sha1_context;

#endif	/* MBEDTLS_SHA1_ALT */


#ifdef MBEDTLS_SHA256_ALT

/* Provide the structure definition needed for the mbedtls SHA-256 APIs. */
typedef struct hs_sha_multi_update mbedtls_sha256_context;

#endif	/* MBEDTLS_SHA256_ALT */


#ifdef MBEDTLS_SHA512_ALT

/* Provide the structure definition needed for the mbedtls SHA-512/384 APIs. */
typedef struct hs_sha_multi_update mbedtls_sha512_context;

#endif	/* MBEDTLS_SHA512_ALT */


#endif	/* MBEDTLS_HS_SHA_H_ */
