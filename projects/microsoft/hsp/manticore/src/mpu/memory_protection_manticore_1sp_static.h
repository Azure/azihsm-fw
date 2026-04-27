// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MEMORY_PROTECTION_MANTICORE_1SP_STATIC_H_
#define MEMORY_PROTECTION_MANTICORE_1SP_STATIC_H_

#include "memory_protection_manticore_1sp.h"


/* Internal functions declared to allow for static initialization. */
int memory_protection_manticore_1sp_configure_hsp_mpu (const struct memory_protection *mem_protect);
int memory_protection_manticore_1sp_configure_soc_fences (
	const struct memory_protection *mem_protect);


/**
 * Constant initializer for the memory protection API.
 */
#define	MEMORY_PROTECTION_MANTICORE_1SP_API_INIT  { \
		.configure_hsp_mpu = memory_protection_manticore_1sp_configure_hsp_mpu, \
		.configure_soc_fences = memory_protection_manticore_1sp_configure_soc_fences, \
	}


/**
 * Initialize a static instance of a handler for 1SP memory protection configuration.  This can be a
 * constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param mpu_ptr The MPU driver for HSP.
 * @param exe_start Start address in TCM for 1SP executable and read only memory.  This must be 4kB
 * aligned.
 * @param ro_start Start address in TCM for 1SP non-executable, but still read only memory.  This
 * must immediately follow the executable region and must be 4kB aligned.
 * @param rw_start Start address in TCM for 1SP read/write and non-executable memory.  This must
 * immediately follow the read only region and must be 4kB aligned.
 * @param memory_end End of 1SP memory.  This must be 4kB aligned.
 */
#define	memory_protection_manticore_1sp_static_init(mpu_ptr, exe_start_arg, ro_start_arg, \
	rw_start_arg, memory_end_arg)	{ \
		.base = MEMORY_PROTECTION_MANTICORE_1SP_API_INIT, \
		.mpu = mpu_ptr, \
		.exe_start = exe_start_arg, \
		.ro_start = ro_start_arg, \
		.rw_start = rw_start_arg, \
		.memory_end = memory_end_arg, \
	}


#endif	/* MEMORY_PROTECTION_MANTICORE_1SP_STATIC_H_ */
