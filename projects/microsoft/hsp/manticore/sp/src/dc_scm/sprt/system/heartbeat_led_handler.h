// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HEARTBEAT_LED_HANDLER_H_
#define HEARTBEAT_LED_HANDLER_H_

#include "drivers/hsp_gpio.h"
#include "system/periodic_task.h"


/**
 * Variable context for the LED handler.
 */
struct heartbeat_led_handler_state {
	platform_clock next;	/**< Time for the next execution. */
};

/**
 * Periodic handler for toggling the DC-SCM heartbeat LED.
 */
struct heartbeat_led_handler {
	struct periodic_task_handler base;			/**< Base handler API. */
	struct heartbeat_led_handler_state *state;	/**< Variable context for the handler. */
	const struct hsp_gpio *gpio;				/**< Driver for the HSP GPIOs */
};


int heartbeat_led_handler_init (struct heartbeat_led_handler *handler,
	struct heartbeat_led_handler_state *state, const struct hsp_gpio *gpio);
void heartbeat_led_handler_release (const struct heartbeat_led_handler *handler);


#endif	/* HEARTBEAT_LED_HANDLER_H_ */
