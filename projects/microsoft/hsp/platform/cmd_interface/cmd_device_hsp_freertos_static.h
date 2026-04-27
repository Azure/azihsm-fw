// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_DEVICE_HSP_FREERTOS_STATIC_H_
#define CMD_DEVICE_HSP_FREERTOS_STATIC_H_

#include "cmd_interface/cmd_device_hsp_freertos.h"
#include "cmd_interface/cmd_device_hsp_static.h"


/* Internal functions declared to allow for static initialization. */
int cmd_device_hsp_freertos_get_heap_stats (const struct cmd_device *device,
	struct cmd_device_heap_stats *heap);


/**
 * Constant initializer for the heap stats request.
 */
#ifdef CMD_ENABLE_HEAP_STATS
#define	CMD_DEVICE_HSP_FREERTOS_HEAP_STATS_API  \
	.get_heap_stats = cmd_device_hsp_freertos_get_heap_stats
#else
#define	CMD_DEVICE_HSP_FREERTOS_HEAP_STATS_API
#endif


/**
 * Constant initializer for the device command API.
 */
#define	CMD_DEVICE_HSP_FREERTOS_API_INIT  { \
		.get_uuid = cmd_device_hsp_get_uuid, \
		.reset = cmd_device_hsp_reset, \
		.get_reset_counter = cmd_device_hsp_get_reset_counter, \
		CMD_DEVICE_HSP_FREERTOS_HEAP_STATS_API \
	}


/**
 * Initialize a static instance of a handler for HSP device requests running on FreeRTOS.
 *
 * There is no validation done on the arguments.
 *
 * @param counter_ptr The reset counter manager for the device.
 * @param fuse_regs_ptr Register interface to the fuse controller.
 * @param sw_regs_ptr Register interface for software management registers, include HSP reset.
 * @param total_heap_ptr The total amount of space reserved by the application for the FreeRTOS
 * heap.
 */
#define	cmd_device_hsp_freertos_static_init(counter_ptr, fuse_regs_ptr, sw_regs_ptr, \
	total_heap_ptr)	{ \
		.base = cmd_device_hsp_static_init_with_api (CMD_DEVICE_HSP_FREERTOS_API_INIT, \
			counter_ptr, fuse_regs_ptr, sw_regs_ptr), \
		.total_heap = total_heap_ptr, \
	}


#endif	/* CMD_DEVICE_HSP_FREERTOS_STATIC_H_ */
