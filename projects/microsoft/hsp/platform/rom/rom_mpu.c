// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include "hsp_top.h"
#include "rom_mpu.h"
#include "logging/code_path_integrity.h"


/**
 * Define all pages in a register for Read and Execute access.
 */
#define	ROM_MPU_RX		0x55555555

/**
 * Define all pages in a register for Read and Write access.
 */
#define	ROM_MPU_RW		0x33333333

/**
 * Define all pages in a register for Read, Write, and Execute access.
 */
#define	ROM_MPU_RWX		0x77777777


/**
 * Configure the MPU for SP DRAM.  No settings will be locked.  Read and Write permissions will
 * always be set.
 *
 * @param mpu Base address of the SP DRAM MPU registers.
 * @param execute Flag indicating if execute access should be set for the memory.
 */
void rom_mpu_set_dram (struct Creg_regs_spdram_mpu_regs *mpu, bool execute)
{
	uint32_t access = (execute) ? ROM_MPU_RWX : ROM_MPU_RW;
	size_t i;

	for (i = 0; i < (sizeof (mpu->SPDRAM_USER_ATTRIB) / sizeof (uint32_t)); i++) {
		mpu->SPDRAM_USER_ATTRIB[i] = access;
		mpu->SPDRAM_PRIVILEGE_ATTRIB[i] = access;
	}

	code_path_integrity_secure_message_no_trace (1);
	code_path_integrity_secure_message_no_trace (access);
}

/**
 * Configure the MPU for SP IRAM.  No settings will be locked.  Read and Write permissions will
 * always be set.
 *
 * @param mpu Base address of the SP IRAM MPU registers.
 * @param execute Flag indicating if execute access should be set for the memory.
 */
void rom_mpu_set_iram (struct Creg_regs_spiram_mpu_regs *mpu, bool execute)
{
	uint32_t access = (execute) ? ROM_MPU_RWX : ROM_MPU_RW;
	size_t i;

	for (i = 0; i < (sizeof (mpu->SPIRAM_USER_ATTRIB) / sizeof (uint32_t)); i++) {
		mpu->SPIRAM_USER_ATTRIB[i] = access;
		mpu->SPIRAM_PRIVILEGE_ATTRIB[i] = access;
	}

	code_path_integrity_secure_message_no_trace (2);
	code_path_integrity_secure_message_no_trace (access);
}

/**
 * Configure the MPU to disallow execute permissions on SP IRAM and DRAM, but only in Production or
 * Secure states.
 *
 * @param sec_state The current security state of the device.
 * @param mpu_dram MPU registers for SP DRAM.
 * @param mpu_iram MPU registers for SP IRAM.
 */
void rom_mpu_initialize (enum hsp_security_state sec_state,
	struct Creg_regs_spdram_mpu_regs *mpu_dram, struct Creg_regs_spiram_mpu_regs *mpu_iram)
{
	if ((sec_state == HSP_SECURITY_STATE_PRODUCTION) || (sec_state == HSP_SECURITY_STATE_SECURE)) {
		rom_mpu_set_dram (mpu_dram, false);
		rom_mpu_set_iram (mpu_iram, false);
	}
}
