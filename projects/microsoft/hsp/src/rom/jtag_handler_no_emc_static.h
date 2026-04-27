// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef JTAG_HANDLER_NO_EMC_STATIC_H_
#define JTAG_HANDLER_NO_EMC_STATIC_H_

#include "rom/jtag_handler_no_emc.h"
#include "rom/jtag_handler_static.h"


/* Internal function declared to allow for static initialization. */
int jtag_handler_no_emc_handle_msg (const struct jtag_handler *jtag);


/**
 * Initialize a static handler for JTAG mailbox commands specific to systems that
 * support Canary programming.
 *
 * This macro sets up a static JTAG handler instance for handling messages in systems
 * that support Canary programming and do not support EMC. It initializes the base handler
 * and then overrides necessary definitions for handling messages specific to systems that
 * do not support EMC.
 *
 * There is no validation done on the arguments.
 *
 * @param mailbox_ptr Interface to the mailbox containing the messages.
 * @param fuses_ptr Interface to the HSP fuses.
 * @param ccs_ptr Interface to the HSP CCS and KSU for secure key management.
 * @param rng_ptr Random number generator to use during message handling, such as for
 * generating the SOCID.
 * @param public_keys List of public keys that can be retrieved over JTAG. Each array entry maps to
 * an index in the request. Unsupported indices must have the key buffer set to null.
 * @param socid_prefix_ptr Pointer to a prefix for the SOCID.
 * @param socid_prefix_len Length of prefix SOCID.
 * @param rot Interface to the ROT
 */
#define	jtag_handler_no_emc_static_init(mailbox_ptr, fuses_ptr, ccs_ptr, rng_ptr, public_keys, \
	socid_prefix_ptr, prefix_len, rot_ptr)	{ \
		.base = jtag_handler_static_init (jtag_handler_no_emc_handle_msg, mailbox_ptr, fuses_ptr, \
			ccs_ptr, rng_ptr, public_keys), \
		.socid_prefix = socid_prefix_ptr, \
		.socid_prefix_len = prefix_len, \
		.rot = rot_ptr, \
	}


#endif	/* JTAG_HANDLER_NO_EMC_STATIC_H_ */
