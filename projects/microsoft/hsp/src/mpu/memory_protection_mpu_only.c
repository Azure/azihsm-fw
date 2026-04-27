// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "memory_protection_mpu_only.h"
#include "common/unused.h"


int memory_protection_mpu_only_configure_hsp_mpu (const struct memory_protection *mem_protect)
{
	const struct memory_protection_mpu_only *hsp =
		(const struct memory_protection_mpu_only*) mem_protect;
	size_t i;
	int status;

	if (mem_protect == NULL) {
		return MEMORY_PROTECTION_INVALID_ARGUMENT;
	}

	if (hsp->regions == NULL) {
		/* No regions have been defined, so there is nothing to do. */
		return 0;
	}

	for (i = 0; i < hsp->region_count; i++) {
		status = hsp->mpu->set_region_attributes (hsp->mpu, hsp->regions[i].start,
			hsp->regions[i].end - hsp->regions[i].start, hsp->regions[i].protection_level,
			hsp->regions[i].page_attributes);
		if (status != 0) {
			return status;
		}
	}

	return 0;
}

int memory_protection_mpu_only_configure_soc_fences (const struct memory_protection *mem_protect)
{
	if (mem_protect == NULL) {
		return MEMORY_PROTECTION_INVALID_ARGUMENT;
	}

	/* No SoC fences to configure. */
	return 0;
}

/**
 * Initialize a handler for configuring HSP memory protections using the MPU.
 *
 * @param mem_protect The memory protection handler to initialize.
 * @param mpu The MPU driver for HSP.
 * @param regions A list of memory regions that should be configured in the MPU.
 * @param count The number of memory regions in the list.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int memory_protection_mpu_only_init (struct memory_protection_mpu_only *mem_protect,
	const struct mpu_interface *mpu, const struct memory_protection_mpu_only_region *regions,
	size_t count)
{
	if ((mem_protect == NULL) || (mpu == NULL)) {
		return MEMORY_PROTECTION_INVALID_ARGUMENT;
	}

	if (((regions != NULL) && (count == 0)) || ((regions == NULL) && (count != 0))) {
		return MEMORY_PROTECTION_INVALID_ARGUMENT;
	}

	memset (mem_protect, 0, sizeof (struct memory_protection_mpu_only));

	mem_protect->base.configure_hsp_mpu = memory_protection_mpu_only_configure_hsp_mpu;
	mem_protect->base.configure_soc_fences = memory_protection_mpu_only_configure_soc_fences;

	mem_protect->mpu = mpu;
	mem_protect->regions = regions;
	mem_protect->region_count = count;

	return 0;
}

/**
 * Release the resources used for HSP memory protections.
 *
 * @param mem_protect The memory protection handler to release.
 */
void memory_protection_mpu_only_release (const struct memory_protection_mpu_only *mem_protect)
{
	UNUSED (mem_protect);
}
