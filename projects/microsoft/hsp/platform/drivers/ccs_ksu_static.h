// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CCS_KSU_STATIC_H_
#define CCS_KSU_STATIC_H_

#include "hsp_top.h"
#include "drivers/ccs_ksu.h"
#include "trap/hsp_interrupt_handler_static.h"


/* Internal functions declared to allow for static initialization. */
int ccs_ksu_derive_key (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	const SP_MSG_384 *context, uint8_t key_slot, uint32_t key_attributes);
int ccs_ksu_derive_key_using_pcr (const struct ccs_ksu_interface *ccs, uint8_t key_in, uint8_t pcr,
	uint8_t key_slot, uint32_t key_attributes);
int ccs_ksu_generate_random_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t key_attributes);
int ccs_ksu_derive_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_in, uint8_t key_slot,
	uint32_t key_attributes);
int ccs_ksu_derive_fw_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_in, SP_MSG_384 *key,
	uint32_t key_attributes);
int ccs_ksu_certify_ecc_public_key (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	uint8_t pcr, uint8_t key_slot, const SP_MSG_384 *sign_data, SP_ECDSA_P384_PUBLIC *public_key,
	SP_ECDSA_P384_SIGNATURE *signature, uint32_t *public_key_attributes,
	uint32_t *signing_key_attributes);
int ccs_ksu_wrap_key_buffer (const struct ccs_ksu_interface *ccs, uint8_t kek_slot,
	const SP_MSG_384 *key, SP_MSG_512 *wrapped_key, uint32_t key_attributes);
int ccs_ksu_unwrap_key (const struct ccs_ksu_interface *ccs, uint8_t kek_slot,
	const SP_MSG_512 *wrapped_key, uint8_t key_slot);
int ccs_ksu_reset_pcr (const struct ccs_ksu_interface *ccs, uint8_t pcr);
int ccs_ksu_extend_pcr (const struct ccs_ksu_interface *ccs, uint8_t pcr, const SP_MSG_384 *digest);
int ccs_ksu_get_pcr_value (const struct ccs_ksu_interface *ccs, uint8_t pcr, SP_MSG_384 *value);
int ccs_ksu_hmac (const struct ccs_ksu_interface *ccs, uint8_t key_slot, const uint8_t *data,
	size_t length, SP_MSG_384 *hmac, uint32_t *key_attributes);


/**
 * Statically determine the number of supported key slots.  The capabilities register is not useful
 * in this case.
 */
#define	CCS_KSU_STATIC_NUM_KEYS		(sizeof (struct Ksu_ksb_keys) / sizeof (struct ksu_key_slot))

/**
 * Statically determine the number of supported PCRs.  The capabilities register is not useful in
 * this case.
 */
#define	CCS_KSU_STATIC_NUM_PCRS		(sizeof (struct Ksu_ksb_pcrs) / sizeof (struct ksu_pcr_slot))

/**
 * Constant initializer for CCS/KSU Send Key API function.
 */
#ifdef CCS_KSU_ENABLE_SEND_KEY
#define	CCS_KSU_SEND_KEY_ENTRY \
	.send_key = ccs_ksu_send_key,
#else
#define	CCS_KSU_SEND_KEY_ENTRY
#endif

/**
 * Static initialization of the CCS/KSU driver API for SHACK2.
 */
#define	CCS_KSU_INIT_API	{ \
		.is_key_slot_valid = ccs_ksu_is_key_slot_valid, \
		.get_key_attributes = ccs_ksu_get_key_attributes, \
		.set_key = ccs_ksu_set_key, \
		CCS_KSU_SEND_KEY_ENTRY \
		.generate_random_key = ccs_ksu_generate_random_key, \
		.derive_key = ccs_ksu_derive_key, \
		.derive_key_using_pcr = ccs_ksu_derive_key_using_pcr, \
		.generate_random_ecc_key = ccs_ksu_generate_random_ecc_key, \
		.derive_ecc_key = ccs_ksu_derive_ecc_key, \
		.derive_fw_ecc_key = ccs_ksu_derive_fw_ecc_key, \
		.export_fw_ecc_key = ccs_ksu_export_fw_ecc_key, \
		.get_ecc_public_key = ccs_ksu_get_ecc_public_key, \
		.certify_ecc_public_key = ccs_ksu_certify_ecc_public_key, \
		.ecdh_key_exchange = ccs_ksu_ecdh_key_exchange, \
		.ecc_sign = ccs_ksu_ecc_sign, \
		.ecdsa_sign_message = ccs_ksu_ecdsa_sign_message, \
		.ecdsa_sign_hash = ccs_ksu_ecdsa_sign_hash, \
		.ecdsa_sign_hash_and_finish = ccs_ksu_ecdsa_sign_hash_and_finish, \
		.wrap_key_buffer = ccs_ksu_wrap_key_buffer, \
		.unwrap_key = ccs_ksu_unwrap_key, \
		.burn_key = ccs_ksu_burn_key, \
		.reset_pcr = ccs_ksu_reset_pcr, \
		.extend_pcr = ccs_ksu_extend_pcr, \
		.get_pcr_value = ccs_ksu_get_pcr_value, \
		.hmac = ccs_ksu_hmac \
	}


/**
 * Initialize a static CCS/KSU driver instance using SHACK2.  CCS operations will enter a busy
 * waiting loop, actively polling the hardware to determine when they have finished.
 *
 * The interrupt handler will be null for instances initialized in this way.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the CCS driver instance.
 * @param regs_ptr Base address for the CCS registers.
 * @param sha_ptr Interface to the HS-SHA used by this CCS instance.
 * @param aes_ptr Interface to the AES used by this CCS instance.
 * @param pka_ptr Interface to the PKA used by this CCS instance.
 * @param rng_ptr Interface to the RNG used by this CCS instance.
 * @param cmd_buffer Location in HSP shared RAM where CCS commands should be constructed.  This must
 * be a 32-bit aligned address.
 * @param keys_ptr Address for key storage in the KSU.
 * @param num_keys The number of key slots available in the KSU.
 * @param pcrs_ptr Address for PCR storage in the KSU.
 * @param num_pcrs The number of PCRs available in the KSU.
 */
#define	ccs_ksu_static_init_polling(state_ptr, regs_ptr, sha_ptr, aes_ptr, pka_ptr, rng_ptr, \
	cmd_buffer, keys_ptr, num_keys, pcrs_ptr, num_pcrs)	{ \
		.base = CCS_KSU_INIT_API, \
		.base_irq = hsp_interrupt_handler_static_init (NULL), \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.irq = NULL, \
		.sha = sha_ptr, \
		.aes = aes_ptr, \
		.pka = pka_ptr, \
		.rng = rng_ptr, \
		.buffer = cmd_buffer, \
		.keys = keys_ptr, \
		.key_slots = num_keys, \
		.pcrs = pcrs_ptr, \
		.pcr_slots = num_pcrs, \
		.submit_command = ccs_ksu_submit_command_polling, \
	}

/**
 * Initialize a static CCS/KSU driver instance using SHACK2.  CCS operations will block, waiting for
 * an interrupt to indicate when the hardware has finished.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the CCS driver instance.
 * @param regs_ptr Base address for the CCS registers.
 * @param irq_regs_ptr Base address for the CREG registers to control CCS interrupts.
 * @param sha_ptr Interface to the HS-SHA used by this CCS instance.
 * @param aes_ptr Interface to the AES used by this CCS instance.
 * @param pka_ptr Interface to the PKA used by this CCS instance.
 * @param rng_ptr Interface to the RNG used by this CCS instance.
 * @param cmd_buffer Location in HSP shared RAM where CCS commands should be constructed.  This must
 * be a 32-bit aligned address.
 * @param keys_ptr Address for key storage in the KSU.
 * @param num_keys The number of key slots available in the KSU.
 * @param pcrs_ptr Address for PCR storage in the KSU.
 * @param num_pcrs The number of PCRs available in the KSU.
 */
#define	ccs_ksu_static_init_interrupt(state_ptr, regs_ptr, irq_regs_ptr, sha_ptr, aes_ptr, \
	pka_ptr, rng_ptr, cmd_buffer, keys_ptr, num_keys, pcrs_ptr, num_pcrs)	{ \
		.base = CCS_KSU_INIT_API, \
		.base_irq = hsp_interrupt_handler_static_init (ccs_ksu_handle_interrupt), \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.irq = irq_regs_ptr, \
		.sha = sha_ptr, \
		.aes = aes_ptr, \
		.pka = pka_ptr, \
		.rng = rng_ptr, \
		.buffer = cmd_buffer, \
		.keys = keys_ptr, \
		.key_slots = num_keys, \
		.pcrs = pcrs_ptr, \
		.pcr_slots = num_pcrs, \
		.submit_command = ccs_ksu_submit_command_interrupt, \
	}


#endif	/* CCS_KSU_STATIC_H_ */
