// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "manifest_cmd_handler_omc.h"
#include "common/unused.h"
#include "host_fw/host_logging.h"

int manifest_cmd_handler_omc_activation (const struct manifest_cmd_handler *handler, bool *reset)
{
	const struct manifest_cmd_handler_omc *omc = (const struct manifest_cmd_handler_omc*) handler;

	UNUSED (reset);

	if ((omc == NULL) || (omc->flash_mgr == NULL) || (omc->fw == NULL)) {
		return MANIFEST_MANAGER_INVALID_ARGUMENT;
	}

	/* Unlock SoC flash. This needs to be modified after OMC flash manager refactoring. */
	omc->flash_mgr->boot.lock_flash = false;

	debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_HOST_FW,
		HOST_LOGGING_ACTIVE_UPDATE_SUCCESSFUL, omc->fw->get_port_id (omc->fw), 0);

	return omc->flash_mgr->set_flash_for_soc_access (omc->flash_mgr);
}

int manifest_cmd_handler_omc_prepare_manifest (const struct manifest_cmd_interface *cmd,
	uint32_t manifest_size)
{
	UNUSED (cmd);
	UNUSED (manifest_size);

	return MANIFEST_MANAGER_UNSUPPORTED_OP;
}

int manifest_cmd_handler_omc_store_manifest (const struct manifest_cmd_interface *cmd,
	const uint8_t *data, size_t length)
{
	UNUSED (cmd);
	UNUSED (data);
	UNUSED (length);

	return MANIFEST_MANAGER_UNSUPPORTED_OP;
}

/**
 * Initialize a handler for executing PFM commands for OMC.
 *
 * @param handler The PFM handler to initialize.
 * @param state Variable context for the handler.  This must be uninitialized.
 * @param manifest The manifest manager to use during command processing.
 * @param task The task that will be used to execute PFM operations.
 *
 * @return 0 if the task was successfully initialized or an error code.
 */
int manifest_cmd_handler_omc_init (struct manifest_cmd_handler_omc *handler,
	struct manifest_cmd_handler_state *state, const struct manifest_manager *manifest,
	const struct event_task *task, struct omc_flash_manager *flash_mgr, struct host_firmware *fw)
{
	int status;

	if ((handler == NULL) || (flash_mgr == NULL) || (fw == NULL)) {
		return MANIFEST_MANAGER_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (struct manifest_cmd_handler_omc));

	status = manifest_cmd_handler_init (&handler->base, state, manifest, task);
	if (status != 0) {
		return status;
	}

	handler->flash_mgr = flash_mgr;
	handler->fw = fw;

	handler->base.activation = manifest_cmd_handler_omc_activation;
	handler->base.base_cmd.prepare_manifest = manifest_cmd_handler_omc_prepare_manifest;
	handler->base.base_cmd.store_manifest = manifest_cmd_handler_omc_store_manifest;

	return 0;
}

/**
 * Initialize only the variable state for a OMC manifest handler.  The rest of the handler is
 * assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param handler The manifest handler that contains the state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int manifest_cmd_handler_omc_init_state (const struct manifest_cmd_handler_omc *handler)
{
	if ((handler == NULL) || (handler->flash_mgr == NULL)) {
		return MANIFEST_MANAGER_INVALID_ARGUMENT;
	}

	return manifest_cmd_handler_init_state (&handler->base);
}

/**
 * Release the resources used by a OMC manifest handler.
 *
 * @param handler The manifest handler to release.
 */
void manifest_cmd_handler_omc_release (const struct manifest_cmd_handler_omc *handler)
{
	if (handler) {
		manifest_cmd_handler_release (&handler->base);
	}
}
