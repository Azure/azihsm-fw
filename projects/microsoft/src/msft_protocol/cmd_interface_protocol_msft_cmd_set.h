// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_PROTOCOL_MSFT_CMD_SET_H_
#define CMD_INTERFACE_PROTOCOL_MSFT_CMD_SET_H_

#include "cmd_interface/cmd_interface.h"


/**
 * Protocol handler for Microsoft Vendor Defined Protocol (MVDP) messages in a single command set.
 *
 * This protocol handler provides parsing of the command code to provide routing to an appropriate
 * handler for the message.  This layer only parses out the command code and assumes the overall
 * message confirms to MVDP requirements.  Therefore, it must always be preceeded with a
 * cmd_interface_protocol_msft instance in the overall protocol processing stack.
 */
struct cmd_interface_protocol_msft_cmd_set {
	struct cmd_interface_protocol base;	/**< Base protocol handling API. */
};


int cmd_interface_protocol_msft_cmd_set_init (struct cmd_interface_protocol_msft_cmd_set *msft);
void cmd_interface_protocol_msft_cmd_set_release (
	const struct cmd_interface_protocol_msft_cmd_set *msft);


#endif	/* CMD_INTERFACE_PROTOCOL_MSFT_CMD_SET_H_ */
