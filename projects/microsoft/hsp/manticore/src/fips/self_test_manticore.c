// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "self_test_manticore.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "crypto/kat/aes_kat.h"
#include "crypto/kat/aes_key_wrap_kat.h"
#include "crypto/kat/ecdh_kat.h"
#include "crypto/kat/ecdsa_kat.h"
#include "crypto/kat/hash_kat.h"
#include "crypto/kat/kdf_kat.h"
#include "drivers/kat/ccs_ksu_kat.h"
#include "fips/fips_logging.h"
#include "firmware/hsp_fw_util.h"


/**
 * Implementation IDs to use for ECDSA self-test failure log messages.
 */
enum {
	SELF_TEST_MANTICORE_EDCSA_ROM_ID = 0,	/**< Implementation ID for the ECDSA usage in ROM. */
	SELF_TEST_MANTICORE_ECDSA_HW_ID = 1,	/**< Implementation ID for ECDSA usage directly with PKA hardware. */
	SELF_TEST_MANTICORE_ECDSA_SW_ID = 2,	/**< Implementation ID for ECDSA usage through the ECC API. */
};


int self_test_manticore_run_self_test_first (const struct self_test_interface *self_test,
	struct debug_log_entry_info *error_info)
{
	const struct self_test_manticore *manticore =
		TO_DERIVED_TYPE (self_test, const struct self_test_manticore, base_first);
	uint8_t msg_index;
	uint32_t instance_id = 0;
	int status;

	if ((self_test == NULL) || (error_info == NULL)) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_FIPS,
			FIPS_LOGGING_SELF_TEST_NOT_EXECUTED, SELF_TEST_MANTICORE_INVALID_ARGUMENT, 0);

		return SELF_TEST_MANTICORE_INVALID_ARGUMENT;
	}

	status = aes_gcm_kat_run_self_test_encrypt_aes256 (manticore->gcm);
	if (status != 0) {
		msg_index = FIPS_LOGGING_AES_GCM_ENCRYPT_KAT_FAILED;
		goto kat_failure;
	}

	status = aes_key_wrap_kat_run_self_test_wrap_with_padding_aes256 (manticore->aes_kwp);
	if (status != 0) {
		msg_index = FIPS_LOGGING_AES_KEY_WRAP_PADDING_KAT_FAILED;
		goto kat_failure;
	}

	status = ccs_ksu_kat_run_self_test_hmac_sha256 (manticore->ccs, manticore->key_slot);
	if (status != 0) {
		msg_index = FIPS_LOGGING_HMAC_KAT_FAILED;
		goto kat_failure;
	}

	status = ccs_ksu_kat_run_self_test_kdf384 (manticore->ccs, manticore->key_slot);
	if (status != 0) {
		msg_index = FIPS_LOGGING_KBKDF_KAT_FAILED;
		goto kat_failure;
	}

	status = hash_kat_run_self_test_update_sha512 (manticore->hash);
	if (status != 0) {
		msg_index = FIPS_LOGGING_SHA_KAT_FAILED;
		goto kat_failure;
	}

	status = kdf_kat_run_self_test_hkdf_sha256 (manticore->hkdf);
	if (status != 0) {
		msg_index = FIPS_LOGGING_HKDF_KAT_FAILED;
		goto kat_failure;
	}

	status = hsp_fw_run_self_test_verify_signed_image (manticore->pka, manticore->hash);
	if (status != 0) {
		msg_index = FIPS_LOGGING_ECDSA_VERIFY_KAT_FAILED;
		instance_id = SELF_TEST_MANTICORE_EDCSA_ROM_ID;
		goto kat_failure;
	}

	status = ecdsa_kat_run_self_test_sign_p384_sha384 (manticore->ecc, manticore->hash);
	if (status != 0) {
		msg_index = FIPS_LOGGING_ECDSA_SIGN_KAT_FAILED;
		instance_id = SELF_TEST_MANTICORE_ECDSA_SW_ID;
		goto kat_failure;
	}

	return 0;

kat_failure:
	error_info->severity = DEBUG_LOG_SEVERITY_ERROR;
	error_info->component = DEBUG_LOG_COMPONENT_FIPS;
	error_info->msg_index = msg_index;
	error_info->arg1 = instance_id;
	error_info->arg2 = status;
	error_info->format = 1;

	return status;
}

int self_test_manticore_run_self_test_second (const struct self_test_interface *self_test,
	struct debug_log_entry_info *error_info)
{
	const struct self_test_manticore *manticore =
		TO_DERIVED_TYPE (self_test, const struct self_test_manticore, base_second);
	uint8_t msg_index;
	uint32_t instance_id = 0;
	int status;

	if ((self_test == NULL) || (error_info == NULL)) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_FIPS,
			FIPS_LOGGING_SELF_TEST_NOT_EXECUTED, SELF_TEST_MANTICORE_INVALID_ARGUMENT, 0);

		return SELF_TEST_MANTICORE_INVALID_ARGUMENT;
	}

	status = aes_gcm_kat_run_self_test_decrypt_aes256 (manticore->gcm);
	if (status != 0) {
		msg_index = FIPS_LOGGING_AES_GCM_DECRYPT_KAT_FAILED;
		goto kat_failure;
	}

	status = aes_key_wrap_kat_run_self_test_unwrap_with_padding_aes256 (manticore->aes_kwp);
	if (status != 0) {
		msg_index = FIPS_LOGGING_AES_KEY_UNWRAP_PADDING_KAT_FAILED;
		goto kat_failure;
	}

	status = ecdsa_kat_run_self_test_verify_p384_sha384 (manticore->ecc, manticore->hash);
	if (status != 0) {
		msg_index = FIPS_LOGGING_ECDSA_VERIFY_KAT_FAILED;
		instance_id = SELF_TEST_MANTICORE_ECDSA_SW_ID;
		goto kat_failure;
	}

	status = ecdh_kat_run_self_test_p384 (manticore->ecc);
	if (status != 0) {
		msg_index = FIPS_LOGGING_ECDH_KAT_FAILED;
		goto kat_failure;
	}

	return 0;

kat_failure:
	error_info->severity = DEBUG_LOG_SEVERITY_ERROR;
	error_info->component = DEBUG_LOG_COMPONENT_FIPS;
	error_info->msg_index = msg_index;
	error_info->arg1 = instance_id;
	error_info->arg2 = status;
	error_info->format = 1;

	return status;
}

/**
 * Initialize a handler for self-testing all Manticore cryptographic algorithms.
 *
 * @param self_test The self-test handler to initialize.
 * @param gcm An instance of the AES-GCM implementation to use for testing.  This must be a separate
 * instance from any other AES-GCM usage.
 * @param aes_kwp An instance of the AES key wrap with padding implementation to use for testing.
 * This must be a separate instance from any other AES key wrap usage.
 * @param ccs Driver for the CCS to use for hardware algorithm testing.
 * @param key_slot The KSU key slot to use for CCS algorithm testing.  This must not be used for any
 * other key.
 * @param hkdf An instance of the HKDF implementation to use for testing.  This must be a separate
 * instance from any other HKDF usage.
 * @param pka Driver for the PKA hardware to use for ROM ECDSA testing.
 * @param ecc An instance of the ECC implementation to use for ECDSA and ECDH testing.
 * @param hash An instance of the hash implementation to use for ECDSA testing.
 *
 * @return 0 if the self-test handler was initialized successfully or an error code.
 */
int self_test_manticore_init (struct self_test_manticore *self_test,
	const struct aes_gcm_engine *gcm, const struct aes_key_wrap_interface *aes_kwp,
	const struct ccs_ksu_interface *ccs, uint8_t key_slot, const struct hkdf_interface *hkdf,
	const struct ecc_hw *pka, const struct ecc_engine *ecc, const struct hash_engine *hash)
{
	if ((self_test == NULL) || (gcm == NULL) || (aes_kwp == NULL) || (ccs == NULL) ||
		(hkdf == NULL) || (pka == NULL) || (ecc == NULL) || (hash == NULL)) {
		return SELF_TEST_MANTICORE_INVALID_ARGUMENT;
	}

	memset (self_test, 0, sizeof (*self_test));

	self_test->base_first.run_self_test = self_test_manticore_run_self_test_first;
	self_test->base_second.run_self_test = self_test_manticore_run_self_test_second;

	self_test->gcm = gcm;
	self_test->aes_kwp = aes_kwp;
	self_test->ccs = ccs;
	self_test->key_slot = key_slot;
	self_test->hkdf = hkdf;
	self_test->pka = pka;
	self_test->ecc = ecc;
	self_test->hash = hash;

	return 0;
}

/**
 * Release the resourced used for the Manticore self-test handler.
 *
 * @param self_test The self-test handler to release.
 */
void self_test_manticore_release (const struct self_test_manticore *self_test)
{
	UNUSED (self_test);
}
