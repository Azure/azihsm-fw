// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include <string.h>
#include "build_version.h"
#include "hsp_top.h"
#include "init_flash.h"
#include "init_log.h"
#include "init_system.h"
#include "platform_io_api.h"
#include "rot_memory_map.h"
#include "sp_boot.h"
#include "1sp/manticore_1sp.h"
#include "logging/init_logging.h"
#include "logging/manticore_logging.h"
#include "system/real_time_clock_hsp_static.h"


/**
 * Variable context for the system RTC.
 */
static struct real_time_clock_hsp_state system_rtc_state;

/**
 * RTC used by the system.
 */
const struct real_time_clock_hsp system_rtc = real_time_clock_hsp_static_init (&system_rtc_state,
	(struct Creg_regs_creg_rtc_group*) HSP_ADDR_MAP_CREG_RTC_REGS_ADDRESS, 5000);

/**
 * Variable context for the debug log.
 */
static struct logging_flash_state debug_log_context;

/**
 * Flash logger for storing the debug log.
 */
const struct logging_flash debug_logger = logging_flash_static_init (&debug_log_context,
	&flash_external, DEBUG_LOGGING_ADDR);


/**
 * Initialize the debug log and populate it with boot-time log entries.
 */
void initialize_debug_log ()
{
	int status;

	status = real_time_clock_hsp_init_state (&system_rtc);
	if (status == 0) {
		/* This will get configured as RO after initialization has completed. */
		debug_timestamp = &system_rtc.base;
	}
	else {
		platform_printf ("SPRT: Failed to initialize system RTC: 0x%x" NEWLINE, status);
	}

	status = logging_flash_init_state (&debug_logger);
	if (status == 0) {
		/* This will get configured as RO after initialization has completed. */
		debug_log = &debug_logger.base;
	}
	else {
		platform_printf ("SPRT: Failed to initialize debug log: 0x%x" NEWLINE, status);
	}

	debug_log_create_entry ((recovery_boot) ? DEBUG_LOG_SEVERITY_WARNING : DEBUG_LOG_SEVERITY_INFO,
		DEBUG_LOG_COMPONENT_INIT, INIT_LOGGING_BOOT_SOURCE, recovery_boot, reset_source);

	build_version_debug_log (manticore_firmware_descriptor_get_build_version (
		&sp1_shared->fw_descriptor), is_secure_boot_enabled (),
		(manticore_firmware_descriptor_fips_certified (&sp1_shared->fw_descriptor) != 0),
		sp1_shared->service_indicator);

	if (recovery_boot && has_updated_impactful_firmware ()) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_IMPACTFUL_RECOVERY_BOOT, 0, 0);
	}

	log_flash_device_info (&flash_internal, INIT_FLASH_CERBERUS_MAIN);
	log_flash_device_info (&flash_external, INIT_FLASH_CERBERUS_RECOVERY);

	debug_log_flush ();
}
