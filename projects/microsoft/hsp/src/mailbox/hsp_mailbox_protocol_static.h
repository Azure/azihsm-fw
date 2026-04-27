// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_MAILBOX_PROTOCOL_STATIC_H_
#define HSP_MAILBOX_PROTOCOL_STATIC_H_

#include "mailbox/hsp_mailbox_protocol.h"

/* Internal functions declared to allow for static initialization. */
int hsp_mailbox_protocol_send (const struct hsp_mailbox_protocol_interface *protocol,
	const struct hsp_mailbox_msg *message);
int hsp_mailbox_protocol_request_flush (const struct hsp_mailbox_protocol_interface *protocol);
int hsp_mailbox_protocol_receive (const struct hsp_mailbox_protocol_interface *protocol,
	struct hsp_mailbox_msg *message);
int hsp_mailbox_protocol_flush (const struct hsp_mailbox_protocol_interface *protocol);

/**
 * Constant initializer for the HSP mailbox protocol API.
 */
#define HSP_MAILBOX_PROTOCOL_API_INIT \
	{ \
		.send = hsp_mailbox_protocol_send, \
		.request_flush = hsp_mailbox_protocol_request_flush, \
		.receive = hsp_mailbox_protocol_receive, \
		.flush = hsp_mailbox_protocol_flush, \
	}

/**
 * Initialize a static instance of a HSP mailbox protocol interface.
 *
 * There is no validation done on the arguments.
 *
 * @param mailbox_ptr The mailbox providing access to hardware.
 */
#define hsp_mailbox_protocol_static_init(mailbox_ptr) \
	{ \
		.base = HSP_MAILBOX_PROTOCOL_API_INIT, \
		.mailbox = mailbox_ptr, \
	}


#endif	/* HSP_MAILBOX_PROTOCOL_STATIC_H_ */
