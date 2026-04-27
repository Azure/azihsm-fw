// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_interface_msft_base.h"
#include "cmd_interface_msft_rot_log.h"
#include "rot_commands.h"
#include "common/unused.h"


int cmd_interface_msft_rot_log_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	const struct cmd_interface_msft_rot_log *msft = (const struct cmd_interface_msft_rot_log*) intf;
	const struct msft_mctp_protocol_header *header;
	int status;

	if ((msft == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	/* There is no need to check the header length or validate contents in this handler.  That would
	 * have been already done by an earlier layer of the stack before passing the message here. */
	header = (const struct msft_mctp_protocol_header*) request->payload;

	switch (header->command) {
		case ROT_CMD_SEND_LOG_COMMAND:
			status = CMD_HANDLER_MSFT_UNSUPPORTED_CMD;
			break;

		case ROT_CMD_READ_LOG_COMMAND:
			status = rot_read_log_command (msft->log, msft->hash, request);
			break;

		default:
			status = CMD_HANDLER_MSFT_UNKNOWN_COMMAND;
	}

	return status;
}

/**
 * Initialize a command handler for MSFT RoT log command set requests.
 *
 * @param intf The command handler to initialize.
 * @param log The Rot log interface for read log command.
 * @param hash The hashing engine for hash of log data.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int cmd_interface_msft_rot_log_init (struct cmd_interface_msft_rot_log *intf,
	const struct rot_log_interface *log, const struct hash_engine *hash)
{
	if (intf == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	memset (intf, 0, sizeof (struct cmd_interface_msft_rot_log));

	intf->base.process_request = cmd_interface_msft_rot_log_process_request;

	intf->log = log;
	intf->hash = hash;

	return 0;
}

/**
 * Release the resources used by a MSFT RoT command set handler.
 *
 * @param intf The command handler to release.
 */
void cmd_interface_msft_rot_log_release (const struct cmd_interface_msft_rot_log *intf)
{
	UNUSED (intf);
}
