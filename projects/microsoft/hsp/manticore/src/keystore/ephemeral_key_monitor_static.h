// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef EPHEMERAL_KEY_MONITOR_STATIC_H_
#define EPHEMERAL_KEY_MONITOR_STATIC_H_

#include "ephemeral_key_monitor.h"


/* Internal functions declared to allow for static initialization. */
void ephemeral_key_monitor_prepare (const struct periodic_task_handler *handler);
const platform_clock* ephemeral_key_monitor_get_next_execution (
	const struct periodic_task_handler *handler);
void ephemeral_key_monitor_execute (const struct periodic_task_handler *handler);


/**
 * Constant initializer for the ephemeral key monitor handler API.
 */
#define	EPHEMERAL_KEY_MONITOR_HANDLER_API_INIT  { \
		.prepare = ephemeral_key_monitor_prepare, \
		.get_next_execution = ephemeral_key_monitor_get_next_execution, \
		.execute = ephemeral_key_monitor_execute, \
	}


/**
 * Initialize a static instance of an ephemeral key monitor.  This does not initialize
 * the handler state.  This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the handler.
 * @param dmb_ptr The DMB interface used to map GSRAM slot windows.
 * @param key_cache_ptr Flash-backed key cache feeding refill attempts.
 * @param key_buffer_ptr Working buffer: DER key in, PKA-LE payload out.
 * @param key_buffer_len_val Length of the working buffer, in bytes.
 * @param period_ms_val The time between scan ticks, in milliseconds.
 * @param part_persistent_store_base_val The base address of the HSM Part Persistent Store.
 */
#define	ephemeral_key_monitor_static_init(state_ptr, dmb_ptr, key_cache_ptr, key_buffer_ptr, key_buffer_len_val, period_ms_val, part_persistent_store_base_val)  { \
		.base = EPHEMERAL_KEY_MONITOR_HANDLER_API_INIT, \
		.state = state_ptr, \
		.dmb = dmb_ptr, \
		.key_cache = key_cache_ptr, \
		.key_buffer = key_buffer_ptr, \
		.key_buffer_len = key_buffer_len_val, \
		.period_ms = period_ms_val, \
		.part_persistent_store_base = part_persistent_store_base_val, \
	}


#endif	/* EPHEMERAL_KEY_MONITOR_STATIC_H_ */
