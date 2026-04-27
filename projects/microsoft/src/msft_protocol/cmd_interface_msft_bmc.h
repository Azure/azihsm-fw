// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_BMC_H_
#define CMD_INTERFACE_MSFT_BMC_H_

#include <stdint.h>
#include "cmd_interface/cmd_interface.h"
#include "common/observable.h"


/**
 * Command interface for processing responses to BMC protocol requests.  This does not support
 * handling received requests.
 */
struct cmd_interface_msft_bmc {
	struct cmd_interface base;		/**< Base command interface */
	struct observable observable;	/**< Observer manager for the interface. */
};


/* TODO:  Observable needs to support const model in order to support static/const instances. */
int cmd_interface_msft_bmc_init (struct cmd_interface_msft_bmc *intf);
void cmd_interface_msft_bmc_deinit (struct cmd_interface_msft_bmc *intf);


#endif	/* CMD_INTERFACE_MSFT_BMC_H_ */
