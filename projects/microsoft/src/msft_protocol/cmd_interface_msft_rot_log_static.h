// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_ROT_LOG_STATIC_H_
#define CMD_INTERFACE_MSFT_ROT_LOG_STATIC_H_

#include "cmd_interface_msft_base_static.h"
#include "cmd_interface_msft_rot_log.h"


/* Internal functions declared to allow for static initialization. */
int cmd_interface_msft_rot_log_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request);


/**
 * Constant initializer for the command interface API.
 */
#define	CMD_INTERFACE_MSFT_ROT_LOG_API_INIT { \
		.process_request = cmd_interface_msft_rot_log_process_request, \
		.session = NULL, \
	}


/**
 * Initializes a static instance of a MSFT RoT log command set handler.
 *
 * There is no validation done on the arguments.
 *
 * @param log_ptr The Rot log interface for get log data.
 * @param hash_ptr The hashing engine for hash of log data.
 *
 */
#define	cmd_interface_msft_rot_log_static_init(log_ptr, hash_ptr) { \
		.base = CMD_INTERFACE_MSFT_ROT_LOG_API_INIT, \
		.log = log_ptr,\
		.hash = hash_ptr,\
	}


#endif	/* CMD_INTERFACE_MSFT_ROT_LOG_STATIC_H_ */
