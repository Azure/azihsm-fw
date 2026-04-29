// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include <string.h>
#include "cmd_interface_msft.h"
#include "msft_mctp_protocol.h"
#include "common/buffer_util.h"


/**
 * Generate the header segment of a MSFT MCTP protocol request.
 *
 * @param header Buffer to fill with MSFT MCTP protocol header.
 * @param command Command ID to assign to the message.
 * @param command_set Command set to assign to the message.
 * @param version Protocol version used to construct the message.
 *
 * @return 0 if the header was successfully populated or an error code.
 */
int msft_mctp_protocol_populate_header (struct msft_mctp_protocol_header *header, uint8_t command,
	uint8_t command_set, uint16_t version)
{
	if (header == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	header->command_set = command_set;
	buffer_unaligned_write16 (&header->protocol_version, version);
	header->command = command;

	return 0;
}

/**
 * Generate the Cerberus protocol header for a MSFT MCTP message.
 *
 * @param header The Cerberus header to populate.
 */
void msft_mctp_protocol_populate_cerberus_header (struct cerberus_protocol_header *header)
{
	if (header != NULL) {
		memset (header, 0, sizeof (struct cerberus_protocol_header));

		header->msg_type = MCTP_BASE_PROTOCOL_MSG_TYPE_VENDOR_DEF;
		buffer_unaligned_write16 (&header->pci_vendor_id, CERBERUS_PROTOCOL_MSFT_PCI_VID);
		header->rq = 1;
		header->command = MSFT_MCTP_PROTOCOL_ESCAPE_SEQ2;
	}
}

/**
 * Add the Cerberus header to wrap a MSFT MCTP command.
 *
 * @param message The message containing the the MSFT message.  The payload must point to the
 * beginning of the MSFT message and have enough headroom in the buffer to add the Cerberus header.
 * If this is null, nothing is done.
 */
void msft_mctp_protocol_add_cerberus_header (struct cmd_interface_msg *message)
{
	struct cerberus_protocol_header *cerberus;

	if (message == NULL) {
		return;
	}

	cmd_interface_msg_add_protocol_header (message, sizeof (struct cerberus_protocol_header));
	cerberus = (struct cerberus_protocol_header*) message->payload;

	msft_mctp_protocol_populate_cerberus_header (cerberus);
}
