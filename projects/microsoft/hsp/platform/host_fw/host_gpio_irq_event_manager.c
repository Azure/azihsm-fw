// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "host_gpio_irq_event_manager.h"
#include "freertos/hsp_freertos.h"
#include "host_fw/host_irq_control.h"


/**
 * The number of pending notifications that is supported until events get lost.
 */
#define	HOST_GPIO_IRQ_EVENT_MANAGER_QUEUE_DEPTH		8


/**
 * Initialize a manager for sending notifications of GPIO interrupt events to a task for processing.
 *
 * @param notify The event manger to initialize.
 *
 * @return 0 if the manager was initialized successfully or an error code.
 */
int host_gpio_irq_event_manager_init (struct host_gpio_irq_event_manager *notify)
{
	if (notify == NULL) {
		return HOST_IRQ_CTRL_INVALID_ARGUMENT;
	}

	memset (notify, 0, sizeof (struct host_gpio_irq_event_manager));

	notify->event_queue = xQueueCreate (HOST_GPIO_IRQ_EVENT_MANAGER_QUEUE_DEPTH, sizeof (uint8_t));
	if (notify->event_queue == NULL) {
		return HOST_IRQ_CTRL_NO_MEMORY;
	}

	return 0;
}

/**
 * Release a GPIO interrupt event manager.
 *
 * @param notify The event manager to release.
 */
void host_gpio_irq_event_manager_release (struct host_gpio_irq_event_manager *notify)
{
	if (notify) {
		vQueueDelete (notify->event_queue);
	}
}

/**
 * Enable or disable all notifications in response to any GPIO interrupt event.
 *
 * @param notify The event manager to update.
 * @param enable true to enable event notifications or false to disable them.  When disabling
 * notifications, pending notifications will be discarded.
 */
void host_gpio_irq_event_manager_enable_notifications (struct host_gpio_irq_event_manager *notify,
	bool enable)
{
	if (notify) {
		notify->enable_notifications = enable;

		if (!enable) {
			/* Flush the queue to get rid of any notifications that have already been received that
			 * should be suppressed. */
			xQueueReset (notify->event_queue);
		}
	}
}

/**
 * Send a notification for a GPIO interrupt event.
 *
 * @param notify The event manager to notify.
 * @param event The event that should be reported.
 */
void host_gpio_irq_event_manager_send_notification (struct host_gpio_irq_event_manager *notify,
	uint8_t event)
{
	BaseType_t status;
	BaseType_t priority_woken = pdFALSE;

	if (notify) {
		/* Send notifications only if:
		 *  - Notifications are enabled for the manager.
		 *  - The event is not a SPI CS event or the event is different from the previous one. */
		if (notify->enable_notifications &&
			((event != notify->last_event) ||
			((event != GPIO_IRQ_CS0_ASSERTED) && (event != GPIO_IRQ_CS1_ASSERTED)))) {
			status = xQueueSendToBackFromISR (notify->event_queue, &event, &priority_woken);
			if (status == pdPASS) {
				notify->last_event = event;
			}

			freertos_isr_update_yield (priority_woken);
		}
	}
}
