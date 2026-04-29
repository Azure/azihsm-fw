// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MEMORY_PROTECTION_MANTICORE_SPRT_H_
#define MEMORY_PROTECTION_MANTICORE_SPRT_H_

#include "fence_interface.h"
#include "mpu/memory_protection_mpu_only.h"


/**
 * Handler for configuring memory protection during Manticore SPRT execution.
 */
struct memory_protection_manticore_sprt {
	struct memory_protection_mpu_only base;	/**< The base API, providing MPU protections. */
	const struct fence_interface *fence;	/**< Memory fencing interface */
};


int memory_protection_manticore_sprt_init (struct memory_protection_manticore_sprt *mem_protect,
	const struct fence_interface *fence, const struct mpu_interface *mpu,
	const struct memory_protection_mpu_only_region *regions, size_t count);
void memory_protection_manticore_sprt_release (
	const struct memory_protection_manticore_sprt *mem_protect);


#endif	/* MEMORY_PROTECTION_MANTICORE_SPRT_H_ */
