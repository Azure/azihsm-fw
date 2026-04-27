// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef IPC_MESSAGE_HANDLER_STATIC_H_
#define IPC_MESSAGE_HANDLER_STATIC_H_

#include <stdint.h>
#include <string.h>
#include "ipc_message_handler.h"


/* Internal functions declared to allow for static initialization. */
void ipc_message_handler_prepare (const struct periodic_task_handler *handler);
const platform_clock* ipc_message_handler_get_next_execution (
	const struct periodic_task_handler *handler);
void ipc_message_handler_execute (const struct periodic_task_handler *handler);


/**
 * Constant initializer for the IPC Message handler API.
 */
#define	IPC_MESSAGE_HANDLER_API_INIT { \
		.prepare = ipc_message_handler_prepare, \
		.get_next_execution = ipc_message_handler_get_next_execution, \
		.execute = ipc_message_handler_execute, \
	}

/**
 * Initialize a static for IPC message Handler.
 * There are no validation done on the arguments.
 *
 * @param[in]	cmd_interface_ptr	A pointer to an implementation of struct cmd_interface object
 * @param[in]	ipc_channel_ptr		A pointer to an implementation of stuct ipc_channel object
 */
#define	ipc_message_handler_static_init(cmd_interface_ptr, ipc_channel_ptr) { \
		.base = IPC_MESSAGE_HANDLER_API_INIT, \
		.cmd = cmd_interface_ptr, \
		.channel = ipc_channel_ptr, \
	}


#endif	/* IPC_MESSAGE_HANDLER_STATIC_H_ */
