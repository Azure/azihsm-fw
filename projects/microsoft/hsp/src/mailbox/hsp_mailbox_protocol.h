// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_MAILBOX_PROTOCOL_H_
#define HSP_MAILBOX_PROTOCOL_H_

#include "hsp_mailbox_interface.h"
#include "hsp_mailbox_protocol_interface.h"

/**
 * Instance for executing the mailbox protocol.
 */
struct hsp_mailbox_protocol {
	struct hsp_mailbox_protocol_interface base;		/**< Base protocol instance. */
	const struct hsp_mailbox_interface *mailbox;	/**< Mailbox instance on which to apply the protocol. */
};


int hsp_mailbox_protocol_init (struct hsp_mailbox_protocol *protocol,
	const struct hsp_mailbox_interface *mailbox);
void hsp_mailbox_protocol_release (const struct hsp_mailbox_protocol *protocol);


#endif	/* HSP_MAILBOX_PROTOCOL_H_ */
