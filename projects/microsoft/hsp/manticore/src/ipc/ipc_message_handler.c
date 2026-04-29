// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_interface/cmd_interface.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "ipc/ipc_channel.h"
#include "ipc/ipc_message.h"
#include "ipc/ipc_message_handler.h"
#include "logging/manticore_logging.h"


void ipc_message_handler_prepare (const struct periodic_task_handler *handler)
{
	const struct ipc_message_handler *ipc_message_handler = TO_DERIVED_TYPE (handler,
		const struct ipc_message_handler, base);

	/* Enable the IPC receive Interrupt */
	ipc_message_handler->channel->enable (ipc_message_handler->channel);
}

const platform_clock* ipc_message_handler_get_next_execution (
	const struct periodic_task_handler *handler)
{
	UNUSED (handler);

	/* Do not wait to call execute again. */
	return NULL;
}

void ipc_message_handler_execute (const struct periodic_task_handler *handler)
{
	struct ipc_message message = {0};
	struct cmd_interface_msg cmd_message = {0};
	const struct ipc_message_handler *ipc_message_handler = TO_DERIVED_TYPE (handler,
		const struct ipc_message_handler, base);
	int status = 0;

	/* Wait on the receive message on the IPC Channel */
	status = ipc_message_handler->channel->receive (ipc_message_handler->channel, &message, 0);
	if (status == 0) {
		/* Pass the data in cmd_interface_msg */
		cmd_message.data = (uint8_t*) &message;
		cmd_message.length = IPC_MESSAGE_BUFFER_SIZE;
		cmd_message.max_response = IPC_MESSAGE_BUFFER_SIZE;
		cmd_message.payload = (uint8_t*) &message.data;
		cmd_message.payload_length = IPC_MESSAGE_PAYLOAD_SIZE;

		/* Call the Process request for IPC message received */
		status = ipc_message_handler->cmd->process_request (ipc_message_handler->cmd, &cmd_message);
		if (status != 0) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
				MANTICORE_LOGGING_IPC_MESSAGE_HANDLER, status, 0);

			/* If valid response could not be created, create one with a generic IPC error status */
			message.header.response = 1;
			message.header.status = IPC_MESSAGE_FAILED_RESPONSE_STATUS;
		}

		/* Send response message on the IPC */
		ipc_message_handler->channel->send (ipc_message_handler->channel, &message);
	}
}

/**
 * Initialize an instance of IPC Message handler object
 *
 * @param ipc_message_handler A pointer to an initialized IPC message handler object
 * @param config A pointer to an implementation of struct cmd_interface object
 * @param ipc_channel A pointer to an implementation of stuct ipc_channel object
 *
 * @return 0 if completed successfully or an error code.
 */
int ipc_message_handler_init (struct ipc_message_handler *ipc_message_handler,
	const struct cmd_interface *ipc_cmd_interface, const struct ipc_channel *ipc_channel)
{
	if ((ipc_message_handler == NULL) || (ipc_cmd_interface == NULL) || (ipc_channel == NULL)) {
		return IPC_MESSAGE_HANDLER_INVALID_ARGUMENT;
	}

	memset (ipc_message_handler, 0, sizeof (*ipc_message_handler));

	ipc_message_handler->base.prepare = ipc_message_handler_prepare;
	ipc_message_handler->base.get_next_execution = ipc_message_handler_get_next_execution;
	ipc_message_handler->base.execute = ipc_message_handler_execute;

	ipc_message_handler->cmd = ipc_cmd_interface;
	ipc_message_handler->channel = ipc_channel;

	return 0;
}

/**
 * Release a previously initialized IPC message handler and free any associated resoruces.
 * Diasble the IPC channel.
 *
 * @param ipc_message_handler A pointer to un-initialized IPC message handler object
 *
 * @return 0 if completed successfully or an error code.
 */
int ipc_message_handler_release (const struct ipc_message_handler *ipc_message_handler)
{
	if ((ipc_message_handler == NULL) || (ipc_message_handler->channel == NULL)) {
		return IPC_MESSAGE_HANDLER_INVALID_ARGUMENT;
	}

	/* Disable the IPC receive Interrupt */
	ipc_message_handler->channel->disable (ipc_message_handler->channel);

	return 0;
}
