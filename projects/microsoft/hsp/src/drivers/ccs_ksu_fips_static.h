// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CCS_KSU_FIPS_STATIC_H_
#define CCS_KSU_FIPS_STATIC_H_

#include "ccs_ksu_fips.h"


/* Internal functions declared to allow for static initialization. */
int ccs_ksu_fips_is_key_slot_valid (const struct ccs_ksu_interface *ccs, uint8_t key_slot);
int ccs_ksu_fips_get_key_attributes (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t *key_attributes);
int ccs_ksu_fips_set_key (const struct ccs_ksu_interface *ccs, const SP_MSG_384 *key,
	uint8_t key_slot, uint32_t key_attributes);
#ifdef CCS_KSU_ENABLE_SEND_KEY
int ccs_ksu_fips_send_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t dest_addr);
#endif
int ccs_ksu_fips_generate_random_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t key_attributes);
int ccs_ksu_fips_derive_key (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	const SP_MSG_384 *context, uint8_t key_slot, uint32_t key_attributes);
int ccs_ksu_fips_derive_key_using_pcr (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	uint8_t pcr, uint8_t key_slot, uint32_t key_attributes);
int ccs_ksu_fips_generate_random_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t key_attributes);
int ccs_ksu_fips_derive_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	uint8_t key_slot, uint32_t key_attributes);
int ccs_ksu_fips_derive_fw_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	SP_MSG_384 *key, uint32_t key_attributes);
int ccs_ksu_fips_export_fw_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	SP_MSG_384 *key, uint32_t *key_attributes);
int ccs_ksu_fips_get_ecc_public_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	SP_ECDSA_P384_PUBLIC *public_key, uint32_t *key_attributes);
int ccs_ksu_fips_certify_ecc_public_key (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	uint8_t pcr, uint8_t key_slot, const SP_MSG_384 *sign_data, SP_ECDSA_P384_PUBLIC *public_key,
	SP_ECDSA_P384_SIGNATURE *signature, uint32_t *public_key_attributes,
	uint32_t *signing_key_attributes);
int ccs_ksu_fips_ecdh_key_exchange (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	uint8_t key_out, const uint8_t *partner_public_key_and_hash, size_t input_len,
	uint32_t key_attributes);
int ccs_ksu_fips_ecc_sign (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	const SP_MSG_384 *digest, const struct rng_engine *rng, SP_ECDSA_P384_SIGNATURE *signature,
	uint32_t *key_attributes);
int ccs_ksu_fips_ecdsa_sign_message (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	const uint8_t *message, size_t length, const struct hash_engine *hash, enum hash_type hash_algo,
	SP_ECDSA_P384_SIGNATURE *signature, uint32_t *key_attributes);
int ccs_ksu_fips_ecdsa_sign_hash (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	const struct hash_engine *hash, SP_ECDSA_P384_SIGNATURE *signature, uint32_t *key_attributes);
int ccs_ksu_fips_ecdsa_sign_hash_and_finish (const struct ccs_ksu_interface *ccs,
	uint8_t signing_key, const struct hash_engine *hash, SP_ECDSA_P384_SIGNATURE *signature,
	uint32_t *key_attributes);
int ccs_ksu_fips_wrap_key_buffer (const struct ccs_ksu_interface *ccs, uint8_t kek_slot,
	const SP_MSG_384 *key, SP_MSG_512 *wrapped_key, uint32_t key_attributes);
int ccs_ksu_fips_unwrap_key (const struct ccs_ksu_interface *ccs, uint8_t kek_slot,
	const SP_MSG_512 *wrapped_key, uint8_t key_slot);
int ccs_ksu_fips_burn_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot);
int ccs_ksu_fips_reset_pcr (const struct ccs_ksu_interface *ccs, uint8_t pcr);
int ccs_ksu_fips_extend_pcr (const struct ccs_ksu_interface *ccs, uint8_t pcr,
	const SP_MSG_384 *digest);
int ccs_ksu_fips_get_pcr_value (const struct ccs_ksu_interface *ccs, uint8_t pcr,
	SP_MSG_384 *value);
int ccs_ksu_fips_hmac (const struct ccs_ksu_interface *ccs, uint8_t key_slot, const uint8_t *data,
	size_t length, SP_MSG_384 *hmac, uint32_t *key_attributes);

/**
 * Constant initializer for CCS/KSU Send Key API function.
 */
#ifdef CCS_KSU_ENABLE_SEND_KEY
#define	CCS_KSU_SEND_KEY_ENTRY \
	.send_key = ccs_ksu_fips_send_key,
#else
#define	CCS_KSU_SEND_KEY_ENTRY
#endif

/**
 * Constant initializer for the CCS API.
 */
#define	CCS_KSU_FIPS_API_INIT  { \
		.is_key_slot_valid = ccs_ksu_fips_is_key_slot_valid, \
		.get_key_attributes = ccs_ksu_fips_get_key_attributes, \
		.set_key = ccs_ksu_fips_set_key, \
		CCS_KSU_SEND_KEY_ENTRY \
		.generate_random_key = ccs_ksu_fips_generate_random_key, \
		.derive_key = ccs_ksu_fips_derive_key, \
		.derive_key_using_pcr = ccs_ksu_fips_derive_key_using_pcr, \
		.generate_random_ecc_key = ccs_ksu_fips_generate_random_ecc_key, \
		.derive_ecc_key = ccs_ksu_fips_derive_ecc_key, \
		.derive_fw_ecc_key = ccs_ksu_fips_derive_fw_ecc_key, \
		.export_fw_ecc_key = ccs_ksu_fips_export_fw_ecc_key, \
		.get_ecc_public_key = ccs_ksu_fips_get_ecc_public_key, \
		.certify_ecc_public_key = ccs_ksu_fips_certify_ecc_public_key, \
		.ecdh_key_exchange = ccs_ksu_fips_ecdh_key_exchange, \
		.ecc_sign = ccs_ksu_fips_ecc_sign, \
		.ecdsa_sign_message = ccs_ksu_fips_ecdsa_sign_message, \
		.ecdsa_sign_hash = ccs_ksu_fips_ecdsa_sign_hash, \
		.ecdsa_sign_hash_and_finish = ccs_ksu_fips_ecdsa_sign_hash_and_finish, \
		.wrap_key_buffer = ccs_ksu_fips_wrap_key_buffer, \
		.unwrap_key = ccs_ksu_fips_unwrap_key, \
		.burn_key = ccs_ksu_fips_burn_key, \
		.reset_pcr = ccs_ksu_fips_reset_pcr, \
		.extend_pcr = ccs_ksu_fips_extend_pcr, \
		.get_pcr_value = ccs_ksu_fips_get_pcr_value, \
		.hmac = ccs_ksu_fips_hmac, \
	}


/**
 * Initialize the firmware KSU descriptor.
 *
 * @param slots_ptr List of key slots in the firmware KSU.
 * @param count_arg Number of key slots in the firmware KSU.
 */
#define	ccs_ksu_fips_ksu_static_init(slots_ptr, count_arg) { \
		.slots = slots_ptr, \
		.slot_count = count_arg, \
	}

/**
 * Initialize a static FIPS compliant CCS instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the CCS instance.  This must be uninitialized.
 * @param ksu_ptr The firmware KSU memory available for storing private keys.
 * @param ccs_hw_ptr CCS hardware implementation that will be leveraged for non-ECC operations.
 * @param ecc_ptr The ECC hardware accelerator to use for ECC operations.
 * @param hash_ptr A hash engine to use for certifying public keys.
 */
#define	ccs_ksu_fips_static_init(state_ptr, ksu_ptr, ccs_hw_ptr, ecc_ptr, hash_ptr)	{ \
		.base = CCS_KSU_FIPS_API_INIT, \
		.state = state_ptr, \
		.ksu = ksu_ptr, \
		.ccs_hw = ccs_hw_ptr, \
		.ecc = ecc_ptr, \
		.hash = hash_ptr, \
	}


#endif	/* CCS_KSU_FIPS_STATIC_H_ */
