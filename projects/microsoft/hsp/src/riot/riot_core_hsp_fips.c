// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "platform_api.h"
#include "riot_core_hsp_fips.h"
#include "asn1/asn1_util.h"
#include "asn1/ecc_der_util.h"
#include "common/unused.h"


int riot_core_hsp_fips_generate_alias_key (const struct riot_core *riot, const uint8_t *fwid,
	size_t length)
{
	const struct riot_core_hsp_fips *hsp = (const struct riot_core_hsp_fips*) riot;
	uint32_t key_size = 0;
	enum hash_type sig_hash = HASH_TYPE_SHA256;
	SP_MSG_384 serial_num;
	char alias_name[BASE64_LENGTH (SHA384_HASH_LENGTH)];
	int status;

	status = riot_core_hsp_generate_alias_key_setup_keys (&hsp->base, fwid, length, &key_size,
		&sig_hash, &serial_num, alias_name);
	if (status != 0) {
		return status;
	}

	/* Derive the Alias private key and store it in the KSU.  It will also be directly available to
	 * firmware for flows that CCS can't support.
	 *
	 * First, the Alias CDI needs to be regenerated to allow KDF instead of FW key derivations. */
	status = hsp->base.ccs->derive_key (hsp->base.ccs, hsp->base.cdi, (SP_MSG_384*) fwid,
		hsp->base.alias_key,
		CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_KDF_KEY_ALLOWED | key_size);
	if (status != 0) {
		return status;
	}

	status = ccs_ksu_interface_derive_ecdsa_key (hsp->base.ccs, hsp->ecc, hsp->hash,
		hsp->base.alias_key, hsp->base.alias_key, CCS_KSU_ATTR_ECC_SIGN_ALLOWED | key_size, NULL);
	if (status != 0) {
		return status;
	}

	status = hsp->base.ccs->export_fw_ecc_key (hsp->base.ccs, hsp->base.alias_key,
		&hsp->state->base.alias_priv_key, NULL);
	if (status != 0) {
		return status;
	}

	return riot_core_hsp_generate_alias_key_create_certificate (&hsp->base, sig_hash, &serial_num,
		alias_name);
}


/**
 * Initialize the FIPS compliant DICE layer 0 handler for HSP platforms.  This implementation
 * assumes the Device ID key has already been generated, presumably by ROM, and is present in the
 * KSU along with the CDI.
 *
 * Alias key derivation will be handled in a FIPS compliant manner.
 *
 * @param hsp The HSP DICE handler to initialize.
 * @param state Variable context for the handler.  This must be uninitialized.
 * @param ccs Driver for the CCS instance that contains the CDI and Device ID.  This must be a
 * FIPS compliant implementation.
 * @param ecc Driver for the PKA instance that will be used for PCT.
 * @param hash Hash engine that will be used for PCT.
 * @param base64 The base64 encoding engine to use with identity certificate generation.
 * @param x509 The X.509 engine to use for identity certificate generation.
 * @param cdi_slot The KSU slot that contains the DICE CDI.
 * @param device_id_slot The KSU slot that contains the Device ID key.
 * @param alias_slot The KSU slot that will be used for the Alias key.
 * @param device_id_ext A list of additional, custom extensions that should be added to the
 * Device ID certificate and CSR.  At minimum, this should include the DICE TcbInfo extension for
 * layer 0.
 * @param device_id_ext_count The number of custom extensions to add to the Device ID certificate
 * and CSR.
 * @param device_id_pathlen The path length to use for the Device ID CA certificate.  Use a value
 * larger than X509_CERT_MAX_PATHLEN to indicate the certificate should be generated without any
 * path length constraint.
 * @param alias_ext A list of additional, custom extensions that should be added to the
 * Alias certificate.  At minimum, this should include the DICE TcbInfo extension for layer 1.
 * @param alias_ext_count The number of custom extensions to add to the Alias certificate.
 *
 * @return 0 if the DICE handler was successfully initialized or an error code.
 */
int riot_core_hsp_fips_init (struct riot_core_hsp_fips *hsp, struct riot_core_hsp_fips_state *state,
	struct ccs_ksu_interface *ccs, const struct ecc_hw *ecc, const struct hash_engine *hash,
	const struct base64_engine *base64, const struct x509_engine *x509, uint8_t cdi_slot,
	uint8_t device_id_slot, uint8_t alias_slot,
	const struct x509_extension_builder *const *device_id_ext, size_t device_id_ext_count,
	uint8_t device_id_pathlen, const struct x509_extension_builder *const *alias_ext,
	size_t alias_ext_count)
{
	int status;

	if ((hsp == NULL) || (state == NULL) || (ecc == NULL) || (hash == NULL)) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	memset (hsp, 0, sizeof (struct riot_core_hsp_fips));

	status = riot_core_hsp_init_internal (&hsp->base, &state->base, ccs, base64, x509, cdi_slot,
		device_id_slot, alias_slot, device_id_ext, device_id_ext_count, device_id_pathlen,
		alias_ext, alias_ext_count, -1);
	if (status == 0) {
		hsp->base.base.generate_alias_key = riot_core_hsp_fips_generate_alias_key;

		hsp->state = state;
		hsp->ecc = ecc;
		hsp->hash = hash;
	}

	return status;
}

/**
 * Initialize the FIPS compliant DICE layer 0 handler for HSP platforms.  This implementation
 * assumes the Device ID key has already been generated, presumably by ROM, and is present in the
 * KSU along with the CDI.
 *
 * The Alias Certificate that is generated will be a CA certificate.
 *
 * @param hsp The HSP DICE handler to initialize.
 * @param state Variable context for the handler.  This must be uninitialized.
 * @param ccs Driver for the CCS instance that contains the CDI and Device ID.  This must be a
 * FIPS compliant implementation.
 * @param ecc Driver for the PKA instance that will be used for PCT.
 * @param hash Hash engine that will be used for PCT.
 * @param base64 The base64 encoding engine to use with identity certificate generation.
 * @param x509 The X.509 engine to use for identity certificate generation.
 * @param cdi_slot The KSU slot that contains the DICE CDI.
 * @param device_id_slot The KSU slot that contains the Device ID key.
 * @param alias_slot The KSU slot that will be used for the Alias key.
 * @param device_id_ext A list of additional, custom extensions that should be added to the
 * Device ID certificate and CSR.  At minimum, this should include the DICE TcbInfo extension for
 * layer 0.
 * @param device_id_ext_count The number of custom extensions to add to the Device ID certificate
 * and CSR.
 * @param device_id_pathlen The path length to use for the Device ID CA certificate.  Use a value
 * larger than X509_CERT_MAX_PATHLEN to indicate the certificate should be generated without any
 * path length constraint.  This value must be at least 1 to account for the Alias CA certificate.
 * @param alias_ext A list of additional, custom extensions that should be added to the
 * Alias certificate.  At minimum, this should include the DICE TcbInfo extension for layer 1.
 * @param alias_ext_count The number of custom extensions to add to the Alias certificate.
 * @param alias_pathlen The path length to use for the Alias CA certificate.  Use a value
 * larger than X509_CERT_MAX_PATHLEN to indicate the certificate should be generated without any
 * path length constraint.
 *
 * @return 0 if the DICE handler was successfully initialized or an error code.
 */
int riot_core_hsp_fips_init_alias_ca (struct riot_core_hsp_fips *hsp,
	struct riot_core_hsp_fips_state *state, struct ccs_ksu_interface *ccs, const struct ecc_hw *ecc,
	const struct hash_engine *hash, const struct base64_engine *base64,
	const struct x509_engine *x509, uint8_t cdi_slot, uint8_t device_id_slot, uint8_t alias_slot,
	const struct x509_extension_builder *const *device_id_ext, size_t device_id_ext_count,
	uint8_t device_id_pathlen, const struct x509_extension_builder *const *alias_ext,
	size_t alias_ext_count, uint8_t alias_pathlen)
{
	int status;

	if ((hsp == NULL) || (state == NULL) || (ecc == NULL) || (hash == NULL)) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	memset (hsp, 0, sizeof (struct riot_core_hsp_fips));

	status = riot_core_hsp_init_internal (&hsp->base, &state->base, ccs, base64, x509, cdi_slot,
		device_id_slot, alias_slot, device_id_ext, device_id_ext_count, device_id_pathlen,
		alias_ext, alias_ext_count, alias_pathlen);
	if (status == 0) {
		hsp->base.base.generate_alias_key = riot_core_hsp_fips_generate_alias_key;

		hsp->state = state;
		hsp->ecc = ecc;
		hsp->hash = hash;
	}

	return status;
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
int riot_core_hsp_fips_init_state (const struct riot_core_hsp_fips *hsp)
{
	if ((hsp == NULL) || (hsp->state == NULL) || (hsp->ecc == NULL) || (hsp->hash == NULL)) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	return riot_core_hsp_init_state (&hsp->base);
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
int riot_core_hsp_fips_release (const struct riot_core_hsp_fips *hsp)
{
	if (hsp == NULL) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	return riot_core_hsp_release (&hsp->base);
}
