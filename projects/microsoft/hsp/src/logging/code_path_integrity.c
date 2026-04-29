// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "boot_status_log.h"
#include "code_path_integrity.h"
#include "platform_io_api.h"
#include "post_code_log.h"


/* Configurable CPI parameters.  Defaults can be overridden in platform_config.h. */
#include "platform_config.h"
#ifndef HSP_CPI_RANDOM_DELAY_BITS
#define	HSP_CPI_RANDOM_DELAY_BITS		8
#endif


/* Message values to write to the checkpoint hardware when handing off from one chain to another. */
#define	CODE_PATH_INTEGRITY_HAND_OFF_MESSAGE_0		0x11112222
#define	CODE_PATH_INTEGRITY_HAND_OFF_MESSAGE_1		0x33334444


/**
 * The singleton instance that will be used to handle CPI calls.
 */
extern const struct code_path_integrity *const global_cpi;


/**
 * Determine if CPI should use the checkpoint hardware.  Checkpoint is only enabled once the device
 * has moved to the secure state.
 */
#ifndef CODE_PATH_INTEGRITY_ALWAYS_ENABLED
#define	CODE_PATH_INTEGRITY_USE_CHKPT       \
	(global_cpi->fuses->get_security_state (global_cpi->fuses) == HSP_SECURITY_STATE_SECURE)
#else
#define	CODE_PATH_INTEGRITY_USE_CHKPT		true
#endif


/**
 * Handle a code integrity message.
 *
 * @param message The message being sent.
 * @param checkpoint Flag indicating if this message should be sent to the checkpoint hardware.
 * @param trace Flag indicating if this message should be sent over UART.
 */
static void code_path_integrity_message (uint32_t message, bool checkpoint, bool trace)
{
	if (global_cpi) {
		if (trace) {
			platform_printf ("P:%x" NEWLINE, message);

#ifdef LOGGING_SUPPORT_POST_CODE_LOG
			post_code_log_create_entry (message);
#endif

#ifdef LOGGING_SUPPORT_BOOT_STATUS_LOG
			boot_status_log_create_entry (BOOT_STATUS_NO_FAILURE, message);
#endif
		}

		if (checkpoint && CODE_PATH_INTEGRITY_USE_CHKPT) {
			global_cpi->chkpt->write_message (global_cpi->chkpt, message);
		}

		/* Insert a random delay to mitigate timing attacks by adding
		 * randomness to execution flow. */
		code_path_integrity_random_delay ();
	}
}

/**
 * Generate an execution tracing message over UART.  This message is not a security gate, but rather
 * just an informational tracing message.
 *
 * @param message The message (i.e. postcode) to output.
 */
void code_path_integrity_message_trace (uint32_t message)
{
	code_path_integrity_message (message, false, true);
}

/**
 * Update the hardware execution monitor with a security critical message.  This message will
 * additionally pushed as a tracing message.
 *
 * @param message The message (i.e. postcode) to send to the checkpoint.
 */
void code_path_integrity_secure_message_trace (uint32_t message)
{
	code_path_integrity_message (message, true, true);
}

/**
 * Update the hardware execution monitor with a security critical message without generating a
 * tracing message.
 *
 * @param message The message to send to the checkpoint.
 */
void code_path_integrity_secure_message_no_trace (uint32_t message)
{
	code_path_integrity_message (message, true, false);
}

/**
 * Update the configuration for a checkpoint in code execution.
 * - If this is the first checkpoint being configured, the checkpoint hardware will be locked.
 * - If this is a subsequent configuration, the checkpoint will be checked for errors and the device
 * will stall if there is unexpected execution.
 *
 * @param config The configuration to apply to the checkpoint hardware.
 * @param init Flag indicating that this is the first checkpoint in the chain.
 */
static void code_path_integrity_checkpoint_config (const HSP_CHKPT_CONFIG *config, bool init)
{
	if (global_cpi && config && CODE_PATH_INTEGRITY_USE_CHKPT) {
		global_cpi->chkpt->set_config (global_cpi->chkpt, config);

		if (init) {
			global_cpi->chkpt->lock_hw (global_cpi->chkpt);
		}
		else {
			int status = global_cpi->chkpt->check_config (global_cpi->chkpt, config);

			if (status != 0) {
				global_cpi->chkpt_fail (status);
			}
		}
	}
}

/**
 * Configure the hardware execution monitor to begin tracking messages from code execution.
 *
 * @param config The configuration to apply to the checkpoint hardware.
 */
void code_path_integrity_checkpoint_start (const HSP_CHKPT_CONFIG *config)
{
	code_path_integrity_checkpoint_config (config, true);
}

/**
 * Hand-off execution monitoring from one checkpoint chain to another.  This process involves
 * writing messages and a checkpoint to complete the current chain, then immediately initializing a
 * new chain.
 *
 * @param current The current chain to complete.
 * @param next The next chain to start.
 */
void code_path_integrity_checkpoint_hand_off (const HSP_CHKPT_CONFIG *current,
	const HSP_CHKPT_CONFIG *next)
{
	if (global_cpi && current && next && CODE_PATH_INTEGRITY_USE_CHKPT) {
		/* Complete the current checkpoint chain. */
		code_path_integrity_secure_message_no_trace (CODE_PATH_INTEGRITY_HAND_OFF_MESSAGE_0);
		code_path_integrity_secure_message_no_trace (CODE_PATH_INTEGRITY_HAND_OFF_MESSAGE_1);
		code_path_integrity_checkpoint_config (current, false);

		/* Start the new chain. */
		code_path_integrity_checkpoint_start (next);
	}
}

/**
 * Mark a checkpoint in code execution and apply a new configuration to the checkpoint.
 *
 * @param config The configuration to apply to the checkpoint hardware.
 *
 */
void code_path_integrity_checkpoint (const HSP_CHKPT_CONFIG *config)
{
	code_path_integrity_checkpoint_config (config, false);
}

/**
 * Insert a delay of random length, but very short.  This delay is based on CPU clocks rather than
 * longer timer ticks.
 *
 * If code path integrity is not enabled, this will not insert any delay.
 */
void code_path_integrity_random_delay ()
{
	if (global_cpi) {
		volatile uint32_t delay;

		global_cpi->rng->generate_random_buffer (global_cpi->rng, sizeof (delay),
			(uint8_t*) &delay);

		delay &= (1 << HSP_CPI_RANDOM_DELAY_BITS) - 1;
		while (delay-- != 0) {
		}
	}
}
