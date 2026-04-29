// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_interface_msft_manticore.h"
#include "manticore_commands.h"
#include "common/unused.h"
#include "msft_protocol/cmd_interface_msft_base.h"


int cmd_interface_msft_manticore_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	const struct cmd_interface_msft_manticore *msft =
		(const struct cmd_interface_msft_manticore*) intf;
	const struct msft_mctp_protocol_header *header;
	int status;

	if ((msft == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	/* There is no need to check the header length or validate contents in this handler.  That would
	 * have been already done by an earlier layer of the stack before passing the message here. */
	header = (const struct msft_mctp_protocol_header*) request->payload;

	switch (header->command) {
		case MSFT_PROTOCOL_MANTICORE_SET_DRAIN_TIME:
			status = manticore_set_drain_time (request, msft->shutdown_ctrl);
			break;

		case MSFT_PROTOCOL_MANTICORE_GET_DRAIN_TIME:
			status = manticore_get_drain_time (request, msft->shutdown_ctrl);
			break;

		default:
			status = CMD_HANDLER_MSFT_UNKNOWN_COMMAND;
	}

	return status;
}

/**
 * Initialize a command handler for MSFT Manticore command set requests.
 *
 * @param intf The command handler to initialize.
 * @param shutdown_ctrl The handler to shutdown gracefully.
 * @return 0 if the handler was initialized successfully or an error code.
 */
int cmd_interface_msft_manticore_init (struct cmd_interface_msft_manticore *intf,
	const struct graceful_shutdown_control *shutdown_ctrl)
{
	if ((intf == NULL) || (shutdown_ctrl == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	memset (intf, 0, sizeof (struct cmd_interface_msft_manticore));

	intf->base.process_request = cmd_interface_msft_manticore_process_request;

	intf->shutdown_ctrl = shutdown_ctrl;

	return 0;
}

/**
 * Release the resources used by a MSFT Manticore command set handler.
 *
 * @param intf The command handler to release.
 */
void cmd_interface_msft_manticore_release (const struct cmd_interface_msft_manticore *intf)
{
	UNUSED (intf);
}
