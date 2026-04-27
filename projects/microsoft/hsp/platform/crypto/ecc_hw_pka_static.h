// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef ECC_HW_PKA_STATIC_H_
#define ECC_HW_PKA_STATIC_H_

#include "hsp_top.h"
#include "crypto/ecc_hw_pka.h"
#include "trap/hsp_interrupt_handler_static.h"


/* Internal functions declared to allow for static initialization. */
int ecc_hw_pka_get_ecc_public_key (const struct ecc_hw *ecc_hw, const uint8_t *priv_key,
	size_t key_length, struct ecc_point_public_key *pub_key);
int ecc_hw_pka_verify_ecc_public_key (const struct ecc_hw *ecc_hw,
	const struct ecc_point_public_key *pub_key);
int ecc_hw_pka_generate_ecc_key_pair (const struct ecc_hw *ecc_hw, size_t key_length,
	uint8_t *priv_key, struct ecc_point_public_key *pub_key);
int ecc_hw_pka_ecdsa_sign (const struct ecc_hw *ecc_hw, const uint8_t *priv_key, size_t key_length,
	const uint8_t *digest, size_t digest_length, const struct rng_engine *rng,
	struct ecc_ecdsa_signature *signature);
int ecc_hw_pka_ecdsa_verify (const struct ecc_hw *ecc_hw,
	const struct ecc_point_public_key *pub_key, const struct ecc_ecdsa_signature *signature,
	const uint8_t *digest, size_t digest_length);
int ecc_hw_pka_ecdh_compute (const struct ecc_hw *ecc_hw, const uint8_t *priv_key,
	size_t key_length, const struct ecc_point_public_key *pub_key, uint8_t *secret, size_t length);

bool ecc_hw_pka_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param);

int ecc_hw_pka_submit_command_polling (const struct ecc_hw_pka *pka, int error_code);
int ecc_hw_pka_submit_command_interrupt (const struct ecc_hw_pka *pka, int error_code);

int ecc_hw_pka_memory_wipe (const struct ecc_hw_pka *pka);
int ecc_hw_pka_memory_wipe_unsupported (const struct ecc_hw_pka *pka);


/**
 * Constant initializer for the ECC hardware API.
 */
#define	ECC_HW_PKA_API_INIT  { \
		.get_ecc_public_key = ecc_hw_pka_get_ecc_public_key, \
		.verify_ecc_public_key = ecc_hw_pka_verify_ecc_public_key, \
		.generate_ecc_key_pair = ecc_hw_pka_generate_ecc_key_pair, \
		.ecdsa_sign = ecc_hw_pka_ecdsa_sign, \
		.ecdsa_verify = ecc_hw_pka_ecdsa_verify, \
		.ecdh_compute = ecc_hw_pka_ecdh_compute, \
	}

/**
 * Constant initializer for the PKA memory wipe command API.
 */
#ifdef HSP_ADDR_MAP_UPKA_ADDRESS
#define	ECC_HW_PKA_MEMORY_WIPE_INIT		ecc_hw_pka_memory_wipe
#else
#define	ECC_HW_PKA_MEMORY_WIPE_INIT		ecc_hw_pka_memory_wipe_unsupported
#endif


/**
 * Initialize a static PKA driver instance.  PKA operations will enter a busy waiting loop, actively
 * polling the hardware to determine when they have finished.
 *
 * The interrupt handler will be null for instances initialized in this way.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the PKA driver instance.
 * @param regs_ptr Base address for the PKA registers.
 * @param rng_ptr Base address for the RNG registers used by this PKA instance.
 * @param cmd_buffer Location in HSP shared RAM where PKA commands should be constructed.  This
 * must be a 32-bit aligned address.
 */
#define	ecc_hw_pka_static_init_polling(state_ptr, regs_ptr, rng_ptr, cmd_buffer)	{ \
		.base = ECC_HW_PKA_API_INIT, \
		.base_irq = hsp_interrupt_handler_static_init (NULL), \
		.memory_wipe = ECC_HW_PKA_MEMORY_WIPE_INIT, \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.irq = NULL, \
		.rng = rng_ptr, \
		.buffer = cmd_buffer, \
		.submit_command = ecc_hw_pka_submit_command_polling, \
	}

/**
 * Initialize a static PKA driver instance.  PKA operations will block, waiting for an interrupt to
 * indicate when the hardware has finished.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the PKA driver instance.
 * @param regs_ptr Base address for the PKA registers.
 * @param irq_regs_ptr Base address for the CREG registers to control PKA interrupts.
 * @param rng_ptr Base address for the RNG registers used by this PKA instance.
 * @param cmd_buffer Location in HSP shared RAM where PKA commands should be constructed.  This
 * must be a 32-bit aligned address.
 */
#define	ecc_hw_pka_static_init_interrupt(state_ptr, regs_ptr, irq_regs_ptr, rng_ptr, \
	cmd_buffer)	{ \
		.base = ECC_HW_PKA_API_INIT, \
		.base_irq = hsp_interrupt_handler_static_init (ecc_hw_pka_handle_interrupt), \
		.memory_wipe = ECC_HW_PKA_MEMORY_WIPE_INIT, \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.irq = irq_regs_ptr, \
		.rng = rng_ptr, \
		.buffer = cmd_buffer, \
		.submit_command = ecc_hw_pka_submit_command_interrupt, \
	}


#endif	/* ECC_HW_PKA_STATIC_H_ */
