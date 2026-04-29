// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "memory_protection_manticore_1sp.h"
#include "common/array_size.h"
#include "common/unused.h"
#include "logging/code_path_integrity.h"


/**
 * The value to use for a specific checkpoint step.  This uses the memory protection module ID to
 * provide uniqueness.
 */
#define	MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_VALUE(x)      \
	((MSFT_MODULE_MEMORY_PROTECTION << 8) | (x))

/**
 * Checkpoint values used when applying memory protections.
 */
enum {
	MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_MPU_START =
		MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_VALUE (0x01),	/**< Start applying 1SP MPU settings. */
	MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_MPU_ROM =
		MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_VALUE (0x02),	/**< Blocked access to ROM memory. */
	MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_MPU_EXECUTE =
		MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_VALUE (0x03),	/**< Applied execute access for firmware code. */
	MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_MPU_RO =
		MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_VALUE (0x04),	/**< Applied read-only access for constant data. */
	MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_MPU_RW =
		MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_VALUE (0x05),	/**< Applied read/write access for run-time data. */
};


int memory_protection_manticore_1sp_configure_hsp_mpu (const struct memory_protection *mem_protect)
{
	const struct memory_protection_manticore_1sp *manticore =
		(const struct memory_protection_manticore_1sp*) mem_protect;
	int status;

	code_path_integrity_secure_message_no_trace (MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_MPU_START);

	if (mem_protect == NULL) {
		return MEMORY_PROTECTION_INVALID_ARGUMENT;
	}

	/* Block all access to ROM memory. */
	status = manticore->mpu->set_region_attributes (manticore->mpu,
		(const void*) HSP_ADDR_MAP_SP_ROM_ADDRESS, HSP_ADDR_MAP_SP_ROM_SIZE,
		MPU_PROTECTION_LEVEL_USER | MPU_PROTECTION_LEVEL_PRIVILEGE, MPU_PAGE_ATTRIBUTE_LOCKED);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_MPU_ROM ^
		status);

	/* Update MPU settings for the 1SP memory layout.  Do not lock anything since SPRT will need to
	 * change these settings. */
	status = manticore->mpu->set_region_attributes (manticore->mpu, manticore->exe_start,
		manticore->ro_start - manticore->exe_start,
		MPU_PROTECTION_LEVEL_USER | MPU_PROTECTION_LEVEL_PRIVILEGE,
		MPU_PAGE_ATTRIBUTE_READ | MPU_PAGE_ATTRIBUTE_EXECUTE);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_MPU_EXECUTE ^
		status);

	status = manticore->mpu->set_region_attributes (manticore->mpu, manticore->ro_start,
		manticore->rw_start - manticore->ro_start,
		MPU_PROTECTION_LEVEL_USER | MPU_PROTECTION_LEVEL_PRIVILEGE, MPU_PAGE_ATTRIBUTE_READ);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_MPU_RO ^
		status);

	status = manticore->mpu->set_region_attributes (manticore->mpu, manticore->rw_start,
		manticore->memory_end - manticore->rw_start,
		MPU_PROTECTION_LEVEL_USER | MPU_PROTECTION_LEVEL_PRIVILEGE,
		MPU_PAGE_ATTRIBUTE_READ | MPU_PAGE_ATTRIBUTE_WRITE);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (MEMORY_PROTECTION_MANTICORE_1SP_CHKPT_MPU_RW ^
		status);

	return 0;
}

int memory_protection_manticore_1sp_configure_soc_fences (
	const struct memory_protection *mem_protect)
{
	if (mem_protect == NULL) {
		return MEMORY_PROTECTION_INVALID_ARGUMENT;
	}

	/* No fencing to configure in 1SP. */
	return 0;
}

/**
 * Initialize a handler for configuring 1SP memory protections.
 *
 * @param mem_protect The configuration handler to initialize.
 * @param mpu The MPU driver for HSP.
 * @param exe_start Start address in TCM for 1SP executable and read only memory.  This must be 4kB
 * aligned.
 * @param ro_start Start address in TCM for 1SP non-executable, but still read only memory.  This
 * must immediately follow the executable region and must be 4kB aligned.
 * @param rw_start Start address in TCM for 1SP read/write and non-executable memory.  This must
 * immediately follow the read only region and must be 4kB aligned.
 * @param memory_end End of 1SP memory.  This must be 4kB aligned.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int memory_protection_manticore_1sp_init (struct memory_protection_manticore_1sp *mem_protect,
	const struct mpu_interface *mpu, const void *exe_start, const void *ro_start,
	const void *rw_start, const void *memory_end)
{
	if ((mem_protect == NULL) || (mpu == NULL)) {
		return MEMORY_PROTECTION_INVALID_ARGUMENT;
	}

	memset (mem_protect, 0, sizeof (struct memory_protection_manticore_1sp));

	mem_protect->base.configure_hsp_mpu = memory_protection_manticore_1sp_configure_hsp_mpu;
	mem_protect->base.configure_soc_fences = memory_protection_manticore_1sp_configure_soc_fences;
	mem_protect->mpu = mpu;
	mem_protect->exe_start = exe_start;
	mem_protect->ro_start = ro_start;
	mem_protect->rw_start = rw_start;
	mem_protect->memory_end = memory_end;

	return 0;
}

/**
 * Release the resources used by 1SP memory protections.
 *
 * @param mem_protect The configuration handler to release.
 */
void memory_protection_manticore_1sp_release (
	const struct memory_protection_manticore_1sp *mem_protect)
{
	UNUSED (mem_protect);
}
