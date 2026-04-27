// Copyright (c) Microsoft Corporation. All rights reserved.

#include "platform_io_api.h"
#include "rom_logging.h"
#include "logging/boot_status_log.h"


/**
 * Cache for the last ROM error.
 */
static uint8_t last_fail_id = 0;

/**
 * Cache for the detailed error code of the last ROM error.
 */
static uint32_t last_error_code = 0;


/**
 * Log a ROM error.
 *
 * @param fail_id Identifier for the type of failure.
 * @param error_code Detailed error code.
 */
void rom_logging_error (uint8_t fail_id, uint32_t error_code)
{
	rom_logging_print_error (fail_id, error_code);

#ifdef LOGGING_SUPPORT_BOOT_STATUS_LOG
	boot_status_log_create_entry (fail_id, error_code);
#endif

	debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP_ROM, fail_id,
		error_code, 0);
}

/**
 * Print a ROM error without generating a debug log entry.
 *
 * @param fail_id Identifier for the type of failure.
 * @param error_code Detailed error code.
 */
void rom_logging_print_error (uint8_t fail_id, uint32_t error_code)
{
	rom_logging_set_last_error (fail_id, error_code);

	platform_printf ("%x:%x" NEWLINE, fail_id, error_code);
}

/**
 * Set details for the last ROM error condition.  This will not generate any logging or other error
 * indicators.
 *
 * @param fail_id Identifier for the type of failure.
 * @param error_code Detailed error code.
 */
void rom_logging_set_last_error (uint8_t fail_id, uint32_t error_code)
{
	last_fail_id = fail_id;
	last_error_code = error_code;
}

/**
 * Get error details for the last logged error.
 *
 * @param last Outut for the last error information.
 */
void rom_logging_get_last_error (struct ocp_recovery_device_status_vendor *last)
{
	if (last) {
		last->failure_id = last_fail_id;
		last->error_code = last_error_code;
	}
}
