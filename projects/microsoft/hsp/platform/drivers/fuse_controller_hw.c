// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include "fmc_regs.h"
#include "drivers/fuse_controller.h"

/**
 * Set the counter that controls the pulse width for fuse programming.
 *
 * @param fuses The fuse controller to update.
 * @param hsp_clk The HSP clock to use for determining the counter value.
 *
 * @return 0 if the counter was updated successfully or an error code.
 */
int fuse_controller_set_program_pulse_counter (const struct fuse_controller *fuses,
	uint32_t hsp_clk)
{
	uint32_t tpgm;

	/* Need to calculate the counter for a 5us pulse based on the clock frequency. */
	tpgm = (5 * hsp_clk) / 1000000;

	return fuse_controller_execute_command (fuses, FUSE_CONTROLLER_CMD_PROGRAM_DATA,
		FMC_REGS_TPGM_ADDRESS, &tpgm, 1);
}
