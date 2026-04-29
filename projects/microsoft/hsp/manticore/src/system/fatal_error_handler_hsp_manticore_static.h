// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FATAL_ERROR_HANDLER_HSP_MANTICORE_STATIC_H_
#define FATAL_ERROR_HANDLER_HSP_MANTICORE_STATIC_H_

#include "fatal_error_handler_hsp_manticore.h"
#include "system/fatal_error_handler_hsp_static.h"


/* Internal functions declared to allow for static initialization. */
void fatal_error_handler_hsp_manticore_unrecoverable_error (
	const struct fatal_error_handler *handler);


/**
 * Constant initializer for the fatal error handler API.
 */
#define	FATAL_ERROR_HANDLER_HSP_MANTICORE_API_INIT  { \
		.unrecoverable_error = fatal_error_handler_hsp_manticore_unrecoverable_error, \
		.panic = fatal_error_handler_hsp_panic, \
	}


/**
 * Initialize a static instance of a handler for fatal errors on Manticore.
 *
 * There is no validation done on the arguments.
 *
 * @param log_ptr The crash dump handler to use for panic log collection.
 * @param crash_ptr The device handler for triggering a reset.
 * @param flush_ptr Handler for flushing all SoC logs.
 */
#define	fatal_error_handler_hsp_manticore_static_init(log_ptr, crash_ptr, flush_ptr)	{ \
	.base = fatal_error_handler_hsp_static_init_with_api ( \
		FATAL_ERROR_HANDLER_HSP_MANTICORE_API_INIT, log_ptr, crash_ptr), \
	.flush = flush_ptr, \
}


#endif	/* FATAL_ERROR_HANDLER_HSP_MANTICORE_STATIC_H_ */
