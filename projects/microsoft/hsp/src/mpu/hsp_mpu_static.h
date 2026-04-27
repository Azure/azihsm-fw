// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_MPU_STATIC_H_
#define HSP_MPU_STATIC_H_

#include "hsp_mpu.h"


/* Forward declarations */
int hsp_mpu_get_page_size (const struct mpu_interface *mpu, size_t *page_size);
int hsp_mpu_set_region_attributes (const struct mpu_interface *mpu,	const void *region_address,
	size_t region_size, uint32_t protection_level, uint32_t page_attributes);
int hsp_mpu_get_page_attributes (const struct mpu_interface *mpu, const void *address,
	uint32_t protection_level, uint32_t *page_attributes);


/**
 * Static initializer for MPU API
 */
#define HSP_MPU_API_INIT { \
	.get_page_size = hsp_mpu_get_page_size, \
	.set_region_attributes = hsp_mpu_set_region_attributes, \
	.get_page_attributes = hsp_mpu_get_page_attributes, \
}

/**
 * Static initializer for HSP MPU object
 *
 * @param mpu_registers_arg - MPU registers block
 * @param page_size_arg - MPU page size
 * @param memory_entries_arg - array of memory region descriptors
 * @param entries_count_arg - number of memory regions
 */
#define hsp_mpu_static_init(mpu_registers_arg, page_size_arg, memory_entries_arg, \
	entries_count_arg) { \
		.base = HSP_MPU_API_INIT, \
		.mpu_register_block = mpu_registers_arg, \
		.page_size = page_size_arg, \
		.memory_map_entries = memory_entries_arg, \
		.entries_count = entries_count_arg, \
}


#endif	// HSP_MPU_STATIC_H_
