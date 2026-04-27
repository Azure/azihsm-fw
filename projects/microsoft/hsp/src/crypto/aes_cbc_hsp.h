// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef AES_CBC_HSP_H_
#define AES_CBC_HSP_H_

#include <stdint.h>
#include "crypto/aes_cbc.h"
#include "drivers/ccs_ksu_interface.h"
#include "drivers/hsp_aes.h"
#include "splibs/inc/spcryptotypes.h"


/**
 * An HSP hardware driver implementation for AES-CBC operations.
 */
struct aes_cbc_engine_hsp {
	struct aes_cbc_engine base;				/**< The base AES-CBC engine. */
	const struct hsp_aes *aes;				/**< Driver for the HSP AES hardware. */
	const struct ccs_ksu_interface *ccs;	/**< Driver for key management. */
	uint8_t slot_id;						/**< KSU slot to use for the AES key. */
};


int aes_cbc_hsp_init (struct aes_cbc_engine_hsp *engine, const struct hsp_aes *aes,
	const struct ccs_ksu_interface *ccs, uint8_t slot_id);
void aes_cbc_hsp_release (const struct aes_cbc_engine_hsp *engine);


#endif	/* AES_CBC_HSP_H_ */
