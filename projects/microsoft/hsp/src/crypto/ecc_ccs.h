// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef ECC_CCS_H_
#define ECC_CCS_H_

#include "crypto/ecc.h"
#include "drivers/ccs_ksu_interface.h"


/**
 * A context that uses the HSP Complex Command Sequencer (CCS) for ECC private key operations.  A
 * secondary ECC engine is used for public key operations.
 */
struct ecc_engine_ccs {
	struct ecc_engine base;					/**< The base ECC engine. */
	const struct ccs_ksu_interface *ccs;	/**< Interface to the CCS driver. */
	const struct ecc_engine *pub;			/**< Interface to the public key handler. */
};


int ecc_ccs_init (struct ecc_engine_ccs *engine, const struct ccs_ksu_interface *ccs,
	const struct ecc_engine *ecc_pub);
void ecc_ccs_release (const struct ecc_engine_ccs *engine);


#endif	/* ECC_CCS_H_ */
