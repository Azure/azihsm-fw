// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common/type_cast.h"
#include "common/unused.h"
#include "host_fw/host_firmware.h"
#include "host_fw/host_fw_cmd_handler.h"
#include "host_fw/host_fw_cmd_interface.h"
#include "host_fw/host_logging.h"


/**
 * Set the current firmware operation status.
 *
 * @param handler The task handler to update.
 * @param status The status value to set.
 */
static void host_fw_cmd_handler_set_status (const struct host_fw_cmd_handler *handler, int status)
{
	handler->task->lock (handler->task);
	handler->state->fw_status = status;
	handler->task->unlock (handler->task);
}

void host_fw_cmd_handler_execute (const struct event_task_handler *handler,
	struct event_task_context *context, bool *reset)
{
	const struct host_fw_cmd_handler *fw = TO_DERIVED_TYPE (handler,
		const struct host_fw_cmd_handler, base_event);
	const struct host_firmware *host_fw = fw->host_fw;
	int status = 0;
	bool unknown_action = false;

	UNUSED (reset);

	switch (context->action) {
		case HOST_FW_CMD_HANDLER_ACTION_PREPARE: {
			struct host_fw_cmd_prepare_event_context *prepare_event_context =
				(struct host_fw_cmd_prepare_event_context*) context->event_buffer;

			host_fw_cmd_handler_set_status (fw, HOST_FW_CMD_STATUS_PREPARE);

			status = host_fw->prepare_image_update (host_fw, prepare_event_context->total_size,
				prepare_event_context->prioritize_update, prepare_event_context->ctrl_flag);
			if (status != 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HOST_FW,
					HOST_LOGGING_PREPARE_UPDATE, host_fw->get_port_id (host_fw), status);

				status = HOST_FW_CMD_STATUS (HOST_FW_CMD_STATUS_PREPARE_FAIL, status);
			}
			break;
		}

		case HOST_FW_CMD_HANDLER_ACTION_WRITE:
			host_fw_cmd_handler_set_status (fw, HOST_FW_CMD_STATUS_STORE_DATA);

			status = host_fw->write_image_update (host_fw, context->event_buffer,
				context->buffer_length);
			if (status != 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HOST_FW,
					HOST_LOGGING_WRITE_UPDATE_FAILED, host_fw->get_port_id (host_fw), status);

				status = HOST_FW_CMD_STATUS (HOST_FW_CMD_STATUS_STORE_FAIL, status);
			}
			break;

		default:
			debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_HOST_FW,
				HOST_LOGGING_NOTIFICATION_ERROR, host_fw->get_port_id (host_fw), context->action);

			host_fw_cmd_handler_set_status (fw, HOST_FW_CMD_STATUS_INTERNAL_ERROR);

			unknown_action = true;
			break;
	}

	if (!unknown_action) {
		fw->task->lock (fw->task);
		fw->state->fw_status = status;
		fw->task->unlock (fw->task);
	}
}

/**
 * Notify the updater task that a host firmware update event needs to be processed.
 *
 * @param handler The handler that received the event.
 * @param action Host firmware update action that needs to be performed.
 * @param data Data associated with the update event.  Null if there is no data.
 * @param length Length of the event data.
 *
 * @return 0 if the update task was notified successfully or an error code.
 */
static int host_fw_cmd_handler_submit_event (const struct host_fw_cmd_handler *handler,
	uint32_t action, const uint8_t *data, size_t length)
{
	int status;

	status = event_task_submit_event (handler->task, &handler->base_event, action, data, length,
		HOST_FW_CMD_STATUS_STARTING, &handler->state->fw_status);
	if (status != 0) {
		if (status == EVENT_TASK_BUSY) {
			/* Do not change the update status when the task is busy.  Something is running, which
			 * could be using the update status. */
			status = HOST_FW_CMD_TASK_BUSY;
		}
		else if (status == EVENT_TASK_TOO_MUCH_DATA) {
			/* Do not change the command status, since we don't know that state of the task. */
			return HOST_FW_CMD_TOO_MUCH_DATA;
		}
		else if (status == EVENT_TASK_NO_TASK) {
			handler->state->fw_status = HOST_FW_CMD_STATUS_TASK_NOT_RUNNING;
			status = HOST_FW_CMD_NO_TASK;
		}
		else {
			host_fw_cmd_handler_set_status (handler, HOST_FW_CMD_STATUS_START_FAILURE);
		}
	}

	return status;
}

int host_fw_cmd_handler_prepare_update (const struct host_fw_cmd_interface *update,
	uint32_t total_size, bool prioritize_update, uint8_t ctrl_flag)
{
	const struct host_fw_cmd_handler *handler = TO_DERIVED_TYPE (update,
		const struct host_fw_cmd_handler, base);
	struct host_fw_cmd_prepare_event_context prepare_event_context = {0};

	if (handler == NULL) {
		return HOST_FW_CMD_INVALID_ARGUMENT;
	}

	prepare_event_context.total_size = total_size;
	prepare_event_context.prioritize_update = prioritize_update;
	prepare_event_context.ctrl_flag = ctrl_flag;

	return host_fw_cmd_handler_submit_event (handler, HOST_FW_CMD_HANDLER_ACTION_PREPARE,
		(uint8_t*) &prepare_event_context, sizeof (prepare_event_context));
}

int host_fw_cmd_handler_write_update (const struct host_fw_cmd_interface *update,
	const uint8_t *data, size_t length)
{
	const struct host_fw_cmd_handler *handler = TO_DERIVED_TYPE (update,
		const struct host_fw_cmd_handler, base);

	if ((handler == NULL) || (data == NULL)) {
		return HOST_FW_CMD_INVALID_ARGUMENT;
	}

	return host_fw_cmd_handler_submit_event (handler, HOST_FW_CMD_HANDLER_ACTION_WRITE, data,
		length);
}

int host_fw_cmd_handler_get_status (const struct host_fw_cmd_interface *update)
{
	const struct host_fw_cmd_handler *handler = TO_DERIVED_TYPE (update,
		const struct host_fw_cmd_handler, base);
	int status;

	if (handler == NULL) {
		return HOST_FW_CMD_INVALID_ARGUMENT;
	}

	handler->task->lock (handler->task);
	status = handler->state->fw_status;
	handler->task->unlock (handler->task);

	return status;
}

int host_fw_cmd_handler_get_update_remaining (const struct host_fw_cmd_interface *update)
{
	const struct host_fw_cmd_handler *handler = TO_DERIVED_TYPE (update,
		const struct host_fw_cmd_handler, base);
	uint32_t bytes;

	if (handler == NULL) {
		return HOST_FW_CMD_INVALID_ARGUMENT;
	}

	handler->task->lock (handler->task);
	bytes = handler->host_fw->get_remaining_update_bytes (handler->host_fw);
	handler->task->unlock (handler->task);

	return bytes;
}

/**
 * Initialize a handler for host firmware update commands.
 *
 * @param handler Host FW update handler to initialize.
 * @param state Variable context for the handler.  This must be uninitialized.
 * @param host_fw Host firmware interface for handling the update flows.
 * @param task The task that will be used to execute firmware update operations.
 *
 * @return 0 if the update handler was successfully initialized or an error code.
 */
int host_fw_cmd_handler_init (struct host_fw_cmd_handler *handler,
	struct host_fw_cmd_handler_state *state, const struct host_firmware *host_fw,
	const struct event_task *task)
{
	if ((handler == NULL) || (state == NULL) || (host_fw == NULL) || (task == NULL)) {
		return HOST_FW_CMD_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (struct host_fw_cmd_handler));

	handler->base.prepare_update = host_fw_cmd_handler_prepare_update;
	handler->base.write_update = host_fw_cmd_handler_write_update;
	handler->base.get_status = host_fw_cmd_handler_get_status;
	handler->base.get_update_remaining = host_fw_cmd_handler_get_update_remaining;

	handler->base_event.prepare = NULL;
	handler->base_event.execute = host_fw_cmd_handler_execute;

	handler->state = state;
	handler->host_fw = host_fw;
	handler->task = task;

	return host_fw_cmd_handler_init_state (handler);
}

/**
 * Initialize only the variable state for update handler.  The rest of the handler is
 * assumed to have already been initialized.
 *
 * @param handler The update handler to initialize.
 */
int host_fw_cmd_handler_init_state (const struct host_fw_cmd_handler *handler)
{
	if ((handler == NULL) || (handler->state == NULL) || (handler->host_fw == NULL) ||
		(handler->task == NULL)) {
		return HOST_FW_CMD_INVALID_ARGUMENT;
	}

	memset (handler->state, 0, sizeof (struct host_fw_cmd_handler_state));

	handler->state->fw_status = HOST_FW_CMD_STATUS_NONE_STARTED;

	return 0;
}

/**
 * Release the resources used by update handler.
 *
 * @param task The command task to start.
 *
 * @return 0 if the task was started or an error code.
 */
void host_fw_cmd_handler_release (const struct host_fw_cmd_handler *handler)
{
	UNUSED (handler);
}
