// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HOST_GPIO_IRQ_HANDLER_H_
#define HOST_GPIO_IRQ_HANDLER_H_

#include <stdbool.h>
#include <stdint.h>
#include "host_gpio_irq_event_manager.h"
#include "trap/hsp_interrupt_handler.h"


/**
 * CPU interrupt handler for host events triggered by HSP GPIOs.
 */
struct host_gpio_irq_handler {
	struct hsp_interrupt_handler base;			/**< Base API for handling interrupts. */
	struct host_gpio_irq_event_manager *notify;	/**< Notification manager for GPIO IRQs. */
	uint8_t gpio_num;							/**< The GPIO number generating interrupts. */
	uint8_t event_rising;						/**< Notification to trigger on a rising edge interrupt. */
	uint8_t event_falling;						/**< Notification to trigger on a falling edge interrupt. */
};


int host_gpio_irq_handler_init (struct host_gpio_irq_handler *handler,
	struct host_gpio_irq_event_manager *irq_notify, uint8_t gpio, uint8_t on_rising_edge,
	uint8_t on_falling_edge);
void host_gpio_irq_handler_release (const struct host_gpio_irq_handler *handler);


#endif	/* HOST_GPIO_IRQ_HANDLER_H_ */
