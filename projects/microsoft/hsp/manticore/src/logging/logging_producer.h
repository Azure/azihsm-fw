// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef LOGGING_PRODUCER_H_
#define LOGGING_PRODUCER_H_

#include <stdint.h>
#include "logging/ring_buffer_state.h"


/**
 * Structure for the debug log ring buffer.
 */
struct logging_producer {
	void *mem_region_start;	/*< Start of the region to mapped */
	size_t mem_region_size;	/*< Size of the region in bytes */
	uint8_t component;		/*< The component value for debug_log_create_entry() */
};


int logging_producer_init (struct logging_producer *log, void *mem_region_start,
	size_t mem_region_size,	uint8_t component);
int logging_producer_send (const struct logging_producer *log, uint8_t severity, uint8_t msg_index,
	uint32_t arg1, uint32_t arg2);


#endif	/* LOGGING_PRODUCER_H_ */
