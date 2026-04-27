// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HOST_FW_CMD_HANDLER_STATIC_H_
#define HOST_FW_CMD_HANDLER_STATIC_H_

#include "host_fw/host_fw_cmd_handler.h"


/* Internal functions declared to allow for static initialization. */
int host_fw_cmd_handler_prepare_update (const struct host_fw_cmd_interface *update,
	uint32_t total_size, bool prioritize_update, uint8_t ctrl_flag);
int host_fw_cmd_handler_write_update (const struct host_fw_cmd_interface *update,
	const uint8_t *data, size_t length);
int host_fw_cmd_handler_get_status (const struct host_fw_cmd_interface *update);
int host_fw_cmd_handler_get_update_remaining (const struct host_fw_cmd_interface *update);

void host_fw_cmd_handler_execute (const struct event_task_handler *handler,
	struct event_task_context *context, bool *reset);


/**
 * Constant initializer for the Host firmware command handler interface APIs.
 */
#define	HOST_FW_CMD_HANDLER_API_INIT  { \
		.prepare_update = host_fw_cmd_handler_prepare_update, \
		.write_update = host_fw_cmd_handler_write_update, \
		.get_status = host_fw_cmd_handler_get_status, \
		.get_update_remaining = host_fw_cmd_handler_get_update_remaining \
	}

/**
 * Constant initializer for the host firmware update task API.
 */
#define	HOST_FW_UPDATE_HANDLER_EVENT_API_INIT  { \
		.prepare = NULL, \
		.execute = host_fw_cmd_handler_execute \
	}


/**
 * Initialize a static instance of a host firmware update handler.  This does not initialize the
 * handler state.  This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the host FW update handler.
 * @param host_fw_ptr Host firmware instance that will be used by the handler.
 * @param task_ptr The task that will be used to execute update operations.
 */
#define	host_fw_cmd_handler_static_init(state_ptr, host_fw_ptr, task_ptr) { \
		.base = HOST_FW_CMD_HANDLER_API_INIT, \
		.base_event = HOST_FW_UPDATE_HANDLER_EVENT_API_INIT, \
		.state = state_ptr, \
		.host_fw = host_fw_ptr, \
		.task = task_ptr \
	}


#endif	/* HOST_FW_CMD_HANDLER_STATIC_H_ */
