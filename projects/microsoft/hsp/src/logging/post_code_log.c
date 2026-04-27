// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stdint.h>
#include "platform_api.h"
#include "post_code_log.h"
#include "common/buffer_util.h"
#include "common/unused.h"


/* By default, the global singleton for the post code log is defined here.  However, if
 * the target project wants to define this to be a constant instance, it needs to be defined and
 * initialized in that scope. */
#ifndef LOGGING_POST_CODE_LOG_CONST_INSTANCE
const struct logging *post_code_log = NULL;
#endif

/**
 * Create a new entry in the post code log.
 *
 * @param post_code post code of the new entry.
 *
 * @return Completion status, 0 if success or an error code.
 */
int post_code_log_create_entry (uint32_t post_code)
{
	struct post_code_log_entry_info entry;

	if (post_code_log == NULL) {
		return LOGGING_NO_LOG_AVAILABLE;
	}

	buffer_unaligned_write32 (&entry.post_code, post_code);

	return post_code_log->create_entry (post_code_log, (uint8_t*) &entry, sizeof (entry));
}

#ifndef LOGGING_DISABLE_FLUSH
/**
 * Flush any buffered contents of the post code log.
 *
 * @return Completion status, 0 if success or an error code.
 */
int post_code_log_flush ()
{
	if (post_code_log == NULL) {
		return LOGGING_NO_LOG_AVAILABLE;
	}

	return post_code_log->flush (post_code_log);
}
#endif

/**
 * Remove all entries from the post code log.
 *
 * @return Completion status, 0 if success or an error code.
 */
int post_code_log_clear ()
{
	if (post_code_log == NULL) {
		return LOGGING_NO_LOG_AVAILABLE;
	}

	return post_code_log->clear (post_code_log);
}

/**
 * Get the total size of all entries in the post code log.
 *
 * @return The size of the post code log or an error code.  Use ROT_IS_ERROR to check the return value.
 */
int post_code_log_get_size ()
{
	if (post_code_log == NULL) {
		return LOGGING_NO_LOG_AVAILABLE;
	}

	return post_code_log->get_size (post_code_log);
}

/**
 * Read entry data from the post code log.
 *
 * @param offset Offset within the log to start reading data.
 * @param contents Output buffer for the log contents.
 * @param length Maximum number of bytes to read from the log.
 *
 * @return The number of bytes read from the log or an error code.  Use ROT_IS_ERROR to check the
 * return value.
 */
int post_code_log_read_contents (uint32_t offset, uint8_t *contents, size_t length)
{
	if (post_code_log == NULL) {
		return LOGGING_NO_LOG_AVAILABLE;
	}

	return post_code_log->read_contents (post_code_log, offset, contents, length);
}
