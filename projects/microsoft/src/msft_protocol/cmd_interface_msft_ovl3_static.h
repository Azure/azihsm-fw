// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_OVL3_STATIC_H_
#define CMD_INTERFACE_MSFT_OVL3_STATIC_H_

#include "cmd_interface_msft_ovl3.h"
#include "msft_protocol/cmd_interface_msft_base_static.h"


/* Internal functions declared to allow for static initialization. */
int cmd_interface_msft_ovl3_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request);

/**
 * Constant initializer for the command interface API.
 */
#define	CMD_INTERFACE_MSFT_OVL3_API_INIT { \
		.process_request = cmd_interface_msft_ovl3_process_request, \
		.session = NULL, \
	}

/**
 * Initializes a static instance of a MSFT OVL3 command set handler.
 *
 * There is no validation done on the arguments.
 *
 */
#define	cmd_interface_msft_ovl3_static_init(background) { \
		.base = CMD_INTERFACE_MSFT_OVL3_API_INIT, \
		.omc_bgnd = background, \
	}


#endif	/* CMD_INTERFACE_MSFT_OVL3_STATIC_H_ */
