// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "heartbeat_led_handler.h"
#include "manticore_hsp_gpio.h"
#include "common/unused.h"


void heartbeat_led_handler_prepare (const struct periodic_task_handler *handler)
{
	handler->execute (handler);
}

const platform_clock* heartbeat_led_handler_get_next_execution (
	const struct periodic_task_handler *handler)
{
	const struct heartbeat_led_handler *heartbeat = (const struct heartbeat_led_handler*) handler;

	return &heartbeat->state->next;
}

void heartbeat_led_handler_execute (const struct periodic_task_handler *handler)
{
	const struct heartbeat_led_handler *heartbeat = (const struct heartbeat_led_handler*) handler;

	hsp_gpio_toggle (heartbeat->gpio, HEARTBEAT_LED);

	/* Toggle again in another second.  On Manticore, this call will not fail. */
	platform_init_timeout (1000, &heartbeat->state->next);
}

/**
 * Initialize a periodic handler for the DC-SCM heartbeat LED.
 *
 * @param handler The handler to initialize.
 * @param state Variable context for the LED handler.
 * @param gpio Driver for the HSP GPIOs.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int heartbeat_led_handler_init (struct heartbeat_led_handler *handler,
	struct heartbeat_led_handler_state *state, const struct hsp_gpio *gpio)
{
	if ((handler == NULL) || (state == NULL) || (gpio == NULL)) {
		return PERIODIC_TASK_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (*handler));

	handler->base.prepare = heartbeat_led_handler_prepare;
	handler->base.get_next_execution = heartbeat_led_handler_get_next_execution;
	handler->base.execute = heartbeat_led_handler_execute;

	handler->state = state;

	return 0;
}

/**
 * Release the resources used by the DC-SCM heartbeat LED handler.
 *
 * @param handler The handler to release.
 */
void heartbeat_led_handler_release (const struct heartbeat_led_handler *handler)
{
	UNUSED (handler);
}
