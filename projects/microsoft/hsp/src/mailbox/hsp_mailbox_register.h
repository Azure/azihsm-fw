// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_MAILBOX_REGISTER_H_
#define HSP_MAILBOX_REGISTER_H_

#include "common/unused.h"
#include "mailbox/hsp_mailbox_interface.h"
#include "mailbox/hsp_mailbox_register_bank.h"

/**
 * A register implementation of the HSP mailbox API.
 */
struct hsp_mailbox_register {
	struct hsp_mailbox_interface base;			/**< The base hsp_mailbox instance. */
	struct hsp_mailbox_register_bank *send_reg;	/**< The address of the outgoing mailbox registers. */
	struct hsp_mailbox_register_bank *recv_reg;	/**< The address of the incoming mailbox registers. */
};


int hsp_mailbox_register_init (struct hsp_mailbox_register *mailbox,
	struct hsp_mailbox_register_bank *send_reg, struct hsp_mailbox_register_bank *recv_reg);
void hsp_mailbox_register_release (const struct hsp_mailbox_register *mailbox);


#endif	/* HSP_MAILBOX_REGISTER_H_ */
