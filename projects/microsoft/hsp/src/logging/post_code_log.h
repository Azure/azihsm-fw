// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef POST_CODE_LOG_H_
#define POST_CODE_LOG_H_

#include <stdint.h>
#include "logging/logging.h"


/**
 * Global singleton for the post code log.
 */
#ifndef LOGGING_POST_CODE_LOG_CONST_INSTANCE
extern const struct logging *post_code_log;
#else
extern const struct logging *const post_code_log;
#endif

#pragma pack(push, 1)

/**
 * Format for an entry in the post code log.
 */
struct post_code_log_entry_info {
	uint32_t post_code;	/**< Post code Entry */
};

/**
 * Format of the post code log entry as stored in the log.
 */
struct post_code_log_entry {
	struct logging_entry_header header;		/**< Standard logging header. */
	struct post_code_log_entry_info entry;	/**< Information for the log entry. */
};

#pragma pack(pop)


int post_code_log_create_entry (uint32_t post_code);
#ifndef LOGGING_DISABLE_FLUSH
int post_code_log_flush (void);
#endif
int post_code_log_clear (void);
int post_code_log_get_size (void);
int post_code_log_read_contents (uint32_t offset, uint8_t *contents, size_t length);


#endif	/* POST_CODE_LOG_H_ */
