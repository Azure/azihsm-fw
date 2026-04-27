// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HEARTBEAT_LED_HANDLER_STATIC_H_
#define HEARTBEAT_LED_HANDLER_STATIC_H_

#include "heartbeat_led_handler.h"


/* Internal functions declared to allow for static initialization. */
void heartbeat_led_handler_prepare (const struct periodic_task_handler *handler);
const platform_clock* heartbeat_led_handler_get_next_execution (
	const struct periodic_task_handler *handler);
void heartbeat_led_handler_execute (const struct periodic_task_handler *handler);


/**
 * Constant initializer for the periodic handler API.
 */
#define	HEARTBEAT_LED_HANDLER_API_INIT	{ \
		.prepare = heartbeat_led_handler_prepare, \
		.get_next_execution = heartbeat_led_handler_get_next_execution, \
		.execute = heartbeat_led_handler_execute, \
	}


/**
 * Initialize a static handler for toggling the DC-SCM heartbeat LED.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the LED handler.
 * @param gpio_ptr Driver for the HSP GPIOs.
 */
#define	heartbeat_led_handler_static_init(state_ptr, gpio_ptr) { \
		.base = HEARTBEAT_LED_HANDLER_API_INIT, \
		.state = state_ptr, \
		.gpio = gpio_ptr, \
	}


#endif	/* HEARTBEAT_LED_HANDLER_STATIC_H_ */
