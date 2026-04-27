// Copyright (c) Microsoft Corporation. All rights reserved.

#include "logging_producer.h"
#include "logging/manticore_logging_record.h"


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
	size_t mem_region_size,	uint8_t component)
{
	if ((log == NULL) || (mem_region_start == NULL) || (mem_region_size == 0)) {
		return -1;
	}

	log->mem_region_start = mem_region_start;
	log->mem_region_size = mem_region_size;
	log->component = component;

	struct ring_buffer_state *rb =
		(struct ring_buffer_state*) (log->mem_region_start);

	// (Re)initialize the ring buffer state on PoR or recover it if corrupted after reset.
	if (!ring_buffer_is_valid (rb) || (rb->buffer_size != log->mem_region_size)) {
		return ring_buffer_init (rb, log->mem_region_size);
	}

	return 0;
}

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
	uint32_t arg1, uint32_t arg2)
{
	struct ring_buffer_state *rb =
		(struct ring_buffer_state*) (log->mem_region_start);
	struct manticore_logging_record *array = (struct manticore_logging_record*) (rb + 1);

	struct manticore_logging_record params = {
		.severity = severity,
		.component = log->component,
		.msg_index = msg_index,
		.arg1 = arg1,
		.arg2 = arg2,
	};

	return ring_buffer_push_tail (rb, array, params);
}
