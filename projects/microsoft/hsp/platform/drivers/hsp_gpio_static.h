// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_GPIO_STATIC_H_
#define HSP_GPIO_STATIC_H_

#include "hsp_gpio.h"


/* Internal functions declared to allow for static initialization. */
bool hsp_gpio_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param);
bool hsp_gpio_handle_interrupt_no_irq_support (const struct hsp_interrupt_handler *handler,
	uintptr_t param);


/**
 * Constant initializer with GPIO IRQ support.
 */
#define	HSP_GPIO_IRQ_API_INIT	{ \
		.handle_interrupt = hsp_gpio_handle_interrupt \
	}

/**
 * Constant initializer without GPIO IRQ support.
 */
#define	HSP_GPIO_NO_IRQ_API_INIT	{ \
		.handle_interrupt = hsp_gpio_handle_interrupt_no_irq_support \
	}


/**
 * Initialize a static GPIO driver instance.
 *
 * There is no validation done on the arguments.
 *
 * @param gpio_regs_ptr Register interface for the HSP GPIOs.
 * @param irq_handler_ptr Array of interrupt handlers to use for GPIO interrupts.
 * @param gpio_count The total number of GPIOs supported by the HSP.  The irq_handler array must be
 * at least this size.
 */
#define	hsp_gpio_static_init(gpio_regs_ptr, irq_handler_ptr, gpio_count)	{ \
		.base = HSP_GPIO_IRQ_API_INIT, \
		.regs = gpio_regs_ptr, \
		.irq_handler = irq_handler_ptr, \
		.count = gpio_count \
	}

/**
 * Initialize a static GPIO driver instance that allows for reading and writing GPIO values but does
 * not support GPIO driven interrupts.
 *
 * There is no validation done on the arguments.
 *
 * @param gpio_regs_ptr Register interface for the HSP GPIOs.
 * @param gpio_count The total number of GPIOs supported by the HSP.
 */
#define	hsp_gpio_static_init_no_irq_support(gpio_regs_ptr, gpio_count)	{ \
		.base = HSP_GPIO_NO_IRQ_API_INIT, \
		.regs = gpio_regs_ptr, \
		.irq_handler = NULL, \
		.count = gpio_count \
	}


#endif	/* HSP_GPIO_STATIC_H_ */
