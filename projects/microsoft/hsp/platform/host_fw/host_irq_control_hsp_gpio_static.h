// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HOST_IRQ_CONTROL_HSP_GPIO_STATIC_H_
#define HOST_IRQ_CONTROL_HSP_GPIO_STATIC_H_

#include "host_irq_control_hsp_gpio.h"


/* Internal functions declared to allow for static initialization. */
int host_irq_control_hsp_gpio_enable_exit_reset (const struct host_irq_control *control,
	bool enable);
int host_irq_control_hsp_gpio_enable_chip_selects (const struct host_irq_control *control,
	bool enable);
void host_irq_control_hsp_gpio_enable_notifications (const struct host_irq_control *control,
	bool enable);


/**
 * Constant initializer for the host IRQ control API.
 */
#define	HOST_IRQ_CONTROL_HSP_GPIO_API_INIT	{ \
		.enable_exit_reset = host_irq_control_hsp_gpio_enable_exit_reset, \
		.enable_chip_selects = host_irq_control_hsp_gpio_enable_chip_selects, \
		.enable_notifications = host_irq_control_hsp_gpio_enable_notifications \
	}


/**
 * Initialize a static host IRQ control interface using HSP GPIOs.  A single reset indicator and SPI
 * CS lines will be monitored for interrupts.
 *
 * There is no validation done on the arguments.
 *
 * @param gpio_ptr Driver for the HSP GPIOs that generate interrupts.  All GPIOs must be managed by
 * the same driver.
 * @param irq_notify Notification manager for HSP GPIO interrupts.
 * @param reset_irq IRQ handler for the host reset indicator.
 * @param cs0_irq IRQ handler for the host SPI CS0 signal.
 * @param cs1_irq IRQ handler for the host SPI CS1 signal.
 */
#define	host_irq_control_hsp_gpio_static_init_with_cs_irq(gpio_ptr, irq_notify, reset_irq, \
	cs0_irq, cs1_irq)	{ \
		.base = HOST_IRQ_CONTROL_HSP_GPIO_API_INIT, \
		.gpio = gpio_ptr, \
		.notify = irq_notify, \
		.reset = reset_irq, \
		.reset_no_auth = NULL, \
		.cs0 = cs0_irq, \
		.cs1 = cs1_irq, \
	}

/**
 * Initialize a static host IRQ control interface using HSP GPIOs.  Only reset indicators will
 * generate interrupts.
 *
 * There is no validation done on the arguments.
 *
 * @param gpio_ptr Driver for the HSP GPIOs that generate interrupts.  All GPIOs must be managed by
 * the same driver.
 * @param irq_notify Notification manager for HSP GPIO interrupts.
 * @param reset_irq IRQ handler for the host reset indicator.
 * @param no_auth_irq IRQ handler for the indicator of host reset without authentication support.
 */
#define	host_irq_control_hsp_gpio_static_init_reset_irq_only(gpio_ptr, irq_notify, reset_irq, \
	no_auth_irq)	{ \
		.base = HOST_IRQ_CONTROL_HSP_GPIO_API_INIT, \
		.gpio = gpio_ptr, \
		.notify = irq_notify, \
		.reset = reset_irq, \
		.reset_no_auth = no_auth_irq, \
		.cs0 = NULL, \
		.cs1 = NULL, \
	}


#endif	/* HOST_IRQ_CONTROL_HSP_GPIO_STATIC_H_ */
