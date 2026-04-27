// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <string.h>
#include "cmd_interface_msft.h"
#include "cmd_interface_protocol_msft.h"
#include "msft_base_commands.h"
#include "msft_mctp_protocol.h"
#include "common/buffer_util.h"
#include "common/unused.h"


int cmd_interface_protocol_msft_parse_message (const struct cmd_interface_protocol *protocol,
	struct cmd_interface_msg *message, uint32_t *message_type)
{
	const struct msft_mctp_protocol_header *header;
	const struct cerberus_protocol_header *cerberus_header;

	if ((protocol == NULL) || (message == NULL) || (message_type == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	if (message->payload_length < (sizeof (*cerberus_header) + sizeof (*header))) {
		return CMD_HANDLER_MSFT_PAYLOAD_TOO_SHORT;
	}

	cerberus_header = (const struct cerberus_protocol_header*) message->payload;

	/* TODO:  Remove these header checks, as they are from the MCTP layer. */
	if ((cerberus_header->msg_type != MCTP_BASE_PROTOCOL_MSG_TYPE_VENDOR_DEF) ||
		(cerberus_header->integrity_check == 1) ||
		(buffer_unaligned_read16 (&cerberus_header->pci_vendor_id) !=
		CERBERUS_PROTOCOL_MSFT_PCI_VID)) {
		return CMD_HANDLER_UNSUPPORTED_MSG;
	}

	if ((cerberus_header->reserved1 != 0) || (cerberus_header->reserved2 != 0)) {
		return CMD_HANDLER_RSVD_NOT_ZERO;
	}

	/* This protocol does not support encryption. */
	if (cerberus_header->crypt == 1) {
		return CMD_HANDLER_ENCRYPTION_UNSUPPORTED;
	}

	if ((cerberus_header->rq == 0) ||
		(cerberus_header->command != MSFT_MCTP_PROTOCOL_ESCAPE_SEQ2)) {
		return CMD_HANDLER_MSFT_UNSUPPORTED_MSG;
	}

	message->crypto_timeout = true;
	cmd_interface_msg_remove_protocol_header (message, sizeof (*cerberus_header));

	header = (struct msft_mctp_protocol_header*) message->payload;

	*message_type = header->command_set;

	return 0;
}

int cmd_interface_protocol_msft_handle_request_result (
	const struct cmd_interface_protocol *protocol, int result, uint32_t message_type,
	struct cmd_interface_msg *message)
{
	if ((protocol == NULL) || (message == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	UNUSED (message_type);

	/* It is the responsibility of the individual command set handlers to ensure proper error
	 * handling and reporting.  This layer will ensure proper protocol handling based on the result
	 * from the command set processing.
	 * - If there is a general error, unassociated with any specific completion code, the handler
	 *   can return an error code.  This layer will assign the completion code and generate an
	 *   appropriate status response.
	 * - If there is a common error that maps to a known completion code, the handler can return an
	 *   error code for handling at this layer.
	 * - If there is an error that maps to a specific completion code within the command set, the
	 *   command set handler must build the error response itself and return 0.  This will prevent
	 *   error handling in the other layers from generating an incorrect response. */
	switch (result) {
		case 0:
			/* Successful processing or internal error handling.  Nothing to do. */
			break;

		case CMD_HANDLER_UNKNOWN_MESSAGE_TYPE:
			/* There was no handler assigned to the command set specified in the message. */
			msft_base_build_error_response (message, MSFT_BASE_CC_INVALID_CMD,
				CMD_HANDLER_MSFT_UNKNOWN_COMMAND_SET);
			break;

		case CMD_HANDLER_MSFT_BAD_LENGTH:
		case CMD_HANDLER_MSFT_OUT_OF_RANGE:
			/* Report a malformed command if the handler reports a bad length error or a parameter
			 * out of the acceptable range. */
			msft_base_build_error_response (message, MSFT_BASE_CC_MALFORMED_CMD, result);
			break;

		case CMD_HANDLER_MSFT_INCOMPATIBLE:
			/* Report an unsupported protocol version if the handler reports the request is
			 * incompatible. */
			msft_base_build_error_response (message, MSFT_BASE_CC_UNSUPPORTED_VERSION, result);
			break;

		case CMD_HANDLER_MSFT_UNSUPPORTED_CMD:
			/* Report an unsupported command when not supported by the handler. */
			msft_base_build_error_response (message, MSFT_BASE_CC_UNSUPPORTED_CMD, result);
			break;

		case CMD_HANDLER_MSFT_UNKNOWN_COMMAND:
			/* Report an invalid command when the handler doesn't know the command code. */
			msft_base_build_error_response (message, MSFT_BASE_CC_INVALID_CMD, result);
			break;

		case CMD_HANDLER_MSFT_UNSUPPORTED_INDEX:
			/* Report an unsupported parameter when the handler reports a bad index. */
			msft_base_build_error_response (message, MSFT_BASE_CC_UNSUPPORTED_PARAM, result);
			break;

		default:
			/* Report a generic failure for any other error code. */
			msft_base_build_error_response (message, MSFT_BASE_CC_FAILURE, result);
			break;
	}

	/* TODO:  Generate only the Cerberus header fields without MCTP. */
	msft_mctp_protocol_add_cerberus_header (message);

	/* There will always be a response populated when exiting. */
	return 0;
}

/**
 * Initialize a protocol handler for Microsoft Vendor Defined Protocol (MVDP) messages.
 *
 * @param msft The MVDP handler to initialize.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int cmd_interface_protocol_msft_init (struct cmd_interface_protocol_msft *msft)
{
	if (msft == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	memset (msft, 0, sizeof (struct cmd_interface_protocol_msft));

	msft->base.parse_message = cmd_interface_protocol_msft_parse_message;
	msft->base.handle_request_result = cmd_interface_protocol_msft_handle_request_result;

	return 0;
}

/**
 * Release the resources used by a MVDP message protocol handler.
 *
 * @param msft The MVDP handler to release.
 */
void cmd_interface_protocol_msft_release (const struct cmd_interface_protocol_msft *msft)
{
	UNUSED (msft);
}

/**
 * Add a MVDP message protocol header to the message buffer.
 *
 * @param msft The MVDP message protocol handler.
 * @param message The message descriptor containing the payload that should be encapsulated with a
 * MVDP message header.
 *
 * @return 0 if the MVDP message header was added successfully or an error code.
 */
int cmd_interface_protocol_msft_add_header (const struct cmd_interface_protocol_msft *msft,
	struct cmd_interface_msg *message)
{
	const size_t header_len = sizeof (struct cerberus_protocol_header);

	if ((msft == NULL) || (message == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	if (cmd_interface_msg_get_protocol_length (message) < header_len) {
		return CMD_HANDLER_MSFT_NO_HEADER_SPACE;
	}

	msft_mctp_protocol_add_cerberus_header (message);

	return 0;
}
