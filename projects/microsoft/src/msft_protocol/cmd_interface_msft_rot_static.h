// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_ROT_STATIC_H_
#define CMD_INTERFACE_MSFT_ROT_STATIC_H_

#include "cmd_interface_msft_base_static.h"
#include "cmd_interface_msft_rot.h"


/* Internal functions declared to allow for static initialization. */
int cmd_interface_msft_rot_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request);

/**
 * Constant initializer for the command interface API.
 */
#define	CMD_INTERFACE_MSFT_ROT_API_INIT { \
		.process_request = cmd_interface_msft_rot_process_request, \
		.session = NULL, \
	}


/**
 * Initializes a static instance of a MSFT RoT command set handler.
 *
 * There is no validation done on the arguments.
 *
 * @param unlock_ptr The manager for device unlock requests.  This can be null if the device does
 * not support unlock operations.
 * @param rtc_ptr The handler for system clock requests.  This can be null if the device does not
 * support time operations.
 * @param reboot_ptr The background command handler for triggering device resets.  This can be null
 * if the device does not support commands that cause a device reset.
 * @param intrusion_ptr The intrusion state for intrusion requests. This can be null if the device
 * does not support intrusion operations.
 * @param notifier_ptr The MCTP notifier interface for intrusion event registration requests. This
 * can be null if the device does not support intrusion event registration operations.
 * @param cmd_log_ptr The RoT log interface for log command requests. This can be null if the device
 * does not support log operations.
 */
#define	cmd_interface_msft_rot_static_init(unlock_ptr, rtc_ptr, reboot_ptr, intrusion_ptr, \
	notifier_ptr, cmd_log_ptr) { \
		.base = CMD_INTERFACE_MSFT_ROT_API_INIT, \
		.unlock = unlock_ptr, \
		.rtc = rtc_ptr, \
		.reboot = reboot_ptr, \
		.intrusion = intrusion_ptr, \
		.notifier = notifier_ptr, \
		.cmd_log = cmd_log_ptr, \
	}


#endif	/* CMD_INTERFACE_MSFT_ROT_STATIC_H_ */
