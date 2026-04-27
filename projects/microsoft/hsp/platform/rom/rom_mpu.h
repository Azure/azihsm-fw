// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef ROM_MPU_H_
#define ROM_MPU_H_

#include <stdbool.h>
#include "hsp_top.h"
#include "drivers/hsp_security_state.h"


void rom_mpu_set_dram (struct Creg_regs_spdram_mpu_regs *mpu, bool execute);
void rom_mpu_set_iram (struct Creg_regs_spiram_mpu_regs *mpu, bool execute);

void rom_mpu_initialize (enum hsp_security_state sec_state,
	struct Creg_regs_spdram_mpu_regs *mpu_dram, struct Creg_regs_spiram_mpu_regs *mpu_iram);


#endif	/* ROM_MPU_H_ */
