// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_DEVICE_HSP_STATIC_H_
#define CMD_DEVICE_HSP_STATIC_H_

#include "cmd_interface/cmd_device_hsp.h"


/* Internal functions declared to allow for static initialization. */
int cmd_device_hsp_get_uuid (const struct cmd_device *device, uint8_t *buffer, size_t buf_len);
int cmd_device_hsp_reset (const struct cmd_device *device);
int cmd_device_hsp_get_reset_counter (const struct cmd_device *device, uint8_t type, uint8_t port,
	uint16_t *counter);
int cmd_device_hsp_get_heap_stats (const struct cmd_device *device,
	struct cmd_device_heap_stats *heap);


/**
 * Constant initializer for the heap stats request.
 */
#ifdef CMD_ENABLE_HEAP_STATS
#define	CMD_DEVICE_HSP_HEAP_STATS_API	.get_heap_stats = cmd_device_hsp_get_heap_stats
#else
#define	CMD_DEVICE_HSP_HEAP_STATS_API
#endif


/**
 * Constant initializer for the device command API.
 */
#define	CMD_DEVICE_HSP_API_INIT  { \
		.get_uuid = cmd_device_hsp_get_uuid, \
		.reset = cmd_device_hsp_reset, \
		.get_reset_counter = cmd_device_hsp_get_reset_counter, \
		CMD_DEVICE_HSP_HEAP_STATS_API \
	}


/**
 * Initialize a static instance of a handler for HSP device requests.
 *
 * There is no validation done on the arguments.
 *
 * @param counter_ptr The reset counter manager for the device.
 * @param fuse_regs_ptr Register interface to the fuse controller.
 * @param sw_regs_ptr Register interface for software management registers, include HSP reset.
 */
#define	cmd_device_hsp_static_init(counter_ptr, fuse_regs_ptr, sw_regs_ptr)	{ \
		.base = CMD_DEVICE_HSP_API_INIT, \
		.counter = counter_ptr, \
		.fuse_regs = fuse_regs_ptr, \
		.sw_regs = sw_regs_ptr \
	}

/* Internal initializers for use by derived types. */

/**
 * Initialize the base instance of a handler for HSP device requests.
 *
 * There is no validation done on the arguments.
 *
 * @param api The list of API functions to assign to the instance.
 * @param counter_ptr The reset counter manager for the device.
 * @param fuse_regs_ptr Register interface to the fuse controller.
 * @param sw_regs_ptr Register interface for software management registers, include HSP reset.
 */
#define	cmd_device_hsp_static_init_with_api(api, counter_ptr, fuse_regs_ptr, sw_regs_ptr) { \
		.base = api, \
		.counter = counter_ptr, \
		.fuse_regs = fuse_regs_ptr, \
		.sw_regs = sw_regs_ptr \
	}


#endif	/* CMD_DEVICE_HSP_STATIC_H_ */
