// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "fatal_error_handler_hsp.h"
#include "platform_api.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "crashdump/hsp_crashdump_logging.h"

#ifdef PLATFORM_RISCV
#include "splibs/hsprt/riscvcpu.h"
#endif


void fatal_error_handler_hsp_unrecoverable_error (const struct fatal_error_handler *handler)
{
	const struct fatal_error_handler_hsp *hsp =
		TO_DERIVED_TYPE (handler, const struct fatal_error_handler_hsp, base);

	if (handler != NULL) {
		hsp->crash->reset (hsp->crash);
	}

#ifdef PLATFORM_RISCV
	/* If the reset fails for some reason, halt the CPU. */
	CEASE;
#endif
}

void fatal_error_handler_hsp_panic (const struct fatal_error_handler *handler, int error_code,
	const struct debug_log_entry_info *error_log)
{
	const struct fatal_error_handler_hsp *hsp =
		TO_DERIVED_TYPE (handler, const struct fatal_error_handler_hsp, base);
	struct hsp_trap_context context;

	/* Suspend the scheduler to prevent any context switching during panic handling. */
	platform_os_suspend_scheduler ();

	if (handler != NULL) {
		hsp_crashdump_logging_create_panic_context (error_code, error_log, &context);

		hsp_crashdump_handler_collect_hsp_crashdump (hsp->log, &context,
			HSP_CRASHDUMP_PACKET_FAULT_CODE_PANIC, CRASH_DUMP_CRASH_TYPE_NORMAL);

		hsp->crash->reset (hsp->crash);
	}

#ifdef PLATFORM_RISCV
	/* If the reset fails for some reason, halt the CPU. */
	CEASE;
#endif
}

/**
 * Initialize a handler for fatal errors on HSP platforms.
 *
 * @param handler The error handler to initialize.
 * @param log The crash dump handler to use for panic log collection.
 * @param crash The device handler for triggering a reset.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int fatal_error_handler_hsp_init (struct fatal_error_handler_hsp *handler,
	const struct hsp_crashdump_handler *log, const struct cmd_device *crash)
{
	if ((handler == NULL) || (log == NULL) || (crash == NULL)) {
		return FATAL_ERROR_HANDLER_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (*handler));

	handler->base.unrecoverable_error = fatal_error_handler_hsp_unrecoverable_error;
	handler->base.panic = fatal_error_handler_hsp_panic;

	handler->log = log;
	handler->crash = crash;

	return 0;
}

/**
 * Release the resources used for fatal error handling on HSP.
 *
 * @param handler The error handler to release.
 */
void fatal_error_handler_hsp_release (const struct fatal_error_handler_hsp *handler)
{
	UNUSED (handler);
}
