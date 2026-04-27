// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_DEVICE_HSP_FREERTOS_H_
#define CMD_DEVICE_HSP_FREERTOS_H_

#include "cmd_interface/cmd_device_hsp.h"


/**
 * The HSP handler for device operations during command processing.
 */
struct cmd_device_hsp_freertos {
	struct cmd_device_hsp base;	/**< The base device command handler. */
	const uint32_t *total_heap;	/**< The total amount of space allocated for the heap. */
};


int cmd_device_hsp_freertos_init (struct cmd_device_hsp_freertos *device,
	struct counter_manager_registers *counter, const struct Gfc_regs *fuse_regs,
	struct Creg_regs_misc_creg_sw_regs *sw_regs, const uint32_t *total_heap);
void cmd_device_hsp_freertos_release (const struct cmd_device_hsp_freertos *device);


#endif	/* CMD_DEVICE_HSP_FREERTOS_H_ */
