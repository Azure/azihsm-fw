// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include "common/unused.h"
#include "drivers/hsp_dmb.h"
#include "logging/debug_log.h"
#include "logging/logging_collector.h"
#include "logging/manticore_logging.h"
#include "logging/ring_buffer_state.h"


/**
 * Maps the memory region and returns a pointer to the local HSP address
 *
 * @param collector The logging collector instance.
 * @param rb Reference to pointer to the ring buffer state.
 *
 * @return 0 on success.
 */
static int logging_collector_map_mem (const struct logging_collector *collector,
	struct ring_buffer_state **rb)
{
	int status;
	void *hsp_addr;

	status = collector->dmb->map_soc_address (collector->dmb, collector->mem_region_start,
		sizeof (struct ring_buffer_state) + collector->mem_region_records *
		sizeof (struct manticore_logging_record), HSP_DMB_ACCESS_WRITE, &hsp_addr);
	if (status == 0) {
		*rb = (struct ring_buffer_state*) hsp_addr;
	}
	else {
		*rb = NULL;
	}

	return status;
}

/**
 * Unmaps the memory region referenced by the pointer provided.
 *
 * @param collector The logging collector instance.
 * @param rb Pointer to the ring buffer state.
 */
static void logging_collector_unmap_mem (const struct logging_collector *collector,
	struct ring_buffer_state *rb)
{
	collector->dmb->unmap_soc_address (collector->dmb, rb);
}

int logging_collector_create_entry (const struct logging *logging, uint8_t *entry, size_t length)
{
	UNUSED (logging);
	UNUSED (entry);
	UNUSED (length);

	/* No op. */
	return 0;
}

int logging_collector_clear (const struct logging *logging)
{
	UNUSED (logging);

	/* No op. */
	return 0;
}

int logging_collector_get_size (const struct logging *logging)
{
	UNUSED (logging);

	/* No op. */
	return 0;
}

int logging_collector_read_contents (const struct logging *logging, uint32_t offset,
	uint8_t *contents, size_t length)
{
	UNUSED (logging);
	UNUSED (offset);
	UNUSED (contents);
	UNUSED (length);

	/* No op. */
	return 0;
}

static void logging_collector_state_handler (const struct logging_collector *collector, int status)
{
	switch (collector->state->current_state) {
		case LOGGING_COLLECTOR_INITIALIZED:
			if (status == 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_LOG_COLLECTOR_RUNNING, collector->collector_id, 0);
				collector->state->current_state = LOGGING_COLLECTOR_RUNNING;
			}
			else {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_LOG_COLLECTOR_FAULTED, collector->collector_id, status);
				collector->state->current_state = LOGGING_COLLECTOR_FAULTED;
			}
			break;

		case LOGGING_COLLECTOR_RUNNING:
			if (status != 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_LOG_COLLECTOR_FAULTED, collector->collector_id, status);
				collector->state->current_state = LOGGING_COLLECTOR_FAULTED;
			}
			break;

		case LOGGING_COLLECTOR_FAULTED:
			if (status == 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_LOG_COLLECTOR_RUNNING, collector->collector_id, 0);
				collector->state->current_state = LOGGING_COLLECTOR_RUNNING;
			}
			break;
	}
}

int logging_collector_flush (const struct logging *logging)
{
	int status;
	uint32_t max_drain_count;
	uint32_t sender_overflows;
	struct ring_buffer_state *rb;
	struct manticore_logging_record *array;
	const struct logging_collector *collector = (const struct logging_collector*) logging;

	if (collector == NULL) {
		return LOGGING_COLLECTOR_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&collector->state->lock);
	status = logging_collector_map_mem (collector, &rb);

	if (status != 0) {
		logging_collector_state_handler (collector, status);
		platform_mutex_unlock (&collector->state->lock);

		return status;
	}

	if (!ring_buffer_is_valid (rb)) {
		status = LOGGING_COLLECTOR_INVALID_DESCRIPTOR;
	}
	else if ((rb->buffer_size != collector->mem_region_records)) {
		status = LOGGING_COLLECTOR_UNEXPECTED_NUMBER_OF_RECORDS;
	}

	logging_collector_state_handler (collector, status);

	if (status != 0) {
		logging_collector_unmap_mem (collector, rb);
		platform_mutex_unlock (&collector->state->lock);

		return status;
	}

	array = (struct manticore_logging_record*) (rb + 1);

	max_drain_count = ring_buffer_capacity (rb);
	while ((max_drain_count > 0) && !ring_buffer_is_empty (rb)) {
		struct manticore_logging_record log_record;

		if (ring_buffer_pop_head (rb, array, log_record) == 0) {
			debug_log_create_entry (log_record.severity, log_record.component, log_record.msg_index,
				log_record.arg1, log_record.arg2);
		}
		max_drain_count--;
	}

	sender_overflows = ring_buffer_get_overflows (rb);

	if (sender_overflows > collector->state->logged_overflows) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_LOG_COLLECTOR_OVERFLOW_DETECTED, collector->collector_id,
			sender_overflows - collector->state->logged_overflows);
		collector->state->logged_overflows = sender_overflows;
	}

	logging_collector_unmap_mem (collector, rb);
	platform_mutex_unlock (&collector->state->lock);

	return status;
}

/**
 * Initialize the logging collector.
 *
 * @param collector The collector to initialize.
 * @param state The state information for the collector.  This must be uninitialized.
 * @param mem_region_start The start of the memory region to map.
 * @param mem_region_records The size of the memory region to map in records.
 * @param dmb The DMB instance to use for mapping the memory region.
 * @param collector_id The ID of the sender for diagnostic purposes.
 *
 * @return 0 if the collector was successfully initialized or an error code.
 */
int logging_collector_init (struct logging_collector *collector,
	struct logging_collector_state *state, uint64_t mem_region_start, size_t mem_region_records,
	const struct hsp_dmb *dmb, uint32_t collector_id)
{
	if (collector == NULL) {
		return LOGGING_COLLECTOR_INVALID_ARGUMENT;
	}

	collector->base.create_entry = logging_collector_create_entry;
	collector->base.flush = logging_collector_flush;
	collector->base.clear = logging_collector_clear;
	collector->base.get_size = logging_collector_get_size;
	collector->base.read_contents = logging_collector_read_contents;
	collector->dmb = dmb;
	collector->collector_id = collector_id;
	collector->mem_region_start = mem_region_start;
	collector->mem_region_records = mem_region_records;
	collector->state = state;

	return logging_collector_init_state (collector);
}

/**
 * Initialize the state for the logging collector.
 */
int logging_collector_init_state (const struct logging_collector *collector)
{
	int status;

	if ((collector == NULL) || (collector->state == NULL) || (collector->dmb == NULL) ||
		(collector->mem_region_start == 0) || (collector->mem_region_records == 0)) {
		return LOGGING_COLLECTOR_INVALID_ARGUMENT;
	}

	memset (collector->state, 0, sizeof (struct logging_collector_state));

	status = platform_mutex_init (&collector->state->lock);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Release the resources used by the logging collector.  The contents of the memory remains
 * unaffected.
 *
 * @param collector The collector to release.
 */
void logging_collector_release (const struct logging_collector *collector)
{
	if (collector) {
		platform_mutex_free (&collector->state->lock);
	}
}
