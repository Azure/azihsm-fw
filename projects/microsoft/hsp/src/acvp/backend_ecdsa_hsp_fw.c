// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include "backend_ecdsa_hsp_fw.h"
#include "platform_api.h"
#include "acvp/acvp_logging.h"
#include "asn1/ecc_der_util.h"
#include "common/unused.h"
#include "crypto/signature_verification.h"
#include "firmware/hsp_fw_util.h"
#include "logging/debug_log.h"
#include "parser/cipher_definitions.h"

/**
 * The current implementation identifier for the ACVP backend.
 */
extern uint32_t acvp_implementation;


/**
 * Execute an ECDSA HSP FW signature verification ACVP test on the provided data.
 *
 * @param data The container for the parsed test input and test output.
 * @param parsed_flags Flags parsed from the ACVP request.
 *
 * @return 0 if the test was executed successfully, else -1.
 */
static int backend_ecdsa_hsp_fw_sigver (struct ecdsa_sigver_data *data, flags_t parsed_flags);


/**
 * List of registered ECDSA HSP FW engines.
 */
static const struct backend_ecdsa_engine *ecdsa_hsp_fw_engines = NULL;

/**
 * Number of registered ECDSA HSP FW engines.
 */
static size_t ecdsa_hsp_fw_engines_cnt = 0;

/**
 * ECDSA HSP FW backend callback structure.
 */
static const struct ecdsa_backend ecdsa_hsp_fw_impl = {
	.ecdsa_keygen = NULL,
	.ecdsa_keygen_extra = NULL,
	.ecdsa_pkvver = NULL,
	.ecdsa_siggen = NULL,
	.ecdsa_sigver = backend_ecdsa_hsp_fw_sigver,
	.ecdsa_keygen_en = NULL,
	.ecdsa_free_key = NULL
};


/**
 * Get the ECDSA HSP backend callback structure containing the ECDSA HSP FW implementations.
 *
 * @return The ECDSA HSP backend callback structure.
 */
const struct ecdsa_backend* backend_ecdsa_hsp_fw_get_impl ()
{
	return &ecdsa_hsp_fw_impl;
}

/**
 * Register a list of ECDSA engines with the ECDSA HSP FW backend.  If any ECDSA engines were
 * previously registered, they will be replaced by the new list of ECDSA engines.  The engines must
 * remain valid for the lifetime of the ECDSA backend.
 *
 * @param ecdsa The list of ECDSA engines to register.
 * @param num_engines The number of ECDSA engines in the list.
 */
void backend_ecdsa_hsp_fw_register_engines (const struct backend_ecdsa_engine *ecdsa,
	size_t num_engines)
{
	ecdsa_hsp_fw_engines = ecdsa;
	ecdsa_hsp_fw_engines_cnt = num_engines;
}

/**
 * Retrieve the ECDSA HSP FW engine for the specified implementation identifier.
 *
 * @param impl_id The implementation identifier to search for.
 * @param engine Output for the ECDSA engine associated with the given implentation identifier.
 *
 * @return 0 if the ECDSA engine was found or an error code.
 */
static int backend_ecdsa_hsp_fw_get_engine (int impl_id, const struct backend_ecdsa_engine **engine)
{
	size_t i;

	if (engine == NULL) {
		return BACKEND_ECDSA_INVALID_ARGUMENT;
	}

	if (ecdsa_hsp_fw_engines == NULL) {
		return BACKEND_ECDSA_NO_ENGINE;
	}

	for (i = 0; i < ecdsa_hsp_fw_engines_cnt; i++) {
		if (ecdsa_hsp_fw_engines[i].impl_id == impl_id) {
			*engine = &ecdsa_hsp_fw_engines[i];

			return 0;
		}
	}

	return BACKEND_ECDSA_ENGINE_NOT_FOUND;
}

/**
 * Get the hash type from the specified cipher.
 *
 * @param cipher The cipher to get the hash type for.  The cipher is expected to be a bitmask
 * including the hash type to use as defined in cipher_definitions.h.
 * @param hash_type Output for the hash type specified by the cipher.
 *
 * @return 0 if the hash type was determined or an error code.
 */
static int backend_ecdsa_hsp_fw_get_hash_type (uint64_t cipher, enum hash_type *hash_type)
{
	if (hash_type == NULL) {
		return BACKEND_ECDSA_INVALID_ARGUMENT;
	}

	switch (cipher & ACVP_HASHMASK) {
		case ACVP_SHA384:
			*hash_type = HASH_TYPE_SHA384;
			break;

		default:
			return BACKEND_ECDSA_HASH_TYPE_UNSUPPORTED;
	}

	return 0;
}

/**
 * Get the ECC key lengths from the specified cipher.
 *
 * @param cipher The cipher to get the key length for.  The cipher is expected to be a bitmask
 * including the ECC curve type to use as defined in cipher_definitions.h.
 * @param key_length Output for the raw key length specified by the cipher's ECC curve type.
 *
 * @return 0 if the hash type was determined or an error code.
 */
static int backend_ecdsa_get_ecc_key_length (uint64_t cipher, size_t *key_length)
{
	if (key_length == NULL) {
		return BACKEND_ECDSA_INVALID_ARGUMENT;
	}

	switch (cipher & ACVP_CURVEMASK) {
		case ACVP_NISTP384:
			*key_length = ECC_KEY_LENGTH_384;
			break;

		default:
			return BACKEND_ECDSA_CURVE_TYPE_UNSUPPORTED;
	}

	return 0;
}

static int backend_ecdsa_hsp_fw_sigver (struct ecdsa_sigver_data *data, flags_t parsed_flags)
{
	const struct backend_ecdsa_engine *engine;
	enum hash_type hash_type;
	size_t key_length;
	struct ecc_point_public_key pub_key_point;
	SP_ECDSA_P384_SIGNATURE sig;
	int status = 0;

	UNUSED (parsed_flags);

	if ((data == NULL) || (data->component != BACKEND_ECDSA_COMPONENT_TYPE_FULL) ||
		(data->R.buf == NULL) || (data->S.buf == NULL) ||
		(data->Qx.buf == NULL) || (data->Qy.buf == NULL) || (data->msg.buf == NULL)) {
		status = BACKEND_ECDSA_INVALID_ARGUMENT;
		goto exit;
	}

	if (ecdsa_hsp_fw_engines == NULL) {
		status = BACKEND_ECDSA_NO_ENGINE;
		goto exit;
	}

	status = backend_ecdsa_hsp_fw_get_engine (acvp_implementation, &engine);
	if (status != 0) {
		goto exit;
	}

	if (!engine->is_hw) {
		status = BACKEND_ECDSA_INVALID_ECC_IMPLEMENTATION;
		goto exit;
	}

	if (engine->api_type != BACKEND_ECDSA_API_TYPE_MESSAGE) {
		status = BACKEND_ECDSA_API_TYPE_UNSUPPORTED;
		goto exit;
	}

	status = backend_ecdsa_hsp_fw_get_hash_type (data->cipher, &hash_type);
	if (status != 0) {
		goto exit;
	}

	status = backend_ecdsa_get_ecc_key_length (data->cipher, &key_length);
	if (status != 0) {
		goto exit;
	}

	memcpy (pub_key_point.x, data->Qx.buf, key_length);
	memcpy (pub_key_point.y, data->Qy.buf, key_length);
	pub_key_point.key_length = key_length;

	memcpy (sig.Parts.R.AsBytes, data->R.buf, key_length);
	memcpy (sig.Parts.S.AsBytes, data->S.buf, key_length);

	status = hsp_fw_verify_signed_image (engine->ecc.hw, engine->hash, data->msg.buf, data->msg.len,
		&sig, &pub_key_point, NULL, 0, NULL, 0);

	// If signature verification fails, the returned status should be 0.
	if (status == ECC_HW_ECDSA_BAD_SIGNATURE) {
		data->sigver_success = 0;
		status = 0;
	}
	else if (status == 0) {
		data->sigver_success = 1;
	}

exit:
	if (ROT_IS_ERROR (status)) {
		// On failure, set status to -1 to trigger test failure handling in Acvpparser library. Log
		// error to give more information about the failure.
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_ACVP,
			ACVP_LOGGING_TEST_FAILURE, status, 0);

		status = -1;
	}

	return status;
}

/**
 * Register the ECDSA HSP FW backend implementation with the ACVP backend.
 */
void backend_ecdsa_hsp_fw_register_impl (void)
{
	register_ecdsa_impl ((struct ecdsa_backend*) &ecdsa_hsp_fw_impl);
}
