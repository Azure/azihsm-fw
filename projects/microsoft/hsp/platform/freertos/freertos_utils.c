// Copyright (c) Microsoft Corporation. All rights reserved.

#include "freertos_utils.h"
#include "hsp_freertos.h"
#include "trap/hsp_interrupt.h"


/**
 * Converts timeout value to FreeRTOS ticks
 *
 * @param timeout_ms Timeout in milliseconds, 0 if poll, or negative value for max delay
 *
 * @return FreeRTOS tick representation of timeout
 */
TickType_t freertos_utils_get_timeout (int timeout_ms)
{
	return (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS (timeout_ms);
}

/* Base pointer queue helpers */

/**
 * Allocates a FreeRTOS queue for pointers
 *
 * @param queue_ptr Pointer to the queue
 * @param count Count of elements the queue can hold
 *
 * @return NULL if not enough memory, else a handle to the queue
 */
QueueHandle_t freertos_utils_ptr_queue_alloc (size_t count)
{
	return xQueueCreate (count, sizeof (void*));
}

/**
 * Pushes a pointer to the queue
 *
 * @param queue Queue to push to
 * @param ptr Pointer to push to the queue
 * @param timeout_ms Timeout in milliseconds, 0 if poll, or negative value for max delay
 *
 * @return true if successful, else false if timedout
 */
bool freertos_utils_ptr_queue_push (QueueHandle_t queue, const void *ptr, int timeout_ms)
{
	BaseType_t status;
	TickType_t timeout;

	timeout = freertos_utils_get_timeout (timeout_ms);
	status = xQueueSendToBack (queue, &ptr, timeout);

	return (status == pdTRUE);
}

/**
 * Pushes a pointer to the queue from an ISR and updates the interrupt state
 *
 * @param queue Queue to push to
 * @param ptr Pointer to push to the queue
 *
 * @return true if successful, else false if full
 */
bool freertos_utils_ptr_queue_push_isr (QueueHandle_t queue, const void *ptr)
{
	BaseType_t status;
	BaseType_t priority_woken = pdFALSE;

	status = xQueueSendToBackFromISR (queue, &ptr, &priority_woken);
	freertos_isr_update_yield (priority_woken);

	return (status == pdTRUE);
}

/**
 * Pops a pointer from the queue
 *
 * @param queue Queue to pop from
 * @param ptr Pointer to read the popped value to
 * @param timeout_ms Timeout in milliseconds, 0 if poll, or negative value for max delay
 *
 * @return true if successful, else false if failure
 */
bool freertos_utils_ptr_queue_pop (QueueHandle_t queue, void **ptr, int timeout_ms)
{
	BaseType_t status;
	TickType_t timeout;

	timeout = freertos_utils_get_timeout (timeout_ms);
	status = xQueueReceive (queue, ptr, timeout);

	return (status == pdTRUE);
}

/**
 * Pops a pointer from the queue for an ISR and updates the interrupt state
 *
 * @param queue Queue to pop from
 * @param ptr Pointer to read the popped value to
 *
 * @return true if successful, else false if failure
 */
bool freertos_utils_ptr_queue_pop_isr (QueueHandle_t queue, void **ptr)
{
	BaseType_t status;
	BaseType_t priority_woken = pdFALSE;

	status = xQueueReceiveFromISR (queue, ptr, &priority_woken);
	freertos_isr_update_yield (priority_woken);

	return (status == pdTRUE);
}

/* Valid (Non-NULL) pointer queue helpers */

/**
 * Pushes a non-null pointer to the queue
 *
 * @param queue Queue to push to
 * @param ptr Non NULL pointer value to push
 * @param timeout_ms Timeout in milliseconds, 0 if poll, or negative value for max delay
 *
 * @return true if successful, else false if failure
 */
bool freertos_utils_valid_ptr_queue_push (QueueHandle_t queue, const void *ptr, int timeout_ms)
{
	bool status;

	if (ptr == NULL) {
		return false;
	}

	status = freertos_utils_ptr_queue_push (queue, ptr, timeout_ms);

	return status;
}

/**
 * Pushes a non-null pointer to the queue from an ISR and updates the interrupt state
 *
 * @param queue Queue to push to
 * @param ptr Non NULL pointer value to push
 *
 * @return true if successful, else false if failure
 */
bool freertos_utils_valid_ptr_queue_push_isr (QueueHandle_t queue, const void *ptr)
{
	bool status;

	if (ptr == NULL) {
		return false;
	}

	status = freertos_utils_ptr_queue_push_isr (queue, ptr);

	return status;
}

/**
 * Pops a valid pointer from the queue
 *
 * @param queue Queue to pop from
 * @param timeout_ms Timeout in milliseconds, 0 if poll, or negative value for max delay
 *
 * @return NULL if failure, else a valid pointer value
 */
void* freertos_utils_valid_ptr_queue_pop (QueueHandle_t queue, int timeout_ms)
{
	bool status;
	void *ptr;

	status = freertos_utils_ptr_queue_pop (queue, &ptr, timeout_ms);
	if (!status) {
		ptr = NULL;
	}

	return ptr;
}

/**
 * Pops a valid pointer from the queue from an ISR and updates the interrupt state
 *
 * @param queue Queue to pop from
 *
 * @return NULL if failure, else a valid pointer value
 */
void* freertos_utils_valid_ptr_queue_pop_isr (QueueHandle_t queue)
{
	bool status;
	void *ptr;

	status = freertos_utils_ptr_queue_pop_isr (queue, &ptr);
	if (!status) {
		ptr = NULL;
	}

	return ptr;
}
