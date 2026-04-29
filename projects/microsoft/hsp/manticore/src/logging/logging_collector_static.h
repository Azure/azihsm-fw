// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef LOGGING_COLLECTOR_STATIC_H_
#define LOGGING_COLLECTOR_STATIC_H_

#include "logging/logging_collector.h"


/* Internal functions declared to allow for static initialization. */
int logging_collector_create_entry (const struct logging *logging, uint8_t *entry, size_t length);
int logging_collector_flush (const struct logging *logging);
int logging_collector_clear (const struct logging *logging);
int logging_collector_get_size (const struct logging *logging);
int logging_collector_read_contents (const struct logging *logging, uint32_t offset,
	uint8_t *contents, size_t length);


#ifdef LOGGING_DISABLE_FLUSH
#error "LOGGING_DISABLE_FLUSH cannot be defined when using remote core logging collector."
#endif

/**
 * Constant initializer for the logging API.
 */
#define	LOGGING_COLLECTOR_API_INIT  { \
		.create_entry = logging_collector_create_entry, \
		.flush = logging_collector_flush, \
		.clear = logging_collector_clear, \
		.get_size = logging_collector_get_size, \
		.read_contents = logging_collector_read_contents, \
	}


/**
 * Initialize a static instance of a logging collector.  This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the logging collector.
 * @param start_addr Start of the memory region to map.
 * @param block_size Size of the memory region to map.
 * @param dmb_ptr Pointer to the DMB instance to use for mapping the memory region.
 * @param id The ID of the collector.
 *
 * @return Constant instance of the logging collector.
 */
#define	logging_collector_static_init(state_ptr, start_addr, block_size, dmb_ptr, id)	{ \
		.base = LOGGING_COLLECTOR_API_INIT, \
		.state = state_ptr, \
		.mem_region_start = start_addr, \
		.mem_region_records = block_size, \
		.collector_id = id, \
		.dmb = dmb_ptr, \
	}


#endif	/* LOGGING_COLLECTOR_STATIC_H_ */
