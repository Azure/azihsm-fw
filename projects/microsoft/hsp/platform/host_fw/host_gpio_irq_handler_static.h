// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HOST_GPIO_IRQ_HANDLER_STATIC_H_
#define HOST_GPIO_IRQ_HANDLER_STATIC_H_

#include "host_gpio_irq_handler.h"
#include "trap/hsp_interrupt_handler_static.h"


/* Internal functions declared to allow for static initialization. */
bool host_gpio_irq_handler_handle_interrupt (const struct hsp_interrupt_handler *handler,
	uintptr_t param);


/**
 * Initialize a static handler for interrupts from a single HSP GPIO.
 *
 * There is no validation done on the arguments.
 *
 * @param irq_notify Notification manager for HSP GPIO interrupts.
 * @param gpio GPIO number whose interrupts will be handled.
 * @param on_rising_edge Event identifier that should be generated on a rising edge interrupt.
 * @param on_falling_edge Event identifier that should be generated on a falling edge interrupt.
 */
#define	host_gpio_irq_handler_static_init(irq_notify, gpio, on_rising_edge, on_falling_edge)	{ \
		.base = hsp_interrupt_handler_static_init (host_gpio_irq_handler_handle_interrupt), \
		.notify = irq_notify, \
		.gpio_num = gpio, \
		.event_rising = on_rising_edge, \
		.event_falling = on_falling_edge, \
	}


#endif	/* HOST_GPIO_IRQ_HANDLER_STATIC_H_ */
