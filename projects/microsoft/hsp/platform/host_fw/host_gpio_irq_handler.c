// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "host_gpio_irq_handler.h"
#include "common/unused.h"
#include "drivers/hsp_gpio.h"
#include "host_fw/host_irq_control.h"


bool host_gpio_irq_handler_handle_interrupt (const struct hsp_interrupt_handler *handler,
	uintptr_t param)
{
	const struct host_gpio_irq_handler *irq = (const struct host_gpio_irq_handler*) handler;
	const struct hsp_gpio *gpio = (const struct hsp_gpio*) param;
	uint8_t irq_type;
	int status;

	status = hsp_gpio_get_irq_status (gpio, irq->gpio_num, &irq_type);
	if (status != 0) {
		return false;
	}

	if (irq_type & HSP_GPIO_IRQ_FALLING_EDGE) {
		host_gpio_irq_event_manager_send_notification (irq->notify, irq->event_falling);
	}

	if (irq_type & HSP_GPIO_IRQ_RISING_EDGE) {
		host_gpio_irq_event_manager_send_notification (irq->notify, irq->event_rising);
	}

	status = hsp_gpio_clear_irq_status (gpio, irq->gpio_num, irq_type);
	if (status != 0) {
		return false;
	}

	return true;
}

/**
 * Initialize a handler for interrupts from a single HSP GPIO.  This will not register with or
 * enable any interrupts.
 *
 * @param handler The interrupt handler to initialize.
 * @param irq_notify Notification manager for HSP GPIO interrupts.
 * @param gpio GPIO number whose interrupts will be handled.
 * @param on_rising_edge Event identifier that should be generated on a rising edge interrupt.
 * @param on_falling_edge Event identifier that should be generated on a falling edge interrupt.
 *
 * @return 0 if the IRQ handler was initialized successfully or an error code.
 */
int host_gpio_irq_handler_init (struct host_gpio_irq_handler *handler,
	struct host_gpio_irq_event_manager *irq_notify, uint8_t gpio, uint8_t on_rising_edge,
	uint8_t on_falling_edge)
{
	if ((handler == NULL) || (irq_notify == NULL)) {
		return HOST_IRQ_CTRL_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (struct host_gpio_irq_handler));

	handler->base.handle_interrupt = host_gpio_irq_handler_handle_interrupt;

	handler->notify = irq_notify;
	handler->gpio_num = gpio;
	handler->event_rising = on_rising_edge;
	handler->event_falling = on_falling_edge;

	return 0;
}

/**
 * Release an interrupt handler for an HSP GPIO.  This does not disable the interrupt.
 *
 * @param handler The interrupt handler to release.
 */
void host_gpio_irq_handler_release (const struct host_gpio_irq_handler *handler)
{
	UNUSED (handler);
}
