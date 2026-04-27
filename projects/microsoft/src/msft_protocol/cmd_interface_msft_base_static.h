// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_BASE_STATIC_H_
#define CMD_INTERFACE_MSFT_BASE_STATIC_H_

#include "cmd_interface_msft_base.h"


/* Internal functions declared to allow for static initialization. */
int cmd_interface_msft_base_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request);


/**
 * Constant initializer for the command interface API.
 */
#define	CMD_INTERFACE_MSFT_BASE_API_INIT { \
		.process_request = cmd_interface_msft_base_process_request, \
		.session = NULL, \
	}


/**
 * Initializes a static instance of a MSFT Base command set handler.
 *
 * There is no validation done on the arguments.
 *
 * @param cmd_sets_ptr The list of all command sets supported by the device.
 * @param set_count_arg Number of supported command sets in the list.
 * @param device_mgr_ptr The manager for information about external devices.
 * @param cluster_ptr The manager for temperature sensors in the device.  This can be null if the
 * device does not support reading temperature sensors.
 * @param hb_notifier_ptr The instance of MCTP notifier used to send heartbeat notifications.
 * This can be null if the device does not support sending heartbeat notifications.
 */
#define	cmd_interface_msft_base_static_init(cmd_sets_ptr, set_count_arg, device_mgr_ptr, \
	cluster_ptr, hb_notifier_ptr) { \
		.base = CMD_INTERFACE_MSFT_BASE_API_INIT, \
		.cmd_sets = cmd_sets_ptr, \
		.set_count = set_count_arg, \
		.device_mgr = device_mgr_ptr, \
		.temp_sensors = cluster_ptr, \
		.hb_notifier = hb_notifier_ptr, \
	}


#endif	/* CMD_INTERFACE_MSFT_BASE_STATIC_H_ */
