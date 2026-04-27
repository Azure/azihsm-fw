// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef DEVICE_RMA_TRANSITION_HSP_RETEST_STATIC_H_
#define DEVICE_RMA_TRANSITION_HSP_RETEST_STATIC_H_

#include "device_rma_transition_hsp_retest.h"


/* Internal functions declared to allow for static initialization. */
int device_rma_transition_hsp_retest_config_rma (const struct device_rma_transition *rma);


/**
 * Constant initializer for the device RMA API.
 */
#define	DEVICE_RMA_TRANSITION_HSP_RETEST_API_INIT	{ \
		.config_rma = device_rma_transition_hsp_retest_config_rma, \
	}


/**
 * Initialize a static instance of a HSP RMA handler for moving to the RETEST security state.  This
 * can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param fuses_ptr The fuse controller that manages the security state.
 * @param ccs_ptr The CCS that manages KSU keys.
 */
#define	device_rma_transition_hsp_retest_static_init(fuses_ptr, ccs_ptr)	{ \
		.base = DEVICE_RMA_TRANSITION_HSP_RETEST_API_INIT, \
		.fuses = fuses_ptr, \
		.ccs = ccs_ptr, \
	}

/**
 * Initialize a static instance of a HSP RMA handler for moving to the RETEST security state.
 * Before executing the security state transition, SoC memory locations will be erased.  This can be
 * a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param fuses_ptr The fuse controller that manages the security state.
 * @param ccs_ptr The CCS that manages KSU keys.
 * @param dmb_ptr The DMB instance to use for mapping SoC memory addresses.
 * @param sram_ptr A list of SoC memory blocks that should be erased.
 * @param count_arg The number of memory blocks to erase.
 */
#define	device_rma_transition_hsp_retest_static_init_erase_sram(fuses_ptr, ccs_ptr, dmb_ptr, \
	sram_ptr, count_arg)	{ \
		.base = DEVICE_RMA_TRANSITION_HSP_RETEST_API_INIT, \
		.fuses = fuses_ptr, \
		.ccs = ccs_ptr, \
		.dmb = dmb_ptr, \
		.sram = sram_ptr, \
		.sram_count = count_arg, \
	}


#endif	/* DEVICE_RMA_TRANSITION_HSP_RETEST_STATIC_H_ */
