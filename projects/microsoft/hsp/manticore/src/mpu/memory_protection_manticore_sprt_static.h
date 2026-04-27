// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MEMORY_PROTECTION_MANTICORE_SPRT_STATIC_H_
#define MEMORY_PROTECTION_MANTICORE_SPRT_STATIC_H_

#include "memory_protection_manticore_sprt.h"
#include "mpu/memory_protection_mpu_only_static.h"


/* Internal functions declared to allow for static initialization. */
int memory_protection_manticore_sprt_configure_soc_fences (
	const struct memory_protection *mem_protect);


/**
 * Constant initializer for the memory protection API.
 */
#define	MEMORY_PROTECTION_MANTICORE_SPRT_API_INIT  { \
		.configure_hsp_mpu = memory_protection_mpu_only_configure_hsp_mpu, \
		.configure_soc_fences = memory_protection_manticore_sprt_configure_soc_fences, \
	}


/**
 * Initialize a static instance of a handler for SPRT memory protection configuration.  This can be
 * a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param fence_ptr The driver for SoC memory fencing.
 * @param mpu_ptr The MPU driver for HSP.
 * @param regions_ptr A list of memory regions that should be configured in the MPU.
 * @param count_arg The number of memory regions in the list.
 */
#define	memory_protection_manticore_sprt_static_init(fence_ptr, mpu_ptr, regions_ptr, count_arg) { \
		.base = memory_protection_mpu_only_static_init_with_api ( \
			MEMORY_PROTECTION_MANTICORE_SPRT_API_INIT, mpu_ptr, regions_ptr, count_arg), \
		.fence = fence_ptr, \
	}


#endif	/* MEMORY_PROTECTION_MANTICORE_SPRT_STATIC_H_ */
