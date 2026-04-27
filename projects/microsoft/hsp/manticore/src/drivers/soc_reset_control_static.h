// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SOC_RESET_CONTROL_STATIC_H_
#define SOC_RESET_CONTROL_STATIC_H_

#include "drivers/soc_reset_control.h"


/* Internal functions declared to allow for static initialization. */
int soc_reset_control_map_por_registers (const struct soc_reset_control *reset_ctrl,
	void **por_regs);
void soc_reset_control_unmap_por_registers (const struct soc_reset_control *reset_ctrl,
	void *por_regs);
void soc_reset_control_reset_all_cores (const struct soc_reset_control *reset_ctrl, void *por_regs,
	bool reset);
void soc_reset_control_stall_all_cores (const struct soc_reset_control *reset_ctrl, void *por_regs,
	bool stall);
void soc_reset_control_reset_cp_core (const struct soc_reset_control *reset_ctrl, void *por_regs,
	bool reset, enum soc_reset_control_cp_core core);
bool soc_reset_control_is_cp_in_reset (const struct soc_reset_control *reset_ctrl, void *por_regs,
	enum soc_reset_control_cp_core core);
void soc_reset_control_stall_cp_core (const struct soc_reset_control *reset_ctrl, void *por_regs,
	bool stall, enum soc_reset_control_cp_core core);
bool soc_reset_control_is_cp_stalled (const struct soc_reset_control *reset_ctrl, void *por_regs,
	enum soc_reset_control_cp_core core);
void soc_reset_control_reset_fp_core (const struct soc_reset_control *reset_ctrl, void *por_regs,
	bool reset, enum soc_reset_control_fp_core core);
bool soc_reset_control_is_fp_in_reset (const struct soc_reset_control *reset_ctrl, void *por_regs,
	enum soc_reset_control_fp_core core);
void soc_reset_control_stall_fp_core (const struct soc_reset_control *reset_ctrl, void *por_regs,
	bool stall, enum soc_reset_control_fp_core core);
bool soc_reset_control_is_fp_stalled (const struct soc_reset_control *reset_ctrl, void *por_regs,
	enum soc_reset_control_fp_core core);


/**
 * Constant initializer for the reset controller API.
 */
#define	SOC_RESET_CONTROL_API_INIT  \
	.map_por_registers = soc_reset_control_map_por_registers, \
	.unmap_por_registers = soc_reset_control_unmap_por_registers, \
	.reset_all_cores = soc_reset_control_reset_all_cores, \
	.stall_all_cores = soc_reset_control_stall_all_cores, \
	.reset_cp_core = soc_reset_control_reset_cp_core, \
	.is_cp_in_reset = soc_reset_control_is_cp_in_reset, \
	.stall_cp_core = soc_reset_control_stall_cp_core, \
	.is_cp_stalled = soc_reset_control_is_cp_stalled, \
	.reset_fp_core = soc_reset_control_reset_fp_core, \
	.is_fp_in_reset = soc_reset_control_is_fp_in_reset, \
	.stall_fp_core = soc_reset_control_stall_fp_core, \
	.is_fp_stalled = soc_reset_control_is_fp_stalled


/**
 * Initialize a static instance of a driver for controlling reset of Manticore SoC hardware
 * components.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the driver.
 * @param dmb_ptr THe HSP DMB driver for mapping SoC addresses.
 */
#define	soc_reset_control_static_init(state_ptr, dmb_ptr)	{ \
		SOC_RESET_CONTROL_API_INIT, \
		.state = state_ptr, \
		.dmb = dmb_ptr, \
	}


#endif	/* SOC_RESET_CONTROL_STATIC_H_ */
