// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MEMORY_PROTECTION_MANTICORE_1SP_H_
#define MEMORY_PROTECTION_MANTICORE_1SP_H_

#include "fence_interface.h"
#include "mpu/memory_protection.h"
#include "mpu/mpu.h"

/**
 * Handler for configuring memory protection during Manticore 1SP execution.
 */
struct memory_protection_manticore_1sp {
	struct memory_protection base;		/**< The base memory protection API. */
	const struct mpu_interface *mpu;	/**< Interface to the HSP MPU. */
	const void *exe_start;				/**< Start of the executable memory region. */
	const void *ro_start;				/**< Start of the read-only memory region. */
	const void *rw_start;				/**< Start of the read/write memory region. */
	const void *memory_end;				/**< End of the 1SP memory region. */
};


int memory_protection_manticore_1sp_init (struct memory_protection_manticore_1sp *mem_protect,
	const struct mpu_interface *mpu, const void *exe_start, const void *ro_start,
	const void *rw_start, const void *memory_end);
void memory_protection_manticore_1sp_release (
	const struct memory_protection_manticore_1sp *mem_protect);


#endif	/* MEMORY_PROTECTION_MANTICORE_1SP_H_ */
