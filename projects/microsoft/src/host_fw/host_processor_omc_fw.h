// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HOST_PROCESSOR_OMC_FW_H_
#define HOST_PROCESSOR_OMC_FW_H_

#include "host_fw/host_processor.h"
#include "host_fw/host_state_manager.h"
#include "host_fw/omc_flash_manager.h"
#include "manifest/pfm/pfm_manager.h"


/**
 * Manage protection for a single firmware image on SoC flash.  This will only verify and manage
 * firmware images.  SoC control must be managed externally.
 */
struct host_processor_omc_fw {
	struct host_processor base;						/**< Base host management instance. */
	struct omc_soc_firmware *flash;					/**< The manager for the firmware image on flash. */
	const struct host_state_manager *host_state;	/**< State information for the host processor. */
	const struct pfm_manager *pfm;					/**< The manager for host processor PFMs. */
};


int host_processor_omc_fw_init (struct host_processor_omc_fw *host,
	struct host_processor_state *state, struct omc_soc_firmware *flash,
	const struct host_state_manager *host_state, const struct pfm_manager *pfm);
void host_processor_omc_fw_release (const struct host_processor_omc_fw *host);


#endif	/* HOST_PROCESSOR_OMC_FW_H_ */
