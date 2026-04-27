// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HOST_FW_CMD_HANDLER_H_
#define HOST_FW_CMD_HANDLER_H_

#include "cmd_interface/cerberus_protocol.h"
#include "host_fw/host_fw_cmd_interface.h"
#include "system/event_task.h"


/**
 * Action identifiers for the firmware update handler.
 */
enum {
	HOST_FW_CMD_HANDLER_ACTION_PREPARE = 1,	/**< Prepare the flash to receive an update. */
	HOST_FW_CMD_HANDLER_ACTION_WRITE = 2,	/**< Write image data into flash. */
};


/**
 * Context needed to be sent to task handler for prepare image.
 */
struct host_fw_cmd_prepare_event_context {
	uint32_t total_size;	/**< Image size to be updated */
	bool prioritize_update;	/**< Flag indicating prioritization of update  */
	uint8_t ctrl_flag;		/**< Control flag for the platform specific opeations */
};

/**
 * Variable context for host firmware command handler.
 */
struct host_fw_cmd_handler_state {
	int fw_status;	/**< The firmware operation status. */
};

/**
 * The task context for executing requests on a single image of host firmware.
 */
struct host_fw_cmd_handler {
	struct host_fw_cmd_interface base;			/**< The base API for interfacing with the task. */
	struct event_task_handler base_event;		/**< The task event handler interface. */
	struct host_fw_cmd_handler_state *state;	/**< Variable context for the task. */
	const struct host_firmware *host_fw;		/**< Host firmware interface. */
	const struct event_task *task;				/**< Task to handle firmware update events. */
};


int host_fw_cmd_handler_init (struct host_fw_cmd_handler *handler,
	struct host_fw_cmd_handler_state *state, const struct host_firmware *host_fw,
	const struct event_task *task);
int host_fw_cmd_handler_init_state (const struct host_fw_cmd_handler *handler);
void host_fw_cmd_handler_release (const struct host_fw_cmd_handler *handler);


#endif	/* HOST_FW_CMD_HANDLER_H_ */
