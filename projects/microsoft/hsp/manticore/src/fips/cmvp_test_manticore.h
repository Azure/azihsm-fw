// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMVP_TEST_MANTICORE_H_
#define CMVP_TEST_MANTICORE_H_

#include "drivers/hsp_dmb.h"
#include "fips/cmvp_test_interface.h"


/**
 * CMVP certification test handler for Manticore.
 */
struct cmvp_test_manticore {
	struct cmvp_test_interface base;	/**< Base API for CMVP testing. */
	const struct hsp_dmb *dmb;			/**< DMB for accessing SoC memory. */
	uint32_t *test_id;					/**< Location in HSP memory for storing the CMVP test ID. */
	uint64_t soc_test_id_addr;			/**< SoC address where the CMVP test ID will be stored. */
};


int cmvp_test_manticore_init (struct cmvp_test_manticore *cmvp, const struct hsp_dmb *dmb,
	uint32_t *hsp_test_id, uint64_t soc_test_id);
void cmvp_test_manticore_release (const struct cmvp_test_manticore *cmvp);


#endif	/* CMVP_TEST_MANTICORE_H_ */
