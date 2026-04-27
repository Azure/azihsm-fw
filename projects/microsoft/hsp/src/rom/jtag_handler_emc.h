// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef JTAG_HANDLER_EMC_H_
#define JTAG_HANDLER_EMC_H_

#include "jtag_handler.h"


/**
 * ROM handler for messages present during boot in the HSP JTAG mailbox.
 *
 * This structure extends the base JTAG handler to include functionality
 * specific to systems that support EMC.
 * It includes all the common handler functionality provided by the base
 * structure and adds EMC-specific capabilities.
 */
struct jtag_handler_emc {
	struct jtag_handler base;	/**< Base JTAG handler instance. */
	uint8_t socid_tag;			/**< Tag value to use as the first byte of the SOCID. */
};


int jtag_handler_emc_init (struct jtag_handler_emc *jtag, const struct jtag_mailbox *mailbox,
	const struct fuse_controller_interface *fuses, const struct ccs_ksu_interface *ccs,
	const struct rng_engine *rng, const struct jtag_handler_public_key public_keys[4],
	uint8_t socid_tag);
void jtag_handler_emc_release (const struct jtag_handler_emc *jtag);


#endif	/* JTAG_HANDLER_EMC_H_ */
