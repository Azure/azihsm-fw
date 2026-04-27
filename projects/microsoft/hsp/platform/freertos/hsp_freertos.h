// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_FREERTOS_H_
#define HSP_FREERTOS_H_

#include <stdbool.h>
#include "FreeRTOS.h"


int hsp_freertos_init ();

/* Implementation of the platform call for FreeRTOS on HSP. */
void freertos_isr_update_yield (BaseType_t priority_woken);

/* TODO: If an advanced syscall handler is implemented, such as to de/escalate execution privilege,
 * make sure to update required macros in portmacro.h (portYIELD, etc).  OSS uses an argument-less
 * syscall to unconditionally yield the task. */


#endif	// HSP_FREERTOS_H_
