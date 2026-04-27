// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef JTAG_HANDLER_STATIC_H_
#define JTAG_HANDLER_STATIC_H_

#include "rom/jtag_handler.h"

/**
 * Initialize a static handler for JTAG mailbox commands.
 *
 * There is no validation done on the arguments.
 *
 * @param handle_msg_ptr Pointer to the function that will handle JTAG messages.
 * @param mailbox_ptr Interface to the mailbox containing the messages.
 * @param fuses_ptr Interface to the HSP fuses.
 * @param ccs_ptr Interface to the HSP CCS and KSU for secure key management.
 * @param rng_ptr Random number generator to use during message handling, such as for generating the
 * SOCID.
 * @param public_keys List of public keys that can be retrieved over JTAG.  Each array entry maps to
 * an index in the request.  Unsupported indicies must have the key buffer set to null.
 */
#define	jtag_handler_static_init(handle_msg_ptr, mailbox_ptr, fuses_ptr, ccs_ptr, rng_ptr, \
	public_keys) { \
		.handle_msg = handle_msg_ptr, \
		.mailbox = mailbox_ptr, \
		.fuses = fuses_ptr, \
		.ccs = ccs_ptr, \
		.rng = rng_ptr, \
		.keys = public_keys, \
	}


#endif	/* JTAG_HANDLER_STATIC_H_ */
