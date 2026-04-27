// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_PROTOCOL_MSFT_CMD_SET_STATIC_H_
#define CMD_INTERFACE_PROTOCOL_MSFT_CMD_SET_STATIC_H_

#include "cmd_interface_protocol_msft_cmd_set.h"


/* Internal functions declared to allow for static initialization. */
int cmd_interface_protocol_msft_cmd_set_parse_message (
	const struct cmd_interface_protocol *protocol, struct cmd_interface_msg *message,
	uint32_t *message_type);


/**
 * Constant initializer for the protocol handler API.
 */
#define	CMD_INTERFACE_PROTOCOL_MSFT_CMD_SET_API_INIT { \
		.parse_message = cmd_interface_protocol_msft_cmd_set_parse_message, \
	}


/**
 * Initialize a static protocol handler for Microsoft Vendor Defined Protocol (MVDP) messages in a
 * specific command set.
 */
#define	cmd_interface_protocol_msft_cmd_set_static_init { \
		.base = CMD_INTERFACE_PROTOCOL_MSFT_CMD_SET_API_INIT, \
	}


#endif	/* CMD_INTERFACE_PROTOCOL_MSFT_CMD_SET_STATIC_H_ */
