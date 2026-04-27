// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MEMORY_PROTECTION_MPU_ONLY_H_
#define MEMORY_PROTECTION_MPU_ONLY_H_

#include <stddef.h>
#include "memory_protection.h"
#include "common/sram_util.h"
#include "mpu/mpu.h"


/**
 * Specifies a region of SP memory that should have MPU attributes applied.
 */
struct memory_protection_mpu_only_region {
	/**
	 * Address for the start of the memory region.  This address must be 4kB aligned.
	 */
	const void *start;

	/**
	 * Address for the end of the memory region.  This address must be 4kB aligned.
	 *
	 * This should be the first address following the range being protected.  This address will not
	 * have MPU attributes applied to it.
	 */
	const void *end;

	/**
	 * The MPU protection level flags to use for the region.
	 */
	uint32_t protection_level;

	/**
	 * The MPU attributes to set for the region.
	 */
	uint32_t page_attributes;
};

/**
 * Handler for configuring memory protection during SP firmware execution.  This only provides MPU
 * protections and will not configure any memory fences.
 */
struct memory_protection_mpu_only {
	struct memory_protection base;								/**< The base memory protection API. */
	const struct mpu_interface *mpu;							/**< Interface to the HSP MPU. */
	const struct memory_protection_mpu_only_region *regions;	/**< Memory regions to configure. */
	size_t region_count;										/**< The number of memory regions. */
};


int memory_protection_mpu_only_init (struct memory_protection_mpu_only *mem_protect,
	const struct mpu_interface *mpu, const struct memory_protection_mpu_only_region *regions,
	size_t count);
void memory_protection_mpu_only_release (const struct memory_protection_mpu_only *mem_protect);

/* Internal functions for use by derived types. */
int memory_protection_mpu_only_configure_hsp_mpu (const struct memory_protection *mem_protect);
int memory_protection_mpu_only_configure_soc_fences (const struct memory_protection *mem_protect);


#endif	/* MEMORY_PROTECTION_MPU_ONLY_H_ */
