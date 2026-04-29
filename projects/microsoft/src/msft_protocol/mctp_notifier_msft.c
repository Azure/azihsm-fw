// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "msft_protocol_logging.h"
#include "cmd_interface/msg_transport.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "msft_protocol/mctp_notifier_msft.h"


/**
 * Verify MCTP notification response message and fetch completion code.
 *
 * @param resp cmd_interface_msg message descriptor.
 * @param req_mctp_header MCTP header of request message sent.
 * @param resp_mctp_header MCTP header to get response header info.
 *
 * @return 0 if the response parsed successfully or an error code.
 */
static int mctp_notifier_msft_verify_response (struct cmd_interface_msg *resp,
	struct msft_mctp_protocol_header *req_mctp_header,
	struct msft_mctp_protocol_response_header **resp_mctp_header)
{
	struct cerberus_protocol_header *cerberus_header;

	if (resp->payload_length < (sizeof (*cerberus_header) + sizeof (**resp_mctp_header))) {
		return MCTP_NOTIFIER_RESP_PAYLOAD_TOO_SHORT;
	}

	cerberus_header = (struct cerberus_protocol_header*) resp->payload;

	if ((cerberus_header->msg_type != MCTP_BASE_PROTOCOL_MSG_TYPE_VENDOR_DEF) ||
		(cerberus_header->pci_vendor_id != CERBERUS_PROTOCOL_MSFT_PCI_VID) ||
		(cerberus_header->rq != 1) ||
		(cerberus_header->command != MSFT_MCTP_PROTOCOL_ESCAPE_SEQ2)) {
		return MCTP_NOTIFIER_NOTIFICATION_RESP_MISMATCH;
	}

	// Remove Cerberus protocol header
	cmd_interface_msg_remove_protocol_header (resp, sizeof (*cerberus_header));

	*resp_mctp_header = (struct msft_mctp_protocol_response_header*) resp->payload;

	if ((req_mctp_header->command != (*resp_mctp_header)->command) ||
		(req_mctp_header->command_set != (*resp_mctp_header)->command_set) ||
		(req_mctp_header->protocol_version != (*resp_mctp_header)->protocol_version)) {
		return MCTP_NOTIFIER_NOTIFICATION_RESP_MISMATCH;
	}

	return 0;
}

int mctp_notifier_msft_send_notification_request (
	const struct mctp_notifier_interface *notifier,	uint8_t *payload, size_t payload_len,
	uint8_t dest_eid)
{
	const struct mctp_notifier_msft *notify = TO_DERIVED_TYPE (notifier,
		const struct mctp_notifier_msft, base);
	struct cmd_interface_msg request;
	struct msft_mctp_protocol_header *req_mctp_header;
	struct msft_mctp_protocol_response_header *resp_mctp_header;
	int status;

	if ((notify == NULL) || (payload == NULL) || (payload_len == 0)) {
		return MCTP_NOTIFIER_INVALID_ARGUMENT;
	}

	memset (notify->msg_buffer, 0, notify->msg_buffer_len);

	// Create empty request message format
	status = msg_transport_create_empty_request (notify->transport, notify->msg_buffer,
		notify->msg_buffer_len, dest_eid, &request);
	if (status != 0) {
		return status;
	}

	/* TODO: msg_transport handling.
	 * Adjust request payload pointer after transport header and also update payload length */
	cmd_interface_msg_remove_protocol_header (&request, sizeof (struct cerberus_protocol_header));

	req_mctp_header = (struct msft_mctp_protocol_header*) payload;

	// Copy application payload data into request payload
	if (request.payload_length >= payload_len) {
		memcpy (request.payload, payload, payload_len);

		cmd_interface_msg_set_message_payload_length (&request, payload_len);

		/* TODO: Added Cerberus header fields without MCTP.
		 * It should be removed with better msg_transport handling. */
		msft_mctp_protocol_add_cerberus_header (&request);

		// Send notification request message to transport layer
		status = notify->transport->send_request_message (notify->transport, &request,
			notify->resp_timeout_ms, &request);

		if (status != 0) {
			if (!notify->skip_send_fail_log) {
				// Log error code in case of notification failure
				debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, MSFT_LOGGING_COMPONENT_MVDP,
					MSFT_PROTOCOL_LOGGING_NOTIFICATION_FAILED, status,
					((req_mctp_header->command_set << 24) | (req_mctp_header->protocol_version <<
							16) | (req_mctp_header->command << 8) | dest_eid));
			}
		}
		else {
			/* TODO: Response validation should only be part of this mctp_notifier_msft module other
			 *	  handling needs to be moved to core repo
			 *	- core/mctp/mctp_notifier_interface: Defines the abstract interface.
			 *	- core/mctp/mctp_notifier: Implements the interface, contains the registered EID
			 *		array and request buffer. Probably defines an internal abstract API,
			 *		process_response, to be implemented by specific notifiers.
			 *	- microsoft/msft_protocol/mctp_notifier_msft: Provides an implementation for
			 *		process_response to handle response messages for notifications.
			 */

			// Verify response payload and fetch response MSFT MCTP header
			status = mctp_notifier_msft_verify_response (&request, req_mctp_header,
				&resp_mctp_header);

			// Log error code if response not received with success completion code
			if (status != 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, MSFT_LOGGING_COMPONENT_MVDP,
					MSFT_PROTOCOL_LOGGING_NOTIFICATION_FAILED, status,
					((req_mctp_header->command_set << 24) | (req_mctp_header->protocol_version <<
							16) | (req_mctp_header->command << 8) | dest_eid));
			}
			else if (resp_mctp_header->completion_code != 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, MSFT_LOGGING_COMPONENT_MVDP,
					MSFT_PROTOCOL_LOGGING_NOTIFICATION_RESP_ERROR,
					((resp_mctp_header->command_set << 24) | (resp_mctp_header->protocol_version <<
							16) | (resp_mctp_header->command <<
								8) | (resp_mctp_header->completion_code)), dest_eid);
			}
		}
	}
	else {
		status = MCTP_NOTIFIER_PAYLOAD_TOO_LARGE;
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, MSFT_LOGGING_COMPONENT_MVDP,
			MSFT_PROTOCOL_LOGGING_NOTIFICATION_FAILED, status,
			((req_mctp_header->command_set << 24) | (req_mctp_header->protocol_version <<
					16) | (req_mctp_header->command << 8) | dest_eid));
	}

	return 0;
}
