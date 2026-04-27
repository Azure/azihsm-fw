// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef RING_BUFFER_STATE_H_
#define RING_BUFFER_STATE_H_

#include <stddef.h>
#include <string.h>
#include "status/manticore_module_id.h"
#include "status/rot_status.h"


/**
 * Macro library that implements the book keeping for an array based ring buffer using head and
 * tail indices. Supports operation for producer/consumer FIFO usage and overflow counter.
 * Ring buffer array must be allocated by the user.
 *
 * @note Macros that are named with the RB_ prefix should be considered internal to this module.
 */

#define RB_MIN_ARRAY_SIZE	(2)

struct ring_buffer_state {
	size_t buffer_size;	/**< Number of elements in buffer array. */
	size_t head;		/**< Index of head element */
	size_t tail;		/**< Index of tail element */
	uint32_t overflows;	/**< Recorded overflows. */
};

/**
 *  Ring buffer error code definition macro.
 */
#define	RING_BUFFER_ERROR(code) ROT_ERROR (MANTICORE_MODULE_CIRCULAR_QUEUE, code)

/**
 * Error codes that can be generated the ring buffer module
 */
enum {
	RING_BUFFER_INVALID_ARGUMENT = RING_BUFFER_ERROR (0x00),	/**< Invalid buffer parameter */
	RING_BUFFER_NO_MEMORY = RING_BUFFER_ERROR (0x01),			/**< Insufficient memory */
	RING_BUFFER_FULL = RING_BUFFER_ERROR (0x02),				/**< Invalid operation on a full ring buffer */
	RING_BUFFER_EMPTY = RING_BUFFER_ERROR (0x03),				/**< Invalid operation an empty ring buffer */
};


/**
 * Validates that the given ring buffer struct parameters are well formed.
 *
 * @param buffer Ring buffer pointer.
 * @return True if the buffer is valid. False otherwise.
 */
#define ring_buffer_is_valid(rb) ( \
	( \
		(rb != NULL) && ((rb)->head < (rb)->buffer_size) && ((rb)->tail < (rb)->buffer_size) && \
		((rb)->buffer_size >= RB_MIN_ARRAY_SIZE) \
	) \
)

/**
 * Runtime initialization of a ring_buffer structure of the given buffer_size.
 *
 * @param rb Ring buffer pointer.
 * @param _buffer_size Number of elements in the buffer array.
 * @returns 0 on success. RING_BUFFER_INVALID_ARGUMENT if the resultant buffer is invalid.
 */
#define ring_buffer_init(rb, _buffer_size) ( \
	(rb)->buffer_size = _buffer_size, \
	(rb)->head = 0, \
	(rb)->tail = 0, \
	(rb)->overflows = 0, \
	ring_buffer_is_valid (rb) ? ( 0 ) : ( \
		memset (rb, 0, sizeof(*(rb))), \
		RING_BUFFER_INVALID_ARGUMENT \
	) \
)

/**
 * Compile time initialization of both dynamic and constant portions of a static ring_buffer.
 *
 * @param _buffer_size Number of elements in the buffer array.
 */
#define ring_buffer_dynamic_static_init(_buffer_size) { \
	.buffer_size = _buffer_size, \
	.head = 0, \
	.tail = 0, \
	.overflows = 0, \
}

/**
 * Returns a index after the index parameter. If index is the last index in the array,
 * then 0 is returned.
 *
 * @param rb Ring buffer pointer.
 * @param index Element index.
 * @returns The next index after the provided index parameter
 */
#define ring_buffer_get_next_index(rb, index) ( \
	(size_t) ((((size_t) (index) + 1) >= (rb)->buffer_size) ? 0 : (size_t) (index) + 1) \
)

/**
 * Returns a index prior to the element index parameter. If the index parameter is the
 * first element in the array, then the last index in the array is returned.
 *
 * @param rb Ring buffer pointer.
 * @param index Element index.
 */
#define ring_buffer_get_prior_index(rb, index) ( \
	(size_t) (((size_t) (index) <= 0) ? (rb)->buffer_size - 1 : (size_t) (index) - 1) \
)

/**
 * Returns the maximum number of elements that the ring buffer can hold.
 *
 * @note This is the size of the array minus 1.
 * @param rb Ring buffer pointer.
 * @return Maximum number of elements the ring buffer can hold.
 */
#define ring_buffer_capacity(rb) ( \
	((rb)->buffer_size > 0) ? (rb)->buffer_size - 1 : 0 \
)

/**
 * Number of elements currently in the ring_buffer.
 *
 * @param rb Ring buffer pointer.
 * @return Number of elements currently in the buffer.
 */
#define ring_buffer_used(rb) ( \
	((rb)->buffer_size + ((rb)->tail - (rb)->head)) % (rb)->buffer_size \
)

/**
 * Number of elements that can be added to the ring buffer.
 *
 * @param rb Ring buffer pointer.
 * @return The number of elements that can be added to the ring buffer.
 */
#define ring_buffer_available(rb) ( \
	ring_buffer_capacity (rb) - ring_buffer_used (rb) \
)

/**
 * Check if the ring_buffer contains no elements.
 *
 * @param rb Ring buffer pointer.
 * @return true if the ring buffer contains no elements, false otherwise.
 */
#define ring_buffer_is_empty(rb) ( \
	(rb)->head == (rb)->tail \
)

/**
 * Check if the ring_buffer is full and cannot accept more elements.
 *
 * @param rb Ring buffer pointer.
 * @return True if the ring buffer cannot accept more elements, false otherwise.
 */
#define ring_buffer_is_full(rb) ( \
	ring_buffer_get_next_index (rb, (rb)->tail) == (rb)->head \
)

/**
 * Pushes the provided value onto the tail of the ring_buffer. Increments the no effect if the ring
 * buffer is full.
 *
 * @param rb Ring buffer pointer.
 * @param array Buffer array pointer
 * @param value Value to copy into the ring buffer.
 * @return 0 on success. RING_BUFFER_FULL if the buffer is full.
 */
#define ring_buffer_push_tail(rb, array, value) ( \
	(!ring_buffer_is_valid (rb)) || (array == NULL) ? ( \
		RING_BUFFER_INVALID_ARGUMENT \
	) : (\
		ring_buffer_is_full ((rb)) ? ( \
			(rb)->overflows++, \
			RING_BUFFER_FULL \
		) : ( \
			array[(rb)->tail] = (value), \
			/* Memory Barrier goes here (TBD) */ \
			(rb)->tail = ring_buffer_get_next_index (rb, (rb)->tail), \
			0 \
		) \
	) \
)


/**
 * Returns the tail element without changing the state of the buffer.
 * Has no effect if the buffer is empty.
 *
 * @param rb Ring buffer pointer.
 * @param array Buffer array pointer
 * @param value Variable to assign the tail element to.
 * @return 0 on success. RING_BUFFER_EMPTY if the buffer is empty.
 */
#define ring_buffer_peek_tail(rb, array, value) ( \
	(!ring_buffer_is_valid (rb)) || (array == NULL) ? ( \
		RING_BUFFER_INVALID_ARGUMENT \
	) : (\
		ring_buffer_is_empty (rb) ? ( \
			RING_BUFFER_EMPTY \
		) : ( \
			/* The tail index refers to the element after the current tail */ \
			value = array[ring_buffer_get_prior_index (rb, (rb)->tail)], \
			0 \
		) \
	) \
)

/**
 * Pops the current tail element off of the ring buffer. Has no effect if the buffer is empty.
 *
 * @param rb Ring buffer pointer.
 * @param array Buffer array pointer
 * @param value Variable to assign the tail element to.
 * @return 0 on success. RING_BUFFER_EMPTY if the buffer is empty.
 */
#define ring_buffer_pop_tail(rb, array, value) ( \
	(!ring_buffer_is_valid (rb)) || (array == NULL) ? ( \
		RING_BUFFER_INVALID_ARGUMENT \
	) : (\
		ring_buffer_is_empty (rb) ? ( \
			RING_BUFFER_EMPTY \
		) : (\
			/* The tail index refers to the element after the current tail */ \
			value = array[ring_buffer_get_prior_index (rb, (rb)->tail)], \
			(rb)->tail = ring_buffer_get_prior_index (rb, (rb)->tail), \
			0 \
		) \
	) \
)

/**
 * Pushes the provided element value onto the head of the ring_buffer.
 * Has no effect if the ring buffer is full.
 *
 * @param rb Ring buffer pointer.
 * @param array Buffer array pointer
 * @param value Value to copy into the ring buffer.
 * @return 0 on success. RING_BUFFER_FULL if the buffer is full.
 */
#define ring_buffer_push_head(rb, array, value) ( \
	(!ring_buffer_is_valid (rb)) || (array == NULL) ? ( \
		RING_BUFFER_INVALID_ARGUMENT \
	) : (\
		ring_buffer_is_full (rb) ? ( \
			(rb)->overflows++, \
			RING_BUFFER_FULL \
		) : ( \
			array[ring_buffer_get_prior_index (rb, (rb)->head)] = value, \
			/* Memory Barrier goes here (TBD) */ \
			(rb)->head = ring_buffer_get_prior_index (rb, (rb)->head), \
			0 \
		) \
	) \
)

/**
 * Copies the current head element to the provide pointer without changing the state of the buffer.
 * Has no effect if the buffer is empty.
 *
 * @param rb Ring buffer pointer.
 * @param array Buffer array pointer
 * @param value Variable to assign the tail element to.
 * @return 0 on success. RING_BUFFER_EMPTY if the buffer is empty.
 */
#define ring_buffer_peek_head(rb, array, value) ( \
	(!ring_buffer_is_valid (rb)) || (array == NULL) ? ( \
		RING_BUFFER_INVALID_ARGUMENT \
	) : (\
		ring_buffer_is_empty (rb) ? ( \
			RING_BUFFER_EMPTY \
		) : ( \
			value = array[(rb)->head], \
			0 \
		) \
	) \
)

/**
 * Pops the current head element off of the ring buffer.
 *
 * @param rb Ring buffer pointer.
 * @param array Buffer array pointer
 * @param value Variable to assign the tail element to.
 * @return 0 on success. RING_BUFFER_EMPTY if the buffer is empty.
 */
#define ring_buffer_pop_head(rb, array, value) ( \
	(!ring_buffer_is_valid (rb)) || (array == NULL) ? ( \
		RING_BUFFER_INVALID_ARGUMENT \
	) : (\
		ring_buffer_is_empty (rb) ? ( \
			RING_BUFFER_EMPTY \
		) : ( \
			value = array[(rb)->head], \
			(rb)->head = ring_buffer_get_next_index (rb, (rb)->head), \
			0 \
		) \
	) \
)

/**
 * Returns the number of overflows that have occurred on the ring buffer.
 *
 * @param rb Ring buffer pointer.
 */
#define ring_buffer_get_overflows(rb) ( \
	(rb)->overflows \
)

/**
 * Resets overflows on the ring buffer.
 *
 * @param rb Ring buffer pointer.
 */
#define ring_buffer_reset_overflows(rb) ( \
	(rb)->overflows = 0 \
)

#endif	/* RING_BUFFER_STATE_H_ */
