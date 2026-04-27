// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "platform_api.h"
#include "riot_core_hsp_shack1.h"
#include "asn1/asn1_util.h"
#include "asn1/ecc_der_util.h"
#include "common/unused.h"


int riot_core_hsp_shack1_generate_device_id (const struct riot_core *riot, const uint8_t *cdi,
	size_t length)
{
	const struct riot_core_hsp_shack1 *hsp = (const struct riot_core_hsp_shack1*) riot;
	uint32_t hmac_attr = CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_ECC_SIGN_ALLOWED;
	SP_ECDSA_P384_PUBLIC hmac_key;
	SP_MSG_256 serial_num;
	uint32_t dev_id_attributes;
	int status;

	/* The CDI is already in the KSU for this implementation. */
	UNUSED (cdi);
	UNUSED (length);

	status = riot_core_hsp_generate_device_id_setup_keys (&hsp->riot, hmac_attr,
		&dev_id_attributes);
	if (status != 0) {
		return status;
	}

	/* Since SHACK1 KSU does not support HMAC operations, extract public key of the HMAC key.
	 * Serial number is the first 8 bytes of HMAC public key digest rather than an HMAC result. */
	status = hsp->riot.ccs->get_ecc_public_key (hsp->riot.ccs, hsp->riot.alias_key, &hmac_key,
		NULL);
	if (status != 0) {
		return status;
	}

	status = hsp->hash->calculate_sha256 (hsp->hash, hmac_key.AsBytes,
		SP_ECDSA_P256_PUBLIC_KEY_SIZE, serial_num.AsBytes, SP_MSG_256_SIZE);
	if (status != 0) {
		return status;
	}

	/* The subject name will be 32 bytes of the serial number result encoded in base64. */
	status = hsp->riot.base64->encode (hsp->riot.base64, serial_num.AsBytes, SHA256_HASH_LENGTH,
		(uint8_t*) hsp->riot.state->dev_id_name, sizeof (hsp->riot.state->dev_id_name));
	if (status != 0) {
		return status;
	}

	status = hsp->riot.x509->create_self_signed_certificate (hsp->riot.x509,
		&hsp->riot.state->dev_id_cert, hsp->riot.state->dev_id_der, hsp->riot.state->dev_id_length,
		HASH_TYPE_SHA256, serial_num.AsBytes, 8, hsp->riot.state->dev_id_name, X509_CERT_CA,
		hsp->riot.dev_id_ext, hsp->riot.dev_id_ext_count);
	if (status != 0) {
		return status;
	}

	hsp->riot.state->dev_id_attributes = dev_id_attributes;
	hsp->riot.state->dev_id_cert_valid = true;

	return 0;
}

int riot_core_hsp_shack1_generate_alias_key (const struct riot_core *riot, const uint8_t *fwid,
	size_t length)
{
	const struct riot_core_hsp_shack1 *hsp = (const struct riot_core_hsp_shack1*) riot;
	SP_ECDSA_P384_PUBLIC hmac_key;
	SP_MSG_256 serial_num;
	char alias_name[BASE64_LENGTH (SHA384_HASH_LENGTH)];
	uint8_t alias_der[RIOT_CORE_HSP_KSU_SLOT_MAX_DER_LENGTH];
	size_t alias_der_length;
	int status;

	if ((hsp == NULL) || (fwid == NULL) || (length == 0)) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	if (hsp->riot.state->dev_id_attributes == 0) {
		return RIOT_CORE_NO_DEVICE_ID;
	}

	/* Confirm that the FWID length is correct for the CDI.  CCS hardware requires that the value
	 * used in a KDF match the length of the source key.  In this case, it means that the FWID must
	 * match the length of the CDI. */
	if (length != SP_MSG_256_SIZE) {
		return RIOT_CORE_BAD_FWID_LENGTH;
	}

	/* Derive the Alias seed from the CDI based on the FWID value.
	 *
	 * Only the first 256 bits will be used for a 256-bit CDI, so the cast to a 384-bit value
	 * is safe due to the length and CDI pre-checks already performed. */
	status = hsp->riot.ccs->derive_key (hsp->riot.ccs, hsp->riot.cdi, (SP_MSG_384*) fwid,
		hsp->riot.alias_key, CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_KDF_KEY_ALLOWED |
		CCS_KSU_ATTR_ECC_SIGN_ALLOWED);
	if (status != 0) {
		return status;
	}

	/* The Alias certificate serial number and subject name will be derived from the Alias key.
	 * Since CDI key is no longer needed and has KDF functionality permitted, use the CDI key slot
	 * for the new key. */
	status = hsp->riot.ccs->derive_key (hsp->riot.ccs, hsp->riot.alias_key,
		&RIOT_CORE_HSP_SERIAL_NUM_KDF, hsp->riot.cdi,
		CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_ECC_SIGN_ALLOWED);
	if (status != 0) {
		return status;
	}

	/* Since SHACK1 KSU does not support HMAC operations, extract public key of the HMAC key.
	 * Serial number is the first 8 bytes of HMAC public key digest rather than an HMAC result. */
	status = hsp->riot.ccs->get_ecc_public_key (hsp->riot.ccs, hsp->riot.alias_key, &hmac_key,
		NULL);
	if (status != 0) {
		return status;
	}

	status = hsp->hash->calculate_sha256 (hsp->hash, hmac_key.AsBytes,
		SP_ECDSA_P256_PUBLIC_KEY_SIZE, serial_num.AsBytes, SP_MSG_256_SIZE);
	if (status != 0) {
		return status;
	}

	/* The subject name will be 32 bytes of the serial number result encoded in base64. */
	status = hsp->riot.base64->encode (hsp->riot.base64, serial_num.AsBytes, SHA256_HASH_LENGTH,
		(uint8_t*) alias_name, sizeof (alias_name));
	if (status != 0) {
		return status;
	}

	alias_der_length = asn1_encode_integer (hsp->riot.alias_key, alias_der, sizeof (alias_der));

	/* Generate the Alias X.509 certificate signed by the Device ID. */
	status = hsp->riot.x509->create_ca_signed_certificate (hsp->riot.x509,
		&hsp->riot.state->alias_cert, alias_der, alias_der_length, serial_num.AsBytes, 8,
		(char*) alias_name, X509_CERT_END_ENTITY, hsp->riot.state->dev_id_der,
		hsp->riot.state->dev_id_length, HASH_TYPE_SHA256, &hsp->riot.state->dev_id_cert,
		hsp->riot.alias_ext, hsp->riot.alias_ext_count);
	if (status != 0) {
		return status;
	}

	hsp->riot.state->alias_cert_valid = true;

	return 0;
}

int riot_core_hsp_shack1_get_alias_key (const struct riot_core *riot, uint8_t **key, size_t *length)
{
	const struct riot_core_hsp *hsp = (const struct riot_core_hsp*) riot;

	if (key == NULL) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	*key = NULL;
	if ((hsp == NULL) || (length == NULL)) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	if (!hsp->state->alias_cert_valid) {
		return RIOT_CORE_NO_ALIAS_KEY;
	}

	*key = platform_malloc (RIOT_CORE_HSP_KSU_SLOT_MAX_DER_LENGTH);
	if (*key == NULL) {
		return RIOT_CORE_NO_MEMORY;
	}

	/* This call won't fail since the buffer is valid and large enough for any byte value. */
	*length = asn1_encode_integer ((uint8_t) (uintptr_t) hsp->alias_key, *key,
		RIOT_CORE_HSP_KSU_SLOT_MAX_DER_LENGTH);

	return 0;
}

/**
 * Initialize only the variable state of an HSP DICE layer 0 handler.  The rest of the DICE handler
 * is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param hsp The HSP DICE handler that contains the state to initialize.
 *
 * @return 0 if the DICE state was successfully initialized or an error code.
 */
int riot_core_hsp_shack1_init_state (const struct riot_core_hsp_shack1 *hsp)
{
	int status = riot_core_hsp_init_state (&hsp->riot);

	if (status != 0) {
		return status;
	}

	if (hsp->hash == NULL) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	return 0;
}

/**
 * Initialize the DICE layer 0 handler for HSP platforms.  This implementation assumes the Device ID
 * key has already been generated, presumably by ROM, and is present in the KSU along with the CDI.
 *
 * @param hsp The HSP DICE handler to initialize.
 * @param state Variable context for the handler.  This must be uninitialized.
 * @param ccs Driver for the CCS instance that contains the CDI and Device ID.
 * @param base64 The base64 encoding engine to use with identity certificate generation.
 * @param x509 The X.509 engine to use for identity certificate generation.
 * @param hash The hash engine to use for identity certificate generation.
 * @param cdi_slot The KSU slot that contains the DICE CDI.
 * @param device_id_slot The KSU slot that contains the Device ID key.
 * @param alias_slot The KSU slot that will be used for the Alias key.
 * @param device_id_ext A list of additional, custom extensions that should be added to the
 * Device ID certificate and CSR.  At minimum, this should include the DICE TcbInfo extension for
 * layer 0.
 * @param device_id_ext_count The number of custom extensions to add to the Device ID certificate
 * and CSR.
 * @param alias_ext A list of additional, custom extensions that should be added to the
 * Alias certificate.  At minimum, this should include the DICE TcbInfo extension for layer 1.
 * @param alias_ext_count The number of custom extensions to add to the Alias certificate.
 *
 * @return 0 if the DICE handler was successfully initialized or an error code.
 */
int riot_core_hsp_shack1_init (struct riot_core_hsp_shack1 *hsp, struct riot_core_hsp_state *state,
	struct ccs_ksu_interface *ccs, const struct base64_engine *base64,
	const struct x509_engine *x509, const struct hash_engine *hash, uint8_t cdi_slot,
	uint8_t device_id_slot, uint8_t alias_slot,
	const struct x509_extension_builder *const *device_id_ext, size_t device_id_ext_count,
	const struct x509_extension_builder *const *alias_ext, size_t alias_ext_count)
{
	int status;

	if (hash == NULL) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	status = riot_core_hsp_init (&hsp->riot, state, ccs, base64, x509, cdi_slot, device_id_slot,
		alias_slot, device_id_ext, device_id_ext_count, 0, alias_ext, alias_ext_count);
	if (status != 0) {
		return status;
	}

	// Overwrite following API with derived functions
	hsp->riot.base.generate_device_id = riot_core_hsp_shack1_generate_device_id;
	hsp->riot.base.generate_alias_key = riot_core_hsp_shack1_generate_alias_key;
	hsp->riot.base.get_alias_key = riot_core_hsp_shack1_get_alias_key;

	hsp->hash = hash;

	return 0;
}

/**
 * Release an HSP DICE layer 0 handler.  This will wipe all memory used by the handler as well as
 * clear the KSU slots that contain the CDI and Device ID keys.
 *
 * @param hsp The HSP DICE handler to release.
 *
 * @return 0 if the DICE handler was successfully released or an error code.  Returning status from
 * this release call is necessary since a failure to clean up after DICE operations should block
 * execution of subsequent firmware stages.
 */
int riot_core_hsp_shack1_release (const struct riot_core_hsp_shack1 *hsp)
{
	return riot_core_hsp_release (&hsp->riot);
}
