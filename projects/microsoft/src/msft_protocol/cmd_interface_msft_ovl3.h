// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_OVL3_H_
#define CMD_INTERFACE_MSFT_OVL3_H_

#include "cmd_interface/cmd_interface.h"
#include "cmd_interface/omc_background.h"


/**
 * Handler for MSFT OVL3 command set requests.
 */
struct cmd_interface_msft_ovl3 {
	struct cmd_interface base;	/**< Base command handler API instance. */
	struct omc_background *omc_bgnd;
};


int cmd_interface_msft_ovl3_init (struct cmd_interface_msft_ovl3 *intf,
	struct omc_background *background);
void cmd_interface_msft_ovl3_release (const struct cmd_interface_msft_ovl3 *intf);


#endif	/* CMD_INTERFACE_MSFT_OVL3_H_ */
