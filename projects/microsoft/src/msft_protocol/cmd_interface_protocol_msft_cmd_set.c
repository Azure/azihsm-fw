// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <string.h>
#include "cmd_interface_msft.h"
#include "cmd_interface_protocol_msft_cmd_set.h"
#include "msft_mctp_protocol.h"
#include "common/unused.h"


int cmd_interface_protocol_msft_cmd_set_parse_message (
	const struct cmd_interface_protocol *protocol, struct cmd_interface_msg *message,
	uint32_t *message_type)
{
	const struct msft_mctp_protocol_header *header;

	if ((protocol == NULL) || (message == NULL) || (message_type == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	header = (struct msft_mctp_protocol_header*) message->payload;

	*message_type = header->command;

	return 0;
}

/**
 * Initialize a protocol handler for Microsoft Vendor Defined Protocol (MVDP) messages in a specific
 * command set.
 *
 * @param msft The MVDP command set handler to initialize.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int cmd_interface_protocol_msft_cmd_set_init (struct cmd_interface_protocol_msft_cmd_set *msft)
{
	if (msft == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	memset (msft, 0, sizeof (*msft));

	msft->base.parse_message = cmd_interface_protocol_msft_cmd_set_parse_message;

	return 0;
}

/**
 * Release the resources used by a MVDP command set protocol handler.
 *
 * @param msft The MVDP command set handler to release.
 */
void cmd_interface_protocol_msft_cmd_set_release (
	const struct cmd_interface_protocol_msft_cmd_set *msft)
{
	UNUSED (msft);
}
