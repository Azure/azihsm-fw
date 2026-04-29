// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_CRYPTO_H_
#define INIT_CRYPTO_H_

#include "asn1/x509_thread_safe_static.h"
#include "crypto/aes_gcm_mbedtls_static.h"
#include "crypto/aes_key_wrap_with_padding_static.h"
#include "crypto/ecc_hw_pka_static.h"
#include "crypto/ecc_thread_safe_static.h"
#include "crypto/hash_hs_sha_static.h"
#include "crypto/hash_thread_safe_static.h"
#include "crypto/hkdf_static.h"
#include "crypto/rng_thread_safe_static.h"
#include "crypto/rsa_thread_safe_static.h"
#include "drivers/ccs_ksu_static.h"
#include "drivers/hs_sha_static.h"
#include "drivers/hsp_aes_static.h"
#include "fips/manticore_error_state_static.h"
#include "fips/periodic_self_test_handler_static.h"

extern const struct hs_sha hash_hw;
extern const struct ecc_hw_pka pka;
extern const struct hsp_aes aes_hw;
extern const struct ccs_ksu ccs;

extern const struct rng_engine_thread_safe shared_rng;
extern const struct hash_engine_thread_safe shared_hash;
extern const struct ecc_engine_thread_safe shared_ecc;
extern const struct rsa_engine_thread_safe shared_rsa;
extern const struct x509_engine_thread_safe shared_x509;

extern const struct periodic_self_test_handler periodic_self_test_handler;
extern const struct manticore_error_state error_state_handler;
extern const struct aes_gcm_engine_mbedtls aes_gcm;
extern const struct aes_key_wrap_with_padding aes_kwp;
extern const struct hkdf hkdf;

extern const struct hash_engine_hs_sha host_hash;


int initialize_crypto_hardware ();
int initialize_system_crypto ();
int run_crypto_self_tests ();

int initialize_error_state_task ();
int start_error_state_task ();


#endif	/* INIT_CRYPTO_H_ */
