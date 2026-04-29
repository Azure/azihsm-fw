// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_FIPS_ACVP_STATIC_H_
#define CMD_INTERFACE_MSFT_FIPS_ACVP_STATIC_H_

#include "cmd_interface_msft_fips_acvp.h"
#include "msft_protocol/cmd_interface_msft_base_static.h"


/* Internal functions declared to allow for static initialization. */
int cmd_interface_msft_fips_acvp_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request);

/**
 * Constant initializer for the command interface API.
 */
#define	CMD_INTERFACE_MSFT_FIPS_ACVP_API_INIT { \
		.process_request = cmd_interface_msft_fips_acvp_process_request, \
	}


/**
 * Initializes a static instance of a MSFT FIPS command set handler for ACVP commands.
 *
 * There is no validation done on the arguments.
 *
 * @param acvp_ptr The ACVP Proto interface for ACVP test execution.
 */
#define	cmd_interface_msft_fips_acvp_static_init(acvp_ptr) { \
		.base = CMD_INTERFACE_MSFT_FIPS_ACVP_API_INIT, \
		.acvp_proto = acvp_ptr, \
	}


#endif	/* CMD_INTERFACE_MSFT_FIPS_ACVP_STATIC_H_ */
