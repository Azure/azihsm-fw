// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_interface_msft_fips_on_demand_self_test.h"
#include "common/unused.h"
#include "msft_protocol/cmd_interface_msft_base.h"


int cmd_interface_msft_fips_on_demand_self_test_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	const struct cmd_interface_msft_fips_on_demand_self_test *msft =
		(const struct cmd_interface_msft_fips_on_demand_self_test*) intf;
	const struct msft_mctp_protocol_header *header;
	int status;

	if ((msft == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	/* There is no need to check the header length or validate contents in this handler.  That would
	 * have been already done by an earlier layer of the stack before passing the message here. */
	header = (const struct msft_mctp_protocol_header*) request->payload;

	switch (header->command) {
		case FIPS_CMD_ON_DEMAND_SELF_TEST:
			status = fips_on_demand_self_test (msft->self_test, request);
			break;

		case FIPS_CMD_ON_DEMAND_SELF_TEST_RESULT:
			status = fips_on_demand_self_test_result (msft->self_test, request);
			break;

		default:
			status = CMD_HANDLER_MSFT_UNKNOWN_COMMAND;
	}

	return status;
}

/**
 * Initialize a MSFT FIPS command set handler for on-demand self-test commands.
 *
 * @param intf The command handler to initialize.
 * @param self_test The FIPS self-test handler for executing on-demand self-tests.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int cmd_interface_msft_fips_on_demand_self_test_init (
	struct cmd_interface_msft_fips_on_demand_self_test *intf,
	const struct fips_self_test_interface *self_test)
{
	if ((intf == NULL) || (self_test == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	memset (intf, 0, sizeof (*intf));

	intf->base.process_request = cmd_interface_msft_fips_on_demand_self_test_process_request;

	intf->self_test = self_test;

	return 0;
}

/**
 * Release the resources used by a MSFT FIPS command set handler for CMVP certification testing
 * commands.
 *
 * @param intf The command handler to release.
 */
void cmd_interface_msft_fips_on_demand_self_test_release (
	const struct cmd_interface_msft_fips_on_demand_self_test *intf)
{
	UNUSED (intf);
}
