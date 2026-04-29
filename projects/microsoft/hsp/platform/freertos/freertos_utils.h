// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FREERTOS_UTILS_H_
#define FREERTOS_UTILS_H_

#include <stdbool.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"


TickType_t freertos_utils_get_timeout (int timeout_ms);

QueueHandle_t freertos_utils_ptr_queue_alloc (size_t count);

bool freertos_utils_ptr_queue_push (QueueHandle_t queue, const void *ptr, int timeout_ms);
bool freertos_utils_ptr_queue_push_isr (QueueHandle_t queue, const void *ptr);
bool freertos_utils_ptr_queue_pop (QueueHandle_t queue, void **ptr, int timeout_ms);
bool freertos_utils_ptr_queue_pop_isr (QueueHandle_t queue, void **ptr);

bool freertos_utils_valid_ptr_queue_push (QueueHandle_t queue, const void *ptr, int timeout_ms);
bool freertos_utils_valid_ptr_queue_push_isr (QueueHandle_t queue, const void *ptr);
void* freertos_utils_valid_ptr_queue_pop (QueueHandle_t queue, int timeout_ms);
void* freertos_utils_valid_ptr_queue_pop_isr (QueueHandle_t queue);


#endif	/* FREERTOS_UTILS_H_ */
