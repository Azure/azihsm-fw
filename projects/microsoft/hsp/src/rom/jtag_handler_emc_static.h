// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef JTAG_HANDLER_EMC_STATIC_H_
#define JTAG_HANDLER_EMC_STATIC_H_

#include "rom/jtag_handler_emc.h"
#include "rom/jtag_handler_static.h"


/* Internal function declared to allow for static initialization. */
int jtag_handler_emc_handle_msg (const struct jtag_handler *jtag);


/**
 * Initialize a static instance for JTAG mailbox commands specific to systems with EMC support.
 * This macro sets up a static JTAG handler instance for handling messages in systems
 * that support EMC. It initializes the base handler and then
 * overrides necessary definitions for EMC specific message handling.
 *
 * There is no validation done on the arguments.
 *
 * @param mailbox_ptr Interface to the mailbox containing the messages.
 * @param fuses_ptr Interface to the HSP fuses.
 * @param ccs_ptr Interface to the HSP CCS and KSU for secure key management.
 * @param rng_ptr Random number generator to use during message handling, such as for generating
 * the SOCID.
 * @param public_keys List of public keys that can be retrieved over JTAG. Each array entry maps to
 * an index in the request. Unsupported indices must have the key buffer set to null.
 * @param id_tag Identifier to use as the first byte of the SOCID for the device.
 */
#define	jtag_handler_emc_static_init(mailbox_ptr, fuses_ptr, ccs_ptr, rng_ptr, public_keys, \
	id_tag)	{ \
		.base = jtag_handler_static_init (jtag_handler_emc_handle_msg, mailbox_ptr, fuses_ptr, \
			ccs_ptr, rng_ptr, public_keys), \
		.socid_tag = id_tag, \
	}


#endif	/* JTAG_HANDLER_EMC_STATIC_H_ */
