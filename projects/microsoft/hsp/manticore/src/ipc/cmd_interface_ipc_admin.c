// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common/type_cast.h"
#include "common/unused.h"
#include "ipc/cmd_interface_ipc_admin.h"
#include "ipc/ipc_channel.h"
#include "ipc/ipc_message.h"
#include "logging/manticore_logging.h"


/**
 * Process a DOE message received over the IPC channel.
 *
 * @param cmd_interface_ipc_admin A pointer to command interface that will process the request.
 * @param doe_message_base_address DOE message's base address
 *
 * @return 0 if the request was successfully processed or an error code.
 */
static int cmd_interface_ipc_admin_process_doe_interface_message (
	const struct cmd_interface_ipc_admin *cmd_interface_ipc_admin,
	struct ipc_message_doe_payload *payload)
{
	struct doe_cmd_message *doe_message;
	int status;

	status = cmd_interface_ipc_admin->dmb->map_soc_address (cmd_interface_ipc_admin->dmb,
		(uint64_t) payload->buffer_address, sizeof (struct doe_cmd_message), HSP_DMB_ACCESS_WRITE,
		(void**) &doe_message);
	if (status != 0) {
		return status;
	}

	status = doe_interface_process_message (cmd_interface_ipc_admin->doe, doe_message);

	cmd_interface_ipc_admin->dmb->unmap_soc_address (cmd_interface_ipc_admin->dmb, doe_message);

	return status;
}

/**
 * Process the IPC Message and validate received IPC message.
 *
 * @param cmd_interface_ipc_admin The command interface that will process the request.
 * @param request The request data to process. This will be updated to contain a response, if
 * necessary.
 */
static void cmd_interface_ipc_admin_process_ipc_message (
	const struct cmd_interface_ipc_admin *cmd_interface_ipc_admin,
	struct cmd_interface_msg *request)
{
	struct ipc_message_header *header;
	int status = 0;

	header = (struct ipc_message_header*) request->data;

	/* Validate ipc message opcode */
	switch (header->opcode) {
		case IPC_MESSAGE_OPCODE_DOE: {
			struct ipc_message_doe_payload *payload =
				(struct ipc_message_doe_payload*) request->payload;

			/* Process DoE Message Request */
			status = cmd_interface_ipc_admin_process_doe_interface_message (cmd_interface_ipc_admin,
				payload);
			break;
		}

		case IPC_MESSAGE_OPCODE_TDISP_INTERRUPT:
			/* Forward request to PCIe error policy handler */
			status =
				cmd_interface_ipc_admin->tdisp_event_policy->process_request (
				cmd_interface_ipc_admin->tdisp_event_policy, request);
			break;

		default:
			status = CMD_INTERFACE_IPC_ADMIN_INVALID_IPC_OPCODE;
			break;
	}

	ipc_message_build_response (request, MANTICORE_LOGGING_IPC_ADMIN_INTERFACE, status);
}

int cmd_interface_ipc_admin_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	const struct cmd_interface_ipc_admin *cmd_interface_ipc_admin = TO_DERIVED_TYPE (intf,
		const struct cmd_interface_ipc_admin, base);

	if ((intf == NULL) || (request == NULL)) {
		return CMD_INTERFACE_IPC_ADMIN_INVALID_ARGUMENT;
	}

	cmd_interface_ipc_admin_process_ipc_message (
		(struct cmd_interface_ipc_admin*) cmd_interface_ipc_admin, request);

	return 0;
}

/**
 * Initialize an IPC Admin Command Interface.
 *
 * @param cmd_interface_ipc_admin Pointer to initialized Admin Command Interface object.
 * @param dmb The DMB interface to use for DOE message mapping.
 * @param doe The DOE interface to use for DOE message processing.
 * @param tdisp_event_policy The TDISP event policy command interface instance to use for
 * TDISP-related message processing.
 *
 * @return 0 if the request was successfully processed or an error code.
 */
int cmd_interface_ipc_admin_init (struct cmd_interface_ipc_admin *cmd_interface_ipc_admin,
	const struct hsp_dmb *dmb, const struct doe_interface *doe,
	const struct cmd_interface *tdisp_event_policy)
{
	if ((cmd_interface_ipc_admin == NULL) || (dmb == NULL) || (doe == NULL) ||
		(tdisp_event_policy == NULL)) {
		return IPC_CHANNEL_INVALID_ARGUMENT;
	}

	memset (cmd_interface_ipc_admin, 0, sizeof (struct cmd_interface_ipc_admin));

	cmd_interface_ipc_admin->base.process_request = cmd_interface_ipc_admin_process_request;

	cmd_interface_ipc_admin->dmb = dmb;
	cmd_interface_ipc_admin->doe = doe;
	cmd_interface_ipc_admin->tdisp_event_policy = tdisp_event_policy;

	return 0;
}

/**
 * Release a previously initialized Admin Command Interface and free any associated resoruces
 *
 * @param cmd_interface_ipc_admin Pointer to an un-initialized Admin Command Interface object
 */
void cmd_interface_ipc_admin_release (const struct cmd_interface_ipc_admin *cmd_interface_ipc_admin)
{
	UNUSED (cmd_interface_ipc_admin);
}
