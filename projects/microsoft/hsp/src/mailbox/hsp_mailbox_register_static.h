// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_MAILBOX_REGISTER_STATIC_H_
#define HSP_MAILBOX_REGISTER_STATIC_H_

#include "mailbox/hsp_mailbox_register.h"

/* Internal functions declared to allow for static initialization. */
int hsp_mailbox_register_send_fifo_push (const struct hsp_mailbox_interface *mailbox,
	uint32_t data);
int hsp_mailbox_register_send_set_valid (const struct hsp_mailbox_interface *mailbox);
int hsp_mailbox_register_send_set_err (const struct hsp_mailbox_interface *mailbox);
int hsp_mailbox_register_send_get_status (const struct hsp_mailbox_interface *mailbox,
	bool *is_valid, bool *is_err, uint32_t *count);
int hsp_mailbox_register_recv_fifo_pop (const struct hsp_mailbox_interface *mailbox,
	uint32_t *data);
int hsp_mailbox_register_recv_get_status (const struct hsp_mailbox_interface *mailbox,
	bool *is_valid, bool *is_err, uint32_t *count);
int hsp_mailbox_register_recv_clear_valid (const struct hsp_mailbox_interface *mailbox);
int hsp_mailbox_register_recv_clear_err (const struct hsp_mailbox_interface *mailbox);
int hsp_mailbox_register_recv_enable_fifo_valid_interrupt (
	const struct hsp_mailbox_interface *mailbox, bool enabled);
int hsp_mailbox_register_recv_enable_err_interrupt (const struct hsp_mailbox_interface *mailbox,
	bool enabled);

/**
 * Constant initializer for the HSP mailbox register API.
 */
#define HSP_MAILBOX_REGISTER_API_INIT \
	{ \
		.send_fifo_push = hsp_mailbox_register_send_fifo_push, \
		.send_set_valid = hsp_mailbox_register_send_set_valid, \
		.send_set_err = hsp_mailbox_register_send_set_err, \
		.send_get_status = hsp_mailbox_register_send_get_status, \
		.recv_fifo_pop = hsp_mailbox_register_recv_fifo_pop, \
		.recv_get_status = hsp_mailbox_register_recv_get_status, \
		.recv_clear_valid = hsp_mailbox_register_recv_clear_valid, \
		.recv_clear_err = hsp_mailbox_register_recv_clear_err, \
		.recv_enable_fifo_valid_interrupt = hsp_mailbox_register_recv_enable_fifo_valid_interrupt, \
		.recv_enable_err_interrupt = hsp_mailbox_register_recv_enable_err_interrupt, \
	}

/**
 * Initialize a static instance of a HSP mailbox register interface.
 *
 * There is no validation done on the arguments.
 *
 * @param send_reg_addr The address of the outgoing mailbox registers.
 * @param recv_reg_addr The address of the incoming mailbox registers.
 */
#define hsp_mailbox_register_static_init(send_reg_addr, recv_reg_addr) \
	{ \
		.base = HSP_MAILBOX_REGISTER_API_INIT, \
		.send_reg = send_reg_addr, \
		.recv_reg = recv_reg_addr, \
	}


#endif	/* HSP_MAILBOX_REGISTER_STATIC_H_ */
