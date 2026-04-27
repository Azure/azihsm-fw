// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANIFEST_CMD_HANDLER_OMC_STATIC_H_
#define MANIFEST_CMD_HANDLER_OMC_STATIC_H_

#include "manifest_cmd_handler_omc.h"
#include "manifest/manifest_cmd_handler.h"
#include "manifest/manifest_cmd_handler_static.h"
#include "manifest/manifest_manager_null_static.h"


/* Internal functions declared to allow for static initialization. */
int manifest_cmd_handler_omc_activation (const struct manifest_cmd_handler *handler, bool *reset);
int manifest_cmd_handler_omc_prepare_manifest (const struct manifest_cmd_interface *cmd,
	uint32_t manifest_size);
int manifest_cmd_handler_omc_store_manifest (const struct manifest_cmd_interface *cmd,
	const uint8_t *data, size_t length);
int manifest_cmd_handler_finish_manifest (const struct manifest_cmd_interface *cmd, bool activate);
int manifest_cmd_handler_get_status (const struct manifest_cmd_interface *cmd);


/* Initialize the OMC manifest cmd interface APIs to support static initialization. */
#define MANIFEST_CMD_HANDLER_OMC_COMMAND_API_INIT { \
		.prepare_manifest = manifest_cmd_handler_omc_prepare_manifest, \
		.store_manifest = manifest_cmd_handler_omc_store_manifest, \
		.finish_manifest = manifest_cmd_handler_finish_manifest, \
		.get_status = manifest_cmd_handler_get_status \
	}

/* Iniitialze OMC manifest cmd handler APIs to support static initialization. */
#define	manifest_cmd_handler_omc_api_init(state_ptr, manifest_ptr, task_ptr)	{ \
		.base_cmd = MANIFEST_CMD_HANDLER_OMC_COMMAND_API_INIT, \
		.base_event = MANIFEST_CMD_HANDLER_EVENT_API_INIT, \
		.state = state_ptr, \
		.manifest = manifest_ptr, \
		.task = task_ptr, \
		.activation =  manifest_cmd_handler_omc_activation \
	}

/**
 * Initialize a static instance of a omc manifest handler.  This does not initialize the handler state.
 * This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the manifest handler.
 * @param manifest_ptr The manifest manager to use during command processing.
 * @param task_ptr The task that will be used to execute manifest operations.
 * @param flash_ptr The flash manager instance for the overlake flash.
 */
#define	manifest_cmd_handler_omc_static_init(state_ptr, manifest_ptr, task_ptr, flash_ptr, fw_ptr)	{ \
		.base = manifest_cmd_handler_omc_api_init(state_ptr, manifest_ptr, task_ptr), \
		.flash_mgr = flash_ptr, \
		.fw = fw_ptr \
	}


#endif	/* MANIFEST_CMD_HANDLER_OMC_STATIC_H_ */
