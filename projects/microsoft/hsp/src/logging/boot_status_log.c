// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stdint.h>
#include "boot_status_log.h"
#include "platform_api.h"
#include "common/buffer_util.h"
#include "common/unused.h"
#include "firmware/hsp_fw_1sp.h"
#include "rom/rom_logging.h"


/* By default, the global singleton for the boot status log is defined here.  However, if
 * the target project wants to define this to be a constant instance, it needs to be defined and
 * initialized in that scope. */
#ifndef LOGGING_BOOT_STATUS_LOG_CONST_INSTANCE
const struct logging *boot_status_log = NULL;
const struct boot_status_gpio_interface *boot_status_gpio = NULL;
#endif

/**
 * Provide boot status mapping with post code.
 *
 * @param post_code post code to be mapped with boot status.
 *
 * @return boot status, unmapped or a mapped boot status.
 */
static uint8_t get_boot_status_mapped_to_post_code_entry (uint32_t post_code)
{
	switch (post_code) {
		case ROM_LOGGING_TRACE_START:
			return BOOT_STATUS_HSP_ROM_START;

		case ROM_LOGGING_TRACE_JTAG_MESSAGE:
			return BOOT_STATUS_HSP_ROM_JTAG_MSG;

		case ROM_LOGGING_TRACE_VERIFY_MANIFEST:
			return BOOT_STATUS_HSP_ROM_VERIFY_ROOT_MANIFEST;

		case ROM_LOGGING_TRACE_LOAD_IMAGE_SLOT_A:
			return BOOT_STATUS_HSP_ROM_FETCH_SP1_SLOTA;

		case ROM_LOGGING_TRACE_LOAD_IMAGE_SLOT_B:
			return BOOT_STATUS_HSP_ROM_FETCH_SP1_SLOTB;

		case ROM_LOGGING_TRACE_VERIFY_1SP_HEADER:
			return BOOT_STATUS_HSP_ROM_VERIFY_SP1;

		case ROM_LOGGING_TRACE_JUMP_TO_1SP:
			return BOOT_STATUS_HSP_ROM_JUMP_TO_SP1;

		case ROM_LOGGING_TRACE_ENTER_RECOVERY:
		case ROM_LOGGING_TRACE_ENTER_RECOVERY_STATUS:
			return BOOT_STATUS_HSP_ROM_RECOVERY;

		case ROM_LOGGING_TRACE_HALT:
			return BOOT_STATUS_HSP_ROM_HALT;

		default:
			return BOOT_STATUS_UNMAPPED_POSTCODE;
	}
}

/**
 * Provide boot status mapping with error code.
 *
 * @param fail_id fail id to be mapped with boot status.
 * @param errcode errcode to be mapped with boot status.
 *
 * @return boot status, unknown error if unmapped or a mapped boot status.
 */
static uint8_t get_boot_status_mapped_to_err_code_entry (uint8_t fail_id, uint32_t errcode)
{
	switch (errcode) {
		case HSP_FW_1SP_BAD_IMAGE_MARKER:
			return BOOT_STATUS_HSP_ROM_SP1_INVALID_FIRMWARE_ID;
	}

	switch (fail_id) {
		case ROM_LOGGING_FAIL_SVN_UPDATE:
			return BOOT_STATUS_HSP_ROM_1SP_REVOCATION_FAILED;

		case ROM_LOGGING_FAIL_SLOT_A:
		case ROM_LOGGING_FAIL_SLOT_B:
			return BOOT_STATUS_HSP_ROM_FETCH_SP1_FAILED;
	}

	return BOOT_STATUS_HSP_ROM_UNKNOWN_ERROR;
}

/**
 * Provide logging code mapping with boot status.
 *
 * @param fail_id fail id to be mapped with boot status.
 * @param logging_code logging code to be mapped with boot status.
 *
 * @return boot status, mapped boot status.
 */
static uint8_t boot_status_get_mapped_log_entry (uint8_t fail_id, uint32_t logging_code)
{
	if (logging_code & ROT_ERROR_MARKER) {
		return get_boot_status_mapped_to_err_code_entry (fail_id, logging_code);
	}
	else {
		return get_boot_status_mapped_to_post_code_entry (logging_code);
	}
}

/**
 * Create a new entry in the boot status log.
 *
 * @param fail_id fail id to be mapped with boot status.
 * @param logging_code logging code to be mapped with boot status.
 *
 * @return Completion status, 0 if success or an error code.
 */
int boot_status_log_create_entry (uint8_t fail_id, uint32_t logging_code)
{
	struct boot_status_log_entry_info entry;
	uint8_t boot_status;

	if (boot_status_log == NULL) {
		return LOGGING_NO_LOG_AVAILABLE;
	}

	boot_status = boot_status_get_mapped_log_entry (fail_id, logging_code);
	if (boot_status) {
		entry.boot_status = boot_status;
		if (boot_status_gpio != NULL) {
			boot_status_gpio->write (boot_status_gpio,
				MAKE_GPIO_BOOT_STATUS (BOOT_STATUS_SRC_HSP, boot_status));
		}

		return boot_status_log->create_entry (boot_status_log, (uint8_t*) &entry, sizeof (entry));
	}
	else {
		return LOGGING_NO_LOG_AVAILABLE;
	}
}

#ifndef LOGGING_DISABLE_FLUSH
/**
 * Flush any buffered contents of the boot status log.
 *
 * @return Completion status, 0 if success or an error code.
 */
int boot_status_log_flush ()
{
	if (boot_status_log == NULL) {
		return LOGGING_NO_LOG_AVAILABLE;
	}

	return boot_status_log->flush (boot_status_log);
}
#endif

/**
 * Remove all entries from the boot status log.
 *
 * @return Completion status, 0 if success or an error code.
 */
int boot_status_log_clear ()
{
	if (boot_status_log == NULL) {
		return LOGGING_NO_LOG_AVAILABLE;
	}

	return boot_status_log->clear (boot_status_log);
}

/**
 * Get the total size of all entries in the boot status log.
 *
 * @return The size of the boot status log or an error code.  Use ROT_IS_ERROR to check the return value.
 */
int boot_status_log_get_size ()
{
	if (boot_status_log == NULL) {
		return LOGGING_NO_LOG_AVAILABLE;
	}

	return boot_status_log->get_size (boot_status_log);
}

/**
 * Read entry data from the boot status log.
 *
 * @param offset Offset within the log to start reading data.
 * @param contents Output buffer for the log contents.
 * @param length Maximum number of bytes to read from the log.
 *
 * @return The number of bytes read from the log or an error code.  Use ROT_IS_ERROR to check the
 * return value.
 */
int boot_status_log_read_contents (uint32_t offset, uint8_t *contents, size_t length)
{
	if (boot_status_log == NULL) {
		return LOGGING_NO_LOG_AVAILABLE;
	}

	return boot_status_log->read_contents (boot_status_log, offset, contents, length);
}
