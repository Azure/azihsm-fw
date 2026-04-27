// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef LOGGING_PRODUCER_H_
#define LOGGING_PRODUCER_H_

#include <stdint.h>
#include "ring_buffer_state.h"


/**
 * Structure for the debug log ring buffer.
 */
struct logging_producer {
	void *mem_region_start;	/*< Start of the region to mapped */
	size_t mem_region_size;	/*< Size of the region in bytes */
	uint8_t component;
};


/**
 * IDs for MSFT components that generate log entries.
 */
enum {
	MSFT_LOGGING_COMPONENT_MANTICORE_SP = 0xf3,	/**< Log entry for Manticore SP firmware messages. */
	MSFT_LOGGING_COMPONENT_MANTICORE_CP0,		/**< Log entry for Manticore CP0 firmware messages. */
	MSFT_LOGGING_COMPONENT_MANTICORE_CP1,		/**< Log entry for Manticore CP1 firmware messages. */
	MSFT_LOGGING_COMPONENT_MANTICORE_FP0,		/**< Log entry for Manticore FP0 firmware messages. */
	MSFT_LOGGING_COMPONENT_MANTICORE_FP1,		/**< Log entry for Manticore FP1 firmware messages. */
	MSFT_LOGGING_COMPONENT_MANTICORE_FP2,		/**< Log entry for Manticore FP2 firmware messages. */
};


/**
 * Initialize the logging producer.
 *
 * @param log The logging producer to initialize.
 * @param mem_region_start Start of the memory region to map.
 * @param mem_region_size Size of the memory region to map.
 * @param component Component ID for the log entries.
 *
 * @return 0 on success.
 */
int logging_producer_init (struct logging_producer *log, void *mem_region_start,
	size_t mem_region_size,	uint8_t component);

/**
 * Sends a log entry to the ring buffer.
 *
 * @param log The ring buffer to send the log entry to.
 * @param severity Severity level of the log entry.
 * @param msg_index Index of the log message.
 * @param arg1 First argument for the log message.
 * @param arg2 Second argument for the log message.
 *
 * @return 0 on success.
 */
int logging_producer_send (const struct logging_producer *log, uint8_t severity, uint8_t msg_index,
	uint32_t arg1, uint32_t arg2);

#endif	/* LOGGING_PRODUCER_H_ */
