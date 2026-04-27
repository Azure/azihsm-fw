// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef JTAG_HANDLER_NO_EMC_H_
#define JTAG_HANDLER_NO_EMC_H_

#include "jtag_handler.h"


/**
 * ROM handler for messages present during boot in the HSP JTAG mailbox.
 *
 * This structure extends the base JTAG handler to include functionality
 * specific to systems that do not support EMC but support Canary programming.
 * It includes all the common handler functionality provided by the base
 * structure and adds Canary-specific capabilities.
 */
struct jtag_handler_no_emc {
	struct jtag_handler base;		/**< Base JTAG handler instance. */
	const struct hw_rot *rot;		/**< Interface to the RoT state for canary program. */
	const uint8_t *socid_prefix;	/**< Value to use as the prefix of the SOCID. */
	size_t socid_prefix_len;		/**< Length of prefix SOCID. */
};


int jtag_handler_no_emc_init (struct jtag_handler_no_emc *jtag,	const struct jtag_mailbox *mailbox,
	const struct fuse_controller_interface *fuses, const struct ccs_ksu_interface *ccs,
	const struct rng_engine *rng, const struct jtag_handler_public_key public_keys[4],
	const uint8_t *socid_prefix, size_t socid_prefix_len, const struct hw_rot *rot);
void jtag_handler_no_emc_release (const struct jtag_handler_no_emc *jtag);

/* Internal functions for use by derived types. */
int jtag_handler_no_emc_program_canary (struct jtag_handler_no_emc *jtag_no_emc,
	union jtag_handler_msg *msg, struct jtag_handler_msg_provision *prov);


#endif	/* JTAG_HANDLER_NO_EMC_H_ */
