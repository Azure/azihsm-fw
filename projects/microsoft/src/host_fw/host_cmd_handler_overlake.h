// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HOST_CMD_HANDLER_OVERLAKE_H_
#define HOST_CMD_HANDLER_OVERLAKE_H_

#include "host_fw/host_cmd_interface.h"


/**
 * Handler for host processor commands on Overlake.
 */
struct host_cmd_handler_overlake {
	struct host_cmd_interface base;		/**< The base command handler API. */
	const struct host_processor *host;	/**< The host processor the handler will operate on. */
};


int host_cmd_handler_overlake_init (struct host_cmd_handler_overlake *handler,
	const struct host_processor *host);
void host_cmd_handler_overlake_release (const struct host_cmd_handler_overlake *handler);


#endif	/* HOST_CMD_HANDLER_OVERLAKE_H_ */
