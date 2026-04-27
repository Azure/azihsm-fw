// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MEMORY_PROTECTION_MPU_ONLY_STATIC_H_
#define MEMORY_PROTECTION_MPU_ONLY_STATIC_H_

#include "memory_protection_mpu_only.h"


/**
 * Constant initializer for the memory protection API.
 */
#define	MEMORY_PROTECTION_MPU_ONLY_API_INIT  { \
		.configure_hsp_mpu = memory_protection_mpu_only_configure_hsp_mpu, \
		.configure_soc_fences = memory_protection_mpu_only_configure_soc_fences, \
	}

/**
 * Initialize a static HSP memory protection handling using the MPU.
 *
 * This is an internal initializer for use with derived types to override the virtual API.
 *
 * @param api The API function pointers to use for the instance.
 * @param mpu_ptr The MPU driver for HSP.
 * @param regions_ptr A list of memory regions that should be configured in the MPU.
 * @param count_arg The number of memory regions in the list.
 */
#define	memory_protection_mpu_only_static_init_with_api(api, mpu_ptr, regions_ptr, count_arg)	{ \
		.base = api, \
		.mpu = mpu_ptr, \
		.regions = regions_ptr, \
		.region_count = count_arg, \
	}

/**
 * Initialize a static instance of a handler for configuring HSP memory protections using the MPU.
 * This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param mpu_ptr The MPU driver for HSP.
 * @param regions_ptr A list of memory regions that should be configured in the MPU.
 * @param count_arg The number of memory regions in the list.
 */
#define	memory_protection_mpu_only_static_init(mpu_ptr, regions_ptr, count_arg) \
	memory_protection_mpu_only_static_init_with_api (MEMORY_PROTECTION_MPU_ONLY_API_INIT, mpu_ptr, \
		regions_ptr, count_arg)


#endif	/* MEMORY_PROTECTION_MPU_ONLY_STATIC_H_ */
