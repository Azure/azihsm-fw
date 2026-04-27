// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANIFEST_CMD_HANDLER_OMC_H_
#define MANIFEST_CMD_HANDLER_OMC_H_

#include "host_fw/omc_flash_manager.h"
#include "manifest/manifest_cmd_handler.h"


/**
 * A handler for executing manifest related requests in OMC.
 */
struct manifest_cmd_handler_omc {
	struct manifest_cmd_handler base;		/**< Base manifest handler. */
	struct omc_flash_manager *flash_mgr;	/**< The SoC flash manager. */
	struct host_firmware *fw;				/**< The host firmware instance. */
};


int manifest_cmd_handler_omc_init (struct manifest_cmd_handler_omc *handler,
	struct manifest_cmd_handler_state *state, const struct manifest_manager *manifest,
	const struct event_task *task, struct omc_flash_manager *flash_mgr, struct host_firmware *fw);
int manifest_cmd_handler_omc_init_state (const struct manifest_cmd_handler_omc *handler);
void manifest_cmd_handler_omc_release (const struct manifest_cmd_handler_omc *handler);


#endif	/* MANIFEST_CMD_HANDLER_OMC_H_ */
