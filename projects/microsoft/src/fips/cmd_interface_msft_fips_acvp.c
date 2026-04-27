// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_interface_msft_fips_acvp.h"
#include "common/unused.h"
#include "msft_protocol/cmd_interface_msft_base.h"


int cmd_interface_msft_fips_acvp_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	const struct cmd_interface_msft_fips_acvp *msft =
		(const struct cmd_interface_msft_fips_acvp*) intf;
	const struct msft_mctp_protocol_header *header;
	int status = 0;

	if ((msft == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	/* There is no need to check the header length or validate contents in this handler.  That would
	 * have been already done by an earlier layer of the stack before passing the message here. */
	header = (const struct msft_mctp_protocol_header*) request->payload;

	switch (header->command) {
		case FIPS_CMD_INIT_ACVP_TEST:
			status = fips_init_acvp_test (msft->acvp_proto, request);
			break;

		case FIPS_CMD_ACVP_TEST:
			status = fips_acvp_test (msft->acvp_proto, request);
			break;

		case FIPS_CMD_GET_ACVP_TEST_RESULTS:
			status = fips_get_acvp_test_results (msft->acvp_proto, request);
			break;

		default:
			status = CMD_HANDLER_MSFT_UNKNOWN_COMMAND;
	}

	return status;
}

/**
 * Initialize a command handler for ACVP commands in FIPS command set requests.
 *
 * @param intf The command handler to initialize.
 * @param acvp_proto The ACVP Proto interface for ACVP test execution.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int cmd_interface_msft_fips_acvp_init (struct cmd_interface_msft_fips_acvp *intf,
	const struct acvp_proto_interface *acvp_proto)
{
	if ((intf == NULL) || (acvp_proto == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	memset (intf, 0, sizeof (struct cmd_interface_msft_fips_acvp));

	intf->base.process_request = cmd_interface_msft_fips_acvp_process_request;

	intf->acvp_proto = acvp_proto;

	return 0;
}

/**
 * Release the resources used by a MSFT FIPS command set handler for ACVP commands.
 *
 * @param intf The command handler to release.
 */
void cmd_interface_msft_fips_acvp_release (const struct cmd_interface_msft_fips_acvp *intf)
{
	UNUSED (intf);
}
