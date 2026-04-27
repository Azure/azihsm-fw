// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "platform_api.h"
#include "riot_core_hsp.h"
#include "asn1/asn1_util.h"
#include "asn1/ecc_der_util.h"
#include "common/unused.h"
#include "logging/code_path_integrity.h"


/**
 * KDF context for generating the HMAC key for certificate serial numbers.  SHA384 hash of
 * "SerialNumber".
 */
const SP_MSG_384 RIOT_CORE_HSP_SERIAL_NUM_KDF = {
	.AsBytes = {
		0x13, 0xa6, 0x2f, 0x4b, 0x6b, 0xf3, 0x38, 0x53,
		0xc2, 0x9b, 0x8c, 0xa9, 0xd7, 0x1a, 0x29, 0x26,
		0xac, 0xba, 0xfc, 0xd4, 0x60, 0xf4, 0x90, 0x53,
		0x82, 0x5b, 0x27, 0x98, 0x44, 0x6a, 0x23, 0x4b,
		0xf9, 0x68, 0x34, 0x16, 0x48, 0x8e, 0xab, 0xaf,
		0x00, 0x2f, 0x86, 0xb3, 0x04, 0x13, 0x24, 0x6a
	}
};

/**
 * The value to use for a specific checkpoint step.  This uses the DICE L0 module ID to provide
 * uniqueness.
 */
#define	RIOT_CORE_HSP_CHKPT_VALUE(x)		((ROT_MODULE_RIOT_CORE << 8) | (x))

/**
 * Checkpoint values used when applying memory protections.
 */
enum {
	RIOT_CORE_HSP_CHKPT_ZEROIZE_START = RIOT_CORE_HSP_CHKPT_VALUE (0x01),	/**< Start to zeroize L0 context. */
	RIOT_CORE_HSP_CHKPT_FREE_CERTS = RIOT_CORE_HSP_CHKPT_VALUE (0x02),		/**< Free DICE certificate contexts. */
	RIOT_CORE_HSP_CHKPT_ZEROIZE_MEMORY = RIOT_CORE_HSP_CHKPT_VALUE (0x03),	/**< Cleared all execution state. */
	RIOT_CORE_HSP_CHKPT_ZEROIZE_CDI = RIOT_CORE_HSP_CHKPT_VALUE (0x04),		/**< Cleared the DICE CDI. */
	RIOT_CORE_HSP_CHKPT_ZEROIZE_DEVID = RIOT_CORE_HSP_CHKPT_VALUE (0x05),	/**< Cleared the Device ID private key. */
};


/**
 * Helper function that validates device ID and CDI keys and generates device ID HMAC key.
 *
 * @param hsp The HSP DICE handler to utilize.
 * @param hmac_attr HMAC key attributes to utilize.
 * @param dev_id_attributes Buffer to fill with Device ID key attributes.
 *
 * @return 0 if completed successfully or an error code.
 */
int riot_core_hsp_generate_device_id_setup_keys (const struct riot_core_hsp *hsp,
	uint32_t hmac_attr, uint32_t *dev_id_attributes)
{
	int status;

	if (hsp == NULL) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	/* First, determine the key length being used based on the attributes in the key slots. */
	status = hsp->ccs->get_key_attributes (hsp->ccs, hsp->cdi, &hsp->state->cdi_attributes);
	if (status != 0) {
		return status;
	}

	if (!(hsp->state->cdi_attributes & CCS_KSU_ATTR_KDF_KEY_ALLOWED)) {
		/* The CDI cannot be used for further KDFs. */
		return RIOT_CORE_NO_CDI;
	}

	status = hsp->ccs->get_key_attributes (hsp->ccs, hsp->device_id_key, dev_id_attributes);
	if (status != 0) {
		return status;
	}

	if (!(*dev_id_attributes & CCS_KSU_ATTR_ECC_SIGN_ALLOWED)) {
		/* The Device ID is not an ECC key. */
		return RIOT_CORE_NO_DEVICE_ID;
	}

	if (!(hsp->state->cdi_attributes & CCS_KSU_ATTR_KEY_SIZE_384) &&
		(*dev_id_attributes & CCS_KSU_ATTR_KEY_SIZE_384)) {
		/* The CDI is 256 bits and the Device ID is 384 bits.  It is not possible to generate a
		 * 384-bit key from a 256-bit seed, so the specified CDI could not have been used for the
		 * Device ID. */
		return RIOT_CORE_BAD_CDI;
	}

	/* If the Device ID is a 384-bit key, align HMAC attributes to this key size. */
	if (*dev_id_attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		hmac_attr |= CCS_KSU_ATTR_KEY_SIZE_384;
	}

	hsp->state->dev_id_length = asn1_encode_integer (hsp->device_id_key, hsp->state->dev_id_der,
		sizeof (hsp->state->dev_id_der));

	/* The Device ID certificate serial number and subject name will be derived from the CDI through
	 * an HMAC KDF.  Since the CDI doesn't support HMAC functionality, a new HMAC key needs to be
	 * derived from the CDI to support this flow.  Temporarily use the Alias key slot for the HMAC
	 * key. */
	return hsp->ccs->derive_key (hsp->ccs, hsp->cdi, &RIOT_CORE_HSP_SERIAL_NUM_KDF, hsp->alias_key,
		hmac_attr);
}

int riot_core_hsp_generate_device_id (const struct riot_core *riot, const uint8_t *cdi,
	size_t length)
{
	const struct riot_core_hsp *hsp = (const struct riot_core_hsp*) riot;
	uint32_t hmac_attr = CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED;
	enum hash_type sig_hash = HASH_TYPE_SHA256;
	size_t serial_length = SHA256_HASH_LENGTH;
	SP_MSG_384 serial_num;
	uint32_t dev_id_attributes;
	int cert_type;
	int status;

	/* The CDI is already in the KSU for this implementation. */
	UNUSED (cdi);
	UNUSED (length);

	status = riot_core_hsp_generate_device_id_setup_keys (hsp, hmac_attr, &dev_id_attributes);
	if (status != 0) {
		return status;
	}

	/* If the Device ID is a 384-bit key, align everything to this key size. */
	if (dev_id_attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		sig_hash = HASH_TYPE_SHA384;
		serial_length = SHA384_HASH_LENGTH;
	}

	/* Determine the serial number, which will be the first 8 bytes from the result of the KDF. */
	status = hsp->ccs->hmac (hsp->ccs, hsp->alias_key, RIOT_CORE_SERIAL_NUM_KDF_DATA,
		RIOT_CORE_SERIAL_NUM_KDF_DATA_LENGTH, &serial_num, NULL);
	if (status != 0) {
		return status;
	}

	/* The subject name will be the entire serial number result encoded in base64. */
	status = hsp->base64->encode (hsp->base64, serial_num.AsBytes, serial_length,
		(uint8_t*) hsp->state->dev_id_name, sizeof (hsp->state->dev_id_name));
	if (status != 0) {
		return status;
	}

	if (hsp->dev_id_pathlen <= X509_CERT_MAX_PATHLEN) {
		cert_type = X509_CERT_CA_PATHLEN (hsp->dev_id_pathlen);
	}
	else {
		cert_type = X509_CERT_CA_NO_PATHLEN;
	}

	status = hsp->x509->create_self_signed_certificate (hsp->x509, &hsp->state->dev_id_cert,
		hsp->state->dev_id_der, hsp->state->dev_id_length, sig_hash, serial_num.AsBytes, 8,
		hsp->state->dev_id_name, cert_type, hsp->dev_id_ext, hsp->dev_id_ext_count);
	if (status != 0) {
		return status;
	}

	hsp->state->dev_id_attributes = dev_id_attributes;
	hsp->state->dev_id_cert_valid = true;

	return 0;
}

int riot_core_hsp_get_device_id_csr (const struct riot_core *riot, const uint8_t *oid,
	size_t oid_length, uint8_t **csr, size_t *length)
{
	const struct riot_core_hsp *hsp = (const struct riot_core_hsp*) riot;
	enum hash_type sig_hash;
	int cert_type;

	if ((hsp == NULL) || (csr == NULL) || (length == NULL)) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	if (hsp->state->dev_id_attributes == 0) {
		return RIOT_CORE_NO_DEVICE_ID;
	}

	if (hsp->state->dev_id_attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		sig_hash = HASH_TYPE_SHA384;
	}
	else {
		sig_hash = HASH_TYPE_SHA256;
	}

	if (hsp->dev_id_pathlen <= X509_CERT_MAX_PATHLEN) {
		cert_type = X509_CERT_CA_PATHLEN (hsp->dev_id_pathlen);
	}
	else {
		cert_type = X509_CERT_CA_NO_PATHLEN;
	}

	return hsp->x509->create_csr (hsp->x509, hsp->state->dev_id_der, hsp->state->dev_id_length,
		sig_hash, hsp->state->dev_id_name, cert_type, oid, oid_length, hsp->dev_id_ext,
		hsp->dev_id_ext_count, csr, length);
}

int riot_core_hsp_get_device_id_cert (const struct riot_core *riot, uint8_t **device_id,
	size_t *length)
{
	const struct riot_core_hsp *hsp = (const struct riot_core_hsp*) riot;

	if ((hsp == NULL) || (device_id == NULL) || (length == NULL)) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	if (hsp->state->dev_id_attributes == 0) {
		return RIOT_CORE_NO_DEVICE_ID;
	}

	return hsp->x509->get_certificate_der (hsp->x509, &hsp->state->dev_id_cert, device_id, length);
}

/**
 * Prepare for Alias Key generation.  This will generate the Alias CDI based on the FWID and produce
 * the serial number to use for the Alias certificate.
 *
 * @param hsp HSP DICE handler used for Alias Key generation.
 * @param fwid The FWID to use for Alias Key derivation.
 * @param length Length of the FWID.
 * @param key_size Output for the length of the Alias Key being generated.  This is represented as a
 * CCS key attribute.
 * @param sig_hash Output for the hash algorithm to use for certificate signing.
 * @param serial_num Output for the Alias certificate serial number.
 * @param alias_name Output for the Alias certificate subject name.
 *
 * @return 0 if the Alias key is ready to be generated or an error code.
 */
int riot_core_hsp_generate_alias_key_setup_keys (const struct riot_core_hsp *hsp,
	const uint8_t *fwid, size_t length, uint32_t *key_size, enum hash_type *sig_hash,
	SP_MSG_384 *serial_num, char alias_name[BASE64_LENGTH (SHA384_HASH_LENGTH)])
{
	size_t serial_length;
	int status;

	if ((hsp == NULL) || (fwid == NULL) || (length == 0)) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	if (hsp->state->dev_id_attributes == 0) {
		return RIOT_CORE_NO_DEVICE_ID;
	}

	/* Confirm that the FWID length is correct for the CDI.  CCS hardware requires that the value
	 * used in a KDF match the length of the source key.  In this case, it means that the FWID must
	 * match the length of the CDI. */
	if (hsp->state->cdi_attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		if (length != SP_MSG_384_SIZE) {
			return RIOT_CORE_BAD_FWID_LENGTH;
		}
	}
	else if (length != SP_MSG_256_SIZE) {
		return RIOT_CORE_BAD_FWID_LENGTH;
	}

	/* Derive the Alias key to match the Device ID key length. */
	if (hsp->state->dev_id_attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		*key_size = CCS_KSU_ATTR_KEY_SIZE_384;
		*sig_hash = HASH_TYPE_SHA384;
		serial_length = SHA384_HASH_LENGTH;
	}
	else {
		*key_size = 0;
		*sig_hash = HASH_TYPE_SHA256;
		serial_length = SHA256_HASH_LENGTH;
	}

	/* Derive the Alias seed from the CDI based on the FWID value.
	 *
	 * Only the first 256 bits will be used for a 256-bit CDI, so the cast to a 384-bit value
	 * is safe due to the length and CDI pre-checks already performed. */
	status = hsp->ccs->derive_key (hsp->ccs, hsp->cdi, (SP_MSG_384*) fwid, hsp->alias_key,
		CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED | *key_size);
	if (status != 0) {
		return status;
	}

	/* Determine the serial number, which will be the first 8 bytes from the result of the KDF. */
	status = hsp->ccs->hmac (hsp->ccs, hsp->alias_key, RIOT_CORE_SERIAL_NUM_KDF_DATA,
		RIOT_CORE_SERIAL_NUM_KDF_DATA_LENGTH, serial_num, NULL);
	if (status != 0) {
		return status;
	}

	/* The subject name will be the entire serial number result encoded in base64. */
	return hsp->base64->encode (hsp->base64, serial_num->AsBytes, serial_length,
		(uint8_t*) alias_name, BASE64_LENGTH (SHA384_HASH_LENGTH));
}

/**
 * Create the X.509 certificate for the Alias Key that is signed with the Device ID.
 *
 * @param hsp HSP DICE handler used for Alias Key generation.
 * @param sig_hash Hash algorithm to use for certificate signature generation.
 * @param serial_num Serial number to assign to the Alias certificate.  Only the first 8 bytes will
 * be used.
 * @param alias_name Subject name to use for the Alias certificate.
 *
 * @return 0 if the certificate was create successfully or an error code.
 */
int riot_core_hsp_generate_alias_key_create_certificate (const struct riot_core_hsp *hsp,
	enum hash_type sig_hash, const SP_MSG_384 *serial_num, const char *alias_name)
{
	uint8_t alias_der[RIOT_CORE_HSP_KSU_SLOT_MAX_DER_LENGTH];
	size_t alias_der_length;
	int cert_type;
	int status;

	alias_der_length = asn1_encode_integer (hsp->alias_key, alias_der, sizeof (alias_der));

	if (hsp->alias_pathlen < 0) {
		cert_type = X509_CERT_END_ENTITY;
	}
	else if (hsp->alias_pathlen <= X509_CERT_MAX_PATHLEN) {
		cert_type = X509_CERT_CA_PATHLEN (hsp->alias_pathlen);
	}
	else {
		cert_type = X509_CERT_CA_NO_PATHLEN;
	}

	/* Generate the Alias X.509 certificate signed by the Device ID. */
	status = hsp->x509->create_ca_signed_certificate (hsp->x509, &hsp->state->alias_cert, alias_der,
		alias_der_length, serial_num->AsBytes, 8, alias_name, cert_type, hsp->state->dev_id_der,
		hsp->state->dev_id_length, sig_hash, &hsp->state->dev_id_cert, hsp->alias_ext,
		hsp->alias_ext_count);
	if (status != 0) {
		return status;
	}

	hsp->state->alias_cert_valid = true;

	return 0;
}

int riot_core_hsp_generate_alias_key (const struct riot_core *riot, const uint8_t *fwid,
	size_t length)
{
	const struct riot_core_hsp *hsp = (const struct riot_core_hsp*) riot;
	uint32_t key_size;
	enum hash_type sig_hash;
	SP_MSG_384 serial_num;
	char alias_name[BASE64_LENGTH (SHA384_HASH_LENGTH)];
	int status;

	status = riot_core_hsp_generate_alias_key_setup_keys (hsp, fwid, length, &key_size, &sig_hash,
		&serial_num, alias_name);
	if (status != 0) {
		return status;
	}

	/* Get the Alias private key and store it in the KSU.  It will also be directly available to
	 * firmware for flows that CCS can't support. */
	status = hsp->ccs->derive_fw_ecc_key (hsp->ccs, hsp->alias_key, &hsp->state->alias_priv_key,
		CCS_KSU_ATTR_ECC_SIGN_ALLOWED | key_size);
	if (status != 0) {
		return status;
	}

	status = hsp->ccs->set_key (hsp->ccs, &hsp->state->alias_priv_key, hsp->alias_key,
		CCS_KSU_ATTR_ECC_SIGN_ALLOWED | key_size);
	if (status != 0) {
		return status;
	}

	return riot_core_hsp_generate_alias_key_create_certificate (hsp, sig_hash, &serial_num,
		alias_name);
}

int riot_core_hsp_get_alias_key (const struct riot_core *riot, uint8_t **key, size_t *length)
{
	const struct riot_core_hsp *hsp = (const struct riot_core_hsp*) riot;
	size_t key_length;
	size_t der_length;

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

	if (hsp->state->dev_id_attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		key_length = SP_MSG_384_SIZE;
		der_length = ECC_DER_P384_PRIVATE_NO_PUB_LENGTH;
	}
	else {
		key_length = SP_MSG_256_SIZE;
		der_length = ECC_DER_P256_PRIVATE_NO_PUB_LENGTH;
	}

	*key = platform_malloc (der_length);
	if (*key == NULL) {
		return RIOT_CORE_NO_MEMORY;
	}

	*length = ecc_der_encode_private_key (hsp->state->alias_priv_key.AsBytes, NULL, NULL,
		key_length, *key, der_length);

	return 0;
}

int riot_core_hsp_get_alias_key_cert (const struct riot_core *riot, uint8_t **alias_key,
	size_t *length)
{
	const struct riot_core_hsp *hsp = (const struct riot_core_hsp*) riot;

	if ((hsp == NULL) || (alias_key == NULL) || (length == NULL)) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	if (!hsp->state->alias_cert_valid) {
		return RIOT_CORE_NO_ALIAS_KEY;
	}

	return hsp->x509->get_certificate_der (hsp->x509, &hsp->state->alias_cert, alias_key, length);
}

/**
 * Common initialization for the HSP DICE layer 0 handler.
 *
 * The Alias key generation function will remain unset by this call and must be set by the caller.
 *
 * @param hsp The HSP DICE handler to initialize.
 * @param state Variable context for the handler.  This must be uninitialized.
 * @param ccs Driver for the CCS instance that contains the CDI and Device ID.
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
 * path length constraint.  If the Alias certificate is a CA, this value must be at least 1.
 * @param alias_ext A list of additional, custom extensions that should be added to the
 * Alias certificate.  At minimum, this should include the DICE TcbInfo extension for layer 1.
 * @param alias_ext_count The number of custom extensions to add to the Alias certificate.
 * @param alias_pathlen The path length to use for the Alias CA certificate.  Use a value
 * larger than X509_CERT_MAX_PATHLEN to indicate the certificate should be generated without any
 * path length constraint.  Set this to -1 to make the Alias certificate an end entity.
 *
 * @return 0 if the DICE handler was successfully initialized or an error code.
 */
int riot_core_hsp_init_internal (struct riot_core_hsp *hsp, struct riot_core_hsp_state *state,
	struct ccs_ksu_interface *ccs, const struct base64_engine *base64,
	const struct x509_engine *x509, uint8_t cdi_slot, uint8_t device_id_slot, uint8_t alias_slot,
	const struct x509_extension_builder *const *device_id_ext, size_t device_id_ext_count,
	uint8_t device_id_pathlen, const struct x509_extension_builder *const *alias_ext,
	size_t alias_ext_count, int alias_pathlen)
{
	if (hsp == NULL) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	memset (hsp, 0, sizeof (struct riot_core_hsp));

	hsp->base.generate_device_id = riot_core_hsp_generate_device_id;
	hsp->base.get_device_id_csr = riot_core_hsp_get_device_id_csr;
	hsp->base.get_device_id_cert = riot_core_hsp_get_device_id_cert;
	hsp->base.get_alias_key = riot_core_hsp_get_alias_key;
	hsp->base.get_alias_key_cert = riot_core_hsp_get_alias_key_cert;

	hsp->state = state;
	hsp->ccs = ccs;
	hsp->base64 = base64;
	hsp->x509 = x509;
	hsp->dev_id_ext = device_id_ext;
	hsp->dev_id_ext_count = device_id_ext_count;
	hsp->dev_id_pathlen = device_id_pathlen;
	hsp->alias_ext = alias_ext;
	hsp->alias_ext_count = alias_ext_count;
	hsp->alias_pathlen = alias_pathlen;
	hsp->cdi = cdi_slot;
	hsp->device_id_key = device_id_slot;
	hsp->alias_key = alias_slot;

	return riot_core_hsp_init_state (hsp);
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
int riot_core_hsp_init (struct riot_core_hsp *hsp, struct riot_core_hsp_state *state,
	struct ccs_ksu_interface *ccs, const struct base64_engine *base64,
	const struct x509_engine *x509, uint8_t cdi_slot, uint8_t device_id_slot, uint8_t alias_slot,
	const struct x509_extension_builder *const *device_id_ext, size_t device_id_ext_count,
	uint8_t device_id_pathlen, const struct x509_extension_builder *const *alias_ext,
	size_t alias_ext_count)
{
	int status;

	status = riot_core_hsp_init_internal (hsp, state, ccs, base64, x509, cdi_slot, device_id_slot,
		alias_slot, device_id_ext, device_id_ext_count, device_id_pathlen, alias_ext,
		alias_ext_count, -1);
	if (status == 0) {
		hsp->base.generate_alias_key = riot_core_hsp_generate_alias_key;
	}

	return status;
}

/**
 * Initialize the DICE layer 0 handler for HSP platforms.  This implementation assumes the Device ID
 * key has already been generated, presumably by ROM, and is present in the KSU along with the CDI.
 *
 * The Alias Certificate that is generated will be a CA certificate.
 *
 * @param hsp The HSP DICE handler to initialize.
 * @param state Variable context for the handler.  This must be uninitialized.
 * @param ccs Driver for the CCS instance that contains the CDI and Device ID.
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
int riot_core_hsp_init_alias_ca (struct riot_core_hsp *hsp, struct riot_core_hsp_state *state,
	struct ccs_ksu_interface *ccs, const struct base64_engine *base64,
	const struct x509_engine *x509, uint8_t cdi_slot, uint8_t device_id_slot, uint8_t alias_slot,
	const struct x509_extension_builder *const *device_id_ext, size_t device_id_ext_count,
	uint8_t device_id_pathlen, const struct x509_extension_builder *const *alias_ext,
	size_t alias_ext_count, uint8_t alias_pathlen)
{
	int status;

	status = riot_core_hsp_init_internal (hsp, state, ccs, base64, x509, cdi_slot, device_id_slot,
		alias_slot, device_id_ext, device_id_ext_count, device_id_pathlen, alias_ext,
		alias_ext_count, alias_pathlen);
	if (status == 0) {
		hsp->base.generate_alias_key = riot_core_hsp_generate_alias_key;
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
int riot_core_hsp_init_state (const struct riot_core_hsp *hsp)
{
	if ((hsp == NULL) || (hsp->state == NULL) || (hsp->ccs == NULL) || (hsp->base64 == NULL) ||
		(hsp->x509 == NULL)) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	/* There must always be at least two extensions for the Device ID (TcbInfo, Ueid) and one
	 * extension for the Alias (TcbInfo). */
	if ((hsp->dev_id_ext == NULL) || (hsp->dev_id_ext_count < 2) || (hsp->alias_ext == NULL) ||
		(hsp->alias_ext_count == 0)) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	/* When the Alias is a CA certificate, the Device ID cert must at least be pathlen 1. */
	if ((hsp->alias_pathlen >= 0) && (hsp->dev_id_pathlen < 1)) {
		return RIOT_CORE_BAD_PATHLEN_CONSTRAINT;
	}

	memset (hsp->state, 0, sizeof (struct riot_core_hsp_state));

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
int riot_core_hsp_release (const struct riot_core_hsp *hsp)
{
	SP_MSG_384 zero = {{0}};
	int status;

	code_path_integrity_secure_message_no_trace (RIOT_CORE_HSP_CHKPT_ZEROIZE_START);

	if (hsp == NULL) {
		return RIOT_CORE_INVALID_ARGUMENT;
	}

	/* Release certificate contexts. */
	if (hsp->state->dev_id_cert_valid) {
		hsp->x509->release_certificate (hsp->x509, &hsp->state->dev_id_cert);
	}

	if (hsp->state->alias_cert_valid) {
		hsp->x509->release_certificate (hsp->x509, &hsp->state->alias_cert);
	}

	code_path_integrity_secure_message_no_trace (RIOT_CORE_HSP_CHKPT_FREE_CERTS);

	/* Clear DICE execution context. */
	riot_core_clear (hsp->state, sizeof (struct riot_core_hsp_state));

	code_path_integrity_secure_message_no_trace (RIOT_CORE_HSP_CHKPT_ZEROIZE_MEMORY);

	/* Clear the CDI and Device ID KSU slots. */
	status = hsp->ccs->set_key (hsp->ccs, &zero, hsp->cdi, 0);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (RIOT_CORE_HSP_CHKPT_ZEROIZE_CDI ^ status);

	status = hsp->ccs->set_key (hsp->ccs, &zero, hsp->device_id_key, 0);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (RIOT_CORE_HSP_CHKPT_ZEROIZE_DEVID ^ status);

	return 0;
}
