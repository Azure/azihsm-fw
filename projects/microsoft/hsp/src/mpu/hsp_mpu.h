// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_MPU_H_
#define HSP_MPU_H_

#include "common/sram_util.h"
#include "mmio/mmio_register_block.h"
#include "mpu/mpu.h"


/**
 * This struct binds together a specific memory region defined by starting address and region
 * length with a block of registers responsible for controlling protection for the pages of
 * that region. One entry must be defined for each hardware supported region. For example,
 * most HSP implementations would at least have two entries: one entry for the ITCM SRAM region
 * and another for DTCM.
 */
struct hsp_mpu_memory_map_entry {
	struct sram_block memory_region;	/**< Memory region for this entry */
	size_t user_register_offset;		/**< Offset of USER attribute registers for this region */
	size_t privileged_register_offset;	/**< Offset of PRIVILEGED attribute registers for this region */
};

/**
 * HSP implementation of MPU driver
 */
struct hsp_mpu {
	struct mpu_interface base;									/**< MPU interface */
	const struct mmio_register_block *mpu_register_block;		/**< MMIO register block for MPU */
	size_t page_size;											/**< MPU supported page size */
	const struct hsp_mpu_memory_map_entry *memory_map_entries;	/**< Memory map entries */
	size_t entries_count;										/**< Number of memory map entries */
};


int hsp_mpu_init (struct hsp_mpu *mpu, const struct mmio_register_block *mpu_registers,
	size_t page_size, const struct hsp_mpu_memory_map_entry *memory_entries, size_t entries_count);
void hsp_mpu_release (const struct hsp_mpu *mpu);


#endif	// HSP_MPU_H_
