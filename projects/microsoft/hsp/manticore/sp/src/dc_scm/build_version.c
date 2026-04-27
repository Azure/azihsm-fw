// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdio.h>
#include <string.h>
#include "build_version.h"
#include "logging/init_logging.h"
#include "logging/manticore_logging.h"


/**
 * Generate a string version for a specified build version number
 *
 * @param version The build version to convert.  This must be an array of 8 bytes.
 * @param secure Flag indicating if the firmware is booting securely.
 * @param fips Flag indicating if the firmware is FIPS certified.
 * @param str Output for the version string.
 * @param length Length of the output buffer.
 *
 * @return 0 if the version string was generated successfully or -1 if not.  On failure, the version
 * string will be truncated.
 */
int build_version_to_string (const uint8_t *version, bool secure, bool fips, char *str,
	size_t length)
{
	const char *ext_str = "";
	int str_length;

	switch (BUILD_VERSION_EXTENSION_TYPE (version)) {
		case BUILD_VERSION_EXTENSION_PROD:
			if (fips) {
				ext_str = "fips";
			}
			break;

		case BUILD_VERSION_EXTENSION_RC:
			ext_str = "rel";
			break;

		case BUILD_VERSION_EXTENSION_DEV:
			ext_str = "dev";
			break;

		case BUILD_VERSION_EXTENSION_BETA:
			ext_str = "beta";
			break;

		case BUILD_VERSION_EXTENSION_EVB:
			ext_str = "evb";
			break;

		case BUILD_VERSION_EXTENSION_REC:
			ext_str = "rec";
			break;

		case BUILD_VERSION_EXTENSION_TEST:
			ext_str = "test";
			break;

		default:
			break;
	}

	str_length = snprintf (str, length, "%u.%u.%u.%u-%lu%s%s", BUILD_VERSION_MAJOR (version),
		BUILD_VERSION_MINOR (version), BUILD_VERSION_RELEASE (version),
		BUILD_VERSION_BUILD (version), BUILD_VERSION_EXTENSION_NUMBER (version), ext_str,
		secure ? "" : "(X)");
	if (str_length < (int) length) {
		return 0;
	}
	else {
		/* In the error case, make sure the string is correctly terminated. */
		str[length - 1] = '\0';

		return -1;
	}
}

/**
 * Generate a debug log message for a specified build version number.
 *
 * @param version The build version to log.  This must be an array of 8 bytes.
 * @param secure Flag indicating if the firmware is booting securely.
 * @param fips Flag indicating if the firmware is FIPS certified.
 * @param service_indicator Service indicator string for the firmware execution context.
 */
void build_version_debug_log (const uint8_t *version, bool secure, bool fips,
	const char *service_indicator)
{
	uint32_t version_info;

	/* Bit 0 indicates secure booting. */
	version_info = secure ? (1U << 0) : 0;

	/* Bits 1:4 indicate extension type. */
	switch (BUILD_VERSION_EXTENSION_TYPE (version)) {
		case BUILD_VERSION_EXTENSION_PROD:
			version_info |= (2U << 1);
			break;

		case BUILD_VERSION_EXTENSION_RC:
			version_info |= (1U << 1);
			break;

		case BUILD_VERSION_EXTENSION_BETA:
			version_info |= (3U << 1);
			break;

		case BUILD_VERSION_EXTENSION_EVB:
			version_info |= (4U << 1);
			break;

		case BUILD_VERSION_EXTENSION_REC:
			version_info |= (8U << 1);
			break;

		case BUILD_VERSION_EXTENSION_TEST:
			version_info |= (12U << 1);
			break;

		default:
			break;
	}

	/* Bits 5:31 indicate the extension number. */
	version_info |= (BUILD_VERSION_EXTENSION_NUMBER (version) << 5);

	debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_INIT,
		INIT_LOGGING_FW_VERSION, BUILD_VERSION_FW_VERSION (version), version_info);

	/* The FIPS version extension and/or approved mode can't be encoded in the existing FW version
	 * log entry, so create a separate log message indicating the FIPS mode of operation. */
	debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
		MANTICORE_LOGGING_FIPS_MODE, fips, (strcmp ("FIPS", service_indicator) == 0));
}
