// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FATAL_ERROR_HANDLER_HSP_MANTICORE_H_
#define FATAL_ERROR_HANDLER_HSP_MANTICORE_H_

#include "logging/log_flush_handler.h"
#include "system/fatal_error_handler_hsp.h"


/**
 * A fatal error handler for Manticore that extends HSP fatal error handling with flushing of SoC
 * debug logs.
 */
struct fatal_error_handler_hsp_manticore {
	struct fatal_error_handler_hsp base;	/**< Base handler instance. */
	const struct log_flush_handler *flush;	/**< Handler to flush SoC logs. */
};


int fatal_error_handler_hsp_manticore_init (struct fatal_error_handler_hsp_manticore *handler,
	const struct hsp_crashdump_handler *log, const struct cmd_device *crash,
	const struct log_flush_handler *flush);
void fatal_error_handler_hsp_manticore_release (
	const struct fatal_error_handler_hsp_manticore *handler);


#endif	/* FATAL_ERROR_HANDLER_HSP_MANTICORE_H_ */
