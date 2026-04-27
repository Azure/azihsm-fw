// Copyright (c) Microsoft Corporation. All rights reserved.

#include "hsp_top.h"
#include "init_crashdump.h"
#include "init_crypto.h"
#include "init_i2c.h"
#include "init_log_flush_handlers.h"
#include "init_system.h"
#include "manticore_rom.h"
#include "periodic_task_freertos_static.h"
#include "sp_boot.h"
#include "task_log_id.h"
#include "task_priority.h"
#include "task_stack_size.h"
#include "asn1/x509_mbedtls_static.h"
#include "common/array_size.h"
#include "crypto/aes_ecb_hsp_static.h"
#include "crypto/ecc_ecc_hw_static.h"
#include "crypto/kat/aes_kat.h"
#include "crypto/kat/aes_key_wrap_kat.h"
#include "crypto/kat/ecdh_kat.h"
#include "crypto/kat/ecdsa_kat.h"
#include "crypto/kat/hash_kat.h"
#include "crypto/kat/kdf_kat.h"
#include "crypto/rng_hsp_static.h"
#include "crypto/rsa_mbedtls_static.h"
#include "drivers/kat/ccs_ksu_kat.h"
#include "fips/self_test_hsp_rng_hw_static.h"
#include "fips/self_test_manticore_static.h"
#include "firmware/hsp_fw_util.h"
#include "firmware/manticore_device_keys.h"
#include "sprt/manticore_sprt.h"

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
#include "crypto/kat/aes_kat_vectors.h"
#include "crypto/kat/aes_key_wrap_kat_vectors.h"
#include "crypto/kat/ecc_kat_vectors.h"
#include "crypto/kat/hash_kat_vectors.h"
#include "crypto/kat/kdf_kat_vectors.h"
#include "fips/self_test_cmvp_fault_injection_static.h"
#endif


/**
 * Time interval between periodic self-test executions.
 */
#ifndef MANTICORE_ENABLE_FIPS_CMVP_TESTING
#define	MANTICORE_PERIODIC_SELF_TEST_INTERVAL		(24 * 60 * 60 * 1000)	/* 24 hours */
#else
#define	MANTICORE_PERIODIC_SELF_TEST_INTERVAL		(15 * 1000)				/* 15 seconds */
#endif


/**
 * Data populated by ROM that can be used with local static initialization.
 */
static struct manticore_rom_shared_sram *const rom_shared_static =
	(struct manticore_rom_shared_sram*) HSP_ADDR_MAP_SHAREDRAM_ADDRESS;

/**
 * Location in shared SRAM for hardware crypto buffers.
 */
static struct manticore_sprt_shared_sram_crypto *const crypto_cmd =
	(struct manticore_sprt_shared_sram_crypto*) &rom_shared_static->internal;

/**
 * Variable context for the HS-SHA driver.
 */
static struct hs_sha_state hash_hw_context;

/**
 * Driver for the HS-SHA.
 */
const struct hs_sha hash_hw = hs_sha_static_init_polling (&hash_hw_context,
	(struct Sha_regs*) HSP_ADDR_MAP_SHA_ADDRESS, &crypto_cmd->hs_sha.cmd, crypto_cmd->hs_sha.data,
	MANTICORE_CRYPTO_HS_SHA_BUFFER_SIZE);

/* Reference to the HS-SHA HW driver for use by mbedtls. */
const struct hs_sha *const mbedtls_hs_sha = &hash_hw;

/**
 * Variable context for the PKA driver.
 */
static struct ecc_hw_pka_state pka_context;

/**
 * Driver for the PKA.
 */
const struct ecc_hw_pka pka = ecc_hw_pka_static_init_polling (&pka_context,
	(struct Pka_regs*) HSP_ADDR_MAP_PKA_ADDRESS, &rng_hw, &crypto_cmd->pka);

/* Global reference to the PKA driver for use by mbedtls ECC functions. */
const struct ecc_hw_pka *const mbedtls_ecc_pka = &pka;

/**
 * Variable context for the AES driver.
 */
static struct hsp_aes_state aes_context;

/**
 * Driver for the HW AES engine.
 */
const struct hsp_aes aes_hw = hsp_aes_static_init_polling (&aes_context,
	(struct Aes_regs*) HSP_ADDR_MAP_AES_ADDRESS, &crypto_cmd->aes.cmd, crypto_cmd->aes.data,
	MANTICORE_CRYPTO_AES_BUFFER_SIZE, (struct ksu_key_slot*) HSP_ADDR_MAP_KSB_KEYS_ADDRESS,
	CCS_KSU_STATIC_NUM_KEYS);

/**
 * Variable context for the CCS driver.
 */
static struct ccs_ksu_state ccs_context;

/**
 * Driver for the CCS and KSU.
 */
const struct ccs_ksu ccs = ccs_ksu_static_init_polling (&ccs_context,
	(struct Ccs_regs*) HSP_ADDR_MAP_CCS_ADDRESS, &hash_hw, &aes_hw, &pka, &rng_hw, &crypto_cmd->ccs,
	(struct ksu_key_slot*) HSP_ADDR_MAP_KSB_KEYS_ADDRESS, CCS_KSU_STATIC_NUM_KEYS,
	(struct ksu_pcr_slot*) HSP_ADDR_MAP_KSB_PCRS_ADDRESS, CCS_KSU_STATIC_NUM_PCRS);

/**
 * Hardware RNG that will be shared between multiple components.
 */
static const struct rng_engine_hsp system_rng = rng_hsp_static_init (&rng_hw);

/**
 * Variable context for the thread-safe RNG wrapper.
 */
static struct rng_engine_thread_safe_state shared_rng_context;

/**
 * Wrapper for the shared RNG engine.
 */
const struct rng_engine_thread_safe shared_rng =
	rng_thread_safe_static_init (&shared_rng_context, &system_rng.base);

/* Set the entropy source for mbedTLS. */
const struct hsp_rng_hw *const mbedtls_entropy = &rng_hw;

/**
 * Variable context for the system hash engine.
 */
static struct hash_engine_hs_sha_state system_hash_context;

/**
 * Hash engine that will be shared between multiple components.
 */
static const struct hash_engine_hs_sha system_hash = hash_hs_sha_static_init (&system_hash_context,
	&hash_hw);

/**
 * Variable context for the shared hash engine.
 */
static struct hash_engine_thread_safe_state shared_hash_context;

/**
 * Wrapper for the shared hash engine.
 */
const struct hash_engine_thread_safe shared_hash =
	hash_thread_safe_static_init (&shared_hash_context, &system_hash.base);

/**
 * Variable context for the host flash hash engines.
 */
static struct hash_engine_hs_sha_state host_hash_context;

/**
 * Hash engine for host flash verification.
 */
const struct hash_engine_hs_sha host_hash = hash_hs_sha_static_init (&host_hash_context, &hash_hw);

/**
 * ECC engine that will be shared between multiple components.
 */
static const struct ecc_engine_ecc_hw system_ecc = ecc_ecc_hw_static_init (&pka.base, NULL);

/**
 * Variable context for the shared ECC engine.
 */
static struct ecc_engine_thread_safe_state shared_ecc_context;

/**
 * Wrapper for the shared ECC engine.
 */
const struct ecc_engine_thread_safe shared_ecc = ecc_thread_safe_static_init (&shared_ecc_context,
	&system_ecc.base);

/**
 * RSA engine that will be shared between multiple components.
 *
 * TODO:  Replace with a PKA-based RSA implementation.
 */
static const struct rsa_engine_mbedtls system_rsa =
	rsa_mbedtls_static_init_with_external_rng (&shared_rng.base);

/**
 * Variable context for the thread-safe RSA wrapper.
 */
static struct rsa_engine_thread_safe_state shared_rsa_context;

/**
 * Wrapper for the shared RSA engine.
 */
const struct rsa_engine_thread_safe shared_rsa =
	rsa_thread_safe_static_init (&shared_rsa_context, &system_rsa.base);

/**
 * Variable context for the shared X.509 engine.
 */
static struct x509_engine_mbedtls_state system_x509_context;

/**
 * X.509 engine that will be shared between multiple components.
 */
static const struct x509_engine_mbedtls system_x509 =
	x509_mbedtls_static_init (&system_x509_context);

/**
 * Variable context for the thread-safe X.509 wrapper.
 */
static struct x509_engine_thread_safe_state shared_x509_context;

/**
 * Wrapper for the shared X.509 engine.
 */
const struct x509_engine_thread_safe shared_x509 =
	x509_thread_safe_static_init (&shared_x509_context, &system_x509.base);

/**
 * Variable context for the AES-GCM engine.
 */
static struct aes_gcm_engine_mbedtls_state aes_gcm_state;

/**
 * AES-GCM engine that will be used for executing self-tests.
 */
const struct aes_gcm_engine_mbedtls aes_gcm = aes_gcm_mbedtls_static_init (&aes_gcm_state);

/**
 * AES-ECB interface for running AES key wrap self-tests.
 */
static const struct aes_ecb_engine_hsp kwp_ecb =
	aes_ecb_hsp_static_init (&aes_hw, &ccs.base, MANTICORE_DEVICE_KEYS_SELF_TEST);

/**
 * AES key wrap handler to use for self-tests.
 */
const struct aes_key_wrap_with_padding aes_kwp =
	aes_key_wrap_with_padding_static_init (&kwp_ecb.base);

/**
 * Variable context for self-testing HKDF key derivations.
 */
static struct hkdf_state hkdf_context;

/**
 * HKDF handler for running self-tests of the HKDF algorithm.
 */
const struct hkdf hkdf = hkdf_static_init (&hkdf_context, &shared_hash.base);

/**
 * Variable context for the FIPS error state handler.
 */
static struct manticore_error_state_state error_state_handler_context;

/**
 * Handler for the FIPS error state in response to self-test failures.
 */
const struct manticore_error_state error_state_handler =
	manticore_error_state_static_init (&error_state_handler_context, &fips_i2c.base_entry, &soc_api,
	&log_flush,	&device_cmd.base.base);

/**
 * Execution context for DRBG periodic self-tests.
 */
static const struct self_test_hsp_rng_hw drbg_self_test =
	self_test_hsp_rng_hw_static_init (&rng_hw);

/**
 * Execution context for all Manticore cryptographic algorithm self-tests.
 */
static const struct self_test_manticore manticore_self_test =
	self_test_manticore_static_init (&aes_gcm.base, &aes_kwp.base.base, &ccs.base,
	MANTICORE_DEVICE_KEYS_SELF_TEST, &hkdf.base, &pka.base, &shared_ecc.base, &shared_hash.base);

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
/**
 * Execution context for injecting faults into periodic self-tests to support CMVP testing.
 */
static const struct self_test_cmvp_fault_injection cmvp_self_test =
	self_test_cmvp_fault_injection_static_init (&rng_hw);
#endif

/**
 * List of handler that that will be used to execute periodic self-tests.
 */
static const struct self_test_interface *const periodic_self_tests[] = {
#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
	&ras_fault_inj_test.base,
	&cmvp_self_test.base_first,
#endif
	&drbg_self_test.base,
	&manticore_self_test.base_first,
#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
	&cmvp_self_test.base_second,
#endif
	&manticore_self_test.base_second
};

/**
 * Variable context for managing periodic self-test execution.
 */
static struct periodic_self_test_handler_state periodic_self_test_context;

/**
 * Handler for executing periodic self-tests.  Periodic self-tests will get executed at CRITICAL
 * priority on the watchdog task.  This ensures that no other tasks will execute while self-tests
 * are running.
 */
const struct periodic_self_test_handler periodic_self_test_handler =
	periodic_self_test_handler_static_init (&periodic_self_test_context, periodic_self_tests,
	ARRAY_SIZE (periodic_self_tests), MANTICORE_PERIODIC_SELF_TEST_INTERVAL,
	&error_state_handler.base_error_task);

/**
 * List of handlers for the error state task.  This task can only support a single handler.
 */
static const struct periodic_task_handler *const error_state_handlers[] = {
	&error_state_handler.base_task
};

/**
 * Variable context for the FIPS error state task.
 */
static struct periodic_task_freertos_state error_state_task_context;

/**
 * Task to handle the FIPS error state.  This task will run at a higher priority that any other
 * task, ensuring that error state handling preempts all other operations.
 */
static const struct periodic_task_freertos error_state_task =
	periodic_task_freertos_static_init (&error_state_task_context, error_state_handlers,
	ARRAY_SIZE (error_state_handlers), ERROR_STATE_TASK_LOG_ID);

/**
 * Statically allocated task control block for the FIPS error state task.
 */
static StaticTask_t error_state_task_tcb;

/**
 * Statically allocated stack for the FIPS error state task.
 */
static StackType_t error_state_task_stack[ERROR_STATE_TASK_STACK_WORDS];


/**
 * Initialize the HW crypto engines.  The RNG and fuses are not initialized, since they were handled
 * during early init.
 *
 * @return 0 if the crypto engines were successfully initialized or an error code.
 */
int initialize_crypto_hardware ()
{
	int status;

	status = hs_sha_init_state (&hash_hw);
	if (status != 0) {
		return status;
	}

	status = ecc_hw_pka_init_state (&pka);
	if (status != 0) {
		return status;
	}

	status = hsp_aes_init_state (&aes_hw);
	if (status != 0) {
		return status;
	}

	status = ccs_ksu_init_state (&ccs);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Initialize crypto engines shared between system components.
 *
 * @return 0 if the operations were successful or an error code.
 */
int initialize_system_crypto ()
{
	int status;

	status = hash_hs_sha_init_state (&system_hash);
	if (status != 0) {
		return status;
	}

	status = hash_hs_sha_init_state (&host_hash);
	if (status != 0) {
		return status;
	}

	status = aes_gcm_mbedtls_init_state (&aes_gcm);
	if (status != 0) {
		return status;
	}

	status = hkdf_init_state (&hkdf);
	if (status != 0) {
		return status;
	}

	status = x509_mbedtls_init_state (&system_x509);
	if (status != 0) {
		return status;
	}

	/* Initialize thread-safe wrappers that are shared among system components. */
	status = rng_thread_safe_init_state (&shared_rng);
	if (status != 0) {
		return status;
	}

	status = hash_thread_safe_init_state (&shared_hash);
	if (status != 0) {
		return status;
	}

	status = ecc_thread_safe_init_state (&shared_ecc);
	if (status != 0) {
		return status;
	}

	status = rsa_thread_safe_init_state (&shared_rsa);
	if (status != 0) {
		return status;
	}

	status = x509_thread_safe_init_state (&shared_x509);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Execute self-tests on the crypto components.
 *
 * @return 0 if the self-tests have been executed successfully or an error code.
 */
int run_crypto_self_tests ()
{
	int status;

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
	enum cmvp_test_case_cast_algorithm algo = CMVP_TEST_CASE_ALGORITHM_NONE;
	uint8_t *corrupt;
	bool corrupt_wait = false;

	status = get_cmvp_test_case (CMVP_TEST_CASE_BOOT_STAGE_SPRT);
	if (status != 0) {
		return status;
	}

	if ((cmvp_test != 0) &&
		(cmvp_test_case_get_test_type (cmvp_test) == CMVP_TEST_CASE_CAST_NEGATIVE_TEST) &&
		(cmvp_test_case_get_cast_type (cmvp_test) == CMVP_TEST_CASE_CAST_TYPE_PRE_OPERATIONAL)) {
		algo = cmvp_test_case_get_cast_algorithm (cmvp_test);
	}

	switch (algo) {
		case CMVP_TEST_CASE_ALGORITHM_SHA_HW_SW:
			corrupt = (uint8_t*) &SHA_KAT_VECTORS_UPDATE_DATA_2[16];
			break;

		case CMVP_TEST_CASE_ALGORITHM_HKDF_SW:
			corrupt = (uint8_t*) &KDF_KAT_VECTORS_HKDF_EXTRACT_IKM[16];
			break;

		case CMVP_TEST_CASE_ALGORITHM_AES_KWP_UNWRAP_SW:
			corrupt_wait = true;
		/* fall through */ /* no break */

		case CMVP_TEST_CASE_ALGORITHM_AES_KWP_WRAP_SW:
			corrupt = (uint8_t*) &AES_KEY_WRAP_KAT_VECTORS_KWP_KEY[16];
			break;

		case CMVP_TEST_CASE_ALGORITHM_AES_DECRYPT_SW:
			corrupt_wait = true;
		/* fall through */ /* no break */

		case CMVP_TEST_CASE_ALGORITHM_AES_ENCRYPT_SW:
			corrupt = (uint8_t*) &AES_GCM_KAT_VECTORS_256_KEY[16];
			break;

		case CMVP_TEST_CASE_ALGORITHM_ECDSA_VERIFY_ROM:
			corrupt = (uint8_t*) &ECC_KAT_VECTORS_P384_ECC_PUBLIC.x[16];
			break;

		case CMVP_TEST_CASE_ALGORITHM_ECDSA_SIGN_SW:
			corrupt = (uint8_t*) &ECC_KAT_VECTORS_P384_ECC_PRIVATE_DER[16];
			break;

		case CMVP_TEST_CASE_ALGORITHM_ECDSA_VERIFY_SW:
			corrupt = (uint8_t*) &ECC_KAT_VECTORS_P384_SHA384_ECDSA_SIGNATURE_DER[16];
			corrupt_wait = true;
			break;

		case CMVP_TEST_CASE_ALGORITHM_ECDH_HW:
			corrupt = (uint8_t*) &ECC_KAT_VECTORS_P384_ECC_PRIVATE_DER[16];
			corrupt_wait = true;
			break;

		default:
			corrupt = NULL;
			break;
	}

	/* Corrupt the KAT vector, unless corruption needs to be delayed until the first set of
	 * self-tests have been executed. */
	if ((corrupt != NULL) && !corrupt_wait) {
		*corrupt ^= 0xff;
		corrupt = NULL;
		cmvp_test = 0;
	}

	/* Configure the firmware to trigger a PCT failure, if requested. */
	trigger_cmvp_pct_failure ();
#endif

	status = hash_kat_run_self_test_update_sha512 (&shared_hash.base);
	if (status != 0) {
		return status;
	}

	status = kdf_kat_run_self_test_hkdf_sha256 (&hkdf.base);
	if (status != 0) {
		return status;
	}

	status = aes_key_wrap_kat_run_self_test_wrap_with_padding_aes256 (&aes_kwp.base.base);
	if (status != 0) {
		return status;
	}

	status = aes_gcm_kat_run_self_test_encrypt_aes256 (&aes_gcm.base);
	if (status != 0) {
		return status;
	}

	status = hsp_fw_run_self_test_verify_signed_image (&pka.base, &shared_hash.base);
	if (status != 0) {
		return status;
	}

	status = ecdsa_kat_run_self_test_sign_p384_sha384 (&shared_ecc.base, &shared_hash.base);
	if (status != 0) {
		return status;
	}

	/* Run remaining self-tests that use the same KAT vectors as some previous tests.  Normally,
	 * this doesn't matter, but for CMVP testing, these need to be separated so KAT vector
	 * corruption can be handled efficiently. */
#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
	if (corrupt != NULL) {
		*corrupt ^= 0xff;
		cmvp_test = 0;
	}
#endif

	status = aes_key_wrap_kat_run_self_test_unwrap_with_padding_aes256 (&aes_kwp.base.base);
	if (status != 0) {
		return status;
	}

	status = aes_gcm_kat_run_self_test_decrypt_aes256 (&aes_gcm.base);
	if (status != 0) {
		return status;
	}

	status = ecdsa_kat_run_self_test_verify_p384_sha384 (&shared_ecc.base, &shared_hash.base);
	if (status != 0) {
		return status;
	}

	status = ecdh_kat_run_self_test_p384 (&shared_ecc.base);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Initialize the task for handling the FIPS error state after a cryptographic self-test failure.
 *
 * @return 0 if the error state task was initialized successfully or an error code.
 */
int initialize_error_state_task ()
{
	int status;

	status = manticore_error_state_init_state (&error_state_handler);
	if (status != 0) {
		return status;
	}

	return periodic_task_freertos_init_state (&error_state_task);
}

/**
 * Start the task for handling the FIPS error state.
 *
 * @return 0 if the error state task was started successfully or an error code.
 */
int start_error_state_task ()
{
	int status;

	/* This uses the "invalid" priority to make it the highest priority task.  FreeRTOS is
	 * configured to have an extra priority level than it normally would. */
	status = periodic_task_freertos_allocate_static (&error_state_task, &error_state_task_tcb,
		error_state_task_stack, ERROR_STATE_TASK_STACK_WORDS, "ErrState", CERBERUS_PRIORITY_COUNT);
	if (status != 0) {
		return status;
	}

	periodic_task_freertos_start (&error_state_task);

	return 0;
}
