// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_LOGGING_RECORD_H_
#define MANTICORE_LOGGING_RECORD_H_

#include <stdint.h>

/**
 * Data object definition for debug log entries used by other Manticore cores (CP and FP).
 */
struct manticore_logging_record {
	uint8_t severity;	/*< The severity value for debug_log_create_entry() */
	uint8_t component;	/*< The component value for debug_log_create_entry() */
	uint8_t msg_index;	/*< The msg_index value for debug_log_create_entry() */
	uint8_t _reserved;	/*< Reserved for future use */
	uint32_t arg1;		/*< The arg1 value for debug_log_create_entry() */
	uint32_t arg2;		/*< The arg2 value for debug_log_create_entry() */
};


#endif	/* MANTICORE_LOGGING_RECORD_H_ */
