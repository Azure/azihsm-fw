// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "host_control_hsp_gpio.h"
#include "common/unused.h"


int host_control_hsp_gpio_hold_processor_in_reset (const struct host_control *control, bool reset)
{
	const struct host_control_hsp_gpio *hsp = (const struct host_control_hsp_gpio*) control;

	if (hsp == NULL) {
		return HOST_CONTROL_INVALID_ARGUMENT;
	}

	return hsp_gpio_write (hsp->gpio, hsp->reset_ctrl, !reset);
}

int host_control_hsp_gpio_is_processor_held_in_reset (const struct host_control *control)
{
	const struct host_control_hsp_gpio *hsp = (const struct host_control_hsp_gpio*) control;
	int value;

	if (hsp == NULL) {
		return HOST_CONTROL_INVALID_ARGUMENT;
	}

	value = hsp_gpio_read (hsp->gpio, hsp->reset_ctrl);
	if ((value != 0) && (value != 1)) {
		return value;
	}

	return !value;
}

int host_control_hsp_gpio_is_processor_in_reset (const struct host_control *control)
{
	const struct host_control_hsp_gpio *hsp = (const struct host_control_hsp_gpio*) control;
	int value;

	if (hsp == NULL) {
		return HOST_CONTROL_INVALID_ARGUMENT;
	}

	value = hsp_gpio_read (hsp->gpio, hsp->reset_state);
	if ((value != 0) && (value != 1)) {
		return value;
	}

	return !value;
}

int host_control_hsp_gpio_enable_processor_flash_access (const struct host_control *control,
	bool enable)
{
	const struct host_control_hsp_gpio *hsp = (const struct host_control_hsp_gpio*) control;

	if (hsp == NULL) {
		return HOST_CONTROL_INVALID_ARGUMENT;
	}

	return hsp_gpio_write (hsp->gpio, hsp->spi_mux, enable);
}

int host_control_hsp_gpio_processor_has_flash_access (const struct host_control *control)
{
	const struct host_control_hsp_gpio *hsp = (const struct host_control_hsp_gpio*) control;

	if (hsp == NULL) {
		return HOST_CONTROL_INVALID_ARGUMENT;
	}

	return hsp_gpio_read (hsp->gpio, hsp->spi_mux);
}

/**
 * Initialize the API for using HSP GPIOs to control host hardware.
 *
 * @param control The API instance to initialize.
 * @param gpio Driver for the HSP GPIOs that provide control over host hardware.  All GPIOs must be
 * managed by the same driver.
 * @param reset_ctrl The GPIO number for host processor reset control.
 * @param reset_ind The GPIO number that indicates when the processor is in reset.
 * @param mux_sel The GPIO number for the SPI filter mux select.
 *
 * @return 0 if the API instance was successfully initialized or an error code.
 */
int host_control_hsp_gpio_init (struct host_control_hsp_gpio *control, const struct hsp_gpio *gpio,
	uint8_t reset_ctrl, uint8_t reset_ind, uint8_t mux_sel)
{
	if ((control == NULL) || (gpio == NULL)) {
		return HOST_CONTROL_INVALID_ARGUMENT;
	}

	memset (control, 0, sizeof (struct host_control_hsp_gpio));

	control->base.hold_processor_in_reset = host_control_hsp_gpio_hold_processor_in_reset;
	control->base.is_processor_held_in_reset = host_control_hsp_gpio_is_processor_held_in_reset;
	control->base.is_processor_in_reset = host_control_hsp_gpio_is_processor_in_reset;
	control->base.enable_processor_flash_access =
		host_control_hsp_gpio_enable_processor_flash_access;
	control->base.processor_has_flash_access = host_control_hsp_gpio_processor_has_flash_access;

	control->gpio = gpio;
	control->reset_ctrl = reset_ctrl;
	control->reset_state = reset_ind;
	control->spi_mux = mux_sel;

	return 0;
}

/**
 * Release the resources used by an HSP GPIO host control API instance.
 *
 * @param control The API instance to release.
 */
void host_control_hsp_gpio_release (const struct host_control_hsp_gpio *control)
{
	UNUSED (control);
}
