// Copyright (c) Microsoft Corporation. All rights reserved.

#include "attestation/attestation.h"
#include "ipc/cmd_interface_ipc_hsm.h"
#include "ipc/ipc_message.h"
#include "logging/manticore_logging.h"


/**
 * Convert the error code into the IPC message response status code.
 *
 * @param opcode The IPC command opcode used.
 * @param error_code Error code value to convert.
 *
 * @return Response status code for IPC request message.
 */
static int cmd_interface_ipc_convert_to_ipc_status (uint32_t opcode, int error_code)
{
	int ipc_error_code;

	if (error_code == 0) {
		return IPC_MESSAGE_STATUS_CODE_SUCCESS;
	}

	/* TODO: Add additional error mapping once IPC status code redesign is complete */
	switch (opcode) {
		case IPC_MESSAGE_OPCODE_GET_CERT_CHAIN_LEN:
		case IPC_MESSAGE_OPCODE_GET_CERT:
			if ((error_code == ATTESTATION_INVALID_CERT_NUM) ||
				(error_code == ATTESTATION_CERT_NOT_AVAILABLE) ||
				(error_code == CMD_INTERFACE_IPC_HSM_GET_CERT_CHAIN_LEN_TOO_MANY) ||
				(error_code == CMD_INTERFACE_IPC_HSM_GET_CERT_LEN_BUF_TOO_SMALL)) {
				ipc_error_code = IPC_MESSAGE_STATUS_CODE_OPERATION_FAILED;
			}
			else {
				ipc_error_code = IPC_MESSAGE_STATUS_CODE_UNKNOWN_STATUS;
			}
			break;

		default:
			ipc_error_code = IPC_MESSAGE_STATUS_CODE_UNKNOWN_STATUS;
	}

	return ipc_error_code;
}

/**
 * Construct IPC message response and log any error which has occurred.
 *
 * @param message The response message to populate.  If this is null, nothing will be done.
 * @param log_message_id The log entry message identifier.
 * @param req_status The status code encountered during request processing.
 */
void ipc_message_build_response (struct cmd_interface_msg *message, uint8_t log_message_id,
	uint32_t req_status)
{
	struct ipc_message_header *header;

	if (message == NULL) {
		return;
	}

	header = (struct ipc_message_header*) message->data;

	header->response = 1;

	if (req_status == 0) {
		header->status = 0;
	}
	else {
		header->status = cmd_interface_ipc_convert_to_ipc_status (header->opcode, req_status);

		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			log_message_id, header->opcode, req_status);
	}
}
