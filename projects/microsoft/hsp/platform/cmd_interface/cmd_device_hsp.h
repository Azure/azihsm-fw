// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_DEVICE_HSP_H_
#define CMD_DEVICE_HSP_H_

#include "cmd_interface/cmd_device.h"
#include "cmd_interface/counter_manager_registers.h"


/* Defined in HSP register definition. */
struct Gfc_regs;
struct Creg_regs_misc_creg_sw_regs;

/**
 * The HSP handler for device operations during command processing.
 */
struct cmd_device_hsp {
	struct cmd_device base;							/**< The base device command handler. */
	struct counter_manager_registers *counter;		/**< Reset counter instance. */
	const struct Gfc_regs *fuse_regs;				/**< Register interface for the fuse controller. */
	struct Creg_regs_misc_creg_sw_regs *sw_regs;	/**< Register interface for SW reset. */
};


int cmd_device_hsp_init (struct cmd_device_hsp *device,	struct counter_manager_registers *counter,
	const struct Gfc_regs *fuse_regs, struct Creg_regs_misc_creg_sw_regs *sw_regs);
void cmd_device_hsp_release (const struct cmd_device_hsp *device);

/* Internal functions for use by derived types. */
int cmd_device_hsp_init_no_heap_stats (struct cmd_device_hsp *device,
	struct counter_manager_registers *counter, const struct Gfc_regs *fuse_regs,
	struct Creg_regs_misc_creg_sw_regs *sw_regs);


#endif	/* CMD_DEVICE_HSP_H_ */
