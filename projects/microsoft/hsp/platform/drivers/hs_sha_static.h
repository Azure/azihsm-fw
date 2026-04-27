// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HS_SHA_STATIC_H_
#define HS_SHA_STATIC_H_

#include "drivers/hs_sha.h"
#include "trap/hsp_interrupt_handler_static.h"


/* Internal functions declared to allow for static initialization. */
bool hs_sha_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param);

int hs_sha_execute_command_polling (const struct hs_sha *sha, int error_code);
int hs_sha_execute_command_interrupt (const struct hs_sha *sha, int error_code);


/**
 * Initialize a static HS-SHA driver instance.  Hash operations will enter a busy waiting loop,
 * actively polling the hardware to determine when they have finished.
 *
 * The interrupt handler will be null for instances initialized in this way.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the HS-SHA driver instance.
 * @param regs_ptr Base address for the HS-SHA registers.
 * @param cmd_buffer Location in HSP shared RAM where HS-SHA commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param msg_buf Location in HSP shared RAM that can be used to stage data for HS-SHA, when
 * necessary.
 * @param buffer_length Size of the shared message buffer.  This needs to be at least large enough
 * to hold one block of data for SHA-512 (128 bytes).
 */
#define	hs_sha_static_init_polling(state_ptr, regs_ptr, cmd_buffer, msg_buf, buffer_length)	{ \
		.base = hsp_interrupt_handler_static_init (NULL), \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.irq = NULL, \
		.buffer = cmd_buffer, \
		.msg_buffer = msg_buf, \
		.max_msg = buffer_length, \
		.execute_command = hs_sha_execute_command_polling, \
	}

/**
 * Initialize a static HS-SHA driver instance.  Hash operations will block, waiting for an interrupt
 * to indicate when the hardware has finished.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the HS-SHA driver instance.
 * @param regs_ptr Base address for the HS-SHA registers.
 * @param irq_regs_ptr Base address for the CREG registers to control HS-SHA interrupts.
 * @param cmd_buffer Location in HSP shared RAM where HS-SHA commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param msg_buf Location in HSP shared RAM that can be used to stage data for HS-SHA, when
 * necessary.
 * @param buffer_length Size of the shared message buffer.  This needs to be at least large enough
 * to hold one block of data for SHA-512 (128 bytes).
 */
#define	hs_sha_static_init_interrupt(state_ptr, regs_ptr, irq_regs_ptr, cmd_buffer, msg_buf, \
	buffer_length)	{ \
		.base = hsp_interrupt_handler_static_init (hs_sha_handle_interrupt), \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.irq = irq_regs_ptr, \
		.buffer = cmd_buffer, \
		.msg_buffer = msg_buf, \
		.max_msg = buffer_length, \
		.execute_command = hs_sha_execute_command_interrupt, \
	}


#endif	/* HS_SHA_STATIC_H_ */
