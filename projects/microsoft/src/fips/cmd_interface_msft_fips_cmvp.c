// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_interface_msft_fips_cmvp.h"
#include "common/unused.h"
#include "msft_protocol/cmd_interface_msft_base.h"


int cmd_interface_msft_fips_cmvp_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	const struct cmd_interface_msft_fips_cmvp *msft =
		(const struct cmd_interface_msft_fips_cmvp*) intf;
	const struct msft_mctp_protocol_header *header;
	int status;

	if ((msft == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	/* There is no need to check the header length or validate contents in this handler.  That would
	 * have been already done by an earlier layer of the stack before passing the message here. */
	header = (const struct msft_mctp_protocol_header*) request->payload;

	switch (header->command) {
		case FIPS_CMD_CMVP_TEST_CASE:
			status = fips_cmvp_test_case (msft->cmvp, msft->background, request);
			break;

		default:
			status = CMD_HANDLER_MSFT_UNKNOWN_COMMAND;
	}

	return status;
}

/**
 * Initialize a MSFT FIPS command set handler for CMVP certification testing commands.
 *
 * @param intf The command handler to initialize.
 * @param cmvp The CMVP interface for certification test execution.
 * @param background The background command handler.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int cmd_interface_msft_fips_cmvp_init (struct cmd_interface_msft_fips_cmvp *intf,
	const struct cmvp_test_interface *cmvp, const struct cmd_background *background)
{
	if ((intf == NULL) || (cmvp == NULL) || (background == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	memset (intf, 0, sizeof (*intf));

	intf->base.process_request = cmd_interface_msft_fips_cmvp_process_request;

	intf->cmvp = cmvp;
	intf->background = background;

	return 0;
}

/**
 * Release the resources used by a MSFT FIPS command set handler for CMVP certification testing
 * commands.
 *
 * @param intf The command handler to release.
 */
void cmd_interface_msft_fips_cmvp_release (const struct cmd_interface_msft_fips_cmvp *intf)
{
	UNUSED (intf);
}
