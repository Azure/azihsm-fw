// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "fatal_error_handler_hsp_manticore.h"
#include "common/type_cast.h"
#include "common/unused.h"


void fatal_error_handler_hsp_manticore_unrecoverable_error (
	const struct fatal_error_handler *handler)
{
	const struct fatal_error_handler_hsp_manticore *manticore =
		TO_DERIVED_TYPE (handler, const struct fatal_error_handler_hsp_manticore, base);

	if (handler != NULL) {
		log_flush_handler_immediate_flush (manticore->flush);
	}

	fatal_error_handler_hsp_unrecoverable_error (handler);
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
int fatal_error_handler_hsp_manticore_init (struct fatal_error_handler_hsp_manticore *handler,
	const struct hsp_crashdump_handler *log, const struct cmd_device *crash,
	const struct log_flush_handler *flush)
{
	int status;

	if ((handler == NULL) || (flush == NULL)) {
		return FATAL_ERROR_HANDLER_INVALID_ARGUMENT;
	}

	status = fatal_error_handler_hsp_init (&handler->base, log, crash);
	if (status == 0) {
		handler->base.base.unrecoverable_error =
			fatal_error_handler_hsp_manticore_unrecoverable_error;

		handler->flush = flush;
	}

	return status;
}

/**
 * Release the resources used for fatal error handling on HSP.
 *
 * @param handler The error handler to release.
 */
void fatal_error_handler_hsp_manticore_release (
	const struct fatal_error_handler_hsp_manticore *handler)
{
	UNUSED (handler);
}
