// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "common/unused.h"
#include "drivers/jtag_mailbox.h"
#include "rom/jtag_handler.h"


void jtag_mailbox_read (const struct jtag_mailbox *mailbox, union jtag_handler_msg *msg)
{
	if ((mailbox != NULL) && (msg != NULL)) {
		msg->raw[0] = mailbox->regs->J2P_MBOX0;
		msg->raw[1] = mailbox->regs->J2P_MBOX1;
	}
}

void jtag_mailbox_write (const struct jtag_mailbox *mailbox, const union jtag_handler_msg *msg)
{
	if ((mailbox != NULL) && (msg != NULL)) {
		mailbox->regs->J2P_MBOX0 = msg->raw[0];
		mailbox->regs->J2P_MBOX1 = msg->raw[1];
	}
}

/**
 * Initialize a driver for the HSP JTAG mailbox.
 *
 * @param mailbox The JTAG mailbox driver instance to initialize.
 * @param regs Base address for the CREG misc registers containing the mailbox.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int jtag_mailbox_init (struct jtag_mailbox *mailbox, struct Creg_regs_misc_regs *regs)
{
	if ((mailbox == NULL) || (regs == NULL)) {
		return JTAG_MAILBOX_INVALID_ARGUMENT;
	}

	memset (mailbox, 0, sizeof (struct jtag_mailbox));

	mailbox->read = jtag_mailbox_read;
	mailbox->write = jtag_mailbox_write;

	mailbox->regs = regs;

	return 0;
}

/**
 * Release the resources used by a JTAG mailbox driver.
 *
 * @param mailbox The JTAG driver instance to release.
 */
void jtag_mailbox_release (const struct jtag_mailbox *mailbox)
{
	UNUSED (mailbox);
}
