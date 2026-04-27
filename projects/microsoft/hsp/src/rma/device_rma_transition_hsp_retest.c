// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "device_rma_transition_hsp_retest.h"
#include "common/unused.h"


int device_rma_transition_hsp_retest_config_rma (const struct device_rma_transition *rma)
{
	const struct device_rma_transition_hsp_retest *hsp =
		(const struct device_rma_transition_hsp_retest*) rma;
	int status;

	if (hsp == NULL) {
		return DEVICE_RMA_TRANSITION_INVALID_ARGUMENT;
	}

	if (hsp->dmb != NULL) {
		status = sram_erase_soc_memory_blocks (hsp->dmb, hsp->sram, hsp->sram_count);
		if (status != 0) {
			return status;
		}
	}

	status = ccs_ksu_interface_zeroize_ksu (hsp->ccs);
	if (status != 0) {
		return status;
	}

	return hsp->fuses->change_security_state (hsp->fuses, HSP_SECURITY_STATE_RETEST);
}

/**
 * Initialize a HSP RMA handler for moving to the RETEST security state.
 *
 * @param hsp The HSP RMA handler to initialize.
 * @param fuses The fuse controller that manages the security state.
 * @param ccs The CCS that manages KSU keys.
 *
 * @return 0 if the RMA handler was successfully initialized or an error code.
 */
int device_rma_transition_hsp_retest_init (struct device_rma_transition_hsp_retest *hsp,
	const struct fuse_controller_interface *fuses, const struct ccs_ksu_interface *ccs)
{
	if ((hsp == NULL) || (fuses == NULL) || (ccs == NULL)) {
		return DEVICE_RMA_TRANSITION_INVALID_ARGUMENT;
	}

	memset (hsp, 0, sizeof (struct device_rma_transition_hsp_retest));

	hsp->base.config_rma = device_rma_transition_hsp_retest_config_rma;

	hsp->fuses = fuses;
	hsp->ccs = ccs;

	return 0;
}

/**
 * Initialize a HSP RMA handler for moving to the RETEST security state.  Before executing the
 * security state transition, SoC memory locations will be erased.
 *
 * @param hsp The HSP RMA handler to initialize.
 * @param fuses The fuse controller that manages the security state.
 * @param ccs The CCS that manages KSU keys.
 * @param dmb The DMB instance to use for mapping SoC memory addresses.
 * @param sram A list of SoC memory blocks that should be erased.
 * @param count The number of memory blocks to erase.
 *
 * @return 0 if the RMA handler was successfully initialized or an error code.
 */
int device_rma_transition_hsp_retest_init_erase_sram (struct device_rma_transition_hsp_retest *hsp,
	const struct fuse_controller_interface *fuses, const struct ccs_ksu_interface *ccs,
	const struct hsp_dmb *dmb, const struct soc_sram_block *sram, size_t count)
{
	if ((hsp == NULL) || (fuses == NULL) || (ccs == NULL) || (dmb == NULL) || (sram == NULL)) {
		return DEVICE_RMA_TRANSITION_INVALID_ARGUMENT;
	}

	memset (hsp, 0, sizeof (struct device_rma_transition_hsp_retest));

	hsp->base.config_rma = device_rma_transition_hsp_retest_config_rma;

	hsp->fuses = fuses;
	hsp->ccs = ccs;
	hsp->dmb = dmb;
	hsp->sram = sram;
	hsp->sram_count = count;

	return 0;
}

/**
 * Release the resources used by a HSP RMA handler.
 *
 * @param hsp The HSP RMA handler to release.
 */
void device_rma_transition_hsp_retest_release (const struct device_rma_transition_hsp_retest *hsp)
{
	UNUSED (hsp);
}
