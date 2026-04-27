// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_MANTICORE_H_
#define CMD_INTERFACE_MSFT_MANTICORE_H_

#include "cmd_interface/cmd_interface.h"
#include "firmware/graceful_shutdown.h"


/**
 * Handler for MSFT Manticore command set requests.
 */
struct cmd_interface_msft_manticore {
	struct cmd_interface base;								/**< Base command handler API instance. */
	const struct graceful_shutdown_control *shutdown_ctrl;	/**< graceful shutdown control config. */
};


int cmd_interface_msft_manticore_init (struct cmd_interface_msft_manticore *intf,
	const struct graceful_shutdown_control *shutdown_ctrl);
void cmd_interface_msft_manticore_release (const struct cmd_interface_msft_manticore *intf);


#endif	/* CMD_INTERFACE_MSFT_MANTICORE_H_ */
