// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_STATIC_H_
#define CMD_INTERFACE_MSFT_STATIC_H_

#include "cmd_interface_msft.h"


/* Internal functions declared to allow for static initialization. */
int cmd_interface_msft_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request);


/**
 * Constant initializer for the command interface API.
 */
#define	CMD_INTERFACE_MSFT_API_INIT { \
		.process_request = cmd_interface_msft_process_request, \
		.session = NULL, \
	}


/**
 * Initializes a static instance of a command set entry for a MSFT command handler.
 *
 * There is no validation done on the arguments.
 *
 * @param intf_ptr A pointer to a command handler instance.
 * @param set_id_arg The command set ID to associate with the entry.
 */
#define	cmd_interface_msft_cmd_set_entry_static_init(intf_ptr, set_id_arg) { \
		.intf = intf_ptr, \
		.set_id = set_id_arg, \
	}

/**
 * Initializes a static instance of a MSFT command handler.
 *
 * There is no validation done on the arguments.
 *
 * @param sets_array An array of entries for supported command sets.
 * @param sets_count The number of supported command sets.
 */
#define	cmd_interface_msft_static_init(sets_array, sets_count) { \
		.base = CMD_INTERFACE_MSFT_API_INIT, \
		.entries = sets_array, \
		.entry_count = sets_count, \
	}


#endif	/* CMD_INTERFACE_MSFT_STATIC_H_ */
