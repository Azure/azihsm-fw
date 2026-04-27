// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "logging_producer.h"
#include "manticore_logging_record.h"
#include "M7MemMap.h"

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
