// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_PROTOCOL_MSFT_H_
#define CMD_INTERFACE_PROTOCOL_MSFT_H_

#include "cmd_interface/cmd_interface.h"


/**
 * Protocol handler for Microsoft Vendor Defined Protocol (MVDP) messages.
 */
struct cmd_interface_protocol_msft {
	struct cmd_interface_protocol base;	/**< Base protocol handling API. */
};


int cmd_interface_protocol_msft_init (struct cmd_interface_protocol_msft *msft);
void cmd_interface_protocol_msft_release (const struct cmd_interface_protocol_msft *msft);

int cmd_interface_protocol_msft_add_header (const struct cmd_interface_protocol_msft *msft,
	struct cmd_interface_msg *message);


#endif	/* CMD_INTERFACE_PROTOCOL_MSFT_H_ */
