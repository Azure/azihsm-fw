// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_interface_msft.h"
#include "common/unused.h"
#include "msft_protocol/msft_base_commands.h"
#include "msft_protocol/msft_mctp_protocol.h"


/**
 * Validate that a received message is a valid MSFT MCTP protocol message and process the Cerberus
 * header.
 *
 * @param intf The command interface handling the message.
 * @param message The message that was received.  Upon successful return, the Cerberus header will
 * have been removed from the message payload.
 * @param min_length The minimum length to be a valid MSFT message.
 *
 * @return 0 if the message was successfully processed or an error code.
 */
static int cmd_interface_msft_process_msft_mctp_protocol_message (const struct cmd_interface *intf,
	struct cmd_interface_msg *message, size_t min_length)
{
	uint8_t command_id;
	uint8_t command_set;
	int status;

	status = cmd_interface_process_cerberus_protocol_message (intf, message, &command_id,
		&command_set, true, true);
	if (status != 0) {
		return status;
	}

	if ((command_set != 1) || (command_id != MSFT_MCTP_PROTOCOL_ESCAPE_SEQ2)) {
		return CMD_HANDLER_MSFT_UNSUPPORTED_MSG;
	}

	if (message->length < min_length) {
		return CMD_HANDLER_MSFT_PAYLOAD_TOO_SHORT;
	}

	message->crypto_timeout = true;
	cmd_interface_msg_remove_protocol_header (message, sizeof (struct cerberus_protocol_header));

	return 0;
}

/**
 * Find the command handler to use for processing a received message.
 *
 * @param msft The MSFT command handler to query.
 * @param message The message that will be handled.
 * @param handler Output for the handler that matches the command set identifier.
 *
 * @return 0 if the command set has an available handler or an error code.
 */
static int cmd_interface_msft_find_command_handler (const struct cmd_interface_msft *msft,
	const struct cmd_interface_msg *message, const struct cmd_interface **handler)
{
	struct msft_mctp_protocol_header *header = (struct msft_mctp_protocol_header*) message->payload;
	size_t i;

	for (i = 0; i < msft->entry_count; i++) {
		if (msft->entries[i].set_id == header->command_set) {
			*handler = msft->entries[i].intf;

			return 0;
		}
	}

	return CMD_HANDLER_MSFT_UNKNOWN_COMMAND_SET;
}

int cmd_interface_msft_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	const struct cmd_interface_msft *msft = (const struct cmd_interface_msft*) intf;
	const struct cmd_interface *handler;
	int status;

	status = cmd_interface_msft_process_msft_mctp_protocol_message (intf, request,
		MSFT_MCTP_PROTOCOL_MIN_REQUEST_LEN);
	if (status != 0) {
		return status;
	}

	status = cmd_interface_msft_find_command_handler (msft, request, &handler);
	if (status == 0) {
		/* Command set handlers will be called with the Cerberus protocol header removed from the
		 * payload.
		 *
		 * It is the responsibility of the command set handler to ensure proper error handling and
		 * reporting.
		 * - If there is a general error, unassociated with any specific completion code, the
		 *   handler can return an error code.  This layer will assign the completion code and
		 *   generate an appropriate status response.
		 * - If there is a common error that maps to a known completion code, the handler can return
		 *   an error code for handling at this layer.
		 * - If there is an error that maps to a specific completion code, the command set handler
		 *   must build the error response itself and return 0.  This will prevent error handling
		 *   in the other layers from generating an incorrect response. */
		status = handler->process_request (handler, request);
		switch (status) {
			case 0:
				/* Successful processing or internal error handling.  Nothing to do. */
				break;

			case CMD_HANDLER_MSFT_BAD_LENGTH:
			case CMD_HANDLER_MSFT_OUT_OF_RANGE:
				/* Report a malformed command if the handler reports a bad length error or a
				 * parameter out of the acceptable range. */
				msft_base_build_error_response (request, MSFT_BASE_CC_MALFORMED_CMD, status);
				break;

			case CMD_HANDLER_MSFT_INCOMPATIBLE:
				/* Report an unsupported protocol version if the handler reports the request is
				 * incompatible. */
				msft_base_build_error_response (request, MSFT_BASE_CC_UNSUPPORTED_VERSION, status);
				break;

			case CMD_HANDLER_MSFT_UNSUPPORTED_CMD:
				/* Report an unsupported command when not supported by the handler. */
				msft_base_build_error_response (request, MSFT_BASE_CC_UNSUPPORTED_CMD, status);
				break;

			case CMD_HANDLER_MSFT_UNKNOWN_COMMAND:
				/* Report an invalid command when the handler doesn't know the command code. */
				msft_base_build_error_response (request, MSFT_BASE_CC_INVALID_CMD, status);
				break;

			case CMD_HANDLER_MSFT_UNSUPPORTED_INDEX:
				/* Report an unsupported parameter when the handler reports a bad index. */
				msft_base_build_error_response (request, MSFT_BASE_CC_UNSUPPORTED_PARAM, status);
				break;

			default:
				/* Report a generic failure for any other error code. */
				msft_base_build_error_response (request, MSFT_BASE_CC_FAILURE, status);
				break;
		}
	}
	else {
		msft_base_build_error_response (request, MSFT_BASE_CC_INVALID_CMD, status);
	}

	msft_mctp_protocol_add_cerberus_header (request);

	return 0;
}

/**
 * Initializes a command set list entry for a MSFT command interface.
 *
 * @param entry The list entry to initialize.
 * @param intf The command handler instance to associate to the entry.
 * @param set_id The command set identifier to associate with this entry.
 *
 * @return 0 if successful, else an error code.
 */
int cmd_interface_msft_cmd_set_entry_init (struct cmd_interface_msft_cmd_set_entry *entry,
	struct cmd_interface *intf, uint8_t set_id)
{
	if ((entry == NULL) || (intf == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	memset (entry, 0, sizeof (*entry));

	entry->intf = intf;
	entry->set_id = set_id;

	return 0;
}

/**
 * Initialize MSFT command interface instance
 *
 * @param intf The MSFT command interface instance to initialize
 * @param entries An array of supported command sets.  There must be at least one command set
 * supported.
 * @param entry_count The number of supported command sets.
 *
 * @return Initialization status, 0 if success or an error code.
 */
int cmd_interface_msft_init (struct cmd_interface_msft *intf,
	const struct cmd_interface_msft_cmd_set_entry *entries, size_t entry_count)
{
	if ((intf == NULL) || (entries == NULL) || (entry_count == 0)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	memset (intf, 0, sizeof (struct cmd_interface_msft));

	intf->entries = entries;
	intf->entry_count = entry_count;

	intf->base.process_request = cmd_interface_msft_process_request;

	return 0;
}

/**
 * Release a MSFT command interface instance.
 *
 * @param intf The MSFT command interface instance to release.
 */
void cmd_interface_msft_release (const struct cmd_interface_msft *intf)
{
	UNUSED (intf);
}

/**
 * Gets the number of command sets that this instance can dispatch to.
 *
 * @param intf The MSFT command interface instance.
 *
 * @return The number of child command sets or 0 if intf is NULL.
 */
size_t cmd_interface_msft_get_cmd_set_count (const struct cmd_interface_msft *intf)
{
	if (intf == NULL) {
		return 0;
	}

	return intf->entry_count;
}
