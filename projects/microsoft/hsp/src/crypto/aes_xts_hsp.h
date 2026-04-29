// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef AES_XTS_HSP_H_
#define AES_XTS_HSP_H_

#include <stdint.h>
#include "crypto/aes_xts.h"
#include "drivers/ccs_ksu_interface.h"
#include "drivers/hsp_aes.h"
#include "splibs/inc/spcryptotypes.h"


/**
 * An HSP hardware driver implementation for AES-XTS operations.
 *
 * This implementation can only support AES-128.
 */
struct aes_xts_engine_hsp {
	struct aes_xts_engine base;				/**< The base AES-XTS engine. */
	const struct hsp_aes *aes;				/**< Driver for the HSP AES hardware. */
	const struct ccs_ksu_interface *ccs;	/**< Driver for key management. */
	uint8_t slot_id;						/**< KSU slot to use for the AES key. */
};


int aes_xts_hsp_init (struct aes_xts_engine_hsp *engine, const struct hsp_aes *aes,
	const struct ccs_ksu_interface *ccs, uint8_t slot_id);
void aes_xts_hsp_release (const struct aes_xts_engine_hsp *engine);

int aes_xts_hsp_derive_xts_key (const struct aes_xts_engine_hsp *engine, uint8_t src_key,
	const SP_MSG_384 *context);


#endif	/* AES_XTS_HSP_H_ */
