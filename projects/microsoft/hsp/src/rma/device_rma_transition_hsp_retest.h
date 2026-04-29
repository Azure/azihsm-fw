// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef DEVICE_RMA_TRANSITION_HSP_RETEST_H_
#define DEVICE_RMA_TRANSITION_HSP_RETEST_H_

#include "common/sram_util.h"
#include "drivers/ccs_ksu_interface.h"
#include "drivers/fuse_controller_interface.h"
#include "drivers/hsp_dmb.h"
#include "rma/device_rma_transition.h"


/**
 * Device RMA handler for HSP that will move the device into the RETEST security state.
 */
struct device_rma_transition_hsp_retest {
	struct device_rma_transition base;				/**< Base RMA API instance. */
	const struct fuse_controller_interface *fuses;	/**< Fuse controller for the device. */
	const struct ccs_ksu_interface *ccs;			/**< CCS driver for the device. */
	const struct hsp_dmb *dmb;						/**< DMB driver for the device. */
	const struct soc_sram_block *sram;				/**< Blocks of SoC SRAM to clear during transition. */
	size_t sram_count;								/**< Number of SoC SRAM blocks. */
};


int device_rma_transition_hsp_retest_init (struct device_rma_transition_hsp_retest *hsp,
	const struct fuse_controller_interface *fuses, const struct ccs_ksu_interface *ccs);
int device_rma_transition_hsp_retest_init_erase_sram (struct device_rma_transition_hsp_retest *hsp,
	const struct fuse_controller_interface *fuses, const struct ccs_ksu_interface *ccs,
	const struct hsp_dmb *dmb, const struct soc_sram_block *sram, size_t count);
void device_rma_transition_hsp_retest_release (const struct device_rma_transition_hsp_retest *hsp);


#endif	/* DEVICE_RMA_TRANSITION_HSP_RETEST_H_ */
