// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "host_irq_control_hsp_gpio.h"
#include "common/unused.h"


int host_irq_control_hsp_gpio_enable_exit_reset (const struct host_irq_control *control,
	bool enable)
{
	const struct host_irq_control_hsp_gpio *hsp = (const struct host_irq_control_hsp_gpio*) control;

	if (hsp == NULL) {
		return HOST_IRQ_CTRL_INVALID_ARGUMENT;
	}

	if (enable) {
		return hsp_gpio_enable_interrupt (hsp->gpio, hsp->reset->gpio_num,
			HSP_GPIO_IRQ_RISING_EDGE | HSP_GPIO_IRQ_FALLING_EDGE, &hsp->reset->base);
	}
	else {
		return hsp_gpio_enable_interrupt (hsp->gpio, hsp->reset->gpio_num,
			HSP_GPIO_IRQ_FALLING_EDGE, &hsp->reset->base);
	}
}

int host_irq_control_hsp_gpio_enable_chip_selects (const struct host_irq_control *control,
	bool enable)
{
	const struct host_irq_control_hsp_gpio *hsp = (const struct host_irq_control_hsp_gpio*) control;
	int status;

	if (hsp == NULL) {
		return HOST_IRQ_CTRL_INVALID_ARGUMENT;
	}

	if ((hsp->cs0 == NULL) || (hsp->cs1 == NULL)) {
		return HOST_IRQ_CTRL_IRQ_NOT_SUPPORTED;
	}

	if (enable) {
		status = hsp_gpio_enable_interrupt (hsp->gpio, hsp->cs0->gpio_num,
			HSP_GPIO_IRQ_FALLING_EDGE, &hsp->cs0->base);
		if (status != 0) {
			return status;
		}

		status = hsp_gpio_enable_interrupt (hsp->gpio, hsp->cs1->gpio_num,
			HSP_GPIO_IRQ_FALLING_EDGE, &hsp->cs1->base);
	}
	else {
		status = hsp_gpio_disable_all_interrupts (hsp->gpio, hsp->cs0->gpio_num);
		if (status != 0) {
			return status;
		}

		status = hsp_gpio_disable_all_interrupts (hsp->gpio, hsp->cs1->gpio_num);
	}

	return status;
}

void host_irq_control_hsp_gpio_enable_notifications (const struct host_irq_control *control,
	bool enable)
{
	const struct host_irq_control_hsp_gpio *hsp = (const struct host_irq_control_hsp_gpio*) control;

	if (hsp == NULL) {
		return;
	}

	host_gpio_irq_event_manager_enable_notifications (hsp->notify, enable);
}

/**
 * Initialize an interface to control host interrupt generation from HSP GPIOs.  A single reset
 * indicator and SPI CS lines will be monitored for interrupts.  Interrupts will be enabled as part
 * of initialization.
 *
 * @param hsp The IRQ control interface to initialize.
 * @param gpio Driver for the HSP GPIOs that generate interrupts.  All GPIOs must be managed by the
 * same driver.
 * @param irq_notify Notification manager for HSP GPIO interrupts.
 * @param reset_irq IRQ handler for the host reset indicator.
 * @param cs0_irq IRQ handler for the host SPI CS0 signal.
 * @param cs1_irq IRQ handler for the host SPI CS1 signal.
 *
 * @return 0 if the interface was initialized successfully or an error code.
 */
int host_irq_control_hsp_gpio_init_with_cs_irq (struct host_irq_control_hsp_gpio *hsp,
	const struct hsp_gpio *gpio, struct host_gpio_irq_event_manager *irq_notify,
	const struct host_gpio_irq_handler *reset_irq, const struct host_gpio_irq_handler *cs0_irq,
	const struct host_gpio_irq_handler *cs1_irq)
{
	if ((hsp == NULL) || (gpio == NULL) || (irq_notify == NULL) || (reset_irq == NULL) ||
		(cs0_irq == NULL) || (cs1_irq == NULL)) {
		return HOST_IRQ_CTRL_INVALID_ARGUMENT;
	}

	memset (hsp, 0, sizeof (struct host_irq_control_hsp_gpio));

	hsp->base.enable_exit_reset = host_irq_control_hsp_gpio_enable_exit_reset;
	hsp->base.enable_chip_selects = host_irq_control_hsp_gpio_enable_chip_selects;
	hsp->base.enable_notifications = host_irq_control_hsp_gpio_enable_notifications;

	hsp->gpio = gpio;
	hsp->notify = irq_notify;
	hsp->reset = reset_irq;
	hsp->cs0 = cs0_irq;
	hsp->cs1 = cs1_irq;

	return host_irq_control_hsp_gpio_enable_irq (hsp);
}

/**
 * Initialize an interface to control host interrupt generation from HSP GPIOs.  Only reset
 * indicators will generate interrupts.  Interrupts will be enabled as part of initialization.
 *
 * @param hsp The IRQ control interface to initialize.
 * @param gpio Driver for the HSP GPIOs that generate interrupts.  All GPIOs must be managed by the
 * same driver.
 * @param irq_notify Notification manager for HSP GPIO interrupts.
 * @param reset_irq IRQ handler for the host reset indicator.
 * @param no_auth_irq IRQ handler for the indicator of host reset without authentication support.
 *
 * @return 0 if the interface was initialized successfully or an error code.
 */
int host_irq_control_hsp_gpio_init_reset_irq_only (struct host_irq_control_hsp_gpio *hsp,
	const struct hsp_gpio *gpio, struct host_gpio_irq_event_manager *irq_notify,
	const struct host_gpio_irq_handler *reset_irq, const struct host_gpio_irq_handler *no_auth_irq)
{
	if ((hsp == NULL) || (gpio == NULL) || (irq_notify == NULL) || (reset_irq == NULL) ||
		(no_auth_irq == NULL)) {
		return HOST_IRQ_CTRL_INVALID_ARGUMENT;
	}

	memset (hsp, 0, sizeof (struct host_irq_control_hsp_gpio));

	hsp->base.enable_exit_reset = host_irq_control_hsp_gpio_enable_exit_reset;
	hsp->base.enable_chip_selects = host_irq_control_hsp_gpio_enable_chip_selects;
	hsp->base.enable_notifications = host_irq_control_hsp_gpio_enable_notifications;

	hsp->gpio = gpio;
	hsp->notify = irq_notify;
	hsp->reset = reset_irq;
	hsp->reset_no_auth = no_auth_irq;

	return host_irq_control_hsp_gpio_enable_irq (hsp);
}

/**
 * Release an HSP GPIO host interrupt control interface.  Interrupts will be disabled.
 *
 * @param hsp The IRQ control interface to release.
 */
void host_irq_control_hsp_gpio_release (const struct host_irq_control_hsp_gpio *hsp)
{
	if (hsp) {
		hsp_gpio_disable_all_interrupts (hsp->gpio, hsp->reset->gpio_num);

		if (hsp->cs0) {
			hsp_gpio_disable_all_interrupts (hsp->gpio, hsp->cs0->gpio_num);
		}

		if (hsp->cs1) {
			hsp_gpio_disable_all_interrupts (hsp->gpio, hsp->cs1->gpio_num);
		}

		if (hsp->reset_no_auth) {
			hsp_gpio_disable_all_interrupts (hsp->gpio, hsp->reset_no_auth->gpio_num);
		}
	}
}

/**
 * Enable interrupts for all monitored GPIOs.
 *
 * @param hsp The IRQ control interface for interrupts that should be enabled.
 *
 * @return 0 if all interrupts were enabled successfully or an error code.
 */
int host_irq_control_hsp_gpio_enable_irq (const struct host_irq_control_hsp_gpio *hsp)
{
	int status;

	if ((hsp == NULL) || (hsp->gpio == NULL) || (hsp->notify == NULL) || (hsp->reset == NULL)) {
		return HOST_IRQ_CTRL_INVALID_ARGUMENT;
	}

	if ((hsp->reset_no_auth == NULL) && ((hsp->cs0 == NULL) || (hsp->cs1 == NULL))) {
		return HOST_IRQ_CTRL_INVALID_ARGUMENT;
	}

	if ((hsp->cs0 == NULL) && (hsp->reset_no_auth == NULL)) {
		return HOST_IRQ_CTRL_INVALID_ARGUMENT;
	}

	if (hsp->reset_no_auth) {
		status = hsp_gpio_enable_interrupt (hsp->gpio, hsp->reset_no_auth->gpio_num,
			HSP_GPIO_IRQ_RISING_EDGE | HSP_GPIO_IRQ_FALLING_EDGE, &hsp->reset_no_auth->base);
		if (status != 0) {
			return status;
		}
	}

	return hsp_gpio_enable_interrupt (hsp->gpio, hsp->reset->gpio_num, HSP_GPIO_IRQ_FALLING_EDGE,
		&hsp->reset->base);
}
