// Copyright (c) Microsoft Corporation. All rights reserved.

#include "init_crashdump.h"
#include "init_error.h"
#include "init_log_flush_handlers.h"
#include "init_system.h"
#include "system/fatal_error_handler_hsp_manticore_static.h"


/**
 * Fatal error handler for Manticore SPRT.
 */
static const struct fatal_error_handler_hsp_manticore manticore_fatal =
	fatal_error_handler_hsp_manticore_static_init (&hsp_crashdump_handler, &device_cmd.base.base,
	&log_flush);

/* Global singleton for fatal error handling. */
const struct fatal_error_handler *const fatal_error = &manticore_fatal.base.base;
