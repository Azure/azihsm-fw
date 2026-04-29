// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include <string.h>
#include "ccs_ksu_kat.h"
#include "common/buffer_util.h"
#include "crypto/kat/hmac_kat_vectors.h"
#include "drivers/kat/ccs_ksu_kat_vectors.h"


/**
 * Run a known answer test (KAT) for the CCS HMAC command using SHA-256.
 *
 * @param ccs The CCS instance to test.
 * @param key_slot A key slot in the KSU that will be populated with the key for the test.  Any
 * existing key in this slot will be lost.
 *
 * @return 0 if the self-test passed or an error code.
 */
int ccs_ksu_kat_run_self_test_hmac_sha256 (const struct ccs_ksu_interface *ccs, uint8_t key_slot)
{
	SP_MSG_384 output = {0};
	int status;
	int clear_key;

	if (ccs == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	status = ccs->set_key (ccs, &CCS_KSU_KAT_VECTORS_HMAC_SHA256_KEY, key_slot,
		CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED);
	if (status != 0) {
		return status;
	}

	status = ccs->hmac (ccs, key_slot, HMAC_KAT_VECTORS_CALCULATE_DATA,
		HMAC_KAT_VECTORS_CALCULATE_DATA_LEN, &output, NULL);
	if (status != 0) {
		goto exit;
	}

	status = buffer_compare (HMAC_KAT_VECTORS_CALCULATE_SHA256_MAC, output.AsBytes,
		SP_MSG_256_SIZE);
	if (status != 0) {
		status = CCS_KSU_HMAC_SELF_TEST_FAILED;
	}

exit:
	memset (output.AsBytes, 0, SP_MSG_384_SIZE);

	clear_key = ccs->set_key (ccs, &output, key_slot, 0);
	if (status == 0) {
		status = clear_key;
	}

	return status;
}

/**
 * Run a known answer test (KAT) for the CCS HMAC command using SHA-384.
 *
 * @param ccs The CCS instance to test.
 * @param key_slot A key slot in the KSU that will be populated with the key for the test.  Any
 * existing key in this slot will be lost.
 *
 * @return 0 if the self-test passed or an error code.
 */
int ccs_ksu_kat_run_self_test_hmac_sha384 (const struct ccs_ksu_interface *ccs, uint8_t key_slot)
{
	SP_MSG_384 output = {0};
	int status;
	int clear_key;

	if (ccs == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	status = ccs->set_key (ccs, &CCS_KSU_KAT_VECTORS_HMAC_SHA384_KEY, key_slot,
		CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
	if (status != 0) {
		return status;
	}

	status = ccs->hmac (ccs, key_slot, HMAC_KAT_VECTORS_CALCULATE_DATA,
		HMAC_KAT_VECTORS_CALCULATE_DATA_LEN, &output, NULL);
	if (status != 0) {
		goto exit;
	}

	status = buffer_compare (HMAC_KAT_VECTORS_CALCULATE_SHA384_MAC, output.AsBytes,
		SP_MSG_384_SIZE);
	if (status != 0) {
		status = CCS_KSU_HMAC_SELF_TEST_FAILED;
	}

exit:
	memset (output.AsBytes, 0, SP_MSG_384_SIZE);

	clear_key = ccs->set_key (ccs, &output, key_slot, 0);
	if (status == 0) {
		status = clear_key;
	}

	return status;
}

/**
 * Run a known answer test (KAT) for the CCS KDF commands using SHA-256.
 *
 * @param ccs The CCS instance to test.
 * @param key_slot A key slot in the KSU that will be populated with the key for the test.  Any
 * existing key in this slot will be lost.
 *
 * @return 0 if the self-test passed or an error code.
 */
int ccs_ksu_kat_run_self_test_kdf256 (const struct ccs_ksu_interface *ccs, uint8_t key_slot)
{
	SP_MSG_384 output = {0};
	int status;
	int clear_key;

	if (ccs == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	status = ccs->set_key (ccs, &CCS_KSU_KAT_VECTORS_HMAC_SHA256_KEY, key_slot,
		CCS_KSU_ATTR_KDF_KEY_ALLOWED);
	if (status != 0) {
		return status;
	}

	status = ccs->derive_key (ccs, key_slot, &CCS_KSU_KAT_VECTORS_KDF256_CONTEXT, key_slot,
		CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED);
	if (status != 0) {
		goto exit;
	}

	/* Since the KDF output cannot be inspected directly, use an HMAC to verify that the key is
	 * correct. */
	status = ccs->hmac (ccs, key_slot, HMAC_KAT_VECTORS_CALCULATE_DATA,
		HMAC_KAT_VECTORS_CALCULATE_DATA_LEN, &output, NULL);
	if (status != 0) {
		goto exit;
	}

	status = buffer_compare (CCS_KSU_KAT_VECTORS_KDF256_HMAC, output.AsBytes, SP_MSG_256_SIZE);
	if (status != 0) {
		status = CCS_KSU_KDF_SELF_TEST_FAILED;
	}

exit:
	memset (output.AsBytes, 0, SP_MSG_384_SIZE);

	clear_key = ccs->set_key (ccs, &output, key_slot, 0);
	if (status == 0) {
		status = clear_key;
	}

	return status;
}

/**
 * Run a known answer test (KAT) for the CCS KDF commands using SHA-384.
 *
 * @param ccs The CCS instance to test.
 * @param key_slot A key slot in the KSU that will be populated with the key for the test.  Any
 * existing key in this slot will be lost.
 *
 * @return 0 if the self-test passed or an error code.
 */
int ccs_ksu_kat_run_self_test_kdf384 (const struct ccs_ksu_interface *ccs, uint8_t key_slot)
{
	SP_MSG_384 output = {0};
	int status;
	int clear_key;

	if (ccs == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	status = ccs->set_key (ccs, &CCS_KSU_KAT_VECTORS_HMAC_SHA384_KEY, key_slot,
		CCS_KSU_ATTR_KDF_KEY_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
	if (status != 0) {
		return status;
	}

	status = ccs->derive_key (ccs, key_slot, &CCS_KSU_KAT_VECTORS_KDF384_CONTEXT, key_slot,
		CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
	if (status != 0) {
		goto exit;
	}

	/* Since the KDF output cannot be inspected directly, use an HMAC to verify that the key is
	 * correct. */
	status = ccs->hmac (ccs, key_slot, HMAC_KAT_VECTORS_CALCULATE_DATA,
		HMAC_KAT_VECTORS_CALCULATE_DATA_LEN, &output, NULL);
	if (status != 0) {
		goto exit;
	}

	status = buffer_compare (CCS_KSU_KAT_VECTORS_KDF384_HMAC, output.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		status = CCS_KSU_KDF_SELF_TEST_FAILED;
	}

exit:
	memset (output.AsBytes, 0, SP_MSG_384_SIZE);

	clear_key = ccs->set_key (ccs, &output, key_slot, 0);
	if (status == 0) {
		status = clear_key;
	}

	return status;
}
