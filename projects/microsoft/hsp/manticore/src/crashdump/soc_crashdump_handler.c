// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "soc_crashdump_handler_static.h"
#if SOC_CRASHDUMP_HANDLER_PRINTF
#include "platform_io_api.h"
#endif
#include "common/type_cast.h"
#include "common/unused.h"
#include "logging/debug_log.h"
#include "logging/manticore_logging.h"

#ifndef MANTICORE_DISABLE_CRASHDUMP
/**
 * Collect SoC crashdumps from ARM cores.
 *
 * @param[in] handler The instance of SoC crashdump handler.
 * @param[out] num_available The number of crashdumps that are available for collecting.
 * @param[out] failed_core_id The core ID that failed to save its crashdump.
 *
 * @return 0 if the crashdump collecting succeeded or an error code.
 */
int soc_crashdump_handler_get_crashdumps (const struct soc_crashdump_handler *handler,
	uint32_t *num_available, enum soc_crashdump_arm_core_id *failed_core_id)
{
	int status;
	bool available[SOC_CRASHDUMP_ARM_CORE_ID_NUM_OF_ARM_CORE_IDS];
	enum soc_crashdump_arm_core_id av_id;
	uint32_t local_num_available;
	enum soc_crashdump_arm_core_id local_failed_core_id;

#if SOC_CRASHDUMP_HANDLER_PRINTF
	const char *core_names[] = {"CP0", "CP1", "FP0", "FP1", "FP2"};
#endif

	if (handler == NULL) {
		return SOC_CRASHDUMP_INTERFACE_INVALID_ARGUMENT;
	}

	if (num_available == NULL) {
		num_available = &local_num_available;
	}

	if (failed_core_id == NULL) {
		failed_core_id = &local_failed_core_id;
	}

	/* If the crash case is an ARM core ran into exception, since crash was processed
	 * by either exception handler or ISR, crashdumps should be ready in
	 * short time frame. */
	status = handler->soc_api->get_crashdumps_from_cores (handler->soc_api,	handler->fw_version,
		available, failed_core_id);
	if (status != 0) {
		return status;
	}

	*num_available = 0;
	for (av_id = SOC_CRASHDUMP_ARM_CORE_ID_CP0_ID;
		av_id < SOC_CRASHDUMP_ARM_CORE_ID_NUM_OF_ARM_CORE_IDS; av_id++) {
		if (available[av_id] == true) {
#if SOC_CRASHDUMP_HANDLER_PRINTF
			if (*num_available == 0) {
				platform_printf ("Collected crashdump from %s", core_names[av_id]);
			}
			else {
				platform_printf (", %s", core_names[av_id]);
			}
#endif
			(*num_available)++;
		}
	}

	return status;
}
#endif	// MANTICORE_DISABLE_CRASHDUMP

void soc_crashdump_handler_task_handler_prepare (const struct periodic_task_handler *task_handler)
{
	const struct soc_crashdump_handler *handler = TO_DERIVED_TYPE (task_handler,
		const struct soc_crashdump_handler, base);
	int status;
	enum soc_crashdump_arm_core_id failed_core_id;

	handler->state->first_check = true;
	handler->state->unrecoverable_fault = false;
	handler->state->run_monitor = false;

	/* Delay the start of the SoC crash monitoring until the system is expected to be running.
	 * This just delays the start of handler execution.  Actual crash monitoring won't start until
	 * explicitly told to start by system initialization code. */
	platform_init_timeout (SOC_CRASHDUMP_HANDLER_EXE_START_DELAY, &handler->state->next);

	/* Set status to 1 for all ARM cores before starting crashdump_monitor. */
	status = handler->soc_api->set_all_core_status (handler->soc_api, &failed_core_id);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_CRASHDUMP_SET_CORE_STATUS_FAILURE, status, failed_core_id);
	}
}

const platform_clock* soc_crashdump_handler_task_handler_next_execution (
	const struct periodic_task_handler *task_handler)
{
	const struct soc_crashdump_handler *handler = TO_DERIVED_TYPE (task_handler,
		const struct soc_crashdump_handler, base);

	return &handler->state->next;
}

void soc_crashdump_handler_task_handler_execute (const struct periodic_task_handler *task_handler)
{
	int status;
	const struct soc_crashdump_handler *handler = TO_DERIVED_TYPE (task_handler,
		const struct soc_crashdump_handler, base);
	uint32_t core_status[SOC_CRASHDUMP_ARM_CORE_ID_NUM_OF_ARM_CORE_IDS];
	enum soc_crashdump_arm_core_id id;
	enum soc_crashdump_arm_core_id failed_core_id = SOC_CRASHDUMP_ARM_CORE_ID_CP0_ID;

#if SOC_CRASHDUMP_HANDLER_PRINTF
	enum soc_crashdump_arm_core_id count_id;
	const char *core_names[] = {"CP0", "CP1", "FP0", "FP1", "FP2"};
#endif

	if (!handler->state->run_monitor) {
		/* The crash monitor has not been started yet.  Do nothing aside from setting the next
		 * execution time. */
		goto exit_time_only;
	}

	/* Get all ARM core status. */
	status = handler->soc_api->get_all_core_status (handler->soc_api, &failed_core_id, core_status);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_CRASHDUMP_GET_CORE_STATUS_FAILURE, status, failed_core_id);

		goto exit;
	}

	/* Verify each of ARM core status. */
	for (id = SOC_CRASHDUMP_ARM_CORE_ID_CP0_ID; id < SOC_CRASHDUMP_ARM_CORE_ID_NUM_OF_ARM_CORE_IDS;
		id++) {
		/* If status of an ARM core is not 0, clear it. */
#if SOC_CRASHDUMP_HANDLER_PRINTF_OFF_BY_DEFAULT
		platform_printf ("%s core status %d" NEWLINE, core_names[id], core_status[id]);
#endif
		if (core_status[id]) {
			status = handler->soc_api->clear_core_status (handler->soc_api, id);
			if (status != 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_CRASHDUMP_CLEAR_CORE_STATUS_FAILURE, status, id);

				goto exit;
			}
		}
		/* If status of an ARM core is 0, the core stuck. */
		else {
			if (*handler->soc_api->crash_count >= MAX_CRASH_COUNT) {
#if SOC_CRASHDUMP_HANDLER_PRINTF
				/* Carriage-return at the end helps preserve existing SP UART logs. The message is
				   continuous to ensure it is captured after attaching to UART. */
				platform_printf ("UNRECOVERABLE FAULT: Check debuglog then power cycle" NEWLINE);
#endif
				if (!handler->state->unrecoverable_fault) {
					debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
						MANTICORE_LOGGING_CRASHDUMP_RESET_REACHED_MAX,
						*handler->soc_api->crash_count, id);
					handler->state->unrecoverable_fault = true;
				}

				break;
			}

			(*handler->soc_api->crash_count)++;

#if SOC_CRASHDUMP_HANDLER_PRINTF
			platform_printf ("Crash count = %d. Cores (status not updated): ",
				*handler->soc_api->crash_count);

			for (count_id = SOC_CRASHDUMP_ARM_CORE_ID_CP0_ID;
				count_id < SOC_CRASHDUMP_ARM_CORE_ID_NUM_OF_ARM_CORE_IDS; count_id++) {
				if (!core_status[count_id]) {
					platform_printf ("%s ", core_names[count_id]);
				}
			}
			platform_printf (NEWLINE);
#endif

#ifndef MANTICORE_DISABLE_CRASHDUMP
			uint32_t num_available;

			/* If the crash case is an ARM core ran into exception, since crash was processed
			 * by either exception handler or ISR, crashdumps should be ready in
			 * short time frame. */
			status = soc_crashdump_handler_get_crashdumps (handler, &num_available,
				&failed_core_id);

			if (status != 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_CRASHDUMP_GET_ARM_CORE_CRASHDUMP_FAILURE, status,
					failed_core_id);

				goto exit;
			}

			if (num_available > 0) {
#if SOC_CRASHDUMP_HANDLER_PRINTF
				platform_printf (NEWLINE);
				platform_printf (
					"%s got exception, or preempted by another core over wakeup1 interrupt"
					NEWLINE, core_names[id]);
#endif
			}
			else {
				/* If num_available equal to 0, it means no ARM core ran into exception.
				 * In other words, an ARM core could be hanging. */
#if SOC_CRASHDUMP_HANDLER_PRINTF
				platform_printf ("%s was hanging" NEWLINE, core_names[id]);
#endif
				/* Generate interrupt to preempt ARM core operation from hanging. */
				status = handler->soc_api->trigger_crash_int (handler->soc_api);
				if (status != 0) {
					debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
						MANTICORE_LOGGING_CRASHDUMP_TRIGGER_ARM_CORE_INT_FAILURE, status, 0);

					goto exit;
				}

#if SOC_CRASHDUMP_HANDLER_PRINTF
				platform_printf ("Triggered wakeup 1 interrupt to ARM cores." NEWLINE);
#endif
				/* Pulling crashdumps. */
				status = soc_crashdump_handler_get_crashdumps (handler, &num_available,
					&failed_core_id);
				if (status != 0) {
					debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
						MANTICORE_LOGGING_CRASHDUMP_GET_ARM_CORE_CRASHDUMP_FAILURE, status,
						failed_core_id);

					goto exit;
				}

#if SOC_CRASHDUMP_HANDLER_PRINTF
				if (num_available != 0) {
					platform_printf (NEWLINE);
				}
#endif
				if (num_available < SOC_CRASHDUMP_ARM_CORE_ID_NUM_OF_ARM_CORE_IDS) {
					debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
						MANTICORE_LOGGING_CRASHDUMP_NOT_ALL_ARM_CORE_CRASHDUMP_AVAIVABLE, id,
						num_available);
				}
			}

			/* Best-effort attempt to flush prior to the reset. Failures are ignored. */
			log_flush_handler_immediate_flush (handler->log_flush);

			/* Reset SPRT, 1SP will make warm boot. */
			status = handler->soc_api->reset (handler->soc_api);
			if (status != 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_CRASHDUMP_RESET_FAILURE, status, 0);

				goto exit;
			}

			/* Should never get here. */
			goto exit;
#endif	/* MANTICORE_DISABLE_CRASHDUMP */
		}
	}

exit:

	if ((*handler->soc_api->crash_count > 0) &&
		(*handler->soc_api->crash_count < MAX_CRASH_COUNT) &&
		!handler->state->first_check) {
#if SOC_CRASHDUMP_HANDLER_PRINTF
		platform_printf ("Resetting crash_counter" NEWLINE);
#endif
		*handler->soc_api->crash_count = 0;
	}

	handler->state->first_check = false;

exit_time_only:
	/* Monitor ARM cores again in another second.  On Manticore, this call will not fail. */
	platform_init_timeout (handler->refresh_period, &handler->state->next);
}

/**
 * Initialize crashdump SoC handler.
 *
 * @param[in] handler The crashdump handler to initialize.
 * @param[in] state The variable context for the crashdump handler.
 * @param[in] refresh_period The amount of time between calls to execute SoC crashdump handler.
 * @param[in] soc_api The SoC interface instance.
 * @param[in] log_flush The log flusher used to flush the log buffers before crashdump.
 * @param[in] fw_version The build version number for the firmware package.
 * @param[in] fw_version_len The length of build version number for the firmware package.
 *
 * @return 0 if the crashdump SoC handler was successfully initialized or an error code.
 */
int soc_crashdump_handler_init (struct soc_crashdump_handler *handler,
	struct soc_crashdump_handler_state *state, const uint32_t refresh_period,
	const struct soc_crashdump_interface *soc_api, const struct log_flush_handler *log_flush,
	uint8_t *fw_version, size_t fw_version_len)
{
	if ((handler == NULL) || (state == NULL) || (soc_api == NULL) ||
		(fw_version == NULL) || (fw_version_len == 0)) {
		return SOC_CRASHDUMP_INTERFACE_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (struct soc_crashdump_handler));

	handler->base.prepare = soc_crashdump_handler_task_handler_prepare;
	handler->base.execute = soc_crashdump_handler_task_handler_execute;
	handler->base.get_next_execution = soc_crashdump_handler_task_handler_next_execution;

	handler->state = state;
	handler->refresh_period = refresh_period;
	handler->soc_api = soc_api;
	handler->log_flush = log_flush;
	handler->fw_version = fw_version;
	handler->fw_version_len = fw_version_len;

	return 0;
}

/**
 * Release the resources used for crashdump SoC handler.
 *
 * @param[in] handler The crashdump handler to release.
 */
void soc_crashdump_handler_release (const struct soc_crashdump_handler *handler)
{
	UNUSED (handler);
}

/**
 * Start monitoring for SoC ARM core crashes.  While the handler will be active along with the task
 * that contains it, there will be no interaction with the SoC until this function has been called.
 *
 * @param handler The crashdump handler to start.
 */
void soc_crashdump_handler_start_crash_monitor (const struct soc_crashdump_handler *handler)
{
	if (handler) {
		handler->state->run_monitor = true;
	}
}
