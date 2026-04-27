// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef JTAG_MAILBOX_STATIC_H_
#define JTAG_MAILBOX_STATIC_H_

#include "drivers/jtag_mailbox.h"


/* Internal functions declared to allow for static initialization. */
void jtag_mailbox_read (const struct jtag_mailbox *mailbox, union jtag_handler_msg *msg);
void jtag_mailbox_write (const struct jtag_mailbox *mailbox, const union jtag_handler_msg *msg);


/**
 * Initialize a static JTAG mailbox driver instance.
 *
 * There is no validation done on the arguments.
 *
 * @param regs_ptr Base address for the CREG misc registers containing the mailbox.
 */
#define	jtag_mailbox_static_init(regs_ptr)	{ \
		.read = jtag_mailbox_read, \
		.write = jtag_mailbox_write, \
		.regs = regs_ptr, \
	}


#endif	/* JTAG_MAILBOX_STATIC_H_ */
