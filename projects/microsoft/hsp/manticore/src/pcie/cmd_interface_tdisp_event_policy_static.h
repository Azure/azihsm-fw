// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef CMD_INTERFACE_TDISP_EVENT_POLICY_STATIC_H_
#define CMD_INTERFACE_TDISP_EVENT_POLICY_STATIC_H_

#include "cmd_interface_tdisp_event_policy.h"


/* Internal function declared to allow for static initialization. */
int cmd_interface_tdisp_event_policy_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request);

void cmd_interface_tdisp_event_policy_on_close_session (
	const struct spdm_protocol_session_observer *spdm_observer, uint32_t *session_id);

void cmd_interface_tdisp_event_policy_on_set_stop (
	const struct ide_driver_observer *ide_observer, struct ide_driver_observer_key_set *key_set);

void cmd_interface_tdisp_event_policy_on_stop_interface (
	const struct tdisp_driver_observer *tdisp_observer, uint32_t *function_index);

void cmd_interface_tdisp_event_policy_on_soft_reset (
	const struct host_processor_observer *host_observer);


/**
 * Constant initializer for the TDISP event policy interface.
 */
#define	CMD_INTERFACE_TDISP_EVENT_POLICY_API_INIT	{ \
		.process_request = cmd_interface_tdisp_event_policy_process_request, \
	}

/**
 * Constant initializer for the SPDM protocol session observer.
 */
#define CMD_INTERFACE_TDISP_EVENT_POLICY_SPDM_PROTOCOL_SESSION_OBSERVER_INIT { \
		.on_new_session = NULL, \
		.on_close_session = cmd_interface_tdisp_event_policy_on_close_session, \
	}

/**
 * Constant initializer for the IDE driver observer.
 */
#define CMD_INTERFACE_TDISP_EVENT_POLICY_IDE_DRIVER_OBSERVER_INIT { \
		.on_set_stop = cmd_interface_tdisp_event_policy_on_set_stop, \
	}

/**
 * Constant initializer for the TDISP driver observer.
 */
#define CMD_INTERFACE_TDISP_EVENT_POLICY_TDISP_DRIVER_OBSERVER_INIT { \
		.on_start_interface = NULL, \
		.on_stop_interface = cmd_interface_tdisp_event_policy_on_stop_interface, \
	}

/**
 * Constant initializer for the host processor observer.
 */
#define CMD_INTERFACE_TDISP_EVENT_POLICY_HOST_PROCESSOR_OBSERVER_INIT { \
		.on_soft_reset = cmd_interface_tdisp_event_policy_on_soft_reset, \
		.on_active_mode = NULL, \
		.on_bypass_mode = NULL, \
		.on_recovery = NULL, \
	}

/**
 * TDISP event policy static initialization.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr TDISP event policy state pointer.
 * @param ipc_to_admin_ptr IPC Admin channel pointer.
 * @param ide_ptr IDE driver interface pointer.
 * @param tdisp_ptr TDISP driver interface pointer.
 * @param ipc_timeout_ms_arg The timeout to use per function when sending IPC messages to Admin,
 * in milliseconds.
 */
#define	cmd_interface_tdisp_event_policy_static_init(state_ptr, ipc_to_admin_ptr, ide_ptr, tdisp_ptr, \
	ipc_timeout_per_fn_ms_arg) { \
		.base = CMD_INTERFACE_TDISP_EVENT_POLICY_API_INIT, \
		.state = state_ptr, \
		.ipc_to_admin = ipc_to_admin_ptr, \
		.ide = ide_ptr, \
		.tdisp = tdisp_ptr, \
		.ipc_timeout_per_fn = ipc_timeout_per_fn_ms_arg, \
		.spdm_observer = CMD_INTERFACE_TDISP_EVENT_POLICY_SPDM_PROTOCOL_SESSION_OBSERVER_INIT, \
		.ide_observer = CMD_INTERFACE_TDISP_EVENT_POLICY_IDE_DRIVER_OBSERVER_INIT, \
		.tdisp_observer = CMD_INTERFACE_TDISP_EVENT_POLICY_TDISP_DRIVER_OBSERVER_INIT, \
		.host_observer = CMD_INTERFACE_TDISP_EVENT_POLICY_HOST_PROCESSOR_OBSERVER_INIT \
	}


#endif	/* CMD_INTERFACE_TDISP_EVENT_POLICY_STATIC_H_ */
