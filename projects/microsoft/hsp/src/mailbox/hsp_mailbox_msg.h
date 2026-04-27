// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_MAILBOX_MSG_H_
#define HSP_MAILBOX_MSG_H_

#include <stddef.h>
#include <stdint.h>


/**
 * Maximum size of the mailbox messages
 */
#define HSP_MAILBOX_MAX_SIZE 4

/**
 * Common structure used between mailbox protocol and interface handlers
 */
struct hsp_mailbox_msg {
	uint32_t msg[HSP_MAILBOX_MAX_SIZE];	/**< incoming and outgoing message buffer. */
	size_t msg_sz;						/**< size of the valid message buffer used for incoming/outgoing messages. */
};


#endif	/* HSP_MAILBOX_MSG_H_ */
