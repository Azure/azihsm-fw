// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_CRASHDUMP_HW_ERR_HANDLER_MPU_ERR_STATIC_H_
#define HSP_CRASHDUMP_HW_ERR_HANDLER_MPU_ERR_STATIC_H_

#include "hsp_crashdump_hw_err_handler_mpu_err.h"
#include "hsp_crashdump_hw_err_handler_static.h"


/* Internal functions declared to allow for static initialization. */
bool hsp_crashdump_hw_err_handler_mpu_err (const struct hsp_interrupt_handler *handler,
	uintptr_t param);

/**
 * Initialize a static instance of a crashdump mpu error handling.
 *
 * There is no validation done on the arguments
 * @param[in] hsp_crshdump_handler The HSP crashdump handler pointer.
 * @param[in] creg_regs The Creg register pointer.
 * @param[in] mpu_reg_set The CREG offset for the hardware MPU registers.
 * @param[in] mpu_dram_reg_set The CREG offset for the hardware MPU SPDRAM registers..
 * @param[in] mpu_iram_reg_set The CREG offset for the hardware MPU SPIRAM registers.
 * @param[in] mpu_rom_reg_set The CREG offset for the hardware MPU SPROM registers.
 */
#define hsp_crashdump_hw_err_handler_mpu_err_static_init(hsp_crshdump_handler, \
	creg_regs, mpu_reg_set, mpu_dram_reg_set, mpu_iram_reg_set, \
	mpu_rom_reg_set) { \
	.base = hsp_crashdump_hw_err_handler_static_init (hsp_crshdump_handler, \
		creg_regs, hsp_crashdump_hw_err_handler_mpu_err), \
	.mpu_regs = mpu_reg_set, \
	.mpu_dram_regs = mpu_dram_reg_set, \
	.mpu_iram_regs = mpu_iram_reg_set, \
	.mpu_rom_regs = mpu_rom_reg_set, \
}


#endif	/* HSP_CRASHDUMP_HW_ERR_HANDLER_MPU_ERR_STATIC_H_ */
