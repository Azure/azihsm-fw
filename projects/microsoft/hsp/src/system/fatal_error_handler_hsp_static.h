// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FATAL_ERROR_HANDLER_HSP_STATIC_H_
#define FATAL_ERROR_HANDLER_HSP_STATIC_H_

#include "fatal_error_handler_hsp.h"


/* Internal functions declared to allow for static initialization. */
void fatal_error_handler_hsp_panic (const struct fatal_error_handler *handler, int error_code,
	const struct debug_log_entry_info *error_log);


/**
 * Constant initializer for the fatal error handler API.
 */
#define	FATAL_ERROR_HANDLER_HSP_API_INIT  { \
		.unrecoverable_error = fatal_error_handler_hsp_unrecoverable_error, \
		.panic = fatal_error_handler_hsp_panic, \
	}

/**
 * Internal initialization of the HSP fatal error handler that can be used by derived types to
 * override the API configuration.
 *
 * There is no validation done on the arguments.
 *
 * @param api Initializer to use for the fatal error handler API.
 * @param log_ptr The crash dump handler to use for panic log collection.
 * @param crash_ptr The device handler for triggering a reset.
 */
#define fatal_error_handler_hsp_static_init_with_api(api, log_ptr, crash_ptr)	{ \
		.base = api, \
		.log = log_ptr, \
		.crash = crash_ptr, \
	}


/**
 * Initialize a static instance of a handler for fatal errors on HSP platforms.
 *
 * There is no validation done on the arguments.
 *
 * @param log_ptr The crash dump handler to use for panic log collection.
 * @param crash_ptr The device handler for triggering a reset.
 */
#define	fatal_error_handler_hsp_static_init(log_ptr, crash_ptr) \
	fatal_error_handler_hsp_static_init_with_api (FATAL_ERROR_HANDLER_HSP_API_INIT, log_ptr, \
		crash_ptr)


#endif	/* FATAL_ERROR_HANDLER_HSP_STATIC_H_ */
