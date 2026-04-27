// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef RIOT_CORE_HSP_H_
#define RIOT_CORE_HSP_H_

#include "asn1/base64.h"
#include "asn1/x509.h"
#include "asn1/x509_extension_builder.h"
#include "crypto/hash.h"
#include "drivers/ccs_ksu_interface.h"
#include "riot/riot_core.h"


/**
 * The maximum length required to encode a KSU slot number.
 */
#define	RIOT_CORE_HSP_KSU_SLOT_MAX_DER_LENGTH		4

extern const SP_MSG_384 RIOT_CORE_HSP_SERIAL_NUM_KDF;

/**
 * Variable context for DICE layer 0 on HSP.
 */
struct riot_core_hsp_state {
	uint32_t cdi_attributes;									/**< KSU attributes for the CDI. */
	uint32_t dev_id_attributes;									/**< KSU attributes for the Device ID key. */
	uint8_t dev_id_der[RIOT_CORE_HSP_KSU_SLOT_MAX_DER_LENGTH];	/**< Encoded KSU slot number of the Device ID. */
	size_t dev_id_length;										/**< Length of the Device ID KSU slot. */
	char dev_id_name[BASE64_LENGTH (SHA384_HASH_LENGTH)];		/**< The name for the Device ID cert. */
	struct x509_certificate dev_id_cert;						/**< The X.509 certificate for the Device ID. */
	SP_MSG_384 alias_priv_key;									/**< The Alias private key. */
	struct x509_certificate alias_cert;							/**< The X.509 certificate for the Alias key. */
	bool dev_id_cert_valid;										/**< Flag indicating validity of the Device ID cert. */
	bool alias_cert_valid;										/**< Flag indicating validity of the Alias key cert. */
};

/**
 * DICE layer 0 implementation using HSP with keys and CDI stored in CCS.
 */
struct riot_core_hsp {
	struct riot_core base;									/**< Base API instance. */
	struct riot_core_hsp_state *state;						/**< Variable context for DICE. */
	const struct ccs_ksu_interface *ccs;					/**< Driver for the CCS to use for DICE. */
	const struct base64_engine *base64;						/**< Base64 encoding engine to use for DICE. */
	const struct x509_engine *x509;							/**< X.509 engine to use for DICE. */
	const struct x509_extension_builder *const *dev_id_ext;	/**< List of custom extensions added to the Device ID certificate. */
	size_t dev_id_ext_count;								/**< Number of custom extensions in the Device ID certificate. */
	uint8_t dev_id_pathlen;									/**< The path length constraint to use for the Device ID certificate. */
	const struct x509_extension_builder *const *alias_ext;	/**< List of custom extensions added to the Alias certificate. */
	size_t alias_ext_count;									/**< Number of custom extensions in the Alias certificate. */
	int alias_pathlen;										/**< The path length constraint if the Alias is a CA. */
	uint8_t cdi;											/**< KSU slot that contains the CDI. */
	uint8_t device_id_key;									/**< KSU slot that contains the Device ID key. */
	uint8_t alias_key;										/**< KSU slot that contains the Alias key. */
};


int riot_core_hsp_init (struct riot_core_hsp *hsp, struct riot_core_hsp_state *state,
	struct ccs_ksu_interface *ccs, const struct base64_engine *base64,
	const struct x509_engine *x509, uint8_t cdi_slot, uint8_t device_id_slot, uint8_t alias_slot,
	const struct x509_extension_builder *const *device_id_ext, size_t device_id_ext_count,
	uint8_t device_id_pathlen, const struct x509_extension_builder *const *alias_ext,
	size_t alias_ext_count);
int riot_core_hsp_init_alias_ca (struct riot_core_hsp *hsp, struct riot_core_hsp_state *state,
	struct ccs_ksu_interface *ccs, const struct base64_engine *base64,
	const struct x509_engine *x509, uint8_t cdi_slot, uint8_t device_id_slot, uint8_t alias_slot,
	const struct x509_extension_builder *const *device_id_ext, size_t device_id_ext_count,
	uint8_t device_id_pathlen, const struct x509_extension_builder *const *alias_ext,
	size_t alias_ext_count, uint8_t alias_pathlen);
int riot_core_hsp_init_state (const struct riot_core_hsp *hsp);
int riot_core_hsp_release (const struct riot_core_hsp *hsp);

/* Internal functions for use by derived types. */
int riot_core_hsp_init_internal (struct riot_core_hsp *hsp, struct riot_core_hsp_state *state,
	struct ccs_ksu_interface *ccs, const struct base64_engine *base64,
	const struct x509_engine *x509, uint8_t cdi_slot, uint8_t device_id_slot, uint8_t alias_slot,
	const struct x509_extension_builder *const *device_id_ext, size_t device_id_ext_count,
	uint8_t device_id_pathlen, const struct x509_extension_builder *const *alias_ext,
	size_t alias_ext_count, int alias_pathlen);

int riot_core_hsp_generate_device_id_setup_keys (const struct riot_core_hsp *hsp,
	uint32_t hmac_attr, uint32_t *dev_id_attributes);
int riot_core_hsp_generate_device_id (const struct riot_core *riot, const uint8_t *cdi,
	size_t length);
int riot_core_hsp_get_device_id_csr (const struct riot_core *riot, const uint8_t *oid,
	size_t oid_length, uint8_t **csr, size_t *length);
int riot_core_hsp_get_device_id_cert (const struct riot_core *riot, uint8_t **device_id,
	size_t *length);
int riot_core_hsp_generate_alias_key_setup_keys (const struct riot_core_hsp *hsp,
	const uint8_t *fwid, size_t length, uint32_t *key_size, enum hash_type *sig_hash,
	SP_MSG_384 *serial_num, char alias_name[BASE64_LENGTH (SHA384_HASH_LENGTH)]);
int riot_core_hsp_generate_alias_key_create_certificate (const struct riot_core_hsp *hsp,
	enum hash_type sig_hash, const SP_MSG_384 *serial_num, const char *alias_name);
int riot_core_hsp_get_alias_key (const struct riot_core *riot, uint8_t **key, size_t *length);
int riot_core_hsp_get_alias_key_cert (const struct riot_core *riot, uint8_t **alias_key,
	size_t *length);


#endif	/* RIOT_CORE_HSP_H_ */
