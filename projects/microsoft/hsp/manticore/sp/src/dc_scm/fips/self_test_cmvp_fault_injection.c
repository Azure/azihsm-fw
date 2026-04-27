// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "self_test_cmvp_fault_injection.h"
#include "sp_boot.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "crypto/kat/aes_kat_vectors.h"
#include "crypto/kat/aes_key_wrap_kat_vectors.h"
#include "crypto/kat/ecc_kat_vectors.h"
#include "crypto/kat/hash_kat_vectors.h"
#include "crypto/kat/kdf_kat_vectors.h"
#include "drivers/kat/ccs_ksu_kat_vectors.h"
#include "drivers/kat/hsp_rng_hw_kat_vectors.h"
#include "fips/cmvp_test_case.h"
#include "fips/self_test_manticore.h"


/**
 * REPCNT value for RNG RCT fault inject test.
 */
#define RNG_RCT_TEST_FAULT_INJECTION_REPCNT_CUTOFF		8
/**
 * RESEED value for RNG RCT fault inject test.
 */
#define RNG_RCT_TEST_FAULT_INJECTION_RESEED_INTERVAL	2
/**
 * APT cutoff value for RNG APT fault inject test.
 */
#define RNG_APT_TEST_FAULT_INJECTION_APT_CUTOFF			10
/**
 * RESEED value for RNG APT fault inject test.
 */
#define RNG_APT_TEST_FAULT_INJECTION_RESEED_INTERVAL	2

/**
 * Set the RCT and reseed intervals to low values for the test.
 *
 * @param[in] cmvp The CMVP fault injection instance.
 */
static void self_test_cmvp_fault_injection_rng_rct_test (
	const struct self_test_cmvp_fault_injection *cmvp)
{
	/* Set the RCT and reseed intervals to low values for the test. */
	cmvp->rng->regs->repcnt_cutoff = RNG_RCT_TEST_FAULT_INJECTION_REPCNT_CUTOFF;
	cmvp->rng->regs->reseed_interval = RNG_RCT_TEST_FAULT_INJECTION_RESEED_INTERVAL;
}

/**
 * Set the APT and reseed intervals to low values for the test.
 *
 * @param[in] cmvp The CMVP fault injection instance.
 */
static void self_test_cmvp_fault_injection_rng_apt_test (
	const struct self_test_cmvp_fault_injection *cmvp)
{
	/* Set the RCT and reseed intervals to low values for the test. */
	cmvp->rng->regs->apt_cutoff = RNG_APT_TEST_FAULT_INJECTION_APT_CUTOFF;
	cmvp->rng->regs->reseed_interval = RNG_APT_TEST_FAULT_INJECTION_RESEED_INTERVAL;
}

int self_test_cmvp_fault_injection_run_self_test_first (const struct self_test_interface *self_test,
	struct debug_log_entry_info *error_info)
{
	const struct self_test_cmvp_fault_injection *cmvp = TO_DERIVED_TYPE (self_test,
		const struct self_test_cmvp_fault_injection, base_first);
	enum cmvp_test_case_cast_algorithm algo = CMVP_TEST_CASE_ALGORITHM_NONE;
	uint8_t *corrupt = NULL;

	UNUSED (self_test);
	UNUSED (error_info);

	if ((cmvp_test != 0) &&
		(cmvp_test_case_get_test_type (cmvp_test) == CMVP_TEST_CASE_CAST_NEGATIVE_TEST) &&
		(cmvp_test_case_get_cast_type (cmvp_test) == CMVP_TEST_CASE_CAST_TYPE_PERIODIC)) {
		algo = cmvp_test_case_get_cast_algorithm (cmvp_test);
	}

	switch (algo) {
		/* HSP RNG */
		case CMVP_TEST_CASE_ALGORITHM_RNG_APT_HEALTH:
			self_test_cmvp_fault_injection_rng_apt_test (cmvp);
			clear_cmvp_test_case ();
			break;

		case CMVP_TEST_CASE_ALGORITHM_RNG_RCT_HEALTH:
			self_test_cmvp_fault_injection_rng_rct_test (cmvp);
			clear_cmvp_test_case ();
			break;

		/* HSP HW DRBG */
		case CMVP_TEST_CASE_ALGORITHM_DRBG_INSTANTIATE_HW:
			corrupt = (uint8_t*) &RNG_HSP_HW_KAT_INSTANTIATE_INPUT.AsBytes[16];
			break;

		case CMVP_TEST_CASE_ALGORITHM_DRBG_RESEED_HW:
			corrupt = (uint8_t*) &RNG_HSP_HW_KAT_RESEED_INPUT.AsBytes[16];
			break;

		/* AES-GCM encrypt */
		case CMVP_TEST_CASE_ALGORITHM_AES_ENCRYPT_SW:
			corrupt = (uint8_t*) &AES_GCM_KAT_VECTORS_256_KEY[16];
			break;

		/* AES key wrap with padding */
		case CMVP_TEST_CASE_ALGORITHM_AES_KWP_WRAP_SW:
			corrupt = (uint8_t*) &AES_KEY_WRAP_KAT_VECTORS_KWP_KEY[16];
			break;

		/* CCS HMAC-SHA-256 */
		case CMVP_TEST_CASE_ALGORITHM_HMAC_HW:
			corrupt = (uint8_t*) &CCS_KSU_KAT_VECTORS_HMAC_SHA256_KEY.AsBytes[16];
			break;

		/* CCS KBKDF HMAC-SHA-384 */
		case CMVP_TEST_CASE_ALGORITHM_KBKDF_HW:
			corrupt = (uint8_t*) &CCS_KSU_KAT_VECTORS_HMAC_SHA384_KEY.AsBytes[16];
			break;

		/* SHA-512 */
		case CMVP_TEST_CASE_ALGORITHM_SHA_HW_SW:
			corrupt = (uint8_t*) &SHA_KAT_VECTORS_UPDATE_DATA_2[16];
			break;

		/* HKDF HMAC-SHA-256 */
		case CMVP_TEST_CASE_ALGORITHM_HKDF_SW:
			corrupt = (uint8_t*) &KDF_KAT_VECTORS_HKDF_EXTRACT_IKM[16];
			break;

		/* ECDSA verify (ROM) */
		case CMVP_TEST_CASE_ALGORITHM_ECDSA_VERIFY_ROM:
			corrupt = (uint8_t*) &ECC_KAT_VECTORS_P384_ECC_PUBLIC.x[16];
			break;

		/* ECDSA sign (SW) */
		case CMVP_TEST_CASE_ALGORITHM_ECDSA_SIGN_SW:
			corrupt = (uint8_t*) &ECC_KAT_VECTORS_P384_ECC_PRIVATE_DER[16];
			break;

		default:
			corrupt = NULL;
			break;
	}

	if (corrupt != NULL) {
		*corrupt ^= 0xff;
		clear_cmvp_test_case ();
	}

	/* Configure the firmware to trigger a PCT failure, if requested. */
	trigger_cmvp_pct_failure ();

	return 0;
}

int self_test_cmvp_fault_injection_run_self_test_second (
	const struct self_test_interface *self_test, struct debug_log_entry_info *error_info)
{
	enum cmvp_test_case_cast_algorithm algo = CMVP_TEST_CASE_ALGORITHM_NONE;
	uint8_t *corrupt = NULL;

	UNUSED (self_test);
	UNUSED (error_info);

	if ((cmvp_test != 0) &&
		(cmvp_test_case_get_test_type (cmvp_test) == CMVP_TEST_CASE_CAST_NEGATIVE_TEST) &&
		(cmvp_test_case_get_cast_type (cmvp_test) == CMVP_TEST_CASE_CAST_TYPE_PERIODIC)) {
		algo = cmvp_test_case_get_cast_algorithm (cmvp_test);
	}

	switch (algo) {
		/* AES-GCM decrypt */
		case CMVP_TEST_CASE_ALGORITHM_AES_DECRYPT_SW:
			corrupt = (uint8_t*) &AES_GCM_KAT_VECTORS_256_KEY[16];
			break;

		/* AES key unwrap */
		case CMVP_TEST_CASE_ALGORITHM_AES_KWP_UNWRAP_SW:
			corrupt = (uint8_t*) &AES_KEY_WRAP_KAT_VECTORS_KWP_KEY[16];
			break;

		/* ECDSA verify (SW) */
		case CMVP_TEST_CASE_ALGORITHM_ECDSA_VERIFY_SW:
			corrupt = (uint8_t*) &ECC_KAT_VECTORS_P384_SHA384_ECDSA_SIGNATURE_DER[16];
			break;

		/* ECDH */
		case CMVP_TEST_CASE_ALGORITHM_ECDH_HW:
			corrupt = (uint8_t*) &ECC_KAT_VECTORS_P384_ECC_PRIVATE_DER[16];
			break;

		default:
			corrupt = NULL;
			break;
	}

	if (corrupt != NULL) {
		*corrupt ^= 0xff;
		clear_cmvp_test_case ();
	}

	return 0;
}

/**
 * Initialize an instance for injecting faults into periodic self-tests to support CMVP testing.
 *
 * @param cmvp The fault injection to initialize.
 *
 * @return 0 if the fault injection was initialized successfully or an error code.
 */
int self_test_cmvp_fault_injection_init (struct self_test_cmvp_fault_injection *cmvp,
	const struct hsp_rng_hw *rng)
{
	if (cmvp == NULL) {
		return SELF_TEST_MANTICORE_INVALID_ARGUMENT;
	}

	memset (cmvp, 0, sizeof (*cmvp));

	cmvp->base_first.run_self_test = self_test_cmvp_fault_injection_run_self_test_first;
	cmvp->base_second.run_self_test = self_test_cmvp_fault_injection_run_self_test_second;

	cmvp->rng = rng;

	return 0;
}

void self_test_cmvp_fault_injection_release (const struct self_test_cmvp_fault_injection *cmvp)
{
	UNUSED (cmvp);
}
