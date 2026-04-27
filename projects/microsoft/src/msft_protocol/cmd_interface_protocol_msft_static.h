// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_PROTOCOL_MSFT_STATIC_H_
#define CMD_INTERFACE_PROTOCOL_MSFT_STATIC_H_

#include "cmd_interface_protocol_msft.h"


/* Internal functions declared to allow for static initialization. */
int cmd_interface_protocol_msft_parse_message (const struct cmd_interface_protocol *protocol,
	struct cmd_interface_msg *message, uint32_t *message_type);
int cmd_interface_protocol_msft_handle_request_result (
	const struct cmd_interface_protocol *protocol, int result, uint32_t message_type,
	struct cmd_interface_msg *message);


/**
 * Constant initializer for the protocol handler API.
 */
#define	CMD_INTERFACE_PROTOCOL_MSFT_API_INIT { \
		.parse_message = cmd_interface_protocol_msft_parse_message, \
		.handle_request_result = cmd_interface_protocol_msft_handle_request_result, \
	}


/**
 * Initialize a static protocol handler for Microsoft Vendor Defined Protocol (MVDP) messages.
 */
#define	cmd_interface_protocol_msft_static_init { \
		.base = CMD_INTERFACE_PROTOCOL_MSFT_API_INIT, \
	}


#endif	/* CMD_INTERFACE_PROTOCOL_MSFT_STATIC_H_ */
