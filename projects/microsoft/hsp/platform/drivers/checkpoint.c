// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "common/unused.h"
#include "drivers/checkpoint.h"


/**
 * Write a single register in the checkpoint hardware.
 *
 * @param chkpt The checkpoint driver instance.
 * @param reg The register that should be written.
 * @param val The value to write.
 */
static void checkpoint_write_register (const struct checkpoint *chkpt, volatile uint32_t *reg,
	uint32_t val)
{
	HSP_CHKPT_STATUS status;

	/* Need to make sure the buffer is not full before writing to a register. */
	do {
		status.AsUint32 = chkpt->regs->CHKPT_STATUS;
	} while (status.BufferFull);

	*reg = val;
}

/**
 * Read a single register in the checkpoint hardware.
 *
 * @param reg The register to read.
 *
 * @return The register value.
 */
static uint32_t checkpoint_read_register (volatile uint32_t *reg)
{
	return *reg;
}

void checkpoint_set_config (const struct checkpoint *chkpt, const HSP_CHKPT_CONFIG *config)
{
	HSP_CHKPT_STATUS status;

	if (chkpt && config) {
		/* Need to make sure the previous buffer has been fully consumed before writing the next
		 * configuration. */
		do {
			status.AsUint32 = chkpt->regs->CHKPT_STATUS;
		} while (!status.BufferEmpty && status.HashBusy);

		checkpoint_write_register (chkpt, &chkpt->regs->CHKPT_EXP_DIGEST0, config->Digest0);
		checkpoint_write_register (chkpt, &chkpt->regs->CHKPT_EXP_DIGEST1, config->Digest1);
		checkpoint_write_register (chkpt, &chkpt->regs->CHKPT_EXP_DIGEST2, config->Digest2);
		checkpoint_write_register (chkpt, &chkpt->regs->CHKPT_EXP_DIGEST3, config->Digest3);

		checkpoint_write_register (chkpt, &chkpt->regs->CHKPT_TIMER_IV0, config->Timer0);
		checkpoint_write_register (chkpt, &chkpt->regs->CHKPT_TIMER_IV1, config->Timer1);

		checkpoint_write_register (chkpt, &chkpt->regs->CHKPT_FEN_P, config->Fence.AsUint32);
		checkpoint_write_register (chkpt, &chkpt->regs->CHKPT_CTRL_P, config->Ctrl);

		/* Read the active fence to force the enable of fences. */
		checkpoint_read_register (&chkpt->regs->CHKPT_FEN_A);
	}
}

int checkpoint_check_config (const struct checkpoint *chkpt, const HSP_CHKPT_CONFIG *config)
{
	HSP_CHKPT_STATUS status;

	if ((chkpt == NULL) || (config == NULL)) {
		return CHECKPOINT_INVALID_ARGUMENT;
	}

	/* When the checkpoint is successfully reached, the active digest is cleared. */
	if (chkpt->regs->CHKPT_DIGEST0 != 0) {
		return CHECKPOINT_DIGEST_MISMATCH;
	}

	/* And the pending fencing bits become active. */
	if (chkpt->regs->CHKPT_FEN_A != config->Fence.AsUint32) {
		return CHECKPOINT_FENCE_NOT_APPLIED;
	}

	/* As well as the pending control bits. */
	if (chkpt->regs->CHKPT_CTRL_A != config->Ctrl) {
		return CHECKPOINT_CTRL_NOT_APPLIED;
	}

	/* If there was an checkpoint timeout or overflow, report that also. */
	status.AsUint32 = chkpt->regs->CHKPT_STATUS;
	if (status.TimerExpired || status.BufferOverflow) {
		return CHECKPOINT_HW_ERROR (status.AsUint32);
	}

	return 0;
}

void checkpoint_lock_hw (const struct checkpoint *chkpt)
{
	if (chkpt) {
		checkpoint_write_register (chkpt, &chkpt->regs->CHKPT_CTRL_A, 1);
	}
}

void checkpoint_write_message (const struct checkpoint *chkpt, uint32_t message)
{
	if (chkpt) {
		checkpoint_write_register (chkpt, &chkpt->regs->CHKPT_MSG, message);
	}
}

/**
 * Initialize a driver interface to the hardware checkpoint for code flow enforcement.
 *
 * @param chkpt The checkpoint driver to initialize.
 * @param regs Base address for the checkpoint registers.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int checkpoint_init (struct checkpoint *chkpt, struct Creg_regs_chkpt_regs *regs)
{
	if ((chkpt == NULL) || (regs == NULL)) {
		return CHECKPOINT_INVALID_ARGUMENT;
	}

	memset (chkpt, 0, sizeof (struct checkpoint));

	chkpt->set_config = checkpoint_set_config;
	chkpt->check_config = checkpoint_check_config;
	chkpt->lock_hw = checkpoint_lock_hw;
	chkpt->write_message = checkpoint_write_message;

	chkpt->regs = regs;

	return 0;
}

/**
 * Release the resources used for a checkpoint driver.
 *
 * @param chkpt The checkpoint driver to release.
 */
void checkpoint_release (const struct checkpoint *chkpt)
{
	UNUSED (chkpt);
}
