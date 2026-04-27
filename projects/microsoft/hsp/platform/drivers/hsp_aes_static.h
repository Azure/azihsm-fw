// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_AES_STATIC_H_
#define HSP_AES_STATIC_H_

#include "drivers/hsp_aes.h"
#include "trap/hsp_interrupt_handler_static.h"


/* Internal functions declared to allow for static initialization. */
int hsp_aes_encrypt (const struct hsp_aes *aes, enum hsp_aes_mode mode, uint8_t key,
	const SP_MSG_128 *iv, const uint8_t *plaintext, size_t length, uint8_t *ciphertext,
	size_t out_length, SP_MSG_128 *next_iv);
int hsp_aes_decrypt (const struct hsp_aes *aes, enum hsp_aes_mode mode, uint8_t key,
	const SP_MSG_128 *iv, const uint8_t *ciphertext, size_t length, uint8_t *plaintext,
	size_t out_length, SP_MSG_128 *next_iv);

bool hsp_aes_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param);

int hsp_aes_execute_command_polling (const struct hsp_aes *aes, int error_code);
int hsp_aes_execute_command_interrupt (const struct hsp_aes *aes, int error_code);


/**
 * Initialize a static AES driver instance.  AES operations will enter a busy waiting loop, actively
 * polling the hardware to determine when they have finished.
 *
 * The interrupt handler will be null for instances initialized in this way.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the AES driver.
 * @param regs_ptr Base address for the AES registers.
 * @param cmd_buffer Location in HSP shared RAM where AES commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param msg_buf Location in HSP shared RAM that can be used to stage data for AES operations, when
 * necessary.
 * @param buffer_length Size of the shared message buffer.  This needs to be at least large enough
 * to hold one block of data for AES (16 bytes).
 * @param keys_ptr Address for key storage in the KSU.
 * @param num_keys The number of key slots available in the KSU.
 */
#define	hsp_aes_static_init_polling(state_ptr, regs_ptr, cmd_buffer, msg_buf, buffer_length, \
	keys_ptr, num_keys)	{ \
		.base = hsp_interrupt_handler_static_init (NULL), \
		.encrypt = hsp_aes_encrypt, \
		.decrypt = hsp_aes_decrypt, \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.irq = NULL, \
		.buffer = cmd_buffer, \
		.msg_buffer = msg_buf, \
		.max_msg = HSP_AES_BLOCK_ALIGN (buffer_length), \
		.keys = keys_ptr, \
		.key_slots = num_keys, \
		.execute_command = hsp_aes_execute_command_polling, \
	}

/**
 * Initialize a static AES driver instance.  AES operations will block, waiting for an interrupt to
 * indicate when the hardware has finished.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the AES driver.
 * @param regs_ptr Base address for the AES registers.
 * @param irq_regs_ptr Base address for the CREG registers to control AES interrupts.
 * @param cmd_buffer Location in HSP shared RAM where AES commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param msg_buf Location in HSP shared RAM that can be used to stage data for AES operations, when
 * necessary.
 * @param buffer_length Size of the shared message buffer.  This needs to be at least large enough
 * to hold one block of data for AES (16 bytes).
 * @param keys_ptr Address for key storage in the KSU.
 * @param num_keys The number of key slots available in the KSU.
 */
#define	hsp_aes_static_init_interrupt(state_ptr, regs_ptr, irq_regs_ptr, cmd_buffer, msg_buf, \
	buffer_length, keys_ptr, num_keys)	{ \
		.base = hsp_interrupt_handler_static_init (hsp_aes_handle_interrupt), \
		.encrypt = hsp_aes_encrypt, \
		.decrypt = hsp_aes_decrypt, \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.irq = irq_regs_ptr, \
		.buffer = cmd_buffer, \
		.msg_buffer = msg_buf, \
		.max_msg = HSP_AES_BLOCK_ALIGN (buffer_length), \
		.keys = keys_ptr, \
		.key_slots = num_keys, \
		.execute_command = hsp_aes_execute_command_interrupt, \
	}


#endif	/* HSP_AES_STATIC_H_ */
