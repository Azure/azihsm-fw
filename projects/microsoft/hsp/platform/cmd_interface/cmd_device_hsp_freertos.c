// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <string.h>
#include "cmd_device_hsp_freertos.h"
#include "FreeRTOS.h"
#include "hsp_top.h"
#include "common/buffer_util.h"
#include "common/unused.h"
#include "drivers/hsp_fuses.h"
#include "splibs/inc/spcryptotypes.h"


int cmd_device_hsp_freertos_get_heap_stats (const struct cmd_device *device,
	struct cmd_device_heap_stats *heap)
{
	const struct cmd_device_hsp_freertos *freertos = (const struct cmd_device_hsp_freertos*) device;
	HeapStats_t stats;

	if ((device == NULL) || (heap == NULL)) {
		return CMD_DEVICE_INVALID_ARGUMENT;
	}

	memset (heap, 0xff, sizeof (struct cmd_device_heap_stats));

	vPortGetHeapStats (&stats);

	heap->total = *freertos->total_heap;
	heap->free = stats.xAvailableHeapSpaceInBytes;
	heap->min_free = stats.xMinimumEverFreeBytesRemaining;
	heap->free_blocks = stats.xNumberOfFreeBlocks;
	heap->max_block = stats.xSizeOfLargestFreeBlockInBytes;
	heap->min_block = stats.xSizeOfSmallestFreeBlockInBytes;

	return 0;
}

/**
 * Initialize a device command handler for HSP running FreeRTOS.
 *
 * @param device The handler to initialize.
 * @param counter The reset counter manager for the device.
 * @param fuse_regs Register interface to the fuse controller.
 * @param sw_regs Register interface for software management registers, including HSP reset.
 * @param total_heap The total amount of space reserved by the application for the FreeRTOS heap.
 * This is a pointer to allow the possibility for the value to be dynamically determined by linker
 * output.
 *
 * @return 0 if the handler was successfully initialized or an error code.
 */
int cmd_device_hsp_freertos_init (struct cmd_device_hsp_freertos *device,
	struct counter_manager_registers *counter, const struct Gfc_regs *fuse_regs,
	struct Creg_regs_misc_creg_sw_regs *sw_regs, const uint32_t *total_heap)
{
	int status;

	status = cmd_device_hsp_init_no_heap_stats (&device->base, counter, fuse_regs, sw_regs);

#ifdef CMD_ENABLE_HEAP_STATS
	if (status == 0) {
		device->base.base.get_heap_stats = cmd_device_hsp_freertos_get_heap_stats;

		device->total_heap = total_heap;
	}
#endif

	return status;
}

/**
 * Release a Manticore device command handler.
 *
 * @param device The handler to release.
 */
void cmd_device_hsp_freertos_release (const struct cmd_device_hsp_freertos *device)
{
	if (device) {
		cmd_device_hsp_release (&device->base);
	}
}
