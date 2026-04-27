// Copyright (c) Microsoft Corporacmd_interface_ipc_admintion. All rights reserved.

#ifndef CMD_INTERFACE_IPC_ADMIN_STATIC_H_
#define CMD_INTERFACE_IPC_ADMIN_STATIC_H_

#include <stdint.h>
#include <string.h>
#include "ipc/cmd_interface_ipc_admin.h"


/* Internal functions declared to allow for static initialization. */
int cmd_interface_ipc_admin_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request);


/**
 * Constant initializer for the cmd_interface_ipc_admin.
 */
#define	CMD_INTERFACE_IPC_ADMIN_API_STATIC_INIT { \
		.process_request = cmd_interface_ipc_admin_process_request, \
	}

/**
 * Static initialization of command handler for IPC Admin messages.
 * There is no validation done on the arguments.
 *
 * @param dmb_ptr A pointer to an implementation of struct hsp_dmb
 * @param doe_ptr A pointer to an implementation of struct doe_interface
 * @param tdisp_event_policy_ptr A pointer to an implementation of struct cmd_interface
 */
#define	cmd_interface_ipc_admin_static_init(dmb_ptr, doe_ptr, tdisp_event_policy_ptr) { \
	.base = CMD_INTERFACE_IPC_ADMIN_API_STATIC_INIT, \
	.dmb = dmb_ptr, \
	.doe = doe_ptr, \
	.tdisp_event_policy = tdisp_event_policy_ptr, \
}


#endif	/* CMD_INTERFACE_IPC_ADMIN_H_ */
