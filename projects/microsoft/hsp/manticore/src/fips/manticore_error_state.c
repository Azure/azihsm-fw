// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "manticore_error_state.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "logging/manticore_logging.h"


/**
 * Wait for the crash dump to be available from the all SoC cores.
 *
 * @param manticore The Manticore error state handler to use.
 *
 * @return none.
 */
static void manticore_get_crashdump (const struct manticore_error_state *manticore)
{
	int status;
	enum soc_crashdump_arm_core_id id;
	uint32_t num_available = 0;
	bool available[SOC_CRASHDUMP_ARM_CORE_ID_NUM_OF_ARM_CORE_IDS];
	enum soc_crashdump_arm_core_id failed_core_id = SOC_CRASHDUMP_ARM_CORE_ID_CP0_ID;

	/* Wait for the crash dump to be available from all SoC cores. */
	status = manticore->soc_entry->get_crashdumps_from_cores (manticore->soc_entry,	NULL, available,
		&failed_core_id);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_CRASHDUMP_GET_ARM_CORE_CRASHDUMP_FAILURE, status,	failed_core_id);
	}
	else {
		/* Calculate total number of cores generated crashdump information */
		for (id = SOC_CRASHDUMP_ARM_CORE_ID_CP0_ID;
			id < SOC_CRASHDUMP_ARM_CORE_ID_NUM_OF_ARM_CORE_IDS; id++) {
			if (available[id]) {
				num_available++;
			}
		}

		/* Log the core id that can't collect the crashdump */
		for (id = SOC_CRASHDUMP_ARM_CORE_ID_CP0_ID;
			id < SOC_CRASHDUMP_ARM_CORE_ID_NUM_OF_ARM_CORE_IDS; id++) {
			if (!available[id]) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_CRASHDUMP_NOT_ALL_ARM_CORE_CRASHDUMP_AVAIVABLE, id,
					num_available);
			}
		}
	}
}

const platform_clock* manticore_error_state_get_next_execution (
	const struct periodic_task_handler *handler)
{
	UNUSED (handler);

	/* This uses event-based execution, so always be be ready to run. */
	return NULL;
}

void manticore_error_state_execute (const struct periodic_task_handler *handler)
{
	const struct manticore_error_state *manticore =
		TO_DERIVED_TYPE (handler, const struct manticore_error_state, base_task);
	struct debug_log_entry_info log = {0};
	int status;

	status = platform_semaphore_wait (&manticore->state->error_event, 0);
	if (status != 0) {
		/* Semaphore error, just exit. */
		return;
	}

	/* Locally store the debug log entry to prevent possible corruption by other interrupts or
	 * tasks. */
	if (manticore->state->log_isr.format != 0) {
		memcpy (&log, &manticore->state->log_isr, sizeof (log));
	}
	else if (manticore->state->log_task.format != 0) {
		memcpy (&log, &manticore->state->log_task, sizeof (log));
	}

	/* First, enter the error state for HSP since accessing SoC resources can cause the task to
	 * sleep on a mutex.  No need to provide log details to this call since the log message will be
	 * handled by this task.  Entering the error state on HSP will ensure no output is generated for
	 * MCTP requests. */
	manticore->hsp_entry->enter_error_state (manticore->hsp_entry, NULL);

	/* Next, enter the error state for the SoC by crashing the SoC cores, halting all execution. */
	manticore->soc_entry->trigger_crash_int (manticore->soc_entry);

	/* Update the debug log with details about the error. */
	if (log.format != 0) {
		debug_log_create_entry (log.severity, log.component, log.msg_index, log.arg1, log.arg2);
	}

	manticore_get_crashdump (manticore);

	log_flush_handler_immediate_flush (manticore->soc_log);
	debug_log_flush ();

	/* Finally, reset the device to resolve the error condition. */
	status = manticore->handle_error->reset (manticore->handle_error);
	if (status != 0) {
		/* If reset were to ever fail (which may be impossible), just spin here and wait for the
		 * watchdog to cause a reset. */
		while (1) {
		}
	}
}

void manticore_error_state_enter_error_state_from_task (
	const struct error_state_entry_interface *entry, const struct debug_log_entry_info *error_log)
{
	const struct manticore_error_state *manticore =
		TO_DERIVED_TYPE (entry, const struct manticore_error_state, base_error_task);
	int status;

	if (error_log != NULL) {
		memcpy (&manticore->state->log_task, error_log, sizeof (*error_log));
	}

	status = platform_semaphore_post (&manticore->state->error_event);
	if (status != 0) {
		/* On error, just spin so the requesting task doesn't yield execution.  The watchdog should
		 * eventually trigger. */
		while (1) {
		}
	}
}

void manticore_error_state_enter_error_state_from_isr (
	const struct error_state_entry_interface *entry, const struct debug_log_entry_info *error_log)
{
	const struct manticore_error_state *manticore =
		TO_DERIVED_TYPE (entry, const struct manticore_error_state, base_error_isr);
	int status;

	if (error_log != NULL) {
		memcpy (&manticore->state->log_isr, error_log, sizeof (*error_log));
	}

	status = platform_semaphore_post_from_isr (&manticore->state->error_event);
	if (status != 0) {
		/* On error, just spin to stay stuck in the ISR.  Eventually the watchdog will trigger a
		 * reset. */
		while (1) {
		}
	}
}

/**
 * Initialize the FIPS error state handler for Manticore.
 *
 * @param handler The error state handler to initialize.
 * @param state Variable context for the error state handler.  This must be uninitialized.
 * @param hsp_entry Error state entry handler for HSP.
 * @param soc_entry Error state entry handler for the rest of the SoC.
 * @param soc_log Debug log handler for SoC cores.
 * @param handle_error Interface to resolve the error state through a device reset.
 *
 * @return 0 if the error state handler was initialized successfully or an error code.
 */
int manticore_error_state_init (struct manticore_error_state *handler,
	struct manticore_error_state_state *state, const struct error_state_entry_interface *hsp_entry,
	const struct soc_crashdump_interface *soc_entry, const struct log_flush_handler *soc_log,
	const struct cmd_device *handle_error)
{
	if (handler == NULL) {
		return ERROR_STATE_ENTRY_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (*handler));

	handler->base_task.prepare = NULL;
	handler->base_task.get_next_execution = manticore_error_state_get_next_execution;
	handler->base_task.execute = manticore_error_state_execute;

	handler->base_error_task.enter_error_state = manticore_error_state_enter_error_state_from_task;
	handler->base_error_isr.enter_error_state = manticore_error_state_enter_error_state_from_isr;

	handler->state = state;
	handler->hsp_entry = hsp_entry;
	handler->soc_entry = soc_entry;
	handler->soc_log = soc_log;
	handler->handle_error = handle_error;

	return manticore_error_state_init_state (handler);
}

/**
 * Initialize only the variable state for a FIPS error state handler for Manticore.  The rest of the
 * instance is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param handler The error state handler that contains the state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int manticore_error_state_init_state (const struct manticore_error_state *handler)
{
	if ((handler == NULL) || (handler->state == NULL) || (handler->hsp_entry == NULL) ||
		(handler->soc_entry == NULL) || (handler->soc_log == NULL) ||
		(handler->handle_error == NULL)) {
		return ERROR_STATE_ENTRY_INVALID_ARGUMENT;
	}

	memset (handler->state, 0, sizeof (*handler->state));

	return platform_semaphore_init (&handler->state->error_event);
}

/**
 * Release the resources used for handling the Manticore FIPS error state.
 *
 * @param handler The error state handler to release.
 */
void manticore_error_state_release (const struct manticore_error_state *handler)
{
	if (handler) {
		platform_semaphore_free (&handler->state->error_event);
	}
}
