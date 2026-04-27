// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "bmc_commands.h"
#include "cmd_interface_msft.h"
#include "cmd_interface_msft_bmc.h"
#include "msft_mctp_protocol.h"
#include "common/unused.h"


static int cmd_interface_msft_bmc_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	UNUSED (intf);
	UNUSED (request);

	return CMD_HANDLER_MSFT_UNSUPPORTED_OPERATION;
}

/**
 * Initialize BMC command interface instance
 *
 * @param intf The BMC command interface instance to initialize
 *
 * @return Initialization status, 0 if success or an error code.
 */
int cmd_interface_msft_bmc_init (struct cmd_interface_msft_bmc *intf)
{
	if (intf == NULL) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	memset (intf, 0, sizeof (struct cmd_interface_msft_bmc));

	intf->base.process_request = cmd_interface_msft_bmc_process_request;

	return 0;
}

/**
 * Deinitialize BMC command interface instance
 *
 * @param intf The BMC command interface instance to deinitialize
 */
void cmd_interface_msft_bmc_deinit (struct cmd_interface_msft_bmc *intf)
{
	UNUSED (intf);
}
