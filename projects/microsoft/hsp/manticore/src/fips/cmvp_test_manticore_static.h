// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMVP_TEST_MANTICORE_STATIC_H_
#define CMVP_TEST_MANTICORE_STATIC_H_

#include "cmvp_test_manticore.h"


/* Internal functions declared to allow for static initialization. */
int cmvp_test_manticore_trigger_test_case (const struct cmvp_test_interface *cmvp,
	uint32_t test_id);


/**
 * Constant initializer for the CMVP test base API.
 */
#define	CMVP_TEST_MANTICORE_API_INIT  { \
	.trigger_test_case = cmvp_test_manticore_trigger_test_case, \
}


/**
 * Initialize a static handler for CMVP tests on Manticore.
 *
 * There is no validation done on the arguments.
 *
 * @param dmb_ptr DMB to use for accessing SoC addresses.
 * @param hsp_test_id_ptr Buffer to use for storing CMVP test identifiers.
 * @param soc_test_id_arg SoC address to use for storing CMVP test identifiers.
 */
#define	cmvp_test_manticore_static_init(dmb_ptr, hsp_test_id_ptr, soc_test_id_arg)	{ \
		.base = CMVP_TEST_MANTICORE_API_INIT, \
		.dmb = dmb_ptr, \
		.test_id = hsp_test_id_ptr,\
		.soc_test_id_addr = soc_test_id_arg, \
	}


#endif	/* CMVP_TEST_MANTICORE_STATIC_H_ */
