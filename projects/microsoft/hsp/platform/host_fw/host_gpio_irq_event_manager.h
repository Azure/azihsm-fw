// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HOST_GPIO_IRQ_EVENT_MANAGER_H_
#define HOST_GPIO_IRQ_EVENT_MANAGER_H_

#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"


/**
 * Notification values for different GPIO interrupt events.
 */
enum {
	GPIO_IRQ_RESET_ENTER = (1U << 0),	/**< An external host has entered reset and can be authenticated. */
	GPIO_IRQ_RESET_EXIT = (1U << 1),	/**< An external host has exited an authenticated reset. */
	GPIO_IRQ_CS0_ASSERTED = (1U << 2),	/**< An external host has asserted SPI flash CS0. */
	GPIO_IRQ_CS1_ASSERTED = (1U << 3),	/**< An external host has asserted SPI flash CS1. */
	GPIO_IRQ_HOST_DOWN = (1U << 4),		/**< An external host has entered reset but cannot be authenticated. */
	GPIO_IRQ_HOST_UP = (1U << 5),		/**< An external host has exited an unauthenticated reset.*/
};


/**
 * Manage notifications being sent to system tasks based on HSP GPIO interrupts.
 */
struct host_gpio_irq_event_manager {
	QueueHandle_t event_queue;	/**< The queue for reporting GPIO interrupts. */
	bool enable_notifications;	/**< Flag to enable delivery of IRQ notifications. */
	uint8_t last_event;			/**< Keep track of the last event received. */
};


int host_gpio_irq_event_manager_init (struct host_gpio_irq_event_manager *notify);
void host_gpio_irq_event_manager_release (struct host_gpio_irq_event_manager *notify);

void host_gpio_irq_event_manager_enable_notifications (struct host_gpio_irq_event_manager *notify,
	bool enable);

void host_gpio_irq_event_manager_send_notification (struct host_gpio_irq_event_manager *notify,
	uint8_t event);


#endif	/* HOST_GPIO_IRQ_EVENT_MANAGER_H_ */
