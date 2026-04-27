// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_IPC_HSM_STATIC_H_
#define CMD_INTERFACE_IPC_HSM_STATIC_H_

#include <stdint.h>
#include <string.h>
#include "ipc/cmd_interface_ipc_hsm.h"


/* Internal functions declared to allow for static initialization. */
int cmd_interface_ipc_hsm_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request);


/**
 * Constant initializer for the cmd_interface_ipc_hsm.
 */
#define	CMD_INTERFACE_IPC_HSM_API_INIT { \
		.process_request = cmd_interface_ipc_hsm_process_request, \
	}

/**
 * Static initialization of command handler HSM.
 * There is no validation done on the arguments.
 *
 * @param[in]	dmb_ptr	The HSP DMB device driver instance to be used as MMU and allow access to SoC memory space.
 * @param[in]	key_manager_ptr	A pointer to an implementation of struct RSA key manager object
 * @param[in]	key_ptr A pointer to a buffer to read the key from the flash.
 * @param[in]	key_buf_size_val Size of the key buffer.
 * @param[in]	attestation_ptr A pointer to an instance of struct struct attestation_responder.
 * @param[in]	hash_ptr Hash engine to use for certificate handling.
 */
#define	cmd_interface_ipc_hsm_static_init(dmb_ptr, key_manager_ptr, key_ptr, key_size_val, \
	attestation_ptr, hash_ptr) { \
		.base = CMD_INTERFACE_IPC_HSM_API_INIT, \
		.dmb = dmb_ptr, \
		.key_manager = key_manager_ptr, \
		.key = key_ptr, \
		.key_size = key_size_val, \
		.attestation = attestation_ptr, \
		.hash = hash_ptr, \
}


#endif	/* CMD_INTERFACE_IPC_HSM_STATIC_H_ */
