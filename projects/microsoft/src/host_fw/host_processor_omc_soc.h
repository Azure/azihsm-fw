// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HOST_PROCESSOR_OMC_SOC_H_
#define HOST_PROCESSOR_OMC_SOC_H_

#include "overlake_board_id.h"
#include "host_fw/host_control.h"
#include "host_fw/host_processor.h"
#include "host_fw/omc_flash_manager.h"


/**
 * Handler for OMC SoC events to enforce protection on all firmware images.
 */
struct host_processor_omc_soc {
	struct host_processor base;				/**< Base host management instance. */
	const struct host_control *control;		/**< The interface for hardware control of the host. */
	struct omc_flash_manager *flash;		/**< The manager for SoC flash. */
	struct host_processor *boot;			/**< Protection for the boot firmware. */
	enum overlake_board_type board_type;	/**< The type of OMC board. */

	/**
	 * Take ownership of the SoC flash and validate the contents.
	 *
	 * @param host The SoC handler to execute.
	 * @param hash The hash engine to use for firmware validation.
	 * @param rsa The RSA engine to use for signature verification.
	 *
	 * @return 0 if all operations were completed successfully or an error code.
	 */
	int (*soc_boot_complete) (const struct host_processor_omc_soc *host,
		const struct hash_engine *hash, const struct rsa_engine *rsa);
};


int host_processor_omc_soc_init (struct host_processor_omc_soc *host,
	struct host_processor_state *state, const struct host_control *control,
	struct omc_flash_manager *flash, struct host_processor *boot_fw,
	enum overlake_board_id board_id);
void host_processor_omc_soc_release (const struct host_processor_omc_soc *host);

int host_processor_omc_soc_set_soc_flash_access (const struct host_processor_omc_soc *host);


#endif	/* HOST_PROCESSOR_OMC_SOC_H_ */
