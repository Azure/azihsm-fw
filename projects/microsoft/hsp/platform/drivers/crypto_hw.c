// Copyright (c) Microsoft Corporation. All rights reserved.

#include "crypto_hw.h"
#include "hsp_top.h"
#include "splibs/hsprt/riscvcpu.h"
#include "splibs/inc/spstatus.h"
#include "splibs/inc/sptypes.h"
#include "system/fatal_error.h"


/* Configurable crypto parameters.  Defaults can be overridden in platform_config.h. */
#include "platform_config.h"
/**
 * The max crypto command execution wait time (ms) as default.
 */
#ifndef HSP_CRYPTO_DRIVER_CMD_EXE_WAIT_TIME
#define HSP_CRYPTO_DRIVER_CMD_EXE_WAIT_TIME			1000
#endif

/**
 * Check if a particular crypto hardware block has generated an interrupt.  If so, signal that the
 * hardware operation has completed.
 *
 * @param irq Base address for the CREG registers to control crypto hardware interrupts.
 * @param irq_mask Bit mask in the crypto IRQ registers to use for control and status.
 * @param done Semaphore that will be signaled when the hardware has completed processing.
 *
 * @return true if the crypto hardware had a pending interrupt that was handled or false if not.
 */
bool crypto_hw_handle_interrupt (struct Creg_regs_creg_crypto_group *irq, uint32_t irq_mask,
	platform_semaphore *done)
{
	bool handled = false;

	/* If Interrupts are not enabled, don't process the interrupt, since it can't be for this crypto
	 * block. */
	if (irq->CRYPTO_DONE_INTEN & irq_mask) {
		/* Check if there was an interrupt for this HW. */
		if (irq->CRYPTO_DONE_INTSTS & irq_mask) {
			irq->CRYPTO_DONE_INTSTS = irq_mask;
			handled = true;
		}

		if (irq->CRYPTO_ERR_INTSTS & irq_mask) {
			irq->CRYPTO_ERR_INTSTS = irq_mask;
			handled = true;
		}

		if (handled) {
			platform_semaphore_post_from_isr (done);
		}
	}

	return handled;
}

/**
 * Submit a command to the hardware for execution and block until the hardware has completed the
 * request.  The function will block, waiting for an interrupt to signal completion of operation.
 *
 * @param irq Base address for the CREG registers to control crypto hardware interrupts.
 * @param irq_mask Bit mask in the crypto IRQ registers to use for control and status.
 * @param done Semaphore that will be signaled when the hardware has completed processing.
 * @param cmd_ptr Address of the command to submit to the hardware.
 * @param cmd_reg The command register for the crypto hardware block.
 * @param status_reg The status register for the crypto hardware block.
 * @param busy_mask Bit mask for the status bit indicating that the hardware is busy.
 * @param parse_command_status Callback function to parse the hardware status bits.
 * @param error_code Error to return when the the command failure bit is set.
 * @param timeout_code Error to return when the the command execution timeout.
 *
 * @return 0 if the command was successfully executed or an error code.
 */
int crypto_hw_submit_command_interrupt (struct Creg_regs_creg_crypto_group *irq, uint32_t irq_mask,
	platform_semaphore *done, void *cmd_ptr, volatile uint32_t *cmd_reg,
	volatile uint32_t *status_reg, uint32_t busy_mask,
	crypto_hw_parse_status_callback parse_command_status, int error_code, int timeout_code)
{
	uint32_t hw_status;
	int status;

	status = platform_semaphore_reset (done);
	if (status != 0) {
		return status;
	}

	/* Make sure the interrupt status is clear. */
	irq->CRYPTO_DONE_INTSTS = irq_mask;
	irq->CRYPTO_ERR_INTSTS = irq_mask;

	DMB;
	*cmd_reg = (uint32_t) ((uintptr_t) cmd_ptr);
	DMB;

	hw_status = *status_reg;
	if (hw_status & busy_mask) {
		/* HW is busy.  Enable interrupts to be notified of completion. */
		irq->CRYPTO_DONE_INTEN |= irq_mask;
		irq->CRYPTO_ERR_INTEN |= irq_mask;

		status = platform_semaphore_wait (done, HSP_CRYPTO_DRIVER_CMD_EXE_WAIT_TIME);

		/* Disable interrupts after processing is complete. */
		irq->CRYPTO_ERR_INTEN &= ~irq_mask;
		irq->CRYPTO_DONE_INTEN &= ~irq_mask;

		/* Timeout occurred. */
		if (status == 1) {
			fatal_error_unrecoverable_error (timeout_code);

			return timeout_code;
		}

		hw_status = *status_reg;
	}

	return (status == 0) ? parse_command_status (hw_status, error_code) : status;
}

/**
 * Submit a command to the hardware for execution and block until the hardware has completed the
 * request.  The function will spin in a busy loop, polling the hardware to determine when the
 * operation has completed.
 *
 * @param cmd_ptr Address of the command to submit to the hardware.
 * @param cmd_reg The command register for the crypto hardware block.
 * @param status_reg The status register for the crypto hardware block.
 * @param busy_mask Bit mask for the status bit indicating that the hardware is busy.
 * @param parse_command_status Callback function to parse the hardware status bits.
 * @param error_code Error to return when the the command failure bit is set.
 * @param timeout_code Error to return when the the command execution timeout.
 *
 * @return 0 if the command was successfully executed or an error code.
 */
int crypto_hw_submit_command_polling (void *cmd_ptr, volatile uint32_t *cmd_reg,
	volatile uint32_t *status_reg, uint32_t busy_mask,
	crypto_hw_parse_status_callback parse_command_status, int error_code, int timeout_code)
{
	platform_clock time_to_timeout;
	int timeout;
	uint32_t status_reg_val;
	int status;

	DMB;
	*cmd_reg = (uint32_t) ((uintptr_t) cmd_ptr);
	DMB;

	status = platform_init_timeout (HSP_CRYPTO_DRIVER_CMD_EXE_WAIT_TIME, &time_to_timeout);
	if (status != 0) {
		fatal_error_unrecoverable_error (status);

		return status;
	}

	/* Wait for the command to complete. */
	do {
		status_reg_val = *status_reg;

		timeout = platform_has_timeout_expired (&time_to_timeout);
		if ((timeout != 0) && (timeout != 1)) {
			return timeout;
		}
	} while ((status_reg_val & busy_mask) && (!timeout));

	/* Timeout occurred. */
	if (status_reg_val & busy_mask) {
		fatal_error_unrecoverable_error (timeout_code);

		return timeout_code;
	}

	return parse_command_status (status_reg_val, error_code);
}
