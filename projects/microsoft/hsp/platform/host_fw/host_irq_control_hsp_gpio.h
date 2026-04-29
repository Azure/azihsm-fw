// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HOST_IRQ_CONTROL_HSP_GPIO_H_
#define HOST_IRQ_CONTROL_HSP_GPIO_H_

#include <stdbool.h>
#include "host_gpio_irq_event_manager.h"
#include "host_gpio_irq_handler.h"
#include "drivers/hsp_gpio.h"
#include "host_fw/host_irq_control.h"


/**
 * Interface for controlling external host events that are reported using HSP GPIOs.
 */
struct host_irq_control_hsp_gpio {
	struct host_irq_control base;						/**< Base IRQ control API. */
	const struct hsp_gpio *gpio;						/**< Driver for the HSP GPIOs that generate interrupts. */
	struct host_gpio_irq_event_manager *notify;			/**< Notification manager for interrupts from HSP GPIOs. */
	const struct host_gpio_irq_handler *reset;			/**< Interrupt handler for host reset events. */
	const struct host_gpio_irq_handler *reset_no_auth;	/**< Interrupt handler for host resets without authentication. */
	const struct host_gpio_irq_handler *cs0;			/**< Interrupt handler for SPI CS0 assertion. */
	const struct host_gpio_irq_handler *cs1;			/**< Interrupt handler for SPI CS1 assertion. */
};


int host_irq_control_hsp_gpio_init_with_cs_irq (struct host_irq_control_hsp_gpio *hsp,
	const struct hsp_gpio *gpio, struct host_gpio_irq_event_manager *irq_notify,
	const struct host_gpio_irq_handler *reset_irq, const struct host_gpio_irq_handler *cs0_irq,
	const struct host_gpio_irq_handler *cs1_irq);
int host_irq_control_hsp_gpio_init_reset_irq_only (struct host_irq_control_hsp_gpio *hsp,
	const struct hsp_gpio *gpio, struct host_gpio_irq_event_manager *irq_notify,
	const struct host_gpio_irq_handler *reset_irq, const struct host_gpio_irq_handler *no_auth_irq);
void host_irq_control_hsp_gpio_release (const struct host_irq_control_hsp_gpio *hsp);

int host_irq_control_hsp_gpio_enable_irq (const struct host_irq_control_hsp_gpio *hsp);


#endif	/* HOST_IRQ_CONTROL_HSP_GPIO_H_ */
