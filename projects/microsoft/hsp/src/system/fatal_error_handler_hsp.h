// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FATAL_ERROR_HANDLER_HSP_H_
#define FATAL_ERROR_HANDLER_HSP_H_

#include "cmd_interface/cmd_device.h"
#include "crashdump/hsp_crashdump_handler.h"
#include "system/fatal_error_handler.h"


/**
 * A fatal error handler for HSP devices that will store panic information to persistent RAM.
 */
struct fatal_error_handler_hsp {
	struct fatal_error_handler base;			/**< Base handler instance. */
	const struct hsp_crashdump_handler *log;	/**< Handler for crash dump collection. */
	const struct cmd_device *crash;				/**< Handler to crash the device via a reset. */
};


int fatal_error_handler_hsp_init (struct fatal_error_handler_hsp *handler,
	const struct hsp_crashdump_handler *log, const struct cmd_device *crash);
void fatal_error_handler_hsp_release (const struct fatal_error_handler_hsp *handler);

/* Internal functions for use by derived types. */
void fatal_error_handler_hsp_unrecoverable_error (const struct fatal_error_handler *handler);


#endif	/* FATAL_ERROR_HANDLER_HSP_H_ */
