// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_FIPS_CMVP_STATIC_H_
#define CMD_INTERFACE_MSFT_FIPS_CMVP_STATIC_H_

#include "cmd_interface_msft_fips_cmvp.h"
#include "msft_protocol/cmd_interface_msft_base_static.h"


/* Internal functions declared to allow for static initialization. */
int cmd_interface_msft_fips_cmvp_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request);


/**
 * Constant initializer for the command interface API.
 */
#define	CMD_INTERFACE_MSFT_FIPS_CMVP_API_INIT { \
		.process_request = cmd_interface_msft_fips_cmvp_process_request, \
	}


/**
 * Initializes a static instance of a MSFT FIPS command set handler for CMVP certification testing
 * commands.
 *
 * There is no validation done on the arguments.
 *
 * @param cmvp_ptr The CMVP interface for certification test execution.
 * @param background_ptr The background command handler.
 */
#define	cmd_interface_msft_fips_cmvp_static_init(cmvp_ptr, background_ptr) { \
		.base = CMD_INTERFACE_MSFT_FIPS_CMVP_API_INIT, \
		.cmvp = cmvp_ptr, \
		.background = background_ptr, \
	}


#endif	/* CMD_INTERFACE_MSFT_FIPS_CMVP_STATIC_H_ */
